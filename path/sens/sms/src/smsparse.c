/* smsparse.c - parse SMS radar CAN messages
*/
#include <sys_os.h>
#include "sys_rt.h"
#include "local.h"
#include "timestamp.h"
//#include "data_log.h"
#include "db_clt.h"
#include "db_utils.h"
#include <sys/stat.h>
#include "smsparse.h"

#define TRACE		1
#define USE_DATABASE	2
#define MYSQL		4
int output_mask = 0;    // 1 trace, 2 DB server, 4 MySQL

unsigned char debug = 0;

int newsockfd;
db_clt_typ *pclt = NULL;
#define BUFSIZE	1500

static int sig_list[] = 
{
    SIGINT,
    SIGQUIT,
    SIGTERM,
    ERROR,
};

static jmp_buf exit_env;

static void sig_hand( int code )
{
    longjmp( exit_env, code );
}


rcv_msg_hdr_t rcv_msg_hdr;

sensctl_typ sensctl;
objctl_typ objctl;
smsobj_typ object[MAXOBJ]; 

// All variables used in messages are created
static db_id_t db_vars_list[] = {
        {DB_SMS_OBJARR0_VAR, sizeof(smsobjarr_typ)},
        {DB_SMS_OBJARR1_VAR, sizeof(smsobjarr_typ)},
        {DB_SMS_OBJARR2_VAR, sizeof(smsobjarr_typ)},
        {DB_SMS_OBJARR3_VAR, sizeof(smsobjarr_typ)},
        {DB_SMS_OBJARR4_VAR, sizeof(smsobjarr_typ)},
        {DB_SMS_OBJARR5_VAR, sizeof(smsobjarr_typ)},
        {DB_SMS_OBJARR6_VAR, sizeof(smsobjarr_typ)},
        {DB_SMS_OBJARR7_VAR, sizeof(smsobjarr_typ)},
        {DB_SMS_OBJARR8_VAR, sizeof(smsobjarr_typ)},
        {DB_SMS_OBJARR9_VAR, sizeof(smsobjarr_typ)},
        {DB_SMS_OBJARR10_VAR, sizeof(smsobjarr_typ)},
        {DB_SMS_OBJARR11_VAR, sizeof(smsobjarr_typ)},
        {DB_SMS_OBJARR12_VAR, sizeof(smsobjarr_typ)},
        {DB_SMS_OBJARR13_VAR, sizeof(smsobjarr_typ)},
        {DB_SMS_OBJARR14_VAR, sizeof(smsobjarr_typ)},
        {DB_SMS_OBJARR15_VAR, sizeof(smsobjarr_typ)},
        {DB_SMS_CMD_VAR, sizeof(sndmsg_t)},
        {DB_SMS_CMDCTR_VAR, sizeof(int)},
        {DB_SMS_OBJLIST_VAR, sizeof(objlist_typ)},
        {DB_SMS_SENSCTL_VAR, sizeof(sensctl_typ)}
};

#define NUM_DB_VARS    sizeof(db_vars_list)/sizeof(db_id_t)

int main (int argc, char *argv[]) {

    int status = 1;
    int option;
    int retval;
    int index = 0;
    int error=0;
    int tcp_input = 1;     // 1 TCP socket, 0 file or stdin redirect
    char *local_ip = "172.16.1.50";
    char *remote_ip = "172.16.1.200";

    char hostname[MAXHOSTNAMELEN+1];
    char *domain = DEFAULT_SERVICE; //for QNX6, use e.g. "ids"
    int xport = COMM_PSX_XPORT;    //for QNX6, no-op
    
    char buf[BUFSIZE];
    sms_can_msg_t message;
    smscmd_t command;
    char *pcommand;
    int cmdctr = 0;
    int ctr = 0;
    int localcmdctr = 0;
    char tmp = 0;
    sndmsg_t sndmsg;
    int simulation_mode = 0;

    memcpy(&command, &tmp, sizeof(smscmd_t) );
    sprintf(&command.trainseq[0], "%c%c%c%c%c%c%c%c%c",
       0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4);
    command.numpkts = 1;
    command.pktno = 0;
    command.ip[0] = 10;
    command.ip[1] = 0;
    command.ip[2] = 0;
    command.ip[3] = 1;
    command.canno = 0;
    command.CAN_ID = htons(0x03F2);
    command.cmd_data.cmdtarg = 0; 
    command.cmd_data.data[0] = 0;
    command.cmd_data.data[1] = 0;
    command.cmd_data.data[2] = 0;
    command.cmd_data.data[3] = 64;
    command.cmd_data.value = simulation_mode; 
    sprintf(&command.tailseq[0], "%c%c%c%c%c%c%c%c%c",
       0xE7, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7);

    while( (option = getopt(argc, argv, "d:o:vfwshl:r:") ) != EOF ) {
        switch( option ) {
            case 'd' :
                debug = (unsigned char)atoi(optarg);
                break;
            case 'o':
                output_mask = atoi(optarg);    
                break;
            case 'f' :
                tcp_input = 0;
                break;
            case 's' :
                simulation_mode = 4;
                break;
	    case 'l' :
		local_ip = strdup(optarg);
		break;
	    case 'r' :
		remote_ip = strdup(optarg);
		break;
            case 'h' :
            default:
                printf("Usage: %s -d (debug,  1=data receive 2=text to message return\n\t\t\t\t\t4=check head & tail 8=text to message \n\t\t\t\t\t16=sensor control message \n\t\t\t\t\t32=object control message \n\t\t\t\t\t64=parsed data 128=command sent)\n\t\t\t    -o (output mask, 1=trace file 2=use DB 4=MySQL)\n\t\t\t    -f < \"input file\"\n\t\t\t    -s (simulation mode)\n",
			argv[0]);
                exit(EXIT_FAILURE);
                break; 
        }
    }

    if(tcp_input){
        newsockfd = OpenSMSConnection(local_ip, remote_ip);
        if( newsockfd < 0) {
            printf("OpenSMSConnection failed. Exiting...\n");
            exit(EXIT_FAILURE);
            }
        }
    else
        newsockfd = STDIN_FILENO;

    if (debug) {
	fprintf(stderr, "OpenSMSConnection succeeded\n");
	fflush(stderr);
    }
    if (output_mask & USE_DATABASE) {
        get_local_name(hostname, MAXHOSTNAMELEN);
        if ((pclt = db_list_init(argv[0], hostname, domain, xport, 
            db_vars_list, NUM_DB_VARS, NULL, 0)) == NULL) {
                printf("Database initialization error in %s. numvars %d sizeof\
db_vars_list %d sizeof(smsobjarr_t) %d sizeof(sndmsg_t) %d\n", 
		argv[0],NUM_DB_VARS,sizeof(db_vars_list),
		sizeof(smsobjarr_typ), sizeof(sndmsg_t));
                exit(EXIT_FAILURE);
                }
        }

    if( setjmp( exit_env ) != 0 ) {
        if(pclt != 0)
            db_list_done(pclt, db_vars_list, NUM_DB_VARS, NULL, 0);
        close(newsockfd);
	fflush(NULL);
        exit( EXIT_SUCCESS);
        }
    else
        sig_ign( sig_list, sig_hand );

    // Set simulation mode
    if(tcp_input){
    	smscommand( &command, 0, 0, 0, 64, simulation_mode, newsockfd);
    }
    pcommand = (char *)&command;
    for(index=0; index<sizeof(smscmd_t);index++) 
        printf("%#0X ", pcommand[index] & 0xFF);
    printf("\n\n");
    while( error < 10 ) {
       if(tcp_input) {
            if( (retval=recv( newsockfd, buf, BUFSIZE, 0) ) < 0) {
             perror("recv");
             printf("RECV ERROR: %d\n", retval);
             error++;
             continue;
             }
            }
        if(debug & CASE_RCV)
            print_log(CASE_RCV, buf, NULL, NULL, NULL, NULL);
	if( (retval >= 48) && (retval < BUFSIZE) ) {
            if( (status = check_head_tail(retval, &rcv_msg_hdr, buf, debug ) ) 
                == EOF) {
                perror("check_head_tail");
                error++;
                continue;
                }
            for( index = 29; index < (retval - 9); index += 10 ) { 

                if( (status = text_to_msg(index, &message, buf, debug) ) == EOF) 	            {
                    perror("text_to_msg");
                    error++;
                    continue;
                    }
                if(debug & CASE_TEXT_TO_MSG_RETURN)
                    print_log(CASE_TEXT_TO_MSG_RETURN, &message, NULL, NULL, 
                        NULL, NULL);
                parse_msg(&message);
    		if (output_mask & USE_DATABASE) {
                    if( (++ctr % 5) == 0) {
                        if( db_clt_read(pclt, DB_SMS_CMDCTR_VAR, sizeof(int), 
                        	&cmdctr ) == 0) {
                           	printf("SMSPARSE.C:Cannot read SMS command counter\n");
                           	}
                        else {
                            if(cmdctr > localcmdctr) {
                                if( db_clt_read(pclt, DB_SMS_CMD_VAR, 
		        					sizeof(sndmsg_t), &sndmsg) == 0) {
                                    printf("SMSPARSE.C: Cannot read SMS command\n");
                                    ctr = 0;
                                    }
                                else {
                                    localcmdctr = cmdctr;
                                    smscommand( &command, sndmsg.data[0], 
				        			sndmsg.data[1], sndmsg.data[2], 
				        			sndmsg.data[3], 
				        			sndmsg.value, newsockfd);
	    	                    	}
                                }
                            }
	                	}//end of if( (++ctr % 5) == 0)
	            	}//end of if (output_mask & USE_DATABASE)
                }//end of for loop
            }//end of if( (retval >= 48) && (retval < BUFSIZE) )
	else {
	    ++error;
	    printf("RECV: out of range. retval %d\n", retval);
	    }
    }
    perror("high error count:");
    fflush(NULL);
    return -1;
}

int parse_msg(sms_can_msg_t *message) {
    
    smsobjarr_typ smsarr[SMSARRMAX];
    unsigned char obj;
    int arr;
    int arrmember;
    static char objindex = 0;
    static objlist_typ objlist;
    struct timespec tmspec;
    struct tm *timestamp;
    static FILE *tracefd;
    
        switch(message->CAN_ID) {
            case 0x500:
                 sensctl.bump_d_net = message->data[0];
                 sensctl.canstat = message->data[1];
                 sensctl.sensorspres =
                    (unsigned short) ( (message->data[2] & 0X00FF) |
                    (    (message->data[3] << 8) & 0X0F00)
                    );
                 sensctl.enetstat = (message->data[3] >> 4) & 0XFF;
                 sensctl.timestamp =
                    (unsigned int)( (message->data[4] & 0X000000FF) + //timestamp 1
                    ((message->data[5] <<  8) & 0X0000FF00) + //timestamp 2
                    ((message->data[6] << 16) & 0X00FF0000) + //timestamp 3
                    ((message->data[7] << 24) & 0XFF000000)   //timestamp 4
                    );
                 if(debug & CASE_SENSOR_CTL)
                    print_log(CASE_SENSOR_CTL, message, &sensctl, NULL, NULL, NULL);
		if(output_mask & USE_DATABASE)
			db_clt_write(pclt, DB_SMS_SENSCTL_VAR, 
				sizeof(sensctl_typ), &sensctl); 
			
                 break;
            case 0x501:
		memset( &objlist, 0, sizeof(objlist_typ));
		objindex = 0;
		get_current_timestamp(&objlist.ts);	// start of object list
		objctl.no_obj = message->data[0]; // number of objects
		objlist.num_obj =objctl.no_obj; // number of objects
//printf("objlist.num_obj set to %d\n", objlist.num_obj);
//
		objctl.no_msgs = message->data[1]; // number of messages
		objctl.t_scan = message->data[2]; // this cycle time (ms)
		objlist.t_scan = objctl.t_scan;
		objctl.mode = message->data[3];//mode (0=normal tracking,
                		             //      1=simulation)
		objlist.mode = objctl.mode; 
		objctl.cyc_cnt =
                    (unsigned int)( (message->data[4] & 0X000000FF) + //curr cyc cnt B0
                    ((message->data[5] <<  8) & 0X0000FF00) + //byte 1
                    ((message->data[6] << 16) & 0X00FF0000) + //byte 2
                    ((message->data[7] << 24) & 0XFF000000)   //byte 3
                    );
		objlist.cyc_cnt = objctl.cyc_cnt;
                if(debug & CASE_OBJECT_CTL) {
		    sms_print_obj_list(message, &objctl, &objlist);
//                    print_log(CASE_OBJECT_CTL, message, &objctl, NULL, NULL, NULL);
		}
                break;
            case 0x03F2:
#define READ_INT	8
#define READ_FLOAT	9
                printf("cmd_type: command message type\n");
		if(message->data[2] == READ_INT)
			printf("READPARM: Parameter %d integer value is %d\n", message->data[1], 
                    	(int)( (message->data[4] & 0X000000FF) +  //integer parameter byte 1
                    	((message->data[5] <<  8) & 0X0000FF00) + //integer parameter byte 2
                    	((message->data[6] << 16) & 0X00FF0000) + //integer parameter byte 3
                    	((message->data[7] << 24) & 0XFF000000) ) //integer parameter byte 4
                    	);
		else
		if(message->data[2] == READ_FLOAT) {
			printf("READPARM: Parameter %d float value is %f\n", message->data[1],
                    	(float)( (message->data[4] & 0X000000FF) + //float parameter byte 1
                    	((message->data[5] <<  8) & 0X0000FF00) +  //float parameter byte 2
                    	((message->data[6] << 16) & 0X00FF0000) +  //float parameter byte 3
                    	((message->data[7] << 24) & 0XFF000000) )  //float parameter byte 4
                    	);
			printf("READPARM: Parameter %d bytes are %#x %#x %#x %#x\n", message->data[1],
                    	message->data[7] , //float parameter byte 1
                    	message->data[6] ,  //float parameter byte 2
                    	message->data[5] ,  //float parameter byte 3
                    	message->data[4]    //float parameter byte 4
                    	);
		}
		else
			printf("READPARM: Could not read parameter\n");	
		break;
            default:
                if( message->CAN_ID >= 0x510 && message->CAN_ID <= 0x5FF ) {

                    obj = (unsigned char)(((message->data[7]) >> 2) & 0X3F);
                    arr = obj / MAXDBOBJS;
                    arrmember = obj % MAXDBOBJS;

					//Get timestamp
                    if( clock_gettime(CLOCK_REALTIME, &tmspec) < 0 ) {
                    	perror("timestamp");
                        }
                    else {
                    	timestamp = localtime( (time_t *)&tmspec.tv_sec );
                   		smsarr[arr].object[arrmember].smstime.hour = timestamp->tm_hour;
                    	smsarr[arr].object[arrmember].smstime.min = timestamp->tm_min;
                    	smsarr[arr].object[arrmember].smstime.sec= timestamp->tm_sec;
                    	smsarr[arr].object[arrmember].smstime.millisec =  tmspec.tv_nsec/1000000;
                    	}

		    // doesn't matter if smsarr is ever cleared,
		    // only object IDs in the object list arrays
		    // are valid, and new data is written only for them 
		    objlist.arr[(unsigned char) objindex++] = obj;
                    smsarr[arr].object[arrmember].obj = obj;
                    smsarr[arr].object[arrmember].xrange =
                        0.1 * ((short)((message->data[0]) |
                              ((message->data[1] & 0X3F) << 8)) -
                              8192);
                    smsarr[arr].object[arrmember].yrange =
                        0.1 * ((short)(((message->data[1] & 0XC0) >> 6) |
                              (message->data[2] << 2) |
                              ((message->data[3] & 0X0F) << 10)) -
                              8192);
                    smsarr[arr].object[arrmember].xvel =
                        0.1 * ((short)(((message->data[3] & 0XF0) >> 4) |
                              (((message->data[4] & 0X7F))  << 4)) -
                              1024);
                    smsarr[arr].object[arrmember].yvel =
                        0.1 * ((short)(((message->data[4] & 0X80) >> 7) |
                              (((message->data[5])) << 1) |
                              (((message->data[6] & 0X03)) << 9)) -
                              1024);
                    smsarr[arr].object[arrmember].length =
                        0.2 * ((short)(((message->data[6] >> 2) & 0X3F) |
                              ((message->data[7] << 6) & 0XC0))
                              );
                    if(debug & CASE_OBJECT_DATA) {
			sms_print_obj_data(message, obj, 
				&smsarr[0],
				arr, arrmember, objindex);
//                	    print_log(CASE_OBJECT_DATA, &obj, &smsarr[arr], &arr, &arrmember, message);
		    }
		    // to keep cyc_cnt consistent for all members in
		    // a given DB VAR, don't write to DB server
		    // until end of list, and then write valid
		    // DB VARs with all their members at the same time
		    if(objindex >= objctl.no_obj - 1) { 
			if(output_mask & USE_DATABASE) {
			    int i;
			    for (i = 0; i < objctl.no_obj; i++) {
				int sms_obj_id = objlist.arr[i];  
				int db_arr = sms_obj_id / MAXDBOBJS; 
				int db_var = DB_VAR_SMS_OBJ(sms_obj_id);
				
				smsarr[db_arr].cyc_cnt = objctl.cyc_cnt;
				// may get written more than once
				db_clt_write(pclt, db_var,
					sizeof(smsobjarr_typ), &smsarr[db_arr]);
			    }
        	    	    db_clt_write(pclt, DB_SMS_OBJLIST_VAR, 
					sizeof(objlist_typ), &objlist); 
			}
		    }
                    if(output_mask & TRACE) {

                        if( tracefd == NULL ) {
                            tracefd = fopen("sms.out","w");
                            if( tracefd == NULL ) {
                                perror("trace file:");
                                exit(EXIT_FAILURE);
                                }
                            }
                            fprintf( tracefd,"%2.2d:%2.2d:%2.3f %4d%2.2d%2.2d ",
                                timestamp->tm_hour,
                                timestamp->tm_min,
                                timestamp->tm_sec + tmspec.tv_nsec/1000000000.0,
                                timestamp->tm_year + 1900,
                                timestamp->tm_mon + 1,
                                timestamp->tm_mday
                                );
                            fprintf(tracefd, "%2.2d %4.1f %4.1f %4.1f %4.1f %4.1f ",
                                obj,
                                smsarr[arr].object[arrmember].xrange,
                                smsarr[arr].object[arrmember].yrange,
                                smsarr[arr].object[arrmember].xvel,
                                smsarr[arr].object[arrmember].yvel,
                                smsarr[arr].object[arrmember].length
                                );
			    			fprintf(tracefd, "%#0x %#0x %#0x %d %d %u %d %u\n",
								sensctl.enetstat & 0xFF,
								sensctl.canstat & 0xFF,
								sensctl.sensorspres & 0xFFF,
								objctl.no_obj,
								objctl.no_msgs,
								objctl.t_scan & 0xFF,
								objctl.mode,
								objctl.cyc_cnt
								); 
                    }


				}
                else
                    if(debug)
                        printf("EXT_CAN: Unknown message type %#x (default \
case)\n", message->CAN_ID);
                break;
        }
    return 0;
}
