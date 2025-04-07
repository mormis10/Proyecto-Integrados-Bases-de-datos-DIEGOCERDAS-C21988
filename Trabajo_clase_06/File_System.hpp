#ifndef FILESYSTEM_H
#define FILESYSTEM_H
#include <string>

#define BLOCK_SIZE 256 // para este caso bloques de 256
#define TOTAL_BLOCKS 4096
#define MAX_FILES 128

#include "Resources.hpp"

class FileSystem{
public:
// Las hacemos const para declarar que estas referencias no van a cambar, 
FileSystem(const std::string disk_name);
void format();
void create_file(const std::string file_name, const std::string content);
void read_file(const std::string file_name);
void delete_file(const std::string file_name);
void save_file(const std::string file_name, int block);
private:
std::string disk_path;
int block_size = BLOCK_SIZE;
int total_blocks = TOTAL_BLOCKS;
int fat_entries = TOTAL_BLOCKS;
void load_fat();
void safe_fat();
int allocate_block();
void free_blocks(int start_block);
void load_directory();
// Va a representar la file allocation table 
// vamos a poder saber que bloques están ocupados
// y cómo están encadenados estos bloques
int fat[TOTAL_BLOCKS];
// Esto es como un directorio simulado
FileEntry directory[MAX_FILES];
};

#endif

