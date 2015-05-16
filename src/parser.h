#ifndef _PARSER_H_
#define _PARSER_H_

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <list>
#include <map>
#include <math.h>
#include <bt_lib.h>

#define BENCODED_STRING 1
#define BENCODED_INTEGER 2
#define BENCODED_LIST 3
#define BENCODED_DICT 4


class bencodedObject {
 public:
  int type;
  virtual void printValue(std::stringstream &) = 0;  
};

class bencodedString: public bencodedObject {
 public:
  std::string value;
  bencodedString(char **str);
  void printValue(std::stringstream &);
};

class bencodedDict: public bencodedObject {
 public:
  std::map<bencodedString *, bencodedObject *> bmap;
  bencodedDict();
  void insert(char **);
  void printValue(std::stringstream &);
  bencodedObject *getValueForKey(std::string);
};

class bencodedInteger: public bencodedObject {
 public:
  int value;
  bencodedInteger(char **str);
  void printValue(std::stringstream &);
};

class bencodedList: public bencodedObject {
 public:
  std::list<bencodedObject *> lst;
  bencodedList(); 
  void insert(char **str);
  void printValue(std::stringstream &);
};

bencodedDict *parse_torrent_file(char *);
bt_info_t *extract_bt_info(bencodedDict *);
void printInfo(bt_info_t *);
#endif
