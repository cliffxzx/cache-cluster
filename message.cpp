#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/asio.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/stream.hpp>

#include "message.hpp"

using namespace std;
using namespace boost;
using namespace boost::asio;
using namespace boost::iostreams;

namespace gossip::message {

Header::Header() : message_type(0), attempt_num(0), sequence_num(0), reserved(1) {}
Header::Header(uint8_t t_message_type, uint32_t t_sequence_num) : message_type(t_message_type), sequence_num(t_sequence_num), reserved(1) {}
string Header::to_string() const {
  string serial_str;
  back_insert_device<string> inserter(serial_str);
  stream<back_insert_device<string>> s(inserter);
  boost::archive::text_oarchive oa(s);
  oa << *this;
  s.flush();
  return serial_str;
}

Message::Message() {}
string Message::to_string() const {
  string serial_str;
  back_insert_device<string> inserter(serial_str);
  stream<back_insert_device<string>> s(inserter);
  boost::archive::text_oarchive oa(s);
  oa << header;
  s.flush();
  return serial_str;
}

Hello::Hello() {}
Hello::Hello(uint16_t max_attempt) {
  header.message_type = (uint8_t)Type::Hello;
  header.sequence_num = 0;
}
string Hello::to_string() const {
  string serial_str;
  back_insert_device<string> inserter(serial_str);
  stream<back_insert_device<string>> s(inserter);
  boost::archive::text_oarchive oa(s);
  oa << header << self;
  s.flush();
  return serial_str;
}

Welcome::Welcome() {
  header.message_type = (uint8_t)Type::Welcome;
}
string Welcome::to_string() const {
  string serial_str;
  back_insert_device<string> inserter(serial_str);
  stream<back_insert_device<string>> s(inserter);
  boost::archive::text_oarchive oa(s);

  // oa << header << hello_sequence_num << self;

  s.flush();
  return serial_str;
}

Memberlist::Memberlist() {
  header.message_type = (uint8_t)Type::Memberlist;
}
string Memberlist::to_string() const {
  ostringstream oss;
  boost::archive::text_oarchive oa(oss);
  // oa << header << members;
  return oss.str();
}

Ack::Ack() {
  header.message_type = (uint8_t)Type::Ack;
}
string Ack::to_string() const {
  ostringstream oss;
  boost::archive::text_oarchive oa(oss);
  // oa << header << ack_sequence_num;
  return oss.str();
}

Data::Data() {
  header.message_type = (uint8_t)Type::Data;
}
string Data::to_string() const {
  ostringstream oss;
  boost::archive::text_oarchive oa(oss);
  // oa << header << data_version << data;
  return oss.str();
}

Status::Status() {
  header.message_type = (uint8_t)Type::Status;
}
string Status::to_string() const {
  ostringstream oss;
  boost::archive::text_oarchive oa(oss);
  // oa << header << data_version;
  return oss.str();
}
}; // namespace gossip::message
