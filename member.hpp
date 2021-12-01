#ifndef CACHE_CLUSTER_MEMBER_HPP
#define CACHE_CLUSTER_MEMBER_HPP

#include <boost/asio.hpp>
#include <boost/uuid/uuid.hpp>

using namespace boost;
using namespace boost::asio;

namespace gossip {

class Member {
  uuids::uuid uid_;
  ip::udp::endpoint addr_;

public:
  Member();
  Member(ip::udp::endpoint t_addr);
  Member(ip::address addr, ip::port_type port);

  uuids::uuid &uid();
  const uuids::uuid &uid() const;

  ip::udp::endpoint &address();
  const ip::udp::endpoint &address() const;
};

}; // namespace gossip

#endif