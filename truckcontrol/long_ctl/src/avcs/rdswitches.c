/**\file
 * 
 * rdswitches.c - client for das_man compiled with dmm32.o
 *
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

#define MANUAL1			0X01
#define M1BIT			0x00

#define MANUAL2			0X02
#define M2BIT			0x01

#define AUTO1			0X04
#define A1BIT			0x02

#define AUTO2			0X08
#define A2BIT			0x03

jmp_buf exit_env;

static char *usage= "-d domain -v verbose -w ver verbose -o output mask (1 trace, 2 DB, 4 MySQL, 8 init DB vars, can OR) -n db_number (660 default) -p # inputs to scan";

#define TRACE_FILE      1
#define USE_DB          2
#define USE_MYSQL       4

static void sig_hand( int sig);

static int sig_list[] = 
{
        SIGINT,
        SIGQUIT,
        SIGTERM,
        ERROR,
};

static void sig_hand( int code) {
	longjmp( exit_env, code );
}

int main(int argc, char *argv[]) {

        char *ppath = "/dev/dmm32";
        unsigned long sample_rate = 50;
        char pscan[] = {0,1,2};
        int num_scan = 3;
	FILE *fd = 0;
	int err;
	float pbuf[DAS_MAX_CHANNEL];
	long port_A_data;
	long_dig_in_typ	dig_in;
	das_typ *pdas = NULL;
	db_clt_typ *pclt = NULL;	/* Database client pointer */
	char hostname[MAXHOSTNAMELEN + 1];
	char *domain = DEFAULT_SERVICE;
        int dmm32_db_num = DB_LONG_INPUT_VAR;
        int output_mask = 0;    // 1 trace, 2 DB server, 4 MySQL
        int verbose = 0;
        int option;
	unsigned char outchar = 0;
	unsigned char manual1;
	unsigned char manual2;
	unsigned char auto1;
	unsigned char auto2;
	int ctr =0;

    while ((option = getopt(argc, argv, "d:o:n:vc:p:")) != EOF) {
      switch(option) {
        case 'd':
                domain = strdup(optarg);
                break;
        case 'o':
                output_mask = atoi(optarg);     
                break;
        case 'c':
                outchar = atoi(optarg);     
                break;
        case 'v':
                verbose = 1;
		break;
        case 'n':
                dmm32_db_num = atoi(optarg);
                break;
        case 'p':
                num_scan = atoi(optarg);
                break;
        default:
                printf("Usage: %s %s\n",argv[0], usage);
                exit(EXIT_FAILURE);
                break;
        }
      }

        if (output_mask & USE_DB) {
		get_local_name(hostname, MAXHOSTNAMELEN);
		pclt = db_list_init(argv[0], hostname, domain, COMM_OS_XPORT, 
			NULL, 0, NULL, 0); 
		}

        if ( setjmp(exit_env) != 0) {
                /* Log out from the database. */
                if (pclt != NULL) {
			printf("RDSWITCHES.C: Calling db_list_done\n");
			db_list_done( pclt, NULL, 0, NULL, 0 );
			}
		if(pdas != NULL) {
			printf("Exiting rddmm32\n");
			das_done(pdas);
			}
		if(fd != NULL) {
			fclose(fd);
			}
                exit( EXIT_SUCCESS);
        }
        else 
                sig_ign( sig_list, sig_hand);

        if( (pdas = das_init(ppath, sample_rate, pscan, num_scan, 0)) 
		== NULL ) {
                printf("das_init failed\n");
                exit(EXIT_FAILURE);
                }
	printf("RDSWITCHES.C:das_init succeeded. pdas %#x fd %d sizeof(pdas->volts) %d pdas->result_size %d sizeof(pbuf) %d\n", 
		(int)pdas, pdas->fd, sizeof(pdas->volts), 
		pdas->result_size, sizeof(pbuf));
	if( (err = DAS_AD_ENQUEUE(pdas)) != EOK ) 
		printf("RDSWITCHES.C:das_ad_enqueue failed error %d\n", err);
	port_A_data = PORT_CFG_MODE | PORT_A_DIR_IN | PORT_B_DIR_OUT |
			PORT_C_LO_DIR_IN | PORT_C_HI_DIR_IN;
	printf("RDSWITCHES.C:port_A_data %#x\n", (unsigned int)port_A_data);
	if( (err = DAS_DIGITAL_DIR(pdas, DIGIO_CONTROL, 
			port_A_data)) != EOK ) 
		printf("RDSWITCHES.C:das_digital_dir failed error %d\n", err);

        if (output_mask & TRACE_FILE) {
		fd = fopen("rdswitches.log", "w");
		if(fd <= 0) {
			perror("fopen");
			exit(EXIT_FAILURE);
			}
		}

        while(1) {
                if( das_ad_pulse(pdas) ) {
			if( DAS_DIGITAL_IN(pdas, PORT_A, &port_A_data) == EOK) {
				if(verbose) {
					if(++ctr == 50) {
				      		printf("RDSWITCHES.C:port A input %#x\n", 
						(unsigned int)port_A_data);
						ctr = 0;
					}
				}
			}
                if (output_mask & USE_DB) {
                        get_current_timestamp(&dig_in.ts);
                        manual1 = (port_A_data & MANUAL1) >> M1BIT; 
                        manual2 = (port_A_data & MANUAL2) >> M2BIT; 
                        auto1 = (port_A_data & AUTO1) >> A1BIT; 
                        auto2 = (port_A_data & AUTO2) >> A2BIT; 

			// Digio is pulled UP to +5 VDC (logical 1), so if a 
			// digital input is disconnected, it'll read a "1".
			// Thus we AND the inputs together in case a wire is 
			// broken.
			dig_in.manualctl = manual1 & manual2;
			dig_in.autoctl = auto1 & auto2;

                        if( db_clt_write( pclt, DB_LONG_DIG_IN_VAR,
                                sizeof( long_dig_in_typ ),
                                &dig_in) == FALSE )
                                printf("RDSWITCHES.C:clt_update error: ");
                        }

		}
	}
}
