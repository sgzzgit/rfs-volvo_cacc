/* Simulates a sensor sending to the control processor */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <sys_os.h>
#include "can_client.h"
#include "msg_descriptors.h"
#include "msg_packer.h"

typedef struct
{
    int id;
    uint64_t data; // the actual data to send
    size_t count;  // how many times to send at each interval
    int frequency; /* hertz */
    int period;    /* milliseconds */
    int last_sent;  /* milliseconds */
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
    struct timespec timedelay;
    int fd = -1;
    char* port = "/dev/can1";
    uint8_t extended = 0;
    int nmax = 0;
    int opt = 0;
    int verbose = 0;

    msg_type messages[5]; // 6 types of messages to be sent
    int num_msg = sizeof(messages) / sizeof(messages[0]);

    // initialize the messages
    memset(messages, 0, sizeof(messages));
    messages[0].id = 6;
    messages[0].frequency = 20;
    messages[1].id = 7;
    messages[1].frequency = 20;
    messages[2].id = 8;
    messages[2].frequency = 10;

    for (i = 0; i < num_msg; i++)
    {
	messages[i].count = 1;  // have them all send only 1 for now
	messages[i].period = 1.0 / messages[i].frequency;
    }

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
    
    fd = can_open(port, O_WRONLY);
    if (fd == -1 || fd == 0)
    {
	fprintf (stderr, "error opening %s\n", port);
	exit(EXIT_FAILURE);
    }
    printf ("can port opened. fd = %d\n", fd);

    // keep on sending data until someone decides to press ctrl+C
    for (;;) 
    {
	unsigned now_ms;
	struct timespec timeread;

	clock_gettime(CLOCK_REALTIME, &timeread);
	now_ms = timeread.tv_sec * 1000 + timeread.tv_nsec / 1000000;

	for (i = 0; i < num_msg; ++i)
	{
	    // see how long it has been since that message was last sent
	    unsigned long diff = now_ms - messages[i].last_sent;
	    int j = 0;

	    // if not longer than the period, move on
	    if (diff < messages[i].period)
		continue;

	    for (j = 0; j < messages[i].count; j++)
	    {
		can_write(fd, messages[i].id, 0, &messages[i].data, 8);
		messages[i].data++;
	    }

	    messages[i].last_sent = now_ms;
	}

	// sleep for 10 ms to avoid using all the cpu
//	nanosleep(&timedelay, NULL);
    }
    
    return 0;
}

