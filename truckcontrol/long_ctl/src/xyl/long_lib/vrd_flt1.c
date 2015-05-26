/*********************************************************************

       vrd_flt1.c
       Doppler Radar Filter
       Tested for car platoon at 8,6,4 [m] at RFS
       Re-write the input & output              04_20_03
       Tested OK.                               06_06_03
       EVT-300 struct TYP has been changed;     12_16_08
       
                                By XY_LU May 4, 2000
**********************************************************************/

//#include <math.h>
//#include <stdio.h>
#include <sys_os.h>

#include "veh_long.h"
#include "veh_trk.h"
#include "timestamp.h"
#include "evt300.h"


int vrd_flt1(float delta_t, evt300_radar_typ* evrd_pt, evrd_out_typ* evrd_out_pt)
{

   static float tgt_rg_old=0.0, tgt_rt_old=0.0;
   static float tgt_rg_buff=0.0, tgt_rt_buff=0.0;  
 //  static float tgt_az_old=0.0;
 //  static float tgt_mg_old=0.0; 
   static float tgt_rg_d=0.0, tgt_rt_d=0.0;
   static float evrd_start_t=0.0, rate_lmt=0.0;
   static int evrd_start_s=1, evrd_start_s1=1;
   short unsigned tgt_id,tgt_lock, i=0;
   float tgt_rg, tgt_rt, tgt_az, tgt_mg;

   short unsigned id[7]={0,0,0,0,0,0,0},lock[7]={0,0,0,0,0,0,0};
   float rg[7]={0.0,0.0,0.0,0.0,0.0,0.0,0.0}, rt[7]={0.0,0.0,0.0,0.0,0.0,0.0,0.0};
   float az[7]={0.0,0.0,0.0,0.0,0.0,0.0,0.0}, mg[7]={0.0,0.0,0.0,0.0,0.0,0.0,0.0};
  
   for (i=0;i<7;i++)
      {
         id[i] = evrd_pt->target[i].id;
         rg[i] = 0.03048*evrd_pt->target[i].range;                //[m], 1.16  added on Dec. 5, 01
         rt[i] = 0.03048*evrd_pt->target[i].rate;                 //[m/s]
         az[i] = 0.002*evrd_pt->target[i].azimuth;                //[rad]
         mg[i] = -0.543*evrd_pt->target[i].mag;                   //[dB]
         lock[i] = (int)(evrd_pt->target[i].lock/255);            //1 = locked, 0 = not locked
      }


   // Choose target_id based on rt
   // Target ID will not vary for lane change
   
    for (i=0;i<7;i++)
       {
          if (fabs(rt[i]) > 0.5)
             tgt_id = i;
       }
    tgt_rg=rg[tgt_id];
    tgt_lock=lock[tgt_id];
    tgt_az=az[tgt_id];
    tgt_mg=mg[tgt_id];
    tgt_lock=lock[tgt_id];
   
    
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
             evrd_start_s=0;
             tgt_rg_old = tgt_rg;
             tgt_rg_buff = tgt_rg;             
             tgt_rt_old = tgt_rt;             
             tgt_rt_buff = tgt_rt;
             evrd_start_t += delta_t;
             rate_lmt = 3.0;            
       }
    else
       {             
          evrd_start_s1=0;
          tgt_rg_d=(tgt_rg - tgt_rg_old)/delta_t;
          tgt_rt_d=(tgt_rt - tgt_rt_old)/delta_t;
          
          if (fabs(tgt_rg_d)>350.0 || (fabs(tgt_rt)<0.001) || (fabs(tgt_rt_d) < 0.5) )  // or 400.0
             tgt_rg = tgt_rg_old;
          else
            {
               if (tgt_rg > tgt_rg_old+rate_lmt*delta_t)  
                 tgt_rg = tgt_rg_old+rate_lmt*delta_t;
               if (tgt_rg < tgt_rg_old-5.0*delta_t)
                 tgt_rg = tgt_rg_old-5.0*delta_t;

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
     evrd_out_pt-> tgt_id=tgt_id+1;       // Practical target ID
     evrd_out_pt-> tgt_lock=0;            // To be filtered yet
     evrd_out_pt-> f_mode=0;              // To be filtered yet

     return 1;
       
}

