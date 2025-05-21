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

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "NachosOpenFileTable.h"
#include <fcntl.h>     
#include <unistd.h>    
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>



extern NachosOpenFilesTable *openFilesTable = new NachosOpenFilesTable();


/*
 *  System call interface: Halt()
 */
void NachOS_Halt() {		// System call 0
 
	DEBUG('a', "Shutdown, initiated by user program.\n");
   	interrupt->Halt();

}
void returnFromSystemCall() {

        machine->WriteRegister( PrevPCReg, machine->ReadRegister( PCReg ) );
        machine->WriteRegister( PCReg, machine->ReadRegister( NextPCReg ));		
        machine->WriteRegister( NextPCReg, machine->ReadRegister( NextPCReg ) + 4 );	

}     


/*
 *  System call interface: void Exit( int )
 */
void NachOS_Exit() {
    DEBUG('u', "Exit system call\n");	// Información para debug

   // faltan más cosas que debe completar Exit
   //
   currentThread->Finish();	// Current thread is finishing
}


/*
 *  System call interface: SpaceId Exec( char * )
 */
void NachOS_Exec() {		// System call 2
}


/*
 *  System call interface: int Join( SpaceId )
 */
void NachOS_Join() {		// System call 3
}


/*
 *  System call interface: void Create( char * )
 */
void NachOS_Create() {		// System call 4
}


/*
 *  System call interface: OpenFileId Open( char * )
 */
void NachOS_Open() {		// System call 5
  // Primero necesitamos tener en memoria de kernel, lo que se encuentra en memoria de usuario nachito
  // Recordando que la instancia Machine es global nachito
  int useraddress = machine->ReadRegister(4);
  // hay que guardar la memoria del usuario del archivo en la memoria del sistema
  char filename[256];
  // Nuestra variable para iterar mientras leemos la cadena
  int i = 0;
  // nuestro caracter pivote para guardar la estructura
  char c = '\0';
  // 
  do{
   //Primero empezamos leyendo en la posición de useraddress
   // leemos solo un char en esa posición
   // lo almacenamos en nuestro pivote nachito
   machine->ReadMem(useraddress +1,1,(int *)&c);
   // lo metemos en nuestro buffer, guarda y después incrementa 
   filename[i++] = c;

  }while(i < 256 && c != '\0');

  filename[256] = '\0';


  // Ahora que tenemos el nombre del archivo hacemos un open con un llamado al sitema anfitrión
  // pedimos que sea para lectura y escritura
  int unixhandle = open(filename,O_RDWR);
  if(unixhandle == -1){
    printf("Error: archivo %s no se pudo abrir.\n", filename);
    machine->WriteRegister(2, -1);
    returnFromSystemCall();
    return;
  }

  // Ahora tenemos que registrar el archivo en nuestra tabla de archivos abiertos
  // lo registramos como ocupado
  int nachosHandle = openFilesTable->Open(unixhandle);
  if(nachosHandle == -1){
   close(unixhandle);
   machine->WriteRegister(2,-1);
   returnFromSystemCall();
   return;
  }
  // le devolmeos el handle al usuario
  machine->WriteRegister(2,nachosHandle);
  returnFromSystemCall();
}


/*
 *  System call interface: OpenFileId Write( char *, int, OpenFileId )
 */
void NachOS_Write() { 
   //Ojito nacho, con estas flags y sus siggnificados
    int addr = machine->ReadRegister(4);  // Dirección en la memoria del usuario
    int size = machine->ReadRegister(5);  // Cantidad de bytes a escribir
    int file = machine->ReadRegister(6);  // Descriptor del archivo (0:stdin, 1:stdout)
    // en este caso va a ser para la salida de la consola

    DEBUG('u', "Write system call: addr=%d, size=%d, file=%d\n", addr, size, file);

    if (file == 1) {  
        char *buffer = new char[size + 1];
        int value;

        for (int i = 0; i < size; i++) {
            if (machine->ReadMem(addr + i, 1, &value)) {
                buffer[i] = (char)value;
            } else {
                printf("Error al leer de la memoria del usuario\n");
                delete[] buffer;
                return;
            }
        }
        buffer[size] = '\0';  
        printf("%s", buffer);
        delete[] buffer;
        returnFromSystemCall();
    } else {
        printf("Solo se soporta escritura a stdout (descriptor 1) por ahora\n");
    }
}



/*
 *  System call interface: OpenFileId Read( char *, int, OpenFileId )
 */
void NachOS_Read() {		// System call 7
}


/*
 *  System call interface: void Close( OpenFileId )
 */
void NachOS_Close() {		// System call 8
}


/*
 *  System call interface: void Fork( void (*func)() )
 */
void NachOS_Fork() {		// System call 9
}


/*
 *  System call interface: void Yield()
 */
void NachOS_Yield() {		// System call 10
}


/*
 *  System call interface: Sem_t SemCreate( int )
 */
void NachOS_SemCreate() {		// System call 11
}


/*
 *  System call interface: int SemDestroy( Sem_t )
 */
void NachOS_SemDestroy() {		// System call 12
}


/*
 *  System call interface: int SemSignal( Sem_t )
 */
void NachOS_SemSignal() {		// System call 13
}


/*
 *  System call interface: int SemWait( Sem_t )
 */
void NachOS_SemWait() {		// System call 14
}


/*
 *  System call interface: Lock_t LockCreate( int )
 */
void NachOS_LockCreate() {		// System call 15
}


/*
 *  System call interface: int LockDestroy( Lock_t )
 */
void NachOS_LockDestroy() {		// System call 16
}


/*
 *  System call interface: int LockAcquire( Lock_t )
 */
void NachOS_LockAcquire() {		// System call 17
}


/*
 *  System call interface: int LockRelease( Lock_t )
 */
void NachOS_LockRelease() {		// System call 18
}


/*
 *  System call interface: Cond_t LockCreate( int )
 */
void NachOS_CondCreate() {		// System call 19
}


/*
 *  System call interface: int CondDestroy( Cond_t )
 */
void NachOS_CondDestroy() {		// System call 20
}


/*
 *  System call interface: int CondSignal( Cond_t )
 */
void NachOS_CondSignal() {		// System call 21
}


/*
 *  System call interface: int CondWait( Cond_t )
 */
void NachOS_CondWait() {		// System call 22
}


/*
 *  System call interface: int CondBroadcast( Cond_t )
 */
void NachOS_CondBroadcast() {		// System call 23
}


/*
 *  System call interface: Socket_t Socket( int, int )
 */
void NachOS_Socket() {			// System call 30
 // leemos el registro cuatro que está reservado para el primer argumento del syscall
 int domain = machine->ReadRegister(4);
 //El tipo de socket
 int type = machine->ReadRegister(5);

 int sockid = socket(domain,type,0);
 machine->WriteRegister(2,sockid);

}


/*
 *  System call interface: Socket_t Connect( char *, int )
 */
void NachOS_Connect() {		// System call 31
 int sockid = machine->ReadRegister(4);
 int ip_pointer = machine->ReadRegister(5);
 int port = machine->ReadRegister(6);
 char ip[32];
 int i = 0;
 char c;
}


/*
 *  System call interface: int Bind( Socket_t, int )
 */
void NachOS_Bind() {		// System call 32
}


/*
 *  System call interface: int Listen( Socket_t, int )
 */
void NachOS_Listen() {		// System call 33
}


/*
 *  System call interface: int Accept( Socket_t )
 */
void NachOS_Accept() {		// System call 34
}


/*
 *  System call interface: int Shutdown( Socket_t, int )
 */
void NachOS_Shutdown() {	// System call 25
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

void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

    switch ( which ) {

       case SyscallException:
          switch ( type ) {
             case SC_Halt:		// System call # 0
                NachOS_Halt();
                break;
             case SC_Exit:		// System call # 1
                NachOS_Exit();
                break;
             case SC_Exec:		// System call # 2
                NachOS_Exec();
                break;
             case SC_Join:		// System call # 3
                NachOS_Join();
                break;

             case SC_Create:		// System call # 4
                NachOS_Create();
                break;
             case SC_Open:		// System call # 5
                NachOS_Open();
                break;
             case SC_Read:		// System call # 6
                NachOS_Read();
                break;
             case SC_Write:		// System call # 7
                NachOS_Write();
                break;
             case SC_Close:		// System call # 8
                NachOS_Close();
                break;

             case SC_Fork:		// System call # 9
                NachOS_Fork();
                break;
             case SC_Yield:		// System call # 10
                NachOS_Yield();
                break;

             case SC_SemCreate:         // System call # 11
                NachOS_SemCreate();
                break;
             case SC_SemDestroy:        // System call # 12
                NachOS_SemDestroy();
                break;
             case SC_SemSignal:         // System call # 13
                NachOS_SemSignal();
                break;
             case SC_SemWait:           // System call # 14
                NachOS_SemWait();
                break;

             case SC_LckCreate:         // System call # 15
                NachOS_LockCreate();
                break;
             case SC_LckDestroy:        // System call # 16
                NachOS_LockDestroy();
                break;
             case SC_LckAcquire:         // System call # 17
                NachOS_LockAcquire();
                break;
             case SC_LckRelease:           // System call # 18
                NachOS_LockRelease();
                break;

             case SC_CondCreate:         // System call # 19
                NachOS_CondCreate();
                break;
             case SC_CondDestroy:        // System call # 20
                NachOS_CondDestroy();
                break;
             case SC_CondSignal:         // System call # 21
                NachOS_CondSignal();
                break;
             case SC_CondWait:           // System call # 22
                NachOS_CondWait();
                break;
             case SC_CondBroadcast:           // System call # 23
                NachOS_CondBroadcast();
                break;

             case SC_Socket:	// System call # 30
		NachOS_Socket();
               break;
             case SC_Connect:	// System call # 31
		NachOS_Connect();
               break;
             case SC_Bind:	// System call # 32
		NachOS_Bind();
               break;
             case SC_Listen:	// System call # 33
		NachOS_Listen();
               break;
             case SC_Accept:	// System call # 32
		NachOS_Accept();
               break;
             case SC_Shutdown:	// System call # 33
		NachOS_Shutdown();
               break;

             default:
                printf("Unexpected syscall exception %d\n", type );
                ASSERT( false );
                break;
          }
          break;

       case PageFaultException: {
          break;
       }

       case ReadOnlyException:
          printf( "Read Only exception (%d)\n", which );
          ASSERT( false );
          break;

       case BusErrorException:
          printf( "Bus error exception (%d)\n", which );
          ASSERT( false );
          break;

       case AddressErrorException:
          printf( "Address error exception (%d)\n", which );
          ASSERT( false );
          break;

       case OverflowException:
          printf( "Overflow exception (%d)\n", which );
          ASSERT( false );
          break;

       case IllegalInstrException:
          printf( "Ilegal instruction exception (%d)\n", which );
          ASSERT( false );
          break;

       default:
          printf( "Unexpected exception %d\n", which );
          ASSERT( false );
          break;
    }

}

