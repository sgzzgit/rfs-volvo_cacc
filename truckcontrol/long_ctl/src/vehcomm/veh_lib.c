/**\file
 *	veh_lib.c 
 *		Library for interconversion of old "comm_packet_t" with J2735 "BSMCACC_t"
 *
 * Copyright (c) 2015   Regents of the University of California
 *
 */
#include <sys_os.h>
#include <db_clt.h>
#include <db_utils.h>
#include <timestamp.h>
#include <local.h>
#include <sys_rt.h>
#include <sys_ini.h>
#include "path_gps_lib.h"
#include "long_comm.h"
#include "udp_utils.h"

#include "asn_application.h"
#include "asn_internal.h"       /* for _ASN_DEFAULT_STACK_MAX */
#include "asn_SEQUENCE_OF.h" 
#include "BSMCACC.h"
 
 
int vehcomm2BSM(BSMCACC_t *BSMCACC, veh_comm_packet_t *comm_pkt);

int user_Float, user_Float1;

int vehcomm2BSM(BSMCACC_t *BSMCACC, veh_comm_packet_t *comm_pkt) {

	TemporaryID_t *myid;

	myid = (TemporaryID_t *)calloc(1, sizeof(TemporaryID_t));
//	strcpy(myid, "476");

	BSMCACC->msgID = 2;
//	BSMCACC->blob1.lat =  123000000;

	BSMCACC->caccData.globalTime = (int)(comm_pkt->global_time * 50);      // From long_ctl or trk_comm_mgr
	user_Float = (int)(comm_pkt->user_float * BSM_FLOAT_MULT);
	BSMCACC->caccData.userDF1 = user_Float;
	user_Float1 = (int)(comm_pkt->user_float1 * BSM_FLOAT_MULT);
	BSMCACC->caccData.userDF2 = user_Float1;
	BSMCACC->caccData.userDI1 = comm_pkt->user_ushort_1;
	BSMCACC->caccData.userDI2 = comm_pkt->user_ushort_2;
	BSMCACC->caccData.vehGrpPos = comm_pkt->my_pip;  // My position-in-platoon (i.e. 1, 2, or 3)
	BSMCACC->caccData.vehManID = comm_pkt->maneuver_id;
	BSMCACC->caccData.vehFltMode = comm_pkt->fault_mode;
	BSMCACC->caccData.vehManDes = comm_pkt->maneuver_des_1;
//	BSMCACC->caccData.vehManDes2 = comm_pkt->maneuver_des_2;
	BSMCACC->caccData.grpSize = comm_pkt->pltn_size;
	BSMCACC->blob1.msgCnt = comm_pkt->sequence_no;
	BSMCACC->blob1.longitude = (int)(comm_pkt->longitude * LONG_LAT_MULT);
	BSMCACC->blob1.latitude = (int)(comm_pkt->latitude * LONG_LAT_MULT);
	BSMCACC->blob1.heading = (short)(comm_pkt->heading * HEADING_MULT);
	BSMCACC->caccData.userBit1 = comm_pkt->user_bit_1;
	BSMCACC->caccData.userBit2 = comm_pkt->user_bit_2;
	BSMCACC->caccData.userBit3 = comm_pkt->user_bit_3;
	BSMCACC->caccData.userBit4 = comm_pkt->user_bit_4;
	BSMCACC->caccData.desAcc = (int)(comm_pkt->acc_traj * BSM_FLOAT_MULT); 		//Desired acceleration from profile (m/s^2)
	BSMCACC->caccData.desSpd = (int)(comm_pkt->vel_traj * BSM_FLOAT_MULT);	//Desired velocity from profile (m/s)
	BSMCACC->blob1.speed.speed = (short)(comm_pkt->velocity * 50);				//Current velocity (m/s)
	BSMCACC->blob1.accelSet.longAcc = (int)(comm_pkt->accel * BSM_FLOAT_MULT);            //Current acceleration (m/s^2)
	BSMCACC->caccData.distToPVeh = (int)(comm_pkt->range * BSM_FLOAT_MULT);	//Range from *dar (m)
	BSMCACC->caccData.relSpdPVeh = (int)(comm_pkt->rate * BSM_FLOAT_MULT);	//Relative velocity from *dar (m/s)
	BSMCACC->blob1.id = 
		*OCTET_STRING_new_fromBuf(&asn_DEF_OCTET_STRING, comm_pkt->object_id, -1);

	return 0;
}

int BSM2vehcomm(BSMCACC_t *BSMCACC, veh_comm_packet_t *comm_pkt) {

	if(BSMCACC->msgID == 2) {
		comm_pkt->global_time = BSMCACC->caccData.globalTime / 50.0;      // From long_ctl or trk_comm_mgr
		comm_pkt->user_float = BSMCACC->caccData.userDF1  / BSM_FLOAT_MULT;
		comm_pkt->user_float1 = BSMCACC->caccData.userDF2  / BSM_FLOAT_MULT;
		comm_pkt->user_ushort_1 = BSMCACC->caccData.userDI1;
		comm_pkt->user_ushort_2 = BSMCACC->caccData.userDI2;
		comm_pkt->my_pip = BSMCACC->caccData.vehGrpPos;  // My position-in-platoon (i.e. 1, 2, or 3)
		comm_pkt->maneuver_id = BSMCACC->caccData.vehManID;
		comm_pkt->fault_mode = BSMCACC->caccData.vehFltMode;
		comm_pkt->maneuver_des_1 = BSMCACC->caccData.vehManDes;
		comm_pkt->maneuver_des_2 = BSMCACC->caccData.vehManDes;
		comm_pkt->pltn_size = BSMCACC->caccData.grpSize;
        	comm_pkt->sequence_no = BSMCACC->blob1.msgCnt;
		comm_pkt->user_bit_1  = BSMCACC->caccData.userBit1;
		comm_pkt->user_bit_2  = BSMCACC->caccData.userBit2;
		comm_pkt->user_bit_3  = BSMCACC->caccData.userBit3;
		comm_pkt->user_bit_4  = BSMCACC->caccData.userBit4;
		comm_pkt->acc_traj = BSMCACC->caccData.desAcc / BSM_FLOAT_MULT; //Desired acceleration from profile (m/s^2)
		comm_pkt->vel_traj = BSMCACC->caccData.desSpd / BSM_FLOAT_MULT;	//Desired velocity from profile (m/s)
			//XYL: ref_v = reference speed=requested speed by controller = desired speed
		comm_pkt->velocity = BSMCACC->blob1.speed.speed / 50.0;         //Current velocity (m/s)
			//XYL: velocity=measured speed by controller (=current velocity);
		comm_pkt->accel = BSMCACC->blob1.accelSet.longAcc / BSM_FLOAT_MULT;            //Current acceleration (m/s^2)
        	comm_pkt->range = BSMCACC->caccData.distToPVeh / BSM_FLOAT_MULT;	//Range from *dar (m)
        	comm_pkt->rate = BSMCACC->caccData.relSpdPVeh / BSM_FLOAT_MULT;             //Relative velocity from *dar (m/s)
		comm_pkt->longitude = (double)(BSMCACC->blob1.longitude / LONG_LAT_MULT);
		comm_pkt->latitude = (double)(BSMCACC->blob1.latitude / LONG_LAT_MULT);
		comm_pkt->heading = (double)(BSMCACC->blob1.heading / HEADING_MULT);

//      	BSMCACC->blob1.id = "1"; //comm_pkt->object_id[GPS_OBJECT_ID_SIZE + 1];
        	return 0;
	}
	else {
		printf("BSMCACC->msgID != 2\n");
		return -1;
	}
}

/*
J2735BSMESSAGE DEFINITIONS AUTOMATIC TAGS ::= 
BEGIN
 
 
BSMCACC ::=  SEQUENCE {
-- Part I
    msgID   [0]DSRCmsgID,   -- 1 byte
    blob1   [1]BSMblob,     -- Sent as a single octet blob 

-- Part II, contains CACC specific data
    caccData [2]CaccData    -- CACC defined Data
}
 
 
 
--CACC DEFINITIONS IMPLICIT TAGS::= BEGIN
-- Data Frames 
-- DE_DSRC_MessageID (Desc Name) Record 37
DSRCmsgID ::= ENUMERATED {
   reserved                        (0), 
   alaCarteMessage                 (1),  -- ACM
   basicSafetyMessage              (2),  -- BSM, heartbeat msg
   basicSafetyMessageVerbose       (3),  -- used for testing only
   commonSafetyRequest             (4),  -- CSR
   emergencyVehicleAlert           (5),  -- EVA
   intersectionCollisionAlert      (6),  -- ICA
   mapData                         (7),  -- MAP, GID, intersections 
   nmeaCorrections                 (8),  -- NMEA
   probeDataManagement             (9),  -- PDM
   probeVehicleData                (10), -- PVD
   roadSideAlert                   (11), -- RSA
   rtcmCorrections                 (12), -- RTCM
   signalPhaseAndTimingMessage     (13), -- SPAT
   signalRequestMessage            (14), -- SRM
   signalStatusMessage             (15), -- SSM
   travelerInformation             (16), -- TIM
   
   ... -- # LOCAL_CONTENT
} 
   -- values to 127 reserved for std use
   -- values 128 to 255 reserved for local use
 
BSMblob ::= SEQUENCE {
   -- made up of the following 38 packed bytes:
   msgCnt      MsgCount,             --x- 1 byte
   id          TemporaryID,          --x- 4 bytes
   secMark     DSecond,              --x- 2 bytes, vehicle CAN Bus sync time
   latitude    Latitude,             --x- 4 bytes 
   longitude   Longitude,            --x- 4 bytes
   elev        Elevation,            --x- 2 bytes
   accuracy    PositionalAccuracy,   --x- 4 bytes
   speed       TransmissionAndSpeed, --x- 2 bytes
   heading     Heading,              --x- 2 byte
   angle       SteeringWheelAngle,   --x- 1 byte
   accelSet    AccelerationSet4Way,  --x- accel set  (four way) 7 bytes
   brakes      BrakeSystemStatus,    --x- 2 bytes
   size        VehicleSize	     --x- 3 bytes
} 
 
CaccData ::= SEQUENCE {
    flags       [0]CACCFlags,               -- Radar\Lidar, ACC Switch, ACC Setting type, ACC Resume\Engaged, (ACC) Cancel Button, ACC\CACC Flag
    setSpeed    [1]Velocity,
    throtPos    [2]ThrottlePosition,
    lclPN       [3]OCTET STRING (SIZE(4)) OPTIONAL,  -- PinPoint Local Position North in mm, (I32) 4-Byte Signed Int. from getLocalPose: Method ID = 9
    lclPE       [4]OCTET STRING (SIZE(4)) OPTIONAL,  -- PinPoint Local Position East in mm, (I32) 4-Byte Signed Int. from getLocalPose: Method ID = 9
    lclPD       [5]OCTET STRING (SIZE(4)) OPTIONAL,  -- PinPoint Local Position Down in mm, (I32) 4-Byte Signed Int. from getLocalPose: Method ID = 9
    roll        [6]OCTET STRING (SIZE(2)) OPTIONAL,  -- PinPoint Roll; 180 deg/2^15, (I16) 2-Byte Signed Int. from getLocalPose: Method ID = 9
    pitch       [7]OCTET STRING (SIZE(2)) OPTIONAL,  -- PinPoint Pitch; 180 deg/2^15, (I16) 2-Byte Signed Int. from getLocalPose: Method ID = 9
    yaw         [8]OCTET STRING (SIZE(2)) OPTIONAL,  -- PinPoint Yaw; 180 deg/2^15, (I16) 2-Byte Signed Int. from getLocalPose: Method ID = 9
    hPosAcry    [9]OCTET STRING (SIZE(4)) OPTIONAL,  -- PinPoint North Accuracy m, (F32) 4-Byte Float, getFilterAccuracy: Method ID =6
    vPosAcry    [10]OCTET STRING (SIZE(4)) OPTIONAL, -- PinPoint North Accuracy m, (F32) 4-Byte Float, getFilterAccuracy: Method ID =6
    fwrdVel     [11]OCTET STRING (SIZE(4)) OPTIONAL, -- PinPoint North Velocity m/s, (F32) 4-Byte Float, newRawGpsData: Signal ID = 6
    rightVel    [12]OCTET STRING (SIZE(4)) OPTIONAL, -- PinPoint East Velocity m/s, (F32) 4-Byte Float
    downVel     [13]OCTET STRING (SIZE(4)) OPTIONAL, -- PinPoint Down Velocity m/s, (F32) 4-Byte Float
    velAcc      [14]OCTET STRING (SIZE(4)) OPTIONAL, -- Velocity Accuracy m/s, (F32) 4-Byte Float
    fwrdAcc     [15]OCTET STRING (SIZE(2)) OPTIONAL, -- PinPoint Forward Acceleration mm/s^2, (I16), 2-Byte Signed Int., getBodyAcceleration: Method ID =18
    rightAcc    [16]OCTET STRING (SIZE(2)) OPTIONAL, -- PinPoint Right Acceleration mm/s^2, (I16), 2-Byte Signed Int., getBodyAcceleration: Method ID =18
    dwnAcc      [17]OCTET STRING (SIZE(2)) OPTIONAL, -- PinPoint Down Acceleration mm/s^2, (I16), 2-Byte Signed Int., getBodyAcceleration: Method ID =18 
    grpID       [18]INTEGER (0..7),         -- Group ID
    grpSize     [19]INTEGER (0..15),        -- Group Size
    grpMode     [20]INTEGER (0..7),         -- Group Mode
    grpManDes   [21]INTEGER (0..127),       -- Group Maneuver desired
    grpManID    [22]INTEGER (0..127),       -- Group Maneuver ID
    vehID       [23]OCTET STRING (SIZE(1)), -- Vehicle Unique ID 
    frntCutIn   [24]INTEGER (0..7),         -- Front cut-in flag
    vehGrpPos   [25]INTEGER (0..15),        -- Vehicles' Position in Group
    vehFltMode  [26]INTEGER (0..15),        -- Vehicle fault mode ID
    vehManDes   [27]INTEGER (0..127),       -- Vehicle Maneuver desired
    vehManID    [28]INTEGER (0..127),       -- Vehicle Maneuver ID
    distToPVeh  [29]INTEGER (0..127),       -- distance to preceding veh, in meters
    relSpdPVeh  [30]INTEGER (0..127),       -- relative speed to preceding veh in m/s, range +/- 40, E = (N/0.625)+64
    disToLVeh   [31]INTEGER (0..127),       -- distance to lead veh, in meters
    relSpdLVeh  [32]INTEGER (0..127),       -- relative speed to lead veh in m/s range +/- 40, E = (N/0.625)+64
    desTGapPVeh [33]INTEGER (0..30),        -- Desired time gap to preceding veh in 100 ms increments, range 0-3.0 s
    desTGapLVeh [34]INTEGER (0..30),        -- Desired time gap to lead veh in 100 ms increments, range 0-3.0 s
    estDisPVeh  [35]INTEGER (0..150),       -- Estimated distance gap to preceding veh in m 0-150, **<try to reduce to 1 Byte, need to know acceptable resolution>
    estDisLVeh  [36]INTEGER (0..150),       -- Estimated distance gap to lead veh in m 0-150 **<try to reduce to 1 Byte, need to know acceptable resolution>
    desSpd      [37]INTEGER (0..35),        -- Desired speed (control) in m/s 0-35
    desTrq      [38]INTEGER (0..2500),      -- Desired torque (control) in N-m 0-2500 **<try to reduce to 1 Byte, need to know acceptable resolution>
    desAcc	[39]INTEGER (0..500),       -- Desired acceleration in m/s^2, 0.01 m/s^2 resolution
    userDI1     [40]INTEGER,    -- User Defined integer 1 
    userDI2     [41]INTEGER,    -- User Defined integer 2 
    userDF1     [42]INTEGER,    -- User Defined float 1 
    userDF2     [43]INTEGER,    -- User Defined float 2 
    userDF3     [44]INTEGER,    -- User Defined float 3 
    utcTime     [45]DDateTime,   -- time with mSec precision
    globalTime	[46]INTEGER,			-- Global time in sec, 0.02 sec resolution (actually a counter, starting from when the software
						-- starts up, and incrementing each servo loop, the loop time of which is 0.02 sec)
    userBit1	[47]BOOLEAN,    -- User Defined bit 1 
    userBit2	[48]BOOLEAN,    -- User Defined bit 2 
    userBit3	[49]BOOLEAN,    -- User Defined bit 3 
    userBit4	[50]BOOLEAN	-- User Defined bit 4 
}
   VehicleSize ::=  SEQUENCE {
   width     VehicleWidth,
   length    VehicleLength
   }  -- 3 bytes in length
 
-- Data Elements
 
MsgCount ::= INTEGER (0..127)
 
TemporaryID ::= OCTET STRING (SIZE(4)) -- a 4 byte string array
 
Latitude ::= INTEGER (-900000000..900000001)
    -- LSB = 1/10 micro degree
    -- Providing a range of plus-minus 90 degrees
 
Longitude ::= INTEGER (-1800000000..1800000001)
    -- LSB = 1/10 micro degree
    -- Providing a range of plus-minus 180 degrees
 
Elevation ::= OCTET STRING (SIZE(2))
    -- 1 decimeter LSB (10 cm)
    -- Encode elevations from 0 to 6143.9 meters
    -- above the reference ellipsoid as 0x0000 to 0xEFFF.
    -- Encode elevations from -409.5 to -0.1 meters,
    -- i.e. below the reference ellipsoid, as 0xF001 to 0xFFFF
    -- unknown as 0xF000
 
PositionalAccuracy ::= OCTET STRING (SIZE(4)) 
  -- And the bytes defined as folllows
 
  -- Byte 1: semi-major accuracy at one standard dev 
  -- range 0-12.7 meter, LSB = .05m
  -- 0xFE=254=any value equal or greater than 12.70 meter
  -- 0xFF=255=unavailable semi-major value 
 
  -- Byte 2: semi-minor accuracy at one standard dev 
  -- range 0-12.7 meter, LSB = .05m
  -- 0xFE=254=any value equal or greater than 12.70 meter
  -- 0xFF=255=unavailable semi-minor value 
 
  -- Bytes 3-4: orientation of semi-major axis 
  -- relative to true north (0~359.9945078786 degrees)
  -- LSB units of 360/65535 deg  = 0.0054932479
  -- a value of 0x0000 =0 shall be 0 degrees
  -- a value of 0x0001 =1 shall be 0.0054932479degrees 
  -- a value of 0xFFFE =65534 shall be 359.9945078786 deg
  -- a value of 0xFFFF =65535 shall be used for orientation unavailable 
  -- (In NMEA GPGST)
 
TransmissionAndSpeed ::= SEQUENCE {
    transmisson TransmissionState,
    speed Velocity
}

 
Heading ::= INTEGER (0..28800) 
   -- LSB of 0.0125 degrees
   -- A range of 0 to 359.9875 degrees
   
SteeringWheelAngle ::= OCTET STRING (SIZE(1)) 
    -- LSB units of 1.5 degrees.  
    -- a range of -189 to +189 degrees
    -- 0x01 = 00 = +1.5 deg
    -- 0x81 = -126 = -189 deg and beyond
    -- 0x7E = +126 = +189 deg and beyond
    -- 0x7F = +127 to be used for unavailable
    
AccelerationSet4Way ::=  SEQUENCE {
	longAcc Acceleration,          --x- Along the Vehicle Longitudinal axis
	latAcc  Acceleration,          --x- Along the Vehicle Lateral axis
	vertAcc VerticalAcceleration,  --x- Along the Vehicle Vertical axis
	yaw  YawRate
}
 
Acceleration ::= INTEGER (-2000..2001) 
    -- LSB units are 0.01 m/s^2
   -- the value 2000 shall be used for values greater than 2000     
   -- the value -2000 shall be used for values less than -2000  
   -- a value of 2001 shall be used for Unavailable
   
VerticalAcceleration ::= INTEGER (-127..127) 
   -- LSB units of 0.02 G steps over 
   -- a range +1.54 to -3.4G 
   -- and offset by 50  Value 50 = 0g, Value 0 = -1G
   -- value +127 = 1.54G, 
   -- value -120 = -3.4G
   -- value -121 for ranges -3.4 to -4.4G
   -- value -122 for ranges -4.4 to -5.4G
   -- value -123 for ranges -5.4 to -6.4G
   -- value -124 for ranges -6.4 to -7.4G
   -- value -125 for ranges -7.4 to -8.4G
   -- value -126 for ranges larger than -8.4G
   -- value -127 for unavailable data
 
YawRate ::= INTEGER (-32767..32767) 
   -- LSB units of 0.01 degrees per second (signed)
   
BrakeSystemStatus ::= OCTET STRING (SIZE(2))
   -- Encoded with the packed content of: 
   -- SEQUENCE {
   --   wheelBrakes        BrakeAppliedStatus,
   --                      -x- 4 bits
   --   wheelBrakesUnavailable  BOOL
   --                      -x- 1 bit (1=true)
   --   spareBit
   --                      -x- 1 bit, set to zero
   --   traction           TractionControlState,
   --                      -x- 2 bits
   --   abs                AntiLockBrakeStatus, 
   --                      -x- 2 bits
   --   scs                StabilityControlStatus,
   --                      -x- 2 bits
   --   brakeBoost         BrakeBoostApplied, 
   --                      -x- 2 bits
   --   auxBrakes          AuxiliaryBrakeStatus,
   --                      -x- 2 bits
   --   }
 
 
ThrottlePosition ::= INTEGER (0..200) -- LSB units are 0.5 percent   
 
VehicleWidth ::= INTEGER (0..1023) -- LSB units are 1 cm
   
VehicleLength ::= INTEGER (0..16383) -- LSB units are 1 cm
 
TransmissionState ::= ENUMERATED {
   neutral         (0), -- Neutral, speed relative to the vehicle alignment
   park            (1), -- Park, speed relative the to vehicle alignment
   forwardGears    (2), -- Forward gears, speed relative the to vehicle alignment
   reverseGears    (3), -- Reverse gears, speed relative the to vehicle alignment 
   reserved1       (4),      
   reserved2       (5),      
   reserved3       (6),      
   unavailable     (7), -- not-equipped or unavailable value,
                        -- speed relative to the vehicle alignment
 
   ... -- # LOCAL_CONTENT
   }
 
Velocity ::= INTEGER (0..8191) -- Units of 0.02 m/s
          -- The value 8191 indicates speed is unavailable
 
CACCFlags ::= OCTET STRING (SIZE(1))
    -- Radar\Lidar          (1),    "00000001"  0=off, 1=on
    -- ACC Switch           (2),    "00000010"  0=False\off, 1=True\on
    -- ACC Setting type     (4),    "00000100"  0=Manual, 1=Auto
    -- ACC Resume\Engaged   (8),    "00001000"  0=off, 1=on
    -- ACC) Cancel Button   (16),   "00010000"  0=off, 1=on
    -- ACC\CACC Flag        (32),   "00100000"  0=off, 1=on  

DDateTime ::= SEQUENCE {
   year    DYear,    -- 2 bytes
   month   DMonth,   -- 1 byte
   day     DDay,     -- 1 byte
   hour    DHour,    -- 1 byte
   minute  DMinute,  -- 1 byte
   second  DSecond   -- 2 bytes 
}
DYear ::= INTEGER (0..9999) -- units of years
DMonth ::= INTEGER (0..15) -- units of months
DDay ::= INTEGER (0..31)  -- units of days
DHour ::= INTEGER (0..31) -- units of hours 
DMinute ::= INTEGER (0..63) -- units of minutes
DSecond ::= INTEGER (0..65535) -- units of milliseconds

END
-- END of CACC module
*/
