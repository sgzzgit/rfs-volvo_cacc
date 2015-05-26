/**
 *	Waits on a trigger message for a particular J1939 DB VAR.
 *	Either counts the messages (-n 0) or prints up to N of them
 *	to a file (-n N). If more than N are received, starts over.
 *
 *	Used for standalone testing of J1939 bus activity and of
 *	writes to the data server.
 *
 *
 * 	Copyright (c) 2010   Regents of the University of California
 *
 *
 */

char *usage="Usage: %s: -d DB VAR number -f output file -n number of messages -v verbose \n";

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

static void sig_hand(int sig)
{
	longjmp(exit_env, 1);
}

/**
 *	Prints all saved messages of the triggered type.
 */	
void
print_dbvs(j1939_dbv_info *info, unsigned char *msg_buf, 
			int num_msg, FILE *fpout, int numeric)
{
	int i;
	int size = info->dbv_size;
	unsigned char *cur_msg;

	cur_msg = msg_buf;
	for (i = 0; i < num_msg; i++) {
		info->print_dbv((void *) cur_msg, fpout, numeric); 
		cur_msg += size;
	}
}

int main(int argc, char *argv[])
{
	trig_info_typ trig_msg;
	db_clt_typ *pclt;	/* pointer to CLT database */
	db_data_typ *dbv;	/* data returned from database */
	j1939_dbv_info *info;	/* pointer to method info for pgn type */
	int dbv_size;
	char *filename = "j1939.out";
	FILE *fpout;		
	int ch;		
	int db_number = J1939_DB_OFFSET; /* Number of database variable */
	int num_msg = 1000;
	int numeric = 1;	/// default: print in numeric format 
	unsigned char *msg_buf;	/// buffer is malloced
	unsigned char *cur_msg;	/// insert location in msg_buf
	int msg_count = 0;	/// number of messages saved to buffer
	int trig_count = 0;

	while ((ch = getopt(argc, argv, "d:f:n:v")) != EOF) {
                switch (ch) {
                case 'd': db_number = atoi(optarg);
                          break;
		case 'f': filename = strdup(optarg);
			  break;
                case 'n': num_msg = atoi(optarg); //used buffer output
                          break;
		case 'v': numeric = 0;
			  break;
                default: printf(
					 argv[0]);
                          break;
                }
        }
	if (db_number < J1939_DB_OFFSET || db_number > J1939_DB_OFFSET + 99)
		exit(EXIT_FAILURE);

	/* Log in to the database. */
	pclt = j1939_database_init(argv);

	fpout = fopen(filename, "w");

	if(setjmp(exit_env) != 0) {
		fprintf(fpout,"trigj1939 exits, msg_count %d, trig_count %d\n",
			 msg_count, trig_count);
		fflush(fpout);
		if (msg_count > 0 && num_msg > 0)
			print_dbvs(info, msg_buf, msg_count, fpout, numeric);
		clt_logout(pclt);
		fclose(fpout);
		exit(EXIT_SUCCESS);
	} else
		sig_ign(sig_list, sig_hand);

	/* Create the desired data base trigger (depends on database
         * VAR and TYP being assigned the same number)
	 */
	if (clt_trig_set(pclt, db_number, db_number) == FALSE) {
		fprintf(stderr, 
			"database 0x%x: DB VAR %d trigger set failed\n",
			 (unsigned int) pclt, db_number);
		longjmp(exit_env,1);	
	}

	/* get the info pointer for the data base variable */
	info = get_jdbv_info(db_number);

	dbv_size = info->dbv_size;

	/// Allocate the buffer, if needed
	if (num_msg > 0) {
		msg_buf = malloc(dbv_size * num_msg);	
		cur_msg = msg_buf;
	} else
		cur_msg = malloc(dbv_size);	


	fprintf(fpout, "Triggering on DB VAR %d\n", db_number);

	/* This loop waits for a trigger message from a data base variable.
	 * When a trigger is detected, the variable fields are written to
	 * the output file.
	 */ 
	for (;;) {
		int rcv_type= clt_ipc_receive(pclt, &trig_msg, sizeof(trig_msg));
		trig_count++;
		if (rcv_type == DB_TIMER) {
			fprintf(stderr, "Unexpected timer alarm!");
			continue;
		}
	        if (DB_TRIG_VAR(&trig_msg) == db_number) {
			if ((dbv = read_from_database(db_number, pclt)) 
					!= NULL) {
				msg_count++;
				memcpy(cur_msg, dbv, dbv_size); 
				if (num_msg > 0)
					cur_msg += dbv_size;
				// print and start over saving when num_msg is reached
				if (msg_count == num_msg) {
					print_dbvs(info, msg_buf,
						 num_msg, fpout, numeric);
					cur_msg = msg_buf;
					msg_count = 0;
				}
			}
			else { 
				fprintf(stderr, "%s exiting database error\n",
						argv[0]);
				break;
			}
	    	}
	}
	longjmp(exit_env,1);	/* go to exit code if trigger loop terminates */
}
