#include <iostream>
#include <fstream>
#include <sstream>
#include <bitset>
#include <arpa/inet.h>	// htonl, htons, ntohl & ntohs, may need <netinet/in.h>

#include "AsnJ2735Lib.h"
#include "MapDataStruct.h"
#include "wmeUtils.h"

using namespace std;

const double AsnJ2735Lib::msPosAccur2Lsb = 20.0;
const double AsnJ2735Lib::msOrient2Lsb = 65535 / 360.0;
const double AsnJ2735Lib::msSpeed2Lsb = 50.0;
const double AsnJ2735Lib::msHeading2Lsb = 80.0;
const double AsnJ2735Lib::msLsb2SteeringAngle = 1.5;
const int AsnJ2735Lib::msTimeMarkUnKnown = 0;
const double AsnJ2735Lib::kmph2mps = 10./36.0;
const double AsnJ2735Lib::msUnavailableElement = -1000.0;	
const BSM_element_t msInitialBsmElement = {{0,0,0,0.0,0.0,AsnJ2735Lib::msUnavailableElement,AsnJ2735Lib::msUnavailableElement,
	AsnJ2735Lib::msUnavailableElement,AsnJ2735Lib::msUnavailableElement,7,AsnJ2735Lib::msUnavailableElement,0.0,AsnJ2735Lib::msUnavailableElement,
	AsnJ2735Lib::msUnavailableElement,AsnJ2735Lib::msUnavailableElement,AsnJ2735Lib::msUnavailableElement,0.0,0x800,5.0,2.0}};
const map<uint32_t,phasestate_enum_t::vehicular> AsnJ2735Lib::msPhaseState2ColorMap = createLightColorMap();
const map<uint32_t,phasestate_enum_t::pedestrian> AsnJ2735Lib::msPedState2ColorMap = createPedLightColorMap();

map<uint32_t,phasestate_enum_t::vehicular> AsnJ2735Lib::createLightColorMap(void)
{
	map<uint32_t,phasestate_enum_t::vehicular> m;
		m[0x00000000] = phasestate_enum_t::UNKNOWN;
		m[0x00000001] = phasestate_enum_t::GREEN;
		m[0x00000010] = phasestate_enum_t::GREEN;
		m[0x00000002] = phasestate_enum_t::YELLOW;
		m[0x00000020] = phasestate_enum_t::YELLOW;
		m[0x00000004] = phasestate_enum_t::RED;
		m[0x00000040] = phasestate_enum_t::RED;
		m[0x00000008] = phasestate_enum_t::FLASHING;
		m[0x00000080] = phasestate_enum_t::FLASHING;
	return m;
}

map<uint32_t,phasestate_enum_t::pedestrian> AsnJ2735Lib::createPedLightColorMap(void)
{
	map<uint32_t,phasestate_enum_t::pedestrian> m;
	m[0x00] = phasestate_enum_t::UNAVAILABLE;
	m[0x01] = phasestate_enum_t::STOP;
	m[0x02] = phasestate_enum_t::CAUTION;
	m[0x03] = phasestate_enum_t::WALK;
	return m;
}

AsnJ2735Lib::AsnJ2735Lib(const char* ptr)
{
	// allocate memory for MapDataStruct
	mpMapData = new NmapData::MapDataStruct(ptr);	
	if (mpMapData->getMapVersion() > 0)
	{
		// encode mapdata
		encode_mapdata_payload();	
	}
	else
	{
		cerr << "Failed to read nmap file " << ptr << endl;
	}
}
	
AsnJ2735Lib::~AsnJ2735Lib(void)
{
	// free memory for MapDataStruct
	delete mpMapData;
}

int AsnJ2735Lib::byteNums(const uint32_t id) const
{
	if (id <= 0xFF)
		return 1;
	else if (id <= 0xFFFF)
		return 2;
	else if (id <= 0xFFFFFF)
		return 3;
	else 
		return 4;
}

phasestate_enum_t::vehicular AsnJ2735Lib::getPhaseState(const uint32_t singalLightState) const
{
	map<uint32_t, phasestate_enum_t::vehicular>::const_iterator it = msPhaseState2ColorMap.find(singalLightState);
	if (it == msPhaseState2ColorMap.end())
	{
		return phasestate_enum_t::UNKNOWN;
	}
	else
	{
		return (it->second);
	}
}

phasestate_enum_t::pedestrian AsnJ2735Lib::getPedPhaseState(const uint32_t pedLightState) const
{
	map<uint32_t, phasestate_enum_t::pedestrian>::const_iterator it = msPedState2ColorMap.find(pedLightState);
	if (it == msPedState2ColorMap.end())
	{
		return phasestate_enum_t::UNAVAILABLE;
	}
	else
	{
		return (it->second);
	}
}

uint8_t AsnJ2735Lib::getMapVersion(void) const
{
	return (mpMapData->getMapVersion());
}

size_t AsnJ2735Lib::getIntersectionIdList(uint32_t* list,const size_t size) const
{
	return (mpMapData->getIntersectionIdList(list,size));
}

uint32_t AsnJ2735Lib::getIntersectionIdByName(const char* intersectionName) const
{
	return (mpMapData->getIntersectionIdByName(intersectionName));
}

size_t AsnJ2735Lib::getIntersectionNameById(const uint32_t intersectionId,char *intersectionName,const size_t size) const
{
	return (mpMapData->getIntersectionNameById(intersectionId,intersectionName,size));
}

string AsnJ2735Lib::getIntersectionNameById(const uint32_t intersectionId) const
{
	return (mpMapData->getIntersectionNameById(intersectionId));
}

uint8_t AsnJ2735Lib::getControlPhaseByLaneId(const uint32_t intersectionId,const uint8_t laneId) const
{
	return (mpMapData->getControlPhaseByLaneId(intersectionId,laneId));
}

string AsnJ2735Lib::getIntersectionNameByIndex(const uint8_t intersectionIndex) const
{
	return (mpMapData->getIntersectionNameByIndex(intersectionIndex));
}

int	AsnJ2735Lib::getIndexByIntersectionId(const uint32_t intersectionId) const
{
	return (mpMapData->getIndexByIntersectionId(intersectionId));
}

void AsnJ2735Lib::updateLocationAware(const GeoUtils::vehicleTracking_t& vehicleTrackingState,GeoUtils::locationAware_t& vehicleLocationAware) const
{
	mpMapData->updateLocationAware(vehicleTrackingState,vehicleLocationAware);
}

bool AsnJ2735Lib::locateVehicleInMap(const GeoUtils::connectedVehicle_t& cv,GeoUtils::vehicleTracking_t& cvTrackingState) const
{
	return (mpMapData->locateVehicleInMap(cv,cvTrackingState));
}

void AsnJ2735Lib::getPtDist2D(const GeoUtils::vehicleTracking_t& vehicleTrackingState, GeoUtils::point2D_t& pt) const
{
	return (mpMapData->getPtDist2D(vehicleTrackingState,pt));
}

size_t AsnJ2735Lib::get_mapdata_payload(const char* intersectionName,char* ptr,const size_t size,bool withHeader) const
{
	uint32_t intersectionId = getIntersectionIdByName(intersectionName);
	return (get_mapdata_payload(intersectionId,ptr,size,withHeader));
}

size_t AsnJ2735Lib::get_mapdata_payload(const uint32_t intersectionId,char* ptr,const size_t size,bool withHeader) const
{
	int index = mpMapData->getIndexByIntersectionId(intersectionId);
	if (!ptr || index < 0 || mpMapData->mpIntersection[index].mapPayload.size() + 1 > size)
	{
		return 0;
	}
	else if (withHeader)
	{
		msg_enum_t::MSGTYPE msgtype = msg_enum_t::MAP_PKT;
		string str = wmeUtils::fillAradaUdp(msgtype,mpMapData->mpIntersection[index].mapPayload.c_str(),
			mpMapData->mpIntersection[index].mapPayload.size());
		memcpy(ptr,str.c_str(),str.size() + 1);
		return (str.size() + 1);
	}
	else
	{
		memcpy(ptr,mpMapData->mpIntersection[index].mapPayload.c_str(), mpMapData->mpIntersection[index].mapPayload.size() + 1);
		return (mpMapData->mpIntersection[index].mapPayload.size());
	}	
}

uint16_t AsnJ2735Lib::crc16(const char *data_p, const size_t len) const
{
	u_char i;
	uint32_t data;
	uint32_t crc = 0;
	size_t length = len;
	
	if (length == 0)
	{
		return (static_cast<uint16_t>( ~crc & 0xFFFF ));
	}
	
	do
	{
		for (i=0, data=(uint32_t)0xff & *data_p++; i < 8; i++, data >>= 1)
		{
			if ((crc & 0x0001) ^ (data & 0x0001))
			{
				crc = (crc >> 1) ^ 0x8408;
			}
			else  
			{
				crc >>= 1;
			}
		}
	} while (--length);

	crc = ~crc;
	data = crc;
	crc = (crc << 8) | (data >> 8 & 0xff);
	
	return (static_cast<uint16_t>(crc & 0xFFFF));
}

bool AsnJ2735Lib::isPhasePermitted(const uint8_t checkPhase, const uint8_t permittedPhases) const
{
	if ( checkPhase >= NEMAPHASES ) 
	{
		return false;
	}
  bitset<NEMAPHASES> bs(permittedPhases);
  if (bs.test(checkPhase))
    return true;
  else
    return false;
}

uint8_t AsnJ2735Lib::getNumPhases (const uint8_t permittedPhases) const
{
	uint8_t ret = 0;
	for (uint8_t i=0;i<NEMAPHASES;i++)
	{
		if (isPhasePermitted(i,permittedPhases))
		{
			ret ++;
		}
	}
	return (ret);
}

unsigned long AsnJ2735Lib::uchar2ulong(const u_char* ptr,const int offset, const int size) const
{
	// for decoding numerical element with fixed size (network byte order)
	unsigned long l = 0;
	
	for (int i = 0; i < size; i++)
	{
		l = (l << 8) | ptr[offset+i];
	}
	
	return (l);
}	

long AsnJ2735Lib::uchar2long(const u_char* ptr,const int offset, const int size) const
{
	// for decoding numerical element with fixed size (network byte order)
	long l = 0;
	
	// keep the sign bit
	if ( (ptr[offset] >> 7) )
	{
		l = -1;
	}
	
	for (int i = 0; i < size; i++)
	{
		l = (l << 8) | ptr[offset+i];
	}

	return (l);
}

bool AsnJ2735Lib::long2uchar(u_char* ptr,const int offset,const long l,const int size) const
{
	// for encoding: byte order is network byte order, size is given
	INTEGER_t st = (INTEGER_t){NULL,0};	

	if (asn_long2INTEGER(&st,l) < 0 )
	{
		return false;
	}
	
	if (l < 0)
	{
		for (int i=0; i<size; i++)
		{
			ptr[offset+i] = 0xFF;
		}
	}
	else
	{
		for (int i=0; i<size; i++)
		{
			ptr[offset+i] = 0x00;
		}
	}	
	
	int byte2copy = (st.size <= size) ? st.size : size;			
	// copy integer body
	for (int i = 0; i < byte2copy; i++)
	{
		ptr[offset+size-1-i] = st.buf[st.size-1-i];
	}
		
	free(st.buf);
	return true;
}

bool ::AsnJ2735Lib::ulong2uchar(u_char* ptr,const int offset,const unsigned long l,const int size) const
{
	// for encoding: byte order is network byte order, size is given
	INTEGER_t st = (INTEGER_t){NULL,0};	

	if (asn_ulong2INTEGER(&st,l) < 0 )
	{
		return false;
	}
	
	int byte2copy = (st.size <= size) ? st.size : size;
			
	// copy integer body
	for (int i = 0; i < byte2copy; i++)
	{
		ptr[offset+size-1-i] = st.buf[st.size-1-i];
	}
		
	free(st.buf);
	return true;
}

int AsnJ2735Lib::ulong2char(char* ptr,const int offset,const unsigned long l) const
{
	INTEGER_t st = (INTEGER_t){NULL,0};	

	if (asn_ulong2INTEGER(&st,l) < 0 )
	{
		return (0);
	}
		
	// skip leading empty byte (due to sign bit can be on)
	int iStart = 0;
	for (int i=0;i<st.size; i++)
	{
		if (static_cast<int>(st.buf[i]) != 0)
		{
			iStart = i;
			break;
		}
	}
		
	int byteCount = 0;
	for (int i=iStart; i<st.size;i++)
	{
		ptr[offset+byteCount] = st.buf[i];
		byteCount ++;
	}
	
	free(st.buf);
	return (byteCount);
}

double AsnJ2735Lib::uchar2elevation(const u_char* ptr,const int offset,const int size) const
{
	/* 1 decimetre LSB (10 cm) (0..614390)
		Encode elevations from 0 to 6143.9 meters above the reference ellipsoid as 0x0000 to 0xEFFF.  
		Encode elevations from -409.5 to -0.1 meters below the reference ellipsoid, as 0xF001 to 0xFFFF
		unknown as 0xF000 
	*/
		
	double elev;
	
	uint16_t ul = static_cast<uint16_t>(uchar2ulong(ptr,offset,size));
	if (ul == 0xF000)
	{
		elev = 2*msUnavailableElement;
	}
	else if (ul <= 0xEFFF)
	{
		elev = DsrcConstants::deca2unit<uint16_t>(ul);
	}
	else
	{
		elev = DsrcConstants::deca2unit<int16_t>(static_cast<int16_t>(ul));
	}
	return (elev);
}

double AsnJ2735Lib::uchar2accel(const u_char* ptr,const int offset,const int size) const
{
	/* Acceleration (INTEGER) (-2000..2001) (size 2)
		LSB units are 0.01 m/s^2
		the value 2000 shall be used for values greater than 2000     
		the value -2000 shall be used for values less than -2000  
		a value of 2001 shall be used for Unavailable
	*/

	double accel;
	long l = uchar2long(ptr,offset,size);

	if (l >= 2001 || l < -2000)
	{
		accel = 2*msUnavailableElement;
	}
	else
	{
		accel = DsrcConstants::hecto2unit<long>(l);
	}
	return (accel);
}

double AsnJ2735Lib::uchar2semiAccuracy(const u_char ch) const
{
	/* semiAccuracy (1 byte)
		semi-accuracy at one standard dev, range 0-12.7 meter, LSB = .05m
		0xFE=254=any value equal or greater than 12.70 meter
		0xFF=255=unavailable semi-accuracy value 
	*/

	double semiAccuracy;
	if (ch == 0xFF)
	{
		semiAccuracy = 2*msUnavailableElement;
	}
	else
	{
		semiAccuracy = ch / msPosAccur2Lsb;
	}
	return (semiAccuracy);
}

u_char AsnJ2735Lib::semiAccuracy2uchar(const double semiAccuracy) const
{
	u_char ch;
	if (semiAccuracy <= msUnavailableElement)
	{
		ch = 0xFF;
	}
	else
	{
		ch = static_cast<u_char>(semiAccuracy * msPosAccur2Lsb);
	}
	return (ch);
}

double AsnJ2735Lib::uchar2orientation(const u_char* ptr,const int offset,const int size) const
{
	/* relative to true north (0~359.9945078786 degrees)
		LSB units of 360/65535 deg  = 0.0054932479
		a value of 0x0000 =0 shall be 0 degrees
		a value of 0x0001 =1 shall be 0.0054932479degrees 
		a value of 0xFFFE =65534 shall be 359.9945078786 deg
		a value of 0xFFFF =65535 shall be used for orientation unavailable 
	*/
	
	double orientation;
	unsigned long ul = uchar2ulong(ptr,offset,size);
	if (ul == 0xFFFF)
	{
		orientation = 2*msUnavailableElement;
	}
	else
	{
		orientation = static_cast<double>(ul) / msOrient2Lsb;
	}
	return (orientation);
}

uint16_t AsnJ2735Lib::convertLaneAttributes(const uint16_t nmapLaneAttributes) const
{
	// nmap lane attributes bit map (bit on)              DSRC DE_VehicleLaneAttributes      
	//																										noLaneData	
	//	0 (LSB)	Lane provides a two-way travel						egressPath
	//	1	Straight maneuver permitted											maneuverStraightAllowed
	//	2	Left turn maneuver permitted										maneuverLeftAllowed
	//	3	Right turn maneuver permitted										maneuverRightAllowed
	//	4	Yield																						yield
	//	5	No U-turn 																			maneuverNoUTurn
	//	6	No turn on red 																	maneuverNoTurnOnRed
	//	7	No stopping 																		maneuverNoStop
	//	8	HOV lane																				noStop
	//	9	Bus only lane																		noTurnOnRed
	// 10	Bus and taxi only lane													hovLane
	// 11	Shared two-way left turn lane										busOnly
	// 12	Bike lane																				busAndTaxiOnly
	// 13	Reserved																				maneuverHOVLane
	// 14	Reserved																				maneuverSharedLane
	// 15																									maneuverBikeLane
	
	// (LSB) 8 bits are the same and nmap lane attributes do not use the high 8 bits 
	
	return (nmapLaneAttributes & 0xFF);
}

bool AsnJ2735Lib::decode_payload(const char* ptr,const size_t size,MSG_UNION_t* ps_msg,FILE* fp) const
{
	wmeUtils::wsmp_t wsmp;
	memset(ps_msg,0,sizeof(MSG_UNION_t));
	ps_msg->srm.reset();
	ps_msg->type = msg_enum_t::UNKNOWN_PKT;
	
	size_t offset = wmeUtils::decodewsm(&wsmp,(uint8_t*)ptr,size);
	if (offset > 0)
	{
		// wsmp header decoded		
		switch(wsmp.psid)
		{
		case wmeUtils::WSM_PSID_BSM:
			if (decode_bsm_payload(ptr+offset,(size_t)wsmp.txlength,&(ps_msg->bsm),fp))
				ps_msg->type = msg_enum_t::BSM_PKT;
			break;
		case wmeUtils::WSM_PSID_SPAT:
			if (decode_spat_payload(ptr+offset,(size_t)wsmp.txlength,&(ps_msg->spat),fp))
				ps_msg->type = msg_enum_t::SPAT_PKT;
			break;
		case wmeUtils::WSM_PSID_MAP:
			if (decode_mapdata_payload(ptr+offset,(size_t)wsmp.txlength,&(ps_msg->mapdata),fp))
				ps_msg->type = msg_enum_t::MAP_PKT;		
			break;			
		case wmeUtils::WSM_PSID_SRM:
			if (decode_srm_payload(ptr+offset,(size_t)wsmp.txlength,&(ps_msg->srm),fp))
				ps_msg->type = msg_enum_t::SRM_PKT;		
				break;
		case wmeUtils::WSM_PSID_ART:
			if (decode_art_payload(ptr+offset,(size_t)wsmp.txlength,&(ps_msg->art)))
				ps_msg->type = msg_enum_t::ART_PKT;		
				break;
		default:
			;
		}
	}
	if (ps_msg->type == msg_enum_t::UNKNOWN_PKT)
	{
		// no header found
		if (decode_bsm_payload(ptr,size,&(ps_msg->bsm),fp))
			ps_msg->type = msg_enum_t::BSM_PKT;
		else if (decode_spat_payload(ptr,size,&(ps_msg->spat),fp))
			ps_msg->type = msg_enum_t::SPAT_PKT;
		else if (decode_mapdata_payload(ptr,size,&(ps_msg->mapdata),fp))
			ps_msg->type = msg_enum_t::MAP_PKT;
		else if (decode_srm_payload(ptr,size,&(ps_msg->srm),fp))
			ps_msg->type = msg_enum_t::SRM_PKT;
		else if (decode_art_payload(ptr,size,&(ps_msg->art)))
			ps_msg->type = msg_enum_t::ART_PKT;
	}
	if (ps_msg->type != msg_enum_t::UNKNOWN_PKT)
		return true;
	else
		return false;
}

void AsnJ2735Lib::decode_bsmblob1_payload(const u_char* pbolb,BOLB1_element_t* ps_bsmblob1) const
{
	int offset = 0;
	long l;
	unsigned long ul;
	// blob1::msgCnt (long) (size 1)
	ps_bsmblob1->msgCnt = pbolb[offset];
	offset += 1;
	// blob1::TemporaryID (OCTET_STRING_t) (size 4)
	ps_bsmblob1->id = uchar2ulong(pbolb,offset,4);
	offset += 4;	
	// blob1::DSecond (long) (size 2)
	ps_bsmblob1->ms = uchar2ulong(pbolb,offset,2);
	offset += 2;	
	// blob1::Latitude (long) (size 4)
	ps_bsmblob1->lat = DsrcConstants::damega2unit<long>(uchar2long(pbolb,offset,4));
	offset += 4;		
	// blob1::Longitude (long) (size 4) 
	ps_bsmblob1->lon = DsrcConstants::damega2unit<long>(uchar2long(pbolb,offset,4));
	offset += 4;		
	// blob1::Elevation (OCTET_STRING_t) (size 2)
	ps_bsmblob1->elev = uchar2elevation(pbolb,offset,2);		
	offset += 2;		
	// blob1::PositionalAccuracy (OCTET_STRING_t) (size 4)
		// semiMajorAccuracy (size 1)
		ps_bsmblob1->semiMajorAccuracy = uchar2semiAccuracy(pbolb[offset]);
		offset += 1;					
		// semiMinorAccuracy (size 1)
		ps_bsmblob1->semiMinorAccuracy = uchar2semiAccuracy(pbolb[offset]);
		offset += 1;						
		// orientation of semi-major axis (size 2)
		ps_bsmblob1->orientation = uchar2orientation(pbolb,offset,2);
		offset += 2;					
	// blob1::TransmissionAndSpeed (OCTET_STRING_t) (SIZE 2)
	ul = uchar2ulong(pbolb,offset,2);	
	offset += 2;
		// Bits 14~16 to be made up of the data element DE_TransmissionState ::= (0..7) 
		// Bits 1~13 to be made up of the data element DE_Speed ::= INTEGER (0..8191) -- Units of 0.02 m/s
		// The value 8191 indicates that speed is unavailable
//		ps_bsmblob1->transState	= static_cast<uint8_t>(((ul & 0xE000) >> 13));
//		ul = (ul & 0x1FFF);
		ps_bsmblob1->transState	= static_cast<uint8_t>(ul & 0x07);
    ul = (ul >> 3);
		if (ul == 0x1FFF)
		{
			ps_bsmblob1->speed = 2*msUnavailableElement;
		}
		else
		{
			ps_bsmblob1->speed = static_cast<double>(ul) / msSpeed2Lsb * kmph2mps;
		}
	// blob1::Heading (long) (0..28800) (size 2)
		// LSB of 0.0125 degrees
		// A range of 0 to 359.9875 degrees
	ps_bsmblob1->heading = uchar2ulong(pbolb,offset,2) / msHeading2Lsb;
	offset += 2;	
	// blob1::SteeringWheelAngle (OCTET_STRING_t) (size 1)
		// angle: LSB units of 1.5 degrees.  
		// a range of -189 to +189 degrees
		// 0x01 = 00 = +1.5 deg
		// 0x81 = -126 = -189 deg and beyond
		// 0x7E = +126 = +189 deg and beyond
		// 0x7F = +127 to be used for unavailable
	if (pbolb[offset] == 0x7F)
	{
		ps_bsmblob1->steeringAngle = 2*msUnavailableElement;
	}
	else
	{
		ps_bsmblob1->steeringAngle = static_cast<int8_t>(pbolb[offset]) * msLsb2SteeringAngle;
	}
	offset += 1;						
	// blob1::AccelerationSet4Way (OCTET_STRING_t) (size 7)
		// long Acceleration (long) (-2000..2001) (size 2)
		ps_bsmblob1->accelLon = uchar2accel(pbolb,offset,2);
		offset += 2;
		// lat Acceleration (long) (-2000..2001) (size 2)
		ps_bsmblob1->accelLat = uchar2accel(pbolb,offset,2);
		offset += 2;
		// VerticalAcceleration (long) (-127..127) (size 1)
			// LSB units of 0.02 G steps over 
			// a range +1.54 to -3.4G 
			// and offset by 50  Value 50 = 0g, Value 0 = -1G
			// value +127 = 1.54G, 
			// value -120 = -3.4G
			// value -121 for ranges -3.4 to -4.4G
			// value -122 for ranges -4.4 to -5.4G
			// value -123 for ranges -5.4 to -6.4G
			// value -124 for ranges -6.4 to -7.4G
			// value -125 for ranges -7.4 to -8.4G
			// value -126 for ranges larger than -8.4G
			// value -127 for unavailable data	
		int8_t accelVert = static_cast<int8_t>(pbolb[offset]);
		offset += 1;			
		switch(accelVert)
		{
		case -127:
			ps_bsmblob1->accelVert = 2*msUnavailableElement;
			break;
		case -126:
			ps_bsmblob1->accelVert = -8.4;
			break;
		case -125:
			ps_bsmblob1->accelVert = -7.9;
			break;
		case -124:
			ps_bsmblob1->accelVert = -6.9;
			break;
		case -123:
			ps_bsmblob1->accelVert = -5.9;
			break;
		case -122:
			ps_bsmblob1->accelVert = -4.9;
			break;
		case -121:
			ps_bsmblob1->accelVert = -3.9;
			break;
		default:
			ps_bsmblob1->accelVert = accelVert * 0.02 - 1.0;
		}
		// YawRate (long) (-32767..32767) (size 2)
			// LSB units of 0.01 degrees per second (signed)
		l = uchar2long(pbolb,offset,2);
		offset += 2;	
		ps_bsmblob1->yawRate = DsrcConstants::hecto2unit<long>(l);
	// blob1::BrakeSystemStatus (OCTET_STRING_t) (size 2)
	ul = uchar2ulong(pbolb,offset,2);
	offset += 2;		
		// wheelBrakes        			BrakeAppliedStatus  		4 bits
		// wheelBrakesUnavailable  	BOOL 										1 bit (1=true)
		// spareBit																					1 bit (set to 0)
		// traction           			TractionControlState 		2 bits
		// abs                			AntiLockBrakeStatus 		2 bits
		// scs                			StabilityControlStatus	2 bits
		// brakeBoost         			BrakeBoostApplied 			2 bits
		// auxBrakes          			AuxiliaryBrakeStatus 		2 bits
	ps_bsmblob1->brakes = static_cast<uint16_t>(ul);
	// blob1::VehicleSize (3 bytes)
	ul = uchar2ulong(pbolb,offset,3);
	offset += 3;				
		//VehicleWidth ::= INTEGER (0..1023),  10 bits, LSB units are 1 cm
		//VehicleLength::= INTEGER (0..16383), 14 bits, LSB units are 1 cm	
		ps_bsmblob1->vehWidth = DsrcConstants::hecto2unit<unsigned long>(ul >> 14);
		ps_bsmblob1->vehLen = DsrcConstants::hecto2unit<unsigned long>(ul & 0x3FFF);
}

bool AsnJ2735Lib::decode_bsm_payload(const char* ptr,const size_t size,BSM_element_t* ps_bsm,FILE* fp) const
{
	asn_dec_rval_t rval; 							// Decoder return value 
	BasicSafetyMessage_t* pbsm = 0; 	// Type to decode
	memset(ps_bsm,0,sizeof(BSM_element_t));
	
	rval = ber_decode(0, &asn_DEF_BasicSafetyMessage,(void **)&pbsm, ptr, size);
	if (rval.code != RC_OK)
	{
		SEQUENCE_free(&asn_DEF_BasicSafetyMessage, pbsm, 0);
		return false;
	}
	
	if (fp)
	{
		xer_fprint(fp, &asn_DEF_BasicSafetyMessage, pbsm);	
	}
	
	// BasicSafetyMessage::msgID (INTEGER_t)(size 1)
	if (pbsm->msgID.buf[0] != DSRCmsgID_basicSafetyMessage)
	{
		SEQUENCE_free(&asn_DEF_BasicSafetyMessage, pbsm, 0);
		return false;
	}
	// BasicSafetyMessage::*safetyExt	(OPTIONAL)
	// BasicSafetyMessage*status (OPTIONAL)		
	// BasicSafetyMessage::blob1 (OCTET_STRING_t) (size BSMBLOB1SIZE)
	if (pbsm->blob1.size != BSMBLOB1SIZE)
	{
		SEQUENCE_free(&asn_DEF_BasicSafetyMessage, pbsm, 0);
		return false;
	}
	u_char* pbolb = (u_char *)(pbsm->blob1.buf);
	
	// decoding bolb
	decode_bsmblob1_payload(pbolb,&(ps_bsm->bolb1_element));
	
	// free pbsm
	SEQUENCE_free(&asn_DEF_BasicSafetyMessage, pbsm, 0);
	return true;
}

int AsnJ2735Lib::encode_bsmblob1_payload(const BOLB1_element_t* ps_bsmblob1,u_char* bolb1) const
{
	int offset;
	long l;
	unsigned long ul;
	
	offset = 0;
	// blob1::msgCnt (long) (0..127)	
	bolb1[offset] = ps_bsmblob1->msgCnt;
	offset += 1;
	// blob1::TemporaryID (OCTET_STRING_t) (size 4)	
	ulong2uchar(bolb1,offset,ps_bsmblob1->id,4);
	offset += 4;
	// blob1::DSecond (long) (0..65535) (size 2)	
	ulong2uchar(bolb1,offset,(unsigned long)(ps_bsmblob1->ms),2);
	offset += 2;	
	// blob1::Latitude (long) (-900000000..900000001) (size 4)
	l = DsrcConstants::unit2damega<long>(ps_bsmblob1->lat);
	long2uchar(bolb1,offset,l,4);	
	offset += 4;		
	// blob1::Longitude (long) (-1800000000..1800000001) (size 4) 
	l = DsrcConstants::unit2damega<long>(ps_bsmblob1->lon);
	long2uchar(bolb1,offset,l,4);	
	offset += 4;		
	// blob1::Elevation (OCTET_STRING_t) (-409.5..6143.9)(size 2)
	if (ps_bsmblob1->elev <= msUnavailableElement)
	{
		l = 0xF000;
	}
	else 
	{
		l = DsrcConstants::unit2deca<long>(ps_bsmblob1->elev);
	}
	long2uchar(bolb1,offset,l,2);
	offset += 2;		
	// blob1::PositionalAccuracy (OCTET_STRING_t) (size 4)
		// semi-major accuracy (1 byte) (0..12.7)
		bolb1[offset] = semiAccuracy2uchar(ps_bsmblob1->semiMajorAccuracy);
		offset += 1;		
		// semi-minor accuracy (1 byte) (0..12.7)
		bolb1[offset] = semiAccuracy2uchar(ps_bsmblob1->semiMinorAccuracy);
		offset += 1;		
		// orientation of semi-major axis (0..360)(2 bytes)
		if (ps_bsmblob1->orientation <= msUnavailableElement)
		{
			ul = 0xFFFF;
		}
		else
		{
			ul = static_cast<unsigned long>(ps_bsmblob1->orientation * msOrient2Lsb);
		}
		ulong2uchar(bolb1,offset,ul,2);	
		offset += 2;		
	// blob1::TransmissionAndSpeed (OCTET_STRING_t) (2 bytes)
	if (ps_bsmblob1->speed <= msUnavailableElement)
	{
		ul = static_cast<unsigned long>(0xFFF8) + (ps_bsmblob1->transState);
	}
	else
	{
		ul = (static_cast<unsigned long>(ps_bsmblob1->speed * msSpeed2Lsb) << 3) + (ps_bsmblob1->transState & 0x07);
	}	
	ulong2uchar(bolb1,offset,ul,2);
	offset += 2;
	// blob1::Heading (2 bytes) (0..28800)
	ul = static_cast<unsigned long>(ps_bsmblob1->heading * msHeading2Lsb);
	ulong2uchar(bolb1,offset,ul,2);
	offset += 2;
	// blob1::SteeringWheelAngle (1 byte) (-189..189)
	if (ps_bsmblob1->steeringAngle <= msUnavailableElement)
	{
		bolb1[offset] = 0x7F;
	}
	else
	{
		bolb1[offset] = static_cast<u_char>(ps_bsmblob1->steeringAngle * msLsb2SteeringAngle);		
	}
	offset += 1;
	// blob1::AccelerationSet4Way (OCTET_STRING_t) (size 7)
		// long Acceleration (2 bytes) (-2000..2001)
		if (ps_bsmblob1->accelLon <= msUnavailableElement)
		{
			l = 2001;
		}
		else
		{
			l = DsrcConstants::unit2hecto<long>(ps_bsmblob1->accelLon);
		}
		long2uchar(bolb1,offset,l,2);	
		offset += 2;
		// lat  Acceleration (2 bytes)
		if (ps_bsmblob1->accelLat <= msUnavailableElement)
		{
			l = 2001;
		}
		else
		{
			l = DsrcConstants::unit2hecto<long>(ps_bsmblob1->accelLat); 
		}
		long2uchar(bolb1,offset,l,2);	
		offset += 2;
		// VerticalAcceleration (1 byte) (-3.4..1.54)
		if (ps_bsmblob1->accelVert <= msUnavailableElement || ps_bsmblob1->accelVert > 1.54)
		{
			bolb1[offset] = 0x81;
		}
		else if (ps_bsmblob1->accelVert <= -8.4)
		{
			bolb1[offset] = 0x82;
		}
		else if (ps_bsmblob1->accelVert <= -7.4)
		{
			bolb1[offset] = 0x83;
		}
		else if (ps_bsmblob1->accelVert <= -6.4)
		{
			bolb1[offset] = 0x84;
		}
		else if (ps_bsmblob1->accelVert <= -5.4)
		{
			bolb1[offset] = 0x85;
		}
		else if (ps_bsmblob1->accelVert <= -4.4)
		{
			bolb1[offset] = 0x86;
		}
		else if (ps_bsmblob1->accelVert <= -3.4)
		{
			bolb1[offset] = 0x87;
		}
		else
		{
			bolb1[offset] = static_cast<u_char>((ps_bsmblob1->accelVert + 1.0)/0.02);
		}
		offset += 1;	
		// YawRate (2 bytes) (-32767..32767) 
		l = DsrcConstants::unit2hecto<long>(ps_bsmblob1->yawRate);
		long2uchar(bolb1,offset,l,2);	
		offset += 2;	
	// blob1::BrakeSystemStatus (OCTET_STRING_t) (size 2)
	ulong2uchar(bolb1,offset,ps_bsmblob1->brakes,2);	
	offset += 2;	
	// blob1::VehicleSize (3 bytes)
	ul = ((DsrcConstants::unit2hecto<unsigned long>(ps_bsmblob1->vehWidth)) << 14) 
		+ DsrcConstants::unit2hecto<unsigned long>(ps_bsmblob1->vehLen);
	ulong2uchar(bolb1,offset,ul,3);	
	offset += 3;	
	return (offset);
}

ssize_t AsnJ2735Lib::encode_bsm_payload(const BSM_element_t* ps_bsm,char* ptr,const size_t size,bool withHeader) const
{
	asn_enc_rval_t rval; 	// Encoder return value 
	
	BasicSafetyMessage_t* pbsm = (BasicSafetyMessage_t *)calloc(1, sizeof(BasicSafetyMessage_t));
	// BasicSafetyMessage::msgID (INTEGER_t)(size 1)
	asn_long2INTEGER(&(pbsm->msgID),DSRCmsgID_basicSafetyMessage);		
	// fill mpBsmEncode
	u_char blob1[BSMBLOB1SIZE] = {};
	int offset = encode_bsmblob1_payload(&(ps_bsm->bolb1_element),blob1);	
	// BasicSafetyMessage::blob1 (OCTET_STRING_t)
	OCTET_STRING_fromBuf(&(pbsm->blob1),(char*)blob1,offset);
	// BasicSafetyMessage::*safetyExt	(OPTIONAL)
	// BasicSafetyMessage*status (OPTIONAL)
	
	// encode BSM
	rval = der_encode_to_buffer(&asn_DEF_BasicSafetyMessage, pbsm, ptr, size);
	// free pbsm
	SEQUENCE_free(&asn_DEF_BasicSafetyMessage, pbsm, 0);
	
	if (withHeader)
	{
		msg_enum_t::MSGTYPE msgtype = msg_enum_t::BSM_PKT;	
		string str = wmeUtils::fillAradaUdp(msgtype,ptr,rval.encoded);
		memcpy(ptr,str.c_str(),str.size() + 1);
		return (str.size() + 1);
	}
	else
	{
		return (rval.encoded);	
	}
}

bool AsnJ2735Lib::decode_spat_payload(const char* ptr,const size_t size,SPAT_element_t* ps_spat,FILE* fp) const
{
	bool ret = true;
	asn_dec_rval_t rval; 	// Decoder return value 
	SPAT_t* pspat = 0; 		// Type to decode
	memset(ps_spat,0,sizeof(SPAT_element_t));
	
	rval = ber_decode(0, &asn_DEF_SPAT,(void **)&pspat, ptr, size);
	if (rval.code != RC_OK)
	{
		SEQUENCE_free(&asn_DEF_SPAT, pspat, 0);
		return false;
	}
	
	if (fp)
	{
		xer_fprint(fp, &asn_DEF_SPAT, pspat);	
	}
		
	// SPAT::DSRCmsgID_t msgID (INTEGER_t) (size 1)
	if (pspat->msgID.buf[0] != DSRCmsgID_signalPhaseAndTimingMessage)
	{
		SEQUENCE_free(&asn_DEF_SPAT, pspat, 0);
		return false;
	}
	// SPAT::intersections (A_SEQUENCE_OF IntersectionState_t) (one intersection per SPaT)
	if (pspat->intersections.list.count != 1)
	{
		SEQUENCE_free(&asn_DEF_SPAT, pspat, 0);
		return false;
	}
	IntersectionState_t* pIntsectionState = (IntersectionState_t *)(pspat->intersections.list.array[0]);
	// IntersectionState::*name (OCTET_STRING_t) (size 1..63)(OPTIONAL)
	// IntersectionState::id (OCTET_STRING_t) (size 2..4)		
		/*	higher 16 bits as the operational region (state etc)
				lower 16 bits as the unique intersection ID in the region				
				note that often only the lower 16 bits of this value will be sent as the operational region (state etc) will be known and not sent each time */				
	ps_spat->id = uchar2ulong((u_char *)(pIntsectionState->id.buf),0,pIntsectionState->id.size);
				
	// IntersectionState::status(OCTET_STRING_t) (size 1)
		/* with bits set as follows Bit #:
				0    Manual Control is enabled.  Timing reported is per programmed values, etc but person at cabinet can manually request that certain intervals are terminated early (e.g. green).
				1    Stop Time is activated and all counting/timing has stopped.
				2    Intersection is in Conflict Flash.
				3    Preemption is Active
				4    Transit Signal Priority (TSP) is Active
				5    Reserved
				6    Reserved
				7    Reserved as zero */	
	ps_spat->status = pIntsectionState->status.buf[0];
	// IntersectionState::*timeStamp (long)(0..12002)(OPTIONAL)
	ps_spat->timeStamp = 12002;
	if (pIntsectionState->timeStamp)
	{	
		ps_spat->timeStamp = static_cast<uint16_t>(*(pIntsectionState->timeStamp));
	}
	// IntersectionState::*lanesCnt (OPTIONAL)
	// IntersectionState::*priority (OPTIONAL)
	// IntersectionState::*preempt (OPTIONAL)
	// IntersectionState::states (A_SEQUENCE_OF MovementState_t)	
	// initial
	PhaseState_element_t* pPhaseState;
  bitset<NEMAPHASES> bs_vehPhase;
  bitset<NEMAPHASES> bs_pedPhase;  
  bs_vehPhase.reset();
  bs_pedPhase.reset();
	for (int i=0;i<pIntsectionState->states.list.count;i++)
	{	
		MovementState_t* pMovementState = (MovementState_t *)(pIntsectionState->states.list.array[i]);
		// MovementState::*movementName (OPTIONAL)
		// MovementState::*laneCnt (OPTIONAL)
		// MovementState::laneSet
		if (pMovementState->laneSet.size <= 0)
		{
			ret = false;
			break;
		}
		uint8_t phaseId = mpMapData->getControlPhaseByLaneId(ps_spat->id,pMovementState->laneSet.buf[0]);

		if (!(phaseId >= 1 && phaseId <= NEMAPHASES))
		{
			ret = false;
			break;
		}
		--phaseId;
		
		// determine whether the movement is for vehicular or pedestrian
		bool isPedMovement = false;
		if (pMovementState->currState || pMovementState->yellState)
		{
			isPedMovement = false;
			// permittedPhases
      bs_vehPhase.set(phaseId);
			// structure PhaseState_element_t
			pPhaseState = &(ps_spat->phaseState[phaseId]);
		}
		else if (pMovementState->pedState || pMovementState->yellPedState)
		{
			isPedMovement = true;
			// permittedPedPhases
      bs_pedPhase.set(phaseId);
			// structure PhaseState_element_t
			pPhaseState = &(ps_spat->pedPhaseState[phaseId]);
		}
		else
		{
			continue;
		}
		
		// fill PhaseState_element_t
		// currState (MovementState::*currState | MovementState::*pedState) (OPTIONAL)
		pPhaseState->currState = phasestate_enum_t::UNKNOWN;
		// nextState (MovementState::*yellState | MovementState::*yellPedState) (OPTIONAL)
		pPhaseState->nextState = phasestate_enum_t::UNKNOWN;
		if (!isPedMovement)
		{
			// vehicular movement
			if (pMovementState->currState)
			{
				// MovementState::*currState (long)(OPTIONAL)			
				pPhaseState->currState = static_cast<uint32_t>(getPhaseState(static_cast<uint32_t>(*(pMovementState->currState))));
			}		
			if (pMovementState->yellState)
			{
				// MovementState::*yellState (long)(OPTIONAL)			
				pPhaseState->nextState = static_cast<uint32_t>(getPhaseState(static_cast<uint32_t>(*(pMovementState->yellState))));
			}
		}
		else 
		{
			// pedestrian movement
			if (pMovementState->pedState)
			{
				// PedestrianSignalState_t *pedState (INTEGER_t)(OPTIONAL)
				pPhaseState->currState = static_cast<uint32_t>(uchar2ulong((u_char *)(pMovementState->pedState->buf),0,pMovementState->pedState->size));
			}
			if (pMovementState->yellPedState)
			{
				// PedestrianSignalState_t *yellPedState (INTEGER_t)(OPTIONAL)
				pPhaseState->nextState = static_cast<uint32_t>(uchar2ulong((u_char *)(pMovementState->yellPedState->buf),0,pMovementState->yellPedState->size));
			}
		}
		// MovementState::timeToChange (long)
		pPhaseState->timeToChange = static_cast<uint16_t>(pMovementState->timeToChange);
		// MovementState::*stateConfidence(INTEGER_t)(OPTIONAL)
		pPhaseState->stateConfidence = StateConfidence_unKnownEstimate;
		if (pMovementState->stateConfidence)
		{
			pPhaseState->stateConfidence = uchar2ulong((u_char *)(pMovementState->stateConfidence->buf),0,pMovementState->stateConfidence->size);
		}
		// clearanceIntv (MovementState::*yellTimeToChange) (long) (OPTIONAL)
		pPhaseState->clearanceIntv = msTimeMarkUnKnown;
		if (pMovementState->yellTimeToChange)
		{
			pPhaseState->clearanceIntv = static_cast<uint16_t>(*(pMovementState->yellTimeToChange));
		}
		// yellStateConfidence (MovementState::*yellStateConfidence) (INTEGER_t)(OPTIONAL) 
		pPhaseState->yellStateConfidence = StateConfidence_unKnownEstimate;
		if (pMovementState->yellStateConfidence)
		{
			pPhaseState->yellStateConfidence = uchar2ulong((u_char *)(pMovementState->yellStateConfidence->buf),0,pMovementState->yellStateConfidence->size);
		}
		// MovementState::*vehicleCount (OPTIONAL)
		// MovementState::*pedDetect (OPTIONAL)
		// MovementState::*pedCount (OPTIONAL)
	}
	ps_spat->permittedPhases = static_cast<uint8_t>(bs_vehPhase.to_ulong());
	ps_spat->permittedPedPhases = static_cast<uint8_t>(bs_pedPhase.to_ulong());
	
	// free pspat
	SEQUENCE_free(&asn_DEF_SPAT, pspat, 0);
	return (ret);
}

ssize_t AsnJ2735Lib::encode_spat_payload(const SPAT_element_t* ps_spat,char* ptr,const size_t size,const char* pIntersectionName,bool withHeader) const
{
	SPAT_element_t spat_in;
	memcpy(&spat_in,ps_spat,sizeof(SPAT_element_t));
	uint32_t intersectionId = getIntersectionIdByName(pIntersectionName);
	if (intersectionId == 0)
	{
		return (-1);
	}
	spat_in.id = intersectionId;
	return (encode_spat_payload(&spat_in,ptr,size,withHeader));
}
	
ssize_t AsnJ2735Lib::encode_spat_payload(const SPAT_element_t* ps_spat,char* ptr,const size_t size,bool withHeader) const
{
	asn_enc_rval_t	rval; 	// Encoder return value 
	char buffer[50];
	int len;
	
	uint8_t numPhases = getNumPhases(ps_spat->permittedPhases);
	uint8_t numPedPhases = getNumPhases(ps_spat->permittedPedPhases);
	if (numPhases == 0 && numPedPhases == 0)
	{
		// nothing to encode
		return (-1);
	}
	
	SPAT_t* pspat = (SPAT_t *)calloc(1, sizeof(SPAT_t));
	// SPAT::msgID (INTEGER_t) (size 1)
	asn_long2INTEGER(&(pspat->msgID),DSRCmsgID_signalPhaseAndTimingMessage);
	// SPAT::*name (DescriptiveName_t) (size 1..63)(OPTIONAL)
	// SPAT::struct SPAT__intersections intersections (A_SEQUENCE_OF IntersectionState_t) (one intersection per SPaT)
	
	IntersectionState_t* pIntsectionState = (IntersectionState_t *)calloc(1, sizeof(IntersectionState_t));
	// IntersectionState::*name (DescriptiveName_t) (size 1..63)(OPTIONAL)
	len = getIntersectionNameById(ps_spat->id,buffer,sizeof(buffer));	
	if (len <= 0)
	{
		return (-1);
	}
	pIntsectionState->name = OCTET_STRING_new_fromBuf(&asn_DEF_DescriptiveName,buffer,len);
	// IntersectionState::id (IntersectionID_t->OCTET_STRING_t) (size 2..4)
	int bytenum = byteNums(ps_spat->id);
	uint32_t intId = (htonl(ps_spat->id) >> ((sizeof(uint32_t) - bytenum) * 8));
	OCTET_STRING_fromBuf(&(pIntsectionState->id),(char*)&intId,bytenum);
	// IntersectionState::status(IntersectionStatusObject_t->OCTET_STRING_t) (size 1)
	OCTET_STRING_fromBuf(&(pIntsectionState->status),(char*)&(ps_spat->status),1);
	// IntersectionState::*timeStamp (TimeMark_t->long)(0..12002)(OPTIONAL)
	pIntsectionState->timeStamp = (TimeMark_t *)calloc(1, sizeof(TimeMark_t));
	*(pIntsectionState->timeStamp) = (TimeMark_t)(ps_spat->timeStamp);
	// IntersectionState::*lanesCnt (1..255)(OPTIONAL)
	// IntersectionState::*priority	(OPTIONAL)
	// IntersectionState::*preempt (OPTIONAL)
	// IntersectionState::states (A_SEQUENCE_OF MovementState_t) (Vehicle & Ped)
	
	// states::MovementState for vehicle
	for (uint8_t i=0;i<NEMAPHASES;i++)
	{
		if (!isPhasePermitted(i,ps_spat->permittedPhases))
		{
			continue;
		}
		const PhaseState_element_t* pPhaseState = &(ps_spat->phaseState[i]);
		MovementState_t* pMovementState = (MovementState_t *)calloc(1, sizeof(MovementState_t));
	
		// MovementState::*movementName (OCTET_STRING_t) (size 1..63)(OPTIONAL)
		// MovementState::*laneCnt (long)(0..255)(OPTIONAL)
		// MovementState::laneSet (OCTET_STRING_t) (size 1..32)
		len = static_cast<int>(mpMapData->getLaneSetByPhase(ps_spat->id,(uint8_t)(i+1),buffer));
		if (len == 0)
		{
			continue;
		}
		OCTET_STRING_fromBuf(&(pMovementState->laneSet),buffer,len);
		// MovementState::*specialState	(OPTIONAL)
		// MovementState::*currState (SignalLightState_t, long)(OPTIONAL)
		pMovementState->currState = (SignalLightState_t *)calloc(1, sizeof(SignalLightState_t));
		*(pMovementState->currState) = (SignalLightState_t)(pPhaseState->currState);
		// MovementState::timeToChange (long)
		pMovementState->timeToChange = pPhaseState->timeToChange;
		// MovementState::*stateConfidence(INTEGER_t)(OPTIONAL)
		pMovementState->stateConfidence = (StateConfidence_t *)calloc(1, sizeof(StateConfidence_t));
		asn_long2INTEGER(pMovementState->stateConfidence,pPhaseState->stateConfidence);
		// MovementState::*yellState (long)(OPTIONAL)
		pMovementState->yellState = (SignalLightState_t *)calloc(1, sizeof(SignalLightState_t));
		*(pMovementState->yellState) = (SignalLightState_t)(pPhaseState->nextState);
		// MovementState::*yellTimeToChange (long)(OPTIONAL)
		pMovementState->yellTimeToChange = (TimeMark_t *)calloc(1, sizeof(TimeMark_t));
		*(pMovementState->yellTimeToChange) = (TimeMark_t)(pPhaseState->clearanceIntv);
		// MovementState::*yellStateConfidence (INTEGER_t)(OPTIONAL)
		pMovementState->yellStateConfidence = (StateConfidence_t *)calloc(1, sizeof(StateConfidence_t));
		asn_long2INTEGER(pMovementState->yellStateConfidence,pPhaseState->yellStateConfidence);
		// MovementState::*vehicleCount (OPTIONAL)
		
		// add MovementState_t to IntersectionState_t
		asn_sequence_add(&(pIntsectionState->states), pMovementState);
	}

	// states::MovementState for pedestrian
	for (uint8_t i=0;i<NEMAPHASES;i++)
	{
		if (!isPhasePermitted(i,ps_spat->permittedPedPhases))
		{
			continue;
		}
		const PhaseState_element_t* pPhaseState = &(ps_spat->pedPhaseState[i]);	
		MovementState_t* pPedMovementState = (MovementState_t *)calloc(1, sizeof(MovementState_t));				
		// MovementState::*movementName (OCTET_STRING_t) (size 1..63)(OPTIONAL)
		// MovementState::*laneCnt (long)(0..255)(OPTIONAL)
		// MovementState::laneSet (OCTET_STRING_t)
		len = static_cast<int>(mpMapData->getLaneSetByPedPhase(ps_spat->id,(uint8_t)(i+1),buffer));
		if (len == 0)
		{
			continue;
		}
		OCTET_STRING_fromBuf(&(pPedMovementState->laneSet),buffer,len);
		// MovementState::*specialState	(OPTIONAL)
		// MovementState::*pedState (INTEGER_t)(OPTIONAL)
		pPedMovementState->pedState = (PedestrianSignalState_t *)calloc(1, sizeof(PedestrianSignalState_t));
		asn_long2INTEGER(pPedMovementState->pedState,getPedPhaseState(pPhaseState->currState));
		// MovementState::timeToChange (long)
		pPedMovementState->timeToChange = pPhaseState->timeToChange;
		// MovementState::*stateConfidence(INTEGER_t)(OPTIONAL)
		pPedMovementState->stateConfidence = (StateConfidence_t *)calloc(1, sizeof(StateConfidence_t));
		asn_long2INTEGER(pPedMovementState->stateConfidence,pPhaseState->stateConfidence);
		// MovementState::*yellPedState (INTEGER_t)(OPTIONAL)
		pPedMovementState->yellPedState = (PedestrianSignalState_t *)calloc(1, sizeof(PedestrianSignalState_t));
		asn_long2INTEGER(pPedMovementState->yellPedState,getPedPhaseState(pPhaseState->nextState));
		// MovementState::*yellTimeToChange (long)(OPTIONAL)
		pPedMovementState->yellTimeToChange = (TimeMark_t *)calloc(1, sizeof(TimeMark_t));
		*(pPedMovementState->yellTimeToChange) = (TimeMark_t)(pPhaseState->clearanceIntv);
		// MovementState::*yellStateConfidence (INTEGER_t)(OPTIONAL)
		pPedMovementState->yellStateConfidence = (StateConfidence_t *)calloc(1, sizeof(StateConfidence_t));
		asn_long2INTEGER(pPedMovementState->yellStateConfidence,pPhaseState->yellStateConfidence);
		// MovementState::*pedDetect (OPTIONAL)
		// MovementState::*pedCount (OPTIONAL)
	
		// add MovementState to IntersectionState
		asn_sequence_add(&(pIntsectionState->states), pPedMovementState);		
	}
		
	// add IntersectionState to SPAT
	asn_sequence_add(&(pspat->intersections), pIntsectionState);	
	
	// encode SPaT
	rval = der_encode_to_buffer(&asn_DEF_SPAT, pspat, ptr, size);
	// free pspat
	SEQUENCE_free(&asn_DEF_SPAT, pspat, 0);
	if (withHeader)
	{
		msg_enum_t::MSGTYPE msgtype = msg_enum_t::SPAT_PKT;
		string str = wmeUtils::fillAradaUdp(msgtype,ptr,rval.encoded);
		memcpy(ptr,str.c_str(),str.size() + 1);
		return (str.size() + 1);
	}
	else
	{
		return (rval.encoded);	
	}
}

bool AsnJ2735Lib::decode_mapdata_payload(const char* ptr,const size_t size,Mapdata_element_t* ps_mapdata,FILE *fp) const
{
	asn_dec_rval_t rval; 				// Decoder return value 
	MapData_t* pmapdata = 0; 		// Type to decode
	memset(ps_mapdata,0,sizeof(Mapdata_element_t));
	
	rval = ber_decode(0, &asn_DEF_MapData,(void **)&pmapdata, ptr, size);
	if (rval.code != RC_OK)
	{
		SEQUENCE_free(&asn_DEF_MapData, pmapdata, 0);
		return false;
	}
		
	if (fp)
	{
		xer_fprint(fp, &asn_DEF_MapData, pmapdata);	
	}
	
	// DSRCmsgID_t	 msgID
	if (pmapdata->msgID.buf[0] != DSRCmsgID_mapData)
	{
		SEQUENCE_free(&asn_DEF_MapData, pmapdata, 0);
		return false;
	}
	
	// MsgCount_t	 msgCnt
	ps_mapdata->mapVersion = static_cast<uint8_t>(pmapdata->msgCnt);
	ps_mapdata->id = 0;
	// MapData__intersections *intersections (A_SEQUENCE_OF Intersecion_t) (one intersection per mapdata)
	if (pmapdata->intersections)
	{
		if (pmapdata->intersections->list.count != 1)
		{
			SEQUENCE_free(&asn_DEF_MapData, pmapdata, 0);
			return false;
		}
		Intersection_t* pIntersection = (Intersection_t *)(pmapdata->intersections->list.array[0]);
		// Intersection::IntersectionID_t	 id
		ps_mapdata->id = uchar2ulong((u_char *)(pIntersection->id.buf),0,pIntersection->id.size);
	}
	
	// free pmapdata
	SEQUENCE_free(&asn_DEF_MapData, pmapdata, 0);
	return (true);
}

void AsnJ2735Lib::encode_mapdata_payload(void) const
{
	char buffer[MAXDSRCMSGSIZE];
	int consumedBytes;
	
	for (size_t i = 0; i< mpMapData->mpIntersection.size(); i++)
	{
		consumedBytes = encode_mapdata_payload(mpMapData->mpIntersection[i].id, buffer, MAXDSRCMSGSIZE);
		if (consumedBytes > 0)
		{
			mpMapData->mpIntersection[i].mapPayload.assign(buffer,(size_t)consumedBytes);
		}
	}
}

ssize_t AsnJ2735Lib::encode_mapdata_payload(const uint32_t intersectionId,char* ptr,size_t size) const
{
	asn_enc_rval_t	rval;  // Encoder return value

	int index = mpMapData->getIndexByIntersectionId(intersectionId);
	
	if (index < 0)
	{
		// can't find intersectionId from MapDataStruct
		return (-1);
	}
	
	if (mpMapData->mpIntersection[index].mpApproaches.empty())
	{
		// nothing to encode
		return (-1);
	}
	
	MapData_t* pmapdata = (MapData_t *)calloc(1, sizeof(MapData_t));
	// MapData::msgID (INTEGER_t)(size 1)
	asn_long2INTEGER(&(pmapdata->msgID),DSRCmsgID_mapData);
	// MapData::msgCnt (long) (0..127) (size 1)	
	pmapdata->msgCnt = mpMapData->getMapVersion();	
	// MapData::*name (OCTET_STRING_t) (size 1..63) (OPTIONAL)
	// MapData::*layerType (INTEGER_t) (size 1) (OPTIONAL)
	// MapData::LayerID_t	*layerID (long) (0..100) (size 1) (OPTIONAL)
	// MapData::*dataParameters	(OPTIONAL)
	// MapData::*intersections (size 1..32) (A_SEQUENCE_OF Intersection_t list) (one per Map)
	pmapdata->intersections = (MapData::MapData__intersections*)calloc(1, sizeof(MapData::MapData__intersections));	
	
	// MapData::*intersections::Intersection
	Intersection_t* pIntersection = (Intersection_t *)calloc(1, sizeof(Intersection_t));
	// fill Intersection
	// Intersection::*name (OCTET_STRING_t) (size 1..63) (OPTIONAL)
	// Intersection::id (OCTET_STRING_t) (size 2..4)	
	int bytenum = byteNums(intersectionId);
	uint32_t intId = (htonl(intersectionId) >> ((sizeof(uint32_t) - bytenum) * 8));
	OCTET_STRING_fromBuf(&(pIntersection->id),(char*)&intId,bytenum);
	// Intersection::*refPoint (OPTIONAL)	
	pIntersection->refPoint = (Position3D_t *)calloc(1, sizeof(Position3D_t));
		// Intersection::*refPoint::lat (long)
		pIntersection->refPoint->lat = mpMapData->mpIntersection[index].geoRef.latitude;
		// Intersection::*refPoint::Long (long)
		pIntersection->refPoint->Long = mpMapData->mpIntersection[index].geoRef.longitude;
		// Intersection::*refPoint::*elevation (OCTET_STRING_t) (in decimetre) (size 2)
		int16_t refElev = htons(static_cast<int16_t>(DsrcConstants::deca2unit<int32_t>(mpMapData->mpIntersection[index].geoRef.elevation)));
		pIntersection->refPoint->elevation = OCTET_STRING_new_fromBuf(&asn_DEF_Elevation,(char*)&refElev,2);
	// Intersection::*refInterNum (OCTET_STRING_t) (size 2..4) (OPTIONAL)
	// Intersection::*orientation (long) (OPTIONAL)
	// Intersection::*laneWidth (long) (OPTIONAL) (laneWidth is provided by lane below)
	// Intersection::*type (OCTET_STRING_t) (size 1) (OPTIONAL)
	// Intersection::*preemptZones (A_SEQUENCE_OF SignalControlZone_t list) (size 1..32) (OPTIONAL)
	// Intersection::*priorityZones (A_SEQUENCE_OF SignalControlZone_t list) (size 1..32) (OPTIONAL)
	// Intersection::approaches (A_SEQUENCE_OF ApproachObject_t list) (size 1..32) 	(each ApproachObject_t include one approach and one egress)
	int approachObjNums = NEMAPHASES / 2;
	for (int i = 0; i < approachObjNums; i++)
	{
		if (mpMapData->mpIntersection[index].mpApproaches[i*2].mpLanes.size() == 0 &&
			mpMapData->mpIntersection[index].mpApproaches[i*2+1].mpLanes.size() == 0)
		{
			// empty ApproachObject_t
			continue;
		}
		ApproachObject_t* pApproachObj = (ApproachObject_t *)calloc(1, sizeof(ApproachObject_t));
		// ApproachObject::*refPoint (OPTIONAL) (Refer to MapData_t::intersections::*refPoint)
		// ApproachObject::*laneWidth	(long) (OPTIONAL) (Info include by lane below)
		for (int j=0;j<2;j++)
		{
			if (mpMapData->mpIntersection[index].mpApproaches[i*2+j].mpLanes.size() > 0)
			{
				if (mpMapData->mpIntersection[index].mpApproaches[i*2+j].type == NmapData::maneuver_enum_t::APPROACH)
				{
					// ApproachObject::*approach
					pApproachObj->approach = (Approach_t *)calloc(1, sizeof(Approach_t));
					// Approach::*name (OCTET_STRING_t) (OPTIONAL)
					// Approach::*id (long) (0..127)(OPTIONAL)
					// Approach::*computedLanes (OPTIONAL)
					// Approach::*trainsAndBuses (OPTIONAL)
					// Approach::*barriers (OPTIONAL)
					// Approach::*drivingLanes (OPTIONAL)
					addDrivingLanes2approachObj(pApproachObj->approach,index,i*2+j,true);
					// Approach::*crosswalks (OPTIONAL)
					if (mpMapData->mpIntersection[index].mpApproaches[i*2+j+NEMAPHASES-i].mpLanes.size() > 0)
					{
						int broadcastlanes = 0;
						for (size_t k = 0; k < mpMapData->mpIntersection[index].mpApproaches[i*2+j+NEMAPHASES-i].mpLanes.size(); k++)
						{
							int broadcastpts = 0;
							for (size_t ii = 0; ii < mpMapData->mpIntersection[index].mpApproaches[i*2+j+NEMAPHASES-i].mpLanes[k].mpNodes.size();ii++)
							{	
								if (mpMapData->mpIntersection[index].mpApproaches[i*2+j+NEMAPHASES-i].mpLanes[k].mpNodes[ii].isBroadcast)
								{
									++broadcastpts;
								}
							}
							if (broadcastpts >= 2)
							{
								++broadcastlanes;
							}
						}
						if (broadcastlanes > 0)
						{
							addCrosswalk2approachObj(pApproachObj->approach,index,i*2+j+NEMAPHASES-i);
						}
					}
				}
				else if (mpMapData->mpIntersection[index].mpApproaches[i*2+j].type == NmapData::maneuver_enum_t::EGRESS)
				{
					// ApproachObject::*egress
					pApproachObj->egress = (Approach_t *)calloc(1, sizeof(Approach_t));
					// Approach::*id (long) (0..127)(OPTIONAL)					
					// Approach::*drivingLanes (OPTIONAL)
					addDrivingLanes2approachObj(pApproachObj->egress,index,i*2+j,false);
					// Approach::*crosswalks (OPTIONAL) (added in approach)
				}
			}
		}
		
		// add ApproachObject to Intersection::approaches
		asn_sequence_add(&(pIntersection->approaches), pApproachObj);
	}
	// add Intersection to MapData
	asn_sequence_add(pmapdata->intersections, pIntersection);		
	
	// MapData::crc (OCTET_STRING_t) (size 2) (created with the CRC-CCITT polynomial(KERMIT))
	uint16_t crc = htons(crc16((char *)pmapdata, sizeof(*pmapdata)));
	OCTET_STRING_fromBuf(&(pmapdata->crc),(char*)&crc,2);
	
	// encode mapdata
	rval = der_encode_to_buffer(&asn_DEF_MapData, pmapdata, ptr, size);

	// free pmapdata
	SEQUENCE_free(&asn_DEF_MapData, pmapdata, 0);
		
	return (rval.encoded);	
}

void AsnJ2735Lib::addDrivingLanes2approachObj(struct Approach* pApproach,const int index,const int approachIndex,const bool isIngress) const
{	
	char buffer[20];
	int offset = 0;
	int16_t d_eOffset,d_nOffset;
	int32_t	offsets;
	int prevNodeIndex;
	
	pApproach->drivingLanes = (Approach::Approach__drivingLanes *)calloc(1, sizeof(Approach::Approach__drivingLanes));	
	for (size_t j=0; j < mpMapData->mpIntersection[index].mpApproaches[approachIndex].mpLanes.size(); j++)
	{
		VehicleReferenceLane_t* pDrivingLane = (VehicleReferenceLane_t *)calloc(1, sizeof(VehicleReferenceLane_t));	
		// *drivingLanes::laneNumber (OCTET_STRING_t) (size 1)
		OCTET_STRING_fromBuf(&(pDrivingLane->laneNumber),(char*)&(mpMapData->mpIntersection[index].mpApproaches[approachIndex].mpLanes[j].id),1);
		// *drivingLanes::*laneWidth (long) (0..32767)(units of 1 cm) (OPTIONAL)
		pDrivingLane->laneWidth = (LaneWidth_t *)calloc(1, sizeof(LaneWidth_t));
		*(pDrivingLane->laneWidth) = mpMapData->mpIntersection[index].mpApproaches[approachIndex].mpLanes[j].width;
		// *drivingLanes::laneAttributes (long)
		pDrivingLane->laneAttributes = convertLaneAttributes(mpMapData->mpIntersection[index].mpApproaches[approachIndex].mpLanes[j].attributes);
		// *drivingLanes::*keepOutList	(OPTIONAL)				
		// *drivingLanes::nodeList (size 1..64)
		prevNodeIndex = -1;
		for (size_t k = 0; k < mpMapData->mpIntersection[index].mpApproaches[approachIndex].mpLanes[j].mpNodes.size(); k++)
		{
			// *drivingLanes::nodeList::Offsets_t (size 4..8)
			// OCTET_STRING_t, Made up of SEQUENCE { 	
			//		-- xOffset  INTEGER (-32767..32767), (size 2)
			//		-- yOffset  INTEGER (-32767..32767), (size 2)
			//		-- if 6 or 8 bytes in length:
			//		-- zOffset  INTEGER (-32767..32767) OPTIONAL, (size 2)
			//		-- all above in signed values where the LSB is in units of 1.0 cm   
			//		-- if 8 bytes in length:
			//		-- width    LaneWidth  OPTIONAL (size 2)
			//		-- a length of 7 bytes is never used				
			if (!mpMapData->mpIntersection[index].mpApproaches[approachIndex].mpLanes[j].mpNodes[k].isBroadcast)
				continue;
			if (prevNodeIndex < 0)
			{
				d_eOffset = static_cast<int16_t>(mpMapData->mpIntersection[index].mpApproaches[approachIndex].mpLanes[j].mpNodes[k].ptNode.x);
				d_nOffset = static_cast<int16_t>(mpMapData->mpIntersection[index].mpApproaches[approachIndex].mpLanes[j].mpNodes[k].ptNode.y);
			}
			else
			{
				d_eOffset = static_cast<int16_t>(mpMapData->mpIntersection[index].mpApproaches[approachIndex].mpLanes[j].mpNodes[k].ptNode.x
					- mpMapData->mpIntersection[index].mpApproaches[approachIndex].mpLanes[j].mpNodes[prevNodeIndex].ptNode.x);
				d_nOffset = static_cast<int16_t>(mpMapData->mpIntersection[index].mpApproaches[approachIndex].mpLanes[j].mpNodes[k].ptNode.y
					- mpMapData->mpIntersection[index].mpApproaches[approachIndex].mpLanes[j].mpNodes[prevNodeIndex].ptNode.y);
			}
			prevNodeIndex = static_cast<int>(k);
			offsets = htonl((d_eOffset << 16) + d_nOffset);
			Offsets_t* pOffset = OCTET_STRING_new_fromBuf(&asn_DEF_Offsets,(char*)&offsets,4);			
			// add Offsets to *drivingLanes
			asn_sequence_add(&(pDrivingLane->nodeList), pOffset);
		}

		// ConnectsTo_t	*connectsTo (OCTET_STRING_t) (list of pair bytes, connectToLandId & connectToManuver)
		if (isIngress && !(mpMapData->mpIntersection[index].mpApproaches[approachIndex].mpLanes[j].mpConnectTo.empty()))
		{
			offset = 0;
			for (size_t k = 0; k < mpMapData->mpIntersection[index].mpApproaches[approachIndex].mpLanes[j].mpConnectTo.size(); k++)
			{
				buffer[offset] = mpMapData->mpIntersection[index].mpApproaches[approachIndex].mpLanes[j].mpConnectTo[k].laneId;
				offset++;
				buffer[offset] = mpMapData->mpIntersection[index].mpApproaches[approachIndex].mpLanes[j].mpConnectTo[k].laneManeuver;
				offset++;
			}
			pDrivingLane->connectsTo = OCTET_STRING_new_fromBuf(&asn_DEF_OCTET_STRING,buffer,offset);
		}
		
		// add *drivingLanes to Approach
		asn_sequence_add(pApproach->drivingLanes, pDrivingLane);
	}
}

void AsnJ2735Lib::addCrosswalk2approachObj(struct Approach* pApproach,const int index,const int approachIndex) const
{	
	int16_t d_eOffset,d_nOffset;
	int32_t	offset;
	int prevNodeIndex;
	
	pApproach->crosswalks = (Approach::Approach__crosswalks *)calloc(1, sizeof(Approach::Approach__crosswalks));	
	for (size_t j=0; j < mpMapData->mpIntersection[index].mpApproaches[approachIndex].mpLanes.size(); j++)
	{
		CrosswalkLane_t* pCrossWalk = (CrosswalkLane_t *)calloc(1, sizeof(CrosswalkLane_t));	
		// *crosswalks::laneNumber (OCTET_STRING_t) (size 1)
		OCTET_STRING_fromBuf(&(pCrossWalk->laneNumber),(char*)&(mpMapData->mpIntersection[index].mpApproaches[approachIndex].mpLanes[j].id),1);
		// *crosswalks::*laneWidth (long) (0..32767)(units of 1 cm) (OPTIONAL)
		pCrossWalk->laneWidth = (LaneWidth_t *)calloc(1, sizeof(LaneWidth_t));	
		*(pCrossWalk->laneWidth) = mpMapData->mpIntersection[index].mpApproaches[approachIndex].mpLanes[j].width;
		// *crosswalks::laneAttributes (INTEGER_t)
		asn_long2INTEGER(&(pCrossWalk->laneAttributes),(uint16_t)(mpMapData->mpIntersection[index].mpApproaches[approachIndex].mpLanes[j].attributes));	
		// *crosswalks::*keepOutList	(OPTIONAL)				
		// *crosswalks::nodeList (size 1..64)
		prevNodeIndex = -1;		
		for (size_t k = 0; k < mpMapData->mpIntersection[index].mpApproaches[approachIndex].mpLanes[j].mpNodes.size(); k++)
		{
			// *crosswalks::nodeList::Offsets_t (size 4..8)
			// OCTET_STRING_t, Made up of SEQUENCE { 	
			//		-- xOffset  INTEGER (-32767..32767), (size 2)
			//		-- yOffset  INTEGER (-32767..32767), (size 2)
			//		-- if 6 or 8 bytes in length:
			//		-- zOffset  INTEGER (-32767..32767) OPTIONAL, (size 2)
			//		-- all above in signed values where the LSB is in units of 1.0 cm   
			//		-- if 8 bytes in length:
			//		-- width    LaneWidth  OPTIONAL (size 2)
			//		-- a length of 7 bytes is never used			
			if (!mpMapData->mpIntersection[index].mpApproaches[approachIndex].mpLanes[j].mpNodes[k].isBroadcast)
				continue;
			if (prevNodeIndex < 0)
			{
				d_eOffset = static_cast<int16_t>(mpMapData->mpIntersection[index].mpApproaches[approachIndex].mpLanes[j].mpNodes[k].ptNode.x);
				d_nOffset = static_cast<int16_t>(mpMapData->mpIntersection[index].mpApproaches[approachIndex].mpLanes[j].mpNodes[k].ptNode.y);
			}
			else
			{
				d_eOffset = static_cast<int16_t>(mpMapData->mpIntersection[index].mpApproaches[approachIndex].mpLanes[j].mpNodes[k].ptNode.x
					- mpMapData->mpIntersection[index].mpApproaches[approachIndex].mpLanes[j].mpNodes[prevNodeIndex].ptNode.x);
				d_nOffset = static_cast<int16_t>(mpMapData->mpIntersection[index].mpApproaches[approachIndex].mpLanes[j].mpNodes[k].ptNode.y
					- mpMapData->mpIntersection[index].mpApproaches[approachIndex].mpLanes[j].mpNodes[prevNodeIndex].ptNode.y);
			}
			prevNodeIndex = static_cast<int>(k);			
			offset = htonl((d_eOffset << 16) + d_nOffset);
			Offsets_t* pOffset = OCTET_STRING_new_fromBuf(&asn_DEF_Offsets,(char*)&offset,4);
			
			// add Offsets to CrosswalkLane
			asn_sequence_add(&(pCrossWalk->nodeList), pOffset);
		}		
		// add CrosswalkLane to Approach
		asn_sequence_add(pApproach->crosswalks, pCrossWalk);
	}
}

ssize_t AsnJ2735Lib::encode_srm_payload(const SRM_element_t* ps_srm,char* ptr,const size_t size,bool withHeader) const
{
	asn_enc_rval_t	rval; 	// Encoder return value 
	
	SignalRequestMsg_t* psrm = (SignalRequestMsg_t *)calloc(1,sizeof(SignalRequestMsg_t)); 
	// SignalRequestMsg::msgID (INTEGER_t) (size 1)
	asn_long2INTEGER(&(psrm->msgID),DSRCmsgID_signalRequestMessage);
	// SignalRequestMsg::msgCnt (long) (size 1)
	psrm->msgCnt = ps_srm->msgCnt;
	// SignalRequestMsg::request
	//	request::id
	int bytenum = byteNums(ps_srm->signalRequest_element.id);
	uint32_t intId = (htonl(ps_srm->signalRequest_element.id) >> ((sizeof(uint32_t) - bytenum) * 8));
	OCTET_STRING_fromBuf(&(psrm->request.id),(char*)&intId,bytenum);
	//	request::*requestedAction OCTET_STRING_t
	if (ps_srm->signalRequest_element.requestedAction == priorityrequest_enum_t::CANCELPRIORITY 
		|| ps_srm->signalRequest_element.requestedAction == priorityrequest_enum_t::CANCELPREEMP)
	{
		psrm->request.isCancel = OCTET_STRING_new_fromBuf(&asn_DEF_OCTET_STRING,(char *)(&(ps_srm->signalRequest_element.requestedAction)),1);
	}
	else if (ps_srm->signalRequest_element.requestedAction == priorityrequest_enum_t::REQUESTPRIORITY 
		|| ps_srm->signalRequest_element.requestedAction == priorityrequest_enum_t::REQUESTPREEMP)
	{
		psrm->request.requestedAction = OCTET_STRING_new_fromBuf(&asn_DEF_OCTET_STRING,(char *)(&(ps_srm->signalRequest_element.requestedAction)),1);	
	}
	//	request::*inLane 
	psrm->request.inLane = OCTET_STRING_new_fromBuf(&asn_DEF_OCTET_STRING,(char *)(&(ps_srm->signalRequest_element.inLaneId)),1);
	//	request::*outLane 
	psrm->request.outLane = OCTET_STRING_new_fromBuf(&asn_DEF_OCTET_STRING,(char *)(&(ps_srm->signalRequest_element.outLaneId)),1);
	//	request::type (NTCIPVehicleclass_t OCTET_STRING)
	uint8_t type = static_cast<uint8_t>(((ps_srm->signalRequest_element.NTCIPVehicleclass.NTCIPvehicleClass_type << 4) 
		+ (ps_srm->signalRequest_element.NTCIPVehicleclass.NTCIPvehicleClass_level)) & 0xFF);
	OCTET_STRING_fromBuf(&(psrm->request.type),(char*)&type,1);
	//	request::*codeWord
	psrm->request.codeWord = OCTET_STRING_new_fromBuf(&asn_DEF_OCTET_STRING,ps_srm->signalRequest_element.codeWord,(int)strlen(ps_srm->signalRequest_element.codeWord));
	if (ps_srm->signalRequest_element.requestedAction == priorityrequest_enum_t::REQUESTPRIORITY 
		|| ps_srm->signalRequest_element.requestedAction == priorityrequest_enum_t::REQUESTPREEMP)
	{
		/// including timeOfService & endOfService for priority request only, no need for cancel request
		// SignalRequestMsg::*timeOfService
		psrm->timeOfService = (DTime_t *)calloc(1,sizeof(DTime_t));
		psrm->timeOfService->hour = (DHour_t)ps_srm->timeOfService.hour;
		psrm->timeOfService->minute = (DMinute_t)ps_srm->timeOfService.min;
		psrm->timeOfService->second = (DSecond_t)ps_srm->timeOfService.sec;
		// SignalRequestMsg::*endOfService
		psrm->endOfService = (DTime_t *)calloc(1,sizeof(DTime_t));
		psrm->endOfService->hour = (DHour_t)ps_srm->endOfService.hour;
		psrm->endOfService->minute = (DMinute_t)ps_srm->endOfService.min;
		psrm->endOfService->second = (DSecond_t)ps_srm->endOfService.sec;
	}
	// SignalRequestMsg::*transitStatus 
	psrm->transitStatus = (TransitStatus_t *)calloc(1,sizeof(TransitStatus_t));
	psrm->transitStatus->buf = (uint8_t *)calloc(1,1);  
	psrm->transitStatus->size = 1;
	psrm->transitStatus->buf[0] = ps_srm->transitStatus;
	psrm->transitStatus->bits_unused = 2;
	// SignalRequestMsg::*vehicleVIN
	psrm->vehicleVIN = (VehicleIdent_t *)calloc(1,sizeof(VehicleIdent_t));
	//	vehicleVIN::*name
	psrm->vehicleVIN->name = OCTET_STRING_new_fromBuf(&asn_DEF_OCTET_STRING,ps_srm->vehIdent_element.vehName,(int)strlen(ps_srm->vehIdent_element.vehName));
	//	vehicleVIN::*vin
	psrm->vehicleVIN->vin = OCTET_STRING_new_fromBuf(&asn_DEF_OCTET_STRING,ps_srm->vehIdent_element.vehVin,(int)strlen(ps_srm->vehIdent_element.vehVin));
	//	vehicleVIN::*ownerCode
	psrm->vehicleVIN->ownerCode = OCTET_STRING_new_fromBuf(&asn_DEF_OCTET_STRING,ps_srm->vehIdent_element.vehOwnerCode,(int)strlen(ps_srm->vehIdent_element.vehOwnerCode));
	//	vehicleVIN::*id
	uint32_t vid = htonl(ps_srm->vehIdent_element.vehId);
	psrm->vehicleVIN->id = OCTET_STRING_new_fromBuf(&asn_DEF_OCTET_STRING,(char*)&vid,4);
	//	vehicleVIN::*vehicleType (VehicleType_t ENUMERATED_t INTEGER_t)
	asn_long2INTEGER(psrm->vehicleVIN->vehicleType,ps_srm->vehIdent_element.vehType);
	//	vehicleVIN::*vehicleClass
	// SignalRequestMsg::vehicleData
	OCTET_STRING_fromBuf(&(psrm->vehicleData),ps_srm->BSMblob,BSMBLOB1SIZE);
	// SignalRequestMsg::*status
	psrm->status = OCTET_STRING_new_fromBuf(&asn_DEF_OCTET_STRING,(char*)&(ps_srm->requestStatus),1);	
	
	// encode SRM
	rval = der_encode_to_buffer(&asn_DEF_SignalRequestMsg, psrm, ptr, size);
	
	// free psrm
	SEQUENCE_free(&asn_DEF_SignalRequestMsg, psrm, 0);
	
	if (withHeader)
	{
		msg_enum_t::MSGTYPE msgtype = msg_enum_t::SRM_PKT;	
		string str = wmeUtils::fillAradaUdp(msgtype,ptr,rval.encoded);
		memcpy(ptr,str.c_str(),str.size() + 1);
		return (str.size() + 1);
	}
	else
	{
		return (rval.encoded);	
	}
}

ssize_t AsnJ2735Lib::encode_art_payload(const ART_element_t* ps_art,char* ptr,const size_t size,bool withHeader) const
{
	ssize_t offset = 0;
	ptr[offset] = wmeUtils::DSRCmsgID_ART;		//msgID
	offset++;
	ptr[offset] = ps_art->msgCnt;
	offset++;
	memcpy(ptr+offset,(void*)&(ps_art->id),4);
	offset += 4;
	ptr[offset] = ps_art->status;
	offset++;
	memcpy(ptr+offset,(void*)&(ps_art->ms),2);
	offset += 2;
	memcpy(ptr+offset,(void*)&(ps_art->priorityCause),4);
	offset += 4;
	ptr[offset] = ps_art->requestNums;
	offset++;
	if (sizeof(ps_art->requestNums) * ps_art->requestNums + offset >= size)
		return (-1);
	for (uint8_t i=0; i< ps_art->requestNums; i++)
	{
		memcpy(ptr+offset,(void*)&(ps_art->request[i].id),4);
		offset += 4;
		ptr[offset] = ps_art->request[i].inLaneId;
		offset++;
		ptr[offset] = ps_art->request[i].outLaneId;
		offset++;
		ptr[offset] = ps_art->request[i].contolPhase;
		offset++;		
		ptr[offset] = ps_art->request[i].classType;
		offset++;
		ptr[offset] = ps_art->request[i].classLevel;
		offset++;
		ptr[offset] = ps_art->request[i].requestStatus;
		offset++;		
		ptr[offset] = ps_art->request[i].transitStatus;
		offset++;
		ptr[offset] = ps_art->request[i].timeOfService.hour;
		offset++;
		ptr[offset] = ps_art->request[i].timeOfService.min;
		offset++;
		memcpy(ptr+offset,(void*)&(ps_art->request[i].timeOfService.sec),2);
		offset += 2;
		ptr[offset] = ps_art->request[i].endOfService.hour;
		offset++;
		ptr[offset] = ps_art->request[i].endOfService.min;
		offset++;
		memcpy(ptr+offset,(void*)&(ps_art->request[i].endOfService.sec),2);
		offset += 2;
		memcpy(ptr+offset,(void*)&(ps_art->request[i].etaOffset),2);
		offset += 2;
	}
	if (withHeader)
	{
		msg_enum_t::MSGTYPE msgtype = msg_enum_t::ART_PKT;	
		string str = wmeUtils::fillAradaUdp(msgtype,ptr,static_cast<size_t>(offset));
		memcpy(ptr,str.c_str(),str.size() + 1);
		return (str.size() + 1);
	}
	else
	{
		return (offset);	
	}
}

bool AsnJ2735Lib::decode_srm_payload(const char* ptr,const size_t size,SRM_element_t* ps_srm,FILE *fp) const
{
	asn_dec_rval_t rval; 	
	SignalRequestMsg_t* psrm = 0;
	memset(ps_srm,0,sizeof(ps_srm));
	ps_srm->reset();
	
	rval = ber_decode(0, &asn_DEF_SignalRequestMsg,(void **)&psrm, ptr, size);
	if (rval.code != RC_OK)
	{
		SEQUENCE_free(&asn_DEF_SignalRequestMsg, psrm, 0);
		return false;
	}
	
	if (fp)
	{
		xer_fprint(fp, &asn_DEF_SignalRequestMsg, psrm);	
	}
		
	// SignalRequestMsg::msgID (INTEGER_t) (size 1)
	if (psrm->msgID.buf[0] != DSRCmsgID_signalRequestMessage)
	{
		SEQUENCE_free(&asn_DEF_SignalRequestMsg, psrm, 0);
		return false;
	}
	// SignalRequestMsg::msgCnt (long) (size 1)
	ps_srm->msgCnt = static_cast<uint8_t>(psrm->msgCnt);
	// SignalRequestMsg::request::id
	ps_srm->signalRequest_element.id = uchar2ulong((u_char *)(psrm->request.id.buf),0,psrm->request.id.size);
	//	request::*requestedAction OCTET_STRING_t	
	ps_srm->signalRequest_element.requestedAction = priorityrequest_enum_t::UNKNOWREQUEST;
	if (psrm->request.isCancel)
	{
		ps_srm->signalRequest_element.requestedAction = uchar2ulong((u_char *)(psrm->request.isCancel->buf),0,psrm->request.isCancel->size);
	}
	else if (psrm->request.requestedAction)
	{
		ps_srm->signalRequest_element.requestedAction = uchar2ulong((u_char *)(psrm->request.requestedAction->buf),0,psrm->request.requestedAction->size);
	}
	//	request::*inLane 	
	ps_srm->signalRequest_element.inLaneId = 0;
	if (psrm->request.inLane)
	{
		ps_srm->signalRequest_element.inLaneId = uchar2ulong((u_char *)(psrm->request.inLane->buf),0,psrm->request.inLane->size);
	}
	//	request::*outLane 	
	ps_srm->signalRequest_element.outLaneId = 0;
	if (psrm->request.outLane)
	{
		ps_srm->signalRequest_element.outLaneId = uchar2ulong((u_char *)(psrm->request.outLane->buf),0,psrm->request.outLane->size);
	}
	//	request::type (NTCIPVehicleclass_t OCTET_STRING)	
	uint8_t vehClass = uchar2ulong((u_char *)(psrm->request.type.buf),0,psrm->request.type.size);
	ps_srm->signalRequest_element.NTCIPVehicleclass.NTCIPvehicleClass_type = ((vehClass >> 4) & 0x0F);
	ps_srm->signalRequest_element.NTCIPVehicleclass.NTCIPvehicleClass_level = (vehClass & 0x0F);
	//	request::*codeWord
	memset(ps_srm->signalRequest_element.codeWord,'\0',sizeof(ps_srm->signalRequest_element.codeWord));
	if (psrm->request.codeWord)
	{
		memcpy(ps_srm->signalRequest_element.codeWord,(char*)psrm->request.codeWord->buf,psrm->request.codeWord->size);
	}
	// SignalRequestMsg::*timeOfService
	memset(&(ps_srm->timeOfService),0,sizeof(ps_srm->timeOfService));
	if (psrm->timeOfService)
	{
		ps_srm->timeOfService.hour = static_cast<uint8_t>(psrm->timeOfService->hour);
		ps_srm->timeOfService.min = static_cast<uint8_t>(psrm->timeOfService->minute);
		ps_srm->timeOfService.sec = static_cast<uint16_t>(psrm->timeOfService->second);
	}
	// SignalRequestMsg::*endOfService
	memset(&(ps_srm->endOfService),0,sizeof(ps_srm->endOfService));
	if (psrm->endOfService)
	{
		ps_srm->endOfService.hour = static_cast<uint8_t>(psrm->endOfService->hour);
		ps_srm->endOfService.min = static_cast<uint8_t>(psrm->endOfService->minute);
		ps_srm->endOfService.sec = static_cast<uint16_t>(psrm->endOfService->second);
	}
	// SignalRequestMsg::*transitStatus 
	ps_srm->transitStatus = 0;
	if (psrm->transitStatus)
	{
		ps_srm->transitStatus = psrm->transitStatus->buf[0];
	}
	// SignalRequestMsg::*vehicleVIN
	memset(ps_srm->vehIdent_element.vehName,'\0',sizeof(ps_srm->vehIdent_element.vehName));
	memset(ps_srm->vehIdent_element.vehVin,'\0',sizeof(ps_srm->vehIdent_element.vehVin));
	memset(ps_srm->vehIdent_element.vehOwnerCode,'\0',sizeof(ps_srm->vehIdent_element.vehOwnerCode));
	ps_srm->vehIdent_element.vehId = 0;
	ps_srm->vehIdent_element.vehType = 0;
	if (psrm->vehicleVIN)
	{	
		//	vehicleVIN::*name
		if (psrm->vehicleVIN->name)
		{
			memcpy(ps_srm->vehIdent_element.vehName,psrm->vehicleVIN->name->buf,psrm->vehicleVIN->name->size);
		}
		//	vehicleVIN::*vin
		if (psrm->vehicleVIN->vin)
		{
			memcpy(ps_srm->vehIdent_element.vehVin,psrm->vehicleVIN->vin->buf,psrm->vehicleVIN->vin->size);
		}
		//	vehicleVIN::*ownerCode
		if (psrm->vehicleVIN->ownerCode)
		{
			memcpy(ps_srm->vehIdent_element.vehOwnerCode,psrm->vehicleVIN->ownerCode->buf,psrm->vehicleVIN->ownerCode->size);
		}		
		//	vehicleVIN::*id
		if (psrm->vehicleVIN->id)
		{
			ps_srm->vehIdent_element.vehId = uchar2ulong((u_char *)(psrm->vehicleVIN->id->buf),0,psrm->vehicleVIN->id->size);
		}
		//	vehicleVIN::*vehicleType (VehicleType_t ENUMERATED_t INTEGER_t)
		if (psrm->vehicleVIN->vehicleType)
		{
			ps_srm->vehIdent_element.vehType = psrm->vehicleVIN->vehicleType->buf[0];
		}
		//	vehicleVIN::*vehicleClass
	}
	// SignalRequestMsg::vehicleData
	memcpy(ps_srm->BSMblob,(char*)psrm->vehicleData.buf,BSMBLOB1SIZE);
	// SignalRequestMsg::*status
	ps_srm->requestStatus = 0;
	if (psrm->status)
	{
		ps_srm->requestStatus = psrm->status->buf[0];
	}
		
	// free psrm
	SEQUENCE_free(&asn_DEF_SignalRequestMsg, psrm, 0);
	return true;
}

bool AsnJ2735Lib::decode_art_payload(const char* ptr,const size_t size,ART_element_t* ps_art) const
{
	memset(ps_art,0,sizeof(ART_element_t));
	size_t offset = 0;	
	if (ptr[offset] != (char)wmeUtils::DSRCmsgID_ART)
		return false;
	offset++;
	ps_art->msgCnt = ptr[offset];
	offset++;
	memcpy(&(ps_art->id),ptr+offset,4);
	offset += 4;
	ps_art->status = ptr[offset];
	offset++;
	memcpy(&(ps_art->ms),ptr+offset,2);
	offset += 2;
	memcpy(&(ps_art->priorityCause),ptr+offset,4);
	offset += 4;	
	ps_art->requestNums = ptr[offset];
	offset++;
	if (ps_art->requestNums * sizeof(ART_request_t) + offset >= size)
		return false;
	for (uint8_t i = 0; i < ps_art->requestNums; i++)
	{
		memcpy(&(ps_art->request[i].id),ptr+offset,4);
		offset += 4;
		ps_art->request[i].inLaneId = ptr[offset];
		offset++;
		ps_art->request[i].outLaneId = ptr[offset];
		offset++;
		ps_art->request[i].contolPhase = ptr[offset];
		offset++;		
		ps_art->request[i].classType = ptr[offset];
		offset++;
		ps_art->request[i].classLevel = ptr[offset];
		offset++;
		ps_art->request[i].requestStatus = ptr[offset];
		offset++;		
		ps_art->request[i].transitStatus = ptr[offset];
		offset++;
		ps_art->request[i].timeOfService.hour = ptr[offset];
		offset++;
		ps_art->request[i].timeOfService.min = ptr[offset];
		offset++;
		memcpy(&(ps_art->request[i].timeOfService.sec),ptr+offset,2);
		offset += 2;
		ps_art->request[i].endOfService.hour = ptr[offset];
		offset++;
		ps_art->request[i].endOfService.min = ptr[offset];
		offset++;
		memcpy(&(ps_art->request[i].endOfService.sec),ptr+offset,2);
		offset += 2;
		memcpy(&(ps_art->request[i].etaOffset),ptr+offset,2);
		offset += 2;
	}
	return true;
}

void AsnJ2735Lib::mapPayloadHex_fprintf(FILE *fp) const
{
	for (size_t i = 0; i< mpMapData->mpIntersection.size(); i++)
	{
		mapPayloadHex_fprintf(mpMapData->mpIntersection[i].id,fp);
	}
}

void AsnJ2735Lib::mapPayloadHex_fprintf(const uint32_t intersectionId,FILE* fp) const
{
	int index = mpMapData->getIndexByIntersectionId(intersectionId);
	fprintf(fp,"MAP_Name\t%s :: %zu bytes\n",mpMapData->mpIntersection[index].name.c_str(),
		mpMapData->mpIntersection[index].mapPayload.size());
	payloadHex_fprintf(mpMapData->mpIntersection[index].mapPayload.c_str(),
		mpMapData->mpIntersection[index].mapPayload.size(),fp);
}

void AsnJ2735Lib::payloadHex_fprintf(const char *ptr,size_t size,FILE* fp) const
{
	for (size_t i = 0; i<size; i++)
	{
		fprintf(fp,"%02x", (ptr[i] & 0xff));
	}
	fprintf(fp,"\n");
}

