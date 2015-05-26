/*****************************************************************************

    tran_ref.c
    
 Based on gen_ref.c
 
 To dynamically generate reference trajectory for truck.

 To be used with ref_ini.c

 Simulated in truck_ref.

 Modified on               07/16/01
 Compiled on               07/19/01
 Modified on               08/24/01
 Tested   on               03/06/02
 Used Until                03/20/02
 Modified last part on     03/20/02
 Begin to use g_coeff on   03/20/02
 To put in trnsition on    03/22/02
 Corrected man_id[0] on    04/09/03
 Changed from switch-loop to if-loop such that it can always be called in all maneuvers  10/22/03 
 man_id = 7 is added for cruise speed    10_27_03 CRO
 Tested for transition at CRO for high sped  OK but not perfect yet                      10/28/03
 
                                                   XY_LU

*****************************************************************************/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>


#include "veh_long.h"
#define PI 3.1415927


void tran_ref(float step_t, float t_control, int man_des, float v_point[N_pt], float c[N_pt-1],
               float d[N_pt-1], control_config_typ* config_pt, float grade, control_state_typ *con_st_pt,
               int man_id[2], float x_ref[2])
               
{
    static float a=0.0, v=0.0, v_tmp=0.0; 
    static float cruise_t=0.0, t_brk=0.0, acc_t=0.0, acc_up_t=0.0, dcc_t=0.0, dcc_t_1=0.0;
    static float  a_buff=0.0, t_temp=0.0, t_temp1=0.0, t_smth=0.0;
    static float v_start=1.0, v_final=0.0;
    const  float smth_v=0.8;
    static float g_coeff=0.0;  
    static int acc_s=1, sw_1=ON;
    int i;
    static float Max_spd, Max_dcc, stop_period,t_ctrl, delta_t; // Max_acc
    

  
     //  man_des  and  man_id  should have tha same value:
         

      Max_spd = config_pt-> max_spd;

      if (man_id[0] == 3 || man_id[0] == 4 || man_id[0] == 7)
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
                        config_pt -> max_acc = c[i]*v+d[i] - g_coeff*sin(grade);                                                                                                                  
           }

      if (fabs(grade) < 0.02)                          
         g_coeff=0.0;     //9.81; 
      else
         g_coeff=0.0;
                 
 //printf("before SW: Man_des, Max_spd: %3i\t %3.4f\n", man_des, Max_spd);
 //fflush(stdout);          
  
 //switch( man_des )
 //    {
         
// printf("Man_des, Max_spd: %3i\t %3.4f\n", man_des, Max_spd);
// fflush(stdout);

      
      if (con_st_pt-> drive_mode == 3)   // Re-initialize time variables
        {
           cruise_t=0.0;
           t_brk=0.0;
           acc_up_t=0.0;
           dcc_t=0.0;
           dcc_t_1=0.0;
           t_temp=0.0;
           t_temp1=0.0;
        }
       
    /*  if (man_des == 0 || man_des == 1)
           {
               // man_id[0] = man_des;
                v = v_start;
                a = 0.0;                
           }*/
       
     if (man_des !=29)
           {
         
            if(con_st_pt-> drive_mode == 1)
              {
                if ((acc_s == 1) && (v <=  Max_spd-smth_v))
                  {
                   // man_id[0] = man_des;
                    for (i=0;i<N_pt-1;i++)
                      {
                       if (v >= v_point[i] && v < v_point[i+1])
                         {                            
                           a=c[i]*v+d[i] - g_coeff*sin(grade);                         
                           v += 0.5*delta_t*(a+c[i]*(v+delta_t*a) + d[i] - g_coeff*sin(grade));
                           acc_t += delta_t;
                           a_buff=a;
                         }                                                        
                      }
                    t_brk=t_ctrl;
                    
//printf("Point_1:, drive mode, man_des, man_id[0], v:  %3i\t %3i\t %3i\t %6.5f   ", con_st_pt-> drive_mode, man_des, man_id[0], v);                     // For DBG
//fflush(stdout);                    
                  }
                else if ( (acc_s == 1) && (v < Max_spd-0.02) )
                       {
                          // man_id[0] = man_des;
                           t_smth=2.0*smth_v/a_buff;
                           if (t_temp <= t_smth)
                              {
                                 t_temp += delta_t;                   
                                 a=-(a_buff/t_smth)*t_temp + a_buff;
                                 v += a*delta_t;
                                 acc_t += delta_t;
                              }                                                        
                       }        
                else if ( v > Max_spd-0.05 )  
                       {
                        // man_id[0] = man_des;
                         acc_s=0;
                         a=0.0;
                         v =  Max_spd;
                         cruise_t += delta_t;
                         t_brk=t_ctrl;                                           
                       }
                else;
               //  if (cruise_t >= 5.0)
               //       man_id[0] = man_des;                                 
              }
           
        
            //if (con_state. manu_auto_sw == ON)
           if (con_st_pt-> drive_mode == 2)
              {
               // man_id[0] = man_des;
                if (sw_1 == ON)
                  {
                   // a = con_st_pt-> acc;             
                    v = con_st_pt-> spd;
                    v_tmp = con_st_pt-> spd;
                    if (v < Max_spd - 0.01)
                      acc_s = 1;
                    else if (v > Max_spd + 0.01)
                      acc_s = -1;
                    else
                      acc_s = 0;                   
                    sw_1 = OFF;
                  }
                if (acc_s == 1)
                  {
                  if (v < Max_spd - 3.0*smth_v)
                    {
                      acc_up_t +=delta_t;
                      if (acc_up_t < 5.0)
                         a=(acc_up_t/5.0)*config_pt-> max_acc;
                      else
                         a=config_pt-> max_acc;
                      v += a*delta_t;
                    }

                  else if (v <=  Max_spd-smth_v)
                    {           
                     // for (i=0;i<N_pt-1;i++)
                     // {
                       //if (v >= v_point[i] && v < v_point[i+1])
                       //  {                            
                           //a=c[i]*v+d[i] - g_coeff*sin(grade);                         
                           //v += 0.5*delta_t*(a+c[i]*(v+delta_t*a) + d[i] - g_coeff*sin(grade));
                           //acc_t += delta_t;
                           a=config_pt-> max_acc;
                           v += a*delta_t;
                           a_buff=a;
                       //  }                                                        
                     // }
                    }
                  else if (v < Max_spd-0.02)
                       {
                           t_smth=2.0*smth_v/a_buff;
                           if (t_temp1 <= t_smth)
                              {
                                 t_temp1 += delta_t;                   
                                 a=-(a_buff/t_smth)*t_temp1 + a_buff;
                                 v += a*delta_t;
                                 acc_t += delta_t;
                              }                                                        
                       }        
                  else if (v > Max_spd-0.05)  
                       {
                         man_id[0] = man_des;
                         acc_s=0;
                         a=0.0;
                         v =  Max_spd;
                         cruise_t += delta_t;
                         t_brk=t_ctrl;                                           
                       }
                   else;
                                
                   }
                 else if (acc_s == -1)   // Decelerate to Max_spd
                   {
                      if (v > Max_spd + 4.0)
                         {
                            dcc_t += delta_t;
                            if (dcc_t < 5.0)
                               a= -(dcc_t/5.0)*Max_dcc;
                            else                
                               a = -Max_dcc;              // ??? Using linear decc
                            v += a*delta_t;
                         }
                      else if (v > Max_spd+2.0)
                         {
                            a = -Max_dcc;            
                            v += a*delta_t;
                         }
                       else if ( (v > Max_spd) && (dcc_t_1 < 6.0) )
                         {
                            dcc_t_1 += delta_t;                                                         
                            v = (Max_spd+1.0) - sin(dcc_t_1*PI/12.0);   // spd based //0.125*PI
                            a = -(PI/12.0)*cos(dcc_t_1*PI/12.0);              // derivative of v                                                
                         }
                      else
                         {
                            a=0.0;
                            v=Max_spd;
                         }
                         
                   }       
                 else  // (acc_s == 0)
                   {
                      a=0.0;
                      v=Max_spd;                      
                   }
                                                      
              }
              
       
           if(con_st_pt-> drive_mode == 3)
              {
                  a=config_pt->max_acc;  //con_st_pt-> acc;
                  v=con_st_pt-> spd;
                  sw_1 = ON;
                  man_id[0] = man_des;
              }
            
         
           }
        else if (man_des == 29)
           {
            //if (con_state. auto_sw == ON)
            if (con_st_pt-> drive_mode == 1)
              {               
                stop_period=2.0*( Max_spd/Max_dcc);

            /*    if (t_ctrl < t_brk + 0.5*stop_period)                           
                  {
                      
                     //stop_period should be recalculated.
                    
                      a=( Max_dcc)*(0.5*sin( (t_ctrl-t_brk)*2.0*PI/stop_period +0.5*PI)-0.5) + g_coeff*sin(grade);                  
                      v += a*delta_t;
                      man_id[0] = man_des;
                  }
                else
                  {
                      if (v >=0.001)
                         {
                           a=-Max_dcc;
                           v+=a*delta_t;
                           man_id[0] = man_des;
                         }
                        else
                         {
                           a=0.0;
                           v=v_final;
                           man_id[0] = man_des;
                         }                        
                  }  */                       // Trajectory planning is OK but brake not ale to follow, Mar 20 02

                             //Appoach 2

               
             /*  if (a > -Max_dcc+0.02 && sw == 1)                           
                  {
                      a=( Max_dcc)*(0.5*sin( (t_ctrl-t_brk)*2.0*PI/stop_period +0.5*PI)-0.5) + g_coeff*sin(grade);                  
                      v += a*delta_t;
                      man_id[0] = man_des;
                  }
               else
                  {
                      sw=0;
                      if (v >=0.01)
                         {
                           a=-Max_dcc;
                           v+=a*delta_t;
                           man_id[0] = man_des;
                         }
                        else
                         {
                           a=0.0;
                           v=v_final;
                           man_id[0] = man_des;
                         }                        
                   }  */                                       //  TBD   Apr 24 02
                           
               // Approach 3

                if (t_ctrl < t_brk + stop_period)                           
                  {  
                     //if (v >= 2.0)
                                         if (t_ctrl < t_brk + 0.5*stop_period)  
                       {                     
                        a=( Max_dcc)*(0.5*sin( (t_ctrl-t_brk)*2.0*PI/stop_period +0.5*PI)-0.5) + g_coeff*sin(grade);                  
                        v += a*delta_t;
                        man_id[0] = man_des;                                               
                       }
                     else if (v >=0.1)
                       {
                        a=-Max_dcc;    // In case on a slope
                        v+= a*delta_t;
                        man_id[0] = man_des;
                       }
                     else
                       {
                        a=0.0;
                        v=v_final;
                        man_id[0] = man_des;
                       }                                           
                  }
                
                else
                  {                                            
                     a = 0.0;
                     v = v_final;
                     man_id[0] = man_des;                        
                  }
              }
            //else if (con_state. manu_auto_sw == ON)
            else if(con_st_pt-> drive_mode == 3)
              {    
                 v=con_st_pt-> spd;
                                                       
                 if (v >=0.1)
                    {
                       a=-Max_dcc;
                       v+=a*delta_t;
                       man_id[0] = man_des;
                    }
                 else
                    {
                       a=0.0;
                       v=v_final;
                       man_id[0] = man_des;
                    }              
              }
            //else if (con_state. manu_sw == ON)
           else if (con_st_pt-> drive_mode == 3) 
              {
                  a=config_pt-> max_acc;
                  v=con_st_pt-> spd;
                  sw_1 = ON;
                  man_id[0] = man_des;
              }
            else;                                                 
           }      
     //}
    if (con_st_pt-> drive_mode == 3) 
       a = config_pt-> max_acc;
    if (a > config_pt-> max_acc)
       a = config_pt-> max_acc;
    if (a < -Max_dcc)
       a = -Max_dcc;
    x_ref[0]=a;
    x_ref[1]=v;  
    
//printf("Point_2, drive mode,  man_des, man_id[0], v: %3i\t %3i\t %3i\t %6.5f\n", con_st_pt-> drive_mode, man_des, man_id[0], v);                     // For DBG
//fflush(stdout);        
 
}    

