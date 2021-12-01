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

Header::Header() {}
Header::Header(uint8_t t_message_type, uint32_t t_sequence_num) : message_type(t_message_type), sequence_num(t_sequence_num), reserved(1) {}
Header::operator string() const {
  std::string serial_str;
  back_insert_device<std::string> inserter(serial_str);
  stream<back_insert_device<std::string>> s(inserter);
  boost::archive::text_oarchive oa(s);

  // oa << *this;

  s.flush();
  return serial_str;
}

Message::Message() {}
Message::operator string() const {
  std::string serial_str;
  back_insert_device<std::string> inserter(serial_str);
  stream<back_insert_device<std::string>> s(inserter);
  boost::archive::text_oarchive oa(s);

  // oa << header;

  s.flush();
  return serial_str;
}

Hello::Hello() {
  header.message_type = (uint8_t)Type::Hello;
}
Hello::operator string() const {
  std::string serial_str;
  back_insert_device<std::string> inserter(serial_str);
  stream<back_insert_device<std::string>> s(inserter);
  boost::archive::text_oarchive oa(s);

  // oa << header << self;

  s.flush();
  return serial_str;
}

Welcome::Welcome() {
  header.message_type = (uint8_t)Type::Welcome;
}
Welcome::operator string() const {
  std::string serial_str;
  back_insert_device<std::string> inserter(serial_str);
  stream<back_insert_device<std::string>> s(inserter);
  boost::archive::text_oarchive oa(s);

  // oa << header << hello_sequence_num << self;

  s.flush();
  return serial_str;
}

Memberlist::Memberlist() {
  header.message_type = (uint8_t)Type::Memberlist;
}
Memberlist::operator string() const {
  ostringstream oss;
  boost::archive::text_oarchive oa(oss);
  // oa << header << members;
  return oss.str();
}

Ack::Ack() {
  header.message_type = (uint8_t)Type::Ack;
}
Ack::operator string() const {
  ostringstream oss;
  boost::archive::text_oarchive oa(oss);
  // oa << header << ack_sequence_num;
  return oss.str();
}

Data::Data() {
  header.message_type = (uint8_t)Type::Data;
}
Data::operator string() const {
  ostringstream oss;
  boost::archive::text_oarchive oa(oss);
  // oa << header << data_version << data;
  return oss.str();
}

Status::Status() {
  header.message_type = (uint8_t)Type::Status;
}
Status::operator string() const {
  ostringstream oss;
  boost::archive::text_oarchive oa(oss);
  // oa << header << data_version;
  return oss.str();
}
}; // namespace gossip::message
