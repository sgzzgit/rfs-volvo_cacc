#include <iostream>
#include <sstream>
#include <iomanip> 			// setw, setfill
#include <arpa/inet.h>	// htonl, htons, ntohl & ntohs, may need <netinet/in.h>

#include "wmeUtils.h"

using namespace std;

size_t wmeUtils::decodewsm(wmeUtils::wsmp_t *pwsmp,const uint8_t* ptr,const size_t size)
{
  ///  ptr in network byte order
	size_t offset = 0;
	
	if (!pwsmp)
		return (0);
		
	memset(pwsmp,0,sizeof(wmeUtils::wsmp_t));
	
	pwsmp->version = ptr[offset];
	++offset;
	// wsmp::psid (size 1..4)
	if ((ptr[offset] & 0xF0) == 0xE0)
	{
		// 4 bytes
		pwsmp->psid = (ptr[offset] << 18) + (ptr[offset+1] << 12) + (ptr[offset+2] << 6) + ptr[offset+3] - 0x3C82080;
		offset += 4;
	}
	else if ((ptr[offset] & 0xE0) == 0xC0)
	{
		// 3 bytes
		pwsmp->psid = (ptr[offset] << 12) + (ptr[offset+1] << 6) + ptr[offset+2] - 0xE2080;
		offset += 3;
	}
	else if ((ptr[offset] & 0xC0) == 0x80)
	{
		// 2 bytes
		pwsmp->psid = (ptr[offset] << 6) + ptr[offset+1] - 0x3080;
		offset += 2;
	}
	else if ((ptr[offset] & 0x80) == 0x00)
	{
		// 1 byte
		pwsmp->psid = ptr[offset];
		++offset;
	}
	else
		return 0;
	
	// find WSMP waveid
	while(1)
	{
		switch(ptr[offset])
		{
		case wmeUtils::WSMPWAVEID:			
			pwsmp->waveId = ptr[offset];
			break;
		case wmeUtils::CHANNUM:
			offset += 2; // skip elementLen
			pwsmp->channel = ptr[offset];
			break;
		case wmeUtils::DATARATE:
			offset += 2; // skip elementLen
			pwsmp->rate = ptr[offset];
			break;
		case wmeUtils::TRANSMITPW:
			offset += 2; // skip elementLen
			pwsmp->txpower = ptr[offset];
			break;
		default:
			;
		}
		++offset;
		if (pwsmp->waveId == wmeUtils::WSMPWAVEID || offset > size || offset > 20)
			break;
	}
	if (pwsmp->waveId != wmeUtils::WSMPWAVEID)
		return 0;
	pwsmp->txlength = static_cast<uint16_t>( ((ptr[offset] << 8) + ptr[offset+1]) & 0xFFFF );
	offset += 2;
	return offset;
}

int wmeUtils::encodewsm(const wmeUtils::wsmp_t *pwsmp,uint8_t* ptr,const int size)
{
	if (size < WSMPMAXLEN)
		return 0;
		
	int offset = 0;
	ptr[offset] = pwsmp->version;
	++offset;
	if (pwsmp->psid < 0x80)
	{
    /// 1 byte
		ptr[offset] = static_cast<uint8_t>(pwsmp->psid);
		++offset;
	}
	else if (pwsmp->psid <= 0x7FF)
	{
    /// 2 bytes
		ptr[offset] = static_cast<uint8_t>((pwsmp->psid >> 6) + 0xC0);
		ptr[offset+1] = static_cast<uint8_t>((pwsmp->psid & 0x3F) + 0x80);
		offset += 2;
	}
  else if (pwsmp->psid <= 0xFFFF) 
	{
    /// 3 bytes
    ptr[offset] = static_cast<uint8_t>((pwsmp->psid >> 12) + 0xE0);
    ptr[offset+1] = static_cast<uint8_t>(((pwsmp->psid >> 6) & 0x3F) + 0x80);
    ptr[offset+2] = static_cast<uint8_t>((pwsmp->psid & 0x3F) + 0x80);
		offset += 3;
 	}
	else if (pwsmp->psid <= 0x10FFFF)
	{
    /// 4 bytes
    ptr[offset] = static_cast<uint8_t>((pwsmp->psid >> 18) + 0xF0);
    ptr[offset+1] = static_cast<uint8_t>(((pwsmp->psid >> 12) & 0x3F) + 0x80);
    ptr[offset+2] = static_cast<uint8_t>(((pwsmp->psid >> 6) & 0x3F) + 0x80);
    ptr[offset+3] = static_cast<uint8_t>((pwsmp->psid & 0x3F) + 0x80);
		offset += 4;
	}
	else
		return 0;
	
	ptr[offset] = CHANNUM;
	++offset;
	ptr[offset] = 1;
	++offset;	
	ptr[offset] = pwsmp->channel;
	++offset;	
	ptr[offset] = DATARATE;
	++offset;	
	ptr[offset] = 1;
	++offset;	
	ptr[offset] = pwsmp->rate;
	++offset;	
	ptr[offset] = TRANSMITPW;
	++offset;	
	ptr[offset] = 1;
	++offset;	
	ptr[offset] = pwsmp->txpower;
	++offset;	
	ptr[offset] = pwsmp->waveId;
	++offset;
	ptr[offset] = static_cast<uint8_t>((pwsmp->txlength >> 8) & 0xFF);
	ptr[offset+1] = static_cast<uint8_t> (pwsmp->txlength & 0xFF);
	return (offset+2);
}

void wmeUtils::getwsmheaderinfo(const uint8_t* ptr,const size_t size,size_t& offset,size_t& payloadLen)
{
	wmeUtils::wsmp_t wsmp;
	payloadLen = 0;
	
	offset = wmeUtils::decodewsm(&wsmp,ptr,size);
	if (offset > 0)
		payloadLen = wsmp.txlength;
}

std::string wmeUtils::fillAradaUdp(msg_enum_t::MSGTYPE type,const char* ptr,const size_t size)
{
	// for working with Arada OBU/RSU immediate UDP forwarding.  
	// type should be one of the broadcast messages: 
	//	SPaT, MAP and ART on RSU and BSM & SRM on OBU
	wmeUtils::arada_udp_header_t udpHeader;
	map<msg_enum_t::MSGTYPE,wmeUtils::arada_udp_header_t>::const_iterator it = wmeUtils::aradaUdpHeaderMap.find(type);
	if (it == wmeUtils::aradaUdpHeaderMap.end())
	{
		cerr << "Can't find entry in aradaUdpHeaderMap for message type " << MSGNAME[type] << ", aradaUdpHeaderMap size = " << wmeUtils::aradaUdpHeaderMap.size() << endl;
		return std::string();
	}
	
	udpHeader = it->second;
	ostringstream oss;
	oss << "Version=" << static_cast<int>(udpHeader.version) << endl;
	oss << "Type=" << udpHeader.name << endl;
	oss << "PSID=" << "0x" << hex << uppercase << udpHeader.psid << nouppercase << dec << endl;
	oss << "Priority=" << static_cast<int>(udpHeader.priority) << endl;
	if (udpHeader.txMode == wmeUtils::channelmode_enum_t::CONTINUOUS)
		oss << "TxMode=CONT" << endl;
	else
		oss << "TxMode=ALT" << endl;	
	oss << "TxChannel=" << static_cast<int>(udpHeader.txChannel) << endl;
	oss << "TxInterval=" << static_cast<int>(udpHeader.txInterval) << endl;	
	oss << "DeliveryStart=" << endl;
	oss << "DeliveryStop=" << endl;
	if (udpHeader.signature)
		oss << "Signature=True" << endl;
	else	
		oss << "Signature=False" << endl;
	if (udpHeader.encryption)
		oss << "Encryption=True" << endl;
	else
		oss << "Encryption=False" << endl;
	oss << "Payload=" << hex;
	for (size_t i=0; i<size;i++)
	{
		oss << uppercase << setw(2) << setfill('0') << static_cast<unsigned int>((unsigned char)ptr[i]);
	}
  oss << dec << endl;
	return (oss.str());
}	

