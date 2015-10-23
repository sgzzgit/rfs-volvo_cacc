/*********************************************************************************

     long_trk_func.c - functions for long_trk.c

     comm() has been changed a lot;                         05_09_10
     revised after unification                              04_12_11
     started to build for trruck CACC						04_07_15
     
*********************************************************************************/

#include <sys_os.h>
#include <stdlib.h>
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
	control_state_typ *con_st_pt, vehicle_info_typ *veh_info_pt,
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

    con_st_pt-> comm_coord=OFF;   // Not used since coodination communication is not independent
    
    if (pltn_inf_pt-> pltn_size == 1)
    {
             con_st_pt-> comm_leader=ON;
             con_st_pt-> comm_pre=ON;
             con_st_pt-> comm_back=ON;
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
               con_st_pt-> comm_leader=ON;
               f_index_pt-> comm_leader=0;
           }
        else
           {	                             
               con_st_pt-> comm_leader=OFF;
               if ((comm_f_counter[1] > COMM_ERR_BOUND) && (handshake_start == 1) )
               {
               	  f_index_pt-> comm_leader=1;
               	  f_index_pt-> comm=1;
           	   }           
           }
           
        if (handshake_start_arr[2] == ON)
           {  
               con_st_pt-> comm_pre=ON;               
               f_index_pt-> comm_pre=0;
           }
        else
           {
           	   con_st_pt-> comm_pre=OFF;
           	   if ( (comm_f_counter[2] > COMM_ERR_BOUND) && (handshake_start == 1) )
           	   {
                  f_index_pt-> comm_pre=1;
                  f_index_pt-> comm=1;
           	   }   
           }
        if (handshake_start_arr[3] == ON)
           {  
               con_st_pt-> comm_back=ON;
               f_index_pt-> comm_back=0;
           }
        else
           {  
               con_st_pt-> comm_back=OFF;
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
         //if ( (synchrn_sw == 1) && (con_st_pt-> comm_leader == ON))
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
                       con_st_pt-> pre_v = 0.0;
                       con_st_pt-> pre_a = 0.0;
                       con_st_pt-> lead_v = 0.0;
                       con_st_pt-> lead_a = 0.0;
                       //con_st_pt-> pre_mag_counter = 0;    // Should come from communication.
                     }
                   else
                     {
                       con_st_pt-> pre_v = comm_receive_pt[veh_info_pt-> veh_id - 1].vel_traj;     //velocity;  // Due to comm error,12_03_03
                       con_st_pt-> pre_a = comm_receive_pt[veh_info_pt-> veh_id - 1].acc_traj;     //accel;
                       con_st_pt-> lead_v = comm_receive_pt[1].vel_traj;
                       con_st_pt-> lead_a = comm_receive_pt[1].acc_traj;
//                     con_st_pt-> pre_mag_counter = comm_receive_pt1-> marker_counter;    // Should come from communication.
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

    con_st_pt-> pltn_vel=0.0;
    con_st_pt-> pltn_acc=0.0;
    con_st_pt-> pltn_dcc=0.0;
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

  	 jbus_rd_pt->  we= pv_can-> EEC1_EngineSpeed;
     jbus_rd_pt->  w_p= pv_can-> ETC1_TransInputShaftSpeed;          // no tq converter
     jbus_rd_pt->  w_t= pv_can-> ETC1_TransmissionOutputShaftSpeed;  // no tq converter
     jbus_rd_pt->  eng_mode= 0 ;   // TBD
     jbus_rd_pt->  driver_dmd_percent_tq= pv_can-> EEC1_DrvrDemandEngPercentTorque;
     jbus_rd_pt->  actual_eng_tq= pv_can-> EEC1_ActualEnginePercentTorque; 
     jbus_rd_pt->  eng_tq= 1200.0 ; // TBD
  	 jbus_rd_pt->  eng_dmd_percent_tq= pv_can-> EEC1_EngDemandPercentTorque;
  	 jbus_rd_pt->  eng_tq_mode= pv_can-> EEC1_EngineTorqueMode;                                                        
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

	 if (pv_can-> CCVS_CruiseControlSetSpeed < 0.0)
		 jbus_rd_pt->  CC_set_v=0.0;
     else
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
int actuate(long_output_typ *pcmd, con_output_typ* con_out_pt, control_state_typ* con_st_pt,
	long_params *pparams, long_output_typ *inactive_ctrl, manager_cmd_typ * mng_cmd_pt, 
	switch_typ *sw_pt, jbus_read_typ* jbus_rd_pt) 
{       
  //float tmp_rate;
  float max_tq_we, tmp_rate;

     if (tq_we(jbus_rd_pt-> we, &max_tq_we) != 1)
		 fprintf(stderr, "Call tq_we err!");
	 con_st_pt-> max_tq_we = max_tq_we;
	 
	 //max_tq_we=ENGINE_REF_TORQUE;


     if (con_out_pt-> con_sw_1 == 1)  // Tq cmd
      {
	      pcmd->engine_command_mode = TSC_TORQUE_CONTROL;	       		  
          pcmd->engine_torque = (con_out_pt-> y1)/ENGINE_REF_TORQUE;
          if (pcmd->engine_torque < (MIN_TORQUE/ENGINE_REF_TORQUE) )
             pcmd->engine_torque = (MIN_TORQUE/ENGINE_REF_TORQUE);
		
		  tmp_rate=(con_st_pt-> ref_v - con_st_pt-> spd)/2.0;
		   if (tmp_rate > 0.45)
              tmp_rate = 0.45;      
           if (tmp_rate < -0.45)
              tmp_rate = -0.45;    
		  
		   //pcmd->engine_torque = 0.85*(pcmd->engine_torque)*100.0;
		   pcmd->engine_torque = (1.0+tmp_rate)*(pcmd->engine_torque)*100.0;   

		  if (pcmd->engine_torque > 99.0)
                pcmd->engine_torque = 99.0;
          pcmd->engine_torque =pcmd->engine_torque +0.5*rand()/RAND_MAX; 

		  pcmd->engine_retarder_command_mode = TSC_TORQUE_CONTROL;   
          pcmd->engine_retarder_torque = -0.0; // Negative percentage torque                                                              
                                                                   
          pcmd->brake_command_mode = EXAC_NOT_ACTIVE; //04_09_03
      }
      else                                                         
      {                                                        
              //pcmd->engine_command_mode = TSC_OVERRIDE_DISABLED; 
              pcmd->engine_torque = 100.0*(MIN_TORQUE/ENGINE_REF_TORQUE); 			 
              pcmd->engine_speed = +0.0;                           
      }                                                        
      if( (jbus_rd_pt-> accel_pedal_pos1) > 2.0)
		pcmd->engine_command_mode = TSC_OVERRIDE_DISABLED; // indicating driver taking over                                                            
                   
      if (con_out_pt-> con_sw_3 == 1)  //jk_cmd                 
      {   
	     pcmd->engine_retarder_command_mode = TSC_TORQUE_CONTROL;	   
		  tmp_rate=(con_st_pt-> ref_v - con_st_pt-> spd)/2.0;
		   if (tmp_rate > 0.45)
              tmp_rate = 0.45;      
           if (tmp_rate < -0.45)
              tmp_rate = -0.45;    
         //pcmd->engine_retarder_torque = -100.0*(con_out_pt-> y17)/JK_REF_TORQUE;
		 pcmd->engine_retarder_torque = -100.0*(1.0-tmp_rate)*(con_out_pt-> y17)/max_f(con_st_pt-> max_jk_we,1.0);
         pcmd->engine_command_mode = TSC_TORQUE_CONTROL; //Use this if possible                                                                
         pcmd->engine_torque = 0.0;
		 //pcmd->brake_command_mode = EXAC_NOT_ACTIVE; //04_09_03
		 if (pcmd->engine_retarder_torque < -99.0)
			pcmd->engine_retarder_torque = -99.0;
		 pcmd->engine_retarder_torque = pcmd->engine_retarder_torque - 0.5*rand()/RAND_MAX; 
      }                                                        
      else
      {            
		  pcmd->engine_retarder_command_mode = TSC_TORQUE_CONTROL;	             
          pcmd->engine_retarder_torque = -0.0;                 
      }   // jk cmd end                                                     
                                                                        
          
      if (con_out_pt-> con_sw_5 == 1)                           // E brk cmd           
      {  		  
	      pcmd->brake_command_mode = EXAC_ACTIVE;      
	       
          
	           if (pparams->vehicle_type == VEH_TYPE_TRUCK_VOLVO_1)
                      pcmd->ebs_deceleration = - 0.075*(con_out_pt-> y16); //0.48 for bl_rent     // 0.82 for gl_ucr; 10_29_03 for 0.5           
               else
                   {                                               
                      if (con_st_pt-> pre_v > 9.0)
                         pcmd->ebs_deceleration = - 0.05*(con_out_pt-> y16);               
                      else
                         pcmd->ebs_deceleration = - 0.085*(con_out_pt-> y16);
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
                                                              
                                                                             
                                                                   
      if ((mng_cmd_pt-> drive_mode == 0) || (mng_cmd_pt-> drive_mode == 1) || 
		(mng_cmd_pt-> f_manage_index==3) || (sw_pt->brake_sw == 1) )
     	  memcpy(pcmd ,inactive_ctrl, sizeof(long_output_typ) );
                                                                                                                                                                                                                        
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
