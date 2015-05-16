#include <sha1check.h>

using namespace std;

char *getSHA1(char *data) {
  unsigned char *hash = (unsigned char *) new char[20];
  SHA1((unsigned char *) data, strlen(data), hash);
  return (char *) hash;
}

bool validSHA1(char *data, unsigned char *expected_hash) {
  unsigned char *hash = (unsigned char *) new char[20];
  SHA1((unsigned char *) data, strlen(data), hash);
  for (int i = 0; i < 20; i++) {
    printf("%02x %02x\n", hash[i], expected_hash[i]);
//    if (*(hash + i) != *(expected_hash + i)) {
//      free(hash);
//      return false;
//    }
  }
  free(hash);
  return true;
}

/*int main(int argc, char *argv[]) {
  char *torrent_file_name = argv[1];
  char *file_name = argv[2];
  MetaInfo *metaInfo = extractMetaInfo(torrent_file_name);
  const char *expected_hash_full = metaInfo->pieces;
  
  char *ehash = new char[20];
  memcpy(ehash, expected_hash_full, 20);
  
  int plength = (int) metaInfo->piece_length;
  ifstream ifs(file_name);
  char *buffer = new char[plength];
  ifs.read(buffer, plength);
  cout << plength << '\n';
  if (validSHA1(buffer, (unsigned char *) ehash)) {
    cout << "Match YAY!!\n";
  } else {
    cout << "No match sob\n";
  }
  ifs.close();
  free(buffer);
  return 0;
}*/
