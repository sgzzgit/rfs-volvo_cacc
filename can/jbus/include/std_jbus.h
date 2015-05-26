/**\file
 *	Standard header files used by most Jbus routines
 *	Ifdef'd for differeneces between QNX4 and QNX6.
 *	
 */
#ifndef STD_JBUS_H
#define STD_JBUS_H

#ifdef __QNXNTO__
#include <sys_qnx6.h>
#include <local.h>
#include <sys_rt.h>
#include <sys_ini.h>
#include <das_clt.h>
#include <timing.h>
#include <timestamp.h>
#include "db_clt.h"
#include "jbus.h"
#include "j1939.h"
#include "j1939pdu.h"
#include "j1939db.h"
#include "j1587.h"
#include "j1939scale.h"
#include "j1587scale.h"
#include "jbus_vars.h"
#include "can_defs.h"
#include "lai.h"
#include "laipdu.h"
#include "laiscale.h"
#else	// assume QNX4
#include <assert.h>
#include <fcntl.h>
//#include <i86.h>
#include <local.h>
#include <math.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <sys/kernel.h>
//#include <sys/proxy.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/timeb.h>
//#include <sys/timers.h>
#include <sys/types.h>
//#include <sys_das.h>
#include <sys_lib.h>
#include <sys_list.h>
#include <sys_rt.h>
#include <time.h>
#include <timing.h>
#include <timestamp.h>
#include <unistd.h>
#include "db_comm.h" 
#include "db_clt.h"
#include "j1939.h"
#include "j1939db.h"
#include "jbus.h"
#include "jbus_vars.h"
#include "j1939pdu.h"
#include "j1939scale.h"
#include "j1587scale.h"
//#include "can_clt.h"
#include "lai.h"
#include "laiscale.h"
// ChannelCreate only on QNX6, needed for timer initialization
#define ChannelCreate(x) x
#endif

#ifndef ERROR
#define ERROR 0
#endif

#endif
