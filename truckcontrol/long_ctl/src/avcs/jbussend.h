/*
 * jbussend.h	Header file for jbussend.c 
 *
 * Copyright (c) 2002   Regents of the University of California
 *
 */
#include <sys_os.h>
#include <local.h>
#include <sys_list.h>
#include <sys/select.h>
#include <sys_lib.h>
#include <sys_rt.h>
#include <sys_ini.h>
#include "db_comm.h"
#include "db_clt.h"
#include "timestamp.h"
#include "jbus_vars.h"
#include "jbus_extended.h"
#include "j1939.h"
#include "j1939pdu_extended.h"
#include "j1939db.h"
#include "clt_vars.h"
#include "vehicle.h"
#include <timing.h>
#include <path_gps_lib.h>

/* repetition interval should be 10 milliseconds for engine,
 * 50 milliseconds for retarder, 40 milliseconds for EBS
 */
#define JBUS_INTERVAL_MSECS	5


/* info_check_type holds info read from the database, to be used for safety
 * checks -- in particular, to ensure that the transmission retarder is
 * not turned on during acceleration.
 */
typedef struct {
	float percent_engine_torque;
	float accelerator_pedal_position;
	float latitude;
} info_check_type;

/* 
 * send_jbus_type will be initialized differently for each type
 * of command that can be sent.  
 * jbus_cmd_type will hold the data for the most recently sent command
 * and be acted on by the functions in send_jbus_type.
 */
typedef struct {
	int dbv;	/* database variable number of the command type */
	union {
		j1939_tsc1_typ tsc1;
		j1939_exac_typ exac;
		path_gps_point_t gps;
	} cmd;
	struct timeb last_time;	/* last time a command was sent */	
	int interval;	/* broadcast interval when active */
	int heartbeat;	/* heartbeat interval when inactive (0 if none) */
	int override_limit;	/* time limit for same active command */
	info_check_type *pinfo;	/* pointer to safety check info */
} jbus_cmd_type;

typedef struct {
	int active;	/* if 0, send no messages of this type */
	int total_sent;	/* may wrap for long runs, for debugging */
	int slot_number;	/* needed for STB, maybe later with SSV CAN */
	int fd;		/* file descriptor of device file */
	jbus_cmd_type cmd;	/* last command of this type sent */
	int (*is_ready_to_send)(long_output_typ *ctrl, jbus_cmd_type *cmd);
	void (*update_command)(long_output_typ *ctrl, jbus_cmd_type *cmd, 
			int dosend);
	void (*cmd_to_pdu)(struct j1939_pdu *pdu, void *jcmd);
} send_jbus_type;	

#define JBUS_SEND_ENGINE_SRC_ACC		0
#define JBUS_SEND_ENGINE_RETARDER_SRC_ACC	1
#define	JBUS_SEND_EBS			2
#define	JBUS_SEND_TRANS_RETARDER	3
#define	JBUS_SEND_GPS			4
#define NUM_JBUS_SEND			2

/* is_ready_to_send functions */
extern int ready_to_send_engine (long_output_typ *ctrl, jbus_cmd_type *cmd); 
extern int ready_to_send_engine_retarder (long_output_typ *ctrl,
		 jbus_cmd_type *cmd); 
extern int ready_to_send_ebs (long_output_typ *ctrl, jbus_cmd_type *cmd); 
extern int ready_to_send_trans_retarder (long_output_typ *ctrl,
		 jbus_cmd_type *cmd); 

/* update_command functions */
extern void update_engine_tsc (long_output_typ *ctrl, jbus_cmd_type *cmd,
	 int dosend);
extern void update_engine_retarder_tsc (long_output_typ *ctrl, jbus_cmd_type *cmd,
	int dosend);
extern void update_brake_exac (long_output_typ *ctrl, jbus_cmd_type *cmd,
	int dosend);
extern void update_trans_retarder_tsc (long_output_typ *ctrl, jbus_cmd_type *cmd,
	int dosend);
extern void update_gps(long_output_typ *ctrl, jbus_cmd_type *cmd,
	int dosend);

/* overall initialization and exit functions */
extern int send_jbus_init (jbus_func_t *pjbf,
	send_jbus_type *msg, info_check_type *pinfo,
	int active_mask, char *engine_file,
	char *brake_file, char *trans_file);
extern void send_jbus_exit (jbus_func_t *pjbf, send_jbus_type *msg);  
