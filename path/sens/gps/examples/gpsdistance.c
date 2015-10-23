/**\file 
 *
 * gpsdistance.c      
 * I		Read GPS data in the format timestamp lat long
 * 		from stdin and writes timestamp distance on stdout. 
 * 		Timestamp is can be any format without spaces.
 *
 *  Copyright (c) 2008   Regents of the University of California
 */


#include <sys_os.h>
#include <sys_rt.h>
#include <local.h>
#include <timestamp.h>
#include "path_gps_lib.h"

int main(int argc, char *argv[])
{
	int verbose = 0;	/// print extra info for debugging	
	path_gps_point_t p1, p2;
	double distance;
	char buf[80];
	char ts[80];		/// timestamp, format not specified 

	memset(&p1, 0, sizeof(p1));
	memset(&p2, 0, sizeof(p2));
	memset(ts, 0, 80);
	while (fgets(buf, 80, stdin)) {
		sscanf(buf, "%s %lf %lf %lf %lf", ts,
			       	&p1.latitude, &p1.longitude, 
			       	&p2.latitude, &p2.longitude); 
		distance = path_gps_get_distance(&p1, &p2, 0, 0.0000001);
		printf("%s %.6f\n", ts, distance);		
	}
}
