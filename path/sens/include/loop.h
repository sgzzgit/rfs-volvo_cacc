/**\file
 *
 * Header file for loop structures wrtten to DB server
 *
 *
 *
 */

//#ifndef _loop_h_
//#define _loop_h_
 
#define DB_LOOP_TYPE 7003
#define DB_LOOP_VAR  7003

#define MAX_LOOPS_PER_LANE 2

typedef struct {
        int laneNum;
	double ts;
        double Inductance[MAX_LOOPS_PER_LANE];
}LOOPS_TYP;
 
