/*********************************************************************************

    coording.h

    Collection of info for coordination manager

    Started                      04/03/03
    Updated on                   11/25/09

                                 XY_LU

*********************************************************************************/

#ifndef COORDING_H
#define COORDING_H

#define PLATOON_SIZE                     1
#define RFS                              0
#define CRO                              3
#define N11                               6
#define I15                                 2
#define NVD                              7  // to be activated
#define t_wait                           20.0
#define test_site                      RFS            // NVD; addd on 04_14_11

/* Struct for vehicle info */   // Used for coordination

typedef struct      // To be sent from each vehicle to coordination manager
{
    unsigned int ready;
    unsigned int fault_mode;  // Define the level of fault of a vehicle
    unsigned short veh_id;
    unsigned short veh_type;
    unsigned short pltn_id;
    unsigned short pltn_size;          
    unsigned short pre_fault_mode;
    unsigned short lead_fault_mode;
    unsigned short pltn_fault_mode;
    float run_dist;
    float spd;
    int man_id1;
    int man_id2;
    float man_t_limit;         
}vehicle_info_typ;


/* Struct of fault mode for communication only */

typedef struct   // From each vehicle to coordination manager
{
    unsigned   pltn_id  :6;
    unsigned   veh_id  :8;
    unsigned   comm_coord   :1;
    unsigned   comm  :1;
    unsigned   CAN_bus  :1;
    unsigned   J_bus  :1;
    unsigned   radar   :1;
    unsigned   mag_meter  :1; // Determined in mag_dist
    unsigned   gps  :1;
    unsigned   throt  :1;
    unsigned   air_brk  :1;
    unsigned   jake_brk  :1;
    unsigned   trans_rtdr  :1;
    unsigned   wh_spd  :1;
    unsigned   we  :1;
    unsigned   HMI  :1;         
} f_mode_comm_typ;

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
    short unsigned user_ushort_1;   
    short unsigned user_ushort_2;      
    short unsigned lane_id;    
    short unsigned platoon_id; 
    short unsigned maneuver_id; 
    short unsigned fault_mode;     
    short unsigned comm_reply;   // added on 03/06/09
}comm_info_typ;


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
    int man_des;    
    int f_manage_index;   // For platoon fault mode    
}manager_cmd_typ;

#endif  /* COORDING_H */
