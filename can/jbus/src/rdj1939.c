/**\file
 * 	Reads messages from the j1939 bus 
 *                   and saves data in the database
 *  Process to communicate with the J1939 Serial Network.  This process
 *  receives a message from the J1939 port (which may be an ordinary serial
 *  port with the B&B converter or a port to the SSV CAN Board) and puts the
 *  data into the database. The converter and the CAN port driver must be
 *  initialized by some other process first.
 *
 *  Options:
 *	-f filename for input
 *	-t puts bytes from every frame received on stdout  
 *	-v prints debug messages
 *
 *  If database open fails, data will be written to stdout instead of
 *  to the database.
 *
 *  The signals SIGINT, SIGQUIT, and SIGTERM are trapped, and cause the
 *  process to terminate.  Upon termination log out of the database.
 *
 *
 * Copyright (c) 2005   Regents of the University of California
 * 
 * Ported to QNX6 May 2005
 *
 */

#include "std_jbus_extended.h"

int fpin;	
jmp_buf env;

static int sig_list[] = 
{
	SIGINT,
	SIGQUIT,
	SIGTERM,
	SIGALRM,
	ERROR,
};

static void sig_hand(int sig)
{
	if (sig == SIGALRM)
		return;
	else
		longjmp(env, 1);
}

static struct avcs_timing tmg;

/** these variables are used to set special modes during
 * tracing and debugging */
extern int generic;
extern int j1939_debug;

/** True if only storing a single Parameter Group in database. */
int single_pgn = 0;
int pgn_to_keep[32]; // make it higher if you want to keep more pgn

/** Processes Broadcast Announce Messages from Transport Protocol. */
void
process_bam(struct j1939_multipacket *cfg_ptr, struct j1939_pdu *pdu,
		db_clt_typ *pclt, FILE *fp)
{
	int i;
	if (cfg_ptr->in_progress && ((single_pgn && cfg_ptr->pgn == pgn_to_keep[0])
		|| !single_pgn)) {
	/* write partial message to database */
		write_pgn_to_database(stdout, cfg_ptr->pgn,
			&cfg_ptr->pdu[0], pclt, j1939_debug);	
	}
	cfg_ptr->numpackets = pdu->data_field[3];
	if (cfg_ptr->numpackets < 1 || cfg_ptr->numpackets > J1939_MAX_FRAMES){
		if (j1939_debug) {
			j1939_pdu_typ pdu_out;
			get_current_timestamp(&(pdu_out.timestamp));
			pdu_to_pdu(pdu, &pdu_out);
			printf("Bad packet count: ");
			print_pdu(&pdu_out, fp, 0);
		}
		return;
	}
	cfg_ptr->in_progress = 1;
	for (i = 0; i < cfg_ptr->numpackets; i++)
		cfg_ptr->received[i] = 0;
}

/** Processes packets of a multipacket message. */
void
process_multipacket(struct j1939_multipacket *cfg_ptr, struct j1939_pdu *pdu,
		db_clt_typ *pclt, FILE *fp)
{
	int i;
	int sequence = pdu->data_field[0];
	timestamp_t current_time;
	get_current_timestamp (&current_time);
	if (sequence < 1 || sequence > J1939_MAX_FRAMES){
		if (j1939_debug) {
			j1939_pdu_typ pdu_out;
			pdu_out.timestamp = current_time;
			pdu_to_pdu(pdu, &pdu_out);
			fprintf(fp, "Bad sequence number: ");
			print_pdu(&pdu_out, fp, 0);
		}
		return;
	}
			
	if (!cfg_ptr->in_progress) {
		if (j1939_debug)
			printf("packet received with no BAM\n");
		cfg_ptr->in_progress = 1;
		cfg_ptr->numpackets = (pdu->src_address == 0)?4:3;	
		cfg_ptr->pgn = (pdu->src_address == 0)?ECFG:RCFG; 
		for (i = 0; i < cfg_ptr->numpackets; i++)
			cfg_ptr->received[i] = 0;
	}

	cfg_ptr->pdu[sequence-1] = *pdu;
	cfg_ptr->received[sequence-1] = 1;
	cfg_ptr->timestamp[sequence-1] = current_time;

	if ((sequence == cfg_ptr->numpackets) &&
		 ((single_pgn && cfg_ptr->pgn == pgn_to_keep[0])
		  || !single_pgn)) {
		write_pgn_to_database(fp, cfg_ptr->pgn, 
			&cfg_ptr->pdu[0], pclt, j1939_debug);
		cfg_ptr->in_progress = 0;
		cfg_ptr->numpackets = 0;
	}	
}

struct j1939_multipacket retarder_config;
struct j1939_multipacket engine_config;
struct j1939_multipacket *cfg_ptr;

int main(int argc, char *argv[] )
{
	struct j1939_pdu pdu;	/// Protocol Data Unit 
	db_clt_typ *pclt;      /// Database pointer 
	char *fname = J1939STB_SERIAL_DEVICE_NAME; 
	int external = 0;	/// external from jbus, internal converter  
	int slot_or_type;	/// external slot, internal type 
	int fpin;		/// serial port or input file 
	char *ftmgname = "rdj1939.tmg";
	FILE *fptmg;		/// output for timing information 
	int trace = 0;
	volatile long rcv_errors = 0;
	volatile long num_received = 0;
	int ch;
	int use_can = 0;
	jbus_func_t jfunc;
	int db_num;
	char *token;
	char *domain = DEFAULT_SERVICE;

	/* Use B&B J1939STB converter by default */
	jfunc.send = send_stb;
	jfunc.receive = receive_stb;
	jfunc.init = init_stb;
	jfunc.close = close_stb;

	while ((ch = getopt(argc, argv, "a:cd:f:s:tvg")) != EOF) {
		switch (ch) {
		case 'a': ftmgname = strdup(optarg);
			break;
		case 'c': jfunc.send = send_can;
			jfunc.receive = receive_can;
			jfunc.init = init_can;
			jfunc.close = close_can;
			use_can = 1;
			break;
		case 'd': domain = strdup(optarg);
			break;
		case 'f': fname = strdup(optarg);
			break;
		case 't': trace = 1;
			break;
		case 'g': generic = 1;	/* save as generic to database */
			break;
		case 's':
			token = strtok(optarg, ",");
			while (token != NULL)
			{
				db_num = atoi(token) - J1939_DB_OFFSET;
				pgn_to_keep[single_pgn] = db_ref[db_num].pgn;
				printf("db_num %d, pgn_to_keep 0x%x\n",
					   db_num, pgn_to_keep[single_pgn]);
				single_pgn++;

				token = strtok(NULL, ",");
			}
			break;
		case 'v': j1939_debug = 1;
			break;
		default: 
			printf("Usage: %s [-a <AVCS timing output>", argv[0]);
			printf("\t -c (CAN card vs serial STB) -d (debug)\n");
			printf("\t -t (trace) -f <CAN port>\n");
			printf("\t -s <db num to save>[,<db num to save>,..]. \n");
			printf("\t-g (generic save to DB)]\n");
			break;
		}
	}

	/* Initialize device port. */  
	if ((fpin = jfunc.init(fname, O_RDONLY, NULL)) == -1) {
		printf("Error opening %s device %s for input\n",
				use_can?"CAN":"STB", fname);
		exit(EXIT_FAILURE);
	}

	printf("%s: opened %s, %s\n", argv[0], fname, use_can?"CAN":"STB");
	fflush(stdout);

	/** Open file for timing information output */
	fptmg = fopen(ftmgname, "w");

	if (fptmg == NULL) {
		printf("Failed to open %s as timing output file\n", ftmgname);
		exit(EXIT_FAILURE);
	}
	pclt = j1939_database_init(argv);

	printf("rdj1939 opens database, ptr 0x%08x\n", (unsigned int) pclt);
	fflush(stdout);

	avcs_start_timing(&tmg);
	/*	set jmp */
	if (setjmp(env) != 0) {
		jfunc.close(&fpin);
		close_local_database(pclt);
		printf("rdj1939 terminated: %ld messages,", num_received); 
		printf(" %ld receive errors\n", rcv_errors);
		printf("%d DB receive count, %d DB update errors\n", 
			j1939db_receive_count, j1939db_update_err);
		fflush(stdout);
		avcs_end_timing(&tmg);
		avcs_print_timing(fptmg, &tmg);
		exit(EXIT_SUCCESS);
	}
	else 
		sig_ign(sig_list, sig_hand);

	engine_config.in_progress = 0;
	retarder_config.in_progress = 0;

	while (1) {
		int pgn;
		int rcv_val;

		tmg.exec_num++;
		rcv_val = jfunc.receive(fpin, &pdu, &external, &slot_or_type); 

		if (rcv_val == J1939_RECEIVE_MESSAGE_ERROR) {	
			rcv_errors++;
			continue;
		}
		if (rcv_val == J1939_RECEIVE_FATAL_ERROR)	
			break;

		/* received valid message */
		num_received++;
		pgn = (unsigned int) pdu.pdu_format << 8 | pdu.pdu_specific;
		if (trace){
			j1939_pdu_typ pdu_out;
			get_current_timestamp(&(pdu_out.timestamp));
			pdu_to_pdu(&pdu, &pdu_out);

			if (external) {
				print_pdu(&pdu_out, stdout, 1);
			} else
				printf("internal 0x%2x\n", slot_or_type);
			fflush(stdout);
		}
		if (!use_can && !external) {
			printf("Received internal message\n");
			continue;
		}	
			
		/* In "generic" mode, write all PDUs to database as byte
 		 * streams, don't translate into specific PDU formats or
		 * assemble multipacket messages 
		 */	
		if (generic) {
			write_pgn_to_database(stdout, pgn, &pdu, pclt,
				j1939_debug);
			continue;
		}
		if ((pgn == TPCM) && (pdu.data_field[0] == 32)){
		 /* Broadcast Announce (BAM)*/
			int broadcast_pgn = pdu.data_field[7];
			broadcast_pgn <<= 8;
			broadcast_pgn |= pdu.data_field[6];
			broadcast_pgn <<= 8;
			broadcast_pgn |= pdu.data_field[5];
			switch (broadcast_pgn) {
			case ECFG:
				cfg_ptr = &engine_config;
				break;
			case RCFG:
				cfg_ptr = &retarder_config;
				break;
			default:
				if (j1939_debug) {
					j1939_pdu_typ pdu_out;
					get_current_timestamp(&(pdu_out.timestamp));
					pdu_to_pdu(&pdu, &pdu_out);
					printf("Unrecognized BAM:");
					print_pdu(&pdu_out, stdout, 0);
				}
				continue;
				break;
			}
			cfg_ptr->pgn = broadcast_pgn;
			process_bam(cfg_ptr, &pdu, pclt, stdout);
		} else if (pgn == TPDT) {
		/* part of multipacket message */
			switch (pdu.src_address) {
			case 0: cfg_ptr = &engine_config;
				break;
			case 0x10:
			case 0xf: cfg_ptr = &retarder_config;
				break;
			default:
				if (j1939_debug) {
					j1939_pdu_typ pdu_out;
					get_current_timestamp(&(pdu_out.timestamp));
					pdu_to_pdu(&pdu, &pdu_out);
					printf("Unrecognized source for TPDT:");
					print_pdu(&pdu_out, stdout, 0);
				}
				continue;
				break;
			}	
			process_multipacket(cfg_ptr, &pdu, pclt, stdout);
		}
		else
		{
			int i = 0;
			int pgn_matches = 0;

			for (; i < single_pgn; i++)
				if (pgn == pgn_to_keep[i])
				{
					pgn_matches = 1;
					break;
				}
			
		    if (((single_pgn && pgn_matches) || 
				  pgn_to_keep[0] == PDU || !single_pgn) && !trace)
			{
				write_pgn_to_database(stdout, pgn, &pdu, pclt,
									  j1939_debug);
			}
		}
	}
	longjmp(env, 1);
}			

