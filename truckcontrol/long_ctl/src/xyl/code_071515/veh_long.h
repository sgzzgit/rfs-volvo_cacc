/**********************************************************************

            veh_long.h
            
            collection of most info for vehicle control
            
              updated on 03/25/03
              updated on 03/27/03
              updated on 03/31/03
              updated on 04/02/03
              updated on 04/03/03
              updated on 04/11/03
              updated on 04/20/03
              updated on 10/21/03    added ref_v, ref_a to con_state_typ
              updated on 11/25/09    removed delco and occuner radar
              updated on 05/05/10
              updated on 08/16/10
              started to build for trruck CACC						04_07_15    


                          
                                   XY_Lu

***********************************************************************/

#ifndef VEH_LONG_H
#define VEH_LONG_H

/*      Structure for the controller configuration.     */

//#ifndef FREIGHTLINER_TRUCK
//#define FREIGHTLINER_TRUCK
//#endif

#ifndef VOLVO_TRUCK
#define VOLVO_TRUCK
#endif

#define SIM                       1
#define pi                          3.14159265
#define mph2mps          0.44694
#define rpm2radps		 0.1047
#define v_threshold      0.1
#define N_pt                    25
#define OFF                     0
#define ON                       1
#define off                        0
#define on  
#ifndef FALSE
	#define FALSE                0
#endif
#ifndef TRUE
	#define TRUE                  1
#endif
#define G                          9.81
#define we_idle            700.0        //[rpm]
#define we_max             2100.0         //[rpm]
#define MASS               8339.0        //[kg]            
#define bool_typ           int
#define MAX_DCC                                  0.8
#define DES_FOLLOW_DIST              10.0     
#define COMB_LENGTH                      19.3                     // Truck length in meters
#define TRACTOR_LENGTH              13.7                      // Tractor length in meters
#define TRANS_T			            3.0  // Tranistion time between drive_modes
#define SET_SPEED			25.0	//in [m/s]; ~25mph
//#define T_GAP				1.5     //[s]
#define MIN_CONTR_SPD		5.0	//[m/s]	~ 10[mph] ~ 16.09[k/hr]	
#define MIN_TORQUE_CMD		0.05	// minimum percentage torque can be achieved though auto control: 5%
#define COMM_ERR_BOUND		20
#define V_DELTA				1.0
#define JK_EBS
#define MIN_TORQUE			320.0
#define ENGINE_REF_TORQUE	2589.0  // N.m for Volvo truck
#define JK_REF_TORQUE		1543.0  // N.m for Volvo truck

/*        
#define SPLIT_DIST                               2.0                 // 6.0 for NVD
#define JOIN_DIST                                 2.0                  // 6.0 for NVD
#define E_SPLIT_DIST                          2.0                  // 6.0 for NVD
#define CRUISE_T                                  5.0                // 30.0 for NVD
#define SPLIT_T                                      6.0                // 15.0 for NVD
#define JOIN_T                                        8.0                 //35.0 for NVD
*/

#define SPLIT_DIST                               4.0 
#define JOIN_DIST                                 4.0
#define E_SPLIT_DIST                          4.0
#define CRUISE_T                                 30.0
#define SPLIT_T                                     25.0
#define JOIN_T                                       35.0


#define MAX_BUFFER_SIZE               80000                          
#define MAX_THROTTLE                     4.85  //4.95                        // old:4.5;  4.95 is OK
#define MIN_THROTTLE                     1.45
#define START_THROTTLE               2.0
#define MAX_BRAKE                        1.5
#define STOP_BRAKE                       0.45 
#define MIN_BRAKE                        0.0      //1.2
#define INI_BRAKE                        0.5
#define MAX_WE                           2100.0                         // 2500 [rpm] = 261.800 [rad/s]
#define MIN_WE                           600.0                          //  550 [rpm] = 57.596  [rad/s]
#define STOP_BRAKE_PRESSURE              300.0
#define ACC_OFF_VOLTAGE                  0.0
#define t_ctrl_1                         (0.14) 
#define t_ctrl_2                         (-5.05)
//#define comm_err_bound                   20

//#include "track.h" 

/* Struct for fault bedect */
typedef struct
{
         unsigned   comm_coord   :1;
         unsigned   comm_leader  :1;
         unsigned   comm_pre  :1;
         unsigned   comm_back  :1;
         unsigned   comm  :1;
         unsigned   J_bus_1  :1;
         unsigned   J_bus_2  :1;
         unsigned   e_vrd  :1;     //  dtermined in vrd_dist
         unsigned   lidar  :1;       // determined in ldr_dist
         unsigned   mdl    :1;     // Added on 04/05/10  XYL
         unsigned   mag_meter  :1; // Determined in mag_dist
         unsigned   gps  :1;
         unsigned   throt  :1;
         unsigned   brk  :1;
         unsigned   brk_1  :1;
         unsigned   brk_2  :1;
         unsigned   brk_3  :1;
         unsigned   brk_4  :1;
         unsigned   brk_5  :1;
         unsigned   brk_6  :1;
         unsigned   jake_2  :1;
         unsigned   jake_4  :1;
         unsigned   jake_6  :1;
         unsigned   trans_rtdr  :1;
         unsigned   spd  :1;
         unsigned   wh_spd_1  :1;
         unsigned   wh_spd_2  :1;
         unsigned   wh_spd_3  :1;
         unsigned   wh_spd_4  :1;
         unsigned   we  :1;
         unsigned   we1  :1;
         unsigned   pm  :1;
         unsigned   fuel  :1;
         unsigned   pitch  :1;
         unsigned   acc  :1;       
         unsigned   yaw  :1;
         unsigned   HMI  :1;
         unsigned   sw  :1;
} fault_index_typ;

/* Struct for switches */
typedef struct      // May replaced with  pinput_switch structure
{
  unsigned fan_sw  :1;
     unsigned comp_sw  :1;
     unsigned cond_sw  :1;
     unsigned steer_sw  :1;
     unsigned alt_sw  :1;
     unsigned actuator_sw  :1;
     unsigned radar1_sw  :1;
     unsigned radar2_sw  :1;
     //unsigned auto_sw  :1;
     //unsigned manu_sw  :1;
     unsigned HMI_sw  :1;
     unsigned brake_sw  :1;
     // the following are added on 04/18/15  
     unsigned park_brk_sw:1;
	 unsigned two_spd_axle_sw:1;
	 unsigned park_brk_release:1;
	 unsigned clutch_sw:1;
	 unsigned brk_sw:1;
	 unsigned CC_pause_sw:1;
	 unsigned CC_enable_sw:1;
	 unsigned CC_active:1;
	 unsigned CC_acel_sw:1;
	 unsigned CC_resume_sw:1;
	 unsigned CC_coast_sw:1;
	 unsigned CC_set_sw:1;
	 unsigned CC_state:1;
	 unsigned Pto_state:1;
	 unsigned eng_shutdwn_override_sw:1;
	 unsigned eng_test_mode_sw:1;
	 unsigned eng_idle_decre_sw:1;
	 unsigned end_idle_incre_sw:1;
	 unsigned driveline_enaged:1;
	 unsigned lockup:1;
	 unsigned gshift_sw:1; 
	 unsigned eng_overspeed_enbale:1;
	 unsigned progressive_shift_disable:1;
	 unsigned jk_enable_shift_assist_sw:1;
     unsigned jk_enable_brake_assist_sw:1;
     unsigned jk_mode:1;
     unsigned jk_require_brk_light:1;
	 unsigned foundation_brk_use:1;
	 unsigned halt_brk_mode:1;
	 unsigned XBR_accel_limit:1;
	 unsigned XBR_active_contr_mode:1;
} switch_typ;



/* Struct for maneuver ID */
/*typedef struct
{
        unsigned splitting   :1;
        unsigned split_complete   :1;
        unsigned double_splitting   :1;
        unsigned joining   :1; 
        unsigned join_complete   :1;
        unsigned rejoining   :1;
        unsigned rejoin_complete   :1;
        unsigned behind_joining   :1;
        unsigned behind_splitting   :1;
        unsigned lane_changing   :1;
        unsigned lane_change_complete   :1;
        unsigned preceeding_lane_changing   :1;
        unsigned lane_change_back   :1;
        unsigned lane_change_back_complete   :1;
        unsigned b_merging  :1;
        unsigned b_merging_complete :1;
        unsigned m_merging   :1;
        unsigned m_merging_complete   :1;
} maneuver_config_typ;   */                                     // Not used anymore

/* Struct for initialization */
typedef struct
{
        /* parameter to be read from realtime.ini to avoid recompiling after each parameter changing */
        unsigned short mass_sw;    // 0: Bobtail; 1: empty trailer; 2: half loaded; 3: fully loaded.
        unsigned short task;
        unsigned short run;
	   unsigned short dir;
        float max_spd;              // Determined in realtime.ini
        float max_acc;              // Determined in gen_ref/tran_ref dynamically; Vehicle self maximum acceleration capability
        float max_dcc;              // Determined in realtime.ini
        float para1;
        float para2;
        float para3;
        float para4;
        int data_log;               // Data log interval                
        int pltn_size;              // Platoon size
        bool_typ static_run;        // Static (1) or dynamic (0) run            
        bool_typ test_actuators;    // Test throttle and brake actuators (1) or not (0).    
        bool_typ truck_platoon;     // Truck platooning is on (1) or off (0) 
		bool_typ truck_CACC;		// Truck CACC is on (1) or off (0); 04/15/15 
		bool_typ truck_ACC;	
		float ACC_tGap;
		float CACC_tGap;
		int control_mode;

        bool_typ use_comm;          // Truck platooning is on (1) or off (0)   
        bool_typ eaton_radar;       // Eaton radar (1) ON or (0) OFF. 
        //bool_typ denso_lidar;     // added on 08/16/10; do we need them
        //bool_typ mdl_lidar;       // added on 08/16/10
        bool_typ save_data;         // Save data to log file (1) ON or (0) OFF    
        bool_typ run_data;          // Save data to log file using data log interval (1) ON or (0) OFF    
        bool_typ read_data;         // Save different data to log file (1) ON or (0) OFF    
        bool_typ trans_sw;          // Write auto/manual switch to db (1) ON or (0) OFF
        bool_typ use_gps;           // Read self_gps from db (1) ON or (0) OFF
        bool_typ use_mag;           // Call mag_dist
        bool_typ handle_faults;     // Fault detection and management 
        bool_typ end_scenario;      // Change lane back and rejoin (1) or not (0).  
        float step_start_time;      // Time to begin the step. [sec]                
        float step_end_time;        // Time to end the step. [sec]                  
        float tq_cmd_coeff;         //   
        float spd_cmd_coeff;        //    
        float jk_cmd_coeff;         //        
        float trtd_cmd_coeff;       //           
} control_config_typ;


/*      Struct for state variables    */

typedef struct
{
        unsigned short comm_coord;
        unsigned short comm_leader;
        unsigned short comm_pre;
        unsigned short comm_back;
        float des_f_dist;
        float split_f_dist;
        float join_f_dist;
        float temp_dist;
        float man_dist_var2;
        float man_dist_var3;
        float max_brake;
        float min_brake;
        float ini_brake;
        float stop_brake;
        float spd;
        float acc;
        float fuel_rt; 
	    float manu_acc;
        float manu_speed;
        float manu_throt;
        float manu_spd_cmd;
        float manu_tq_cmd;
        float manu_brk;
        float manu_rtd;
        float manu_jake2;
        float manu_jake4;
        float manu_jake6;
        float auto_acc;
        float auto_speed;
        float auto_throt;
        float auto_spd_cmd;
        float auto_tq_cmd;
        float auto_brk;
        float auto_rtd;
        float auto_jake2;
        float auto_jake4;
        float auto_jake6;
        float tran_start_t;
        float lead_v;
        float lead_a;
        float pre_v;
        float pre_a;
        float ref_v;
        float ref_a;
        float pltn_vel;
        float pltn_acc;
        float pltn_dcc;
		float tg;
        float front_range;        // Fused from all range measure including communication 
        float front_range_rate;   // Fused from all range_rate measure including communication 
        float rear_range;
        float rear_range_rate;
		float ACC_tGap;
		float ACC_dGap;
		float CACC_tGap;
		float CACC_dGap;
       
        float radar_rg;			// Fused from radar raneg and relative v
        float radar_rt;			// Fused radar rt and relative v		
       
		float max_tq_we;
		float max_jk_we;
        //float mdl_rg;        
} control_state_typ;

typedef struct
{
          float y1;
          float y2;
          float y3;
          float y4;
          float y5;
          float y6;
          float y7;
          float y8;
          float y9;
          float y10;
          float y11;
          float y12;
          float y13;
          float y14;
          float y15;
          float y16;
          float y17;
          float y18;
          float y19;
          float y20;
          int con_sw_1;
          int con_sw_2;
          int con_sw_3;
          int con_sw_4;
          int con_sw_5;         
}con_output_typ;


typedef struct
{
  	 float we;    
	 float w_p;
     float w_t;  
     int eng_mode; 
     float driver_dmd_percent_tq;
     float actual_eng_tq; 
     float eng_tq; 
  	 float eng_dmd_percent_tq;
  	 int   eng_tq_mode;
	 float bp;
	 float brk_pedal_pos;
	 float brk_demand;	 
	 int ebs_brake_switch;
	 float accel_pedal_volt;
	 float accel_pedal_pos;
	 float accel_pedal_pos1;
	 float accel_pedal_pos2;
	 int actual_max_eng_percent_tq;	 
	 float percent_load_at_current_spd;  
	 int road_v_limit_status;
	 float nominal_fric_percent_tq; 
	 float eng_des_Op_v;	 
	 int EBS_sw;
	 int ABS_amber_warning;
	 int ABS_Operation;
	 int anti_lock_active;
	 int ASR_brk_contr_active;
	 int ASR_eng_contr_active;
	 int EBS_red_warning;
	 float jk_selection;
	 float front_axle_spd;
	 float v;
	 float fl_axle_diff;
     float fr_axle_diff;
     float rl_axle_diff;
     float rr_axle_diff;
     float rl_axle2_diff;
     float rr_axle2_diff;
     float CC_set_v;
	 float trans_out_we;
	 float trans_in_we;
	 float percent_clutch_slip;
	 int selected_gear; 
	 float actual_gear_ratio;  
	 float gear;
	 int trans_range_selected;
	 int trans_range_attained;
	 float gear_flt;     
     float fuel_m;
     float instant_fuel_economy;
	 float mean_fuel_economy;
	 float throttle_valve_pos;       
	 float jk_des_percent_tq;   
	 float jk_actual_percent_tq;  	  
	 int jk_driver_dmd_percent_tq;
	 int jk_selection_Non_Eng;     
     float jk_max_percent_tq;      
     float pm;   
	 float lat_accel;
	 float long_accel;
	 float steer_angle;	
	 float yaw_rt;
	 float VP_lat;
	 float VP_long;
}jbus_read_typ;

typedef struct                          // added on 03_18_09
{
	float ego_a;				// from Volvo X-PC
	float ego_v;				// from Volvo X-PC
	int target_avail;			// from Volvo X-PC
	float target_a;				// from Volvo X-PC
	float target_v;				// from Volvo X-PC
	float target_d;				// from Volvo X-PC  
	float vrd_range;
    float vrd_range_rate;
    float lidar_range;
    float lidar_range_rate;
	float mag_space;
    float mag_range;
    float mag_range_rate;
    float gps_range;		
    float gps_range_rate;
    int mag_counter;
    int pre_mag_counter;
    int mag_number;
}sens_read_typ;			            // read externally fused sensor data from Volvo X-PC


/*
typedef struct                          // added on 03_11_09
{
       float tgt_rg;
       float tgt_rt;
       float tgt_mg;
       float tgt_az;
       unsigned short tgt_id;
       unsigned short tgt_lock;
       unsigned short f_mode;
}evrd_in_typ;

typedef struct
{
       float tgt_rg;
       float tgt_rt;
       float tgt_mg;
       float tgt_az;
       unsigned short tgt_id;
       unsigned short tgt_lock;
       unsigned short f_mode;
}evrd_out_typ;*/

/*typedef struct
{
       float long_pos;
       float long_rt;     
       float lat_pos;
       float lat_rt;
       float vert_pos;
       float vert_rt;
       unsigned short tgt_id;
       unsigned short tgt_stat;
       unsigned short f_mode;
}ldr_out_typ;*/


/*typedef struct
{
       float rg;       
       int cnt;
}mdl_out_typ;*/
              
typedef struct {
	double longitude;	// in degrees, +/-
	double latitude;	// in degrees, +/-
	float altitude;	        // in meters, +/-
	double speed;		// in meters/sec
	float heading;		// course made good, true north
} local_gps_typ;

#endif /* VEH_LONG_H */
