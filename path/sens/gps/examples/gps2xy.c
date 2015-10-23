/**\file 
 *
 * 		Reads lat long pairs from stdin
 *		Writes corresponding XY (Y axis N, X axis E) to stdout
 *		The first point on stdin is used as the origin.
 *
 *  Copyright (c) 2008   Regents of the University of California
 *
 *  The signals SIGINT, SIGQUIT, and SIGTERM are trapped, and cause the
 *  process to terminate. SIGALRM is used for Posix timers on Linux.
 *
 */


#include <sys_os.h>
#include <sys_rt.h>
#include <local.h>
#include <timing.h>
#include <timestamp.h>
#include <db_clt.h>
#include <db_utils.h>
#include "path_gps_lib.h"
#undef DO_TRACE

jmp_buf env;
static void sig_hand(int sig);

static int sig_list[] = {
	SIGINT,
	SIGQUIT,
	SIGTERM,
	SIGALRM,
	ERROR,
};

int main(int argc, char *argv[])
{
	int flags = 0;		/// flags can be set to GPS_ELLIPSOID_EARTH
	int verbose = 0;	/// if 1, print debugging info	
	path_gps_point_t pt;	/// current point 
	path_gps_point_t origin;	///origin
	double xout;		/// current x output
	double yout;		/// current y output
	char buf[80];		/// hold current line of input
	int option;
	while ((option = getopt(argc, argv, "ev")) != EOF) {
		switch (option) {
		case 'e':
			flags |= GPS_ELLIPSOID_EARTH;
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			fprintf(stderr, "Usage %s: \n", argv[0]); 
			fprintf(stderr, " -v  (verbose to stderr) \n ");
			fprintf(stderr, "\n");
			exit(EXIT_FAILURE);
			break;
		}
	}
	if (!fgets(buf, 79, stdin)) {
		perror("fgets failed");
		exit(EXIT_FAILURE);
	} else {
		sscanf(buf, "%lf %lf", &origin.latitude, &origin.longitude);
		if (verbose)
			fprintf(stderr, "Origin is (%lf, %lf)\n",
			origin.latitude, origin.longitude);
	}

	if (setjmp(env) != 0) {
		exit(EXIT_SUCCESS);
	} else
               sig_ign(sig_list, sig_hand);

	memset(buf, 0, 80);

	while (1) {
		if (!fgets(buf, 79, stdin)) {
			perror("fgets failed");
			break;
		}
		if (verbose)
			fprintf(stderr, "%s", buf);
		sscanf(buf, "%lf %lf", &pt.latitude, &pt.longitude);
		path_gps_latlong2xy(&pt, origin, &yout, &xout, 1, flags);
		printf("%7lf %7lf\n", xout, yout);
	}
}

static void sig_hand(int sig)
{
	longjmp(env, 1);
}
