#include <string>

typedef struct{
 // Nombre del archivo
 char name[32];
 //Bloque en el que inicia el archivo
 int start_block;
 // Tamaño del archivo 
 int size; 
 bool used;

}FileEntry;