#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include <arpa/inet.h>
#include <iostream>
#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <list>
#include <math.h>

#define HANDSHAKING 1
#define CHOKE 2
#define UNCHOKE 3
#define INTERESTED 4
#define NOT_INTERESTED 5
#define HAVE 6
#define BITFIELD 7
#define REQUEST 8
#define PIECE 9
#define CANCEL 10


class Message {
  int type;
  int size;
  char *data;
 public:
  Message(int, int, char *);
  ~Message();
  char *getData();
  int getSize();
  int getType();
  void toByteArray(char **, int *);
};

void makeMessages(char *, int, std::list<Message *> *);
Message *makeBitfield(bool *, int);
Message *makeHave(int);
Message *makeNotInterested();
Message *makeInterested();
Message *makeUnchoke();
Message *makeChoke();
Message *makePiece(int, int, char *, int);
Message *makeRequest(int, int, int);

bool *getBitfield(Message *, int *);
void getPieceAttributes(Message *, int *, int *, int *, char **);
void getRequestAttributes(Message *, int *, int *, int *);
#endif
