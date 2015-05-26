#include <math.h>
#include <stdio.h>

float usyn_flt(float in_dat)

{
   static float x[2]={0.0,0.0}, out_dat=0.0;
   static float x_old[2]={0.0,0.0};
   

   x[0]=0.2779*x_old[0] - 0.4152*x_old[1] + 0.5872*in_dat;
   x[1]=0.4152*x_old[0] + 0.8651*x_old[1] + 0.1908*in_dat;  
   out_dat = 0.1468*x[0] + 0.6594*x[1] + 0.0675*in_dat;
   x_old[0]=x[0];
   x_old[1]=x[1];
   return out_dat;
}


