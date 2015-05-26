/* set_fault.c - test program for setting control system fault codes
**
*/

#include <db_include.h>
#include <db_utils.h>
#include <sys/select.h>
#include <sys/timeb.h>

#include "db_clt.h"
#include "veh_trk.h"
#include "clt_vars.h"
#include "long_ctl.h"

db_clt_typ *pclt;              /* Database client pointer */

int main( int argc, char *argv[] ){

        long_dig_out_typ dig_out;
        char hostname[MAXHOSTNAMELEN+1];
        char *domain = DEFAULT_SERVICE;
        timestamp_t ts;
	int option;

	while ((option = getopt(argc, argv, "hmlarcz")) != EOF) {
		switch(option) {
        		case 'h':
                		dig_out.outchar = LED_RED;
                        	dig_out.amber_flash = 0;
                		break;
        		case 'm':
                        	dig_out.outchar = LED_GRN | LED_AMBER;
                        	dig_out.amber_flash = 1;
                		break;
        		case 'l':
                        	dig_out.outchar = LED_GRN | LED_AMBER;
                        	dig_out.amber_flash = 0;
                		break;
        		case 'a':
                        	dig_out.outchar = LED_GRN;
                        	dig_out.amber_flash = 0;
                		break;
        		case 'c':
                        	dig_out.outchar = LED_AMBER;
                        	dig_out.amber_flash = 0;
                		break;
        		case 'r':
                        	dig_out.outchar = LED_GRN | LED_BLUE;
                        	dig_out.amber_flash = 0;
                		break;
        		case 'z':
                        	dig_out.outchar = 0;
                        	dig_out.amber_flash = 0;
                		break;
			default: printf(" Usage: \n");
				printf("-h: red\n");
				printf("-m: green and flashing yellow\n");
				printf("-l: green and yellow\n");
				printf("-a: green\n");
				printf("-c: yellow (not flashing)\n");
				printf("-r: green and blue \n");
				printf("-z: turn off all lights \n");
				exit (EXIT_FAILURE);
				break;
		}
	}

        get_local_name(hostname, MAXHOSTNAMELEN);
        pclt = db_list_init(argv[0], hostname, domain, COMM_OS_XPORT,
                NULL, 0, NULL, 0);

        get_current_timestamp(&ts);
        print_timestamp(stdout, &ts);
        /* Now write the trigger to the database. */
        db_clt_write( pclt, DB_LONG_DIG_OUT_VAR, sizeof(long_dig_out_typ), &dig_out);
        clt_logout( pclt );
        return 0;
}
