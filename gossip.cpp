#include <algorithm>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/asio.hpp>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "gossip.hpp"

using namespace std;
using namespace boost;
using namespace boost::asio;
using namespace gossip;
using namespace gossip::message;

namespace gossip {

Gossip::Gossip(ip::address addr, ip::port_type port)
    : endpoint_(ip::udp::endpoint(addr, port)),
      socket_(service_, endpoint_),
      state(State::INITIALIZED) {
}

void Gossip::run() {
  while (state != State::DESTROYED) {
    if (service_.stopped()) {
      service_.restart();
    }

    if (!service_.poll()) {
      receive();
      send();

      std::this_thread::sleep_for(std::chrono::milliseconds(gossip_tick_interval()));
    }
  }
}

void Gossip::handle_hello(Hello hello, ip::udp::endpoint sender) {
  Welcome welcome;
  welcome.hello_sequence_num = hello.header.sequence_num;
  welcome.header.destination = Member(sender);
  enqueue_message(welcome, Member(sender), SpreadingType::DIRECT);

  memberlist_.push_back(welcome.header.destination);
}

Gossip::Error Gossip::receive() {
  std::string recv_buffer;
  ip::udp::endpoint sender;
  socket_.async_receive_from(
      dynamic_buffer(recv_buffer).prepare(max_output_messages()),
      sender,
      [this, sender, recv_buffer](system::error_code ec, size_t length) {
        string recv_buffer_ = recv_buffer.substr(0, recv_buffer.find('\n'));
        istringstream iss(recv_buffer_),
            is2(recv_buffer_);
        archive::text_iarchive ia(iss), ia2(is2);
        Header temp;
        ia >> temp;
        switch (temp.message_type) {
          case (uint8_t)Type::Hello: {

            Hello test;
            ia2 >> test;
            handle_hello(test, sender);
            return Error::NONE;
          }
            // case (uint8_t)Type::Welcome:
            //   ia2 >> temp;
            //   return Error::NONE;
            // case (uint8_t)Type::Memberlist:
            //   ia2 >> temp;
            //   return Error::NONE;
            // case (uint8_t)Type::Data:
            //   ia2 >> temp;
            //   return Error::NONE;
            // case (uint8_t)Type::Ack:
            //   ia2 >> temp;
            //   return Error::NONE;
            // case (uint8_t)Type::Status:
            //   ia2 >> temp;
            //   return Error::NONE;
        }
        return Error::INVALID_MESSAGE;
      });

  return Error::NONE;
}

Gossip::Error Gossip::send() {
  if (this->state != State::JOINING && this->state != State::CONNECTED)
    return Error::BAD_STATE;

  while (!this->message_.empty()) {
    Message message(this->message_.back());
    this->message_.pop_back();
    if (message.header.attempt_num < 1)
      continue;

    send_(message);
  }

  return Error::NONE;
}

Gossip::Error Gossip::enqueue_message(Message t_message, Member t_member, SpreadingType t_spreading_type) {
  Message message = t_message;
  message.header.destination = t_member;
  message.header.attempt_num = message_retry_attempts();

  switch (t_spreading_type) {
    case SpreadingType::DIRECT:
      message_.push_back(message);
      return Gossip::Error::NONE;
    case SpreadingType::RANDOM: {
      vector<Member> reservoir;
      reservoir.reserve(this->message_rumor_factor());
      sample(memberlist_.begin(), memberlist_.end(),
             back_inserter(reservoir),
             this->message_rumor_factor(),
             std::mt19937{std::random_device{}()});
      for (const Member member : reservoir) {
        message_.push_back(message);
      }
      return Gossip::Error::NONE;
    }
    case SpreadingType::BROADCAST: {
      for (const Member member : memberlist_) {
        message_.push_back(message);
      }
      return Gossip::Error::NONE;
    }
  }
}

// Todo hello to every member:270
template <class InputIterator>
inline Gossip::Error Gossip::add_members(const InputIterator first, const InputIterator last) {
  if (this->state != State::INITIALIZED)
    return Gossip::Error::BAD_STATE;

  if (last - first == 0) {
    this->state = State::CONNECTED;
    return Gossip::Error::NONE;
  }

  for (auto it = first; it != last; ++it) {
    Member member = *it;
    Error res = enqueue_message(Hello(), member, SpreadingType::DIRECT);
    if ((uint32_t)res < 0)
      return res;
  }

  this->state = State::JOINING;
  return Gossip::Error::NONE;
}

template Gossip::Error Gossip::add_members(
    const vector<Member>::iterator first,
    const vector<Member>::iterator last);

/** The interval in milliseconds between retry attempts. */
int32_t &Gossip::message_retry_interval() { return message_retry_interval_; }
const int32_t &Gossip::message_retry_interval() const { return message_retry_interval_; }

/** The maximum number of attempts to deliver a message. */
int32_t &Gossip::message_retry_attempts() { return message_retry_attempts_; }
const int32_t &Gossip::message_retry_attempts() const { return message_retry_attempts_; }

/** The number of members that are used for further gossip propagation. */
int32_t &Gossip::message_rumor_factor() { return message_rumor_factor_; }
const int32_t &Gossip::message_rumor_factor() const { return message_rumor_factor_; }

/** The maximum supported size of the message including a gossip overhead. */
int32_t &Gossip::message_max_size() { return message_max_size_; }
const int32_t &Gossip::message_max_size() const { return message_max_size_; }

/** The maximum number of unique messages that can be stored in the outbound message queue. */
int32_t &Gossip::max_output_messages() { return max_output_messages_; }
const int32_t &Gossip::max_output_messages() const { return max_output_messages_; }

/** The time interval in milliseconds that determines how often the Gossip tick event should be triggered. */
int32_t &Gossip::gossip_tick_interval() { return gossip_tick_interval_; }
const int32_t &Gossip::gossip_tick_interval() const { return gossip_tick_interval_; }

void Gossip::send_(Message t_message) {
  string message = string(t_message) + '\n';
  socket_.async_send_to(
      buffer(message), t_message.header.destination.address(),
      [this](boost::system::error_code ec, std::size_t length) {
        if (ec)
          return;
      });
}
}; // namespace GossipProtocal