#ifndef SYS_QNX6_H
#define SYS_QNX6_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/timeb.h>
#include <sys/time.h>
#include <sys/types.h>	
// The next two lines for message queses
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/select.h>
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
#include <sys/iomsg.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>
#include <sys/siginfo.h>
#include <time.h>

#define _fmemcpy(s1,s2,n) memcpy(s1,s2,n)
#define _fmemset(s,c,n) memset(s,c,n)
#define IS_PACKED	__attribute__((__packed__))
#define COMM_OS_XPORT	COMM_QNX6_XPORT
#define _localtime(x,y)	localtime_r(x,y)

#endif /* SYS_QNX6_H */
