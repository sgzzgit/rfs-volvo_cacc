/**\file
 * Transmits a J1939 stream for testing, reading the format
 * produced used for the files in can/jbus/data/truck 
 * (see README in that directory).
 *
 * When program reaches end of input file, closes the file and reopens
 * at the beginning. 
 *
 * Copyright (c) 2009   Regents of the University of California
 *
 */

#include "std_jbus_extended.h"

static int sig_list[]=
{
        SIGINT,
        SIGQUIT,
        SIGTERM,
        SIGALRM,
        ERROR
};

static jmp_buf exit_env;

static void sig_hand(int code)
{
	if (code == SIGALRM)
		return;
	else 
		longjmp(exit_env, 1);
}

int 
main (int argc, char **argv)
{
	char buffer[80];
	struct j1939_pdu pdu;		/* Protocol Data Unit */
	int n = 0;
	int slot_number;
	int fd;
	FILE *fpin;  
	jbus_func_t jfunc;
	int ch;
	char *devname = "/dev/can1";
	char *in_name = "eng.dat"; 
	double millisecs;
	double start_of_program;
	double start_of_trace;
	double time_to_send;	// since start
	double current_time;
	timestamp_t ts;
	int have_message;
	posix_timer_typ *ptmr;
	int interval = 5; // millisecs; should be less than intervals in file

	/* by default use B&B J1939 STB converter */
	jfunc.send = send_stb;
	jfunc.receive = receive_stb;
	jfunc.init = init_stb;
	jfunc.close = close_stb;
        while ((ch = getopt(argc, argv, "cf:i:t:")) != EOF) {
                switch (ch) {
		case 'c':
			jfunc.send = send_can;
			jfunc.receive = receive_can;
			jfunc.init = init_can;
			jfunc.close = close_can;
			break;
                case 'f': 
			devname = strdup(optarg);
			break;
		case 'i': 
			in_name = strdup(optarg);
			break;
		case 't': 
			interval = atoi(optarg);
			break;
                default: 
			printf( "Usage: %s [-c(CAN, not STB) ", argv[0]);
			printf("-f CAN device name ");
			printf("-i input file name ");
			printf("-t wake up interval ");
			exit(EXIT_FAILURE);
			break;
                }
        }

	fd = (jfunc.init)(devname, O_WRONLY, NULL);
	if (fd ==  -1) {
		perror("init CAN");
		printf(" %s fd %d\n", devname, fd);
		fflush(stdout);
		exit(EXIT_FAILURE);
	}

	if (!(fpin = fopen(in_name, "r"))){   
		perror("fopen in_name\n");
		exit(EXIT_FAILURE);
	}

        /* Initialize the timer. */
        if ((ptmr = timer_init(interval, ChannelCreate(0))) == NULL) {
		printf("timer_init failed\n");
		exit( EXIT_FAILURE );
	}

	if (setjmp(exit_env) != 0) {
                jfunc.close((int *)fd);
		if (fpin != NULL) 
			fclose(fpin);
                exit(EXIT_SUCCESS);
        }
        else
                sig_ign(sig_list, sig_hand);

	start_of_program = 0.0;	// start time initialized first time in loop
	start_of_trace = 0.0;	// start time comes from first line of file
	time_to_send = -1.0;	// initialize so test is true  
	have_message = 0;  	// initialize so no message sent 
	current_time = 0.0;	// time since start of program
	memset(&pdu, 0, sizeof(pdu));
	while (1) {
		get_current_timestamp(&ts);
		if (start_of_program == 0.0)
			start_of_program = TS_TO_MS(&ts);
		current_time = TS_TO_MS(&ts) - start_of_program; //initially 0 
		while (time_to_send < current_time) {
			if (have_message) {	// FALSE only first time
				jfunc.send(fd, &pdu, slot_number);
				memset(&pdu, 0, sizeof(pdu));
			}

			fgets(buffer, 80, fpin);
			if (feof(fpin)){
				fclose(fpin);
				if(!(fpin = fopen(in_name, "r"))){
					perror("fopen in_name\n");
					exit (EXIT_FAILURE);
				} 
			}

			pdu.numbytes = 8; 
			sscanf(buffer,
		 "%lf %u %u %u %u %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu", 
				&millisecs,
				&pdu.priority, 
				&pdu.pdu_format, 
				&pdu.pdu_specific,
				&pdu.src_address,
				&pdu.data_field[0],
				&pdu.data_field[1],
				&pdu.data_field[2],
				&pdu.data_field[3],
				&pdu.data_field[4],
				&pdu.data_field[5],
				&pdu.data_field[6],
				&pdu.data_field[7]);

			if (start_of_trace == 0.0)
				start_of_trace = millisecs;
			time_to_send = millisecs - start_of_trace;

			have_message = 1;
			slot_number = n % 15 + 1; //only used with B&B STB
			n++;
		}
		TIMER_WAIT(ptmr);
	}
	longjmp(exit_env, 2);
}			
