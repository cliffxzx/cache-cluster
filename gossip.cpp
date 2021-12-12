#include <algorithm>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/asio.hpp>
#include <boost/core/demangle.hpp>
#include <boost/log/trivial.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <deque>
#include <iostream>
#include <map>
#include <queue>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "gossip.hpp"

using boost::archive::text_iarchive;
using boost::asio::buffer;
using boost::asio::dynamic_buffer;
using boost::asio::io_service;
using boost::asio::ip::address;
using boost::asio::ip::port_type;
using boost::asio::ip::udp;
using boost::core::demangle;
using boost::system::error_code;
using gossip::Member;
using gossip::message::Hello;
using gossip::message::IMessages;
using gossip::message::IMessages_Ptr;
using gossip::message::Message;
using gossip::message::Welcome;
using std::async;
using std::future;
using std::istringstream;
using std::make_shared;
using std::mt19937;
using std::random_device;
using std::set;
using std::shared_future;
using std::shared_ptr;
using std::string;
using std::thread;
using std::chrono::milliseconds;
using std::this_thread::sleep_for;

namespace gossip {

Gossip::Gossip(const Member t_self_member,
               const ReceiverFn t_receiver)
    : m_self_member(make_shared<Member>(t_self_member)),
      m_socket(m_context, t_self_member.address()),
      m_receiver(t_receiver) {}

void Gossip::run() {
  try {
    while (m_state != State::DESTROYED) {
      if (m_context.stopped()) {
        m_context.restart();
      }

      if (!m_context.poll()) {
        m_receive_handler();
        m_send_handler();

        sleep_for(milliseconds(gossip_tick_interval()));
      }
    }
  } catch (const std::exception &ex) {
    std::cerr << ex.what() << std::endl;
  }
}

template <IMessages IMessage>
Error Gossip::enqueue_message(const IMessage t_message,
                              const Spreading t_spreading,
                              const Member::shared_ptr t_member) {
  Message::shared_ptr message = make_shared<IMessage>(t_message);
  message->m_header.remain_attempt = message_retry_attempts();

  switch (t_spreading) {
    case Spreading::DIRECT:
      message->m_header.destination = t_member;
      m_message.insert(message);
      return Error::NONE;
    case Spreading::RANDOM: {
      decltype(m_memberlist) reservoir;
      sample(m_memberlist.begin(), m_memberlist.end(),
             inserter(reservoir, reservoir.begin()),
             this->message_rumor_factor(),
             mt19937{random_device{}()});
      for (auto member : reservoir) {
        message->m_header.destination = t_member;
        m_message.insert(message);
      }
      return Error::NONE;
    }
    case Spreading::BROADCAST: {
      for (auto member : m_memberlist) {
        message->m_header.destination = t_member;
        m_message.insert(message);
      }
      return Error::NONE;
    }
  }
}

template Error Gossip::enqueue_message(const Welcome t_message,
                                       const Spreading t_spreading,
                                       const Member::shared_ptr t_member);

Error Gossip::add_member(const Member t_member) {
  if (m_state != State::INITIALIZED)
    return Error::BAD_STATE;

  auto member = make_shared<Member>(Member(t_member));
  Error res = enqueue_message(Hello(self_member()), Spreading::DIRECT, member);
  if ((uint32_t)res < 0)
    return res;

  this->m_state = State::JOINING;
  return Error::NONE;
}

Error Gossip::m_receive(const string t_data, const Member t_sender) {
  BOOST_LOG_TRIVIAL(debug) << "Gossip::m_receive_hander:"
                           << "\t[address]:" << t_sender.address();

  istringstream iss(t_data);
  text_iarchive ia(iss);
  Message::shared_ptr message;
  ia >> message;
  BOOST_LOG_TRIVIAL(trace) << "Gossip::m_receive:"
                           << "\t -> " << demangle(typeid(*message.get()).name());

  message->receive(*this, m_temp_sender);

  return Error::NONE;
}

template <IMessages_Ptr IMessage_Ptr>
Error Gossip::m_send(IMessage_Ptr t_message) {
  string message = to_string(t_message);
  BOOST_LOG_TRIVIAL(debug) << "Gossip::m_send:"
                           << "\t[address]:" << t_message->m_header.destination->address();

  m_socket.async_send_to(
      buffer(message),
      t_message->m_header.destination->address(),
      [this](boost::system::error_code ec, std::size_t length) {
        if (ec)
          return;
      });

  return Error::NONE;
}

void Gossip::m_receive_handler() {
  if (m_state != State::JOINING && m_state != State::CONNECTED)
    return;

  m_temp_recv_buffer.clear();
  m_socket.async_receive_from(
      dynamic_buffer(m_temp_recv_buffer).prepare(max_output_messages()),
      m_temp_sender,
      [this](const error_code ec, const size_t length) {
        if (ec) {
          BOOST_LOG_TRIVIAL(error) << "Gossip::m_receive_handler:"
                                   << "\t[address]:" << ec.message();

          return;
        }

        m_receive(m_temp_recv_buffer.substr(0, length), m_temp_sender);
      });
}

void Gossip::m_send_handler() {
  if (this->m_state != State::JOINING && this->m_state != State::CONNECTED)
    return;

  while (!this->m_message.empty()) {
    BOOST_LOG_TRIVIAL(trace) << "Gossip::m_send_handler:"
                             << "\t -> " << demangle(typeid(**m_message.begin()).name());
    Message message = **m_message.begin();
    if (message.m_header.remain_attempt <= 0) {
      if (message_retry_attempts() > 1) {
        m_memberlist.erase(message.m_header.destination);
      }

      this->m_message.erase(this->m_message.begin());
      continue;
    }

    m_send(*m_message.begin());
    this->m_message.erase(this->m_message.begin());
  }

  return;
}

int32_t &Gossip::message_retry_interval() { return m_message_retry_interval; }
const int32_t &Gossip::message_retry_interval() const { return m_message_retry_interval; }

int32_t &Gossip::message_retry_attempts() { return m_message_retry_attempts; }
const int32_t &Gossip::message_retry_attempts() const { return m_message_retry_attempts; }

int32_t &Gossip::message_rumor_factor() { return m_message_rumor_factor; }
const int32_t &Gossip::message_rumor_factor() const { return m_message_rumor_factor; }

int32_t &Gossip::message_max_size() { return m_message_max_size; }
const int32_t &Gossip::message_max_size() const { return m_message_max_size; }

int32_t &Gossip::max_output_messages() { return m_max_output_messages; }
const int32_t &Gossip::max_output_messages() const { return m_max_output_messages; }

int32_t &Gossip::gossip_tick_interval() { return m_gossip_tick_interval; }
const int32_t &Gossip::gossip_tick_interval() const { return m_gossip_tick_interval; }

const Member::shared_ptr &Gossip::self_member() const { return m_self_member; }
}; // namespace gossip