#include "db_include.h"
#include "db_utils.h"
#include "sys_buff.h"
#include "clt_vars.h"
#include "replay_truck.h"

static int sig_list[] =
{
        SIGINT,
        SIGQUIT,
        SIGTERM,
        SIGALRM,
        ERROR,
};

static jmp_buf exit_env;

static void sig_hand(int code)
{
        if (code == SIGALRM)
                return;
        else
                longjmp(exit_env, code);
}

db_id_t db_vars_list[] =  {
        {1001, sizeof(long_vehicle_state)},
        {1002, sizeof(long_output_typ)},
        {1003, sizeof(long_dig_out_typ)},
        {1004, sizeof(veh_comm_packet_t)},
        {1005, sizeof(timestamp_t)},
        {1006, sizeof(double)},
};
int num_db_vars = sizeof(db_vars_list)/sizeof(db_id_t);

const char *usage = "-d <Database number (Modulo 4!)> -f <filename> -i (loop interval) -v (verbose)\n"; 

int main(int argc, char *argv[]) {
	char strbuf[5000];
	FILE *filestream; 
	char *filename; 
	int use_db = 1;
        int option;
        int exitsig;
        db_clt_typ *pclt;
        char hostname[MAXHOSTNAMELEN+1];
        int interval = 1000;      /// Number of milliseconds between saves
        posix_timer_typ *ptimer;       /* Timing proxy */
        char *domain = DEFAULT_SERVICE; // usually no need to change this
        int xport = COMM_OS_XPORT;      // set correct for OS in sys_os.h
        int verbose = 0;
        int pv_var = 0;
        extern long_vehicle_state pv;
	veh_comm_packet_t *pcomm_pkt1 = &pv.lead_trk;
	veh_comm_packet_t *pcomm_pkt2 = &pv.second_trk;
	veh_comm_packet_t *pcomm_pkt3 = &pv.third_trk;
	veh_comm_packet_t *pself_comm_pkt;

        while ((option = getopt(argc, argv, "d:i:f:v")) != EOF) {
                switch(option) {
                case 'd':
//                        pv_var = atoi(optarg);
			use_db = 1;
                        break;
                case 'i':
                        interval= atoi(optarg);
                        break;
		case 'f':
			filename = strdup(optarg);
			break;
                case 'v':
                        verbose = 1;
                        break;
               default:
                        printf("Usage: %s %s\n", argv[0], usage);
                        exit(EXIT_FAILURE);
                        break;
                }
        }

        db_vars_list[0].id = pv_var;
        db_vars_list[1].id = pv_var + 1;
        db_vars_list[2].id = pv_var + 2;
        db_vars_list[3].id = pv_var + 3;
        db_vars_list[4].id = pv_var + 4;
        db_vars_list[5].id = pv_var + 5;
printf("Got to 1\n");
	if(use_db) {
		get_local_name(hostname, MAXHOSTNAMELEN);
		if ( (pclt = db_list_init(argv[0], hostname, domain, xport, NULL, 0, NULL, 0)) == NULL) {
			printf("Database initialization error in %s.\n", argv[0]);
			exit(EXIT_FAILURE);
		}
		/* Setup a timer for every 'interval' msec. */
		if ((ptimer = timer_init(interval, DB_CHANNEL(pclt) )) == NULL) {
			printf("Unable to initialize wrfiles timer\n");
			exit(EXIT_FAILURE);
		}
	}

	if(( exitsig = setjmp(exit_env)) != 0) {
		db_list_done(pclt, NULL, 0, NULL, 0);
		exit(EXIT_SUCCESS);
	} else
		sig_ign(sig_list, sig_hand);

printf("Got to 2\n");
	memset(strbuf, 0, 1000);
	filestream = fopen(filename, "r");
printf("Got to 3\n");
	while(fgets(strbuf, 1000, filestream) != NULL) {
		get_data_log_line(strbuf, file_spec, num_file_columns);
printf("Got to 4\n");
		if(use_db) {
//			db_clt_write(pclt, pv_var, sizeof(long_vehicle_state), &pv);
//			long_read_vehicle_state(pclt, &control_state);
//			db_clt_write(pclt, DB_LONG_OUTPUT_VAR, sizeof(long_output_typ), &long_out);
//			db_clt_write(pclt, DB_LONG_DIG_OUT_VAR, sizeof(long_dig_out_typ), &dig_out);
//			db_clt_write(pclt, DB_COMM_TX_VAR, sizeof(veh_comm_packet_t), &comm_tx);
			db_clt_write(pclt, DB_COMM_LEAD_TRK_VAR, sizeof(veh_comm_packet_t), pcomm_pkt1);
			db_clt_write(pclt, DB_COMM_SECOND_TRK_VAR, sizeof(veh_comm_packet_t), pcomm_pkt2);
			db_clt_write(pclt, DB_COMM_THIRD_TRK_VAR, sizeof(veh_comm_packet_t), pcomm_pkt3);
			db_clt_write(pclt, DB_COMM_TX_VAR, sizeof(veh_comm_packet_t), pself_comm_pkt);


			printf("strbuf %s\n", strbuf);
		}
		TIMER_WAIT(ptimer);
	}
	return 0;
}
