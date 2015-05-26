/*****************************************************************************

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
                                                   XY_LU

*****************************************************************************/

#include <stdio.h>
#include <math.h>


#include "veh_long.h"
#define PI 3.1415927


void gen_ref(float step_t, float t_control, int man_des_tmp, float* v_point_tmp, float* c_tmp,
               float* d_tmp, control_config_typ* config_pt_tmp,  float grade_tmp, int *man_id, float x_ref[2])
               
{
    static float a=0.0, v=0.0 ;
    static float cruise_t=0.0, t_brk=0.0, acc_t=0.0, a_buff=0.0, t_temp=0.0, t_smth=0.0;
    static float v_start=1.0, v_final=0.0;
    const  float smth_v=0.8;
    static float g_coeff=0.0;
    static float  delta_t=0.0, t_ctrl=0.0;  
    static int acc_s=1;
    static int i=0, man_des=0;
    static short int sw =1;
    static float Max_spd=0.0, Max_dcc=0.0, stop_period=0.0;
    static float grade=0.0;
    static float * v_point;
    static float * c;
    static float * d;
    static control_config_typ* config_pt;

  
         /*  Definition of man_des:
         *      1 : stay at rest
         *      3 : accelerate to desired speed using your specified profile
         *      29 : decelerate to desired speed using your specified profile
         */

        /* Definition of man_id:
         *      0  : stay at rest
         *      1  : accelerate to desired speed using specified profile
         *      2  : constant speed following (cruising)
         *      3  : have been cruising for more than 5 seconds
         *      4  : deceleration to desired speed         
         *      5  : track a sinosuidal trajectory of specified frequency and amplitude // XY_LU
         *      6  : manual control mode
         *      9  : decelerate to desired speed using specified profile
         *      10 : brake to stop
         */

      
      delta_t=step_t;
      t_ctrl=t_control;
      grade=grade_tmp;
      man_des=man_des_tmp;
      v_point=v_point_tmp;
      c=c_tmp;
      d=d_tmp;
      config_pt=config_pt_tmp;
      
      Max_spd = config_pt -> max_spd;
      Max_dcc = config_pt -> max_dcc;
          
          //printf("max_spd: %2.5f\n", Max_spd);
          //fflush(stdout);

      for (i=0;i<N_pt-1;i++)
           {
                  if (v >= v_point[i] && v < v_point[i+1])                                                    
                        config_pt -> max_acc = c[i]*v+d[i] - g_coeff*sin(grade);                                                                                                                                              
           }
     
          
      if (grade > 0.02)                          
         g_coeff=0.0;     //9.81; 
      else
         g_coeff=0.0;
           
  
 switch( man_des )
        {
        case 0:
                man_id[0] = 0;
                v = v_start;
                a = 0.0;
                break;
        case 3:

                if ((acc_s == 1) && (v <=  Max_spd-smth_v))
                  {
                    man_id[0] = 3;
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
                                        
                                        
//printf("Point_1 in case 3,  man_des, man_id[0], v: %3i\t %3i\t %6.5f\n", man_des, man_id[0], v);                     // For DBG
//fflush(stdout);
                  }

                else if ((acc_s == 1) && (v < Max_spd-0.02))
                       {
                           man_id[0] = 3;     
                           t_smth=2.0*smth_v/a_buff;
                           if (t_temp <= t_smth)
                              {
                                 t_temp += delta_t;                   
                                 a=-(a_buff/t_smth)*t_temp + a_buff;
                                 v += a*delta_t;
                                 acc_t += delta_t;
                                 t_brk=t_ctrl;                                 
                              }                                                                           
                        }        
                else if ( v > Max_spd-0.05 )  
                       {
                         man_id[0] = 5;
                         acc_s=0;
                         a=0.0;
                         v =  Max_spd;
                         cruise_t += delta_t;
                         t_brk=t_ctrl;                                           
                       }
                 else
                       t_brk=t_ctrl;
              //   if (cruise_t >= 5.0)
              //        man_id[0] = 5;                                 
                 break;
                                                           
         case 29:                
                stop_period=2.0*( Max_spd/Max_dcc);

            // Approach 1
            /*    if (t_ctrl < t_brk + 0.5*stop_period)                           
                  {
                      a=( Max_dcc)*(0.5*sin( (t_ctrl-t_brk)*2.0*PI/stop_period +0.5*PI)-0.5) + g_coeff*sin(grade);                  
                      v += a*delta_t;
                      man_id[0] = 29;
                  }
                else
                  {
                      if (v >=0.001)
                         {
                           a=-Max_dcc;
                           v+=a*delta_t;
                           man_id[0] = 29;
                         }
                        else
                         {
                           a=0.0;
                           v=v_final;
                           man_id[0] = 30;
                         }                        
                  }  */                       // Trajectory planning is OK but brake not able to follow, Mar 20 02

               //Appoach 2

               
             /*  if (a > -Max_dcc+0.02 && sw == 1)                           
                  {
                      a=( Max_dcc)*(0.5*sin( (t_ctrl-t_brk)*2.0*PI/stop_period +0.5*PI)-0.5) + g_coeff*sin(grade);                  
                      v += a*delta_t;
                      man_id[0] = 9;
                  }
               else
                  {
                      sw=0;
                      if (v >=0.01)
                         {
                           a=-Max_dcc;
                           v+=a*delta_t;
                           man_id[0] = 9;
                         }
                        else
                         {
                           a=0.0;
                           v=v_final;
                           man_id[0] = 10;
                         }                        
                   }  */                                       //  TBD   Apr 24 02
                           
               // Approach 3
               if (t_ctrl < t_brk + stop_period)                           
                  {  
                     if (v >= 2.0)
                       {                     
                        a=( Max_dcc)*(0.5*sin( (t_ctrl-t_brk)*2.0*PI/stop_period +0.5*PI)-0.5) + g_coeff*sin(grade);                  
                        v += a*delta_t;
                        man_id[0] =29;                                               
                       }
                     else if (v >=0.01)
                       {
                        a=-Max_dcc;    // In case on a slope
                        v+= a*delta_t;
                        man_id[0] = 29;
                       }
                     else
                       {
                        a=0.0;
                        v=v_final;
                        man_id[0] = 30;
                       }                                           
                  }
                
                else
                  {                                            
                     a = 0.0;
                     v = v_final;
                     man_id[0] = 30;                        
                  }                                                   
                break;     
         }

    x_ref[0]=a;
    x_ref[1]=v;  
}    

