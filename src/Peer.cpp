#include <Peer.h>

using namespace std;

void Peer::sendToPeer(char *msg, int len) {
  int sendlen = 0;
  while (sendlen < len) {
    int bytes_send = send(sockfd, msg, (len - sendlen), 0);
    if (bytes_send < 0) {
      printf("ERROR: %s\n", strerror(errno));
      return;
    }
    sendlen += bytes_send;
  }
}

void Peer::readFromPeer(char **dst, int *len) {
  char *buffer = new char[MAX_BUFFER_LENGTH];
  *len = recv(sockfd, buffer, MAX_BUFFER_LENGTH, 0);
  if (*len == 0) {
    delete [] buffer;
    return;
  }
  *dst = buffer;
  return;
}

void extract_part(char **orig, char **extract, int *length, int size) {
  char *val = new char[size];
  memcpy(val, *orig, size);
  *orig = (*orig) + size;
  *extract = val;
  *length -= size;
}

bool handshake(Peer *peer) {
  list<Message *> out_messages;
  list<char *> expected_values;

  char *value1 = new char[20];
  char *exp1 = new char[20];
  int i = 19;
  bzero(value1, 20);
  bzero(exp1, 20);
  sprintf(value1, "%cBitTorrentProtocol", (unsigned char) i);
  memcpy(exp1, value1, 20);
  out_messages.push_back(new Message(HANDSHAKING, 20, value1));
  expected_values.push_back(exp1);

  char *value2 = new char[8];
  char *exp2 = new char[8];
  bzero(value2, 8);
  bzero(exp2, 8);
  out_messages.push_back(new Message(HANDSHAKING, 8, value2));
  expected_values.push_back(exp2);

  char *value3 = new char[20];
  char *exp3 = new char[20];
  memcpy(value3, peer->info_hash, 20);
  memcpy(exp3, peer->info_hash, 20);
  out_messages.push_back(new Message(HANDSHAKING, 20, value3));
  expected_values.push_back(exp3);

  while (out_messages.size() > 0) {
    Message *message = out_messages.front();
    char *byte_array;
    int array_len;
    message->toByteArray(&byte_array, &array_len);
    peer->sendToPeer(byte_array, array_len);
    out_messages.pop_front();
    delete message;
  }

  bool handshakeComplete = false;
  while (!handshakeComplete) {
    char *recv_bytes;
    int length;
    peer->readFromPeer(&recv_bytes, &length);
    list<Message *> *in_messages = makeMessages(recv_bytes, length);
    while (in_messages->size() > 0) {
      if (expected_values.size() == 0) {
        handshakeComplete = true;
        break;
      }

      Message *message = in_messages->front();
      char *expected_string = expected_values.front();
      if (strcmp(message->getData(), expected_string) != 0) {
        return false;
      }
      in_messages->pop_front();
      expected_values.pop_front();
      delete message;
      delete [] expected_string;
    }
  }
  return true;
}

Peer::Peer(peer_t *pt, const char *hash) {
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  int flag = connect(sockfd, pt->addr->ai_addr, pt->addr->ai_addrlen);
  if (flag < 0) {
    printf("ERROR: %s\n", strerror(errno));
    good = false;
    sockfd = -1;
    return;
  }
  if (!handshake(this)) {
    good = false;
    sockfd = -1;
    return;
  }
  
  info_hash = new char[20];
  memcpy(info_hash, hash, 20);
  good = true;
}

Peer::Peer(int fd, const char *hash) {
  sockfd = fd;
  if (!handshake(this)) {
    good = false;
    sockfd = -1;
    return;
  }
  info_hash = new char[20];
  memcpy(info_hash, hash, 20);
  good = true;
}

bool Peer::isGood() {
  return good;
}

void Peer::update() {
  cout << "In update\n";
}


