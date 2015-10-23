/**\file
 *
 * can_rx2.c
 *
 * Listens and computes reception statistics for arrivals on the CAN port..
 *
 *	Usage for SSV_CAN or ECAN cards (i82527): 
 *		can_rx2  -p /dev/can1
 #
 *	Usage for Steinhoff driver with SJA1000 chips:
 *		can_rx2  -p 1 
 *
 * Links against different libraries for different CAN cards;
 * code is symbolically linked into different directories with
 * CAN driver specific Makefiles.
*/

#include <sys_os.h>
#include <sys/neutrino.h>
#include "local.h"
#include "sys_rt.h"
#include "timestamp.h"
#include "can_defs.h"
#include "can_client.h"

static int sig_list[]=
{
        SIGINT,
        SIGQUIT,
        SIGTERM,
        SIGALRM,        // for timer
        ERROR
};                                                                
static jmp_buf exit_env;                                          
static void sig_hand(int code)                                  
{                                                                 
        if (code == SIGALRM)                                      
                return;                                           
        else                                                      
                longjmp(exit_env, code);                        
}                                                    

int main(int argc, char **argv)
{
    int size;
    int fd;
    unsigned long id;  
    unsigned char data[8];
    char *port = "/dev/can1";	// use "1" to "4" for steinhoff
    int opt;
    int status;
    
    int filter = 0;
    double timediff_threshold = 0.0;
    int initiated = 0;
    timestamp_t prevtime;
    unsigned int prevmsg = 0;
    int num_received = 0;
    
    double mean_timediff = 0.0;
    double M2_timediff = 0.0;
    double variance_timediff = 0.0;
    double mean_dropped = 0.0;
    double M2_dropped = 0.0;
    double variance_dropped = 0.0;
    double total_timediff = 0.0;
    double total_square_timediff = 0.0;
    double total_dropped = 0.0;
    double total_square_dropped = 0.0;
    
    while ((opt = getopt(argc, argv, "p:")) != -1) {
	switch (opt) {
	case 'p':
	    port = strdup(optarg);
	    break;
	case 'f':
	    timediff_threshold = atof(optarg);
	    break;
	default:
	    printf("Usage: %s -p <port>\n", argv[0]);
	    exit(1);
	}
    }
    
    printf("can_rx2: trying to open %s\n", port); 
    fflush(stdout);
    
    fd = can_open(port, O_RDONLY);
    
    if (fd == -1)
	exit(EXIT_FAILURE);	// error message printed by can_open 
    
    printf("program %s, device name %s, fd: %d\n", argv[0], port, fd);
    fflush(stdout);
    
    /** Not yet implemented for PATH CAN drivers, only Steinhoff
      */
    (void) can_print_config(stdout, fd);
    fflush(stdout);
    (void) can_print_status(stdout, fd);
    fflush(stdout);
    
    if(setjmp(exit_env) != 0) {
	printf("num of data points: %d\n",
	       num_received);

	printf("statistics using incremental calculations:\n");
	printf("\tdropped: mean=%f\tvariance=%f\n",
	       mean_dropped, variance_dropped);
	printf("\ttimediff (seconds): mean=%f\tvariance=%.12f\n",
	       mean_timediff, variance_timediff);

	printf("statistics using totals for run:\n");
	mean_dropped = total_dropped/num_received;
	variance_dropped = total_square_dropped/num_received -
				(mean_dropped * mean_dropped);
	mean_timediff = total_timediff/num_received;
	variance_timediff = total_square_timediff/num_received -
				(mean_timediff * mean_timediff);
	printf("\tdropped: mean=%f\tvariance=%f\n",
	       mean_dropped, variance_dropped);
	printf("\ttimediff (seconds): mean=%f\tstd_dev=%.8f\n",
	       mean_timediff, sqrt(variance_timediff));
	
	
	if (fd != -1)
	    status = can_close(&fd);
	if (status != -1)			
	    exit(EXIT_SUCCESS);
	else
	    exit(EXIT_FAILURE);	// can_close prints error info
    } else
	sig_ign(sig_list, sig_hand);
    
    
    for(;;) {
	timestamp_t ts;
	id = 0;
	size = can_read(fd,&id,(char *)NULL,data,8);

	if (size < 0) 
	    perror("can_read");
	else {
	    get_current_timestamp(&ts);	    
	    
	    // if not initiated, set prevtime as ts
	    // else, continue as normal
	    if (!initiated)
	    {
		initiated = 1;
		prevtime = ts;
		prevmsg = *((unsigned*)data);
	    }
	    else
	    {
		double timediff =
		    (ts.hour - prevtime.hour) * 3600 +
		    (ts.min - prevtime.min) * 60 +
		    (ts.sec - prevtime.sec) +
		    (ts.millisec - prevtime.millisec) * 0.001;
		double timediff_delta = timediff - mean_timediff;
		int dropped = *((unsigned*)data) - prevmsg - 1;
		double dropped_delta = dropped - mean_dropped;
		
		double inverse_numreceived;

		num_received++;

		// Start computing difference and dropped totals
		// on second message 
		if (num_received > 0) {
		    total_timediff += timediff;
		    total_square_timediff += (timediff * timediff);
		    total_dropped += dropped;
		    total_square_dropped += (1.0 * dropped * dropped);
		}

		inverse_numreceived = 1.0 / num_received;

		prevtime = ts;
		prevmsg = *((unsigned*)data);

		// filters
		if (filter && timediff > timediff_threshold)
		    continue;
		// TODO: implement a simple threshold filter for dropped

		// compute the statistics
		mean_timediff += timediff_delta * inverse_numreceived;
		M2_timediff += timediff_delta * (timediff - mean_timediff);
		variance_timediff = M2_timediff * inverse_numreceived;
		
		mean_dropped += dropped_delta * inverse_numreceived;
		M2_dropped += dropped_delta * (dropped - mean_dropped);
		variance_dropped = M2_dropped * inverse_numreceived;
	    }
	}
    }
    
}

