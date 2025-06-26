#pragma once

#include <string>
#include <vector>
#include <cstdint>

const int BLOCK_SIZE = 256;  // Size of each block in bytes
const int NAME_SIZE = 32;    // Maximum size of a file name
const int DIR_ENTRIES = 7;   // Number of entries in a directory block
const std::string DISK_FILENAME = "disk.bin";  // Default disk filename


struct DirEntry {
  char name[NAME_SIZE];  // File name
  uint32_t dataBlock;    // Data block where the figure's content is stored
};

struct DirectoryBlock {
  DirEntry entries[DIR_ENTRIES];  // Directory entries in this block
  uint32_t nextBlock;             // Next directory block (0 if none)
};

class FileSystem {
 public:
  
  FileSystem(const std::string& diskFilename = DISK_FILENAME);

  void initDisk(int numDirBlocks);

  bool figureExists(const std::string& name);

  void addFigure(const std::string& name, const std::string& content);

  void loadFiguresFromFolder(const std::string& path);

  std::string getFigureList();

  std::string getFigureContent(const std::string& name);

  int countFigures(const std::string& path);

  std::string getFigureNamesCSV();

  void readBlock(int blockNum, char* buffer);
  
 private:
  std::string diskFilename; 
  int nextFreeBlock = 0;

  

  void writeBlock(int blockNum, const char* data);

  int getBlockCount();

  int allocateBlock();

  int appendToDirectory(const std::string& name, int dataStartBlock);

  std::vector<int> storeFigureData(const std::string& content);
};