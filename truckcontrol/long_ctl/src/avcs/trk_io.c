/**\file
 * 
 * trk_io.c.c - client for das_that reads brake pressure (A/D) and
 *		automatic/manual switches (digital in) and
 *		writes fault codes and watchdog signal (digital out)
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

/** Brake pressure comes from the front (FB), middle (MB) and rear (RB)
 */
#define FB_AXLE	0
#define MB_AXLE	1
#define RB_AXLE	2

/** Control codes for the Diamond Systems DMM 32 card
 */

/* Page 1 control (dig I/O) */
#define PORT_A          0x0c
#define PORT_B          0x0d
#define PORT_C          0x0e
#define DIGIO_CONTROL   0x0f

#define PORT_CFG_MODE		0x80
#define PORT_A_DIR_OUT		0x00
#define PORT_A_DIR_IN		0x10
#define PORT_B_DIR_OUT		0x00
#define PORT_B_DIR_IN		0x02
#define PORT_C_HI_DIR_OUT	0x00
#define PORT_C_HI_DIR_IN	0x08
#define PORT_C_LO_DIR_OUT	0x00
#define PORT_C_LO_DIR_IN	0x01

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
                                                                     
#define NCIN			0X03  // Inputs A0 & A1 (pins 8 & 7,respectively)
				      // are not connected. Since they should be
				      // tied to ground, they should evaluate to 
				      // zero.

/** To set watchdog signal, we toggle the output every time we are
 *  awake as long as the longitudinal control process is alive. The
 *  following constant is the maximum amount of times that we can
 *  see digout.xy_alive the same.
 */
#define MAX_DEAD 5

static char *usage= "-d domain -c output byte value (0-255, no DB) -v verbose -w very_verbose -o output mask (1 trace, 2 DB) -n db_number (660 default) -p # inputs to scan -r sample rate (Hz) -s stand-alone";

/** During regular operation, program will write to PATH DB server, -o 2
 *  -o3 will write to both DB server and trace file.
 */
#define TRACE_FILE      1
#define USE_DB          2
#define USE_MYSQL       4

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

/**
 *	Diamond DMM32-AT card is initialized to do an A/D conversion
 *	at a given sampling rate. 
 *
 *	Code waits at das_ad_pulse for the results of an A/D scan
 *	to be delivered, then writes results from the card
 *	to the DB data server and reads from the data server for
 *	values to set to the LEDs. Flips the watchdog value whenever
 *	the process awakes at das_ad_pulse, and changes the
 *	digital output pin that is being watched by the watchdog
 *	circuit. If the process fails to awake at das_ad_pulse, or
 *	the driver dies and doesn't change the digital output pin
 *	being watched, the watchdog circuit will set the red LED.
 */
int main(int argc, char *argv[]) {

        char *ppath = "/dev/dmm32";
        unsigned long sample_rate = 50;	//For now this has now effect
        char pscan[] = {0,1,2};
        int num_scan = 3;
        int index;
	FILE *fd = 0;
        int ctr = 0;
        int flashctr = 0;
	unsigned char flashing = 0;
	int err;
	float pbuf[DAS_MAX_CHANNEL];
	long port_A_data;
	long_input_typ	long_input;
	das_typ *pdas = NULL;
	db_clt_typ *pclt = NULL;	/// Database client pointer 
	char hostname[MAXHOSTNAMELEN + 1];
	char *domain = DEFAULT_SERVICE;
        int dmm32_db_num = DB_LONG_INPUT_VAR;
        int output_mask = 0;    /// 1 trace, 2 DB server, 4 MySQL
        int verbose = 0;
        int very_verbose = 0;
        int option;
	unsigned char outchar = 0;
	unsigned char test_outchar = LED_BLUE;	/// for tests without DB
	long_dig_in_typ dig_in;
	long_dig_out_typ dig_out;
	unsigned char watchdog = 0;
	unsigned char manual1;
	unsigned char manual2;
	int manctr = 0;
	unsigned char auto1;
	unsigned char auto2;
	int autoctr = 0;
	unsigned char brake1;
	unsigned char brake2;
	int brakectr = 0;
	unsigned char last_manual1 = 0;
	unsigned char last_auto1 = 0;
	unsigned char last_brake1 = 0;
	unsigned char ncerr;
	unsigned char standalone = 0;
	static unsigned char last_xy_alive = 0;
	static unsigned char is_dead_count = 0;

	memset(&dig_out, 0, sizeof(long_dig_out_typ));

	while ((option = getopt(argc, argv, "c:d:n:o:p:r:svw")) != EOF) {
		switch(option) {
		case 'c':
			test_outchar = atoi(optarg);     
			break;
		case 'd':
			domain = strdup(optarg);
			break;
		case 'n':
			dmm32_db_num = atoi(optarg);
			break;
		case 'o':
			output_mask = atoi(optarg);     
			break;
		case 'p':
			num_scan = atoi(optarg);
			break;
		case 'r':
			sample_rate = atoi(optarg);
			break;
		case 's':
			standalone = 1;
			break;
		case 'v':
			verbose = 1;
			break;
		case 'w':
			very_verbose = 1;
			break;
		default:
			printf("Usage: %s %s\n",argv[0], usage);
			exit(EXIT_FAILURE);
			break;
		}
	}

	/** das_init done before db_list_init and setjmp so that failure
	 * does not cause problems with DB server when process
	 * fails to log out.
	 */

        if ((pdas = das_init(ppath, sample_rate, pscan, num_scan, 0)) 
		== NULL ) {
                printf("das_init failed\n");
                exit(EXIT_FAILURE);
	}
	printf("%s:das_init succeeded. ", argv[0]);
	printf("pdas %#x fd %d sizeof(pdas->volts) %d ", 
		(int)pdas, pdas->fd, sizeof(pdas->volts)); 
	printf("pdas->result_size %d sizeof(pbuf) %d\n", 
		pdas->result_size, sizeof(pbuf));

        if (output_mask & USE_DB) {
		get_local_name(hostname, MAXHOSTNAMELEN);
		pclt = db_list_init(argv[0], hostname, domain, COMM_OS_XPORT, 
			NULL, 0, NULL, 0); 
	}

        if (setjmp(exit_env) != 0) {
		dig_out.outchar = 0;	// why not just set outchar?
		dig_out.amber_flash = 0;	// what does this do? not used?
		if (DAS_DIGITAL_OUT(pdas, PORT_B, dig_out.outchar, 0,
			NULL, NULL) != EOK ) {
			printf("Failure to turn out the lights\n");
                }

                /* Log out from the database. */
                if (pclt != NULL) {
			printf("%s: Calling db_list_done\n", argv[0]);
			db_list_done(pclt, NULL, 0, NULL, 0 );
		}
		if (pdas != NULL) {
			printf("Exiting %s\n", argv[0]);
			das_done(pdas);
		}
		if (fd != NULL) {
			fclose(fd);
		}
                exit(EXIT_SUCCESS);
        } else 
                sig_ign(sig_list, sig_hand);

	if ((err = DAS_AD_ENQUEUE(pdas)) != EOK ) 
		printf("%s: das_ad_enqueue failed error %d\n", argv[0], err);

	port_A_data = PORT_CFG_MODE | PORT_A_DIR_IN | PORT_B_DIR_OUT |
			PORT_C_LO_DIR_IN | PORT_C_HI_DIR_IN;
	printf("%s: port_A_data %#x\n", argv[0], (unsigned int)port_A_data);

	if ((err = DAS_DIGITAL_DIR(pdas, DIGIO_CONTROL, 
			port_A_data)) != EOK) 
		printf("%s: das_digital_dir failed error %d\n", argv[0], err);

        if (output_mask & TRACE_FILE) {
		fd = fopen("trk_io.log", "w");
		if (fd <= 0) {
			perror("fopen");
			exit(EXIT_FAILURE);
		}
	}

	while(1) {	
		/** We wake up at an interval specified in ms by Ticks
		 * in realtime.ini. This is set by the DAS open call
		 * to wake up the driver at certain intervals.
		 * We get analog inputs when awakened by the client notify
		 * pulse sent by the driver.
		 */
		if (das_ad_pulse(pdas)) {
			if (very_verbose) {
				timestamp_t ts;
				get_current_timestamp(&ts);
				print_timestamp(stdout, &ts);
				printf("\n");
			}
			if (verbose) {
				if ((++ctr%50) == 0 ) {
					for(index = 0; index < num_scan;
						 index++){ 
						printf("%- 3.2f ", 
							pdas->volts[index]);
					}
				}
			}
		}

		/* Set digital outputs */
		if (output_mask & USE_DB) {
			db_clt_read(pclt, DB_LONG_DIG_OUT_VAR,
				sizeof(long_dig_out_typ), &dig_out);
			outchar = dig_out.outchar;
		} else
			outchar = test_outchar;

		if (last_xy_alive == dig_out.xy_alive) { 
			is_dead_count++;
			if (is_dead_count == 0)
				is_dead_count = MAX_DEAD;
		} else {
			is_dead_count = 0;
		}
		last_xy_alive = dig_out.xy_alive;

		if ((is_dead_count < MAX_DEAD) || standalone)
			watchdog = 1 - watchdog;  // flip from 1 to 0, 0 to 1	

		// Adjust count to get desired appearance of LED flash
		if (++flashctr >= 10) {
			if (flashing) {
				flashing = 0;
				if (dig_out.amber_flash)
					outchar &= LED_NOT_AMBER;
			} else {
				flashing = 1;
				if (dig_out.amber_flash)
					outchar |= LED_AMBER;
			}
			flashctr = 0;
		}

		/** watchdog will change every time we wake up as long
		 *  as the longitudinal control process is alive
		 *  (or we are in standalone test mode). 
		 */
		if (watchdog) 
			outchar |= WDOGBIT; 
		else
			outchar &= NWDOGBIT;

		if (DAS_DIGITAL_OUT(pdas, PORT_B, outchar, 0,
			NULL, NULL) == EOK ) {
			if (verbose) {
				if ((ctr%50) == 0 ) {
					printf("port B output %#x ", outchar);
					fflush(stdout);
				}
			}
			
		}

		/* Get digital inputs */
		if (DAS_DIGITAL_IN(pdas, PORT_A, &port_A_data) == EOK ) {
			if (verbose) 
				if ((ctr%50) == 0 )
					printf("port A input %#x\n", 
						(unsigned char)port_A_data);
		}

		if (output_mask & USE_DB) {
		/* Write brake values to db */
			get_current_timestamp(&long_input.ts);
			long_input.fb_axle = pdas->volts[FB_AXLE];
			long_input.mb_axle = pdas->volts[MB_AXLE];
			long_input.rb_axle = pdas->volts[RB_AXLE];
			db_clt_write(pclt, DB_LONG_INPUT_VAR, 
				sizeof(long_input_typ), &long_input);

			/* Write control switch values to db */
			get_current_timestamp(&dig_in.ts);
			manual1 = (port_A_data & MANUAL1) >> M1BIT;
			manual2 = (port_A_data & MANUAL2) >> M2BIT;
			auto1 = (port_A_data & AUTO1) >> A1BIT;
			auto2 = (port_A_data & AUTO2) >> A2BIT;
			brake1 = (port_A_data & BRAKE1) >> B1BIT;
			brake2 = (port_A_data & BRAKE2) >> B2BIT;
			ncerr = port_A_data & NCIN;
									     
			if(ncerr)
				printf("NC input read error: ncerr %d\n",ncerr);
			// Check for a broken wire
			if ( (manual1 != manual2) || (auto1 != auto2) ) {
				print_timestamp(stderr, &dig_in.ts);
				dig_in.man_autoswerr = TRUE;
				fprintf(stderr,"Auto/Manual switch error: manual1 %d",
					manual1);
				fprintf(stderr," manual2 %d ",
					manual2);
				fprintf(stderr,"auto1 %d auto2 %d port A input %#x\n",
					auto1, auto2, 
					(unsigned char)port_A_data);
			}
			else
				dig_in.man_autoswerr = FALSE;
			if (brake1 != brake2) {
				dig_in.brakeswerr = TRUE;
				fprintf(stderr,"Brake switch error: brake1 %d",
					brake1);
				fprintf(stderr," brake2 %d\n",
					brake2);
			}
			else
				dig_in.brakeswerr = FALSE;
									     
			manctr++;
			autoctr++;
			brakectr++;

			// Check for a transition
			if (manual1 != last_manual1) {
				print_timestamp(stderr, &dig_in.ts);
				fprintf(stderr, 
				    "last_manual1 %d -> manual1 %d manctr %d\n",
					last_manual1, manual1, manctr);
				manctr = 0;
			}
			if (auto1 != last_auto1) {
				print_timestamp(stderr, &dig_in.ts);
				fprintf(stderr, 
					"last_auto1 %d -> auto1 %d autoctr %d\n",
					last_auto1, auto1, autoctr);
				autoctr = 0;
			}
			if (brake1 != last_brake1) {
				print_timestamp(stderr, &dig_in.ts);
				fprintf(stderr, 
				    "last_brake1 %d -> brake1 %d brakectr %d\n",
					last_brake1, brake1, brakectr);
				brakectr = 0;
			}

			last_manual1 = manual1;
			last_auto1 = auto1;
			last_brake1 = brake1;

			dig_in.manualctl = manual1;        
			dig_in.autoctl = auto1;              
			dig_in.brakesw = brake1 & brake2;              

			db_clt_write(pclt, DB_LONG_DIG_IN_VAR,
				 sizeof( long_dig_in_typ ), &dig_in);                  
		}

		if (output_mask & TRACE_FILE) {
			if ((ctr%50) == 0 ) {
				for(index = 0; index < num_scan; index++){ 
					fprintf(fd,"%- 3.2f ", 
						pdas->volts[index]);
				}
				fprintf(fd,"\n");
				fflush(fd);
			}
		}
	}
}
