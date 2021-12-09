#ifndef CACHE_CLUSTER_MESSAGE_HPP
#define CACHE_CLUSTER_MESSAGE_HPP

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <string>

#include "member.hpp"

using namespace std;
using namespace boost;
using namespace boost::asio;
using namespace gossip;

namespace gossip::message {
static constexpr uint8_t PROTOCOL_ID_LENGTH = 5;
static constexpr char PROTOCOL_ID[PROTOCOL_ID_LENGTH] = "ptcs";

enum class Type : int8_t {
  Header = 0x00,
  Hello = 0x01,
  Welcome = 0x02,
  Memberlist = 0x03,
  Ack = 0x04,
  Data = 0x05,
  Status = 0x06,
};

class Hello : public Message {
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive &ar, const unsigned int version) {
    ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(Message);
    ar &BOOST_SERIALIZATION_NVP(self);
  };

public:
  std::shared_ptr<Member> self;

  Hello(std::shared_ptr<Member> t_self = nullptr, Header header = Header());
  virtual string to_string() const override;
};

class Welcome : public Message {
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive &ar, const unsigned int version) {
    ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(Message);
    ar &BOOST_SERIALIZATION_NVP(self);
    ar &BOOST_SERIALIZATION_NVP(hello_sequence);
  };

public:
  std::shared_ptr<Member> self;
  uint32_t hello_sequence;

  Welcome(std::shared_ptr<Member> t_self = nullptr, uint32_t hello_sequence = 0, Header header = Header());
  virtual string to_string() const override;
};

class Memberlist : public Message {
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive &ar, const unsigned int version) {
    ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(Message);
    ar &BOOST_SERIALIZATION_NVP(members);
  };

public:
  vector<Member> members;

  Memberlist(map<string, std::shared_ptr<Member>> t_members = map<string, std::shared_ptr<Member>>(), Header header = Header());
  virtual string to_string() const override;
};

class Ack : public Message {
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive &ar, const unsigned int version) {
    ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(Message);
    ar &BOOST_SERIALIZATION_NVP(ack_sequence);
  };

public:
  uint32_t ack_sequence;

  Ack(uint32_t hello_sequence = 0, Header header = Header());
  virtual string to_string() const override;
};

class Data : public Message {
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive &ar, const unsigned int version) {
    ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(Message);
    ar &BOOST_SERIALIZATION_NVP(data);
  };

public:
  string data;

  Data(string t_data = "", Header header = Header());
  virtual string to_string() const override;
};
}; // namespace gossip::message

namespace boost::serialization {

template <class Archive, class Protocol>
void save(Archive &ar, const message::Type &e, const unsigned int version) {
  int8_t type = (int8_t)e;
  ar &BOOST_SERIALIZATION_NVP(type);
}

template <class Archive, class Protocol>
void load(Archive &ar, message::Type &e, const unsigned int version) {
  int8_t type;
  ar &BOOST_SERIALIZATION_NVP(type);
  e = (message::Type)type;
}

template <class Archive, class Protocol>
void serialize(Archive &ar, message::Type &e, const unsigned int version) {
  split_free(ar, e, version);
}
} // namespace boost::serialization

#endif