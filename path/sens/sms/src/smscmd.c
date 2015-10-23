/* smscmd.c - send commands to SMS radar vi database
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


int main(int argc, char *argv[]) {

    int option;
    int device = 0;
    int param_no = 30;
    int value = 0;
    int type = 4;
    int action = 64;

	char hostname[MAXHOSTNAMELEN+1];
	char *domain = DEFAULT_SERVICE; //for QNX6, use e.g. "ids"
	int xport = COMM_PSX_XPORT;	//for QNX6, no-op
    db_clt_typ *pclt = NULL;
    sndmsg_t   command;
    int cmdctr;

    while( (option = getopt(argc, argv, "d:p:t:a:v:hi:f:") ) != EOF ) {
        switch( option ) {
            case 'd' :
                device = atoi(optarg);
                break;
            case 'p' :
                param_no = atoi(optarg);
                break;
            case 'v' :
                value = atoi(optarg);
                break;
            case 't' :
                type = atoi(optarg);
                break;
            case 'a' :
                action = atoi(optarg);
                break;
            case 'i' : //Read parameter value as int
		device = 0;
                param_no = atoi(optarg);
                type = 4;
                action = 64;
                break;
            case 'f' : //Read parameter value as float
		device = 0;
                param_no = atoi(optarg);
                type = 5;
                action = 64;
                break;
            case 'h' :
            default:
                printf("\nUsage:\n%s -d <device> -p <parameter> -t <type> \
-a <action> -v <value>\n\tOr\n%s -i <read int parameter> -f <read float parameter>\n\n(See doc/UMRR_Interface_Documentation.pdf, pp. 52-53 for parameter definitions)\n\n",
argv[0], argv[0]);
                exit(EXIT_FAILURE);
                break; 
        }
    }

    get_local_name(hostname, MAXHOSTNAMELEN);
    if ((pclt = db_list_init(argv[0], hostname, domain, xport, 
	NULL, 0, NULL, 0)) == NULL) {
	printf("Database initialization error in %s.\n", 
	argv[0]);
	exit(EXIT_FAILURE);
		         }
    command.cmdtarg = 0;
    command.data[0] = device & 0xFF;
    command.data[1] = param_no & 0xFF;
    command.data[2] = type & 0xFF;
    command.data[3] = action & 0xFF;
    if(type == 4)
        command.value = device & 0xFF;
    else
    if(type == 5)
        command.value = (float)value;
    else
	command.value = value;
printf("SMSCMD: targ  %d device %d param %d type %d action %d value %d\n",
    command.cmdtarg,
    command.data[0],
    command.data[1],
    command.data[2],
    command.data[3],
    command.value);

    db_clt_write(pclt, DB_SMS_CMD_VAR, sizeof(sndmsg_t), &command);
    db_clt_read(pclt, DB_SMS_CMDCTR_VAR, sizeof(int), &cmdctr);
    ++cmdctr;
    db_clt_write(pclt, DB_SMS_CMDCTR_VAR, sizeof(int), &cmdctr);
    db_list_done(pclt, NULL, 0, NULL, 0);
    return 0;
}
