/*******************************************************************************
 *
 *            FILE: long_vrd.c  -  For longitudinal control using Eaton Vorad radar only

 
              Based  on  new_con.c 
 *
 *            All time reading moved forward
 *            
              Each radar read differently
              The one signed to generic_radar_range is practically used
              Delco and Eaton Vorad working parallelly
              Eaton Vorad calibrated based on Delco measurement
 *
 *            Tested for two cars on at RFS                Dec. 5, 01  
 *
 *                                                                      By  XY_LU
                                                           Started on Nov. 5, 01
 * 
 *******************************************************************************
 * Copyright (c) 1996,1997   Regents of the University  of California
 *
 *      HISTORY
 *
 *      Reconstruction by separating maneuver part and fault detection part 
 *      from longitudinal controller part as subrutines; Further clearup; 
 *      Add protections to avoid going over bound problem in engine mappings.
 *      - by X.Y. Lu in June/July 1999
 *       
 *
 *      Overall cleanup and restructuring
 *      - by B. Bougler in March/April 1999
 *
 *      Fault management system to safely split cars from a faulty car
 *      and/or use emergency controller/maneuver in the faulty car.
 *      - developed and implemented by R. Rajamani and S. Choi in
 *      March/April 1997
 *
 *      Fault detection system that uses new communication software
 *      with handshaking and detects faults in radar, brake actuator 
 *      and throttle actuator.
 *      - developed and implemented by R. Rajamani and S. Choi in
 *      February/March 1997
 *
 *      Control software revamped and re-written for the San Diego 1997 demo.
 *      - by S. Choi and R. Rajamani in December 1996
 *
 *      Original split and join algorithms developed and implemented.
 *      - T. Connoly in 1995 (now deleted)
 *
 *      Control software modified for new architecture and for the LeSabre.
 *      - B. Bougler in 1995/1996
 *
 *      Control software implemented in the QNX operating system (eng2surf.c)
 *      - L. Chen in 1993 
 *
 *      Feedback of acceleration and velocity from lead and preceding cars
 *      for string stability of platoon.
 *      - concept developed by D. Swaroop and Prof. J.K. Hedrick in 1992
 *
 *      Observer to estimate inter-car spacing using the original
 *      O'Conner radar.
 *      - developed and implemented by S. Choi in 1993
 *
 *      Original engine control algorithms. 
 *      - developed and implemented by S. Choi in MS-DOS
 *      - Copyrighted in 1993
 */

#include <stdio.h>
#include <math.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

#include <local.h>
#include <sys_ini.h>
#include <sys_rt.h>
#include <sys_arma.h>
#include <sys_list.h>
#include <sys_buff.h>

#include "db_comm.h"
#include "db_clt.h"
#include "clt_vars.h"
#include "track.h"
#include "veh_iols.h"   /*      New vehicle IO for Le Sabre     */
#include "sabre1.h"     /*      New engine map for Le Sabre     */
#include "veh_iomb.h"


/*      Parameters for I15. For N11, values were: 1.0, 6.0 and 30.0     */

#define SPLITTING_DIST          7.0     /* Distance to split [m] */
#define DES_FOLLOW_DIST         10.0     /* Inter-car spacing [m] */
#define FALLBACK_TIME_PAR       46.0    /* Time parameter for car #8 to fall back to end of platoon [sec] */

#define STOP_BRAKE_PRESSURE     300.0   /* Brake pressure required when car is stopped [psi] */
#define MAN_TEMP_TO_PRESS       0.07267 /*      tmtopm  */
#define BRAKE_FLUID_COEFF       0.1670  /*      Cq      */

/*      Vehicle parameters.     */

#define MASS                    1701.0  /*      [kg]    */
#define I_ENG                   0.169
#define I_WHL                   2.603   /*      Lincoln */
#define VEHICLE_WHEEL_RADIUS    0.323   /*      [m]     */
#define VEHICLE_DRAG_COEFF      0.3693
#define VEHICLE_ROLL_FRICTION   0.0     /*      ?       */

#define MAX_DATA_LOG            30000
#define NUM_AIR_INDEX           15
#define NUM_AIR_VALS            14
#define MAX_DCC                 0.65


/*      Testing site location.  */

#define TEST_SITE               "TestSite"
#define RFS                     0
#define I15                     2
#define CRO                     3
#define N11                     6

/*      Types of fault.         */

#define NO_FAULT                0
#define THROTTLE_FAULT          3
#define BRAKE_FAULT             4
#define RADAR_FAULT             5
#define COMM_LEAD_FAULT         6       /* Lose communication with lead car */
#define CAN_FAULT_MAG           7       /* With magnetic observer working */
#define CAN_FAULT_SPD           8       /* With magnetic observer NOT working => speed control */
#define FAULT_STOP              9       /* Fault that requires to come to a complete stop */
#define COMM_PRECEEDING_FAULT   16      /* Lose communication with preceeding car */    

/*      Types of maneuver.      */

#define INITIAL                 1                       
#define READY                   2
#define ACCEL                   3
#define CRUISE                  4
#define SPLIT                   5
#define SPLIT_DONE              6
#define CHANGE_LANE             7
#define CHGL_DONE               8
#define JOIN                    9
#define JOIN_DONE               10
#define CHANGE_LANE_BACK        17
#define CHGL2_DONE              18
#define REJOIN                  19
#define REJOIN_DONE             20      
#define DECEL                   29
#define STOP                    30 

/*      Controller configuration parameters that the user can modify.   */

#define DATA_LOG_INTERVAL       "DataLog"       
#define DELCO_RADAR             "DelcoRadar"
#define END_SCENARIO            "EndScenario"
#define STATIC_RUN              "StaticRun"
#define TEST_ACTUATORS          "TestActuators"
#define STEP_START_TIME         "StepStartTime"
#define STEP_END_TIME           "StepEndTime"
#define THROT_DOWN              "ThrotDown"
#define THROT_UP                "ThrotUp"
#define BRAKE_DOWN              "BrakeDown"
#define BRAKE_UP                "BrakeUp"
#define MAX_SPD                 "MaxSpeed"

/*      Function definitions.   */




/*      Structure for the controller configuration.     */

typedef struct
{
        int data_log;           /* Data log interval.                           */                                                              
        bool_typ delco_radar;   /* Delco (1) or O'Conner (0) radar.             */
        bool_typ end_scenario;  /* Change lane back and rejoin (1) or not (0).  */
        bool_typ static_run;    /* Static (1) or dynamic (0) run.               */
        bool_typ test_actuators;/* Test throttle and brake actuators (1) or not (0).    */
        float step_start_time;  /* Time to begin the step. [sec]                */
        float step_end_time;    /* Time to end the step. [sec]                  */
        float throt_down;       /* Value before and after the step. [deg]       */
        float throt_up;         /* Throttle value during the step. [deg]        */
        float brake_down;       /* Value before and after the step. [psi]       */
        float brake_up;         /* Brake value during the step. [psi]           */
        float max_spd;
} control_config_typ;

/*      Structure for the maneuver configuration.     */

typedef struct
{
        int splitting;
        int split_complete;
        int double_splitting;
        int joining;
        float initial_join_distance;    
        int join_complete;
        int rejoining;
        int rejoin_complete;
        int behind_joining;
        int behind_splitting;
        int lane_changing;
        int lane_change_complete;
        int preceeding_lane_changing;
        int lane_change_back;
        int lane_change_back_complete;   
} maneuver_config_typ;

extern bool_typ verbose_flag;           /* Used to save data in test.dat */
static control_config_typ config;
static control_config_typ* config_pt;
static maneuver_config_typ* m_state_pt;
static maneuver_config_typ m_state;

static double time_control=0.0;             /* Set in spd_ctrl. [sec] */
static unsigned long tick=0;              /* Set in spd_ctrl. [sample ticks] */
static unsigned long tick_filter=0;       /* Set in spd_ctrl. [sample ticks for filtering] */
static long data_log_count=0;

static int test_site;
static buff_typ *pbuff;                 /* Buffer pointer */
db_clt_typ *pclt;                       /* Database client pointer */
db_data_typ db_data;
evt300_radar_typ evt300_radar1;		/// updated for improved driver
evt300_radar_typ *pvor_radar;

static float track_length=0.0;
static float stop_dist=0.0;            // After this point, begin to stop
static float marker_spacing=0.0;



/*      spd_init()
 *
 *      This routine is called when the controller first starts. It
 *      countains run-time initializations for the brake and throttle
 *      controller configurations. 
 *      The AVCS system uses DOS/Windows style configuration files
 *      which uses sections containing values relevant to the application.
 *      The filename will default to "realtime.ini" and the section name
 *      to the controller program name, here "d99_long".
 */

bool_typ spd_init( char *pfile, char *psect, char *argv[] )
{
        FILE *pfin1, *pfin2;
        char hostname[MAXHOSTNAMELEN+1];

        time_control = 0.0;
        tick = 0L;
        tick_filter = 0L;

        if( psect == NULL )
                return( FALSE );

        if( (pfin1 = get_ini_section( pfile, psect )) == NULL )
                return( FALSE );

        /* Read controller initialization values from Windows style
         * "realtime.ini" file. If the desired variable name is found,
         * the value from the file (given by the user) is returned. If
         * the name is not found, the default value (last argument in the
         * get_ini_() call) is returned.
         */

        config.data_log = (int)get_ini_long( pfin1, DATA_LOG_INTERVAL, 5L );
        config.delco_radar = get_ini_bool( pfin1, DELCO_RADAR, TRUE );
        config.end_scenario = get_ini_bool( pfin1, END_SCENARIO, TRUE );
        config.static_run = get_ini_bool( pfin1, STATIC_RUN, FALSE );
        config.test_actuators = get_ini_bool( pfin1, TEST_ACTUATORS, FALSE );
        config.step_start_time = (float)get_ini_double( pfin1, STEP_START_TIME, 2.5 );
        config.step_end_time = (float)get_ini_double( pfin1, STEP_END_TIME, 5.0 );
        config.throt_down = (float)get_ini_double( pfin1, THROT_DOWN, 0.0 );
        config.throt_up = (float)get_ini_double( pfin1, THROT_UP, 5.0 );
        config.brake_down = (float)get_ini_double( pfin1, BRAKE_DOWN, 0.0 );
        config.brake_up = (float)get_ini_double( pfin1, BRAKE_UP, 100.0 );
        config.max_spd = (float)get_ini_double( pfin1, MAX_SPD, 25.0 );

        fclose( pfin1 );
        config_pt = &config;


        /* Read testing site location from realtime.ini. */

        if( (pfin2 = get_ini_section( pfile, "site_section" )) == NULL )
                return( FALSE );
        test_site = (int)get_ini_long( pfin2, TEST_SITE, 1L );
        fclose( pfin2 );

        data_log_count = 0;

        if( (pbuff = buff_init( stdout, MAX_DATA_LOG )) == NULL )
                return( FALSE );

        /* Log in to the database (shared global memory). Default to the
         * the current host.
         */
#ifdef __QNX4__
        sprintf( hostname, "%lu", getnid() );
#else
	get_local_host(hostname, MAXHOSTNAMELEN);
#endif

        if (( pclt = clt_login( argv[0], hostname, DEFAULT_SERVICE,
                COMM_OS_XPORT)) == NULL )
        {
                fprintf( stderr, "Database initialization error in d99_long\n");
                return( FALSE );
        }

        #ifdef RECONSTRUCT
                config.data_log = 1;
                fprintf( stderr, "Saving a RECONSTRUCTIVE run with 30 column array.\n");
        #endif

   if (test_site == RFS)
       {
          track_length = 280.0;      //   [m]
          if (config.max_spd > 25.0)
             config.max_spd = 25.0;  //   [mph]
       }  
   else if( test_site == CRO )
       { 
          
          marker_spacing = 1.2;
          track_length = 2300.0;       
          if (config.max_spd > 70.0)
             config.max_spd=70.0;
       }
    else if( test_site == I15 )
       {
          marker_spacing = 1.2;
          track_length = 14000.0;
          if (config.max_spd > 60.0)
             config.max_spd=70.0;
       }
    else if( test_site == N11 )
       {
          marker_spacing = 1.25;
          track_length = 14000.0;
          if (config.max_spd > 60.0)
             config.max_spd=70.0;
       }
    else;  
       stop_dist = track_length - 0.5*config.max_spd*1609*1609*config.max_spd/(MAX_DCC*3600*3600);      // [m]

     
        
        return( TRUE );
}


/*      spd_done()
 *
 *      This routine is provided to clean up system resources allocated
 *      by the spd_init() or spd_ctrl() when the controller is
 *      exiting. The buff_done() call flushes data to a file.
 */

bool_typ spd_done()
{
        /* Log out from the database. */
        if( pclt != NULL )
                clt_logout( pclt );

        buff_done( pbuff );
        return( TRUE );
}

          

/*      spd_ctrl()
 *
 *      This routine is periodically called by the AVCS control system.
 *      This will occur when new vehicle data is available, which is
 *      generally at 20 msec.
 *      This routine uses relevant parameters from the pinput structure
 *      (measured input variables from the vehicle, described in "veh_iols.h")
 *      and ptrack structure (variables from the desired tracking profile,
 *      described in "track.h"), and sets the desired elements of the poutput
 *      structure (described in "veh_iols.h").
 */

bool_typ spd_ctrl( long_input_typ *pinput,
        track_control_typ *ptrack, long_output_typ *poutput )
{

       static int platoon_size = 8, car_number_initial = 1;
       static int maneuver_id = 0, maneuver_des = 1;
              
       static float time_filter = 0.0;
       static float dt=0.0;                   
       static float t_ctrl=0.0;                
       static float global_time = 0.0;     
       
       
       static float distance, split_dist = 0.0, temp_dist = DES_FOLLOW_DIST;
       static float mag_distance, preceeding_mag_distance;
       static float v, lead_speed=0.0,  preceeding_speed=0.0;               
       static float lead_accel=0.0, acl, preceeding_accel=0.0;
             
       static float we, wr, pm;                                 
       static float alp, alp_des, alp_des_old, alp_desd;   
                    
       static float time_fallback = 0.0, fallback_amp, time_warning=0.0;
       static int warning_init = 0;  

       static int fault_mode = 0, fault_init = 0;
       static int fault_mode_global = 0;     
       static int mag_fault = 0;
       static int faulty_car_global = 0;
       static int join_init=0;
                      
       float auxillary;                                    
       float distance_magnets;                        
       float vel_traj, acc_traj;                                                       
       float generic_radar_range, generic_radar_rangerate;
       float range_obs, relvel_obs;                                                               
       static float preceeding_mag_old;     
       int marker_counter,marker_number,preceeding_marker_number;
                  

       static float wr_raw, wr_dum;
       static float rg, r_star; 
       static float r_starh;                     
       static float eff_in;
       static float beta;                  
       static float eps_dot = 0.0, eps = 0.0;    
       static float initial_dist = DES_FOLLOW_DIST;
   
       static float distance_old = DES_FOLLOW_DIST;
       static float gear, gear_old, tg;          
                 
       static int stay;                       
       static int rfs_goodal = 0;       

       static float pm_des,pmdesd;
       static float pb, pb_des, pb_des_old = 0.0, pb_temp;
       static float tnet, tnet_des, tnet_p;    
       static float tnet_min = 0.0;   /* Le Sabre has no engine brake on Drive */
       static float tnet_a, tnet_b;       
       static float ma, ma_old, mades, mades_old, madesd; 
       static float maod, maod_old; 
       static float temp, temp1;
       static float usyn, usyn_old;         
       static float we_old, pm_old;
       static float beeper_time = 0.0;
       static float pm_flt, wr_flt, we_flt, pb_flt, gear_flt;

       static float buf;
       static float abrk_des;                 
       static float alp_gain; 
 
       float v_fault=0.0, acc_fault=0.0;
       float omega=0.7, omega_time=0.0;
       static int comm_counter = 0;    /* For output and communication only */
       static float stemp_obs = 0.0;
       static float etemp_obs = 0.0;      // Passing var. in fault_management()

        /*--- Vehicle parameters. ---*/
     
       static int  car_id =0;                /* Initialization */
       const float  tman = 301.0;           /* 301 in Kelvin = 82 F */
       const float  hr = 0.323;             /* Wheel radius */
       const float  c_a = 0.3693;           /* Air drag coefficient */
       const float  f_f = 180.0;            /* 150 before */
       const float  f_b = 1.2;
       const float  rd = 1.0/3.06;
       const float  tmtopm = 0.06421;       /* Parameter for 2 surface control law */
       
       static int brake_sw = 0, brake_sw_old;                   
       static int adap_a, adap_b;


       /*** For Eaton Vorad ***/

       static short int tgt1_id=0, tgt2_id=0, tgt1_lock=0, tgt2_lock=0;  //This should be filtered in the main part.
       static float tgt1_rg=0.0, tgt1_rt=0.0, tgt1_az=0.0, tgt1_mg=0.0 ;
       static float tgt2_rg=0.0, tgt2_rt=0.0, tgt2_az=0.0, tgt2_mg=0.0 ;
       static float tgt1_rg_flt=0.0, tgt1_rt_flt=0.0, tgt1_az_flt=0.0, tgt1_mg_flt=0.0 ;
       
      // static float tgt1_rg_old=0.0, tgt1_rt_old=0.0, tgt1_az_old=0.0, tgt1_mg_old=0.0 ;
     //  static float tgt2_rg_old=0.0, tgt2_rt_old=0.0, tgt2_az_old=0.0, tgt2_mg_old=0.0 ;
       
     //  static float tgt2_rg_flt=0.0, tgt2_rt_flt=0.0, tgt2_az_flt=0.0, tgt2_mg_flt=0.0 ;
     //  float tgt1_rt_d=0.0, tgt1_rg_d=0.0, tgt2_rt_d=0.0;
         float tgt1_rg_raw=0.0, tgt1_rt_raw=0.0;
     //  static float evrd_start_t=0.0;
      // static int evrd_start_s=1, evrd_start_s1=1;

       /*** Other Radars  ***/
       float delco_rg=0.0, delco_rt=0.0, oconner_rg=0.0, oconner_rt=0.0; 
     

      /**************************************************************************************/
                         
                          /*------ Design Parameters and variables ------*/

      /**************************************************************************************/
    
       static float s_1=0.0, s_2=0.0, s_3=0.0;
    /*   static float flt1a[2], flt3a[2],flt4a[2],flt01a,flt02a;
       float flt1b[2], flt3b[2], flt4b[2],flt01b,flt02b;             */
       
       float temp5=0.0, temp5a, temp5b;  
   /*    static float x1r=0.0, x1r_old=0.0, x1r_d=0.0;  
       static float x2r=0.0, x2r_old=0.0, x2r_d=0.0, x3r=0.0;
       static float x2r_flt_old;   */
       
       static float f_f_hat=250.0, f_b_hat=1.0, s_1_d_old=0.0, s_1_dd=0.0, s_1_old=0.0;
       static float s_1_d=0.0, ff_gain = 1.2, ff_gain_old=2.0;
       static float s_1_flt=0.0, s_1_d_flt=0.0, temp3=0.0, tnet_b_old=0.0, tnet_b_flt=0.0; 
  
       static float k1=0.0, k2=0.0, k3=0.0;
      // static float k01=0.0, k02=0.1;
     //  const float epsilon1=0.05, epsilon2=0.05;
    //   const float Tau1=0.01,  Tau2=0.015;                     //Tau1=0.01, Tau2=0.014;            

       static float ff_t=0.0, slid=0.0, slid_old=0.0, slid_flt=0.0, slid_dot=0.0;
       static float slid1=0.0, slid1_old=0.0, slid1_dot=0.0; 
       
       static float K[3]={0.0,0.0,0.0}; 
       float int_dist=0.0, dp=0.0;       
       static float ref_speed=0.0;
       static int gain_s=0;


       static float pm_temp=0.0, usyn1=0.0, slid2=0.0, slid2_old=0.0, slid2_d=0.0;
       static float run_dist=0.0;             
                    
/********************************************/

#ifndef OFFICE
        static float v1, v2, v3, v4, v5, v6, vel_10, vel_raw;
        static float oconner_rg_old = DES_FOLLOW_DIST;
#endif
        
/*********************************************/
        
        char print_buff[MAX_LINE_LEN+1];
        
        db_data_typ db_data;                    /* Retrieve data from database */
        fault_status_typ fault_msg_to_boon;
        marker_pos_typ *pmarker_pos;            /* Marker position data structure pointer */
        fault_status_typ *pfault_message;       /* Fault message data structure pointer */
        
      
  

           
     /*---Function declaration---*/
     
  
       void fault_detect(long_input_typ *, float, float, int, float, int *); 
       void fault_manage(long_input_typ *, track_control_typ *, int , float,
                          float, float, float, int *,float *, float *, float *,
                          float *, float *);                  
       static float eng_tbl(float, float, int , int);     
       static float gear_tbl(float);  
       void fault_diag(int, int, int, int, int, int, int, float, float,
                        long_input_typ*, control_config_typ,
                        maneuver_config_typ , int*, long_output_typ*);
       void car_ref2( float, float, int, int, float, float, float *, float *, int *, int*);             
       void scenario(int, int, int, int, int,  float, float, float, float, float, 
                     long_input_typ*, control_config_typ, track_control_typ*,
                     maneuver_config_typ*, int*, float*, float*,
                     float*, float*, float*, long_output_typ *);             
       void distance_observer( float, float, float, maneuver_config_typ ,
                        float, float *, int *, float *);
       void vrd_flt(float, float, float, float, float, float *, float *, float *, float *);
       float usyn_filter(float);
       float svg3(float);
       float svg5(float);
       float svg3a(float);
       float svg5a(float);
       float svg3b(float);
       float svg5b(float);
       float svg7(float);
       float svg7a(float);
       float svg7b(float);
       float svg9(float);
       float svg9a(float);
       float svg9b(float);
       float svg11(float);
       float rg_filter(float );
       float rt_filter(float );
       float sig_d_filter(float );   
       
       void gain(float, float, float, int, float, float, float *);
       void smooth(float, float, float, float, float, float, float *);                
       float SAT(float,float);
       float SIGN(float);  
       float SINSAT(float,float);

     /**********************************************************************************

                                          Intialization
 
     **********************************************************************************/

  
    if ( time_control < 3.0*pinput->interval )
       {   
           m_state. splitting=0;
           m_state. split_complete=0;
           m_state. double_splitting=0;
           m_state. joining=0;
           m_state. initial_join_distance=DES_FOLLOW_DIST;  
           m_state. join_complete=0;
           m_state. rejoining=0;
           m_state. rejoin_complete=0;
           m_state. behind_joining=0;
           m_state. behind_splitting=0;
           m_state. lane_changing=0;
           m_state. lane_change_complete=0;
           m_state. preceeding_lane_changing=0;
           m_state. lane_change_back=0;
           m_state. lane_change_back_complete=0;                
       }
    m_state_pt = &m_state;                                             

        /***************************************/
        /*      Select the kind of radar.      */
        /***************************************/

#ifdef OFFICE
        generic_radar_range = pinput->radar_data.radar_range;
        generic_radar_rangerate = pinput->radar_data.radar_rangerate;
#else
        if( config.delco_radar == TRUE )
        {
                delco_rg = pinput->radar_data.radar_range;
                delco_rt = pinput->radar_data.radar_rangerate;
        }
        else
        {
                oconner_rg = pinput->oconner;
                oconner_rt = (pinput->oconner - oconner_rg_old)/pinput->interval;
                oconner_rg_old = pinput->oconner;
        }
#endif


        /*************************************************/
        /*      Re-definition of measured variables.     */
        /*      Move forward on Dec. 03 01               */
        /*************************************************/

        /* Start counting when the throttle switch is on and when the car is moving */
        if( (pinput->actuator_sw == TRUE) && (maneuver_id != 0) && (maneuver_id != 10) )
        {
                tick_filter++;
                rfs_goodal = 1;
        }

        else if (rfs_goodal == 1)
                tick_filter++;

        time_filter = tick_filter * pinput->interval;                             // button mus be ON

      /* Start counting when the throttle switch is on or in case of a static run */
        if( (pinput->actuator_sw == TRUE) || (config.static_run == TRUE) ) 
                tick++;
        time_control = tick * pinput->interval;                                   // button mus be ON

        t_ctrl = time_control;
        dt = pinput->interval;         
        we = pinput->eng_rpm;
        pm = pinput->eng_press;
        alp = pinput->eng_throt;
        pb = pinput->brake_press;
        acl = pinput->long_acc;

      /*--- Eaton Vorad radar data ---*/
	
	db_clt_read(pclt, DB_EVT300_RADAR1_VAR, sizeof(evt300_radar_typ),
			&evt300_radar1);
             
        pvor_radar= &evt300_radar;

        tgt1_id = pvor_radar->target[0].id;
        tgt1_rg = 0.03048*pvor_radar->target[0].range*1.2;            //[m], 1.2  added on Dec. 5, 01
        tgt1_rt = 0.03048*pvor_radar->target[0].rate;                 //[m/s]
        tgt1_az = 0.002*pvor_radar->target[0].azimuth;                //[rad]
        tgt1_mg = -0.543*pvor_radar->target[0].mag;                   //[dB]
        tgt1_lock = (int)(pvor_radar->target[0].lock/255);            //1 = locked, 0 = not locked
        tgt1_rt_raw=tgt1_rt;                                         // For output only
        tgt1_rg_raw=tgt1_rg;                                         // For output only  

        tgt2_id = pvor_radar->target[1].id;
        tgt2_rg = 0.03048*pvor_radar->target[1].range*1.2;
        tgt2_rt = 0.03048*pvor_radar->target[1].rate; 
        tgt2_az = 0.002*pvor_radar->target[1].azimuth;
        tgt2_mg = -0.543*pvor_radar->target[1].mag;
        tgt2_lock = (int)(pvor_radar->target[1].lock/255);

 /*       // Target 1
    
    if ( ((fabs(tgt1_rg) < 1.5)) && (evrd_start_s == 1))
       {
             tgt1_rg_old = tgt1_rg;
             tgt1_rg_flt = tgt1_rg;             
             tgt1_rt_old = tgt1_rt;             
             tgt1_rt_flt = tgt1_rt;
       }
    else if ( (fabs(tgt1_rg) >= 1.5) && (evrd_start_t <0.1) && (evrd_start_s1 == 1) )
       {
             tgt1_rg_old = tgt1_rg;
             tgt1_rg_flt = tgt1_rg;             
             tgt1_rt_old = tgt1_rt;             
             tgt1_rt_flt = tgt1_rt;
             evrd_start_t += dt;
             evrd_start_s=0;            
       }
    else
       {
             evrd_start_s=0;
             evrd_start_s1=0;
        tgt1_rg_d=(tgt1_rg - tgt1_rg_old)/dt;
     
        if (fabs(tgt1_rg_d)>350.0 || fabs(tgt1_rt)<0.001)  // or 400.0
            tgt1_rg = tgt1_rg_old;
        else
           {
               if (tgt1_rg > tgt1_rg_old+20.0*dt)
                 tgt1_rg = tgt1_rg_old+20.0*dt;
               if (tgt1_rg < tgt1_rg_old-20.0*dt)
                 tgt1_rg = tgt1_rg_old-20.0*dt;

           }
        
        if (tgt1_rt > tgt1_rt_old+3.0*dt)
           tgt1_rt = tgt1_rt_old+3.0*dt;
        if (tgt1_rt < tgt1_rt_old-3.0*dt)
           tgt1_rt = tgt1_rt_old-3.0*dt;
        tgt1_rt_old = tgt1_rt;
  
        tgt1_rg_old = tgt1_rg;
        
            
        tgt1_rg_flt = 0.1*tgt1_rg+0.9*tgt1_rg_flt;
        tgt1_rt_flt = 0.1*tgt1_rt+0.9*tgt1_rt_flt;
       // tgt1_rg_flt = rg_filter(tgt1_rg_flt);
       // tgt1_rt_flt = rg_filter(tgt1_rt_flt);
       }   */

        vrd_flt(dt,tgt1_rg,tgt1_rt,tgt1_az,tgt1_mg,&tgt1_rg_flt,&tgt1_rt_flt,&tgt1_az_flt,&tgt1_mg_flt);

        // Using Eaton Vorad      Based on global_time
        
        if (time_filter < 3.0)
            generic_radar_range = DES_FOLLOW_DIST;
        else if (time_filter <= 9.0)
            generic_radar_range = (DES_FOLLOW_DIST) + (tgt1_rg_flt - DES_FOLLOW_DIST)*(time_filter-3.0)/6.0;
        else                                                                               // Dec 5, 01 
           generic_radar_range = tgt1_rg_flt;             
        generic_radar_rangerate = tgt1_rt_flt; 



         
        /******************************************/
        /*      Read or update the database.      */
        /******************************************/
  
                              
        /* Update the fault feedback for communication between cars */
        fault_msg_to_boon.fault_id = fault_mode;
        fault_msg_to_boon.car_id = pinput->mycar_id;
        if( clt_update( pclt, DB_FAULT_FEEDBACK_VAR, DB_FAULT_FEEDBACK_TYPE, \
                sizeof( fault_status_typ ), (void *) &fault_msg_to_boon) == FALSE)
                perror( "clt_update( DB_FAULT_FEEDBACK_VAR ) in d99_long\n");

        /* Read the fault status of whole platoon */
        if ( clt_read( pclt, DB_FAULT_STATUS_VAR, DB_FAULT_STATUS_TYPE,
                       &db_data ) == FALSE )
            fprintf( stderr, "clt_read( DB_FAULT_STATUS_VAR ) in d99_long\n");
        pfault_message = (fault_status_typ *) db_data.value.user;
        platoon_size = pfault_message->platoon_size;
        fault_mode_global = pfault_message->fault_id;
        faulty_car_global = pfault_message->car_id;

        /* Read lateral inputs */
        if ( clt_read( pclt, DB_MARKER_POS_VAR, DB_MARKER_POS_TYPE,
                &db_data ) == FALSE )
                fprintf( stderr, "clt_read( DB_MARKER_POS_VAR ) in d99_long\n");

        pmarker_pos = (marker_pos_typ *) db_data.value.user;
        marker_number = pmarker_pos->marker_number;
        marker_counter = pmarker_pos->marker_counter;
        mag_distance = pmarker_pos->timestamp;
 
        /* Read information from preceeding vehicle */ 
        preceeding_marker_number = ptrack->preceeding.long_usr5;
        preceeding_mag_distance = ptrack->preceeding.distance;

        /* Prefiltering of information from previous car */
        if( test_site == RFS )
                marker_spacing = 1.0;
        else if( (test_site == CRO) || (test_site == I15) )
                marker_spacing = 1.2;
        else if( test_site == N11 )
                marker_spacing = 1.25;
        if( time_filter < 0.1 )
                preceeding_mag_old = preceeding_mag_distance;
        if( (preceeding_mag_distance - preceeding_mag_old) > marker_spacing ) 
                preceeding_mag_distance = preceeding_mag_old + marker_spacing;
        else if ( (preceeding_mag_distance - preceeding_mag_old) < -marker_spacing )
                preceeding_mag_distance = preceeding_mag_old - marker_spacing;

        preceeding_mag_old = preceeding_mag_distance;


        car_id = pinput->platoon_pos;           // Read in car_id

                  /*------Call fault detect subrutines------*/                          

   if( fault_init != 1 )
       fault_mode = 0;
       
   if( (time_control > 1.0) && (config.static_run == FALSE) && (maneuver_des != 0) )
       fault_detect(pinput,pb_des,alp_des,car_id,generic_radar_range,&fault_mode);

   fault_diag(car_id, maneuver_id, fault_init, mag_fault, fault_mode_global,
              faulty_car_global, maneuver_des, time_filter, generic_radar_range,
              pinput, config, m_state, &fault_mode, poutput);

   
        /**************************************************/
        /*      Detect if the PCM is working properly.    */
        /**************************************************/

        if( ( (pinput->gear > 1) && ( v < 15.0*1.6/3.6 ) )
                || ( (pinput->gear > 2) && (v < 32.0*1.6/3.6 ) )
                || ( (pinput->gear > 3) && (v < 67.0*1.6/3.6 ) ) )
        {
                if( warning_init == 0 )
                {
                        fprintf(stderr, "WARNING : THE PCM IS NOT WORKING CORRECTLY\n");
                        warning_init = 1;
                }
                time_warning += pinput->interval;
                /* Turn the beeper on for 5 sec */
                if( time_warning < 5.0 ) 
                        poutput->beeper = 1;
                else 
                        poutput->beeper = 0;
        }
        if (time_warning >= 5.0)
            time_warning = 0.0;                                       //Reset by XY_LU
  
      
          /*************************************************************/
        /*      Set the desired maneuver and the feedback maneuver   */
        /*      depending on maneuver ID.                            */
        /*************************************************************/

      
        /* Definition of maneuver ID:
         *      0 : stay at rest
         *      1 : accelerate to desired speed using specified profile
         *      2 : constant speed following (cruising)
         *      3 : have been cruising for more than 5 seconds
         *      9 : decelerate to desired speed using specified profile
         *      10 : brake to stop
         */

        if ( (car_id == 0) || (car_id == 1) )
        {
                 /* Speed profile based on time for RFS */
                if( test_site == RFS )
                {
                        if ( (run_dist > stop_dist) || (maneuver_id == 10) )                      
                                pinput->maneuver_des = 29;
                }

                if( pinput->maneuver_des < 50 )
                        maneuver_des = pinput->maneuver_des;

                /* Set desired speed, acceleration and maneuver ID */
             car_ref2( pinput->interval, time_control, maneuver_des, test_site, 
                  config_pt->max_spd, MAX_DCC,&vel_traj, &acc_traj, &maneuver_id, &gain_s);
            
               
        }
        else 
        {
                /* If car #2 or more, get maneuver ID through communication with lead car */
                maneuver_id = ptrack->leader.long_usr6;
                if( (maneuver_des == 0) || (maneuver_des == 1) )  
                        maneuver_id = 0;
                if( pinput->maneuver_des < 50 )
                        maneuver_des = pinput->maneuver_des;
        }

        /* Set the maneuver feedback:
         *      2 : staying at rest, ready to move
         *      4 : cruising
         *      30 : have come to a stop
         */
        poutput->maneuver_feedback = pinput->maneuver_des;
        if( maneuver_des == 1 ) 
        {
                if( (pinput->actuator_sw == 1) || (config.static_run == TRUE) ) 
                        poutput->maneuver_feedback = 2;
        }
        else if( (maneuver_des == 3) && (maneuver_id == 3) )
                poutput->maneuver_feedback = 4;
        else if( (maneuver_des == 29) && (maneuver_id == 10) ) 
                poutput->maneuver_feedback = 30;
        

      scenario(car_id, maneuver_des, fault_mode_global, faulty_car_global, join_init,
               time_filter, vel_traj, acc_traj, t_ctrl, generic_radar_range, pinput, 
               config, ptrack, m_state_pt, &car_number_initial, &lead_speed, 
               &lead_accel, &split_dist, &preceeding_speed, &preceeding_accel, poutput);


   /*************************************************************************
   **************************************************************************

                         LONGITUDINAL CONTROLLER 

  ***************************************************************************
  *************************************************************************/

        

        /************************************************************/
        /*      Measure the 6 wheel speeds and process to obtain    */
        /*      the speed of the car.                               */
        /************************************************************/

        #ifdef OFFICE
                wr = pinput->speed1;
        #else
                v1 = pinput->speed1;
                v2 = pinput->speed2;
                v3 = pinput->speed3;
                v4 = pinput->speed4;
                v5 = pinput->speed5;
                v6 = pinput->speed6;
                if (v1 >= 39.5)
                        v1 = 0.0;
                if (v2 >= 39.5)
                        v2 = 0.0;
                if (v3 >= 39.5)
                        v3 = 0.0;
                if (v4 >= 39.5)
                        v4 = 0.0;
                if (v5 >= 39.5)
                        v5 = 0.0;
                if (v6 >= 39.5)
                        v6 = 0.0;

                /* Wheel velocity fusion */
                vel_10 = (v1 + v2 + v3 +  v4)/4.0;
                vel_raw = (v5 + v6)/2.0;
                if( vel_10 >= 7.0 )
                        wr = vel_10;
                else if( vel_10 >= 4.0 )
                        wr = vel_10*(vel_10 - 4.0)/3.0 + vel_raw*(7.0 - vel_10)/3.0;
                else
                        wr = vel_raw;
        #endif

//      if( wr < 0.4 )
//              wr = 0.0;

        if( (maneuver_id == 0) || (maneuver_id == 10) )
        {
                wr_dum = wr;
                wr = 0.0;
                stay = 1;
        }
        if( (maneuver_id == 1) && (stay == 1) )
        {
                if( wr == wr_dum )
                        wr = 0.0;
                else
                        stay = 0;
        }

        /* Filter the speed */
        if( time_filter < 0.03 )
        {
               wr_raw = wr;
               wr_flt = wr;
        }
        else
        {
                if( wr - wr_raw > 0.1 )
                        wr_raw += 0.1;
                else if(wr - wr_raw < -0.1 )
                        wr_raw += -0.1;
                else
                        wr_raw = wr;
                if( maneuver_id != 1 )
                        wr_flt = 0.3*wr_raw+0.7*wr_flt;
                else
                        wr_flt = 1.0*wr_raw+0.0*wr_flt;
        }
        v = wr_flt;     /* pre-filtered and LPF */

        run_dist += v*dt;

        /* Control starting routine initialization: begin and end */
   /*     if( time_filter < 0.07 )
                trans = 1;
        else if( time_filter > 5.0 )
                trans = 0;                         Removed by XY_LU */

        if( preceeding_speed <= 0.0 )
             preceeding_speed = 0.0;                 


        /*****************************************************************/
        /*      Calculate time after gear change begins and gear ratio   */
        /*****************************************************************/

        gear = pinput->gear;

        if( time_filter < 0.07 )
        {
              gear_old = gear;
              gear_flt = gear;
        }
        if( (gear != gear_old) || (time_filter < 0.07) )
                tg = 0.0;       /* Elapsed time after gear shifting */
        else
                tg = tg + dt;
        if( (maneuver_id == 0) || (maneuver_id == 10) )
                tg = 0.0;
        if( time_filter >= 0.07 )
                gear_old = gear;

        if( (tg < 1.2) && (time_filter > 1.2) && (gear_flt < 1.5) )
              ;
        else if( (tg < 0.6) && (time_filter > 0.6) && (gear_flt < 2.5) )
              ;
        else if( (tg < 0.6) && (time_filter > 0.6) && (gear_flt < 3.5) )
              ;
        else
              gear_flt = 0.2*gear + 0.8*gear_flt;
        rg = gear_tbl( gear_flt );

        r_star = rg*rd;
        r_starh = r_star*hr;
        eff_in = (MASS*hr*hr+I_WHL)*r_star*r_star+I_ENG;
        beta = eff_in/r_starh;
 

        /*******************************************************************/
        /*      Measure radar signal, use observer to calculate estimated  */
        /*      range and range rate.                                      */
        /*******************************************************************/

        if ( (car_id == 0) || (car_id == 1) ) 
        {       
                /* If platoon single (car #0) or platoon lead (car #1),
                 * harcode the distance. */
                distance = DES_FOLLOW_DIST;
                eps_dot = v - preceeding_speed;
                /* Calculate spacing error only when car is moving */
                if( (maneuver_id != 0) && (maneuver_id != 10) )
                        eps += eps_dot * pinput->interval;
                temp_dist = DES_FOLLOW_DIST;
        
                if( maneuver_des == 5 ) 
                        temp_dist += - split_dist;
                else if( maneuver_des == 9 ) 
                        temp_dist = m_state. initial_join_distance + split_dist;
                if( (m_state. split_complete == 1) && (maneuver_des != 5) && (join_init == 0) )
                        temp_dist += -split_dist;
//              if( (join_complete == 1) && (maneuver_des != 9) )
//                      temp_dist += split_dist;

                distance = DES_FOLLOW_DIST - eps;
                distance_observer(v,preceeding_speed,pinput->interval, m_state,
                        distance,&range_obs,&mag_fault,&auxillary);

                distance_magnets = range_obs + 0.15;
               
        }
        else
        {
                /* If car #2 or more, get distance through radar reading */
                distance = generic_radar_range;
                if( time_filter < 0.1 ) 
                        distance_old = distance;
                if( (distance - distance_old) > 0.5) 
                        distance = distance_old + 0.5;
                else if( (distance - distance_old) < -0.5) 
                        distance = distance_old - 0.5;
                distance_old = distance;

                distance_observer(v, preceeding_speed, pinput->interval, m_state,
                     generic_radar_range, &range_obs, &mag_fault, &auxillary);
                distance_magnets = range_obs + 0.15;
                distance = distance_magnets;                         //Consider carefully  XY_LU
                
                /* When the car is at rest */
                if( maneuver_id == 0 ) 
                {
//                      initial_dist = generic_radar_range;
                        if( distance_magnets > 2.0 ) 
                                initial_dist = distance_magnets;
                        if( initial_dist < 2.0 ) 
                                initial_dist = DES_FOLLOW_DIST;
                        temp_dist = initial_dist;
                }

                /* When the car is moving */
                if( (maneuver_id != 0) && (maneuver_id != 10) && (time_filter >= 0.0) )
                
/////Old smooth
           temp_dist = DES_FOLLOW_DIST + (initial_dist - DES_FOLLOW_DIST) * exp(-(time_filter-0.0)/20);

///////////////// New smooth                
      
           /*    {
                if (time_filter>=0.0 && time_filter<= 20.0)
                  {               
                   smooth(1.3, time_filter, 0.0, 20.0, initial_dist, DES_FOLLOW_DIST, &int_dist);
                   temp_dist = initial_dist + int_dist;
                  }       
               }*/

           //  dist_flt(pinput->interval, 0.5, 1.0, distance, DES_FOLLOW_DIST, &temp_dist);
            
                if( (maneuver_des == 5) && 
                        ( (m_state. splitting == 1) || (m_state. double_splitting == 1) ) )
                        temp_dist += -split_dist;

                if( (maneuver_des != 5) && (m_state. split_complete == 1) )
                        temp_dist = m_state. initial_join_distance;

                if( (maneuver_des == 9) && (m_state. joining == 1) )
                        temp_dist += split_dist;

                if( (maneuver_des == 19) && (m_state. rejoining == 1) )
                        temp_dist += split_dist;

//              if( (m_state. split_complete == 1) && (maneuver_des != 5) && (m_state. join_init == 0) )
//                      temp_dist += -split_dist;

                eps_dot = v - preceeding_speed;

                /* Make the lane changed car fall back to the end of the platoon */
                if( (maneuver_des == 9) && (m_state. lane_changing == 1) )
                {
//                      BB before:  fallback_amp = 0.3 * (platoon_size - car_number_initial);
                        fallback_amp = 0.6 * (platoon_size - car_number_initial);
                        if( time_fallback <= FALLBACK_TIME_PAR )
                                eps_dot = v - (preceeding_speed - fallback_amp*(1-exp(-0.5*time_fallback)) );
                        else
                        {
                                eps_dot = v - (preceeding_speed - fallback_amp*(1-exp(-0.5*time_fallback)) 
                                        + fallback_amp * ( 1- exp(0.5*FALLBACK_TIME_PAR - 0.5*time_fallback)) );
                        }
                        time_fallback += pinput->interval;
                }

//              if( (maneuver_des == 7) &&
//                      (m_state. preceeding_lane_changing == 1) )
                if( (m_state. joining != 1) &&
                        (m_state. preceeding_lane_changing == 1) )
                        eps += eps_dot * pinput->interval;
//              else if(m_state. lane_changing == 1)

                else if( (m_state. lane_changing == 1) && (m_state. rejoining != 1) )
                        eps += eps_dot * pinput->interval;
                else
                        eps = (distance - temp_dist);         // Original " - " sign   XY_LU   

                if( (stay == 1 ) )
                {
                        if( (maneuver_id != 0) && (maneuver_id != 10) )
                                eps += eps_dot * pinput->interval;
                }

        }

        /*****************************************************/
        /*      Modify eps and eps_dot in case of a fault.   */
        /*****************************************************/

        /* If car #2 or more and there is a fault, do autonomous control */
        if( (car_id != 0) && (car_id != 1) )
        {
                if( fault_mode != 0 )
                {
//                      omega = 0.2;    /* Choi's choice of gain for autonomous mode control */
                        if( (fault_mode != 5) && (fault_mode != 7) ) 
                        {
                                omega = 0.3 + 0.4*exp( - (distance_old - DES_FOLLOW_DIST) );
                                omega_time += pinput->interval;
                                if( omega_time < 20.0 ) 
                                        omega = 0.5;
                                else
                                        omega = 0.3 + 0.2 * exp(-(omega_time-20.0));
                                if( (distance < 12.0) || (distance > 18.0) )
                                        omega = 0.5;
                                        
                                if( omega < 0.3 ) 
                                        omega = 0.3;
                                else if( omega > 0.7 ) 
                                        omega = 0.7;
                                omega = 0.3;
                                
                            // omega is always 0.3 if fault_mode  !=5, 7.
                        }

                        /* Undo the maneuver feedback so that no other maneuver can be performed */
                        if( poutput->maneuver_feedback == 6 ) 
                                poutput->maneuver_feedback = 5;
                        else if( poutput->maneuver_feedback == 8 ) 
                                poutput->maneuver_feedback = 7;
                        else if( poutput->maneuver_feedback == 10 ) 
                                poutput->maneuver_feedback = 9;

                        fault_init = 1;
                        fault_manage(pinput,ptrack,fault_mode,v,range_obs,
                                generic_radar_range,generic_radar_rangerate,
                                &maneuver_id,&stemp_obs,&etemp_obs,
                                &v_fault,&acc_fault,&temp_dist);                

                        /* Set autonomous control speed and acceleration */
                        preceeding_speed = v_fault;
                        preceeding_accel = acc_fault;                          
                        eps = stemp_obs;
                        eps_dot = etemp_obs;
                }
        }

        if( car_id == 1 )
        {
                if( fault_mode != 0 ) 
                        fault_init = 1;
        }

        /* In case of a fault, turn the beeper on for 5 sec */
        if( fault_init == 1 )
        {
                beeper_time += pinput->interval;
                if( (beeper_time < 5.0) && (maneuver_id != 10) )
                        poutput->beeper = 1;
                else 
                        poutput->beeper = 0;
        }
        if (beeper_time >= 5.0)
                beeper_time=0.0;                           // Added by XY_LU
        if( (fault_init == 0) && (generic_radar_range > 2.0) && (m_state. lane_changing == 0)
            && (m_state. lane_change_back == 0) )
                poutput->beeper = 0;
                
     temp3 += s_2*dt;
     slid = temp3 + k1*s_2;
     if (time_filter<0.07)
        slid_old = slid;
     else
        slid = 0.3*slid_old + 0.7*slid;        //svg3(slid);
     slid_dot = (slid - slid_old)/dt;     
     slid_old=slid;   
     temp5a = slid_dot + k2*slid;  // + 0.3*SIGN(slid)*pow(fabs(SAT(0.15,slid)),2.0);      
     if (time_filter<0.07)
         {
           s_1_old = s_1;
           s_1_flt = s_1;
           slid1_old = slid1; 
         }
      else
         s_1_flt = 0.3*s_1_old + 0.7*s_1;      //svg3a(s_1);            
      s_1_d = (s_1_flt - s_1_old)/dt;
      slid1 = s_1 + k1*s_1_d;
      slid1 = 0.3*slid1_old + 0.7*slid1;       //svg3b(slid1);    
      slid1_dot = (slid1 - slid1_old)/dt;
      s_1_old = s_1;                           //s_1_flt;
      slid1_old = slid1;
      temp5b = slid1 + k2*slid1_dot;  // + 0.3*SIGN(slid1)*pow(fabs(SAT(0.15,slid1)),2.0);
      temp5 = 0.0*temp5a + 1.0*temp5b;
      ff_gain = max(ff_gain_old,max(2.0,(v-45.0)/5.0));
      ff_gain_old=ff_gain;
      if (time_filter < 0.07)
         { 
           f_f_hat = f_f;
           f_b_hat = f_b;
         } 
      else if (adap_a == 1)
          f_f_hat -= ff_gain*temp5;             
      else if (adap_b == 1)
        {
          f_f_hat -= ff_gain*temp5;                   
          f_b_hat -= 0.0001* temp5;     
        }   
      ref_speed=(0.5+0.5*exp(time_filter/10.0))*lead_speed + (0.5-0.5*exp(time_filter/10.0))*preceeding_speed;                  
 if( time_filter < 0.07 )
 {
       alp_des = alp;
       alp_des_old = alp;
       alp_desd = 0.0;
       alp_gain = 0.0;
       ma = pm / (tmtopm*tman);
       ma_old = ma;
       mades = ma;
       mades_old = mades;
       usyn = preceeding_accel - (k1+k2)*s_2 - k1*k2*s_1;         
       usyn_old = usyn;
       tnet = eng_tbl(we_flt,pm,3,1);
       usyn = (tnet - c_a*r_starh*v*v - r_starh*f_f)/beta;        
 }
    
else
 {       
       s_1= -(distance-temp_dist);                    
              
       s_2=(v- preceeding_speed);                                
      if (time_filter < 8.0)
        {
         k1 = 0.25 + 0.457*time_filter/8.0;
         k2 = 0.5  + 0.207*time_filter/8.0;
        }
      else
        {
         k1=0.707;
         k2=0.707;
        }
      usyn= -(k1+k2)*s_2 - k1*k2*s_1 + preceeding_accel;  
      usyn=usyn_filter(usyn);     
 }
       

      

        /*****************************************************************/
        /*      Calculate tnet_des with modification to the longitudinal */
        /*      force balance equation at low speeds (the regular        */
        /*      longitudinal force balance equation is very inaccurate   */
        /*      at speeds below 6 mph)                                   */
        /*****************************************************************/

        if( v > 2.45 )
              tnet_p = 0.0;
        else
        {
              temp = min(300*(2.45-v)/2.45,85.0);
              tnet_p = - beta*9.81/(2333.3*1.2)*temp; 
        }
        tnet_des = beta*usyn                                                         
              + c_a*r_starh*v*v  + r_starh*f_f_hat;

        if( maneuver_id != 1 )
              tnet_des += tnet_p;

        /* Le Sabre has no engine brake on Drive */
        tnet_min = 0.0; 


        /**********************************************************/
        /*      Control law to calculate desired throttle angle   */
        /*      Directly from engine mapping.      By XY_LU       */
        /**********************************************************/

        if( tnet_des >= tnet_min )
        {
              adap_a = 1;
              adap_b = 0;
              tnet_a = tnet_des;
              tnet_b = 0.0;
        }
        else 
        {
              adap_a = 0;
              adap_b = 1;
              tnet_a = tnet_min;
              tnet_b = tnet_des - tnet_min;
        }
        
        
        /* Filter we and pm */
        if( time_filter < 0.07 )
        {
                we_old = we;
                we_flt = we;
                pm_old = pm;
        }
        else
        {

            if( (time_filter <= 1.5) || (tg < 0.8) || (tg > 1.5) )
                {
                      if( we - we_old > 5.0 )
                            we = we_old + 5.0;
                      else if( we - we_old < -5.0 )
                            we = we_old - 5.0;
                      if( pm - pm_old > 0.2 )
                            pm = pm_old + 0.2;
                      else if( pm - pm_old < -0.2 )
                            pm = pm_old - 0.2;
                 }
            else
                 {
                      if( we - we_old > 20.0 )
                            we = we_old + 20.0;
                      else if( we - we_old < -20.0 )
                            we = we_old - 20.0;
                      if( pm - pm_old > 0.8 )
                            pm = pm_old + 0.8;
                      else if (pm - pm_old < -0.8)
                            pm = pm_old - 0.8;
                }
                
                we_old = we;
                
                we_flt = we*0.3 + we_flt*0.7;
         }             
                            
        alp_des = eng_tbl(we_flt, tnet_a,1,4);                  
        pm_des = eng_tbl(we_flt,tnet_a,1,3);
        pm= svg3(pm);
        s_3=pm-pm_des;
   
        usyn1 = - 0.1*s_3;        //+2.0*pm_temp);
        if (time_filter <= 10.0)
          alp_des = alp_des + 0.3*usyn1*time_filter/10.0;
        else      
          alp_des = alp_des + (0.3+0.5*fabs(s_1))*usyn1;                     

        /**********************************************************/
        if( (time_filter < 1.6) || (tg > 1.6) )                         //2.0
 {
         
      

          alp_des = 0.2*alp_des + 0.8*alp_des_old;                      //New  

          alp_desd = (alp_des - alp_des_old)/dt;                                
          alp_des_old = alp_des;
 }
        else
 {                     
         
           temp1=0.6*sin(PI*2.0/2.4*tg);
                              
           alp_des = 0.2*(alp_des-temp1) + 0.8*alp_des_old;           

           alp_desd = (alp_des - alp_des_old)/dt;                           
  
           alp_des_old = alp_des;

  
 }
        alp_desd = min(alp_desd,1.5);
        alp_desd = max(alp_desd,-1.5);
        alp_des = max(alp_des,0.0);
        alp_des = min(alp_des,30.0);               // Why 25.0 ?        


        /*********************************************/

        /*      Calculate desired brake pressure     */

        /*********************************************/

        /* Brake control */
        if( time_filter < 0.07 )
        {
                brake_sw = 0;
                brake_sw_old = 0;
                pb_des = 0.0; 
                pb_flt = pb;
        }

        brake_sw_old = brake_sw;
        if( (brake_sw == 0) && (tnet_des <= 20.0) )
                brake_sw = 1;
        else if( (brake_sw == 1) && (tnet_des >= 25.0) )
                brake_sw = 0;

        if( tnet_b < 0.0 )
        {
      /*          tnet_b_flt = 0.2*tnet_b+0.8*tnet_b_old;                               XY_LU */
                abrk_des = - tnet_b/beta;
                pb_des = f_b_hat*abrk_des*2333.3/9.81*1.5; /* g-force to psi */ /*XY_LU*/
                tnet_b_old = tnet_b;
                alp_des = 0.0;
        }
        else if( brake_sw == 0 )
                pb_des = 0.0;   /* No brake pressure */
        else
/*             pb_des = 20.0;   Fill up the brake line with pressure (stand-by mode) */
// NEW MODIFICATION TODAY
                pb_des = 0.0;   /* No brake pressure */

        if( maneuver_id == 0 ) 
                pb_des = STOP_BRAKE_PRESSURE;
        else if( maneuver_id == 10 ) 
        {
//              pb_temp += (- pb_temp +  (150.0 - pb_des_old) ) / (0.5*150.0);
                pb_temp += (- pb_temp +  (STOP_BRAKE_PRESSURE - pb_des_old) ) / 
                        (0.5*STOP_BRAKE_PRESSURE);
                pb_des = pb_temp + pb_des_old;
                alp_des = 0.0; 
        }
        else
        {
                /* Do we need a filter on desired brake ????? */
//              pb_des = pb_des + (-pb_des + pb_des_old) * pinput->interval / 0.01 ;
                pb_des_old = pb_des;
                pb_temp = 0.0;
       }

        if( pb_des > 800.0 ) 
                pb_des = 800.0;

        /************************************************************************/
        /*      Send out throttle and brake commands under no-fault scenario.   */
        /************************************************************************/

//      if(tnet_b < 0.0)
        if( pb_des > 100.0 )
                poutput->brake = pb_des + 0.4*(pb_des - pb);
        else
                poutput->brake = pb_des;
        if ((maneuver_id==1 || maneuver_id==2 || maneuver_id==3) && alp_des<2.3)
           alp_des = 2.3;
        if( time_filter < 0.03 )
           poutput->throttle = 0.0;
        else
           poutput->throttle = 1.7*(alp_des - 0.0*(alp_des - alp))*25.0/85.0;

         if (poutput->throttle >15.0)
              poutput->throttle = 15.0;                     // added by XY_LU      

        /*******************************************************************/
        /*      Static tests to check performance of throttle and brake    */
        /*      actuators.                                                 */
        /*******************************************************************/

        if( config.test_actuators == TRUE )
        {
                if( time_control < config.step_start_time )
                {
                        poutput->throttle = config.throt_down;
                        poutput->brake = config.brake_down;
                }
                else if( time_control < config.step_end_time )
                {
                        poutput->throttle = config.throt_up;
                        poutput->brake = config.brake_up;
                }
                else
                {
                        poutput->throttle = config.throt_down;
                        poutput->brake = config.brake_down;
                }

                if (poutput->maneuver_feedback > 50)
                    poutput->maneuver_feedback = 50;     // To avoid simulation problem.    XY_LU
                if (fault_mode_global > 100)
                    fault_mode_global = 100;             // To avoid simulation problem.    XY_LU
        }

        /***********************************************************/
        /*      Set the variables that need to be communicated     */
        /*      to the following vehicles.                         */
        /***********************************************************/

        comm_counter ++;

        if( car_id == 1 )
        { /* Lead vehicle */
                poutput->long_usr1 = comm_counter;
                poutput->long_usr2 = acc_traj;          //usyn;  //pinput->long_acc;
                poutput->long_usr3 = vel_traj;
                poutput->long_usr4 = (float) mag_distance;
                poutput->long_usr5 = marker_number;
                poutput->long_usr6 = maneuver_id;
                poutput->target_gap = temp_dist;
                poutput->actual_gap = distance;
        }
        else
        { /* Following vehicle */
                poutput->long_usr1 = comm_counter;
                poutput->long_usr2 = usyn;          //pinput->long_acc;
                poutput->long_usr3 = v;
                poutput->long_usr4 = (float) mag_distance;
                poutput->long_usr5 = marker_number;
                poutput->long_usr6 = maneuver_id;
                poutput->target_gap = temp_dist;
                if( fault_mode == 6 ) 
                        poutput->actual_gap = generic_radar_range;
                else
                        poutput->actual_gap = distance_magnets; //generic_radar_range;
        }

        poutput->throttle_status = 0;
        poutput->brake_status = 0;
        poutput->radar_status = 0;

        if( pinput->actuator_sw == 1 )
                poutput->throttle_status |= 1;
        if( fault_mode == 3 )
                poutput->throttle_status |= 2;
        else if( fault_mode == 4 )
                poutput->brake_status |= 2;
        else if( fault_mode == 5 )
                poutput->radar_status |= 2;

        /*******************************/
        /*      Save data to buffer.   */
        /*******************************/

        /* Timestamp */
        if( fault_mode == 6 )
                global_time += pinput->interval;
        else
                global_time = ptrack->leader.time;

        /* Measured radar distance */
        if( car_id >= 2 )
                temp = generic_radar_range;
        else
                temp = temp_dist - eps;


        /* Save data every data_log */
        if( (verbose_flag == TRUE) && ((data_log_count++ % config.data_log) == 0) )
        {
                #ifdef RECONSTRUCT
                /* For reconstructive run in Buick */
                        sprintf( print_buff,
                        "%i %i %i %i %i %i %f %f %f %f %f %f %f %f %i\n",
                                pinput->car_id,                         //1
                                pinput->mycar_id,                       //2
                                pinput->platoon_pos,                    //3
                                pinput->actuator_sw,                    //4
                                pinput->gear,                           //5
                                pinput->maneuver_des,                   //6
                                pinput->eng_rpm,                        //7
                                pinput->eng_press,                      //8
                                pinput->eng_throt,                      //9
                                pinput->brake_press,                    //10
                                pinput->long_acc,                       //11
                                wr,                                     //12    
                                generic_radar_range,                    //13
                                generic_radar_rangerate,                //14
                                pinput->radar_counter );                //15
                        buff_add( pbuff, print_buff, strlen( print_buff ) );  
                        sprintf( print_buff,
                        "%i %f %f %f %i %f %f %i %f %i %i %f %i %i %i\n",
                                pinput->brake_counter,                  //16
                                ptrack->leader.time,                    //17
                                lead_speed,                             //18
                                lead_accel,                             //19
                                ptrack->leader.long_usr6,               //20
                                preceeding_speed,                       //21
                                preceeding_accel,                       //22
                                ptrack->preceeding.long_usr5,           //23
                                ptrack->preceeding.distance,            //24
                                marker_number,                          //25
                                marker_counter,                         //26
                                mag_distance,                           //27
                                pfault_message->fault_id,               //28
                                pfault_message->car_id,                 //29
                                pfault_message->platoon_size );         //30
                        buff_add( pbuff, print_buff, strlen( print_buff ) );
                #else
                /* For general purpose testing in Buick or faking on office laptop */
                        sprintf( print_buff,
 "%6.3f %6.3f %6.3f %6.3f %3i %3i %3i %6.3f %6.3f %6.3f %6.3f %6.3f %6.3f %6.3f %6.3f %6.3f %6.3f %6.3f %6.3f %6.3f %6.3f %6.3f %6.3f %6.3f %6.3f %6.3f %3i %6.3f %6.3f %6.3f %6.3f %6.3f \n",
                                global_time,                    //1 
                                s_1,                            //2    
                                s_2,                            //3    
                                s_3,                            //4    
                                maneuver_id,                    //5   
                                fault_mode,                     //6
                                maneuver_des,                   //7     
                                preceeding_speed,               //8     
                                v,                              //9    
                                temp_dist,                      //10 
                                usyn,                           //11
                                usyn1,                          //12   
                                poutput->throttle,              //13    
                                pinput->eng_throt,              //14     
                                poutput->brake,                 //15    
                                pb,                             //16
                                preceeding_accel,               //17    
                                lead_accel,                     //18      
                                acl,                            //19    
                                generic_radar_range,            //20  
                                distance,                       //21
                                generic_radar_rangerate,        //22
                                tgt1_rg_flt,                    //23
                                tgt1_rt_flt,                    //24    
                                tg,                             //25     
                                gear_flt,                       //26   
                                fault_mode_global,              //27   
                                we_flt,                         //28   
                                f_f_hat,                        //29  
                                f_b_hat,                        //30
                                delco_rg,                       //31
                                delco_rt);                      //32         
                buff_add( pbuff, print_buff, strlen( print_buff ) );
                #endif
        }

        return( TRUE );
                          
}

/**********************************************************************
***********************************************************************

                                Subroutines.
          
*********************************************************************
*********************************************************************/

void fault_diag(int car_id, int man_id, int f_init, int mag_f,  int f_mode_global,
                 int f_car_global, int man_des, float t_filter, float gen_radar_range,
                 long_input_typ* pinput, control_config_typ config, 
                 maneuver_config_typ m_state, int* f_mode, long_output_typ* poutput)
{   
       static float  time_split=0.0;             
       static int use_magnets = 1;
       static int mode9_f_init = 0;              
       static int radar_init = 0;
       static int radar_counter_old = 0;
       static int radar_zero_counter = 0;              
       static int can_fail_counter = 0, old_f_mode = 0;
              
        /*****************************************************************/
        /*      Check for radar, brake and throttle actuator faults.     */
        /*****************************************************************/


        /* If you are still at rest and the radar drops to less than 2 meters.
         * => do not declare a fault but turn the beeper on and ask to move the car
         * to get a new radar reading. 
         */
        if( (man_id == 0) && (car_id != 1) )
        {
                if( (gen_radar_range < 2.0 ) && (config.static_run == FALSE) )
                {
//                      f_mode = 5;
                        poutput->beeper = 1;
                        if( radar_init == 0 ) 
                                fprintf(stderr, "MOVE THE CAR - YOU ARE MISALIGNED !\n");
                        radar_init = 1;
                }
        }

        /* If the estimated distance is too far off from the radar reading.
         * => if you are doing a JOIN or REJOIN, declare CAN bus fault with the
         * magnetic observer working (fault #7) or not working (fault #8).
         */ 
        if( *f_mode == 5 )
        {
                if( f_init != 1 ) 

                        *f_mode = 0;
                *f_mode = old_f_mode;
                if( ( (m_state. preceeding_lane_changing != 1) || (m_state. joining == 1) )
                        && ( (m_state. lane_changing != 1) || (m_state. rejoining == 1) ) )
                {
                        if( mag_f == 1 ) 
                        {
                                *f_mode = 5;
                                if( use_magnets == 1 )
                                        *f_mode = 7;
                                if( (t_filter < 21.0)
                                        || ( (m_state. joining == 1) && (time_split < 2.0) )
                                        || ( (m_state. rejoining == 1) && (time_split < 2.0) ) )
                                {
                                        *f_mode = 8;
                                        use_magnets = 0;
                                }
                        }
                }
        }

        /* If you are moving and the radar drops to less than 2 meters
         * for at least 75 timesteps (ie 1.5 sec).
         * => if you are doing a JOIN or REJOIN, declare a fault #7 or #8.
         */
        if( ( (m_state. lane_changing != 1) || (m_state. rejoining == 1) )
                && ( (m_state. preceeding_lane_changing != 1) || (m_state. joining == 1) ) )
        {
                if( (car_id != 1) && (gen_radar_range < 2.0) )
                {
                        radar_zero_counter ++;
                        if( (radar_zero_counter > 75) && (config.static_run == FALSE) )
                        {
                                if( use_magnets == 1 )
                                        *f_mode = 7;
                                if( (t_filter < 21.0)
                                        || ( (m_state. joining == 1) && (time_split < 2.0) )
                                        || ( (m_state. rejoining == 1) && (time_split < 2.0) ) )
                                {
                                        *f_mode = 8;
                                        use_magnets = 0;
                                }
                        }
                }
                else 
                        radar_zero_counter = 0;
        }

        /* If the CAN counter for the Delco radar gets stuck for at least
         * 25 steps (ie 0.5 sec).
         * => declare a fault #7 or #8.
         */
        if( config.delco_radar == TRUE )
        {
                if( pinput->radar_counter == radar_counter_old )
                        can_fail_counter ++;
                else
                {
                        can_fail_counter = 0;
                        radar_counter_old = pinput->radar_counter;
                }
                if( can_fail_counter >= 25 ) 
                {
                        if( use_magnets == 1 )
                                *f_mode = 7;
                        if( (t_filter < 21.0)
                                || ( (m_state. joining == 1) && (time_split < 2.0) )
                                || ( (m_state. rejoining == 1) && (time_split < 2.0) ) )
                        {
                                *f_mode = 8;
                                use_magnets = 0;
                        }
                }
        }

        /* Fault in any preceeding car */
        if( (f_mode_global != 0) && (f_car_global < pinput->mycar_id) )
                *f_mode = 6;

        /* Lose communication with lead car */
        if( (f_mode_global == 6) && (f_car_global == pinput->mycar_id) )
                *f_mode = 6;

        /* Lose communication with preceeding car (NEW LINES JULY 97) */
        if( (f_mode_global == 16) && (f_car_global == pinput->mycar_id) )
                *f_mode = 6;

//      if( (man_id == 10) && (*f_mode == 6) )
//              *f_mode = 0;

        /* If preceeding car is doing a lane change.
         * => if radar fault, disregard because unused and keep old fault.
         */
        if( (m_state. preceeding_lane_changing == 1) && (m_state. joining == 0) )
        {
//              if( *f_mode == 5 ) *f_mode = 0;
                if( *f_mode == 5 ) 
                        *f_mode = old_f_mode;
        }

        /* If you are doing a lane change.
         * => if radar fault, disregard because unused and keep old fault.
         * => if a preceeding car has a fault, declare fault #8.
         * => if you car loses communication with the preceeding one, declare fault #8.
         * => if you car has any other fault, declare fault #9.
         *
         * If you are doing a lane change back.
         * => if you car has a fault, declare fault #9.
         */
        if( (m_state. lane_changing == 1) && (m_state. rejoining == 0) )
        {
//              if( (*f_mode == 5) && (f_init != 1) ) *f_mode  = 0;
                if( *f_mode == 5 ) 
                        *f_mode = old_f_mode;
                if( *f_mode == 6 ) 
                {
                        if( (f_mode_global != 0) && (f_car_global < pinput->mycar_id) )
                                *f_mode = 8;
                        if( (f_mode_global != 0) && (f_car_global == pinput->mycar_id) )
                        {
                                if( f_mode_global == 16 ) 
                                        *f_mode = 8;
                                else 
                                {
                                        *f_mode = 9;
                                        mode9_f_init = 1;
                                }
                        }
                }
                if( (m_state. lane_change_back == 1) && (man_des == 17) )

             {
                        if( *f_mode != 0 ) 
                                *f_mode = 9;
                }
                if( mode9_f_init == 1 ) 
                        *f_mode = 9;
        }

        if( man_des == 0 ) 
               *f_mode = 0;

        old_f_mode = *f_mode;
}


void scenario(int car_id, int man_des, int f_mode_global,
              int f_car_global, int join_init, float t_filter, float v_traj,
              float a_traj, float t_ctrl, float gen_radar_range,
              long_input_typ* pinput,control_config_typ config,track_control_typ *  ptrack,
              maneuver_config_typ* m_state_pt, int* car_num_init, 
              float* lead_spd,float* lead_acl, float* split_dst,
              float* pre_spd, float* pre_acl, long_output_typ* poutput)
{        
       static float lead_acl_old=0.0, lead_spd_old=0.0;    
       static float split_accel = 0.0, split_speed = 0.0;
       static float split_speed_amp=0.0, split_acc_amp=0.0;     
       static float split_dst_amp=0.0;
       static float time_split = 0.0, split_time_amp=0.0;
       static float time_lane_change = 0.0, time_lane_change_back=0.0;
       static int lane_change_init = 0;
       float omega_split = 0.0;  
           
        /**************************************************************/
        /*      Set lead and preceeding car speed and acceleration.   */
        /**************************************************************/

        if( (car_id == 0) || (car_id == 1) ) 
        {
                /* If platoon single (car #0) or platoon lead (car #1),
                 * get speed and acceleration from speed profile */
                (*lead_spd) = v_traj;
                (*lead_acl) = a_traj;
                (*pre_spd) = v_traj;
                (*pre_acl) = a_traj;
        }
        else
        {
                /* If car #2 or more, get speed and acceleration through communication
                 * with lead and preceeding cars */
                (*lead_spd) = ptrack->leader.velocity;
                (*lead_acl) = ptrack->leader.accel;
                (*pre_spd) = ptrack->preceeding.velocity;
                (*pre_acl) = ptrack->preceeding.accel;
        }

        if( car_id == 12 )
        {
                if (t_ctrl < 0.07)
                {
                        lead_spd_old = *lead_spd;
                        lead_acl_old = *lead_acl;
                }

                if( *lead_spd - lead_spd_old > 1.0)
                        (*lead_spd) = lead_spd_old + 1.0;
                else if( ((*lead_spd) - lead_spd_old) < -1.0)
                        (*lead_spd) = lead_spd_old - 1.0;
                (*lead_spd) = (*lead_spd)*1.0 + lead_spd_old*0.0;
                (*lead_acl) = (*lead_acl)*0.05 + lead_acl_old*0.95;
                lead_spd_old = (*lead_spd);
                lead_acl_old = (*lead_acl);
        }

        /* If car #3 or more, filter preceeding speed and acceleration */
        if( car_id > 2 )
        {
                (*pre_spd) = (*lead_spd) + ((*pre_spd) - (*lead_spd)) *
                        (1 -  exp(-t_filter/10) );
                (*pre_acl) = (*lead_acl) + ((*pre_acl) - (*lead_acl)) *
                        (1 -  exp(-t_filter/10) );
        }

        /* If you are doing lane change, follow lead speed and acceleration (open loop control) */
        if( (m_state_pt-> lane_changing == 1) && (m_state_pt-> rejoining == 0) )
        {
                (*pre_spd) = (*lead_spd);
                (*pre_acl) = (*lead_acl);
        }


        /******************************************/
        /*      State machine for maneuvers.      */
        /******************************************/

        /* Definition of maneuver desired:
         *      5 : split to new spacing
         *      9 : join to regular spacing
         *      17 : lane change back
         *      19 : rejoin to the end of the platoon
         */

        /* SPLIT from car #1 if my car has no fault */
        if( m_state_pt-> splitting == 0 )
        {
                if( (man_des == 5) && 
                        (pinput->mycar_id == pinput->car_id) )
                {
                        if( (f_mode_global != 0) && (f_car_global == pinput->mycar_id+1) )
                                m_state_pt-> splitting = 0;
                        else
                                m_state_pt-> splitting = 1;
                        m_state_pt-> split_complete = 0;
                        m_state_pt-> joining = 0;
                        m_state_pt-> behind_joining = 0;
                        m_state_pt-> rejoining = 0;
                }
        }

        /* DOUBLE SPLIT for car #3 (because 2 split distances) */
        if( m_state_pt-> double_splitting == 0 )
        {
                if( (man_des == 5) && 
                        ( pinput->mycar_id == pinput->car_id+1) )
                {
                        m_state_pt-> double_splitting = 1;
                        m_state_pt-> split_complete = 0;
                        m_state_pt-> joining = 0;
                        m_state_pt-> behind_joining = 0;
                        m_state_pt-> rejoining = 0;
                }
        }
        
        /* BEHIND SPLIT for cars behind car #3 */
        if( m_state_pt-> behind_splitting == 0 )
        {
                if( (man_des == 5) && 
                        (pinput->mycar_id > (pinput->car_id+1)) ) 
                {
                        m_state_pt-> behind_splitting = 1;
                        m_state_pt-> joining = 0;
                        m_state_pt-> behind_joining = 0;
                        m_state_pt-> rejoining = 0;
                }
        }

        /* JOIN for old car #3 */
        if( m_state_pt-> joining == 0 )
        {
                if( (man_des == 9) && 
                        (pinput->mycar_id == pinput->car_id) )
                {
                        m_state_pt-> joining = 1;
                        m_state_pt-> splitting = 0;
                        m_state_pt-> behind_splitting = 0;
                        m_state_pt-> double_splitting = 0;
                }
        }

        /* BEHIND JOIN for cars behind old car #3 if lane change is completed */
        if( m_state_pt-> behind_joining == 0 )
        {
                if( (man_des == 9) && 
                        (pinput->mycar_id > pinput->car_id) ) 
                {
                        if( m_state_pt-> lane_changing == 1 ) 
                                m_state_pt-> behind_joining = 0;
                        else
                                m_state_pt-> behind_joining = 1;
                        m_state_pt-> splitting = 0;
                        m_state_pt-> behind_splitting = 0;
                        m_state_pt-> double_splitting = 0;
                }
        }

        /* LANE CHANGE BACK to the original lane */
        if( m_state_pt-> lane_change_back == 0 )
        {
                if( (man_des == 17) && 
                        (pinput->mycar_id == pinput->car_id) )
                        m_state_pt-> lane_change_back = 1;
        }

        /* REJOIN to the end of the platoon */
        if( m_state_pt-> rejoining == 0 )
        {
                if( (man_des == 19) && 
                        (pinput->mycar_id == pinput->car_id) )
                {
                        m_state_pt-> rejoining = 1;
                        join_init = 0;
                }
        }

        if( m_state_pt-> join_complete == 1 ) 
                m_state_pt-> preceeding_lane_changing = 0;
        if( m_state_pt-> rejoin_complete == 1 ) 
                m_state_pt-> lane_changing = 0;

        /*********************************************************/
        /*      Set split speed, acceleration and distance.      */
        /*********************************************************/

        if( ( (man_des == 5) &&
                (  (m_state_pt-> splitting == 1) || (m_state_pt-> double_splitting == 1) || (m_state_pt-> behind_splitting == 1) ) )
         || ( (man_des == 9) && 
                ( (m_state_pt-> joining == 1) || (m_state_pt-> behind_joining == 1) ) )
         || ( (man_des == 19) && (m_state_pt-> rejoining == 1) ) )
        {
                split_acc_amp = 0.5*0.15;
                split_dst_amp = SPLITTING_DIST;
                if( join_init == 0 ) 
                        m_state_pt-> initial_join_distance = DES_FOLLOW_DIST + SPLITTING_DIST;
                if( pinput->mycar_id == 1 ) 
                        m_state_pt-> initial_join_distance = DES_FOLLOW_DIST + split_dst_amp;
                if( (man_des == 9) || (man_des == 19) )
                {
                        if( join_init == 0 ) 
                                time_split = 0.0;

                        if( (m_state_pt-> preceeding_lane_changing == 1) && (join_init == 0) )
                                m_state_pt-> initial_join_distance = gen_radar_range;

                        /* Distance between car #3 and car #1 after car #2 does a lane change */
                        if( (m_state_pt-> behind_joining == 1) && (join_init == 0) )
                                m_state_pt-> initial_join_distance = 2.0*SPLITTING_DIST + 2.0*DES_FOLLOW_DIST + 4.8;

                        /* Take radar reading as initial distance if you rejoin the end of the platoon */
                        if( (m_state_pt-> rejoining == 1) && (join_init == 0) )
                                m_state_pt-> initial_join_distance = gen_radar_range;
                        join_init = 1;
                        split_dst_amp = m_state_pt-> initial_join_distance - DES_FOLLOW_DIST;
                }

                if( (config.static_run == TRUE) && (split_dst_amp < 0.1) ) 
                        split_dst_amp = 4.0;   

                omega_split = PI*sqrt(2*split_acc_amp/split_dst_amp);
                split_time_amp = 2*PI/omega_split;
                split_speed_amp = - PI * split_acc_amp/omega_split;

                time_split += pinput->interval;
                if( time_split < split_time_amp )
                {
                        split_accel = - split_acc_amp*(1 - cos(omega_split*time_split) )/2;
                        split_speed = - split_acc_amp*
                            (time_split - sin(omega_split*time_split)/omega_split)/2;
                        (*split_dst) = - split_acc_amp/2 *(0.5*time_split*time_split
                            +(cos(omega_split*time_split) - 1)/omega_split/omega_split );
                }
                else
                {
                        split_accel = split_acc_amp*(1 - cos(omega_split*time_split) )/2.0;
                        split_speed = split_speed_amp + split_acc_amp/2.0 *
                                (time_split - split_time_amp
                              - sin(omega_split*time_split)/omega_split);
                        (*split_dst) = - split_dst_amp/2.0
                                + split_speed_amp * (time_split-split_time_amp)
                                + split_acc_amp/2 * (pow(time_split-split_time_amp,2)/2.0
                                + (cos(omega_split*time_split) - 1)/(omega_split*omega_split));
                }

                /* Set the maneuver feedback:
                 *      5: still m_state_pt-> splitting because fault in the car just behind 
                 *      6: split complete
                 *      9: still m_state_pt-> joining because no second lane change
                 *      10: join complete
                 *      20: rejoin complete
                 */
                if( time_split > 2*split_time_amp ) 
                {
                        if( man_des == 5 ) 
                        {
                                if( (f_mode_global != 0) && (f_car_global == pinput->mycar_id+1) )
                                        poutput->maneuver_feedback = 5;
                                else
                                        poutput->maneuver_feedback = 6;
                                if( (m_state_pt-> splitting == 1) || (m_state_pt-> double_splitting == 1) )
                                        m_state_pt-> split_complete = 1;
                        }
                        else if( man_des == 9 ) 
                        {
                                if( config.end_scenario == FALSE ) 
                                        poutput->maneuver_feedback = 9;
                                else
                                        poutput->maneuver_feedback = 10;
                                if( m_state_pt-> joining == 1 )
                                        m_state_pt-> join_complete = 1;
                        }
                        else if( man_des == 19 ) 
                        {
                                poutput->maneuver_feedback = 20;
                                if( m_state_pt-> rejoining == 1 )
                                        m_state_pt-> rejoin_complete = 1;
                        }
                        split_accel = 0.0;
                        split_speed = 0.0;
                        (*split_dst) = - split_dst_amp;
                }

                /* Fake the maneuver feedback after 1 sec for static run */     
                if( config.static_run == TRUE )
                {
                        if( time_split > 1.0 )
                        {
                                if( man_des == 5 ) 
                                        poutput->maneuver_feedback = 6;
                                else if( man_des == 9 ) 
                                        poutput->maneuver_feedback = 10;
                                else if( man_des == 19 ) 
                                        poutput->maneuver_feedback = 20;
                        }
                }

                /* SPLIT */     
                if( man_des == 5 )
                {
                        if( m_state_pt-> splitting == 1 )
                        {
                        /* I am doing a split */
                                (*lead_acl) += split_accel;
                                (*lead_spd) += split_speed;
                                (*pre_acl) += split_accel;
                                (*pre_spd) += split_speed;
                        }
                        else if( m_state_pt-> double_splitting == 1 )
                        { 
                        /* I am not doing a split, but I am behind a car that is doing a split */
                                (*lead_acl) = (*lead_acl) + 2*split_accel;
                                (*lead_spd) = (*lead_spd) + 2*split_speed;
                                (*pre_acl) += split_accel;
                                (*pre_spd) += split_speed;
                        }
                        else if( m_state_pt-> behind_splitting == 1 )
                        {
                        /* I am not doing a split, but I am behind a car that is doing a double-split */
                                (*lead_acl) = (*lead_acl) + 2*split_accel;
                                (*lead_spd) = (*lead_spd) + 2*split_speed;
                                (*split_dst) = 0.0;
                        }
                }
                /* JOIN or REJOIN */    
                else if( (man_des == 9) || (man_des == 19) )
                {
                        if( (m_state_pt-> joining == 1) || (m_state_pt-> rejoining == 1) )
                        {
                        /* I am doing a join */
                                (*lead_acl) += -split_accel;
                                (*lead_spd) += -split_speed;
                                (*pre_acl) += -split_accel;
                                (*pre_spd) += -split_speed;
                        }
                        else if( m_state_pt-> behind_joining == 1 )
                        {
                        /* I am not doing a join, but I am behind a car that is doing a join */
                                (*lead_acl) += - split_accel;
                                (*lead_spd) += - split_speed;
                                (*split_dst) = 0.0;
                        }
                }

        }
        else 
                time_split = 0.0;

        if( ((m_state_pt-> join_complete == 1) && (man_des != 9))
                || ((m_state_pt-> rejoin_complete == 1) && (man_des != 19)) )
                m_state_pt-> split_complete = 0;

        /*************************************/
        /*      Setup for lane change.       */
        /*************************************/

        /* Definition of maneuver desired:      maneuver feedback:
         *      7 : change lane                         8 : change lane complete
         *      17 : change lane back                   18 : lane change back complete
         */

        /* LANE CHANGE */
        if( man_des == 7 )
        {
                time_lane_change += pinput->interval;
                
                /* Save the initial car ID because of the ID change */
                if( lane_change_init == 0 )
                {
                        (*car_num_init) = pinput->mycar_id;
                        if( pinput->mycar_id == pinput->car_id ) 
                                m_state_pt-> lane_changing = 1;
                        if( pinput->mycar_id == pinput->car_id+1 ) 
                                m_state_pt-> preceeding_lane_changing = 1;
                        lane_change_init = 1;
                }
 
                /* Turn the beeper on for 1 sec */ 
                if( (m_state_pt-> lane_changing == 1) || (m_state_pt-> preceeding_lane_changing == 1) )
                {
                        if( m_state_pt-> lane_changing == 1 ) 
                                poutput->beeper = 1;
                        if( time_lane_change > 1.0 ) 
                        {
                                m_state_pt-> lane_change_complete = 1;
                                if( m_state_pt-> lane_changing == 1 ) 
                                        poutput->maneuver_feedback = 8;
                                poutput->beeper = 0;
                        }
                }

                /* Follow the lead speed and acceleration (open loop control) */
                (*pre_spd) = (*lead_spd);
                (*pre_acl) = (*lead_acl);
        }
        /* LANE CHANGE BACK */
        else if( (man_des == 17) && (m_state_pt-> lane_change_back == 1) )
        {
                time_lane_change_back += pinput->interval;
                if( time_lane_change_back < 1.0 )
                        poutput->beeper = 1;
                else
                {
                        m_state_pt-> lane_change_back_complete = 1;
                        poutput->maneuver_feedback = 18;
                        poutput->beeper = 0;
                        time_lane_change_back = 0.0;        // Added by XY_LU  
                }
        }

}


/********************************************************************
      car_ref()

      similar but different from trajectory()

      Real-time code for Le Sabre
   
      This function calculates desired velocity and acceleration, 
      given maneuver_des for robust test of controllers. This is  
      an spd based approach but keep acc continuous.
      gain_s is used to signal gain transition time.

      This is usual trajectory planning.
      It ca use either Expeontial acc up or sinusoidal acc up.                                                 
                                                                  
              Written and tested by XY_LU Jan. 12, 2001                
********************************************************************/


void car_ref2( float delta_t, float t_ctrl, int maneuver_des, int test_site, float des_spd, float decel,  
               float *vel_traj, float *acc_traj, int *maneuver_id , int *gain_s)
{
        static float temp1=0.0, temp3=0.0, ttemp=0.0, ttemp1=0.0;
        static float t_wait=0.0, t_cruise = 5.0 ;
        static float v_next=0.0;
        static float v_final=0.0, v_start = 0.0, dt_brk=0.1, max_spd=0.0;
        static float  acc_start = 0.654;
        static float v_exp_init = 5.0, L_t = 5.0;
        static float  v_des=0.0, a_des=0.0;
        static float sino_start_t=0.0, x_des=0.0;
        const short int exp_s=0;

        v_start = 0.0;
        v_final = 0.0;
        max_spd=des_spd*1609.0/3600.0;    // [m/s]
        v_exp_init=0.7*max_spd;
        switch( test_site )
        {
        case RFS:
                acc_start = 0.654;
                decel = 0.65;
                break;
                
        case CRO:
                decel = 0.5; L_t = 10.0; v_exp_init = 10.0;
                /* Normal operation without splitting */
                // L_t = 15.0; v_exp_init = 15.0;
                break;
                
        case I15:
                decel = 0.5; v_exp_init = 55.0; L_t = 105.0;                       
                acc_start = 0.6;

            /* The following parameters work even in the NORTH side hills */    
            //v_exp_init = 35.0; L_t = 65.0;                              

            /* Profile used during certification */
            //    acc_start = 0.7; v_exp_init = 25.0; L_t = 45.0;                         
                break;
                
        case N11:           
                acc_start = 0.7; decel = 0.7;
                L_t = 20.0; v_exp_init = 10.0;
                break;
        }


        
        temp1 = max((max_spd - v_exp_init), 0.0);              /* v_o */
        temp3 = acc_start / (max_spd - temp1);                 /* lambda */
        if (exp_s == 1)
           L_t = 10.0*(des_spd/40.0);                             /* Exp rising */
        else
           L_t=(max_spd-temp1)*PI/(2.0*acc_start);                /* Sin rising  */
        
        
        /*  Definition of maneuver_des:
         *      1 : stay at rest
         *      3 : accelerate to desired speed using your specified profile
         *      29 : decelerate to desired speed using your specified profile
         */

        /* Definition of maneuver ID:
         *      0  : stay at rest
         *      1  : accelerate to desired speed using specified profile
         *      2  : constant speed following (cruising)
         *      3  : have been cruising for more than 5 seconds
         *      5  : track a sinosuidal trajectory of specified frequency and amplitude // XY_LU
         *      9  : decelerate to desired speed using specified profile
         *      10 : brake to stop
         */

      if( maneuver_des == 0 ) 
                maneuver_des = 1;
      else if( (maneuver_des > 3) && (maneuver_des <= 27) ) 
             maneuver_des = 3;

      switch( maneuver_des )
        {
        case 1:
                *maneuver_id = 0;
                v_des = v_start;
                a_des = 0.0;
                t_wait = t_ctrl;
                t_wait = t_wait + 15.0; //BB before: 13.0 sec
                break;
        case 3:
                if( t_ctrl <= t_wait )
                  {
                        *maneuver_id = 0;
                        v_des = v_start;
                        a_des = 0.0;
                  }
                else if(t_ctrl < t_wait + temp1/acc_start)
                  {
                        *maneuver_id = 1;
                        ttemp = t_ctrl - t_wait;
                        v_des = acc_start * ttemp; 
                        a_des = acc_start;
                        v_next = v_des; 
                  }
                else if(t_ctrl <= t_wait + temp1/acc_start + L_t)                      
                  {
                        *maneuver_id = 1;
                        if (exp_s == 1)                                                  // Exp rising
                         {                           
                           ttemp = t_ctrl - t_wait - temp1/acc_start ;
                           v_des = temp1 + (max_spd - temp1)
                                   *(1.0 - exp(-temp3*ttemp));
                           v_next = v_des;
                           a_des = (max_spd - temp1)*temp3*exp(-temp3*ttemp);                                    
                         }
                        else                                                            // Sin  rising
                         {                 
                           ttemp = t_ctrl - t_wait- temp1/acc_start;
                           v_des = temp1 + (max_spd - temp1)*sin(ttemp*PI/(2.0*L_t));
                           v_next = v_des;
                           a_des = (max_spd - temp1)*PI*cos(ttemp*PI/(2.0*L_t))/(2.0*L_t);
                         }
                  }                 
                else  // if(t_ctrl <= t_wait + temp1/acc_start + L_t + t_cruise) 
                  {
                        *gain_s = 1;
                        *maneuver_id = 2;
                        v_des = v_next;
                        a_des = 0.0;
                        t_cruise = t_ctrl - t_wait - temp1/acc_start - L_t; // If fixed, then stop can be controlled here 
                        if( t_cruise >= 2.0 ) 
                                *maneuver_id = 3;
                  }                
                break;
        case 29:        
                if( ttemp1 <= dt_brk )  //( t_ctrl <= t_wait + temp1/acc_start + L_t + t_cruise + dt_brk)
                  {
                      *maneuver_id = 9;
                      dt_brk = (max_spd - v_final) / decel;
                      ttemp1 += delta_t;
                      ttemp = t_ctrl - t_wait - temp1/acc_start - L_t - t_cruise;
                      v_des = v_next - (v_next - v_final)/dt_brk*ttemp1
                              + (v_next - v_final)/2./PI*sin(2.*PI/dt_brk*ttemp1);
                      a_des = (v_next - v_final)/dt_brk*(-1.0+cos(2.*PI/dt_brk*ttemp1));                        
                      if( v_des < 0.5 ) 
                                *maneuver_id = 10;
                  }
               else
                  {
                        *maneuver_id = 2;
                        v_des = v_final;
                        a_des = 0.0;
                        if( v_des < 0.5 ) 
                           *maneuver_id = 10;
                  }
                break;
        }

      /* Return desired velocity and acceleration */
      //x_des += v_des*delta_t;
      *acc_traj = a_des;
      *vel_traj = v_des;
}


/*****************************/
/*      fault_detect()       */
/*****************************/
void fault_detect( long_input_typ *pinput, float pb_des, float alp_des, int car_id,
        float radar_range, int *fault_mode )
{
        static float brake_res_f_old = 0.0, throttle_res_f_old = 0.0;
        float brake_res, brake_res_f, tau_brake_res = 1.0/6.28, brake_threshold = 290.0;
        float throttle_res, throttle_res_f, tau_throttle_res = 1.0/3.28, throttle_threshold = 8.0; //4.0
        float radar_rate, radar_threshold = 20.0;       //tau_throttle_res = 1.0/6.28,
        static float radar_range_old = 6.0;
        static int bad_radar_counter = 0;             
        static int first_time = 0;

//      if ( (pinput->system_status & 0x40) != NULL )
//              *fault_mode = 1;        /* autonomous brake TT on */
//      if ( (pinput->mode_status & 0x04) == NULL )
//              *fault_mode = 2;        /* autonomous mode not engaged */


        /* THROTTLE FAULT */
        alp_des=1.65*alp_des*25.0/85.0;              // Properly scale before comparision.   XY_LU
        throttle_res = pinput->eng_throt - alp_des;
        throttle_res_f = throttle_res_f_old + ( throttle_res - throttle_res_f_old) 
                                 * pinput->interval /tau_throttle_res ;
        throttle_res_f_old = throttle_res_f;

        if( fabs(throttle_res_f) > throttle_threshold )
                *fault_mode = 3;

        /* BRAKE FAULT */
        brake_res = pinput->brake_press - pb_des;
        brake_res_f = brake_res_f_old + (brake_res - brake_res_f_old) 
                                 * pinput->interval /tau_brake_res ;
        brake_res_f_old = brake_res_f;

        if( fabs(brake_res_f) > brake_threshold )
                *fault_mode = 4;

        /* RADAR FAULT */
        if( (car_id != 0) && (car_id != 1) )
        {
                if( first_time == 0 )
                {
                        radar_range_old = radar_range;
                        first_time = 1;
                }
                radar_rate = fabs(radar_range - radar_range_old);
                if( radar_rate > ((bad_radar_counter+1)*radar_threshold * pinput->interval) )
                        bad_radar_counter ++;
                else
                {
                        bad_radar_counter = 0;
                        radar_range_old = radar_range;
                }
        }

        if( bad_radar_counter > 4 )
                *fault_mode = 5;

}


/******************************/
/*      fault_manage()        */
/******************************/
void fault_manage(long_input_typ *pinput, track_control_typ *ptrack,
        int fault_mode, float v, float range_obs,
        float radar_range, float radar_rangerate,
        int *maneuver_id, float *eps, float *eps_dot,
        float *v_fault, float *acc_fault, float *fault_des_follow_dist )
{
//      float final_follow_dist = 5.0;
        float final_follow_dist = 15.0, time_temp, temp_dist;
        float stop_time, v_des, v_final, decel;
        float dt_brk, ttemp, a_des;
        static int counter = 0;
        static float pos_err = 0.0, vel_err = 0.0, v_initial = 10.0, initial_dist = DES_FOLLOW_DIST;
        static float vel_err_f = 0.0, vel_err_f_old = 0.0;
        static float vel_err_int = 0.0;
        static float tau_vel = 2*0.2/6.28;
        static float range_rate_old = 0.0;
        static float radar_f, radar_f_old, tau_radar = 2*0.2/6.28;
        static float prefilter_radar_range, prefilter_radar_old;

        switch( fault_mode )
        {
        case 3:
        case 4:
        case 6:
                stop_time = 1500.0;
                counter++;
                time_temp = pinput->interval * counter;
                if( counter < 2 )
                {
                        initial_dist = radar_range;
                        v_initial = v;
                        prefilter_radar_old = radar_range;
                        prefilter_radar_range = radar_range;
                        radar_rangerate = 0.0;
                        range_rate_old = - radar_rangerate;  
                }
                if( time_temp < stop_time )
                {
                        vel_err = - radar_rangerate;  // ? not sure of sign
                        if( (vel_err - range_rate_old) > 0.07 ) 
                                vel_err = range_rate_old + 0.07;
                        else if( (vel_err - range_rate_old) < -0.07 )
                                vel_err = range_rate_old - 0.07;
                        range_rate_old = vel_err;
//                      vel_err = v - ptrack->preceeding.velocity;
                        vel_err_f = vel_err_f_old + ( vel_err - vel_err_f_old) 
                                 * pinput->interval /tau_vel ;
                        vel_err_f_old = vel_err_f; 

                        if( radar_range > 2.0 )
                                prefilter_radar_range = radar_range;
                        if( (prefilter_radar_range - prefilter_radar_old) > 0.5 ) 
                                prefilter_radar_range = prefilter_radar_old + 0.5;
                        else if ( (prefilter_radar_range - prefilter_radar_old) < -0.5 )
                                prefilter_radar_range = prefilter_radar_old - 0.5;
                        prefilter_radar_old = prefilter_radar_range;

//  radar_f = radar_f_old + ( radar_range - radar_f_old) 
//  NEW LINE TODAY
                        radar_f = radar_f_old + ( prefilter_radar_range - radar_f_old) 
                                 * pinput->interval /tau_radar ;
                        radar_f_old = radar_f; 

                        temp_dist = final_follow_dist + (initial_dist - final_follow_dist) * exp(-0.25*2*time_temp/10);
//  temp_dist = initial_dist;

//  pos_err = - (radar_range - temp_dist);
                        pos_err = - (radar_f - temp_dist);
//  v_des = v + radar_rangerate;
                        v_des = v - vel_err_f;
//  v_des = ptrack->preceeding.velocity;
                        a_des = 0.0;
//  a_des = ptrack->preceeding.accel;
                        if( *maneuver_id == 9 ) 
                                a_des = -0.25;
                        *maneuver_id = 2;
                        if( v_des < 0.5 ) 
                                *maneuver_id = 10;
                        temp_dist = temp_dist + 1.25*v;
                }
                break;  
        case 5:         
                /* RADAR FAULT */
                stop_time = 0.1;
                counter++;
                time_temp = counter * pinput->interval;
                if( counter < 2 )
                        v_initial = v;
                if( time_temp < stop_time )
                {
                        v_des = v_initial;
                        a_des = 0.0;
                        vel_err = v - v_des;
                        *maneuver_id = 2;
                        if( (*maneuver_id != 0) && (*maneuver_id != 10) )
                                pos_err += vel_err * pinput->interval;
                        temp_dist = *fault_des_follow_dist;
                        vel_err_f = vel_err;
                }
                break;
        case 7:         
                /* CAN BUS FAULT */
                stop_time = 1500.0;
                counter++;
                time_temp = counter * pinput->interval;
                if( counter < 2 )
                        initial_dist = range_obs;
                if( time_temp < stop_time )
                {
                        v_des = ptrack->leader.velocity;
                        a_des = ptrack->leader.accel;
                        vel_err = v - v_des;
                        vel_err_int += vel_err * pinput->interval;
                        temp_dist = final_follow_dist - initial_dist
                            - (final_follow_dist - initial_dist) *
                               exp(-2*time_temp/10);
//                      pos_err = vel_err_int + temp_dist;
                        temp_dist += initial_dist;
                        pos_err = - range_obs + temp_dist;
                        vel_err_f = vel_err;
                }
                break;
        case 8:         
                /* OPEN LOOP: FOLLOW LEAD CAR VELOCITY AND ACCELERATION */
                stop_time = 1500.0;
                counter++;
                time_temp = counter * pinput->interval;
                if( time_temp < stop_time )
                {
                        v_des = ptrack->leader.velocity;
                        a_des = ptrack->leader.accel;
                        vel_err = v - v_des;
                        if( time_temp <= 40.0 )
                                vel_err = v - (v_des - 0.3*(1-exp(-0.5*time_temp)) ) ;
                        else
                        {
                                vel_err = v - (v_des - 0.3*(1-exp(-0.5*time_temp)) 
                                       + 0.3 * ( 1- exp(0.5*40.0 - 0.5*time_temp)));
                        }
                        vel_err_int += vel_err * pinput->interval;
                        temp_dist = 0.0;
                        pos_err = vel_err_int + temp_dist;
                        temp_dist += initial_dist;
                        temp_dist = *fault_des_follow_dist;
                        vel_err_f = vel_err;
                }
                break;
        case 9:         
                /* COME TO A STOP */
                stop_time = 0.1;
                counter++;
                time_temp = counter * pinput->interval;
                if( counter < 2 )
                        v_initial = v;
                if( time_temp < stop_time )
                {
                        v_des = v_initial;
                        a_des = 0.0;
                        vel_err = v - v_des;
                        *maneuver_id = 2;
                        if( (*maneuver_id != 0) && (*maneuver_id != 10) )
                                pos_err += vel_err * pinput->interval;
                        temp_dist = *fault_des_follow_dist;
                        vel_err_f = vel_err;
                }
                break;
        }

        /* The following "braking to stop" procedure is common for all fault modes,
         * although the deceleration magnitude should be set differently for
         * different cars, based on car_id (platoon position).
         */

        if( time_temp > stop_time )
        {
                decel = 0.75; 
                v_final = 0.0;
                dt_brk =  (v_initial - v_final)/decel;
                if( time_temp <= (stop_time + dt_brk) )
                {
                        *maneuver_id = 9;
                        ttemp = time_temp - stop_time;
                        v_des = v_initial - (v_initial - v_final)/dt_brk*ttemp
                              + (v_initial - v_final)/2.0/PI*sin(2.0*PI/dt_brk*ttemp);
                        a_des = (v_initial - v_final)/dt_brk*(-1.0+cos(2.0*PI/dt_brk*ttemp));
                        if( v_des < 0.5 ) 
                                *maneuver_id = 10;
                }
                else
                {
                        *maneuver_id = 2;
                        v_des = v_final;
                        a_des = 0.0;
                        if( v_des < 0.5 ) 
                                *maneuver_id = 10;
                }
                vel_err = v - v_des;
                if( (*maneuver_id != 0) && (*maneuver_id != 10) )
                        pos_err += vel_err * pinput->interval;
                temp_dist = *fault_des_follow_dist;
                vel_err_f = vel_err;
        }
 
        *v_fault = v_des;
        *acc_fault = a_des;
        *eps = pos_err;
        *eps_dot = vel_err_f;
        *fault_des_follow_dist = temp_dist;
  
        if( *v_fault > 100.0 )
        {
                fprintf(stderr,"Beginning of junk!\n");
                fprintf(stderr, "%6.3f %6.3f %6.3f %6.3f\n",v_des,*v_fault,v,radar_rangerate);
        }

}


/********************************/
/*      distance_observer()     */
/********************************/
     
void distance_observer( float speed, float preceeding_speed, float sampling_time,
                         maneuver_config_typ m_state, float radar_range,
                         float *range_obs, int *fault, float *auxillary )
{
        static float est_dist = 0.0, time = 0.0, k1_gain = 0.2;
        static float est_dist_initial;
        static float radar_f, radar_f_old;
        static float prefilter_radar, prefilter_radar_old;
        static float radar_range_old, alarm_time = 0.0;
        static int radar_init1 = 0, radar_init2 = 0;
        static int alarm = 0;
        static float error = 0.0, int_error = 0.0;
        float tau_radar = 3.0/6.28;
        
        time += sampling_time;
        if( time < 2*sampling_time )
        {
                est_dist = radar_range;
                est_dist_initial = radar_range;
                radar_f = radar_range;
                radar_f_old = radar_range;
                radar_range_old = radar_range;
                prefilter_radar_old = radar_range;
        }

        k1_gain = 0.2;
        if( m_state. splitting == 1 ) 
        {
                tau_radar = 0.021;
                k1_gain = 5.0;
                /* NEW LINES TODAY */
                tau_radar = 1.0/6.28;
                k1_gain = 2.5;
        }
        if( m_state. double_splitting == 1 ) 
        {
                tau_radar = 0.021;
                k1_gain = 2.0;
                /* NEW LINES TODAY */
                tau_radar = 1.0/6.28;
                k1_gain = 1.5;
        }
        if( m_state. split_complete == 1 ) 
        {
                tau_radar = 3.0/6.28;
                k1_gain = 0.2;
        }
        if( (m_state. joining == 1) && (m_state. join_complete == 0) )
        {
                tau_radar = 0.021;
                k1_gain = 5.0;
                /* NEW LINES TODAY */
                tau_radar = 1.0/6.28;
                k1_gain = 2.5;
        }
        if( (m_state. rejoining == 1) && (m_state. rejoin_complete == 0) )
        {
                tau_radar = 0.021;
                k1_gain = 5.0;
                /* NEW LINES TODAY */
                tau_radar = 1.0/6.28;
                k1_gain = 2.5;
        }
        
        prefilter_radar = radar_range;
        if( (prefilter_radar - prefilter_radar_old) > 0.2) 
                prefilter_radar = prefilter_radar_old + 0.2;
        else if( (prefilter_radar - prefilter_radar_old) < -0.2) 
                prefilter_radar = prefilter_radar_old - 0.2;
        prefilter_radar_old = prefilter_radar;

        // radar_f = radar_f_old + (radar_range - radar_f_old) 
        // NEW LINES TODAY

        radar_f = radar_f_old + (prefilter_radar - radar_f_old) 
                        * sampling_time / tau_radar;
        radar_f_old = radar_f;

        if( fabs(radar_range - radar_range_old) < 1.0 ) 
                alarm = 0;
        else 
                alarm = 1;

        if( (m_state. preceeding_lane_changing == 1) && (m_state. joining == 1) 
                && (radar_init1 == 0) )
        {
                est_dist = radar_range;
                prefilter_radar_old = radar_range;
                radar_f_old = radar_range;
                radar_init1 = 1;
                alarm = 0;
        }

        if( (m_state. rejoining == 1) && (radar_init2 == 0) )
        {
                est_dist = radar_range;
                prefilter_radar_old = radar_range;
                radar_f_old = radar_range;
                radar_init2 = 1;
                alarm = 0;
        }

        if( alarm == 1 ) 
                k1_gain = 0.0;

        radar_range_old = radar_range;

        est_dist += sampling_time * (-(speed-preceeding_speed)
                        + k1_gain * (-est_dist + radar_f) );

/*
if( ( (m_state. splitting == 1) || (m_state. double_splitting == 1) )
    && (m_state. split_complete == 0) )
                est_dist = radar_f;
*/

        if( alarm == 1 )
        {
                error += radar_range - est_dist;
                alarm_time += sampling_time;
                if( alarm_time > 1.0 )
                {
                        if( fabs(error) > 50.0 ) 
                                *fault = 1;
                        else
                        {
                                *fault = 0;
                                alarm = 0;
                                error = 0;
                                alarm_time = 0.0;
                        }
                }
        }

// if(fabs(speed) < 2.0) est_dist = radar_range;

        int_error += radar_range - est_dist;

        *range_obs = est_dist;
        *auxillary = error;
}


/**************************/
/*      eng_tbl()         */
/**************************/

static float eng_tbl( float we, float in_value, int in_index, int out_index )
{
        float  outa, outb, a, b, r, ww;
        float ta=0.0,tb=0.0;                        
        int i, j, k;
        int ind_I=0, ind_J=0, ind_K=0; 

        ww = we * 2.0 * PI / 60.0;      /* rpm to rad/s */

        if( ww < dat[0][0] )
        {
                ind_I = 0;
                ww = dat[0][0];
        }
        else if( ww >= dat[254][0] )
        {
                ind_I = 15;
                ww = dat[254][0];
        }
        else
        {
                for( i = 0; i < 16; i++ )
                        if( (ww >= dat[i*15][0] && ww < dat[(i+1) * 15][0] ) ||
                            (ww <= dat[i*15][0] && ww > dat[(i+1) * 15][0] ) ) 
                               {ind_I=i; break;}
        }                                                                              


        if( in_value < dat[ind_I*15][in_index] )
        {
              ind_J = 0;
                /**/ in_value = dat[ind_I*15][in_index];   /**/
        }
        else if( in_value > dat[ind_I*15 + 14][in_index] )
        {
              ind_J = 13;
                /**/ in_value = dat[ind_I*15 + 14][in_index];   /**/
        }
        else
        {
            for( j = 0; j < 14; j++ )
              if( (in_value >= dat[ind_I*15 + j][in_index] && in_value < dat[ind_I*15 + j + 1][in_index])||
                  (in_value <= dat[ind_I*15 + j][in_index] && in_value > dat[ind_I*15 + j + 1][in_index]) )
                               {ind_J=j; break;}
        }

        if( in_value < dat[(ind_I + 1)*15][in_index] )
        {
             ind_K = 0;
                /**/ in_value = dat[(ind_I+1)*15][in_index];   /**/
        }
        else if( in_value > dat[(ind_I + 1)*15 + 14][in_index] )
        {
             ind_K = 13;
                /**/ in_value = dat[(ind_I + 1)*15 + 14][in_index];   /**/
        }
        else
        {
             for( k = 0; k < 14; k++ )
                if( (in_value >= dat[(ind_I + 1)*15 + k][in_index] 
                     && in_value < dat[(ind_I + 1)*15 +k +1][in_index] ) ||
                    (in_value <= dat[(ind_I + 1)*15 + k][in_index] 
                     && in_value > dat[(ind_I + 1)*15 +k +1][in_index] ) )
                             {ind_K=k; break;}
        }                                                              
               

       a = ww - dat[ind_I*15][0];                                   
        b = dat[(ind_I+1) *15][0] - ww;

        ta = (dat[(ind_I+1) *15 + ind_K][in_index] * a
                        + dat[ind_I*15 + ind_J][in_index] * b) / (a + b);

        outa = (dat[(ind_I+1) *15  + ind_K][out_index] * a
                        + dat[ind_I*15 + ind_J][out_index] * b) / (a + b);

        tb = (dat[(ind_I+1) *15  + ind_K + 1][in_index] * a
                        + dat[ind_I*15 + ind_J + 1][in_index] * b) / (a + b);

        outb = (dat[(ind_I+1) *15 + ind_K + 1][out_index] * a
                        + dat[ind_I*15 + ind_J + 1][out_index] * b) / (a + b);
        a =((in_value) - (ta));
        b = ((tb) - (in_value)); 
        r = (outb*a + outa*b) / (a + b); 
        return( r );
}

/**************************/
/*      gear_tbl()        */
/**************************/
float gear_tbl(float dd )
{
        float a, b, p, q;
        int i;
        static ind_I=0;

        static float index[4] = {1.0, 2.0, 3.0, 4.0}; /* gear position */
        static float value[4] = {0.3423, 0.6378, 1.0, 1.4184}; /* gear ratio */

        b = dd;

        if( b <= index[0] )
                return( value[0] );
        else if( b >= index[3] )
                return( value[3] );
        else
        {
                for( i=0; i<3; i++ )
                        if((b >= index[i]) && (b < index[i+1]))
                            {ind_I=i; break;}
                                                                     
                p = b - index[ind_I];
                q = index[ind_I+1] - b;
                a = (value[ind_I]*q + value[ind_I+1]*p)/(p+q);
                return( a );
        }
}



/*********************** Filters ******************************/

float usyn_filter(float in_dat)

{
   float x[2]={0.0,0.0}, out_dat=0.0;
   static float x_old[2]={0.0,0.0};
   

   x[0]=0.2779*x_old[0] - 0.4152*x_old[1] + 0.5872*in_dat;
   x[1]=0.4152*x_old[0] + 0.8651*x_old[1] + 0.1908*in_dat;  
   out_dat = 0.1468*x[0] + 0.6594*x[1] + 0.0675*in_dat;
   x_old[0]=x[0];
   x_old[1]=x[1];
   return out_dat;
}

float svg3(float sig_in)
{
    const int i_max=3;
    const float c[3]={-0.166667,0.333333,0.833333};
            
    static float x[3]={0.0,0.0,0.0};                                        
    float y=0.0;
    int i;
    
    x[i_max-1]=sig_in;
    for (i=0;i<i_max;i++)
      y+=c[i]*x[i];
    for (i=0;i<i_max-1;i++)
      x[i]=x[i+1];   
    return y;
}

float svg5(float sig_in)
{
    const int i_max=5;
    const float c[5]={-0.200000,0.000000,0.200000,0.400000,0.600000};
            
    static float x[5]={0.0,0.0,0.0,0.0,0.0};                                        
    float y=0.0;
    int i;
    
    x[i_max-1]=sig_in;
    for (i=0;i<i_max;i++)
      y+=c[i]*x[i];
    for (i=0;i<i_max-1;i++)
      x[i]=x[i+1];   
    return y;
}

float svg3a(float sig_in)
{
    const int i_max=3;
    const float c[3]={-0.166667,0.333333,0.833333};
            
    static float x[3]={0.0,0.0,0.0};                                        
    float y=0.0;
    int i;
    
    x[i_max-1]=sig_in;
    for (i=0;i<i_max;i++)
      y+=c[i]*x[i];
    for (i=0;i<i_max-1;i++)
      x[i]=x[i+1];   
    return y;
}

float svg5a(float sig_in)
{
    const int i_max=5;
    const float c[5]={-0.200000,0.000000,0.200000,0.400000,0.600000};
            
    static float x[5]={0.0,0.0,0.0,0.0,0.0};                                        
    float y=0.0;
    int i;
    
    x[i_max-1]=sig_in;
    for (i=0;i<i_max;i++)
      y+=c[i]*x[i];
    for (i=0;i<i_max-1;i++)
      x[i]=x[i+1];   
    return y;
}


float svg3b(float sig_in)
{
    const int i_max=3;
    const float c[3]={-0.166667,0.333333,0.833333};
            
    static float x[3]={0.0,0.0,0.0};                                        
    float y=0.0;
    int i;
    
    x[i_max-1]=sig_in;
    for (i=0;i<i_max;i++)
      y+=c[i]*x[i];
    for (i=0;i<i_max-1;i++)
      x[i]=x[i+1];   
    return y;
}

float svg5b(float sig_in)
{
    const int i_max=5;
    const float c[5]={-0.200000,0.000000,0.200000,0.400000,0.600000};
            
    static float x[5]={0.0,0.0,0.0,0.0,0.0};                                        
    float y=0.0;
    int i;
    
    x[i_max-1]=sig_in;
    for (i=0;i<i_max;i++)
      y+=c[i]*x[i];
    for (i=0;i<i_max-1;i++)
      x[i]=x[i+1];   
    return y;
}
float svg7(float sig_in)
{
    const int i_max=7;
    const float c[7]={-0.178572,-0.071429,0.035714,0.142857,0.250000,0.357143,0.464286};
            
    static float x[7]={0.0,0.0,0.0,0.0,0.0,0.0,0.0};                                        
    float y=0.0;
    int i;
    
    x[i_max-1]=sig_in;
    for (i=0;i<i_max;i++)
      y+=c[i]*x[i];
    for (i=0;i<i_max-1;i++)
      x[i]=x[i+1];   
    return y;
}


float svg7a(float sig_in)
{
    const int i_max=7;
    const float c[7]={-0.178572,-0.071429,0.035714,0.142857,0.250000,0.357143,0.464286};
            
    static float x[7]={0.0,0.0,0.0,0.0,0.0,0.0,0.0};                                        
    float y=0.0;
    int i;
    
    x[i_max-1]=sig_in;
    for (i=0;i<i_max;i++)
      y+=c[i]*x[i];
    for (i=0;i<i_max-1;i++)
      x[i]=x[i+1];   
    return y;
}

float svg7b(float sig_in)
{
    const int i_max=7;
    const float c[7]={-0.178572,-0.071429,0.035714,0.142857,0.250000,0.357143,0.464286};
            
    static float x[7]={0.0,0.0,0.0,0.0,0.0,0.0,0.0};                                        
    float y=0.0;
    int i;
    
    x[i_max-1]=sig_in;
    for (i=0;i<i_max;i++)
      y+=c[i]*x[i];
    for (i=0;i<i_max-1;i++)
      x[i]=x[i+1];   
    return y;
}




float svg9(float sig_in)
{
    const int i_max=9;
    const float c[9]={-0.155556,-0.088889,-0.022222,0.044444,0.111111,0.177778,0.244444,
                         0.311111,0.377778};
            
    static float x[9]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};                                        
    float y=0.0;
    int i;
    
    x[i_max-1]=sig_in;
    for (i=0;i<i_max;i++)
      y+=c[i]*x[i];
    for (i=0;i<i_max-1;i++)
      x[i]=x[i+1];   
    return y;
}

float svg9a(float sig_in)
{
    const int i_max=9;
    const float c[9]={-0.155556,-0.088889,-0.022222,0.044444,0.111111,0.177778,0.244444,
                         0.311111,0.377778};
            
    static float x[9]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};                                        
    float y=0.0;
    int i;
    
    x[i_max-1]=sig_in;
    for (i=0;i<i_max;i++)
      y+=c[i]*x[i];
    for (i=0;i<i_max-1;i++)
      x[i]=x[i+1];   
    return y;
}

float svg9b(float sig_in)
{
    const int i_max=9;
    const float c[9]={-0.155556,-0.088889,-0.022222,0.044444,0.111111,0.177778,0.244444,
                         0.311111,0.377778};
            
    static float x[9]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};                                        
    float y=0.0;
    int i;
    
    x[i_max-1]=sig_in;
    for (i=0;i<i_max;i++)
      y+=c[i]*x[i];
    for (i=0;i<i_max-1;i++)
      x[i]=x[i+1];   
    return y;
}




float svg11(float sig_in)
{
    const int i_max=11;
    const float c[11]={-0.136364,-0.090909,-0.045455,0.000000,0.045455,0.090909,0.136364,
                          0.181818,0.227273,0.272727,0.318182};
            
    static float x[11]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};                                        
    float y=0.0;
    int i;
    
    x[i_max-1]=sig_in;
    for (i=0;i<i_max;i++)
      y+=c[i]*x[i];
    for (i=0;i<i_max-1;i++)
      x[i]=x[i+1];   
    return y;
}


float rg_filter(float in_dat)

{
   float x[2]={0.0,0.0}, out_dat=0.0;
   static float x_old[2]={0.0,0.0};
   

   x[0]=0.2779*x_old[0] - 0.4152*x_old[1] + 0.5872*in_dat;
   x[1]=0.4152*x_old[0] + 0.8651*x_old[1] + 0.1908*in_dat;  
   out_dat = 0.1468*x[0] + 0.6594*x[1] + 0.0675*in_dat;
   x_old[0]=x[0];
   x_old[1]=x[1];
   return out_dat;
}


float rt_filter(float in_dat)

{
   float x[2]={0.0,0.0}, out_dat=0.0;
   static float x_old[2]={0.0,0.0};
   

   x[0]=0.2779*x_old[0] - 0.4152*x_old[1] + 0.5872*in_dat;
   x[1]=0.4152*x_old[0] + 0.8651*x_old[1] + 0.1908*in_dat;  
   out_dat = 0.1468*x[0] + 0.6594*x[1] + 0.0675*in_dat;
   x_old[0]=x[0];
   x_old[1]=x[1];
   return out_dat;
}

float sig_d_filter(float in_dat)

{
   float x[2]={0.0,0.0}, out_dat=0.0;
   static float x_old[2]={0.0,0.0};
   

   x[0]=0.2779*x_old[0] - 0.4152*x_old[1] + 0.5872*in_dat;
   x[1]=0.4152*x_old[0] + 0.8651*x_old[1] + 0.1908*in_dat;  
   out_dat = 0.1468*x[0] + 0.6594*x[1] + 0.0675*in_dat;
   x_old[0]=x[0];
   x_old[1]=x[1];
   return out_dat;
}



/********************** Math. Func. ****************************/


float SAT(float Ep, float u )
  {
  
   if (u<=-Ep) 
       return -1.0;
   else if ((u>-Ep) && (u<Ep))
       return u/Ep;
   else 
       return 1.0;
  }  


float SIGN(float x)
  {
   if(x>0.0) 
     return 1.0;
   else if (x==0.0) 
     return 0.0;
   else
     return -1.0;
   }                                   

float SINSAT(float Ep, float x)
  {
   if (x>=Ep) 
      return 1.0;
   else  if (x<=-Ep) 
      return -1.0;
   else
      return sin(x*PI/(2.0*Ep));
  }



/*************************************************************************************
           Gain schedualing test for dynamic back-stepping control

                                       By   XY_LU April, 2000
***************************************************************************************/                                       


void gain(float t_filter, float sample_t, float spd, int man_id, float s1,
           float tg, float k[3])
{

  const float k0_min=0.05, k0_mid=0.25, k0_max=0.7, v_interval=5.0;
  const float k1_min=0.25, k1_mid=1.4, k1_max=3.5;
  const float k2_min=5.0, k2_mid=18.0, k2_mid1=22.0, k2_max=22.0;
//    const float k2_min=3.0, k2_mid=6.0, k2_mid1=10.0, k2_max=12.0;
  
  static float k1_temp=0.0, k1_temp1=0.0, k1_old=0.0, k0_old=0.0, temp_t=0.0, temp_t1=0.0, temp_t2=0.0;

  if (t_filter < 0.07)
    {
     k[0] = k0_min;
     k[1] = k1_min;
     k[2] = k2_max;
    }
  if (spd<v_interval && man_id == 1)
    {
      k[1]=k1_min + spd*(k1_max-k1_min)/v_interval + 0.8*SIGN(s1)*(s1*s1/(0.8+s1*s1));
      k[1]=max(k[1],k1_old);
      k1_old=k[1];
      k1_temp=k[1];
      k[0]=k0_min+spd*(k0_mid-k0_min)/v_interval;
      k[0]=max(k[0],k0_old);
      k0_old=k[0];
      k[2]=k2_max + spd*(k2_mid1-k2_max)/v_interval  + 3.0*SIGN(s1)*(s1*s1/(0.1+s1*s1));           //
    }
  else if  (spd<19.0 && man_id != 0)
    {
      k[1]=k1_temp + 0.8*SIGN(s1)*(s1*s1/(0.8+s1*s1)) ;
//      k[1]=max(k[1],k1_temp);
      k1_temp1=k[1];
      k[0]=k0_mid;
      if (man_id == 1)
        {
         if (spd<10.0)
          k[2]=k2_mid1 + (spd-v_interval)*(k2_mid-k2_mid1)/5.0+ 2.0*SIGN(s1)*(s1*s1/(0.1+s1*s1)); //
         else
          k[2]=k2_mid + 2.0*SIGN(s1)*(s1*s1/(0.1+s1*s1));                                         //
        }
      if (man_id == 9)  
        {
         temp_t2 += sample_t;
         if (temp_t2<4.0)
          k[2]=k2_mid + temp_t2*(k2_mid1-k2_mid)/4.0 + 2.0*SIGN(s1)*(s1*s1/(0.1+s1*s1));         //
         else
          k[2]=k2_mid1 + 2.0*SIGN(s1)*(s1*s1/(0.1+s1*s1));                                       //
        }  
    }

  else if (spd>=19.0)
    {
      
      if (man_id != 9)
        {
          temp_t += sample_t;
          if (temp_t<4.0)
            {
              k[0]=k0_mid + temp_t*(k0_max-k0_mid);
              k[1]=k1_temp1 + temp_t*(k1_mid-k1_temp);
              k[2]=k2_mid + temp_t*(k2_min-k2_mid);
              
            }
          else
            {
              k[0]=k0_max;
              k[1]=k1_mid;
              k[2]=k2_min;
            }  
        }
       
      if (man_id == 9 || man_id == 10)
        {
          temp_t1 += sample_t;
          if (temp_t1<4.0)
            {
              k[1]=k1_mid + temp_t1*(k1_max-k1_mid);
              k[0]=k0_max + temp_t1*(k0_mid-k0_max);
              k[2]=k2_min + temp_t*(k2_mid-k2_min);
            }
          else
            {
              k[0]=k0_mid;
              k[1]=k1_max;
              k[2]=k2_mid;
            }  
        } 
    }
     
   if( ((t_filter > 1.6) && (tg < 1.6)) && (man_id != 9 && man_id != 10))
      k[1] = k[1] - 0.8*cos(tg*PI/3.2);                                
}

/*********************************************************************
C^1 connection of two constant value functions. It is used when

                  ini_t <= time_flt <=fin_t
**********************************************************************/

void smooth(float beta, float time_flt, float ini_t, float fin_t, float ini_value, float fin_val, 
            float *int_val)
{
       *int_val = (fin_val-ini_value)*pow(sin(PI*(time_flt-ini_t)/(2.0*(fin_t-ini_t))), beta);                                     
}

/*********************************************************************
Dynamic real-time contract mapping

                                By XY_LU May 4, 2000
**********************************************************************/

void dist_flt(float sample_t, float rate, float e_rate, float a_value, float b_value, 
            float *int_val)
{
  float temp=a_value-b_value;
  *int_val = a_value - SIGN(temp)*rate*sample_t*
             (exp(e_rate*temp)-exp(-e_rate*temp))/(exp(e_rate*temp)+exp(-e_rate*temp));                                     
}

/*********************************************************************
       Doppler Radar
                                By XY_LU May 4, 2000
**********************************************************************/

void vrd_flt(float delta_t, float tgt_rg, float tgt_rt, float tgt_az, float tgt_mg, float *tgt_rg_flt, float *tgt_rt_flt,
              float *tgt_az_flt, float *tgt_mg_flt)
{

   static float tgt_rg_old=0.0, tgt_rt_old=0.0;
   static float tgt_rg_buff=0.0, tgt_rt_buff=0.0;  
 //  static float tgt_rg_raw=0.0, tgt_rt_raw=0.0; 
   static float tgt_az_old=0.0;
   static float tgt_mg_old=0.0; 
   float tgt_rg_d=0.0;
   static float evrd_start_t=0.0;
   static int evrd_start_s=1, evrd_start_s1=1;



    if ( ((fabs(tgt_rg) < 1.5)) && (evrd_start_s == 1))
       {
             tgt_rg_old = tgt_rg;
             tgt_rg_buff = tgt_rg;             
             tgt_rt_old = tgt_rt;             
             tgt_rt_buff = tgt_rt;
       }
    else if ( (fabs(tgt_rg) >= 1.5) && (evrd_start_t <0.1) && (evrd_start_s1 == 1) )
       {
             tgt_rg_old = tgt_rg;
             tgt_rg_buff = tgt_rg;             
             tgt_rt_old = tgt_rt;             
             tgt_rt_buff = tgt_rt;
             evrd_start_t += delta_t;
             evrd_start_s=0;            
       }
    else
       {
             evrd_start_s=0;
             evrd_start_s1=0;
        tgt_rg_d=(tgt_rg - tgt_rg_old)/delta_t;
     
        if (fabs(tgt_rg_d)>350.0 || fabs(tgt_rt)<0.001)  // or 400.0
            tgt_rg = tgt_rg_old;
        else
           {
               if (tgt_rg > tgt_rg_old+20.0*delta_t)
                 tgt_rg = tgt_rg_old+20.0*delta_t;
               if (tgt_rg < tgt_rg_old-20.0*delta_t)
                 tgt_rg = tgt_rg_old-20.0*delta_t;

           }
        
        if (tgt_rt > tgt_rt_old+3.0*delta_t)
           tgt_rt = tgt_rt_old+3.0*delta_t;
        if (tgt_rt < tgt_rt_old-3.0*delta_t)
           tgt_rt = tgt_rt_old-3.0*delta_t;
        tgt_rt_old = tgt_rt;
  
        tgt_rg_old = tgt_rg;
        
            
        *tgt_rg_flt = 0.1*tgt_rg+0.9*tgt_rg_buff;
        *tgt_rt_flt = 0.1*tgt_rt+0.9*tgt_rt_buff;
       // tgt_rg_buff = rg_filter(tgt_rg_buff);
       // tgt_rt_buff = rg_filter(tgt_rt_buff);
       }

       *tgt_az_flt = tgt_az;       // To be filtered yet
       *tgt_mg_flt = tgt_mg;       // To be filtered yet
}




