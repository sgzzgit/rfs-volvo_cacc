/*
** can_clt.cpp
**
**
** Jorge M. Estrela da Silva, 2002
** ISEP, Porto, Portugal
**
*/

#include <sys/types.h>
#include <sys/kernel.h>
#include <sys/name.h>
#include <unistd.h>
#include <string.h>

#include "can_defs.h"
#include <stdio.h>
#include <sys/fd.h>

int can_set_filter( int fd, unsigned long id, unsigned long mask )
{
	can_clt_filter_msg_t msg;
	
	msg.type = _CAN_FILTER;
	msg.fd = fd;
	msg.id = id;
	msg.mask = mask;

	return(Sendfd(fd,&msg,0,sizeof(msg),0));
}


int can_read( int fd, unsigned long *id, char *extended, void *data, unsigned char size) {
	can_clt_msg_t msg;

	msg.type = _I82527_READ;
	msg.fd = fd;
	msg.msg.size = size;

	if(Sendfd(fd,&msg,&msg,sizeof(msg),sizeof(msg))<0)
		return(-1);
	
	
	if(id!=NULL)
		*id = CAN_ID(msg.msg);
	if(extended!=NULL)
		*extended = IS_EXTENDED_FRAME(msg.msg);
	memcpy(data,msg.msg.data,size>8?8:size);
		
	return(msg.msg.size);
}

int can_write( int fd, unsigned long id, char extended, void *data, unsigned char size) {
	can_clt_msg_t msg;

	msg.type = _I82527_WRITE;
	msg.fd = fd;
	msg.msg.size = size>8? 8 : size;
	msg.msg.id = id;
	if(extended) SET_EXTENDED_FRAME(msg.msg);

	memcpy( msg.msg.data, data, msg.msg.size );


	return(Sendfd(fd,&msg,0,sizeof(msg),0)==-1?-1:size);
}

int can_arm( int fd, pid_t proxy )
{
	can_clt_arm_msg_t msg;
	

	msg.type = _CAN_ARM;
	msg.fd = fd;
	msg.proxy = proxy;
	
	return(Sendfd(fd,&msg,0,sizeof(msg),0));

}

int can_empty_q( int fd )
{
	can_clt_empty_q_msg_t msg;
	

	msg.type = _CAN_EMPTY_Q;
	msg.fd = fd;
	
	return(Sendfd(fd,&msg,0,sizeof(msg),0));

}




