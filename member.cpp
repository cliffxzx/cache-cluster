#include <boost/asio.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <istream>
#include <sstream>
#include <string>

#include "member.hpp"

using boost::asio::ip::address;
using boost::asio::ip::port_type;
using boost::asio::ip::udp;
using boost::uuids::random_generator;
using boost::uuids::uuid;
using std::istream;
using std::istringstream;
using std::string;

namespace gossip {

Member::Member(const udp::endpoint t_addr) : m_addr(t_addr) {}
Member::Member(const string t_addr) { istringstream(t_addr) >> *this; }

istream &operator>>(istream &in, Member &t_member) {
  string ip;
  port_type port;
  in >> ip >> port;

  t_member.m_addr = udp::endpoint(address::from_string(ip), port);
  return in;
};

const uuid &Member::uid() const { return m_uid; };
const udp::endpoint &Member::address() const { return m_addr; };

}; // namespace gossip

BOOST_CLASS_EXPORT(gossip::Member)