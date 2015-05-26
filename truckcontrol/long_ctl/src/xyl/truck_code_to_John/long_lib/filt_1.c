

#include <stdio.h>
#include <math.h>



 void filt_1(float  h, float tau, float  sig0, float  sig1, float  z0, float * z1)
  {   float x;
      float k1,k2,k3,k4;
     
      x=z0; 
   
      k1=h*(-x+sig0)/tau;
      k2=h*(-(x+0.5*k1)+ 0.5*(sig0+sig1) )/tau;
      k3=h*(-(x+0.5*k2)+ 0.5*(sig0+sig1) )/tau;
      k4=h*(-(x+k3)+sig1)/tau;
      
      (*z1)= x+(k1+2.0*k2+2.0*k3+k4)/6.0;      
  }


