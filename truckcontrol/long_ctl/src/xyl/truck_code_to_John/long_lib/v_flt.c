/***************************************************************


      v_flt.c
      
      Filtring and fusion of vehicle speed     04_28_03

***************************************************************/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "veh_long.h"     
  
void v_flt( float delta_t, float time_flt, int man_des, float v1, float v2, float v3, float* v_out, fault_index_typ* f_index_pt_tmp)
{
     
       float v=0.0;
       static float v_final_t=0.0, v_old=0.0, v_flt=0.0;     

          
       if (time_flt < 1.0)        
          v = time_flt*v1;       
       else
          v=v1;        
       if ( (man_des == 29) && (v1 < 1.0) )
          {
             if (v_final_t < 1.5)
                {
                    v_final_t += delta_t;
                    v = (1.0-v_final_t*0.6667)*v1;
                }
             else
                v = 0.0;
          }           
       if (v < 0.0) 
         v=0.0;
       if( time_flt < 0.03 )
           {
               v_old = v;
               v_flt = v;
           }
       else
           {
               if( v - v_old > 0.04 )
                  v_flt = v_old + 0.04;
               else if(v - v_old < -0.04 )
                  v_flt = v_old - 0.04;
               else;
                  v_flt = 0.4*v_old+0.6*v_flt;           // (0.5,0.5)
           }
        *v_out = v_flt;
        v_old=v_flt;
        f_index_pt_tmp-> spd=0;           // Fault mode of speed sensor 
    

}            
