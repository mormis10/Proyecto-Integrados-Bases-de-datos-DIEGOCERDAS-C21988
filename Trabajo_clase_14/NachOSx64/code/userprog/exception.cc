// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "NachosOpenFileTable.h"
#include "copyright.h"
#include "syscall.h"
#include "system.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

extern NachosOpenFilesTable *openFilesTable = new NachosOpenFilesTable();
extern Lock *lock = new Lock("Lock for page table");
#define AF_INET_NachOS 2
#define SOCK_STREAM_NachOS 1
bool socket_ = false;

/*
 *  System call interface: Halt()
 */
void NachOS_Halt() { // System call 0

  DEBUG('a', "Shutdown, initiated by user program.\n");
  interrupt->Halt();
}
void returnFromSystemCall() {

  machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
  machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
  machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg) + 4);
}

/*
 *  System call interface: void Exit( int )
 */
void NachOS_Exit() {
  DEBUG('u', "Exit system call\n"); // Información para debug

  // faltan más cosas que debe completar Exit
  //
  currentThread->Finish(); // Current thread is finishing
}

/*
 *  System call interface: SpaceId Exec( char * )
 */
void NachOS_Exec() { // System call 2
}

/*
 *  System call interface: int Join( SpaceId )
 */
void NachOS_Join() { // System call 3
}

/*
 *  System call interface: void Create( char * )
 */
void NachOS_Create() { // System call 4
  lock->Acquire();
  int addr = machine->ReadRegister(4);
  char filename[256];
  int value;
  int i = 0;

  do {
    if (!machine->ReadMem(addr + i, 1, &value)) {
      printf("Error leyendo la memoria del usuario\n");
      return;
    }
    filename[i++] = (char)value;
  } while (i < 255 && value != '\0');

  // Garantizamos el final de la cadena
  filename[256] = '\0';

  int file = creat(filename, 0600);
  close(file);
  // printf("Se logró con éxito\n");
  lock->Release();
  returnFromSystemCall();
}

/*
 *  System call interface: OpenFileId Open( char * )
 */
void NachOS_Open() { // System call 5
  // Primero necesitamos tener en memoria de kernel, lo que se encuentra en
  // memoria de usuario nachito Recordando que la instancia Machine es global
  // nachito
  int useraddress = machine->ReadRegister(4);
  // hay que guardar la memoria del usuario del archivo en la memoria del
  // sistema
  char filename[256];
  // Nuestra variable para iterar mientras leemos la cadena
  int i = 0;
  // nuestro caracter pivote para guardar la estructura
  int c;
  do {
    // Primero empezamos leyendo en la posición de useraddress
    //  leemos solo un char en esa posición
    //  lo almacenamos en nuestro pivote nachito
    machine->ReadMem(useraddress + i, 1, &c);
    // lo metemos en nuestro buffer, guarda y después incrementa
    printf("%c ", (char)c);
    filename[i++] = (char)c;

  } while (i < 256 && c != '\0');

  filename[256] = '\0';

  // Ahora que tenemos el nombre del archivo hacemos un open con un llamado al
  // sitema anfitrión pedimos que sea para lectura y escritura
  int unixhandle = open(filename, O_RDWR);
  if (unixhandle == -1) {
    printf("Error: archivo %s no se pudo abrir.\n", filename);
    machine->WriteRegister(2, -1);
    returnFromSystemCall();
    return;
  }

  // Ahora tenemos que registrar el archivo en nuestra tabla de archivos
  // abiertos lo registramos como ocupado
  int nachosHandle = openFilesTable->Open(unixhandle);
  if (nachosHandle == -1) {
    close(unixhandle);
    machine->WriteRegister(2, -1);
    returnFromSystemCall();
    return;
  }
  // le devolmeos el handle al usuario
  machine->WriteRegister(2, nachosHandle);
  returnFromSystemCall();
}

/*
 *  System call interface: OpenFileId Write( char *, int, OpenFileId )
 */
void NachOS_Write() {
  // Ojito nacho, con estas flags y sus siggnificados
  int addr = machine->ReadRegister(4); // Dirección en la memoria del usuario
  int size = machine->ReadRegister(5); // Cantidad de bytes a escribir
  int file =
      machine->ReadRegister(6); // Descriptor del archivo (0:stdin, 1:stdout)
  // en este caso va a ser para la salida de la consola

  DEBUG('u', "Write system call: addr=%d, size=%d, file=%d\n", addr, size,
        file);

  char *buffer = new char[size + 1];
  int value;
  for (int i = 0; i < size; i++) {
    if (machine->ReadMem(addr + i, 1, &value)) {
      buffer[i] = (char)value;
    } else {
      printf("Error al leer de la memoria del usuario\n");
      delete[] buffer;
      returnFromSystemCall();
      return;
    }
  }

  buffer[size + 1] = '\0';
  if (!socket_) {
    if (file == 1) {
      delete[] buffer;
      returnFromSystemCall();
    } else if (file >= 2) {
      // Vamos a escribirlo
      int unixHandle = openFilesTable->getUnixHandle(file);
      if (unixHandle != -1) {
        int bytes_written = write(unixHandle, buffer, size);
        if (bytes_written == -1) {
          printf("Error escribiendo\n");
          returnFromSystemCall();
          return;
        }
        returnFromSystemCall();
      } else {
        printf("File descriptor no soportado\n");
        returnFromSystemCall();
      }
    }
  } else {
    int written_bytes = write(file, buffer, size);
    printf("Entrón en el modo socket");
    returnFromSystemCall();
  }
}

/*
 *  System call interface: OpenFileId Read( char *, int, OpenFileId )
 */
void NachOS_Read() { // System call 7
  lock->Acquire();
  int userAddress = machine->ReadRegister(4);
  int size = machine->ReadRegister(5);
  int file = machine->ReadRegister(6);
  // Necesitamos
  if (!socket_) {
    if (!openFilesTable->isOpened(file)) {
      // mandamos al registro de retorno un error
      machine->WriteRegister(2, -1);
      returnFromSystemCall();
      return;
    }
    int unixHandle = openFilesTable->getUnixHandle(file);
    char *buffer = new char[size + 1];
    int bytesread = read(unixHandle, buffer, size);
    if (bytesread < 0) {
      delete[] buffer;
      machine->WriteRegister(2, -1);
      returnFromSystemCall();
      return;
    }
    int value;
    for (int i = 0; i < bytesread; i++) {
      value = (int)buffer[i];
      machine->WriteMem(userAddress + i, 1, value);
    }

    delete[] buffer;
    machine->WriteRegister(2, bytesread);
    returnFromSystemCall();
  } else {
   char *buffer = new char[size + 1];
    int bytesread = read(file, buffer, size);
    buffer[bytesread] = '\0';
    printf("Contenido del buffer %s\n", buffer);
    delete[] buffer;
    returnFromSystemCall();
  }
  lock->Release();
}

/*
 *  System call interface: void Close( OpenFileId )
 */
void NachOS_Close() { // System call
  lock->Acquire();
  int nachosHandle = machine->ReadRegister(4);
  // Hacemos la comprobación de que esté abierto en esta filetable
  if (nachosHandle < 0) {
    // Tenemos que devolever el error al cerrar el archivo
    // Registro de retorno envíamos el error
    machine->WriteRegister(2, -1);
    returnFromSystemCall();
    lock->Release();
    return;
  }

  // Estos serían para no tocar el bitmap
  if (nachosHandle <= 2) {
    // Envíamos el éxito en el registro
    machine->WriteRegister(2, 0);
    returnFromSystemCall();
    return;
  }

  if (!openFilesTable->isOpened(nachosHandle)) {
    machine->WriteRegister(2, -1);
    returnFromSystemCall();
    return;
  }

  // Cerramos la tabla
  openFilesTable->Close(nachosHandle);
  machine->WriteRegister(2, 0);
  lock->Release();
  returnFromSystemCall();
}

/*
 *  System call interface: void Fork( void (*func)() )
 */
void NachOS_Fork() { // System call 9
}

/*
 *  System call interface: void Yield()
 */
void NachOS_Yield() { // System call 10
}

/*
 *  System call interface: Sem_t SemCreate( int )
 */
void NachOS_SemCreate() { // System call 11
}

/*
 *  System call interface: int SemDestroy( Sem_t )
 */
void NachOS_SemDestroy() { // System call 12
}

/*
 *  System call interface: int SemSignal( Sem_t )
 */
void NachOS_SemSignal() { // System call 13
}

/*
 *  System call interface: int SemWait( Sem_t )
 */
void NachOS_SemWait() { // System call 14
}

/*
 *  System call interface: Lock_t LockCreate( int )
 */
void NachOS_LockCreate() { // System call 15
}

/*
 *  System call interface: int LockDestroy( Lock_t )
 */
void NachOS_LockDestroy() { // System call 16
}

/*
 *  System call interface: int LockAcquire( Lock_t )
 */
void NachOS_LockAcquire() { // System call 17
}

/*
 *  System call interface: int LockRelease( Lock_t )
 */
void NachOS_LockRelease() { // System call 18
}

/*
 *  System call interface: Cond_t LockCreate( int )
 */
void NachOS_CondCreate() { // System call 19
}

/*
 *  System call interface: int CondDestroy( Cond_t )
 */
void NachOS_CondDestroy() { // System call 20
}

/*
 *  System call interface: int CondSignal( Cond_t )
 */
void NachOS_CondSignal() { // System call 21
}

/*
 *  System call interface: int CondWait( Cond_t )
 */
void NachOS_CondWait() { // System call 22
}

/*
 *  System call interface: int CondBroadcast( Cond_t )
 */
void NachOS_CondBroadcast() { // System call 23
}

/*
 *  System call interface: Socket_t Socket( int, int )
 */
void NachOS_Socket() { // System call 30
  lock->Acquire();
  // leemos el registro cuatro que está reservado para el primer argumento del
  // syscall
  int domain = machine->ReadRegister(4);
  // El tipo de socket
  int type = machine->ReadRegister(5);

  int sockid = socket(AF_INET, 1, 0);
  printf("id sokcet %i", sockid);
  machine->WriteRegister(2, sockid);
  lock->Release();
  printf("Socket creado con éxito\n");
  socket_ = true;
  returnFromSystemCall();
}

/*
 *  System call interface: Socket_t Connect( char *, int )
 */
void NachOS_Connect() { // System call 31
  lock->Acquire();
  int sockid = machine->ReadRegister(4);
  int ip_pointer = machine->ReadRegister(5);
  int port = machine->ReadRegister(6);
  char ip[32];
  int i = 0;
  // mejor guardo esto como un enteeo
  int value;

  do {
    if (machine->ReadMem(ip_pointer + i, 1, &value)) {
      ip[i++] = (int)value;
    } else {
      printf("Ocurrió un error obtniendo la ip designada\n");
      returnFromSystemCall();
      return;
    }
  } while (ip[i] != '\0' && i < 32);

  struct sockaddr_in server_addr;
  // Esto ya lo sabemos vamos a limpiar la basura
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  // Realizamos la conexión
  inet_pton(AF_INET, ip, &server_addr);

  int st =
      connect(sockid, (struct sockaddr *)&server_addr, sizeof(server_addr));

  machine->WriteRegister(2, st);
  printf("Connect realizado con éxito\n");
  lock->Release();
  returnFromSystemCall();
}

/*
 *  System call interface: int Bind( Socket_t, int )
 */
void NachOS_Bind() { // System call 32
}

/*
 *  System call interface: int Listen( Socket_t, int )
 */
void NachOS_Listen() { // System call 33
}

/*
 *  System call interface: int Accept( Socket_t )
 */
void NachOS_Accept() { // System call 34
}

/*
 *  System call interface: int Shutdown( Socket_t, int )
 */
void NachOS_Shutdown() { // System call 25
}

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions
//	are in machine.h.
//----------------------------------------------------------------------

void ExceptionHandler(ExceptionType which) {
  int type = machine->ReadRegister(2);

  switch (which) {

  case SyscallException:
    switch (type) {
    case SC_Halt: // System call # 0
      NachOS_Halt();
      break;
    case SC_Exit: // System call # 1
      NachOS_Exit();
      break;
    case SC_Exec: // System call # 2
      NachOS_Exec();
      break;
    case SC_Join: // System call # 3
      NachOS_Join();
      break;

    case SC_Create: // System call # 4
      NachOS_Create();
      break;
    case SC_Open: // System call # 5
      NachOS_Open();
      break;
    case SC_Read: // System call # 6
      NachOS_Read();
      break;
    case SC_Write: // System call # 7
      NachOS_Write();
      break;
    case SC_Close: // System call # 8
      NachOS_Close();
      break;

    case SC_Fork: // System call # 9
      NachOS_Fork();
      break;
    case SC_Yield: // System call # 10
      NachOS_Yield();
      break;

    case SC_SemCreate: // System call # 11
      NachOS_SemCreate();
      break;
    case SC_SemDestroy: // System call # 12
      NachOS_SemDestroy();
      break;
    case SC_SemSignal: // System call # 13
      NachOS_SemSignal();
      break;
    case SC_SemWait: // System call # 14
      NachOS_SemWait();
      break;

    case SC_LckCreate: // System call # 15
      NachOS_LockCreate();
      break;
    case SC_LckDestroy: // System call # 16
      NachOS_LockDestroy();
      break;
    case SC_LckAcquire: // System call # 17
      NachOS_LockAcquire();
      break;
    case SC_LckRelease: // System call # 18
      NachOS_LockRelease();
      break;

    case SC_CondCreate: // System call # 19
      NachOS_CondCreate();
      break;
    case SC_CondDestroy: // System call # 20
      NachOS_CondDestroy();
      break;
    case SC_CondSignal: // System call # 21
      NachOS_CondSignal();
      break;
    case SC_CondWait: // System call # 22
      NachOS_CondWait();
      break;
    case SC_CondBroadcast: // System call # 23
      NachOS_CondBroadcast();
      break;

    case SC_Socket: // System call # 30
      NachOS_Socket();
      break;
    case SC_Connect: // System call # 31
      NachOS_Connect();
      break;
    case SC_Bind: // System call # 32
      NachOS_Bind();
      break;
    case SC_Listen: // System call # 33
      NachOS_Listen();
      break;
    case SC_Accept: // System call # 32
      NachOS_Accept();
      break;
    case SC_Shutdown: // System call # 33
      NachOS_Shutdown();
      break;

    default:
      printf("Unexpected syscall exception %d\n", type);
      ASSERT(false);
      break;
    }
    break;

  case PageFaultException: {
    break;
  }

  case ReadOnlyException:
    printf("Read Only exception (%d)\n", which);
    ASSERT(false);
    break;

  case BusErrorException:
    printf("Bus error exception (%d)\n", which);
    ASSERT(false);
    break;

  case AddressErrorException:
    printf("Address error exception (%d)\n", which);
    ASSERT(false);
    break;

  case OverflowException:
    printf("Overflow exception (%d)\n", which);
    ASSERT(false);
    break;

  case IllegalInstrException:
    printf("Ilegal instruction exception (%d)\n", which);
    ASSERT(false);
    break;

  default:
    printf("Unexpected exception %d\n", which);
    ASSERT(false);
    break;
  }
}
