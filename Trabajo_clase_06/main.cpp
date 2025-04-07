#include "File_System.hpp"

int main(){
    FileSystem prueba("disk.mfs");
    prueba.format();
    
    std::string figura1 = R"(
     /\\_/\\  
    ( o   o ) 
    (   "   ) 
     \\~(*)~/
      ~    ~ 
    )";

    std::string figura2 = R"(
     .-"      "-.  
    /            \\ 
   |,  .-.  .-.  ,| 
   | )(_o/  \\o_)( | 
   |/     /\\     \\| 
    (_     ^^     _) 
     \\__|IIIIII|__/  
      | \\IIIIII/ |    
      \\          /   
       `-.____.-'   
    )";

    prueba.create_file("gatito.txt",figura1);
    prueba.read_file("gatito.txt");
    return 0;
}