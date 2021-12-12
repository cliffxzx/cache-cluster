#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <boost/serialization/export.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <memory>
#include <string>
#include <type_traits>

#include "member.hpp"

using std::is_base_of;
using std::shared_ptr;

namespace gossip {
class Gossip;
class Member;
enum class Error;
namespace message {
class Message {
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive &ar, const unsigned int version) {
    ar &m_header;
  };

public:
  using shared_ptr = std::shared_ptr<Message>;
  class Header {
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version) {
      ar &remain_attempt;
      ar &sequence;
      ar &destination;
    }

  public:
    uint32_t sequence = 0;
    uint32_t remain_attempt = 0;
    Member::shared_ptr destination = nullptr;

    Header() = default;
    Header(const uint32_t t_sequence,
           const uint32_t t_reamain_attempt,
           const Member::shared_ptr t_destination);
  };

  Header m_header{};

  Message() = default;
  Message(const Header t_header);
  virtual Error receive(Gossip &self, const Member t_sender) const;
};

template <typename T>
concept IMessages = is_base_of<Message, T>::value;
template <typename T>
concept IMessages_Ptr = is_base_of<shared_ptr<Message>, T>::value;

template <IMessages_Ptr IMessage_Ptr>
const string to_string(IMessage_Ptr t_message);
} // namespace message
} // namespace gossip

namespace gossip::message {
class Hello : public Message {
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive &ar, const unsigned int version) {
    ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(Message);
    ar &m_self_member;
  };

public:
  Member::shared_ptr m_self_member = nullptr;

  Hello() = default;
  Hello(const Header t_header);
  Hello(const Member::shared_ptr t_self_member);
  Hello(const Header t_header,
        const Member::shared_ptr t_self_member);

  virtual Error receive(Gossip &self, const Member t_sender) const override;
};
}; // namespace gossip::message

namespace gossip::message {
class Welcome : public Message {
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive &ar, const unsigned int version) {
    ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(Message);
    ar &m_self_member;
  };

public:
  Member::shared_ptr m_self_member = nullptr;

  Welcome() = default;
  Welcome(const Header t_header);
  Welcome(const Member::shared_ptr t_self_member);
  Welcome(const Header t_header,
          const Member::shared_ptr t_self_member);

  virtual Error receive(Gossip &self, const Member t_sender) const override;
};
}; // namespace gossip::message

// class Welcome : public Message {
//   friend class boost::serialization::access;
//   template <class Archive>
//   void serialize(Archive &ar, const unsigned int version) {
//     ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(Message);
//     ar &m_self_member;
//     ar &hello_sequence;
//   };

// public:
//   Member::shared_ptr m_self_member;
//   uint32_t hello_sequence;

//   Welcome(Member::shared_ptr t_self_member = nullptr, uint32_t hello_sequence = 0, Header header = Header());
// };

// class Memberlist : public Message {
//   friend class boost::serialization::access;
//   template <class Archive>
//   void serialize(Archive &ar, const unsigned int version) {
//     ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(Message);
//     ar &members;
//   };

// public:
//   vector<Member> members;

//   Memberlist(map<string, Member::shared_ptr> t_members = map<string, Member::shared_ptr>(), Header header = Header());
// };

// class Ack : public Message {
//   friend class boost::serialization::access;
//   template <class Archive>
//   void serialize(Archive &ar, const unsigned int version) {
//     ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(Message);
//     ar &ack_sequence;
//   };

// public:
//   uint32_t ack_sequence;

//   Ack(uint32_t hello_sequence = 0, Header header = Header());
// };

// class Data : public Message {
//   friend class boost::serialization::access;
//   template <class Archive>
//   void serialize(Archive &ar, const unsigned int version) {
//     ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(Message);
//     ar &data;
//   };

// public:
//   string data;

//   Data(string t_data = "", Header header = Header());
// };
// }; // namespace gossip::message

// namespace boost::serialization {

// template <class Archive, class Protocol>
// void save(Archive &ar, const message::Type &e, const unsigned int version) {
//   int8_t type = (int8_t)e;
//   ar &type;
// }

// template <class Archive, class Protocol>
// void load(Archive &ar, message::Type &e, const unsigned int version) {
//   int8_t type;
//   ar &type;
//   e = (message::Type)type;
// }

// template <class Archive, class Protocol>
// void serialize(Archive &ar, message::Type &e, const unsigned int version) {
//   split_free(ar, e, version);
// }
// } // namespace boost::serialization
// } // namespace gossip::message
#endif