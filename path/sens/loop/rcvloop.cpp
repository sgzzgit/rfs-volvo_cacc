/**\file
 *
 *              Later create a J2735 structure.
 *
 *              Records local timestamp when received as well as
 *              UTC time and local timestamp when sent from the message.
 *
 *  Copyright (c) 2008   Regents of the University of California
 *
 *  The signals SIGINT, SIGQUIT, and SIGTERM are trapped, and cause the
 *  process to terminate. SIGALRM is used for Posix timers on Linux.
 *
 */


#include <sys_os.h>

extern "C"{
        #include <sys_rt.h>
        #include <local.h>
        #include <db_clt.h>
        #include <timestamp.h>
        #include <timing.h>
        #include <udp_utils.h>
        #include <data_log.h>
        #include "path_gps_lib.h"
}
#include "vloop.h"

jmp_buf env;
static void sig_hand(int sig);

static int sig_list[] = {
        SIGINT,
        SIGQUIT,
        SIGTERM,
        SIGALRM,
        ERROR,
};


virtual_loops_typ vloop_typ;
db_clt_typ *pclt;
int var = 7001;

int main(int argc, char *argv[])
{
        int status;

        char *domain = DEFAULT_SERVICE;
        char hostname[MAXHOSTNAMELEN];
        int stay_alive = FALSE;
        int xport = COMM_PSX_XPORT;

        FILE *fpout;            /// file pointer to write output
        int old_fileday = 99;   /// invalid day value for initialiation
        int file_serialno = 0;  /// start numbering files at 0
        char suffix[80];        /// file extension for data logging
        char prefix[80];        /// identifier for files logged
        double start_time;      /// when last file was opened
        int file_time = 5;      /// minutes file duration

        int verbose = 0;        /// if 1, print extra info for debugging
        int udp_port = 7015;    /// port for receiving heartbeat
        int option;

        virtual_loops_typ vloop_typ;
        //path_gps_point_t hb;  /// fill in from GPS messages received
        int sd_in;              /// socket descriptor for UDP send
        int bytes_rcvd;         /// returned from recvfrom
        struct sockaddr_in src_addr;    /// used in recvfrom call
        unsigned int socklen;

        strncpy(suffix, ".dat", 80);
        strncpy(prefix, "/big/data/", 80);
        /* Read and interpret any user switches. */
        /* Read and interpret any user switches. */
        while ((option = getopt(argc, argv, "f:vu:")) != EOF) {
                switch(option) {
                case 'f':
                        strncpy(prefix, optarg, 80);
                        break;
                case 'v':
                        verbose = 1;    //
                        break;
                case 'u':
                        udp_port = atoi(optarg);
                        break;
                default:
                        fprintf(stderr, "Usage %s: ", argv[0]);
                        fprintf(stderr, " -f <prefix>");
                        fprintf(stderr, " -v  (verbose) ");
                        fprintf(stderr, " -u  (UDP port number for input) ");
                        fprintf(stderr, "\n");
                        exit(EXIT_FAILURE);
                }
        }
        printf("PATH virtual loop receive app, files output to %s\n", prefix);
        fflush(stdout);

        //Init database
        get_local_name(hostname,MAXHOSTNAMELEN+1);
        if ((pclt = clt_login(argv[0],hostname,domain,xport))==NULL)
	{
		printf("clt login %s %s %s %d failed\n ",argv[0],hostname,domain,xport);
		exit(EXIT_FAILURE);
	}

	printf("clt_create,pclt\n");
	if (!clt_create(pclt,var,var,sizeof(virtual_loops_typ)))
	{
		printf("clt create failed\n");
		clt_logout(pclt);
		exit(EXIT_FAILURE);
	}

        if (setjmp(env) != 0) 
        {
                if (pclt!=NULL)
			clt_destroy(pclt,var,var);
		clt_logout(pclt);  
                exit(EXIT_SUCCESS);
        }else
	{
                sig_ign(sig_list, sig_hand);
	}	



        sd_in = udp_allow_all(udp_port);
        if (sd_in < 0) {
                printf("failure opening socket on %d\n", udp_port);
                exit(EXIT_FAILURE);
        }

        if (!open_data_log(&fpout, prefix, &start_time,
                                 &old_fileday, &file_serialno, suffix)){
                         printf("Error opening %s%d%s\n",
                                 prefix, file_serialno, suffix);
                         exit (EXIT_FAILURE);
        }

        socklen = sizeof(src_addr);
        memset(&src_addr, 0, socklen);
        while (1) {
                timestamp_t ts;
                bytes_rcvd = recvfrom(sd_in, &vloop_typ, sizeof(vloop_typ), 0, (struct sockaddr *) &src_addr, &socklen);
                         //       (struct sockaddr *) &src_addr, &socklen);
                if (bytes_rcvd < 0) {
                         printf("test\n");
		}
                if (!clt_update(pclt,var,var,sizeof(virtual_loops_typ),&vloop_typ))
		{
			printf("clt update failed\n");
		}

                printf("%d\n",bytes_rcvd);
                /// use source IP to identify source of message in output
                fprintf(fpout, " 0x%08x ", src_addr.sin_addr.s_addr);
                //      path_gps_print_point(fpout, &vloop_typ);
                fprintf(fpout, "\n");

                /* if (verbose) {
                        fprintf(stdout, " 0x%08x ", src_addr.sin_addr.s_addr);
                        path_gps_print_point(stdout, &hb);
                        fprintf(stdout, "\n");
                }
*/
                reopen_data_log(&fpout, file_time,
                                        prefix,
                                        &start_time, &old_fileday,
                                        &file_serialno, suffix);
        }
}

static void sig_hand(int sig)
{
        longjmp(env, 1);
}

