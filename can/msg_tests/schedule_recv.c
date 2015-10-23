#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <sys_os.h>
#include "can_client.h"

typedef struct
{
    int id;
    int frequency;
    int num_received;
} msg_type_recv;

int dummy_recv(int id, uint64_t* msg)
{
    return 0;
}

int main(int argc, char* argv[])
{
    int i = 0;
    struct timespec timeread;
    struct timespec timedelay;
    char* port = "/dev/can1";
    int fd = 0;
    int opt = 0;
    unsigned prev_time = 0; 
    msg_type_recv messages[4];
    unsigned num_msg = sizeof(messages) / sizeof(messages[0]);
    int verbose = 0;

    memset(&messages, 0, sizeof(messages));
    
    // parse the command-line
    while ((opt = getopt(argc, argv, "p:v")) != -1) {
	switch (opt) {
	case 'p':
	    port = strdup(optarg);
	    break;
	case 'v':
	    verbose = 1;
	    break;
	default:
	    printf("Usage: %s -p <port>\n", argv[0]);
	    exit(1);
	}
    }

    // initialize timedelay struct
    timedelay.tv_sec = 0;
    timedelay.tv_nsec = 10000000;   /* 10 millsecs */

    // can can port for read-only
    fd = can_open(port, O_RDONLY);
    if (fd == -1)
    {
	fprintf(stderr, "failed to open can port.");
	exit(1);
    }
    else
    {
	printf("can port opened.");
    }

    // loop until user have had enough and decides to ctrl+C :)
    for (;;)
    {
	unsigned now_ms;
	uint8_t data[8];
	int size;
	unsigned long id;

	// get the time now
	clock_gettime(CLOCK_REALTIME, &timeread);
	now_ms = timeread.tv_sec * 1000 + timeread.tv_nsec / 1000000;

	// loop until no more messages to read
	if ((size = can_read(fd, &id, (char*)NULL, data, 8)) >= 0)
	{
	    int matched = 0;

	    if (verbose)
	    {
		printf ("msg received: id=%d, data=%x,%x,%x,%x,%x,%x,%x,%x\n",
			id, data[0], data[1], data[2], data[3],
			data[4], data[5], data[6], data[7]);
	    }

	    for (i = 0; i < num_msg; i++)
	    {
		if (messages[i].id == 0)
		{
		    messages[i].id = id;
		    if (verbose)
			printf ("setting index %d to id %d\n", i, id);
		}

//		printf ("matching coming id %d with %d\n", id, messages[i].id);

		if (messages[i].id == id)
		{
		    messages[i].num_received++;
		    matched = 1;
		    break;
		}
	    }

	    if (!matched)
		fprintf (stderr, "id %d does not match any existing.\n", id);
	}

	// for each second, print the frequency and reset it
	if (now_ms - prev_time >= 1000)
	{
	    for (i = 0; i < num_msg; i++)
	    {
		printf("id = %d, frequency = %d Hz\n", 
		       messages[i].id, messages[i].num_received);
		messages[i].frequency = messages[i].num_received;
		messages[i].num_received = 0;
	    }

	    prev_time = now_ms;
	}

	
//	nanosleep(&timedelay, NULL);
    }
}
