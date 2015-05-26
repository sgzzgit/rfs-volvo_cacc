/************************************************************************************************

              ref_dist.c             

              Distance filtering for vehicle longitrudinal control
              Fusing of radar and mag distance and produce temp_dist
			  Only Radar used for Demo on 12/19/08; needs changing back if lidar is available.
			  Changed made   on  04_24_11;
              
                                By XY_LU May 8, 2003
*************************************************************************************************/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "veh_long.h"
#include "coording.h"


int ref_dist(float delta_t, float t_filter, int man_id[2], vehicle_info_typ* veh_info_pt, 
            fault_index_typ* f_index_pt, control_state_typ* con_st_pt, control_config_typ* config_pt)
{

     //static float lead_speed=0.0, lead_accel=0.0, preceding_speed=0.0;  // preceding_accel=0.0;
     static float distance=0.0, distance_old=0.0, dist_crt_t=0.0, /*real_dist=0.0,*/ range_obs=0.0;           // temp_dist_d=0.0;
     static float eps=0.0, eps_dot=0.0, dist_trans_t=0.0, dist_buff=0.0, initial_dist=0.0;
     static float following_dist=0.0, tmp_dist=0.0, tmp_dist_d=0.0;
     static float mag_start_dist=0.0, mag_distance=0.0;
     const float rate_1=0.5, rate_2=1.0;
     static unsigned short start_sw=1;
     
     const int new_smth =0;                                              // Switch for using new_smooth()  
     
     void cntract(float, float, float, float, float, float *, float *);

     following_dist = con_st_pt-> des_f_dist;   
     range_obs=con_st_pt-> radar_rg;   // from fusion of both lidar and radar;
     //range_obs=con_st_pt-> vrd_range;    // from radar only; 12/18/08
     
 if ( (veh_info_pt-> veh_id == 0) || (veh_info_pt-> veh_id == 1)) 
     {       
            con_st_pt-> temp_dist = con_st_pt-> des_f_dist;                          
            eps_dot = con_st_pt-> spd - con_st_pt-> pre_v;                                         
            if( (man_id[0] != 0) && (man_id[0] != 30) )         
               eps += eps_dot * delta_t;                                           
            distance = con_st_pt-> des_f_dist - eps;                                          
     }
 else
    {     
            if( t_filter < 0.1 ) 
                distance_old = distance;
           
              if( (distance - distance_old) > 0.5)                             // for cars
                distance = distance_old + 0.5;
            else if( (distance - distance_old) < -0.5) 
                distance = distance_old - 0.5;
            distance_old = distance; 
            
           /* if( (distance - distance_old) > 0.25)                              // for trucks  // changed back on 05/22/11
                distance = distance_old + 0.25;
            else if( (distance - distance_old) < -0.25) 
                distance = distance_old - 0.25;
            distance_old = distance; */
                                              
              
            if (t_filter > 7.0)                                 // Added on Aug. 25 02
                {
                    if (dist_crt_t < 6.0)
                       {
                           dist_crt_t +=delta_t;
                           range_obs -= config_pt-> para4*0.15*delta_t/6.0;      // 4.5;0.2                      09_27_03
                       }
                    else
                           range_obs = range_obs-config_pt-> para4*0.15;         // 4.5                      09_27_03
                }
            distance = range_obs + config_pt-> para4*0.15;                       // Should not be removed    08_25_02


          /*  if (t_filter < 1.0)
                {
                    mag_start_dist = range_obs-config_pt-> para4*0.15;            // distance;               08_21_02
                    mag_distance = range_obs-config_pt-> para4*0.15;              // distance;               08_21_02
                }
            else
                mag_distance = mag_start_dist + con_st_pt-> mag_range;     // After 1 [s],  it is independently estimated.
                                  
            if (f_index_pt-> mag_meter == 0)                                // Needs changing
                {
                    distance = 1.0*range_obs+ 0.0*mag_distance; // 0.618, 0.382
                    dist_buff = distance;
                    dist_trans_t = 0.0;
                   // real_dist = con_st_pt-> radar_rg;          //05_07_03                         
                }                
            else
                {
                    if (dist_trans_t < 3.0)
                       {
                          dist_trans_t += delta_t;
                          distance = dist_buff*(3.0-dist_trans_t)/3.0 + (dist_trans_t/3.0)*mag_distance;
                       }
                    else 
                       distance = mag_distance;
                   // real_dist = mag_distance;                       // Fixed                                                 08_21_02                              
                }  */                                                 // Not used yet, 10_02_03                                      
                             
//////////////////////// Mag_dist estimate END
                
       if (start_sw == 1)    // begin start phase
	      {	     
            if (t_filter < 0.05)
                {      
                   initial_dist = distance;                          //10_02_03                    
                   con_st_pt-> temp_dist = initial_dist;
                }

            else
                {    
                   if (new_smth == 0) 
                      con_st_pt-> temp_dist = con_st_pt-> des_f_dist + (initial_dist - con_st_pt-> des_f_dist) * exp(-(t_filter-0.0)/20.0);
                                  
                   if (new_smth == 1)   // New smooth            
                      cntract(delta_t, rate_1, rate_2, distance, following_dist, &tmp_dist, &tmp_dist_d);
                } 
          if  (veh_info_pt-> man_id1 == 7)   // close the door
                	start_sw =0;
        }  // start phase end
      else
           con_st_pt-> temp_dist = con_st_pt-> des_f_dist;       // Added on 12_03_03  for reset after maneuver finished successfully   
    }    // veh_id > 1 end
   
    con_st_pt-> front_range = distance;
        
    return 1;
       
}

