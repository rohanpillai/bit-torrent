#include <FileManager.h>

using namespace std;

FileManager::FileManager(char *file_name, int len, int pl, int num_pieces, bool complete) {
  length = len;
  number_pieces = num_pieces;
  piece_length = pl;
  fs.open(file_name, fstream::in | fstream::out | fstream::binary);
  if (fs.fail()) {
    printf("Failed to open file\n");
    fs = NULL;
    return;
  }

  srand(time(NULL));
  buffer = new char[length];
  have = new bool[number_pieces];
  if (complete) {
    fs.read(buffer, length);
    for (int i=0; i<number_pieces; i++) {
      have[i] = true;
    }
  } else {
    for (int i=0; i<number_pieces; i++) {
      have[i] = false;
      needed.insert(i);
    }
  }
}

char *FileManager::getPiece(int index) {
  return buffer + index*piece_length;
}

void FileManager::writePiece(int index, char *data) {
  if (index > number_pieces - 1) {
    printf("Index of piece exceeded number of pieces\n");
    delete [] data;
    return;
  }
  if (have[index] == true) {
    delete [] data;
    return;
  }
  
  memcpy(buffer + index*piece_length, data, piece_length);
  have[index] = true;
  needed.erase(index);
  delete [] data;
}

int FileManager::pickPieceToGet() {
  return rand() % needed.size();
}

bool *FileManager::getHave() {
  return have;
}
