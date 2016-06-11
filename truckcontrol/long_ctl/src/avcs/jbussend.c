/*
 * jbussend.c 	Process to send commands, including heartbeat
 *		commands, to all j1939 buses in the vehicle.	
 *
 *		Updated for QNX6
 *
 * Copyright (c) 2008   Regents of the University of California
 *
 */
#include "jbussend.h"
#include "jbussendGPS.h"

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

#define MAX_LOOP_NUMBER 20

static db_clt_typ *pclt = NULL;  	/* Database pointer defined here so it can be used in the ready_to_send calls*/

/**
 * main calls active_loop every 5 milliseconds, loop numbers range from 0 to 20 
 * Engine, retarders, and brake messages can be sent every 20 milliseconds, 
 * brake warning message every 100 milliseconds,
 * with 20 millisecond start-up latency for engine, brakes, and retarders,
 * and 100 millisecond start-up latency for brake warning.
 * N.B.: Sending of different messages is spaced by one millisecond - see the
 * logic below. This was done in the original code; I found later (JAS, 5/6/2016)
 * that if you send them simultaneously, one of them gets lost!
 */
int active_loop(int i, int loop_number)

{
	switch(i) {
	case JBUS_SEND_ENGINE_SRC_ACC:
		 return (loop_number == 1) || (loop_number == 5) || (loop_number == 9) || (loop_number == 13) || (loop_number == 17);
	case JBUS_SEND_ENGINE_RETARDER_SRC_ACC:
		 return (loop_number == 2) || (loop_number == 6) || (loop_number == 10) || (loop_number == 14) || (loop_number == 18);
	case JBUS_SEND_XBR: 
		 return (loop_number == 3) || (loop_number == 7) || (loop_number == 11) || (loop_number == 15) || (loop_number == 19);
	case JBUS_SEND_XBR_WARN: 
		 return (loop_number % 20 == 0);
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
	if (!(loop_number == 0 || loop_number == 20))
		return;

        valid_read = clt_read(pclt, DB_J1939_EEC1_VAR,
				 DB_J1939_EEC1_TYPE, &db_data);
	if (valid_read) {
		eec1 = *((j1939_eec1_typ *) &db_data.value.user);
		pinfo->percent_engine_torque = 
				eec1.EEC1_ActualEnginePercentTorque; 
	} else
		update_invalid_read++;

        valid_read =clt_read(pclt, DB_J1939_EEC2_VAR, 
				DB_J1939_EEC2_TYPE, &db_data);
	if (valid_read) {
		eec2 = *((j1939_eec2_typ *) &db_data.value.user);
		pinfo->accelerator_pedal_position =
				 eec2.EEC2_AccelPedalPos1;
	} else
		update_invalid_read++;

}

int ready_to_send_engine_src_acc (long_output_typ *ctrl, jbus_cmd_type *cmd) 
{
	struct timeb current;
	struct timeb *last_sent = &cmd->last_time;
	j1939_tsc1_e_acc_typ *tsc1_e_acc = (j1939_tsc1_e_acc_typ *)&cmd->cmd.tsc1;
	int new_mode = ctrl->engine_command_mode;
	static int old_mode = 0;
	static float last_engine_torque = 1;
	static char state_change_counter = 0;
	can_debug_t engine_debug;
	int ret;

        /* When you shift from positive to negative torque:
        Send three TSC messages to the EMS with EngineOverrideControlMode = 0 and
        EngineTorqueLimit as 255 (Not available). After this you stop sending TSC to the EMS.
        At the same time, start sending the TSC message to the retarder (as done now).
        Do the same procedure when you go back from negative to positive torque.
        JBYTE_NOT_AVAILABLE = 0xFF (255)
        TSC_OVERRIDE_DISABLED = 0 */

//        if((last_engine_torque >= 0) && (ctrl->engine_torque < 0)) {
	if((new_mode == TSC_OVERRIDE_DISABLED) && (old_mode != TSC_OVERRIDE_DISABLED)) { 
                state_change_counter = 4;
	}

	old_mode = new_mode;

	last_engine_torque = ctrl->engine_torque;

        if(state_change_counter > 0) {
                tsc1_e_acc->EnOvrdCtrlM = 0;
                tsc1_e_acc->EnRTrqTrqLm = 255;
                new_mode = TSC_OVERRIDE_DISABLED;
		state_change_counter--; 
        }
	else
		state_change_counter = 0;

	ftime(&current);
	engine_debug.can_debug_pdu = 0x0016;
	engine_debug.EnOvrdCtrlM = BITS21(tsc1_e_acc->EnOvrdCtrlM);
	engine_debug.EnRSpdCtrlC = BITS21(tsc1_e_acc->EnRSpdCtrlC);
	engine_debug.EnOvrdCtrlMPr = BITS21(tsc1_e_acc->EnOvrdCtrlMPr);
	engine_debug.state_change_counter = BITS21(state_change_counter);
	engine_debug.new_mode = BITS21(new_mode);
 	engine_debug.engine_torque = code_percent_m125_to_p125(ctrl->engine_torque);
 	engine_debug.last_engine_torque = code_percent_m125_to_p125(last_engine_torque);
 	engine_debug.EnRTrqTrqLm = code_percent_m125_to_p125(tsc1_e_acc->EnRTrqTrqLm);

        ret = /* send heartbeat message? */
                ((TIMEB_SUBTRACT(last_sent, &current) >= cmd->heartbeat)
		&& 
		(cmd->heartbeat)	//i.e. cmd->heartbeat != 0
                &&
                (new_mode != TSC_OVERRIDE_DISABLED) 	//Control disabled?
		&& 
		(ctrl->engine_torque >= 0))		//Torque > 0?
		|| 
		(state_change_counter > 0) 		//Going from 0 or negative engine torque to positive torque?
                ;
	
	engine_debug.ret = (ret != 0);
	db_clt_write(pclt, DB_ENGINE_DEBUG_VAR, sizeof(can_debug_t), &engine_debug);

	return ret;

}

int ready_to_send_engine_retarder_src_acc (long_output_typ *ctrl, jbus_cmd_type *cmd) 
{
	struct timeb current;
	struct timeb *last_sent = &cmd->last_time;
	j1939_tsc1_er_acc_typ *tsc_er_acc = (j1939_tsc1_er_acc_typ *)&cmd->cmd.tsc1;
	int new_mode = ctrl->engine_retarder_command_mode;
	static int old_mode = 0;
	static float last_engine_retarder_torque = 1;
	static char state_change_counter = 0;
	can_debug_t engine_retarder_debug;
	int ret;

        /* When you shift from positive to negative torque:
        Send three TSC messages to the EMS with EngineOverrideControlMode = 0 and
        EngineTorqueLimit as 255 (Not available). After this you stop sending TSC to the EMS.
        At the same time, start sending the TSC message to the retarder (as done now).
        Do the same procedure when you go back from negative to positive torque.
        JBYTE_NOT_AVAILABLE = 0xFF (255)
        TSC_OVERRIDE_DISABLED = 0 */

//        if((last_engine_retarder_torque <= 0) && (ctrl->engine_retarder_torque > 0)) {
	if((new_mode == TSC_OVERRIDE_DISABLED) && (old_mode != TSC_OVERRIDE_DISABLED)) {
                state_change_counter = 4;
	}

	old_mode = new_mode;

	last_engine_retarder_torque = ctrl->engine_retarder_torque;

        if(state_change_counter > 0) {
                tsc_er_acc->EnOvrdCtrlM = 0;
                tsc_er_acc->EnRTrqTrqLm = 255;
                new_mode = TSC_OVERRIDE_DISABLED;
        	state_change_counter--;
        }
	else
		state_change_counter = 0;

	ftime(&current);

	engine_retarder_debug.can_debug_pdu = 0x0017;
	engine_retarder_debug.EnOvrdCtrlM = BITS21(tsc_er_acc->EnOvrdCtrlM);
	engine_retarder_debug.EnRSpdCtrlC = BITS21(tsc_er_acc->EnRSpdCtrlC);
	engine_retarder_debug.EnOvrdCtrlMPr = BITS21(tsc_er_acc->EnOvrdCtrlMPr);
	engine_retarder_debug.state_change_counter = BITS21(state_change_counter);
	engine_retarder_debug.new_mode = BITS21(new_mode);
 	engine_retarder_debug.engine_torque = code_percent_m125_to_p125(ctrl->engine_retarder_torque);
 	engine_retarder_debug.last_engine_torque = code_percent_m125_to_p125(last_engine_retarder_torque);
 	engine_retarder_debug.EnRTrqTrqLm = code_percent_m125_to_p125(tsc_er_acc->EnRTrqTrqLm);

        ret = /* send heartbeat message? */
                ((TIMEB_SUBTRACT(last_sent, &current) >= cmd->interval)
		&& 
		(cmd->interval)				//i.e. cmd->interval != 0
		&&
                (new_mode != TSC_OVERRIDE_DISABLED) 
                &&
		(ctrl->engine_retarder_torque <= 0))	//Retarder torque < 0?
		|| 
		(state_change_counter > 0) 		//Going from 0 or negative engine torque to positive torque?
                ;
	engine_retarder_debug.ret = (ret != 0);
	db_clt_write(pclt, DB_ENGINE_RETARDER_DEBUG_VAR, sizeof(can_debug_t), &engine_retarder_debug);
        return  ret;/* send message every interval? */
}

int ready_to_send_xbr (long_output_typ *ctrl, jbus_cmd_type *cmd) 
{
	struct timeb current;
	struct timeb *last_sent = &cmd->last_time;

	ftime(&current);

	/* message sent every 20 ms whether active or inactive */
	return (TIMEB_SUBTRACT(last_sent, &current) >= cmd->interval);
}

int ready_to_send_xbr_warn (long_output_typ *ctrl, jbus_cmd_type *cmd) 
{
	struct timeb current;
	struct timeb *last_sent = &cmd->last_time;

	ftime(&current);
 
	/* message sent every 100 ms whether active or inactive */
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

void update_engine_tsc (long_output_typ *ctrl, jbus_cmd_type *cmd, int dosend)
{
	struct timeb current;
	j1939_tsc1_e_acc_typ *tsc1_e_acc = (j1939_tsc1_e_acc_typ *)&cmd->cmd.tsc1;
	static int engine_speed_mode = 0;
	static struct timeb last_mode_change;


	/* There is an override time limit only for engine speed control */
	if (ctrl->engine_command_mode == TSC_SPEED_CONTROL) {
		if (engine_speed_mode && 
			TIMEB_SUBTRACT(&last_mode_change, &current) >
				cmd->override_limit) {
			engine_speed_mode = 0;
			tsc1_e_acc->EnOvrdCtrlM = TSC_OVERRIDE_DISABLED;
		} else {
			engine_speed_mode = 1;
			tsc1_e_acc->EnOvrdCtrlM = TSC_SPEED_CONTROL;
		}
	} else {
		engine_speed_mode = 0;
		tsc1_e_acc->EnOvrdCtrlM = ctrl->engine_command_mode;
	}
	tsc1_e_acc->EnRSpdSpdLm = ctrl->engine_speed;
	if( (ctrl->engine_priority == TSC_HIGHEST) ||  
	    (ctrl->engine_priority == TSC_HIGH) ||  
	    (ctrl->engine_priority == TSC_MEDIUM) ||  
	    (ctrl->engine_priority == TSC_LOW) ) 
		tsc1_e_acc->EnOvrdCtrlMPr = ctrl->engine_priority;

	/* convert torque to percent */
	tsc1_e_acc->EnRTrqTrqLm = ctrl->engine_torque;

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
	j1939_tsc1_er_acc_typ *tsc1_er_acc = (j1939_tsc1_er_acc_typ *)&cmd->cmd.tsc1;


	tsc1_er_acc->EnOvrdCtrlM = ctrl->engine_retarder_command_mode;
	tsc1_er_acc->EnRTrqTrqLm = ctrl->engine_retarder_torque;
	if( (ctrl->engine_retarder_priority == TSC_HIGHEST) ||  
	    (ctrl->engine_retarder_priority == TSC_HIGH) ||  
	    (ctrl->engine_retarder_priority == TSC_MEDIUM) ||  
	    (ctrl->engine_retarder_priority == TSC_LOW) ) 
		tsc1_er_acc->EnOvrdCtrlMPr = ctrl->engine_retarder_priority;
	if (dosend) {
		ftime(&current);
		cmd->last_time = current;
	}
}
#define DEBUG_BRAKE	1
void update_brake_volvo_xbr(long_output_typ *ctrl, jbus_cmd_type *cmd,
				int dosend)
{
	struct timeb current;
	j1939_volvo_xbr_typ *volvo_xbr = &cmd->cmd.volvo_xbr;

	volvo_xbr->XBRControlMode = ctrl->brake_command_mode;
	volvo_xbr->ExternalAccelerationDemand = ctrl->ebs_deceleration; 
	if( (ctrl->brake_priority == TSC_HIGHEST) ||  
	    (ctrl->brake_priority == TSC_HIGH) ||  
	    (ctrl->brake_priority == TSC_MEDIUM) ||  
	    (ctrl->brake_priority == TSC_LOW) ) 
		volvo_xbr->XBRPriority = ctrl->brake_priority;
#ifdef DEBUG_BRAKE
	printf("long_output send Volvo brake mode %d deceleration %.3f priority %d\n", 
		volvo_xbr->XBRControlMode,
		volvo_xbr->ExternalAccelerationDemand,
		volvo_xbr->XBRPriority
	);
#endif
	if (dosend) {
		ftime(&current);
		cmd->last_time = current;
	}
}

void update_brake_volvo_xbr_warn(long_output_typ *ctrl, jbus_cmd_type *cmd,
				int dosend)
{
	struct timeb current;
	j1939_volvo_xbr_warn_typ *volvo_xbr_warn = &cmd->cmd.volvo_xbr_warn;

	volvo_xbr_warn->src_address = 0x2A;
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
 */

int send_jbus_init (jbus_func_t *pjbf, send_jbus_type *msg, 
		info_check_type *pinfo,
		int active_mask, char *engine_file,
		char *brake_file, char *trans_file)
{
	int i;
	int active_message_types = 0;
	int engine_fd = -1;
	int brake_fd = -1;
	struct timeb current;

	ftime(&current);	/* use to initialize "last_time" */
	for (i = 0; i < NUM_JBUS_SEND; i++) {
		send_jbus_type *pm = &msg[i];
		jbus_cmd_type *cmd = &msg[i].cmd;
		j1939_tsc1_e_acc_typ *tsc1_e_acc = (j1939_tsc1_e_acc_typ *)&cmd->cmd.tsc1;
		j1939_tsc1_er_acc_typ *tsc1_er_acc = (j1939_tsc1_er_acc_typ *)&cmd->cmd.tsc1;
		j1939_volvo_xbr_typ *volvo_xbr = &cmd->cmd.volvo_xbr;
		j1939_volvo_xbr_warn_typ *volvo_xbr_warn = &cmd->cmd.volvo_xbr_warn;

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

		/* same initializations for all types */
		pm->active = 1;
		cmd->last_time = current;
		cmd->pinfo = pinfo;

		/* set up default values for inactive message;
		 * we pretend to be the source address that the ECM
		 * we are commanding is willing to listen to
		 */
		switch (i) {
		case JBUS_SEND_ENGINE_SRC_ACC:
			pm->slot_number = 11;
			pm->is_ready_to_send = ready_to_send_engine_src_acc;
			pm->update_command = update_engine_tsc;
			pm->cmd_to_pdu = (void *)tsc1_to_pdu;
			cmd->dbv = DB_J1939_TSC1_E_ACC_VAR;
			cmd->interval = 20;
			cmd->heartbeat = 20;
			cmd->override_limit = 5000;
//			tsc1_e_acc->EnOvrdCtrlMPr = TSC_HIGHEST; 
			tsc1_e_acc->EnOvrdCtrlMPr = TSC_HIGH; 
//			tsc1_e_acc->EnOvrdCtrlMPr = TSC_MEDIUM; 
//			tsc1_e_acc->EnOvrdCtrlMPr = TSC_LOW; 
			/* Cummins only supports speed control condition 01 */
			tsc1_e_acc->EnRSpdCtrlC = 1;
			tsc1_e_acc->EnOvrdCtrlM = TSC_OVERRIDE_DISABLED;
			tsc1_e_acc->EnRSpdSpdLm = 0.0;
			tsc1_e_acc->EnRTrqTrqLm = 0.0;
			tsc1_e_acc->destination_address = J1939_ADDR_ENGINE;
			tsc1_e_acc->destination_address = 0;
			tsc1_e_acc->src_address = J1939_ADDR_ACC;
			break;
		case JBUS_SEND_ENGINE_RETARDER_SRC_ACC:
			pm->slot_number = 12;
			pm->is_ready_to_send = ready_to_send_engine_retarder_src_acc;
			pm->update_command = update_engine_retarder_tsc;
			pm->cmd_to_pdu = (void *)tsc1_to_pdu;
			cmd->dbv = DB_J1939_TSC1_ER_ACC_VAR;
			cmd->interval = 20;
			cmd->heartbeat = 20;	/* not used */
			cmd->override_limit = 0;	/* not used */
//			tsc1_er_acc->EnOvrdCtrlMPr = TSC_HIGHEST;
			tsc1_er_acc->EnOvrdCtrlMPr = TSC_HIGH;
//			tsc1_er_acc->EnOvrdCtrlMPr = TSC_MEDIUM;
//			tsc1_er_acc->EnOvrdCtrlMPr = TSC_LOW;
			/* Cummins only supports speed control condition 01 */
			tsc1_er_acc->EnRSpdCtrlC = 1;
			tsc1_er_acc->EnOvrdCtrlM = TSC_OVERRIDE_DISABLED; 
			tsc1_er_acc->EnRSpdSpdLm = 0.0;
			tsc1_er_acc->EnRTrqTrqLm = 0.0;
			tsc1_er_acc->destination_address = J1939_ADDR_ENG_RTDR;
			tsc1_er_acc->destination_address = 0x0F;
			tsc1_er_acc->src_address = J1939_ADDR_ACC;
			break;

		case JBUS_SEND_XBR:
			pm->slot_number = 13;
			pm->is_ready_to_send = ready_to_send_xbr;
			pm->update_command = update_brake_volvo_xbr;
			pm->cmd_to_pdu = (void *) volvo_xbr_to_pdu;
			cmd->dbv = DB_J1939_VOLVO_XBR_VAR;
			cmd->interval = 20;
			cmd->heartbeat = 20;	
			cmd->override_limit = 0;	//not used
			volvo_xbr->ExternalAccelerationDemand = 0.0;
			volvo_xbr->XBREBIMode = 2;
			volvo_xbr->XBRPriority = 3;
			volvo_xbr->XBRControlMode = 0;
			volvo_xbr->XBRUrgency = 0;
			volvo_xbr->spare1 = 0xFF;
			volvo_xbr->spare2 = 0xFF;
			volvo_xbr->spare3 = 0xFF;
			volvo_xbr->XBRMessageCounter = 0;
			volvo_xbr->XBRMessageChecksum = 0;

			volvo_xbr->pdu_format = 0xEF;
			volvo_xbr->destination_address = J1939_ADDR_BRAKE;
			volvo_xbr->src_address = J1939_ADDR_ACC;
			break;

		case JBUS_SEND_XBR_WARN:
			pm->slot_number = 14;
			pm->is_ready_to_send = ready_to_send_xbr_warn;
			pm->update_command = update_brake_volvo_xbr_warn;
			pm->cmd_to_pdu = (void *) volvo_xbr_warn_to_pdu;
			cmd->dbv = DB_J1939_VOLVO_XBR_WARN_VAR;
			cmd->interval = 100;
			cmd->heartbeat = 100;	
			cmd->override_limit = 0;	//not used
			volvo_xbr_warn->byte1 = 0xFF;
			volvo_xbr_warn->byte2 = 0x31;
			volvo_xbr_warn->byte3 = 0xFF;
			volvo_xbr_warn->byte4 = 0xFF;
			volvo_xbr_warn->byte5 = 0xFF;
			volvo_xbr_warn->byte6 = 0xFF;
			volvo_xbr_warn->byte7 = 0xFF;
			volvo_xbr_warn->byte8 = 0xFF;
			volvo_xbr_warn->pdu_format = 0xFF;
			volvo_xbr_warn->destination_address = 0x10;
			volvo_xbr_warn->src_address = J1939_ADDR_ACC;
			break;

		}
	}
	if (!active_message_types) return 0;

	if (brake_file == NULL && engine_file != NULL)
		brake_file = engine_file;
	if (trans_file == NULL && engine_file != NULL)
		trans_file = engine_file;

	if (msg[JBUS_SEND_ENGINE_SRC_ACC].active ||
		 	msg[JBUS_SEND_ENGINE_RETARDER_SRC_ACC].active) {
		if (engine_file == NULL) {
			printf("no engine device specified\n"); 
			fflush(stdout);
			return 0;	/* must be defined */
		}
		engine_fd = (pjbf->init)(engine_file, O_WRONLY, NULL);
		if (engine_fd < 0) {
			printf("open error, engine file");
			printf(" %s fd %d\n", engine_file, engine_fd);
			fflush(stdout);
			return 0;
		} else if (debug) {
			printf("successfully open engine %s\n", engine_file);
			fflush(stdout);
		}
		msg[JBUS_SEND_ENGINE_SRC_ACC].fd = engine_fd;
		msg[JBUS_SEND_ENGINE_RETARDER_SRC_ACC].fd = engine_fd;
		printf("engine_fd %d\n", engine_fd);	
	}
	if (msg[JBUS_SEND_XBR].active && brake_file != NULL) {
		if (strcmp(brake_file, engine_file) == 0 && engine_fd >= 0)  
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
		msg[JBUS_SEND_XBR].fd = brake_fd;
		msg[JBUS_SEND_XBR_WARN].fd = brake_fd;
	}
	if (debug) {
		printf("Initialized %d messages\n ", active_message_types);
		fflush(stdout);
	}
	return active_message_types;
}

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
	j1939_volvo_xbr_typ *pvolvo_xbr;
	j1939_volvo_xbr_warn_typ *pvolvo_xbr_warn;
	int i;

	for (i = 0; i < NUM_JBUS_SEND; i++) {
		pm = &msg[i];
		cmd = &pm->cmd;
		if (pm->active) {
			if (i == JBUS_SEND_XBR) {
				pvolvo_xbr= &cmd->cmd.volvo_xbr;
				pvolvo_xbr->XBRControlMode =
					 XBR_NOT_ACTIVE;
				volvo_xbr_to_pdu(&pdu, pvolvo_xbr);
			}
			else if (i == JBUS_SEND_XBR_WARN) {
				pvolvo_xbr_warn= &cmd->cmd.volvo_xbr_warn;
				volvo_xbr_warn_to_pdu(&pdu, pvolvo_xbr_warn);
			}
			else {
				ptsc = &cmd->cmd.tsc1;
				ptsc->EnOvrdCtrlM = 
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
	XBR_NOT_ACTIVE,	/* brake command mode */
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
	j1939_tsc1_e_acc_typ *tsc1_e_acc = (j1939_tsc1_e_acc_typ *)&cmd->cmd.tsc1;
	j1939_tsc1_er_acc_typ *tsc1_er_acc = (j1939_tsc1_er_acc_typ *)&cmd->cmd.tsc1;
	j1939_volvo_xbr_typ *volvo_xbr = (j1939_volvo_xbr_typ *)&cmd->cmd.volvo_xbr;
	int dbv = cmd->dbv;

	printf("Message to %s\n", 
		(dbv == DB_J1939_TSC1_E_ACC_VAR) ? "engine":
		((dbv == DB_J1939_TSC1_ER_ACC_VAR)? "engine retarder":
		((dbv == DB_J1939_VOLVO_XBR_VAR)? "brake":
		((dbv == DB_J1939_VOLVO_XBR_WARN_VAR)? "brake warning":
		((dbv == DB_J1939_TSC1_TRTDR_VAR)? "trans retarder":
				"unknown")))));
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
	case DB_J1939_TSC1_E_ACC_VAR:
		printf("\tmode %d, engine speed %.3f, torque %.3f\n",
			tsc1_e_acc->EnOvrdCtrlM,
			tsc1_e_acc->EnRSpdSpdLm,
			tsc1_e_acc->EnRTrqTrqLm);
		printf("\tdestination %d, source %d\n", tsc1_e_acc->destination_address,
			tsc1_e_acc->src_address);
		break;
	case DB_J1939_TSC1_ER_ACC_VAR:
		printf("\tmode %d, torque %.3f\n",
			tsc1_er_acc->EnOvrdCtrlM,
			tsc1_er_acc->EnRTrqTrqLm);
		printf("\tdestination %d, source %d\n", tsc1_er_acc->destination_address,
			tsc1_er_acc->src_address);
		break;
	case DB_J1939_VOLVO_XBR_VAR:
		printf("\tebi mode %d, deceleration %.3f\n",
			volvo_xbr->XBREBIMode,
			volvo_xbr->ExternalAccelerationDemand);
 		printf("\tcontrol mode %d, priority %d\n",
			volvo_xbr->XBRControlMode,
			volvo_xbr->XBRPriority);
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
	long_output_typ ctrl;		/* Variable containing control values */
	posix_timer_typ *ptimer;      	/* Timing proxy */
	int interval = JBUS_INTERVAL_MSECS; /* Main loop execution interval */
	int loop_number = 0;		/* Used to space out messages */
	char *engine_file = NULL;
	char *brake_file = NULL;
	char *trans_file = NULL;
	jbus_func_t jfunc;
	int active_mask = (1 << NUM_JBUS_SEND) - 1;	/* all active */  
	int rtn_jmp = -1;	/* value returned from setjmp */
	char hostname[MAXHOSTNAMELEN+1]; /* used in opening database */
	char *progname = argv[0];	/* used in opening database */
	char *vehicle_str = "Blue";	/* vehicle identifier */
	info_check_type current_info;	/* info for monitoring safety */
	FILE *fpin;

	get_local_name(hostname, MAXHOSTNAMELEN);

        while ((ch = getopt(argc, argv, "a:b:cde:t:v:")) != EOF) {
                switch (ch) {
		case 'a': active_mask = atoi(optarg);
			  break;	
		case 'b': brake_file = strdup(optarg);
			  break;
		case 'c': jfunc.send = send_can;
			  jfunc.receive = receive_can;
			  jfunc.init = init_can;
			  break;
		case 'd': debug = 1;
			  break;
		case 'e': engine_file = strdup(optarg);
			  break;
		case 't': trans_file = strdup(optarg);
			  break;
		case 'v': vehicle_str = strdup(optarg);
			  break;
                default:  printf( "Usage: %s ", argv[0]);
			  printf("-b brake device -e engine device\n");
                	  printf("-c(use CAN) -d(debug) \n");
			  exit(EXIT_FAILURE);
                          break;
                }
        }
	if (debug) {
		printf("jbussend: active mask 0x%x\n", active_mask);
		printf("Eng %s Brake %s Trans %s", engine_file,
			brake_file ? brake_file : engine_file,
			trans_file ? trans_file : engine_file);
		fflush(stdout);
	}

	if ((fpin = get_ini_section("/home/truckcontrol/test/realtime.ini", vehicle_str))
                                                                 == NULL) {
                printf("Cannot get ini file %s, section %s\n",
                   "realtime.ini", vehicle_str);
                fflush(stdout);
        }
	else
		printf("Initialization file %s\n", "realtime.ini");

        max_engine_torque = get_ini_double(fpin, "EngineReferenceTorque",
                                        MAX_ENGINE_TORQUE);    /* RPM */
        max_retarder_torque = get_ini_double(fpin, "RetarderReferenceTorque",
                                        MAX_RETARDER_TORQUE);    /* RPM */

	if (!send_jbus_init(&jfunc, msg, &current_info, active_mask,
		 engine_file, brake_file, trans_file)) {
		printf("Send structure initialization failed\n");
		fflush(stdout);
		exit (EXIT_FAILURE);
	}
	if (debug) {
		int i;
		for (i = 0; i < NUM_JBUS_SEND; i++)
			print_send_jbus_type(&msg[i]); 
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
		printf("jbussend, %d engine, %d engine retarder, %d brake, ",
			msg[JBUS_SEND_ENGINE_SRC_ACC].total_sent,
			msg[JBUS_SEND_ENGINE_RETARDER_SRC_ACC].total_sent,
			msg[JBUS_SEND_XBR].total_sent);
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
		int i;	/* counter for message types */
		tmg.exec_num++;
		
		ctrl = get_ctrl_var(&pclt, hostname, progname);
//printf("Got to 5.5 loop number %d\n", loop_number);
		for (i = 0; i < NUM_JBUS_SEND; i++) { 
			send_jbus_type *pm = &msg[i];

			if (pm->active) {	
				int can_send;
				jbus_cmd_type *cmd = &pm->cmd;
				can_send = active_loop(i, loop_number)
					&& (pm->is_ready_to_send)(&ctrl, cmd);
				pm->update_command(&ctrl, cmd, can_send);
//printf("Got to 5.5 i %d pm->active %d can_send %d\n", i, pm->active, can_send);
				if (can_send) {
					if (i == JBUS_SEND_ENGINE_SRC_ACC) {
						(pm->cmd_to_pdu)(&pdu, &cmd->cmd.tsc1);	
/*
						printf("jbussend: Got to 5.5 %d millisec src_address %#hhx dest address %#hhx loop num %d\n",
							cmd->last_time.millitm,
							cmd->cmd.tsc1.src_address,
							cmd->cmd.tsc1.destination_address,
							loop_number
						);
*/
					}
					if (i == JBUS_SEND_ENGINE_RETARDER_SRC_ACC) {
						(pm->cmd_to_pdu)(&pdu, &cmd->cmd.tsc1);	
/*
						printf("jbussend: Got to 5.5 %d millisec src_address %#hhx dest address %#hhx \n",
							cmd->last_time.millitm,
							cmd->cmd.tsc1.src_address,
							cmd->cmd.tsc1.destination_address
						);
*/
					}

					if (i == JBUS_SEND_XBR) {
						(pm->cmd_to_pdu)(&pdu, &cmd->cmd.volvo_xbr);	
/*
						printf("jbussend: Got to 6 %d millisec src_address %#hhx dest address %#hhx XBREBIMode #%hhx XBRPriority %#hhx XBRControlMode %#hhx XBRUrgency %#hhx spare1 %#hhx spare2 %#hhx spare3 %#hhx XBRMessageCounter %#hhx XBRMessageChecksum %#hhx \n",
							cmd->last_time.millitm,
							cmd->cmd.volvo_xbr.src_address,
							cmd->cmd.volvo_xbr.destination_address,
							cmd->cmd.volvo_xbr.XBREBIMode,
							cmd->cmd.volvo_xbr.XBRPriority,
							cmd->cmd.volvo_xbr.XBRControlMode,
							cmd->cmd.volvo_xbr.XBRUrgency,
							cmd->cmd.volvo_xbr.spare1,
							cmd->cmd.volvo_xbr.spare2,
							cmd->cmd.volvo_xbr.spare3,
							cmd->cmd.volvo_xbr.XBRMessageCounter,
							cmd->cmd.volvo_xbr.XBRMessageChecksum
						);
*/
					}
					else if (i == JBUS_SEND_XBR_WARN) {
						(pm->cmd_to_pdu)(&pdu, &cmd->cmd.volvo_xbr_warn);	
/*
						printf("jbussend: Got to 6.5 %d millisec src_address %#hhx dest address %#hhx priority %d R %d DP %d pdu_format %#hhx pdu_specific %#hhx src_address %#hhx \n",
							cmd->last_time.millitm,
							cmd->cmd.volvo_xbr_warn.src_address,
							cmd->cmd.volvo_xbr_warn.destination_address,
							pdu.priority,
							pdu.R,
							pdu.DP,
							pdu.pdu_format,
							pdu.pdu_specific,
							pdu.src_address
						);
*/
					}

				else {
						(pm->cmd_to_pdu)(&pdu, &cmd->cmd.tsc1);	
/*
						printf("jbussend: Got to 7 src_address %#hhx dest address %#hhx EnOvrdCtrlMPr #%hhx EnRSpdCtrlC %#hhx EnOvrdCtrlM %#hhx EnRSpdSpdLm %.3f EnRTrqTrqLm %.3f\n",
							cmd->cmd.tsc1.src_address,
							cmd->cmd.tsc1.destination_address,
							cmd->cmd.tsc1.EnOvrdCtrlMPr,
							cmd->cmd.tsc1.EnRSpdCtrlC,
							cmd->cmd.tsc1.EnOvrdCtrlM,
							cmd->cmd.tsc1.EnRSpdSpdLm,
							cmd->cmd.tsc1.EnRTrqTrqLm
						);
*/
				}
					jfunc.send(pm->fd, &pdu, pm->slot_number);
					if (debug) {	
							j1939_pdu_typ pdu_out;
							pdu_to_pdu(&pdu, &pdu_out);
							print_pdu(&pdu_out, stdout, 1);
					}
					pm->total_sent++;
				}
			}
		}
		loop_number++;
		if (loop_number == MAX_LOOP_NUMBER) loop_number = 0;

		/* update info before waiting */
		update_info_for_check(pclt, &current_info, loop_number);

		/* Now wait for proxy from timer */
		TIMER_WAIT( ptimer );
	}
}
