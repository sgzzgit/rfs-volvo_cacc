/**************************************************************
 Using linear v-a relation
 Shift speed point is based on CalTrans new spec data
 
 To be used with spd_init only run once

                Simulated 

**************************************************************/

#include <stdio.h>
#include <math.h>

#include "veh_long.h"


void ref_ini(control_config_typ* config_pt, float v_point[N_pt], float c[N_pt-1], float d[N_pt-1])
              
{
    int i=0;
    static float coeff=1.0;
    const float x[N_pt]={  0.0,     1.9,      5.60,   5.80,           //For new Freightliner, from data
                           6.50,     7.20,     12.10,   12.25,               
                           13.25,    14.25,   16.00,    16.40,              
                           17.45,    18.50,   22.40,    23.40,             
                           24.45,    25.50,   26.60,    27.60,
                           28.65,    29.00,   30.10,   31.282,               
                           31.2861}; 

       static float y[N_pt]={ 0.0,   0.0,  0.0,   0.0,      // Reduced on 05_20_03 // 1.05  Old// Reduced in May, 2003; Tested form two vehicles 10_29_03; for 3 vehs SR722 Nevada Sept. 2010
                           0.0,    0.0,  0.0,    0.0,
                           0.0,   0.0,  0.0,    0.0,
                           0.0,    0.0,  0.0,   0.0,
                           0.0,   0.0,  0.0,   0.0,
                           0.0,   0.0,  0.0,   0.0,
                           0.0};  
                                                   
  /* 
               y[N_pt]={  1.26667,   0.55,  0.53,   0.52,      // Reduced on 05_20_03 // 1.05  Old// Reduced in May, 2003; Tested form two vehicles 10_29_03
                           0.32,    0.50,  0.48,    0.44,
                           0.24,   0.40,  0.30,    0.28,
                           0.1,    0.22,  0.15,   0.14,
                           0.05,   0.12,  0.10,   0.10,
                           0.03,   0.07,  0.06,   0.05,
                           0.01};  */
                           
 /*            y[N_pt]={  1.26667,   0.55,  0.53,   0.52,      // Modified to increase cruise time, 10_29_03
                           0.32,    0.50,  0.48,    0.44,        // Used for Oct. 30, 03
                           0.24,   0.40,  0.34,    0.31,
                           0.11,    0.24,  0.16,   0.14,
                           0.05,   0.11,  0.095,   0.095,
                           0.03,   0.07,  0.06,   0.05,
                           0.01}; */

    
     /*    y[N_pt]={  1.1,    0.57,  0.55,   0.53,      // Modified to increase cruise for empty trailer, 10_29_03
                           0.32,    0.52,  0.50,    0.45,
                           0.25,   0.45,  0.40,    0.31,
                           0.18,    0.33,  0.29,   0.17,
                           0.08,   0.12,  0.098,   0.095,
                           0.04,   0.08,  0.07,   0.05,
                           0.012};  */                 // Tested on 12_01_03 for single vehicle


                           
   /*                                                                        // used in CRO, 2003
    if (config_pt-> mass_sw == 0)
      {
           y[0]=1.05;
           y[1]=0.55;
      }
    else if (config_pt-> mass_sw == 1)
       {
           y[0]= 0.65;
           y[1]= 0.55;
       } 
    else if (config_pt-> mass_sw == 2)
       {
           y[0]=0.50;
           y[1]=0.53;
       }
    else if (config_pt-> mass_sw == 3)
       {                                             
           y[0]=0.35;
           y[1]=0.50;
       }
    else; */

   // if (config_pt-> task == 0)     // Modified to increase cruise time for empty trailer, aggressive acceleration; 12_01_03
   //    {
          coeff=0.9;
           
          y[0]=1.1;
          y[1]=0.57;
          y[2]=0.55;
          y[3]=0.53;      
          y[4]=0.32;
          y[5]=0.52;
          y[6]=0.50;
          y[7]=0.45;
          y[8]=0.25;
          y[9]=0.45;
          y[10]=0.40;
          y[11]=0.31;
          y[12]=0.18;
          y[13]=0.33;
          y[14]=0.29;
          y[15]=0.17;
          y[16]=0.08;
          y[17]=0.12;
          y[18]=0.098;
          y[19]=0.095;
          y[20]=0.04;
          y[21]=0.08;
          y[22]=0.07;
          y[23]=0.05;
          y[24]=0.012;
   /*    }
    else if (config_pt-> task == 4)       // transtion
       {
           coeff = 0.9;
           
           y[0]=1.26667;
           y[1]=0.55;
           y[2]=0.53;
           y[3]=0.52;     
           y[4]=0.32;
           y[5]=0.50;
           y[6]=0.48;
           y[7]=0.44;       
           y[8]=0.24;
           y[9]=0.40;
           y[10]=0.34;
           y[11]=0.31;
           y[12]=0.11;
           y[13]=0.24;
           y[14]=0.16;
           y[15]=0.14;
           y[16]=0.05;
           y[17]=0.11;
           y[18]=0.095;
           y[19]=0.095;
           y[20]=0.03;
           y[21]=0.07;
           y[22]=0.06;
           y[23]=0.05;
           y[24]=0.01;
       }
    else                       // Used for Oct. 30, 03         
       {
           coeff = 0.9;
           
           y[0]=1.26667;
           y[1]=0.55;
           y[2]=0.53;
           y[3]=0.52;     
           y[4]=0.32;
           y[5]=0.50;
           y[6]=0.48;
           y[7]=0.44;       
           y[8]=0.24;
           y[9]=0.40;
           y[10]=0.34;
           y[11]=0.31;
           y[12]=0.11;
           y[13]=0.24;
           y[14]=0.16;
           y[15]=0.14;
           y[16]=0.05;
           y[17]=0.11;
           y[18]=0.095;
           y[19]=0.095;
           y[20]=0.03;
           y[21]=0.07;
           y[22]=0.06;
           y[23]=0.05;
           y[24]=0.01;
       }*/
                              
    
    for (i=0;i<=N_pt-2;i++)
       {
          c[i]=coeff*(y[i]-y[i+1])/(x[i]-x[i+1]);
          d[i]=coeff*(-x[i+1]*y[i]+x[i]*y[i+1])/(x[i]-x[i+1]);
       }
    for (i=0;i<=N_pt-1;i++)
       v_point[i]=x[i];
    
}    
    


