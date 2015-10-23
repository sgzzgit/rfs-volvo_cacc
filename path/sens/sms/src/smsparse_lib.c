/* smsparse_lib.c - function definitions used in smsparse.c, the SMS radar
** database client.
*/

#include <sys_os.h>
#include "smsparse.h"

int OpenSMSConnection(char *local_ip, char* remote_ip){

    int sockfd;
    int newsockfd;
    int backlog = 1;
    struct sockaddr_in local_addr;
    socklen_t localaddrlen = sizeof(local_addr);
    struct sockaddr_in remote_addr;
    unsigned short rcv_port = 2946;

    // Open connection to SMS subnetwork controller
    sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if( sockfd < 0 ) {
        perror("socket");
        return -1;
    }

    /** set up local socket addressing and port */
    memset(&local_addr, 0, sizeof(struct sockaddr_in));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr =  inet_addr(local_ip);//htonl(INADDR_ANY);//
    local_addr.sin_port = htons(rcv_port);

    if( (bind(sockfd, (struct sockaddr *)&local_addr, 
        sizeof(local_addr) ) ) < 0) {
        perror("bind");
        close(sockfd);
        return -2;
    }
    
    if( (listen(sockfd, backlog )) < 0) {
        perror("listen");
        close(sockfd);
        return -3;
    }
   
    /** set up remote socket addressing and port */
    memset(&remote_addr, 0, sizeof(struct sockaddr_in));
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_addr.s_addr = inet_addr(remote_ip);//htonl(INADDR_ANY);
    remote_addr.sin_port = htons(rcv_port);
    localaddrlen = sizeof(remote_addr);

    if( (newsockfd = accept(sockfd, (struct sockaddr *)&remote_addr, 
	&localaddrlen) ) < 0 ) { 
        perror("accept");
        close(sockfd);
        return -4;
        };
         close(sockfd);
   return newsockfd;
}

int CloseSMSConnection(int sockfd){
    close(sockfd);
    return 0;
}

// Not on separate line, add "\n" if not part of other debug 
void sms_print_raw_can(sms_can_msg_t *pmsg)
{            
	printf(" CAN %04hx %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx ",
		pmsg->CAN_ID,
		pmsg->data[0], //src ID
		pmsg->data[1], //CAN stat
		pmsg->data[2], 
		pmsg->data[3], //Ethernet state
		pmsg->data[4],
		pmsg->data[5],
		pmsg->data[6],
		pmsg->data[7]
	);
}

// Prints essential objctl data with timestamp when objlist type is
// initialized, checks that data is the same in the two structures, 
// and for other consistency errors, silent if OK
void sms_print_obj_list(sms_can_msg_t *pmsg, objctl_typ *pctl, objlist_typ *plist)
{
	printf("objctl/list: ");	// to distinguish this line from others
	print_timestamp(stdout, &plist->ts); 	// essential part of trace 
	sms_print_raw_can(pmsg);
	printf(" #objs %hhd cycle cnt %d ",
              pctl->no_obj, // number of objects
              pctl->cyc_cnt     //current cycle count byte 0
	);
	if (pctl->no_obj != plist->num_obj)
		printf(" NUMOBJ doesn't match ");
	if (pctl->t_scan != 48)
		printf("tscan NOT 48 ms, %hhd ms ", pctl->t_scan);
	if (pctl->no_msgs != 1)
		printf("number of messages NOT 1, %hhd ", pctl->no_msgs);
	if (pctl->mode != 0)
		printf("mode NOT 0, %hhd ", pctl->mode);
	if (pctl->t_scan != plist->t_scan)
		printf(" T_SCAN doesn't match ");
	if (pctl->mode != plist->mode)
		printf(" MODE doesn't match ");
	if (pctl->cyc_cnt != plist->cyc_cnt)
		printf(" CYCLE_COUNT doesn't match ");
	printf("\n");
}

// Prints object data with timestamp, and index information for debugging data structures 
void sms_print_obj_data(sms_can_msg_t *pmsg, unsigned char obj_id, 
		smsobjarr_typ *psmsarr, 
		int array_index, int subarray_index, unsigned char listindex)
{	
	smsobj_typ *pobj = &psmsarr[array_index].object[subarray_index];

	printf("objdata: ");	// to distinguish this line from others
	print_timestamp(stdout, &pobj->smstime); // essential part of trace 
	sms_print_raw_can(pmsg);
	printf(" id %hhu list %hhd, arr %d sub %d ", 
		obj_id, 
		listindex,
		array_index,
		subarray_index
		);
	printf("xr %.1f m  yr %.1f m  xv %.1f m/s  yv %.1f m/s length %.1f m\n",
		pobj->xrange,
		pobj->yrange,
		pobj->xvel,
		pobj->yvel,
		pobj->length
		);
}
	
int print_log(unsigned char caseno, void *ptr1, void *ptr2, void *ptr3, void *ptr4, void *ptr5) {
   char *text;
   sms_can_msg_t *message;
   rcv_msg_hdr_t *rcv_msg_hdr;
   int *index;
   int *retval;
   sensctl_typ *sensctl;
   objctl_typ *objctl;
   smsobjarr_typ *smsarr;
   int *arr;
   int *arrmember;
   unsigned char *obj;
   char *pcommand;
    char idx;
//Ethernet state
char *et_state[]={
	"ET_NOTCONNECT",
	"ET_CONNECTING",
	"ET_ISCONNECT",
	"ET_RELIEVE_ERROR",
	"ET_ERROR_STATE",
	"ET_WAIT_OF_REPEAT_DATA"
} ;

   switch(caseno){
        case CASE_RCV:
            text = (char *)ptr1;
            printf("CASE_RCV1: %c%c%c%c%c%c%c%c%c %d %d %d.%d.%d.%d %d %d\
 %d %d %d %d %d %d %d\n",
		      text[0], text[1], text[2], text[3], text[4], text[5], 
		      text[6], text[7],text[8],text[9] & 0XFF,text[10] & 0XFF,
		      text[11] & 0XFF, text[12] & 0XFF,text[13] & 0XFF,
		      text[14] & 0XFF,text[15] & 0XFF, text[16] & 0XFFFF,
		      text[18] & 0XFF,text[19] & 0XFFFF,text[21]&0XFF,
		      text[22] & 0XFF,text[23] & 0XFF,text[24] & 0XFFFFFF,
		      text[27]&0XFFFF);
	       printf("CASE_RCV2: 1st (sensor_control) & 2nd (object_control)\
messages:\nID %0#x %#0x last Bumper IP %d %#0x %#0x %#0x %#0x\
 %#0x %#0x %#0x\nID %x %x %#0x %#0x %#0x %#0x %#0x %#0x %#0x %#0x\n", 
		      text[29] & 0XFF, text[30] & 0XFF, text[31] & 0XFF,
		      text[32] & 0XFF,
		      text[33] & 0XFF,text[34] & 0XFF,text[35] & 0XFF,
		      text[36] & 0XFF,text[37] & 0XFF,text[38] & 0XFF,
		      text[39] & 0XFF,text[40] & 0XFF,text[41] & 0XFF,
		      text[42] & 0XFF,text[43] & 0XFF,text[44] & 0XFF,
		      text[45] & 0XFF,text[46] & 0XFF,text[47] & 0XFF,
		      text[48] & 0XFF);
            break;

        case CASE_TEXT_TO_MSG_RETURN:
              message = (sms_can_msg_t *)ptr1;
	          printf("CASE_TEXT_TO_MSG_RETURN: %hx %#x %#x %#x %#x %#x %#x %#x %#x\n", 
	               message->CAN_ID,
	               message->data[0] & 0xFF,
	               message->data[1] & 0xFF,
	               message->data[2] & 0xFF,
	               message->data[3] & 0xFF,
	               message->data[4] & 0xFF,
	               message->data[5] & 0xFF,
	               message->data[6] & 0xFF,
	               message->data[7] & 0xFF
			       );
            break;

        case CASE_CHECK_HEAD_TAIL:
            rcv_msg_hdr = (rcv_msg_hdr_t *)ptr1;
            retval = (int *)ptr2;
            text = (char *)ptr3;
            printf("CASE_CHECK_HEAD_TAIL1: trainseq %c%c%c%c%c%c%c%c%c\n",
		      rcv_msg_hdr->trainseq[0],rcv_msg_hdr->trainseq[1],
              rcv_msg_hdr->trainseq[2],rcv_msg_hdr->trainseq[3],
		      rcv_msg_hdr->trainseq[4],rcv_msg_hdr->trainseq[5],
		      rcv_msg_hdr->trainseq[6],rcv_msg_hdr->trainseq[7],
		      rcv_msg_hdr->trainseq[8]
		      );
	       printf("CASE_CHECK_HEAD_TAIL2: %c%c%c%c%c%c%c%c%c\n",
		      text[*retval - 9], text[*retval - 8], text[*retval - 7], 
		      text[*retval - 6], text[*retval - 5], text[*retval - 4], 
		      text[*retval - 3], text[*retval - 2], text[*retval - 1]
		      );
            printf("CASE_CHECK_HEAD_TAIL3:%d %d %d.%d.%d.%d %d %d %d %d %d %d\
%d %d %d\n",
       	      rcv_msg_hdr->version & 0XFF,
     	      rcv_msg_hdr->numbytes & 0XFF,
     	      rcv_msg_hdr->ip[0] & 0XFF,
     	      rcv_msg_hdr->ip[1] & 0XFF,
     	      rcv_msg_hdr->ip[2] & 0XFF,
     	      rcv_msg_hdr->ip[3] & 0XFF,
     	      rcv_msg_hdr->lasterr & 0XFF,
     	      rcv_msg_hdr->errcnt & 0XFFFF,
     	      rcv_msg_hdr->nosynccnt & 0XFF,
     	      rcv_msg_hdr->diag & 0XFFFF,
     	      rcv_msg_hdr->can2err & 0XFF,
     	      rcv_msg_hdr->can3err & 0XFF,
     	      rcv_msg_hdr->last_umrr & 0XFF,
     	      rcv_msg_hdr->unused[3] & 0XFFFFFF,
     	      rcv_msg_hdr->crc & 0XFFFF
	          );
            break;
        case CASE_TEXT_TO_MESSAGE:
            text = (char *)ptr1;
            index = (int *)ptr2;
            message = (sms_can_msg_t *)ptr3;
            printf("CASE_TEXT_TO_MESSAGE1: %hx %d %#x %#x %#x %#x %#x %#x\
%#x\n", 
              message->CAN_ID,
              message->data[0] & 0xFF,
              message->data[1] & 0xFF,
              message->data[2] & 0xFF,
              message->data[3] & 0xFF,
              message->data[4] & 0xFF,
              message->data[5] & 0xFF,
              message->data[6] & 0xFF,
              message->data[7] & 0xFF
		      );
            printf("CASE_TEXT_TO_MESSAGE2: %x %x %d %d %d %d %d %d %d %d\n", 
	          text[*index + 0] & 0XFF, text[*index + 1] & 0XFF,
	          text[*index + 2] & 0XFF, text[*index + 3] & 0XFF,
              text[*index + 4] & 0XFF, text[*index + 5] & 0XFF,
	          text[*index + 6] & 0XFF, text[*index + 7] & 0XFF,
	          text[*index + 8] & 0XFF, text[*index + 9] & 0XFF
	          );
            break;

        case CASE_SENSOR_CTL:
            message = (sms_can_msg_t *)ptr1;
            sensctl = (sensctl_typ *)ptr2;
            printf("CASE_SENSOR_CTL1: sensor_control message type, \
message %#0x %#0x %#0x %#0x %#0x %#0x %#0x %#0x \n",
              message->data[0] & 0XFF, //src ID
              message->data[1] & 0XFF, //CAN stat
              message->data[2] & 0XFF, 
              message->data[3] & 0XFF, //Ethernet state
              message->data[4] & 0XFF, 
              message->data[5] & 0XFF, 
              message->data[6] & 0XFF, 
              message->data[7] & 0XFF
			  ); 
             printf("CASE_SENSOR_CTL2: sensor_control message type, \
src ID %#0x CAN stat %#0x sensors present %#0x enet state %s timestamp %#0x\n",
              sensctl->bump_d_net & 0xFF, //src ID
              sensctl->canstat & 0xFF, //CAN stat
              sensctl->sensorspres, 
	      et_state[(int)sensctl->enetstat],
	      sensctl->timestamp
	      ); 
            break;

        case CASE_OBJECT_CTL:
            message = (sms_can_msg_t *)ptr1;
            objctl = (objctl_typ *)ptr2;
            printf("CASE_OBJECT_CTL1: object_control message type, \
message %#0x %#0x %#0x %#0x %#0x %#0x %#0x %#0x \n",
              message->data[0] & 0XFF, //src ID
              message->data[1] & 0XFF, //CAN stat
              message->data[2] & 0XFF, 
              message->data[3] & 0XFF, //Ethernet state
              message->data[4] & 0XFF, 
              message->data[5] & 0XFF, 
              message->data[6] & 0XFF, 
              message->data[7] & 0XFF
	      ); 
            printf("CASE_OBJECT_CTL2: object_control message type, \
#objs %d #msgs %d tscan %u ms mode %d cycle cnt %#0x\n", 
              objctl->no_obj, // number of objects
              objctl->no_msgs, // number of messages
              objctl->t_scan & 0xFF, // this cycle time t_scan (ms)
              objctl->mode,//mode (0=normaltracking,1=simulation)
              objctl->cyc_cnt     //current cycle count byte 0
	          ); 
            break;

        case CASE_OBJECT_DATA:
            obj = (unsigned char *)ptr1;
            smsarr = (smsobjarr_typ *)ptr2;
            arr = (int *)ptr3;
            arrmember = (int *)ptr4;
            message = (sms_can_msg_t *)ptr5;
printf("CASE_OBJECT_DATA: ID %hx %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x \
%2.2x %2.2x\n",
    message->CAN_ID,
    message->data[0] & 0xFF,
    message->data[1] & 0xFF,
    message->data[2] & 0xFF,
    message->data[3] & 0xFF,
    message->data[4] & 0xFF,
    message->data[5] & 0xFF,
    message->data[6] & 0xFF,
    message->data[7] & 0xFF
    );

            printf("CASE_OBJECT_DATA: object_data_1 message type, \
obj #%u xrange %.1f m  yrange %.1f m  xvel %.1f m/s  yvel %.1f m/s \
length %.1f m\n", 
              *obj, 
              smsarr[*arr].object[*arrmember].xrange,
              smsarr[*arr].object[*arrmember].yrange,
              smsarr[*arr].object[*arrmember].xvel,
              smsarr[*arr].object[*arrmember].yvel,
              smsarr[*arr].object[*arrmember].length  
              );
            break;
        case CASE_COMMAND:
            pcommand = (char *)ptr1;
            printf("CASE_COMMAND: ");
    	    for(idx=0; idx<sizeof(smscmd_t);idx++)
                printf("%#0x ", pcommand[(int)idx]);
            printf("\n\n");
            break;

    }//end of switch
    return 0;
}//end of print_log

int check_head_tail( int retval, rcv_msg_hdr_t *rcv_msg_hdr, char *text,
    int debug) {
    char *trainseqstd = "AFAFAFAFA";
    char *tailseqstd  = "EDEDEDEDE";
    int error;
    memcpy(rcv_msg_hdr, text, 29);
/*
    error = sscanf( &text[0], "%9c%c%c%c%c%c%c%c%hu%c%hu%c%c%c%c%c%c%hu",
    	    rcv_msg_hdr->trainseq[0],   //9 bytes
       	    rcv_msg_hdr->version,       //1 byte
     	    rcv_msg_hdr->numbytes,      //1 byte
     	    rcv_msg_hdr->ip[0],         //4 bytes
     	    rcv_msg_hdr->ip[1],         //4 bytes
     	    rcv_msg_hdr->ip[2],         //4 bytes
     	    rcv_msg_hdr->ip[3],         //4 bytes
     	    rcv_msg_hdr->lasterr,       //1
     	    rcv_msg_hdr->errcnt,        //2
     	    rcv_msg_hdr->nosynccnt,     //1
     	    rcv_msg_hdr->diag,          //2
     	    rcv_msg_hdr->can2err,       //1
     	    rcv_msg_hdr->can3err,       //1
     	    rcv_msg_hdr->last_umrr,     //1
     	    rcv_msg_hdr->unused[0],     //3
     	    rcv_msg_hdr->unused[1],     //3
     	    rcv_msg_hdr->unused[2],     //3
     	    rcv_msg_hdr->crc            //2
	        );
    if( error == EOF ) {
    	perror("check_head_tail:sscanf");
        return EOF;
        }
*/
    if( strncmp( text, trainseqstd, 9) != 0 ) {
    	perror("check_head_tail:strncmp trainseq");
        return EOF;
	   }
     if( strncmp( &text[retval - 9], tailseqstd, 9) != 0 ){
    	perror("check_head_tail:strncmp tailseq");
        return EOF;
        }
	   if(debug & CASE_CHECK_HEAD_TAIL)
        print_log(CASE_CHECK_HEAD_TAIL, rcv_msg_hdr, &retval, text, NULL, NULL);
    return error;
} 

int text_to_msg(int index, sms_can_msg_t *message, char *text, unsigned char debug) {

    int error;

//printf("T:1:index %d message %#x text %#x ", index, (uint)message, (uint)text)
;
    memcpy( message, &text[index], 10);
//printf("T:2:index %d message %#x text %#x ", index, (uint)message, (uint)text)
;
    message->CAN_ID = ntohs(message->CAN_ID);
    if(debug & CASE_TEXT_TO_MESSAGE)
        print_log(CASE_TEXT_TO_MESSAGE, text, &index, &message, NULL, NULL);
//printf("T:3:\n");
    return error;
}

int smscommand(smscmd_t *command, uchar_typ devID, uchar_typ parm, 
    uchar_typ parm_type, uchar_typ action, int value, int socketfd) {

    command->cmd_data.data[0] = devID;
    command->cmd_data.data[1] = parm;
    command->cmd_data.data[2] = parm_type;
    command->cmd_data.data[3] = action;
    if( (parm_type == 1) || (parm_type == 5) )
        command->cmd_data.value = (float)value;
    else
        command->cmd_data.value = value;

    if( send(socketfd, command, sizeof(smscmd_t), 0) < 0) 
        perror("smscommand send");
    return 0;
}
