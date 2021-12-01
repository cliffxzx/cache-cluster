#include <algorithm>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/asio.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/thread.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_serialize.hpp>
#include <functional>
#include <iostream>
#include <iterator>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "gossip_protocol.hpp"
#include "message.hpp"
#include "utils.hpp"

using namespace std;
using namespace boost;
using namespace boost::asio;

namespace GossipProtocal {

Member::Member() : uid_(boost::uuids::random_generator()()) {}
Member::Member(ip::udp::endpoint t_addr) : uid_(boost::uuids::random_generator()()), addr_(t_addr) {}
Member::Member(ip::address addr, ip::port_type port) : uid_(boost::uuids::random_generator()()),
                                                       addr_(ip::udp::endpoint(addr, port)) {}

template <class Archive>
void Member::serialize(Archive &ar, const unsigned int version) {
  ar &uid_;
  ar &addr_;
}

template <class Archive>
void VectorRecord::serialize(Archive &ar, const unsigned int version) {
  ar &(this->sequence_number);
  ar &(this->member_id);
}

template <class Archive>
void VectorClock::serialize(Archive &ar, const unsigned int version) {
  ar &(this->current_idx);
  ar &(this->records);
}

GossipProtocal::GossipProtocal(ip::address addr, ip::port_type port)
    : endpoint_(ip::udp::endpoint(addr, port)),
      socket_(service_, endpoint_),
      state(State::INITIALIZED) {
}

void GossipProtocal::run() {
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

void GossipProtocal::handle_hello(Message::Hello hello, ip::udp::endpoint sender) {
  Message::Welcome welcome;
  welcome.hello_sequence_num = hello.header.sequence_num;
  welcome.header.destination = Member(sender);
  enqueue_message(welcome, Member(sender), SpreadingType::DIRECT);

  memberlist_.push_back(welcome.header.destination);
}

GossipProtocal::Error GossipProtocal::receive() {
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
        Message::Header temp;
        ia >> temp;
        switch (temp.message_type) {
          case (uint8_t)Message::Type::Hello: {

            Message::Hello test;
            ia2 >> test;
            handle_hello(test, sender);
            return Error::NONE;
          }
            // case (uint8_t)Message::Type::Welcome:
            //   ia2 >> temp;
            //   return Error::NONE;
            // case (uint8_t)Message::Type::Memberlist:
            //   ia2 >> temp;
            //   return Error::NONE;
            // case (uint8_t)Message::Type::Data:
            //   ia2 >> temp;
            //   return Error::NONE;
            // case (uint8_t)Message::Type::Ack:
            //   ia2 >> temp;
            //   return Error::NONE;
            // case (uint8_t)Message::Type::Status:
            //   ia2 >> temp;
            //   return Error::NONE;
        }
        return Error::INVALID_MESSAGE;
      });

  return Error::NONE;
}

GossipProtocal::Error GossipProtocal::send() {
  if (this->state != State::JOINING && this->state != State::CONNECTED)
    return Error::BAD_STATE;

  while (!this->message_.empty()) {
    Message::Message message(this->message_.back());
    this->message_.pop_back();
    if (message.header.attempt_num < 1)
      continue;

    send_(message);
  }

  return Error::NONE;
}

GossipProtocal::Error GossipProtocal::enqueue_message(Message::Message t_message, Member t_member, SpreadingType t_spreading_type) {
  Message::Message message = t_message;
  message.header.destination = t_member;
  message.header.attempt_num = message_retry_attempts();

  switch (t_spreading_type) {
    case SpreadingType::DIRECT:
      message_.push_back(message);
      return GossipProtocal::Error::NONE;
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
      return GossipProtocal::Error::NONE;
    }
    case SpreadingType::BROADCAST: {
      for (const Member member : memberlist_) {
        message_.push_back(message);
      }
      return GossipProtocal::Error::NONE;
    }
  }
}

// Todo hello to every member:270
template <class InputIterator>
inline GossipProtocal::Error GossipProtocal::add_members(const InputIterator first, const InputIterator last) {
  if (this->state != State::INITIALIZED)
    return GossipProtocal::Error::BAD_STATE;

  if (last - first == 0) {
    this->state = State::CONNECTED;
    return GossipProtocal::Error::NONE;
  }

  for (auto it = first; it != last; ++it) {
    Member member = *it;
    Error res = enqueue_message(Message::Hello(), member, SpreadingType::DIRECT);
    if ((uint32_t)res < 0)
      return res;
  }

  this->state = State::JOINING;
  return GossipProtocal::Error::NONE;
}

template GossipProtocal::Error GossipProtocal::add_members(
    const vector<Member>::iterator first,
    const vector<Member>::iterator last);

/** The interval in milliseconds between retry attempts. */
int32_t &GossipProtocal::message_retry_interval() { return message_retry_interval_; }
const int32_t &GossipProtocal::message_retry_interval() const { return message_retry_interval_; }

/** The maximum number of attempts to deliver a message. */
int32_t &GossipProtocal::message_retry_attempts() { return message_retry_attempts_; }
const int32_t &GossipProtocal::message_retry_attempts() const { return message_retry_attempts_; }

/** The number of members that are used for further gossip propagation. */
int32_t &GossipProtocal::message_rumor_factor() { return message_rumor_factor_; }
const int32_t &GossipProtocal::message_rumor_factor() const { return message_rumor_factor_; }

/** The maximum supported size of the message including a protocol overhead. */
int32_t &GossipProtocal::message_max_size() { return message_max_size_; }
const int32_t &GossipProtocal::message_max_size() const { return message_max_size_; }

/** The maximum number of unique messages that can be stored in the outbound message queue. */
int32_t &GossipProtocal::max_output_messages() { return max_output_messages_; }
const int32_t &GossipProtocal::max_output_messages() const { return max_output_messages_; }

/** The time interval in milliseconds that determines how often the Gossip tick event should be triggered. */
int32_t &GossipProtocal::gossip_tick_interval() { return gossip_tick_interval_; }
const int32_t &GossipProtocal::gossip_tick_interval() const { return gossip_tick_interval_; }

void GossipProtocal::send_(Message::Message t_message) {
  string message = string(t_message) + '\n';
  socket_.async_send_to(
      buffer(message), t_message.header.destination.addr_,
      [this](boost::system::error_code ec, std::size_t length) {
        if (ec)
          return;
      });
}
}; // namespace GossipProtocal