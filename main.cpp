#include <algorithm>
#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <vector>

#include "gossip_protocol.hpp"

using namespace std;
using namespace boost;
using namespace boost::asio;

int main(int argc, char *argv[]) {
  if (argc == 1) {
    argc = 5;
    char *temp[] = {(char *)"", (char *)"0.0.0.0", (char *)"12345", (char *)"0.0.0.0", (char *)"12346"};
    argv = temp;
  }

  if (!(argc % 2)) {
    cout << "Error: should be: cache-cluster Self-IP Self-Port [[O1-IP] [O1-Port], [O2-IP] [O2-Port], ...]\n";
    return 1;
  }

  ip::address ip = ip::address::from_string("0.0.0.0");
  ip::port_type port = 12345;

  if (argc >= 3) {
    system::error_code ec;
    ip = ip::address::from_string(argv[1], ec);
    if (ec) {
      cout << "Error: can't parse Self-IP";
      return 1;
    }

    port = stoi(string(argv[2]));
  }

  try {
    GossipProtocal::GossipProtocal boss(ip, port);
    vector<GossipProtocal::Member> members;
    for (int w = 3; w < argc; w += 2) {
      system::error_code ec;
      ip::address oip = ip::address::from_string(argv[w], ec);
      if (ec) {
        cout << "Error: can't parse O" << w / 2 << "-IP";
        return 1;
      }

      ip::port_type oport = stoi(string(argv[w + 1]));
      members.push_back(GossipProtocal::Member(oip, oport));
    }
    boss.add_members(members.begin(), members.end());

    boss.run();
  } catch (const std::exception &ex) {
    cout << ex.what() << std::endl;
  }
  return 0;
}
