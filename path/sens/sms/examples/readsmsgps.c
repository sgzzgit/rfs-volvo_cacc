/* Reads database to get object data from SMS radar and GPS data from mobile 
** SOBU-based GPS and saves to file.  
*/

#include <sys_os.h>
#include "smsparse.h"
#include "timestamp.h"
#include "path_gps_lib.h"
#include "sys_rt.h"
#include "db_clt.h"
#include "db_utils.h"
#include "gpio.h"
#include <time.h>

#define STDOUT	1
#define TRACE	2

static int sig_list[] = 
{
    SIGINT,
    SIGQUIT,
    SIGTERM,
    SIGALRM,
    ERROR,
};

static jmp_buf exit_env;

static void sig_hand( int code )
{
	if(code == SIGALRM)
		return;
    longjmp( exit_env, code );
}

static int trig_list[] = {
	DB_SMS_OBJARR0_VAR,
	DB_SMS_OBJARR1_VAR, 
	DB_SMS_OBJARR2_VAR, 
	DB_SMS_OBJARR3_VAR, 
	DB_SMS_OBJARR4_VAR, 
	DB_SMS_OBJARR5_VAR, 
	DB_SMS_OBJARR6_VAR, 
	DB_SMS_OBJARR7_VAR, 
	DB_SMS_OBJARR8_VAR, 
	DB_SMS_OBJARR9_VAR, 
	DB_SMS_OBJARR10_VAR, 
	DB_SMS_OBJARR11_VAR, 
	DB_SMS_OBJARR12_VAR,
	DB_SMS_OBJARR13_VAR, 
	DB_SMS_OBJARR14_VAR, 
	DB_SMS_OBJARR15_VAR
};

#define NUM_TRIG_VARS    sizeof(trig_list)/sizeof(int)

int main (int argc, char *argv[]) {

    db_clt_typ *pclt = NULL;
    char hostname[MAXHOSTNAMELEN+1];
    char *domain = DEFAULT_SERVICE; //for QNX6, use e.g. "ids"
    int xport = COMM_PSX_XPORT;    //for QNX6, no-op
    trig_info_typ trig_info;        /* Trigger info */
    int recv_type;

    int output_mask = 0;    // 1=stdout, 2=trace file

    smsobj_typ my_sms_array[65];  // smsobj_typ defined in smsparse
    int k;
    
    // each one has 5 objects; defined in smsparse.h
    smsobjarr_typ sms_arr[SMSARRMAX];
    int group_num;       // counts up to 13 groups of 5 each
    int obj_num;         // counts objects in a group
    objlist_typ objlist;
    int objlstidx;

    path_gps_point_t veh_gps;
    unsigned char digin;
    int option;
    FILE *tracefd;
    char *filename;
    struct tm tm;
    struct tm *ptm = &tm;
    time_t mytime;

    /* Read and interpret any user switches. */
    while ( (option = getopt( argc, argv, "o:" )) != EOF ) {
        switch( option ) {
            case 'o':
                output_mask = atoi(optarg);
                break;
            default:
                printf("USAGE: %s -o <output mask, 1=stdout, 2=trace file>\n",
			argv[0]);
                exit( EXIT_FAILURE );
            }
        }

    get_local_name(hostname, MAXHOSTNAMELEN);

    if ((pclt = db_list_init(argv[0], hostname, domain, xport, 
        NULL, 0, trig_list, NUM_TRIG_VARS)) == NULL) {
            printf("Database initialization error in %s.\n",  argv[0]);
            exit(EXIT_FAILURE);
            }

    if(output_mask & TRACE) {
	mytime = time(NULL);
	ptm = localtime(&mytime);
	sprintf(filename,"/big/data/readsmsgps.%02d-%02d-%02d_%02d:%02d:%02d",
		ptm->tm_mday, ptm->tm_mon, ptm->tm_year,
		ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
    	tracefd = fopen(filename,"w");
    	if(tracefd == NULL) {
    		printf("Couln't open trace file. Exiting....");
    		exit(EXIT_FAILURE);
    		}
    	}
    if(output_mask & STDOUT) {
		tracefd = stdout;
		}
    if( setjmp( exit_env ) != 0 ) {
        db_list_done( pclt, NULL, 0, trig_list, NUM_TRIG_VARS );

        if(tracefd)
        	fclose(tracefd);
        exit( EXIT_SUCCESS );
        }
    else
        sig_ign( sig_list, sig_hand );

    while(1) {
	recv_type = clt_ipc_receive(pclt, &trig_info, sizeof(trig_info));

	// read list of SMS object ids
	if( db_clt_read(pclt, DB_SMS_OBJLIST_VAR, sizeof(objlist), 
		&objlist) == 0) {
		printf("READSMSGPS: Cannot read objlist\n");
		}

    	// read SMS data from data bucket and assign to flattened array
    	k = 0; // increment after each assignment
    	for (group_num = 0; group_num < SMSARRMAX; group_num++) {
    	    db_clt_read(pclt, DB_SMS_OBJARR0_VAR + group_num,
    	        sizeof(smsobjarr_typ), &sms_arr[group_num]);
    	    for (obj_num = 0; obj_num < MAXDBOBJS; obj_num++) {
    	         my_sms_array[k]= sms_arr[group_num].object[obj_num];
    	         k++;
    	         }
    	    }

    	// read GPS data from mobile unit
    	db_clt_read(pclt, DB_GPS_PT_RCV_VAR,
    	    sizeof(path_gps_point_t), &veh_gps);
        if (db_clt_read( pclt, DB_DIG_IN_VAR,
            sizeof(char), &digin) == FALSE)
            fprintf( stderr, "READSMSGPS:clt_read( DB_DIG_IN_VAR ) failed.\n");



    	if(output_mask & TRACE) {
    		fprintf(tracefd, "%2.2d:%2.2d:%2.3f %2.2d:%2.2d:%2.3f ",
    			veh_gps.local_time.hour,
    			veh_gps.local_time.min,
    			veh_gps.local_time.sec +
    			veh_gps.local_time.millisec/1000.0,
    			veh_gps.utc_time.hour,
    			veh_gps.utc_time.min,
    			veh_gps.utc_time.sec +
    			veh_gps.utc_time.millisec/1000.0
    			);
    		fprintf(tracefd, "%.7lf %.7lf %.7f %u %d ",
    			veh_gps.longitude, veh_gps.latitude, 
    			veh_gps.heading, digin & 0xFC, objlist.num_obj);
            fprintf( tracefd,"%2.2d:%2.2d:%2.3f ",
            	my_sms_array[objlist.arr[0]].smstime.hour,
            	my_sms_array[objlist.arr[0]].smstime.min,
            	my_sms_array[objlist.arr[0]].smstime.sec + 
            	my_sms_array[objlist.arr[0]].smstime.millisec/1000.0 
            	);	

			for(objlstidx = 0; objlstidx < objlist.num_obj; objlstidx++) {
				fprintf(tracefd, "%d %4.1f %4.1f %4.1f %4.1f %4.1f ",
					(char)(my_sms_array[objlist.arr[objlstidx]].obj & 0XFF),
					my_sms_array[objlist.arr[objlstidx]].xrange,
					my_sms_array[objlist.arr[objlstidx]].yrange,
					my_sms_array[objlist.arr[objlstidx]].xvel,
					my_sms_array[objlist.arr[objlstidx]].yvel,
					my_sms_array[objlist.arr[objlstidx]].length
    	            );
    	        }
    	    fprintf(tracefd,"\n");
    	    }
    	}//end of while loop
}
