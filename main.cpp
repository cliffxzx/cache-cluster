#include <algorithm>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "gossip.hpp"

using boost::asio::ip::udp;
using boost::program_options::error;
using boost::program_options::notify;
using boost::program_options::options_description;
using boost::program_options::parse_command_line;
using boost::program_options::store;
using boost::program_options::value;
using boost::program_options::variables_map;
using gossip::Gossip;
using gossip::Member;
using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::launch;
using std::make_unique;
using std::string;
using std::unique_ptr;
using std::vector;

unique_ptr<error> parse_args(
    int argc, char *argv[],
    Member &self_member,
    vector<Member> &memberlist) {
  options_description options("Cache Cluster CLI");
  options.add_options()
      .
      operator()("help", "TODO")
      .
      operator()("self,s",
                 value(&self_member)
                     ->value_name("[ip] [port]"),
                 "The endpoint of self")
      .
      operator()("memberlist,m",
                 value(&memberlist)
                     ->value_name("[ip] [port]")
                     ->multitoken(),
                 "The endpoints of memberlist");

  variables_map args;
  try {
    store(parse_command_line(argc, argv, options), args);
    notify(args);
  } catch (error e) {
    cerr << e.what() << endl;
    return make_unique<error>(e);
  }

  return nullptr;
}

int main(int argc, char *argv[]) {
  Member self_member("0.0.0.0 7777");
  vector<Member> memberlist;

  parse_args(argc, argv, self_member, memberlist);

  if (memberlist.empty()) {
    memberlist.insert(memberlist.end(), {"0.0.0.0 7777"});
  }

  try {
    auto receiver = [](string data) {
      cerr << "main::run::receiver: " << data;
    };

    auto server = Gossip(self_member, receiver);
    for (auto member : memberlist) {
      server.add_member(member);
    }

    future<void> res = async(launch::async, &Gossip::run, &server);

    string command;
    while (cin >> command) {
      cout << command;
    }

    res.get();
  } catch (const std::exception &ex) {
    cerr << ex.what() << std::endl;
  }

  return 0;
}
