#include <algorithm>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <deque>
#include <map>
#include <queue>
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

Gossip::Gossip(ip::address addr, ip::port_type port, ReceiverFn t_receiver)
    : self_(ip::udp::endpoint(addr, port)),
      socket_(service_, self_.address()),
      state(State::INITIALIZED),
      receiver_fn_(t_receiver) {
}

void Gossip::run() {
  while (state != State::DESTROYED) {
    if (service_.stopped()) {
      service_.restart();
    }

    if (!service_.poll()) {
      receive_();
      handle_send();

      std::this_thread::sleep_for(std::chrono::milliseconds(gossip_tick_interval()));
    }
  }
}

Gossip::Error Gossip::handle_receive(std::shared_ptr<Message> &t_message, ip::udp::endpoint t_sender) {
  switch (t_message->header.type) {
    case Type::Hello: {
      auto member = std::make_shared<Member>(self());
      auto sender = std::make_shared<Member>(t_sender);
      std::shared_ptr<Message> welcome = std::make_shared<Welcome>(member, t_message->header.sequence);
      enqueue_message(welcome, sender, SpreadingType::DIRECT);

      memberlist_.emplace(to_string(sender->uid()), sender);

      std::shared_ptr<Message> memberlist = std::make_shared<Memberlist>(memberlist_);
      enqueue_message(memberlist, sender, SpreadingType::BROADCAST);

      break;
    }
    case Type::Welcome: {
      state = State::CONNECTED;
      std::shared_ptr<Welcome> welcome = std::dynamic_pointer_cast<Welcome>(t_message);
      auto sender = std::make_shared<Member>(t_sender);
      memberlist_.emplace(to_string(sender->uid()), sender);
      message_.erase(find_if(message_.begin(), message_.end(), [welcome](std::shared_ptr<Message> val) { return val->header.sequence == welcome->hello_sequence; }));

      break;
    }
    case Type::Memberlist: {
      std::shared_ptr<Memberlist> memberlist = std::dynamic_pointer_cast<Memberlist>(t_message);
      for (auto member : memberlist->members) {
        auto m_member = std::make_shared<Member>(member);
        memberlist_.emplace(to_string(member.uid()), m_member);
      }

      auto sender = std::make_shared<Member>(t_sender);
      std::shared_ptr<Message> ack = std::make_shared<Ack>(t_message->header.sequence);
      enqueue_message(ack, sender, SpreadingType::DIRECT);

      break;
    }
    case Type::Ack: {
      std::shared_ptr<Ack> ack = std::dynamic_pointer_cast<Ack>(t_message);

      message_.erase(find_if(message_.begin(), message_.end(), [ack](std::shared_ptr<Message> val) { return val->header.sequence == ack->ack_sequence; }));

      break;
    }
    case Type::Data: {
      std::shared_ptr<Data> data = std::dynamic_pointer_cast<Data>(t_message);
      std::shared_ptr<Message> m_data = data;
      auto sender = std::make_shared<Member>(t_sender);
      enqueue_message(m_data, sender, SpreadingType::RANDOM);
      receiver_fn_(data->data);
      break;
    }
  }
}

Gossip::Error Gossip::handle_send() {
  if (this->state != State::JOINING && this->state != State::CONNECTED)
    return Error::BAD_STATE;

  while (!this->message_.empty()) {
    Message message = *message_.front();
    if (message.header.remain_attempt <= 0) {
      if (message_retry_attempts() > 1) {
        memberlist_.erase(to_string(message.header.destination->uid()));
      }

      this->message_.pop_front();
      continue;
    }

    send_(message_.front());
    message_.pop_front();
  }

  return Error::NONE;
}

Gossip::Error Gossip::enqueue_message(std::shared_ptr<Message> &t_message, std::shared_ptr<Member> &t_member, SpreadingType t_spreading_type) {
  t_message->header.remain_attempt = message_retry_attempts();

  switch (t_spreading_type) {
    case SpreadingType::DIRECT:
      t_message->header.destination = t_member;
      message_.push_back(t_message);
      return Gossip::Error::NONE;
    case SpreadingType::RANDOM: {
      decltype(memberlist_) reservoir;
      sample(memberlist_.begin(), memberlist_.end(),
             inserter(reservoir, reservoir.begin()),
             this->message_rumor_factor(),
             std::mt19937{std::random_device{}()});
      for (auto member : reservoir) {
        t_message->header.destination = member.second;
        message_.push_back(t_message);
      }
      return Gossip::Error::NONE;
    }
    case SpreadingType::BROADCAST: {
      for (auto member : memberlist_) {
        t_message->header.destination = member.second;
        message_.push_back(t_message);
      }
      return Gossip::Error::NONE;
    }
  }
}

Gossip::Error Gossip::add_member(Member t_member) {
  if (state != State::INITIALIZED)
    return Gossip::Error::BAD_STATE;

  auto temp = std::make_shared<Hello>(std::make_shared<Member>(self()));
  std::shared_ptr<Message> hello = temp;
  hello->header.remain_attempt = message_retry_attempts();
  auto member = std::make_shared<Member>(Member(t_member));
  Error res = enqueue_message(hello, member, SpreadingType::DIRECT);
  if ((uint32_t)res < 0)
    return res;

  this->state = State::JOINING;
  return Gossip::Error::NONE;
}

void Gossip::send(string t_data) {
  std::shared_ptr<Member> member = nullptr;
  std::shared_ptr<Message> data = std::make_shared<Data>(t_data);
  enqueue_message(data, member, SpreadingType::RANDOM);
}

int32_t &Gossip::message_retry_interval() {
  return message_retry_interval_;
}
const int32_t &Gossip::message_retry_interval() const {
  return message_retry_interval_;
}

int32_t &Gossip::message_retry_attempts() {
  return message_retry_attempts_;
}
const int32_t &Gossip::message_retry_attempts() const {
  return message_retry_attempts_;
}

int32_t &Gossip::message_rumor_factor() {
  return message_rumor_factor_;
}
const int32_t &Gossip::message_rumor_factor() const {
  return message_rumor_factor_;
}

int32_t &Gossip::message_max_size() {
  return message_max_size_;
}
const int32_t &Gossip::message_max_size() const {
  return message_max_size_;
}

int32_t &Gossip::max_output_messages() {
  return max_output_messages_;
}
const int32_t &Gossip::max_output_messages() const {
  return max_output_messages_;
}

int32_t &Gossip::gossip_tick_interval() {
  return gossip_tick_interval_;
}
const int32_t &Gossip::gossip_tick_interval() const {
  return gossip_tick_interval_;
}

Member &Gossip::self() { return self_; };
const Member &Gossip::self() const { return self_; };

// TODO: Let switch be more general
Gossip::Error Gossip::receive_() {
  if (state != State::JOINING && state != State::CONNECTED)
    return Error::BAD_STATE;

  recv_buffer_.clear();
  socket_.async_receive_from(
      dynamic_buffer(recv_buffer_).prepare(max_output_messages()),
      sender_,
      [this](system::error_code ec, size_t length) {
        BOOST_LOG_TRIVIAL(debug) << "\n\tReceived from address:"
                                 << to_string(sender_)
                                 << "\n\tReceived message:"
                                 << recv_buffer_.substr(0, length);
        istringstream iss(recv_buffer_.substr(0, length));
        archive::text_iarchive ia(iss);
        Message *message;
        ia >> message;
        auto p_message = std::shared_ptr<Message>(message);
        if ((uint8_t)handle_receive(p_message, sender_) < 0)
          return Error::INVALID_MESSAGE;
      });

  return Error::NONE;
}

void Gossip::send_(std::shared_ptr<Message> &t_message) {
  string message = t_message->to_string();
  BOOST_LOG_TRIVIAL(debug) << "\n\tSend to address:"
                           << to_string(t_message->header.destination->address())
                           << "\n\tSend message:"
                           << message;
  socket_.async_send_to(
      buffer(message), t_message->header.destination->address(),
      [this](boost::system::error_code ec, std::size_t length) {
        if (ec)
          return;
      });
}
}; // namespace gossip