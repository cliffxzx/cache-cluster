#ifndef CACHE_CLUSTER_MESSAGE_HPP
#define CACHE_CLUSTER_MESSAGE_HPP

#include <string>
#include <boost/serialization/serialization.hpp>

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

  virtual string to_string() const;

  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive &ar, const unsigned int version) {
    ar &message_type;
    ar &reserved;
    ar &sequence_num;
  }
};

class Message {
public:
  Message();
  Header header;
  virtual string to_string() const;

  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive &ar, const unsigned int version) {
    ar &header;
  };


};

class Hello : public Message {
public:
  Member self;

  Hello();
  Hello(uint16_t max_attempts);

  virtual string to_string() const override;

  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive &ar, const unsigned int version) {
    ar &header;
    ar &self;
  };
};

class Welcome : public Message {
public:
  uint32_t hello_sequence_num;
  Member self;
  Welcome();
  virtual string to_string() const override;
};

class Memberlist : public Message {
public:
  vector<Member> members;
  Memberlist();
  virtual string to_string() const override;
};

class Ack : public Message {
public:
  uint32_t ack_sequence_num;
  Ack();
  virtual string to_string() const override;
};

class Data : public Message {
public:
  vector<uint8_t> data;
  Data();
  virtual string to_string() const override;
};

class Status : public Message {
public:
  Status();
  virtual string to_string() const override;
};

}; // namespace gossip::message

#endif