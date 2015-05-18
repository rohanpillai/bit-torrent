#include <PeerManager.h>

void PeerManager::addPeer(Peer *peer) {
  pool.push_front(peer);
}

void PeerManager::updatePeers() {
  for (list<Peer *>::iterator it = pool.begin(); it != pool.end(); it++) {
    (*it)->update();
  }
}

