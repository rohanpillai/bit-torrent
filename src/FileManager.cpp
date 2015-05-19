#include <FileManager.h>

using namespace std;

WriteObject::WriteObject(int i, int l, char *d) {
  index = i;
  length = l;
  data = d;
}

FileManager::FileManager(char *file_name, int len, int pl, int num_pieces, bool complete) {
  length = len;
  number_pieces = num_pieces;
  piece_length = pl;
  fs = fopen(file_name, "w");
  if (fs == NULL) {
    printf("Failed to open file\n");
    good = false;
    return;
  }

  good = true;
  buffer = new char[length];
  have = new bool[number_pieces];
  if (complete) {
    int bytes_read = fread(buffer, sizeof(char), length, fs);
    printf("bytes read %d\n", bytes_read);
    for (int i=0; i<number_pieces; i++) {
      have[i] = true;
    }
    fclose(fs);
  } else {
    for (int i=0; i<number_pieces; i++) {
      have[i] = false;
      needed.insert(i);
    }
  }
}

bool FileManager::isGood() {
  return good;
}

char *FileManager::getPointerToData(int index, int begin) {
  return buffer + index*piece_length + begin;
}

void FileManager::addToWriteQueue(int index, int length, char *data) {
  WriteObject *obj = new WriteObject(index, length, data);
  write_queue->push_back(obj);
}

void FileManager::write() {
  while (write_queue->size() > 0) {
    WriteObject *obj = write_queue->front();
    int index = obj->index;
    int length = obj->length;
    char *data = obj->data;

    if (index > number_pieces - 1) {
      printf("Index of piece exceeded number of pieces\n");
      write_queue->pop_front();
      delete obj;
      continue;
    }
    if (have[index] == true) {
      write_queue->pop_front();
      delete obj;
      continue;
    }
  
    memcpy(buffer + index*piece_length, data, length);
    have[index] = true;
    needed.erase(index);
    write_queue->pop_front();
    delete [] data;
    delete obj;
  }
}

set<int> *FileManager::getNeeded() {
  return &needed;
}

bool *FileManager::getHave() {
  return have;
}

int FileManager::sizeofPiece(int index) {
  if (index < number_pieces - 1) {
    return piece_length;
  } else {
    return (length - index*piece_length);
  }
}

void FileManager::save() {
  fwrite(buffer, sizeof(char), length, fs);
  fclose(fs);
}
