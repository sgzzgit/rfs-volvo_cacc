#include <stdio.h>
#include <math.h>
#include <stdlib.h>


#define PI 3.1415927


float sinsat(float Ep, float x)
  {
   if (x>=Ep) 
      return 1.0;
   else  if (x<=-Ep) 
      return -1.0;
   else
      return sin(x*PI/(2.0*Ep));
  }

