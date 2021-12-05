#ifndef CACHE_CLUSTER_GOSSIP_PROTOCOL_HPP
#define CACHE_CLUSTER_GOSSIP_PROTOCOL_HPP

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <deque>
#include <functional>
#include <vector>
#include <memory>

#include "member.hpp"
#include "message.hpp"

using namespace std;
using namespace boost;
using namespace boost::asio;
using namespace gossip::message;

namespace gossip {

class Gossip {
  typedef std::function<void(Gossip *, system::error_code, string)> ReceiverFn;

public:
  Gossip(ip::address addr, ip::port_type port);
  void run();

  enum class Error : int32_t {
    NONE = 0,
    INIT_FAILED = -1,
    ALLOCATION_FAILED = -2,
    BAD_STATE = -3,
    INVALID_MESSAGE = -4,
    BUFFER_NOT_ENOUGH = -5,
    NOT_FOUND = -6,
    WRITE_FAILED = -7,
    READ_FAILED = -8
  };

  enum class State : int32_t {
    INITIALIZED,
    JOINING,
    CONNECTED,
    LEAVING,
    DISCONNECTED,
    DESTROYED
  };

  enum class SpreadingType {
    DIRECT,
    RANDOM,
    BROADCAST
  };

  void handle_hello(Hello hello, ip::udp::endpoint sender);

  Error receive();

  Error send();

  Error enqueue_message(Message t_message, Member t_member, SpreadingType t_spreading_type);

  Error add_member(Member t_member);

  /** The interval in milliseconds between retry attempts. */
  int32_t &message_retry_interval();
  const int32_t &message_retry_interval() const;

  /** The maximum number of attempts to deliver a message. */
  int32_t &message_retry_attempts();
  const int32_t &message_retry_attempts() const;

  /** The number of members that are used for further gossip propagation. */
  int32_t &message_rumor_factor();
  const int32_t &message_rumor_factor() const;

  /** The maximum supported size of the message including a protocol overhead. */
  int32_t &message_max_size();
  const int32_t &message_max_size() const;

  /** The maximum number of unique messages that can be stored in the outbound message queue. */
  int32_t &max_output_messages();
  const int32_t &max_output_messages() const;

  /** The time interval in milliseconds that determines how often the Gossip tick event should be triggered. */
  int32_t &gossip_tick_interval();
  const int32_t &gossip_tick_interval() const;

  /** The time interval in milliseconds that determines how often the Gossip tick event should be triggered. */
  string &recv_buffer();
  const string &recv_buffer() const;

private:
  int32_t message_retry_interval_ = 10000;
  int32_t message_retry_attempts_ = 3;
  int32_t message_rumor_factor_ = 3;
  int32_t message_max_size_ = 512;
  int32_t max_output_messages_ = 100;
  int32_t gossip_tick_interval_ = 1000;
  io_service service_;
  ip::udp::endpoint endpoint_;
  ip::udp::socket socket_;
  string recv_buffer_;
  map<string, unique_ptr<Member>> memberlist_;
  deque<unique_ptr<Message>> message_;
  State state;
  void send_(Message t_message);
};
}; // namespace gossip

#endif