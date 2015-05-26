/*******************************************************************************************************************************************

    vary_ref.c
    
 Based on tran_ref.c  
 Based on gen_ref.c
 
 con_state. drive_mode=1;    //manual_sw; = 0-stay, 1-auto,  2-auto_manual,  3-manual
 
 
 To dynamically generate reference trajectory for truck.

 To be used with ref_ini.c

 Simulated in truck_ref.

 Modified on               07/16/01
 Compiled on               07/19/01
 Modified on               08/24/01
 Tested   on               03/06/02
 Used Until                03/20/02
 Modified last part on     03/20/02
 Begin to use G on   03/20/02
 To put in trnsition on    03/22/02
 Corrected man_id[0] on    04/09/03
 Changed from switch-loop to if-loop such that it can always be called in all maneuvers  10/22/03 
 man_id = 7 is added for cruise speed    10_27_03 CRO
 Tested for transition at CRO for high sped  OK but not perfect yet                                          10/28/03
 Modified for variable Max_spd based on tran_ref()                                                                       09/06/10
 Removed man_id[0] assignment here; moved to maneuver();                                                  03/26/11
 Both man_id and man_des are used;                                                                                               03/26/11
 Deceleration has been approved;                                                                                                      03/27/11
 
                                                   XY_LU

***********************************************************************************************************************************************/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>


#include "veh_long.h"
#define PI 3.1415927


void vary_ref(float step_t, float t_control, int man_des, float v_point[N_pt], float c[N_pt-1],
               float d[N_pt-1], control_config_typ* config_pt, float grade, 
               control_state_typ *con_st_pt, int man_id[2], float x_ref[2])        
{
    static float a=0.0, v=0.0, v_tmp=0.0;
    static float cruise_t=0.0, t_brk=0.0, acc_t=0.0, dcc_t=0.0, dcc_intv=0.0;
    static int dcc_intv_sw =1;
    static float  a_buff=0.0, t_temp=0.0, t_temp1=0.0, t_smth=0.0;
    static float stop_period, v_final=0.0;
    const  float smth_v=0.8;
   // static float G=0.0;  
    static int acc_s=1, sw_1=ON, max_spd_sw=1;
    int i;
    static float Max_spd=0.0, Max_spd_pre=0.0,Max_dcc=0.0,t_ctrl=0.0, delta_t=0.0; 
   
      Max_spd = config_pt-> max_spd;
      if (Max_spd_pre-config_pt-> max_spd > 0.5)
		  dcc_intv_sw=1;
    
      if (man_id[0] == 4)
         Max_dcc = 0.5*config_pt-> max_dcc;   // for transition
      else
         Max_dcc = config_pt-> max_dcc;

      delta_t = step_t;
      t_ctrl = t_control;
      //Max_acc = con_state-> pltn_acc;      // For pltn 
      //Max_dcc = con_state-> pltn_dcc;
      for (i=0;i<N_pt-1;i++)
           {
                  if (v >= v_point[i] && v < v_point[i+1])                                                    
                        config_pt -> max_acc = c[i]*v+d[i] - G*sin(grade);                                                                                                                  
           }

      
      
      if (con_st_pt-> drive_mode == 3)   // manual; Re-initialize time variables
        {
           cruise_t=0.0;
           t_brk=0.0;
           acc_t=0.0;
           dcc_t=0.0;
           t_temp=0.0;
           t_temp1=0.0;
        }       
       
     if (man_des !=29)
        {
                if (Max_spd < Max_spd_pre-v_threshold)
                        {
                                acc_s=-1;                               
                    			dcc_t=0.0;
                        }
                if (Max_spd > Max_spd_pre+v_threshold)
                        {
                                acc_s=1;
                    			t_temp=0.0;
                    			acc_t=0.0;                            
                        }
                
            if(con_st_pt-> drive_mode == 1)  // auto
              {
                if (acc_s == 1)
                 {  
	                 dcc_t=0.0;                  // added for maultiple decelerations;   03_26_10
                    if (v <=  Max_spd-smth_v)
                      {                   
                        for (i=0;i<N_pt-1;i++)
                                {
                                        if (v >= v_point[i] && v < v_point[i+1])
                                                {                            
                                                        a=c[i]*v+d[i] - G*sin(grade);                         
                                                        v += 0.5*delta_t*(a+c[i]*(v+delta_t*a) + d[i] - G*sin(grade));                                                    
                                                        a_buff=a;
                                                }                                                        
                                }                                                                
                      }
                   else if (v < Max_spd-v_threshold)
                       {                          
                           t_smth=2.0*smth_v/a_buff;
                           if (t_temp <= t_smth)
                              {
                                 t_temp += delta_t;                   
                                 a=-(a_buff/t_smth)*t_temp + a_buff;
                                 v += a*delta_t;                                 
                              }                                                        
                       }        
                   else if ( v > Max_spd-v_threshold)  
                       {                                               
                         a=0.0;
                         v =  Max_spd;
                         cruise_t += delta_t;                                                 
                       }
                        else;                                   
                 }  //(acc_s == 1)      end
                                  
               if (acc_s == -1)                 // Decelerate to Max_spd
                 {
	                 acc_t=0.0;                  // added for maultiple accelerations;   03_26_10
	                dcc_t += delta_t; 
	                if (dcc_intv_sw == 1)
	                {
                    	dcc_intv=0.5*PI*(Max_spd_pre-Max_spd)/Max_dcc;
                    	dcc_intv_sw = 0;
                    }
                    if (dcc_t <= dcc_intv )
                    {
                   		a=-Max_dcc*sin(dcc_t*PI/dcc_intv);
                   		v += a*delta_t;
                     }
                   else
                    {
	                   a=0.0;
	                   v=Max_spd;
                     }
                  //printf("%lf %lf %lf %lf %lf %lf %lf %lf\n", Max_spd, Max_spd_pre, Max_dcc, PI, dcc_intv, dcc_t, a, v);  
                  }   // dcc end                     
              }  // (drive_mode == 1)   end
            
                                
           if (con_st_pt-> drive_mode == 2)  // auto_manual; for transition control
              {
                if (sw_1 == ON)
                  {
                    v = con_st_pt-> spd;
                    v_tmp = con_st_pt-> spd;
                    if (v < Max_spd - v_threshold)
                      acc_s = 1;
                    else if (v > Max_spd + v_threshold)
                      acc_s = -1;
                    else
                      acc_s = 0;                   
                    sw_1 = OFF;
                  }
                if (acc_s == 1)
                  {
                  if (v < Max_spd - 3.0*smth_v)
                    {
	                  dcc_t=0.0;                  // added for maultiple decelerations;   03_26_10
                      acc_t +=delta_t;
                      if (acc_t < 5.0)
                         a=(acc_t/5.0)*config_pt-> max_acc;
                      else
                         a=config_pt-> max_acc;
                      v += a*delta_t;
                    }

                  else if (v <=  Max_spd-smth_v)
                    {                                
                           a=config_pt-> max_acc;
                           v += a*delta_t;
                           a_buff=a;                      
                    }
                  else if (v < Max_spd-v_threshold)
                       {
                           t_smth=2.0*smth_v/a_buff;
                           if (t_temp1 <= t_smth)
                              {
                                 t_temp1 += delta_t;                   
                                 a=-(a_buff/t_smth)*t_temp1 + a_buff;
                                 v += a*delta_t;                                 
                              }                                                        
                       }        
                  else if (v > Max_spd-v_threshold)  
                       {
                         acc_s=0;
                         a=0.0;
                         v =  Max_spd;
                         cruise_t += delta_t;
                       }
                   else
                           {
                           a=0.0;
                           v=Max_spd;
                       }
                   }
                 else if (acc_s == -1)   // Decelerate to Max_spd
                   {
	                   acc_t=0.0;                  // added for maultiple accelerations;   03_26_10
	                   dcc_t += delta_t; 
	                   if (dcc_intv_sw == 1)
	                   {
                    	   dcc_intv=0.5*PI*(Max_spd_pre-Max_spd)/Max_dcc;
                    	   dcc_intv_sw = 0;
                       }
                       if (dcc_t <= dcc_intv )
                       {
                   		  a=-Max_dcc*sin(dcc_t*PI/dcc_intv);
                   		  v += a*delta_t;
                       }
                      else
                      {
	                     a=0.0;
	                     v=Max_spd;
                      }
                   }      // dcc end 
                 else  // (acc_s == 0)
                   {
                      a=0.0;
                      v=Max_spd;                      
                   }
                                                      
              }   // drive_moe == 2 end
              
       
           if(con_st_pt-> drive_mode == 3)
              {
                  a=config_pt->max_acc;  //con_st_pt-> acc;
                  v=con_st_pt-> spd;
                  sw_1 = ON;
              }                    
        }       //(man_des != 29)     end

     else    // (man_des=29)
        {        
            if (con_st_pt-> drive_mode == 1)
              {  
                t_brk+= delta_t;             
                stop_period=2.0*( Max_spd/Max_dcc);                                      

                if (t_brk <= stop_period)                           
                  {  
                     if (t_brk < 0.5*stop_period)  
                       {                     
                        a=(Max_dcc)*(0.5*sin(t_brk*2.0*PI/stop_period +0.5*PI)-0.5) - G*sin(grade);                  
                        v += a*delta_t;
                       }
                     else if (v >=0.1)
                       {
                        a=-Max_dcc;    // In case on a slope
                        v+= a*delta_t;
                       }
                     else
                       {
                        a=0.0;
                        v=v_final;
                       }                                           
                  }
                
                else
                  {                                            
                     a = 0.0;
                     v = v_final;
                  }
              }            
            else if(con_st_pt-> drive_mode == 3)
              {    
                 v=con_st_pt-> spd;
                                                       
                 if (v >=0.1)
                    {
                       a=-Max_dcc;
                       v+=a*delta_t;
                    }
                 else
                    {
                       a=0.0;
                       v=v_final;
                    }              
              }            
           else if (con_st_pt-> drive_mode == 3) 
              {
                  a=config_pt-> max_acc;
                  v=con_st_pt-> spd;
                  sw_1 = ON;
              }
           else;
        }   // (man_des == 29)
                                                                           
    if (con_st_pt-> drive_mode == 3) 
       a = config_pt-> max_acc;
    if (a > config_pt-> max_acc)
       a = config_pt-> max_acc;
    if (a < -Max_dcc)
       a = -Max_dcc;
    x_ref[0]=a;
    x_ref[1]=v;      
 
    Max_spd_pre=config_pt-> max_spd;    // update buffer
}    

