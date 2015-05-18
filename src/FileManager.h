#include <iostream>
#include <fstream>
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

class FileManager {
  fstream fs;
  char *buffer;
  int length;
  int piece_length;
  int number_pieces;
  bool *have;
  set<int> needed;
 public:
  FileManager(char *);
  char *getPiece(int);
  void writePiece(int, char *);
  int pickPieceToGet();
  bool *getHave();  
};

