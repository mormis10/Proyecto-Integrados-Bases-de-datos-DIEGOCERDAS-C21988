#include "Fork_Server.h"

void signal_handler(int signal){
    std::cout << "\n[SHUTDOWN FORK SERVER] Ctrl+C recibido. Señal" <<signal;
    std::cout<<" Terminando...\n";
    runningF = false;
}

int main() {
    signal(SIGINT, signal_handler);
    Fork_server fork;
    return 0;
}