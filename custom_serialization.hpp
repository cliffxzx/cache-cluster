#ifndef CACHE_CLUSTER_SERIALIZATION_HPP
#define CACHE_CLUSTER_SERIALIZATION_HPP

#include <boost/asio.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_serialize.hpp>
#include <string>

#include "member.hpp"
#include "message.hpp"

using namespace std;
using namespace boost;
using namespace boost::asio;
using namespace gossip;
using namespace gossip::message;

namespace boost {
namespace serialization {

template <class Archive, class Protocol>
void save(Archive &ar, const ip::basic_endpoint<Protocol> &e, const unsigned int version) {
  string ip = e.address().to_string();
  ip::port_type port = e.port();
  ar &ip;
  ar &port;
}
template <class Archive, class Protocol>
void load(Archive &ar, ip::basic_endpoint<Protocol> &e, const unsigned int version) {
  string ip;
  ip::port_type port;
  ar &ip;
  ar &port;
  e = ip::basic_endpoint<Protocol>(ip::address::from_string(ip), port);
}

template <class Archive, class Protocol>
void serialize(Archive &ar, ip::basic_endpoint<Protocol> &e, const unsigned int version) {
  split_free(ar, e, version);
}

template <class Archive>
void serialize(Archive &ar, Member &e, const unsigned int version) {
  ar &e.uid();
  ar &e.address();
}

template <class Archive>
void serialize(Archive &ar, Header &e, const unsigned int version) {
  ar &e.message_type;
  ar &e.reserved;
  ar &e.sequence_num;
}

template <class Archive>
void serialize(Archive &ar, Hello &e, const unsigned int version) {
  ar &e.header;
  ar &e.self;
};

} // namespace serialization
} // namespace boost

#endif
