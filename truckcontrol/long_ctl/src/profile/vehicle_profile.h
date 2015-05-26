/* FILE
 *	vehicle_profile.h
 *
 * Header file for profile of vehicle information.
 *
 * Copyright (c) 2002   Regents of the University of California
 */

#ifndef VEHICLE_PROFILE_H
#define VEHICLE_PROFILE_H

typedef struct {
        float time;             /*   sec    */
        float velocity;         /*   m/s   */
        float engine_speed;     /*   rpm   */
        float engine_torque;    /*   N-m   */
        float retarder_torque;  /*   N-m   */
        float fuel_rate;        /*    %    */
        float deceleration;     /*   m/s^2 */
	float brtsc1_time;	/*   sec, when last sent */
	unsigned char brtsc1_mode;	
	float brtsc1_torque;
} profile_item;

extern profile_item *read_profile (char *filename, int *pnumitems);
extern int write_profile (char *filename, int numitems, 
		profile_item *profile_array);
extern void read_profile_item (char *linebuffer, profile_item *pitem);
extern void write_profile_item (char *linebuffer, profile_item *pitem); 
extern db_clt_typ *database_init_for_profile (char **argv, int create_dbvs);
extern void update_profile_item (db_clt_typ *pclt, profile_item *pitem,
		timestamp_t *start_time);
extern int trig_profile(db_clt_typ *pclt);
extern void copy_to_command (profile_item *pitem, long_output_typ *pcmd, 
		int engine_mode); 

#endif /* VEHICLE_PROFILE_H */
