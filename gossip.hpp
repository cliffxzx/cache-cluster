#ifndef GOSSIP_PROTOCOL_HPP
#define GOSSIP_PROTOCOL_HPP

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <deque>
#include <functional>
#include <future>
#include <memory>
#include <vector>

#include "member.hpp"
#include "message.hpp"

using boost::asio::io_context;
using boost::asio::ip::address;
using boost::asio::ip::port_type;
using boost::asio::ip::udp;
using gossip::Member;
using gossip::message::IMessages;
using gossip::message::IMessages_Ptr;
using gossip::message::Message;
using std::async;
using std::future;
using std::set;
using std::shared_future;
using std::shared_ptr;
using std::string;
using std::thread;

namespace gossip {
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

enum class State {
  INITIALIZED,
  JOINING,
  CONNECTED,
  LEAVING,
  DISCONNECTED,
  DESTROYED
};

enum class Spreading {
  DIRECT,
  RANDOM,
  BROADCAST
};

class Gossip {
  typedef std::function<void(string)> ReceiverFn;
  ReceiverFn m_receiver;

  int32_t m_message_retry_interval = 10000;
  int32_t m_message_retry_attempts = 3;
  int32_t m_message_rumor_factor = 3;
  int32_t m_message_max_size = 65535;
  int32_t m_max_output_messages = 65535;
  int32_t m_gossip_tick_interval = 500;

  State m_state = State::INITIALIZED;
  Member::shared_ptr m_self_member;
  set<Member::shared_ptr> m_memberlist;
  set<Message::shared_ptr> m_message;

  io_context m_context;
  udp::socket m_socket = udp::socket(m_context);

  string m_temp_recv_buffer;
  udp::endpoint m_temp_sender;

  void m_receive_handler();
  void m_send_handler();
  Error m_receive(const string t_data, const Member t_sender);
  template <IMessages_Ptr IMessage_Ptr>
  Error m_send(IMessage_Ptr t_message);

public:
  Gossip() = default;
  Gossip(const Member t_self_member, const ReceiverFn t_receiver);

  void run();

  template <typename Streamable>
  future<Error> send(const Streamable data);

  template <IMessages IMessage>
  Error enqueue_message(const IMessage t_message,
                        const Spreading t_spreading,
                        const Member::shared_ptr t_member = nullptr);

  Error add_member(const Member t_member);

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

  const Member::shared_ptr &self_member() const;
};
}; // namespace gossip

#endif