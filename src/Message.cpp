#include <Message.h>

using namespace std;

Message::Message(int _type, int _size, char *_data) {
  type = _type;
  size = _size;
  data = _data;
}

Message::~Message() {
  delete [] data;
}

char *Message::getData() {
  return data;
}

void Message::toByteArray(char **array, int *len) {
  char *byte_array = new char[size + 4 + 4];
  int *n_type = new int[1];
  int *n_size = new int[1];
  *n_type = htonl(type);
  *n_size = htonl(size);
  memcpy(byte_array, n_type, 4);
  memcpy(byte_array + 4, n_size, 4);
  memcpy(byte_array + 8, data, size);
  *array = byte_array;
  *len = size + 2*sizeof(int);
}

list<Message *> *makeMessages(char *_recv, int len) {
  char *orig_recv = _recv;
  int navigated_len = 0;
  list<Message *> *lst = new list<Message *>;
  while (navigated_len < len) {
    int *n_type = (int *) _recv;
    int *n_size = (int *) (_recv + sizeof(int));
    int _type = ntohl(*n_type);
    int _size = ntohl(*n_size);
  
    char *_data = new char[_size];
    memcpy(_data, (_recv + 2*sizeof(int)), _size);
    Message *message = new Message(_type, _size, _data);
    lst->push_back(message);
    navigated_len += _size + 2*sizeof(int);
    _recv = orig_recv + navigated_len;
  }
  delete [] orig_recv;
  return lst;
}

