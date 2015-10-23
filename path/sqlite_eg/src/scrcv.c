/**\file 
 *
 * scrcv.c      Reads signal countdown message with TSP information
 *		and writes to database.
 *		
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
#include <timestamp.h>
#include <timing.h>
#include <udp_utils.h>
#include <sqlite3.h>
#include "rcv_tsp_msg.h"
#include "path_gps_lib.h"

jmp_buf env;
static void sig_hand(int sig);

static int sig_list[] = {
	SIGINT,
	SIGQUIT,
	SIGTERM,
	SIGALRM,
	ERROR,
};

/**
 *      callback function, used with query returning statements
 *      to handle results; no results from queries yet in this
 *      program
 */
static int callback(void *not_used, int argc, char **argv, char **az_col){
        return 0;
}

/** 
 *	Convenience wrapper for sqlite3_exec that does error checking.
 */
static void sqlt3_ex(sqlite3 *sqlt, char *cmd_str)
{
        char *zerr = NULL;
        if (sqlite3_exec(sqlt, cmd_str, callback, 0, &zerr) != SQLITE_OK) {
                        fprintf(stderr, "sqlite3 error: %s\n", zerr);
                        sqlite3_free(zerr);
        }
}

/**
 *      Copies characters from current location until before the next
 *      comma (or *) from 'sentence' into 'field_buf', null-terminating
 *      the string.
 *
 *      Returns index in sentence of character after first comma found.
 *
 *      field_buf[0] will be 0 if no characters between commas.
 */

int get_field(int start, char *sentence, char *field_buf, int max)
{
        int i = 0;
        while ((sentence[start+i] != ',') &&
                        (sentence[start+i] != '*') && i < max) {
                field_buf[i] = sentence[start+i];
                i++;
        }
        field_buf[i] = '\0';
        return start+i+1;
}

#define MAX_FIELD_LENGTH 20

/* packet format (11 commas):
 * $TSPMSG,yyyy-mm-dd,hh:mm:ss,ms,intersection_id,signal_state,time_to_next,
 * reqst_bus_id,reqst_type,reqst_phase,bus_time_saved,canPassOrNot\n
 */
int parse_tsp_msg(char* instr, tsp_msg_typ *pp)
{	
	int len = (int)(strlen(instr));
	int index = 0;
	char buf[MAX_FIELD_LENGTH];

	// validate the packet
	index = get_field(index, instr, buf, MAX_FIELD_LENGTH); 
	if (strncmp(buf, "$TSPMSG", MAX_FIELD_LENGTH) != 0)
		return (-1);
	index = get_field(index, instr, buf, MAX_FIELD_LENGTH);
	if (buf[0])
		strncpy(pp->rcv_date_str, buf, 12);
	index = get_field(index, instr, buf, MAX_FIELD_LENGTH);
	if (buf[0])
		strncpy(pp->rcv_time_str, buf, 10);
	index = get_field(index, instr, buf, MAX_FIELD_LENGTH);
	pp->rcv_ms =  buf[0]? atoi(buf):0;	 
	index = get_field(index, instr, buf, MAX_FIELD_LENGTH);
	pp->intersection_id = buf[0] ? atoi(buf) : 0;
	index = get_field(index, instr, buf, MAX_FIELD_LENGTH);
	pp->signal_state = buf[0] ? atoi(buf) : 0;
	index = get_field(index, instr, buf, MAX_FIELD_LENGTH);
	pp->time_to_next = buf[0] ? atoi(buf) : 0;
	index = get_field(index, instr, buf, MAX_FIELD_LENGTH);
	pp->reqst_bus_id = buf[0] ? atoi(buf) : 0;
	index = get_field(index, instr, buf, MAX_FIELD_LENGTH);
	pp->reqst_type = buf[0] ? atoi(buf) : 0;
	index = get_field(index, instr, buf, MAX_FIELD_LENGTH);
	pp->reqst_phase = buf[0] ? atoi(buf) : 0;
	index = get_field(index, instr, buf, MAX_FIELD_LENGTH);
	pp->bus_time_saved = buf[0] ? atoi(buf) : 0;
	index = get_field(index, instr, buf, MAX_FIELD_LENGTH);
	pp->canPassOrNot = buf[0] ? atoi(buf) : 0;
	return (TRUE);
}

/**
 *	Creates update string for TSP message in sqlite table	
 *	Initial table must have been loaded with line with link ID 
 */
void tsp_msg_sql_update_str(char *cmd_str, tsp_msg_typ *pm, int link_id)
{
	char tmp_str[MAX_CMD_STR];
	int i;
	
	snprintf(tmp_str, MAX_CMD_STR, 
	"phase=%d,countdown=%d,tsptype=%d,bustimesaved=%d,canPassOrNot=%d",
			pm->signal_state,
			pm->time_to_next,
			pm->reqst_type,
			pm->bus_time_saved,		
			pm->canPassOrNot);
	snprintf(cmd_str, MAX_CMD_STR, "update s set %s where LinkID = %d",
		tmp_str, link_id);
}

#define BUFFER_SIZE 200

int main(int argc, char *argv[])
{
	int status;
	
	int verbose = 0;	/// if 1, print extra info for debugging	
	int do_timing = 0;	/// if 1, print users and sys time at end 
	struct avcs_timing timing;	
	int udp_port = 7015;	/// port for receiving heartbeat
	int option;

	char buf[BUFFER_SIZE];	/// fill in from TSP messages received
	tsp_msg_typ tsp_msg;	/// parse TSP message into structure
	int sd_in;		/// socket descriptor for UDP receive
	int bytes_rcvd;		/// returned from recvfrom
	struct sockaddr_in src_addr;	/// used in recvfrom call
	unsigned int socklen;
        sqlite3 *sqlt;
        char *sqlt_name = "sc.db";/// default name for sqlite database
	char cmd_str[MAX_CMD_STR];
	int link_id = 50007;	/// used to identify line to update
	int modulo = 5;		/// by default print only one time in 5
	int counter = 0;	/// count up to modulo, then reset to 0


	/* Read and interpret any user switches. */
	while ((option = getopt(argc, argv, "d:i:m:vtu:")) != EOF) {
		switch(option) {
		case 'd':
			sqlt_name = strdup(optarg);	
			break;
		case 'i':
			link_id = atoi(optarg);	
			break;
		case 'm':
			modulo = atoi(optarg);	
			break;
		case 't':
			do_timing = 1;	// call timing utilities	
			break;
		case 'v':
			verbose = 1;	//
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

	sd_in = udp_allow_all(udp_port);
	if (sd_in < 0) {
		printf("failure opening socket on %d\n", udp_port);
		exit(EXIT_FAILURE);
	}

        if (sqlite3_open(sqlt_name, &sqlt) != 0) {
                fprintf(stderr, "Can't open %s\n", sqlite3_errmsg(sqlt));
                sqlite3_close(sqlt);
                exit(EXIT_FAILURE);
        }

	if (setjmp(env) != 0) {
		if (do_timing) {
			printf("Exiting %s\n", argv[0]);
			fflush(stdout);
			avcs_end_timing(&timing);
			avcs_print_timing(stdout, &timing);
			fflush(stdout);
		}
		exit(EXIT_SUCCESS);
	} else
               sig_ign(sig_list, sig_hand);

	socklen = sizeof(src_addr);
	memset(&src_addr, 0, socklen);

	if (do_timing) 
		avcs_start_timing (&timing);
	while (1) {
		timestamp_t ts;
		timing.exec_num++;
		bytes_rcvd = recvfrom(sd_in, &buf, sizeof(buf), 0,
				(struct sockaddr *) &src_addr, &socklen);
		if (bytes_rcvd < 0) {
			perror("recvfrom failed\n");
			continue;
		}

		if (verbose) {
			fprintf(stdout, "%s\n", buf);
			fprintf(stdout, "\n");
		}
		parse_tsp_msg(buf, &tsp_msg);
		if (verbose) {
			printf("%s %s %d %d %d %d %d %d\n",
				tsp_msg.rcv_date_str,
				tsp_msg.rcv_time_str,
				tsp_msg.rcv_ms,
				tsp_msg.reqst_type,
				tsp_msg.signal_state,
				tsp_msg.time_to_next,
				tsp_msg.bus_time_saved,
				tsp_msg.canPassOrNot);
		}
		if (counter == 0) {
			tsp_msg_sql_update_str(cmd_str, &tsp_msg, link_id);

			if (verbose)
				printf("tsp_msg_sql_insert_str: %s\n", cmd_str);
			sqlt3_ex(sqlt, cmd_str);
		}
		counter++;
		if (counter == modulo) counter = 0;
	}
}

static void sig_hand(int sig)
{
	longjmp(env, 1);
}
