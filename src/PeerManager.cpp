#include <PeerManager.h>

using namespace std;

PeerManager::PeerManager(char *file_name, bt_info_t *bt_info, bool complete) {
  file_length = bt_info->length;
  piece_length = bt_info->piece_length;
  num_pieces = bt_info->num_pieces;
  piece_hashes = bt_info->piece_hashes;
  good = true;
  seeding = false;
  if (complete) {
    seeding = true;
  }

  srand(time(NULL));
  fileManager = new FileManager(file_name, bt_info->length, bt_info->piece_length, bt_info->num_pieces, complete);
  if (!fileManager->isGood()) {
    printf("Unable to create Peer Manager due to bad File manager\n");
    good = false;
    return;
  }
  
}

int pickRandom(set<int> s) {
  int r = rand() % s.size();
  set<int>::const_iterator it(s.begin());
  advance(it, r);
  return *it;
}

int PeerManager::pickPiece(Peer *peer) {
  set<int> *could_get = peer->getCouldGet();
  set<int> *needed = fileManager->getNeeded();
  set<int> intersect;
  set_intersection(could_get->begin(), could_get->end(), needed->begin(), needed->end(), inserter(intersect, intersect.begin()));
  if (intersect.size() > 0) {
    return pickRandom(intersect);
  } else {
    return -1;
  }
}

void PeerManager::addPeer(Peer *peer) {
  bool *have = fileManager->getHave();
  if (peer->exchangeBitfield(have, num_pieces)) {
    pool.push_front(peer);
    peer->setFileManager(fileManager);
    if (!seeding) {
      int index = pickPiece(peer);
      if (index >= 0) {
        int length = fileManager->sizeofPiece(index);
        peer->downloadPiece(index, length);
        peer->makeActive();
      }
    }
  }
}

void PeerManager::updatePeers() {
  for (list<Peer *>::iterator it = pool.begin(); it != pool.end(); it++) {
    (*it)->update();
    if ((*it)->completePiece() && (*it)->isActive()) {
      (*it)->reset();
      (*it)->makeInactive();
      fileManager->write();
      set<int> *needed = fileManager->getNeeded();
      if (needed->size() == 0) {
        fileManager->save();
        seeding = true; 
      } else {
        int index = pickPiece((*it));
        if (index >= 0) {
          int length = fileManager->sizeofPiece(index);
          (*it)->downloadPiece(index, length);
          (*it)->makeActive();
        }
      }
    }
  }
}

