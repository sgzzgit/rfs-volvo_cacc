/*********************************************************************
C^1 connection of two constant value functions. It is used when

                  ini_t <= time_filter <=fin_t
**********************************************************************/

#include <stdio.h>
#include <math.h>

#include "veh_long.h"

#define PI 3.1415927

void smooth(float beta, float time_filter, float ini_t, float fin_t, float ini_value, float fin_val, 
            float *int_val)
{
       *int_val = (fin_val-ini_value)*pow(sin(PI*(time_filter-ini_t)/(2.0*(fin_t-ini_t))), beta);                                     
}

