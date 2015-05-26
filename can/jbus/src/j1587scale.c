/**\file
 *	Conversion and scaling of standard J1587 parameters. 
 * 	Byte format used for transmission is converted
 *	to conventional float or integer format. See SAE J1587
 *	documentation for a description of the parameter with a
 *	given PID.
 *
 *	Some of these parameters have not yet been used at path
 *	so no actually DB VAR type exists, for these the decode
 *	to db_value will be to just one field as an unstructured type.
 */

#include "old_include/std_jbus.h"

#undef DEBUG
#ifdef DEBUG
#define	DEBUG_PRINT(a,b)	printf(a,b)
#else
#define DEBUG_PRINT(a,b)
#endif	

int current_mid;

void convert_transmitter_system_status
   (int *param, void *pdbv) /// PID # 2 
{
	unsigned char *db_value = (unsigned char *)pdbv;
	if (db_value) *db_value =*param & 0xff;
	DEBUG_PRINT("TransmitterSystemStatus = %d\n",*param & 0xff);
}

void convert_attention_warning_status
   (int *param, void *pdbv) /// PID # 44 
{
	unsigned char *db_value = (unsigned char *)pdbv;
	if (db_value) *db_value =*param & 0xff;
	DEBUG_PRINT("ProtectLampStatus = %d\n",(*param & 0x10) >> 4);
	DEBUG_PRINT("AmberLampStatus = %d\n",(*param & 0x04) >> 2);
	DEBUG_PRINT("RedLampStatus = %d\n",(*param & 0x01));
}
void convert_trans_retarder_status
   (int *param, void *pdbv) /// PID # 47 
{
	unsigned char *db_value = (unsigned char *)pdbv;
	if (db_value) *db_value =*param & 0x01;
	DEBUG_PRINT("TransRetarderStatus = %d\n",*param & 0x01);
}

void convert_extended_range_barometric_press
   (int *param, void *pdbv) /// PID # 48 
{
	float *db_value = (float *)pdbv;
	if (db_value) *db_value =*param * 0.6;
   DEBUG_PRINT("ExtRangeBarometricPress = %f kPa\n",*param*0.6); /* kPa */
}
void convert_ABS_control_status
   (int *param, void *pdbv) /// PID # 49 
{
	int *db_value = (int *)pdbv;
	if (db_value) *db_value =*param & 0x40; 
   DEBUG_PRINT("ABSOffRoadFunctionSwitch = %d\n",(*param & 0x40) >> 6);
   DEBUG_PRINT("ABSRetarderControl = %d\n",(*param & 0x10) >> 4);
   DEBUG_PRINT("ABSBrakeControl = %d\n",(*param & 0x04) >> 2);
   DEBUG_PRINT("ABSWarningLamp = %d\n",(*param & 0x01));
}
void convert_percent_throttle_valve_position
   (int *param, void *pdbv) /// PID # 51 
{
	float *db_value = (float *)pdbv;
	if (db_value) *db_value =*param*0.4;
   DEBUG_PRINT("PercentThrottleValvePos = %f\n",*param*0.4); /* % */
}
void convert_brake_switch_status
   (int *param, void *pdbv) /// PID # 65 
{
	int *db_value = (int *)pdbv;
	if (db_value) *db_value =*param & 0x04;
   DEBUG_PRINT("BrakeSystemSwitchStatus = %d\n",(*param & 0x04) >> 2);
   DEBUG_PRINT("ServiceBrakeSwitchStatus = %d\n",(*param & 0x01));
}
void convert_torque_limiting_factor
   (int *param, void *pdbv) /// PID # 68 
{
	float *db_value = (float *)pdbv;
	if (db_value) *db_value =*param*0.5;
   DEBUG_PRINT("TorqueLimitingFactor = %f\n",*param*0.5); /*  */
}
void convert_parking_brake_switch_status
   (int *param, void *pdbv) /// PID # 70 
{
	int *db_value = (int *)pdbv;
	if (db_value) *db_value =*param & 0x80;
   DEBUG_PRINT("ParkingBrakeStatus = %d\n",(*param & 0x80) >> 7);
}
void convert_idle_shutdown_timer_status
   (int *param, void *pdbv) /// PID # 71 
{
	unsigned char *db_value = (unsigned char *)pdbv;
	if (db_value) *db_value = *param;	
   DEBUG_PRINT("IdleShutdownTimerStatus = %d\n",(*param & 0x80) >> 7);
   DEBUG_PRINT("IdleShutdownTimerFunction = %d\n",(*param & 0x08) >> 3);
   DEBUG_PRINT("IdleShutdownTimerOverride = %d\n",(*param & 0x04) >> 2);
   DEBUG_PRINT("EngineShutdownByIdleTimer = %d\n",(*param & 0x02) >> 1);
   DEBUG_PRINT("DriverAlertMode = %d\n",(*param & 0x01));
}
void convert_forward_rear_drive_axle_temp
   (int *param, void *pdbv) /// PID # 77 
{
	float *db_value = (float *)pdbv;
	if (db_value) *db_value =*param*1.2;
   DEBUG_PRINT("ForRearDriveAxleTemp = %f Deg. F\n",*param*1.2); /* Deg. F */
}
void convert_rear_rear_drive_axle_temp
   (int *param, void *pdbv) /// PID # 78 
{
	float *db_value = (float *)pdbv;
	if (db_value) *db_value =*param*1.2;
   DEBUG_PRINT("RearRearDriveAxleTemp = %f Deg. F\n",*param*1.2); /* Deg. F */
}
void convert_road_speed_limit_status
   (int *param, void *pdbv) /// PID # 83 
{
	unsigned char *db_value = (unsigned char *)pdbv;
	if (db_value) *db_value =*param >> 7;
	DEBUG_PRINT("RoadSpeedLimitStatus = %d\n",*param >> 7);
}
void convert_road_speed
   (int *param, void *pdbv) /// PID # 84 
{
	float *db_value = (float *)pdbv;
	if (db_value) *db_value =*param*0.805;
   if(current_mid == 128) /* Only Read This From the Engine */
   {
	if (db_value) *db_value =*param * 0.805;
  	DEBUG_PRINT("RoadSpeed = %f kph\n",*param*0.805); /* kph */
  	DEBUG_PRINT("%f\n",*param*0.805);
   }
}
void convert_cruise_cont_status
   (int *param, void *pdbv) /// PID # 85 
{
	unsigned char *db_value = (unsigned char *)pdbv;
	if (db_value) *db_value = *param & 0xff;
   DEBUG_PRINT("CruiseMode = %d\n",(*param & 0x80) >> 7);
   DEBUG_PRINT("ClutchSwitch = %d\n",(*param & 0x40) >> 6);
   DEBUG_PRINT("BrakeSwitch = %d\n",(*param & 0x20) >> 5);
   DEBUG_PRINT("AccelSwitch = %d\n",(*param & 0x10) >> 4);
   DEBUG_PRINT("ResumeSwitch = %d\n",(*param & 0x08) >> 3);
   DEBUG_PRINT("CoastSwitch = %d\n",(*param & 0x04) >> 2);
   DEBUG_PRINT("SetSwitch = %d\n",(*param & 0x02) >> 1);
   DEBUG_PRINT("CruiseContSwitch = %d\n",(*param & 0x01));
}
void convert_cruise_cont_set_kph
   (int *param, void *pdbv) /// PID # 86 
{
	float *db_value = (float *)pdbv;
	if (db_value) *db_value = *param*0.5;
   DEBUG_PRINT("CruiseContSetKph = %f kph\n",*param*0.5); /* kph */
}
void convert_power_takeoff_status
   (int *param, void *pdbv) /// PID # 89 
{
	unsigned char *db_value = (unsigned char *)pdbv;
	if (db_value) *db_value =*param & 0xff;
	DEBUG_PRINT("PTOMode = %d\n",(*param & 0x80) >> 7);
	DEBUG_PRINT("ClutchSwitch = %d\n",(*param & 0x40) >> 6);
	DEBUG_PRINT("BrakeSwitch = %d\n",(*param & 0x20) >> 5);
	DEBUG_PRINT("AccelSwitch = %d\n",(*param & 0x10) >> 4);
	DEBUG_PRINT("ResumeSwitch = %d\n",(*param & 0x08) >> 3);
	DEBUG_PRINT("CoastSwitch = %d\n",(*param & 0x04) >> 2);
	DEBUG_PRINT("SetSwitch = %d\n",(*param & 0x02) >> 1);
	DEBUG_PRINT("PTOControlSwitch = %d\n",(*param & 0x01));
}
void convert_percent_throttle
   (int *param, void *pdbv) /// PID # 91 
{
	float *db_value = (float *)pdbv;
	if (db_value) *db_value =*param * 0.4;
	DEBUG_PRINT("PercentThrottlePosition = %f \%\n",*param*0.4);
	DEBUG_PRINT("%f\n",*param*0.4); /* % */
}
void convert_percent_engine_load
   (int *param, void *pdbv) /// PID # 92 
{
	float *db_value = (float *)pdbv;
	if (db_value) *db_value =*param * 0.5;
	DEBUG_PRINT("PercentEngineLoad = %f \%\n",*param*0.5); 
}
void convert_output_torque
   (int *param, void *pdbv) /// PID # 93 
{
	float *db_value = (float *)pdbv;
	short bit = *param;
	if (db_value) *db_value = *param * 27.1;

   if((bit >> 7) == 1)
   {
      /* Value is Negative, Take 2's Complement */
      bit = (~bit & 0xff)+0x01;
  	DEBUG_PRINT("OutputTorque = %f N-m\n",-27.1*((float) bit)); /* N-m */
   }
   else
   {
      /* Value is Positive */
  	DEBUG_PRINT("OutputTorque = %f N-m\n",27.1*((float) bit)); /* N-m */
   }
}
void convert_fuel_level
   (int *param, void *pdbv) /// PID # 96 
{
	float *db_value = (float *)pdbv;
	if (db_value) *db_value =*param * 0.5;
   DEBUG_PRINT("FuelLevel = %f \%\n",*param*0.5); 
}
void convert_engine_oil_pressure
   (int *param, void *pdbv) /// PID # 100 
{
	float *db_value = (float *)pdbv;
	if (db_value) *db_value =*param * 3.45;
	DEBUG_PRINT("EngineOilPress = %f kPa\n",*param*3.45); /* kPa */
}
void convert_boost_pressure
   (int *param, void *pdbv) /// PID # 102 
{
	float *db_value = (float *)pdbv;
	if (db_value) *db_value =*param * 0.862;
	DEBUG_PRINT("BoostPress = %f kPa\n",*param*0.862); /* kPa */
}
void convert_turbo_speed
   (int *param, void *pdbv) /// PID # 103 
{
	float *db_value = (float *)pdbv;
	if (db_value) *db_value =*param * 500.0;
	DEBUG_PRINT("TurboSpeed = %f rpm\n",*param*500.0); /* rpm */
}
void convert_intake_manifold_air_temp
   (int *param, void *pdbv) /// PID # 105 
{
	float *db_value = (float *)pdbv;
	if (db_value) *db_value = *param*1.0; 
   DEBUG_PRINT("IntakeManifoldAirTemp = %f Deg. F\n",*param*1.0); /* Deg. F */
}
void convert_air_Inlet_press
   (int *param, void *pdbv) /// PID # 106 
{
	float *db_value = (float *)pdbv;
	if (db_value) *db_value = *param*1.0; 
   DEBUG_PRINT("AirInletPress = %f kPa\n",*param*1.724); /* kPa */
}
void convert_barometric_press
   (int *param, void *pdbv) /// PID # 108 
{
	float *db_value = (float *)pdbv;
	if (db_value) *db_value = *param*0.431;
   DEBUG_PRINT("BarometricPress = %f kPa\n",*param*0.431); /* kPa */
}
void convert_engine_coolant_temp
   (int *param, void *pdbv) /// PID # 110 
{
	float *db_value = (float *)pdbv;
	if (db_value) *db_value = *param*1.0;
   DEBUG_PRINT("EngineCoolantTemp = %f Deg. F\n",*param*1.0); /* Deg. F */
}
void convert_coolant_level
   (int *param, void *pdbv) /// PID # 111 
{
	float *db_value = (float *)pdbv;
	if (db_value) *db_value = *param*0.5;
	DEBUG_PRINT("CoolantLevel = %f %\n",*param*0.5); /* % */
}
void convert_hydraulic_retarder_oil_temp
   (int *param, void *pdbv) /// PID # 120 
{
	float *db_value = (float *)pdbv;
	if (db_value) *db_value =*param * 2.0;
	DEBUG_PRINT("HydraulicRetarderOilTemp = %f Deg. F\n",*param*2.0); /* Deg. F */
}
void convert_engine_retarder_status
   (int *param, void *pdbv) /// PID # 121 
{
	float *db_value = (float *)pdbv;
	if (db_value) *db_value = *param & 0xff; 
   DEBUG_PRINT("EngineRetarderStatus = %d\n",(*param & 0x80) >> 7);
   DEBUG_PRINT("Cylinder8Active = %d\n",(*param & 0x10) >> 4);
   DEBUG_PRINT("Cylinder6Active = %d\n",(*param & 0x08) >> 3);
   DEBUG_PRINT("Cylinder4Active = %d\n",(*param & 0x04) >> 2);
   DEBUG_PRINT("Cylinder3Active = %d\n",(*param & 0x02) >> 1);
   DEBUG_PRINT("Cylinder2Active = %d\n",(*param & 0x01));
}
void convert_comp_specific_para_req
   (int *param, void *pdbv) /// PID # 128 
{
	int *db_value = (int *)pdbv;
	if (db_value) *db_value = *param;
}
void convert_ATC_control_status
   (int *param, void *pdbv) /// PID # 151 
{
	unsigned char *db_value = (unsigned char *)pdbv;
	if (db_value) *db_value = *param;
   DEBUG_PRINT("ATCSpinOutSignalDetect = %d\n",(*param & 0x40) >> 6);
   DEBUG_PRINT("ATCEngineControl = %d\n",(*param & 0x10) >> 4);
   DEBUG_PRINT("ATCBrakeControl = %d\n",(*param & 0x04) >> 2);
   DEBUG_PRINT("ATCStatusLamp = %d\n",(*param & 0x01));

   DEBUG_PRINT("ATCSnowMudFuncSwitch = %d\n",(*(param+1) & 0x01));
}
void convert_switched_battery_potential
   (int *param, void *pdbv) /// PID # 158 
{
	int *db_value = (int *)pdbv;
	if (db_value) *db_value = *param;
   DEBUG_PRINT("SwitchedBatteryVolts = %f Vdc\n",
           0.05*((float)(*(param+1)*256 + *param))); /* Volts */
}
void convert_transmission_range_sel
   (int *param, void *pdbv) /// PID # 162 
{
	int *db_value = (int *)pdbv;
	char c = *param;	/* assume second character not interesting */
	if (db_value) *db_value = *param;
	switch (c) {
	case 'N': *db_value = 0;
			break;
	case 'R': *db_value = -1;
			break;
	case 'P': *db_value = 251;	/* J1939 standard for Park */ 
			break;
	default:  if (c >= '1' && c <= '9')
			*db_value = c - '0';
		  else
			*db_value = 255;	/* error code */  
		break;
	}
	DEBUG_PRINT("TransRangeSelected = %c\n",(char) *param);
}

void convert_transmission_range_att
   (int *param, void *pdbv) /// PID # 163 
{
	int *db_value = (int *)pdbv;
	char c = (char) *param;	/* assume second character not interesting */
	if (db_value) *db_value = *param;
	switch (c) {
	case 'N': *db_value = 0;
			break;
	case 'R': *db_value = -1;
			break;
	case 'P': *db_value = 251;	/* J1939 standard for Park */ 
			break;
	default:  if (c >= '1' && c <= '9')
			*db_value = c - '0';
		  else
			*db_value = 255;	/* error code */  
		break;
	}
	DEBUG_PRINT("TransRangeAttained = %c\n",(char) *param);
}

void convert_battery_volts
   (int *param, void *pdbv) /// PID # 168 
{
	float *db_value = (float *)pdbv;
	if (db_value) *db_value = 0.05*((float)(*(param+1)*256 + *param));
   DEBUG_PRINT("BatteryVolts = %f Vdc\n",
           0.05*((float)(*(param+1)*256 + *param))); /* Volts */
}

void convert_ambient_air_temp
   (int *param, void *pdbv) /// PID # 171 
{
	float *db_value = (float *)pdbv;
	int bit;
	if (db_value) *db_value = 0.05*((float)(*(param+1)*256 + *param));
   bit = *(param+1)*256 + *param;
   if((bit >> 15) == 1)
   {
      /* Value is Negative */
      /* Take 2's Complement */
      bit = (~bit & 0xffff)+0x01;
  	DEBUG_PRINT("AmbAirTemp = %f Deg. F\n",-0.25*((float) bit)); /* Deg. F */
   }
   else
   {
      /* Value is Positive */
  	DEBUG_PRINT("AmbAirTemp = %f Deg. F\n",0.25*((float) bit)); /* Deg. F */
   }
}
void convert_air_inlet_temp
   (int *param, void *pdbv) /// PID # 172 
{
	float *db_value = (float *)pdbv;
	int bit;
	if (db_value) *db_value = 0.05*((float)(*(param+1)*256 + *param));
   bit = *(param+1)*256 + *param;
   if((bit >> 15) == 1)
   {
      /* Value is Negative */
      /* Take 2's Complement */
      bit = (~bit & 0xffff)+0x01;
  	DEBUG_PRINT("AirInletTemp = %f Deg. F\n",-0.25*((float) bit)); /* Deg. F */
   }
   else
   {
      /* Value is Positive */
  	DEBUG_PRINT("AirInletTemp = %f Deg. F\n",0.25*((float) bit)); /* Deg. F */
   }
}
void convert_exhaust_gas_temp
   (int *param, void *pdbv) /// PID # 173 
{
	float *db_value = (float *)pdbv;
	int bit;
	if (db_value) *db_value = 0.05*((float)(*(param+1)*256 + *param));
   bit = *(param+1)*256 + *param;
   if((bit >> 15) == 1)
   {
      /* Value is Negative */
      /* Take 2's Complement */
      bit = (~bit & 0xffff)+0x01;
  	DEBUG_PRINT("ExhaustGasTemp = %f Deg. F\n",-0.25*((float) bit)); /* Deg. F */
   }
   else
   {
      /* Value is Positive */
  	DEBUG_PRINT("ExhaustGasTemp = %f Deg. F\n",0.25*((float) bit)); /* Deg. F */
   }
}
void convert_fuel_temp
   (int *param, void *pdbv) /// PID # 174 
{
	float *db_value = (float *)pdbv;
	int bit;
	if (db_value) *db_value = 0.05*((float)(*(param+1)*256 + *param));
   bit = *(param+1)*256 + *param;
   if((bit >> 15) == 1)
   {
      /* Value is Negative */
      /* Take 2's Complement */
      bit = (~bit & 0xffff)+0x01;
  	DEBUG_PRINT("FuelTemp = %f Deg. F\n",-0.25*((float) bit)); /* Deg. F */
   }
   else
   {
      /* Value is Positive */
  	DEBUG_PRINT("FuelTemp = %f Deg. F\n",0.25*((float) bit)); /* Deg. F */
   }
}
void convert_engine_oil_temp
   (int *param, void *pdbv) /// PID # 175 
{
	float *db_value = (float *)pdbv;
   int bit;
   bit = *(param+1)*256 + *param;
   if((bit >> 15) == 1)
   {
      /* Value is Negative */
      /* Take 2's Complement */
      bit = (~bit & 0xffff)+0x01;
	if (db_value) *db_value =-0.25*((float) bit);
  	DEBUG_PRINT("EngineOilTemp = %f Deg. F\n",-0.25*((float) bit)); /* Deg. F */
   }
   else
   {
      /* Value is Positive */
	if (db_value) *db_value =0.25*((float) bit);
  	DEBUG_PRINT("EngineOilTemp = %f Deg. F\n",0.25*((float) bit)); /* Deg. F */
   }
}
void convert_transmission_oil_temp
   (int *param, void *pdbv) /// PID # 177 
{
	float *db_value = (float *)pdbv;
	short bit;
	bit = TWOBYTES(param[1], param[0]);
	if (db_value) *db_value =0.25 * bit;
}
void convert_trip_fuel
   (int *param, void *pdbv) /// PID # 182 
{
	float *db_value = (float *)pdbv;
	if (db_value) *db_value = 0.125*((float)(*(param+1)*256 + *param));
   DEBUG_PRINT("TripFuel = %f gal\n",
           0.125*((float)(*(param+1)*256 + *param))); /* gal */
}
void convert_fuel_rate
   (int *param, void *pdbv) /// PID # 183 
{
	float *db_value = (float *)pdbv;
	if (db_value) *db_value = (4.34e-6)*((float)(*(param+1)*256 + *param));
   DEBUG_PRINT("FuelRate = %f gal/dec\n",
           (4.34e-6)*((float)(*(param+1)*256 + *param))); /* gal/sec */
}
void convert_instantaneous_MPG
   (int *param, void *pdbv) /// PID # 184 
{
	float *db_value = (float *)pdbv;
	if (db_value) *db_value = (1.0/256.0)*((float)(*(param+1)*256 + *param));
   DEBUG_PRINT("InstMPG = %f MPG\n",
           (1.0/256.0)*((float)(*(param+1)*256 + *param))); /* MPG */
}
void convert_avg_MPG
   (int *param, void *pdbv) /// PID # 185 
{
	float *db_value = (float *)pdbv;
	if (db_value) *db_value = (1.0/256.0)*((float)(*(param+1)*256 + *param));
   DEBUG_PRINT("AvgMPG = %f MPG\n",
           (1.0/256.0)*((float)(*(param+1)*256 + *param))); /* MPG */
}
void convert_power_takeoff_set_speed
   (int *param, void *pdbv) /// PID # 187 
{
	float *db_value = (float *)pdbv;
	if (db_value) *db_value = (1.0/256.0)*((float)(*(param+1)*256 + *param));
   DEBUG_PRINT("PowTakeoffSetSpeed = %f rpm\n",
           (0.25)*((float)(*(param+1)*256 + *param))); /* rpm */
}
void convert_engine_speed
   (int *param, void *pdbv) /// PID # 190 
{
	float *db_value = (float *)pdbv;
	if (db_value) *db_value = 0.25*((float)(*(param+1)*256 + *param)); 
   DEBUG_PRINT("EngineSpeed = %f rpm\n",
           0.25*((float)(*(param+1)*256 + *param))); /* rpm */
   DEBUG_PRINT("%f\n", 0.25*((float)(*(param+1)*256 + *param))); /* rpm */
}
void convert_transmission_output_shaft_speed
   (int *param, void *pdbv) /// PID # 191 
{
	float *db_value = (float *)pdbv;
	if (db_value) *db_value =0.25 * TWOBYTES(param[1], param[0]);
	DEBUG_PRINT("TransOutShaftSpeed = %f rpm\n",
           0.25*((float)(*(param+1)*256 + *param))); /* rpm */
}
void convert_trans_sys_diag_code
   (int *param, void *pdbv) /// PID # 194 
{
	int *db_value = (int *)pdbv;
	if (db_value) *db_value = *param; 
}
void convert_speed_sensor_calib
   (int *param, void *pdbv) /// PID # 228 
{
	int *db_value = (int *)pdbv;
	if (db_value) *db_value = *param; 
   DEBUG_PRINT("SpeedSensorCalib = %d Pul/km\n",
                      (int) (0.621*(*(param+4)*16777216 + *(param+3)*65536
                    + *(param+2)*256 + *(param+1)))); /* Pulses per km */
}
void convert_total_idle_hours
   (int *param, void *pdbv) /// PID # 235 
{
	int *db_value = (int *)pdbv;
	if (db_value) *db_value = *param; 
   DEBUG_PRINT("TotalIdleHours = %d hr\n",
                      (int) (0.05*(*(param+4)*16777216 + *(param+3)*65536
                    + *(param+2)*256 + *(param+1)))); /* hr */
}
void convert_total_idle_fuel_used
   (int *param, void *pdbv) /// PID # 236 
{
	int *db_value = (int *)pdbv;
	if (db_value) *db_value = *param; 
   DEBUG_PRINT("TotalIdleFuel = %d gal\n",
                      (int) (0.125*(*(param+4)*16777216 + *(param+3)*65536
                    + *(param+2)*256 + *(param+1)))); /* gal */
}
void convert_trip_distance
   (int *param, void *pdbv) /// PID # 244 
{
	float *db_value = (float *)pdbv;
	if (db_value) *db_value = (int) (0.16*(*(param+4)*16777216 + *(param+3)*65536
                    + *(param+2)*256 + *(param+1)));
   DEBUG_PRINT("TripDistanceKm = %d km\n",
                      (int) (0.16*(*(param+4)*16777216 + *(param+3)*65536
                    + *(param+2)*256 + *(param+1)))); /* km */
}
void convert_total_vehicle_km
   (int *param, void *pdbv) /// PID # 245 
{
	float *db_value = (float *)pdbv;
	if (db_value) *db_value = (int) (0.161*(*(param+4)*16777216 + *(param+3)*65536 + *(param+2)*256 + *(param+1)));
   DEBUG_PRINT("TotalVehicleKm = %d km\n",
                      (int) (0.161*(*(param+4)*16777216 + *(param+3)*65536
                    + *(param+2)*256 + *(param+1)))); /* km */
}
void convert_total_engine_hours
   (int *param, void *pdbv) /// PID # 247 
{
	float *db_value = (float *)pdbv;
	if (db_value) *db_value = (int) (0.161*(*(param+4)*16777216 + *(param+3)*65536 + *(param+2)*256 + *(param+1)));
   DEBUG_PRINT("TotalEngineHours = %d hr\n",
                      (int) (0.05*(*(param+4)*16777216 + *(param+3)*65536
                    + *(param+2)*256 + *(param+1)))); /* hr */
}
void convert_total_PTO_hours
   (int *param, void *pdbv) /// PID # 248 
{
	float *db_value = (float *)pdbv;
	if (db_value) *db_value = (int) (0.161*(*(param+4)*16777216 + *(param+3)*65536 + *(param+2)*256 + *(param+1)));
   DEBUG_PRINT("TotalPTOHours = %d hr\n",
                      (int) (0.05*(*(param+4)*16777216 + *(param+3)*65536
                    + *(param+2)*256 + *(param+1)))); /* hr */
}
void convert_total_fuel_used
   (int *param, void *pdbv) /// PID # 250 
{
	float *db_value = (float *)pdbv;
	if (db_value) *db_value = (int) (0.161*(*(param+4)*16777216 + *(param+3)*65536 + *(param+2)*256 + *(param+1)));
   DEBUG_PRINT("TotalFuelUsed = %d gal\n",
                      (int) (0.125*(*(param+4)*16777216 + *(param+3)*65536
                    + *(param+2)*256 + *(param+1)))); /* gal */
}
void convert_data_link_escape
   (int *param, void *pdbv) /// PID # 254 
{
	int *db_value = (int *)pdbv;
	if (db_value) *db_value = *param;
}

void no_op
   (int *param, void *pdbv) /// PID # 255 
{
	int *db_value = (int *)pdbv;
	if (db_value) *db_value = *param;
}

void 
print_enga(void *pdbv, FILE  *fp, int numeric)

{
	j1587_enga_typ *enga = (j1587_enga_typ *)pdbv;

	fprintf(fp, "ENGA ");
	print_timestamp(fp, &enga->timestamp);
	if (numeric) {
		fprintf(fp," %.2f", enga->road_speed);
		fprintf(fp," %.2f", enga->percent_throttle);
		fprintf(fp," %.2f", enga->percent_engine_load);
		fprintf(fp," %.2f", enga->engine_speed);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp,"Road speed %.2f\n",
			 enga->road_speed);
		fprintf(fp,"Percent throttle %.2f\n",
			 enga->percent_throttle);
		fprintf(fp,"Percent engine load %.2f\n",
			 enga->percent_engine_load);
		fprintf(fp,"Engine speed (rpm) %.2f\n",
			 enga->engine_speed);
	}
}

void 
print_engb(void *pdbv, FILE  *fp, int numeric)

{
	j1587_engb_typ *engb = (j1587_engb_typ *)pdbv;
	fprintf(fp, "ENGB ");
	print_timestamp(fp, &engb->timestamp);
	if (numeric) {
		fprintf(fp," %d", engb->cruise_control_status);
		fprintf(fp," %d", engb->engine_retarder_status);
		fprintf(fp," %f", engb->fuel_rate);
		fprintf(fp," %.2f", engb->instantaneous_mpg);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp,"Cruise control status %d\n",
			 engb->cruise_control_status);
		fprintf(fp,"Engine retarder status %d\n",
			 engb->engine_retarder_status);
		fprintf(fp,"Fuel rate %f\n",
			 engb->fuel_rate);
		fprintf(fp,"Instantaneous MPG %.2f\n",
			 engb->instantaneous_mpg);
	}
}

void 
print_engc(void *pdbv, FILE  *fp, int numeric)

{
	j1587_engc_typ *engc = (j1587_engc_typ *)pdbv;
	fprintf(fp, "ENGC ");
	print_timestamp(fp, &engc->timestamp);
	if (numeric) {
		fprintf(fp," %d", engc->transmitter_system_status);
		fprintf(fp," %d", engc->idle_shutdown_timer_status);
		fprintf(fp," %d", engc->road_speed_limit_status);
		fprintf(fp," %d", engc->power_takeoff_status);
		fprintf(fp," %.2f", engc->oil_pressure);
		fprintf(fp," %.2f", engc->boost_pressure);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp,"Transmitter system status %d\n",
			 engc->transmitter_system_status);
		fprintf(fp,"Idle shutdown timer status %d\n",
			 engc->idle_shutdown_timer_status);
		fprintf(fp,"Road speed limit status %d\n",
			 engc->road_speed_limit_status);
		fprintf(fp,"Power takeoff status %d\n",
			 engc->power_takeoff_status);
		fprintf(fp,"Oil pressure %.2f\n",
			 engc->oil_pressure);
		fprintf(fp,"Boost pressure %.2f\n",
			 engc->boost_pressure); 
	} 
}

void
print_engd(void *pdbv, FILE *fp, int numeric)

{
	j1587_engd_typ *engd = (j1587_engd_typ *)pdbv;
	fprintf(fp, "ENGD ");
	print_timestamp(fp, &engd->timestamp);
	if (numeric) {
		fprintf(fp," %.2f", engd->intake_air_temperature);
		fprintf(fp," %.2f", engd->barometric_pressure);
		fprintf(fp," %.2f", engd->engine_coolant_temperature);
		fprintf(fp," %.2f", engd->volts);
		fprintf(fp," %.2f", engd->oil_temperature);
		fprintf(fp, "\n");

	} else {
		fprintf(fp,"Intake air temperature %.2f\n",
			 engd->intake_air_temperature);
		fprintf(fp,"Barometric pressure %.2f\n",
			 engd->barometric_pressure);
		fprintf(fp,"Engine coolant temperature %.2f\n",
			 engd->engine_coolant_temperature);
		fprintf(fp,"Volts %.2f\n", engd->volts);
		fprintf(fp,"Oil temperature %.2f\n", engd->oil_temperature);

	}
}

void
print_enge(void *pdbv, FILE *fp, int numeric)

{
	j1587_enge_typ *enge = (j1587_enge_typ *)pdbv;
	fprintf(fp, "ENGE ");
	print_timestamp(fp, &enge->timestamp);
	if (numeric) {
		fprintf(fp," %.2f", enge->trans_sys_diag_code);
		fprintf(fp, "\n"); 
	} else {
		fprintf(fp,"Transmitter system diagnostics and occurrence count %.2f\n", enge->trans_sys_diag_code);
	}
}

void
print_engf(void *pdbv, FILE *fp, int numeric)

{
	j1587_engf_typ *engf = (j1587_engf_typ *)pdbv;
	fprintf(fp, "ENGF ");
	print_timestamp(fp, &engf->timestamp);
	if (numeric) {
		fprintf(fp," %.2f", engf->cruise_control_set_MPH);
		fprintf(fp," %.2f", engf->trip_fuel);
		fprintf(fp," %.2f", engf->average_MPG);
		fprintf(fp, "\n");
	} else {
		fprintf(fp,"Cruise control set MPH %.2f\n", engf->cruise_control_set_MPH);
		fprintf(fp,"Trip fuel %.2f\n", engf->trip_fuel);
		fprintf(fp,"Average MPG %.2f\n", engf->average_MPG);
	}
}

void
print_engg(void *pdbv, FILE *fp, int numeric)

{
	j1587_engg_typ *engg = (j1587_engg_typ *)pdbv;
	fprintf(fp, "ENGG ");
	print_timestamp(fp, &engg->timestamp);
	if (numeric) {
		fprintf(fp," %.2f", engg->trip_distance);
		fprintf(fp," %.2f", engg->total_vehicle_miles);
		fprintf(fp, "\n");
	} else {
		fprintf(fp,"Trip distance %.2f\n", engg->trip_distance);
		fprintf(fp,"Total vehicle miles %.2f\n",
			 engg->total_vehicle_miles);
	}
}

void 
print_trans(void *pdbv, FILE  *fp, int numeric)

{
	j1587_trans_typ *trans = (j1587_trans_typ *)pdbv;
	fprintf(fp, "TRANS ");
	print_timestamp(fp, &trans->timestamp);
	if (numeric) {
		fprintf(fp," %.2f", trans->hydraulic_retarder_oil_temp);
		fprintf(fp," %.2f", trans->transmission_oil_temp);
		fprintf(fp," %.2f", trans->transmission_output_shaft_speed);
		fprintf(fp," %d", trans->transmission_retarder_status);
		fprintf(fp," %d", trans->transmission_range_selected);
		fprintf(fp," %d", trans->transmission_range_attained);
		fprintf(fp," %d", trans->attention_warning_status);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp,"Hydraulic retarder oil temp %.2f\n",
			 trans->hydraulic_retarder_oil_temp);
		fprintf(fp,"Transmission oil temp %.2f\n",
			 trans->transmission_oil_temp);
		fprintf(fp,"Transmission output shaft speed %.2f\n",
			 trans->transmission_output_shaft_speed);
		fprintf(fp,"Transmission retarder status %d\n",
			 trans->transmission_retarder_status);
		fprintf(fp,"Transmission range selected %d\n",
			 trans->transmission_range_selected);
		fprintf(fp,"Transmission range attained %d\n",
			 trans->transmission_range_attained);
		fprintf(fp,"Attention/warning status %d\n",
			 trans->attention_warning_status);
	}
}
