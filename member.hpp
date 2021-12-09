#ifndef CACHE_CLUSTER_MEMBER_HPP
#define CACHE_CLUSTER_MEMBER_HPP

#include <boost/asio.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_serialize.hpp>
#include <string>

using namespace std;
using namespace boost;
using namespace boost::asio;

namespace boost::serialization {

template <class Archive, class Protocol>
void save(Archive &ar, const ip::basic_endpoint<Protocol> &e, const unsigned int version) {
  string ip = e.address().to_string();
  ip::port_type port = e.port();
  ar &BOOST_SERIALIZATION_NVP(ip);
  ar &BOOST_SERIALIZATION_NVP(port);
}
template <class Archive, class Protocol>
void load(Archive &ar, ip::basic_endpoint<Protocol> &e, const unsigned int version) {
  string ip;
  ip::port_type port;
  ar &BOOST_SERIALIZATION_NVP(ip);
  ar &BOOST_SERIALIZATION_NVP(port);
  e = ip::basic_endpoint<Protocol>(ip::address::from_string(ip), port);
}

template <class Archive, class Protocol>
void serialize(Archive &ar, ip::basic_endpoint<Protocol> &e, const unsigned int version) {
  split_free(ar, e, version);
}

} // namespace boost::serialization

namespace gossip {

class Member {
  uuids::uuid uid_;
  ip::udp::endpoint addr_;

  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive &ar, const unsigned int version) {
    ar & BOOST_SERIALIZATION_NVP(uid_);
    ar & BOOST_SERIALIZATION_NVP(addr_);
  }

public:
  Member();
  Member(ip::udp::endpoint t_addr);
  Member(ip::address addr, ip::port_type port);

  // uuids::uuid &uid();
  const uuids::uuid &uid() const;

  // ip::udp::endpoint &address();
  const ip::udp::endpoint &address() const;
};

}; // namespace gossip

#endif