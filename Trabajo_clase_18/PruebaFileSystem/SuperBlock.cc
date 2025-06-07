#include <cstring>
#include <iostream>
#include "File_system.h"
#include <fstream> 
#include <vector>

SuperBlock::SuperBlock() {

}

SuperBlock::SuperBlock(std::fstream& disk, File_system* syst, int size): syst(syst) {
  this -> size = size;

  disk.seekg(std::fstream::beg);

  disk.read(reinterpret_cast<char*>(&(this->initBlocks)), sizeof(int));

  if (this->initBlocks != 0){
    disk.read(reinterpret_cast<char*>(&(this->totalBlocks)), sizeof(int));
    disk.read(reinterpret_cast<char*>(&(this->freeBlocks)), sizeof(int));
    disk.read(reinterpret_cast<char*>(&(this->usedBlocks)), sizeof(int));
    disk.read(reinterpret_cast<char*>(&(this->inodeCount)), sizeof(int));
  } else {
    this -> initBlocks = sizeof(SuperBlock)/sizeBlocks + 1;
    this -> totalBlocks = this->size/sizeBlocks;
    this -> freeBlocks = this->totalBlocks - this -> initBlocks;
    this -> usedBlocks = 0;
    this -> inodeCount = 0;
  }

  if (inodeCount > 0) {
    int current = 0;
    disk.read(reinterpret_cast<char*>(&(current)), sizeof(int));

    while (current != 0) {
      disk.seekg(current*sizeBlocks +  sizeof(bool), std::fstream::beg);
      Inode t;
      disk.read(reinterpret_cast<char*>(&t), sizeof(Inode));
      this->inodeTable.push_back(t);
      current = t.nextInode;
    }
  }
}

int SuperBlock::ad_file(std::fstream& disk, const char* name, char* text) {
  int result = adIno_file(disk, name);
  if (result != 0) {
    std::cout << "Error al agregar inode para el archivo: " << name << std::endl;
    return result;
  }

  result = wr_file(disk, name, text);
  if (result != 0) {
    std::cout << "Error al escribir el archivo: " << name << std::endl;
    return result;
  }

  return 0;
}

int SuperBlock::adIno_file(std::fstream& disk, const char* name) {
  if (find_inode_by_name(name) != -1) {
    std::cout << "Prueba otro nombre, ese ya esta tomado" << std::endl;
    return -1;
  }

  int inodeBlock = this -> empty_B(disk);
  if (inodeBlock < 0) {
    std::cout << "No hay espacio para archivos, el inodo" << std::endl;
    return -1;
  }

  Inode t = Inode(name, inodeBlock);

  if(inodeTable.empty()) {
    disk.seekp(inodeBlock*sizeBlocks + sizeof(bool), std::fstream::beg);
    disk.write(reinterpret_cast<const char*>(&t), sizeof(Inode)); 
    this -> inodeTable.push_back(t);
  } else {
    disk.seekp(inodeBlock*sizeBlocks + sizeof(bool), std::fstream::beg);
    disk.write(reinterpret_cast<const char*>(&t), sizeof(Inode)); 
    inodeTable[inodeTable.size() - 1].nextInode = inodeBlock;
    this -> inodeTable.push_back(t);
  }

  this -> inodeCount ++;

  return 0;
}

int SuperBlock::wr_file(std::fstream& disk, const char* name, char* text) {
  int tableIndex = find_inode_by_name(name);
  if (tableIndex == -1) {
    std::cerr << "Inodo correspondiente no encontrado.\n";
    return -1;
  }

  int sizeText = strlen(text);
  char* ptr = text;
  int remaining = sizeText;
  int blockSizeUsable = sizeBlocks - sizeof(bool) - sizeof(int);

  int currentBlock = empty_B(disk);
  if (currentBlock < 0) {
    std::cerr << "No hay bloques disponibles.\n";
    return -1;
  }

  
  this->inodeTable[tableIndex].startBlock = currentBlock;
  this->inodeTable[tableIndex].size = sizeText;
  this->inodeTable[tableIndex].countBlock = 0;

  while (remaining > 0) {
    int blockStart = currentBlock * sizeBlocks;

    
    disk.seekp(blockStart + sizeof(bool), std::ios::beg);
    int toWrite = (remaining <= blockSizeUsable) ? remaining : blockSizeUsable;
    disk.write(ptr, toWrite);
    ptr += toWrite;
    remaining -= toWrite;

    
    int nextBlock = (remaining > 0) ? empty_B(disk) : 0;
    if (remaining > 0 && nextBlock < 0) {
      std::cerr << "No hay bloques disponibles para continuar escritura.\n";
      return -1;
    }

    
    disk.seekp(blockStart + sizeBlocks - sizeof(int), std::ios::beg);
    disk.write(reinterpret_cast<const char*>(&nextBlock), sizeof(int));

    
    currentBlock = nextBlock;
    this->inodeTable[tableIndex].countBlock++;
  }

  return 0;
}

int SuperBlock::rm_file(std::fstream& disk, const char* name) {
  int tableIndex = find_inode_by_name(name);

  if (tableIndex == -1) {
    std::cout << "inode correspondiente no encontrado" << std::endl;
    return -1;
  }

  
  int currentBlock = this -> inodeTable[tableIndex].startBlock;
  int blockToRM = this -> inodeTable[tableIndex].countBlock;

  for (int i = 0; i < blockToRM; i++) {
    currentBlock = this -> clear_block(disk, currentBlock);
    this -> usedBlocks --;
    this -> freeBlocks ++;
  }

  int origBlock = this -> inodeTable[tableIndex].origBlock;

  if (tableIndex != 0) {
    if (tableIndex == (int)this->inodeTable.size() - 1) {
      this->inodeTable[tableIndex - 1].nextInode = 0;
    } else {
      this->inodeTable[tableIndex - 1].nextInode =
      this->inodeTable[tableIndex + 1].origBlock;
    }
  }

  this -> clear_block(disk, origBlock);
  this -> usedBlocks --;
  this -> freeBlocks ++;
  this -> inodeTable.erase(inodeTable.begin() + tableIndex);

  return 0;
}

char* SuperBlock::rd_file(std::fstream& disk, const char* name) {
  int tableIndex = find_inode_by_name(name);
  if (tableIndex == -1) {
    std::cerr << "Inodo correspondiente no encontrado.\n";
    return nullptr;
  }

  int blockSizeUsable = sizeBlocks - sizeof(bool) - sizeof(int);
  int totalSize = this->inodeTable[tableIndex].size;
  int currentBlock = this->inodeTable[tableIndex].startBlock;

  if (currentBlock < 0 || totalSize <= 0) {
    std::cerr << "Inodo no tiene bloques asignados o archivo vacío.\n";
    return nullptr;
  }

  char* buffer = new char[totalSize + 1];  
  char* ptr = buffer;
  int remaining = totalSize;

  while (true) {
    int blockStart = currentBlock * sizeBlocks;

    std::cout << "Leyendo bloque: " << currentBlock << ", restantes: " << remaining << std::endl;

    disk.seekg(blockStart + sizeof(bool), std::ios::beg);

    int toRead = (remaining < blockSizeUsable) ? remaining : blockSizeUsable;
    disk.read(ptr, toRead);

    if (disk.gcount() != toRead) {
      std::cerr << "Error leyendo bloque en disco.\n";
      delete[] buffer;
      return nullptr;
    }

    ptr += toRead;
    remaining -= toRead;

    
    int nextBlock;
    disk.seekg(blockStart + sizeBlocks - sizeof(int), std::ios::beg);
    disk.read(reinterpret_cast<char*>(&nextBlock), sizeof(int));

    std::cout << "Siguiente bloque: " << nextBlock << std::endl;

    if (remaining == 0) break;

    if (nextBlock <= 0) {
      std::cerr << "Archivo incompleto o dañado: puntero inválido a siguiente bloque.\n";
      delete[] buffer;
      return nullptr;
    }

    currentBlock = nextBlock;
  }

  buffer[totalSize] = '\0';  
  return buffer;
}

int SuperBlock::empty_B(std::fstream& disk) {
  while(true) {
    for (int i = this->initBlocks; i < this->totalBlocks; i++) {
      std::streampos pos = i * sizeBlocks;

      disk.seekg(pos, std::ios::beg);
      bool use;
      disk.read(reinterpret_cast<char*>(&use), sizeof(bool));

      if (!use) {
        disk.seekp(pos, std::ios::beg); 
        use = true;
        disk.write(reinterpret_cast<const char*>(&use), sizeof(bool));

        this->usedBlocks++;
        this->freeBlocks--;
        return i;
      }
    }

    std::cerr << "¡Sin bloques! Intentando extender disco...\n";
    this->syst->ExtendDisk(rewindBlocks);
  }
}

int SuperBlock::save_Bs(std::fstream& disk) {
  disk.seekp(0, std::fstream::beg);
  
  disk.write(reinterpret_cast<const char*>(&(this->initBlocks)), sizeof(int));
  disk.write(reinterpret_cast<const char*>(&(this->totalBlocks)), sizeof(int));
  disk.write(reinterpret_cast<const char*>(&(this->freeBlocks)), sizeof(int));
  disk.write(reinterpret_cast<const char*>(&(this->usedBlocks)), sizeof(int));
  disk.write(reinterpret_cast<const char*>(&(this->inodeCount)), sizeof(int));
  if (!inodeTable.empty()) {
    disk.write(reinterpret_cast<const char*>(&(this->inodeTable[0].origBlock)), sizeof(int));
  } else {
    int zero = 0;
    disk.write(reinterpret_cast<const char*>(&zero), sizeof(int));
  }

  for (const Inode& node : inodeTable) {
    int block = node.origBlock;
    disk.seekp(block * sizeBlocks + sizeof(bool), std::fstream::beg);
    disk.write(reinterpret_cast<const char*>(&node), sizeof(Inode));
  }

  return 0;
}

int SuperBlock::find_inode_by_name(const char* name) {
  for (int i = 0; i < static_cast<int>(this->inodeTable.size()); ++i) {
    if (std::strcmp(inodeTable[i].filename, name) == 0) {
      return i;
    }
  }
  return -1;
}

int SuperBlock::clear_block(std::fstream& disk, int block_index) {
  int next = 0;
  disk.seekg(block_index * sizeBlocks + sizeBlocks - sizeof(int), std::fstream::beg);
  disk.read(reinterpret_cast<char*>(&next), sizeof(int)); 

  char* zero_block = new char[sizeBlocks]();
  disk.seekp(block_index * sizeBlocks, std::fstream::beg);
  disk.write(zero_block, sizeBlocks);
  delete[] zero_block;

  return next;
}