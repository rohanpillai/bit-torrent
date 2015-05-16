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

bencodedString::bencodedString(char **str) {
  char *next;
  value = getValue(*str, &next);
  *str = next;
  type = BENCODED_STRING;
}

void bencodedString::printValue(stringstream &ss) {
  ss << '"' << value << '"';
}

bencodedDict::bencodedDict() {
  type = BENCODED_DICT;
}

bencodedInteger::bencodedInteger(char **str) {
  ostringstream oss;
  while (*(*str) != 'e') {
    oss << *(*str);
    (*str)++;
  }
  (*str)++;
  value = stoi(oss.str()); 
  type = BENCODED_INTEGER;
}

void bencodedInteger::printValue(stringstream &oss) {
  oss << value ;
}

bencodedList::bencodedList() {
  type = BENCODED_LIST;
}
void bencodedList::insert(char **str) {
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
void bencodedList::printValue(stringstream &oss) {
  oss << "[";
  for (list<bencodedObject *>::iterator it=lst.begin(); it != lst.end(); it++) {
    (*it)->printValue(oss);
    oss << ',';
  }
  oss << "]\n";
}

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

void bencodedDict::printValue(stringstream &oss) {
  oss << "{";
  for (map<bencodedString *, bencodedObject *>::iterator it = bmap.begin(); it != bmap.end(); it++) {
    oss << '(';
    ((*it).first)->printValue(oss);
    oss << " , ";
    ((*it).second)->printValue(oss);
    oss << ')';
  }
  oss << "}\n";
}

bencodedObject *bencodedDict::getValueForKey(string key) {
  for (map<bencodedString *, bencodedObject *>::iterator it = bmap.begin(); it != bmap.end(); it++) {
    if (key.compare(((*it).first)->value) == 0) {
      return (*it).second;
    }
  }
  return NULL;
}

bencodedDict *parse_torrent_file(char *file_name) {
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

bt_info_t *extract_bt_info(bencodedDict *metadata) {

  string info("info");
  string name("name");
  string piece_length("piece length");
  string length("length");
  string pieces("pieces");

  bt_info_t *bt_info = (bt_info_t *) malloc(sizeof(bt_info_t));

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
  
  strncpy(bt_info->name, (((bencodedString *)bname)->value).c_str(), FILE_NAME_MAX);

  bencodedObject *bpiece_length= binfo_dict->getValueForKey(piece_length);
  if (bpiece_length->type != BENCODED_INTEGER) {
    cout << "The value for piece_length in the torrent file should be an integer\n";
    return NULL;
  }
  bt_info->piece_length = (((bencodedInteger *) bpiece_length)->value); 

  bencodedObject *bfile_length= binfo_dict->getValueForKey(length);
  if (bfile_length->type != BENCODED_INTEGER) {
    cout << "The value for file length in the torrent file should be an integer\n";
    return NULL;
  }
  bt_info->length = (((bencodedInteger *) bfile_length)->value); 
  
  bt_info->num_pieces = (int) ceil(((float) bt_info->length)/((float) bt_info->piece_length));

  bencodedObject *bpieces = binfo_dict->getValueForKey(pieces);
  if (bpieces->type != BENCODED_STRING) {
    cout << "The value for pieces in the torrent file should be a string\n";
    return NULL;
  }
  const char *piece_hashes = (((bencodedString *) bpieces)->value).c_str(); 
  bt_info->piece_hashes = new char*[bt_info->num_pieces];

  for (int i=0; i < bt_info->num_pieces; i++) {
    bt_info->piece_hashes[i] = new char[20];
    strncpy(bt_info->piece_hashes[i], (piece_hashes + i*20), 20);
  }
  return bt_info;
}

void printInfo(bt_info_t *bt_info) {
  printf("Name: %s\n", bt_info->name);
  printf("File length: %d\n", bt_info->length);
  printf("Piece length: %d\n", bt_info->piece_length);
  printf("Number of pieces: %d\n", bt_info->num_pieces);
  for (int i=0; i< bt_info->num_pieces; i++) {
    printf("Hash value for piece %d: ", i + 1);
    for (int j=0; j < 20; j++) {
      printf("%02x", (unsigned char) *(bt_info->piece_hashes[i] + j));
    }
    printf("\n");
  }
}
