/*********************************************************************************

     long_trk_func.c - functions for long_trk.c

     comm() has been changed a lot;                         05_09_10
     revised after unification                              04_12_11
     started to build for trruck CACC						04_07_15
     
*********************************************************************************/

#include <sys_os.h>
#include <stdio.h>
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


/* comm - Verifies communication among trucks. Using veh_info_pt->pltn_size,
**	comm checks whether a message from each of the other trucks has been
**	received, and whether the timestamp from each truck has changed 
**	correctly. 
**
**	Returns: 0 on success, or a positive bit-mapped integer containing 
**	the identit(ies) of the offending truck(s).
*/
int cacc_comm(float local_t, float *pglobal_time, float delta_t, control_state_typ *con_st_pt, vehicle_info_typ *veh_info_pt,
	comm_info_typ *comm_info_pt, fault_index_typ *f_ind_pt, veh_comm_packet_t *comm_receive_pt, 
	veh_comm_packet_t *comm_send_pt, pltn_info_typ* pltn_inf_pt, control_config_typ *cnfg_pt) 
{

	static int comm_counter[MAX_TRUCK]={0,0,0,0,0};
	//static int comm_counter_old[MAX_TRUCK]={0,0,0,0,0}; 
	static int comm_f_counter[MAX_TRUCK]={0,0,0,0,0};
	static float comm_time_old[MAX_TRUCK]={0.0,0.0,0.0,0.0,0.0};
	unsigned short handshake_start = OFF;                    // for passing out info
	static unsigned short handshake_start_arr[MAX_TRUCK] = {0,0,0,0,0};    // local only; indicating link with one in the platoon with ID=pip
	float global_time = *pglobal_time;                                     // globl time of the latoon
	int error = 0;
	int pip;
	static int first_time=1;
    static unsigned short g_t_init_count=0;
	static unsigned short N_veh_comm=0; // number of veh in comm range and taking to each other
	//static unsigned short comm_prt_sw=ON, comm_prt1_sw = ON;
	static unsigned short synchrn_sw = 1,  global_t_sw=OFF; 


/******************************************************************************
*******************************************************************************
        Here begins Xiao-Yun's code
*******************************************************************************
******************************************************************************/

    con_st_pt-> comm_coord=OFF;   // Not used since coodination communication is not independent
    
    if (pltn_inf_pt-> pltn_size == 1)
    {                          
         veh_info_pt-> comm_p[0]=0;            
         handshake_start=ON;
		 comm_info_pt->comm_reply = 1;            
//         comm_reply_arr[1]=ON;
         f_ind_pt-> comm=0;                                                                                                                       
    } 
    else                                   // Platoon size >= 2
    {  
	     // Cycling through all possible positions-in-platoon using the
	     // pip as an index, and ignoring this vehicle's pip, check to 
	     // see whether a received message's global time has changed.
	     
	    N_veh_comm=0;
	    for (pip = 1; pip <= pltn_inf_pt->pltn_size; pip++) // Logic deal with subject vehicle with each individual veh in platoon
	    {		  
		   if (pip != veh_info_pt-> veh_id)     
		   {              
              if (comm_receive_pt[pip]. global_time != comm_time_old[pip])
              {
                 comm_counter[pip]++; 
                 if (comm_counter[pip] > 100) 
                    comm_counter[pip] = 0;                                                                                				 
				 comm_f_counter[pip] = 0;
				 veh_info_pt-> comm_p[pip-1]=0;
                // N_veh_comm++;       // should not be here     
              }
              else   // IF LOOP 2
              {
                 comm_f_counter[pip]++;
				 if (comm_f_counter[pip] > COMM_ERR_BOUND)   // changed on 07/02/16
                 {
                    handshake_start_arr[pip] = OFF;
					veh_info_pt-> comm_p[pip-1]=1;                           
				 	error |= 0x01 << pip;
                 }
              } 
			  comm_time_old[pip] = comm_receive_pt[pip].global_time;  
	  	
			  if (veh_info_pt-> comm_p[0] ==1)  //lead veh only
               	  f_ind_pt-> comm=1;
			  else
				  f_ind_pt-> comm=0;				
           }  
		   else   // self
		   {
				comm_f_counter[pip]=0;
				veh_info_pt-> comm_p[pip-1]= 0;
		   }  // IF LOOP end 
	    }     // for loop end
		
		
		if (veh_info_pt-> comm_p[0] == 0)  // comm with leader; for CACC only
		{		
			handshake_start = 1;		
			comm_info_pt->comm_reply = 1;				
        }  
		if (veh_info_pt-> veh_id == 1)     // added on 03/22/16
		{
			handshake_start = 1;		
			//comm_info_pt->comm_reply = 1;	
			//f_ind_pt-> comm=0; 
		}
    }  // end of pltn size > 1    

#ifdef FOR_PLATOONING
	if (N_veh_comm == (pltn_inf_pt->pltn_size - 1) )
	{
		handshake_start = 1;		
		comm_info_pt->comm_reply = 1;
	}
	else
	{
		handshake_start = 0;		
		comm_info_pt->comm_reply = 0;
	}
#endif

		
	/*if ((comm_prt_sw == ON) && (handshake_start == ON))  // removed on 07_02_16
		{
			comm_prt_sw = OFF;
//			comm_prt1_sw = ON;
			fprintf(stderr, "Handshaking with all ON!\n");
		}*/




    if (veh_info_pt-> veh_id == 1)
       global_time = local_t;         // It is the same as t_ctrl
    else                              // When vehicle_pip >1, use leader vehicle timing as global time
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

    if (f_ind_pt-> comm == 0)
        {
             if (veh_info_pt-> veh_id > 1)			  
                {                   
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
						//con_st_pt-> pre_v = comm_receive_pt[veh_info_pt-> veh_id - 1].vel_traj;     //velocity; changed on 07_04_16
						//con_st_pt-> pre_a = comm_receive_pt[veh_info_pt-> veh_id - 1].acc_traj;     //accel;
						con_st_pt-> pre_v = comm_receive_pt[cnfg_pt-> MyPltnPos - 1].vel_traj;        //velocity;  changed on 07_04_16
						con_st_pt-> pre_a = comm_receive_pt[cnfg_pt-> MyPltnPos - 1].acc_traj;        //accel;
						con_st_pt-> lead_v = comm_receive_pt[1].vel_traj;
						con_st_pt-> lead_a = comm_receive_pt[1].acc_traj;
//                     con_st_pt-> pre_mag_counter = comm_receive_pt1-> marker_counter;    // Should come from communication.
                     }
                }
             //for (pip=1; pip<= (pltn_inf_pt->pltn_size); pip++)      	
             //   pltn_inf_pt-> pltn_fault_mode=max_i(pltn_inf_pt-> pltn_fault_mode, comm_receive_pt[pip]. fault_mode);
        }
	else
		{
			//veh_info_pt-> fault_mode=3;                  // added on 09_13_10
			//pltn_inf_pt-> pltn_fault_mode=3;
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
int read_jbus(float delta_t, float t_filter, long_vehicle_state *pv_can, long_params *pparams, jbus_read_typ* jbus_rd_pt, sens_read_typ* sens_rd_pt, 
			  switch_typ* sw_rd_pt, vehicle_info_typ* veh_info_pt)
{
	static float we_old=0.0, target_d_buff=0.0;
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
	 jbus_rd_pt->  brk_pres = pv_can-> VBRK_BrkAppPressure;
	 jbus_rd_pt->  brk_prm_pres = pv_can-> VBRK_BrkPrimaryPressure;
	 jbus_rd_pt->  brk_2nd_pres = pv_can-> VBRK_BrkSecondaryPressure;
	 jbus_rd_pt->  park_brk_status = pv_can-> VBRK_BrkStatParkBrkActuator;
	 jbus_rd_pt->  park_brk_red_signal = pv_can-> VBRK_ParkBrkRedWarningSignal;
	 jbus_rd_pt->  park_brk_release_status = pv_can-> VBRK_ParkBrkReleaseInhibitStat;

	 /***************************
	 		Road Grade			
	 ***************************/

	 if ( (pv_can-> VP15_RoadInclinationVP15-3.55) < -1.5)
		jbus_rd_pt->  grade = pv_can-> VP15_RoadInclinationVP15-4.5;  //3.55
	 if ( (pv_can-> VP15_RoadInclinationVP15-3.55) > 1.5)
		jbus_rd_pt->  grade = pv_can-> VP15_RoadInclinationVP15-3.55;  //3.55
	 
	 sens_rd_pt->ego_a=pv_can-> Volvo_EgoAcc;
	 sens_rd_pt->ego_v=pv_can-> Volvo_EgoVel*0.2778;  // from [km/hr] ==> [m/s]
	 sens_rd_pt->target_a=pv_can->Volvo_TargetAcc; 
	 sens_rd_pt->target_v=pv_can-> Volvo_TargetVel;   // [m/s]

	 
	 if (t_filter < 1.0)
	 {
		target_d_buff=pv_can-> Volvo_TargetDist;
		sens_rd_pt->target_d=pv_can-> Volvo_TargetDist;
	 }
	 else
	 {		 
			if ( (((int)pv_can-> Volvo_TargetAvailable == 1) && (pv_can-> Volvo_TargetDist > target_d_buff+20.0*delta_t)) ||
				 ((int)pv_can-> Volvo_TargetAvailable == 0) )
			{
				veh_info_pt-> cut_out=ON;
				veh_info_pt-> cut_in=OFF;
			}
			if (((int)pv_can-> Volvo_TargetAvailable == 1) && (pv_can-> Volvo_TargetDist < target_d_buff-20.0*delta_t))
			{
				veh_info_pt-> cut_in=ON;
				veh_info_pt-> cut_out=OFF;
			}

		 if (pv_can-> Volvo_TargetDist > target_d_buff+0.04)
			sens_rd_pt->target_d=target_d_buff+0.04;
		 else if (pv_can-> Volvo_TargetDist < target_d_buff-0.04)
			sens_rd_pt->target_d=target_d_buff-0.04;
		 else
			sens_rd_pt->target_d=pv_can-> Volvo_TargetDist;
		 
		 if ((int)pv_can-> Volvo_TargetAvailable == 1)
		     target_d_buff=sens_rd_pt->target_d;
		 veh_info_pt-> weight = pv_can-> VP15_VehicleWeightVP15;

	 }
	 
	 if (veh_info_pt-> cut_in ==ON)
		veh_info_pt-> cut_in_t += delta_t;
	 else
		veh_info_pt-> cut_in_t = 0.0;

	 sens_rd_pt->target_avail=(int)pv_can-> Volvo_TargetAvailable;

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
int actuate(float delta_t, long_output_typ *pcmd, con_output_typ* con_out_pt, control_state_typ* con_st_pt,
	long_params *pparams, long_output_typ *inactive_ctrl, manager_cmd_typ * mng_cmd_pt, 
	switch_typ *sw_pt, jbus_read_typ* jbus_rd_pt, control_config_typ* cnfg_pt, fault_index_typ* f_ind_pt, vehicle_info_typ *veh_info_pt) 
{       
  float max_tq_we, tmp_rate;
  static float eng_tq_tmp=0.0, eng_tq_buff=0.0, eng_torq_buff=0.0, eng_retard_buff=0.0;
  static int eng_tq_ini=1, eng_retard_ini=1;
  static float trans_t=0.0, tq_f_t=0.0, jk_f_t=0.0, we_buff=600.0;
  static int cmd_count=0;

	cmd_count++;
	if (cmd_count > 20)
		cmd_count=0;

     if (tq_we(jbus_rd_pt-> we, &max_tq_we) != 1)
		 fprintf(stderr, "Call tq_we err!");
	 con_st_pt-> max_tq_we = max_tq_we;
	 
	 
     if (con_out_pt-> con_sw_1 == 1)  // Tq cmd
      {

	      pcmd->engine_command_mode = TSC_TORQUE_CONTROL;
		  pcmd->engine_retarder_command_mode = XBR_NOT_ACTIVE;
		  pcmd->engine_retarder_command_mode = TSC_OVERRIDE_DISABLED; 
		  pcmd->engine_retarder_torque=0.0;		  
          pcmd->brake_command_mode = XBR_NOT_ACTIVE; 
		  pcmd->brake_command_mode = TSC_OVERRIDE_DISABLED; 
		  pcmd->ebs_deceleration = 0.0;

		  	      
		  pcmd->engine_priority=TSC_MEDIUM;     /*TSC_HIGHEST :0; TSC_HIGH: 1; TSC_MEDIUM: 2; TSC_LOW: 3  */
		  
          eng_tq_tmp = (con_out_pt-> y1)/ENGINE_REF_TORQUE;
		
	if (veh_info_pt-> veh_id == 1)
	{
		if (con_st_pt-> max_spd < 26.0*mph2mps)
		{
			tmp_rate=(con_st_pt-> ref_v - con_st_pt-> spd)/3.0;
			if (tmp_rate > 0.3)
				tmp_rate = 0.3;      
			if (tmp_rate < -0.3)
				tmp_rate = -0.3;    	
			eng_tq_tmp = (1.0+tmp_rate)*(eng_tq_tmp)*108.0;	
		}
		else
		  eng_tq_tmp = (eng_tq_tmp)*100.0;			
	}
	else
	    eng_tq_tmp = (eng_tq_tmp)*100.0;

	      if (mng_cmd_pt-> trans_mode == 0)
		  {			  
			  eng_tq_buff=pcmd->engine_torque;
			  trans_t=0.0;
		  }
		  if (mng_cmd_pt-> trans_mode > 12) // except manual to auto		  
		  {
			  trans_t += delta_t;
			  pcmd->engine_torque=eng_tq_buff*(1.0-trans_t/TRANS_T) + (trans_t/TRANS_T)*eng_tq_tmp;			  
		  }
		  else
			  pcmd->engine_torque=eng_tq_tmp;

			// added on 04_14_16
		   if (eng_tq_ini==1 && mng_cmd_pt-> drive_mode > 1)
	       {
				eng_torq_buff=pcmd->engine_torque;
				eng_tq_ini=0;
		   }
		   if (mng_cmd_pt-> drive_mode <= 1)
				eng_tq_ini=1;

		   if ((pcmd->engine_torque > 40.0) && (pcmd->engine_torque > eng_torq_buff+10.0*delta_t) )
			   pcmd->engine_torque = eng_torq_buff+10.0*delta_t;
		   if ((pcmd->engine_torque > 60.0) && (pcmd->engine_torque > eng_torq_buff+9.0*delta_t) )
			   pcmd->engine_torque = eng_torq_buff+9.0*delta_t;
		   if ((pcmd->engine_torque > 80.0) && (pcmd->engine_torque > eng_torq_buff+8.0*delta_t) )
			   pcmd->engine_torque = eng_torq_buff+8.0*delta_t;


		   eng_torq_buff=pcmd->engine_torque;

	      if (pcmd->engine_torque > 98.5)
			pcmd->engine_torque = 98.5;
	
		  
		  if (cmd_count == 10)
			   pcmd->engine_torque =pcmd->engine_torque + 1.0;
		  if (cmd_count == 20)
			   pcmd->engine_torque =pcmd->engine_torque - 1.0;
		  
		  // tq contr fault
		  //if ( (pcmd->engine_torque > 90.0) && ( jbus_rd_pt-> long_accel <=0.1) )
		  if ( (pcmd->engine_torque > 80.0) && ( jbus_rd_pt-> we - we_buff < 2.0) )
			  tq_f_t += delta_t;
		  else
			  tq_f_t=0.0;
		  we_buff=jbus_rd_pt-> we;

		  if (tq_f_t > 0.5)
			f_ind_pt-> torq =1;
		  else
			f_ind_pt-> torq =0;
		  
		  eng_retard_ini=1;

		  if (mng_cmd_pt-> control_mode == 1 || mng_cmd_pt-> control_mode == 2 ) // short dist cut-in
		  {
			  pcmd->engine_torque = 0.0;
			  pcmd->engine_command_mode = XBR_NOT_ACTIVE;
			  pcmd->engine_retarder_command_mode = TSC_TORQUE_CONTROL;		    
			  pcmd->engine_retarder_priority=TSC_HIGH;   /*TSC_HIGHEST :0; TSC_HIGH: 1; TSC_MEDIUM: 2; TSC_LOW: 3  */ 
			  pcmd->engine_retarder_torque=-10.0;	
			  if (cmd_count == 10)
			      pcmd->engine_retarder_torque =  pcmd->engine_retarder_torque - 1.0;
		      if (cmd_count == 20)
			      pcmd->engine_retarder_torque =  pcmd->engine_retarder_torque + 1.0;
		  }


      }
      else                                                         
      {                                                        
          pcmd->engine_command_mode = TSC_OVERRIDE_DISABLED; 
		  pcmd->engine_command_mode = XBR_NOT_ACTIVE;
		  pcmd->engine_torque = 0.0;
		  
      }  

      if( (jbus_rd_pt-> accel_pedal_pos1 > 2.0) || ( sw_pt-> brk_sw == 1) )
		pcmd->engine_command_mode = TSC_OVERRIDE_DISABLED; // indicating driver taking over   

	 

	  /*******************************************

	     Engine retarder command

	  *******************************************/
                   
      if (con_out_pt-> con_sw_3 == 1)  //jk_cmd                 
      {   	  		
		 //pcmd->engine_retarder_torque = 100.0*(con_out_pt-> y12)/JK_REF_TORQUE;	
		 pcmd->engine_retarder_torque = 100.0*(con_out_pt-> y12)/max_f(100.0, (con_st_pt-> max_jk_we));

		 if (pcmd->engine_retarder_torque > 0.0)
			pcmd->engine_retarder_torque = 0.0;

		 if (eng_retard_ini==1 && mng_cmd_pt-> drive_mode > 1)
	       {
				eng_retard_buff=pcmd->engine_retarder_torque;
				eng_retard_ini=0;
		   }
		 if (mng_cmd_pt-> drive_mode <= 1)
				eng_retard_ini=1;

		if ((pcmd->engine_retarder_torque < -60.0) && (pcmd->engine_retarder_torque < eng_retard_buff-15.0*delta_t) )		
		 	pcmd->engine_retarder_torque = eng_retard_buff-15.0*delta_t;
		 if ((pcmd->engine_retarder_torque < -75.0) && (pcmd->engine_retarder_torque < eng_retard_buff- 12.5*delta_t) )		
		 	pcmd->engine_retarder_torque = eng_retard_buff-12.5*delta_t;
		 if ((pcmd->engine_retarder_torque < -90.0) && (pcmd->engine_retarder_torque < eng_retard_buff-10.0*delta_t) )		
		 	pcmd->engine_retarder_torque = eng_retard_buff-10.0*delta_t;

		 eng_retard_buff=pcmd->engine_retarder_torque;
		
		if (veh_info_pt-> veh_id == 1)
		{
			if (pcmd->engine_retarder_torque < -5.0)
				pcmd->engine_retarder_torque=0.25*(pcmd->engine_retarder_torque);
			else if (pcmd->engine_retarder_torque < -6.0)
				//pcmd->engine_retarder_torque=0.3*(pcmd->engine_retarder_torque);
				pcmd->engine_retarder_torque=0.3*(pcmd->engine_retarder_torque);
			else if (pcmd->engine_retarder_torque < -8.0)
				//pcmd->engine_retarder_torque=0.4*(pcmd->engine_retarder_torque);
				pcmd->engine_retarder_torque=0.5*(pcmd->engine_retarder_torque);	
			else if (pcmd->engine_retarder_torque < -10.0)
				pcmd->engine_retarder_torque=0.8*(pcmd->engine_retarder_torque);
				
			else;
		}
		else
		{
			if (pcmd->engine_retarder_torque < -5.0)				
				pcmd->engine_retarder_torque=0.5*(pcmd->engine_retarder_torque);
			else if (pcmd->engine_retarder_torque < -6.0)
				pcmd->engine_retarder_torque=0.65*(pcmd->engine_retarder_torque);
			else if (pcmd->engine_retarder_torque < -8.0)
				pcmd->engine_retarder_torque=0.8*(pcmd->engine_retarder_torque);			
			else;
		}
		  
		 if (pcmd->engine_retarder_torque < -98.5)
			pcmd->engine_retarder_torque = -98.5;
        
		 pcmd->engine_command_mode = XBR_NOT_ACTIVE; 
		 pcmd->engine_torque =0.0;		 
		 pcmd->engine_retarder_command_mode = XBR_ACTIVE;		 
		 pcmd->engine_retarder_priority=TSC_HIGHEST;  /*TSC_HIGHEST :0; TSC_HIGH: 1; TSC_MEDIUM: 2; TSC_LOW: 3  */ 	
		 
		  if (cmd_count == 10)
			    pcmd->engine_retarder_torque =  pcmd->engine_retarder_torque - 1.0;
		  if (cmd_count == 20)
			    pcmd->engine_retarder_torque =  pcmd->engine_retarder_torque + 1.0;
		 
		 // jk fault
		 if ( (pcmd->engine_retarder_torque <= -90.0) && (jbus_rd_pt-> long_accel > -0.1) )
			jk_f_t +=delta_t;
		 else
			jk_f_t=0.0;
		 if (jk_f_t > 0.5)
			f_ind_pt-> jake =1;
		 else
			f_ind_pt-> jake =0;
		 if (f_ind_pt-> jake == 1)
			pcmd->engine_retarder_priority=TSC_HIGHEST;
		 
		 
		 eng_tq_ini=1;
      }                                                        
      else
      {   		 
		  pcmd->engine_retarder_command_mode = XBR_NOT_ACTIVE;
		  pcmd->engine_retarder_command_mode = TSC_OVERRIDE_DISABLED;
		  pcmd->engine_retarder_torque=-0.0;
      }   // jk cmd end                                                     
        
	 
	  /*******************************************

	    Service brake

	  *******************************************/        
      if (con_out_pt-> con_sw_5 == 1)                           // E brk cmd           
      {  		  
	     pcmd->brake_command_mode = XBR_ACTIVE; 		  
	     pcmd->brake_priority=TSC_HIGHEST;              /*TSC_HIGHEST :0; TSC_HIGH: 1; TSC_MEDIUM: 2; TSC_LOW: 3  */    
		 pcmd->engine_command_mode = TSC_OVERRIDE_DISABLED; //Use this if possible   		   
         pcmd->engine_torque = 0.0;
         
		/* if (con_out_pt-> y11 < 0.0)                             // using usyn 03_01_16              
			pcmd->ebs_deceleration = con_out_pt-> y14;     // changed on 11_03_15
         if (pcmd->ebs_deceleration > - 1.2)
			 pcmd->ebs_deceleration = - 1.2;          
		 if (pcmd->ebs_deceleration < - 2.5)
			 pcmd->ebs_deceleration = - 2.5; */         

          

		   eng_tq_ini=1;
      }                                                        
      else                                                         
      {       		   
			pcmd->brake_command_mode = TSC_OVERRIDE_DISABLED; 
		    pcmd->brake_command_mode = XBR_NOT_ACTIVE;
			pcmd->ebs_deceleration = -0.0;
      }   // E-brake end 
              
	  if ( (pcmd->ebs_deceleration <= -1.2) && (jbus_rd_pt-> long_accel >=0.0) )
		  f_ind_pt-> brk = 1;
	  else
		  f_ind_pt-> brk = 0;
                                                                             
                                                                   
      if ((mng_cmd_pt-> drive_mode == 0) || (mng_cmd_pt-> drive_mode == 1) || 
		(mng_cmd_pt-> f_manage_index==3) || (sw_pt->brake_sw == 1) )
     	  memcpy(pcmd ,inactive_ctrl, sizeof(long_output_typ) );
                                                                                                                                                                                                                        
        return 0;                                                  
}                                                                  

