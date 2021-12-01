#ifndef CACHE_CLUSTER_UTILS_HPP
#define CACHE_CLUSTER_UTILS_HPP

#include <boost/asio.hpp>
#include <boost/serialization/split_free.hpp>
#include <string>

using namespace std;
using namespace boost;
using namespace boost::asio;

namespace boost {
namespace serialization {

template <class Archive, class Protocol>
inline void save(Archive &ar, const ip::basic_endpoint<Protocol> &e, const unsigned int version) {
  string ip = e.address().to_string();
  ip::port_type port = e.port();
  ar &ip;
  ar &port;
}
template <class Archive, class Protocol>
inline void load(Archive &ar, ip::basic_endpoint<Protocol> &e, const unsigned int version) {
  string ip;
  ip::port_type port;
  ar &ip;
  ar &port;
  e = ip::basic_endpoint<Protocol>(ip::address::from_string(ip), port);
}

template <class Archive, class Protocol>
inline void serialize(Archive &ar, ip::basic_endpoint<Protocol> &e, const unsigned int file_version) {
  split_free(ar, e, file_version);
}

} // namespace serialization
} // namespace boost


#endif
