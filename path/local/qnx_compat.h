// To avoid cluttering code ported from QNX with #ifdefs, in particular
// the database code where db_comm.c is heavily switched on COMM_QNX_XPORT,
// we #define these away here. 
#ifndef PATH_QNX_COMPAT_H
#define PATH_QNX_COMPAT_H

#if (defined __QNXNTO__) || (! defined __QNX__) 
// QNX4isms

// to allow compilation on other systems of QNX-specific sections
typedef int nid_t;
#define qnx_name_attach(x,y)	0	// not an error, unused
#define qnx_name_detach(x,y)	0	// not an error, unused
#define qnx_name_locate(x,y,z,w)	0
#define qnx_proxy_attach(w, x, y, z)  0
#define qnx_proxy_detach(x) 0

struct _mxfer_entry {
	int dummy;
};
#define _setmx(x,y,z)	
#define Sendmx(x,y,z,w,v)	0
#define Receivemx(x,y,z)	0
#define Replymx(x,y,z)	0
#define Trigger(x)	0
#define Receive(x, y, z)	0

#define get_local_name(x,y)	gethostname(x,y)
#define DB_CHANNEL(pclt)	0

#endif  /* QNX 4isms */

//QNX6isms
#ifndef __QNXNTO__

#define ChannelCreate(x)	x

struct _msg_info {
        int dummy;
};

struct _pulse {
        long int type;
};

typedef struct _name_attach {
        int chid;
} name_attach_t;

#define name_attach(x,y,z)              0
#define name_detach(x,y)                0
#define name_open(x,y)                  0
#define MsgReceive_r(w, x, y, z)        0
#define MsgReceivePulse_r(w, x, y, z)   0
#define MsgReply_r(w, x, y, z)          0
#define MsgSend_r(v, w, x, y, z)        0
#define MsgDeliverEvent_r( x, y)        0
#define SIGEV_PULSE_INIT(v, w, x, y, z) 0
#define _IO_CONNECT                     0
#define _PULSE_CODE_MINAVAIL            0
#define EOK                             0

#endif /* QNX 6isms */

/* allowed in any QNX */
#ifndef __QNX__
#define setprio(x, y)  0
#endif

/* Linux-isms */
#ifdef __QNX__
#define msgrcv(v, w, x, y, z) 0 
#define msgget(x, y)   0
#define msgsnd(w, x, y, z)  0
#define msgctl(x, y, z)  0
#endif

#endif /* PATH_QNX_COMPAT_H */
