

#include <stdio.h>
#include <math.h>
#include <unistd.h>


 
 void filt_2(float h, float tau, float sig0, float sig1, float z0[2], float z[2])
  {
      float x[2];
      float k1,k2,k3,k4,m1,m2,m3,m4;
      
   
     x[0]=z0[0];
     x[1]=z0[1];

     k1=h*x[1];
     m1=h*2.0*(-tau*x[1]-x[0]+sig0)/(tau*tau);
     k2=h*(x[1]+0.5*m1);
     m2=h*2.0*(-tau*(x[1]+0.5*m1)-(x[0]+0.5*k1)+0.5*(sig0+sig1))/(tau*tau);
     k3=h*(x[1]+0.5*m2);
     m3=h*2.0*(-tau*(x[1]+0.5*m2)-(x[0]+0.5*k2)+0.5*(sig0+sig1))/(tau*tau);
     k4=h*(x[1]+m3); 
     m4=h*2.0*(-tau*(x[1]+m3)-(x[0]+k3)+sig1)/(tau*tau);

     z[0]=x[0]+(k1+2.0*k2+2.0*k3+k4)/6.0;
     z[1]=x[1]+(m1+2.0*m2+2.0*m3+m4)/6.0;  
  }

