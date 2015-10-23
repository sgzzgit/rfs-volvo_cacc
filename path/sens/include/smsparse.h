/* smsparse.h - header file for smsparse.c
*/

#include <timestamp.h>

#define CASE_RCV                0X01
#define CASE_TEXT_TO_MSG_RETURN 0X02
#define CASE_CHECK_HEAD_TAIL    0X04
#define CASE_TEXT_TO_MESSAGE    0X08
#define CASE_SENSOR_CTL         0x10
#define CASE_OBJECT_CTL         0x20
#define CASE_OBJECT_DATA        0x40
#define CASE_COMMAND		0x80

typedef struct {
    unsigned short CAN_ID;
    unsigned char data[8];
} IS_PACKED sms_can_msg_t;

typedef struct {
    char    trainseq[9];
    char    version;
    char    numbytes;
    char    ip[4];
    char    lasterr;
    unsigned short  errcnt;
    char    nosynccnt;
    unsigned short  diag;
    char    can2err;
    char    can3err;
    char    last_umrr;
    char    unused[3];
    unsigned short    crc;
} IS_PACKED rcv_msg_hdr_t;

typedef struct {
    char cmdtarg;
    char data[4];
    int value;
} IS_PACKED sndmsg_t;

typedef struct {
    char        trainseq[9];
    char        numpkts;
    char        pktno;
    char        ip[4];
    char        canno;
    unsigned short CAN_ID;
    sndmsg_t    cmd_data;
    char        tailseq[9];
} IS_PACKED smscmd_t;

#define MAXOBJ  64
typedef struct {
    timestamp_t smstime;
    float   xrange;
    float   yrange;
    float   xvel;
    float   yvel;
    float   length;
    char    obj;
} IS_PACKED smsobj_typ;

// We are limited to 128 bytes per database message. smsobj_typ is (now)
// 26 bytes, giving 130 bytes for a 5-object array, so we go with 4.
#define MAXDBOBJS   4
typedef struct dbobjectarr{
    int cyc_cnt; // used to check that all DB VARS read are in same object list
    smsobj_typ object[MAXDBOBJS];
} IS_PACKED smsobjarr_typ;
#define SMSARRMAX  16 

typedef struct {
	timestamp_t ts;
	unsigned char num_obj;
	char	t_scan;
	char	mode;
	unsigned int	cyc_cnt;
	unsigned char arr[MAXOBJ];
} IS_PACKED objlist_typ;

#define DB_SMS_OBJARR0_TYPE         5010  /* smsobjarr_typ */
#define DB_SMS_OBJARR1_TYPE         5011  /* smsobjarr_typ */
#define DB_SMS_OBJARR2_TYPE         5012  /* smsobjarr_typ */
#define DB_SMS_OBJARR3_TYPE         5013  /* smsobjarr_typ */
#define DB_SMS_OBJARR4_TYPE         5014  /* smsobjarr_typ */
#define DB_SMS_OBJARR5_TYPE         5015  /* smsobjarr_typ */
#define DB_SMS_OBJARR6_TYPE         5016  /* smsobjarr_typ */
#define DB_SMS_OBJARR7_TYPE         5017  /* smsobjarr_typ */
#define DB_SMS_OBJARR8_TYPE         5018  /* smsobjarr_typ */
#define DB_SMS_OBJARR9_TYPE         5019  /* smsobjarr_typ */
#define DB_SMS_OBJARR10_TYPE        5020  /* smsobjarr_typ */
#define DB_SMS_OBJARR11_TYPE        5021  /* smsobjarr_typ */
#define DB_SMS_OBJARR12_TYPE        5022  /* smsobjarr_typ */
#define DB_SMS_OBJARR13_TYPE        5023  /* smsobjarr_typ */
#define DB_SMS_OBJARR14_TYPE        5024  /* smsobjarr_typ */
#define DB_SMS_OBJARR15_TYPE        5025  /* smsobjarr_typ */
#define DB_SMS_OBJLIST_TYPE         5026  /* */
#define DB_SMS_SENSCTL_TYPE         5027  /* */

#define DB_SMS_OBJARR0_VAR          DB_SMS_OBJARR0_TYPE
#define DB_SMS_OBJARR1_VAR          DB_SMS_OBJARR1_TYPE
#define DB_SMS_OBJARR2_VAR          DB_SMS_OBJARR2_TYPE
#define DB_SMS_OBJARR3_VAR          DB_SMS_OBJARR3_TYPE
#define DB_SMS_OBJARR4_VAR          DB_SMS_OBJARR4_TYPE
#define DB_SMS_OBJARR5_VAR          DB_SMS_OBJARR5_TYPE
#define DB_SMS_OBJARR6_VAR          DB_SMS_OBJARR6_TYPE
#define DB_SMS_OBJARR7_VAR          DB_SMS_OBJARR7_TYPE
#define DB_SMS_OBJARR8_VAR          DB_SMS_OBJARR8_TYPE
#define DB_SMS_OBJARR9_VAR          DB_SMS_OBJARR9_TYPE
#define DB_SMS_OBJARR10_VAR         DB_SMS_OBJARR10_TYPE
#define DB_SMS_OBJARR11_VAR         DB_SMS_OBJARR11_TYPE
#define DB_SMS_OBJARR12_VAR         DB_SMS_OBJARR12_TYPE
#define DB_SMS_OBJARR13_VAR         DB_SMS_OBJARR13_TYPE
#define DB_SMS_OBJARR14_VAR         DB_SMS_OBJARR14_TYPE
#define DB_SMS_OBJARR15_VAR         DB_SMS_OBJARR15_TYPE
#define DB_SMS_OBJLIST_VAR          DB_SMS_OBJLIST_TYPE
#define DB_SMS_SENSCTL_VAR          DB_SMS_SENSCTL_TYPE 

#define DB_SMS_CMD_TYPE             5030
#define DB_SMS_CMD_VAR              DB_SMS_CMD_TYPE
#define DB_SMS_CMDCTR_TYPE          5031
#define DB_SMS_CMDCTR_VAR           DB_SMS_CMDCTR_TYPE

// macro to return the DB VAR where a particular object id is stored
#define DB_VAR_SMS_OBJ(obj_id) ((DB_SMS_OBJARR0_VAR) + ((obj_id)/(MAXDBOBJS)))

// macro to return the index within an smsobjectarr_typ where the given
// object corresponding to the given object id can be found
#define DB_VAR_SMS_OBJ_INDEX(obj_id) ((obj_id) % (MAXDBOBJS))

//Sensor control
typedef struct {
	char	bump_d_net;
	char	canstat;
	unsigned short	sensorspres;  
	char	enetstat;
	unsigned int	timestamp;
} IS_PACKED sensctl_typ;

//Object control
typedef struct {
	char	no_obj;
	char	no_msgs;
	char	t_scan;
	char	mode;
	unsigned int	cyc_cnt;
} IS_PACKED objctl_typ;

int text_to_msg(int, sms_can_msg_t *, char *, unsigned char);
int parse_msg(sms_can_msg_t *);
extern void sig_ign(int *plist, void (*pfunc)(int));
int check_head_tail( int, rcv_msg_hdr_t *, char *, int);
int OpenSMSConnection(char *local_ip, char *remote_ip);
int CloseSMSConnection(int);
int print_log(unsigned char, void *, void *, void *, void *, void *);
void sms_print_obj_list(sms_can_msg_t *pmsg, objctl_typ *pctl, objlist_typ *plist);
void sms_print_obj_data(sms_can_msg_t *pmsg, unsigned char obj_id, 
	smsobjarr_typ *psmsarr, int array_index, int subarray_index,
	unsigned char listindex);
int smscommand(smscmd_t *, uchar_typ, uchar_typ, uchar_typ, uchar_typ, int,int);
