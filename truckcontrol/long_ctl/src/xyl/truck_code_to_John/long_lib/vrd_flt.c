/*********************************************************************

       vrd_flt.c
       Doppler Radar Filter
       Tested for car platoon at 8,6,4 [m] at RFS
       Re-write the input & output              04_20_03
       Tested OK.                               06_06_03
       Rate changed and tested with Lidar       09_19_03
       DB structurechanged to array;            12_16_08
       
                                By XY_LU May 4, 2000
**********************************************************************/

//#include <math.h>
//#include <stdio.h>
#include <sys_os.h>

#include "veh_long.h"
#include "veh_trk.h"
#include "timestamp.h"
#include "evt300.h"


int vrd_flt(float delta_t, evt300_radar_typ* evrd_pt, evrd_out_typ* evrd_out_pt)
{

   static float tgt_rg_old=0.0, tgt_rt_old=0.0;
   static float tgt_rg_buff=0.0, tgt_rt_buff=0.0;  
 //  static float tgt_az_old=0.0;
 //  static float tgt_mg_old=0.0; 
   static float tgt_rg_d=0.0, tgt_rt_d=0.0;
   static float evrd_start_t=0.0;
   static float rate_lmt=3.0;
   static int evrd_start_s=1, evrd_start_s1=1;
   static short unsigned tgt_id=0,tgt_lock=0;
   static float tgt_rg=0.0, tgt_rt=0.0, tgt_az=0.0, tgt_mg=0.0;

   
   tgt_id = evrd_pt->target[0].id;
   tgt_rg = 0.03048*evrd_pt->target[0].range;                //[m], 1.16  added on Dec. 5, 01
   tgt_rt = 0.03048*evrd_pt->target[0].rate;                 //[m/s]
   tgt_az = 0.002*evrd_pt->target[0].azimuth;                //[rad]
   tgt_mg = -0.543*evrd_pt->target[0].mag;                   //[dB]
   tgt_lock = (int)(evrd_pt->target[0].lock/255);            //1 = locked, 0 = not locked

   tgt_rg_d=(tgt_rg - tgt_rg_old)/delta_t;
   tgt_rt_d=(tgt_rt - tgt_rt_old)/delta_t;

       
    if ( ((fabs(tgt_rg) < 1.5)) && (evrd_start_s == 1))
       {
             tgt_rg_old = tgt_rg;
             tgt_rg_buff = tgt_rg;             
             tgt_rt_old = tgt_rt;             
             tgt_rt_buff = tgt_rt;
             rate_lmt = 30.0;
       }
    else if ( (fabs(tgt_rg) >= 1.5) && (evrd_start_t <0.1) && (evrd_start_s1 == 1) )
       {
             tgt_rg_old = tgt_rg;
             tgt_rg_buff = tgt_rg;             
             tgt_rt_old = tgt_rt;             
             tgt_rt_buff = tgt_rt;
             evrd_start_t += delta_t;
             evrd_start_s=0;
             rate_lmt = 10.0;
              
       }
    else
       {
          evrd_start_s=0;
          evrd_start_s1=0;
          //tgt_rg_d=(tgt_rg - tgt_rg_old)/delta_t;
          //tgt_rt_d=(tgt_rt - tgt_rt_old)/delta_t;

          
          if (fabs(tgt_rg_d)>400.0 || (fabs(tgt_rt)<0.001) || (fabs(tgt_rt_d) < 0.1) )  // or 350.0; To be modified, 06_06_03
             tgt_rg = tgt_rg_old;
          else
           {
              if (tgt_rg > tgt_rg_old+rate_lmt*delta_t)  
                 tgt_rg = tgt_rg_old+rate_lmt*delta_t;
              if (tgt_rg < tgt_rg_old-rate_lmt*delta_t)
                 tgt_rg = tgt_rg_old-rate_lmt*delta_t;

           }
        
          if (tgt_rt > tgt_rt_old+3.0*delta_t)
             tgt_rt = tgt_rt_old+3.0*delta_t;
          if (tgt_rt < tgt_rt_old-3.0*delta_t)
             tgt_rt = tgt_rt_old-3.0*delta_t;
          tgt_rt_old = tgt_rt; 
          tgt_rg_old = tgt_rg;
                    
          evrd_out_pt-> tgt_rg = 0.3*tgt_rg+0.7*tgt_rg_buff;
          evrd_out_pt-> tgt_rt = 0.3*tgt_rt+0.7*tgt_rt_buff;
          tgt_rg_buff=tgt_rg;
          tgt_rt_buff=tgt_rt;
       }
     
     evrd_out_pt-> tgt_az = tgt_az;       // To be filtered yet
     evrd_out_pt-> tgt_mg = tgt_mg;       // To be filtered yet
     evrd_out_pt-> tgt_id = tgt_lock;              // To be filtered yet
     evrd_out_pt-> tgt_lock = tgt_lock;     // To be filtered yet
     evrd_out_pt-> f_mode = 0;              // To be filtered yet

     return 1;
       
}

