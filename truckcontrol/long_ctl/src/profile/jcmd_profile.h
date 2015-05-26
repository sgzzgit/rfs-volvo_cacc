/* FILE
 *   jcmd_profile.h
 *
 * Header file for profile of vehicle information.
 *
 * Copyright (c) 2002   Regents of the University of California
 *
 */

#ifndef JCMD_PROFILE_H
#define JCMD_PROFILE_H

typedef struct {
	float time;
        unsigned char tsc_engine_override_control_modes;
        float tsc_engine_speed;     /* RPM */
        float tsc_engine_torque;    /* percent reference torque */
        int tsc_engine_destination_address;       /* engine or retarder */
        int tsc_engine_src_address;    /* sent in header, important for logging */
        unsigned char tsc_eretarder_override_control_modes;
        float tsc_eretarder_torque;    /* percent reference torque */
        int tsc_eretarder_destination_address;       /* engine or retarder */
        int tsc_eretarder_src_address;    /* sent in header, important for logging */
        unsigned char ebs_override_control_mode_priority;
        unsigned char external_deceleration_control_mode;
        float requested_deceleration_to_ebs;
        unsigned char edc_override_control_mode_priority;
        unsigned char edc_override_control_modes;
        float requested_torque_to_edc;
} profile_item;

extern profile_item *read_profile (char *filename, int *pnumitems);
extern int write_profile (char *filename, int numitems, 
		profile_item *profile_array);
extern void read_profile_item (char *linebuffer, profile_item *pitem);
extern void write_profile_item (char *linebuffer, profile_item *pitem); 
extern db_clt_typ *database_init_for_profile (char **argv, int create_dbvs);
extern void update_profile_item (db_clt_typ *pclt, profile_item *pitem,
		struct timeb *start_time);
extern int trig_profile(db_clt_typ *pclt);

#endif /* JCMD_PROFILE_H */
