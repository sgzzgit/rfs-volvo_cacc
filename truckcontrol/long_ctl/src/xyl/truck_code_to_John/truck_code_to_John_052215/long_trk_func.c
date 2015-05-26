/*********************************************************************************

     long_trk_func.c - functions for long_trk.c

     comm() has been changed a lot;                         05_09_10
     revised after unification                              04_12_11
     started to build for trruck CACC						04_07_15
     
*********************************************************************************/

#include <sys_os.h>
#include <timestamp.h>
#include <coording.h>
#include "veh_long.h"
#include "path_gps_lib.h"
#include "long_comm.h"
#include "veh_trk.h"
#include "long_ctl.h"
#include "long_trk_func.h"
#include "long_trk.h"


/* comm - Verifies communication among trucks. Using vehicle_info_pt->pltn_size,
**	comm checks whether a message from each of the other trucks has been
**	received, and whether the timestamp from each truck has changed 
**	correctly. 
**
**	Returns: 0 on success, or a positive bit-mapped integer containing 
**	the identit(ies) of the offending truck(s).
*/
int comm(float local_t, float *pglobal_time, float delta_t, 
	control_state_typ *con_state_pt, vehicle_info_typ *veh_info_pt,
	comm_info_typ *comm_info_pt, fault_index_typ *f_index_pt,
	//unsigned short *pcomm_err_bound,
	veh_comm_packet_t *comm_receive_pt,
	veh_comm_packet_t *comm_send_pt, pltn_info_typ* pltn_inf_pt) 
{

	static int comm_counter[MAX_TRUCK]={0,0,0,0,0};
	static int comm_counter_old[MAX_TRUCK]={0,0,0,0,0}; 
	static int comm_f_counter[MAX_TRUCK]={0,0,0,0,0};
	static float comm_time_old[MAX_TRUCK]={0,0,0,0,0};
	unsigned short handshake_start = OFF;                    // for passing out info
	static unsigned short handshake_start_arr[MAX_TRUCK] = {0,0,0,0,0};    // local only; indicating link with one in the platoon with ID=pip
	float global_time = *pglobal_time;                                     // globl time of the latoon
	int error = 0;
	int pip;
	static int first_time=1;
    static unsigned short g_t_init_count=0, N_veh_comm=0;
	static unsigned short comm_prt_sw=ON, comm_prt1_sw = ON;
	static unsigned short synchrn_sw = 1,  global_t_sw=OFF;            


/******************************************************************************
*******************************************************************************
        Here begins Xiao-Yun's code
*******************************************************************************
******************************************************************************/

    con_state_pt-> comm_coord=OFF;   // Not used since coodination communication is not independent
    
    if (pltn_inf_pt-> pltn_size == 1)
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
	    for(pip = 1; pip <= pltn_inf_pt->pltn_size; pip++) // Logic deal with subject vehicle with each individual veh in platoon
	    {
		   if (local_t <0.03)      // Initialization
              handshake_start_arr[pip] = OFF;
		   if(pip != veh_info_pt-> veh_id)                                        // IF LOOP 1
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
                 if ((comm_f_counter[pip] > COMM_ERR_BOUND) && (comm_prt1_sw == ON) && (handshake_start_arr[pip] == ON))
                 {
                    handshake_start_arr[pip] = OFF;
                    comm_prt1_sw = OFF;
                    comm_prt_sw = ON;
					//veh_info_pt->fault_mode = 3;
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
               if ((comm_f_counter[1] > COMM_ERR_BOUND) && (handshake_start == 1) )
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
           	   if ( (comm_f_counter[2] > COMM_ERR_BOUND) && (handshake_start == 1) )
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
               if ( (comm_f_counter[3] > COMM_ERR_BOUND) && (handshake_start == 1) )
               {
                  f_index_pt-> comm_back=1;
                  f_index_pt-> comm=1;
           	   }   
           }
    }  // end of pltn size > 1

   
	
	if (N_veh_comm == (pltn_inf_pt->pltn_size - 1) )
	{
		handshake_start = 1;
		comm_info_pt->comm_reply = 1;
	}
	
		
	if ((comm_prt_sw == ON) && (handshake_start == ON))
		{
			comm_prt_sw = OFF;
//			comm_prt1_sw = ON;
			fprintf(stderr, "Handshaking with all ON!\n");
		}

    if (veh_info_pt-> veh_id == 1)
       global_time = local_t;                  // It is the same as t_ctrl
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
            global_time += delta_t;
       }

    if (f_index_pt-> comm == 0)
        {

             if (veh_info_pt-> veh_id == 1)
			    {
				                        
			    }
             else
                {
                   //if ( time_filter<=0.001 )  // To avoid comm remainder problem
				   if ( local_t <= 0.2 )  // To avoid comm remainder problem
                     {
                       con_state_pt-> pre_v = 0.0;
                       con_state_pt-> pre_a = 0.0;
                       con_state_pt-> lead_v = 0.0;
                       con_state_pt-> lead_a = 0.0;
                       //con_state_pt-> pre_mag_counter = 0;    // Should come from communication.
                     }
                   else
                     {
                       con_state_pt-> pre_v = comm_receive_pt[veh_info_pt-> veh_id - 1].vel_traj;     //velocity;  // Due to comm error,12_03_03
                       con_state_pt-> pre_a = comm_receive_pt[veh_info_pt-> veh_id - 1].acc_traj;     //accel;
                       con_state_pt-> lead_v = comm_receive_pt[1].vel_traj;
                       con_state_pt-> lead_a = comm_receive_pt[1].acc_traj;
//                     con_state_pt-> pre_mag_counter = comm_receive_pt1-> marker_counter;    // Should come from communication.
                     }
                }
             for (pip=1; pip<= (pltn_inf_pt->pltn_size); pip++)      	
                pltn_inf_pt-> pltn_fault_mode=max_i(pltn_inf_pt-> pltn_fault_mode, comm_receive_pt[pip]. fault_mode);
        }
	else
		{
			veh_info_pt-> fault_mode=3;                  // added on 09_13_10
			pltn_inf_pt-> pltn_fault_mode=3;
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
	
	 pltn_inf_pt-> handshake= handshake_start;
	*pglobal_time = global_time;
	
	

	return error;
}

/******************************************************************************
*******************************************************************************
        JBus info reading
*******************************************************************************
******************************************************************************/

// read JBis SW info
int read_sw(long_vehicle_state *pv_can, switch_typ* sw_rd_pt) 
{
	 sw_rd_pt->  park_brk_sw= pv_can-> CCVS_ParkingBrakeSwitch;
	 sw_rd_pt->  two_spd_axle_sw= pv_can-> CCVS_TwoSpeedAxleSwitch;
	 sw_rd_pt->  park_brk_release= pv_can-> CCVS_ParkBrakeReleaseInhibitRq;
	 sw_rd_pt->  clutch_sw= pv_can-> CCVS_ClutchSwitch;
	 sw_rd_pt->  brk_sw= pv_can-> CCVS_BrakeSwitch;
	 sw_rd_pt->  CC_pause_sw= pv_can-> CCVS_CruiseCtrlPauseSwitch;
	 sw_rd_pt->  CC_enable_sw= pv_can-> CCVS_CruiseControlEnableSwitch;
	 sw_rd_pt->  CC_active= pv_can-> CCVS_CruiseControlActive;
	 sw_rd_pt->  CC_acel_sw= pv_can-> CCVS_CruiseControlAccelerateSwitch;
	 sw_rd_pt->  CC_resume_sw= pv_can-> CCVS_CruiseControlResumeSwitch;
	 sw_rd_pt->  CC_coast_sw= pv_can-> CCVS_CruiseControlCoastSwitch;
	 sw_rd_pt->  CC_set_sw= pv_can-> CCVS_CruiseControlSetSwitch;
	 sw_rd_pt->  CC_state= pv_can-> CCVS_CruiseControlState;
	 sw_rd_pt->  Pto_state= pv_can-> CCVS_PtoState;
	 sw_rd_pt->  eng_shutdwn_override_sw= pv_can-> CCVS_EngShutdownOverrideSwitch;
	 sw_rd_pt->  eng_test_mode_sw= pv_can-> CCVS_EngTestModeSwitch;
	 sw_rd_pt->  eng_idle_decre_sw= pv_can-> CCVS_EngIdleDecrementSwitch;
	 sw_rd_pt->  end_idle_incre_sw= pv_can-> CCVS_EngIdleIncrementSwitch;
	 sw_rd_pt->  driveline_enaged= pv_can-> ETC1_TransmissionDrivelineEngaged;
	 sw_rd_pt->  lockup= pv_can-> ETC1_TorqueConverterLockupEngaged;	 
	 sw_rd_pt->  gshift_sw= pv_can-> ETC1_TransmissionShiftInProcess;
	 sw_rd_pt->  eng_overspeed_enbale= pv_can-> ETC1_MomentaryEngineOverspeedEnable;
	 sw_rd_pt->  progressive_shift_disable= pv_can-> ETC1_ProgressiveShiftDisable;
	 sw_rd_pt->  jk_enable_shift_assist_sw= pv_can-> ERC1ERRetarderEnableShiftAssistSw;
     sw_rd_pt->  jk_enable_brake_assist_sw= pv_can-> ERC1ERRtdrEnablBrakeAssistSwitch;
	 sw_rd_pt->  jk_mode= pv_can-> ERC1ERRetarderTorqueMode;
	 sw_rd_pt->  jk_require_brk_light= pv_can-> ERC1ERRetarderRqingBrakeLight;
	 sw_rd_pt->  foundation_brk_use=pv_can-> EBC5_FoundationBrakeUse;
	 sw_rd_pt->  halt_brk_mode=pv_can-> EBC5_HaltBrakeMode;
	 sw_rd_pt->  XBR_accel_limit=pv_can-> EBC5_XBRAccelerationLimit;
	 sw_rd_pt->  XBR_active_contr_mode=pv_can-> EBC5_XBRActiveControlMode;

	 return 0;     
} // read_sw end

// read other JBus info
int read_jbus(float t_filter, long_vehicle_state *pv_can, long_params *pparams, jbus_read_typ* jbus_rd_pt, switch_typ* sw_rd_pt) 
{
	static float we_old=0.0;
	float we=0.0;

    /************* read JBus info *****************/

	 //jbus_rd_pt->  long_accel= pv_can-> long_accel;
	 //jbus_rd_pt->  lat_accel= pv_can-> lat_accel;
	 //jbus_rd_pt->  fb_applied= pv_can-> fb_applied;
  	 //jbus_rd_pt->  rb_applied= pv_can-> rb_applied;
  	 //jbus_rd_pt->  fb_monitor= pv_can-> fb_monitor;
  	 //jbus_rd_pt->  rb_monitor= pv_can-> rb_monitor;
  	 //jbus_rd_pt->  fb_axle= pv_can-> fb_axle;
  	 //jbus_rd_pt->  mb_axle= pv_can-> mb_axle;
  	 //jbus_rd_pt->  rb_axle= pv_can-> rb_axle;
  	 //jbus_rd_pt->  trans_retarder_voltage= pv_can-> trans_retarder_voltage;
  	 jbus_rd_pt->  we= pv_can-> EEC1_EngineSpeed;
     //jbus_rd_pt->  we_old= 600.0; // TBD
     //jbus_rd_pt->  we_flt=  pv_can-> EEC1_EngineSpeed; // TBD
     jbus_rd_pt->  w_p= pv_can-> ETC1_TransInputShaftSpeed;          // no tq converter
     jbus_rd_pt->  w_t= pv_can-> ETC1_TransmissionOutputShaftSpeed;  // no tq converter
     jbus_rd_pt->  eng_mode= 0 ;   // TBD
     jbus_rd_pt->  driver_dmd_percent_tq= pv_can-> EEC1_DrvrDemandEngPercentTorque;
     jbus_rd_pt->  actual_eng_percent_tq= pv_can-> EEC1_ActualEnginePercentTorque; 
     jbus_rd_pt->  eng_tq= 1200.0 ; // TBD
  	 jbus_rd_pt->  eng_dmd_percent_tq= pv_can-> EEC1_EngDemandPercentTorque;
  	 jbus_rd_pt->  eng_tq_mode= pv_can-> EEC1_EngineTorqueMode;
	 jbus_rd_pt->  bp= (pv_can-> fb_applied + pv_can-> rb_applied)*0.5;
	 //jbus_rd_pt->  bp= (pv_can-> fb_axle + pv_can-> mb_axle + pv_can-> rb_axle)*3.33333;
                                                               
	 jbus_rd_pt->  brk_pedal_pos= pv_can-> EBC1_BrakePedalPosition;
	 jbus_rd_pt->  brk_demand= pv_can-> EBC1_BrakePedalPosition;	 
	 jbus_rd_pt->  ebs_brake_switch= pv_can-> EBSBrakeSwitch;
	 jbus_rd_pt->  accel_pedal_volt= pv_can-> acc_pedal_voltage;
	 jbus_rd_pt->  accel_pedal_pos= pv_can-> accelerator_pedal_position;
	 jbus_rd_pt->  accel_pedal_pos1= pv_can-> EEC2_AccelPedalPos1;
	 jbus_rd_pt->  accel_pedal_pos2= pv_can-> EEC2_AccelPedalPos2;
	 jbus_rd_pt->  actual_max_eng_percent_tq= pv_can-> EEC2_ActMaxAvailEngPercentTorque;	 
	 jbus_rd_pt->  percent_load_at_current_spd= pv_can-> EEC2_EnginePercentLoadAtCurrentSpd;  
	 jbus_rd_pt->  road_v_limit_status= pv_can-> EEC2_RoadSpeedLimitStatus;
	 jbus_rd_pt->  nominal_fric_percent_tq= pv_can-> EEC3_NominalFrictionPercentTorque; 
	 jbus_rd_pt->  eng_des_Op_v=  pv_can-> EEC3_EngsDesiredOperatingSpeed;
	
	 jbus_rd_pt->  brk_pedal_pos= pv_can-> EBC1_BrakePedalPosition;
	 jbus_rd_pt->  EBS_sw= pv_can-> EBSBrakeSwitch;
	 jbus_rd_pt->  ABS_amber_warning= pv_can-> ABSEBSAmberWarningSignal;
	 jbus_rd_pt->  ABS_Operation= pv_can-> EBC1_ABSFullyOperational;
	 jbus_rd_pt->  anti_lock_active= pv_can-> EBC1_AntiLockBrakingActive;
	 jbus_rd_pt->  ASR_brk_contr_active= pv_can-> EBC1_ASRBrakeControlActive;
	 jbus_rd_pt->  ASR_eng_contr_active= pv_can-> EBC1_ASREngineControlActive;
	 jbus_rd_pt->  EBS_red_warning= pv_can-> EBC1_EBSRedWarningSignal;
	 jbus_rd_pt->  jk_selection= pv_can-> EBC1_EngRetarderSelection;
	 jbus_rd_pt->  front_axle_spd= pv_can-> EBC2_FrontAxleSpeed;
	 jbus_rd_pt->  v= pv_can-> CCVS_VehicleSpeed;
	 jbus_rd_pt->  fl_axle_diff= pv_can-> EBC2_RelativeSpeedFrontAxleLeftWheel;
     jbus_rd_pt->  fr_axle_diff= pv_can-> EBC2_RlativeSpeedFrontAxleRightWheel;
     jbus_rd_pt->  rl_axle_diff= pv_can-> EBC2_RelativeSpeedRearAxle1LeftWheel;
     jbus_rd_pt->  rr_axle_diff= pv_can-> EBC2_RlativeSpeedRearAxle1RightWheel;
     jbus_rd_pt->  rl_axle2_diff= pv_can-> EBC2_RlativeSpeedRearAxle2RightWheel;
     jbus_rd_pt->  rr_axle2_diff= pv_can-> EBC2_RlativeSpeedRearAxle2RightWheel;
     jbus_rd_pt->  CC_set_v= pv_can-> CCVS_CruiseControlSetSpeed;     

	 jbus_rd_pt->  trans_out_we= pv_can-> ETC1_TransmissionOutputShaftSpeed;
	 jbus_rd_pt->  trans_in_we= pv_can-> ETC1_TransInputShaftSpeed;
	 jbus_rd_pt->  percent_clutch_slip= pv_can-> ETC1_PercentClutchSlip;
	 
	 jbus_rd_pt->  selected_gear= pv_can-> ETC2_TransmissionSelectedGear; 
	 jbus_rd_pt->  actual_gear_ratio= pv_can-> ETC2_TransmissionActualGearRatio;  
	 jbus_rd_pt->  gear= pv_can-> ETC2_TransmissionCurrentGear;
	 jbus_rd_pt->  trans_range_selected= pv_can-> ETC2_TransmissionRangeSelected;	
	 jbus_rd_pt->  trans_range_attained= pv_can-> ETC2_TransmissionRangeAttained;
	 jbus_rd_pt->  gear_flt= pv_can-> ETC2_TransmissionCurrentGear;
        
     jbus_rd_pt->  fuel_m= pv_can-> LFE_EngineFuelRate;
     jbus_rd_pt->  instant_fuel_economy= pv_can-> LFE_EngineInstantaneousFuelEconomy;
	 jbus_rd_pt->  mean_fuel_economy= pv_can-> LFE_EngineAverageFuelEconomy;
	 jbus_rd_pt->  throttle_valve_pos= pv_can-> LFE_EngineThrottleValve1Position;
    
     
	 jbus_rd_pt->  jk_des_percent_tq= pv_can-> ERC1ERIntendedRtdrPercentTorque;   
	 jbus_rd_pt->  jk_actual_percent_tq= pv_can-> ERC1ERActualEngineRetPercentTrq;  	 

	 jbus_rd_pt->  jk_driver_dmd_percent_tq= pv_can-> ERC1ERDrvrsDmandRtdrPerctTorque;
	 jbus_rd_pt->  jk_selection_Non_Eng= pv_can-> ERC1ERRetarderSelectionNonEng;     
     jbus_rd_pt->  jk_max_percent_tq= pv_can-> ERC1ERActlMxAvlbRtdrtPerctTorque;      
     jbus_rd_pt->  pm= pv_can-> boost_pressure;
	 jbus_rd_pt->  lat_accel= pv_can-> VDC2_LateralAcceleration;
	 jbus_rd_pt->  long_accel= pv_can-> VDC2_LongitudinalAcceleration;
	 jbus_rd_pt->  steer_angle= pv_can-> VDC2_SteeringWheelAngle;	
	 jbus_rd_pt->  yaw_rt= pv_can-> VDC2_YawRate;
	 jbus_rd_pt->  VP_lat= pv_can-> VP_X_TGW_Latitude_BB1_X_TGW;
	 jbus_rd_pt->  VP_long= pv_can-> VP_X_TGW_Longitude_BB1_X_TGW;

	

	/*************** read JBus info end ***************/
                                                                                                                                                                                    
                                                                  
    if( t_filter < 0.07 )                                      
        {      
			    we= jbus_rd_pt-> we;
                we_old = we;                                      
                jbus_rd_pt-> we = we;                       
        }                                                         
    else                                                          
        {                                                                                                                           
            if ((sw_rd_pt-> gshift_sw) == OFF)                                                                      
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
            jbus_rd_pt-> we = we*0.2 + (jbus_rd_pt-> we)*0.8;
        }                                                         
                                                                  

        return 0;                                                 
} // read_jbus() end


/******************************************************************************
*******************************************************************************
	Control actuations
*******************************************************************************
******************************************************************************/
int actuate(long_output_typ *pcmd, float *pengine_reference_torque, float *pt_ctrl,/* int *pmaneuver_des,*/ int *maneuver_id,
	con_output_typ* con_output_pt, control_state_typ* con_state_pt, control_config_typ* config, control_config_typ* config_pt, 
	long_params *pparams, float *pminimum_torque, long_output_typ *inactive_ctrl, manager_cmd_typ * manager_cmd_pt, 
	switch_typ *sw_pt, vehicle_info_typ *veh_info_pt, jbus_read_typ* jbus_rd_pt) 
{

  float engine_reference_torque = *pengine_reference_torque;        
  float t_ctrl = *pt_ctrl;  
  float minimum_torque = *pminimum_torque;
  float tmp_rate;
  float max_tq_we;

     if (tq_we(jbus_rd_pt-> we, &max_tq_we) != 1)
		 fprintf(stderr, "Call tq_we err!");

     if (con_output_pt-> con_sw_1 == 1)  // Tq cmd
      {
	       pcmd->engine_command_mode = TSC_TORQUE_CONTROL;
	       if (test_site == NVD)
	       {	          
			  if (veh_info_pt-> veh_id == 1)
              {
                //pcmd->engine_torque = con_output_pt-> y1;
				pcmd->engine_torque = (con_output_pt-> y1)/max_tq_we;
              }               	                             
			  else // veh_id > 1
	          {
                tmp_rate=(con_state_pt->front_range - con_state_pt-> temp_dist)/(con_state_pt-> des_f_dist); 
                if (maneuver_id[0] != 29)
                      {
	                    
                        if (maneuver_id[0] == 7)
                        {                 			           
                 			if (tmp_rate > 0.35)
                 				tmp_rate = 0.35;	
                 			if (tmp_rate < -0.5)
                 				tmp_rate = -0.5;	
                 			//pcmd->engine_torque = (con_output_pt-> y1)*(1.0+tmp_rate);    // Modified on 09_10_10
							pcmd->engine_torque = (con_output_pt-> y1)*(1.0+tmp_rate)/max_tq_we;
                        }
                        else
                        {                         	
                        	if (tmp_rate > 0.35)
                                tmp_rate = 0.35;
                        	if (tmp_rate < -0.5)
                                tmp_rate = -0.5;
                        	//pcmd->engine_torque = (con_output_pt-> y1)*(1.0+tmp_rate);    // Modified on 09_10_10
							pcmd->engine_torque = (con_output_pt-> y1)*(1.0+tmp_rate)/max_tq_we;
                        }
                      }
                 else
                      {
	                    if (tmp_rate > 0.05)
                            tmp_rate = 0.05;
                        if (tmp_rate < -0.5)
                            tmp_rate = -0.5;
                        pcmd->engine_torque = (con_output_pt-> y1)*(1.0+tmp_rate);    // Modified on 09_10_10
                      	//pcmd->engine_torque = con_output_pt-> y1;    // default
						pcmd->engine_torque = (con_output_pt-> y1)*(1.0+tmp_rate)/max_tq_we;
                  	  }
              }  // veh_id > 1
			  
          } // NVD end
          if (test_site ==RFS)
          		pcmd->engine_torque = (con_output_pt-> y1)/max_tq_we;
          if (pcmd->engine_torque > engine_reference_torque)
                pcmd->engine_torque = engine_reference_torque/max_tq_we;
          if (pcmd->engine_torque < minimum_torque/max_tq_we)
                 pcmd->engine_torque = minimum_torque/max_tq_we;
              
              pcmd->engine_retarder_command_mode = TSC_TORQUE_CONTROL;   // Use this if possible
              pcmd->engine_retarder_torque = -0.0; // Negative torque, not percentage                                                               

              pcmd->trans_retarder_command_mode = TSC_TORQUE_CONTROL;    // Use this if possible                                                    
              pcmd->trans_retarder_value = -0.0; // Negative percentage
                                                                   
              pcmd->brake_command_mode = EXAC_NOT_ACTIVE; //04_09_03
      }
      else                                                         
      {                                                        
              //pcmd->engine_command_mode = TSC_OVERRIDE_DISABLED; 
              pcmd->engine_torque = minimum_torque/max_tq_we;                
              pcmd->engine_speed = +0.0;                           
      }                                                        
                                                                   
                   
      if (con_output_pt-> con_sw_3 == 1)  //jk_cmd                 
      {   
	      pcmd->engine_retarder_command_mode = TSC_TORQUE_CONTROL;
	      if (test_site == NVD)
	      {
	         if (pparams->vehicle_type == VEH_TYPE_VOLVO_1)
	         {
              	 pcmd->engine_retarder_torque = -(con_output_pt-> y17); // Negative torque, not percentage
	         }  // BLUE end

			 if (pparams->vehicle_type == VEH_TYPE_VOLVO_2)
			 {
				 tmp_rate=(con_state_pt->front_range - con_state_pt-> temp_dist)/(con_state_pt-> des_f_dist);  
	             if (tmp_rate > 0.65)
                     tmp_rate = 0.65;	
                 if (tmp_rate < -0.55)
                     tmp_rate = -0.55;	                                                  
                 //pcmd->engine_retarder_torque = -con_output_pt-> y17; // Negative torque, not percentage; changed 09_13_10
			     pcmd->engine_retarder_torque = (-con_output_pt-> y17)*(1.0-tmp_rate); // Negative torque, not percentage
			 }   // GOLD end

			 if (pparams->vehicle_type == VEH_TYPE_VOLVO_3)  
			 {
				tmp_rate=(con_state_pt->front_range - con_state_pt-> temp_dist)/(con_state_pt-> des_f_dist);            
                                                                                                                                                                                           //   Removed on 05_22_11
                  if (tmp_rate > 0.45)
                    tmp_rate = 0.45;    
                  if (tmp_rate < -0.65)
                    tmp_rate = -0.65;                    
                    pcmd->engine_retarder_torque = (-con_output_pt-> y17)*(1.0-tmp_rate); // Negative torque, not percentage   
                          
                if ((maneuver_id[0] == 29) && (pcmd->engine_retarder_torque > -1000.0) )
                 {
                   pcmd->engine_retarder_torque = -1000.0;                                                                // Added on 09_15_10 
                 }  
			 }   // SILVR end                                                             			 
         }   // NVD end
         if (test_site ==RFS)
         	 pcmd->engine_retarder_torque = -(con_output_pt-> y17);
         pcmd->engine_command_mode = TSC_TORQUE_CONTROL; //Use this if possible                                                                
         pcmd->engine_torque = 0.0;
      }                                                        
      else
      {                                                        
             
              pcmd->engine_retarder_torque = -0.0;                 
      }   // jk cmd end                                                     
                                                                        
          
      if (con_output_pt-> con_sw_5 == 1)                           // E brk cmd           
      {  
	       pcmd->brake_command_mode = EXAC_ACTIVE;      
	       if (test_site == NVD)
	       {
	          if (veh_info_pt-> veh_id == 1)
				{
                   pcmd->ebs_deceleration = - 0.075*(con_output_pt-> y16); //0.48 for bl_rent     // 0.82 for gl_ucr; 10_29_03 for 0.5           
				}   

			  if (veh_info_pt-> veh_id == 2)
				{
				   tmp_rate=(con_state_pt->front_range - con_state_pt-> temp_dist)/(con_state_pt-> des_f_dist);  	                                                      
	               if (tmp_rate > 0.65)
                  		 tmp_rate = 0.65;	
              	   if (tmp_rate < -0.45)
                  		 tmp_rate = -0.45;	                                                
                   if (con_state_pt-> pre_v > 9.0)
                         pcmd->ebs_deceleration = - 0.05*(con_output_pt-> y16)*(1.0-tmp_rate);                    
                   else
                         pcmd->ebs_deceleration = - 0.085*(con_output_pt-> y16)*(1.0-tmp_rate);                                                                  
				}  // GOLD end

			  if (veh_info_pt-> veh_id == 3)
				{
				   tmp_rate=(con_state_pt->front_range - con_state_pt-> temp_dist)/(con_state_pt-> des_f_dist); 
                  //tmp_rate_v = con_state_pt-> ref_v - con_state_pt-> spd;                                                                                     
                   if (tmp_rate > 0.45)
                           tmp_rate = 0.45;       
                   if (tmp_rate < -0.45)
                           tmp_rate = -0.45;         
                                                                             
                   if (con_state_pt-> pre_v > 9.0)
                           pcmd->ebs_deceleration = - 0.0375*(con_output_pt-> y16)*(1.0-tmp_rate);                    
                   else
                           pcmd->ebs_deceleration = - 0.050*(con_output_pt-> y16)*(1.0-tmp_rate);                                                                  
				}  
           } // NVD end
          if (test_site == RFS)
           {
	           if (pparams->vehicle_type == VEH_TYPE_VOLVO_1)
                      pcmd->ebs_deceleration = - 0.075*(con_output_pt-> y16); //0.48 for bl_rent     // 0.82 for gl_ucr; 10_29_03 for 0.5           
               else
                   {                                               
                      if (con_state_pt-> pre_v > 9.0)
                         pcmd->ebs_deceleration = - 0.05*(con_output_pt-> y16);               
                      else
                         pcmd->ebs_deceleration = - 0.085*(con_output_pt-> y16);
                   }
           }
              //pcmd->engine_command_mode = TSC_OVERRIDE_DISABLED; 
           pcmd->engine_command_mode = TSC_TORQUE_CONTROL; //Use this if possible                                                                
           pcmd->engine_torque = 0.0;
      }                                                        
      else                                                         
      {                                                        
              pcmd->brake_command_mode = EXAC_NOT_ACTIVE;
              pcmd->ebs_deceleration = -0.0;                       
      }   // E-brake end 
                                                              
                                                      
                                                                   
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
          pcmd->ebs_deceleration = - 1.0;  
          pcmd-> fan_override = 1;                           
		  pcmd-> fan_control = 1;                 
                        
     }                                                             
                                                                   
      if ((manager_cmd_pt-> drive_mode == 0) || (manager_cmd_pt-> drive_mode == 1) || (sw_pt-> manu_sw ==1) || 
		(manager_cmd_pt-> f_manage_index==3) || (sw_pt->brake_sw == 1) )
     	  memcpy(pcmd ,inactive_ctrl, sizeof(long_output_typ) );
                                                                   

        *pengine_reference_torque = engine_reference_torque;       
        //*pvehicle_pip = vehicle_pip;                                 
        *pt_ctrl = t_ctrl;                                         
        //*pmaneuver_des = maneuver_des;                             
		*pminimum_torque = minimum_torque;
                                                                   
        return 0;                                                  
}                                                                  

/******************************************************************************
*******************************************************************************
	Configure SWs
*******************************************************************************
******************************************************************************/

#ifdef UN_REMOVED

int config_sw(int *preading_sw, int *pmanu_auto_sw, 
	unsigned short *phandshake_start, int *preading_sw_old, switch_typ *sw_pt, 
	long_vehicle_state *pv, vehicle_info_typ* vehicle_info_pt, 
	veh_comm_packet_t *comm_receive_pt, unsigned short *pprt1_sw, fault_index_typ* f_ind_pt) {
	
	int reading_sw = *preading_sw; 
	int manu_auto_sw = *pmanu_auto_sw; 
	unsigned short handshake_start = *phandshake_start;
	int reading_sw_old = *preading_sw_old;
	char pip;
	unsigned char actuator_sw = ON;
	unsigned short prt1_sw = *pprt1_sw;
	int ret_brake = 0;

     sw_pt-> comp_sw=1;
     sw_pt-> cond_sw=0;
     sw_pt-> steer_sw=1;
     sw_pt-> alt_sw=1;
     //sw_pt-> actuator_sw=1;
     sw_pt-> radar1_sw=1;
     sw_pt-> radar2_sw=1;
     //sw_pt-> brake_sw=1;
     //sw_pt-> throt_sw=1;

     reading_sw=long_rdswitch(pv->dig_in);

     ret_brake = long_rdbrake(pv->dig_in);
     if( ret_brake == 1)
	sw_pt->brake_sw = 1;

     if ( (reading_sw == 3) && (manu_auto_sw == 0)) // uncommitted state
        {
           sw_pt-> manu_sw = 1;
           manu_auto_sw = 1;
           fprintf(stderr, "Got uncommitted switch state the first time\n");
           f_ind_pt -> sw=0;
         }     
     if ( (reading_sw==1) && (sw_pt-> manu_sw==0)) // 0: auto mode, 1: manual mode, 2: request for automatic; -1: error.
        {
           if(sw_pt->manu_sw == 0)
           	fprintf(stderr, "Got manual switch state\n");
           sw_pt-> auto_sw=0;
           sw_pt-> manu_sw=1;
           f_ind_pt -> sw=0;
        }
     if ((reading_sw == 2) && (sw_pt-> auto_sw==0))
        {
           if(sw_pt->auto_sw == 0)
           	fprintf(stderr, "Got auto switch state\n");
           sw_pt-> auto_sw=1;
           sw_pt-> manu_sw=0;
           f_ind_pt -> sw=0;
        }
     if (reading_sw == -1)
        {
           if( (sw_pt->auto_sw == 1) || (sw_pt->manu_sw == 1) )
           	  fprintf(stderr, "Error in switch state\n");           
           f_ind_pt -> sw=1;
        }                                               
                                                                  
     if (vehicle_info_pt-> pltn_size == 1)    // This is not used in control yet;   12_01_09                                                      
       {
           if (sw_pt-> auto_sw == 1)                              
              sw_pt-> actuator_sw = ON;                           
       }                                                          
     else                                                         
       {                                                          
        // For 3 veh                                    
		actuator_sw = ON;
		for(pip  = 1; pip <= vehicle_info_pt->pltn_size; pip++) 
		{
			if( pip != vehicle_info_pt->veh_id ) 
			{
           			if ( (handshake_start == ON) && (sw_pt->auto_sw == 1) && (prt1_sw == ON) && (comm_receive_pt[(int)pip].user_ushort_2==1))
               				actuator_sw &= ON;     				
           			else                                
              				actuator_sw = OFF;
			}
		}
		sw_pt-> actuator_sw = actuator_sw;
       }                                                          
                                                                  
     reading_sw_old=reading_sw;                                         


	*preading_sw = reading_sw; 
	*pmanu_auto_sw = manu_auto_sw; 
	*phandshake_start = handshake_start;
	*preading_sw_old = reading_sw_old;
     return 0;      
}

#endif
/******************************************************************************
*******************************************************************************
	Initialization
*******************************************************************************
******************************************************************************/

int set_init_leds(db_clt_typ *pclt, unsigned short *pprt1_sw, 
	unsigned short *pprt_buff, /*unsigned short *phandshake_start,*/ 
	switch_typ *sw_pt, vehicle_info_typ* vehicle_info_pt,
    veh_comm_packet_t *comm_receive_pt, control_state_typ *con_state_pt, 
	manager_cmd_typ *manager_cmd_pt, pltn_info_typ* pltn_inf_pt) {

	unsigned short prt1_sw = *pprt1_sw;
	unsigned short prt_buff = *pprt_buff;
	//unsigned short handshake_start = *phandshake_start;
	unsigned short user_ushort_2 = 1;
	int pip;


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

	
	for(pip = 1; pip <= pltn_inf_pt->pltn_size ; pip++) 
	{
		if(pip != vehicle_info_pt->veh_id) 
		{
			user_ushort_2 &= comm_receive_pt[pip].user_ushort_2;
		}
	}
	if ( (pltn_inf_pt-> handshake == ON) && (sw_pt->auto_sw == 1) && (user_ushort_2 == 1) ) 
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
      if ((manager_cmd_pt-> drive_mode == 0) || (manager_cmd_pt-> drive_mode == 1) || (sw_pt-> manu_sw ==1) ||
                (manager_cmd_pt-> f_manage_index==3) || (sw_pt->brake_sw == 1) )
    {
	    if (long_setled(pclt, FLT_HI) != 0)
               fprintf(stderr, " Setting FLT_HI fail 2!\n"); 
        vehicle_info_pt-> ready = 0;
        user_ushort_2=0;
    }   


	*pprt1_sw = prt1_sw;
	*pprt_buff = prt_buff;
	//*phandshake_start = handshake_start;
	return 0;     
}

/******************************************************************************
*******************************************************************************
	Setting RT synch
*******************************************************************************
******************************************************************************/
int set_time_sync(float *pt_ctrl, float *pdt, 
	float *ptime_filter, /*int *pvehicle_pip,*/ 
	vehicle_info_typ* veh_info_pt, pltn_info_typ* pltn_inf_pt) {

    float t_ctrl = *pt_ctrl;     
    float dt = *pdt; 
    float time_filter = *ptime_filter; 
    //int vehicle_pip = *pvehicle_pip;    

    if (pltn_inf_pt-> pltn_size == 1)    // Changed on 11_14_08
       {
          if (pltn_inf_pt-> ready == 1)
             t_ctrl +=dt;
       }

    if (veh_info_pt-> ready == 1)
             t_ctrl +=dt;
         
    if ( t_ctrl >= t_wait )
       time_filter += dt; 


    *pt_ctrl = t_ctrl; 
    *pdt = dt; 
    *ptime_filter = time_filter; 
    //*pvehicle_pip = vehicle_pip;
     return 0;       
}

/******************************************************************************
*******************************************************************************
	Signal processing
*******************************************************************************
******************************************************************************/


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
