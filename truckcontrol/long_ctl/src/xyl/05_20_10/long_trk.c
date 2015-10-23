/*********************************************************************

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
       Add sens_read_typ          Apr. 11, 03
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
                                                              
       XY_LU


*********************************************************************/

#include <sys_os.h>
#include <timestamp.h>
#include <evt300.h>
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
#include "mdl.h"
#include "densolidar.h"
#include "long_trk_func.h"
#include <time.h>



//#define USE_GPS
//#define LAT_INPUT

static int test_site = RFS; 
static float track_length=260.0;
static float minimum_torque = 320.0;
static float stop_period=0.0;
static float stop_dist=0.0;              
static long data_log_count=0;
static float max_speed=0.0;
//static unsigned short task=0;       

extern bool_typ verbose_flag;           // Needs changing back

static buff_typ *pbuff;                 // Buffer pointer, defined by Paul 
db_clt_typ *pclt=NULL;                   // Database client pointer
db_data_typ db_data_vrd;
db_data_typ db_data_lat;
db_data_typ db_data_sens;
db_data_typ db_data_mark;
db_data_typ db_data_input;
evt300_radar_typ *pvor_radar;
db_data_typ db_data_lidarMA;
db_data_typ db_data_lidarMB;
long_lidarA_typ *plidar_A;
long_lidarB_typ *plidar_B;
mdl_lidar_typ *pmdl_lidar;

// For communication
db_data_typ db_data_comm_1;
db_data_typ db_data_comm_2;
db_data_typ db_data_comm_3;
db_data_typ db_data_comm_send;
veh_comm_packet_t comm_receive_pt[MAX_TRUCK];
veh_comm_packet_t comm_send_pt;

FILE* pout;                              // 04_04_03

// For veh_long.h
static fault_index_typ f_index;
static fault_index_typ* f_index_pt; 
static switch_typ switching;
static switch_typ* sw_pt;
static control_config_typ config;      
static control_config_typ* config_pt;
static control_state_typ con_state;
static control_state_typ* con_state_pt;
static road_info_typ road_info;
static road_info_typ* road_info_pt;
static con_output_typ con_output;
static con_output_typ* con_output_pt;
static sens_read_typ sens_read;
static sens_read_typ* sens_read_pt;
static evrd_out_typ evrd_out;
static evrd_out_typ* evrd_out_pt;
static evrd_in_typ evrd_in;
static evrd_in_typ* evrd_in_pt;
static ldr_out_typ ldr_out;
static ldr_out_typ* ldr_out_pt;
static mdl_out_typ ldr_mdl;
static mdl_out_typ* ldr_mdl_pt;

// For coording.h
static vehicle_info_typ vehicle_info;
static vehicle_info_typ* vehicle_info_pt;
static f_mode_comm_typ f_mode_comm;
static f_mode_comm_typ* f_mode_comm_pt;
static comm_info_typ comm_info;
static comm_info_typ* comm_info_pt;
static manager_cmd_typ manager_cmd;
static manager_cmd_typ* manager_cmd_pt;
static path_gps_point_t self_gps_point;       // read from self GPS database variable
static path_gps_point_t lead_gps;       // take from veh_comm_packet_t
static path_gps_point_t prec_gps;       // take from veh_comm_packet_t

long_params *pparams;
long_vehicle_state *pv;
cbuff_typ *pbuff1;
buffer_item *pdata;
long_private *ppriv;
private_params *pcparams;
char *ini_file;

float c[N_pt-1]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
float d[N_pt-1]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
float v_p[N_pt]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};




static long_private cmd_private;    // Sue
const unsigned short comm_err_bound = 20;

/*********************************************

The following struct is used in exit_tasks()
and transitions

*********************************************/

static long_output_typ inactive_ctrl =
{
        600.0,                   /* engine speed, truck idle, don't care since disabled */
        400.0,                   /* engine torque, truck idle, don't care since disabled */
        0.0,                     /* retarder torque, don't care since disabled */
        TSC_OVERRIDE_DISABLED,   /* engine command mode */
        TSC_OVERRIDE_DISABLED,   /* engine retarder command mode */
        0.0,                     /* accelerator pedal voltage -- not used by jbussend */
        0.0,                     /* ebs deceleration */
        EXAC_NOT_ACTIVE,         /* brake command mode */
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
  
//    unsigned short j=0;
    
    void ref_ini(control_config_typ*, float *, float *, float *);

    ini_file = pparams->long_cfg_file;    /* initialization file name */ 
         /* file pointer for initialization files, both are used */
 
 //   verbose_flag=TRUE;
    

       /*** Sue's Code ***/
       pparams = &pctrl->params;
       pbuff1 = &pctrl->buff;

       pcparams = &cmd_private.cmd_params;

        /*** XY_Lu's Code ***/
     

       pout=fopen("/big/data/test.dat","w");                  // 04_16_03, working now
          if (pout == NULL)
          {
             printf("Open output file for writing fails!");
             fflush(stdout);
          }
       //if( ( pbuff = buff_init( pout, MAX_BUFFER_SIZE ) ) == NULL)    // 04_04_03          
       //   return(FALSE);  

         
        /* Log in to the database (shared global memory).  Default to the
         * current host. */
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
    config.task=(unsigned short)get_ini_long( pfin1,"Task", 1L );
    config.run=(unsigned short)get_ini_long( pfin1,"Run", 1L );
    
    config.max_spd=(float)get_ini_double(pfin1, "MaxSpeed", 15.0 );
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
    config.pltn_size = get_ini_long( pfin1, "PlatoonSize", 2L );
    config.use_comm = get_ini_bool( pfin1, "UseComm", TRUE );
    config.eaton_radar = get_ini_bool( pfin1, "EatonRadar", TRUE );
    config.save_data = get_ini_bool( pfin1, "SaveData", TRUE );
    config.run_data = get_ini_bool( pfin1, "RunData", TRUE );
    config.read_data = get_ini_bool( pfin1, "ReadData", TRUE );
    config.trans_sw = get_ini_bool( pfin1, "TransSW", TRUE );
    config.use_gps = get_ini_bool( pfin1, "UseGPS", TRUE );
    config.use_mag = get_ini_bool( pfin1, "UseMagnets", TRUE );
    config.handle_faults= get_ini_bool( pfin1, "HandleFaults", FALSE);

    config.end_scenario = get_ini_bool( pfin1, "EndScenario", TRUE );
    config.step_start_time = (float)get_ini_double( pfin1, "StepStartTime", 2.5 );
    config.step_end_time = (float)get_ini_double( pfin1, "StepEndTime", 5.0 );
    config.tq_cmd_coeff = (float)get_ini_double( pfin1, "TqCmdCoff", 1.0 );
    config.spd_cmd_coeff = (float)get_ini_double( pfin1, "SpdCmdCoeff", 1.0 );
    config.jk_cmd_coeff = (float)get_ini_double( pfin1, "JKCmdCoeff", 1.0 );
    config.trtd_cmd_coeff = (float)get_ini_double( pfin1, "TrtdCmdCoeff", 1.0 );   

    fclose(pfin1);
    
    data_log_count=0;               // initialization 

     /***********************************************************/
     /*        Initializing test track related parameters       */
     /***********************************************************/
    
    if( test_site == RFS )
       {
          if (config.max_spd > 15.0) //26.0
             config.max_spd=15.0;
          config.max_spd=(config.max_spd)*1609.0/3600.0;         
          config.max_dcc=MAX_DCC;
          con_state. mag_space = 1.0;
          track_length = 250.0;   //350.0;        // [m]
       }
    else if( test_site == CRO )
       { 
         if (config.max_spd > 20.0)
             config.max_spd=20.0;
          config.max_spd=(config.max_spd)*1609.0/3600.0;
          con_state. mag_space = 1.2;
          track_length = 250.0;                        // 2350m, Tested on 04_29_03
          //track_length = 1000.0;                        
  
          config.max_dcc=MAX_DCC;
       }
    else if( test_site == I15 )
       {
         if (config.max_spd > 20.0)
             config.max_spd=20.0;
          config.max_spd=(config.max_spd)*1609.0/3600.0; 
          con_state. mag_space = 1.2;
          track_length = 14000.0;          
          config.max_dcc=MAX_DCC;
       }
    else if( test_site == N11 )
       {
          if (config.max_spd > 60.0)
             config.max_spd=60.0; 
          config.max_spd=(config.max_spd)*1609.0/3600.0;
          con_state. mag_space = 1.25;
          track_length = 300.0;

          config.max_dcc=MAX_DCC;
       }
    else;
    
   //task = 0;                                            
 
   
     /*** Initialization ***/

   // From veh_long.h
   
     memset(&comm_send_pt, 0, sizeof(veh_comm_packet_t));
 
     memset(&f_index, 0, sizeof(f_index));

     switching. fan_sw=0;
     switching. comp_sw=1;
     switching. cond_sw=1;
     switching. steer_sw=1;
     switching. alt_sw=1;
     switching. actuator_sw=1;    
     switching. radar1_sw=1;
     switching. radar2_sw=1;
     switching. auto_sw=0;        
     switching. manu_sw=0;
     switching. HMI_sw=1;                    

     
     memset(&con_state, 0, sizeof(con_state));
     con_state. comm_coord=OFF;
     con_state. comm_leader=OFF;
     con_state. comm_pre=OFF;
     con_state. comm_back=OFF;
     con_state. des_f_dist=DES_FOLLOW_DIST; // Necesary to update for maneuvers // But changed according to  task number for support of USC & UCR
     con_state. split_f_dist=SPLIT_FINAL_DIST;
     con_state. join_f_dist=JOIN_FINAL_DIST;
     if (con_state. join_f_dist<4.0)
         con_state. join_f_dist=4.0;        
     con_state. drive_mode=1;           
     con_state. max_brake=MAX_BRAKE;        
     con_state. min_brake=MIN_BRAKE;        
     con_state. ini_brake=INI_BRAKE;        
     con_state. stop_brake=STOP_BRAKE;
          
     
     memset(&road_info, 0, sizeof(road_info));      
     memset(&con_output, 0, sizeof(con_output));           
     memset(&sens_read, 0, sizeof(sens_read));
     sens_read. gshift_sw=OFF;                      
     sens_read. lockup=OFF;                          
     sens_read. driveline_engaged=OFF;    
     memset(&evrd_in, 0, sizeof(evrd_in));
     memset(&evrd_out, 0, sizeof(evrd_out));
     memset(&ldr_out, 0, sizeof(ldr_out));
     memset(&ldr_mdl, 0, sizeof(ldr_mdl));             
     memset(&vehicle_info, 0, sizeof(vehicle_info));
     vehicle_info. veh_id=1;                                
     memset(&f_mode_comm, 0, sizeof(f_mode_comm));
     f_mode_comm. pltn_id=1;                        
     memset(&comm_info, 0, sizeof(comm_info));                                    
     memset(&manager_cmd, 0, sizeof(manager_cmd));        
     manager_cmd. stop_dist=200; 
          
     vehicle_info_pt = &vehicle_info;
     f_mode_comm_pt = &f_mode_comm;
     comm_info_pt = &comm_info;
     manager_cmd_pt = &manager_cmd; 
     con_state_pt = &con_state;
     f_index_pt = &f_index;
     config_pt = &config;
     sw_pt = &switching;   
     road_info_pt = &road_info;
     con_output_pt = &con_output;
     sens_read_pt = &sens_read;
     evrd_in_pt = &evrd_in;
     evrd_out_pt = &evrd_out;
     ldr_out_pt = &ldr_out;
     ldr_mdl_pt = &ldr_mdl;
                              
     //config_pt-> max_acc=0.0; 
     //config_pt-> max_dcc=0.0;  
     
              
        max_speed = config_pt->max_spd;       // local and differnt unit
       //stop_dist = track_length - 0.5*max_speed*max_speed/MAX_DCC;                     // Used before Nov. 25 02

        if (config_pt-> task == 4)      // Transition between manual and automatic
          track_length = 20000.0;
        stop_period=2.0*(config.max_spd) / (config.max_dcc);                                          // Used after Nov. 25 02
        stop_dist = track_length - ((config.max_spd)*stop_period - 0.25*(config.max_dcc)*stop_period*stop_period); 
                   
        manager_cmd_pt-> stop_dist=stop_dist;
       



        pcmd->engine_command_mode = TSC_OVERRIDE_DISABLED;
        pcmd->brake_command_mode = EXAC_NOT_ACTIVE;
        pcmd->engine_retarder_command_mode = TSC_OVERRIDE_DISABLED;
        pcmd->trans_retarder_command_mode = TSC_OVERRIDE_DISABLED;

        pcparams->gather_data = 30000;
    
        if (pparams->vehicle_type == VEH_TYPE_TRUCK_BLUE)
                minimum_torque = 320.0;
        else
                minimum_torque = 350.0;
        /* Initialize data buffer */
        init_circular_buffer(pbuff1, pcparams->gather_data, sizeof(buffer_item));  // Using pbuff1
        ftime(&pctrl->start_time);
        
        

        if (config_pt-> task == 0)     // To support to USC Fred
           {
              if (config_pt-> run == 1)
                 con_state. des_f_dist=10.3;
              if (config_pt-> run == 2)
                 con_state. des_f_dist=8.3;
              if (config_pt-> run == 4)
                 con_state. des_f_dist=6.3;
              if (config_pt-> run == 5)
                 con_state. des_f_dist=4.5;
       


              if (config_pt-> run == 7)
                 con_state. des_f_dist=10.3;
              if (config_pt-> run == 8)
                 con_state. des_f_dist=8.3;
              if (config_pt-> run == 10)
                 con_state. des_f_dist=6.3;
              if (config_pt-> run == 11)
                 con_state. des_f_dist=4.5;

              if (config_pt-> run == 13)
                 con_state. des_f_dist=10.3;
              if (config_pt-> run == 14)
                 con_state. des_f_dist=8.3;
              if (config_pt-> run == 16)
                 con_state. des_f_dist=6.0;
              if (config_pt-> run == 17)
                 con_state. des_f_dist=4.5;

              if (config_pt-> run == 100)
                 con_state. des_f_dist=3.6;
           }

        if (config_pt-> task == 1)     // For our own platooning
           {
              if (config_pt-> run == 1)
                 con_state. des_f_dist=10.0;
              if (config_pt-> run == 2)
                 con_state. des_f_dist=8.0;
              if (config_pt-> run == 4)
                 con_state. des_f_dist=6.0;
              if (config_pt-> run == 5)
                 con_state. des_f_dist=4.0;
              if (config_pt-> run == 6)
                 con_state. des_f_dist=3.0;                 
           }
        if (config_pt-> task == 2)      // Splitting
           {
              con_state. des_f_dist=6.0;  //  8.0==> 10.0       // For test only
              con_state. split_f_dist=10.0;              
           }
     
           
        if (config_pt-> task == 3)      // Joining
           {
              con_state. des_f_dist=8.0;   // 10.0 ==> 8.0
              con_state. join_f_dist=6.0;           
           }
        if (config_pt-> task == 4)      // Transition between manual and automatic
           track_length = 20000.0;

        /*** Protection for Safety ***/ 

        if (con_state. des_f_dist < 3.0)
           con_state. des_f_dist=3.0;

        if (con_state. des_f_dist < 6.0)
           config.max_dcc=MAX_DCC+0.1*(6.0-con_state. des_f_dist);    // To split a little for protection in short dist following.

  
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
    const int radar_sw = 5;
/*--- All the time related variables ---*/

    static float global_time=0.0, local_time=0.0;    
    static float t_ctrl=0.0, dt=0.0, time_filter=0.0;    
    static unsigned short handshake_start = OFF, prt_buff=0, comm_prt1_sw = ON;
    static unsigned short synchrn_sw = 1, comm_prt_sw=ON, prt1_sw =ON, global_t_sw=OFF;                     // Only used once
 	static int read_sw=0, read_sw_old=0, manu_auto_sw=0;
    
    /*--- Sensor Readings From Data Base---*/

    
    //static float fuel=0.0, fuel_m=0.0; // fuel_old=0.2, fuel_temp=0.0;  
    float fuel_m1=0.0, fuel_m2=0.0;
    float fuel_val1_pos=0.0, fuel_val2_pos=0.0;
    
 

    static float pitch_flt=0.0;
       
    float v1=0.0, v2=0.0, v3=0.0;
    float fl_axle_diff=0.0, fr_axle_diff=0.0, rl_axle_diff=0.0, rr_axle_diff=0.0;
    static float v=0.0;       //v0=0.0;
  
    static float acl=0.0, acl_old=0.0;
    static float acc_pedal_pos=0.0, brk_pedal_pos=0.0;
    float percent_load_v=0.0;
   
    
   /* static float  preceding_speed=0.0;  // preceding_accel=0.0;
    static float distance=0.0, distance_old=0.0, temp_dist=0.0, dist_crt_t=0.0, real_dist=0.0,            // temp_dist_d=0.0;
    static float eps=0.0, eps_dot=0.0, dist_trans_t=0.0, dist_buff=0.0, initial_dist=0.0; */

    //static float range_obs=0.0;
    static float mag_dist_est=0.0; // mag_start_dist=0.0, mag_distance=0.0; 
    static float pre_move_dist=0.0, move_dist=0.0; 
                                       
    float lid_hi_rg[8]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
    float lid_hi_rt[8]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
   
    float we=0.0;
    static float we_old=0.0;
    // float we_flt=0.0;
    //float pm=0.0;
//    static float pm_flt=0.0;
    float eng_tq = 0.0;
    float nominal_fric_tq=0.0;
    
    float fan_spd_est=0.0; 
    float jk_tq=0.0;
    float jk_percent_tq=0.0;
   

    float trans_rtd_value=0.0;
    float trans_rtd_mode=0.0;
    float trans_rtd_volt=0.0;

//    unsigned short j=0;
        
    static float gear=0.0, gear_old=0.0, gear_flt=0.0, tg=0.0; // , rg=0.0;
 
    float actual_gear_ratio;            // from transmission 

    int eng_mode;                       // from engine 
    int jk_mode;                        // from engine retarder 
   
    int selected_gear;                  // from transmission 
    
    int fan_drive_state;                // from engine 
    int ebs_brake_switch;               // from EBS 
    int abs_ebs_amber_warning_state;    // from EBS 
    
       
    //char print_buff[550];   // 400 will be enough for control         //Removed on 11/14/08

    //static short unsigned tr_cmd_off = OFF;
   

    /*--- Vehicle Parameters ---*/
    
    static int  vehicle_pip = 1;             // Initialization
    

    /*--- Maneuver Coordination ---*/

    static int maneuver_id[2]={0,0}, maneuver_des=0;          // for single vehicle use
    static int pre_maneuver_id[2]={0,0};
    int  Index[3]={0,0,0};
   

    static float run_dist=0.0;                              

    
         
    /*--- Fault Detect ---*/
    

//    float auxillary;          // updated in rad_dist()

    /*--- Transition Control ---*/

    
    //static int auto_trans_old=0; 
 
    float inst_brk_power=0.0;
    float brk_demand=0.0;
    
  

    float engine_reference_torque;
    int ctrl_interval;
//    unsigned buff_err=0;                               // For DBG
//    static int gather_data=0;
//    buffer_item current_item;


// declarations for timing checking                    // For DBG on Aug 6 03

        static struct timespec prev, curr;
        double difference;
        static int time_sw = 1;

     // At initialization  for timing                                        // For DBG on Aug 6 03
        if (time_sw == 1)
           {
              clock_gettime( CLOCK_REALTIME, &prev); 
              time_sw = 0;            
           }

        /** Set up pointer to input values from data server in
	 *  in vehicle_state that has been read by long_read_vehicle_state
	 *  in ../common/long_ctl.c main program.
	 */
        
        pv = &pctrl->vehicle_state;

	/** Set up pointer to configuration information set up in
	 *  ../common/long_ctl.c man program
	 */ 
		pparams = &pctrl->params;

		/* Set up pointers for data gathering
		 */
		pbuff1 = &pctrl->buff;
		pdata = (buffer_item *) pbuff1->data_array;
		
		ppriv = (long_private *) pctrl->plong_private;  // Control switch to different cases
		
		pcparams = &ppriv->cmd_params;
		
	       

		/* copy execution state for reference after update */
		//int current_state = ppriv->execution_state;                  // Directly control switch to different cases

        /* copy unchanging parameters to local variables for easy reference */
        engine_reference_torque = pparams->engine_reference_torque;
        //desired_vehicle_speed = pcparams->desired_vehicle_speed; 
        //delta_torque = pcparams->delta_torque;
        //trq_time_limit = pcparams->trq_time_limit;
        
        ctrl_interval = pparams->ctrl_interval;   /* in milliseconds */
     
     /*******************************************/
     /*                                         */
     /* Set up local variables with vehicle state */
     /*    information from data server		*/
     /*                                         */
     /*******************************************/

     if (con_state_pt-> drive_mode == 3) 
        vehicle_info_pt-> pltn_size = 1;
     else
       vehicle_info_pt-> pltn_size = config.pltn_size;                 // Should get from avcs_xyl
        

     if (config_pt-> task == 0)                          // For support to USC
        {
           if (config_pt->run == 3 || config_pt->run == 6 || config_pt->run == 9 || config_pt->run == 12 || config_pt->run == 15 || config_pt->run == 18)    // For suporting USC & UCR
              {
                 vehicle_info_pt-> pltn_size = 1;
                 vehicle_pip = 1;
                 vehicle_info_pt-> veh_id = 1;           
              }
        }

     if (config_pt-> task == 4)                         // Transition
        {
           vehicle_info_pt-> pltn_size = 1;
           vehicle_pip = 1;
           vehicle_info_pt-> veh_id = 1;           
        }
        
        
        
if(config.truck_platoon == TRUE) {
     con_state_pt-> drive_mode = 1;
     vehicle_info_pt-> pltn_size = config.pltn_size; 
     vehicle_pip = pparams->vehicle_position;
     vehicle_info_pt-> veh_id = vehicle_pip;
}
else {
     con_state_pt-> drive_mode = 1;
     vehicle_info_pt-> pltn_size = 1;                  
     vehicle_info_pt-> veh_id = 1;                      
     vehicle_pip = 1;
}



       /*----  From Communication  ----*/
if(config.use_comm == TRUE) {
    if (vehicle_info_pt-> pltn_size == 2)
    {
	comm_receive_pt[1] = pv->lead_trk;
	comm_receive_pt[2] = pv->second_trk;
    }
    else if (vehicle_info_pt-> pltn_size == 3)        // need to be modified
    {       
	comm_receive_pt[1] = pv->lead_trk;
	comm_receive_pt[2] = pv->second_trk;
	comm_receive_pt[3] = pv->third_trk;
    }
}
    
       /*--- Eaton Vorad radar data ---*/

if(config.eaton_radar == TRUE) {
        pvor_radar= &pv->evt300;
}

       /*--- Denso Lidar data ---*/

if(pparams->denso_lidar == 1) {
        plidar_A = &pv->lidarA;
        plidar_B = &pv->lidarB; 
                
     lid_hi_rg[0]=1.28*plidar_A-> h_dist_1;         // Range: 0 --162.56[m]
     lid_hi_rt[0] =1.28*plidar_A-> h_velocity_1;    // Range: -64 -- +64[m/s]; Negative means approaching
     lid_hi_rg[1]=1.28*plidar_A-> h_dist_2;         
     lid_hi_rt[1] =1.28*plidar_A-> h_velocity_2;  
     lid_hi_rg[2]=1.28*plidar_A-> h_dist_3;         
     lid_hi_rt[2] =1.28*plidar_A-> h_velocity_3;  
     lid_hi_rg[3]=1.28*plidar_A-> h_dist_4;         
     lid_hi_rt[3] =1.28*plidar_A-> h_velocity_4;  
     lid_hi_rg[4]=1.28*plidar_B-> h_dist_5;         
     lid_hi_rt[4] =1.28*plidar_B-> h_velocity_5;  
     lid_hi_rg[5]=1.28*plidar_B-> h_dist_6;         
     lid_hi_rt[5] =1.28*plidar_B-> h_velocity_6;  
     lid_hi_rg[6]=1.28*plidar_B-> h_dist_7;         
     lid_hi_rt[6] =1.28*plidar_B-> h_velocity_7;  
     lid_hi_rg[7]=1.28*plidar_B-> h_dist_8;         
     lid_hi_rt[7] =1.28*plidar_B-> h_velocity_8;
}

	/*--- MDL lidar data ---*/

if( pparams->mdl_lidar == TRUE ) {
	pmdl_lidar = &pv->mdl_lidar;
}

if(pparams->denso_lidar == 1) {
	self_gps_point = pv->self_gps;
}

	/**************************************/
	/*                                    */
	/*               Timing               */
	/*                                    */
	/**************************************/

    dt = ctrl_interval * 0.001; // in seconds 
    local_time += dt;                                                                        
	/***************************************************/
	/*                                                 */
	/*        Setup Communication Link and             */
	/*        get information from front and back      */
	/*        vehicles on non-fault basis              */
	/*                                                 */
	/***************************************************/
if(config.use_comm == TRUE) 
	comm( &handshake_start, &vehicle_pip, &local_time, &global_time, 
		&comm_prt_sw,  &comm_prt1_sw, &synchrn_sw, &global_t_sw, &dt, 
		&time_filter, con_state_pt, vehicle_info_pt, comm_info_pt,
		f_index_pt, &comm_err_bound, comm_receive_pt,
		&comm_send_pt);

          /**************************************/
          /*  Read in sensor measurement        */ 
          /*  from database to local variables  */ 
          /*                                    */
          /**************************************/

read_sens(&gear, &selected_gear, &fan_drive_state, &actual_gear_ratio, 
	&percent_load_v, &v1, &v2, &v3, &fl_axle_diff, &fr_axle_diff, 
	&rl_axle_diff, &rr_axle_diff, &acl, &eng_mode, &we, &fan_spd_est, 
	&eng_tq, &nominal_fric_tq, &fuel_m1, &fuel_m2, &fuel_val1_pos, 
	&fuel_val2_pos,  &acc_pedal_pos, &jk_mode, &jk_tq, &jk_percent_tq, 
	&ebs_brake_switch, &abs_ebs_amber_warning_state, &inst_brk_power, 
	&brk_pedal_pos, &brk_demand, &trans_rtd_mode, &trans_rtd_value, 
	&trans_rtd_volt, &time_filter, &we_old, pv, pparams, sens_read_pt);
     
    
        /*******************************************/
        /*                                         */                       
        /*    Update switching state data base     */
        /*                                         */
        /*******************************************/
if( config.trans_sw == TRUE ) 
	config_sw(&read_sw, &manu_auto_sw, &handshake_start, &read_sw_old, 
	sw_pt, pv, vehicle_info_pt, comm_receive_pt, &prt1_sw);

	/****************************************/
	/*                                   	*/
	/*	Starting mode			*/
	/*		Set LEDs		*/
	/*		Decide synchonized time	*/
	/*		based on communication	*/
	/*                                   	*/
	/****************************************/

set_init_leds(pclt, &prt1_sw, &prt_buff, &handshake_start, sw_pt, 
	vehicle_info_pt, comm_receive_pt); 
set_time_sync(&t_ctrl, &dt, &time_filter, &vehicle_pip, vehicle_info_pt);

        
	/***************************************/
	/*                                     */
	/*	Signal Processing              */
	/*                                     */
	/***************************************/

v_flt( dt, time_filter, maneuver_des, v1, v2, v3, &v, f_index_pt);
       
process_sigs(&dt, &run_dist, &v, &acl, &acl_old, lid_hi_rg, lid_hi_rt, 
	maneuver_id, pre_maneuver_id, &radar_sw, vehicle_info_pt, &config,
	con_state_pt, evrd_out_pt, ldr_out_pt, pvor_radar, f_index_pt, 
	pmdl_lidar, pparams);

if( config.use_mag == TRUE) {                                     
     mag_dist(dt, time_filter, con_state_pt, &pre_move_dist, &move_dist, &mag_dist_est);                                                         
     con_state_pt-> mag_range=0.0;
     con_state_pt-> mag_range_rate=0.0;                           
}                                                                 

	/*******************************************/
	/*                                         */
	/*    Update control state data base       */
	/*                                         */
	/*******************************************/  
     
     con_state_pt-> spd=v;    
     con_state_pt-> acc=acl;
     con_state_pt-> fuel_rt= pv-> fuel_rate; 
   
     if ( (con_state_pt-> drive_mode == 1) || (con_state_pt-> drive_mode == 2) )  // Messurement update here; cmd update later
         {            
             con_state_pt-> auto_acc=acl;
             con_state_pt-> auto_speed=v;
             con_state_pt-> auto_throt=pv-> fuel_rate;
             con_state_pt-> auto_spd_cmd=0.0;
             con_state_pt-> auto_tq_cmd=0.0;
             con_state_pt-> auto_brk=sens_read_pt-> bp;
             con_state_pt-> auto_rtd=0.0;
             con_state_pt-> auto_jake2=0.0;
             con_state_pt-> auto_jake4=0.0;
             con_state_pt-> auto_jake6=0.0;     
         }
     if (con_state_pt-> drive_mode == 3)
         {
             con_state_pt-> manu_acc=acl;
             con_state_pt-> manu_speed=v;
             con_state_pt-> manu_throt=pv-> fuel_rate;          
             con_state_pt-> manu_spd_cmd=0.0;
             con_state_pt-> manu_tq_cmd=eng_map(sens_read_pt->we_flt, sens_read_pt-> fuel_m,3,1,Index);    //Equivalent tq_cmd
             con_state_pt-> manu_brk=sens_read_pt-> bp;
             con_state_pt-> manu_rtd=pv-> trans_retarder_value;  // or: pv-> trans_retarder_voltage
             con_state_pt-> manu_jake2=0.0;
             con_state_pt-> manu_jake4=0.0;
             con_state_pt-> manu_jake6=0.0;
             
         }  
     con_state_pt-> pltn_vel=0.0;
     con_state_pt-> pltn_acc=0.0;
     con_state_pt-> pltn_dcc=0.0;

        /*****************************************************************/
        /*                                                               */
        /*      Calculate time after gear change begins and gear ratio   */
        /*                                                               */
	/*****************************************************************/
       
        if( time_filter < 0.07 )
          {
              gear_old = gear;
              gear_flt= gear;
          }

        if(sens_read_pt-> gshift_sw == OFF) 
                tg = 0.0;       
        else
                tg += dt;
        sens_read_pt-> tg = tg;
                  
 
#ifdef GEAR_FLT

        if( (maneuver_id[0] == 0) || (maneuver_id[0] == 30) )
                tg = 0.0;
        if( time_filter >= 0.07 )
                gear_old = gear;

        if( (tg < 1.2) && (time_filter > 1.2) && (gear_flt < 1.5) )
              ;
        else if( (tg < 0.6) && (time_filter > 0.6) && (gear_flt < 2.5) )
              ;
        else if( (tg < 0.6) && (time_filter > 0.6) && (gear_flt < 3.5) )
              ;
        else if( (tg < 0.6) && (time_filter > 0.6) && (gear_flt < 4.5) )
              ;
        else if( (tg < 0.6) && (time_filter > 0.6) && (gear_flt < 5.5) )
              ;
        else
             sens_read_pt-> gear_flt = 0.2*gear + 0.8*gear_flt;  
#endif

        sens_read_pt-> gear_flt = gear;    
        sens_read_pt-> rg = gear_tbl( gear );
        

     /*---- Filtering Reference Signal ---*/
    

        if( con_state_pt-> pre_v <= 0.0 )
            con_state_pt-> pre_v = 0.0;


 
            
	/*******************************************/
	/*                                         */
	/*       Fault Detect and Management       */
	/*       Setting LED                       */
	/*                                         */
	/*******************************************/
	
if( (config.handle_faults == TRUE) && (maneuver_des > 1) )
{ 
   
   if (vehicle_info_pt-> pltn_size >= 1)       
     {
	    if (vehicle_info_pt-> veh_id == 1)
	       {
		      if(f_index_pt-> comm == 1) 		         
			     vehicle_info_pt-> fault_mode = 3;
	       }
	    else
	       {
		       if (f_index_pt-> comm == 1)
		          vehicle_info_pt-> fault_mode = 1;
		       if (((f_index_pt-> e_vrd==1)&&(f_index_pt-> lidar==0)) || ((f_index_pt-> e_vrd==0)&&(f_index_pt-> lidar==1)) )		                    		       	  
                  vehicle_info_pt-> fault_mode = 2;                                                      
               if ( (f_index_pt-> comm == 1) || ((f_index_pt-> e_vrd==1)&&(f_index_pt-> lidar==1)) )                                            
                  vehicle_info_pt-> fault_mode = 3;                     		          
           }       
     } 
     
     if ( (f_index_pt-> spd == 1) || (f_index_pt-> J_bus_1 == 1) )         
           vehicle_info_pt-> fault_mode = 3; 
           
     if (f_index_pt-> comm == 0)
        manager_cmd_pt-> f_manage_index = max_i(vehicle_info_pt-> fault_mode, vehicle_info_pt-> pltn_fault_mode);    	    
               
     if (manager_cmd_pt-> f_manage_index == 1)        
        {
            fprintf(stderr, " Setting FLT_LOW f_index_pt->comm %d vehicle_info_pt->fault_mode %d f_index_pt->comm_coord %d\n",
				              f_index_pt->comm, vehicle_info_pt->fault_mode, f_index_pt->comm_coord );
	        if (long_setled(pclt, FLT_LOW) != 0)
               fprintf(stderr, " Setting FLT_LOW fail! \n");             
        }
        
     if (manager_cmd_pt-> f_manage_index == 2)
        {
            fprintf(stderr, " Setting FLT_MED f_index_pt->comm %d vehicle_info_pt->fault_mode %d\n",
				                          f_index_pt->comm, vehicle_info_pt->fault_mode );
	        if (long_setled(pclt, FLT_MED) != 0)
                fprintf(stderr, " Setting FTL_MED fail! \n");            
        }
     if (manager_cmd_pt-> f_manage_index  == 3)
        {
	        //sw_pt-> auto_sw = 0;
			//sw_pt-> manu_sw = 1;	
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
                        

     // To determine maneuver_des.  Rewritten on 10_21_03

     if ( dvi(time_filter, sw_pt, con_state_pt, vehicle_info_pt, &maneuver_des) != 1 )    // Vehicle id is resigned here.
         fprintf(stderr, " Calling DVI fail! \n");
               
    
    // printf("In Run Tasks: test site: %i; vehicle id: %i\n", 
    //		test_site, vehicle_info_pt-> veh_id);
    
    //if (vehicle_info_pt-> ready == 1)
    if (t_ctrl > 0.001)                                //05/05/10                  
    {
     	if ((vehicle_pip == 0) || (vehicle_pip == 1))  //05_05_10                                                     
         {
            if (coording(dt, test_site, track_length, con_state_pt, config_pt, f_mode_comm_pt, vehicle_info_pt, manager_cmd_pt) != 1)
                  fprintf(stderr, "\n Calling Coordination fail! \n");         
            else
            	maneuver_des = manager_cmd_pt-> man_des;
         }
     	else           // vehicle_pip > 1     
         {
            
            //if (vehicle_info_pt-> ready == 1)                                        //05/05/10
               //{
                 // maneuver_des = comm_receive_pt[1]. maneuver_id;
                  // maneuver_id[0] = maneuver_des;                        // Should be moved into maneuver()
            if (coording(dt, test_site, track_length, con_state_pt, config_pt, f_mode_comm_pt, vehicle_info_pt, manager_cmd_pt) != 1)
                 fprintf(stderr, "\n Calling Coordination fail! \n");  
            else       
                 maneuver_des = manager_cmd_pt-> man_des;                 // For temporary test at CRO
            if ( (comm_receive_pt[1]. maneuver_id == 29) || (comm_receive_pt[1]. maneuver_id == 30) )  // Over write the local coording when bring to stop
                     maneuver_des = comm_receive_pt[1]. maneuver_id;                               
         }    
    }       
  
        // To determine maneuver_id       
     if ( maneuver(dt, t_ctrl, time_filter, v_p, c, d, pitch_flt, config_pt, con_state_pt, vehicle_info_pt,  // time filter added on Dec. 1, 03
                 f_mode_comm, maneuver_des, maneuver_id) != 1 )
        fprintf(stderr, " Calling maneuver fail! \n");                

        // Moved here on 10_21_03
     if (time_filter > 0.0)
        {
           if (ref_dist(dt, time_filter, maneuver_id, vehicle_info_pt, f_index_pt, con_state_pt, config_pt) != 1)
              fprintf(stderr, " Calling ref_dist fail! \n");
        }
    
     if (local_time <= t_wait)
        con_state_pt-> temp_dist = con_state_pt-> front_range;

     if (vehicle_pip == 1)
        {
           pre_maneuver_id[0]=comm_receive_pt[vehicle_info_pt->pltn_size]. maneuver_des_1; // Update by communication
           pre_maneuver_id[1]=comm_receive_pt[vehicle_info_pt->pltn_size]. maneuver_des_2; // Update by communication
           //maneuver_id[0]=pre_maneuver_id[0];
        }
     if (vehicle_pip > 1)
        {
           pre_maneuver_id[0]=comm_receive_pt[vehicle_pip - 1]. maneuver_des_1; // Update by communication
           pre_maneuver_id[1]=comm_receive_pt[vehicle_pip - 1]. maneuver_des_2; // Update by communication
           //maneuver_id[0]=pre_maneuver_id[0];
        }
    
     // For all vehicles, over write maneuver_des here   // 10_21_03           
     if ( (maneuver_des == 29) && (con_state_pt-> pre_v < 1.2) ) 
         maneuver_id[0] = 30;
    
	/***************************************************/
	/*                                                 */
	/*           Close-Loop  Controllers               */
	/*                                                 */
	/***************************************************/
   
     if ( control( dt, time_filter, vehicle_pip, maneuver_id, 
	sens_read_pt, config_pt, con_state_pt, sw_pt, con_output_pt) != 1)
        fprintf(stderr, " Calling control fail! \n");  

	/*****************************/
	/*                           */
	/*  Activating  actuators    */
	/*                           */
	/*****************************/

actuate(pcmd, &engine_reference_torque, &vehicle_pip, &t_ctrl, &maneuver_des, 
	maneuver_id, con_output_pt, con_state_pt, &config, config_pt, pparams, 
	&minimum_torque, &inactive_ctrl);


	/************************************/
	/*                                  */
	/*     Update Communication data.   */
	/*                                  */
	/************************************/

if(config.use_comm == TRUE) {
      comm_send_pt.global_time = local_time;  // Each vehicle has a local time to broadcast.        
      comm_send_pt.acc_traj = con_state_pt-> pre_a; 
      comm_send_pt.vel_traj = con_state_pt-> pre_v;
      comm_send_pt.velocity = con_state_pt-> spd;
      comm_send_pt.accel = con_state_pt-> acc;

      comm_send_pt.my_pip = vehicle_pip;       // Determined in the beginning in handshaking.
      comm_send_pt.maneuver_id = maneuver_des;
      //comm_send_pt.fault_mode = manager_cmd_pt-> f_manage_index;
      comm_send_pt.fault_mode = vehicle_info_pt-> fault_mode;
      comm_send_pt.maneuver_des_1 = maneuver_id[0];
      comm_send_pt.maneuver_des_2 = maneuver_id[1];
//      memcpy(&comm_send_pt.gps, &self_gps_point, sizeof(path_gps_point_t);
      comm_send_pt.user_ushort_1 = comm_info_pt-> comm_reply;         // acknowledge receiving; 03_09_09      
      comm_send_pt.user_ushort_2 = (unsigned short) sw_pt-> auto_sw;  // acknowledge the vehicle is in automade; 11_20_09
      comm_send_pt.user_float=sens_read_pt-> bp; // Added on 11_25_09
      comm_send_pt.pltn_size = vehicle_info_pt->pltn_size;
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
        // save the difference as part of your logging, or use in updating
        // time since start of your program 
        prev = curr;

         
        /*******************************/
        /*                             */
        /*      Save data to buffer.   */
        /*                             */
        /*******************************/

if( (config.save_data == TRUE) && ((data_log_count++ % config.data_log) == 0) ) {
if( config.run_data == TRUE ) {
        fprintf(pout, "%6.4f %3i %3i %3i %3i %3i %4.3f %4.3f %4.3f %4.3f %4.3f ",  
             local_time,                         //1
             maneuver_id[0],                     //2             
             maneuver_des,                       //3
             handshake_start,                    //4
             vehicle_pip,                        //5              
             vehicle_info_pt->pltn_size,         //6
             global_time,                        //7
             comm_receive_pt[1]. global_time,    //8     
             comm_receive_pt[2]. global_time,    //9       
             time_filter,                        //10 
             t_ctrl                              //11
        );
        fprintf(pout, "%4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %3i %3i %3i ",
             con_state_pt-> vrd_range,           //12
             con_state_pt-> vrd_range_rate,      //13                        
             con_state_pt-> mdl_rg,              //14                 
             con_state_pt-> lidar_range,         //15
             con_state_pt-> radar_rg,            //16  
             con_state_pt-> radar_rt,            //17  
             con_state_pt-> front_range,         //18                             
             vehicle_info_pt-> fault_mode,       //19
             vehicle_info_pt-> pltn_fault_mode,  //20
             manager_cmd_pt-> f_manage_index     //21       
        );
        fprintf(pout, "%3i %3i %3i %3i %3i ",
             f_index_pt-> J_bus_1,	             //22
             comm_info_pt-> comm_reply,          //23
             f_index_pt-> comm_pre,              //24
             f_index_pt-> comm_back,             //25
             f_index_pt-> comm                   //26
        );                  
        fprintf(pout,"%4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f ",  
             con_output_pt-> y7,                //27   
             con_output_pt-> y8,                //28  
             con_output_pt-> y9,                //29  
             con_output_pt-> y10,               //30
             con_output_pt-> y11,               //31  
             con_output_pt-> y12,               //32  
             con_output_pt-> y13,               //33  
             con_output_pt-> y14,               //34    
             con_output_pt-> y15,               //35     
             con_output_pt-> y16                //36  
        );
        fprintf(pout, "%4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f ",               
             con_state_pt-> pre_v,              //37
             con_state_pt-> pre_a,              //38
             con_state_pt-> lead_v,             //39
             con_state_pt-> lead_a,             //40
             v,                                 //41            
             vehicle_info_pt-> run_dist,        //42
             manager_cmd_pt-> stop_dist,        //43          
             con_state_pt-> temp_dist,           //44  
             con_state_pt-> ref_v,              //45    
             con_state_pt-> ref_a,              //46 
             con_state_pt-> manu_speed,         //47
             con_state_pt-> manu_acc            //48                       
        );         
        fprintf(pout, "%3i %3i %3i %3i %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %3i %3i %3i ",
             sw_pt-> auto_sw,                    //49
             sw_pt-> manu_sw,                    //50
             read_sw,                            //51
             sens_read_pt-> gshift_sw,           //52 
             gear,                               //53 
             pcmd->ebs_deceleration,             //54  
             pv-> fb_axle,                       //55
             pv-> mb_axle,                       //56 
             pv-> rb_axle,                       //57 
             sens_read_pt-> bp,                  //58  
             sens_read_pt-> we_flt,              //59       
             vehicle_info_pt->ready,             //60
             comm_receive_pt[1].user_ushort_2,   //61
             comm_receive_pt[2].user_ushort_2    //62
 
        ); 
	fprintf(pout, "\n");
}
if( config.read_data == TRUE ) {
        fprintf(pout, "%4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f ",
             fl_axle_diff,               //1
             fr_axle_diff,               //2
             rl_axle_diff,               //3
             rr_axle_diff,               //4
             acl,                        //5
             we,                         //6
             v1,                         //7
             v2,                         //8
             v3,                         //9
             fan_spd_est,                //10                           
             eng_tq,                     //11
             nominal_fric_tq,            //12
             fuel_m1,                    //13
             fuel_m2,                    //14
             fuel_val1_pos,              //15
             fuel_val2_pos,              //16
             acc_pedal_pos               //17
        );     
             
        fprintf(pout, "%3i %3i %3i %3i %3i %3i %3i %3i %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f %4.3f\n",
             read_sw,                    //18
             eng_mode,                   //19
             jk_mode,                    //20
             ebs_brake_switch,           //21 
             abs_ebs_amber_warning_state,//22 
             sens_read_pt-> gshift_sw,   //23                                                  
             sens_read_pt-> lockup,      //24                                                             
             sens_read_pt-> driveline_engaged, //25                                                                   
             jk_tq,                      //26                                    
             jk_percent_tq,              //27
             inst_brk_power,             //28
             brk_pedal_pos,              //29
             brk_demand,                 //30
             trans_rtd_mode,             //31
             trans_rtd_value,            //32
             trans_rtd_volt,             //33
             sens_read_pt-> fuel_m,      //34  
             sens_read_pt-> grade,       //35
             sens_read_pt-> trans_out_we,//36
             sens_read_pt-> w_t,         //37
             sens_read_pt-> pm,          //38
             pv-> fb_axle,               //39
             pv-> mb_axle,               //40
             pv-> rb_axle,               //41
             local_time                  //42
   		);   
}
   		                              
    } 

    // Toggle watchdog timer. Hardware watchdog looks for rising edges, and so
    // sees every other toggle. HW timer activates red LED after 100 ms, so we
    // can miss at most 3 toggles if the loop interval is 20 ms.
    long_setled(pclt, TOGGLE_WATCHDOG);

  return 1;     
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
        float idle_torque = 400.0;      /* idle torque, param later? */
        float max_torque = pctrl->params.engine_reference_torque;
        float trq_range = max_torque - idle_torque;
        float vlow = 1.0;       /* active voltage lower and upper bounds */
        float vhigh = 3.0;
        float vrange = vhigh - vlow;
        float trq_val = engine_torque - idle_torque;
        if (trq_val < 0) trq_val = 0;
        return (vlow + (trq_val/trq_range) * vrange);
}
