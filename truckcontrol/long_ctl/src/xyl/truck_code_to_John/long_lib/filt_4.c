

#include <stdio.h>
#include <signal.h>
#include <math.h>



 
void filt_4(float h, float tau1, float tau2, float sig0, float sig1, float z0[2], float z[2])
  {
      float x[2];
      float k1,k2,k3,k4,m1,m2,m3,m4;
         
     x[0]=z0[0];
     x[1]=z0[1];
 
     k1=h*(-x[0]+ sig0)/tau1;
     m1=h*(-x[1]+ (-x[0]+sig0)/tau1 )/tau2;
     k2=h*(-(x[0]+0.5*k1)+ 0.5*(sig0+sig1) )/tau1;
     m2=h*(-(x[1]+0.5*m1)+( -(x[0]+0.5*k1)+ 0.5*(sig0+sig1))/tau1 )/tau2;
     k3=h*(-(x[0]+0.5*k2)+ 0.5*(sig0+sig1) )/tau1;
     m3=h*(-(x[1]+0.5*m2)+( -(x[0]+0.5*k2)+ 0.5*(sig0+sig1))/tau1 )/tau2;
     k4=h*(-(x[0]+k3)+ sig1)/tau1;    
     m4=h*(-(x[1]+m3)+( -(x[0]+k3)+ sig1)/tau1 )/tau2;

     z[0]=x[0]+(k1+2.0*k2+2.0*k3+k4)/6.0;
     z[1]=x[1]+(m1+2.0*m2+2.0*m3+m4)/6.0;                                             
  }


