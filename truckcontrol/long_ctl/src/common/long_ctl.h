/* FILE
 *   long_ctl.h
 *
 * Structures and includes for longitudinal control programs.
 *
 */

#ifndef LONG_CTL_H
#define LONG_CTL_H

#include <sys_os.h>
#include <sys_list.h>
#include <sys_buff.h>
#include <sys/stat.h>
#include <sys_lib.h>
#include <sys_rt.h>
#include <sys_ini.h>
#include "db_comm.h"
#include "db_clt.h"
#include "db_utils.h"
#include "timestamp.h"
#include "jbus_vars.h"
#include "jbus_extended.h"
#include "j1939.h"
#include "j1939pdu_extended.h"
#include "j1939db.h"
#include "evt300.h"
#include <timing.h>
#include "clt_vars.h"
#include "densolidar.h"
#include "mdl.h"
#include "path_gps_lib.h"
#include "long_comm.h"
#include "veh_trk.h"
#include "dvi.h"

// defined in long_ctl.c, used by tasks to control how much to print
extern int long_ctl_verbose;

/* Longitudinal control structures */

/* long_vehicle_state includes description of a vehicle's current state
 * as read from the database, or transmitted by vehicle-to-vehicle
 * communication from another vehicle. 
 */
typedef struct {
/* long_input_typ data */
  float acc_pedal_voltage;

/* j1939 bus data */
  float EEC1_EngineSpeed;		/* from engine */
  float EEC1_DrvrDemandEngPercentTorque;		/* from engine */
  float EEC1_ActualEnginePercentTorque;		/* from engine */
  float EEC1_EngDemandPercentTorque;		/* from engine */
  int EEC1_EngineTorqueMode;		/* from engine */
  float accelerator_pedal_position;	/* from engine */

  int EEC2_AccelPedal1LowIdleSwitch;
  int EEC2_AccelPedal2LowIdleSwitch;
  int EEC2_AccelPedalKickdownSwitch;
  float EEC2_AccelPedalPos1;
  float EEC2_AccelPedalPos2;
  int  EEC2_ActMaxAvailEngPercentTorque;
  float EEC2_EnginePercentLoadAtCurrentSpd;
  int  EEC2_RoadSpeedLimitStatus;

  float EEC3_NominalFrictionPercentTorque;
  float EEC3_EstEngPrsticLossesPercentTorque;
  float EEC3_EngsDesiredOperatingSpeed;

  float EBC1_BrakePedalPosition;
  int EBSBrakeSwitch;
  int ABSEBSAmberWarningSignal;
  int EBC1_ABSFullyOperational;
  int EBC1_AntiLockBrakingActive;
  int EBC1_ASRBrakeControlActive;
  int EBC1_ASREngineControlActive;
  int EBC1_ASROffroadSwitch;
  int EBC1_EBSRedWarningSignal;
  float EBC1_EngRetarderSelection;
  int EBC1_RemoteAccelEnableSwitch;
  int EBC1_SrcAddrssOfCtrllngDvcFrBrkCntrl;

  float EBC2_FrontAxleSpeed;	    /* from ebc2 on D1 */
  float EBC2_RelativeSpeedFrontAxleLeftWheel;  /* from ebc2 on D1 */
  float EBC2_RlativeSpeedFrontAxleRightWheel; /* from ebc2 on D1 */
  float EBC2_RelativeSpeedRearAxle1LeftWheel;  /* from ebc2 on D1 */
  float EBC2_RlativeSpeedRearAxle1RightWheel; /* from ebc2 on D1 */
  float EBC2_RelativeSpeedRearAxle2LeftWheel;  /* from ebc2 on D1 */
  float EBC2_RlativeSpeedRearAxle2RightWheel; /* from ebc2 on D1 */

  unsigned char EBC5_FoundationBrakeUse;
  unsigned char EBC5_HaltBrakeMode;
  unsigned char EBC5_XBRAccelerationLimit;
  unsigned char EBC5_XBRActiveControlMode;

  float VBRK_BrkAppPressure;
  float VBRK_BrkPrimaryPressure;
  float VBRK_BrkSecondaryPressure;
  unsigned char VBRK_BrkStatParkBrkActuator;
  unsigned char VBRK_ParkBrkRedWarningSignal;
  unsigned char VBRK_ParkBrkReleaseInhibitStat;

  unsigned char CCVS_ParkingBrakeSwitch;
  unsigned char CCVS_TwoSpeedAxleSwitch;
  float CCVS_VehicleSpeed;
  unsigned char CCVS_ParkBrakeReleaseInhibitRq ;
  unsigned char CCVS_ClutchSwitch;
  unsigned char CCVS_BrakeSwitch;
  unsigned char CCVS_CruiseCtrlPauseSwitch;
  unsigned char CCVS_CruiseControlEnableSwitch;
  unsigned char CCVS_CruiseControlActive;
  unsigned char CCVS_CruiseControlAccelerateSwitch;
  unsigned char CCVS_CruiseControlResumeSwitch;
  unsigned char CCVS_CruiseControlCoastSwitch;
  unsigned char CCVS_CruiseControlSetSwitch;
  float CCVS_CruiseControlSetSpeed;
  unsigned char CCVS_CruiseControlState;
  unsigned char CCVS_PtoState;
  unsigned char CCVS_EngShutdownOverrideSwitch;
  unsigned char CCVS_EngTestModeSwitch;
  unsigned char CCVS_EngIdleDecrementSwitch;
  unsigned char CCVS_EngIdleIncrementSwitch;

  unsigned char ETC1_TransmissionDrivelineEngaged;
  unsigned char ETC1_TorqueConverterLockupEngaged;
  unsigned char ETC1_TransmissionShiftInProcess;
  float ETC1_TransmissionOutputShaftSpeed;
  float ETC1_PercentClutchSlip;
  unsigned char ETC1_MomentaryEngineOverspeedEnable;
  unsigned char ETC1_ProgressiveShiftDisable;
  float ETC1_TransInputShaftSpeed;
  unsigned char ETC1_SrcAddrssOfCtrllngDvcFrTrnsCtrl;

  char ETC2_TransmissionSelectedGear;
  float ETC2_TransmissionActualGearRatio;
  char ETC2_TransmissionCurrentGear;
  int ETC2_TransmissionRangeSelected;
  int ETC2_TransmissionRangeAttained;

  float LFE_EngineFuelRate;
  float LFE_EngineInstantaneousFuelEconomy;
  float LFE_EngineAverageFuelEconomy;
  float LFE_EngineThrottleValve1Position;

  unsigned char ERC1ERRetarderEnableShiftAssistSw;
  unsigned char ERC1ERRtdrEnablBrakeAssistSwitch;
  unsigned char ERC1ERRetarderTorqueMode;
  float ERC1ERActualEngineRetPercentTrq;
  float ERC1ERIntendedRtdrPercentTorque;
  unsigned char ERC1ERRetarderRqingBrakeLight;
  unsigned char ERC1ERSrcAddrOfCtrlDvcFrRtdrCtrl;
  int ERC1ERDrvrsDmandRtdrPerctTorque;
  int ERC1ERRetarderSelectionNonEng;
  int ERC1ERActlMxAvlbRtdrtPerctTorque;

  float IC1_EngAirIntakePress;
  float IC1_EngDslPrtclateFilterIntakePress;
  float IC1_EngIntakeManifold1Press;
  float IC1_EngIntakeManifold1Temp;

  float MVS_X_E_AppliedVehicleSpeedLimit_BB1_X_E;

  // Engine control by transmission TSC1_E_T
  unsigned char TSC1_EMSTECU_EnOvrdCtrlM;
  unsigned char TSC1_EMSTECU_EnRSpdCtrlC;
  unsigned char TSC1_EMSTECU_EnRSpdSpdLm;
  unsigned char TSC1_EMSTECU_EnRTrqTrqLm;
  unsigned char TSC1_EMSTECU_OvrdCtrlMPr;
  unsigned char TSC1_EMSTECU_src_address;
  unsigned char TSC1_EMSTECU_destination_address;

  // Engine control by brake TSC1_E_A
  unsigned char TSC1_EMSABS_EnOvrdCtrlM;
  unsigned char TSC1_EMSABS_EnRSpdCtrlC;
  unsigned char TSC1_EMSABS_EnRSpdSpdLm;
  unsigned char TSC1_EMSABS_EnRTrqTrqLm;
  unsigned char TSC1_EMSABS_OvrdCtrlMPr;
  unsigned char TSC1_EMSABS_src_address;
  unsigned char TSC1_EMSABS_destination_address;

  // Engine control by CC TSC1_E_V
  unsigned char TSC1_EMSVMCUes_EnOvrdCtrlM;
  unsigned char TSC1_EMSVMCUes_EnRSpdCtrlC;
  unsigned char TSC1_EMSVMCUes_EnRSpdSpdLm;
  unsigned char TSC1_EMSVMCUes_EnRTrqTrqLm;
  unsigned char TSC1_EMSVMCUes_OvrdCtrlMPr;
  unsigned char TSC1_EMSVMCUes_src_address;
  unsigned char TSC1_EMSVMCUes_destination_address;

  // Engine control by ACC TSC1_E_ACC
  unsigned char TSC1_EMS_ACC_EnOvrdCtrlM;
  unsigned char TSC1_EMS_ACC_EnRSpdCtrlC;
  unsigned char TSC1_EMS_ACC_EnRSpdSpdLm;
  unsigned char TSC1_EMS_ACC_EnRTrqTrqLm;
  unsigned char TSC1_EMS_ACC_OvrdCtrlMPr;
  unsigned char TSC1_EMS_ACC_src_address;
  unsigned char TSC1_EMS_ACC_destination_address;


  // Engine retarder control by transmission TSC1_ER_T
  unsigned char TSC1_EMSrTECU_EnOvrdCtrlM;
  unsigned char TSC1_EMSrTECU_EnRSpdCtrlC;
  unsigned char TSC1_EMSrTECU_EnRSpdSpdLm;
  unsigned char TSC1_EMSrTECU_EnRTrqTrqLm;
  unsigned char TSC1_EMSrTECU_OvrdCtrlMPr;
  unsigned char TSC1_EMSrTECU_src_address;
  unsigned char TSC1_EMSrTECU_destination_address;

  // Engine retarder control by brake TSC1_ER_A
  unsigned char TSC1_EMSrABS_EnOvrdCtrlM;
  unsigned char TSC1_EMSrABS_EnRSpdCtrlC;
  unsigned char TSC1_EMSrABS_EnRSpdSpdLm;
  unsigned char TSC1_EMSrABS_EnRTrqTrqLm;
  unsigned char TSC1_EMSrABS_OvrdCtrlMPr;
  unsigned char TSC1_EMSrABS_src_address;
  unsigned char TSC1_EMSrABS_destination_address;

  // Engine retarder control by CC TSC1_ER_V
  unsigned char TSC1_ER_V_EnOvrdCtrlM;
  unsigned char TSC1_ER_V_EnRSpdCtrlC;
  unsigned char TSC1_ER_V_EnRSpdSpdLm;
  unsigned char TSC1_ER_V_EnRTrqTrqLm;
  unsigned char TSC1_ER_V_OvrdCtrlMPr;
  unsigned char TSC1_ER_V_src_address;
  unsigned char TSC1_ER_V_destination_address;

  // Engine retarder control by ACC TSC1_ER_ACC
  unsigned char TSC1_ER_ACC_EnOvrdCtrlM;
  unsigned char TSC1_ER_ACC_EnRSpdCtrlC;
  unsigned char TSC1_ER_ACC_EnRSpdSpdLm;
  unsigned char TSC1_ER_ACC_EnRTrqTrqLm;
  unsigned char TSC1_ER_ACC_OvrdCtrlMPr;
  unsigned char TSC1_ER_ACC_src_address;
  unsigned char TSC1_ER_ACC_destination_address;

  float VDC2_SteeringWheelAngle;
  float VDC2_SteeringWheelAngleSensorType;
  unsigned char VDC2_SteeringWheelTurnCounter;
  float VDC2_YawRate;
  float VDC2_LongitudinalAcceleration;
  float VDC2_LateralAcceleration;
  float VP_X_TGW_Latitude_BB1_X_TGW;
  float VP_X_TGW_Longitude_BB1_X_TGW;;

  float VOLVO_XBR_ExternalAccelerationDemand;       ///
  unsigned char VOLVO_XBR_src_address;
  unsigned char VOLVO_XBR_destination_address;
  unsigned char VOLVO_XBR_XBREBIMode;       ///
  unsigned char VOLVO_XBR_XBRPriority;      ///
  unsigned char VOLVO_XBR_XBRControlMode;   ///
  unsigned char VOLVO_XBR_XBRUrgency;       ///
  unsigned char VOLVO_XBR_spare1;           /// 0xFF
  unsigned char VOLVO_XBR_spare2;           /// 0xFF
  unsigned char VOLVO_XBR_spare3;           /// 0xFF
  unsigned char VOLVO_XBR_XBRMessageCounter;///
  unsigned char VOLVO_XBR_XBRMessageChecksum;///

  unsigned char VOLVO_XBR_WARN_src_address;
  unsigned char VOLVO_XBR_WARN_byte1;
  unsigned char VOLVO_XBR_WARN_byte2;
  unsigned char VOLVO_XBR_WARN_byte3;
  unsigned char VOLVO_XBR_WARN_byte4;
  unsigned char VOLVO_XBR_WARN_byte5;
  unsigned char VOLVO_XBR_WARN_byte6;
  unsigned char VOLVO_XBR_WARN_byte7;
  unsigned char VOLVO_XBR_WARN_byte8;

  unsigned char VP15_EcoRollStatus;
  unsigned char VP15_AutomaticHSARequest;
  unsigned char VP15_EngineShutdownRequest;
  float VP15_RoadInclinationVP15;
  float VP15_PermittedHillHolderP;
  unsigned char VP15_RecommendedGearshift;
  unsigned char VP15_EcoRollActiveStatus;
  unsigned char VP15_ClutchOverloadStatus;
  unsigned char VP15_PowerDownAcknowledge;
  unsigned char VP15_DirectionGearAllowed;
  float VP15_VehicleWeightVP15;

  float Volvo_TargetDist;
  float Volvo_TargetVel;
  float Volvo_TargetAcc;
  float Volvo_TargetAvailable;
  float Volvo_EgoVel;
  float Volvo_EgoAcc;
  float Volvo_EgoRoadGrade;

  unsigned char CAN1_BusLoad;
  unsigned char CAN1_ExtData;
  unsigned char CAN1_StdData;

  unsigned char CAN2_BusLoad;

/* Xiao-Yun: 04/14/2015
** LOCAL VARIABLES BELOW THIS LINE MAY OR MAY NOT BE ASSOCIATED WITH A 
** CAN MESSAGE. THEY MAY BE DELETED LATER.
*/
  int fan_drive_state;		/* from engine */
  float nominal_friction_torque; /* from engine */
  float boost_pressure;
  float estimated_percent_fan_speed;
  float exhaust_gas_pressure;
  float rack_position;
  float natural_gas_mass_flow;
  float instantaneous_brake_power;
  float trans_retarder_value;
  float trans_retarder_mode;
  float coolant_load_increase;
  unsigned char trans_retarder_source;
  unsigned char acc_cacc_request; // ACC/CACC control signal from DVI; No request=0, ACC=1, CACC=2
  unsigned char gap_request; // No request=0, else gap levels 1-5;
  evt300_radar_typ evt300;
  long_lidarA_typ lidarA;
  long_lidarB_typ lidarB;
  mdl_lidar_typ mdl_lidar;
  path_gps_point_t self_gps;	/// self GPS information
  veh_comm_packet_t lead_trk;	/// packet from LEAD vehicle
  veh_comm_packet_t second_trk;	/// packet from SECOND vehicle
  veh_comm_packet_t third_trk;	/// packet from THIRD vehicle
  long_dig_in_typ dig_in;	/// switches digital in
} long_vehicle_state;

/**
 * long_params 
 * Structure includes initialization conditions and parameters
 * for the current vehicle and the current test. These will not change
 * throughout the control, and are set once at the beginning, either
 * from command line arguments, or by reading an initialization file.
 *
 */

#define LONG_FILENAME_LEN 80
typedef struct {
  char avcs_cfg_file[LONG_FILENAME_LEN];	/* per vehicle file */
  char long_cfg_file[LONG_FILENAME_LEN];	/* longitudinal control */
  char data_file[LONG_FILENAME_LEN];	/* file for data gathering */
  int ctrl_interval;    /* time between command updates, millisecs */
  int max_iterations;	/* >0 maximum iterations control loop, 0 none */
  int cmd_test;		/* test to run; used only for cmdtest project! */
  /* vehicle characteristics */
  float max_engine_speed;		 /* maximum engine speed allowed */
  float engine_reference_torque; 	 /* vehicle's reference torque */
  float retarder_reference_torque;	/* retarder reference torque */
  int vehicle_type;	/* truck, 40-foot bus, 60-foot bus */
  int *dbv_list;
  int dbv_list_size;
  int vehicle_position;	/* currently also from script */
  bool_typ mdl_lidar;
  bool_typ denso_lidar;
} long_params;

/**
 * Data from last "data_size" control intervals can be saved in this
 * structure. The structure is initialized in init_tasks, and printed
 * out in exit_tasks.
 */

typedef struct {
  void *data_array;	/* circular buffer */
  int data_size;  /* number of structures malloced */
  int data_count; /* control intervals saved (<=data_size) */
  int data_start; /* index of oldest item in the circular buffer */
} long_data_buffer;

/**
 * Main longitudinal control structure. For real control, may include
 * other substructures that are common to all controllers. Data specific
 * to a particular control is passed by plong_private pointer
 */

typedef struct {
  struct timeb start_time;	
  long_vehicle_state vehicle_state;
  long_params params;
  cbuff_typ buff;
  void * plong_private;
} long_ctrl;

/* Default time interval for checking and changing control */
#define CTRL_MSECS       20

/* Vehicle type definitions */
#define VEH_TYPE_TRUCK_SILVR	1
#define VEH_TYPE_BUS40	2
#define VEH_TYPE_BUS60	3
#define VEH_TYPE_TRUCK_BLUE	4
#define VEH_TYPE_TRUCK_GOLD	5
#define VEH_TYPE_TRUCK_VOLVO_1	6
#define VEH_TYPE_TRUCK_VOLVO_2	7
#define VEH_TYPE_TRUCK_VOLVO_3	8

/* prototypes for functions in long_utils library */
//extern void init_circular_buffer(long_data_buffer *pbuff, int gather_data,
//				int item_size);
//extern int get_circular_index (long_data_buffer *pbuff);
extern void long_set_dbv_list(long_ctrl *pctrl);
extern FILE *long_get_output_stream (char *prefix);
extern int long_set_params(int argc, char **argv, long_params *pparams);
extern void long_print_params(FILE *fp, long_params *pparams);
extern db_clt_typ *long_database_init(char *phost, char *pclient,
				 long_ctrl *pctrl);
extern int long_trigger(db_clt_typ *pclt, unsigned int db_num);
extern int long_wait_for_init(db_clt_typ *pclt, long_ctrl *pctrl);
extern int long_read_vehicle_state(db_clt_typ *pclt, long_ctrl *pctrl);
extern int long_setled(db_clt_typ *pclt, int faultcode);
extern int long_rdswitch(long_dig_in_typ dig_in);
extern int long_rdbrake(long_dig_in_typ dig_in);

/* prototypes for functions that must be provided by individual controller */
extern int init_tasks(db_clt_typ *pclt, long_ctrl *pctrl, long_output_typ *pcmd);
extern int run_tasks(db_clt_typ *pclt, long_ctrl *pctrl, long_output_typ *pcmd);
extern int exit_tasks(db_clt_typ *pclt, long_ctrl *pctrl, long_output_typ *pcmd);

/* prototypes for reading and writing gpio */
extern int rdswitch( db_clt_typ *pclt );
extern int setled( db_clt_typ *pclt, int faultcode );

#define FLT_HI                 1
#define FLT_MED                2
#define FLT_LOW                3
#define FLT_AUTO               4
#define FLT_RDY2ROLL           5
#define LONG_CTL_ACTIVE        6
#define LONG_CTL_INACTIVE      7
#define TOGGLE_WATCHDOG        8

#define LED_RED        0x80 // Pin 9 (B7)
#define LED_AMBER      0x40 // Pin 10 (B6)
#define LED_NOT_AMBER  0xBF
#define LED_GRN        0x20 // Pin 11 (B5)
#define LED_BLUE       0x10 // Pin 12 (B4)
#define WDOGBIT        0x08 // Pin 13 (B3)
#define NWDOGBIT       0xF7

#endif /* LONG_CTL_H */
