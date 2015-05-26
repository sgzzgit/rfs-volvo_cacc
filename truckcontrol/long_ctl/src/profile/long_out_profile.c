/* FILE
 *   long_out_profile.c
 *
 *   For profiles of the longitudinal output variable. 
 *
 * Copyright (c) 2002   Regents of the University of California
 *
 */

#include "long_out_profile.h"

/* reads a profile item from a character string */

void read_profile_item (char *linebuffer, profile_item *pitem)
{
	sscanf(linebuffer, "%f %d %f %f %d %f %d %f",
			&pitem->time,
			&pitem->engine_command_mode,
			&pitem->engine_speed,
			&pitem->engine_torque,
			&pitem->engine_retarder_command_mode,
			&pitem->engine_retarder_torque,
			&pitem->brake_command_mode,
			&pitem->ebs_deceleration);
}

/* writes a profile item to a character string */

void write_profile_item (char *linebuffer, profile_item *pitem)
{
	sprintf(linebuffer, "%.3f %d %.3f %.3f %d %.3f %d %.3f\n",
			pitem->time,
			pitem->engine_command_mode,
			pitem->engine_speed,
			pitem->engine_torque,
			pitem->engine_retarder_command_mode,
			pitem->engine_retarder_torque,
			pitem->brake_command_mode,
			pitem->ebs_deceleration);
}

/* out of range error values used to signal error to higher level routines */

static profile_item error_item = {
	-255.0,	/* time */
	-255.0, /* engine speed */
	-255.0, /* engine torque */
	255.0, /* retarder torque */
	255, /* engine command mode */
	255, /* retarder command mode */
	255.0, /* deceleration */
	255, /* brake command mode */
};
	
	
/* updates a profile item from the data base; if data base read fails,
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

	if (clt_read(pclt, DB_LONG_OUTPUT_VAR, DB_LONG_OUTPUT_TYPE, &db_data)){
		long_output_typ *p;
		p = (long_output_typ *) db_data.value.user;
		pitem->engine_speed = p->engine_speed;
		pitem->engine_torque = p->engine_torque;
		pitem->engine_command_mode = p->engine_command_mode;
		pitem->engine_retarder_command_mode =
				 p->engine_retarder_command_mode;
		pitem->brake_command_mode = p->brake_command_mode;
		pitem->ebs_deceleration = p->ebs_deceleration;
	}
}

static dbv_size_type dbv_used[] = {
	DB_LONG_OUTPUT_VAR, sizeof(long_output_typ),
};

static int profile_num_dbvs = sizeof(dbv_used)/sizeof(int);

