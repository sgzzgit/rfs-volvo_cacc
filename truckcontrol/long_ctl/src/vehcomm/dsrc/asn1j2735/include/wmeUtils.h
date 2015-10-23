#ifndef _WME_UITLS_H
#define _WME_UITLS_H

#include <stdint.h>		// c++11 <cstdint>
#include <string>
#include <cstring>
#include <map>

#include "msgenum.h"

namespace wmeUtils
{
	struct channelmode_enum_t
	{
		enum channelMode{ALTERNATING,CONTINUOUS};
    enum channelInterface{ATH0_ = 0,ATH1_= 1};
	};
	
	/// DSRC message Id for ART (not in asn1)
	static const uint8_t DSRCmsgID_ART        = 20;
  /// WME interface
  static const char *iface[]                = {"ath0","ath1"};
  /// WME channel
  static const uint8_t wmechannel[2]        = {182,172};
	/// WME PSID
	static const uint32_t WSM_PSID_BSM        =	0x20;
	static const uint32_t WSM_PSID_SPAT       =	0xBFE0;
	static const uint32_t WSM_PSID_MAP        = 0xBFF0;
	static const uint32_t WSM_PSID_SRM        = 0x40;
	static const uint32_t WSM_PSID_ART        = 0x80;
  /// WME priority
  static const uint8_t txPriority           = 2;
	/// WSMP header tag
	static const uint8_t WSMPWAVEID	          =	0x80;
	static const uint8_t TRANSMITPW           = 0x04;
	static const uint8_t CHANNUM              = 0x0F;
	static const uint8_t DATARATE             = 0x10;
	static const uint8_t WSMPMAXLEN	          = 0x11;
	/// MMITSS header msgid
  static const uint16_t msg_header          = 0xFFFF;
	static const uint8_t msgid_bsm            = 0x40;
	static const uint8_t msgid_spat           = 0x41;
	static const uint8_t msgid_map            = 0x42;
	static const uint8_t msgid_srm            = 0x43;
	static const uint8_t msgid_art            = 0x44;
  static const uint8_t msgid_softcall       = 0x45;
  static const uint8_t msgid_spat_internal  = 0x46;
  
  static const char *MSGNAME[] = {"Unknown","BSM","SPaT","MAP","SRM","ART"};
  
	/// WSMP header
	struct wsmp_t
	{
		uint8_t		version;
		uint32_t	psid;
		uint8_t		channel;
		uint8_t		rate;
		uint8_t		txpower;
		uint8_t		waveId;
		uint16_t	txlength;
	}__attribute__ ((packed));
		
	/// Arada WSMP UDP header
	struct arada_udp_header_t
	{
		uint8_t 		version;
		std::string	name;
    std::string iface;
		uint32_t 		psid;
		uint8_t			txChannel;
		uint8_t			priority;	
		wmeUtils::channelmode_enum_t::channelMode	txMode;
		uint8_t			txInterval;
		bool				signature;
		bool				encryption;
	};
	
	/// Savari WSMP UDP header
	struct savari_udap_header_t 
	{
		uint8_t		type;
		uint32_t 	len;
		uint32_t 	seconds;
		uint16_t 	msecs;
		uint32_t 	intersectionID;
	}__attribute__ ((packed));
	
	/// MMITSS message header header
	struct mmitss_udp_header_t
	{
		uint16_t	msgheader;						
		uint8_t		msgid;
		uint32_t 	ms_since_midnight;
	}__attribute__ ((packed));
  
  struct wme_channel_config_t
  {
    wmeUtils::channelmode_enum_t::channelInterface txIface;
    std::string iface;
    uint32_t 		psid;
    uint8_t     channel;
    wmeUtils::channelmode_enum_t::channelMode	txMode;
		uint8_t			priority;	    
  };
  
	inline std::map<msg_enum_t::MSGTYPE,wmeUtils::arada_udp_header_t> creat_arada_header_map(void)
	{
		std::map<msg_enum_t::MSGTYPE,wmeUtils::arada_udp_header_t> m;
		msg_enum_t::MSGTYPE msgtype;
		
		msgtype = msg_enum_t::BSM_PKT;
    wmeUtils::arada_udp_header_t bsmUdpHeader = {2,std::string(wmeUtils::MSGNAME[msgtype]),std::string(wmeUtils::iface[1]),WSM_PSID_BSM,wmeUtils::wmechannel[1],
      txPriority,wmeUtils::channelmode_enum_t::CONTINUOUS,0,false,false};
		m[msgtype] = bsmUdpHeader;
		msgtype = msg_enum_t::MAP_PKT;
    wmeUtils::arada_udp_header_t mapUdpHeader = {2,std::string(wmeUtils::MSGNAME[msgtype]),std::string(wmeUtils::iface[1]),WSM_PSID_MAP,wmeUtils::wmechannel[1],
      txPriority,wmeUtils::channelmode_enum_t::CONTINUOUS,0,false,false};
		m[msgtype] = mapUdpHeader;
		msgtype = msg_enum_t::SPAT_PKT;
    wmeUtils::arada_udp_header_t spatUdpHeader = {2,std::string(wmeUtils::MSGNAME[msgtype]),std::string(wmeUtils::iface[1]),WSM_PSID_SPAT,wmeUtils::wmechannel[1],
      txPriority,wmeUtils::channelmode_enum_t::CONTINUOUS,0,false,false};
		m[msgtype] = spatUdpHeader;
		msgtype = msg_enum_t::SRM_PKT;	
    wmeUtils::arada_udp_header_t srmUdpHeader = {2,std::string(wmeUtils::MSGNAME[msgtype]),std::string(wmeUtils::iface[0]),WSM_PSID_SRM,wmeUtils::wmechannel[0],
      txPriority,wmeUtils::channelmode_enum_t::CONTINUOUS,0,false,false};
		m[msgtype] = srmUdpHeader;
		msgtype = msg_enum_t::ART_PKT;		
    arada_udp_header_t artUdpHeader = {2,std::string(wmeUtils::MSGNAME[msgtype]),std::string(wmeUtils::iface[0]),WSM_PSID_ART,wmeUtils::wmechannel[0],
      txPriority,wmeUtils::channelmode_enum_t::CONTINUOUS,0,false,false};
		m[msgtype] = artUdpHeader;
		return m;
	};	
	static const std::map<msg_enum_t::MSGTYPE,wmeUtils::arada_udp_header_t> aradaUdpHeaderMap = wmeUtils::creat_arada_header_map();
	
	inline std::map<uint8_t,uint32_t> creat_msgid2psid_map(void)
	{
		std::map<uint8_t,uint32_t> m;
		m[msgid_bsm] = WSM_PSID_BSM;
		m[msgid_srm] = WSM_PSID_SRM;
		m[msgid_spat] = WSM_PSID_SPAT;
		m[msgid_map] = WSM_PSID_MAP;
		m[msgid_art] = WSM_PSID_ART;
		return m;	
	};
	static const std::map<uint8_t,uint32_t> mmitssMsgid2psidMap = wmeUtils::creat_msgid2psid_map();
	
  inline std::map<std::string,wmeUtils::wme_channel_config_t> creat_app2wmechannel_map(void)
  { 
    std::map<std::string,wmeUtils::wme_channel_config_t> m;
		msg_enum_t::MSGTYPE msgtype;
    
		msgtype = msg_enum_t::BSM_PKT;
    wmeUtils::wme_channel_config_t bsmChannelConfg = {wmeUtils::channelmode_enum_t::ATH1_,std::string(wmeUtils::iface[wmeUtils::channelmode_enum_t::ATH1_]),
      WSM_PSID_BSM,wmeUtils::wmechannel[wmeUtils::channelmode_enum_t::ATH1_],wmeUtils::channelmode_enum_t::CONTINUOUS,txPriority};
    m[std::string(wmeUtils::MSGNAME[msgtype])] = bsmChannelConfg;
		msgtype = msg_enum_t::SPAT_PKT;    
    wmeUtils::wme_channel_config_t spatChannelConfg = {wmeUtils::channelmode_enum_t::ATH1_,std::string(wmeUtils::iface[wmeUtils::channelmode_enum_t::ATH1_]),
      WSM_PSID_SPAT,wmeUtils::wmechannel[wmeUtils::channelmode_enum_t::ATH1_],wmeUtils::channelmode_enum_t::CONTINUOUS,txPriority};
    m[std::string(wmeUtils::MSGNAME[msgtype])] = spatChannelConfg;
		msgtype = msg_enum_t::MAP_PKT;        
    wmeUtils::wme_channel_config_t mapChannelConfg = {wmeUtils::channelmode_enum_t::ATH1_,std::string(wmeUtils::iface[wmeUtils::channelmode_enum_t::ATH1_]),
      WSM_PSID_MAP,wmeUtils::wmechannel[wmeUtils::channelmode_enum_t::ATH1_],wmeUtils::channelmode_enum_t::CONTINUOUS,txPriority};
    m[std::string(wmeUtils::MSGNAME[msgtype])] = mapChannelConfg;
		msgtype = msg_enum_t::SRM_PKT;            
    wmeUtils::wme_channel_config_t srmChannelConfg = {wmeUtils::channelmode_enum_t::ATH0_,std::string(wmeUtils::iface[wmeUtils::channelmode_enum_t::ATH0_]),
      WSM_PSID_SRM,wmeUtils::wmechannel[wmeUtils::channelmode_enum_t::ATH0_],wmeUtils::channelmode_enum_t::CONTINUOUS,txPriority};
    m[std::string(wmeUtils::MSGNAME[msgtype])] = srmChannelConfg;
		msgtype = msg_enum_t::ART_PKT;                
    wmeUtils::wme_channel_config_t artChannelConfg = {wmeUtils::channelmode_enum_t::ATH0_,std::string(wmeUtils::iface[wmeUtils::channelmode_enum_t::ATH0_]),
      WSM_PSID_ART,wmeUtils::wmechannel[wmeUtils::channelmode_enum_t::ATH0_],wmeUtils::channelmode_enum_t::CONTINUOUS,txPriority};
    m[std::string(wmeUtils::MSGNAME[msgtype])] = artChannelConfg;
    return m;
  };  
	static const std::map<std::string,wmeUtils::wme_channel_config_t> wmwAppChannelMap = wmeUtils::creat_app2wmechannel_map();
  
	size_t decodewsm(wsmp_t *pwsmp,const uint8_t* ptr,const size_t size);
	int encodewsm(const wsmp_t *pwsmp,uint8_t* ptr,const int size);	
	void getwsmheaderinfo(const uint8_t* ptr,const size_t size,size_t& offset,size_t& payloadLen);
	std::string fillAradaUdp(msg_enum_t::MSGTYPE type,const char* ptr,const size_t size);
};

#endif
