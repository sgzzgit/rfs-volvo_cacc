/**\file
 *
 * ibeo_dump.c
 *
 * Reads file from standard input containing binary data 
 * in format used in April 2008 by hwy12/test/collect_ibeo.php.  
 *
 * Usage: ibeo_dump -v <foo.ibeo  >ibeo_dump.log
 *
 * Writes file foo.ldr with filename created from first header line in file
 * 
 */

#include "ibeo.h"

// must define this for ibeo_utils library even though not used
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

/// Maximum size of object data message is 3136, according to 
/// IBEO ALASCA User Manual, page 28, section 4.7.3
static unsigned char object_buf[3136];	

/// Format filename according to conventions used in CICAS Observation
/// experiments April 2008
void cicas_obs_make_filename(char *filename, int month, int day, 
		int year, timestamp_t *pts)
{
	char am_or_pm[3];	/// holds AM or PM
	char month_str[10];	/// holds name of month

	strncpy(am_or_pm, "AM", 3);
	if (pts->hour > 12) {
		strncpy(am_or_pm, "PM", 3);
		pts->hour -= 12;
	}
	switch (month) {
	case 1: strncpy(month_str, "January", 10);
		break;
	case 2: strncpy(month_str, "February", 10);
		break;
	case 3: strncpy(month_str, "March", 10);
		break;
	case 4: strncpy(month_str, "April", 10);
		break;
	case 5: strncpy(month_str, "May", 10);
		break;
	case 6: strncpy(month_str, "June", 10);
		break;
	case 7: strncpy(month_str, "July", 10);
		break;
	case 8: strncpy(month_str, "August", 10);
		break;
	case 9: strncpy(month_str, "September", 10);
		break;
	case 10: strncpy(month_str, "October", 10);
		break;
	case 11: strncpy(month_str, "November", 10);
		break;
	case 12: strncpy(month_str, "December", 10);
		break;
	default: strncpy(month_str, "Unknown", 10);
		break;
	}
	sprintf(filename,"%s-%d-%d-%02d_%02d_%02d_%s.ldr",
		month_str, day, year, pts->hour, pts->min, pts->sec,
		am_or_pm);
} 

/// Read up to but not including next pipe character '|' into tmp 
/// "size" should be length of string, not size of array

int get_next_hdr_str(FILE *fp, unsigned char *tmp, int size)
{
	unsigned char byte = '\0';
	int i = 0;
	if (fread(&byte, 1, 1, fp) == 0)
		return size+1;	// to indicate EOF or error
	while (byte != '|' && i < size) {
		tmp[i++] = byte;
		if (fread(&byte, 1, 1, fp) == 0) 
			return size+1;	// to indicate EOF or error
	}
	tmp[i] = '\0';
	printf("get_next_hdr_str: %s\n", tmp);
	return i;
}

#define TEMP_SIZE 32
/// Read the header added to IBEO ALASCA data during CICAS Observation
/// April 2008 and set up filename for output, timestamp for next
/// object and size variables. 
/// Read the data type which will be used to decide whether to
/// save or skip the message body.

int input_cicas_obs_header(FILE *fp, int *ptype, char *filename,
			int *psize, timestamp_t *pts)
{
	unsigned char tmp[TEMP_SIZE];
	int month, day, year;
	int i;	// count for debugging
	int data_type;
	int retval = 1;		/// return TRUE if no errors
	int converted = 0;	/// number of sscanf arguments converter

	/// get date string
	i = get_next_hdr_str(fp, tmp, TEMP_SIZE-1);
	converted = sscanf(tmp, "%d-%d-%d", &year, &month, &day); 
	if (i > TEMP_SIZE - 1 || converted != 3)
		retval = 0;

	/// get time string
	i = get_next_hdr_str(fp, tmp, TEMP_SIZE-1);
	converted = sscanf(tmp, "%d:%d:%d", &pts->hour, &pts->min, &pts->sec); 
	if (i > TEMP_SIZE - 1 || converted != 3)
		retval = 0;

	/// skip system time
	i = get_next_hdr_str(fp, tmp, TEMP_SIZE-1);

	/// skip timestamp
	i = get_next_hdr_str(fp, tmp, TEMP_SIZE-1);
	
	/// get data type
	i = get_next_hdr_str(fp, tmp, TEMP_SIZE-1);
	*ptype = atoi(tmp);
	if (i > TEMP_SIZE - 1) 
		retval = 0;
		
	/// get message body size 
	i = get_next_hdr_str(fp, tmp, TEMP_SIZE-1);
	if (i > TEMP_SIZE - 1) 
		retval = 0;
	*psize = atoi(tmp);

	cicas_obs_make_filename(filename, month, day, year, pts);

	return retval;
}


int main(int argc,char **argv)
{
	int i;				// temporary for loops
	int ch;				// used with getopt
	int verbose = 0;		// print verbose to stderr 
	ibeo_list_typ ibeo_list;	// current IBEO list info
	ibeo_obj_typ ibeo_obj[32];	// maximum 32 objects
	can_std_id_t id = IBEO_DATA_ID;	// not part of Ethernet format
					// set so ibeo_parse_message won't fail
	unsigned char data[8];		// holds CAN message block				
	FILE *fpout = NULL;		// output file
	FILE *fp = stdin;		// later may want to open named file
	char filename[80];		// name of output file, set from header
	timestamp_t ts;			// timestamp from header
	int bytes_left = 0;		// set from header
	int data_type = 0;		// set from header
	int first_time = 1;		// set to zero after opening file
	int exit_code;			// set by longjmp, returned by setjmp
	int check;			// needed by ibeo_parse_message		
	unsigned char cycle_counter;	// needed by ibeo_parse_message
	int all_object_count = 0;	// used for debugging 

        while ((ch = getopt(argc, argv, "v")) != EOF) {
                switch (ch) {
                case 'v': verbose = 1; 
			break;
		default:
			printf("Usage: %s -v <verbose>\n", argv[0]);
			exit(EXIT_FAILURE);
			break;
		}
	}
	if (verbose)
		printf("id 0x%x\n", id);

        /* Exit code after signal or longjmp  */
        if ((exit_code = setjmp(env)) != 0) {
		printf("exiting with code %d\n", exit_code); 
		switch (exit_code) {
		case 1: printf("Error opening output file\n");
			break;
		case 2:
		case 3:
		case 4: printf("Error while skipping scan data\n");
			break;
		case 5: printf("Error unknown IBEO ALASCA data type\n");
			break;
		case 6: printf("Error while reading object data\n");
			break;
		case 7: printf("EOF or error while reading header\n");
			break;
		default: 
			break;
		}
		if (fpout)
			fclose(fpout);
                exit(EXIT_SUCCESS);
        } else
                sig_ign(sig_list, sig_hand);
	
	/// Loop exits on end of file or error processing header
	while (input_cicas_obs_header(fp, &data_type, filename,
					&bytes_left, &ts)) {
		if (verbose) {
			print_timestamp(stdout, &ts);
			printf(" %s %d %d\n", filename, data_type, bytes_left);
		} 
		if (first_time) {
			if (!(fpout = fopen(filename,"w")))
				longjmp(env,1);
			first_time = 0;
		}
		switch (data_type) {
		case IBEO_ALASCA_COMPRESSED_SCAN_DATA_TYPE:
		case IBEO_ALASCA_255_DATA_TYPE:
			{
				unsigned char byte;
			/// read and ignore compressed scan data
			/// exit with different codes if EOF or missing '\n'
				for (i = 0; i < bytes_left; i++) {
					if (fread(&byte, 1, 1, fp) == 0)
						longjmp(env,2);
				}
				if (fread(&byte, 1, 1, fp) == 0)
					longjmp(env,3);
				if (byte != '\n')
					longjmp(env,4);
			}
			break;
		case IBEO_ALASCA_OBJECT_DATA_TYPE:
			while (bytes_left > 0)  {
				ibeo_data_t current;

				if (fread(data, 1, 8, fp) < 8)
					longjmp(env,6);
				bytes_left -= 8;
					
				if (verbose) {
					for (i = 0; i < 8; i++)
						printf("%03hhu ", data[i]);
					printf("\n");
				}
				if (!ibeo_parse_message(&current, id, data,
					&cycle_counter, &check)) {
					printf("IBEO parse error\n");
					continue;
				}
				if (verbose) {
					if (current.oid == 2) {
					    all_object_count++;
					    printf(
			"all_object_count % d point_count %d x %d y %d\n",
						all_object_count,
						current.msg.obj.point_count,
						current.msg.obj.position_x,
						current.msg.obj.position_y);
					}
					if (current.oid == 1)
					    printf("object_count %d cycle %d\n",
						current.msg.lst.object_count,
						current.msg.lst.cycle_counter);
				}
				ibeo_pack_data(&ibeo_list, &ibeo_obj[0], 
					&current);
				if (current.oid == IBEO_DATA_END_ID 
//					&& !current.msg.end.cycle_error && 
//					!current.msg.end.checksum_error) {
									){
						ibeo_print_database(fpout,
							 &ibeo_list,
							 &ibeo_obj[0], &ts);
				}
			}
			break;
		default:	/// unknown data type; parse error?
			longjmp(env,5);
			break;
		}
		
	}
	longjmp(env,7);
}
