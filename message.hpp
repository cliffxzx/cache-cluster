#ifndef CACHE_CLUSTER_MESSAGE_HPP
#define CACHE_CLUSTER_MESSAGE_HPP

#include <string>

#include "member.hpp"

using namespace std;
using namespace boost;
using namespace boost::asio;
using namespace gossip;

namespace gossip::message {
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
  uint8_t message_type;
  uint16_t reserved;
  uint16_t attempt_num;
  uint32_t sequence_num;
  Member destination;

  Header();
  Header(uint8_t t_message_type, uint32_t t_sequence_num);

private:
  operator string() const;
};

class Message {
public:
  Message();
  Header header;
  operator string() const;
};

class Hello : public Message {
public:
  Member self;
  Hello();
  operator uint8_t() const;
  operator string() const;
};

class Welcome : public Message {
public:
  uint32_t hello_sequence_num;
  Member self;
  Welcome();
  operator uint8_t() const;
  operator string() const;
};

class Memberlist : public Message {
public:
  vector<Member> members;
  Memberlist();
  operator uint8_t() const;
  operator string() const;
};

class Ack : public Message {
public:
  uint32_t ack_sequence_num;
  Ack();
  operator uint8_t() const;
  operator string() const;
};

class Data : public Message {
public:
  // VectorRecord data_version;
  vector<uint8_t> data;
  Data();
  operator uint8_t() const;
  operator string() const;
};

class Status : public Message {
public:
  // VectorClock data_version;
  Status();
  operator uint8_t() const;
  operator string() const;
};

}; // namespace GossipProtocol::Message

#endif