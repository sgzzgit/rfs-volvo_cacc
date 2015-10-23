#ifndef SYS_LINUX_H
#define SYS_LINUX_H

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/timeb.h>
#include <sys/time.h>
#include <sys/types.h>	// The next two lines for message queses
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <time.h>
#include <math.h>
#include <errno.h>
#include <local.h>
#include <qnx_compat.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>


#define IS_PACKED	__attribute__((__packed__))
#define COMM_OS_XPORT	COMM_PSX_XPORT	
#define atoh(x)	strtol((x), NULL, 16)
#define _localtime(x,y)	localtime_r(x,y)

#endif /* SYS_LINUX_H */
