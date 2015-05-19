#ifndef __FILEMANAGER_H__
#define __FILEMANAGER_H__

#include <iostream>
#include <fstream>
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <list>

class WriteObject {
 public:
  int index;
  int length;
  char *data;
  WriteObject(int, int, char *);
};

class FileManager {
  FILE *fs;
  char *buffer;
  int length;
  int piece_length;
  int number_pieces;
  bool *have;
  std::set<int> needed;
  bool good;
  std::list<WriteObject *> *write_queue;
 public:
  FileManager(char *, int, int, int, bool);
  char *getPiece(int);
  void addToWriteQueue(int, int, char *);
  void write();
  std::set<int> *getNeeded(); 
  char *getPointerToData(int, int);
  bool *getHave(); 
  bool isGood();
  int sizeofPiece(int);
  void save();
};

#endif
