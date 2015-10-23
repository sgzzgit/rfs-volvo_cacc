#ifndef _ASN_J2735_LIB_H
#define _ASN_J2735_LIB_H

#include <string.h>
#include <cstring>
#include <stdint.h>	// c++11 <cstdint>
#include <map>

// asn1
#include <asn_application.h>
#include "BasicSafetyMessage.h"
#include "SPAT.h"
#include "MapData.h"
#include "SignalRequestMsg.h"
// asn1j2735
#include "dsrcMsgs.h"
#include "geoUtils.h"

// forward declaration
namespace NmapData
{
	class MapDataStruct;
};

class AsnJ2735Lib
{
	private:
		static const double msPosAccur2Lsb;
		static const double msOrient2Lsb;
		static const double msSpeed2Lsb;
		static const double msHeading2Lsb;
		static const double msLsb2SteeringAngle;
		static const int msTimeMarkUnKnown;
		static const double kmph2mps;
		static const BSM_element_t msInitialBsmElement;
		static const std::map<uint32_t,phasestate_enum_t::vehicular> msPhaseState2ColorMap; 	// signalLightState -> lightColor
		static const std::map<uint32_t,phasestate_enum_t::pedestrian> msPedState2ColorMap;		// pedSignalState	-> crossWalkingSign
				
		// for storing nmap and encoded payload
		NmapData::MapDataStruct *mpMapData;
		
		// encoding mapdata
		void encode_mapdata_payload(void) const;
		ssize_t encode_mapdata_payload(const uint32_t intersectionId,char* ptr,size_t size) const;
		// calculate CRC-CCITT (in mapdata)
		uint16_t crc16(const char *data_p,const size_t length) const;
		// for encoding SPaT
		uint8_t getNumPhases(const uint8_t permittedPhases) const;	
		bool isPhasePermitted(const uint8_t checkPhase, const uint8_t permittedPhases) const;
		// for decoding phaseState
		static std::map<uint32_t,phasestate_enum_t::vehicular> createLightColorMap(void);
		static std::map<uint32_t,phasestate_enum_t::pedestrian> createPedLightColorMap(void);
		phasestate_enum_t::vehicular getPhaseState(const uint32_t singalLightState) const;
		phasestate_enum_t::pedestrian getPedPhaseState(const uint32_t pedLightState) const;
		
		// for decoding numerical variable with fixed size
		unsigned long uchar2ulong(const u_char* ptr,const int offset, const int size) const;
		long uchar2long(const u_char* ptr,const int offset, const int size) const;
		// for encoding numerical variable with fixed size
		bool long2uchar(u_char* ptr,const int offset,const long l,const int size) const;		
		bool ulong2uchar(u_char* ptr,const int offset,const unsigned long l,const int size) const;
		// for encoding numerical variables with varying size
		int ulong2char(char* ptr,const int offset,const unsigned long l) const;
		int byteNums(const uint32_t id) const;
		// type conversion	
		double uchar2elevation(const u_char* ptr,const int offset,const int size) const;		
		double uchar2accel(const u_char* ptr,const int offset,const int size) const;
		double uchar2semiAccuracy(const u_char ch) const;
		u_char semiAccuracy2uchar(const double semiAccuracy) const;
		double uchar2orientation(const u_char* ptr,const int offset,const int size) const;
		uint16_t convertLaneAttributes(const uint16_t nmapLaneAttributes) const;
		void addDrivingLanes2approachObj(struct Approach* pApproach,const int index,const int approachIndex,const bool isIngress) const;
		void addCrosswalk2approachObj(struct Approach* pApproach,const int index,const int approachIndex) const;

	public:
		// constructor
		AsnJ2735Lib(const char* ptr);		
		// destructor
		~AsnJ2735Lib();
		
		static const double msUnavailableElement;
		
		// get static map info
		uint8_t getMapVersion() const;
		size_t getIntersectionIdList(uint32_t* list,const size_t size) const;
		uint32_t getIntersectionIdByName(const char* intersectionName) const;				
		size_t getIntersectionNameById(const uint32_t intersectionId,char *intersectionName,const size_t size) const;		
		std::string getIntersectionNameById(const uint32_t intersectionId) const;		
		uint8_t getControlPhaseByLaneId(const uint32_t intersectionId,const uint8_t laneId) const;
		std::string getIntersectionNameByIndex(const uint8_t intersectionIndex) const;
		int	getIndexByIntersectionId(const uint32_t intersectionId) const;  
		// get encoded map payload with or without udp file header
		size_t get_mapdata_payload(const uint32_t intersectionId,char* ptr,const size_t size,bool withHeader) const;    
		size_t get_mapdata_payload(const char* intersectionName,char* ptr,const size_t size,bool withHeader) const;
		// decode msg payload without wsmp header
		void decode_bsmblob1_payload(const u_char* pbolb,BOLB1_element_t* ps_bsmblob1) const;
		bool decode_bsm_payload(const char* ptr,const size_t size,BSM_element_t* ps_bsm,FILE* fp) const;
		bool decode_spat_payload(const char* ptr,const size_t size,SPAT_element_t* ps_spat,FILE* fp) const;
		bool decode_mapdata_payload(const char* ptr,const size_t size,Mapdata_element_t* ps_mapdata,FILE *fp) const;		
		bool decode_srm_payload(const char* ptr,const size_t size,SRM_element_t* ps_srm,FILE *fp) const;	
		bool decode_art_payload(const char* ptr,const size_t size,ART_element_t* ps_art) const;
		// decode msg payload with or without wsmp header
		bool decode_payload(const char* ptr,const size_t size,MSG_UNION_t* ps_msg,FILE* fp) const;
		
		// encode msg payload with or without udp file header
		int encode_bsmblob1_payload(const BOLB1_element_t* ps_bsmblob1,u_char* bolb1) const;		
		ssize_t	encode_bsm_payload(const BSM_element_t* ps_bsm,char* ptr,const size_t size,bool withHeader) const;
		ssize_t encode_spat_payload(const SPAT_element_t* ps_spat,char* ptr,const size_t size,const char* pIntersectionName,bool withHeader) const;
		ssize_t encode_spat_payload(const SPAT_element_t* ps_spat,char* ptr,const size_t size,bool withHeader) const;
		ssize_t encode_srm_payload(const SRM_element_t* ps_srm,char* ptr,const size_t size,bool withHeader) const;
		ssize_t encode_art_payload(const ART_element_t* ps_art,char* ptr,const size_t size,bool withHeader) const;
		
		// track vehicle on map (trajectoryAware)
		bool locateVehicleInMap(const GeoUtils::connectedVehicle_t& cv,GeoUtils::vehicleTracking_t& cvTrackingState) const;
		void getPtDist2D(const GeoUtils::vehicleTracking_t& vehicleTrackingState, GeoUtils::point2D_t& pt) const;	
		void updateLocationAware(const GeoUtils::vehicleTracking_t& vehicleTrackingState,
			GeoUtils::locationAware_t& vehicleLocationAware) const;
		void getwsmheaderinfo(const char* ptr,const size_t size,size_t& offset,size_t& payloadLen) const;		
		
		// print
		void mapPayloadHex_fprintf(FILE *fp) const;						
		void mapPayloadHex_fprintf(const uint32_t intersectionId,FILE* fp) const;
		void payloadHex_fprintf(const char *ptr,const size_t byteSize,FILE* fp) const;
};

#endif
