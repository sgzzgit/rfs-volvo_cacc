#ifndef _MMITSSRSUUAWARE_H
#define _MMITSSRSUUAWARE_H

#include <csetjmp>
#include <csignal>
#include <map>
#include <vector>
#include <stdint.h>

#include "dsrcMsgs.h"
#include "timeUtils.h"
#include "geoUtils.h"
#include "rsuConfig.h"
#include "wmeUtils.h"

/// interrupt signal
#define ERROR -1
static int sig_list[] = 
{
	/// list of signals for interruption, handled by sig_hand ()
	SIGABRT,
	SIGINT,			
	SIGQUIT,		
	SIGTERM,		
	SIGKILL,
	SIGSEGV,
	ERROR
};
static jmp_buf exit_env;
static void sig_hand( int code )
{
	longjmp( exit_env, code );
};
static void sig_ign(int *sigList, void sig_hand(int sig))
{
  int i = 0;
  while (sigList[i] != ERROR)
	{
    signal(sigList[i], sig_hand);
    i++;
  };
};
/// enum for soft-call status
struct softcall_enum_t
{
	enum action		{NOACTION = 0, CALLED = 1, CANCELLED = 2};
	enum callObj 	{NONE = 0, PED = 1, VEH = 2, PRIORITY = 3};
	enum callType {UNKNOWN = 0,CALL = 1,EXTENSION = 2, CANCEL = 3};
};
struct priorityRequestServer_enum_t
{
	enum PRGupdate {INITIATE,MAINTAIN,UPDATE,CANCEL};
	enum PRSstate {NONE,EARLYGREEN,GREENEXTENSION};
};
/// structure for sending soft-calls
struct softcall_t
{
	uint16_t	internal_msg_header; 
	uint8_t   soft_call_msg_id; 		
	uint32_t  ms_since_midnight;
	uint8_t	  softcallPhase;  // Bit mapped phase, bits 0-7 phase 1-8
	uint8_t   softcallObj;
	uint8_t	  softcallType;
	uint8_t 	softcallNum;    // how many times the SET soft-call commands need to be sent to the controller
                                    // = 1 when softcallType = 1 or 3 (i,e., call or cancel)
                                    // = x when softcallObj = 2 & softcallType = 2 (i.e., vehicular phase extension)
                                    // = 0 when call_obj = 3 & call_type = 2 (i.e., priority green extension)
	void reset(void)
	{
		internal_msg_header = wmeUtils::msg_header;
		soft_call_msg_id		=	wmeUtils::msgid_softcall;
		ms_since_midnight		= 0;
		softcallPhase				= 0;
		softcallObj					= static_cast<uint8_t>(softcall_enum_t::NONE);
		softcallType				= static_cast<uint8_t>(softcall_enum_t::UNKNOWN);
		softcallNum					= 0;
	};
}__attribute__((packed));
/// structure for receiving raw spat
struct ownspat_t
{
	uint16_t	internal_msg_header; 
	uint8_t	  msgid; 								
	uint32_t  ms_since_midnight;
	SPAT_element_t	spat;
}__attribute__((packed));

/// structure to hold connected vehicle status including geo-location, map matched location, location/signal aware,
///		and priority request and soft-call status
struct cvStatusAware_t
{
	bool isOnApproach;				// in an approach of this intersection (given by intersection name)
	bool isPhaseCalled;				// call vehicle phase at most once for each vehicle if it's needed
	bool isExtensionCalled;		// extend vehicle phase green at most once for each vehicle if it's needed
	bool isVehInQueue;				// for queue estimation
	std::vector<GeoUtils::connectedVehicle_t> cvStatus; // variable size if the vehicle is on approach, one entry if not.  
	void reset(void)
	{
		isOnApproach = false;
		isPhaseCalled = false;
		isExtensionCalled = false;
		isVehInQueue = false;
		cvStatus.clear();
	};
};
/// structure to hold vehicular phase extension call status at this intersection
struct signalPhaseExtension_t
{
	softcall_enum_t::action callAction;
	long long callTms;
	std::map<uint32_t,uint8_t> callVehicles;
	void reset(void)
	{
		callAction = softcall_enum_t::NOACTION;
		callTms = 0;
	};
};
/// structure to hold PRS' request table entry
struct priorityRequestTableEntry_t
{
	uint32_t 	requestVehId;
	uint8_t		msgCnt;
	priorityRequestServer_enum_t::PRGupdate requestUpdateStatus;
	long long initiatedTms;
	long long receivedTms;
	long long requestedServiceStartTms;
	long long requestedServiceEndTms;
	uint8_t		requestedAction;
	uint8_t		requestInLaneId;
	uint8_t		requestOutLaneId;
	uint8_t		requestPhase;
	uint8_t		requestVehicleClassType;
	uint8_t		requestVehicleClassLevel;
	uint8_t 	requestVehicleTransitStatus;	
	DTime_element_t requestTimeOfService;									// optional in SRM_element_t
	DTime_element_t requestEndOfService;									// optional in SRM_element_t
	priorityrequest_enum_t::requeststatus	requestStatus;	// determined here
};
/// structure to hold MMITSS signal control status
struct mmitssSignalControlStatus_t
{
	priorityRequestServer_enum_t::PRSstate prsAction;
	uint8_t artMsgCnt;
	uint8_t callPhase;
	uint32_t priorityVehicleId;
	uint8_t priorityCycleCnt;	 		
	long long artTms;
	long long priorityTms;
	std::map<uint32_t,priorityRequestTableEntry_t> prsList; // key vehId
	long long phaseCallTms[NEMAPHASES];
	signalPhaseExtension_t phaseExtStatus[NEMAPHASES];
	void reset(void)
	{
		prsAction = priorityRequestServer_enum_t::NONE;
		artMsgCnt = 0;
		callPhase = 0;
		priorityVehicleId = 0;
		priorityCycleCnt = 0;
		artTms = 0;
		priorityTms = 0;
		prsList.clear();
		for (int i=0; i<NEMAPHASES; i++)
		{
			phaseCallTms[i] = 0;
			phaseExtStatus[i].reset();
		};
	};
};

void cleanVehList(std::map<uint32_t,cvStatusAware_t>& list,const long long tms,const long long timeout);
void cleanPrsList(std::map<uint32_t,priorityRequestTableEntry_t>& list,const long long tms,const long long timeout);
void logSoftcall(const timeUtils::dateTimeStamp_t& ts,const softcall_t& s_softcall,const char* callTye);
void logART(const timeUtils::dateTimeStamp_t& ts,const char* buf,const size_t& size,
	const uint8_t& priorityCycleCnt,const uint32_t& priorityCause,const uint8_t& requestNums);

#endif
