/* FILE
 *   long_out_profile.h
 *
 * Header file for longitudinal output profile 
 *
 * Copyright (c) 2002   Regents of the University of California
 *
 */

#ifndef LONG_OUT_PROFILE_H
#define LONG_OUT_PROFILE_H

typedef struct
{
	float time;
        float engine_speed;
        float engine_torque;
        float engine_retarder_torque;
        unsigned char engine_command_mode;   /* 0=disable, 1=speed, 2=torque,
                                              * 3=speed/torque limit */
        unsigned char engine_retarder_command_mode;  /* 0=disable, 2=torque,
                                                      * 3=torque limit */
        float ebs_deceleration;
        unsigned char brake_command_mode;    /* 0=not active, 1=active */
} profile_item;

extern profile_item *read_profile (char *filename, int *pnumitems);
extern int write_profile (char *filename, int numitems, 
		profile_item *profile_array);
extern void read_profile_item (char *linebuffer, profile_item *pitem);
extern void write_profile_item (char *linebuffer, profile_item *pitem); 
extern db_clt_typ *database_init_for_profile (char **argv, int create_dbvs);
extern void update_profile_item (db_clt_typ *pclt, profile_item *pitem,
		struct timeb *start_time);
extern int trig_profile (db_clt_typ *pclt);

#endif /* LONG_OUT_PROFILE_H */
