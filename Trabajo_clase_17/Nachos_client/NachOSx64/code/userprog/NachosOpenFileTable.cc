#include "NachosOpenFileTable.h"
#include "copyright.h"
#include "system.h"

NachosOpenFilesTable::NachosOpenFilesTable(){
    this->openFiles = new int[MAX_FILES];
    this->openFilesMap = new BitMap(MAX_FILES);
    this->usage = -1;
    // Ojito con esto marcamos el 0 para stdin standar input, 1 para standar input y 2 para standar error
    // por lo tanto estos no tienen un archivo abierto asociado, ojito con eso
    this->openFilesMap->Mark(0);
    this->openFilesMap->Mark(1);
    this->openFilesMap->Mark(2);
    // Ahora los marcamos
    this->openFiles[0] = 0;
    this->openFiles[1] = 1;
    this->openFiles[2] = 2;
}

NachosOpenFilesTable::~NachosOpenFilesTable(){
    delete[] this->openFiles;
    delete openFilesMap;
}

int NachosOpenFilesTable::Open(int unixHandle){
    int pos = this->openFilesMap->Find();
    if(pos == -1){
        return -1; 
    }
    this->openFiles[pos] = unixHandle;
    return pos;
}

int NachosOpenFilesTable::Close(int nachos_handle){
    if(!this->openFilesMap->Test(nachos_handle)){
        return -1; // La instancia que queremos cerrar no está abierta
    }

    //deslockeamos del bitmap 
    this->openFilesMap->Clear(nachos_handle); // lo marcamos como menos 1
    this->openFiles[nachos_handle] = -1; // Lo dejamos libre

    return 0;
}

bool NachosOpenFilesTable::isOpened(int nachosHandle){
    return this->openFilesMap->Test(nachosHandle);
}

int NachosOpenFilesTable::getUnixHandle(int nachosHandle){
    if(!this->isOpened(nachosHandle)){
        return -1;
    }else{
        return this->openFiles[nachosHandle];
    }
}


void NachosOpenFilesTable::addThread(){
    this->usage++;
}

void NachosOpenFilesTable::delThread(){
    this->usage--;
}

void NachosOpenFilesTable::Print(){
    printf("Tabla de archivos abiertos Nachito\n");
    for(int i = 0; i< MAX_FILES; i++){
        if(this->openFilesMap->Test(i)){
            printf("  NachosHandle %d => UnixHandle %d\n", i, openFiles[i]);
        }
    }
  printf("Hilos usando esta tabla: %d\n", usage);
}