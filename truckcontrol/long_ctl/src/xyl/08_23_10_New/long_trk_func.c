/*********************************************************************************

     long_trk_func.c - functions for long_trk.c

     comm() has been changed a lot;                           05_09_10
     
     
     
*********************************************************************************/

#include <sys_os.h>
#include <timestamp.h>
#include <coording.h>
#include <evt300.h>
#include "veh_long.h"
#include "path_gps_lib.h"
#include "long_comm.h"
#include "veh_trk.h"
#include "long_ctl.h"
#include "long_trk_func.h"
#include "long_trk.h"
#include "mdl.h"

/* comm - Verifies communication among trucks. Using vehicle_info_pt->pltn_size,
**	comm checks whether a message from each of the other trucks has been
**	received, and whether the timestamp from each truck has changed 
**	correctly. 
**
**	Returns: 0 on success, or a positive bit-mapped integer containing 
**	the identit(ies) of the offending truck(s).
*/
int comm(unsigned short *phandshake_start,int *pvehicle_pip, 
	float *plocal_time, float *pglobal_time, unsigned short *pcomm_prt_sw,  
	unsigned short *pcomm_prt1_sw, unsigned short *psynchrn_sw, 
	unsigned short *pglobal_t_sw, float *pdt, float *ptime_filter,
	control_state_typ *con_state_pt, vehicle_info_typ *vehicle_info_pt,
	comm_info_typ *comm_info_pt, fault_index_typ *f_index_pt,
	unsigned short *pcomm_err_bound,
	veh_comm_packet_t *comm_receive_pt,
	veh_comm_packet_t *comm_send_pt) 
{

	static int comm_counter[MAX_TRUCK]={0,0,0,0,0};
	static int comm_counter_old[MAX_TRUCK]={0,0,0,0,0}; 
	static int comm_f_counter[MAX_TRUCK]={0,0,0,0,0};
	static float comm_time_old[MAX_TRUCK]={0,0,0,0,0};
	unsigned short handshake_start = *phandshake_start;                    // for passing out info
	static unsigned short handshake_start_arr[MAX_TRUCK] = {0,0,0,0,0};    // local only; indicating link with one in the platoon with ID=pip
	//static unsigned short comm_reply_arr[MAX_TRUCK] = {0,0,0,0,0};         // for no use
	int vehicle_pip = *pvehicle_pip;                                       // subject veh ID
	float local_time = *plocal_time;                                       // local time of subject veh
	float global_time = *pglobal_time;                                     // globl time of the latoon
	unsigned short comm_prt_sw = *pcomm_prt_sw;
	unsigned short comm_prt1_sw = *pcomm_prt1_sw;
	unsigned short synchrn_sw = *psynchrn_sw;
	unsigned short global_t_sw = *pglobal_t_sw; 
	float dt = *pdt; 
	float time_filter = *ptime_filter ;
	unsigned short comm_err_bound = *pcomm_err_bound;
	int error = 0;
	int pip;
	static int first_time=1;
    static unsigned short g_t_init_count=0, N_veh_comm=0;
//	static int tmp_ctr = 0;

/******************************************************************************
*******************************************************************************
        Here begins Xiao-Yun's code
*******************************************************************************
******************************************************************************/

    con_state_pt-> comm_coord=OFF;   // Not used since coodination communication is not independent
    
    if (vehicle_info_pt-> pltn_size == 1)
    {
             con_state_pt-> comm_leader=ON;
             con_state_pt-> comm_pre=ON;
             con_state_pt-> comm_back=ON;
             f_index_pt-> comm_leader=0;
             f_index_pt-> comm_pre=0;
             f_index_pt-> comm_back=0;
             handshake_start=ON;
             handshake_start_arr[1]=ON;
             //comm_reply_arr[1]=ON;
             f_index_pt-> comm=0;                                                                                                                       
    } 
    else                                   // Platoon size >= 2
    {  
	     // Cycling through all possible positions-in-platoon using the
	     // pip as an index, and ignoring this vehicle's pip, check to 
	     // see whether a received message's global time has changed.
	     
	    N_veh_comm=0;
	    for(pip = 1; pip <= vehicle_info_pt->pltn_size; pip++) // Logic deal with subject vehicle with each individual veh in platoon
	    {
		   if (local_time <0.03)      // Initialization
              handshake_start_arr[pip] = OFF;
		   if(pip != vehicle_pip)                                        // IF LOOP 1
		   {              
              if (comm_receive_pt[pip]. global_time != comm_time_old[pip])
              {
                 comm_counter[pip]++; 
                 if (comm_counter[pip] > 100) 
                    comm_counter[pip] = 0;                                                               
                 comm_time_old[pip] = comm_receive_pt[pip].global_time;   
                 N_veh_comm++;          
              }
              else   // IF LOOP 2
              {
                 comm_f_counter[pip]++;
                 if ((comm_f_counter[pip] > comm_err_bound) && (comm_prt1_sw == ON) && (handshake_start_arr[pip] == ON))
                 {
                    handshake_start_arr[pip] = OFF;
                    comm_prt1_sw = OFF;
                    comm_prt_sw = ON;
					//vehicle_info_pt->fault_mode = 3;
                    fprintf(stderr, "Communication Error with truck no. %d!\n", pip ); 
                    //f_index_pt-> comm_back=1;  //assigned later
                    //f_index_pt-> comm=1;       //assigned later
				 	error |= 0x01 << pip;
                 }
              }
                            
              
			
              if (comm_counter[pip] != comm_counter_old[pip])               // IF LOOP 2; Problem 1  05_09_10, XYL              
              {
				   if(first_time)
				   {
					first_time = 0;
					fprintf(stderr,"Initializing comm_reply to 1\n");
				   }			   		                                
                   handshake_start_arr[pip] = ON;     // It is on only when it communicates with the veh pip;                                 
                   comm_f_counter[pip] = 0;	
                   comm_counter_old[pip] = comm_counter[pip];       // Update buffer 	
			  }	   
			  else 
			  {
				   handshake_start_arr[pip]  = OFF;
				        
				   //comm_reply_arr[pip] = 0;
                   //comm_receive_pt[pip].user_ushort_1 = 0;
				   //fprintf(stderr, "\ncomm_reply_arr[%d] not set because either:\n    1. (comm_receive_pt[%d].my_pip = %d) != (pip = %d), or\n    2. (comm_receive_pt[%d].pltn_size = %d) != (vehicle_info_pt->pltn_size = %d)\n",
				   //     pip, pip, 
				//		comm_receive_pt[pip].my_pip, 
					//	pip,
					//	pip, 
					//	comm_receive_pt[pip].pltn_size,
					//	vehicle_info_pt->pltn_size);
				    //fprintf(stderr, "Also setting comm_receive_pt[%d].user_ushort_1 to 0 so handshaking is not\nenabled.  Something is wrong with truck %d's configuration, and we should not\nindicate successful handshaking\n", pip, pip);
		      }	
		      	                                 
              //}  // IF LOOP 2
           }  // IF LOOP 1               
	    }     // for loop end
	    
	    
	    if (handshake_start_arr[1] == ON)  
           {                      	
               con_state_pt-> comm_leader=ON;
               f_index_pt-> comm_leader=0;
           }
        else
           {	                             
               con_state_pt-> comm_leader=OFF;
               if ((comm_f_counter[1] > comm_err_bound) && (handshake_start == 1) )
               {
               	  f_index_pt-> comm_leader=1;
               	  f_index_pt-> comm=1;
           	   }           
           }
           
        if (handshake_start_arr[2] == ON)
           {  
               con_state_pt-> comm_pre=ON;               
               f_index_pt-> comm_pre=0;
           }
        else
           {
           	   con_state_pt-> comm_pre=OFF;
           	   if ( (comm_f_counter[2] > comm_err_bound) && (handshake_start == 1) )
           	   {
                  f_index_pt-> comm_pre=1;
                  f_index_pt-> comm=1;
           	   }   
           }
        if (handshake_start_arr[3] == ON)
           {  
               con_state_pt-> comm_back=ON;
               f_index_pt-> comm_back=0;
           }
        else
           {  
               con_state_pt-> comm_back=OFF;
               if ( (comm_f_counter[3] > comm_err_bound) && (handshake_start == 1) )
               {
                  f_index_pt-> comm_back=1;
                  f_index_pt-> comm=1;
           	   }   
           }
    }  // end of pltn size > 1

    
	//for(pip = 1; pip <= vehicle_info_pt->pltn_size; pip++)    
	//{
	//	if(pip != vehicle_pip) 		 
	//	{
	//		handshake_start &= handshake_start_arr[pip];           // assign only when all the others are true
	//		comm_info_pt->comm_reply &= handshake_start_arr[pip];  // changed 05_19_10	
	//	}
	//}
	
	if (N_veh_comm == (vehicle_info_pt->pltn_size - 1) )
	{
		handshake_start = 1;
		comm_info_pt->comm_reply = 1;
	}
//	fprintf(stderr, "Number of vehicles connected is: %i\n", N_veh_comm);
	
//	if (N_veh_comm == (vehicle_info_pt->pltn_size - 1) )   //removed on 08_02_10
	
		
	if ((comm_prt_sw == ON) && (handshake_start == ON))
		{
			comm_prt_sw = OFF;
//			comm_prt1_sw = ON;
			fprintf(stderr, "Handshaking with all ON!\n");
		}

    if (vehicle_pip == 1)
       global_time = local_time;                  // It is the same as t_ctrl
    else                            // When vehicle_pip >1, use leader vehicle timing as global time
       {
         //if ( (synchrn_sw == 1) && (con_state_pt-> comm_leader == ON))
         if ( (synchrn_sw == 1) && (handshake_start == ON))                   // changed 05/20/10
            {
               //global_time = comm_receive_pt[1].global_time;
               //synchrn_sw = 0;
               //global_t_sw = ON;
               
               g_t_init_count++;
               global_time = comm_receive_pt[1].global_time;
               if (g_t_init_count >=3) 
               {
               		synchrn_sw = 0;
               		global_t_sw = ON;
               }
            }
         if (global_t_sw == ON)
            global_time += dt;
       }

    if (f_index_pt-> comm == 0)
        {

             if (vehicle_pip == 1)
			    {
				                        
			    }
             else
                {
                   if ( time_filter<=0.001 )  // To avoid comm remainder problem
                     {
                       con_state_pt-> pre_v = 0.0;
                       con_state_pt-> pre_a = 0.0;
                       con_state_pt-> lead_v = 0.0;
                       con_state_pt-> lead_a = 0.0;
                       con_state_pt-> pre_mag_counter = 0;    // Should come from communication.
                     }
                   else
                     {
                       con_state_pt-> pre_v = comm_receive_pt[vehicle_pip - 1].vel_traj;     //velocity;  // Due to comm error,12_03_03
                       con_state_pt-> pre_a = comm_receive_pt[vehicle_pip - 1].acc_traj;     //accel;
                       con_state_pt-> lead_v = comm_receive_pt[1].vel_traj;
                       con_state_pt-> lead_a = comm_receive_pt[1].acc_traj;
//                     con_state_pt-> pre_mag_counter = comm_receive_pt1-> marker_counter;    // Should come from communication.
                     }
                }
             for (pip=1; pip<= (vehicle_info_pt->pltn_size); pip++)      	
                vehicle_info_pt-> pltn_fault_mode=max_i(vehicle_info_pt-> pltn_fault_mode, comm_receive_pt[pip]. fault_mode);
        }


    // To be updated when communication is setup

    con_state_pt-> pltn_vel=0.0;
    con_state_pt-> pltn_acc=0.0;
    con_state_pt-> pltn_dcc=0.0;
/******************************************************************************
*******************************************************************************
        Here ends Xiao-Yun's code
*******************************************************************************
******************************************************************************/

	*phandshake_start = handshake_start;
	*pvehicle_pip = vehicle_pip;
	*plocal_time = local_time;
	*pglobal_time = global_time;
	*pcomm_prt_sw = comm_prt_sw;
	*pcomm_prt1_sw = comm_prt1_sw;
	*psynchrn_sw = synchrn_sw;
	*pglobal_t_sw = global_t_sw; 
	*pdt = dt; 
	*ptime_filter = time_filter ;

	return error;
}


int read_sens(float *pgear, int *pselected_gear, int *pfan_drive_state,
        float *pactual_gear_ratio, float *ppercent_load_v, float *pv1,
        float *pv2, float *pv3, float *pfl_axle_diff, float *pfr_axle_diff,
        float *prl_axle_diff, float *prr_axle_diff, float *pacl,int *peng_mode,
        float *pwe, float *pfan_spd_est,float *peng_tq,float *pnominal_fric_tq,
        float *pfuel_m1, float *pfuel_m2, float *pfuel_val1_pos,
        float *pfuel_val2_pos,  float *pacc_pedal_pos, int *pjk_mode,
        float *pjk_tq, float *pjk_percent_tq, int *pebs_brake_switch,
        int *pabs_ebs_amber_warning_state, float *pinst_brk_power,
        float *pbrk_pedal_pos, float *pbrk_demand, float *ptrans_rtd_mode,
        float *ptrans_rtd_value, float *ptrans_rtd_volt,
        float *ptime_filter, float *pwe_old, long_vehicle_state *pv,
	long_params *pparams, sens_read_typ* sens_read_pt) {

        float gear = *pgear;
        int selected_gear = *pselected_gear;
        int fan_drive_state = *pfan_drive_state;
        float actual_gear_ratio = *pactual_gear_ratio;
        float percent_load_v = *ppercent_load_v;
        float v1 = *pv1;
        float v2 = *pv2;
        float v3 = *pv3;
        float fl_axle_diff = *pfl_axle_diff;
        float fr_axle_diff = *pfr_axle_diff;
        float rl_axle_diff = *prl_axle_diff;
        float rr_axle_diff = *prr_axle_diff;
        float acl = *pacl;
        int eng_mode = *peng_mode;
        float we = *pwe;
        float fan_spd_est = *pfan_spd_est;
        float eng_tq = *peng_tq;
        float nominal_fric_tq = *pnominal_fric_tq;
        float fuel_m1 = *pfuel_m1;
        float fuel_m2 = *pfuel_m2;
        float fuel_val1_pos = *pfuel_val1_pos;
        float fuel_val2_pos = *pfuel_val2_pos;
        float acc_pedal_pos = *pacc_pedal_pos;
        int jk_mode = *pjk_mode;
        float jk_tq = *pjk_tq;
        float jk_percent_tq = *pjk_percent_tq;
        int ebs_brake_switch = *pebs_brake_switch;
        int abs_ebs_amber_warning_state = *pabs_ebs_amber_warning_state;
        float inst_brk_power = *pinst_brk_power;
        float brk_pedal_pos = *pbrk_pedal_pos;
        float brk_demand = *pbrk_demand;
        float trans_rtd_mode = *ptrans_rtd_mode;                  
        float trans_rtd_value = *ptrans_rtd_value;                
        float trans_rtd_volt = *ptrans_rtd_volt;                  
        float time_filter = *ptime_filter;                        
        float we_old = *pwe_old;                                  
                                                                  
/******************************************************************************
*******************************************************************************
        Here begins Xiao-Yun's code                               
*******************************************************************************
******************************************************************************/
#ifdef LAT_INPUT                                                  
    con_state_pt-> mag_counter = pmarker_pos-> marker_counter;    
    con_state_pt-> mag_number = pmarker_pos-> marker_number;      
#endif                                                            
    gear = pv-> current_gear;                         // from transmission
    selected_gear = pv-> selected_gear;               // from transmission

    fan_drive_state = pv-> fan_drive_state;           // from engine
                                                                  
                                                                  
    actual_gear_ratio = pv-> actual_gear_ratio;       // from transmission

    percent_load_v = pv-> percent_load_current_speed;             
                                                                  
                                                                  
    v2 = pv->vehicle_speed;                      // From Sue's Code, Feb. 25 03
     if (pparams->vehicle_type == VEH_TYPE_TRUCK_BLUE) {          
        v1 = pv-> front_axle_speed*1.03306; //0.979783236;    // 0.979717*2342.5/2340.0; For rented trailer, mass_sw == 6; 12_02_03 //According to the survey by Chris Cherry
        }
     if (pparams->vehicle_type == VEH_TYPE_TRUCK_GOLD) {
        v1 = pv-> front_axle_speed*0.9796992;  //0.975548*2347.8/2340; For rented trailer, 12_02_03  //*0.9981 for gl_rent; //*0.997;                 //0.9921;
               // from ebc2 on D1
        }                                                         
     if (pparams->vehicle_type == VEH_TYPE_TRUCK_SILVR) {
        v1 = pv-> front_axle_speed;
        }                                                         
    fl_axle_diff = pv-> front_left_wheel_relative;    // from ebc2 on D1
    fr_axle_diff = pv-> front_right_wheel_relative;   // from ebc2 on D1
    rl_axle_diff = pv-> rear1_left_wheel_relative;    // from ebc2 on D1
    rr_axle_diff = pv-> rear1_right_wheel_relative;   // from ebc2 on D1
    acl = pv-> long_accel;                                        

    // Engine                                                     
    eng_mode = pv-> engine_mode;                      // from engine
    we = pv-> engine_speed;                                       
                                                                  
    v3=pv-> output_shaft_speed*0.0115577;                         
    fan_spd_est = pv-> estimated_percent_fan_speed;   // If available, can be used to detect FAN power consumption                                

    eng_tq = pv-> engine_torque;                      // from engine
    nominal_fric_tq = pv-> nominal_friction_torque;   // from engine
                                                                  
    fuel_m1 = pv-> fuel_flow_rate1;                               
    fuel_m2 = pv-> fuel_flow_rate2;                               
                                                      // From engine
    fuel_val1_pos = pv-> fuel_valve1_position;                    
    fuel_val2_pos = pv-> fuel_valve2_position;                    
    acc_pedal_pos = pv-> accelerator_pedal_position;              
    // Braking System                                             
    jk_mode = pv-> retarder_mode;                     // from engine retarder
    jk_tq = pv-> retarder_torque;                     // from engine retarder
                                                                  
    jk_percent_tq = pv-> actual_retarder_percent_torque;          
                                                                  
    ebs_brake_switch = pv-> ebs_brake_switch_status;  // from EBS 
                                                                  
    abs_ebs_amber_warning_state = pv-> abs_ebs_amber_warning_state;  // from EBS 
    inst_brk_power = pv-> instantaneous_brake_power;              
                                                                  
    brk_pedal_pos = pv-> brake_pedal_position;                    
                                                                  
    brk_demand = pv-> brake_demand;                   // from EBS 
                                                                  
    trans_rtd_mode = pv-> trans_retarder_mode;                    
    trans_rtd_value = pv-> trans_retarder_value;                  
    trans_rtd_volt = pv-> trans_retarder_voltage;                 
                                                                  
    sens_read_pt-> fuel_m=pv-> fuel_rate*4.0;    // %  To be compared with the calculated value                             
    sens_read_pt-> grade=0.0;
    sens_read_pt-> trans_out_we=(pv-> output_shaft_speed);             // [rpm]
                                                                  
    sens_read_pt-> w_t=pv-> input_shaft_speed;                         // [rpm]
                                                                  
    sens_read_pt-> pm=pv-> boost_pressure;                        
    if (pparams->vehicle_type == VEH_TYPE_TRUCK_BLUE)             
        sens_read_pt-> bp= (pv-> fb_axle + pv-> mb_axle + pv-> rb_axle)*3.33333;
    else                                                          
        sens_read_pt-> bp= (pv-> mb_axle + pv-> rb_axle)*5.0;     
  // Gold: only midlle and rear sensor Ok; front not; For Blue, no reading due to card problem. 08_01_03                                          
  // All three brake transducers are on tractors; None on trailer
    sens_read_pt-> gshift_sw=pv-> shift_in_progress;              
    sens_read_pt-> lockup=pv-> lockup_engaged;                    
    sens_read_pt-> driveline_engaged=pv-> driveline_engaged;      
                                                                  
                                                                  
    if( time_filter < 0.07 )                                      
        {                                                         
                we_old = we;                                      
                sens_read_pt-> we_flt = we;                       
        }                                                         
    else                                                          
        {                                                         
                                                                  
            if ((sens_read_pt-> gshift_sw) == OFF)                
          //04_15_03                                               
                {                                                 
                      if( we - we_old > 5.0 )                     
                            we = we_old + 5.0;                    
                      else if( we - we_old < -5.0 )               
                            we = we_old - 5.0;                    
                 }                                                
            else                                                  
                 {                                                
                      if( we - we_old > 20.0 )                    
                            we = we_old + 20.0;                   
                      else if( we - we_old < -20.0 )              
                            we = we_old - 20.0;                   
                }                                                 
                                                                  
            we_old = we;                                          
            sens_read_pt-> we_flt = we*0.2 + (sens_read_pt-> we_flt)*0.8;
        }                                                         
                                                                  
/******************************************************************************
*******************************************************************************
        Here ends Xiao-Yun's code                                 
*******************************************************************************
******************************************************************************/
        *pgear = gear;                                            
        *pselected_gear = selected_gear;                          
        *pfan_drive_state = fan_drive_state;                      
        *pactual_gear_ratio = actual_gear_ratio;                  
        *ppercent_load_v = percent_load_v;                        
        *pv1 = v1;                                                
        *pv2 = v2;                                                
        *pv3 = v3;                                                
        *pfl_axle_diff = fl_axle_diff;                            
        *pfr_axle_diff = fr_axle_diff;                            
        *prl_axle_diff = rl_axle_diff;                            
        *prr_axle_diff = rr_axle_diff;                            
        *pacl = acl;                                              
        *peng_mode = eng_mode;                                    
        *pwe = we;                                                
        *pfan_spd_est = fan_spd_est;                              
        *peng_tq = eng_tq;                                        
        *pnominal_fric_tq = nominal_fric_tq;                      
        *pfuel_m1 = fuel_m1;                                      
        *pfuel_m2 = fuel_m2;                                      
        *pfuel_val1_pos = fuel_val1_pos;                          
        *pfuel_val2_pos = fuel_val2_pos;                          
        *pacc_pedal_pos = acc_pedal_pos;                          
        *pjk_mode = jk_mode;                                      
        *pjk_tq = jk_tq;                                          
        *pjk_percent_tq = jk_percent_tq;                          
        *pebs_brake_switch = ebs_brake_switch;                    
        *pabs_ebs_amber_warning_state = abs_ebs_amber_warning_state;
        *pinst_brk_power = inst_brk_power;                        
        *pbrk_pedal_pos = brk_pedal_pos;                          
        *pbrk_demand = brk_demand;                                
        *ptrans_rtd_mode = trans_rtd_mode;                        
        *ptrans_rtd_value = trans_rtd_value;                      
        *ptrans_rtd_volt = trans_rtd_volt;                        
        *ptime_filter = time_filter;
        *pwe_old = we_old;                                        
                                                                  
        return 0;                                                 
}

int actuate(long_output_typ *pcmd, float *pengine_reference_torque,
        int *pvehicle_pip, float *pt_ctrl, int *pmaneuver_des, int *maneuver_id,
	con_output_typ* con_output_pt, control_state_typ* con_state_pt,
	control_config_typ* config, control_config_typ* config_pt, 
	long_params *pparams, float *pminimum_torque, 
	long_output_typ *inactive_ctrl, manager_cmd_typ * manager_cmd_pt, switch_typ *sw_pt) {

        float engine_reference_torque = *pengine_reference_torque;
        int vehicle_pip = *pvehicle_pip;
        float t_ctrl = *pt_ctrl;
        int maneuver_des = *pmaneuver_des;
	float minimum_torque = *pminimum_torque;

/******************************************************************************
*******************************************************************************
        Here begins Xiao-Yun's code
*******************************************************************************
******************************************************************************/
     if (con_output_pt-> con_sw_1 == 1)  // Tq cmd
          {
              pcmd->engine_command_mode = TSC_TORQUE_CONTROL;
              if (con_output_pt-> y1 > engine_reference_torque)
                 pcmd->engine_torque = engine_reference_torque;
              else if (con_output_pt-> y1 < minimum_torque)
                 pcmd->engine_torque = minimum_torque;
              else
                 pcmd->engine_torque = con_output_pt-> y1;

              //pcmd->engine_retarder_command_mode = TSC_OVERRIDE_DISABLED;
              pcmd->engine_retarder_command_mode = TSC_TORQUE_CONTROL;   // Use this if possible
              pcmd->engine_retarder_torque = -0.0; // Negative torque, not percentage                                                               

              //pcmd->trans_retarder_command_mode = TSC_OVERRIDE_DISABLED;
              pcmd->trans_retarder_command_mode = TSC_TORQUE_CONTROL;    // Use this if possible                                                    
              pcmd->trans_retarder_value = -0.0; // Negative percentage
                                                                   
              pcmd->brake_command_mode = EXAC_NOT_ACTIVE; //04_09_03
                                                                   
          }
      else                                                         
          {                                                        
              //pcmd->engine_command_mode = TSC_OVERRIDE_DISABLED; 
              pcmd->engine_torque = minimum_torque;                
              pcmd->engine_speed = +0.0;                           
          }                                                        
                                                                   
      if (con_output_pt-> con_sw_2 == 1)  // spd cmd               
          {                                                        
              pcmd->engine_speed = +1000.0;             // Desired speed
                                                                   
          }                                                        
      else                                                         
          {                                                        
              pcmd->engine_speed = +1000.0;                        
          }                                                        
                                                                   
      if (con_output_pt-> con_sw_3 == 1)  //jk_cmd                 
          {                                                        
              pcmd->engine_retarder_command_mode = TSC_TORQUE_CONTROL;
              pcmd->engine_retarder_torque = -con_output_pt-> y17; // Negative torque, not percentage

              //pcmd->engine_command_mode = TSC_OVERRIDE_DISABLED; 

              pcmd->engine_command_mode = TSC_TORQUE_CONTROL;//Use this if possible                                                                 
              pcmd->engine_torque = 0.0;
          }                                                        
      else
          {                                                        
              //pcmd->engine_retarder_command_mode = TSC_OVERRIDE_DISABLED;
              pcmd->engine_retarder_torque = -0.0;                 
          }                                                        
                                                                   
      if (con_output_pt-> con_sw_4 == 1)  //trans rtd cmd          
          {
              pcmd->trans_retarder_command_mode = TSC_TORQUE_CONTROL;
              pcmd->trans_retarder_value = -(con_output_pt-> y10); // Negative percentage                                                           

              pcmd->engine_command_mode = TSC_OVERRIDE_DISABLED;   
              //pcmd->engine_command_mode = TSC_TORQUE_CONTROL; //Use this if possible
              //pcmd->engine_torque = 0.0;
          }                                                        
      else                                                         
          {                                                        
              //pcmd->trans_retarder_command_mode = TSC_OVERRIDE_DISABLED;
              pcmd->trans_retarder_value = -0.0;                   
          }
      if (con_output_pt-> con_sw_5 == 1)  // air brk cmd           
          {                                                        
              pcmd->brake_command_mode = EXAC_ACTIVE;              
                if (pparams->vehicle_type == VEH_TYPE_TRUCK_BLUE)
                      pcmd->ebs_deceleration = - 0.075*(con_output_pt-> y16); //0.48 for bl_rent     // 0.82 for gl_ucr; 10_29_03 for 0.5           
                else
                   {                                               
                      if (con_state_pt-> pre_v > 9.0)
                         pcmd->ebs_deceleration = - 0.05*(con_output_pt-> y16);
   // 09_16_03  //4.5, 4.3 good low spd,4.0,3.0                    
                      else
                         pcmd->ebs_deceleration = - 0.085*(con_output_pt-> y16);
                                                                   
                   }
              //pcmd->engine_command_mode = TSC_OVERRIDE_DISABLED; 
              pcmd->engine_command_mode = TSC_TORQUE_CONTROL; //Use this if possible                                                                
              pcmd->engine_torque = 0.0;
          }                                                        
      else                                                         
          {                                                        
              pcmd->brake_command_mode = EXAC_NOT_ACTIVE;
              pcmd->ebs_deceleration = -0.0;                       
                                                                   
          }                                                        
      if (vehicle_pip == 1)                                         
        {
         if (t_ctrl < t_wait-2.02) //1.35 //2.02                   
          {                                                        
              //pcmd-> fan_override = 1;                           
              //pcmd-> fan_control = 1;                            
              pcmd-> brake_command_mode = EXAC_ACTIVE;
              if (pparams->vehicle_type == VEH_TYPE_TRUCK_SILVR) 
              	pcmd-> ebs_deceleration = -0.68*STOP_BRAKE;    
              else
              	pcmd-> ebs_deceleration = -STOP_BRAKE;           
              //pcmd->engine_command_mode = TSC_OVERRIDE_DISABLED; 
              pcmd-> engine_command_mode = TSC_TORQUE_CONTROL; //Use this if possible                                                               
              pcmd-> engine_torque = 0.0;
          }                                                        
        }                                                          
      else                                                         
        {                                                          
         if (t_ctrl < t_wait-1.75) //1.35 //2.02                   
          {                                                        
              //pcmd-> fan_override = 1;
              //pcmd-> fan_control = 1;                            
              pcmd-> brake_command_mode = EXAC_ACTIVE;  
              if (pparams->vehicle_type == VEH_TYPE_TRUCK_SILVR)            
              	pcmd-> ebs_deceleration = -0.69*STOP_BRAKE;   
              else
              	pcmd-> ebs_deceleration = -STOP_BRAKE;               
              //pcmd->engine_command_mode = TSC_OVERRIDE_DISABLED;
              pcmd-> engine_command_mode = TSC_TORQUE_CONTROL; //Use this if possible                                                               
              pcmd-> engine_torque = 0.0;
          }                                                        
        }                                                          

   /*   if ( (sens_read_pt-> we_flt > 2000.0) || (tr_cmd_off ==ON) || (v2 > config_pt-> max_spd + 1.0))                                             
        {
          tr_cmd_off = ON;                                         
          pcmd->engine_command_mode = TSC_OVERRIDE_DISABLED; //Use this if possible                                                                 
          pcmd->engine_torque = 0.0;
          pcmd->engine_speed = +0.0;                               
                                                                   
        }*/                                                        
                                                                   
      if (maneuver_des == 29)                                      
    // Difference between blue and gold truck, 09_27_03            
        {
           if (vehicle_pip == 1)                                    
              {                                                    
                 if(con_state_pt-> pre_v < 0.8*(config_pt-> max_spd)) //0.8 used on 10_30,03                                                        
                    {
                       pcmd->engine_command_mode = TSC_OVERRIDE_DISABLED; //Use this if possible                                                    
                       pcmd->engine_torque = 0.0;
                    }                                              
                                                                   
                  if (con_state_pt-> pre_v < 4.0)     //2.0        
                    {                                              
                       pcmd->engine_command_mode = TSC_OVERRIDE_DISABLED; //Use this if possible                                                    
                       pcmd->engine_torque = 0.0;
                       pcmd->brake_command_mode = EXAC_ACTIVE;
                       if (pparams->vehicle_type == VEH_TYPE_TRUCK_SILVR)    
                       	pcmd->ebs_deceleration = - 0.82*STOP_BRAKE;
                       else
                       	pcmd->ebs_deceleration = - 2.2*STOP_BRAKE;  // 2.0
                                                                   
                    }
                 else if (con_state_pt-> pre_v < 2.0)
                    {                                              
                       pcmd->engine_command_mode = TSC_OVERRIDE_DISABLED;
   //Use this if possible                                          
                       pcmd->engine_torque = 0.0;
                       pcmd->brake_command_mode = EXAC_ACTIVE;     
                       if (pparams->vehicle_type == VEH_TYPE_TRUCK_SILVR)    
                       	pcmd->ebs_deceleration = - 0.82*STOP_BRAKE;
                       else
                       	pcmd->ebs_deceleration = - 2.4*STOP_BRAKE;  // 2.0                                          
                    }
                 else;
              }                                                    
           else                                                    
              {                                                    
//                 if(con_state_pt-> pre_v < 0.8*(config_pt-> max_spd)) // Added on 12_03_03                                                        
//                    {
//                       pcmd->engine_command_mode = TSC_OVERRIDE_DISABLED;
//                       pcmd->engine_torque = 0.0;                
//                    }                                            
                                                                   
                 if (con_state_pt-> pre_v < 4.0)     //2.0         
                    {                                              
                       pcmd->engine_command_mode = TSC_OVERRIDE_DISABLED; //Use this if possible                                                    
                       pcmd->engine_torque = 0.0;
                       pcmd->brake_command_mode = EXAC_ACTIVE; 
                       if (pparams->vehicle_type == VEH_TYPE_TRUCK_SILVR)    
                       	pcmd->ebs_deceleration = - 0.75*STOP_BRAKE;
                       else                       	
                       	pcmd->ebs_deceleration = - 1.7*STOP_BRAKE;  // 2.0
                                                                   
                    }
                 else if (con_state_pt-> pre_v < 2.0)     //2.0    
                    {                                              
                       pcmd->engine_command_mode = TSC_OVERRIDE_DISABLED; //Use this if possible                                                    
                       pcmd->engine_torque = 0.0;
                       pcmd->brake_command_mode = EXAC_ACTIVE;   
                       if (pparams->vehicle_type == VEH_TYPE_TRUCK_SILVR)    
                       	pcmd->ebs_deceleration = - 0.55*STOP_BRAKE;
                       else                       
                       	pcmd->ebs_deceleration = - 1.1*STOP_BRAKE;  // 2.0
                    }                                              
                 else;                                             
              }                                                    
        }                                                          
      if (maneuver_id[0] == 30)                                    
        {                                                          
          pcmd->engine_command_mode = TSC_OVERRIDE_DISABLED; //Use this if possible
          pcmd->engine_torque = 0.0;
          pcmd->brake_command_mode = EXAC_ACTIVE; 
          if (pparams->vehicle_type == VEH_TYPE_TRUCK_SILVR)    
            pcmd->ebs_deceleration = - 1.2*STOP_BRAKE;
          else                       	              
          	pcmd->ebs_deceleration = - 3.1*STOP_BRAKE;               
                                                                   
        }                                                          
                                                                   
     if(config->static_run == TRUE) {
          pcmd->engine_command_mode = TSC_OVERRIDE_DISABLED; //Use this if possible                                                                 
          pcmd->engine_torque = 0.0;
          pcmd->engine_speed = +0.0;                               
          pcmd->brake_command_mode = TSC_OVERRIDE_DISABLED;        
          //pcmd->ebs_deceleration = 0.0;                          
     }                                                             

     if(config->test_actuators == TRUE) {    // For prelimnary test only
          pcmd-> engine_command_mode = TSC_TORQUE_CONTROL;         // For test only on 11/13/08                                                     
          pcmd-> engine_torque = 400.0;
          pcmd->brake_command_mode = EXAC_ACTIVE;                  
          pcmd->ebs_deceleration = - 2.2;                          
     }                                                             
                                                                   
      if ((con_state_pt-> drive_mode == 3) || (sw_pt-> manu_sw ==1) || 
		(manager_cmd_pt-> f_manage_index==3) || (sw_pt->brake_sw == 1) )
      	  memcpy(pcmd ,inactive_ctrl, sizeof(long_output_typ) );
                                                                   
/******************************************************************************
*******************************************************************************
        Here ends Xiao-Yun's code                                  
*******************************************************************************
******************************************************************************/
        *pengine_reference_torque = engine_reference_torque;       
        *pvehicle_pip = vehicle_pip;                                 
        *pt_ctrl = t_ctrl;                                         
        *pmaneuver_des = maneuver_des;                             
		*pminimum_torque = minimum_torque;
                                                                   
        return 0;                                                  
}                                                                  

int config_sw(int *pread_sw, int *pmanu_auto_sw, 
	unsigned short *phandshake_start, int *pread_sw_old, switch_typ *sw_pt, 
	long_vehicle_state *pv, vehicle_info_typ* vehicle_info_pt, 
	veh_comm_packet_t *comm_receive_pt, unsigned short *pprt1_sw, fault_index_typ* f_ind_pt) {
	
	int read_sw = *pread_sw; 
	int manu_auto_sw = *pmanu_auto_sw; 
	unsigned short handshake_start = *phandshake_start;
	int read_sw_old = *pread_sw_old;
	char pip;
	unsigned char actuator_sw = ON;
	unsigned short prt1_sw = *pprt1_sw;
	int ret_brake = 0;
/******************************************************************************
*******************************************************************************
	Here begins Xiao-Yun's code
*******************************************************************************
******************************************************************************/
     sw_pt-> comp_sw=1;
     sw_pt-> cond_sw=0;
     sw_pt-> steer_sw=1;
     sw_pt-> alt_sw=1;
     //sw_pt-> actuator_sw=1;
     sw_pt-> radar1_sw=1;
     sw_pt-> radar2_sw=1;
     //sw_pt-> brake_sw=1;
     //sw_pt-> throt_sw=1;

     read_sw=long_rdswitch(pv->dig_in);

     ret_brake = long_rdbrake(pv->dig_in);
     if( ret_brake == 1)
	sw_pt->brake_sw = 1;

     if ( (read_sw == 3) && (manu_auto_sw == 0)) // uncommitted state
        {
           sw_pt-> manu_sw = 1;
           manu_auto_sw = 1;
           fprintf(stderr, "Got uncommitted switch state the first time\n");
           f_ind_pt -> sw=0;
         }     
     if ( (read_sw==1) && (sw_pt-> manu_sw==0)) // 0: auto mode, 1: manual mode, 2: request for automatic; -1: error.
        {
           if(sw_pt->manu_sw == 0)
           	fprintf(stderr, "Got manual switch state\n");
           sw_pt-> auto_sw=0;
           sw_pt-> manu_sw=1;
           f_ind_pt -> sw=0;
        }
     if ((read_sw == 2) && (sw_pt-> auto_sw==0))
        {
           if(sw_pt->auto_sw == 0)
           	fprintf(stderr, "Got auto switch state\n");
           sw_pt-> auto_sw=1;
           sw_pt-> manu_sw=0;
           f_ind_pt -> sw=0;
        }
     if (read_sw == -1)
        {
           if( (sw_pt->auto_sw == 1) || (sw_pt->manu_sw == 1) )
           	  fprintf(stderr, "Error in switch state\n");
           //sw_pt-> auto_sw=0;
           //sw_pt-> manu_sw=1;
           f_ind_pt -> sw=1;
        }
//       }
//     else
//        {
//           if ( sw_pt-> auto_sw == 1 && read_sw == 1)           
//              {                                                 
//                 sw_pt-> manu_sw=1;                             
//                 sw_pt-> auto_sw=0;                             
//              }                                                 
//                                                                
//           if ( sw_pt-> manu_sw == 1 && read_sw == 0)           
//              {                                                 
//                 sw_pt-> auto_sw=1;                             
//                 sw_pt-> manu_sw=0;                             
//                                                                
//              }                                                 
//        }                                                       
                                                                  
                                                                  
     if (vehicle_info_pt-> pltn_size == 1)    // This is not used in control yet;   12_01_09                                                      
       {
           if (sw_pt-> auto_sw == 1)                              
              sw_pt-> actuator_sw = ON;                           
       }                                                          
     else                                                         
       {                                                          
//	if ( (handshake_start == ON) && (sw_pt-> auto_sw == 1) && 
//		(comm_receive_pt[1]. user_ushort_2 == 1) && 
//		(comm_receive_pt[2]. user_ushort_2 == 1) && (prt1_sw == ON) )
                  // For 3 veh                                    
		actuator_sw = ON;
		for(pip  = 1; pip <= vehicle_info_pt->pltn_size; pip++) {
			if( pip != vehicle_info_pt->veh_id ) {
//fprintf(stderr, "handshake_start %d sw_pt->auto_sw %d prt1_sw %d comm_receive_pt[%d].user_ushort_2 %d actuator_sw %d sw_pt->actuator_sw %d\n", 	
//			handshake_start,
//			sw_pt->auto_sw,
//			prt1_sw,
//			pip,
//			comm_receive_pt[pip].user_ushort_2,
//			actuator_sw,
//			sw_pt->actuator_sw);
           			if ( (handshake_start == ON) && 
				    (sw_pt->auto_sw == 1) && 
				    (prt1_sw == ON) && 
				    (comm_receive_pt[(int)pip].user_ushort_2==1))
               				actuator_sw &= ON;
     				//if (sw_pt-> manu_sw == 1) 
           			else                                // 11_20_09
              				actuator_sw = OFF;
			}
		}
		sw_pt-> actuator_sw = actuator_sw;
       }                                                          
                                                                  
     read_sw_old=read_sw;                                         

/******************************************************************************
*******************************************************************************
	Here ends Xiao-Yun's code
*******************************************************************************
******************************************************************************/
	*pread_sw = read_sw; 
	*pmanu_auto_sw = manu_auto_sw; 
	*phandshake_start = handshake_start;
	*pread_sw_old = read_sw_old;
     return 0;      
}

int set_init_leds(db_clt_typ *pclt, unsigned short *pprt1_sw, 
	unsigned short *pprt_buff, unsigned short *phandshake_start, 
	switch_typ *sw_pt, vehicle_info_typ* vehicle_info_pt,
        veh_comm_packet_t *comm_receive_pt, control_state_typ *con_state_pt, 
	manager_cmd_typ *manager_cmd_pt ) {

	unsigned short prt1_sw = *pprt1_sw;
	unsigned short prt_buff = *pprt_buff;
	unsigned short handshake_start = *phandshake_start;
	unsigned short user_ushort_2 = 1;
	int pip;

/******************************************************************************
*******************************************************************************
	Here begins Xiao-Yun's code for "Starting mode"
*******************************************************************************
******************************************************************************/
    if (sw_pt-> manu_sw == 1)
    {
	    if (long_setled(pclt, FLT_HI) != 0)
               fprintf(stderr, " Setting FLT_HI fail 2!\n"); 
        vehicle_info_pt-> ready = 0;
        user_ushort_2=0;
    }   
    else if (sw_pt-> auto_sw == 1)
    {                  
        if (long_setled(pclt, FLT_AUTO) != 0)
           fprintf(stderr, " Setting FLT_AUTO fail! \n");
        user_ushort_2 = 1;
    } 
    else;          

	
	for(pip = 1; pip <= vehicle_info_pt->pltn_size ; pip++) 
	{
		if(pip != vehicle_info_pt->veh_id) 
		{
			user_ushort_2 &= comm_receive_pt[pip].user_ushort_2;
		}
	}
	if ( (handshake_start == ON) && (sw_pt->auto_sw == 1) && (user_ushort_2 == 1) ) 
	{
		if (long_setled(pclt, FLT_RDY2ROLL) != 0)
			fprintf(stderr, " Setting FLT_RDY2ROLL fail! \n");
		vehicle_info_pt-> ready = 1;
		if (prt1_sw == ON) 
		{  
			prt_buff ++;        
			if (prt_buff > 2) 
			{
				fprintf(stderr, "\nPut to DRIVE Position! \n");
				prt1_sw = OFF;
			} 
		}
	}
      if ((con_state_pt-> drive_mode == 3) || (sw_pt-> manu_sw ==1) ||
                (manager_cmd_pt-> f_manage_index==3) || (sw_pt->brake_sw == 1) )
    {
	    if (long_setled(pclt, FLT_HI) != 0)
               fprintf(stderr, " Setting FLT_HI fail 2!\n"); 
        vehicle_info_pt-> ready = 0;
        user_ushort_2=0;
    }   

/******************************************************************************
*******************************************************************************
	Here ends Xiao-Yun's code
*******************************************************************************
******************************************************************************/
	*pprt1_sw = prt1_sw;
	*pprt_buff = prt_buff;
	*phandshake_start = handshake_start;
	return 0;     
}

int set_time_sync(float *pt_ctrl, float *pdt, 
	float *ptime_filter, int *pvehicle_pip, 
	vehicle_info_typ* vehicle_info_pt) {

    float t_ctrl = *pt_ctrl;     
    float dt = *pdt; 
    float time_filter = *ptime_filter; 
    int vehicle_pip = *pvehicle_pip;
    static int t_ctrl_ini=ON;
/******************************************************************************
*******************************************************************************
	Here begins Xiao-Yun's code
*******************************************************************************
******************************************************************************/

    if (vehicle_info_pt-> pltn_size == 1)    // Changed on 11_14_08
       {
          if (vehicle_info_pt-> ready == 1)
             t_ctrl +=dt;
       }
 /*   else
       {    
          if (vehicle_info_pt-> ready == 1)
             {
                if (vehicle_pip == 1)
                   t_ctrl += dt;
                else if (vehicle_pip == 2)
                   {
	                  if (t_ctrl_ini == ON)
                      {
                      	t_ctrl = t_ctrl_1;
                      	t_ctrl_ini = OFF;
                  	  }
                      t_ctrl += dt;
                                         
                   }  
                else if (vehicle_pip == 3)
                   {                    
                      if (t_ctrl_ini == ON)
                      {
                      	t_ctrl = t_ctrl_2;
                      	t_ctrl_ini = OFF;
                  	  }
                      t_ctrl += dt;
                      if (t_ctrl <= 0.0)
                      	t_ctrl = 0.0;
                   }
                else;        
             }
       } */     // Cahnged on 08_02_10

    if (vehicle_info_pt-> ready == 1)
             t_ctrl +=dt;
 
              
         
    if ( t_ctrl > t_wait )
       time_filter += dt; 

/******************************************************************************
*******************************************************************************
	Here ends Xiao-Yun's code
*******************************************************************************
******************************************************************************/
    *pt_ctrl = t_ctrl; 
    *pdt = dt; 
    *ptime_filter = time_filter; 
    *pvehicle_pip = vehicle_pip;
     return 0;       
}

int process_sigs(float *pdt, float *prun_dist, float *pv, float *pacl, 
	float *pacl_old, float *lid_hi_rg, float *lid_hi_rt, int *maneuver_id, 
	int *pre_maneuver_id, const int *pradar_sw, 
	vehicle_info_typ* vehicle_info_pt, control_config_typ *config, 
	control_state_typ* con_state_pt, evrd_out_typ* evrd_out_pt, 
	ldr_out_typ* ldr_out_pt, evt300_radar_typ *pvor_radar, 
	fault_index_typ* f_index_pt, mdl_lidar_typ *pmdl_lidar, 
	long_params *pparams, float global_time) {

	float dt = *pdt; 
	float run_dist = *prun_dist; 
	float v = *pv; 
	float acl = *pacl; 
	float acl_old = *pacl_old; 
	int radar_sw = *pradar_sw;
	static mdl_out_typ ldr_mdl;
/******************************************************************************
*******************************************************************************
	Here begins Xiao-Yun's code
*******************************************************************************
******************************************************************************/

     if(config->static_run == TRUE)
        run_dist += con_state_pt-> lead_v * dt;
     else
        run_dist += v * dt;

     vehicle_info_pt-> run_dist = run_dist;

     acl = 0.15 * acl + 0.85 * acl_old;
     acl_old = acl;


     if ( ((vehicle_info_pt-> veh_id != 0)  && (config->truck_platoon == FALSE))
||    // This logic needs testing 01_27_10
          ((vehicle_info_pt-> veh_id != 0)  && (config->truck_platoon == TRUE) &&
            ( vehicle_info_pt-> veh_id != 1)) ) {

        if(config->eaton_radar == TRUE) {
            if (vrd_flt(dt, pvor_radar, evrd_out_pt) != 1)   // Tested OK,06_06
                fprintf(stderr, " Calling radar_filter fail! \n");
                                                                  
          //  if (vrd_flt1(dt, pvor_radar, evrd_out_pt) != 1)     
          //      fprintf(stderr, "Calling radar_filter fail! \n");
                                                                  
            con_state_pt-> vrd_range=evrd_out_pt-> tgt_rg;        
            con_state_pt-> vrd_range_rate=evrd_out_pt-> tgt_rt;   
            f_index_pt-> e_vrd = evrd_out_pt-> f_mode;            
                                                                  
        }                                                         
                                                                  
        if(pparams->denso_lidar == 1) {                             
//            if (ldr_flt(dt, lid_hi_rg, lid_hi_rt, ldr_out_pt) != 1)
//               fprintf(stderr, " Calling Lidar_filter fail! \n");
                                                                  
            if (ldr_flt1(dt, lid_hi_rg, lid_hi_rt, ldr_out_pt) != 1)
              //tested rg&rt OK                                   
                fprintf(stderr, " Calling Lidar1_filter fail! \n");
                                                                  
            con_state_pt-> lidar_range=ldr_out_pt-> long_pos;     
            con_state_pt-> lidar_range_rate=ldr_out_pt-> long_rt; 
            f_index_pt-> lidar = ldr_out_pt-> f_mode;             
        }                                                         
        
        if(pparams->mdl_lidar == 1) {                             
//            if (ldr_flt(dt, lid_hi_rg, lid_hi_rt, ldr_out_pt) != 1)
//               fprintf(stderr, " Calling Lidar_filter fail! \n");
              
//            if ( clt_read( pclt, DB_MDL_LIDAR_VAR, DB_MDL_LIDAR_TYPE,
//                       &db_data_lidarMB ) == FALSE )
//            fprintf( stderr, "clt_read( DB_MDI_LIDAR_VAR) \n");
//            pmdl_lidar=(mdl_lidar_typ *) db_data_lidarMB.value.user;  
                     
            ldr_mdl. rg = (float)pmdl_lidar-> range;
	    if( (global_time < 10.0) && (ldr_mdl.rg > 15.0) ) 
		ldr_mdl.rg = 10.0;
            //ldr_mdl. cnt=pmdl_lidar-> data_pulse_cnt;    // Removed     
            mdl_flt(dt, ldr_mdl, con_state_pt );
            
            con_state_pt-> lidar_range=con_state_pt-> mdl_rg;

//            f_index_pt-> mdl = mdl_out_pt-> f_mode;             
        }        
        
          rad_dist(dt, con_state_pt, maneuver_id, pre_maneuver_id, 
		f_index_pt, radar_sw);  // SW added on 07_25_03                                

        }                                                         
/******************************************************************************
*******************************************************************************
	Here ends Xiao-Yun's code
*******************************************************************************
******************************************************************************/
	*pdt = dt; 
	*prun_dist = run_dist; 
	*pv = v; 
	*pacl = acl; 
	*pacl_old = acl_old; 

     return 0;
}

/*******************************************************************************

    other functions

********************************************************************************/
int max_i(int a, int b)
{
    if (a >= b)
        return a;
    else
        return b; 
    
}

int min_i(int a, int b)
{
    if (a >= b)
        return b;
    else
        return a; 
    
}

float max_f(float a, float b)
{
    if (a >= b)
        return a;
    else
        return b; 
    
}

float min_f(float a, float b)
{
    if (a >= b)
        return b;
    else
        return a; 
    
}
