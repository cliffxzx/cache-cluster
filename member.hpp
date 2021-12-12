#ifndef MEMBER_HPP
#define MEMBER_HPP

#include <boost/asio.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_serialize.hpp>
#include <istream>
#include <memory>
#include <string>

using boost::asio::ip::address;
using boost::asio::ip::basic_endpoint;
using boost::asio::ip::port_type;
using boost::asio::ip::udp;
using boost::serialization::split_free;
using boost::uuids::random_generator;
using boost::uuids::uuid;
using std::istream;

using std::string;

namespace boost::serialization {

template <class Archive, class Protocol>
void save(Archive &ar, const basic_endpoint<Protocol> &e, const unsigned int version) {
  string ip{e.address().to_string()};
  ar &ip;

  port_type port{e.port()};
  ar &port;
}
template <class Archive, class Protocol>
void load(Archive &ar, basic_endpoint<Protocol> &e, const unsigned int version) {
  string ip;
  ar &ip;

  port_type port;
  ar &port;

  e = basic_endpoint<Protocol>(address::from_string(ip), port);
}

template <class Archive, class Protocol>
void serialize(Archive &ar, basic_endpoint<Protocol> &e, const unsigned int version) {
  split_free(ar, e, version);
}

} // namespace boost::serialization

namespace gossip {

class Member {
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive &ar, const unsigned int version) {
    ar &m_uid;
    ar &m_addr;
  }

  uuid m_uid{random_generator()()};
  udp::endpoint m_addr;

public:
  using shared_ptr = std::shared_ptr<Member>;

  Member() = default;
  Member(const udp::endpoint t_addr);
  Member(const string t_addr);

  friend istream &operator>>(istream &in, Member &t_member);

  const uuid &uid() const;
  const udp::endpoint &address() const;
};

}; // namespace gossip

#endif