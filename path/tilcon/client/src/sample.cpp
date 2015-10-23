/**\file
 *
 *	sample.cpp	Test Tilcon vertion 5.5 
 *
 *  	The associated Tilcon image file is test.twd
 *
 */

#include <sys_os.h>
#include <tilcon_utils.h>

extern "C" {
#include <sys_rt.h>
#include <db_clt.h>
#include <db_utils.h>
#include <timestamp.h>
#include "clt_vars.h"
#include "sample.h"
}
  
static jmp_buf exit_env;

static int sig_list[]= {
	SIGINT,
	SIGQUIT,
	SIGTERM,
	SIGALRM,	// for timer
	ERROR
};

static void sig_hand( int code )
{
	if (code == SIGALRM)
		return;
	else
		longjmp( exit_env, code );
}

void sample_set_tilcon(pid_t cid, void *data);
static void init_widgets(pid_t cid);

char *main_window_file = "test";

char *main_window_id = "test";

/** Widgets used in this application */

tilcon_widget_t radar_w;
tilcon_widget_t meter_w;
tilcon_widget_t meter_alarm_w;
tilcon_widget_t needle_w;
tilcon_widget_t drawing_w;
tilcon_widget_t textbox_w;


/* used in all functions, set in main; for debugging */
static int verbose = 0;

/* Global because it must be accessed by the callback function 
 * which has a fixed calling sequence.
 */
db_clt_typ *pclt;

int main(int argc, char *argv[])
{

	int errorcode = 0;
	char *domain = DEFAULT_SERVICE;
	char hostname[MAXHOSTNAMELEN +1];		
	int xport = COMM_PSX_XPORT;
	pid_t tilcon_cid;	//channel id for connection with Tilcon kernel
	tilcon_callback_t veh_stat_callback; 
	char *sample_tilcon_dir = "/home/path/tilcon/client/twd";
	int trt_val;	/// returned by wait on TRT_GetInput (Tilcon receive)
	TRT_ReceiveData	rec_data;       
	long blocking_flag = TRT_BLOCK;	// make non-blocking and set our timer?
	long full_queue	= FALSE;
	long *trt_list = (long int *) NULL; // semaphore list, not used on QNX6
	long list_size		= 0;
	char *buff = (char *) NULL; /// holds message if non-Tilcon message received 
	long buff_size		= 0;
	int option;

	/* Read and interpret any user switches. */

	while ((option = getopt(argc, argv, "d:f:i:vw:")) != EOF) {
		switch(option) {
		case 'd':
			domain = strdup(optarg);
			break;
		case 'f':
			main_window_file = strdup(optarg);
			break;
		case 'i':
			sample_tilcon_dir = strdup(optarg);
			break;
		case 'w':
			main_window_id = strdup(optarg);
			break;
		case 'v':
			verbose = 1;
			break;
	        default:
			printf( "Usage %s: -d domain -f twd file name ",
				argv[0]);
			printf( "-w main window ID -i image directory\n");
			exit(EXIT_FAILURE);
	        }
	}

	if ((tilcon_cid =tilcon_start_kernel("OMB_TEST", argv[0], 0)) == -1)
		exit(EXIT_FAILURE);

	veh_stat_callback.set_values = sample_set_tilcon;
	veh_stat_callback.data = NULL;

	if (errorcode = tilcon_init_app(tilcon_cid, 2, sample_tilcon_dir, 
				main_window_file, main_window_id, 0, 3)) {

		printf("Failed to initialized Tilcon application\n");
		exit(EXIT_FAILURE);
	}
//	TRT_SetValues(tilcon_cid, main_window_id, TRT_ATT_X1, 40, NULL);
//	TRT_SetValues(tilcon_cid, main_window_id, TRT_ATT_Y1, 40, NULL);

       /* exit environment set up after Tilcon initialization */

        if (setjmp(exit_env) != 0) {		
		if (pclt != NULL) 
			clt_logout(pclt);
		errorcode =tilcon_exit(tilcon_cid, main_window_id);
		printf("%s exiting, Tilcon error %d\n", argv[0], errorcode);
                exit(EXIT_SUCCESS);
        } else
                sig_ign(sig_list, sig_hand);

	// On Linux, like QNX4, only processes that write the variables
	// should create them; these processes should be started before
	// the processes that read them. This process only reads
	// DB variables, so only needs to login, not do create.

        if (( pclt = clt_login( argv[0], hostname, domain, xport)) == NULL ) {
                   printf("Database initialization error in %s\n", argv[0]);
                   exit( EXIT_FAILURE );
        }

       /* exit environment set up after Tilcon initialization */

	init_widgets(tilcon_cid);
	if (verbose) { 
		printf("%s : initialization completed\n", argv[0]);
		fflush(stdout);
	}

	/** Check TRT_GetInput() to see if a Tilcon event occurred, if
	 * a semaphore was changed, or if a message was received from
	 * another process (e.g. a database trigger or a timer). 
	 * Break out of loop on error.
	 */

	while (TRUE) {
		trt_val = TRT_GetInput(trt_list, list_size, buff, buff_size, 
				&rec_data, blocking_flag);

		/** For a 'notification' from the Tilcon Kernel, c=0.  */

		if (trt_val == 0) {
			if (!tilcon_process_notify(tilcon_cid, &rec_data, 
				&full_queue, &errorcode, &veh_stat_callback,
				main_window_id))
				break;
		} else if ( trt_val == -1 ) {
			/** if c=-1, an error occurred if blocking
			  * (or nothing was waiting, for TRT_NO_BLOCK) 
			  */

			if (blocking_flag == TRT_BLOCK)
				break;

		} else {

		/** Otherwise, a message was received from another process or
		 *if 1, TRT_GetInput took care of calling a callback 
		 * otherwise trt_val is channel id of message 
		 * No action on this branch for now; later, we may learn
		 * how to use Tilcon callbacks or handle timers here.
		 */
			printf("Unknown message received, trt_val %d\n",
				 trt_val);
		}
	}	// end of main loop //
	longjmp(exit_env,1);
	return 0;
}

static void init_widgets(pid_t cid)

{
	unsigned int get_color = 0xdeadbeef;
	tilcon_init_meter(cid, &meter_w, "meter", 50, 1, TRANSPARENT);
	tilcon_init_meter(cid, &needle_w, "needle", 50, 1, TRANSPARENT);
	tilcon_init_meter_alarm(cid, &meter_alarm_w, "meter",
		TRT_RGB(0,255,0), AMBER, SPEEDOMETER_ORANGE,
		 0.0, 50.0, 75.0, 100.0, 0.1);
	tilcon_init_multistate(cid, &radar_w, "radar", 0);
	tilcon_init_drawing(cid, &drawing_w, "drawing", SPEEDOMETER_ORANGE, 
		AMBER);
	tilcon_init_text(cid, &textbox_w, "textbox", "Initialized");
}

/**
 * void sample_set_tilcon()
 *
 * This function sets the values for Tilcon Objects used
 * in this program.
 */

void sample_set_tilcon(pid_t cid, void *data)
{
	radar_typ db_radar;
	meter_typ db_meter;
	drawing_typ db_drawing;
	
	db_clt_read(pclt, DB_RADAR_VAR, sizeof(db_radar), &db_radar);
	db_clt_read(pclt, DB_METER_VAR, sizeof(db_meter), &db_meter);
	db_clt_read(pclt, DB_DRAWING_VAR, sizeof(db_drawing), &db_drawing);
	
	tilcon_set_multistate(cid, &radar_w, db_radar.radar_status);
	tilcon_set_meter(cid, &needle_w, db_meter.meter_val, 					TRANSPARENT);	
	tilcon_set_meter(cid, &meter_w, db_meter.meter_val,
				db_meter.meter_fill);
	tilcon_set_drawing(cid, &drawing_w, db_drawing.fill_color,
				db_drawing.line_color);
	tilcon_set_font_color(cid, &textbox_w, db_drawing.fill_color);

	switch (db_meter.alarm_mode) {
		case 0:
			tilcon_set_meter_alarm(cid, &meter_alarm_w, 
				TRT_RGB(0,255,0), AMBER, SPEEDOMETER_ORANGE, 
				0.0, 50.0, 75.0, 100.0);
			break;
		case 1:
			tilcon_set_meter_alarm(cid, &meter_alarm_w, 
				AMBER, SPEEDOMETER_ORANGE, TRT_RGB(0,255,0),  
				10.0, 25.0, 50.0, 90.0);
			break;
		default:
			tilcon_set_meter_alarm(cid, &meter_alarm_w, 
				SPEEDOMETER_ORANGE, TRT_RGB(0,255,0), AMBER, 
				0.0, 15.0, 30.0, 100.0);
			break;
	}	
}

