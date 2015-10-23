/**\file
 *
 * ibeo_rx.c
 *
 * Reads CAN bus connected to an IBEO laser scanner and writes data 
 * to database.
 *
 * Usage: ibeo_rx /dev/can1
 *
 * OS-version-specific code encapsulaed in functions in ibeo_qnx4.c
 * and ibeo_qnx6.c.
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
	db_clt_typ *pclt;		// database client pointer
	FILE *fpin;			// input log file in test mode 
	char new_data[80];		// for reading line in test mode
	char *devname = "/dev/can1";	// CAN device name
	char *domain = "";		// not used on QNX4, must set on QNX6
	unsigned char cycle_counter = 0;// current scanner cycle 
	int check = 0;			// running checksum
	ibeo_list_typ ibeo_list;	// current IBEO list info
	ibeo_obj_typ ibeo_obj[16];	// maximum 16 objects
	
        while ((ch = getopt(argc, argv, "d:ef:i:t")) != EOF) {
                switch (ch) {
                case 'd': domain = strdup(optarg);
                          break;
                case 'e': echo = 1; 
                          break;
                case 'f': devname = strdup(optarg);
                          break;
                case 'i': ibeo_can_base = atoi(optarg);
                          break;
                case 't': test_mode = 1;
                          break;
		default:
			printf("Usage: %s -f devname\n", argv[0]);
			break;
		}
	}
	/* Login to database and create IBEO variables */
        pclt = ibeo_database_init(argv[0], domain);

	if (test_mode) { 
		fpin = fopen(devname, "r");
		fprintf(stderr, "Opening file %s in test mode\n", devname);
	} else {
		fd = can_open(devname);
		fprintf(stderr, "Opening %s\n", devname);
		channel_id = init_can(fd, pclt, argv);
	}


        /* Exit code after signal  */
        if (setjmp(env) != 0) {
		if (pclt != NULL)
			clt_logout(pclt);
                exit(EXIT_SUCCESS);
        }
        else
                sig_ign(sig_list, sig_hand);

	
	while (1) {
		// can_get_message is receive-blocked, but may return without
		// data (return value 0) if some message of the wrong type
		// is received
		ibeo_data_t current;

		if (test_mode) {
			if (!fgets(new_data, 80, fpin)) 
				break;
			sscanf(new_data, "%d %d %d %d %d %d %d %d %d",
				&id,
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
		} else {
			if (!can_get_message(fd, channel_id, &id, data, 8))
				continue;
		}
		if (echo) {
			printf("%8d %3d %3d %3d %3d %3d %3d %3d %3d\n",
				id,
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
		if (current.oid == IBEO_DATA_END_ID &&
			!current.msg.end.cycle_error && 
			!current.msg.end.checksum_error) {
			ibeo_update_database(pclt, &ibeo_list, &ibeo_obj[0]);
			ibeo_print_database(stdout, &ibeo_list, &ibeo_obj[0]);
		}
			
	}
	return 0;
}

