/**\file
 * 
 * fake_switches.c - fake switch inputs to database (trk_io, or any
 * other Diamond A/D driver, MUST NOT be running)
 * 
 *  Copyright (c) 2009   Regents of the University of California
 *
*/

#include <db_include.h>
#include <db_comm.h>
#include "sys_mem.h"
#include "sys_das.h"
#include "db_clt.h"
#include "db_utils.h"
#include "clt_vars.h"
#include "veh_trk.h"
#include "long_ctl.h"	// has definitions for LED_ output control constants

/** Pins used for input from the truck
 */
#define MANUAL1                 0X80 // Pin 1 (A7)
#define M1BIT                   0x07
                                                                     
#define MANUAL2                 0X40  // Pin 2 (A6)
#define M2BIT                   0x06
                                                                     
#define AUTO1                   0X20  // Pin 3 (A5)
#define A1BIT                   0x05
                                                                     
#define AUTO2                   0X10  // Pin 4 (A4)
#define A2BIT                   0x04
                                                                     
#define BRAKE1                  0X08  // Pin 5 (A3)
#define B1BIT                   0x03
                                                                     
#define BRAKE2                  0X04  // Pin 6 (A2)
#define B2BIT                   0x02

static char *usage= "-c DIO byte value manual1=0x80, manual2=0x40, auto1=0x20, auto2=0x10, brake1=0x08, brake2=0x04";

/** Boiler plate for signal handling
 */
jmp_buf exit_env;
static void sig_hand(int sig);

static int sig_list[] = 
{
        SIGINT,
        SIGQUIT,
        SIGTERM,
        ERROR,
};

static void sig_hand(int code) {
	longjmp(exit_env, code );
}

int main(int argc, char *argv[]) {

	int port_A_data;
	db_clt_typ *pclt = NULL;	/// Database client pointer 
	char hostname[MAXHOSTNAMELEN + 1];
	char *domain = DEFAULT_SERVICE;
        int option;
	long_dig_in_typ dig_in;
	unsigned char manual1;
	unsigned char manual2;
	unsigned char auto1;
	unsigned char auto2;
	unsigned char brake1;
	unsigned char brake2;

	while ((option = getopt(argc, argv, "c:h")) != EOF) {
		switch(option) {
		case 'c':
			port_A_data = strtol(optarg, (char **)NULL, 0);     
			break;
		case 'h':
		default:
			printf("Usage: %s %s\n",argv[0], usage);
			exit(EXIT_FAILURE);
			break;
		}
	}
	get_local_name(hostname, MAXHOSTNAMELEN);
	pclt = db_list_init(argv[0], hostname, domain, COMM_OS_XPORT, 
			NULL, 0, NULL, 0); 

        if (setjmp(exit_env) != 0) {
                /* Log out from the database. */
                if (pclt != NULL) {
			printf("%s: Calling db_list_done\n", argv[0]);
			db_list_done(pclt, NULL, 0, NULL, 0 );
		}
        } else 
                sig_ign(sig_list, sig_hand);

	printf("%s: port_A_data %#x\n", argv[0], (unsigned int)port_A_data);

		/* Write brake values to db */
			/* Write control switch values to db */
			get_current_timestamp(&dig_in.ts);
			manual1 = (port_A_data & MANUAL1) >> M1BIT;
			manual2 = (port_A_data & MANUAL2) >> M2BIT;
			auto1 = (port_A_data & AUTO1) >> A1BIT;
			auto2 = (port_A_data & AUTO2) >> A2BIT;
			brake1 = (port_A_data & BRAKE1) >> B1BIT;
			brake2 = (port_A_data & BRAKE2) >> B2BIT;
			// Check for a broken wire
			if ( (manual1 != manual2) || (auto1 != auto2) ) {
				dig_in.man_autoswerr = TRUE;
			}
			else
				dig_in.man_autoswerr = FALSE;
			if (brake1 != brake2) {
				dig_in.brakeswerr = TRUE;
			}
			else
				dig_in.brakeswerr = FALSE;
									     
			dig_in.manualctl = manual1;        
			dig_in.autoctl = auto1;              
			dig_in.brakesw = brake1 & brake2;              

			printf("\nFollowing are the switch settings:\n");
			printf("manual1 %d manual2 %d dig_in.manualctl %d\n", 
				manual1, manual2, dig_in.manualctl);
			printf("  auto1 %d   auto2 %d dig_in.autoctl %d ", 
				auto1, auto2, dig_in.autoctl);
			printf("\tman_autoswerr %d\n", 
				dig_in.man_autoswerr);
			printf(" brake1 %d  brake2 %d dig_in.brakesw %d ", 
				brake1, brake2, dig_in.brakesw);
			printf("\tbrakeswerr %d\n", 
				dig_in.brakeswerr);
			db_clt_write(pclt, DB_LONG_DIG_IN_VAR,
				 sizeof( long_dig_in_typ ), &dig_in);                  

}
