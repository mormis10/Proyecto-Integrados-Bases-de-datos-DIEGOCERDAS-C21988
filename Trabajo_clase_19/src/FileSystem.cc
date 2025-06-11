
#include "FileSystem.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <cstring>

namespace fs = std::filesystem;


FileSystem::FileSystem(const std::string& diskFilename)
  : diskFilename(diskFilename) {}

void FileSystem::writeBlock(int blockNum, const char* data) {
  std::fstream disk(diskFilename, std::ios::in | std::ios::out | std::ios::binary);
  disk.seekp(blockNum * BLOCK_SIZE);
  disk.write(data, BLOCK_SIZE);
  disk.close();
}

void FileSystem::readBlock(int blockNum, char* buffer) {
  std::ifstream disk(diskFilename, std::ios::binary);
  disk.seekg(blockNum * BLOCK_SIZE);
  disk.read(buffer, BLOCK_SIZE);
  disk.close();
}

int FileSystem::getBlockCount() {
  std::ifstream disk(diskFilename, std::ios::binary | std::ios::ate);
  return static_cast<int>(disk.tellg()) / BLOCK_SIZE;
}

int FileSystem::allocateBlock() {
  std::ofstream disk(diskFilename, std::ios::in | std::ios::out | std::ios::binary);
  disk.seekp(nextFreeBlock * BLOCK_SIZE);
  char emptyBlock[BLOCK_SIZE] = {};  // Initialize with empty data
  disk.write(emptyBlock, BLOCK_SIZE);
  disk.close();
  return nextFreeBlock++;
}

void FileSystem::initDisk(int numDirBlocks) {
  std::ofstream disk(diskFilename, std::ios::binary);
  DirectoryBlock empty = {};  // Create an empty directory block
  for (int i = 0; i < numDirBlocks; ++i) {
    empty.nextBlock = (i == numDirBlocks - 1) ? 0 : i + 1;  // Set next block
    disk.write(reinterpret_cast<char*>(&empty), sizeof(empty));
  }
  disk.close();
  nextFreeBlock = numDirBlocks;  // Set the next available block
}

bool FileSystem::figureExists(const std::string& name) {
  DirectoryBlock dir;
  int blockNum = 0;
  while (true) {
    readBlock(blockNum, reinterpret_cast<char*>(&dir));
    for (int i = 0; i < DIR_ENTRIES; ++i) {
      if (std::strncmp(dir.entries[i].name, name.c_str(), NAME_SIZE) == 0) {
        return true;  // Figure found
      }
    }
    if (dir.nextBlock == 0) break;  // No more directory blocks
    blockNum = dir.nextBlock;  // Move to the next block
  }
  return false;  // Figure not found
}

int FileSystem::appendToDirectory(const std::string& name, int dataStartBlock) {
  DirectoryBlock dir;
  int blockNum = 0;

  while (true) {
    readBlock(blockNum, reinterpret_cast<char*>(&dir));

    // Find an empty entry in the directory block
    for (int i = 0; i < DIR_ENTRIES; ++i) {
      if (dir.entries[i].name[0] == '\0') {  // Empty entry
        std::strncpy(dir.entries[i].name, name.c_str(), NAME_SIZE);
        dir.entries[i].dataBlock = dataStartBlock;
        writeBlock(blockNum, reinterpret_cast<char*>(&dir));
        return blockNum;  // Return the current block number
      }
    }

    // If directory block is full, allocate a new one
    if (dir.nextBlock == 0) {
      int newBlock = allocateBlock();
      dir.nextBlock = newBlock;
      writeBlock(blockNum, reinterpret_cast<char*>(&dir));
      DirectoryBlock newDir = {};  // Create a new directory block
      std::strncpy(newDir.entries[0].name, name.c_str(), NAME_SIZE);
      newDir.entries[0].dataBlock = dataStartBlock;
      writeBlock(newBlock, reinterpret_cast<char*>(&newDir));
      return newBlock;  // Return the new block number
    }

    blockNum = dir.nextBlock;  // Move to the next block
  }
}

std::vector<int> FileSystem::storeFigureData(const std::string& content) {
  std::vector<int> blocks;
  size_t pos = 0;
  while (pos < content.size()) {
    int blockIndex = allocateBlock();
    blocks.push_back(blockIndex);
    char buffer[BLOCK_SIZE] = {};
    size_t chunkSize = std::min<size_t>(BLOCK_SIZE, content.size() - pos);
    std::memcpy(buffer, content.data() + pos, chunkSize);
    writeBlock(blockIndex, buffer);
    pos += chunkSize;
  }
  return blocks;
}

void FileSystem::addFigure(const std::string& name, const std::string& content) {
  if (figureExists(name)) {
    std::cout << "Figure '" << name << "' already exists. Skipping.\n";
    return;  // Skip if the figure already exists
  }
  auto blocks = storeFigureData(content);  // Store the figure's data in blocks
  appendToDirectory(name, blocks.front());  // Add entry to the directory
  std::cout << "Figure '" << name << "' added to disk.\n";
}

int FileSystem::countFigures(const std::string& path) {
  int count = 0;
  for (const auto& entry : std::filesystem::directory_iterator(path)) {
    if (entry.path().extension() == ".txt") {
      ++count;  // Increment count for each .txt file
    }
  }
  return count;
}

void FileSystem::loadFiguresFromFolder(const std::string& path) {
  for (const auto& entry : fs::directory_iterator(path)) {
    if (entry.path().extension() == ".txt") {  // Process .txt files
      std::ifstream file(entry.path());
      std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
      std::string filename = entry.path().stem().string();  // Get filename without extension
      addFigure(filename, content);  // Add the figure to the file system
    }
  }
}

std::string FileSystem::getFigureList() {
  DirectoryBlock dir;
  int blockNum = 0;
  std::ostringstream output;
  output << "Figures stored on disk:\n";

  while (true) {
    readBlock(blockNum, reinterpret_cast<char*>(&dir));
    output << "Directory block #" << blockNum << ":\n";

    for (int i = 0; i < DIR_ENTRIES; ++i) {
      if (dir.entries[i].name[0] != '\0') {
        std::string name(dir.entries[i].name);
        int dataBlock = dir.entries[i].dataBlock;
        int count = 0;
        char buffer[BLOCK_SIZE];

        // Count how many data blocks the figure uses
        while (true) {
          readBlock(dataBlock, buffer);
          ++count;
          if (std::strlen(buffer) < BLOCK_SIZE) break;  
          ++dataBlock;
        }

        output << "- " << name << " (" << count << " data blocks)\n";
      }
    }

    if (dir.nextBlock == 0) break;  
    output << "\n";
    blockNum = dir.nextBlock;  
  }
  output << "\n";  
  return output.str();  // Return the list of figures
}


std::string FileSystem::getFigureContent(const std::string& name) {
  DirectoryBlock dir;
  int blockNum = 0;
  std::string output;

  while (true) {
    readBlock(blockNum, reinterpret_cast<char*>(&dir));
    for (int i = 0; i < DIR_ENTRIES; ++i) {
      if (std::strncmp(dir.entries[i].name, name.c_str(), NAME_SIZE) == 0) {
        int dataBlock = dir.entries[i].dataBlock;
        char buffer[BLOCK_SIZE];
        while (true) {
          readBlock(dataBlock, buffer);
          output.append(buffer, strnlen(buffer, BLOCK_SIZE));  
          if (std::strlen(buffer) < BLOCK_SIZE) break;  
          ++dataBlock;
        }
        return output;  // Return the figure 
      }
    }
    if (dir.nextBlock == 0) break;  // No more directory blocks
    blockNum = dir.nextBlock;  // Move to next directory block
  }
  return "Figure not found.\n"; 
}

