/*
** test_rx.c
**
** Modified from ssv_can/can_rx.cpp for an easy to use trace format
** for the standard ID CAN bus on the IBEO laserscanner.
**
*/

#include "can_clt.h"
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/proxy.h>
#include <sys/kernel.h>
#include <unistd.h>


void main(int argc,char **argv)
{
	pid_t proxy;
	pid_t pid;
	int i;
	int fd;
	unsigned long id;  
	unsigned char size;
	unsigned char data[8];

	if(argc<2)
	{		
		printf("usage: %s device_name\n",argv[0]);
		exit(1);
	}


	
	fd = open(argv[1],O_RDWR);
	if(fd<0)
	{
		perror("can_open");
		exit(1);
	}
	else
	{
		printf("program %s, device name %s, fd: %d\n", argv[0],
			argv[1], fd);
		fflush(stdout);
	}
	
	
	close(fd);

	//can_read(fd,&msg);
	
	
		
	fd = open(argv[1],O_RDWR);
	if(fd<0)
	{
		perror("can_open");
		exit(1);
	}
	else
	{
		printf("program %s, device name %s, fd: %d\n", argv[0],
			argv[1], fd);
		fflush(stdout);
	}

	
	can_set_filter(fd,0,0);//listens all messages
	proxy = qnx_proxy_attach( 0,0,0,0);
	can_arm(fd,proxy);
	
	for(;;)
	{
		pid = Receive(0,0,0);
		
		if(pid == proxy)
		{
			i = can_read(fd,&id,(char *)NULL,data,8);
			printf("%8d %3d %3d %3d %3d %3d %3d %3d %3d\n",
				id,
				data[0],
				data[1],
				data[2],
				data[3],
				data[4],
				data[5],
				data[6],
				data[7]);
			continue;
		}
	}
	
}


