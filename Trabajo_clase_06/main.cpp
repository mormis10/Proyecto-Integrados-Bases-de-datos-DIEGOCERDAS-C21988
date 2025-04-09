#include "File_System.hpp"


void simulation(FileSystem* disk){
    std::string figura1 = R"(
     /\\_/\\  
    ( o   o ) 
    (   "   ) 
     \\~(*)~/
      ~    ~ 
    )";


    disk->create_file("gatito.txt",figura1);
    disk->read_file("gatito.txt");

    printf("Primera simulación realizada con éxito\n");

     std::string figura2 = 
          "=^.^=\n"
        "( o.o )\n"
        " > ^ <\n";

      disk->create_file("gatito_kawai",figura2);
      disk->read_file("gatito_kawai");


    printf("Simulaciones realizadas con éxito\n");

}

int main(){
  FileSystem prueba("disk.mfs");
  prueba.format();
  simulation(&prueba);

  return 0;
}