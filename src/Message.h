#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include <arpa/inet.h>
#include <iostream>
#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <list>

#define HANDSHAKING 1

class Message {
  int type;
  int size;
  char *data;
 public:
  Message(int, int, char *);
  ~Message();
  char *getData();
  void toByteArray(char **, int *);
};

std::list<Message *> *makeMessages(char *, int);

#endif
