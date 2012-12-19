
//#include "ipnc_type.h"
#include "uart_port.h"
//#include "debug.h"


///////////////////////////////////////////////////
//param: name of port
PComPort OpenComPort(char *name)
{
	PComPort pCom =NULL;
	int fd;

	fd = open(name, O_RDWR);

	if(-1 == fd)
		return NULL;
	
	ioctl(fd, TIOCEXCL, NULL);
	
	pCom = malloc(sizeof(ComPort));
	if(pCom == NULL) {
		close(fd);
		return NULL;
	}

	strcpy(pCom->name, name);
	pCom->fd = fd;

    FD_ZERO(&pCom->block_read_fdset);
    FD_ZERO(&pCom->block_write_fdset);
    pCom->timeout.tv_sec = 1;
    pCom->timeout.tv_usec  = 80000;
	return pCom;
}

///////////////////////////////////////////////////////
//param port handle
void CloseComPort(PComPort pCom)
{
	if(pCom == NULL) return;

	ioctl(pCom->fd, TIOCNXCL, NULL);
	close(pCom->fd);

	free(pCom);
	return ;
}

////////////////////////////////////////////////////////
int SetComParam(PComPort pCom, int BaudRate, int Parity, 
                int StopBits, int databits)
{
	struct termios   options;
	int i;
	
	int speed_arr[] = { B38400, B115200, B19200, B9600, B4800, B2400, B1200, B300,
                        B38400, B19200, B9600, B4800, B2400, B1200, B300, };
	
	int name_arr[] = {38400,  115200, 19200,  9600,  4800,  2400,  1200,  300, 38400,  
                      19200,  9600, 4800, 2400, 1200,  300, };

	if(pCom == NULL) return -1;

	tcgetattr(pCom->fd, &options); 
	
	for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++) { 
		if  (BaudRate == name_arr[i]) {  			
		
			cfsetispeed(&options, speed_arr[i]);  
			cfsetospeed(&options, speed_arr[i]);    
		}  
	}


	options.c_cflag &= ~CSIZE; 
	switch (databits) /*设置数据位数*/
	{   
        case 7:		
            options.c_cflag |= CS7; 
            break;
        case 8:     
            options.c_cflag |= CS8;
            break;   
        default:    
            fprintf(stderr,"Unsupported data size\n"); return -1;  
	}

	////////////////////////////////////////////////////////////
	switch (Parity) 
	{   
        case 'n':
        case 'N':   		
            options.c_cflag &= ~PARENB;   /* Clear parity enable */
            options.c_iflag &= ~INPCK;     /* Enable parity checking */ 
            break;  
        case 'o':   
        case 'O':     
            options.c_cflag |= (PARODD | PARENB); /* 设置为奇效验*/  
            options.c_iflag |= INPCK;             /* Disnable parity checking */ 
            break;  
        case 'e':  
        case 'E':   
            options.c_cflag |= PARENB;     /* Enable parity */    
            options.c_cflag &= ~PARODD;   /* 转换为偶效验*/     
            options.c_iflag |= INPCK;       /* Disnable parity checking */
            break;
        case 'S': 
        case 's':  /*as no parity*/   
            options.c_cflag &= ~PARENB;
            options.c_cflag &= ~CSTOPB;break;  
        default:   
            fprintf(stderr,"Unsupported parity\n");    
            return -1;  
	}
	

	/////////////////////////////////////////////
	/* 设置停止位*/  
	switch (StopBits)
	{   
		case 1:    
			options.c_cflag &= ~CSTOPB;  
			break;  
		case 2:    
			options.c_cflag |= CSTOPB;  
            break;
		default:    
            fprintf(stderr,"Unsupported stop bits\n");  
            return -1; 
	} 

	//options.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);  /*Input*/
	//options.c_oflag  &= ~OPOST;   /*Output*/

	options.c_iflag &= ~(BRKINT|IXON|IXOFF|INLCR|ICRNL|IGNCR);
	options.c_iflag |= IGNBRK|IGNPAR;
	options.c_oflag &= ~OPOST;
	options.c_lflag &= ~(ISIG|ICANON|ECHO|ECHOE);
	tcflush(pCom->fd,TCIFLUSH);
	options.c_cc[VTIME] = 0; /* 设置超时15 seconds*/   
	options.c_cc[VMIN] = 0; /* Update the options and do it NOW */
	
	if (tcsetattr(pCom->fd,TCSANOW,&options) != 0)   
	{		
		return -1;  
	} 

		
	return 0;
}

////////////////////////////////////////////////////////
//
int WaitComReadEvent(PComPort pCom, int TimeOutMs)
{
 	int ret;
	struct timeval timeout; 
	
	if(pCom == NULL) return -1;

    FD_ZERO(&pCom->block_read_fdset);
    FD_SET(pCom->fd, &pCom->block_read_fdset);
    timeout.tv_sec = TimeOutMs/1000;
    timeout.tv_usec = (TimeOutMs%1000)*1000;
	ret = select(pCom->fd+1, &pCom->block_read_fdset, NULL, NULL, &timeout);
	if(!ret) return 0;

	if(FD_ISSET(pCom->fd, &pCom->block_read_fdset)) return 1;

	return 0;	
}
////////////////////////////////////////////////////////////
int WaitComWriteEvent(PComPort pCom)
{
 	int ret;
	
	if(pCom == NULL) return -1;

    FD_ZERO(&pCom->block_write_fdset);
	FD_SET(pCom->fd, &pCom->block_write_fdset);

	ret = select(pCom->fd+1, NULL, &pCom->block_write_fdset, NULL, &pCom->timeout);
	if(!ret) return 0;

	if(FD_ISSET(pCom->fd, &pCom->block_write_fdset)) return 1;

	return 0;	
}
///////////////////////////////////////////////////////////
int SendComData(PComPort pCom, char *data,  int len)
{
    int nLeft = len;
    int nTotalWrite = 0;
    int nWrite;
    
	if(pCom == NULL) return -1;
	
    while(nLeft > 0)
    {
        nWrite = write(pCom->fd, data + nTotalWrite, nLeft);
        if(nWrite < 0)
            return nWrite;
        nLeft -= nWrite;
        nTotalWrite += nWrite;
    }
    fsync(pCom->fd);
	//tcdrain(pCom->fd);
	return 0;
}
/////////////////////////////////////////////////////////////

int ReadComData(PComPort pCom, char *data, int MaxLen)
{
	int len;
	if(pCom == NULL) return -1;
	
	len = read(pCom->fd, data, MaxLen);
	return len; 
}


