
#ifndef __PORT_H__
#define __PORT_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <asm/ioctls.h>
#include <sys/ioctl.h>

typedef struct Tag_ComPort
{
char name[32];
int fd;
fd_set block_read_fdset;
fd_set block_write_fdset;
struct timeval timeout;
}ComPort, *PComPort;


////////////////////////////////////////////////
//extern function
PComPort OpenComPort(char *name);
void CloseComPort(PComPort pCom);
int SetComParam(PComPort pCom, int BaudRate, int Parity,
						int StopBits, int databits);
int SendComData(PComPort pCom, char *data,  int len);
int ReadComData(PComPort pCom, char *data, int MaxLen);
int WaitComReadEvent(PComPort pCom, int TimeOutMs);
int WaitComWriteEvent(PComPort pCom);

#endif

