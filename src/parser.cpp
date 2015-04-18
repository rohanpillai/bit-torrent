#include <parser.h>

using namespace std;

bool isString(char *str) {
  if ((((int) str[0]) > 47) && (((int) str[0]) < 58)) {
    return true;
  }
  return false;
}

string getValue(char *str, char **next) {
  ostringstream oss;
  while ((*str) != ':') {
    oss << (*str);
    str++;
  }
  str++;
  int size = stoi(oss.str());
  oss.clear();
  oss.str("");
  for (int i = 0; i < size; i++) {
    oss << (*str);
    str++;
  }
  *next = str;
  return oss.str();
}

class bencodedObject {
 public:
  int type;
  virtual void printValue() = 0;  
};

class bencodedString: public bencodedObject {
 public:
  string value;
  bencodedString(char **str) {
    char *next;
    value = getValue(*str, &next);
    *str = next;
    type = BENCODED_STRING;
  }
  void printValue() {
    cout << value;
  }
};

class bencodedDict: public bencodedObject {
 public:
  map<bencodedString *, bencodedObject *> bmap;
  bencodedDict() {
    type = BENCODED_DICT;
  }
  void insert(char **);
  void printValue();
  bencodedObject *getValueForKey(string);
};

class bencodedInteger: public bencodedObject {
 public:
  long value;
  bencodedInteger(char **str) {
    ostringstream oss;
    while (*(*str) != 'e') {
      oss << *(*str);
      (*str)++;
    }
    (*str)++;
    value = stol(oss.str()); 
    type = BENCODED_INTEGER;
  }
  void printValue() {
    cout << value ;
  }
};
  

class bencodedList: public bencodedObject {
 public:
  list<bencodedObject *> lst;
  bencodedList() {
    type = BENCODED_LIST;
  }
  void insert(char **str) {
    if (isString(*str)) {
      bencodedString* bstr = new bencodedString(str);
      lst.push_back(bstr);
    } else {
      switch (*(*str)) {
        case 'i': {
          (*str)++;
          bencodedInteger* bint = new bencodedInteger(str);
          lst.push_back(bint);
          break;
        }
        case 'e': {
          (*str)++;
          return;
        }
        case 'l': {
          bencodedList *blist = new bencodedList();
          (*str)++;
          blist->insert(str);
          lst.push_back(blist);
          break;
        }
        case 'd': {
          bencodedDict *bdict = new bencodedDict();
          (*str)++;
          bdict->insert(str);
          lst.push_back(bdict);
          break;
        }
        default: {
          cout << "Unexpected value " << *str << '\n';
        }
      }
    }
    insert(str);
  }
  void printValue() {
    cout << "list:";
    for (list<bencodedObject *>::iterator it=lst.begin(); it != lst.end(); it++) {
      (*it)->printValue();
      cout << ',';
    }
    cout << '\n';
  }
};

bencodedObject *constructValue(char **str) {
  if (isString(*str)) {
    bencodedString *value = new bencodedString(str);
    return value;
  }
  switch(*(*str)) {
    case 'i': {
      (*str)++;
      return (new bencodedInteger(str));
      
    }
    case 'l': {
      (*str)++;
      bencodedList *value = new bencodedList();
      value->insert(str);
      return value;
    }
    case 'd': {
      (*str)++;
      bencodedDict *value = new bencodedDict();
      value->insert(str);
      return value;
    }
  }
}

void bencodedDict::insert(char **str) {
  bencodedString *key = new bencodedString(str);
  bencodedObject *value = constructValue(str);
    
  bmap[key] = value;
  if (*(*str) == 'e') {
    (*str)++;
    return;
  }
  insert(str);
}

void bencodedDict::printValue() {
  cout << "dict:\t";
  for (map<bencodedString *, bencodedObject *>::iterator it = bmap.begin(); it != bmap.end(); it++) {
    cout << '(';
    ((*it).first)->printValue();
    cout << " , ";
    ((*it).second)->printValue();
    cout << ')';
  }
  cout << '\n';
}

bencodedObject *bencodedDict::getValueForKey(string key) {
  for (map<bencodedString *, bencodedObject *>::iterator it = bmap.begin(); it != bmap.end(); it++) {
    if (key.compare(((*it).first)->value) == 0) {
      return (*it).second;
    }
  }
  return NULL;
}

bencodedDict *parse_file(char *file_name) {
  ifstream ifs(file_name);
  bencodedDict *bdict = NULL;
  if (ifs) {
    ifs.seekg(0, ifs.end);
    int size = ifs.tellg();
    ifs.seekg(0, ifs.beg);

    char *buffer = new char[size];
    ifs.read(buffer, size);
    ifs.close();

    if (*buffer == 'd') {
      bdict = new bencodedDict();
      buffer++;
      bdict->insert(&buffer);
    }
  } 
  return bdict;
}

MetaInfo *extractMetaInfo(char *file_name) {
  bencodedDict *metadata = parse_file(file_name);
  if (metadata == NULL) {
    cout << "Bad torrent file.\n";
    return NULL;
  }

  string announce("announce");
  string info("info");
  string name("name");
  string piece_length("piece length");
  string length("length");
  string pieces("pieces");

  MetaInfo *metaInfo = (MetaInfo *) malloc(sizeof(MetaInfo));
  bencodedObject *bannounce = metadata->getValueForKey(announce);
  if (bannounce->type != BENCODED_STRING) {
    cout << "The value for announce is the torrent file is not a string\n";
    return NULL;
  }
  metaInfo->announce = (((bencodedString *) bannounce)->value).c_str();
// (metaInfo->announce).assign(((bencodedString *) bannounce)->value);
  bencodedObject *binfo = metadata->getValueForKey(info);
  if (binfo->type != BENCODED_DICT) {
    cout << "The value for info key is not a dictionary\n";
    return NULL;
  }
  bencodedDict *binfo_dict = (bencodedDict *) binfo;
  bencodedObject *bname = binfo_dict->getValueForKey(name);
  if (bname->type != BENCODED_STRING) {
    cout << "The value for name in the torrent file should be a string\n";
    return NULL;
  }
  metaInfo->name = (((bencodedString *) bname)->value).c_str(); 
  bencodedObject *bpieces = binfo_dict->getValueForKey(pieces);
  if (bpieces->type != BENCODED_STRING) {
    cout << "The value for pieces in the torrent file should be a string\n";
    return NULL;
  }
  metaInfo->pieces = (((bencodedString *) bpieces)->value).c_str(); 

  bencodedObject *bpiece_length= binfo_dict->getValueForKey(piece_length);
  if (bpiece_length->type != BENCODED_INTEGER) {
    cout << "The value for piece_length in the torrent file should be an integer\n";
    return NULL;
  }
  metaInfo->piece_length = (((bencodedInteger *) bpiece_length)->value); 

  bencodedObject *bfile_length= binfo_dict->getValueForKey(length);
  if (bfile_length->type != BENCODED_INTEGER) {
    cout << "The value for file length in the torrent file should be an integer\n";
    return NULL;
  }
  metaInfo->file_length = (((bencodedInteger *) bfile_length)->value); 
  
  return metaInfo;
}

void printMetaInfo(MetaInfo *metaInfo) {
  cout << "URL of torrent tracker " << metaInfo->announce << '\n';
  cout << "Name of file: " << metaInfo->name << '\n';
  cout << "Length of file: " << metaInfo->file_length << '\n';
  cout << "Length of piece: " << metaInfo->piece_length << '\n';
  cout << "SHA1 hash of pieces: " << metaInfo->pieces << '\n';
}

int main(int argc, char *argv[]) {
  char *file_name = argv[1];
  MetaInfo *metaInfo = extractMetaInfo(file_name);
  printMetaInfo(metaInfo);
  return 0;
}
