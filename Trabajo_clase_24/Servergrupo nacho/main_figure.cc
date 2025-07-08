#include "Figure_server.h"

void signal_handler(int signal){
    std::cout << "\n[SHUTDOWN FIGURE SERVER] Ctrl+C recibido. Señal " <<signal;
    std::cout<<" Terminando...\n";
    running = false;
}

int main() {
    signal(SIGINT, signal_handler);
    Figure_server figure("Isla6", "172.16.123.101");
    return 0;
}