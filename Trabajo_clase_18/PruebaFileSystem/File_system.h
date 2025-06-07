#pragma once
#include <fstream>
#include <vector>

class SuperBlock;

class File_system {
    std::fstream disk;
    SuperBlock* block;

public:
  File_system();
  ~File_system();

  void ExtendDisk(int additionalBlocks);
  int make_file(const char* name, char* text);
  char* read_file(const char* name);
  int delete_file(const char* name);
  int list_file();

};

const int MAX_FILENAME_LENGTH = 32;
#include <cstring>

struct Inode {
  char filename[MAX_FILENAME_LENGTH];
  int size = 0;
  int origBlock;
  int startBlock = 0;
  int countBlock = 0;
  int nextInode = 0;

  Inode() {
    std::memset(filename, 0, MAX_FILENAME_LENGTH);
  }

  Inode(const char* filename, int origBlock) {
    std::memset(this->filename, 0, MAX_FILENAME_LENGTH);
    std::strncpy(this->filename, filename, MAX_FILENAME_LENGTH - 1);
    this->filename[MAX_FILENAME_LENGTH - 1] = '\0';
    this -> origBlock = origBlock;
  }
};


#define rewindBlocks 64
#define sizeBlocks 256

class SuperBlock {


  public:
    int size;
    int initBlocks;
    int totalBlocks;
    int freeBlocks;
    int usedBlocks;
    int inodeCount;
    std::vector<Inode> inodeTable;
    File_system* syst;
    
    SuperBlock();
    SuperBlock(std::fstream& disk, File_system* syst, int size);
    int ad_file(std::fstream& disk, const char* name, char* text);
    int rm_file(std::fstream& disk, const char* name);
    char* rd_file(std::fstream& disk, const char* name);
    int save_Bs(std::fstream& disk);


  private:
    int adIno_file(std::fstream& disk, const char* name);
    int wr_file(std::fstream& disk, const char* name, char* text);
    int empty_B(std::fstream& disk);
    int find_inode_by_name(const char* name);
    int clear_block(std::fstream& disk, int block_index);
};