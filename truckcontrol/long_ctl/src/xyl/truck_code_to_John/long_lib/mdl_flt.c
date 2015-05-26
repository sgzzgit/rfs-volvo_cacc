/*********************************************************************
       For MDL Lidar filtering
       
       
                                By XY_LU March 10, 2009
**********************************************************************/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "veh_long.h"
//#include "veh_trk.h"


int mdl_flt(float delta_t, mdl_out_typ mdl_in, control_state_typ* con_st_pt)
{

     static float rg=0.0, rg_raw=0.0, rg_old=0.0, rg_raw_old=0.0,rg_d=0.0;
     static float ini_t=0.0;
     static float rg_var=0.0, rg_flt=0.0;
     static unsigned short tgt_id=0, I=0;
     static float x[2]={0.0,0.0}, rg_out=0.0;
     static float x_old[2]={0.0,0.0};
     static int mdl_cnt=0.0;
     
     ini_t += delta_t;     
     rg_raw=mdl_in. rg;
     mdl_cnt=mdl_in. cnt;                      

     if (ini_t < 1.0)
        {
           rg_old = rg_raw;
           rg_flt = rg_raw;                   
           rg_raw_old = rg_raw;           
        }
     if (rg > 100.0)
     {
         rg_raw=0.0;
         mdl_cnt=0;
     }
     if (rg_raw > 2.5 && rg_raw_old < 0.1)
     
     rg_raw = rg_raw_old;
     
     rg_d=(rg_raw - rg_raw_old)/delta_t;    
     rg_raw_old = rg_raw;
    
     if (fabs(rg_d)>80.0)   ////// || fabs(rt)<0.001)  // or 400.0  Aug. 16
            rg = rg_old;
     else
        {
           if (rg_raw > rg_old+8.5*delta_t)         // 8.5 before 05_30_03
               rg = rg_old+8.5*delta_t;
           else if (rg_raw < rg_old-8.5*delta_t)
               rg = rg_old-8.5*delta_t;
           else
               rg = rg_raw;
        }         
     rg_old = rg;

    
          
        // CB        
        x[0]= 0.4320*x_old[0] -  0.3474*x_old[1] + 0.1210*rg;
        x[1]= 0.3474*x_old[0] + 0.9157*x_old[1] + 0.0294*rg;  
        rg_out = 0.4984*x[0] + 2.7482*x[1] + 0.0421*rg; 

        
        x_old[0]=x[0];
        x_old[1]=x[1];
                    
        if (ini_t < 3.0)           
            con_st_pt-> mdl_rg = rg;                         
        else           
            con_st_pt-> mdl_rg = rg_out;                                    
           
     return 1;
     
}

