#include <math.h>
#include <stdio.h>

float svg3a(float sig_in)
{
    const int i_max=3;
    const float c[3]={-0.166667,0.333333,0.833333};
            
    static float x[3]={0.0,0.0,0.0};                                        
    float y=0.0;
    int i;
    
    x[i_max-1]=sig_in;
    for (i=0;i<i_max;i++)
      y+=c[i]*x[i];
    for (i=0;i<i_max-1;i++)
      x[i]=x[i+1];   
    return y;
}

