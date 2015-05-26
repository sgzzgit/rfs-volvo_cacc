/*********************************************************************************

      f_encode.c
      
      To encode fault mode parameter for comminication 
      
                           begin on 05_22_03

                           by XY_LU
*********************************************************************************/

#include <stdio.h>

#include "coording.h"


void f_encode(f_mode_comm_typ* f_comm_pt, int* f_mode_comm)

{

    static int unit=1, zero=0, buff=0;
    
    if (i_comm_pt-> comm = 1)
       buff = buff | unit;
    else
       buff = buff ^ unit;
    if (i_comm_pt-> comm_coord  = 1)
       buff = buff | (unit << 1);
    else
       buff = buff ^ (unit << 1);
    if (i_comm_pt-> CAN_bus = 1)
       buff = buff | (unit << 2);
    else
       buff = buff ^ (unit << 2);
    if (i_comm_pt-> J_bus = 1)
       buff = buff | (unit << 3);
    else
       buff = buff ^ (unit << 3);    
    if (i_comm_pt-> radar  = 1)
       buff = buff | (unit << 4);
    else
       buff = buff ^ (unit << 4);    
    if (i_comm_pt-> mag_meter = 1)
       buff = buff | (unit << 5);
    else
       buff = buff ^ (unit << 5);     
    if (i_comm_pt-> gps = 1)
       buff = buff | (unit << 6);
    else
       buff = buff ^ (unit << 6);    
    if (i_comm_pt-> throt = 1)
       buff = buff | (unit << 7);
    else
       buff = buff ^ (unit << 7);    
    if (i_comm_pt-> air_brk = 1)
       buff = buff | (unit << 8);
    else
       buff = buff ^ (unit << 8);    
    if (i_comm_pt-> jake_brk = 1)
       buff = buff | (unit << 9);
    else
       buff = buff ^ (unit << 9);    
    if (i_comm_pt-> trans_rtdr = 1)
       buff = buff | (unit << 10);
    else
       buff = buff ^ (unit << 10);    
    if (i_comm_pt-> wh_spd = 1)
       buff = buff | (unit << 11);
    else
       buff = buff ^ (unit << 11);
    if (i_comm_pt-> we = 1)
       buff = buff | (unit << 12);
    else
       buff = buff ^ (unit << 12);    
    if (i_comm_pt-> HMI = 1)
       buff = buff | (unit << 13);
    else
       buff = buff ^ (unit << 13);            

    * f_mode_comm=buff;
    return 1;
}
