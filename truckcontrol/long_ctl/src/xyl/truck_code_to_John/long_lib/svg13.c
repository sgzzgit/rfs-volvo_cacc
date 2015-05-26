
#include <math.h>
#include <stdio.h>

float svg13(float sig_in)
{
    const int i_max=13;
    const float c[13]={-0.120879,-0.087912,-0.054945,-0.021978,0.010989,0.043956,0.076923,
                         0.109890,0.142857,0.175824,0.208791,0.241758,0.274725};
            
    static float x[13]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};                                        
    float y=0.0;
    int i;
    
    x[i_max-1]=sig_in;
    for (i=0;i<i_max;i++)
      y+=c[i]*x[i];
    for (i=0;i<i_max-1;i++)
      x[i]=x[i+1];   
    return y;
}

