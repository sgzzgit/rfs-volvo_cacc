/* FILE
 *   jcmd_profile.c
 *
 *  To be included in profile.c for profiles of  
 *  selected vehicle information fields.	
 *
 * Copyright (c) 2002   Regents of the University of California
 *
 */

#include "jcmd_profile.h"

/* reads a profile item from a character string */

void read_profile_item (char *linebuffer, profile_item *pitem)
{
	sscanf(linebuffer, "%f %d %f %f %d %d %d %f %d %d %d %d %f %d %d %f",
		&pitem->time,
		&pitem->tsc_engine_override_control_modes,
		&pitem->tsc_engine_speed,     
		&pitem->tsc_engine_torque,    
		&pitem->tsc_engine_destination_address,       
		&pitem->tsc_engine_src_address,    
		&pitem->tsc_eretarder_override_control_modes,
		&pitem->tsc_eretarder_torque,    
		&pitem->tsc_eretarder_destination_address,
		&pitem->tsc_eretarder_src_address,   
		&pitem->ebs_override_control_mode_priority,
		&pitem->external_deceleration_control_mode,
		&pitem->requested_deceleration_to_ebs,
		&pitem->edc_override_control_mode_priority,
		&pitem->edc_override_control_modes,
		&pitem->requested_torque_to_edc);
}

/* writes a profile item to a character string */

void write_profile_item (char *linebuffer, profile_item *pitem)
{
	sprintf(linebuffer,
	 "%.3f %d %.3f %.3f %d %d %d %.3f %d %d %d %d %.3f %d %d %.3f\n",
		pitem->time,
		pitem->tsc_engine_override_control_modes,
		pitem->tsc_engine_speed,     
		pitem->tsc_engine_torque,    
		pitem->tsc_engine_destination_address,       
		pitem->tsc_engine_src_address,    
		pitem->tsc_eretarder_override_control_modes,
		pitem->tsc_eretarder_torque,    
		pitem->tsc_eretarder_destination_address,
		pitem->tsc_eretarder_src_address,   
		pitem->ebs_override_control_mode_priority,
		pitem->external_deceleration_control_mode,
		pitem->requested_deceleration_to_ebs,
		pitem->edc_override_control_mode_priority,
		pitem->edc_override_control_modes,
		pitem->requested_torque_to_edc);
}

/* out of range error values used to signal error to higher level routines */

static profile_item error_item = {
		-255.0, /* time */
		     4, /* tsc_engine_override_control_modes */
		-255.0, /* tsc_engine_speed */
		-255.0, /* tsc_engine_torque */
	     	   255, /* tsc_engine_destination_address */
		   255, /* tsc_engine_src_address */
		     4, /* tsc_eretarder_override_control_modes */
		-255.0, /* tsc_eretarder_torque */
		   255, /* tsc_eretarder_destination_address */
		   255, /* tsc_eretarder_src_address */
	             4, /* ebs_override_control_mode_priority */
		     4, /* external_deceleration_control_mode */
		-255.0, /* requested_deceleration_to_ebs */
		     4, /* edc_override_control_mode_priority */
		     4, /* edc_override_control_modes */
		-255.0, /* requested_torque_to_edc */
		   255, /* exac_src_address */
};
	
	
/* updates a profile item from the data base; if data base read fails */
 * leave an error code to the data item */

void update_profile_item(db_clt_typ *pclt, profile_item *pitem,
		struct timeb *start_time)

{
	struct timeb current_time;
	db_data_typ db_data;

	/* by initializing to error values, read of other variables
	 * can continue even if read to one of the variables fails
	 */
	*pitem = error_item;

	ftime (&current_time); 
	pitem->time = TIMEB_SUBTRACT(start_time, &current_time) / 1000.0;

	if (clt_read(pclt, DB_J1939_TSC1_VAR, DB_J1939_TSC1_TYPE, &db_data)){
		j1939_tsc1_typ *p;
		p = (j1939_tsc1_typ *) db_data.value.user;
		pitem->tsc_engine_override_control_modes =
			p->override_control_modes;
		pitem->tsc_engine_speed = p->requested_speed_or_limit;     
		pitem->tsc_engine_torque = p->requested_torque_or_limit;    
		pitem->tsc_engine_destination_address = p->destination_address;       
		pitem->tsc_engine_src_address = p->src_address;    
	}

	if (clt_read(pclt, DB_J1939_TSC1_RTDR_VAR, DB_J1939_TSC1_RTDR_TYPE, &db_data)){
		j1939_tsc1_rtdr_typ *p;
		p = (j1939_tsc1_rtdr_typ *) db_data.value.user;
		pitem->tsc_eretarder_override_control_modes =
			p->override_control_modes;
		pitem->tsc_eretarder_torque = p->requested_torque_or_limit;    
		pitem->tsc_eretarder_destination_address = p->destination_address;       
		pitem->tsc_eretarder_src_address = p->src_address;    
	}

	if (clt_read(pclt, DB_J1939_EXAC_VAR, DB_J1939_EXAC_TYPE, &db_data)){
		j1939_exac_typ *p;
		p = (j1939_exac_typ *) db_data.value.user;
		pitem->ebs_override_control_mode_priority =
			p->ebs_override_control_mode_priority;
		pitem->external_deceleration_control_mode =
			p->external_deceleration_control_mode;
		pitem->requested_deceleration_to_ebs =
			p->requested_deceleration_to_ebs;,
		pitem->edc_override_control_mode_priority =
			p->edc_override_control_mode_priority;
		pitem->edc_override_control_modes =
			p->override_control_modes;
		pitem->requested_torque_to_edc =
			p->requested_torque_to_edc;
	}
		
}

static dbv_size_type dbv_used[] = {
	DB_J1939_TSC1_VAR, sizeof(j1939_tsc1_typ),
	DB_J1939_TSC1_RTDR_VAR, sizeof(j1939_tsc1_typ),
	DB_J1939_EXAC_VAR, sizeof(j1939_exac_typ),
};

static int profile_num_dbvs = sizeof(dbv_used)/sizeof(int);

