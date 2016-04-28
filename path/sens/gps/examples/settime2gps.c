/* FILE: settime2gps.c
 *  Process to read the GPRMC GPS message, and use its date and time to
 *  set the system clock.
 */

#include <sys_os.h>

int fpin;	
jmp_buf env;

int gps2systime (int utc_date, float utc_time );

int status;

#define MAX_MSG_LENGTH         128

int main( int argc, char *argv[] )
{
	char readBuff[MAX_MSG_LENGTH];
	unsigned int checksum;
	unsigned int msg_checksum;
	char header[11];
	char temp;
	int i;
	float rmc_utc;
	char rmc_status;
	double rmc_lat;
	char rmc_lat_dir;
	double rmc_long;
	char rmc_long_dir;
	float rmc_sog_knot;
	float rmc_cog_deg;
	int rmc_date;
	float rmc_mag_var;
	char rmc_mag_dir;
	int opt;
	int counter = 0;
	char *serdev = "/dev/ser1";

        /** Get command line options */
        while ((opt = getopt(argc, argv, "p:")) != -1) {
                switch (opt) {
                case 'p':
                        serdev = strdup(optarg);
                        break;
		default:
			printf("Usage: %s -p <serial port>\n", argv[0]);
			break;
		}
	}

	/* Initialize serial port. */  
	fpin = open( serdev,  O_RDONLY );
	if ( fpin <= 0 ) {
		printf( "Error opening device %s for input\n", serdev);
		printf("Usage: %s -p <serial port>\n", argv[0]);
        exit(EXIT_FAILURE);
	}
	while(1)
	    {

	    /* Read until the beginning of a message, signified by the '$' char. */
	    temp = '\0';
	    while ( temp != '$' )
	        read( fpin, &temp, 1 );
//fprintf(stderr,"%c",temp);

	    /* For some (undocumented) reason, the checksum seems to be
	     * the exclusive or of 0x03 and all bytes of the message between
	     * the "$' and '*' characters. */
	    checksum = 0x00;

	    /* Now read the header, i.e. until the first comma.  If more than
	     * 10 characters read, break. */
	    i = 0;
	    read ( fpin, &temp, 1 );	
	    checksum = checksum ^ temp;
//printf("%x ",checksum);
	    while ( (temp != ',') && (i < 10) )
	        {
	        header[i] = temp;
	        i++;
	        read ( fpin, &temp, 1 );
	        checksum = checksum ^ temp;
//printf("%x ",checksum);
//fprintf(stderr,"%c",temp);
	        }
	        
	    if (i >= 10)
	    {
	       printf("Bad msg from GPS.\n");
	      }
	      
	    header[i] = '\0';
	    
	    /* Now read the remainder of this messge into readBuff.  Read no
	     * more than MAX_MSG_LENGTH characters.  Read until the '*' character,
	     * which is the last character before the checksum. */
	    memset( readBuff, 0x0, MAX_MSG_LENGTH );
	    for ( i=0; i<MAX_MSG_LENGTH; i++ )
	        {
	        read( fpin, &readBuff[i], 1 );
//fprintf(stderr,"%c",readBuff[i]);
	        if ( readBuff[i] == '*' )
	            break;
	        else
	            {
	            checksum = checksum ^ readBuff[i];
//	            printf("%x ",checksum);
	            /* If there are 2 consecutive commas, insert a '0'. */
	            if ( (readBuff[i] == ',') && (readBuff[i-1] == ',') )
	                {
	                readBuff[i] = '0';
	                i++;
	                readBuff[i] = ',';
	                }
	            }
	        }
	    /* Replace the '*' character with a comma, and read the next
	     * two characters (which will be the hexadecimal checksum. */
	    /* If the character just before the '*' is a comma, insert a '0'. */
	    if ( readBuff[i-1] == ',' )
	        {
	        readBuff[i] = '0';
	        i++;
	        }
	    readBuff[i] = ',';
	    read( fpin, &readBuff[i+1], 1 );
	    read( fpin, &readBuff[i+2], 1 );
//printf("\n%c %c\n", readBuff[i+1],readBuff[i+2]);

	    readBuff[i+3] = '\0';
//printf("%x ",checksum);

//fprintf(stderr,"\n");
	    /* Now interpret the message depending upon header type. */
	    if ( strcmp( header, "GPRMC" ) == 0 )
	        {
	        sscanf( readBuff, "%f,%c,%lf,%c,%lf,%c,%f,%f,%d,%f,%c,%x",
	                &rmc_utc, &rmc_status, &rmc_lat, &rmc_lat_dir, &rmc_long, &rmc_long_dir,
	                &rmc_sog_knot, &rmc_cog_deg, &rmc_date, &rmc_mag_var,
	                &rmc_mag_dir, &msg_checksum );
	        if ( checksum == msg_checksum )
	            {
		    printf("rmc_date %0d rmc_utc %0f\n", rmc_date, rmc_utc);
		        gps2systime (rmc_date, rmc_utc); 
		        break;
	            }
	        }
	    }
	close(fpin);
	return 0;
}

#define DEBUG
#define QNX6

int gps2systime (int utc_date, float utc_time ) {

        time_t curr_local_time;
        time_t curr_utc_time;
        time_t gm_epoch;
        time_t local_epoch;
        struct tm local_time;
        extern long timezone;
        extern int daylight;

        struct timespec update_tp;
        struct tm gps_tm;
        float utc_time_frac;
        float utc_time_int;
        

        curr_local_time = time(NULL);

#ifdef DEBUG
        printf("Current local time = %d string = %s", \
		(int)curr_local_time, ctime( &curr_local_time));
#endif
        memset( &gps_tm, sizeof(struct tm), 0);
        gps_tm.tm_year = utc_date % 100 + 100;
        gps_tm.tm_mon = (utc_date / 100) % 100 - 1;
        gps_tm.tm_mday = utc_date / 10000;
        
        utc_time_frac = modff(utc_time, &utc_time_int);
        gps_tm.tm_hour = ((int)(utc_time_int) / 10000);
        gps_tm.tm_min = (( (int)(utc_time_int) / 100) % 100) ;
        gps_tm.tm_sec = (int)(utc_time_int) % 100;
        curr_utc_time = mktime(&gps_tm);
#ifdef DEBUG
/*	printf("hour %d min %d sec %d\n",gps_tm.tm_hour, gps_tm.tm_min, \
		gps_tm.tm_sec);
*/
        printf("Desired UTC time   = %d string = %s", \
		(int)curr_utc_time, ctime( &curr_utc_time));
#endif
        local_epoch = curr_utc_time;
        gm_epoch = curr_utc_time;
        
        localtime_r( &local_epoch, &local_time );               
#ifdef QNX6
        local_epoch = gm_epoch - 28800 + 3600 ;
#else
        local_epoch = gm_epoch - 28800 + (3600 * local_time.tm_isdst);
#endif
        update_tp.tv_sec = local_epoch;
        update_tp.tv_nsec = (int)utc_time_frac * 1000000000;
       if( abs( (int)curr_local_time - (int)local_epoch ) > 60 ) {
                if( clock_settime(CLOCK_REALTIME, &update_tp) != 0 ) {
                        perror( "ERROR:gps2systime" );
                        return -1;
                        }
                printf("Changing system clock!\n");
                }
        
#ifdef DEBUG
        printf("Final local time   = %d string = %sdst = %d\n", \
	(int)local_epoch, ctime( &local_epoch), local_time.tm_isdst);
        fflush(stdout);
#endif
        
        return 0;
}

