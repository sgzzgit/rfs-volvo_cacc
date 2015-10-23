/**\file
 *
 * ibeo_rx_file.c
 *
 * Reads file containing binary data from Ethernet message that
 * includes CAN bus message format for IBEO laser scanner.
 *
 * Unused code to actually read the CAN bus or to access database ran on
 * QNX4 and QNX6, not ported to Linux yet.
 *
 * Usage: ibeo_rx -f <filename> 
 *
 * OS-version-specific code encapsulaed in functions in ibeo_qnx4.c,
 * ibeo_qnx6.c, and ibeo_lnx.c.
 *
 */

#include "ibeo.h"


/* Needs to be given different values in each instance if more
 * than one Laserscanner is used
 */
can_std_id_t ibeo_can_base = IBEO_DEFAULT_SENSOR_ID;

/* signal handling set-up */
static int sig_list[] =
{
        SIGINT,
        SIGQUIT,
        SIGTERM,
        ERROR,
};

static jmp_buf env;

static void sig_hand(int sig)
{
        longjmp(env, 1);
}

int main(int argc,char **argv)
{
	int i;				// temporary for loops
	int channel_id;			
	int fd;				// file descriptor for CAN device
	unsigned long id;  		// CAN ID
	unsigned char data[8];		// data from CAN 
	unsigned int read_data[8];	// to read data using sscanf
	int ch;				// used with getopt
	int test_mode = 0;		// read from file, not CAN bus if 1
	int echo = 0;			// echo raw data
	char *devname;			// input file of raw data 
	FILE *fpin;			// input log file in test mode 
	char new_data[80];		// for reading line in test mode
	unsigned char cycle_counter = 0;// current scanner cycle 
	int check = 0;			// running checksum
	ibeo_list_typ ibeo_list;	// current IBEO list info
	ibeo_obj_typ ibeo_obj[16];	// maximum 16 objects
	
        while ((ch = getopt(argc, argv, "ef:")) != EOF) {
                switch (ch) {
                case 'e': echo = 1; 
			break;

                case 'f': devname = strdup(optarg);
			break;
		default:
			printf("Usage: %s -f devname\n", argv[0]);
			break;
		}
	}

	fpin = fopen(devname, "r");
		fprintf(stderr, "Opening file %s in test mode\n", devname);


        /* Exit code after signal  */
        if (setjmp(env) != 0) {
                exit(EXIT_SUCCESS);
        }
        else
                sig_ign(sig_list, sig_hand);

	
	while (1) {
		// can_get_message is receive-blocked, but may return without
		// data (return value 0) if some message of the wrong type
		// is received
		ibeo_data_t current;
		timestamp_t ts;

		if (!fgets(new_data, 80, fpin)) 
				break;
		sscanf(new_data, "%d %d %d %d %d %d %d %d %d",
//CAN ID not part of Ethernet format		&id,	
				&read_data[0], 
				&read_data[1], 
				&read_data[2], 
				&read_data[3], 
				&read_data[4], 
				&read_data[5], 
				&read_data[6], 
				&read_data[7]); 
		for (i = 0; i < 8; i++) 
				data[i] = read_data[i];
		if (echo) {
			printf("%8d %3d %3d %3d %3d %3d %3d %3d %3d\n",
//				id,
				data[0],
				data[1],
				data[2],
				data[3],
				data[4],
				data[5],
				data[6],
				data[7]);
		}
		if (!ibeo_parse_message(&current, id, data, &cycle_counter,
			&check)) {
			fprintf(stderr, "IBEO parse error\n");
			continue;
		}
		ibeo_pack_data(&ibeo_list, &ibeo_obj[0], &current);
		get_current_timestamp(&ts);
		if (current.oid == IBEO_DATA_END_ID &&
			!current.msg.end.cycle_error && 
			!current.msg.end.checksum_error) {
			ibeo_print_database(stdout, &ibeo_list, &ibeo_obj[0],
				&ts);
		}
			
	}
	return 0;
}

