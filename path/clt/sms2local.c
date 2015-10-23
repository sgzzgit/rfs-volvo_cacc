/*\file
 *	
 *	Sample program for testing 2d transform function.
 *	Reads old (x,y) coordinates on stdin, writes new coordinates
 *	on stdout.
 *
 *	By default, converts SMS radar (see path/sens/sms) (X,Y) coordinates
 *	to the local coordinate system John Murray defined for CICAS Zone of
 *	Conflict (see cicas/zoc). 
 *
 */ 

#include <sys_os.h>
#include "xytransform.h"

/** Signal handling boiler plate for clean exits.
 */
static int sig_list[]=
{
        SIGINT,
        SIGQUIT,
        SIGTERM,
        SIGALRM,        // for timer
        ERROR
};
static jmp_buf exit_env;
static void sig_hand( int code )
{
        if (code == SIGALRM)
                return;
        else
                longjmp( exit_env, code );
}

/** Usage information.
 *	Argument parsing is not robust to badly formatted numeical input.
 */
void do_usage(char *progname)
{
	fprintf(stderr, "Usage %s:\n", progname);
	fprintf(stderr, "-x old x coordinate of new origin ");
	fprintf(stderr, "-y old y coordinate of new origin ");
	fprintf(stderr, "-a angle from old-x axis to new (degrees) ");
	fprintf(stderr, "-s scale factor");
	fprintf(stderr, "-v (verbose, for debugging) ");
	fprintf(stderr, "\n");
	exit(1);
}

int main (int argc, char **argv)
{
	int option;		/// for getopt
	int verbose = 0;	/// echo formatted output to stdout
	double angle = 203.0;	/// angle between x-axes, degrees
	double x0 = -120.0;	/// old x-coordinate of new origin
	double y0 = 65.0;	/// old y-coordinate of new origin
	double scale = 1.0;	/// scale factor
	double x, y;		/// inputs
	double newx, newy;	/// outputs
	char buf[80];		/// to read input

        while ((option = getopt(argc, argv, "a:s:x:y:v")) != EOF) {
                switch(option) {
                        case 'a':
				sscanf(optarg, "%lf", &angle);
				break;
                        case 's':
				sscanf(optarg, "%lf", &scale);
                                break;
                        case 'x':
				sscanf(optarg, "%lf", &x0);
                                break;
                        case 'y':
				sscanf(optarg, "%lf", &y0);
                                break;
                        case 'v':
                                verbose = 1;
				break;
                        default:
				do_usage(argv[0]);
                                break;
                }
        }
	memset(buf, 0, 80);
        while (1) {
                if (!fgets(buf, 79, stdin)) {
                        perror("fgets failed");
                        break;
                }
                if (verbose)
                        fprintf(stderr, "%s", buf);
                sscanf(buf, "%lf %lf", &x, &y);
                xy2dtransform(x0, y0, angle, scale, &x, &y, &newx, &newy, 1);
                printf("%7lf %7lf\n", newx, newy);
	}
	exit(EXIT_FAILURE);
}
