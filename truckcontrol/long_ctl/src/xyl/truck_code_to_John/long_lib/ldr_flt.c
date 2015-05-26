/*********************************************************************
       For Lidar filtering
       All 8 targets are used.
       Tested at CRO for two vehicles and modified on 08_22_02
       Not working yet for truck.
                                By XY_LU May 4, 2000
**********************************************************************/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "veh_long.h"

int ldr_flt(float delta_t, float *lid_rg, float *lid_rt, ldr_out_typ* ldr_out_pt)
{

     static float lidar_rg=0.0, lidar_rg_raw=0.0, lidar_rg_old=0.0, lidar_rg_raw_old=0.0,lidar_rg_d=0.0;
     static float lidar_rt=0.0,  lidar_rt_old=0.0, lidar_rt_raw=0.0, lrd_ini_t=0.0;
     static unsigned short lidar_tgt_id=0, I=0;
     static float x[2]={0.0,0.0}, out_tmp=0.0;
     static float x_old[2]={0.0,0.0};
    
	 
	 for (I=0;I<8;I++)
	    {         
           if(lid_rg[I] > 150.0)
              lid_rg[I] = lid_rg[I]-162.56; 
	    }   

     lrd_ini_t += delta_t;

     if (lrd_ini_t  < 0.1)
           {
             lidar_rg_old = lidar_rg_raw;
             //ldr_out_pt-> long_pos = lidar_rg_raw;             
             lidar_rt_old = lidar_rt_raw;             
             //ldr_out_pt-> long_rt = lidar_rt_raw;            
           }
     else if (lrd_ini_t  < 5.0)
        {
           for (I=0;I<8;I++)
              {
                 if ((lid_rg[I]) > 1.0 && (lid_rg[I]) < 100.0)
                    {
                       lidar_rg_raw=lid_rg[I];
                       lidar_tgt_id=I;
                    }
                 lidar_rt_raw=lid_rt[lidar_tgt_id];
				 
              }
        }
     else
        {        
           for (I=0;I<8;I++)
              {
        
                 lidar_rg_raw = max(lidar_rg_raw, lid_rg[I]);       
                 if (lidar_rg_raw ==  lid_rg[I])
                    lidar_tgt_id=I;                                   
                 lidar_rt_raw=lid_rt[lidar_tgt_id];                                                                                        
              }
        }                      
      
     lidar_rg_d=(lidar_rg_raw - lidar_rg_raw_old)/delta_t;
     lidar_rg_raw_old=lidar_rg_raw;

     if (fabs(lidar_rg_d)>70.0)                                  // or 80.0  Aug. 16
         lidar_rg = lidar_rg_old;
     else 
           { 
               if (lidar_rg_raw > lidar_rg_old+8.5*delta_t)      // 8.5 is too high; 2.5, 2.0, 1.5, 1.2
                 lidar_rg = lidar_rg_old+8.5*delta_t;
               else if (lidar_rg_raw < lidar_rg_old-8.5*delta_t)
                 lidar_rg = lidar_rg_old-8.5*delta_t;
               else
                 lidar_rg = lidar_rg_raw;
           } 
        lidar_rg_old = lidar_rg;

     if (lidar_rt_raw > lidar_rt_old+3.0*delta_t)
        lidar_rt = lidar_rt_old+3.0*delta_t;
     else if (lidar_rt_raw < lidar_rt_old-3.0*delta_t)
        lidar_rt = lidar_rt_old-3.0*delta_t;
     else
        lidar_rt = lidar_rt_raw;
     lidar_rt_old = lidar_rt;
//

        // BT
   /*   x[0]=0.2779*x_old[0] - 0.4152*x_old[1] + 0.5872*lidar_rg;
        x[1]=0.4152*x_old[0] + 0.8651*x_old[1] + 0.1908*lidar_rg;  
        out_tmp = 0.1468*x[0] + 0.6594*x[1] + 0.0675*lidar_rg;  */
     
        // CB        
     x[0]= 0.4320*x_old[0] - 0.3474*x_old[1] + 0.1210*lidar_rg;
     x[1]= 0.3474*x_old[0] + 0.9157*x_old[1] + 0.0294*lidar_rg;  
     out_tmp = 0.4984*x[0] + 2.7482*x[1] + 0.0421*lidar_rg; 


     if (lrd_ini_t  < 3.0) 
        ldr_out_pt-> long_pos = lidar_rg;
     else
        ldr_out_pt-> long_pos = out_tmp;      
        
     x_old[0]=x[0];
     x_old[1]=x[1];
                    
     ldr_out_pt-> long_rt = lidar_rt;   //0.3*lidar_rt+0.7*(*lidar_rt_flt);
     
     ldr_out_pt-> lat_pos=0.0;
     ldr_out_pt-> lat_rt=0.0;
     ldr_out_pt-> vert_pos=0.0;
     ldr_out_pt-> vert_rt=0.0;
     ldr_out_pt-> tgt_id=lidar_tgt_id+1;
     ldr_out_pt-> tgt_stat=0;
     ldr_out_pt-> f_mode=0;
           
     return 1;
       
}

