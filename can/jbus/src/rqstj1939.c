/**\file
 * Requests a message of type specified by Parameter Group Number
 *		every t milliseconds, using slot n of the converter. 
 *
 * Copyright (c) 2005   Regents of the University of California
 *
 * Ported to QNX6 May 2005.
 *
 */

#define INTERVAL_MSECS	500

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
	int interval = INTERVAL_MSECS;
	int slot_number = 1;
	int i;
	jbus_func_t jfunc;
	int channel_id;

	/* by default use B&B J1939 STB converter */
	jfunc.send = send_stb;
	jfunc.receive = receive_stb;

        while ((ch = getopt(argc, argv, "cp:n:t:")) != EOF) {
                switch (ch) {
		case 'c': jfunc.send = send_can;
			  jfunc.receive = receive_can;
			  break;
                case 'p': pgn = atoi(optarg);
                          break;
		case 't': interval = atoi(optarg);
			  break;
		case 'n': slot_number = atoi(optarg);
			  break;
                default: printf(
			"Usage: %s [-c <use CAN driver> -p PGN number -i interval]\n",
					 argv[0]);
			  exit(1);
                          break;
                }
        }
	fprintf(stderr, "request PGN %d (0x%x)\n", pgn, pgn);

	channel_id = ChannelCreate(0);
	if ((ptimer = timer_init( interval, channel_id )) == NULL) {
		printf("Unable to initialize wrj1939 timer\n");
		exit(EXIT_FAILURE);
	}

	if( setjmp( exit_env ) != 0 ) {
		exit( EXIT_SUCCESS );
	}
	else
		sig_ign( sig_list, sig_hand );


	/* assemble request from arguments */
	pdu.priority = 6;
	pdu.R = 0;
	pdu.DP = 0;
	pdu.pdu_format = HIBYTE(RQST);
	pdu.pdu_specific = GLOBAL;
	pdu.src_address = 249;	/* Service Tool address */
	pdu.numbytes = 3;
	/* current PGNs are only two bytes, but field is 3 bytes, LSB first */
	pdu.data_field[0] = LOBYTE(pgn);
	pdu.data_field[1] = HIBYTE(pgn);
	pdu.data_field[2] = 0;
	for (i = 3; i < 8; i++)
		pdu.data_field[i] = 0xff;
	
	for ( ; ; ) {
		jfunc.send(STDOUT_FILENO, &pdu, slot_number);

	        /* Now wait for proxy from timer (waits "interval" msec). */
		TIMER_WAIT( ptimer );
	}
}
