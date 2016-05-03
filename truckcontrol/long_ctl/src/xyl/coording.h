/*********************************************************************************

    coording.h

    Collection of info for coordination manager

    Started									04/03/03
    Updated on								11/25/09
	started to build for trruck CACC		04_07_15



                                 XY_LU

*********************************************************************************/

#ifndef COORDING_H
#define COORDING_H

//#define PLATOON_SIZE                     1
#define RFS                              0
#define CRO                              3
#define N11                               6
#define I15                                 2
#define NVD                              7  // to be activated
#define t_wait                           20.0
#define test_site                         RFS
#define MAX_NUM_PLTN				3
#define MAX_PLTN_LEN				3
#define MAX_GRP_LEN				    10
/* Struct for vehicle info */   // Used for coordination

typedef struct      // To be sent from each vehicle to coordination manager
{
	int	comm_p[MAX_PLTN_LEN];
    int ready;
    int fault_mode;    // Define the level of fault of a vehicle
	int grp_id;
	int pltn_id;
    int veh_id;
    int veh_type;
    int pre_fault_mode;
    int lead_fault_mode;
	//int handshake;        // 0: no handshake; 1: with hand shake with proper vehicles in the same lane; added on 04/19/15
	int cut_in;			  // front cut-in; 0: no cut-in
	int cut_out;		  // from cut_out; 0: no cut-out
    float spd;  
    float man_t_limit;   
	float run_dist;
}vehicle_info_typ;  // for subject veh and the whole pltn

typedef struct      // To be sent from each vehicle to coordination manager
{
    int ready;
    int fault_mode;  // Define the level of fault of a vehicle 
	int grp_id;
    int pltn_id;
    int pltn_size;          
    int pltn_fault_mode;
	int handshake;        // 0: no handshake; 1: with hand shake with proper vehicles in the same lane; added on 04/19/15
	int cut_in_pos[MAX_NUM_PLTN][MAX_PLTN_LEN];  // for manual driven veh; 0: no cut-in; i: cut-in front of vehicle i>0; if more than one vehicle cut-in, difficult to detect
	int cut_out_pos[MAX_NUM_PLTN][MAX_PLTN_LEN]; // for manual driven veh; 0: no cut-out; i: cut-out front of vehicle i>0; 
	int man_id1;
    int man_id2;        
    float man_t_limit;         
}pltn_info_typ;  // for subject veh and the whole pltn

typedef struct      // To be sent from each vehicle to coordination manager
{    
    int fault_mode;        // Define the level of fault of a vehicle   
    int grp_id;
    int grp_size;          // number of connected vehicles in the string       
    int grp_fault_mode;
	int handshake;         // 0: no handshake; 1: with hand shake with other
	int cut_in_pos[MAX_GRP_LEN];  // for manual driven veh; 0: no cut-in; i: cut-in front of vehicle i>0; if more than one vehicle cut-in, difficult to detect
	int cut_out_pos[MAX_GRP_LEN]; // for manual driven veh; 0: no cut-out; i: cut-out front of vehicle i>0; 
	int man_des;   
}group_info_typ;  // for subject veh and the whole pltn


typedef struct      // To be sent from each vehicle to coordination manager
{
	float gps_ref_x;
	float gps_ref_y;
	float grade;
	float postmile;
	float curvature;
	int lane_id;
	int sec_id;
}road_info_typ;

/* Struct of fault mode for communication only */

typedef struct   // From each vehicle to coordination manager
{
    unsigned   pltn_id  :6;
    unsigned   veh_id  :8;
    unsigned   comm_coord   :1;
    unsigned   comm  :1;
    unsigned   CAN_bus  :1;  // for Volvo sensor computer
    unsigned   J_bus  :1;    // for JBus only
    unsigned   radar   :1;
    unsigned   mag_meter  :1; // Determined in mag_dist
    unsigned   gps  :1;
    unsigned   tq  :1;
    unsigned   air_brk  :1;
    unsigned   jake_brk  :1;
    unsigned   trans_rtdr  :1;
    unsigned   wh_spd  :1;
    unsigned   we  :1;
    unsigned   HMI  :1;         
} f_mode_comm_typ;  // fault type of the subject vehicle to be broadcasted through DSRC

typedef struct      // Between vehicles
{
    int comm_counter;   
    float acc_traj;   
    float vel_traj;   
    float temp_dist;  
    float acl;    
    float v;      
    float distance;   
    float run_dist;   
    float fuel_rate;   
    float brk_prs;  
    float jk_tq;  
    float trtd_tq;  
    float pitch;    
    float user_float;       
    int marker_number;  
    int marker_counter; 	
	int driver_mode;
    short unsigned user_ushort_1;   
    short unsigned user_ushort_2;      
    short unsigned lane_id;    
    short unsigned platoon_id;	
    short unsigned maneuver_id; 
    short unsigned fault_mode;     
    short unsigned comm_reply;   // added on 03/06/09
	//local_gps_typ gps;
}comm_info_typ;                  // communication packet to be passed in a DSRC coupling group which my contain several platoons


typedef struct    // From coordination manager to each vehicle
{  
    float global_t;     // For synchronization      
    float stop_dist;
    float user_def_1;   
    float user_def_2;
    float user_def_3;   
    float user_def_4;    
    int user_def_5;   
    int user_def_6;
    int user_def_7;   
    int user_def_8;  
	int auto_contr;
    int man_des;    
    int f_manage_index;             // For platoon fault mode    
	int drive_mode;      // 0-stay, 1-manual,  2-ACC,  3-CACC
	int trans_mode;	    // 0: no transition; 1:manual=>ACC; 2:ACC=>CACC; 3:CACC=> ACC; 4:ACC=> Manual; 5:CACC=>Manual
    int drive_mode_buff; // used as buffer state of previous step
	int following_mode;  // added on 11_15_15
	int control_mode;    // moved from config_typ  03/08/16
	float set_v;
	//float t_gap;
	//float d_gap;
}manager_cmd_typ;   // managing the subject vehicle maneuver within a platoon




#endif  /* COORDING_H */
