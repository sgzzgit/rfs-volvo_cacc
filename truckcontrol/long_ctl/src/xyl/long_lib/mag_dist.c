/************************************************************************************
       mag_dist.c
       
       The same as mag_dist_obs.c
       
       To estimate the distance  between vehicles due to moving only based on speed,
       preceeding speed and magnet information.  

                                              X_Y_LU
************************************************************************************/

#include <math.h>
#include <stdio.h>

#include "veh_long.h"

void mag_dist( float delta_t, float time_flt, control_state_typ* con_state_pt,
                         float *pre_move_dist, float *move_dist, float *mag_dist)
{
        static float  mag_int_dist=0.0, pre_mag_int_dist=0.0;
        static float pre_mv_dist=0.0, mv_dist=0.0;
        static int mag_count_old = 0, pre_mag_count_old=0;     
        
        const float err_const=0.15;    /* Offset of marker spacing error due to 
                                       installation and magenet sensor. Modify on site. */
        
        if (time_flt < 0.4)    
          {
            mag_count_old = con_state_pt-> mag_counter;
            pre_mag_count_old = con_state_pt-> pre_mag_counter; 
          }
                            
         mv_dist += con_state_pt-> spd*delta_t;
         pre_mv_dist += con_state_pt-> pre_v*delta_t;
         
         
         if (con_state_pt-> mag_counter == mag_count_old)
            mag_int_dist += con_state_pt-> spd*delta_t;            
         else if ( (fabs(mag_int_dist - con_state_pt-> mag_space) <= err_const) &&       // No fault situation
                                     (con_state_pt-> mag_counter != mag_count_old))
            {
              mag_int_dist += con_state_pt-> spd*delta_t;  
              mv_dist = mv_dist - 0.8*mag_int_dist + 0.8*con_state_pt-> mag_space;
              mag_int_dist = 0.0;
              mag_count_old = con_state_pt-> mag_counter;
            }     
                                  
  
         if (con_state_pt-> pre_mag_counter == pre_mag_count_old)
            pre_mag_int_dist += con_state_pt-> pre_v*delta_t;
         else if ( (fabs(pre_mag_int_dist - con_state_pt-> mag_space)<err_const) &&       // No fault situation
                 (con_state_pt-> pre_mag_counter != pre_mag_count_old))
            {
             pre_mag_int_dist += con_state_pt-> pre_v*delta_t;
             pre_mv_dist = pre_mv_dist - 0.8*pre_mag_int_dist + 0.8*con_state_pt-> mag_space;
             pre_mag_int_dist = 0.0;
             pre_mag_count_old = con_state_pt-> pre_mag_counter;
            }
         *move_dist = mv_dist;
         *pre_move_dist = pre_mv_dist;
         *mag_dist = pre_mv_dist - mv_dist;    
}


