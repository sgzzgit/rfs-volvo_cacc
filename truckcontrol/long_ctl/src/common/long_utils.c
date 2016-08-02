/**\file
 * Utility functions for accessing database or configuration
 *		files to be called by longitudinal control code. 
 *
 * Copyright (c) 2002   Regents of the University of California
 *
 */

#include "long_ctl.h"

static char *month_names[] = {
	"jan",
	"feb",
	"mar",
	"apr",
	"may",
	"jun",
	"jul",
	"aug",
	"sep",
	"oct",
	"nov",
	"dec",
};

/**
 * long_get_output_stream will open an output data file with a unique name in
 * the format "prefix_monxx_yyy.dat" where prefix is a parameter to the
 * function, month is a 3 letter month abbreviation. Serial numbers go from 0 to 999, then start over,
 * so at most 999 data files with the same prefix can be gathered
 * on a given day.. 
 *
 * If the value in old_fileday does not match the current date, the
 * serial number (file_serialno) will be reset to zero.  This will happen
 * the first time the function is called (since old_fileday was deliberately
 * initialized to 99, an invalid day) and if the system ever runs through
 * a day change at midnight.
 *
 * May be used either to open a single file or to open multiple files with
 * the same prefix in the same run. Tries to open files with the same
 * prefix starting from serial number 0, and opens the first one not
 * present.
 */
FILE * long_get_output_stream(char *prefix)
{
	struct timeb timeptr_raw;
	struct tm time_converted;
	static int old_fileday = 99;	/* initialized to illegal value */
	static int file_serialno;
	char filename[80];
	bool_typ file_exists = TRUE;
	FILE *fpout;

	/* Get date information. */
	ftime ( &timeptr_raw );
	_localtime ( &timeptr_raw.time, &time_converted );

	/* If old_fileday is not the same as time_converted.tm_mday, reset
	 * the serial number to zero.  This will happen first time through, or
	 * if we run through midnight. */
	if ( old_fileday != time_converted.tm_mday )
		file_serialno = 0;
	old_fileday = time_converted.tm_mday;

	while (file_exists == TRUE) {
		if (file_serialno > 999)
			return NULL;
		sprintf(filename, "%s_%s%2.2d_%3.3d.dat",
			prefix, month_names[time_converted.tm_mon],
			time_converted.tm_mday, file_serialno);
		fpout = fopen( filename, "r" );
		if ( fpout == NULL )
			file_exists = FALSE;
		else {
			fclose(fpout);
			file_serialno++;
	        }
	}

	/* We've found a valid serial number (and filename for fpout file is
	 * still stored in the variable filename).  
	 */
	fpout = fopen( filename, "w" );
	file_serialno++;
	printf( "Recording data to %s\n", filename );
	return (fpout);

}
	       
/**
 * long_set_params sets up parameters in the control structure
 * used when calculating the control commands. Currently 
 * parses command line arguments and reads task numbers and parameters
 * from initialization files. Design for demo configuration files still to
 * be determined, current set-up used for cmdtest. 
 */
 
int long_set_params(int argc, char **argv, long_params *pparams)

{
        int ch;
	char vehicle_str[80];
	FILE *fpin;

        /* Set up default filenames */
	strcpy(pparams->avcs_cfg_file, "/home/truckcontrol/test/realtime.ini"); 
	strcpy(pparams->long_cfg_file, "long_ctl.ini");
	strcpy(pparams->data_file, "long_ctl.out");
	strcpy(vehicle_str, "C2");

        /* Specify test number with command line argument */
        while ((ch = getopt(argc, argv, "f:o:p:r:s:v:")) != EOF) {
                switch (ch) {
                case 'f': strcpy(pparams->long_cfg_file, optarg);
			  printf("long_cfg_file %s\n", pparams->long_cfg_file);
                          break;
                case 'o': strcpy(pparams->data_file, optarg);
			  printf("data_file %s\n", pparams->data_file);
                          break;
                case 'r': pparams->cmd_test = atoi(optarg);
                          break;
//                case 'p': pparams->vehicle_position = atoi(optarg);
//                          break;
                case 'v': strcpy(vehicle_str, optarg); 
                          break;
                default:  printf("Usage: %s ", argv[0]);
                          printf("-f Command task configuration file\n"); 
                          printf("-r Command task number\n"); 
                          return 0;
                          break;
                }
        }
//	printf("pparams->vehicle_position %d\n", 
//		pparams->vehicle_position);
	if (strcmp(vehicle_str,"Blue") == 0)  
		pparams->vehicle_type = VEH_TYPE_TRUCK_BLUE;
	else if (strcmp(vehicle_str,"Gold") == 0) 
		pparams->vehicle_type = VEH_TYPE_TRUCK_GOLD;
	else if (strcmp(vehicle_str,"Silvr") == 0)  
		pparams->vehicle_type = VEH_TYPE_TRUCK_SILVR;
	else if (strcmp(vehicle_str,"Volvo_1") == 0)  
		pparams->vehicle_type = VEH_TYPE_TRUCK_VOLVO_1;
	else if (strcmp(vehicle_str,"Volvo_2") == 0)  
		pparams->vehicle_type = VEH_TYPE_TRUCK_VOLVO_2;
	else if (strcmp(vehicle_str,"Volvo_3") == 0)  
		pparams->vehicle_type = VEH_TYPE_TRUCK_VOLVO_3;
	else if (strcmp(vehicle_str, "D1") == 0) 
		pparams->vehicle_type = VEH_TYPE_BUS60;
	else
		pparams->vehicle_type = VEH_TYPE_TRUCK_VOLVO_1;

	printf("Vehicle %s,type %d\n", vehicle_str, pparams->vehicle_type);
        if ((fpin = get_ini_section("/home/truckcontrol/test/realtime.ini", vehicle_str))
								 == NULL) {
                printf("long_utils: can't get ini file %s, section %s\n",
                   "/home/truckcontrol/test/realtime.ini", vehicle_str);
                fflush(stdout);
                return 0;
        }
	else
		printf("long_utils: initialization file %s\n", "/home/truckcontrol/test/realtime.ini");

        pparams->max_engine_speed =
		 get_ini_double(fpin, "MaxEngineSpeed", 2000.0);    /* RPM */
        pparams->engine_reference_torque = 
		 get_ini_double(fpin, "EngineReferenceTorque", 
					MAX_ENGINE_TORQUE);    /* RPM */
	pparams->retarder_reference_torque = 
		 get_ini_double(fpin, "RetarderReferenceTorque", 
					MAX_RETARDER_TORQUE);    /* RPM */

	printf("max_engine speed %.3f, engine_reference_torque %.3f\n",
		pparams->max_engine_speed, pparams->engine_reference_torque);
	printf("retarder_reference_torque %.3f\n",
		pparams->retarder_reference_torque);
	/* should these be configurable in the file? */
	pparams->max_iterations = 0;
        pparams->ctrl_interval = get_ini_long(fpin,"CtrlInterval", 20); //in ms
        pparams->vehicle_position = get_ini_long(fpin,"PositionInPlatoon", 1);
        pparams->mdl_lidar = get_ini_bool(fpin,"UseMDL", FALSE);
        pparams->denso_lidar = get_ini_bool(fpin,"UseLidar", FALSE);
	
        return 1;
}

/**
 * long_print_params prints long_params member read from realtime.ini
 * and command line arguments, for documentation or debugging. 
 *
 */
void long_print_params(FILE *fp, long_params *pparams)
{
	fprintf(fp, "avcs_cfg_file %s\n", pparams->avcs_cfg_file);
	fprintf(fp, "long_cfg_file %s\n", pparams->long_cfg_file);
	fprintf(fp, "data_file %s\n", pparams->data_file);
	fprintf(fp, "max_engine_speed %.3f\n", pparams->max_engine_speed);
	fprintf(fp, "engine_reference_torque %.3f\n", pparams->engine_reference_torque);
	fprintf(fp, "retarder_reference_torque %.3f\n", pparams->retarder_reference_torque);
	fprintf(fp, "cmd_test %d\n", pparams->cmd_test);
}

/** Jbus database variable list for 40-foot CNG buses */
static int bus40_jdbv_list[] = {
		DB_J1939_EEC1_VAR,
		DB_J1939_EEC2_VAR,
		DB_J1939_EEC3_VAR,
		DB_J1939_ERC1_VAR,
		DB_J1939_ETC1_VAR,
		DB_J1939_ETC2_VAR,
		DB_J1939_EBC1_VAR,
		DB_J1939_EBC2_VAR,
		DB_J1939_CCVS_VAR,
		DB_J1939_LFE_VAR,
		DB_J1939_FD_VAR,
		DB_J1939_IEC_VAR,
		DB_J1939_IEC_VAR,
		DB_J1939_GFI2_VAR,
		DB_J1939_EI_VAR,
};

/** Jbus database variable list for 60-foot Detroit Diesel bus */
static int bus60_jdbv_list[] = {
		DB_J1939_EEC1_VAR,
		DB_J1939_EEC2_VAR,
		DB_J1939_EEC3_VAR,
		DB_J1939_ERC1_VAR,
		DB_J1939_ETC1_VAR,
		DB_J1939_ETC2_VAR,
		DB_J1939_EBC1_VAR,
		DB_J1939_EBC2_VAR,
		DB_J1939_CCVS_VAR,
		DB_J1939_LFE_VAR,
		DB_J1939_FD_VAR,
		DB_J1939_IEC_VAR,
		DB_J1939_EI_VAR,
};

/** Input variable list for longitudinal control process on Freightliner trucks
 */
static int trk_jdbv_list[] = {
		DB_J1939_EEC1_VAR,
		DB_J1939_EEC2_VAR,
		DB_J1939_EEC3_VAR,
		DB_J1939_ERC1_VAR,
		DB_J1939_ETC1_VAR,
		DB_J1939_ETC2_VAR,
		DB_J1939_EBC1_VAR,
		DB_J1939_EBC2_VAR,
		DB_J1939_EBC5_VAR,
		DB_J1939_CCVS_VAR,
		DB_J1939_LFE_VAR,
		DB_J1939_IEC_VAR,
		DB_J1939_VDC2_VAR,
		DB_J1939_VP_X_VAR,
		DB_J1939_TSC1_E_ACC_VAR,
		DB_J1939_TSC1_ER_ACC_VAR,
		DB_J1939_TSC1_E_A_VAR,
		DB_J1939_TSC1_E_T_VAR,
		DB_J1939_TSC1_E_V_VAR,
		DB_J1939_TSC1_ER_A_VAR,
		DB_J1939_TSC1_ER_T_VAR,
		DB_J1939_TSC1_ER_V_VAR,
		DB_J1939_VOLVO_TARGET_VAR,
		DB_J1939_VOLVO_EGO_VAR,
		DB_J1939_VOLVO_BRK_VAR,
		DB_J1939_VOLVO_VP15_VAR,
};

static db_id_t trk_in_dbv_list[] = {
        {DB_LONG_INPUT_VAR, sizeof( long_input_typ )},
        {DB_LONG_DIG_IN_VAR, sizeof( long_dig_in_typ )},
        {DB_SELF_GPS_POINT_VAR, sizeof( path_gps_point_t )},
        {DB_COMM_LEAD_TRK_VAR, sizeof( veh_comm_packet_t )},
        {DB_COMM_SECOND_TRK_VAR, sizeof( veh_comm_packet_t )},
        {DB_COMM_THIRD_TRK_VAR, sizeof( veh_comm_packet_t )},
        {DB_EVT300_RADAR1_VAR, sizeof( evt300_radar_typ)},
        {DB_LONG_LIDARA_VAR, sizeof( long_lidarA_typ)},
        {DB_LONG_LIDARB_VAR, sizeof( long_lidarB_typ)},
        {DB_MDL_LIDAR_VAR, sizeof( mdl_lidar_typ)},
        {DB_DVI_OUT_VAR, sizeof(dvi_out_t)}
};

static int trk_in_dbv_list_size = sizeof(trk_in_dbv_list)/sizeof(db_id_t);

void long_set_dbv_list(long_ctrl *pctrl)
{
	long_params *pparams = &pctrl->params;
	fprintf(stderr, "Setting dbv list for ");
	switch (pparams->vehicle_type) {
	case VEH_TYPE_TRUCK_SILVR:
	case VEH_TYPE_TRUCK_BLUE:
	case VEH_TYPE_TRUCK_GOLD:
	case VEH_TYPE_TRUCK_VOLVO_1:
	case VEH_TYPE_TRUCK_VOLVO_2:
	case VEH_TYPE_TRUCK_VOLVO_3:
		fprintf(stderr, " truck\n");
		pparams->dbv_list = &trk_jdbv_list[0];
		pparams->dbv_list_size = sizeof(trk_jdbv_list)/sizeof(int);
		break;
	case VEH_TYPE_BUS40:
		fprintf(stderr, " 40-ft bus\n");
		pparams->dbv_list = &bus40_jdbv_list[0];
		pparams->dbv_list_size = sizeof(bus40_jdbv_list)/sizeof(int);
		break;
	case VEH_TYPE_BUS60:
		fprintf(stderr, " 60-ft bus\n");
		pparams->dbv_list = &bus60_jdbv_list[0];
		pparams->dbv_list_size = sizeof(bus60_jdbv_list)/sizeof(int);
		break;
	default:
		fprintf(stderr, " truck\n");
		pparams->dbv_list = &trk_jdbv_list[0];
		pparams->dbv_list_size = sizeof(trk_jdbv_list)/sizeof(int);
		break;
	}
}

/**
 * Create jbus database variables and make them active.
 * long_database_init logs in to the database process and makes sure the
 * variables in the jdbv_list and in the in_dbv_list for the
 * control process have been created; clt_create will return an error
 * if this variable has already been created, so we ignore the return
 * value; this is just to make sure the variable has been created
 * when we read or set a trigger on it. Normally the process that writes
 * the variable will have created it. 
 *
 * The active flag is set in the structure kept by the J1939 support
 * code for the rdj1939 process, for each variable on the vehicle list
 * of jbus variables. For now this active flag is only used by
 * long_read_vehicle_state to decide whether to read a variable from
 * the database, but later the J1939 support code may use this flag
 * to decide whether or not to write the variable to the data base.
 * So it is important that the flag is set during initialization
 * for any jbus database variable of interest. For now, the variable
 * numbers of active jbus variables for each vehicle type are kept in
 * the staticly initialized array bus40_jdbv_list, bus60_jdbv_list and
 * trk_jdbv_list -- later we may want to initialize these dynamically
 * for different applications.
 */ 
db_clt_typ *long_database_init(char *phost, char *pclient, long_ctrl *pctrl)
{
        db_clt_typ *pclt;
	long_params *pparams = &pctrl->params;
	int i;

	long_set_dbv_list(pctrl);

        if ((pclt = clt_login(pclient, phost, DEFAULT_SERVICE,
                                                 COMM_OS_XPORT)) == NULL)
                return((db_clt_typ *) NULL);

	for (i = 0; i < pparams->dbv_list_size; i++) { 
		int db_num = pparams->dbv_list[i]; 
		j1939_dbv_info *pinfo = get_jdbv_info(db_num);
		clt_create(pclt, db_num, db_num, pinfo->dbv_size);
		pinfo->active = 1;
	}

	// The following will need to be generalized if we ever use this code
	// for a project other thatn the truck project.
	for (i = 0; i < trk_in_dbv_list_size; i++) {
		int db_num = trk_in_dbv_list[i].id; 
		clt_create(pclt, db_num, db_num, trk_in_dbv_list[i].size);
	}
		
	return(pclt);
}

/**
 * long_trigger makes sure a variable is written before it is read. 
 * This assumes that these variables are updated frequently. 
 * Also assumes that database variable and type numbers are the same.
 */
int long_trigger(db_clt_typ *pclt, unsigned int db_num)
{
        pid_t trig_pid;
        trig_info_typ trig_msg;

        if (!clt_trig_set(pclt, db_num, db_num))
                return 0;
        while (1) {
// Old QNX4 Receive call: trig_pid = Receive(0, &trig_msg, sizeof(trig_msg));
                trig_pid = clt_ipc_receive(pclt, &trig_msg, sizeof(trig_msg));
                 if (DB_TRIG_VAR(&trig_msg) == db_num)
                        break;
        } 
        return 1;
}

/**
 * long_wait_for_init
 * Makes sure essential database variables used to set state have been
 * written before control begins. Later may interact with DVI or other vehicles.
 */
int long_wait_for_init(db_clt_typ *pclt, long_ctrl *pctrl)
{
	/* Trigger on a few frequently updated variables to make sure
	 * J1939 bus is alive.
	 */
        if (!long_trigger(pclt, DB_J1939_EEC1_VAR))
                return 0;
        if (!long_trigger(pclt, DB_J1939_EEC2_VAR))
                return 0;
        if (!long_trigger(pclt, DB_J1939_CCVS_VAR))
                return 0;
	printf("Engine jbus alive\n");
	fflush(stdout);
        if (!long_trigger(pclt, DB_J1939_ETC1_VAR))
                return 0;
	printf("Transmission jbus alive\n");
	fflush(stdout);
	
	/* Useful fields of messages that might be missing are initialized
	 * to error values in database.
	 */
                                 
//	erc1.actual_retarder_percent_torque = 1.0; /* small out of range */
//	erc1.engine_retarder_torque_mode = -1;	   /* small out of range */
//	if (!clt_update(pclt, DB_J1939_ERC1_VAR, DB_J1939_ERC1_TYPE,
 //                   sizeof(j1939_erc1_typ), (void *) &erc1))
//		return 0;

//	ebc1.EBSBrakeSwitch = 3;
//	ebc1.ABSEBSAmberWarningSignal = 0;
//	ebc1.EBC1_BrakePedalPosition = -1.0;	/* small out of range */
//	ebc1.total_brake_demand = 1.0;		/* small out of range */
//	if (!clt_update(pclt, DB_J1939_EBC1_VAR, DB_J1939_EBC1_TYPE,
//                    sizeof(j1939_ebc1_typ), (void *) &ebc1))
//		return 0;

//	ebc2.EBC2_FrontAxleSpeed = -1.0;		/* small out of range */
//	if (!clt_update(pclt, DB_J1939_EBC2_VAR, DB_J1939_EBC2_TYPE,
//                    sizeof(j1939_ebc2_typ), (void *) &ebc2))
//		return 0;

//	etc2.current_gear = 9;			/* small out of range */
//	if (!clt_update(pclt, DB_J1939_ETC2_VAR, DB_J1939_ETC2_TYPE,
//                    sizeof(j1939_etc2_typ), (void *) &etc2))
//		return 0;
                                 
	long_read_vehicle_state(pclt, pctrl);
        return 1;
}
/**
 * Update vehicle state fields for a variable db_num, after data has
 * been read from data base into pdata_val.
 */ 
void long_update_fields_from_dbv(int db_num, long_vehicle_state *pstate,
			long_params *pparams, void *pdata_val)
{
        j1939_eec1_typ *peec1;
        j1939_eec2_typ *peec2;
        j1939_eec3_typ *peec3;
        j1939_erc1_typ *perc1;
        j1939_ebc1_typ *pebc1;
        j1939_ebc2_typ *pebc2;
        j1939_ebc5_typ *pebc5;
	j1939_volvo_brk_t *pvolvo_brk;
	j1939_volvo_vp15_t *pvolvo_vp15;
        j1939_ccvs_typ *pccvs;
        j1939_etc1_typ *petc1;
	j1939_etc2_typ *petc2;
	j1939_tsc1_typ *ptsc1_e_t;
	j1939_tsc1_typ *ptsc1_e_a;
	j1939_tsc1_typ *ptsc1_e_v;
	j1939_tsc1_typ *ptsc1_er_t;
	j1939_tsc1_typ *ptsc1_er_a;
	j1939_tsc1_typ *ptsc1_er_v;
	j1939_tsc1_typ *ptsc1_e_acc;
	j1939_tsc1_typ *ptsc1_er_acc;
	j1939_lfe_typ *plfe;
	j1939_vdc2_typ *pvdc2;
	j1939_vp_x_typ *pvp_x;
	j1939_fd_typ *pfd;
	j1939_iec_typ *piec;
	j1939_gfi2_typ *pgfi2;
	j1939_ei_typ *pei;
	j1939_volvo_target_typ *pvolvo_target;
	j1939_volvo_ego_typ *pvolvo_ego;
	j1939_volvo_xbr_typ *pvolvo_xbr;
	j1939_volvo_xbr_warn_typ *pvolvo_xbr_warn;
	long_input_typ *plong_in;
	evt300_radar_typ *pevt300;
	dvi_out_t *pdvi_out;
	long_lidarA_typ *plidarA;	
	long_lidarB_typ *plidarB;	
	mdl_lidar_typ *pmdl_lidar;	
	path_gps_point_t *pself_gps;	
	veh_comm_packet_t *plead_trk;
	veh_comm_packet_t *psecond_trk;
	veh_comm_packet_t *pthird_trk;
	long_dig_in_typ *pdig_in;

	switch (db_num) {
	case DB_J1939_VOLVO_TARGET_VAR:
		pvolvo_target = (j1939_volvo_target_typ *) pdata_val;
		pstate->Volvo_TargetDist = pvolvo_target->TargetDist;
		pstate->Volvo_TargetVel = pvolvo_target->TargetVel;
		pstate->Volvo_TargetAcc = pvolvo_target->TargetAcc;
		pstate->Volvo_TargetAvailable = pvolvo_target->TargetAvailable;
		break;
	case DB_J1939_VOLVO_EGO_VAR:
		pvolvo_ego = (j1939_volvo_ego_typ *) pdata_val;
		pstate->Volvo_EgoVel = pvolvo_ego->EgoVel;
		pstate->Volvo_EgoAcc = pvolvo_ego->EgoAcc;
		pstate->Volvo_EgoRoadGrade = pvolvo_ego->EgoRoadGrade;
		break;
	case DB_J1939_TSC1_E_T_VAR:
		ptsc1_e_t = (j1939_tsc1_typ *) pdata_val;
		pstate->TSC1_EMSTECU_OvrdCtrlMPr = ptsc1_e_t->EnOvrdCtrlMPr;
		pstate->TSC1_EMSTECU_EnRSpdCtrlC = ptsc1_e_t->EnRSpdCtrlC;
		pstate->TSC1_EMSTECU_EnOvrdCtrlM = ptsc1_e_t->EnOvrdCtrlM;
		pstate->TSC1_EMSTECU_EnRSpdSpdLm = ptsc1_e_t->EnRSpdSpdLm; 
		pstate->TSC1_EMSTECU_EnRTrqTrqLm = ptsc1_e_t->EnRTrqTrqLm;
		pstate->TSC1_EMSTECU_destination_address = ptsc1_e_t->destination_address;
		pstate->TSC1_EMSTECU_src_address = ptsc1_e_t->src_address;
		break;
	case DB_J1939_TSC1_E_A_VAR:
		ptsc1_e_a = (j1939_tsc1_typ *) pdata_val;
		pstate->TSC1_EMSABS_OvrdCtrlMPr = ptsc1_e_a->EnOvrdCtrlMPr;
		pstate->TSC1_EMSABS_EnRSpdCtrlC = ptsc1_e_a->EnRSpdCtrlC;
		pstate->TSC1_EMSABS_EnOvrdCtrlM = ptsc1_e_a->EnOvrdCtrlM;
		pstate->TSC1_EMSABS_EnRSpdSpdLm = ptsc1_e_a->EnRSpdSpdLm; 
		pstate->TSC1_EMSABS_EnRTrqTrqLm = ptsc1_e_a->EnRTrqTrqLm;
		break;
	case DB_J1939_TSC1_E_V_VAR:
		ptsc1_e_v = (j1939_tsc1_typ *) pdata_val;
		pstate->TSC1_EMSVMCUes_OvrdCtrlMPr = ptsc1_e_v->EnOvrdCtrlMPr;
		pstate->TSC1_EMSVMCUes_EnRSpdCtrlC = ptsc1_e_v->EnRSpdCtrlC;
		pstate->TSC1_EMSVMCUes_EnOvrdCtrlM = ptsc1_e_v->EnOvrdCtrlM;
		pstate->TSC1_EMSVMCUes_EnRSpdSpdLm = ptsc1_e_v->EnRSpdSpdLm; 
		pstate->TSC1_EMSVMCUes_EnRTrqTrqLm = ptsc1_e_v->EnRTrqTrqLm;
		break;
	case DB_J1939_TSC1_E_ACC_VAR:
		ptsc1_e_acc = (j1939_tsc1_typ *) pdata_val;
		pstate->TSC1_EMS_ACC_OvrdCtrlMPr = ptsc1_e_acc->EnOvrdCtrlMPr;
		pstate->TSC1_EMS_ACC_EnRSpdCtrlC = ptsc1_e_acc->EnRSpdCtrlC;
		pstate->TSC1_EMS_ACC_EnOvrdCtrlM = ptsc1_e_acc->EnOvrdCtrlM;
		pstate->TSC1_EMS_ACC_EnRSpdSpdLm = ptsc1_e_acc->EnRSpdSpdLm; 
		pstate->TSC1_EMS_ACC_EnRTrqTrqLm = ptsc1_e_acc->EnRTrqTrqLm;
		break;
	case DB_J1939_TSC1_ER_T_VAR:
		ptsc1_er_t = (j1939_tsc1_typ *) pdata_val;
		pstate->TSC1_EMSrTECU_OvrdCtrlMPr = ptsc1_er_t->EnOvrdCtrlMPr;
		pstate->TSC1_EMSrTECU_EnRSpdCtrlC = ptsc1_er_t->EnRSpdCtrlC;
		pstate->TSC1_EMSrTECU_EnOvrdCtrlM = ptsc1_er_t->EnOvrdCtrlM;
		pstate->TSC1_EMSrTECU_EnRSpdSpdLm = ptsc1_er_t->EnRSpdSpdLm; 
		pstate->TSC1_EMSrTECU_EnRTrqTrqLm = ptsc1_er_t->EnRTrqTrqLm;
		break;
	case DB_J1939_TSC1_ER_A_VAR:
		ptsc1_er_a = (j1939_tsc1_typ *) pdata_val;
		pstate->TSC1_EMSrABS_OvrdCtrlMPr = ptsc1_er_a->EnOvrdCtrlMPr;
		pstate->TSC1_EMSrABS_EnRSpdCtrlC = ptsc1_er_a->EnRSpdCtrlC;
		pstate->TSC1_EMSrABS_EnOvrdCtrlM = ptsc1_er_a->EnOvrdCtrlM;
		pstate->TSC1_EMSrABS_EnRSpdSpdLm = ptsc1_er_a->EnRSpdSpdLm; 
		pstate->TSC1_EMSrABS_EnRTrqTrqLm = ptsc1_er_a->EnRTrqTrqLm;
		break;
	case DB_J1939_TSC1_ER_V_VAR:
		ptsc1_er_v = (j1939_tsc1_typ *) pdata_val;
		pstate->TSC1_ER_V_OvrdCtrlMPr = ptsc1_er_v->EnOvrdCtrlMPr;
		pstate->TSC1_ER_V_EnRSpdCtrlC = ptsc1_er_v->EnRSpdCtrlC;
		pstate->TSC1_ER_V_EnOvrdCtrlM = ptsc1_er_v->EnOvrdCtrlM;
		pstate->TSC1_ER_V_EnRSpdSpdLm = ptsc1_er_v->EnRSpdSpdLm; 
		pstate->TSC1_ER_V_EnRTrqTrqLm = ptsc1_er_v->EnRTrqTrqLm;
		break;
	case DB_J1939_TSC1_ER_ACC_VAR:
		ptsc1_er_acc = (j1939_tsc1_typ *) pdata_val;
		pstate->TSC1_ER_ACC_OvrdCtrlMPr = ptsc1_er_acc->EnOvrdCtrlMPr;
		pstate->TSC1_ER_ACC_EnRSpdCtrlC = ptsc1_er_acc->EnRSpdCtrlC;
		pstate->TSC1_ER_ACC_EnOvrdCtrlM = ptsc1_er_acc->EnOvrdCtrlM;
		pstate->TSC1_ER_ACC_EnRSpdSpdLm = ptsc1_er_acc->EnRSpdSpdLm; 
		pstate->TSC1_ER_ACC_EnRTrqTrqLm = ptsc1_er_acc->EnRTrqTrqLm;
		break;
	case DB_LONG_INPUT_VAR:
		plong_in = (long_input_typ *) pdata_val;
		pstate->acc_pedal_voltage = plong_in->acc_pedal;
		break;
	case DB_J1939_EEC1_VAR: 
		peec1 = (j1939_eec1_typ *) pdata_val;
		pstate->EEC1_EngineSpeed = peec1->EEC1_EngineSpeed;
		pstate->EEC1_ActualEnginePercentTorque = peec1->EEC1_ActualEnginePercentTorque;
		pstate->EEC1_EngineTorqueMode = peec1->EEC1_EngineTorqueMode;
		pstate->EEC1_DrvrDemandEngPercentTorque = peec1->EEC1_DrvrDemandEngPercentTorque;
		pstate->EEC1_EngDemandPercentTorque = peec1->EEC1_EngDemandPercentTorque;
		break;
        case DB_J1939_EEC2_VAR:
		peec2 = (j1939_eec2_typ *) pdata_val;
		pstate->EEC2_AccelPedal1LowIdleSwitch = peec2->EEC2_AccelPedal1LowIdleSwitch;
		pstate->EEC2_AccelPedal2LowIdleSwitch = peec2->EEC2_AccelPedal2LowIdleSwitch;
		pstate->EEC2_AccelPedalKickdownSwitch = peec2->EEC2_AccelPedalKickdownSwitch;
		pstate->EEC2_AccelPedalPos1 = peec2->EEC2_AccelPedalPos1;
		pstate->EEC2_AccelPedalPos2 = peec2->EEC2_AccelPedalPos2;
		pstate->EEC2_ActMaxAvailEngPercentTorque = peec2->EEC2_ActMaxAvailEngPercentTorque;
		pstate->EEC2_EnginePercentLoadAtCurrentSpd = peec2->EEC2_EnginePercentLoadAtCurrentSpd;
		pstate->EEC2_RoadSpeedLimitStatus = peec2->EEC2_RoadSpeedLimitStatus;
		break;
        case DB_J1939_EEC3_VAR:
        	peec3 = (j1939_eec3_typ *) pdata_val;
		pstate->EEC3_NominalFrictionPercentTorque = peec3->EEC3_NominalFrictionPercentTorque ;
		pstate->EEC3_EstEngPrsticLossesPercentTorque = peec3->EEC3_EstEngPrsticLossesPercentTorque ;
		pstate->EEC3_EngsDesiredOperatingSpeed = peec3->EEC3_EngsDesiredOperatingSpeed ;
//		pstate->nominal_friction_torque = 
//			peec3->nominal_friction_percent_torque *
//                                pparams->engine_reference_torque /100.0;
		break;
        case DB_J1939_ERC1_VAR:
        	perc1 = (j1939_erc1_typ *) pdata_val;
		pstate->ERC1ERActlMxAvlbRtdrtPerctTorque = perc1->ERC1ERActlMxAvlbRtdrtPerctTorque;
		pstate->ERC1ERActualEngineRetPercentTrq = perc1->ERC1ERActualEngineRetPercentTrq *
			pparams->retarder_reference_torque /100.0;
		pstate->ERC1ERDrvrsDmandRtdrPerctTorque = perc1->ERC1ERDrvrsDmandRtdrPerctTorque; 
		pstate->ERC1ERIntendedRtdrPercentTorque = perc1->ERC1ERIntendedRtdrPercentTorque;
		pstate->ERC1ERRetarderEnableShiftAssistSw = perc1->ERC1ERRetarderEnableShiftAssistSw;
		pstate->ERC1ERRetarderRqingBrakeLight = perc1->ERC1ERRetarderRqingBrakeLight;
		pstate->ERC1ERRetarderSelectionNonEng = perc1->ERC1ERRetarderSelectionNonEng;
		pstate->ERC1ERRetarderTorqueMode = perc1->ERC1ERRetarderTorqueMode;
		pstate->ERC1ERRtdrEnablBrakeAssistSwitch = perc1->ERC1ERRtdrEnablBrakeAssistSwitch;
		pstate->ERC1ERSrcAddrOfCtrlDvcFrRtdrCtrl = perc1->ERC1ERSrcAddrOfCtrlDvcFrRtdrCtrl;

//		pstate->retarder_torque =
//			 perc1->actual_retarder_percent_torque *
 //                               pparams->retarder_reference_torque /100.0;
//		pstate->retarder_mode = perc1->engine_retarder_torque_mode;
		break;
        case DB_J1939_EBC1_VAR:
		pebc1 = (j1939_ebc1_typ *) pdata_val;
		pstate->EBC1_BrakePedalPosition = pebc1->EBC1_BrakePedalPosition ;
		pstate->EBSBrakeSwitch = pebc1->EBSBrakeSwitch ;
		pstate->ABSEBSAmberWarningSignal = pebc1->ABSEBSAmberWarningSignal ;
		pstate->EBC1_ABSFullyOperational = pebc1->EBC1_ABSFullyOperational ;
		pstate->EBC1_AntiLockBrakingActive = pebc1->EBC1_AntiLockBrakingActive ;
		pstate->EBC1_ASRBrakeControlActive = pebc1->EBC1_ASRBrakeControlActive ;
		pstate->EBC1_ASREngineControlActive = pebc1->EBC1_ASREngineControlActive ;
		pstate->EBC1_ASROffroadSwitch = pebc1->EBC1_ASROffroadSwitch ;
		pstate->EBC1_EBSRedWarningSignal = pebc1->EBC1_EBSRedWarningSignal ;
		pstate->EBC1_EngRetarderSelection = pebc1->EBC1_EngRetarderSelection ;
		pstate->EBC1_RemoteAccelEnableSwitch = pebc1->EBC1_RemoteAccelEnableSwitch ;
		pstate->EBC1_SrcAddrssOfCtrllngDvcFrBrkCntrl = pebc1->EBC1_SrcAddrssOfCtrllngDvcFrBrkCntrl ;
		break;
        case DB_J1939_EBC2_VAR:
		pebc2 = (j1939_ebc2_typ *) pdata_val;
		pstate->EBC2_FrontAxleSpeed = pebc2->EBC2_FrontAxleSpeed ;    
		pstate->EBC2_RelativeSpeedFrontAxleLeftWheel = pebc2->EBC2_RelativeSpeedFrontAxleLeftWheel ;
		pstate->EBC2_RlativeSpeedFrontAxleRightWheel = pebc2->EBC2_RlativeSpeedFrontAxleRightWheel ;
		pstate->EBC2_RelativeSpeedRearAxle1LeftWheel = pebc2->EBC2_RelativeSpeedRearAxle1LeftWheel ;
		pstate->EBC2_RlativeSpeedRearAxle1RightWheel = pebc2->EBC2_RlativeSpeedRearAxle1RightWheel ;
		pstate->EBC2_RelativeSpeedRearAxle2LeftWheel = pebc2->EBC2_RelativeSpeedRearAxle2LeftWheel ;
		pstate->EBC2_RlativeSpeedRearAxle2RightWheel = pebc2->EBC2_RlativeSpeedRearAxle2RightWheel;
		break;
        case DB_J1939_VOLVO_BRK_VAR:
		pvolvo_brk = (j1939_volvo_brk_t *) pdata_val;
		pstate->VBRK_BrkAppPressure  = pvolvo_brk->VBRK_BrkAppPressure ;
		pstate->VBRK_BrkPrimaryPressure  = pvolvo_brk->VBRK_BrkPrimaryPressure ;
		pstate->VBRK_BrkSecondaryPressure  = pvolvo_brk->VBRK_BrkSecondaryPressure ;
		pstate->VBRK_BrkStatParkBrkActuator  = pvolvo_brk->VBRK_BrkStatParkBrkActuator ;
		pstate->VBRK_ParkBrkRedWarningSignal  = pvolvo_brk->VBRK_ParkBrkRedWarningSignal ;
		pstate->VBRK_ParkBrkReleaseInhibitStat  = pvolvo_brk->VBRK_ParkBrkReleaseInhibitStat ;
		break;
        case DB_J1939_VOLVO_VP15_VAR:
		pvolvo_vp15 = (j1939_volvo_vp15_t *) pdata_val;
		pstate->VP15_EcoRollStatus = pvolvo_vp15->VP15_EcoRollStatus;
		pstate->VP15_AutomaticHSARequest = pvolvo_vp15->VP15_AutomaticHSARequest;
		pstate->VP15_EngineShutdownRequest = pvolvo_vp15->VP15_EngineShutdownRequest;
		pstate->VP15_RoadInclinationVP15 = pvolvo_vp15->VP15_RoadInclinationVP15;
		pstate->VP15_PermittedHillHolderP = pvolvo_vp15->VP15_PermittedHillHolderP;
		pstate->VP15_RecommendedGearshift = pvolvo_vp15->VP15_RecommendedGearshift;
		pstate->VP15_EcoRollActiveStatus = pvolvo_vp15->VP15_EcoRollActiveStatus;
		pstate->VP15_ClutchOverloadStatus = pvolvo_vp15->VP15_ClutchOverloadStatus;
		pstate->VP15_PowerDownAcknowledge = pvolvo_vp15->VP15_PowerDownAcknowledge;
		pstate->VP15_DirectionGearAllowed = pvolvo_vp15->VP15_DirectionGearAllowed;
		pstate->VP15_VehicleWeightVP15 = pvolvo_vp15->VP15_VehicleWeightVP15;
		break;
        case DB_J1939_EBC5_VAR:
		pebc5 = (j1939_ebc5_typ *) pdata_val;
		pstate->EBC5_FoundationBrakeUse = pebc5->EBC5_FoundationBrakeUse ;
		pstate->EBC5_HaltBrakeMode = pebc5->EBC5_HaltBrakeMode ;
		pstate->EBC5_XBRAccelerationLimit = pebc5->EBC5_XBRAccelerationLimit ;
		pstate->EBC5_XBRActiveControlMode = pebc5->EBC5_XBRActiveControlMode ;
		break;
//        case DB_J1939_CAN1_VAR:
//		pcan1 = (j1939_can1_typ *) pdata_val;
//		break;
//        case DB_J1939_CAN2_VAR:
//		pcan2 = (j1939_can2_typ *) pdata_val;
//		break;
        case DB_J1939_CCVS_VAR:
		pccvs = (j1939_ccvs_typ *) pdata_val;
		pstate->CCVS_ParkingBrakeSwitch = pccvs->CCVS_ParkingBrakeSwitch ;
		pstate->CCVS_TwoSpeedAxleSwitch = pccvs->CCVS_TwoSpeedAxleSwitch ;
		pstate->CCVS_VehicleSpeed = pccvs->CCVS_VehicleSpeed ;
		pstate->CCVS_ParkBrakeReleaseInhibitRq  = pccvs->CCVS_ParkBrakeReleaseInhibitRq  ;
		pstate->CCVS_ClutchSwitch = pccvs->CCVS_ClutchSwitch ;
		pstate->CCVS_BrakeSwitch = pccvs->CCVS_BrakeSwitch ;
		pstate->CCVS_CruiseCtrlPauseSwitch = pccvs->CCVS_CruiseCtrlPauseSwitch ;
		pstate->CCVS_CruiseControlEnableSwitch = pccvs->CCVS_CruiseControlEnableSwitch ;
		pstate->CCVS_CruiseControlActive = pccvs->CCVS_CruiseControlActive ;
		pstate->CCVS_CruiseControlAccelerateSwitch = pccvs->CCVS_CruiseControlAccelerateSwitch ;
		pstate->CCVS_CruiseControlResumeSwitch = pccvs->CCVS_CruiseControlResumeSwitch ;
		pstate->CCVS_CruiseControlCoastSwitch = pccvs->CCVS_CruiseControlCoastSwitch ;
		pstate->CCVS_CruiseControlSetSwitch = pccvs->CCVS_CruiseControlSetSwitch ;
		pstate->CCVS_CruiseControlState = pccvs->CCVS_CruiseControlState ;
		pstate->CCVS_PtoState = pccvs->CCVS_PtoState ;
		pstate->CCVS_EngShutdownOverrideSwitch = pccvs->CCVS_EngShutdownOverrideSwitch ;
		pstate->CCVS_EngTestModeSwitch = pccvs->CCVS_EngTestModeSwitch ;
		pstate->CCVS_EngIdleDecrementSwitch = pccvs->CCVS_EngIdleDecrementSwitch ;
		pstate->CCVS_EngIdleIncrementSwitch = pccvs->CCVS_EngIdleIncrementSwitch ;
		pstate->CCVS_CruiseControlSetSpeed = pccvs->CCVS_CruiseControlSetSpeed;
		break;
        case DB_J1939_ETC1_VAR:
		petc1 = (j1939_etc1_typ *) pdata_val;
//		pstate->shift_in_progress = petc1->shift_in_progress;
		pstate->ETC1_TransmissionDrivelineEngaged = petc1->ETC1_TransmissionDrivelineEngaged ;
		pstate->ETC1_TorqueConverterLockupEngaged = petc1->ETC1_TorqueConverterLockupEngaged ;
		pstate->ETC1_TransmissionShiftInProcess = petc1->ETC1_TransmissionShiftInProcess ;
		pstate->ETC1_TransmissionOutputShaftSpeed = petc1->ETC1_TransmissionOutputShaftSpeed ;
		pstate->ETC1_PercentClutchSlip = petc1->ETC1_PercentClutchSlip ;
		pstate->ETC1_MomentaryEngineOverspeedEnable = petc1->ETC1_MomentaryEngineOverspeedEnable ;
		pstate->ETC1_ProgressiveShiftDisable = petc1->ETC1_ProgressiveShiftDisable ;
		pstate->ETC1_TransInputShaftSpeed = petc1->ETC1_TransInputShaftSpeed ;
		pstate->ETC1_SrcAddrssOfCtrllngDvcFrTrnsCtrl = petc1->ETC1_SrcAddrssOfCtrllngDvcFrTrnsCtrl ;
		break;
        case DB_J1939_ETC2_VAR:
		petc2 = (j1939_etc2_typ *) pdata_val;
		pstate->ETC2_TransmissionSelectedGear = petc2->ETC2_TransmissionSelectedGear ;
		pstate->ETC2_TransmissionActualGearRatio = petc2->ETC2_TransmissionActualGearRatio ;
		pstate->ETC2_TransmissionCurrentGear = petc2->ETC2_TransmissionCurrentGear ;
		pstate->ETC2_TransmissionRangeSelected = petc2->ETC2_TransmissionRangeSelected ;
		pstate->ETC2_TransmissionRangeAttained = petc2->ETC2_TransmissionRangeAttained ;
		break;
        case DB_J1939_VP_X_VAR:
		pvp_x = (j1939_vp_x_typ *) pdata_val;
		pstate->VP_X_TGW_Longitude_BB1_X_TGW = pvp_x->VP_X_TGW_Longitude_BB1_X_TGW;
		pstate->VP_X_TGW_Latitude_BB1_X_TGW = pvp_x->VP_X_TGW_Latitude_BB1_X_TGW;
		break;
        case DB_J1939_VDC2_VAR:
		pvdc2 = (j1939_vdc2_typ *) pdata_val;
		pstate->VDC2_SteeringWheelTurnCounter= pvdc2->VDC2_SteeringWheelTurnCounter;
		pstate->VDC2_SteeringWheelAngle= pvdc2->VDC2_SteeringWheelAngle;
		pstate->VDC2_YawRate= pvdc2->VDC2_YawRate;
		pstate->VDC2_LateralAcceleration= pvdc2->VDC2_LateralAcceleration;
		pstate->VDC2_LongitudinalAcceleration= pvdc2->VDC2_LongitudinalAcceleration;
		break;
        case DB_J1939_LFE_VAR:
		plfe = (j1939_lfe_typ *) pdata_val;
		pstate->LFE_EngineFuelRate = plfe->LFE_EngineFuelRate ;
		pstate->LFE_EngineInstantaneousFuelEconomy = plfe->LFE_EngineInstantaneousFuelEconomy ;
		pstate->LFE_EngineAverageFuelEconomy = plfe->LFE_EngineAverageFuelEconomy ;
		pstate->LFE_EngineThrottleValve1Position = plfe->LFE_EngineThrottleValve1Position ;
		break;
        case DB_J1939_FD_VAR:
		pfd = (j1939_fd_typ *) pdata_val;
		pstate->fan_drive_state = pfd->fan_drive_state;
		pstate->estimated_percent_fan_speed = pfd->estimated_percent_fan_speed;
		break;
        case DB_J1939_IEC_VAR:
		piec = (j1939_iec_typ *) pdata_val;
		pstate->boost_pressure = piec->boost_pressure;
		break;
        case DB_J1939_GFI2_VAR:
		pgfi2 = (j1939_gfi2_typ *) pdata_val;
//		pstate->fuel_flow_rate1 = pgfi2->fuel_flow_rate1;
//		pstate->fuel_flow_rate2 = pgfi2->fuel_flow_rate2;
//		pstate->fuel_valve1_position = pgfi2->fuel_valve1_position;
//		pstate->fuel_valve2_position = pgfi2->fuel_valve2_position;
		break;
        case DB_J1939_EI_VAR:
		pei = (j1939_ei_typ *) pdata_val; 
		pstate->exhaust_gas_pressure = pei->exhaust_gas_pressure;
		pstate->rack_position = pei->rack_position;
		pstate->natural_gas_mass_flow = pei->natural_gas_mass_flow;
		pstate->instantaneous_brake_power =
			 pei->instantaneous_estimated_brake_power;
		break;
        case DB_J1939_VOLVO_XBR_WARN_VAR:
        	pvolvo_xbr_warn = (j1939_volvo_xbr_warn_typ *) pdata_val;
		pstate->VOLVO_XBR_WARN_src_address = pvolvo_xbr_warn->src_address;
		pstate->VOLVO_XBR_WARN_byte1= pvolvo_xbr_warn->byte1;       ///
		pstate->VOLVO_XBR_WARN_byte2= pvolvo_xbr_warn->byte1;       ///
		pstate->VOLVO_XBR_WARN_byte3= pvolvo_xbr_warn->byte1;       ///
		pstate->VOLVO_XBR_WARN_byte4= pvolvo_xbr_warn->byte1;       ///
		pstate->VOLVO_XBR_WARN_byte5= pvolvo_xbr_warn->byte1;       ///
		pstate->VOLVO_XBR_WARN_byte6= pvolvo_xbr_warn->byte1;       ///
		pstate->VOLVO_XBR_WARN_byte7= pvolvo_xbr_warn->byte1;       ///
		pstate->VOLVO_XBR_WARN_byte8= pvolvo_xbr_warn->byte1;       ///
		break;
        case DB_J1939_VOLVO_XBR_VAR:
        	pvolvo_xbr = (j1939_volvo_xbr_typ *) pdata_val;
		pstate->VOLVO_XBR_ExternalAccelerationDemand = pvolvo_xbr->ExternalAccelerationDemand;       ///
		pstate->VOLVO_XBR_src_address = pvolvo_xbr->src_address;
		pstate->VOLVO_XBR_destination_address = pvolvo_xbr->destination_address;
		pstate->VOLVO_XBR_XBREBIMode = pvolvo_xbr->XBREBIMode;       ///
		pstate->VOLVO_XBR_XBRPriority = pvolvo_xbr->XBRPriority;      ///
		pstate->VOLVO_XBR_XBRControlMode = pvolvo_xbr->XBRControlMode;   ///
		pstate->VOLVO_XBR_XBRUrgency = pvolvo_xbr->XBRUrgency;       ///
		pstate->VOLVO_XBR_spare1 = pvolvo_xbr->spare1;           /// 0xFF
		pstate->VOLVO_XBR_spare2 = pvolvo_xbr->spare2;           /// 0xFF
		pstate->VOLVO_XBR_spare3 = pvolvo_xbr->spare3;           /// 0xFF
		pstate->VOLVO_XBR_XBRMessageCounter = pvolvo_xbr->XBRMessageCounter;///
		pstate->VOLVO_XBR_XBRMessageChecksum = pvolvo_xbr->XBRMessageChecksum;///
		break;
        case DB_J1939_ERC1_TRANS_VAR:
        	perc1 = (j1939_erc1_typ *) pdata_val;
		pstate->ERC1ERRetarderEnableShiftAssistSw = perc1->ERC1ERRetarderEnableShiftAssistSw ;
		pstate->ERC1ERRtdrEnablBrakeAssistSwitch = perc1->ERC1ERRtdrEnablBrakeAssistSwitch ;
		pstate->ERC1ERRetarderTorqueMode = perc1->ERC1ERRetarderTorqueMode ;
		pstate->ERC1ERActualEngineRetPercentTrq = perc1->ERC1ERActualEngineRetPercentTrq ;
		pstate->ERC1ERIntendedRtdrPercentTorque = perc1->ERC1ERIntendedRtdrPercentTorque ;
		pstate->ERC1ERRetarderRqingBrakeLight = perc1->ERC1ERRetarderRqingBrakeLight ;
		pstate->ERC1ERSrcAddrOfCtrlDvcFrRtdrCtrl = perc1->ERC1ERSrcAddrOfCtrlDvcFrRtdrCtrl ;
		pstate->ERC1ERDrvrsDmandRtdrPerctTorque = perc1->ERC1ERDrvrsDmandRtdrPerctTorque ;
		pstate->ERC1ERRetarderSelectionNonEng = perc1->ERC1ERRetarderSelectionNonEng ;
		pstate->ERC1ERActlMxAvlbRtdrtPerctTorque = perc1->ERC1ERActlMxAvlbRtdrtPerctTorque;
		break;
        case DB_EVT300_RADAR1_VAR:
                pevt300 = (evt300_radar_typ *) pdata_val;
                pstate->evt300 = *pevt300;
		break;
        case DB_DVI_OUT_VAR:
                pdvi_out = (dvi_out_t *) pdata_val;
                pstate->acc_cacc_request = pdvi_out->acc_cacc_request;
                pstate->gap_request = pdvi_out->gap_request;
		break;
        case DB_SELF_GPS_POINT_VAR:
                pself_gps = (path_gps_point_t *) pdata_val;
                pstate->self_gps = *pself_gps;
		break;
        case DB_LONG_LIDARA_VAR:
                plidarA = (long_lidarA_typ *) pdata_val;
                pstate->lidarA = *plidarA;
		break;
        case DB_LONG_LIDARB_VAR:
                plidarB = (long_lidarB_typ *) pdata_val;
                pstate->lidarB = *plidarB;
		break;
        case DB_MDL_LIDAR_VAR:
                pmdl_lidar = (mdl_lidar_typ *) pdata_val;
                pstate->mdl_lidar = *pmdl_lidar;
		break;
        case DB_COMM_LEAD_TRK_VAR:
                plead_trk = (veh_comm_packet_t *) pdata_val;
                pstate->lead_trk = *plead_trk;
		break;
        case DB_COMM_SECOND_TRK_VAR:
                psecond_trk = (veh_comm_packet_t *) pdata_val;
                pstate->second_trk = *psecond_trk;
		break;
        case DB_COMM_THIRD_TRK_VAR:
                pthird_trk = (veh_comm_packet_t *) pdata_val;
                pstate->third_trk = *pthird_trk;
		break;
        case DB_LONG_DIG_IN_VAR:
                pdig_in = (long_dig_in_typ *) pdata_val;
                pstate->dig_in = *pdig_in;
		break;
	default:
		fprintf(stderr, "Unknown db_num %d in long_update_fields...\n", db_num);
		break;
	}
}

/**
 * long_read_vehicle_state
 *
 * Used by run_tasks to update vehicle state from J1939 and other sensor
 * data available in the longitudinal input variable. 
 *
 * Can also be used by other trace programs, e.g. veh_trk -v
 */ 
int long_read_vehicle_state(db_clt_typ *pclt, long_ctrl *pctrl)
{
	long_vehicle_state *pstate = &pctrl->vehicle_state;
	long_params *pparams = &pctrl->params;
        db_data_typ db_data;
	int i;
	int clt_read_error = 0;

	// Read Jbus variables
	for (i = 0; i < pparams->dbv_list_size; i++) {
		int db_num = pparams->dbv_list[i];
		j1939_dbv_info *pinfo = get_jdbv_info(db_num);
		if (!pinfo->active)
			continue;
        	if (clt_read(pclt, db_num, db_num, &db_data))
			long_update_fields_from_dbv(db_num, pstate,
				pparams, &db_data.value.user);
		else
		    clt_read_error++;
	}

	// Read other input variables
	for (i = 0; i < trk_in_dbv_list_size; i++) {
		int db_num = trk_in_dbv_list[i].id;
        	if (clt_read(pclt, db_num, db_num, &db_data))
			long_update_fields_from_dbv(db_num, pstate,
				pparams, &db_data.value.user);
		else
		    clt_read_error++;
	}

	/* For now always returns true, later may want to return 0 if there
	 * are read errors.
	 */
        return 1;
}

int long_rdswitch(long_dig_in_typ dig_in)  {

	if( dig_in.man_autoswerr == TRUE )
		return -1;
	// Case if switch is in uncommitted state
	if( (dig_in.manualctl == 0) && (dig_in.autoctl == 0) )
		return 3;
	if(dig_in.manualctl == 1)
		return 1;
	if(dig_in.autoctl == 1)
		return 2;
	return -1;
}

int long_rdbrake(long_dig_in_typ dig_in)  {

	if( dig_in.brakeswerr == TRUE )
		return -1;
	if( dig_in.brakesw == 0)
		return 1;
	if( dig_in.brakesw == 1)
		return 0;
}

int long_setled( db_clt_typ *pclt, int faultcode) {

	static long_dig_out_typ dig_out;

	switch(faultcode) {

		case FLT_HI:
			dig_out.outchar = LED_RED;
			dig_out.amber_flash = 0;
			break;			
		case FLT_MED:
			dig_out.outchar = LED_GRN | LED_AMBER;
			dig_out.amber_flash = 1;
			break;			
		case FLT_LOW:
			dig_out.outchar = LED_GRN | LED_AMBER;
			dig_out.amber_flash = 0;
			break;			
		case FLT_AUTO:
			dig_out.outchar = LED_GRN;
			dig_out.amber_flash = 0;
			break;			
		case FLT_RDY2ROLL:
			dig_out.outchar = LED_GRN | LED_BLUE;
			dig_out.amber_flash = 0;
			break;			
		case LONG_CTL_ACTIVE:
			dig_out.outchar = LED_AMBER;
			dig_out.amber_flash = 0;
			break;			
		case LONG_CTL_INACTIVE:
			dig_out.outchar = 0;
			dig_out.amber_flash = 0;
			break;			
		case TOGGLE_WATCHDOG:
			// trk_io will keep track if this changes
			dig_out.xy_alive++;
			break;			
	}

	if( db_clt_write(pclt, DB_LONG_DIG_OUT_VAR, sizeof(long_dig_out_typ), 
		&dig_out)
		== FALSE )
		return -1;
	return 0;
}
