/**\file 
 *
 *  Copyright (c) 2006   Regents of the University of California
 *
 *  Process to change the current values stored in the database for 
 *  testing for the On-Board Monitoring project 
 *
 */

#include 	<sys_os.h>
#include	"sys_rt.h"
#include	"db_clt.h"
#include	"db_utils.h"
#include	"clt_vars.h"
#include	"obm_test.h"
#include 	"widget.h"	// for color value

static int sig_list[]=
{
	SIGINT,
	SIGQUIT,
	SIGTERM,
	SIGALRM,	// for timer
	ERROR
};

static jmp_buf exit_env;
static void sig_hand(int code)
{
	if (code == SIGALRM)
		return;
	else
		longjmp(exit_env, code);
}

static db_id_t db_vars_list[] = {
	{DB_OBM_RADAR_VAR, sizeof(obm_radar_typ)},
	{DB_OBM_METER_VAR, sizeof(obm_meter_typ)}};

#define NUM_DB_VARS sizeof(db_vars_list)/sizeof(db_id_t)

int main(int argc, char *argv[])
{
	db_clt_typ *pclt;              /* Database client pointer */
	char hostname[MAXHOSTNAMELEN+1];
	char *domain = DEFAULT_SERVICE;
	int value;
	int opt;
	int xport = COMM_PSX_XPORT;	// Posix message queus, Linux
	obm_radar_typ db_radar;
	obm_meter_typ db_meter;
	int color_choice = 0;	// cycles through colors in color_array
	int color_array[] = {BUTTON_GREY, LATERAL_BLUE, WHITE, ACC_BROWN};
	int max_colors = sizeof(color_array)/sizeof(int);

	while ((opt = getopt(argc, argv, "d:x:")) != -1) {
		switch (opt) {
		  case 'd':
			domain = optarg;
			if (strlen(domain) > 15)
				domain[15] = '\0';
			break;
		  case 'x':
			xport = atoi(optarg);	// for QNX4 or other option
			break;
		  default:
			printf("Usage: chgdii -d [domain] -x [xport] \n");
			exit(1);
		}
	}

	/* This program writes DB variables, so it needs to do a db_create
	 * for each variable it writes. This is done by db_list_init,
	 * along with clt_login.
	 */
	  
	get_local_name(hostname, MAXHOSTNAMELEN);
	if ((pclt = db_list_init(argv[0], hostname, domain, xport,
			db_vars_list, NUM_DB_VARS, NULL, 0))
					 == NULL) {
		printf("Database initialization error in %s.\n", argv[0]);
		exit (EXIT_FAILURE);
	}

	/** Catch signals, clean-up and exit */
	if(setjmp(exit_env) != 0) {
		/* Log out from the database and destroy variables */
		if (pclt != NULL)
			db_list_done(pclt, db_vars_list, NUM_DB_VARS,
					NULL, 0);
		exit(EXIT_SUCCESS);
	} else
		sig_ign(sig_list, sig_hand);

	/** Initialize database variables */
	db_radar.radar_status = 1;
	db_clt_write(pclt, DB_OBM_RADAR_VAR, sizeof(obm_radar_typ), &db_radar);

	db_meter.meter_val = 75.0;
	db_meter.meter_fill = color_array[0];
	color_choice = 0;
	db_clt_write(pclt, DB_OBM_METER_VAR, sizeof(obm_meter_typ), &db_meter);

	for(;;) {
		printf("Please enter choice (1,2,3,4,5) (CTRL-C to exit):\n");
		printf("1) Set radar status to 1\n");
		printf("2) Set radar status to 0\n");
		printf("3) Increase meter value by 5\n");
		printf("4) Decrease meter value by 5\n");
		printf("5) Change meter fill color\n");
		scanf("%d", &value);
		switch (value) {
		case 1:
			db_radar.radar_status = 1;
			db_clt_write(pclt, DB_OBM_RADAR_VAR,
				 sizeof(obm_radar_typ), &db_radar);
			break;
		case 2:
			db_radar.radar_status = 0;
			db_clt_write(pclt, DB_OBM_RADAR_VAR,
				 sizeof(obm_radar_typ), &db_radar);
			break;
		case 3:
			db_meter.meter_val += 5.0;
			if ((db_meter.meter_val) > 100.0)
				db_meter.meter_val = 0;
			db_clt_write(pclt, DB_OBM_METER_VAR,
				 sizeof(obm_meter_typ), &db_meter);
			break;
		case 4:
			db_meter.meter_val -= 5.0;
			if ((db_meter.meter_val) < 0.0)
				db_meter.meter_val = 100.0;
			db_clt_write(pclt, DB_OBM_METER_VAR,
				 sizeof(obm_meter_typ), &db_meter);
			break;
		case 5:
			color_choice += 1;
			if (color_choice == max_colors)
				color_choice = 0;
			db_meter.meter_fill = color_array[color_choice];
			db_clt_write(pclt, DB_OBM_METER_VAR,
				 sizeof(obm_meter_typ), &db_meter);
			break;
		default:
			printf("Not a valid choice\n");
			break;
		}
	}
}


