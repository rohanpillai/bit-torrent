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
#include <list>

#define MAX_BUFFER_LENGTH 100

using namespace std;

class Peer {
  int sockfd;
  bool good;
 public:
  unsigned char *info_hash;
  Peer(peer_t *, unsigned char *);
  Peer(int, unsigned char *);
  void sendToPeer(char *, int);
  void readFromPeer(char **, int*);
  bool isGood();
  void update();
};

#endif
