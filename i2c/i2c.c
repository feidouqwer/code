#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>

#include <linux/i2c.h>
#include <linux/i2c-dev.h>


#define I2C_DEVICE_PATH		"/dev/i2c-1"

void print_usage()
{
	printf("usage:\n");
	printf("\ti2c r addr reg\n");
	printf("\ti2c w addr reg val\n");
}


int i2c_read(int fd, unsigned char addr, unsigned char reg, unsigned char *value)
{
	unsigned char mbuf[2][128];
	struct i2c_msg msg[2];
	struct i2c_rdwr_ioctl_data rdwr;
	int rtcd = 0;

	msg[0].addr  = addr;
	msg[0].flags = 0;
	msg[0].len	 = 1;
	msg[0].buf	 = mbuf[0];
	msg[1].addr  = addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len	 = 1;
	msg[1].buf	 = mbuf[1];

	rdwr.msgs = msg;
	rdwr.nmsgs = 2;
	mbuf[0][0] = reg;
	mbuf[1][0] = 0;
	if (ioctl(fd, I2C_RDWR, &rdwr) < 0) {
		perror("ioctl");
		fprintf(stdout, "NG:ioctl %s\n", "I2C_RDWR");
		rtcd = -1;
	}
	else {
	   *value = (unsigned short)mbuf[1][0];
	}

	return rtcd;
}


int i2c_write(int fd, unsigned char addr, unsigned char reg, unsigned char val)
{
	int rcode = 0;
	int errCode = 0;
	int rtcd = 0;
	int num = 0;
	

	unsigned char data[2];

	rcode = ioctl(fd, I2C_SLAVE, addr);
	if (rcode != 0) {
		errCode = errno;
		perror("ioctl(I2C_SLAVE)");
		fprintf(stdout, "NG:I2C_SLAVE error(%d)\n", errCode);
		rtcd = -1;
		return rtcd;
	}

	data[0] = reg;
	data[1] = val;

	printf("adr:%02X <- dt:%02X\n", data[0], data[1]);

	num = write(fd, data, 2);
	if (num < 0) {
		perror("write");
		fprintf(stdout, "NG:i2c write\n");
		rtcd = -1;
		return rtcd;
	}
	if (num != 2) {
		fprintf(stdout, "NG:i2c write size(%d)\n", num);
		rtcd = -1;
		return rtcd;
	}
	
	return rtcd;
}


int str2int(const char *str, int *value)
{
	if(strncmp(str, "0x", 2) == 0)
		sscanf(str, "0x%x", value);
	else if(strncmp(str, "0X", 2) == 0)
		sscanf(str, "0X%x", value);
	else
		sscanf(str, "%d", value);
	return 0;
}

int main(int argc, char *argv[])
{
	int tmp;
	int addr;
	unsigned char reg;
	unsigned char val;
	int fd;
	
	if(argc != 4 && argc != 5)
	{
		print_usage();
		return 0;
	}
	
	//addr = atoi(argv[2]);
	//reg = atoi(argv[3]);
	str2int(argv[2], &tmp);
	addr = tmp;
	str2int(argv[3], &tmp);
	reg = (unsigned char)tmp;
	
	fd = open(I2C_DEVICE_PATH, O_RDWR);
	if(fd < 0)
	{
		printf("Failed to open %s\n", I2C_DEVICE_PATH);
		return -1;
	}
#if 0	
	if(ioctl(fd, I2C_TENBIT, 0) < 0 ||
	   ioctl(fd, I2C_SLAVE, addr) < 0)
	{
		printf("Failed to set i2c attribute.\n");
		perror("failed");
		close(fd);
		return -1;
	}
#endif	
	if(strcmp(argv[1], "r") == 0 && argc == 4)
	{
		printf("Read i2c 0x%02x reg 0x%02x\n", addr, reg);
		if(i2c_read(fd, addr, reg, &val) < 0)
		{
			printf("Failed to read i2c.\n");
		}
		else
		{
			printf("i2c read value: 0x%02x\n", val);
		}
	}
	else if(strcmp(argv[1], "w") == 0 && argc == 5)
	{
		//val = atoi(argv[4]);
		str2int(argv[4], &tmp);
		val = (unsigned char)tmp;
		printf("Write i2c 0x%02x reg 0x%02x val 0x%02x\n", addr, reg, val);
		if(i2c_write(fd, addr, reg, val) < 0)
		{
			printf("i2c write failed!\n");
		}
		else
		{
			printf("i2c write successfaully.\n");
		}
	}
	else
	{
		print_usage();
	}
	close(fd);
	return 0;
}

