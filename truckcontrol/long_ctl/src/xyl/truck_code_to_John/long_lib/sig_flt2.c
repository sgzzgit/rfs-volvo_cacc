

#include <math.h>
#include <stdio.h>


float sig_flt2(float speed)
{
    static float Y[3];
    static float X[3];
    X[2]=speed;
    Y[2]=0.1483*X[0]+0.0553*X[1]+0.1483*X[2]+1.0301*Y[1]-0.3820*Y[0];
    X[0]=X[1];
    X[1]=X[2];
    Y[0]=Y[1];
    Y[1]=Y[2];
    return Y[2];
}

