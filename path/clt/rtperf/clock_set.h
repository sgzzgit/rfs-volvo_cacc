/**\file
 *
 * Header file for clock set and clock skew measurement programs.
 */
#ifndef PATH_CLOCK_SET_H
#define PATH_CLOCK_SET_H

#include "path_msg_type.h"


#define RBUFSIZE	1500	// max size for a single packet
#define UDP_PACKET_SIZE	255	// max size for change channel packets
#define CLOCK_SET_PORT	7015
#define CLOCK_MONITOR_PORT	7016
#define CLOCK_SET_IP "192.168.20.255"

// since timespec values are unsigned, this will return absolute
// value of difference, return value of 1 means negative
static inline int diff_timespec(struct timespec min,
			struct timespec sub, struct timespec *ptm)
{
	 int retval = 0;	// min - sub is positive
	 long int difference;
	
	 difference = min.tv_nsec - sub.tv_nsec;
	 if  (min.tv_sec ==  sub.tv_sec ) { //less than a second's difference	
		ptm->tv_sec = 0;
		if (difference < 0) { 
			retval = 1;	// negative
			ptm->tv_nsec = -difference;	
		} else
			ptm->tv_nsec = difference;
	} else if (min.tv_sec > sub.tv_sec) {	// positive 
		if (difference < 0) {	// must borrow
			ptm->tv_sec = min.tv_sec - sub.tv_sec - 1;
			ptm->tv_nsec = 1000000000 + difference;
		} else {
			ptm->tv_sec = min.tv_sec - sub.tv_sec;
			ptm->tv_nsec = difference;
		}
	} else if (min.tv_sec < sub.tv_sec) { // negative
		retval = 1;	// computer absolute value, sub - min	
		if (difference > 0) {	// must borrow
			ptm->tv_sec = sub.tv_sec - min.tv_sec - 1;	
			ptm->tv_nsec = 1000000000 - difference;
		} else {
			ptm->tv_sec = sub.tv_sec - min.tv_sec;
			ptm->tv_nsec = -difference;
		}
	}
	return retval;
}

// return value in seconds
static inline double timespec2double(struct timespec tm)
{
	return(tm.tv_sec + tm.tv_nsec/1000000000.0);
}

#endif
