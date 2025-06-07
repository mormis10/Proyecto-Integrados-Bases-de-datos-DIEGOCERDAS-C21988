#include "syscall.h"

int main(){
    int sock_id = Socket(AF_INET_NachOS, SOCK_STREAM_NachOS);
    //const char * whale = (char *) "GET /aArt/index.php?disk=Disk-01&fig=whale-1.txt\r\nHTTP/v1.1\r\nhost: redes.ecci\r\n\r\n";
    char a[512];
    const char * ip = "127.0.0.1";
    const char *solicitud = "GET /imagen_ascii HTTP/1.1\r\nHost: 163.178.104.62\r\n\r\n";
    Connect(sock_id,ip,1234);
    Write(solicitud, 84, sock_id);
    Read(a,512,sock_id);
    //Ahora lo escribo en consola
    Write(a,512,1);
    Close(sock_id);
}