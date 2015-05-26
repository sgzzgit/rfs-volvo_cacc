/******************************************************************************
      rad_dist.c
      Based on distance_observer()
      After fault detect and data fusion of two radar systems.
      2nd layer radar fault detect here is for all the radar distance estimation.
      Including primary 2nd layer fault detect of radars
          Using simply fused radar data.                            05_30_03

      Re-written on on                                              03/25/03
      Modified on                                                   05/08/03
      Using 4 different distance filtering strategies               08/01/03
      Make sure radar catch the target in 5[s] for initialization   08/08/03
      Filter 1 changed and tested                                   09/19/03
      Re-written using switch                                       09/27/03
      Initialization roblem for rad_range detected and solved.      09/30/03 
      Run time target loss problem detected and solved              10/01/03

                     by XY_Lu

      
*******************************************************************************/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "veh_long.h"


     
void rad_dist( float delta_t, control_state_typ* con_st_pt, int maneuver_id[2], int pre_maneuver_id[2],
                fault_index_typ* fault_index_pt, int rad_sw)
{
        static float est_dist = 0.0, time = 0.0, k1_gain = 0.2, temp1=0.0;
        static float est_dist_initial=0.0;
        static float radar_f=0.0, radar_f_old=0.0;
        static float prefilter_radar=0.0, prefilter_radar_old=0.0;
        static float radar_rg_old=0.0, alarm_time = 0.0, radar_rg_tmp=0.0;
        static int radar_init1 = 0, radar_init2 = 0;
        static int alarm = 0;
        static float error = 0.0, int_error = 0.0;
        static float spd_bsd_rg=0.0;
        static float tau_radar = 3.0/6.28;
        float radar_rg=0.0, radar_rt=0.0;
        float a,b,c,d,e,f;
       

        // Method 1
        
        a=max(1.0,(con_st_pt-> lidar_range)*(con_st_pt-> lidar_range));
        b=max(1.0,(con_st_pt-> vrd_range)*(con_st_pt-> vrd_range));
        //temp1=0.1*1.5*con_st_pt-> vrd_range+0.9*con_st_pt-> lidar_range;
        //b=max(1.0,temp1*temp1);
        
       if ( (con_st_pt-> vrd_range < 3.0) && (con_st_pt-> lidar_range < 3.0) )
           radar_rg = con_st_pt-> des_f_dist;
        else if ( (con_st_pt-> vrd_range < 3.0) || (con_st_pt-> lidar_range < 3.0) )
           radar_rg = max(con_st_pt-> lidar_range,con_st_pt-> vrd_range);
        else
            {
                radar_rg = (a/(a+b))*(con_st_pt-> vrd_range)+(b/(a+b))*(con_st_pt-> lidar_range);
                radar_rg = 0.2*radar_rg+0.8*(con_st_pt-> lidar_range);                                                                   // added on 05_22_11
           }
            
        c=max(0.001, (con_st_pt-> lidar_range_rate)*(con_st_pt-> lidar_range_rate));
        d=max(0.001,(con_st_pt-> vrd_range_rate)*(con_st_pt-> vrd_range_rate));                
        radar_rt = (c/(c+d))*(con_st_pt-> vrd_range_rate)+(d/(c+d))*(con_st_pt-> lidar_range_rate);
        con_st_pt-> radar_rt = radar_rt;
        time += delta_t;

   switch( rad_sw )
     {
        case 1:              
             con_st_pt-> radar_rg = radar_rg;
             break;          
                              
        case 2:                                
              if (time < 5.0)   // Make sure radar catch the target in 5[s] 
                 radar_rg_tmp = radar_rg;
              e = max(1.0, radar_rg*radar_rg);
              f = max(1.0, radar_rg_tmp*radar_rg_tmp);
              con_st_pt-> radar_rg = (e/(e+f))*radar_rg_tmp + (f/(e+f))*radar_rg;  
              radar_rg_tmp = radar_rg;        
              break;  
         
        case 3:     
                 con_st_pt-> radar_rg =  radar_rg;       
             break; 
        case 4:     
                 con_st_pt-> radar_rg =  radar_rg;                    
                   
           break;  
           
          case 5:
	break; 
     case 6:     
      
           con_st_pt-> radar_rg = est_dist;            
           break;  
     case 7:     
      
           con_st_pt-> radar_rg = est_dist;    // This is incorrect yet 05_30_03        
           break;  
     case 8:     
      
           con_st_pt-> radar_rg = est_dist;    // This is incorrect yet 05_30_03        
           break; 
     }  //  switch END
}

 
