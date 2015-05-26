/***********************************************************************************************

     dvi()

     To manage driver vehicle interface (DVI)
     This sub-modular will have the higest priority which determines driving mode
         and/or given some driver's command; It determines driving mode and some parameters
         such as max spd, acc, desired distance etc.
     


***********************************************************************************************/
 

#include <stdio.h>

#include "veh_long.h"
#include "coording.h"


int dvi(float global_t, switch_typ* sw_pt, control_state_typ* con_st_pt, vehicle_info_typ* veh_info_pt, int* man_des)
{

     static unsigned short message1_s=ON, message2_s=ON, message3_s=ON;
     static unsigned short time_shift_s=ON;

    
     // To determine transition logic
                 
     if ( global_t <= t_wait )
          {
               if ( (sw_pt-> auto_sw == ON) && (message1_s == ON))
                    {
                         message1_s = OFF;
                         message2_s = OFF;
                         message3_s = ON;
                         //con_st_pt-> drive_mode = 1;                                     
                         //time_shift_s=ON;                                       // To allow multiple transition and tran_ref to write
                         //*man_des = 1;
                         fprintf(stderr, "\n\nAutomatic mode ON!\n\n\n");       // Should passe some info to DVI                                                                                                                  
                    }                  
          }
     else  
          {
               if( (sw_pt-> auto_sw == ON) && (message2_s == ON))                       
                   {
                         message2_s = OFF;
                         message3_s = ON;                               
                         time_shift_s=ON;
                         //con_st_pt-> drive_mode = 2;
                         //veh_info_pt-> veh_id = 1;                              // After transition, veh_id =1;     
                         //con_st_pt-> tran_start_t = global_t;                   // Only sampling at this instant                           
                         //con_st_pt-> manu_speed = (con_st_pt-> spd);            // Vehicle speed at the instant of transition
                         //*man_des = 4; 
                         fprintf(stderr, "\n\nAutomatic mode ON!\n\n\n");       // Should passe some info to DVI                                                      
                                                                                                 
                    }
                      
               if ( (sw_pt-> manu_sw == ON) && (message3_s == ON) )
                    {                                                                         
                         message2_s = ON;                           
                         message3_s = OFF;
                         //time_shift_s=ON;                                       // for debug only  
                         //con_st_pt-> drive_mode = 3;
                         //veh_info_pt-> veh_id = 1;                              // Manual control always have veh_id = 1.
                         //*man_des = 45;                  
                         fprintf(stderr, "\n\nManual mode ON!\n\n\n");          // Should passe some info to DVI                          
                    }                            
         }
                            
     return 1;  
     
}     


