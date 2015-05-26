/**\file 
 * evt300a.c   Reads message type 82 (or 89) (FE target report) from
 *                   Eaton Vorad
 *                   EVT-300 radar #1 and saves data in the database
 *
 *
 * Copyright (c) 2006   Regents of the University of California
 *
 *  Process to communicate with the Eaton Vorad EVT-300 radar.  This process
 *  reads the message from the radar and puts the data into the database.
 *
 *  The signals SIGINT, SIGQUIT, and SIGTERM are trapped, and cause the
 *  process to terminate.  Upon termination log out of the database.
 *
 */
 
#undef DEBUG_EVT300 /* Change to #define DEBUG_EVT300 for printing status, etc. */
 
#include <sys_os.h>
#include "db_clt.h"
#include "db_utils.h"
#include "clt_vars.h"
#include "silver_truck.h"
#include "silver_evt300.h"
#include "sys_rt.h"
#include "silver_ser_dev.h"

int fpin;
FILE *fpout;	
jmp_buf env;

static void sig_hand( int sig);
static bool_typ ser_driver_read( gen_mess_typ *pMessagebuff);
static int sig_list[] = {
	SIGINT,
	SIGQUIT,
	SIGTERM,
	ERROR,
    };

int status;

#if 0
db_clt_typ *pclt;              /* Database client pointer */
db_data_typ db_data;
#endif

gen_mess_typ now_message;

#define DRV_ID_REQUEST_PER	1 /* no. seconds since last id request */
#define DRV_ID_REQUEST		5

static char CheckSum( gen_mess_typ message)
{
	char sum = 0;
	int i;
	char zero =0;

	for( i=0; i< 59; i++)
		sum += message.data[i];
	sum = zero - sum;

/* Print error if checksum is not correct */
#ifdef DEBUG_EVT300
	if (sum != message.data[59])
	{
	printf("checksum error:  ");
	for(i=0; i<60; i++)
		printf("%x ",message.data[i]);
	printf("\n");
	}
#endif
	return (sum);
}


int main( int argc, char *argv[] )
{
	gen_mess_typ    readBuff;
	char hostname[MAXHOSTNAMELEN+1];

	evt300_mess_typ 	*pevt300_mess;
	ddu_disp_mess_typ 	*pddu_disp_mess;
	driver_ID_mess_typ 	*pdriver_ID_mess;

	long_evradar_typ 	long_radar;
	ddu_display_typ 	ddu_display;
	struct timeb       	timeptr_raw;
	struct tm          	time_converted;
	int option;
	char *domain = "123.32.234.149";
	char *port = EVT300A_SER_PORT;
    char zero = 0;
	char five = 5;
	char drv_id_request[2] = {five, zero - five};
	time_t last_time = 0;

	while ( (option = getopt( argc, argv, "d:p:i:" )) != EOF )
        {
             switch( option )
             {
             case 'd':
                domain = strdup(optarg);
                break;
             case 'p':
                port = strdup(optarg);
                break;
             default:
                fprintf(stderr, "option not recognized\n");
                exit(1);
                break;
             }
	}

#if 0
	/* Log in to the database (shared global memory).  Default to the
	 * the current host. */
	/* Initialize the database. */
	get_local_name(hostname, MAXHOSTNAMELEN);
	printf("hostname = %s\n", hostname);
	if (( pclt = clt_login( argv[0], hostname, DEFAULT_SERVICE, COMM_PSX_XPORT)) == NULL )
	{
		printf("Database initialization error in evt300b.\n");
		exit (EXIT_FAILURE);
	}
	printf("pclt 0x%x, hostname %s , DEFAULT_SERVICE %s COMM_PSX_XPORT %d argv[0] %s\n", (int) pclt, hostname, (char *)DEFAULT_SERVICE, (int)COMM_PSX_XPORT, argv[0]);
	
#endif
	/* Initialize serial port. */  
	fpin = open( port,  O_RDONLY );
	if ( fpin <= 0 ) {
		printf( "Error opening device %s for input\n", port );
	}
	else {
		printf( "Device %s opened for input\n", port );
	}

	fpout = fopen( port, "w" );
	if ( fpout <= 0 ) {
		printf( "Error opening device %s for output\n", port );
	}
	else {
		printf( "Device %s opened for output\n", port );
	}

	/*	set jmp */
	if ( setjmp(env) != 0)
	    {
	    /* Log out from the database. */
#if 0
	    if (pclt != NULL)
	        clt_logout( pclt );
#endif

		exit( EXIT_SUCCESS);
	}
	else 
		sig_ign( sig_list, sig_hand);
      
   memset( &ddu_display, 0x0, sizeof(ddu_display_typ) );

	for (;;)
	    {

		if ( ser_driver_read ( &readBuff ))
	        {
	        switch ( readBuff.data[0] )
	            {
	            case 2:
	                /* Get time of day and save in the database. */
	                ftime ( &timeptr_raw );
			////// _localtime changed to localtime_r in qnx6 /////
	                //_localtime ( &timeptr_raw.time, &time_converted );

	                localtime_r ( &timeptr_raw.time, &time_converted );
	                ddu_display.hour = time_converted.tm_hour;
	                ddu_display.min = time_converted.tm_min;
	                ddu_display.sec = time_converted.tm_sec;
	                ddu_display.millisec = timeptr_raw.millitm;

	                /* Move data from message into database.  */
	               pddu_disp_mess = (ddu_disp_mess_typ *) &readBuff;
      	           ddu_display.power_on = (pddu_disp_mess->light_ctl & PWR_ON) >> 1;
                   ddu_display.sys_fail = (pddu_disp_mess->light_ctl & SYS_FAIL) >> 2;
                   ddu_display.smart_cruise = (pddu_disp_mess->light_ctl & SMART_CRUISE) >> 3;
                   ddu_display.barcode = (pddu_disp_mess->light_ctl & BARCODE) >> 4;
                   ddu_display.targ_det = (pddu_disp_mess->light_ctl & TARG_DET) >> 5;
                   ddu_display.alert1 = (pddu_disp_mess->light_ctl & ALERT1) >> 6;
                   ddu_display.alert2 = (pddu_disp_mess->light_ctl & ALERT2) >> 7;

                   ddu_display.audio_tone_sel = pddu_disp_mess->audio_ctl & TONE_SEL_MASK;
                   ddu_display.start_tone = (pddu_disp_mess->audio_ctl & START_TONE_MASK) >> 7;

                   switch(ddu_display.audio_tone_sel) {
                       case 0:
                           ddu_display.dwnld_tone = 1;
                           break;
                       case 1:
                           ddu_display.creep_alrt = 1;
                           break;
                       case 2:
                           ddu_display.alert1_2_sec = 1;
                           break;
                       case 3:
                           ddu_display.alert2_1_sec = 1;
                           break;
                       case 4:
                           ddu_display.vol_chg = 1;
                           break;
                       case 8:
                           ddu_display.drv_id_ok = 1;
                           break;
                       case 9:
                           ddu_display.drv_id_fail = 1;
                           break;
                       case 10:
                           ddu_display.drv_id_gone = 1;
                           break;
                       case 11:
                           ddu_display.dbg_stat = 1;
                           break;
                       case 12:
                           ddu_display.dbg_slow = 1;
                           break;
                       default:
                           break;
                   }
#ifdef DEBUG_EVT300
                   printf("EVT300: alert1 %d alert2 %d tone alert1 %d tone alert2 %d start tone %d\n", ddu_display.alert1, ddu_display.alert2, ddu_display.alert1_2_sec, ddu_display.alert1_2_sec, ddu_display.start_tone);
                   printf("Driver ID ddu = %s ID status = %#x\n",
			   ddu_display.drv_ID, ddu_display.status );
#endif
#if 0
	                if( clt_update( pclt, DB_DDU_DISPLAY_VAR,
	                    DB_DDU_DISPLAY_TYPE, sizeof( ddu_display_typ ),
	                    (void *)&ddu_display ) == FALSE )
	                   fprintf( stderr, "error in clt_update( DB_DDU_DISPLAY_VAR )\n" );
#endif
	                break;

	            case 82:
	            case 89:
	                /* Get time of day and save in the database. */
	                ftime ( &timeptr_raw );
			////// _localtime changed to localtime_r in qnx6 /////
	                //_localtime ( &timeptr_raw.time, &time_converted );
	                localtime_r ( &timeptr_raw.time, &time_converted );
	                long_radar.hour = time_converted.tm_hour;
	                long_radar.min = time_converted.tm_min;
	                long_radar.sec = time_converted.tm_sec;
	                long_radar.millisec = timeptr_raw.millitm;

	                /* Move data from message into database.  */
	                pevt300_mess = (evt300_mess_typ *) &readBuff;
	                long_radar.mess_ID = pevt300_mess->msgID;
	                long_radar.target_1_id = pevt300_mess->target_1_id;
	                long_radar.target_1_range = pevt300_mess->target_1_range;
	                long_radar.target_1_rate = pevt300_mess->target_1_rate;
	                long_radar.target_1_azimuth = pevt300_mess->target_1_azimuth;
	                long_radar.target_1_mag = pevt300_mess->target_1_mag;
	                long_radar.target_1_lock = pevt300_mess->target_1_lock;
	                long_radar.target_2_id = pevt300_mess->target_2_id;
	                long_radar.target_2_range = pevt300_mess->target_2_range;
	                long_radar.target_2_rate = pevt300_mess->target_2_rate;
	                long_radar.target_2_azimuth = pevt300_mess->target_2_azimuth;
	                long_radar.target_2_mag = pevt300_mess->target_2_mag;
	                long_radar.target_2_lock = pevt300_mess->target_2_lock;
	                long_radar.target_3_id = pevt300_mess->target_3_id;
	                long_radar.target_3_range = pevt300_mess->target_3_range;
	                long_radar.target_3_rate = pevt300_mess->target_3_rate;
	                long_radar.target_3_azimuth = pevt300_mess->target_3_azimuth;
	                long_radar.target_3_mag = pevt300_mess->target_3_mag;
	                long_radar.target_3_lock = pevt300_mess->target_3_lock;
	                long_radar.target_4_id = pevt300_mess->target_4_id;
	                long_radar.target_4_range = pevt300_mess->target_4_range;
	                long_radar.target_4_rate = pevt300_mess->target_4_rate;
	                long_radar.target_4_azimuth = pevt300_mess->target_4_azimuth;
	                long_radar.target_4_mag = pevt300_mess->target_4_mag;
	                long_radar.target_4_lock = pevt300_mess->target_4_lock;
	                long_radar.target_5_id = pevt300_mess->target_5_id;
	                long_radar.target_5_range = pevt300_mess->target_5_range;
	                long_radar.target_5_rate = pevt300_mess->target_5_rate;
	                long_radar.target_5_azimuth = pevt300_mess->target_5_azimuth;
	                long_radar.target_5_mag = pevt300_mess->target_5_mag;
	                long_radar.target_5_lock = pevt300_mess->target_5_lock;
	                long_radar.target_6_id = pevt300_mess->target_6_id;
	                long_radar.target_6_range = pevt300_mess->target_6_range;
	                long_radar.target_6_rate = pevt300_mess->target_6_rate;
	                long_radar.target_6_azimuth = pevt300_mess->target_6_azimuth;
	                long_radar.target_6_mag = pevt300_mess->target_6_mag;
	                long_radar.target_6_lock = pevt300_mess->target_6_lock;
	                long_radar.target_7_id = pevt300_mess->target_7_id;
	                long_radar.target_7_range = pevt300_mess->target_7_range;
	                long_radar.target_7_rate = pevt300_mess->target_7_rate;
	                long_radar.target_7_azimuth = pevt300_mess->target_7_azimuth;
	                long_radar.target_7_mag = pevt300_mess->target_7_mag;
	                long_radar.target_7_lock = pevt300_mess->target_7_lock;
#if 0
	                if( clt_update( pclt, DB_LONG_EVRADAR_VAR,
	                    DB_LONG_EVRADAR_TYPE, sizeof( long_evradar_typ ),
	                    (void *)&long_radar ) == FALSE )
	                   fprintf( stderr, "error in clt_update( DB_LONG_EVRADAR_VAR )\n" );
#endif
	                break;
		         case 67:
                   pdriver_ID_mess = (driver_ID_mess_typ *) &readBuff;
                   ddu_display.status = pdriver_ID_mess->status;
		   ddu_display.drv_ID[0] = '\0';
		   if (ddu_display.status & 0x40) { 
			   int j;
		           int id_len = ddu_display.status & 0x07;
			   for (j =0; j < id_len; j++)
				ddu_display.drv_ID[j] =
					       	pdriver_ID_mess->drv_ID[j];
			   ddu_display.drv_ID[j] = '\0';
		    }			   
#ifdef DEBUG_EVT300
                   printf("Driver ID = %s ID status = %#x\n",
                   ddu_display.drv_ID, ddu_display.status );
#endif
                   break;
               default :
                   printf("readBuff.data[0] = %d\n", readBuff.data[0]);
                   break;
               }
	        }
               if(timeptr_raw.time - last_time > DRV_ID_REQUEST_PER ) {
                   last_time = timeptr_raw.time;
                   if( (fwrite( &drv_id_request, 2, 1, fpout)) <=0 ) {
                      printf("Error on driver id request\n");
                      }
#ifdef DEBUG_EVT300
                   printf("Sent driver id request %#x\n", strtod(drv_id_request, NULL));
#endif
                   fflush(fpout);
                   }

		}
}

static bool_typ ser_driver_read( gen_mess_typ *pMessagebuff) 
{
unsigned char msgbuf [134];
int i, ii, index, id_len;
unsigned char csum;
unsigned char dummy;
static int csum_err;

	/* Read from serial port. */
	/* Blocking read is used, so control doesn't return unless data is
	 * available.  Keep reading until the beginning of a FE target
	 * message (type 82 or 89) is detected. */
	memset( msgbuf, 0x0, 134 );
	while ((msgbuf[0] != 82) && (msgbuf[0] != 89)
               && (msgbuf[0] != 67) && (msgbuf[0] != 2))
	    {
	    read ( fpin, &msgbuf[0], 1);
          printf("%x \n",msgbuf[0]);

	    /* The EVT300 CPU seems to send a lot of message types 34 and 36.
	     * Since (hopefully) once we're synchronized with the radar we'll
	     * stay that way the byte we just read should have been the first
	     * byte of a new message.  If it was a 34 or 36, read enough bytes
	     * for that type of message.  Otherwise print out message type (to
	     * figure out what's happening).
	     */

	    switch ( msgbuf[0] )
	        {
	        case 2:
		    read ( fpin, &msgbuf[1], 1);
		    read ( fpin, &msgbuf[2], 1);
                    break;
	        case 82:
	        case 89:
		    /* FE target message.  Now read next two bytes which are FFT
		     * frame number and number of targets (0-7). */
		    read ( fpin, &msgbuf[1], 1);
		    read ( fpin, &msgbuf[2], 1);
                  printf(" %d %d",msgbuf[1],msgbuf[2]);
		
		    /* If target count is not 0-7 this is not really a target 
                       message.
		     * Take an error exit. */
		    if (msgbuf[2] > 7)
		        return (FALSE);

		    /* Now read data for as many targets as exist. */
		    if ( msgbuf[2] > 0 ) {
		        index = 3;
		        for ( i=0; i<msgbuf[2]; i++ ) {
		            /* Read 8 bytes per target. */
		            for ( ii=0; ii<8; ii++ ) {
		                read ( fpin, &msgbuf[index], 1);
                              printf(" %d",msgbuf[index]);
		                index++;
		                }
	                    }
	        	}
	            break;
	        case 34:
	            /* Message type 34.  Read and ignore the next 2 characters. */
	            read ( fpin, &dummy, 1 );
	            read ( fpin, &dummy, 1 );
	            break;
	        case 36:
	            /* Message type 36.  Read and ignore the next 6 characters. */
	            for ( i=0; i<6; i++ )
	                read ( fpin, &dummy, 1 );
	            break;
           case 67:
               read ( fpin, &msgbuf[1], 1);
               id_len = msgbuf[1] & 0x07;
               for(i = 0; i < id_len; i++){
                   read ( fpin, &msgbuf[2 + i], 1);
               }
#ifdef DEBUG_EVT300
               printf("Got driver ID: ");
               for(i = 0; i < id_len; i++){
                    printf("%c", msgbuf[2 + i] );
               }
               printf(" len = %d\n", id_len);
	       printf(" msgbuf[1] 0x%x\n", msgbuf[1]);
#endif
	            break;
	        default:
	            printf( "Radar 1 ID: %d\n", msgbuf[0] );
	            break;
	        }
	    }
     /*     
     fprintf(stderr, "msg found, type: %d\n",msgbuf[0]);
     */
	/* Now read the checksum.  Always save it in msgbuf[59], regardless
	 * of message type or how many targets are in a FE target message.
	 * This will work just fine,
	 * since the entire buffer was zeroed before reading any data. */

	read ( fpin, &msgbuf[59], 1);
      printf(stderr, " %x\n",msgbuf[59]);

	memcpy( pMessagebuff->data, &msgbuf[0], 60);

	csum = CheckSum( *pMessagebuff );
	if( csum != msgbuf[59])
	    {
	    printf( "Checksum error %d, radar 1.\n", ++csum_err);
	    return (FALSE);
	    }
	else
	    {
	    return (TRUE);
	    }
}

static void sig_hand( int sig)
{
	longjmp( env, 1);
}
