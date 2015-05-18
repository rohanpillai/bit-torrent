#ifndef __PEER_MANAGER__
#define __PEER_MANAGER__

#include <iostream>
#include <list>
#include <Peer.h>

using namespace std;

class PeerManager {
  list<Peer *> pool;
 public:
  void addPeer(Peer *);
  void updatePeers();
};

#endif
