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

int Message::getSize() {
  return size;
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

Message *makeRequest(int index, int begin, int length) {
  char *_data = new char[12];
  int *n_index = new int[1];
  int *n_begin = new int[1];
  int *n_length = new int[1];
  *n_index = htonl(index);
  *n_begin = htonl(begin);
  *n_length = htonl(length);
  memcpy(_data, n_index, 4);
  memcpy(_data + 4, n_begin, 4);
  memcpy(_data + 8, n_length, 4);
  Message *message = new Message(REQUEST, 12, _data);
  delete n_index;
  delete n_begin;
  delete n_length;
  return message;
}

Message *makePiece(int index, int begin, char *piece, int piece_size) {
  int message_size = piece_size + 8;
  char *_data = new char[message_size];
  int *n_index = new int[1];
  int *n_begin = new int[1];
  *n_index = htonl(index);
  *n_begin = htonl(begin);
  memcpy(_data, n_index, 4);
  memcpy(_data + 4, n_begin, 4);
  memcpy(_data + 8, piece, piece_size);
  Message *message = new Message(PIECE, message_size, _data);
  delete n_index;
  delete n_begin;
  return message;
}

Message *makeChoke() {
  int message_size = 1;
  char *_data = new char[1];
  _data[0] = '\0';
  Message *message = new Message(CHOKE, message_size, _data);
  return message;
}

Message *makeUnchoke() {
  int message_size = 1;
  char *_data = new char[1];
  _data[0] = '\0';
  Message *message = new Message(UNCHOKE, message_size, _data);
  return message;
}
  
Message *makeInterested() {
  int message_size = 1;
  char *_data = new char[1];
  _data[0] = '\0';
  Message *message = new Message(INTERESTED, message_size, _data);
  return message;
}

Message *makeNotInterested() {
  int message_size = 1;
  char *_data = new char[1];
  _data[0] = '\0';
  Message *message = new Message(NOT_INTERESTED, message_size, _data);
  return message;
}

Message *makeHave(int _have) {
  int message_size = 4;
  char *_data = new char[4];
  int *n_have = new int[1];
  *n_have = htonl(_have);
  memcpy(_data, n_have, 4);
  Message *message = new Message(HAVE, message_size, _data);
  delete n_have;
  return message;
}

char power_2(int index) {
  char x = 0x01;
  return x<<index;
}

void setBit(char *array, int index) {
  char tmp = power_2(index);
  *array = *array | tmp;
}

Message *makeBitfield(bool *array, int num_pieces) {
  int num_bytes = (int) ceil(((float) num_pieces)/8.0);
  int message_size = num_bytes + 4;
  char *_data = new char[message_size];
  bzero(_data, message_size);

  for (int i = 0; i < num_pieces; i++) {
    if (array[i]) {
      int byte_num = (int) floor(((float) i)/8.0);
      int index = i % 8;
      char *current_byte = _data + byte_num;
      setBit(current_byte, 7 - index);
    }
  }
  printf("Bitfield ");
  for (int i = 0; i < num_bytes; i++) {
    printf("%02x ", (unsigned char) *(_data + i));
  }
  printf("\n");
  int *n_num = new int[1];
  *n_num = htonl(num_bytes);
  memcpy(_data + num_bytes, n_num, 4);
  Message *message = new Message(BITFIELD, message_size, _data);
  return message;
}
