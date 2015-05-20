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
  *len = 0;
  fd_set input;
  FD_ZERO(&input);
  FD_SET(sockfd, &input);
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = TIMEOUT_MILLIS * 1000;
  int n = select(sockfd + 1, &input, NULL, NULL, &timeout);
  if (n == -1) {
    printf("Error while non blocking the socket.\n");
    return;
  } 
  if (n == 0) {
    return;
  }

  char *buffer = new char[MAX_BUFFER_LENGTH];
  *len = recv(sockfd, buffer, MAX_BUFFER_LENGTH, 0);
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

int Peer::getSockfd() {
  return sockfd;
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
    if (expected_values.size() == 0) {
      handshakeComplete = true;
      break;
    }

    char *recv_bytes = new char[MAX_BUFFER_LENGTH];
    int length = recv(peer->getSockfd(), recv_bytes, MAX_BUFFER_LENGTH, 0);
    makeMessages(recv_bytes, length, peer->in_queue);
    while (peer->in_queue->size() > 0) {
      if (expected_values.size() == 0) {
        handshakeComplete = true;
        break;
      }

      Message *message = peer->in_queue->front();
      char *expected_string = expected_values.front();
      if (strcmp(message->getData(), expected_string) != 0) {
        printf("Handshaking failed!");
        return false;
      }
      peer->in_queue->pop_front();
      expected_values.pop_front();
      delete message;
      delete [] expected_string;
    }
  }
  return true;
}

void Peer::Peer_common() {
  active_download = false;
  active_seeding = false;
  piece_complete = false;
}

Peer::Peer(peer_t *pt, unsigned char *hash) {
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  int flag = connect(sockfd, pt->addr->ai_addr, pt->addr->ai_addrlen);
  info_hash = new unsigned char[20];
  memcpy(info_hash, hash, 20);
  in_queue = new list<Message *>();

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
  good = true;
  Peer_common();
}

Peer::Peer(int fd, unsigned char *hash) {
  sockfd = fd;
  info_hash = new unsigned char[20];
  memcpy(info_hash, hash, 20);
  in_queue = new list<Message *>();
  if (!handshake(this)) {
    good = false;
    sockfd = -1;
    return;
  }
  good = true;
  Peer_common();
}

bool Peer::isGood() {
  return good;
}

bool Peer::exchangeBitfield(bool *have, int num_pieces) {
  Message *message = makeBitfield(have, num_pieces);
  char *byte_array;
  int len;
  message->toByteArray(&byte_array, &len);
  sendToPeer(byte_array, len);

  if (in_queue->size() == 0) {
    char *recv_bytes = new char[MAX_BUFFER_LENGTH];
    int length = recv(sockfd, recv_bytes, MAX_BUFFER_LENGTH, 0);
    makeMessages(recv_bytes, length, in_queue);
  }

  Message *in_message = in_queue->front();
  bool *peer_has;
  if (in_message->getType() == BITFIELD) {
    int num_pieces_peer;
    peer_has = getBitfield(in_message, &num_pieces_peer);
    if (num_pieces_peer != num_pieces) {
      printf("The number of pieces in peer does not match the pieces specified by the torrent file.\n");
      return false;
    }
    delete in_message;
    in_queue->pop_front();
  } else {
    printf("Expected a Bitfield message. Received %d message. Aborting peer.\n", in_message->getType());
    return false;
  }
  could_get = new set<int>();
  for (int i = 0; i < num_pieces; i++) {
    if(peer_has[i]) {
      could_get->insert(i);
    }
  }
  printf("Complete exchange of bitfield\n");
  return true;
}

void Peer::setFileManager(FileManager *m) {
  fileManager = m;
}

set<int> *Peer::getCouldGet() {
  return could_get;
}

void Peer::makeRequestForPart() {
  int packet_length = PAYLOAD_LENGTH;
  if ((request_piece_length - request_begin) < packet_length) {
    packet_length = request_piece_length - request_begin;
  }
  Message *message = makeRequest(request_index, request_begin, packet_length);
  char *byte_array;
  int len;
  message->toByteArray(&byte_array, &len);
  sendToPeer(byte_array, len);

  request_begin += packet_length;
  if (request_begin >= request_piece_length) {
    all_request_sent = true;
  }
}

void sendMessage(Peer *peer, Message *message) {
  char *byte_array;
  int len;
  message->toByteArray(&byte_array, &len);
  peer->sendToPeer(byte_array, len);
}

void Peer::downloadPiece(int index, int piece_length) {
  request_index = index;
  request_piece_length = piece_length;
  request_begin = 0;
  incoming_buffer = new char[piece_length];
  active_download = true;
  active_seeding = false;
  all_request_sent = false;
  piece_complete = false;
  makeRequestForPart();
}
 
void Peer::reset() {
  Peer_common();
}

void Peer::update() {

  char *byte_array;
  int len;
  readFromPeer(&byte_array, &len);
  if (len == 0) {
    return;
  }
  makeMessages(byte_array, len, in_queue);

  while (in_queue->size() > 0) {
    Message *message = in_queue->front();
    switch(message->getType()) {
      case(REQUEST): {
        int index, begin, length;
        getRequestAttributes(message, &index, &begin, &length);
        char *buffer = fileManager->getPointerToData(index, begin);
        Message *out_message = makePiece(index, begin, buffer, length);
        sendMessage(this, out_message);
        break;
      }
      case(PIECE): {
        int index, begin, length;
        char *buffer;
        getPieceAttributes(message, &index, &begin, &length, &buffer);
        memcpy(incoming_buffer + begin, buffer, length);
        if ((begin + length) >= request_piece_length) {
          piece_complete = true;
        } else {
          if (active_download) {
            if (!all_request_sent) {
              makeRequestForPart();
            }
          }
        }
        break;
      }
    }
    in_queue->pop_front();
    delete message;
  }
  if (piece_complete) {
    fileManager->addToWriteQueue(request_index, request_piece_length, incoming_buffer);
  }
}

void Peer::makeActive() {
  active = true;
}

void Peer::makeInactive() {
  active = false;
}

bool Peer::isActive() {
  return active;
}

bool Peer::completePiece() {
  return piece_complete;
}

