#ifndef _TYPES_H_
#define _TYPES_H_
#include <iostream>

class MetaInfo {
 public:
  const char *announce;
  const char *name;
  const char *pieces;
  long piece_length;
  long file_length;
};

#endif

