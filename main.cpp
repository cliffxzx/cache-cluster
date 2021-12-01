#include <algorithm>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/asio.hpp>
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

using namespace std;
using namespace boost;
using namespace boost::asio;

namespace boost {
namespace serialization {

template <class Archive, class Protocol>
void save(Archive &ar, asio::ip::basic_endpoint<Protocol> &e, unsigned int version) {
  string ip = e.address().to_string();
  short port = e.port();
  ar &ip;
  ar &port;
}
template <class Archive, class Protocol>
void load(Archive &ar, asio::ip::basic_endpoint<Protocol> &e, unsigned int version) {
  string ip;
  short port;
  ar &ip;
  ar &port;
  e = asio::ip::basic_endpoint<Protocol>(ip, port);
}

template <class Archive, class Protocol>
inline void serialize(Archive &ar, asio::ip::basic_endpoint<Protocol> &e, const unsigned int file_version) {
  split_free(ar, e, file_version);
}

} // namespace serialization
} // namespace boost

typedef boost::shared_ptr<ip::tcp::socket> socket_ptr;
namespace GossipProtocal {
class Member {
public:
  Member() : uid_(boost::uuids::random_generator()()) {}
  Member(ip::udp::endpoint t_addr) : uid_(boost::uuids::random_generator()()), addr_(t_addr) {}
  Member(ip::address addr, ip::port_type port) : uid_(boost::uuids::random_generator()()),
                                                 addr_(ip::udp::endpoint(addr, port)) {}

  ip::udp::endpoint addr_;

private:
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive &ar, const unsigned int version) {
    ar &uid_;
    ar &addr_;
  }

  boost::uuids::uuid uid_;
};

class VectorRecord {
  uint32_t sequence_number;
  uint64_t member_id;
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive &ar, const unsigned int version) {
    ar &(this->sequence_number);
    ar &(this->member_id);
  }
};

class VectorClock {
  uint16_t current_idx;
  vector<VectorRecord> records;
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive &ar, const unsigned int version) {
    ar &(this->current_idx);
    ar &(this->records);
  }
};

namespace Message {
static constexpr uint8_t PROTOCOL_ID_LENGTH = 5;
static constexpr char PROTOCOL_ID[PROTOCOL_ID_LENGTH] = "ptcs";
enum class Type : uint8_t {
  Hello = 0x01,
  Welcome = 0x02,
  Memberlist = 0x03,
  Ack = 0x04,
  Data = 0x05,
  Status = 0x06,
};

class Header {
public:
  char protocol_id[PROTOCOL_ID_LENGTH];
  uint8_t message_type;
  uint16_t reserved;
  uint16_t attempt_num;
  uint32_t sequence_num;
  Member destination;

  Header() {}
  Header(uint8_t t_message_type, uint32_t t_sequence_num) : message_type(t_message_type), sequence_num(t_sequence_num) {}

private:
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive &ar, const unsigned int version) {
    ar &(this->protocol_id);
    ar &(this->message_type);
    ar &(this->reserved);
    ar &(this->sequence_num);
  }

  operator string() const {
    ostringstream oss;
    boost::archive::text_oarchive oa(oss);
    oa << *this;
    return oss.str();
  }
};

class Message {
public:
  Message() {}
  Header header;
  operator string() const {
    ostringstream oss;
    boost::archive::text_oarchive oa(oss);
    oa << header;
    return oss.str();
  }
};

class Hello : public Message {
  Member *self;
  operator uint8_t() const { return (uint8_t)Type::Hello; }
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive &ar, const unsigned int version) {
    ar &(this->header);
    ar &(*this->self);
  }

  operator string() const {
    ostringstream oss;
    boost::archive::text_oarchive oa(oss);
    oa << header << self;
    return oss.str();
  }
};

class Welcome : public Message {
  uint32_t hello_sequence_num;
  Member *self;
  operator uint8_t() const { return (uint8_t)Type::Welcome; }
  operator string() const {
    ostringstream oss;
    boost::archive::text_oarchive oa(oss);
    oa << header << hello_sequence_num << self;
    return oss.str();
  }
};

class Memberlist : public Message {
  vector<Member> members;
  operator uint8_t() const { return (uint8_t)Type::Memberlist; }
  operator string() const {
    ostringstream oss;
    boost::archive::text_oarchive oa(oss);
    oa << header << members;
    return oss.str();
  }
};

class Ack : public Message {
  uint32_t ack_sequence_num;
  operator uint8_t() const { return (uint8_t)Type::Ack; }
  operator string() const {
    ostringstream oss;
    boost::archive::text_oarchive oa(oss);
    oa << header << ack_sequence_num;
    return oss.str();
  }
};

class Data : public Message {
  VectorRecord data_version;
  vector<uint8_t> data;
  operator uint8_t() const { return (uint8_t)Type::Data; }
  operator string() const {
    ostringstream oss;
    boost::archive::text_oarchive oa(oss);
    oa << header << data_version << data;
    return oss.str();
  }
};

class Status : public Message {
  VectorClock data_version;
  operator uint8_t() const { return (uint8_t)Type::Status; }
  operator string() const {
    ostringstream oss;
    boost::archive::text_oarchive oa(oss);
    oa << header << data_version;
    return oss.str();
  }
};

}; // namespace Message

class GossipProtocal {
  typedef std::function<void(GossipProtocal *, system::error_code, string)> ReceiverFn;

public:
  GossipProtocal(
      ip::address addr,
      ip::port_type port,
      ReceiverFn t_receiver) : endpoint_(ip::udp::endpoint(addr, port)),
                               socket_(service_, endpoint_),
                               state(State::INITIALIZED),
                               receiver_(t_receiver) {
  }

  void run() {
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

  Error receive() {
    recv_buffer().clear();
    socket_.async_receive(
        dynamic_buffer(recv_buffer()).prepare(max_output_messages()),
        [this](system::error_code ec, size_t length) {
          return receiver()(this, ec, recv_buffer());
        });

    return Error::NONE;
  }

  Error send() {
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

  Error enqueue_message(Message::Message t_message, Member t_member, SpreadingType t_spreading_type) {
    Message::Message message = t_message;
    message.header.destination = t_member;

    switch (t_spreading_type) {
      case SpreadingType::DIRECT:
        message_.push_back(t_message);
        return Error::NONE;
      case SpreadingType::RANDOM: {
        vector<Member> reservoir;
        reservoir.reserve(this->message_rumor_factor());
        sample(memberlist_.begin(), memberlist_.end(),
               back_inserter(reservoir),
               this->message_rumor_factor(),
               std::mt19937{std::random_device{}()});
        for (const Member member : reservoir) {
          message_.push_back(t_message);
        }
        return Error::NONE;
      }
      case SpreadingType::BROADCAST: {
        for (const Member member : memberlist_) {
          message_.push_back(t_message);
        }
        return Error::NONE;
      }
    }
  }

  // Todo hello to every member:270
  template <class InputIterator>
  Error add_members(const InputIterator first, const InputIterator last) {
    if (this->state != State::INITIALIZED)
      return Error::BAD_STATE;

    if (last - first == 0) {
      this->state = State::CONNECTED;
      return Error::NONE;
    }

    for (auto it = first; it != last; ++it) {
      Member member = *it;
      Error res = enqueue_message(Message::Hello{}, member, SpreadingType::DIRECT);
      if ((uint32_t)res < 0)
        return res;
    }

    this->state = State::JOINING;
    return Error::NONE;
  }

  /** The interval in milliseconds between retry attempts. */
  int32_t &message_retry_interval() {
    return message_retry_interval_;
  }
  const int32_t &message_retry_interval() const {
    return message_retry_interval_;
  }

  /** The maximum number of attempts to deliver a message. */
  int32_t &message_retry_attempts() {
    return message_retry_attempts_;
  }
  const int32_t &message_retry_attempts() const {
    return message_retry_attempts_;
  }

  /** The number of members that are used for further gossip propagation. */
  int32_t &message_rumor_factor() {
    return message_rumor_factor_;
  }
  const int32_t &message_rumor_factor() const {
    return message_rumor_factor_;
  }

  /** The maximum supported size of the message including a protocol overhead. */
  int32_t &message_max_size() {
    return message_max_size_;
  }
  const int32_t &message_max_size() const {
    return message_max_size_;
  }

  /** The maximum number of unique messages that can be stored in the outbound message queue. */
  int32_t &max_output_messages() {
    return max_output_messages_;
  }
  const int32_t &max_output_messages() const {
    return max_output_messages_;
  }

  /** The time interval in milliseconds that determines how often the Gossip tick event should be triggered. */
  int32_t &gossip_tick_interval() {
    return gossip_tick_interval_;
  }
  const int32_t &gossip_tick_interval() const {
    return gossip_tick_interval_;
  }

  /** The time interval in milliseconds that determines how often the Gossip tick event should be triggered. */
  string &recv_buffer() {
    return recv_buffer_;
  }
  const string &recv_buffer() const {
    return recv_buffer_;
  }

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
  vector<Member> memberlist_;
  vector<Message::Message> message_;
  State state;
  ReceiverFn receiver_;

  ReceiverFn &receiver() {
    return receiver_;
  }
  const ReceiverFn &receiver() const {
    return receiver_;
  }
  void send_(Message::Message t_message) {
    string message = string(t_message);
    socket_.async_send_to(
        dynamic_buffer(message).prepare(message.size()), t_message.header.destination.addr_,
        [this](boost::system::error_code ec, std::size_t length) {
          if (ec)
            return;
        });
  }
};
}; // namespace GossipProtocal

int main(int argc, char *argv[]) {
  if (!(argc % 2)) {
    cout << "Error: must be " << argv[0] << "Self-IP Self-Port [[O1-IP] [O1-Port], [O2-IP] [O2-Port],...]\n";
    return 1;
  }

  ip::address ip = ip::address::from_string("0.0.0.0");
  ip::port_type port = 12345;

  if (argc >= 3) {
    system::error_code ec;
    ip = ip::address::from_string(argv[1], ec);
    if (ec) {
      cout << "Error: can't parse Self-IP";
      return 1;
    }

    port = stoi(string(argv[2]));
  }

  try {
    GossipProtocal::GossipProtocal boss(ip, port, [](GossipProtocal::GossipProtocal *self, system::error_code ec, string data) {
      cout << data;
    });
    boss.run();

    vector<GossipProtocal::Member> members;
    for (int w = 3; w < argc; ++w) {
      system::error_code ec;
      ip::address oip = ip::address::from_string(argv[w], ec);
      if (ec) {
        cout << "Error: can't parse O" << w / 2 << "-IP";
        return 1;
      }

      ip::port_type oport = stoi(string(argv[w + 1]));
      members.push_back(GossipProtocal::Member(oip, oport));
    }
    boss.add_members(members.begin(), members.end());

  } catch (const std::exception &ex) {
    std::cout << ex.what() << std::endl;
  }
  return 0;
}
