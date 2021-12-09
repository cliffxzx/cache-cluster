#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/asio.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <map>
#include <memory>

#include "message.hpp"

using namespace std;
using namespace boost;
using namespace boost::asio;
using namespace boost::iostreams;

namespace gossip::message {

Hello::Hello(std::shared_ptr<Member> t_self, Header t_header) : self(t_self) {
  header = Header(Type::Hello,
                  t_header.sequence,
                  t_header.remain_attempt,
                  t_header.reserved,
                  t_header.destination);
}
string Hello::to_string() const {
  string serial_str;
  back_insert_device<string> inserter(serial_str);
  stream<back_insert_device<string>> s(inserter);
  boost::archive::text_oarchive oa(s);
  oa.register_type<Hello>();
  oa << this;
  s.flush();
  return serial_str;
}

Welcome::Welcome(std::shared_ptr<Member> t_self, uint32_t t_hello_sequence, Header t_header)
    : self(t_self),
      hello_sequence(t_hello_sequence) {
  header = Header(Type::Welcome,
                  t_header.sequence,
                  t_header.remain_attempt,
                  t_header.reserved,
                  t_header.destination);
}
string Welcome::to_string() const {
  string serial_str;
  back_insert_device<string> inserter(serial_str);
  stream<back_insert_device<string>> s(inserter);
  boost::archive::text_oarchive oa(s);
  oa.register_type<Welcome>();
  oa << this;
  s.flush();
  return serial_str;
}

Memberlist::Memberlist(map<string, std::shared_ptr<Member>> t_members, Header t_header) {
  header = Header(Type::Memberlist,
                  t_header.sequence,
                  t_header.remain_attempt,
                  t_header.reserved,
                  t_header.destination);

  for (auto member : t_members) {
    members.push_back(*member.second);
  }
}
string Memberlist::to_string() const {
  string serial_str;
  back_insert_device<string> inserter(serial_str);
  stream<back_insert_device<string>> s(inserter);
  boost::archive::text_oarchive oa(s);
  oa.register_type<Memberlist>();
  oa << this;
  s.flush();
  return serial_str;
}

Ack::Ack(uint32_t t_ack_sequence, Header t_header)
    : ack_sequence(t_ack_sequence) {
  header = Header(Type::Ack,
                  t_header.sequence,
                  t_header.remain_attempt,
                  t_header.reserved,
                  t_header.destination);
}
string Ack::to_string() const {
  string serial_str;
  back_insert_device<string> inserter(serial_str);
  stream<back_insert_device<string>> s(inserter);
  boost::archive::text_oarchive oa(s);
  oa << this;
  s.flush();
  return serial_str;
}

Data::Data(string t_data, Header t_header) : data(t_data) {
  header = Header(Type::Data,
                  t_header.sequence,
                  t_header.remain_attempt,
                  t_header.reserved,
                  t_header.destination);
}
string Data::to_string() const {
  string serial_str;
  back_insert_device<string> inserter(serial_str);
  stream<back_insert_device<string>> s(inserter);
  boost::archive::text_oarchive oa(s);
  oa << this;
  s.flush();
  return serial_str;
}
}; // namespace gossip::message

BOOST_CLASS_EXPORT(gossip::message::Data)
BOOST_CLASS_EXPORT(gossip::message::Ack)
BOOST_CLASS_EXPORT(gossip::message::Memberlist)
BOOST_CLASS_EXPORT(gossip::message::Welcome)
BOOST_CLASS_EXPORT(gossip::message::Hello)
BOOST_CLASS_EXPORT(gossip::message::Header)