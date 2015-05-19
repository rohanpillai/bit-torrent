#ifndef __PEER_MANAGER__
#define __PEER_MANAGER__

#include <iostream>
#include <list>
#include <set>
#include <algorithm>
#include <Peer.h>
#include <FileManager.h>
#include <bt_lib.h>

using namespace std;

class PeerManager {
  list<Peer *> pool;
  FileManager *fileManager;
  int file_length;
  int piece_length;
  int num_pieces;
  char **piece_hashes;
  bool good;
  bool seeding;
 public:
  PeerManager(char *, bt_info_t *, bool);
  void addPeer(Peer *);
  void updatePeers();
  int pickPiece(Peer *);
};

#endif
