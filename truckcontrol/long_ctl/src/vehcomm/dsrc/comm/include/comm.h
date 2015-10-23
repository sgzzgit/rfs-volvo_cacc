#ifndef _MMITSSCOMM_H
#define _MMITSSCOMM_H

#include <fstream>
#include <stdint.h>	// c++11 <cstdint>
#include <string>
#include <cstring>
#include <map>

#include "timeUtils.h"
#include "geoUtils.h"

const double defaultSpeed = 10.0;	//in m/s

// structure to hold received BSM (could be multiple cars)
struct BSM_List_t
{
	long long tms;
	BSM_element_t bsmMsg;
};
// structure to hold received SRM (could be multiple cars)
struct SRM_List_t
{
	long long tms;
	SRM_element_t 	srmMsg;
	BOLB1_element_t	srmBlob1;
	bool						isCancel;
	uint8_t					requestedPhase;
	long long 			requestedServiceStartTms;
	long long 			requestedServiceEndTms;	
};
// structure to hold received SPaT (could be multiple intersections)
struct SPAT_List_t
{
	long long tms;
	SPAT_element_t spatMsg;
};
// structure to hold received MAP (could be multiple intersections)
struct MAP_List_t
{
	long long tms;
	Mapdata_element_t mapMsg;
};
// structure to hold received ART (could be multiple intersections)
struct ART_List_t
{
	long long tms;
	ART_element_t artMsg;
};

std::string getLogfileName(const timeUtils::dateTimeStamp_t& ts,const std::string& pathName,const std::string& name,const char* type);
bool openLogFile(std::ofstream& OS,std::string& fileName,const std::string& logFilePath,const timeUtils::dateTimeStamp_t& localts,const std::string& name,const char* type);
void updateBsmList(std::map<uint32_t,BSM_List_t>& list,const BSM_List_t& bsmIn);
void updateSpatTms(const uint16_t spat_timeStamp,const timeUtils::fullTimeStamp_t& timestamp,long long& tms);
void updateSpatList(std::map<uint32_t,SPAT_List_t>& list,const SPAT_List_t& spatIn);
void updateSrmList(std::map<uint32_t,SRM_List_t>& list,const SRM_List_t& srmIn);	
void updateArtList(std::map<uint32_t,ART_List_t>& list,const ART_List_t& artIn);					
void cleanBsmList(std::map<uint32_t,BSM_List_t>& list,const long long tms, const long long timeout);
void cleanSrmList(std::map<uint32_t,SRM_List_t>& list,const long long tms, const long long timeout);
void cleanSpatList(std::map<uint32_t,SPAT_List_t>& list,const long long tms, const long long timeout);
void cleanMapList(std::map<uint32_t,MAP_List_t>& list,const long long tms,const long long timeout);
void cleanArtList(std::map<uint32_t,ART_List_t>& list,const long long tms,const long long timeout);
SPAT_List_t getSpatFromList(const std::map<uint32_t,SPAT_List_t>& list,const uint32_t id);
void updateSignalAware(const std::map<uint32_t,SPAT_List_t>& list,const GeoUtils::locationAware_t& locationAware,GeoUtils::signalAware_t& signalAware);
uint16_t getTime2goDeci(const double& dist2go, const double& speed);
long long getTime2goLL(const double& dist2go, const double& speed);
double getSpeed(const double& speed);
void logAware(const timeUtils::dateTimeStamp_t& ts,const uint8_t msgCnt,const GeoUtils::connectedVehicle_t &cv);
void logPayload(const timeUtils::dateTimeStamp_t& ts,const char* buf,const size_t size,const msg_enum_t::MSGTYPE type);
void hexPrint(const char* buf,const size_t size);

#endif
