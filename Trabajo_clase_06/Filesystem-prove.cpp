#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOCK_SIZE 256 // para este caso bloques de 256

typedef struct{
char file_name[100];
size_t file_size;
int block_ammount;
int* blocks;
}File;


typedef struct{
FILE* disk;
int block_amount;
char* disk_name;
}Data_disk;


void store_file(File* Archivo, ){
 

}

void search_in_disk(Data_disk* disco, const char* name, int block_count){
    disco->block_amount = block_count;
    disco->disk_name = strdup(name);
    disco->disk = fopen(name, "w+b");
    if(disco->disk == nullptr){
        printf("Algo salió mal\n");
        return;
    }
    
    // Ojito ahora tenemos que inicializar los bloques cómo vaciós
    for(int i = 0; i<block_count; i++){
        char empty_block[BLOCK_SIZE] = {0};
        fwrite(empty_block,1,BLOCK_SIZE,disco->disk);
    }

}

int main(){
    File archivo;
    Data_disk disco;
    init_file(&archivo);
    printf("Lo encontrado en el almacenamiento:\n");
    return 0;
}