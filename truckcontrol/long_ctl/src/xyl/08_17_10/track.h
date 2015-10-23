/*	FILE
 *	track.h
 *	Longitudinal tracking structure definition.
 *	To avoid conflicts, the database variable definition
 *	is kept in eng2surf.h.
 *
 */

#ifndef TRACK_H
#define TRACK_H

#define TRACK_PLATOON_SINGLE			0	/*	Standalone operation.	*/
#define TRACK_PLATOON_LEAD				1	/*	Platoon leader.			*/
#define TRACK_PLATOON_FOLLOW			2	/*	Followers can be 2 - 4.	*/
#define TRACK_PLATOON_MAX				4

#define TRACK_INVALID_DIST_SUM			-1.0	/*	Invalid	dist_sum value	*/

typedef struct
{
	int platoon_pos;	/*	0 to 4										*/
	double time;		/*	Timestamp from local vehicle computer.		*/
	float distance;		/*	Distance from this car to preceeding car.	*/
	float velocity;		/*	Longitudinal ground speed.					*/
	float accel;		/*	Longitudinal acceleration.					*/
	float long_usr1;	/*	User defined data, from long_output_typ.	*/
	short unsigned long_usr5;
	short unsigned long_usr6;
} track_data_typ;

typedef struct
{
	track_data_typ leader;		/*	Data from the first car in platoon.		*/
	track_data_typ preceeding;	/*	Data from the next car up in platoon.	*/
	float dist_sum;				/*	Sum of following distances for this,	*/
								/*	and preceeding cars, excluding the		*/
								/*	virtual following distance of the lead	*/
								/*	car.									*/
} track_control_typ;

#endif /* TRACK_H */
