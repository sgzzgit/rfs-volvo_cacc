/* FILE
 *   vehicle_profile.c
 *                      To be included in profile.c for profiles of  
 *			selected vehicle information fields.	
 *
 * Copyright (c) 2002   Regents of the University of California
 *
 */

#include "vehicle_profile.h"

/* reads a profile item from a character string */

void read_profile_item (char *linebuffer, profile_item *pitem)
{
	sscanf(linebuffer, "%f %f %f %f %f %f %f %f %d %f",
			&pitem->time,
			&pitem->velocity,
			&pitem->engine_speed,
			&pitem->engine_torque,
			&pitem->retarder_torque,
			&pitem->fuel_rate,
			&pitem->deceleration,
			&pitem->brtsc1_time,
			&pitem->brtsc1_mode,
			&pitem->brtsc1_torque);
}

/* writes a profile item to a character string */

void write_profile_item (char *linebuffer, profile_item *pitem)
{
	sprintf(linebuffer, "%.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f %d %.3f\n",
			pitem->time,
			pitem->velocity,
			pitem->engine_speed,
			pitem->engine_torque,
			pitem->retarder_torque,
			pitem->fuel_rate,
			pitem->deceleration,
			pitem->brtsc1_time,
			pitem->brtsc1_mode,
			pitem->brtsc1_torque);
}

/* out of range error values used to signal error to higher level routines */

static profile_item error_item = {
	-255.0,	/* time */
	-255.0, /* velocity */
	-255.0, /* engine speed */
	-255.0, /* engine torque */
	255.0, /* retarder torque */
	-255.0, /* fuel_rate */
	255.0, /* deceleration */
	0.0, /* brtsc1_time */
	4, /* brtsc1_mode */
	-255.0, /* brtsc1 torque */
};
	
	
/* updates a profile item from the data base; if data base read fails,
 * leave an error code to the data item */

void update_profile_item(db_clt_typ *pclt, profile_item *pitem,
		timestamp_t *start_time)

{
	timestamp_t current_time;
	int millisecs;
	db_data_typ db_data;

	/* by initializing to error values, read of other variables
	 * can continue even if read to one of the variables fails
	 */
	*pitem = error_item;

	get_current_timestamp(&current_time); 
	(void) time_diff(start_time, &current_time, &millisecs);
	pitem->time = millisecs/1000.0;
/*
	if (clt_read(pclt, DB_J1939_CCVS_VAR, DB_J1939_CCVS_TYPE, &db_data)){
		j1939_ccvs_typ *p;
		p = (j1939_ccvs_typ *) db_data.value.user;
		pitem->velocity = p->wheel_based_vehicle_speed;
	}

	if (clt_read(pclt, DB_J1939_EEC1_VAR, DB_J1939_EEC1_TYPE, &db_data)){
		j1939_eec1_typ *p;
		p = (j1939_eec1_typ *) db_data.value.user;
		pitem->engine_speed = p->engine_speed;
		pitem->engine_torque = p->actual_engine_percent_torque/100.0 *
					MAX_ENGINE_TORQUE;
	}

	if (clt_read(pclt, DB_J1939_ERC1_VAR, DB_J1939_ERC1_TYPE, &db_data)){
		j1939_erc1_typ *p;
		p = (j1939_erc1_typ *) db_data.value.user;
		pitem->retarder_torque = p->actual_retarder_percent_torque;
	}

	if (clt_read(pclt, DB_J1939_LFE_VAR, DB_J1939_LFE_TYPE, &db_data)){
		j1939_lfe_typ *p;
		p = (j1939_lfe_typ *) db_data.value.user;
		pitem->fuel_rate = p->fuel_rate;
	}
		
	if (clt_read(pclt, DB_J1939_EBC1_VAR, DB_J1939_EBC1_TYPE, &db_data)){
		j1939_ebc1_typ *p;
		p = (j1939_ebc1_typ *) db_data.value.user;
		pitem->deceleration = p->total_brake_demand;
	}

	if (clt_read(pclt, DB_J1939_TSC1_BRKSRC_VAR, DB_J1939_TSC1_BRKSRC_TYPE, &db_data)){
		j1939_tsc1_typ *p;
		p = (j1939_tsc1_typ *) db_data.value.user;
		(void) time_diff(start_time, &p->timestamp, &millisecs);
		pitem->brtsc1_time = millisecs/1000.0; 
		pitem->brtsc1_mode = p->override_control_modes;
		pitem->brtsc1_torque = p->requested_torque_or_limit;
	}
*/
		
}

/* Used in sndprof to set values in longitudinal output variable from
 * values in profile.
 */
void copy_to_command (profile_item *pitem, long_output_typ *pcmd,
		 int engine_mode)

{
	pcmd->engine_speed = pitem->engine_speed;
	pcmd->engine_torque = pitem->engine_torque;
	pcmd->engine_retarder_torque = pitem->retarder_torque;
	pcmd->ebs_deceleration = pitem->deceleration;
	pcmd->engine_command_mode = engine_mode;
	if (pcmd->engine_retarder_torque < 0.0)
		pcmd->engine_retarder_command_mode = TSC_TORQUE_CONTROL;
	else
		pcmd->engine_retarder_command_mode = TSC_OVERRIDE_DISABLED;
	if (pcmd->ebs_deceleration < 0.0)
		pcmd->brake_command_mode = EXAC_ACTIVE;
	else
		pcmd->brake_command_mode = EXAC_NOT_ACTIVE;
}


static dbv_size_type dbv_used[] = {
	{DB_J1939_ERC1_VAR, sizeof(j1939_erc1_typ)},
	{DB_J1939_EBC1_VAR, sizeof(j1939_ebc1_typ)},
	{DB_J1939_EEC1_VAR, sizeof(j1939_eec1_typ)},
	{DB_J1939_CCVS_VAR, sizeof(j1939_ccvs_typ)},
	{DB_J1939_LFE_VAR, sizeof(j1939_lfe_typ)},
	{DB_J1939_TSC1_BRKSRC_VAR, sizeof(j1939_tsc1_typ)},
};

static int profile_num_dbvs = sizeof(dbv_used)/sizeof(int);

