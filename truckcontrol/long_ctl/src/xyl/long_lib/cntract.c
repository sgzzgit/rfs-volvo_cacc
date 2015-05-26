/**************************************************************************************
       cntract.c Dynamic real-time contract mapping, two rate to be spplied

                                By XY_LU May 4, 2000
**************************************************************************************/

#include <stdio.h>
#include <math.h>



void cntract(float sample_t, float mag_rate, float e_rate, float a_val, float b_val, 
            float *int_val, float *int_val_d)
{
  static float temp=0.0, temp1=0.0, rate=0.0;

  float SIGN1(float);

  temp=a_val-b_val;
  if (mag_rate <= fabs(temp))
     rate = mag_rate;
  else
     rate = (float)fabs(temp);
  temp1=SIGN1(temp);
  *int_val = a_val - temp1*rate*sample_t*
             (exp(e_rate*temp)-exp(-e_rate*temp))/(exp(e_rate*temp)+exp(-e_rate*temp));
  *int_val_d =  - temp1*rate*sample_t*4.0*
             e_rate/((exp(e_rate*temp)+exp(-e_rate*temp))*(exp(e_rate*temp)+exp(-e_rate*temp)));                                 
}


float SIGN1(float y)
  {
   static float x=0.0;
   x=y;
   if(x>0.0) 
     return 1.0;
   else if (x==0.0) 
     return 0.0;
   else
     return -1.0;
   }         

