#include <unistd.h>
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <cassert>
#include <cstring>
#include "File_system.h"

#define rewindBlocks 64
#define SIZE_DISK (256)

File_system::File_system() {
  try {
    this->disk.open(
      "disk.bin",
      std::fstream::in | std::fstream::out | std::fstream::binary
    );

    if (!this->disk.is_open()) {
      throw std::ios_base::failure("No se pudo abrir el archivo.");
    }

  } catch (std::ios_base::failure& err) {
    std::fstream tdisk("disk.bin", std::fstream::out | std::fstream::binary);
    tdisk.seekp(SIZE_DISK - 1);
    tdisk.write("\0", 1);
    tdisk.close();

    this->disk.open(
      "disk.bin",
      std::fstream::in | std::fstream::out | std::fstream::binary
    );

    if (!this->disk.is_open()) {
      throw std::runtime_error("No se pudo crear y abrir disk.bin.");
    }
  }

  this->block = new SuperBlock(this->disk, this, SIZE_DISK);
}

File_system::~File_system() {
  this->block->save_Bs(this->disk);
  delete this->block;
  this->disk.close();
}

int File_system::make_file(const char* name, char* text) {
  assert(strlen(name) < 32);
  return this->block->ad_file(this->disk, name, text);
}

char* File_system::read_file(const char* name) {
  assert(strlen(name) < 32);
  return this->block->rd_file(this->disk, name);
}

int File_system::delete_file(const char* name) {
  assert(strlen(name) < 32);
  return this->block->rm_file(this->disk, name);
}

void File_system::ExtendDisk(int additionalBlocks) {
  int oldTotal = block->totalBlocks;
  int newTotal = oldTotal + additionalBlocks;

  this->disk.seekp(newTotal * sizeBlocks - 1);
  char zero = 0;
  this->disk.write(&zero, 1);

  this->disk.flush();
  this->disk.close();

  this->disk.open("disk.bin", std::ios::in | std::ios::out | std::ios::binary);

  block->totalBlocks = newTotal;
  block->freeBlocks += additionalBlocks;

  std::cout << "Disco extendido: " << additionalBlocks << " bloques nuevos.\n";
}

char* File_system::list_file() {
  std::string result;

  for (size_t i = 0; i < this->block->inodeTable.size(); ++i) {
    const char* name = this->block->inodeTable[i].filename;

    if (name[0] != '\0') {
      if (!result.empty()) {
        result += ',';  // separador entre nombres
      }
      result += name;
    }
  }

  char* output = new char[result.size() + 1];
  std::strcpy(output, result.c_str());
  return output;
}