#include <fstream>
#include <iostream>
#include <cstring>

#include "File_System.hpp"




FileSystem::FileSystem(const std::string file_name): disk_path(file_name){this->load_fat();}

void FileSystem::load_fat(){
    // le vamos a poner disk a nuestra instancia, 
    // con ayuda de esta herramienta vamos a poder crear un archivos binario
    // lo abrimos en modo de lectura binario

    std::ifstream disk(this->disk_path, std::ios::binary);

    if(!disk.is_open()){
        printf("Ocurrió un error en la apertura del disco nachito\n");
        return;
    }
    // vamos a leer los primeros 4096 bytes y vamos a cargar esa información dentro de nuestro arreglo fat
    disk.read(reinterpret_cast<char*>(fat),sizeof(fat));
}

int FileSystem::allocate_block(){
    // NOS va a dar un índice del primer bloque libre 
    for(int i = 0; i< TOTAL_BLOCKS; i++){
        if(this->fat[i] == -1){
        // ya todo lo que reservé se acabó
        // en este bloque se termina el archivo
        // Ya previamente le había asignado el 
        fat[i] = 0;
        return i;
        }
    }
    return -1;
}

void FileSystem::create_file(const std::string file_name, const std::string content){
    // primer bloque que se encuentre libre 
    int block = this->allocate_block();
    if(block == -1){
        printf("Ocurrió un error nachito\n");
        printf("Ahora no hay bloques disponibles\n");
        return;
    }
   // Abrimos nuestro archivo en modo de lectura y escritura binaria
    std::fstream disk(disk_path, std::ios::in | std::ios::out | std::ios::binary);

    // movemos el puntero de escritura al bloque que se encuentre libre 
    // El bloque libre que hemos 
    disk.seekp(sizeof(this->fat) + sizeof(this->directory) + block * BLOCK_SIZE);
    size_t length = sizeof(this->fat) + sizeof(this->directory) + block * BLOCK_SIZE;
    size_t longitud = content.size(); 
    disk.flush();
    disk.write(content.c_str(),content.size());

    char buffer[MAX_FILES];
    disk.seekg(sizeof(this->fat) + sizeof(this->directory) + block * BLOCK_SIZE);
    size_t prueba = sizeof(this->fat) + sizeof(this->directory) + block * BLOCK_SIZE;
    printf("%ld\n",prueba);
    disk.read(buffer,MAX_FILES);
    printf("Verificación de que se guardo el contenido\n");
    printf("%s\n",buffer);
    //this->safe_fat();
    this->save_file(file_name,block);

    printf("Archivo Creado con éxito\n");
    printf("Creado en el bloque %i",block);
}


void FileSystem::safe_fat(){
// Aquí actualizamos la cantidad de bloques disponibles después de haber añadido el archivo 
// escribimos en el archivo los datos con la fat actualizada
// Volemos a crear una instancia disk con permisos de lectura y escritura
 std::fstream disk(disk_path, std::ios::in | std::ios::out | std::ios::binary);
 // Actualizamos los bloques ocupados de la fat en el archivo
 disk.write(reinterpret_cast<char*>(fat), sizeof(fat));

}

void FileSystem::load_directory(){
    // Volvemos a crear una instacia de archivo binario en lectura
    std::ifstream disk(this->disk_path,std::ios::binary);
    disk.seekg(sizeof(this->fat));
    disk.read(reinterpret_cast<char*>(directory), sizeof(directory));
}

void FileSystem::save_file(const std::string file_name,int block){
    for (int i = 0; i < MAX_FILES; i++) {
        if (!directory[i].used) {
            // copiamos el filename en el nombre del directorio
            strncpy(directory[i].name, file_name.c_str(), sizeof(directory[i].name) - 1);
            // Nos aseguramos de guardar el caracter nulo, ojito con eso
            directory[i].name[sizeof(directory[i].name) - 1] = '\0'; 
            directory[i].start_block = block;
            directory[i].used = true;
            break;
        }
    }
    std::fstream disk(disk_path, std::ios::in | std::ios::out | std::ios::binary);
    disk.seekp(sizeof(fat));
    disk.write(reinterpret_cast<char*>(directory), sizeof(directory));
}

void FileSystem::read_file(const std::string file_name){
    std::cout<<file_name <<"Ojo piojo nacho\n";
    // EL bloque donde se va a encoontrar el archivo que vamos a leer
    this->load_directory();
    //llenando el fat con los datos de los otros archivos
    int block = -1;
    // el buffer que va a almacenar el contenido del archivo
    //Aquí asumimos que el archivo ya está cargado
    for(int i = 0; i<MAX_FILES; i++){
        printf("%s\n",this->directory[i].name);
        if(std::string(this->directory[i].name) == file_name){
            block = this->directory[i].start_block;
            printf("Entra aquí\n");
            break;
        }
    }

    if(block == -1){
        printf("No se pudo encontrar el archivo\n");
        return;
    }
    // El buffer que va a contener los datos del archivo
    char buffer[BLOCK_SIZE];
    std::ifstream disk(this->disk_path, std::ios::binary);
    // Vamos a leer desde la posición del fat 
    // Añadiendo el indice del bloque donde inicia
    disk.seekg(sizeof(fat) + sizeof(this->directory) + block * BLOCK_SIZE);
    size_t prueba = sizeof(fat) + sizeof(this->directory) + block * BLOCK_SIZE;
    printf("%ld\n",prueba);
    disk.read(buffer,MAX_FILES);
    printf("Contenido del archivo '%s':\n", file_name.c_str());
    printf("%s\n",buffer);
    //SIUUUUUUUUU LO LOGRÉE
}

void FileSystem::format(){
    // Aquí formateamos nuestro archivo y lo llenamos para que esté list
    // trunc vacía completamente el archivo en caso de que ya existiera
    std::ofstream disk(this->disk_path, std::ios::binary | std::ios::trunc);
    char zero = 0;
    // Vamos a llenar el archivo de ceros para establecer que está completamente vacío
    for(int i = 0; i< BLOCK_SIZE * TOTAL_BLOCKS; i++){
        // cada vez escribimos un solo byte de valor zero
        disk.write(&zero,1);
    }
    //limpiamos nuestro fat asegurandonos de que no haya basura

    std::memset(this->fat,-1,sizeof(this->fat));
    this->safe_fat();
    printf("El archivo se formateo con éxito nacho\n");
}


void FileSystem::delete_file(const std::string file_name){
    
}

