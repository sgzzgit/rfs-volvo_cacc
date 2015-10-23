/* Simulates a sensor receiving from the control processor */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <sys_os.h>
#include "can_client.h"
//#include "msg_descriptors.h"
//#include "msg_packer.h"
#include "db_clt.h"
#include "db_utils.h"
#include "vaa_msg.h"
#include "vaa_clt_vars.h"


typedef struct
{
    int id;
    uint64_t prev_data; /* the data last received */
    int frequency; /* hertz */
    int period;    /* milliseconds */
    int last_recv;  /* milliseconds */
    int num_received;
    int num_dropped;
} msg_type;

void print_data(uint64_t n)
{
    uint8_t* p = (uint8_t*)&n;
    printf("%.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X",
	   p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
}

int main(int argc, char* argv[])
{
    int i = 0;
    struct timespec timeread;
    struct timespec timedelay;
    unsigned last_update = 0;
    int fd = -1;
    char* port = "/dev/can1";
    uint8_t extended = 0;
    int nmax = 0;
    int opt = 0;
    int verbose = 0;

    /*
    db_clt_typ *pclt;
    char hostname[MAXHOSTNAMELEN+1];
    char *domain = DEFAULT_SERVICE;
    int xport = COMM_OS_XPORT;
    */

    /* initialize the incoming_msg */
    msg_type incoming_msg[3];
    size_t num_msg = sizeof(incoming_msg) / sizeof(incoming_msg[0]);
    memset(incoming_msg, 0, sizeof(incoming_msg));

    incoming_msg[0].id = 6;
    incoming_msg[1].id = 7;
    incoming_msg[2].id = 8;

    // parse the command-line args
    while ((opt = getopt(argc, argv, "e:n:p:v:z")) != -1)
    {
	printf ("opt = %d\n", opt);
	switch (opt)
	{
	case 'e':
	    extended = 1;
	    break;
	case 'n':
	    nmax = atoi(optarg);
	    break;
	case 'p':
	    port = strdup(optarg);
	    break;
	case 'v':
	    verbose = 1;
	    break;
	}
    }

    // initialize the timedelay structure
    timedelay.tv_sec = 0;
    timedelay.tv_nsec = 10000000;   /* 10 millsecs */

    // open can port
    printf("sensor_simulator: trying to open %s\n", port); 
    fflush(stdout);
    
    fd = can_open(port, O_RDONLY);    
    if (fd == -1 || fd == 0)
    {
	fprintf (stderr, "error opening %s\n", port);
	exit(EXIT_FAILURE);
    }
    printf ("can port opened. fd = %d\n", fd);

    // open database
    /*
    get_local_name(hostname, MAXHOSTNAMELEN);
    if ((pclt = db_list_init(argv[0], hostname, domain, xport,
			     NULL, 0, NULL, 0)) == NULL)
    {
	printf("Database initialization error in %s.\n", argv[0]);
	exit(EXIT_FAILURE);
    }
    */

    // keep on sending data until someone decides to press ctrl+C
    for (;;) 
    {
	unsigned now_ms;
	uint8_t data[8];
	int size;
	unsigned long id;

	clock_gettime(CLOCK_REALTIME, &timeread);
	now_ms = timeread.tv_sec * 1000 + timeread.tv_nsec / 1000000;

	size = can_read(fd, &id, (char*)NULL, data, 8);

//	size = rcv_g(pclt, fd, (void*)&vaa_msg_funcs);
	
	if (size >= 0)
	{
	    uint64_t msg = *((uint64_t*)data);

	    if (verbose)
	    {
		printf ("msg received: id=%d, data=%x,%x,%x,%x,%x,%x,%x,%x\n",
			id, data[0], data[1], data[2], data[3],
			data[4], data[5], data[6], data[7]);
	    }

	    // be generic for now and loop to find the correct id
	    for (i = 0; i < num_msg; i++)
	    {
		if (id == incoming_msg[i].id)
		{
		    incoming_msg[i].num_received++;
		    incoming_msg[i].num_dropped += msg - incoming_msg[i].prev_data - 1;
		    incoming_msg[i].prev_data = msg;
		    incoming_msg[i].last_recv = now_ms;
		    break;
		}
	    }

	    if (i == num_msg)
	    {
		fprintf(stderr, "unrecognized id: %d\n", id);
	    }
	}
	
	// print some stats every second
	if (now_ms - last_update >= 1000)
	{
	    last_update = now_ms;

	    for (i = 0; i < num_msg; i++)
	    {
		printf("id = %d, frequency = %d\n",
		       incoming_msg[i].id, incoming_msg[i].num_received);
		incoming_msg[i].num_received = 0;
	    }
	}

	// sleep for 10 ms to avoid using all the cpu
//	nanosleep(&timedelay, NULL);
    }
    
    return 0;
}

