#include <boost/asio.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/serialization.hpp>

#include "member.hpp"

using namespace boost;
using namespace boost::asio;

namespace gossip {

Member::Member() : uid_(uuids::random_generator()()) {}
Member::Member(ip::udp::endpoint t_addr) : uid_(uuids::random_generator()()), addr_(t_addr) {}
Member::Member(ip::address addr, ip::port_type port) : uid_(uuids::random_generator()()),
                                                       addr_(ip::udp::endpoint(addr, port)) {}

// uuids::uuid &Member::uid() { return uid_; };
const uuids::uuid &Member::uid() const { return uid_; };

// ip::udp::endpoint &Member::address() { return addr_; };
const ip::udp::endpoint &Member::address() const { return addr_; };

}; // namespace gossip

BOOST_CLASS_EXPORT(gossip::Member)