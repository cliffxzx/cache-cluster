#include <algorithm>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/asio.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/serialization/split_free.hpp>
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

namespace Message {

Header::Header() {}
Header::Header(uint8_t t_message_type, uint32_t t_sequence_num) : message_type(t_message_type), sequence_num(t_sequence_num), reserved(1) {}

template <class Archive>
void Header::serialize(Archive &ar, const unsigned int version) {
  ar &(this->message_type);
  ar &(this->reserved);
  ar &(this->sequence_num);
}

Header::operator string() const {
  std::string serial_str;
  boost::iostreams::back_insert_device<std::string> inserter(serial_str);
  boost::iostreams::stream<boost::iostreams::back_insert_device<std::string>> s(inserter);
  boost::archive::text_oarchive oa(s);

  oa << *this;

  s.flush();
  return serial_str;
}
Message::operator string() const {
  std::string serial_str;
  boost::iostreams::back_insert_device<std::string> inserter(serial_str);
  boost::iostreams::stream<boost::iostreams::back_insert_device<std::string>> s(inserter);
  boost::archive::text_oarchive oa(s);

  oa << header;

  s.flush();
  return serial_str;
}

Hello::Hello() {
  header.message_type = (uint8_t)Type::Hello;
}
template <class Archive>
void Hello::serialize(Archive &ar, const unsigned int version) {
  ar &(this->header);
  ar &(this->self);
}

Hello::operator string() const {
  std::string serial_str;
  boost::iostreams::back_insert_device<std::string> inserter(serial_str);
  boost::iostreams::stream<boost::iostreams::back_insert_device<std::string>> s(inserter);
  boost::archive::text_oarchive oa(s);

  oa << header << self;

  s.flush();
  return serial_str;
}

Welcome::Welcome() {
  header.message_type = (uint8_t)Type::Welcome;
}
Welcome::operator string() const {
  std::string serial_str;
  boost::iostreams::back_insert_device<std::string> inserter(serial_str);
  boost::iostreams::stream<boost::iostreams::back_insert_device<std::string>> s(inserter);
  boost::archive::text_oarchive oa(s);

  oa << header << hello_sequence_num << self;

  s.flush();
  return serial_str;
}

Memberlist::Memberlist() {
  header.message_type = (uint8_t)Type::Memberlist;
}
Memberlist::operator string() const {
  ostringstream oss;
  boost::archive::text_oarchive oa(oss);
  oa << header << members;
  return oss.str();
}

Ack::Ack() {
  header.message_type = (uint8_t)Type::Ack;
}
Ack::operator string() const {
  ostringstream oss;
  boost::archive::text_oarchive oa(oss);
  oa << header << ack_sequence_num;
  return oss.str();
}

Data::Data() {
  header.message_type = (uint8_t)Type::Data;
}
Data::operator string() const {
  ostringstream oss;
  boost::archive::text_oarchive oa(oss);
  oa << header << data_version << data;
  return oss.str();
}

Status::Status() {
  header.message_type = (uint8_t)Type::Status;
}
Status::operator string() const {
  ostringstream oss;
  boost::archive::text_oarchive oa(oss);
  oa << header << data_version;
  return oss.str();
}
}; // namespace Message
