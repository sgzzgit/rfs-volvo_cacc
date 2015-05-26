/*********************************************************************
       For Lidar filtering
       All 8 targets are used.
       For longitudinal motion only.
       Tested at CRO for two vehicles and modified on 08_22_02
       Range is OK; Rate is not Ok yet.               06_07_03
       Low byte added on                              10_21_03
       
                                By XY_LU May 4, 2000
**********************************************************************/
#include <sys_os.h>

#include "veh_long.h"

int ldr_flt1(float delta_t, float *lid_h_l_rg, float *lid_h_l_rt, ldr_out_typ* ldr_out_pt)
{

     static float lid_rg=0.0, lid_rg_raw=0.0, lid_rg_old=0.0, lid_rg_raw_old=0.0,lid_rg_d=0.0;
     static float lid_rt=0.0,  lid_rt_old=0.0, lid_rt_raw=0.0, lid_ini_t=0.0;
     static float lid_rg_var=0.0, lid_rg_flt=0.0, lid_rt_flt=0.0;
     static unsigned short lid_tgt_id=0, I=0;
     static float x[2]={0.0,0.0}, y[2]={0.0,0.0}, rg_out=0.0, rt_out=0.0;
     static float x_old[2]={0.0,0.0}, y_old[2]={0.0,0.0};

       
     for (I=0;I<8;I++)
        {
          if(lid_h_l_rg[I] > 150.0)
             lid_h_l_rg[I] = lid_h_l_rg[I]-162.56; 
                  //if(lid_h_l_rt[I] > 150.0)
          //   lid_h_l_rt[I] = lid_h_l_rg[I]-162.56;
              //if(lid_h_l_rt[I] < -150.0)
          //   lid_h_l_rt[I] = lid_h_l_rg[I]+162.56;
                                    
        }

     lid_ini_t += delta_t;

     if (lid_ini_t  < 2.0)
        {
           for (I=0;I<8;I++)
              {
                 if (lid_h_l_rg[I] > 1.0 && lid_h_l_rg[I] < 100.0)
                    {
                       lid_rg_raw=lid_h_l_rg[I];
                       lid_tgt_id=I;
                    }
                 lid_rt_raw=lid_h_l_rt[lid_tgt_id];
              }
        }     
    if (lid_ini_t < 5.0)       //10.0
        {
             for  (I=0;I<8;I++)
                  {
                      if (lid_h_l_rg[I] > 1.0 && lid_h_l_rg[I] < 100.0)
                         lid_rg_raw = max(lid_rg_raw, lid_h_l_rg[I]);       
                      if ( fabs(lid_rg_raw - lid_h_l_rg[I])<0.000001)
                         lid_tgt_id=I;                       
                  }
                     lid_rg_raw=lid_h_l_rg[lid_tgt_id];  
             lid_rt_raw=lid_h_l_rt[lid_tgt_id];                // The target for rate choice        
        } 
     else
        {   
             lid_rg_var = 100.0;    //fabs(lid_rg_raw - lid_h_l_rg[0]);               
             for  (I=0;I<8;I++)
                {                                                              
                   lid_rg_var = min(lid_rg_var, fabs(lid_rg_raw_old - lid_h_l_rg[I]));
                   if ( fabs(lid_rg_var - fabs(lid_rg_raw_old -lid_h_l_rg[I]))<0.000001) 
                                   //if ( lid_rg_var == fabs(lid_rg_raw_old -lid_h_l_rg[I]))                                                         
                      lid_tgt_id=I;                                              
                }            
                   
            /* if (fabs(lid_rg_raw_old - lid_rg_raw) > 0.6)      // To remove spikes
                  {
                     for (I=0;I<8;I++)
                         {
                            lid_rg_raw = max(lid_rg_raw, lid_h_l_rg[I]);       
                            if ( fabs(lid_rg_raw - lid_h_l_rg[I])<0.000001)
                               lid_tgt_id=I;                       
                         }
                  } */
              lid_rg_raw=lid_h_l_rg[lid_tgt_id];                           
              lid_rt_raw=lid_h_l_rt[lid_tgt_id];                // The target for rate choice

        }

     if (lid_ini_t < 1.0)
        {
           lid_rg_old = lid_rg_raw;
           lid_rg_flt = lid_rg_raw;             
           lid_rt_old = lid_rt_raw;             
           lid_rt_flt = lid_rt_raw;            
        }       
     lid_rg_d=(lid_rg_raw - lid_rg_raw_old)/delta_t;    
     lid_rg_raw_old = lid_rg_raw;
    
     if (fabs(lid_rg_d)>80.0)   ////// || fabs(lid_rt)<0.001)  // or 400.0  Aug. 16
            lid_rg = lid_rg_old;
     else
        {
           if (lid_rg_raw > lid_rg_old+8.5*delta_t)         // 8.5 before 05_30_03
               lid_rg = lid_rg_old+8.5*delta_t;
           else if (lid_rg_raw < lid_rg_old-8.5*delta_t)
               lid_rg = lid_rg_old-8.5*delta_t;
           else
               lid_rg = lid_rg_raw;
        }         
     lid_rg_old = lid_rg;

     if (lid_rt_raw > lid_rt_old+3.0*delta_t)
         lid_rt = lid_rt_old+3.0*delta_t;
     else if (lid_rt_raw < lid_rt_old-3.0*delta_t)
         lid_rt = lid_rt_old-3.0*delta_t;
     else
         lid_rt = lid_rt_raw;
     lid_rt_old = lid_rt;                        
  
        
        // BT
   /*   x[0]=0.2779*x_old[0] - 0.4152*x_old[1] + 0.5872*lid_rg;
        x[1]=0.4152*x_old[0] + 0.8651*x_old[1] + 0.1908*lid_rg;  
        rg_out = 0.1468*x[0] + 0.6594*x[1] + 0.0675*lid_rg;  */
     
        // CB        
        x[0]= 0.4320*x_old[0] -  0.3474*x_old[1] + 0.1210*lid_rg;
        x[1]= 0.3474*x_old[0] + 0.9157*x_old[1] + 0.0294*lid_rg;  
        rg_out = 0.4984*x[0] + 2.7482*x[1] + 0.0421*lid_rg; 

        y[0]= 0.4320*y_old[0] -  0.3474*y_old[1] + 0.1210*lid_rt;
        y[1]= 0.3474*y_old[0] + 0.9157*y_old[1] + 0.0294*lid_rt;  
        rt_out = 0.4984*y[0] + 2.7482*y[1] + 0.0421*lid_rt;
        
        x_old[0]=x[0];
        x_old[1]=x[1];
        y_old[0]=y[0];
        y_old[1]=y[1];            
        
        if (lid_rt < -100.0)
           lid_rt=lid_rt+162.56;
        if (lid_rt > 100.0)
           lid_rt=lid_rt-162.56;
                   
      /*  if (rt_out < -100.0)
           rt_out=rt_out+162.56;
        if (rt_out > 100.0)
           rt_out=rt_out-162.56;  */              //06_07_03                                    
        
        if (lid_ini_t < 3.0)
           {
              ldr_out_pt-> long_pos = lid_rg;
              ldr_out_pt-> long_rt = lid_rt;  
           }
        else
           {
              ldr_out_pt-> long_pos= rg_out;
              ldr_out_pt-> long_rt = rt_out;
           }           
              
       ldr_out_pt-> lat_pos=0.0;
       ldr_out_pt-> lat_rt=0.0;
       ldr_out_pt-> vert_pos=0.0;
       ldr_out_pt-> vert_rt=0.0;
       ldr_out_pt-> tgt_id=lid_tgt_id+1;         // Practical target ID; Only need to update here.
       ldr_out_pt-> tgt_stat=0;
       ldr_out_pt-> f_mode=0;
           
     return 1;      
}

