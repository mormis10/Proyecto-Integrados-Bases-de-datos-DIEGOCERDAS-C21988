#include <iostream>
#include <string>
#include <sstream>
#include "File_system.h"

void insertarFigura(File_system& fs) {
    std::string name;
    std::cout << "[INPUT] Nombre de la figura: ";
    std::getline(std::cin, name);

    std::cout << "[INPUT] Ingresa el arte ASCII (termina con una línea vacía):" << std::endl;

    std::ostringstream ascii_art;
    std::string line;
    while (true) {
        std::getline(std::cin, line);
        if (line.empty()) break;
        ascii_art << line << "\n";
    }

    std::string content = ascii_art.str();
    char* text = new char[content.size() + 1];
    std::strcpy(text, content.c_str());

    int result = fs.make_file(name.c_str(), text);
    if (result == 0) {
        std::cout << "[OK] Figura guardada con éxito.\n";
    } else {
        std::cerr << "[ERROR] No se pudo guardar la figura.\n";
    }

    delete[] text;
}

void eliminarFigura(File_system& fs) {
    std::string name;
    std::cout << "[INPUT] Nombre de la figura a eliminar: ";
    std::getline(std::cin, name);

    int result = fs.delete_file(name.c_str());
    if (result == 0) {
        std::cout << "[OK] Figura eliminada correctamente.\n";
    } else {
        std::cerr << "[ERROR] No se pudo eliminar la figura (puede que no exista).\n";
    }
}

void listarFiguras(File_system& fs) {
    char* list = fs.list_file();
    if (!list || std::string(list).empty()) {
        std::cout << "[INFO] No hay figuras registradas.\n";
    } else {
        std::cout << "[FIGURAS] " << list << "\n";
    }
    delete[] list;
}

int main() {
    File_system fs;
    while (true) {
        std::cout << "\n===== MENÚ FIGURAS =====\n";
        std::cout << "1. Insertar nueva figura\n";
        std::cout << "2. Eliminar figura\n";
        std::cout << "3. Listar figuras\n";
        std::cout << "0. Salir\n";
        std::cout << "> ";

        std::string opcion;
        std::getline(std::cin, opcion);

        if (opcion == "1") {
            insertarFigura(fs);
        } else if (opcion == "2") {
            eliminarFigura(fs);
        } else if (opcion == "3") {
            listarFiguras(fs);
        } else if (opcion == "0") {
            break;
        } else {
            std::cout << "[ERROR] Opción no válida.\n";
        }
    }

    return 0;
}