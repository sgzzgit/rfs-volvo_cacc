#ifndef _DSRC_MSG_STRUCT_H
#define _DSRC_MSG_STRUCT_H

#include <stdint.h>	// c++11 <cstdint>
#include <cstring>

#include "msgenum.h"

#define NEMAPHASES 			8
#define BSMBLOB1SIZE		(0x26)
#define MAXDSRCMSGSIZE	2000
#define MAXPRIOREQUESTS	50

struct priorityrequest_enum_t
{
	enum vehicularrequest {CANCELPRIORITY = 0x00, REQUESTPRIORITY = 0x10, CANCELPREEMP = 0x80, REQUESTPREEMP = 0x90,UNKNOWREQUEST = 0xF0};
	enum requeststatus	{NOTVALID,REJECTED,NOTNEEDED,QUEUED,ACTIVE,CANCELLED,COMPLETED};
};
/* NTCIP priorityRequestStatusInPRS OBJECT-TYPE SYNTAX INTEGER 
		idleNotValid (1), readyQueued (2), readyOverridden (3), activeProcessing (4), activeCancel (5),
		activeOverride (6), activeNotOverridden (7), closedCanceled (8), ReserviceError (9), closedTimeToLiveError (10),
		closedTimerError (11), closedStrategyError (12), closedCompleted (13), activeAdjustNotNeeded (14), closedFlash (15) */
		
struct connectTo_enum_t
{
	enum maneuverType { UNKNOWN = 0, UTURN = 1, LEFTTURN, RIGHTTURN, STRAIGHTAHEAD, STRAIGHT };
};
struct phasestate_enum_t
{ 
	enum vehicular {UNKNOWN, GREEN, YELLOW, RED, FLASHING};
	enum pedestrian {UNAVAILABLE, STOP, CAUTION, WALK};
	enum confidentce {NOT_AVAILABLE, MINTIME, MAXTIME, TIME_LIKELY_TO_CHANGE};
};

struct BOLB1_element_t
{
	uint8_t		msgCnt;								// INTEGER (0..127)
	uint32_t	id;										// TemporaryID
	uint16_t	ms; 									// INTEGER (0..65535), milliseconds in minute
	double		lat;									// in degree (-90..90)
	double		lon;									// in degree (-180..180)
	double  	elev;									// in meters (-409.5..6143.9) (-1000 = unknown)
	double		semiMajorAccuracy;	 	// in meters (0..12.7) (-1000 = unavailable)
	double		semiMinorAccuracy;		// in meters (0..12.7) (-1000 = unavailable)
	double		orientation;					// in degree (0..360) (-1000 = unavailable)
	uint8_t		transState;						// (0..7)
	double 		speed;								// in m/s (0..163.8) (-1000 = unavailable)
	double		heading;							// in degree (0..360)
	double		steeringAngle;				// in degree (-189..189) (-1000 = unavailable)
	double		accelLon;							// in m/s^2 (-20..20) (-1000 = unavailable)
	double		accelLat;							// in m/s^2 (-20..20) (-1000 = unavailable)
	double		accelVert;						// in g (-3.4..1.54) (-1000 = unavailable)
	double 		yawRate;							// in deg/s (-328..328)
	uint16_t 	brakes;								// not decoded yet
	double 		vehLen;								// in meters (0..163.83)
	double 		vehWidth;							// in meters (0..10.23)
};

struct BSM_element_t
{
	// DSRCmsgID_t msgID 
	// BSMblob_t blob1
	BOLB1_element_t bolb1_element;
	// VehicleSafetyExtension	*safetyExt
	// VehicleStatus	*status	
};

struct PhaseState_element_t
{
	uint32_t	currState;						// Encode signal phase indication per Battelle definition (60606-012_SPaT_CONOPS rev C.pdf)
	uint16_t	timeToChange;					// In units of deciseconds from local UTC time
	uint8_t		stateConfidence;
	uint32_t	nextState;						// the next state of a Motorised lane
	uint16_t	clearanceIntv;				// yellow interval
	uint8_t		yellStateConfidence;
}__attribute__((packed));

struct SPAT_element_t
{
	// DSRCmsgID_t msgID
	// SPAT__intersections intersections (one IntersectionState per SPAT)
	// IntersectionState::IntersectionID_t	 id
	uint32_t	id;								// IntersectionID
	// IntersectionState::DescriptiveName_t	*name
	// IntersectionState::IntersectionStatusObject_t	 status
	uint8_t		status;						// with bits set as follows Bit #:
															//	0		Manual Control is enabled.  Timing reported is per programmed values, etc but person at cabinet can 
															//			manually request that certain intervals are terminated early (e.g. green).
															//	1		Stop Time is activated and all counting/timing has stopped.
															//	2		Intersection is in Conflict Flash.
															//	3		Preemption is Active
															//	4		Transit Signal Priority (TSP) is Active
															//	5		Reserved
															//	6		Reserved
															//	7		Reserved as zero
	// IntersectionState::TimeMark_t	*timeStamp
	uint16_t	timeStamp;				//	(OPTIONAL) INTEGER (0..12002)
															//	In units of deciseconds from local UTC time
															//	A range of 0~600 for even minutes, 601~1200 for odd minutes
															//	12001 to indicate indefinite time
															//	12002 to be used when value undefined or unknown
	uint8_t		permittedPhases;			//	bit set to 1 for active phase (permitted phases)
	uint8_t		permittedPedPhases;	//	bit set to 1 if the phase is active
	// IntersectionState::states::MovementState
	PhaseState_element_t 	phaseState[NEMAPHASES];
	PhaseState_element_t	pedPhaseState[NEMAPHASES];
}__attribute__((packed));

struct Mapdata_element_t
{
	// DSRCmsgID_t msgID
	// MsgCount_t	 msgCnt
	uint8_t mapVersion;
	// struct MapData__intersections *intersections (one Intersection per map-data)
	// intersections::Intersection::IntersectionID_t	 id
	uint32_t id;
};

struct VehicleClass_element_t
{
	uint8_t NTCIPvehicleClass_type;		// defined in asn1/VehicleType.h
	uint8_t NTCIPvehicleClass_level;
};

struct SignalRequest_element_t
{
	// IntersectionID_t	 id
	uint32_t	id;					// IntersectionID
	//SignalRequest::SignalReqScheme::*isCancel (OPTIONAL) (present only when cancelling a prior request)
	//SignalRequest::SignalReqScheme::*requestedAction (OPTIONAL) (size 1)
	uint8_t requestedAction;	// Encoded as follows: 
														// upper nibble:  preemption #:  0x0 for priority
														// 	Bit 7 (MSB) 1 =  preemption and 0 =  Priority 
														// 	Remaining 3 bits: 
														// 	Range of 0..7. The values of 1..6 represent the respective controller preemption or Priority to be activated. 
														//	The value of 7 represents a request for a cabinet flash preemption while the value of 0 is reserved.  
														// lower nibble:  Strategy #:  
														//	Range is 0..15 and is used to specify a desired strategy (if available).  
														//	Currently no strategies are defined and this should be zero.
														// Request priority 	= 0x10 (0001 0000)
														// Request preemption	= 0x90 (1001 0000)
														// Cancel priority		= 0x00 (0000 0000)
														// Cancel preemption	= 0x80 (1000 0000)
	//SignalRequest::LaneNumber::*inLane (OPTIONAL) (size 1)
	uint8_t inLaneId;
	//SignalRequest::LaneNumber::*outLane (OPTIONAL) (size 1)
	uint8_t outLaneId;
	//SignalRequest::NTCIPVehicleclass::type (size 1)
	VehicleClass_element_t NTCIPVehicleclass;
														// Two 4 bit nibbles as: 
														//	NTCIP vehicle class type
														//	NTCIP vehicle class level
	//SignalRequest::CodeWord::*codeWord (OPTIONAL) (size 1..16)
	char codeWord[20];
	void reset(void)
	{
		memset(codeWord,'\0',sizeof(codeWord));
	};
};

struct DTime_element_t
{
	uint8_t 	hour;
	uint8_t 	min;
	uint16_t	sec; // in milliseconds
};

struct VehicleIdent_element_t
{
	// DescriptiveName_t	*name (1..63)
	char vehName[64];
	// VINstring_t	*vin (1..17)
	char vehVin[20];
	// IA5String_t	*ownerCode (1..32)
	char vehOwnerCode[40];
	// TemporaryID_t	*id
	uint32_t vehId;		// same as BSM
	// VehicleType_t	*vehicleType
	uint8_t vehType;	// same as NTCIPvehicleClass_type
	// VehicleIdent__vehicleClass *vehicleClass
//	uint8_t vehGroupType;
	void reset(void)
	{
		memset(vehName,'\0',sizeof(vehName));
		memset(vehVin,'\0',sizeof(vehVin));
		memset(vehOwnerCode,'\0',sizeof(vehOwnerCode));
	};
};

struct SRM_element_t
{
	// DSRCmsgID_t	 msgID
	// msgCnt (size 1)
	uint8_t		msgCnt;			// INTEGER (0..127)
	// SignalRequest
	SignalRequest_element_t signalRequest_element;	
	// DTime::*timeOfService (OPTIONAL) (size 4)
											//	byte 1 hour    DHour 		(0..31)
											//	byte 2 minute  DMinute	(0..63)
											//	byte 3 & 4 second  DSecond (0..65535) in milliseconds
	DTime_element_t timeOfService;		// in local time
	// DTime::*endOfService (OPTIONAL) (size 4)
	DTime_element_t endOfService;			// in local time
	// TransitStatus::*transitStatus (OPTIONAL) (size 1)
											// Bit string
											//	none        (0), -- nothing is active
											//	anADAuse    (1), -- an ADA access is in progress (wheelchairs, kneeling, etc.)
											//	aBikeLoad   (2), -- loading of a bicycle is in progress
											//	doorOpen    (3), -- a vehicle door is open for passenger access
											//	occM        (4),
											//	occL        (5)
											//	bits four and five are used to relate the relative occupancy of the vehicle, with
											//	 00  as least full and 11 indicating a close-to or full condition
	uint8_t transitStatus;
	// VehicleIdent::*vehicleVIN (OPTIONAL)
	VehicleIdent_element_t vehIdent_element;
	// BSMblob::vehicleData
	char BSMblob[BSMBLOB1SIZE];
	// VehicleRequestStatus::*status (size 1)
											// With bits set as follows: 
											//	Bit 7 (MSB)  Brakes-on, see notes for use 
											//	Bit 6 Emergency Use or operation 
											//	Bit 5 Lights in use (see also the light bar element)
											//	Bits 5~0
											//		when a priority, map the values of LightbarInUse to the lower 4 bits and set the 5th bit to zero
											//		when a preemption, map the values of TransistStatus to the lower 5 bits
											//	LightbarInUse ::= ENUMERATED {
											//		unavailable         (0),  -- Not Equipped or unavailable
											//		notInUse            (1),  -- none active
											//		inUse               (2),  
											//		sirenInUse          (3),   
											//		yellowCautionLights (4),  
											//		schooldBusLights    (5),  
											//		arrowSignsActive    (6),  
											//		slowMovingVehicle   (7),   
											//		freqStops           (8),   
											//		reserved            (9)  -- for future use
											//	}
	uint8_t requestStatus;	
	void reset(void)
	{
		signalRequest_element.reset();
		vehIdent_element.reset();
	};
};

struct ART_request_t
{
	uint32_t id;										// TemporaryID
	uint8_t inLaneId;								// optional in SignalRequest_element_t
	uint8_t outLaneId;							// optional in SignalRequest_element_t
	uint8_t	contolPhase;				
	uint8_t classType;
	uint8_t classLevel;
	uint8_t	requestStatus;					// determined by priority request server
	uint8_t transitStatus;					// optional in SRM_element_t	
	DTime_element_t timeOfService;	// optional in SRM_element_t
	DTime_element_t endOfService;		// optional in SRM_element_t
	uint16_t etaOffset;							// in decisecond
};

struct ART_element_t
{
	// DSRCmsgID_t	 msgID
	uint8_t		msgCnt;				// INTEGER (0..127)
	uint32_t	id;						// IntersectionID
	uint8_t		status;				// same in SPAT_element_t
	uint16_t	ms; 					// INTEGER (0..599), deciseconds in minute
	uint32_t	priorityCause;// vehicle TemporaryID that is under priority
	uint8_t		requestNums;	// (0..50)
	ART_request_t request[MAXPRIOREQUESTS];
};

struct MSG_UNION_t 
{
	msg_enum_t::MSGTYPE type;
	union 
	{
		BSM_element_t bsm;
		SRM_element_t srm;
		SPAT_element_t spat;
		Mapdata_element_t mapdata;
		ART_element_t art;
	};
};

#endif
