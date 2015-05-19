#ifndef __PEER_H__
#define __PEER_H__

#include <iostream>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <bt_lib.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <Message.h>
#include <FileManager.h>
#include <list>
#include <set>

#define MAX_BUFFER_LENGTH 2048
#define PAYLOAD_LENGTH 512
#define TIMEOUT_MILLIS 10

using namespace std;

class Peer {
  int sockfd;
  bool good;
  set<int> *could_get;
  char *incoming_buffer;
  bool active_download;
  bool active_seeding;
  bool all_request_sent;
  bool piece_complete;
  bool active;
  int request_index;
  int request_begin;
  int request_piece_length;
  FileManager *fileManager;
 public:
  unsigned char *info_hash;
  list<Message *> *in_queue;
  Peer(peer_t *, unsigned char *);
  Peer(int, unsigned char *);
  void Peer_common();
  int getSockfd();
  void sendToPeer(char *, int);
  void readFromPeer(char **, int*);
  bool isGood();
  bool exchangeBitfield(bool *, int);
  set<int> *getCouldGet();
  void setFileManager(FileManager *);
  void makeRequestForPart();
  void downloadPiece(int, int);
  void update();
  void reset();
  bool completePiece();
  bool isActive();
  void makeActive();
  void makeInactive();
};

#endif
