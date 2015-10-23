#ifndef _MMITSSOBUAWARE_H
#define _MMITSSOBUAWARE_H

#include <csetjmp>
#include <csignal>

#include "dsrcMsgs.h"
#include "timeUtils.h"
#include "geoUtils.h"
#include "obuConfig.h"

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
	SIGSEGV,		// Signal Segmentation Violation
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

struct priorityRequestAction_enum_t
{
	enum Action {NONE,INITIATE,KEEPGOING,CANCEL};
};

//structure to hold priority request info
struct PriorityRequest_t
{
	bool 			isPriorityRequested;
	long long requestedTms;
	long long requestedEta;
	uint8_t		requestedmsgCnt;
	uint32_t	requestedVehicleId;
	uint32_t	requestedIntersectionId;
	uint8_t		requestedInlaneId;
	uint8_t 	requestedOutlaneId;
	uint8_t 	requestedControlPhase;
	uint8_t 	requestedIntersectionIndex;
	uint8_t 	requestedApproachedIndex;
	uint8_t 	requestedLaneIndex;
	void reset(void)
	{
		isPriorityRequested = false;
		requestedTms = 0;
		requestedmsgCnt = 0;
	};
};

void initialSRM(SRM_element_t& cvSRM,OBUconfig::VinConfig& vehvin);
uint8_t getOutlaneId(const std::vector<GeoUtils::connectTo_t>& aConnectTo);
void updatePriorityRequest(PriorityRequest_t& request,const GeoUtils::connectedVehicle_t& cv,const long long tms,const priorityRequestAction_enum_t::Action action);
void updateSRM(SRM_element_t& cvSRM,const PriorityRequest_t& request,const AsnJ2735Lib& lib,const BOLB1_element_t* ps_bsmblob1,const priorityRequestAction_enum_t::Action action);
void dts2DTime(DTime_element_t& dt,const timeUtils::dateTimeStamp_t& ts,const uint16_t millitm);
void logSRM(const timeUtils::dateTimeStamp_t& ts,const char* buf,const size_t size,const priorityRequestAction_enum_t::Action action);

#endif
