/*\file
 * Test sending J1939 formatted messages on the CAN channels
 * Can either send or receive, but not the same instance.
 * For debugging.
 * Does not use DB data server.
 *
 * Copyright (c) 2005-2009   Regents of the University of California
 *
 * Ported to QNX6 in May 2005
 * Updated in July 2009 to use multiple driver interface.
 *
 */

#include "std_jbus_extended.h"

static int sig_list[]=
{
	SIGINT,
	SIGQUIT,
	SIGTERM,
	ERROR
};


static jmp_buf exit_env;


static void sig_hand( int sig)
{
	longjmp( exit_env, 1);
}

int main( int argc, char *argv[] )
{
	int ch;		
	int pgn;		/* Parameter Group Number */
	struct j1939_pdu pdu;		/* Protocol Data Unit */

	posix_timer_typ *ptimer;       /* Timing proxy */
	int interval = 20;
	int slot_number = 1;
	int i;
	int do_send = 0;	/* receives by default */
	int msg_count = 0;
	int fd;			/* "file descriptor" for CAN device */
	char *filename = "/dev/can1";
	int extended = -1;
	int flags = O_RDONLY;


        while ((ch = getopt(argc, argv, "d:f:i:n:p:st:")) != EOF) {
                switch (ch) {
		case 'f': filename = strdup(optarg);
			  break;
		case 'i': interval = atoi(optarg);
			  break;
		case 'n': slot_number = atoi(optarg);
			  break;
                case 'p': pgn = atoi(optarg);
                          break;
		case 's': do_send = 1;
			  flags = O_WRONLY;
			  break;
		case 't': interval = atoi(optarg);
			  break;
		
                default: printf(
			"Usage: %s [-p PGN number -i interval]\n",
					 argv[0]);
			  exit(1);
                          break;
                }
        }
	fprintf(stderr, "request PGN %d (0x%x)\n", pgn, pgn);

	/* Initialize CAN device */
	fd = init_can(filename, flags, NULL);

	if (fd < 0) {
	       printf("Error opening device %s for input\n", filename);
	       exit(EXIT_FAILURE);
	}

	if (do_send) {
		int chid = ChannelCreate(0); 
		if ((ptimer = timer_init(interval, chid )) == NULL) {
			printf("Unable to initialize canj1939 timer\n");
			exit(EXIT_FAILURE);
		}
	}

	if( setjmp( exit_env ) != 0 ) {
		printf("canj1939 exits, %s %d\n", 
			do_send?"sent":"received", msg_count);
		close_can(&fd);
		exit( EXIT_SUCCESS );
	} else
		sig_ign( sig_list, sig_hand );


	if (do_send) {
		/* assemble request from arguments */
		pdu.priority = 6;
		pdu.R = 0;
		pdu.DP = 0;
		pdu.pdu_format = HIBYTE(RQST);
		pdu.pdu_specific = GLOBAL;
		pdu.src_address = 249;	/* Service Tool address */
		pdu.numbytes = 3;
		/* current PGNs are only two bytes, but field is 3 bytes,
		 LSB first */
		pdu.data_field[0] = LOBYTE(pgn);
		pdu.data_field[1] = HIBYTE(pgn);
		pdu.data_field[2] = 0;
		for (i = 3; i < 8; i++)
			pdu.data_field[i] = 0xff;
	}
	
	/* state code and slot number not used */
	if (do_send) {
		for ( ; ; ) {
			send_can(fd, &pdu, slot_number);
			TIMER_WAIT( ptimer );
			msg_count++;
		}
	}
	else {
		for ( ; ; ) {
			if (!receive_can(fd, &pdu, &extended, &slot_number))
				printf("call to receive_can returned error\n");
			else {
				printf("Pr %d PF %d PS %d", pdu.priority, 
					pdu.pdu_format, pdu.pdu_specific);
				printf("SA %d: ", pdu.src_address);
				for (i = 0; i < 8; i++)
					printf("%d ", pdu.data_field[i]);
				printf(" extended %d\n", extended);
				msg_count++;
			}
		}
	}
}
