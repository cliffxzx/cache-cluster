#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/asio.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/log/trivial.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <map>
#include <memory>
#include <stdexcept>

#include "gossip.hpp"

using boost::archive::text_oarchive;
using boost::iostreams::back_insert_device;
using boost::iostreams::stream;
using gossip::Error;
using gossip::Gossip;
using gossip::Member;
using std::logic_error;
using std::make_shared;

namespace gossip::message {

Message::Header::Header(
    const uint32_t t_sequence,
    const uint32_t t_remain_attempt,
    const Member::shared_ptr t_destination)
    : sequence(t_sequence),
      remain_attempt(t_remain_attempt),
      destination(t_destination) {}

Message::Message(const Header t_header) : m_header(t_header) {}
Error Message::receive(Gossip &self, const Member t_sender) const { throw logic_error("Not implemented"); }

template <IMessages_Ptr IMessage_Ptr>
const string to_string(IMessage_Ptr t_message) {
  string serial_str;
  back_insert_device<string> inserter(serial_str);
  stream<back_insert_device<string>> s(inserter);
  text_oarchive oa(s);
  oa << t_message;
  s.flush();
  return serial_str;
}

template const string to_string(Message::shared_ptr t_message);
}; // namespace gossip::message

namespace gossip::message {

Hello::Hello(const Header t_header) { m_header = t_header; };
Hello::Hello(const Member::shared_ptr t_self_member) : m_self_member(t_self_member){};
Hello::Hello(const Header t_header,
             const Member::shared_ptr t_self_member) : m_self_member(t_self_member) { m_header = t_header; };

Error Hello::receive(Gossip &self, const Member t_sender) const {
  auto self_member = self.self_member();
  auto sender_member = make_shared<Member>(t_sender);
  self.enqueue_message(Welcome{self_member}, Spreading::DIRECT, sender_member);

  // self.m_memberlist.emplate(t_sender);

  // Message::shared_ptr memberlist = std::make_shared<Memberlist>(m_memberlist);
  // self.enqueue_message(memberlist, t_sender, Spreading::BROADCAST);

  return Error::NONE;
}
}; // namespace gossip::message

BOOST_CLASS_EXPORT(gossip::message::Hello);

namespace gossip::message {

Welcome::Welcome(const Header t_header) { m_header = t_header; };
Welcome::Welcome(const Member::shared_ptr t_self_member) : m_self_member(t_self_member){};
Welcome::Welcome(const Header t_header,
                 const Member::shared_ptr t_self_member) : m_self_member(t_self_member) { m_header = t_header; };

Error Welcome::receive(Gossip &self, const Member t_sender) const {
  auto self_member = self.self_member();
  auto sender_member = make_shared<Member>(t_sender);
  Message::shared_ptr welcome = make_shared<Welcome>(self_member);
  // self.enqueue_message(welcome, t_sender, Spreading::DIRECT);

  // self.m_memberlist.emplate(t_sender);

  // Message::shared_ptr memberlist = std::make_shared<Memberlist>(m_memberlist);
  // self.enqueue_message(memberlist, t_sender, Spreading::BROADCAST);

  return Error::NONE;
}
}; // namespace gossip::message

BOOST_CLASS_EXPORT(gossip::message::Welcome);

// namespace gossip::message
//       m_state = State::CONNECTED;
//       std::shared_ptr<Welcome> welcome = std::dynamic_pointer_cast<Welcome>(t_message);
//       auto sender = std::make_shared<Member>(t_sender);
//       m_memberlist.emplace(to_string(sender->uid()), sender);
//       m_message.erase(find_if(m_message.begin(), m_message.end(), [welcome](Message::shared_ptr val) { return val->m_header.sequence == welcome->hello_sequence; }));

//       break;
//     }
//     case Type::Memberlist: {
//       std::shared_ptr<Memberlist> memberlist = std::dynamic_pointer_cast<Memberlist>(t_message);
//       for (auto member : memberlist->members) {
//         auto m_member = std::make_shared<Member>(member);
//         m_memberlist.emplace(to_string(member.uid()), m_member);
//       }

//       auto sender = std::make_shared<Member>(t_sender);
//       Message::shared_ptr ack = std::make_shared<Ack>(t_message->m_header.sequence);
//       enqueue_message(ack, sender, Spreading::DIRECT);

//       break;
//     }
//     case Type::Ack: {
//       std::shared_ptr<Ack> ack = std::dynamic_pointer_cast<Ack>(t_message);

//       m_message.erase(find_if(m_message.begin(), m_message.end(), [ack](Message::shared_ptr val) { return val->m_header.sequence == ack->ack_sequence; }));

//       break;
//     }
//     case Type::Data: {
//       std::shared_ptr<Data> data = std::dynamic_pointer_cast<Data>(t_message);
//       Message::shared_ptr m_data = data;
//       auto sender = std::make_shared<Member>(t_sender);
//       enqueue_message(m_data, sender, Spreading::RANDOM);
//       receiver_fn_(data->data);
//       break;
//     }

// namespace gossip::message
//     : self(t_self),
//       hello_sequence(t_hello_sequence) {
//   header = Header(Type::Welcome,
//                   t_header.sequence,
//                   t_header.remain_attempt,
//                   t_header.reserved,
//                   t_header.destination);
// }
// string Welcome::to_string() const {
//   string serial_str;
//   back_insert_device<string> inserter(serial_str);
//   stream<back_insert_device<string>> s(inserter);
//   boost::archive::text_oarchive oa(s);
//   oa.register_type<Welcome>();
//   oa << this;
//   s.flush();
//   return serial_str;
// }

// Memberlist::Memberlist(map<string, std::shared_ptr<Member>> t_members, Header t_header) {
//   header = Header(Type::Memberlist,
//                   t_header.sequence,
//                   t_header.remain_attempt,
//                   t_header.reserved,
//                   t_header.destination);

//   for (auto member : t_members) {
//     members.push_back(*member.second);
//   }
// }
// string Memberlist::to_string() const {
//   string serial_str;
//   back_insert_device<string> inserter(serial_str);
//   stream<back_insert_device<string>> s(inserter);
//   boost::archive::text_oarchive oa(s);
//   oa.register_type<Memberlist>();
//   oa << this;
//   s.flush();
//   return serial_str;
// }

// Ack::Ack(uint32_t t_ack_sequence, Header t_header)
//     : ack_sequence(t_ack_sequence) {
//   header = Header(Type::Ack,
//                   t_header.sequence,
//                   t_header.remain_attempt,
//                   t_header.reserved,
//                   t_header.destination);
// }
// string Ack::to_string() const {
//   string serial_str;
//   back_insert_device<string> inserter(serial_str);
//   stream<back_insert_device<string>> s(inserter);
//   boost::archive::text_oarchive oa(s);
//   oa << this;
//   s.flush();
//   return serial_str;
// }

// Data::Data(string t_data, Header t_header) : data(t_data) {
//   header = Header(Type::Data,
//                   t_header.sequence,
//                   t_header.remain_attempt,
//                   t_header.reserved,
//                   t_header.destination);
// }
// string Data::to_string() const {
//   string serial_str;
//   back_insert_device<string> inserter(serial_str);
//   stream<back_insert_device<string>> s(inserter);
//   boost::archive::text_oarchive oa(s);
//   oa << this;
//   s.flush();
//   return serial_str;
// }
// }
// ; // namespace gossip::message

// BOOST_CLASS_EXPORT(gossip::message::Data)
// BOOST_CLASS_EXPORT(gossip::message::Ack)
// BOOST_CLASS_EXPORT(gossip::message::Memberlist)
// BOOST_CLASS_EXPORT(gossip::message::Welcome)
// BOOST_CLASS_EXPORT(gossip::message::Hello)
// BOOST_CLASS_EXPORT(gossip::message::Header)