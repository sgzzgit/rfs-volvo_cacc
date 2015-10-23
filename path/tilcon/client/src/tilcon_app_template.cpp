/**\file
 *
 *		tilcon_app_template.cpp: Blank Example Tilcon Application for Linux
 *
 *		This example was based on the first OBMS Tilcon Linux App and
 *		was first used to create taurus_eng_main.cpp
 *		
 *  	Parts that need to be filled in to create a working app:
 *
 *		Includes
 *		Tilcon Application Definitions
 *		Custom Color Definitions
 *		Other Application Constants
 *		Declare Widgets
 *		Widget Initialization
 *		Application_Set_Tilcon Function
 *
 */

// Always Include (C++ Headers)
#include <sys_os.h>
#include <tilcon_utils.h>

extern "C" {
// Always Include (C Headers)
#include <local.h>
#include <sys_rt.h>
#include <db_clt.h>
#include <db_utils.h>
#include <timestamp.h>
#include <clt_vars.h>

// Includes C Headers That Provide Specific Data Definitions For This App


}


/**\ Tilcon Application Definitions */

/**\ Tilcon Filename and Window ID */
char *main_window_file = "filname (with no .twd)";
char *main_window_id = "tilcon_window_attribute_id_string (usually filename with no .twd)";

/**\ This should be a unique object space name for every tilcon app */
char *start_kernel_string = "tilcon_app_template";

/**\ This is the default for where the .twd file is stored */
char *app_tilcon_dir = "/home/directorytree/twd";


/**\ Custom Color Definitions for this Application Only
 *
 * Color definitons common to all aps go in '/home/path/tilcon/widget.h'
 * and include the following common colors:
 *
 * Basic Color Definitions
 #define AMBER                   TRT_RGB(255,190,0)
 #define BLACK                   TRT_RGB(0,0,0)
 #define BLUE255                 TRT_RGB(0,0,255)
 #define GREEN255				 TRT_RGB(0,255,0)
 #define GREEN128				 TRT_RGB(0,128,0)
 #define MAROON					 TRT_RGB(128,0,0)
 #define RED255                  TRT_RGB(255,0,0)
 #define WHITE                   TRT_RGB(255,255,255)
 #define SPEEDOMETER_ORANGE      TRT_RGB(255,65,0)
 #define TRANSPARENT			 TRT_Transparent
 *
 * Common Greyscale Definitions
 #define GREY32                  TRT_RGB(32,32,32)		// Really Dark Grey
 #define GREY69                  TRT_RGB(69,69,69)		// Dark Grey
 #define GREY162                 TRT_RGB(162,162,162)	// Background Grey
 #define GREY192                 TRT_RGB(192,192,192)	// Button Grey
 *
 * Enter Any Custom Color Definitions for this app only below.
 *
 */



/**\ Other Application Constants
 *
 * Format: #define VariableName Value
 * Format: char *VariableName = "Text";
 *
 */


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

/** Function Declares */

// Always Needed
static void application_set_tilcon(pid_t cid, void *data);
static void init_widgets(pid_t cid);

// Application Specific



/**\ Declare Widgets used in this application
 * 
 * Example:
 * tilcon_widget_t widgetname_w;
 *
 */



/* used in all functions, set in main; for debugging */
static int verbose = 0;

/* Global because it must be accessed by the callback function 
 * which has a fixed calling sequence.
 */
db_clt_typ *pclt;

#ifdef __QNXNTO__
/** On QNX6, using Cogent for the data server backend, the implementation
 *  requires the clt_create operation for every process that uses a variable.
 *  This is in contract to the original implementation on QNX4 that has been
 *  carried over to Linux, where at most one process is allowed to create
 *  a variable. The db_vars_list is used by db_list_init and db_list_done
 *  to handle create and destroy operations.
 */
static db_id_t db_vars_list[] = {
        };

#define NUM_DB_VARS sizeof(db_vars_list)/sizeof(db_id_t)

#endif	//__QNXNTO__

int main(int argc, char *argv[])
{

	int errorcode = 0;
	char *domain = DEFAULT_SERVICE;
	char hostname[MAXHOSTNAMELEN +1];		
	int xport = COMM_PSX_XPORT;
	pid_t tilcon_cid;	//channel id for connection with Tilcon kernel
	tilcon_callback_t veh_stat_callback; 
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

	while ((option = getopt(argc, argv, "d:f:i:vp:")) != EOF) {
		switch(option) {
		case 'd':
			domain = strdup(optarg);
			break;
		case 'f':
			main_window_file = strdup(optarg);
			break;
		case 'i':
			main_window_id = strdup(optarg);
			break;
		case 'p':
			app_tilcon_dir = strdup(optarg);
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

	if ((tilcon_cid =tilcon_start_kernel(start_kernel_string, argv[0], 0)) == -1)
		exit(EXIT_FAILURE);

	veh_stat_callback.set_values = application_set_tilcon;

	veh_stat_callback.data = NULL;

	if (errorcode = tilcon_init_app(tilcon_cid, 2, app_tilcon_dir, 
				main_window_file, main_window_id, 0, 3)) {

		printf("Failed to initialized Tilcon application\n");
		exit(EXIT_FAILURE);
	}


       /* exit environment set up after Tilcon initialization */

        if (setjmp(exit_env) != 0) {		
		if (pclt != NULL)
#ifdef __QNXNTO__
			db_list_done(pclt, db_vars_list, NUM_DB_VARS, NULL, 0);
#else
			clt_logout(pclt);
#endif
		errorcode =tilcon_exit(tilcon_cid, main_window_id);
		printf("%s exiting, Tilcon error %d\n", argv[0], errorcode);
                exit(EXIT_SUCCESS);
        } else
                sig_ign(sig_list, sig_hand);

#ifdef __QNXNTO__
        if (( pclt = db_list_init( argv[0], hostname, domain, xport,
			db_vars_list, NUM_DB_VARS, NULL, 0)) == NULL ) {
                   printf("Database initialization error in %s\n", argv[0]);
                   exit( EXIT_FAILURE );
	}
#else
	// On Linux, like QNX4, only processes that write the variables
	// should create them; these processes should be started before
	// the processes that read them. This process only reads
	// DB variables, so only needs to login, not do create.

        if (( pclt = clt_login( argv[0], hostname, domain, xport)) == NULL ) {
                   printf("Database initialization error in %s\n", argv[0]);
                   exit( EXIT_FAILURE );
        }
#endif


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



/**\ Widget Initialization
 *
 *	Notes on object parameters:
 *
 *  tilcon_init_button(cid, &objectname_w, "objectname", "button text", in_fill_color, out_fill_color);
 *	tilcon_init_drawing(cid, &objectname_w, "objectname", fill_color, line_color);
 *	tilcon_init_meter(cid, &objectname_w, "objectname", value, precision, indicator_fill_color);
 *    Note: Needles use TRANSPARENT as Indicator Fill
 *	tilcon_init_meter_alarm(cid, &objectname_w, "objectname", 
 *    low_color, mid_color, high_color, start_at, low_to, mid_to, high_to, precision);
 *	tilcon_init_multistate(cid, &objectname_w, "objectname", value);
 *    Note: Treat Colorstate Objects as Multistate Ones
 *	tilcon_init_text(cid, &objectname_w, "objectname", "Char");
 *  tilcon_init_dbl_text(cid, &objectname_w, "objectname", value, format, precision);
 *  tilcon_init_int_text(cid, &objectname_w, "objectname", value, format);
 *
 *  Format: %5.1f = 5 characters total with 1 digit shown after the decimel
 *  Example: "Leading Text  %5.1f Trailing Text"
 *
 */
static void init_widgets(pid_t cid)
{
	
	
	
}
	


/**\ application_set_tilcon()
 *
 * This function continually sets the values for Tilcon objects.
 *
 */
static void application_set_tilcon(pid_t cid, void *data)
{
	/**\ Local Variable Declarations Syntax
	 * 
	 * (static) double name;
	 * (static) int name;
	 * (static) char name[length];
	 *
	 */
	
			
	/**\ Database Variable Type Declarations
	 *
	 * Examples:
	 * gyro_typ db_gyro;			// from gyro.h
	 * path_gps_point_t db_gps;		// from path_gps_lib.h (line 131)
	 *
	 */
	
	
	/**\ Database Reads 
	 *
	 * Example Read Commands:
	 * db_clt_read(pclt, DB_GYRO_VAR, sizeof(db_gyro), &db_gyro);
	 * db_clt_read(pclt, DB_GPS_PT_LCL_VAR, sizeof(db_gps), &db_gps);
	 *
	 * Example Use Commands
	 * db_gyro.gyro_rate
	 * db_gps.num_sats 
	 *
	 */
	
	
	/**\ Widget Update Code Using the Following Tilcon Command Examples
	 *
	 *	tilcon_hide/unhide(cid, &name_w);
	 *	tilcon_dim/undim(cid, &name_w);
	 *	tilcon_set_type(cid, &name_w, parameters);
	 *
	 *	Parameter List by Object Type:
	 *
	 *  button (label_char, in_fill_color, out_fill_color)
	 *	drawing (fill_color, line_color)
	 *	meter (value, indicator_fill_color) *Note: Needles use TRANSPARENT as Indicator Fill
	 *	meter_alarm (low_color, mid_color, high_color, start_at, low_to, mid_to, high_to)
	 *	multistate (value) *Note: Treat Colorstate Objects as Multistate Ones
	 *	text (char)
	 *  dbl_text (value)
	 *  int_text (value)
	 *
	 */

	
	
}

