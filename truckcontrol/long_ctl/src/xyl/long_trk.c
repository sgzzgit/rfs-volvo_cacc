/**********************************************************************

       long_trk.c   for Radar test
          
       Longitudinal control for New Freightliner     
       Started on Feb. 25 2003   
       Based on Sue's code struture ( cmdtasks.c ): 3 basic functions  
                                     init_tasks()
                                     run_tasks()
                                     exit_tasks()
       Compiled the first time on Feb. 26, 03
       Compiled second time on    Mar. 26, 03
       Compled and run on         Mar. 27, 03
                Memory problem
       Add  DBG printf() points   Mar. 30, 03
       Successful run on          Apr. 10, 03 
       Add jbus_read_typ          Apr. 11, 03
       Changed control() entry    Apr. 11, 03
           Best run so far        Apr. 14, 03

       Best run so far. This version 
           can be used for Demo   Apr. 15, 03
       Version used to show to 
           DaimlerChryshler       Apr. 16, 03
       Radar and Lidar added on   Apr. 20, 03
       pcmd = poutput,  in previous longitudinal code
       All J-Bus reading from long_ctl.h
       Test bkst and engine speed Apr. 25 03
       Tested on Gold truck       May  14 03
       Test of radar at RFS       May  16 03
       N. B. maneuver_des_1(2)  and  maneuver_id  in communication are changed in use.
       Communication handshaking and synchronization is OK now.            Jul. 18 03
       Two truck run at RFS OK.                                            Jul. 24 03
       Air brake calibrated after ECM Box changed                          Aug. 01 03
       Brake control changed to modified EBS Box                           Sep. 16, 03
       Communication rewritten again based on Sept. 16 & 18                Sep. 19, 03
       New com driver installed. If loss handshaking for 10  
            conseccutive steps, then annunce comm_error.                   Sep. 22, 03
       For vehicle_1, use comm_receiev_pt_1, for vehicle_2, use both
            comm_receiev_pt_1 and comm_receive_2 are OK.                   Sep. 27, 03
       Blue and Gold trucks are unified by passed variable name.           Sep. 30, 03
       Rearranged div(), coording(), maneuver(), ref_dist()                Oct. 21, 03
       To test transition control                                          Oct. 24, 03
       Put GPS reading on                                                  Oct. 29, 03
       Succefully used for supporting to USC & UCR for 3~10[m] runs        Oct. 30, 03
       Intialize all communication signal before time_filter =0.0          Oct. 30, 03
       From Working/Blue to start with, entry for maneuver changed.        Dec. 01, 03
       For Fred Brwand: task = 0;
       For our own Demo: task = 1,2,3;
       New calibration of speed based on rented trailers and tire presure
          and Chris Cherry's road survey.                                  Dec. 02, 03
       Since some database and communication  are not vailable, it has
          many run-time errors; corrected;                                 Nov. 13, 08
       config_pt used before initilized => run-time error;                 Nov. 13, 08
       after commenting off most sensor data related parts;
          the code begin to activate engine speed;                         Nov. 13, 08                
       data saving changed; corrected;                                     Nov. 14, 08
       still no J-bus information read in yet;                             Nov. 14, 08
       Single vehciel working;                                             Dec. 16, 08
       Radar VAR Updaed; EVT test;                                         Dec. 16, 08
       Some communication pointer not initialized; Run-time error;         Dec. 16, 08
       Problem has been fixed but all radar reading are zeros;             Dec. 16, 08
       All the two vehicles uses comm_receive_pt2 which corresponds to
            proceeding vehicle  as the receiver buffer.                    Dec. 17, 08     
       Commmunication handshake changed;                                   Mar. 06, 09
       MDL lidar added                                                     Mar. 07, 09
       This is the version from Gold truck with new modification;          Mar. 09, 09
       Compiled and run on Gold trk test and mdl lidar test OK;            Mar. 09, 09
       Put back DENSO Lidar and use it as remote sensor; MDL standby;      May  16, 09
       Add Fault detection triggering LED;                                 Nov. 20, 09
       Add brake tranducer reading;                                        Nov. 20, 09
       Add starting mode logic with LED setting;                           Nov. 24, 09
       Sue added long_trk_func.h;                                          Nov. 25, 09
       Sue removed the database reading part;                              Nov. 25, 09
       John has moved all the the initial setup parameters to 
            realtime.ini and use config to read them in;                   Jan. 21, 10  
       Unified code based NVD test for three trucks;                       Feb. 13, 11   
       Compiled and run for one truck;                                     Mar. 23, 11   
       First test of scenarios for two trucks;							   Apr. 15, 11                                                
       started to build for truck CACC										04_07_15
	   data base JBus reading struct changed in long_ctl.h					04_15_15
            
            XY_LU


*********************************************************************/

#include <sys_os.h>
#include <timestamp.h>
#include "long_ctl.h"
#include "long_trk.h"
#include "db_utils.h"
#include "clt_vars.h"     // Definition of clt reading types
#include "coording.h"
#include "veh_long.h"
#include "timestamp.h"
#include "path_gps_lib.h"
#include "long_comm.h"
#include "veh_trk.h"
#include "long_trk_func.h"
#include <time.h>

#define OPEN_LOOP_TQ  20.0


//#define COMM_DATA
//#define TASK_CONTROL
//#define TEST_BRK

static float track_length=260.0;
static float stop_period=0.0;
static float stop_dist=250.0;              
static long data_log_count=0;

extern bool_typ verbose_flag;           // Needs changing back

db_clt_typ *pclt=NULL;                   // Database client pointer
//db_data_typ db_data_vrd;
db_data_typ db_data_lat;
db_data_typ db_data_sens;
db_data_typ db_data_mark;
db_data_typ db_data_input;
evt300_radar_typ *pvor_radar;
//db_data_typ db_data_lidarMA;
//db_data_typ db_data_lidarMB;
//long_lidarA_typ *plidar_A;
//long_lidarB_typ *plidar_B;
//mdl_lidar_typ *pmdl_lidar;

// For communication
db_data_typ db_data_comm_1;
db_data_typ db_data_comm_2;
db_data_typ db_data_comm_3;
db_data_typ db_data_comm_send;
veh_comm_packet_t comm_receive_pt[MAX_TRUCK];   // including GPS from other vehicles; find def of MAX_TRUCK
veh_comm_packet_t comm_send_pt;

// GPS and road grade
static local_gps_typ gps_trk;
static local_gps_typ *gps_trk_pt;
static road_info_typ  road_info;
static road_info_typ* road_info_pt;
static local_pos_typ str_pos;
static local_pos_typ *str_pos_pt;



FILE* pout;                              // 04_04_03

// For veh_long.h
static fault_index_typ f_index;
static fault_index_typ* f_index_pt; 
static control_config_typ config;      
static control_config_typ* config_pt;
static control_state_typ con_state;
static control_state_typ* con_state_pt;
static con_output_typ con_output;
static con_output_typ* con_output_pt;
static switch_typ sw_read;
static switch_typ* sw_read_pt;
static jbus_read_typ jbus_read;
static jbus_read_typ* jbus_read_pt;
static sens_read_typ sens_read;
static sens_read_typ* sens_read_pt;
static pltn_info_typ pltn_info;
static pltn_info_typ* pltn_info_pt;

// For coording.h
static vehicle_info_typ vehicle_info;
static vehicle_info_typ* vehicle_info_pt;
static f_mode_comm_typ f_mode_comm;
static f_mode_comm_typ* f_mode_comm_pt;
static comm_info_typ comm_info;
static comm_info_typ* comm_info_pt;
static manager_cmd_typ manager_cmd;
static manager_cmd_typ* manager_cmd_pt;
static path_gps_point_t self_gps_point;    // read from self GPS database variable
//static path_gps_point_t gps_point_2;       // take from veh_comm_packet_t; accroding to position sequence
//static path_gps_point_t gps_point_3;       // take from veh_comm_packet_t

static path_gps_point_t gps_point[3];

//static evrd_out_typ evrd_out;
//static evrd_out_typ* evrd_out_pt;
//static evrd_in_typ evrd_in;
//static evrd_in_typ* evrd_in_pt;
//static ldr_out_typ ldr_out;
//static ldr_out_typ* ldr_out_pt;
//static mdl_out_typ ldr_mdl;
//static mdl_out_typ* ldr_mdl_pt;

long_params *pparams;
long_vehicle_state *pv;                  // ddefined in long_ctl; most JBus data name changed; 04_15_15
cbuff_typ *pbuff1;
buffer_item *pdata;
long_private *ppriv;
private_params *pcparams;
char *ini_file;

float c[N_pt-1]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
float d[N_pt-1]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
float v_p[N_pt]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};




static long_private cmd_private;    // Sue


/*********************************************

The following struct is used in exit_tasks()
and transitions

*********************************************/

static long_output_typ inactive_ctrl =
{
        600.0,                   /* engine speed, truck idle, don't care since disabled */
        MIN_TORQUE/ENGINE_REF_TORQUE,              /* engine torque, truck idle, don't care since disabled */
        0.0,                     /* retarder torque, don't care since disabled */
        TSC_OVERRIDE_DISABLED,   /* engine command mode */
        TSC_OVERRIDE_DISABLED,   /* engine retarder command mode */
        0.0,                     /* accelerator pedal voltage -- not used by jbussend */
        0.0,                     /* ebs deceleration */
        XBR_NOT_ACTIVE,         /* brake command mode */
        0.0,                     /* percent of maximum transmission retarder activation */
        TSC_OVERRIDE_DISABLED,   /* transmission retarder command mode */
};


              /**************************************************************
              *                                                             *
              *             init_tasks()                                    *
              * Initialization at the beginning of a run of the controller  *
              *                                                             *
              **************************************************************/
 
 

int init_tasks(db_clt_typ *pclt, long_ctrl *pctrl, long_output_typ *pcmd)
{
        /*** XY_LU's code ***/
             
    char hostname[MAXHOSTNAMELEN+1];
    FILE *pfin1; 
    
    void ref_ini(control_config_typ*, float *, float *, float *);

    ini_file = pparams->long_cfg_file;        
 
       /*** Sue's Code ***/
       pparams = &pctrl->params;
       pbuff1 = &pctrl->buff;

       pcparams = &cmd_private.cmd_params;

       pout=fopen("/big/data/test.dat","w");                  // 04_16_03, working now
          if (pout == NULL)
          {
             printf("Open output file for writing fails!");
             fflush(stdout);
          }
 
#ifdef __QNX4__
    sprintf( hostname, "%lu", getnid() );
#else
        get_local_name(hostname, MAXHOSTNAMELEN);
#endif

        /**********************************/
        /*                                */
        /*     Read from realtime.ini     */
        /*                                */
        /**********************************/
        
        /* Read from long_ctl specific ini file */
        ini_file = pparams->avcs_cfg_file;          // Used twice, in long_utils()
  
        /* Read from CommandTestType section */
        if ((pfin1 = get_ini_section(ini_file, "long_trk")) == NULL)
            {
                printf("LONG_TRK.C:Cannot get ini file %s, section %s\n",
                   ini_file, "long_trk"); 
                fflush(stdout);
                return 0;
            }


    config.mass_sw=(unsigned short)get_ini_long(pfin1, "Mass_SW", 0L );
    config.task=(unsigned short)get_ini_long( pfin1,"Task", 0L );
    config.run=(unsigned short)get_ini_long( pfin1,"Run", 1L );
    config.dir=(unsigned short)get_ini_long( pfin1,"Dir", 1L );    // 1 EB; 2 WB
     
    config.max_spd=((float)get_ini_double(pfin1, "MaxSpeed", 1.0L ));
    config.max_acc=(float)get_ini_double(pfin1, "max_acc", 1.0L ); // Initialization
    config.max_dcc=(float)get_ini_double(pfin1, "max_dcc", 1.0L );
    config.para1=(float)get_ini_double(pfin1, "k_1", 1.0L );
    config.para2=(float)get_ini_double(pfin1, "k_2", 1.0L );
    config.para3=(float)get_ini_double(pfin1, "k_3", 1.0L );
    config.para4=(float)get_ini_double(pfin1, "k_4", 1.0L );
    
    config.data_log = (int)get_ini_long( pfin1, "DataLog", 1L );
    config.static_run = get_ini_bool( pfin1, "StaticRun", FALSE );
    config.test_actuators = get_ini_bool( pfin1, "TestActuators", FALSE );
    config.truck_platoon = get_ini_bool( pfin1, "TruckPlatoon", TRUE );
	config.truck_platoon = FALSE;
	config.pltn_size = get_ini_long( pfin1, "PlatoonSize", 2L );	
	config.truck_CACC = get_ini_bool( pfin1, "CACC", FALSE );  
	config.truck_ACC = get_ini_bool( pfin1, "ACC", FALSE );  

	config. ACC_tGap = (float)get_ini_double( pfin1, "ACC_Time_Gap", 2.0L );
	config. CACC_tGap = (float)get_ini_double( pfin1, "CACC_Time_Gap", 1.1L );

    config.use_comm = get_ini_bool( pfin1, "UseComm", TRUE );
    config.eaton_radar = get_ini_bool( pfin1, "EatonRadar", TRUE );
    config.save_data = get_ini_bool( pfin1, "SaveData", TRUE );
    config.run_data = get_ini_bool( pfin1, "RunData", TRUE );
    config.read_data = get_ini_bool( pfin1, "ReadData", TRUE );
    config.trans_sw = get_ini_bool( pfin1, "TransSW", TRUE );
    config.use_gps = get_ini_bool( pfin1, "UseGPS", TRUE );
    config.use_mag = get_ini_bool( pfin1, "UseMagnets", TRUE );
    config.handle_faults= get_ini_bool( pfin1, "HandleFaults", TRUE);

    config.end_scenario = get_ini_bool( pfin1, "EndScenario", TRUE );
    config.step_start_time = (float)get_ini_double( pfin1, "StepStartTime", 2.5 );
    config.step_end_time = (float)get_ini_double( pfin1, "StepEndTime", 5.0 );
    config.tq_cmd_coeff = (float)get_ini_double( pfin1, "TqCmdCoff", 1.0 );
    config.spd_cmd_coeff = (float)get_ini_double( pfin1, "SpdCmdCoeff", 1.0 );
    config.jk_cmd_coeff = (float)get_ini_double( pfin1, "JKCmdCoeff", 1.0 );
    config.trtd_cmd_coeff = (float)get_ini_double( pfin1, "TrtdCmdCoeff", 1.0 );   
	config.MyPltnPos = get_ini_long( pfin1, "MyPltnPos", 2L );	
	config.FollowingMode = get_ini_long( pfin1, "FollowingMode", 1L );	

    fclose(pfin1);
    
    data_log_count=0;               // initialization 

     /***********************************************************/
     /*        Initializing test track related parameters       */
     /***********************************************************/
    
   // From veh_long.h
   
     memset(&comm_send_pt, 0, sizeof(veh_comm_packet_t));
 
     memset(&f_index, 0, sizeof(f_index));

     sw_read. fan_sw=1;
     sw_read. comp_sw=1;
     sw_read. cond_sw=1;
     sw_read. steer_sw=1;
     sw_read. alt_sw=1;
     sw_read. actuator_sw=1;    
     sw_read. radar1_sw=1;
     sw_read. radar2_sw=1;
     //sw_read. auto_sw=0;        
     //sw_read. manu_sw=1;
     sw_read. HMI_sw=1;                    

     
     memset(&con_state, 0, sizeof(con_state));
     //con_state. comm_coord=OFF;
     //con_state. comm_leader=OFF;
     //con_state. comm_pre=OFF;
     //con_state. comm_back=OFF;
     con_state. des_f_dist=DES_FOLLOW_DIST; // Necesary to update for maneuvers // But changed according to  task number for support of USC & UCR
     //con_state. des_f_dist=10.0;
	   
     manager_cmd. drive_mode=0;           
     con_state. max_brake=MAX_BRAKE;        
     con_state. min_brake=MIN_BRAKE;        
     con_state. ini_brake=INI_BRAKE;        
     con_state. stop_brake=STOP_BRAKE;
          
     
     memset(&road_info, 0, sizeof(road_info));      
     memset(&con_output, 0, sizeof(con_output));           
     memset(&jbus_read, 0, sizeof(jbus_read));
     sw_read. gshift_sw=OFF;                      
     sw_read. lockup=OFF;                          
     //sw_read. driveline_engaged=OFF;   // no such reading for Volvo truck 
	 memset(&sens_read, 0, sizeof(sens_read));
     //memset(&evrd_in, 0, sizeof(evrd_in));
     //memset(&evrd_out, 0, sizeof(evrd_out));
     //memset(&ldr_out, 0, sizeof(ldr_out));
     //memset(&ldr_mdl, 0, sizeof(ldr_mdl));             
     memset(&vehicle_info, 0, sizeof(vehicle_info));     
	 memset(&pltn_info, 0, sizeof(pltn_info));
     memset(&f_mode_comm, 0, sizeof(f_mode_comm));
     f_mode_comm. pltn_id=1;                        
     memset(&comm_info, 0, sizeof(comm_info));                                    
     memset(&manager_cmd, 0, sizeof(manager_cmd));        
     manager_cmd. stop_dist=200.0;          // in [s]
     manager_cmd. set_v=SET_SPEED*mph2mps;  // converted on [m/s];
	// manager_cmd. t_gap=T_GAP;				// in [s]

     // GPS & road grade
    // memset(&EB_start,0,sizeof(EB_start));
    // memset(&WB_start,0,sizeof(WB_start));
     memset(&gps_trk,0,sizeof(gps_trk));
     memset(&self_gps_point,0,sizeof(self_gps_point));
     //memset(&gps_point_2,0,sizeof(gps_point_2));
     //memset(&gps_point_3,0,sizeof(gps_point_3));

	 memset(gps_point,0,3 * sizeof(path_gps_point_t));
	 memset(&str_pos,0,sizeof(str_pos));
	 str_pos_pt=&str_pos;

	
     vehicle_info. fault_mode = 1;   // default value
     vehicle_info_pt = &vehicle_info;

	 pltn_info.handshake=OFF;
	 pltn_info_pt = &pltn_info;
     f_mode_comm_pt = &f_mode_comm;	
     comm_info_pt = &comm_info;
     manager_cmd_pt = &manager_cmd; 
     con_state_pt = &con_state;
     f_index_pt = &f_index;
     config_pt = &config;
     sw_read_pt = &sw_read;        
     con_output_pt = &con_output;
     jbus_read_pt = &jbus_read;
	 sens_read_pt = &sens_read;
	 

    // evrd_in_pt = &evrd_in;
    // evrd_out_pt = &evrd_out;
    // ldr_out_pt = &ldr_out;
    // ldr_mdl_pt = &ldr_mdl;
    // EB_start_pt=&EB_start;
    // WB_start_pt=&WB_start;


    /*********************************************

	   Assign Tasks
	**********************************************/

    gps_trk_pt=&gps_trk;
    road_info_pt=&road_info;    
    config.max_dcc=MAX_DCC;   
    
  
       
       
        pcmd->engine_command_mode = TSC_OVERRIDE_DISABLED;
        pcmd->brake_command_mode = XBR_NOT_ACTIVE;
        pcmd->engine_retarder_command_mode = TSC_OVERRIDE_DISABLED;
        //pcmd->trans_retarder_command_mode = TSC_OVERRIDE_DISABLED;

        pcparams->gather_data = 30000;
    
       
        /* Initialize data buffer */
        init_circular_buffer(pbuff1, pcparams->gather_data, sizeof(buffer_item));  // Using pbuff1
        ftime(&pctrl->start_time);
        
		vehicle_info_pt-> veh_id = config.MyPltnPos;		
		manager_cmd_pt-> following_mode =1;


        /*** Protection for Safety ***/ 

      
        ref_ini(config_pt, v_p, c, d);  // To use config_pt, moved back.
        
        if (long_setled(pclt, LONG_CTL_ACTIVE ) != 0)                          
            fprintf(stderr, " Setting LONG_CTL_ACTIVE fail! \n");
            
        return 1;
}





/************************************************************************
 * run_tasks
 * Whatever you want to do: in this case, run simple control to
 * test J1939 send commands.
 ***********************************************************************/


int run_tasks(db_clt_typ *pclt, long_ctrl *pctrl, long_output_typ *pcmd)
{

	static float mytimer = 0;
	static char do_once = 0;
    //const int radar_sw = 8;
/*--- All the time related variables ---*/

    static float global_time=0.0, local_time=0.0; 
    struct tm *timestamp;
    double seconds;
    static float t_ctrl=0.0, dt=0.0, time_filter=0.0;    
    //static unsigned short /*handshake_start = OFF,*/ //prt_buff=0, prt1_sw =ON;
    
    /*--- Sensor Readings From Data Base---*/

    float v1=0.0, v2=0.0, v3=0.0;
    static float v=0.0; // max_spd_buff=25.0;       //v0=0.0;
	static int max_spd_ini=1, jk_ini=0, brk_ini=0;
    static float tg=0.0;
 
    /*--- Maneuver Coordination ---*/

    static int maneuver_id[2]={0,0};
    static int pre_maneuver_id[2]={0,0};
   

    static float run_dist=0.0;   
	static double tmp_frac=0.0;
	int tmp_int;
	static int f_torq_buff=0, f_radar_buff=0, f_lidar_buff=0, f_jake_buff=0, f_brk_buff=0, f_comm_buff=0;
	static int cmd_count=0;

    int ctrl_interval;
	
        static struct timespec prev, curr;
        double difference;
        static int time_sw = 1;

	float min_main(float, float);
	int max_int(int, int);

     // At initialization  for timing                                        // For DBG on Aug 6 03
        if (time_sw == 1)
           {
              clock_gettime( CLOCK_REALTIME, &prev); 
              time_sw = 0;            
           }


        
        pv = &pctrl->vehicle_state;
		pparams = &pctrl->params;
		pbuff1 = &pctrl->buff;
		pdata = (buffer_item *) pbuff1->data_array;		
		ppriv = (long_private *) pctrl->plong_private;  	
		pcparams = &ppriv->cmd_params;
        ctrl_interval = pparams->ctrl_interval;   
		pltn_info_pt->pltn_size=config_pt-> pltn_size;

		if (pltn_info_pt->pltn_size==1)
			vehicle_info_pt-> ready = 1;


		/********************************/
       /*----  From Communication  ----*/
       /********************************/

if(config.use_comm == TRUE) 
{
    if (pltn_info_pt-> pltn_size == 2)
    {
	comm_receive_pt[1] = pv->lead_trk;
	comm_receive_pt[2] = pv->second_trk;
    }
    else if (pltn_info_pt-> pltn_size == 3)        // need to be modified
    {       
	comm_receive_pt[1] = pv->lead_trk;
	comm_receive_pt[2] = pv->second_trk;
	comm_receive_pt[3] = pv->third_trk;
    }
}

	  

      /****************************************************
	                   Define Variable Max Spd
	                      
	    ****************************************************/ 

#ifdef TASK_CONTROL
     if( test_site == RFS )
     {
		 con_state. des_f_dist=10.0;		 
		 
		 if (config_pt-> task == 1)    
		 {
		     //vehicle_info_pt-> veh_id=1;
			
			if (config_pt-> run == 1)
			{
				track_length = 250.0;				
			}
				
			if (config_pt-> run == 2)
			{
				track_length = 19550.0;				
			}			
		    con_state. des_f_dist=DES_FOLLOW_DIST;				              	 
		 }
		 if ( (config_pt-> task == 2) || (config_pt-> task == 3) )   // ACC
		 {					 
			 track_length = 19990.0;
			 config_pt->truck_ACC=TRUE;
			 config_pt-> truck_CACC=FALSE;			 
			 config_pt->  ACC_tGap=1.1;
		 }		
		 if (config_pt-> task == 4)  // CACC 
		 {					 
			 track_length = 20000.0;
			 config_pt->truck_CACC=TRUE;
			 config_pt->truck_ACC=FALSE;			
			 config_pt-> CACC_tGap=0.9;	
		 }
		 if (config_pt-> task == 5)  // HIA
		 {					 
			 track_length = 20000.0;
			 config_pt->truck_CACC=TRUE;
			 config_pt->truck_ACC=FALSE;			 
			 config_pt-> CACC_tGap=1.0;				
		 } 	
    } // RFS end
#endif

    if (vehicle_info_pt-> veh_id == 1)
	{
		 config_pt->truck_ACC=TRUE;
		 config_pt-> truck_CACC=FALSE;	
		 config_pt-> ACC_tGap=1.6;
	}
	else
	{
		 config_pt->truck_CACC=TRUE;
		 config_pt->truck_ACC=FALSE;	
		 config_pt-> CACC_tGap=1.1;	
	}

	config_pt-> max_spd=55.0*mph2mps;
	manager_cmd_pt-> set_v=55.0*mph2mps;

	if (config_pt->max_spd > 55.0*mph2mps)      
		config_pt->max_spd=55.0*mph2mps;  
	if (manager_cmd_pt-> set_v > 55.0*mph2mps)      
		manager_cmd_pt-> set_v=55.0*mph2mps;  


	con_state_pt-> max_spd=manager_cmd_pt-> set_v;

	if (max_spd_ini==1)
	{		
		if (config_pt->truck_ACC == TRUE)			
			con_state. des_f_dist=(con_state_pt-> spd)*(config_pt-> ACC_tGap);
		if (config_pt->truck_CACC == TRUE)			
			con_state. des_f_dist=(con_state_pt-> spd)*(config_pt-> CACC_tGap);
		max_spd_ini=0;
		if (con_state. des_f_dist < DES_FOLLOW_DIST)
			con_state. des_f_dist=DES_FOLLOW_DIST;
	}

    track_length=900000.0;
	
	
	stop_period=2.0*((config.max_spd)*mph2mps) / (config.max_dcc);                                          
    stop_dist = track_length - ((config.max_spd)*stop_period - 0.25*(config.max_dcc)*stop_period*stop_period);  

    con_state_pt->ACC_tGap=config_pt-> ACC_tGap;
    con_state_pt->CACC_tGap=config_pt-> CACC_tGap;

       



   /***********************************************
                GPS and Volvo Distanvce Sensors
   ************************************************/
   
	//memcpy(pv->self_gps, &self_gps_point, sizeof(path_gps_point_t);
	gps_point[vehicle_info_pt-> veh_id-1] = pv->self_gps;  /// read from self GPS unit

	if (vehicle_info_pt-> veh_id == 1)
	{
		if ( fabs(comm_receive_pt[2].longitude) > 0.1 )
		{
			gps_point[1].longitude = comm_receive_pt[2].longitude;
			gps_point[1].latitude = comm_receive_pt[2].latitude;
			gps_point[1].heading = comm_receive_pt[2].heading;
		}
		else
		{
			gps_point[1].longitude = gps_point[0].longitude;
			gps_point[1].latitude = gps_point[0].latitude;
			gps_point[1].heading = gps_point[0].heading;
		}
		if ( fabs(comm_receive_pt[3].longitude) > 0.1 )
		{
			gps_point[2].longitude = comm_receive_pt[3].longitude;
			gps_point[2].latitude = comm_receive_pt[3].latitude;
			gps_point[2].heading = comm_receive_pt[3].heading;	
		}
		else
		{
			gps_point[2].longitude = gps_point[0].longitude;
			gps_point[2].latitude = gps_point[0].latitude;
			gps_point[2].heading = gps_point[0].heading;
		}		
	}
	if (vehicle_info_pt-> veh_id == 2)
	{	
		if ( fabs(comm_receive_pt[1].longitude) > 0.1 )
		{
			gps_point[0].longitude= comm_receive_pt[1].longitude;
			gps_point[0].latitude = comm_receive_pt[1].latitude;
			gps_point[0].heading = comm_receive_pt[1].heading;
		}
		else
		{
			gps_point[0].longitude = gps_point[1].longitude;
			gps_point[0].latitude = gps_point[1].latitude;
			gps_point[0].heading = gps_point[1].heading;
		}
		if ( fabs(comm_receive_pt[3].longitude) > 0.1 )
		{
			gps_point[2].longitude = comm_receive_pt[3].longitude;
			gps_point[2].latitude = comm_receive_pt[3].latitude;
			gps_point[2].heading = comm_receive_pt[3].heading;
		}
		else
		{
			gps_point[2].longitude = gps_point[1].longitude;
			gps_point[2].latitude = gps_point[1].latitude;
			gps_point[2].heading = gps_point[1].heading;
		}		
	}
	if (vehicle_info_pt-> veh_id == 3)
	{
		
		if ( fabs(comm_receive_pt[1].longitude) > 0.1 )
		{
			gps_point[0].longitude= comm_receive_pt[1].longitude;
			gps_point[0].latitude = comm_receive_pt[1].latitude;
			gps_point[0].heading = comm_receive_pt[1].heading;
		}
		else
		{
			gps_point[0].longitude = gps_point[2].longitude;
			gps_point[0].latitude = gps_point[2].latitude;
			gps_point[0].heading = gps_point[2].heading;
		}
		if ( fabs(comm_receive_pt[2].longitude) > 0.1 )
		{
			gps_point[1].longitude = comm_receive_pt[2].longitude;
			gps_point[1].latitude = comm_receive_pt[2].latitude;
			gps_point[1].heading = comm_receive_pt[2].heading;
		}
		else
		{
			gps_point[1].longitude = gps_point[2].longitude;
			gps_point[1].latitude = gps_point[2].latitude;
			gps_point[1].heading = gps_point[2].heading;
		}		
	}
  
	
	//	veh_pos(vehicle_info_pt, gps_point[0], gps_point[1], gps_point[2], str_pos_pt);
	

	/**************************************
	                      Timing               
	**************************************/

    dt = ctrl_interval * 0.001; // in seconds 
    dt = 0.02;
    local_time += dt;  
	time_filter=local_time;
	t_ctrl=local_time;


	/***************************************************/
	/*                                                 */
	/*        Setup Communication Link and             */
	/*        get information from front and back      */
	/*        vehicles on non-fault basis              */
	/*                                                 */
	/***************************************************/

if(config.use_comm == TRUE) 
	cacc_comm( local_time, &global_time, dt, con_state_pt, vehicle_info_pt, comm_info_pt, 
	    f_index_pt, comm_receive_pt, &comm_send_pt, pltn_info_pt);

if ((comm_receive_pt[vehicle_info_pt-> veh_id - 1]. maneuver_des_2 == 1) || (comm_receive_pt[vehicle_info_pt-> veh_id - 1]. maneuver_des_2 == 2))
	manager_cmd_pt-> following_mode =2;
else
	manager_cmd_pt-> following_mode =1;
	   /**********************************/
       /*----  String Configuration  ----*/
       /**********************************/

		

		/*
		if (vehicle_info_pt-> comm_p[0] == 1)
		{
			if (vehicle_info_pt-> veh_id == 2)
				vehicle_info_pt-> veh_id = 1;
			if (vehicle_info_pt-> veh_id == 3)
			{
				if (vehicle_info_pt-> comm_p[(vehicle_info_pt-> veh_id)-1] == 0)
				{
					if ((sens_read_pt->target_avail == 1) && (sens_read_pt->target_d < COUPLE_COEFF*(con_state_pt-> des_f_dist)) )
						vehicle_info_pt-> veh_id = 2;
					else
						vehicle_info_pt-> veh_id = 1;
				}
				else
					vehicle_info_pt-> veh_id = 1;
			}
		}
		*/

          /**************************************/
          /*  Read in sensor measurement        */ 
          /*  from database to local variables  */ 
          /*                                    */
          /**************************************/

read_jbus(dt, time_filter, pv, pparams, jbus_read_pt, sens_read_pt, sw_read_pt, vehicle_info_pt);

read_sw(pv, sw_read_pt);

        
	/***************************************/
	/*                                                       */
	/*	Signal Processing                   */
	/*                                                       */
	/***************************************/

                                                               

	/*******************************************/
	/*                                         */
	/*    Update control state data base       */
	/*                                         */
	/*******************************************/  
     
     con_state_pt-> spd=jbus_read_pt-> v;	 
     con_state_pt-> acc=jbus_read_pt-> long_accel;	 
     con_state_pt-> fuel_rt= jbus_read_pt-> fuel_m;              
     con_state_pt-> auto_acc=jbus_read_pt-> long_accel;	
     con_state_pt-> auto_speed=jbus_read_pt-> v; 
     con_state_pt-> auto_throt=jbus_read_pt-> fuel_m;
     con_state_pt-> auto_spd_cmd=0.0;
     con_state_pt-> auto_tq_cmd=jbus_read_pt->actual_eng_tq;
     con_state_pt-> auto_brk=jbus_read_pt-> bp;
     con_state_pt-> auto_rtd=0.0;
     con_state_pt-> auto_jake2=0.0;
     con_state_pt-> auto_jake4=0.0;
     con_state_pt-> auto_jake6=0.0; 


     //veh_pos(config_pt, comm_info_pt, vehicle_info_pt, pltn_info_pt);

	manager_cmd. set_v=jbus_read_pt-> CC_set_v;
	manager_cmd. set_v=SET_SPEED;
	

	v=con_state_pt-> spd;
	v_flt( dt, time_filter, v1, v2, v3, &v, f_index_pt);


     /*---- Filtering Reference Signal ---*/
    

    if( con_state_pt-> pre_v <= 0.0 )
        con_state_pt-> pre_v = 0.0;


	if(sw_read_pt-> gshift_sw == OFF) 
        tg = 0.0;       
    else
        tg += dt;
    con_state_pt-> tg = tg;
 
	// Run dist est
    if(config_pt->static_run == TRUE)
        run_dist += con_state_pt-> lead_v * dt;
     else
        run_dist += (con_state_pt-> spd) * dt;
	vehicle_info_pt-> run_dist = run_dist;

	if (    ((vehicle_info_pt-> veh_id != 0)  && (config_pt->truck_platoon == FALSE)) || ((vehicle_info_pt-> veh_id != 0)  && (config_pt->truck_platoon == TRUE) &&
            ( vehicle_info_pt-> veh_id != 1))    ) 
     {
	     // Add Volvo radar dist here
          est_dist(dt, sens_read_pt, con_state_pt, maneuver_id, pre_maneuver_id, f_index_pt, manager_cmd_pt);  // SW added on 07_25_03                                

     }             

	/*******************************************/
	/*                                         */
	/*       Fault Detect and Management       */
	/*       Also Setting LED                  */
	/*                                         */
	/*******************************************/
	
//if( (config.handle_faults == TRUE) && (manager_cmd_pt-> man_des > 1) && (maneuver_id[0] != 30))
if( (config.handle_faults == TRUE) && (manager_cmd_pt-> drive_mode > 1))
{    
  
			  if (f_index_pt-> torq == 1 && f_torq_buff == 0)			  
				 vehicle_info_pt-> fault_mode=(vehicle_info_pt-> fault_mode)*3;
			  if (f_index_pt-> torq == 0 && f_torq_buff == 1)	
			  {
				if (vehicle_info_pt-> fault_mode >= 3)
				{
				  tmp_frac=modf((double)((vehicle_info_pt-> fault_mode)/3), &tmp_int);
				  if (tmp_frac < 0.001)
					vehicle_info_pt-> fault_mode=max_int(tmp_int,1);
				}
			  }	
			  f_torq_buff=f_index_pt-> torq;

			  if (f_index_pt-> radar == 1 && f_radar_buff == 0)
				   vehicle_info_pt-> fault_mode=(vehicle_info_pt-> fault_mode)*7;
			  if (f_index_pt-> radar == 0 && f_radar_buff == 1)
			  {
				if (vehicle_info_pt-> fault_mode >= 7)
				{
				  tmp_frac=modf((double)((vehicle_info_pt-> fault_mode)/7), &tmp_int);
				  if (tmp_frac < 0.001)
					vehicle_info_pt-> fault_mode=max_int(tmp_int,1);
				}
			  }
			  f_radar_buff=f_index_pt-> radar;

			  if(f_index_pt-> lidar == 1 && f_lidar_buff == 0)
				   vehicle_info_pt-> fault_mode=(vehicle_info_pt-> fault_mode)*11;
			  if(f_index_pt-> lidar == 0 && f_lidar_buff == 1)
			  {
				if (vehicle_info_pt-> fault_mode >= 11)
				{
				  tmp_frac=modf((double)((vehicle_info_pt-> fault_mode)/11), &tmp_int);
				  if (tmp_frac < 0.001)
					vehicle_info_pt-> fault_mode=max_int(tmp_int,1);
				}
			  }
			  f_lidar_buff=f_index_pt-> lidar;

			  if(f_index_pt-> jake == 1 && f_jake_buff == 0)
				   vehicle_info_pt-> fault_mode=(vehicle_info_pt-> fault_mode)*13;
			  if(f_index_pt-> jake == 0 && f_jake_buff == 1)
			  {
				if (vehicle_info_pt-> fault_mode >= 13)
				{
				  tmp_frac=modf((double)((vehicle_info_pt-> fault_mode)/13), &tmp_int);
				  if (tmp_frac < 0.001)
					vehicle_info_pt-> fault_mode=max_int(tmp_int,1);
				}
			  }
			  f_jake_buff = f_index_pt-> jake;

			  if(f_index_pt-> brk == 1 && f_brk_buff == 0)
				   vehicle_info_pt-> fault_mode=(vehicle_info_pt-> fault_mode)*17;
			   if(f_index_pt-> brk == 0 && f_brk_buff == 1)
			  {
				if (vehicle_info_pt-> fault_mode >= 17)
				{
				  tmp_frac=modf((double)((vehicle_info_pt-> fault_mode)/17), &tmp_int);
				  if (tmp_frac < 0.001)
					vehicle_info_pt-> fault_mode=max_int(tmp_int,1);
				}
			  }
			  f_brk_buff = f_index_pt-> brk;


		  if (vehicle_info_pt-> veh_id == 1)
	       {    
			  if(f_index_pt-> comm == 1 && f_comm_buff == 0)
				   vehicle_info_pt-> fault_mode=(vehicle_info_pt-> fault_mode)*5;
			  if(f_index_pt-> comm == 0 && f_comm_buff == 1)
			  {
				if (vehicle_info_pt-> fault_mode >= 5)
				{
				  tmp_frac=modf((double)((vehicle_info_pt-> fault_mode)/5), &tmp_int);
				  if (tmp_frac < 0.001)
					vehicle_info_pt-> fault_mode=max_int(tmp_int,1);
				}	
			  }
			  f_comm_buff=f_index_pt-> comm;
	       }
		  else
		  {
		  }
         
           
    //manager_cmd_pt-> f_manage_index = max_i(vehicle_info_pt-> fault_mode, pltn_info_pt-> pltn_fault_mode);    	// temporarily removed on     
	 manager_cmd_pt-> f_manage_index = 0;
               
     if (manager_cmd_pt-> f_manage_index == 1)        
        {
	        if (long_setled(pclt, FLT_LOW) != 0)
               fprintf(stderr, " Setting FLT_LOW fail! \n");             
        }      
     if (manager_cmd_pt-> f_manage_index == 2)
        {  
	        if (long_setled(pclt, FLT_MED) != 0)
                fprintf(stderr, " Setting FTL_MED fail! \n");   
        }
     if (manager_cmd_pt-> f_manage_index  == 3)
        {	        
	        if (long_setled(pclt, FLT_HI) != 0)
               fprintf(stderr, " Setting FLT_HI fail! \n");                   
        }	  
}      
     
	/*******************************************/
	/*                                         */
	/*    Update maneuver state data base      */
	/* To determine: maneuver_des & maneuver_id */
	/*                                         */
	/*******************************************/
                        

    // if ( dvi(time_filter, sw_read_pt, con_state_pt, vehicle_info_pt, &maneuver_des) != 1 )    // Vehicle id is resigned here.
    //     fprintf(stderr, " Calling DVI fail! \n");
               
    if (t_ctrl > 0.001)                                //  05/20/10                  
    {
		if (config_pt->truck_ACC == TRUE)
			con_state_pt-> des_f_dist=(con_state_pt-> spd)*(config_pt->ACC_tGap);
		if (config_pt->truck_CACC == TRUE)
			con_state_pt-> des_f_dist=(con_state_pt-> spd)*(config_pt->CACC_tGap);
		if (con_state. des_f_dist < DES_FOLLOW_DIST)
			con_state. des_f_dist=DES_FOLLOW_DIST;
     
        if (coording(dt, track_length, con_state_pt, sens_read_pt, jbus_read_pt, config_pt, sw_read_pt, f_mode_comm_pt, vehicle_info_pt, pltn_info_pt, manager_cmd_pt) != 1)				
           fprintf(stderr, "\n Calling Coordination fail! \n");			      
	   
       if (run_dist > stop_dist)
		   manager_cmd_pt-> man_des=29;  

       if ( maneuver(dt, t_ctrl, time_filter, v_p, c, d, road_info_pt, config_pt, con_state_pt, sens_read_pt, sw_read_pt, vehicle_info_pt,  // time filter removed on 05_22_11
                 f_index_pt, maneuver_id, manager_cmd_pt, pltn_info_pt) != 1 )
            fprintf(stderr, " Calling maneuver fail! \n");                
    } 


	if (time_filter > 0.0)
        {
           if (ref_dist(dt, maneuver_id, sens_read_pt, vehicle_info_pt, manager_cmd_pt, f_index_pt, con_state_pt, config_pt, pltn_info_pt) != 1)
              fprintf(stderr, " Calling ref_dist_1 fail! \n");
        }
	 	
	/*if (vehicle_info_pt-> veh_id > 1)
	{	
		if (local_time <= t_wait)
			con_state_pt-> temp_dist = con_state_pt-> front_range;
	}*/

     if (vehicle_info_pt-> veh_id == 1)
        {
           pre_maneuver_id[0]=comm_receive_pt[pltn_info_pt->pltn_size]. maneuver_des_1; // Update by communication
           pre_maneuver_id[1]=comm_receive_pt[pltn_info_pt->pltn_size]. maneuver_des_2; // Update by communication           
        }
     if (vehicle_info_pt-> veh_id > 1)
        {
           pre_maneuver_id[0]=comm_receive_pt[vehicle_info_pt-> veh_id - 1]. maneuver_des_1; // Update by communication
           pre_maneuver_id[1]=comm_receive_pt[vehicle_info_pt-> veh_id - 1]. maneuver_des_2; // Update by communication           
        }
    
    
	/***************************************************/
	/*                                                 */
	/*           Close-Loop  Controllers               */
	/*                                                 */
	/***************************************************/


	  if ( cacc( dt, maneuver_id,jbus_read_pt, manager_cmd_pt, config_pt, con_state_pt, sw_read_pt, vehicle_info_pt, con_output_pt) != 1)
        fprintf(stderr, " Calling control fail! \n"); 



	/*****************************/
	/*                           */
	/*  Activating  actuators    */
	/*                           */
	/*****************************/

actuate(dt, pcmd, con_output_pt, con_state_pt, pparams, &inactive_ctrl, manager_cmd_pt, sw_read_pt, jbus_read_pt, config_pt, f_index_pt, vehicle_info_pt);

if( (jbus_read_pt-> accel_pedal_pos1) > 5.0)
	pcmd->engine_command_mode = TSC_OVERRIDE_DISABLED; // indicating driver taking over

// Test brake control

#ifdef TEST_BRK

cmd_count ++;

if (jbus_read_pt-> v > 17.0)
{
   jk_ini=1;
   //brk_ini=1;
}

if (jk_ini == 1)
{
         pcmd->engine_retarder_torque = -30.0;

         pcmd->engine_command_mode = TSC_TORQUE_CONTROL;   
		 pcmd->engine_priority=TSC_LOW; 
         pcmd->engine_torque = 0.0;

		 pcmd->brake_command_mode = XBR_NOT_ACTIVE;
		 pcmd->engine_retarder_command_mode =  XBR_ACTIVE;
		 pcmd->engine_retarder_priority=TSC_HIGHEST;  
		  if (cmd_count == 10)
			    pcmd->engine_retarder_torque =  pcmd->engine_retarder_torque - 1.0;
		  if (cmd_count == 20)
			    pcmd->engine_retarder_torque =  pcmd->engine_retarder_torque + 1.0;
		 
}
if(brk_ini == 1)
{
        pcmd->engine_command_mode = TSC_TORQUE_CONTROL;   
		pcmd->engine_priority=TSC_LOW; 
        pcmd->engine_torque = 0.0;

		pcmd->engine_retarder_command_mode = XBR_NOT_ACTIVE;   
        
		pcmd->brake_command_mode = XBR_ACTIVE; 		  
	    //pcmd->brake_priority=TSC_MEDIUM;
		pcmd->brake_priority=TSC_HIGHEST;
		pcmd->ebs_deceleration = -1.4;     // 
}

#endif
	/************************************/
	/*                                  */
	/*     Emergency Braking.			*/
	/*                                  */
	/************************************/

if (manager_cmd_pt-> auto_contr == ON)
{
 if (vehicle_info_pt-> veh_id == 2)
 {
	
	if ( (comm_receive_pt[1].user_bit_3 == 1) || (comm_receive_pt[1]. user_float > 0.1) ) // manual; brake sw || pedal deflection
	{
		pcmd->engine_command_mode = XBR_NOT_ACTIVE;   	
		pcmd->engine_retarder_command_mode = XBR_NOT_ACTIVE;           
		pcmd->brake_command_mode = XBR_ACTIVE; 		  	   
		pcmd->brake_priority=TSC_HIGHEST;
		pcmd->ebs_deceleration = min_main(-1.2, 1.2*(comm_receive_pt[1].accel));  
	}
	/*if (comm_receive_pt[1].rate < -0.5)  // automatic control with ebs_deceleration
	{
		pcmd->engine_command_mode = XBR_NOT_ACTIVE;   		
		pcmd->brake_command_mode = XBR_ACTIVE; 		  	   
		pcmd->brake_priority=TSC_HIGHEST;
		pcmd->ebs_deceleration = comm_receive_pt[1].rate-0.1;  
		pcmd->engine_retarder_command_mode = XBR_ACTIVE; 
		pcmd->engine_retarder_priority=TSC_HIGHEST; 
		pcmd->engine_retarder_torque =  comm_receive_pt[1].user_float1;
		if (cmd_count == 10)
			    pcmd->engine_retarder_torque =  pcmd->engine_retarder_torque + 1.0;
		if (cmd_count == 20)
			    pcmd->engine_retarder_torque =  pcmd->engine_retarder_torque - 1.0;
	}	*/
 }

 if (vehicle_info_pt-> veh_id == 3)
 {
	if ( (comm_receive_pt[1].user_bit_3 == 1) || (comm_receive_pt[1]. user_float > 0.1) ||
		  (comm_receive_pt[2].user_bit_3 == 1) || (comm_receive_pt[2]. user_float > 0.1) ) // brake sw || pedal deflection
	{
		pcmd->engine_command_mode = XBR_NOT_ACTIVE;   	
		pcmd->engine_retarder_command_mode = XBR_NOT_ACTIVE;           
		pcmd->brake_command_mode = XBR_ACTIVE; 		  	   
		pcmd->brake_priority=TSC_HIGHEST;
		pcmd->ebs_deceleration = min_main(-1.2, 1.2*(comm_receive_pt[1].accel)); 
		pcmd->ebs_deceleration = min_main(pcmd->ebs_deceleration, 1.2*(comm_receive_pt[2].accel));
	}
	/*if ((comm_receive_pt[1].rate < -0.5) || (comm_receive_pt[2].rate < -0.5) )  // automatic control; with ebs_deceleration
	{
		pcmd->engine_command_mode = XBR_NOT_ACTIVE;   	
		pcmd->engine_retarder_command_mode = XBR_NOT_ACTIVE;           
		pcmd->brake_command_mode = XBR_ACTIVE; 		  	   
		pcmd->brake_priority=TSC_HIGHEST;
		pcmd->ebs_deceleration = min_main(comm_receive_pt[1].rate-0.1, comm_receive_pt[2].rate-0.1);  
		pcmd->engine_retarder_command_mode = XBR_ACTIVE; 
		pcmd->engine_retarder_priority=TSC_HIGHEST; 
		pcmd->engine_retarder_torque =  min_main(comm_receive_pt[1].user_float1, comm_receive_pt[2].user_float1);
		if (cmd_count == 10)
			    pcmd->engine_retarder_torque =  pcmd->engine_retarder_torque + 1.0;
		if (cmd_count == 20)
			    pcmd->engine_retarder_torque =  pcmd->engine_retarder_torque - 1.0;
	}*/
 }
}
else
{
	pcmd->engine_retarder_command_mode = XBR_NOT_ACTIVE;          
	pcmd->brake_command_mode = XBR_NOT_ACTIVE;          
}


	/************************************/
	/*                                  */
	/*     Update Communication data.   */
	/*                                  */
	/************************************/

if(config.use_comm == TRUE) 
{
      comm_send_pt.global_time = local_time;  // Each vehicle has a local time to broadcast. 
	  if (vehicle_info_pt-> veh_id == 1 && f_index_pt-> torq == 1 )
	  {
		comm_send_pt.vel_traj = min_main(con_state_pt-> ref_v, sens_read_pt->ego_v);   // composite	 
		comm_send_pt.acc_traj = min_main(con_state_pt-> ref_a, sens_read_pt->ego_a);    // composite   
	  }
	  else
	  {
		  comm_send_pt.vel_traj = con_state_pt-> ref_v;   // composite	 
		  comm_send_pt.acc_traj = con_state_pt-> ref_a;    // composite   
	  }
      comm_send_pt.velocity = sens_read_pt->ego_v; //con_state_pt-> spd;     // measured
      comm_send_pt.accel = sens_read_pt->ego_a;    //con_state_pt-> acc;        // measured
	  if (config_pt->truck_ACC == TRUE)
		comm_send_pt.range = (con_state_pt-> ACC_tGap)*(con_state_pt-> spd);
	  if (config_pt->truck_CACC == TRUE)
		comm_send_pt.range = (con_state_pt-> CACC_tGap)*(con_state_pt-> spd);
	  comm_send_pt.rate = pcmd->ebs_deceleration;	  
	  comm_send_pt.user_bit_1=sw_read_pt-> CC_enable_sw;
	  comm_send_pt.user_bit_2=sw_read_pt-> CC_resume_sw;
	  comm_send_pt.user_bit_3=sw_read_pt-> brk_sw;
	  comm_send_pt.user_bit_4=0;
      comm_send_pt.my_pip = vehicle_info_pt-> veh_id;                  // Determined in the beginning in handshaking.
      comm_send_pt.maneuver_id = manager_cmd_pt-> man_des;
      comm_send_pt.fault_mode = vehicle_info_pt-> fault_mode;
      comm_send_pt.maneuver_des_1 = maneuver_id[0];

	  if (vehicle_info_pt-> cut_in == 1)
		comm_send_pt.maneuver_des_2 = 1;            //maneuver_id[1];
	  else if (vehicle_info_pt-> cut_out == 1)
		comm_send_pt.maneuver_des_2 = 2; 
	  else
		comm_send_pt.maneuver_des_2 = 0;

      //memcpy(pv->self_gps, &self_gps_point, sizeof(path_gps_point_t);
      //comm_send_pt.user_ushort_1 = comm_info_pt-> comm_reply;         // acknowledge receiving; 03_09_09   
	  comm_send_pt.user_ushort_1 = (unsigned short) config_pt-> MyPltnPos;
      comm_send_pt.user_ushort_2 = (unsigned short) manager_cmd_pt-> drive_mode;  // acknowledge the vehicle is in automade; 11_20_09
      comm_send_pt.user_float = jbus_read_pt->  brk_pres;         // changed from con_state_pt-> ref_a on 05_31_16
	  comm_send_pt.user_float1 = pcmd-> engine_retarder_torque;   // changed from con_state_pt-> ref_v on 05_31_16 
      comm_send_pt.pltn_size = pltn_info_pt->pltn_size;
//    fprintf(stderr,"sending packet,global time %f\n", global_time);
//   fflush(stderr);
      db_clt_write(pclt, DB_COMM_TX_VAR, sizeof(veh_comm_packet_t),
                         &comm_send_pt);
}

        /*******************************/
        /*                             */
        /*      Timing checking.       */
        /*                             */
        /*******************************/

        clock_gettime( CLOCK_REALTIME, &curr);
         difference = ( curr.tv_sec - prev.tv_sec )
                        + (double) ( curr.tv_nsec - prev.tv_nsec )
                                        / (double) 1000000000L;
        timestamp = localtime( (time_t *)&curr.tv_sec );
        seconds = timestamp->tm_sec + (double)(curr.tv_nsec /
                                        (double) 1000000000L);

        // save the difference as part of your logging, or use in updating
        // time since start of your program 
        prev = curr;

         
        /*******************************/
        /*                             */
        /*      Save data to buffer.   */
        /*                             */
        /*******************************/

if( (config.save_data == TRUE) && ((data_log_count++ % config.data_log) == 0) ) {

#ifdef COMM_DATA
	if ( vehicle_info_pt-> veh_id == 1)
	{
        fprintf(pout, "%02d:%02d:%02.3f %4.3f %3i %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %3i %3i %3i %3i %3i %3i %3i %3i %3i %3i %3i %4.3f %4.3f %3i %3i\n",	
             timestamp->tm_hour,
             timestamp->tm_min,
             seconds, 
			 time_filter,
			 vehicle_info_pt-> veh_id,
			 comm_receive_pt[2].global_time, 
			comm_receive_pt[2].acc_traj, 
			comm_receive_pt[2].vel_traj,
			comm_receive_pt[2].velocity,
			comm_receive_pt[2].accel,
			comm_receive_pt[2].range,
			comm_receive_pt[2].rate,
			comm_receive_pt[2].user_bit_1,
			comm_receive_pt[2].user_bit_2,
			comm_receive_pt[2].user_bit_3,
			comm_receive_pt[2].user_bit_4,
			comm_receive_pt[2].my_pip,                  
			comm_receive_pt[2].maneuver_id,
			comm_receive_pt[2].fault_mode,
			comm_receive_pt[2].maneuver_des_1,
			comm_receive_pt[2].maneuver_des_2,
			comm_receive_pt[2].user_ushort_1,              
			comm_receive_pt[2].user_ushort_2,
			comm_receive_pt[2].user_float,                   
			comm_receive_pt[2].user_float1,
			comm_receive_pt[2].pltn_size,
			pltn_info_pt-> handshake);
		fprintf(pout, "%02d:%02d:%02.3f %4.3f %3i %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %3i %3i %3i %3i %3i %3i %3i %3i %3i %3i %3i %4.3f %4.3f %3i %3i\n",	
             timestamp->tm_hour,
             timestamp->tm_min,
             seconds, 
			 time_filter,
			 vehicle_info_pt-> veh_id,
			 comm_receive_pt[3].global_time, 
			comm_receive_pt[3].acc_traj, 
			comm_receive_pt[3].vel_traj,
			comm_receive_pt[3].velocity,
			comm_receive_pt[3].accel,
			comm_receive_pt[3].range,
			comm_receive_pt[3].rate,
			comm_receive_pt[3].user_bit_1,
			comm_receive_pt[3].user_bit_2,
			comm_receive_pt[3].user_bit_3,
			comm_receive_pt[3].user_bit_4,
			comm_receive_pt[3].my_pip,                  
			comm_receive_pt[3].maneuver_id,
			comm_receive_pt[3].fault_mode,
			comm_receive_pt[3].maneuver_des_1,
			comm_receive_pt[3].maneuver_des_2,
			comm_receive_pt[3].user_ushort_1,              
			comm_receive_pt[3].user_ushort_2,
			comm_receive_pt[3].user_float,                   
			comm_receive_pt[3].user_float1,
			comm_receive_pt[3].pltn_size,
			pltn_info_pt-> handshake);
	}
	if ( vehicle_info_pt-> veh_id == 2)
	{
		fprintf(pout, "%02d:%02d:%02.3f %4.3f %3i %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %3i %3i %3i %3i %3i %3i %3i %3i %3i %3i %3i %4.3f %4.3f %3i %3i\n",	
             timestamp->tm_hour,
             timestamp->tm_min,
             seconds, 
			 time_filter,
			 vehicle_info_pt-> veh_id,
			 comm_receive_pt[1].global_time, 
			comm_receive_pt[1].acc_traj, 
			comm_receive_pt[1].vel_traj,
			comm_receive_pt[1].velocity,
			comm_receive_pt[1].accel,
			comm_receive_pt[1].range,
			comm_receive_pt[1].rate,
			comm_receive_pt[1].user_bit_1,
			comm_receive_pt[1].user_bit_2,
			comm_receive_pt[1].user_bit_3,
			comm_receive_pt[1].user_bit_4,
			comm_receive_pt[1].my_pip,                  
			comm_receive_pt[1].maneuver_id,
			comm_receive_pt[1].fault_mode,
			comm_receive_pt[1].maneuver_des_1,
			comm_receive_pt[1].maneuver_des_2,
			comm_receive_pt[1].user_ushort_1,              
			comm_receive_pt[1].user_ushort_2,
			comm_receive_pt[1].user_float,                   
			comm_receive_pt[1].user_float1,
			comm_receive_pt[1].pltn_size,
			pltn_info_pt-> handshake);
		fprintf(pout, "%02d:%02d:%02.3f %4.3f %3i %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %3i %3i %3i %3i %3i %3i %3i %3i %3i %3i %3i %4.3f %4.3f %3i %3i\n",	
             timestamp->tm_hour,
             timestamp->tm_min,
             seconds, 
			 time_filter,
			 vehicle_info_pt-> veh_id,
			 comm_receive_pt[3].global_time, 
			comm_receive_pt[3].acc_traj, 
			comm_receive_pt[3].vel_traj,
			comm_receive_pt[3].velocity,
			comm_receive_pt[3].accel,
			comm_receive_pt[3].range,
			comm_receive_pt[3].rate,
			comm_receive_pt[3].user_bit_1,
			comm_receive_pt[3].user_bit_2,
			comm_receive_pt[3].user_bit_3,
			comm_receive_pt[3].user_bit_4,
			comm_receive_pt[3].my_pip,                  
			comm_receive_pt[3].maneuver_id,
			comm_receive_pt[3].fault_mode,
			comm_receive_pt[3].maneuver_des_1,
			comm_receive_pt[3].maneuver_des_2,
			comm_receive_pt[3].user_ushort_1,              
			comm_receive_pt[3].user_ushort_2,
			comm_receive_pt[3].user_float,                   
			comm_receive_pt[3].user_float1,
			comm_receive_pt[3].pltn_size,
			pltn_info_pt-> handshake);
	}
	if ( vehicle_info_pt-> veh_id == 3)
	{
		fprintf(pout, "%02d:%02d:%02.3f %4.3f %3i %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %3i %3i %3i %3i %3i %3i %3i %3i %3i %3i %3i %4.3f %4.3f %3i %3i\n",	
             timestamp->tm_hour,
             timestamp->tm_min,
             seconds, 
			 time_filter,
			 vehicle_info_pt-> veh_id,
			 comm_receive_pt[1].global_time, 
			comm_receive_pt[1].acc_traj, 
			comm_receive_pt[1].vel_traj,
			comm_receive_pt[1].velocity,
			comm_receive_pt[1].accel,
			comm_receive_pt[1].range,
			comm_receive_pt[1].rate,
			comm_receive_pt[1].user_bit_1,
			comm_receive_pt[1].user_bit_2,
			comm_receive_pt[1].user_bit_3,
			comm_receive_pt[1].user_bit_4,
			comm_receive_pt[1].my_pip,                  
			comm_receive_pt[1].maneuver_id,
			comm_receive_pt[1].fault_mode,
			comm_receive_pt[1].maneuver_des_1,
			comm_receive_pt[1].maneuver_des_2,
			comm_receive_pt[1].user_ushort_1,              
			comm_receive_pt[1].user_ushort_2,
			comm_receive_pt[1].user_float,                   
			comm_receive_pt[1].user_float1,
			comm_receive_pt[1].pltn_size,
			pltn_info_pt-> handshake);
		fprintf(pout, "%02d:%02d:%02.3f %4.3f %3i %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %3i %3i %3i %3i %3i %3i %3i %3i %3i %3i %3i %4.3f %4.3f %3i %3i\n",	
             timestamp->tm_hour,
             timestamp->tm_min,
             seconds, 
			 time_filter,
			 vehicle_info_pt-> veh_id,
			 comm_receive_pt[2].global_time, 
			comm_receive_pt[2].acc_traj, 
			comm_receive_pt[2].vel_traj,
			comm_receive_pt[2].velocity,
			comm_receive_pt[2].accel,
			comm_receive_pt[2].range,
			comm_receive_pt[2].rate,
			comm_receive_pt[2].user_bit_1,
			comm_receive_pt[2].user_bit_2,
			comm_receive_pt[2].user_bit_3,
			comm_receive_pt[2].user_bit_4,
			comm_receive_pt[2].my_pip,                  
			comm_receive_pt[2].maneuver_id,
			comm_receive_pt[2].fault_mode,
			comm_receive_pt[2].maneuver_des_1,
			comm_receive_pt[2].maneuver_des_2,
			comm_receive_pt[2].user_ushort_1,              
			comm_receive_pt[2].user_ushort_2,
			comm_receive_pt[2].user_float,                   
			comm_receive_pt[2].user_float1,
			comm_receive_pt[2].pltn_size,
			pltn_info_pt-> handshake);
	}

#endif

if( config.run_data == TRUE ) {
        fprintf(pout, "%02d:%02d:%4.3f %4.3f %3i %3i %3i %3i %3i %3i %3i %3i %3i %3i %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f ",
             timestamp->tm_hour,			//1
             timestamp->tm_min,				//2
             seconds,						//3 
			 time_filter,					//4                       
             //t_ctrl,    
             //maneuver_id[0],  
			 vehicle_info_pt-> cut_in,		//5
			 vehicle_info_pt-> cut_out,		//6
			 manager_cmd_pt-> drive_mode,		//7
			 //manager_cmd_pt-> drive_mode_buff,//8 
			 manager_cmd_pt-> coording_mode,	//8 
             manager_cmd_pt-> man_des,			//9                       
             pltn_info_pt-> handshake,			//10                    
             vehicle_info_pt-> veh_id,			//11                                  
             pltn_info_pt->pltn_size,			//12         
			 sw_read_pt-> CC_enable_sw,			//13
			 sw_read_pt-> brk_sw,				//14            
			 con_state_pt-> max_tq_we,			//15
			 con_state_pt-> max_spd,			//16
			 jbus_read_pt->CC_set_v,			//17	
			 con_state_pt-> spd,				//18
			 jbus_read_pt-> lat_accel,			//19			
			 jbus_read_pt-> long_accel,			//20						 			
			 jbus_read_pt-> yaw_rt,				//21			 			 
			 con_state_pt-> max_jk_we,			//22
			 jbus_read_pt-> jk_actual_percent_tq,		//23  	
			 jbus_read_pt-> jk_max_percent_tq,			//24 
			 pcmd->engine_torque,						//25
			 pcmd->engine_retarder_torque				//26                                                 
        );
        
		  fprintf(pout, "%4.3f %4.3f %4.3f %3i %3i %3i %3i %3i ",
		     //comm_receive_pt[1]. user_float1,        
             //comm_receive_pt[2]. user_float1, 
			 //comm_receive_pt[3]. user_float1,  
			 comm_receive_pt[1]. global_time,			//27      
             comm_receive_pt[2]. global_time,			//28 
			 comm_receive_pt[3]. global_time,			//29
			 vehicle_info_pt->comm_p[0],				//30
			 vehicle_info_pt->comm_p[1],				//31
			 vehicle_info_pt->comm_p[2],				//32			                           
			 vehicle_info_pt-> fault_mode,				//33                  
             //manager_cmd_pt-> f_manage_index 
			 f_index_pt-> torq							//34
        ); 
		
        fprintf(pout,"%4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %3i ",
			 con_output_pt-> y1,						//35              
             con_output_pt-> y2,						//36               
             con_output_pt-> y3,						//37                 
             con_output_pt-> y4,						//38               				
             con_output_pt-> y5,						//39               
             con_output_pt-> y6,						//40                 
             con_output_pt-> y7,						//41                  
             con_output_pt-> y8,						//42                 
             con_output_pt-> y9,						//43                  
             con_output_pt-> y10,						//44
             con_output_pt-> y11,   // usyn				//45		           
             con_output_pt-> y12,   // jk				//46
			 con_output_pt-> y13,   // pb_s2_b			//47              
             con_output_pt-> y14,   // brk				//48 
			 con_state_pt-> tmp_var2,				//49
			 //con_state_pt-> man_dist_var2,				//49
             manager_cmd_pt->control_mode				//50                                          
        );
        fprintf(pout, "%4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f ",               
             con_state_pt-> pre_v,              //51
             con_state_pt-> pre_a,              //52
             con_state_pt-> lead_v,             //53
             con_state_pt-> lead_a,             //54                     
             vehicle_info_pt-> run_dist,        //55             
			 jbus_read_pt-> actual_eng_tq,		//56
             con_state_pt-> temp_dist,          //57  
             con_state_pt-> ref_v,              //58    
             con_state_pt-> ref_a,              //59
             con_state_pt-> des_f_dist,         //60    // manu_speed, changed on 04_24_11
             jbus_read_pt-> fuel_m              //61           //con_state_pt-> manu_acc                                   
        );         
        fprintf(pout, "%3i %3i %3i %4.3f %4.3f %4.3f %4.3f %4.3f ",
             manager_cmd_pt-> trans_mode,		//62                            
			 manager_cmd_pt-> auto_contr,		//63
             sw_read_pt-> gshift_sw,			//64            
             jbus_read_pt-> gear,				//65                   
             pcmd->ebs_deceleration,			//66               
             jbus_read_pt-> bp,					//67                   
             jbus_read_pt-> we,					//68              
			 con_state_pt-> front_range			//69			
        );      
	  fprintf(pout, "%4.3f %4.3f %4.3f %4.3f %4.3f %3i %4.3f ",
		  sens_read_pt->ego_a,					//70
		  sens_read_pt->ego_v,					//71
		  sens_read_pt->target_a,				//72
		  sens_read_pt->target_v,				//73
		  sens_read_pt->target_d,				//74
		  sens_read_pt->target_avail,			//75
		  jbus_read_pt-> steer_angle);			//76
	  fprintf(pout, "%4.8lf %4.8lf %4.3f %4.8lf %4.8lf %4.3f %4.8lf %4.8lf %4.3f ", 
             gps_point[0]. latitude,             //77
             gps_point[0]. longitude,            //78 
			 gps_point[0]. heading,			     //79		
			 gps_point[1]. latitude,             //80
             gps_point[1]. longitude,            //81 
			 gps_point[1]. heading,			     //82		
			 gps_point[2]. latitude,             //83
             gps_point[2]. longitude,            //84 
			 gps_point[2]. heading				 //85
			 );	

	  fprintf(pout, "%4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f ", 
		    jbus_read_pt->  brk_pres,			//86
			pv-> VP15_RoadInclinationVP15,		//87
			pv-> VP15_VehicleWeightVP15,		//88
			pv-> Volvo_TargetDist,				//89
			pv-> Volvo_TargetVel,				//90
			pv-> Volvo_TargetAcc,				//91
			pv-> Volvo_TargetAvailable,			//92
			pv-> Volvo_EgoVel,					//93
			pv-> Volvo_EgoAcc,					//94
			pv-> Volvo_EgoRoadGrade);			//95
	  fprintf(pout, "%3i %3i %3i %4.3f",
		    str_pos_pt->local_pos[0],			//96
			str_pos_pt->local_pos[1],			//97
			str_pos_pt->local_pos[2],			//98
			str_pos_pt->ave_heading);			//99

}

if( config.read_data == TRUE ) {
    fprintf(pout, "%3i %3i %3i %3i %3i %3i %3i %3i %3i %3i %3i %3i  %3i %3i %3i %3i %3i %3i %3i %3i %3i %3i %3i %3i %3i %3i %3i %3i %3i %3i %3i ",  // 31 int
	 (int) sw_read_pt->  park_brk_sw,
	 (int) sw_read_pt->  two_spd_axle_sw,
	 (int) sw_read_pt->  park_brk_release,
	 (int) sw_read_pt->  clutch_sw,
	 (int) sw_read_pt->  brk_sw,
	 /*(int) sw_read_pt->  CC_pause_sw,
	 (int) sw_read_pt->  CC_enable_sw,
	 (int) sw_read_pt->  CC_active,
	 (int) sw_read_pt->  CC_acel_sw,
	 (int) sw_read_pt->  CC_resume_sw,*/
	 (int) con_output_pt->con_sw_1,
	 (int) con_output_pt->con_sw_2,
	 (int) con_output_pt->con_sw_3,
	 (int) con_output_pt->con_sw_4,
	 (int) con_output_pt->con_sw_5,	
	 (int) sw_read_pt->  CC_coast_sw,
	 (int) sw_read_pt->  CC_set_sw,
	 (int) sw_read_pt->  CC_state,
	 (int) sw_read_pt->  Pto_state,
	 (int) sw_read_pt->  eng_shutdwn_override_sw,
	 (int) sw_read_pt->  eng_test_mode_sw,
	 (int) sw_read_pt->  eng_idle_decre_sw,
	 (int) sw_read_pt->  end_idle_incre_sw,
	 (int) sw_read_pt->  driveline_enaged,
	 (int) sw_read_pt->  lockup,
	 (int) sw_read_pt->  gshift_sw, 
	 (int) sw_read_pt->  eng_overspeed_enbale,
	 (int) sw_read_pt->  progressive_shift_disable,
	 (int) sw_read_pt->  jk_enable_shift_assist_sw,
     (int) sw_read_pt->  jk_enable_brake_assist_sw,
     (int) sw_read_pt->  jk_mode,
     (int) sw_read_pt->  jk_require_brk_light,
	 (int) sw_read_pt->  foundation_brk_use,
	 (int) sw_read_pt->  halt_brk_mode,
	 (int) sw_read_pt->  XBR_accel_limit,
	 (int) sw_read_pt->  XBR_active_contr_mode);
	                                                                               //17 integers
	fprintf(pout, "%3i %3i %3i %3i %3i %3i %3i %3i %3i %3i %3i %3i %3i %3i %3i %3i %3i ",
		jbus_read_pt-> eng_tq_mode,					//1
		jbus_read_pt-> eng_mode, 					//2
		jbus_read_pt-> ebs_brake_switch,			//3
		jbus_read_pt-> actual_max_eng_percent_tq,	//4
		jbus_read_pt-> road_v_limit_status,			//5
		jbus_read_pt-> EBS_sw,						//6
		jbus_read_pt-> ABS_amber_warning,			//7
		jbus_read_pt-> ABS_Operation,				//8
		jbus_read_pt-> anti_lock_active,			//9
		jbus_read_pt-> ASR_brk_contr_active,		//10
		jbus_read_pt-> ASR_eng_contr_active,		//11
		jbus_read_pt-> EBS_red_warning,				//12
		jbus_read_pt-> selected_gear, 				//13
		jbus_read_pt-> trans_range_selected,		//14
		jbus_read_pt-> trans_range_attained,		//15
		jbus_read_pt-> jk_driver_dmd_percent_tq,	//16
		jbus_read_pt-> jk_selection_Non_Eng     	//17
        );     
             // 47 float numbers
        fprintf(pout, "%4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f",           
			jbus_read_pt-> we,						//1			
			jbus_read_pt-> w_p,						//2
			jbus_read_pt-> w_t,  					//3
			jbus_read_pt-> driver_dmd_percent_tq,	//4
			jbus_read_pt-> actual_eng_tq, 	//5
			jbus_read_pt-> eng_tq, 					//6
			jbus_read_pt-> eng_dmd_percent_tq,		//7
			jbus_read_pt-> bp,						//8
			jbus_read_pt-> brk_pedal_pos,			//9
			jbus_read_pt-> brk_demand,				//10
			jbus_read_pt-> accel_pedal_volt,		//11
			jbus_read_pt-> accel_pedal_pos,			//12
			jbus_read_pt-> accel_pedal_pos1,		//13
			jbus_read_pt-> accel_pedal_pos2,		//14
			jbus_read_pt-> percent_load_at_current_spd,  	//15
			jbus_read_pt-> nominal_fric_percent_tq, //16
			jbus_read_pt-> eng_des_Op_v,			//17			
			jbus_read_pt-> jk_selection,			//18
			jbus_read_pt-> front_axle_spd,			//19
			jbus_read_pt-> v,						//20
			jbus_read_pt-> fl_axle_diff,			//21
			jbus_read_pt-> fr_axle_diff,			//22
			jbus_read_pt-> rl_axle_diff,			//23
			jbus_read_pt-> rr_axle_diff,			//24
			jbus_read_pt-> rl_axle2_diff,			//25
			jbus_read_pt-> rr_axle2_diff,			//26
			jbus_read_pt-> CC_set_v,				//27
			jbus_read_pt-> trans_out_we,			//28
			jbus_read_pt-> trans_in_we,				//29
			jbus_read_pt-> percent_clutch_slip,		//30
			jbus_read_pt-> actual_gear_ratio,  		//31
			jbus_read_pt-> gear, 					//32
			jbus_read_pt-> gear_flt,     			//33
			jbus_read_pt-> fuel_m,					//34
			jbus_read_pt-> instant_fuel_economy,	//35
			jbus_read_pt-> mean_fuel_economy,		//36
			jbus_read_pt-> throttle_valve_pos,     	//37
			jbus_read_pt-> jk_des_percent_tq,   	//38
			jbus_read_pt-> jk_actual_percent_tq,  	//39
			jbus_read_pt-> jk_max_percent_tq,      	//40
			jbus_read_pt-> pm,        				//41
			jbus_read_pt-> lat_accel,			//42
			jbus_read_pt-> long_accel,			//43
			jbus_read_pt-> steer_angle,			//44	
			jbus_read_pt-> yaw_rt,				//45
			jbus_read_pt-> VP_lat,					//46
			jbus_read_pt-> VP_long);					//47
}
    
    fprintf(pout, "\n");
   		                              
    } 

    // Toggle watchdog timer. Hardware watchdog looks for rising edges, and so
    // sees every other toggle. HW timer activates red LED after 100 ms, so we
    // can miss at most 3 toggles if the loop interval is 20 ms.
    long_setled(pclt, TOGGLE_WATCHDOG);

  return 0;     
}
        



/********************************************************************************
 * exit_tasks
 * Performed before logging out of database.
 * Sends deactivate messages and prints out any stored data.
 * Or any other clean-up.
 *******************************************************************************/

int exit_tasks(db_clt_typ *pclt, long_ctrl *pctrl, long_output_typ *pcmd)
{       
         
//        bool_typ status;

        
        /* Deactivate control, will be written by main program */
        
        *pcmd = inactive_ctrl;
        if (!clt_update(pclt, DB_LONG_OUTPUT_VAR, DB_LONG_OUTPUT_TYPE, // To deactivate control befopre writing data; Main program is to do it agin.
                sizeof(long_output_typ), (void *) pcmd))
                        printf("long_ctl update long_out failed\n");
        //status = buff_done (pbuff);
        fflush(pout);
        fclose(pout);  
                                  
        if (long_setled(pclt, LONG_CTL_INACTIVE) != 0)
           fprintf(stderr, "Setting LONG_CTL_INACTIVE fail! \n");
       
        return 1;
}





         /************************************/
         /*                                  */
         /*            Subroutines            */
         /*                                  */
         /************************************/


/***********************************************************
 * trq_to_acc_voltage 
 * Computes a voltage to send as the acc pedal control from
 * the desired engine torque
 * Used in run_tasks()
 **********************************************************/ 


float trq_to_acc_voltage(long_ctrl *pctrl, float engine_torque)
{
        float idle_torque = MIN_TORQUE/ENGINE_REF_TORQUE;      /* idle torque, param later? */
        float max_torque = pctrl->params.engine_reference_torque;
        float trq_range = max_torque - idle_torque;
        float vlow = 1.0;       /* active voltage lower and upper bounds */
        float vhigh = 3.0;
        float vrange = vhigh - vlow;
        float trq_val = engine_torque - idle_torque;
        if (trq_val < 0) trq_val = 0;
        return (vlow + (trq_val/trq_range) * vrange);
}

float min_main(float a, float b)
{
	if (a>=b)
		return b;
	else
		return a;
}

int max_int(int a, int b)
{
	if (a>=b)
		return a;
	else
		return b;
}