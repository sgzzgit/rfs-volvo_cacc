/**************************************************************************************
       Based on  real-time contract mapping
       For spltting and Joining

                                By XY_LU May 18, 2002
**************************************************************************************/

#include <stdio.h>
#include <math.h>
#include "veh_xyl.h"


void cntract(float sample_t, float a_val, float b_val, control_config_typ* config_pt, float y[3])
{
  float temp, temp1,rate;
  float Max_acc, Max_dcc, delta_t;


  float SIGN1(float);

  delta_t=sample_t;
  temp=a_val-b_val;
  Max_acc = config_pt -> max_acc;
  Max_dcc = config_pt -> max_dcc;
  
  if (mag_rate <= fabs(temp))
     rate = fabs(mag_rate);
  else
     rate = fabs(temp);
  temp1=SIGN1(temp);

// Using sinusoidal function here
  
 /*  *int_val = a_val - temp1*rate*delta_t*
             (exp(e_rate*temp)-exp(-e_rate*temp))/(exp(e_rate*temp)+exp(-e_rate*temp));
  *int_val_d =  - temp1*rate*delta_t*4.0*
             e_rate/((exp(e_rate*temp)+exp(-e_rate*temp))*(exp(e_rate*temp)+exp(-e_rate*temp)));   */                              
}


float SIGN1(float x)
  {
   if(x>0.0) 
     return 1.0;
   else if (x==0.0) 
     return 0.0;
   else
     return -1.0;
   }         

