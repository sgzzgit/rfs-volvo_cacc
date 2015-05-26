#include <math.h>
#include <stdio.h>

float svg5(float sig_in)
{
    const int i_max=5;
    const float c[5]={-0.200000,0.000000,0.200000,0.400000,0.600000};
            
    static float x[5]={0.0,0.0,0.0,0.0,0.0};                                        
    float y=0.0;
    int i;
    
    x[i_max-1]=sig_in;
    for (i=0;i<i_max;i++)
      y+=c[i]*x[i];
    for (i=0;i<i_max-1;i++)
      x[i]=x[i+1];   
    return y;
}

