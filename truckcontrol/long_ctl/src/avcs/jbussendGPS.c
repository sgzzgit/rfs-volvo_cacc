/*
 * jbussend.c 	Process to send commands, including heartbeat
 *		commands, to all j1939 buses in the vehicle.	
 *
 *		Updated for QNX6
 *
 * Copyright (c) 2008   Regents of the University of California
 *
 */
#include "jbussendGPS.h"
#include "path_gps_lib.h"
#include <enu2lla.h>
#include "can_defs.h"
#include "can_client.h"
#include "sys_os.h"
#include "db_utils.h"

#define NUM_JBUS_SEND 1
#define DB_VOLVO_GPS_CAN_TYPE	2345
#define DB_VOLVO_GPS_CAN_VAR	DB_VOLVO_GPS_CAN_TYPE

extern void print_long_output_typ (long_output_typ *ctrl);
static struct avcs_timing tmg;

/* these are values for trucks, read realtime.ini for buses */
static float max_engine_torque = MAX_ENGINE_TORQUE;
static float max_retarder_torque = MAX_RETARDER_TORQUE;

int debug = 0;

static int sig_list[]=
{
	SIGINT,
	SIGQUIT,
	SIGTERM,
	SIGALRM,
	ERROR
};

static jmp_buf exit_env;

static void sig_hand(int code)
{
	if (code == SIGALRM)
		return;
	else
		longjmp(exit_env, code);
}

#define MAX_LOOP_NUMBER 8

typedef struct {
	unsigned int sof: 1; //Start-of-frame = 0
	unsigned int gps_position_pdu: 11; // =0x0010
	unsigned int rtr: 1 ; //Remote transmission request=0
	unsigned int extension: 1; //Extension bit=0
	unsigned int reserved: 1; //Reserved bit=0
	unsigned int dlc: 4; 	//Data length code=8
	unsigned int latitude : 31; 	//Max=124.748, Min=-90.000, resolution=1E-07, offset=-90.000
	unsigned int pos_data_valid: 1;
	unsigned int longitude : 32; ; 	//Max=249.497, Min=-180.000, resolution=1E-07, offset=-180.000
	unsigned int crc: 15; 		//Cyclic redundancy check. Calculated
	unsigned int crcd: 1;		//CRC delimiter=1
	unsigned int ack_slot: 1;	//Acknowledge slot=1
	unsigned int ack_delimiter: 1;	//Acknowledge delimiter=1
	unsigned int eof:7;		//End-of-frame=0x07
} IS_PACKED gps_position_t;

typedef struct {
	unsigned short gps_time_pdu; // =0x0013
	unsigned int year :12; 	//Max=4095, Min=0, resolution=1, offset=0
	unsigned int month :4; 	//Max=15, Min=0, resolution=1, offset=0
	unsigned int day :9; 	//Max=511, Min=0, resolution=1, offset=0
	unsigned int hour :5; 	//Max=31, Min=0, resolution=1, offset=0
	unsigned int minute :6; //Max=63, Min=0, resolution=1, offset=0
	unsigned int second :6; //Max=63, resolution=1, offset=0
	unsigned int split_second :10, :12; 	//Max=102.3, Min=0, resolution=0.1, offset=0
} IS_PACKED gps_time_t;

unsigned long LAT_MULTIPLIER=10E7;
unsigned long LAT_MIN	=0;
unsigned long LAT_MAX	=(unsigned int)0x7FFFFFFF;
unsigned long LAT_OFFSET=-90.000;

unsigned long LONGITUDE_MULTIPLIER=10E7;
unsigned long LONGITUDE_MIN=0;
unsigned long LONGITUDE_MAX=(unsigned int)0xFFFFFFFF;
unsigned long LONGITUDE_OFFSET=-180.000;

unsigned long POS_DATA_IS_VALID=0x10000000;
unsigned long POS_DATA_INVALID=0x7FFFFFFF;

int update_gps_position(path_gps_point_t *hb, char *buf) {
	int retval;
	int i;
	unsigned int latitude;
	unsigned int longitude;
	unsigned short checksum = 0;

//	hb->latitude = 37.914773;
//	hb->longitude = -122.334691;
printf("update_gps_position: hb.latitude %.7f hb.longitude %.7f\n", hb->latitude, hb->longitude);
	latitude = (unsigned int)( 10000000 * (hb->latitude + 90.0));
	longitude = (unsigned int)( 10000000 * (hb->longitude + 180.0));
	if( (latitude >= LAT_MIN) &&
	    (latitude >= LAT_MAX) &&
	    (longitude <= LONGITUDE_MIN) &&
	    (longitude >= LONGITUDE_MAX) )
		latitude |= POS_DATA_IS_VALID;
	else
		latitude &= POS_DATA_INVALID;
//printf("latitude %u %#X longitude %u %#X\n", latitude, latitude, longitude, longitude);

	buf[0] = latitude & 0xFF;
	buf[1] = (latitude >> 8) & 0xFF;
	buf[2] = (latitude >> 16) & 0xFF;
	buf[3] = (latitude >> 24) & 0xFF;
	buf[4] = longitude & 0xFF;
	buf[5] = (longitude >> 8) & 0xFF;
	buf[6] = (longitude >> 16) & 0xFF;
	buf[7] = (longitude >> 24) & 0xFF;

	checksum = 0;
	for(i=0; i<8;i++) 
		checksum += buf[i];
	checksum &= 0x7FFF;
	buf[8] = (checksum >> 8) & 0x7;
	buf[9] = checksum & 0xFF;
	buf[10] = 0xBF;
	buf[11] = 0xFF;

	printf("GPS Position msg: ");
	for(i=0; i<12; i++)
		printf("%hhx ", buf[i]);
	printf("\n");

	return 0;
};

int update_gps_time(path_gps_point_t *hb, char *buf) {
	int retval;
	int year = 2015;
	char month = 3;
	short day = 26;
	

	buf[0] = year & 0xFF;
	buf[1] = (year >> 8) & 0x0F;
	buf[1] |= (month << 4) & 0xF0;
	buf[2] = day & 0xFF;
	buf[3] = (day >> 8) & 0x01;
	buf[3] |= (hb->utc_time.hour << 1) & 0x3E;
	buf[3] |= (hb->utc_time.min << 6) & 0xC0;
	buf[4] = (hb->utc_time.min >> 2) & 0x0F;
	buf[4] |= (hb->utc_time.sec << 4) & 0xF0;
	buf[5] = (hb->utc_time.sec >> 4) & 0x03;
	buf[5] |= (hb->utc_time.millisec/100 << 2) & 0xFC;
};

/**
 * main calls active_loop every 5 milliseconds, loop numbers range from 0 to 8
 * Engine message can be sent every 10 milliseconds, retarders and brake
 * every 40 milliseconds, with 10 millisecond start-up latency for engine,
 * 20 ms for brake and 40 ms for retarders..
 */
int active_loop(int i, int loop_number)

{
	switch(i) {
	case JBUS_SEND_ENGINE:
		 return (loop_number % 2 == 0);
	case JBUS_SEND_ENGINE_RETARDER:
		 return (loop_number == 1);
	case JBUS_SEND_EBS: 
		return (loop_number == 3) || (loop_number == 7);
	case JBUS_SEND_TRANS_RETARDER:
		 return (loop_number == 5);
	}
	return 0;
}
static int update_invalid_read = 0;
 
/**
 * update_info_for_check
 * 
 * If database is created and control is active, read the database
 * variables needed to monitor engine behavior.
 */
void update_info_for_check (db_clt_typ *pclt, info_check_type *pinfo,
	int loop_number)

{
	db_data_typ db_data;	
	j1939_eec1_typ eec1;
	j1939_eec2_typ eec2;
	int valid_read;
	

	/* if no database, just return */
	if (pclt == NULL) 
		return;

	/* Only read every 20 milliseconds, since messages are not
	 * updated faster than that */
	if (!(loop_number == 0 || loop_number == 4))
		return;

        valid_read = clt_read(pclt, DB_J1939_EEC1_VAR,
				 DB_J1939_EEC1_TYPE, &db_data);
	if (valid_read) {
		eec1 = *((j1939_eec1_typ *) &db_data.value.user);
		pinfo->percent_engine_torque = 
				eec1.actual_engine_percent_torque; 
	} else
		update_invalid_read++;

        valid_read =clt_read(pclt, DB_J1939_EEC2_VAR, 
				DB_J1939_EEC2_TYPE, &db_data);
	if (valid_read) {
		eec2 = *((j1939_eec2_typ *) &db_data.value.user);
		pinfo->accelerator_pedal_position =
				 eec2.accelerator_pedal_position;
	} else
		update_invalid_read++;

}

int ready_to_send_engine (long_output_typ *ctrl, jbus_cmd_type *cmd) 
{
	struct timeb current;
	struct timeb *last_sent = &cmd->last_time;
	j1939_tsc1_typ *tsc = &cmd->cmd.tsc1;
	int new_mode = ctrl->engine_command_mode;
	int old_mode = tsc->override_control_modes;

	ftime(&current);
 
	return  /* send heartbeat message */
		(((TIMEB_SUBTRACT(last_sent, &current) >= cmd->heartbeat)
				&& cmd->heartbeat))
		|| 
		/* send message to disable control */ 
		(new_mode == TSC_OVERRIDE_DISABLED
			 && old_mode != TSC_OVERRIDE_DISABLED)
		||
		/* called at 10 ms intervals, always send active engine msg */
        	(new_mode != TSC_OVERRIDE_DISABLED) 
		;
}

int ready_to_send_engine_retarder (long_output_typ *ctrl, jbus_cmd_type *cmd) 
{
	struct timeb current;
	struct timeb *last_sent = &cmd->last_time;
	j1939_tsc1_typ *tsc = &cmd->cmd.tsc1;
	int new_mode = ctrl->engine_retarder_command_mode;
	int old_mode = tsc->override_control_modes;

	ftime(&current);
 
	return  /* no heartbeat message for engine retarder */ 
		/* send message to disable control */ 
		((new_mode == TSC_OVERRIDE_DISABLED
			 && old_mode != TSC_OVERRIDE_DISABLED)
		||
		/* send active message every 50 milleseconds */
        	((new_mode != TSC_OVERRIDE_DISABLED) 
		&& (TIMEB_SUBTRACT(last_sent, &current) >= cmd->interval)))
		;
}

int ready_to_send_ebs (long_output_typ *ctrl, jbus_cmd_type *cmd) 
{
	struct timeb current;
	struct timeb *last_sent = &cmd->last_time;

	ftime(&current);
 
	/* message sent every 40 ms whether active or inactive */
	return (TIMEB_SUBTRACT(last_sent, &current) >= cmd->interval);
}

/**
 * Maximum idle percent torque value; greater value indicates transmission
 * retarder should not be enabled.
 */
#define IDLE_PERCENT_TORQUE	20.0

/**
 * Positive value for long_output_typ acc_pedal_control indicates transmission
 * retarder should not be enabled.
 */
#define ACC_OFF_VOLTAGE	0.0

int ready_to_send_trans_retarder (long_output_typ *ctrl, jbus_cmd_type *cmd) 
{
	struct timeb current;
	struct timeb *last_sent = &cmd->last_time;
	j1939_tsc1_typ *tsc = &cmd->cmd.tsc1;
	int new_mode = ctrl->trans_retarder_command_mode;
	int old_mode = tsc->override_control_modes;
	

	ftime(&current);
 
#if 0
	/* make sure not to turn on during acceleration, and to turn
	 * off if active during acceleration.
	 */
	if (new_mode == TSC_TORQUE_CONTROL) {
		if (pinfo->percent_engine_torque > IDLE_PERCENT_TORQUE || 
		    pinfo->accelerator_pedal_position > 0.0 ||
		    ctrl->acc_pedal_control > ACC_OFF_VOLTAGE ) {
			printf("both acc and trans retarder active\n");
			ctrl->trans_retarder_command_mode = new_mode =
				TSC_OVERRIDE_DISABLED;
		}
	}
#endif
	return  /* no heartbeat message for transmission retarder */ 
		/* send message to disable control */ 
		((new_mode == TSC_OVERRIDE_DISABLED
			 && old_mode != TSC_OVERRIDE_DISABLED)
		||
		/* send active message every 50 milleseconds */
        	((new_mode != TSC_OVERRIDE_DISABLED) 
		&& (TIMEB_SUBTRACT(last_sent, &current) >= cmd->interval)))
		;
}

void update_engine_tsc (long_output_typ *ctrl, jbus_cmd_type *cmd, int dosend)
{
	struct timeb current;
	j1939_tsc1_typ *tsc = &cmd->cmd.tsc1;
	static int engine_speed_mode = 0;
	static struct timeb last_mode_change;

	/* There is an override time limit only for engine speed control */
	if (ctrl->engine_command_mode == TSC_SPEED_CONTROL) {
		if (engine_speed_mode && 
			TIMEB_SUBTRACT(&last_mode_change, &current) >
				cmd->override_limit) {
			engine_speed_mode = 0;
			tsc->override_control_modes = TSC_OVERRIDE_DISABLED;
		} else {
			engine_speed_mode = 1;
			tsc->override_control_modes = TSC_SPEED_CONTROL;;
		}
	} else {
		engine_speed_mode = 0;
		tsc->override_control_modes = ctrl->engine_command_mode;
	}
	tsc->requested_speed_or_limit = ctrl->engine_speed;

	/* convert torque to percent */
	tsc->requested_torque_or_limit = 
		(100.0 * ctrl->engine_torque/max_engine_torque);

	if (dosend) {
		ftime(&current);
		cmd->last_time = current;
		if (engine_speed_mode == 0)
			last_mode_change = current;
	}
}

void update_engine_retarder_tsc (long_output_typ *ctrl, jbus_cmd_type *cmd,
				int dosend)
{
	struct timeb current;
	j1939_tsc1_typ *tsc = &cmd->cmd.tsc1;


	tsc->override_control_modes = ctrl->engine_retarder_command_mode;
	tsc->requested_torque_or_limit = 
		(100.0 * ctrl->engine_retarder_torque/max_retarder_torque);
	if (dosend) {
		ftime(&current);
		cmd->last_time = current;
	}
}

void update_brake_exac (long_output_typ *ctrl, jbus_cmd_type *cmd,
				int dosend)
{
	struct timeb current;
	j1939_exac_typ *exac = &cmd->cmd.exac;

	exac->external_deceleration_control_mode = ctrl->brake_command_mode;
#ifdef DEBUG_BRAKE
	printf("long_output send %d ebs deceleration %.3f\n", dosend,
				ctrl->ebs_deceleration);
#endif
	exac->requested_deceleration_to_ebs = ctrl->ebs_deceleration; 
	if (dosend) {
		exac->alive_signal++;
		ftime(&current);
		cmd->last_time = current;
	}
}

void update_trans_retarder_tsc (long_output_typ *ctrl, jbus_cmd_type *cmd,
				int dosend)
{
	struct timeb current;
	j1939_tsc1_typ *tsc = &cmd->cmd.tsc1;


	tsc->override_control_modes = ctrl->trans_retarder_command_mode;

	/* since we do not have a maximum reference torque available,
	 * let the controller decide how to set this as a percent
	 */
	tsc->requested_torque_or_limit =ctrl->trans_retarder_value; 

	if (dosend) {
		ftime(&current);
		cmd->last_time = current;
	}
}


/**
 * Initializes send_jbus_type structures for each active message type. 
 * Message type is active if corresponding bit in active_mask is set to 1.
 * Complications are due to different possible system configurations:
 * one, two or three different device files may be used for the engine,
 * brake and transmission J1939 network communications.
 *
 * Returns 0 if no device file can be opened; uses engine device for
 * brakes and transmission if transmission or brake files are NULL and
 * transmission retarder or brakes are active.
 *
 * Also returns 0 if no active message types, otherwise returns the number
 * of active message types.

int send_jbus_init (jbus_func_t *pjbf, send_jbus_type *msg, 
		info_check_type *pinfo,
		int active_mask, char *gps_file)
{
	int i;
	int active_message_types = 0;
	int gps_fd = -1;
	struct timeb current;

	ftime(&current);	/* use to initialize "last_time" 
	for (i = 0; i < NUM_JBUS_SEND; i++) {
		send_jbus_type *pm = &msg[i];
		jbus_cmd_type *cmd = &msg[i].cmd;
		gps_position_t *gps_position = &cmd->cmd.gps_position;
		gps_time_t *gps_time = &cmd->cmd.gps_time;

		pm->total_sent = 0;
		if (!(active_mask & (1 << i))){
			pm->active = 0;
			if (debug)
				printf("Message type %d inactive\n", i);
			continue;
		} else if (debug) {
			printf("Message type %d active\n", i);
		}
		
		active_message_types++;

		/* same initializations for all types 
		pm->active = 1;
		cmd->last_time = current;
		cmd->pinfo = pinfo;

		/* set up default values for inactive message;
		 * we pretend to be the source address that the ECM
		 * we are commanding is willing to listen to
		 
		switch (i) {
		case JBUS_SEND_GPS:
			pm->slot_number = 11;
			pm->is_ready_to_send = ready_to_send_engine;
			pm->update_command = update_gps;
			pm->cmd_to_pdu = (void *) tsc1_to_pdu;
			cmd->dbv = DB_GPS_PT_LCL_VAR;
			cmd->interval = 10;
			cmd->heartbeat = 200;
			cmd->override_limit = 5000;
			tsc->override_control_mode_priority = TSC_HIGHEST; 
			/* Cummins only supports speed control condition 01 
			tsc->requested_speed_control_conditions = 1;
			tsc->override_control_modes = TSC_OVERRIDE_DISABLED;
			tsc->requested_speed_or_limit = 0.0;
			tsc->requested_torque_or_limit = 0.0;
			tsc->destination_address = J1939_ADDR_ENGINE;
			tsc->src_address = J1939_ADDR_TRANS;
			break;
		case JBUS_SEND_ENGINE_RETARDER:
			pm->slot_number = 12;
			pm->is_ready_to_send = ready_to_send_engine_retarder;
			pm->update_command = update_engine_retarder_tsc;
			pm->cmd_to_pdu = (void *) tsc1_to_pdu;
			cmd->dbv = DB_J1939_TSC1_RTDR_VAR;
			cmd->interval = 40;
			cmd->heartbeat = 0;	/* not used 
			cmd->override_limit = 0;	/* not used 
			tsc->override_control_mode_priority = TSC_HIGHEST;
			/* Cummins only supports speed control condition 01 
			tsc->requested_speed_control_conditions = 1;
			tsc->override_control_modes = TSC_OVERRIDE_DISABLED; 
			tsc->requested_speed_or_limit = 0.0;
			tsc->requested_torque_or_limit = 0.0;
			tsc->destination_address = J1939_ADDR_ENG_RTDR;
			tsc->src_address = J1939_ADDR_TRANS;
			break;
		case JBUS_SEND_EBS:
			pm->slot_number = 13;
			pm->is_ready_to_send = ready_to_send_ebs;
			pm->update_command = update_brake_exac;
			pm->cmd_to_pdu = (void *) exac_to_pdu;
			cmd->dbv = DB_J1939_EXAC_VAR;
			cmd->interval = 40;
			cmd->heartbeat = 40;	
			cmd->override_limit = 0;	 not used 
			exac->ebs_override_control_mode_priority = TSC_HIGHEST;
			exac->external_deceleration_control_mode = EXAC_NOT_ACTIVE;
			exac->requested_deceleration_to_ebs = 0.0;
			exac->edc_override_control_mode_priority = 3;  lowest 
			exac->override_control_modes = TSC_OVERRIDE_DISABLED; 
			exac->requested_torque_to_edc = 0.0;
			exac->alive_signal = 0;
			exac->acc_internal_status = J1939_BYTE_UNDEFINED;
			exac->undefined = J1939_BYTE_UNDEFINED;
			exac->src_address = J1939_ADDR_ACC;
			break;
		case JBUS_SEND_TRANS_RETARDER:
			pm->slot_number = 10;
			pm->is_ready_to_send = ready_to_send_trans_retarder;
			pm->update_command = update_trans_retarder_tsc;
			pm->cmd_to_pdu = (void *) tsc1_to_pdu;
			cmd->dbv = DB_J1939_TSC1_RTDR_VAR;
			cmd->interval = 40;
			cmd->heartbeat = 0;	not used 
			cmd->override_limit = 0;	 not used 
			tsc->override_control_mode_priority = TSC_LOW;
			tsc->requested_speed_control_conditions = 0; /* not used 
			tsc->override_control_modes = TSC_OVERRIDE_DISABLED;
			tsc->requested_speed_or_limit = 0.0;
			tsc->requested_torque_or_limit = 0.0;
			tsc->destination_address = J1939_ADDR_TR_RTDR;
			tsc->src_address = J1939_ADDR_CC;
			break;
		}
	}
	if (!active_message_types) return 0;

	if (brake_file == NULL && gps_file != NULL)
		brake_file = gps_file;
	if (trans_file == NULL && gps_file != NULL)
		trans_file = gps_file;

	if (msg[JBUS_SEND_ENGINE].active ||
		 	msg[JBUS_SEND_ENGINE_RETARDER].active) {
		if (gps_file == NULL) {
			printf("no engine device specified\n"); 
			fflush(stdout);
			return 0;	must be defined 
		}
		engine_fd = (pjbf->init)(gps_file, O_WRONLY, NULL);
		if (engine_fd < 0) {
			printf("open error, engine file");
			printf(" %s fd %d\n", gps_file, engine_fd);
			fflush(stdout);
			return 0;
		} else if (debug) {
			printf("successfully open engine %s\n", gps_file);
			fflush(stdout);
		}
		msg[JBUS_SEND_ENGINE].fd = engine_fd;
		msg[JBUS_SEND_ENGINE_RETARDER].fd = engine_fd;
		printf("engine_fd %d\n", engine_fd);	
	}
	if (msg[JBUS_SEND_EBS].active && brake_file != NULL) {
		if (strcmp(brake_file, gps_file) == 0 && engine_fd >= 0)  
			brake_fd = engine_fd;
		else {
			brake_fd = (pjbf->init)(brake_file, O_WRONLY, NULL);
			if (brake_fd < 0) {
				printf("open error, brake file");
				printf(" %s fd %d\n", brake_file, brake_fd);
				fflush(stdout);
				return 0;
			} else if (debug) {
				printf("successfully open brake file %s\n",
						brake_file); 
				fflush(stdout);
			}
		}
		msg[JBUS_SEND_EBS].fd = brake_fd;
	}
	if (msg[JBUS_SEND_TRANS_RETARDER].active) {
		if (strcmp(trans_file, gps_file) == 0 && engine_fd >= 0)
			trans_fd = engine_fd;
		else if (strcmp(trans_file, brake_file) == 0 && brake_fd >= 0)  
			trans_fd = brake_fd;
		else {
			trans_fd = (pjbf->init)(trans_file, O_WRONLY, NULL);
			if (trans_fd < 0) {
				printf("open error, transmission file");
				printf(" %s fd %d\n", trans_file, trans_fd);
				fflush(stdout);
				return 0;
			} else if (debug) {
				printf("successfully open trans %s\n", trans_file);
				fflush(stdout);
			}
		}
		msg[JBUS_SEND_TRANS_RETARDER].fd = trans_fd;
		printf("trans_fd: %d\n", trans_fd);
	}
	if (debug) {
		printf("Initialized %d messages\n ", active_message_types);
		fflush(stdout);
	}
	return active_message_types;
}
*/

/**
 * Send disable messages before exiting and close files; this should
 * happen only with an emergency shutdown.
 */

void send_jbus_exit (jbus_func_t *pjbf, send_jbus_type *msg)  
{
	struct j1939_pdu pdu;
	send_jbus_type *pm;
	jbus_cmd_type *cmd;
	j1939_tsc1_typ *ptsc;
	j1939_exac_typ *pexac;
	int i;

	for (i = 0; i < NUM_JBUS_SEND; i++) {
		pm = &msg[i];
		cmd = &pm->cmd;
		if (pm->active) {
			if (i == JBUS_SEND_EBS) {
				pexac = &cmd->cmd.exac;
				pexac->external_deceleration_control_mode =
					 EXAC_NOT_ACTIVE;
				exac_to_pdu(&pdu, pexac);
			}
			else {
				ptsc = &cmd->cmd.tsc1;
				ptsc->override_control_modes = 
					TSC_OVERRIDE_DISABLED;
				tsc1_to_pdu(&pdu, ptsc);
			}
			(pjbf->send)(pm->fd, &pdu, pm->slot_number);
		}
	}
}

static long_output_typ inactive_ctrl = {
	600.0, 	/* engine speed, truck idle, don't care since disabled */
	400.0,  /* engine torque, truck idle, don't care since disabled */
          0.0,  /* retarder torque, don't care since disabled */
	TSC_OVERRIDE_DISABLED,	/* engine command mode */ 
	TSC_OVERRIDE_DISABLED,	/* engine retarder command mode */
	 0.0,	/* accelerator pedal voltage -- not used by jbussend */
	 0.0,	/* ebs deceleration */ 
	EXAC_NOT_ACTIVE,	/* brake command mode */
	 0.0,	/* percent of maximum transmission retarder activation */ 
	TSC_OVERRIDE_DISABLED,	/* transmission retarder command mode */
}; 

/* Under automatic control, reads longitudinal control variable from
 * database. If database not active, or variable not created, returns
 * inactive control values. Keeps trying to open database every time
 * it is called if initial failure.
 */

long_output_typ get_ctrl_var (db_clt_typ **ppclt, char *hostname, 
			char *progname)
{
	db_clt_typ *pclt;
	db_data_typ read_data_var;	
	int valid_read;
	if (*ppclt == NULL) {
		pclt = clt_login(progname, hostname, DEFAULT_SERVICE,
			COMM_OS_XPORT);
		*ppclt = pclt;
		if (pclt == NULL) {
			return (inactive_ctrl);
		}
		if (debug)
			printf("%s logged in to database 0x%x\n", progname,
				(unsigned int) pclt);
	} else 
		pclt = *ppclt;
		
	valid_read = clt_read(pclt, DB_LONG_OUTPUT_VAR,
			 DB_LONG_OUTPUT_TYPE, &read_data_var);

	
	if (valid_read) {
		if (debug)
			print_long_output_typ((long_output_typ *) &read_data_var.value.user);
		return *((long_output_typ *) &read_data_var.value.user);
	}

	else {
		clt_logout(pclt);	/* just in case db_slv still there */
		*ppclt = NULL;	/* assume need to reconnect to database */
		return inactive_ctrl;
	}
} 


/* for debugging */

void print_send_jbus_type(send_jbus_type *pm)

{
	jbus_cmd_type *cmd = &pm->cmd;
	j1939_tsc1_typ *tsc = &cmd->cmd.tsc1;
	j1939_exac_typ *exac = &cmd->cmd.exac;
	int dbv = cmd->dbv;

	printf("Message to %s\n", 
		(dbv == DB_J1939_TSC1_VAR) ? "engine":
		((dbv == DB_J1939_TSC1_RTDR_VAR)? "engine retarder":
		((dbv == DB_J1939_EXAC_VAR)? "brake":
		((dbv == DB_J1939_TSC1_TRTDR_VAR)? "trans retarder":
				"unknown"))));
	printf("\tactive %d, slot %d, total %d, fd %d\n",
		 pm->active, pm->slot_number, pm->total_sent, pm->fd);

	printf("\tready_to_send ptr 0x%x, update ptr 0x%x, to_pdu ptr 0x%x\n",
		(unsigned int) pm->is_ready_to_send, 
		(unsigned int) pm->update_command, 
		(unsigned int) pm->cmd_to_pdu);

	printf("\tinterval %d, heartbeat %d, override_limit %d\n",
			cmd->interval, cmd->heartbeat, cmd->override_limit);
	
	printf("\tlast_sent: sec %d millisec %d\n", cmd->last_time.time,
			cmd->last_time.millitm); 

	switch (dbv) {
	case DB_J1939_TSC1_VAR:
		printf("\tmode %d, engine speed %.3f, torque %.3f\n",
			tsc->override_control_modes,
			tsc->requested_speed_or_limit,
			tsc->requested_torque_or_limit);
		printf("\tdestination %d, source %d\n", tsc->destination_address,
			tsc->src_address);
		break;
	case DB_J1939_TSC1_RTDR_VAR:
	case DB_J1939_TSC1_TRTDR_VAR:
		printf("\tmode %d, torque %.3f\n",
			tsc->override_control_modes,
			tsc->requested_torque_or_limit);
		printf("\tdestination %d, source %d\n", tsc->destination_address,
			tsc->src_address);
		break;
	case DB_J1939_EXAC_VAR:
		printf("\tebs mode %d, deceleration %.3f\n",
			exac->external_deceleration_control_mode,
			exac->requested_deceleration_to_ebs);
 		printf("\tedc mode %d, edc torque %.3f, alive_signal %d\n",
			exac->override_control_modes,
			exac->requested_torque_to_edc,
			exac->alive_signal);
		break;
	}
	fflush(stdout);
}

/* for debugging */
void print_long_output_typ (long_output_typ *ctrl)
{
	printf("Ctrl engine speed %.3f\n",  ctrl->engine_speed);
	printf("Ctrl engine torque %.3f\n",  ctrl->engine_torque);
	printf("Ctrl engine retarder torque %.3f\n",  ctrl->engine_retarder_torque);
	printf("Ctrl engine_command_mode %d\n", ctrl->engine_command_mode);
	printf("Ctrl engine_retarder_command_mode %d\n", ctrl->engine_retarder_command_mode);
	printf("Ctrl EBS deceleration %.3f\n",  ctrl->ebs_deceleration);
	printf("Ctrl brake_command_mode %d\n", ctrl->brake_command_mode);
	fflush(stdout);
}

int main( int argc, char *argv[] )
{
	int ch;		
	send_jbus_type msg[NUM_JBUS_SEND];
	struct j1939_pdu pdu;		/* Protocol Data Unit */
        db_clt_typ *pclt = NULL;  	/* Database pointer */
	long_output_typ ctrl;		/* Variable containing control values */
	posix_timer_typ *ptimer;      	/* Timing proxy */
	int interval = JBUS_INTERVAL_MSECS; /* Main loop execution interval */
	int loop_number = 0;		/* Used to space out messages */
	char *gps_file = NULL;
	int gps_fd = -1;
	jbus_func_t jfunc;
	int active_mask = (1 << NUM_JBUS_SEND) - 1;	/* all active */  
	int rtn_jmp = -1;	/* value returned from setjmp */
	char hostname[MAXHOSTNAMELEN+1]; /* used in opening database */
	char *progname = argv[0];	/* used in opening database */
        int xport = COMM_OS_XPORT;
	char *domain = DEFAULT_SERVICE;
	char *vehicle_str = "VLN475";	/* vehicle identifier */
	info_check_type current_info;	/* info for monitoring safety */
	FILE *fpin;
	char use_can = 0;
	path_gps_point_t hb;
	gps_position_t gps_position;
	gps_time_t gps_time;
	int gps_db_num = DB_GPS_PT_LCL_VAR;
	unsigned char buf[16];

	get_local_name(hostname, MAXHOSTNAMELEN);

        while ((ch = getopt(argc, argv, "a:b:cde:t:v:n:g:")) != EOF) {
                switch (ch) {
		case 'a': active_mask = atoi(optarg);
			  break;	
		case 'c': use_can = 1;
			  break;
		case 'd': debug = 1;
			  break;
		case 'g': gps_file = strdup(optarg);
			  break;
		case 'v': vehicle_str = strdup(optarg);
			  break;
		case 'n': gps_db_num = atoi(optarg);
			  break;	
                default:  printf( "Usage: %s ", argv[0]);
                	  printf("-c(use CAN) -d(debug) \n");
                	  printf("-n(db number) -g(/dev/can1) \n");
			  exit(EXIT_FAILURE);
                          break;
                }
        }

        pclt = db_list_init(argv[0], hostname, domain, xport, NULL, 0, NULL, 0);

	if(use_can) {
		gps_fd = can_open(gps_file, O_WRONLY);
		if(gps_fd < 0) {
			printf("Failed to connect to %s. Exiting....\n", gps_file);
			exit(EXIT_FAILURE);
		}
		else
			printf("Connection to %s successful!\n", gps_file);
	}

	if ((ptimer = timer_init( interval, 0 )) == NULL) {
		printf("Unable to initialize jbussend timer\n");
		exit (EXIT_FAILURE);
	}
	avcs_start_timing(&tmg);

	if( (rtn_jmp = setjmp( exit_env )) != 0 ) {
		avcs_end_timing(&tmg);
		send_jbus_exit(&jfunc, msg);
                close_local_database(pclt);

		avcs_print_timing(stdout, &tmg);
		fprintf(stderr, "jbussend exit %d\n", rtn_jmp);
		fflush(stderr);
//		printf("jbussend, %d gps",
//			msg[JBUS_SEND_GPS].total_sent);
		printf("%d invalid data server reads\n", update_invalid_read);

		exit( EXIT_SUCCESS );
	}
	else
		sig_ign( sig_list, sig_hand );

	if (debug) {
		printf("Begin loop, interval %d\n", interval);
		fflush(stdout);
	}
	update_info_for_check(pclt, &current_info, loop_number);
	for ( ; ; ) {

		db_clt_read(pclt, gps_db_num, sizeof(path_gps_point_t), &hb);
		if(use_can) {
			update_gps_position(&hb, buf);
			printf("lat %f long %f\n", hb.latitude, hb.longitude);
			can_write(gps_fd, 0x10, 0, buf, 12);
			update_gps_time(&hb, buf);
			can_write(gps_fd, 0x13, 0, buf, 6);
		}
		/* Now wait for proxy from timer */
		TIMER_WAIT( ptimer );
	}
}
