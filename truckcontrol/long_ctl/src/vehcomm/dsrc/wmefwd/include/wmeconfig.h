#ifndef _MMITSSWMECONFIG_H
#define _MMITSSWMECONFIG_H

#include <string>
#include <cstring>
#include <map>
#include <vector>
#include <stdint.h>		// c++11 <cstdint>

#include "socketUtils.h"
#include "wmeUtils.h"

namespace WMEconfig
{		
	struct appConfigEnum_t
	{
		enum direction{TX,RX,TX_RX};
		enum socketHandle{LISTEN,SEND};
		enum wmeRegisteType{USER,PROVIDER};
	};
	struct appConfig_t
	{
		bool isEnabled;
    wmeUtils::channelmode_enum_t::channelInterface e_iface;    
    appConfigEnum_t::wmeRegisteType e_registerType;
		appConfigEnum_t::direction		e_direction;
		wmeUtils::channelmode_enum_t::channelMode	e_channelMode;
		std::string s_name;
		std::string s_InterfaceName;
		std::string s_psc;
		uint32_t 		psid;
		int					channel;
		int 				priority;
		int					tx_length;
	};
	struct appHandler_t
	{
		std::vector<WMEconfig::appConfig_t> applications;
		std::map<uint32_t,size_t> appPSIDmap;	      /// key: psid, value: index in applications
    std::string s_logFilePath;
		socketUtils::socketAddress_t*	mpListenAddr;
		socketUtils::socketAddress_t*	mpSendAddr;
	};
	
	class configWme
	{
		private:			
			bool configSucceed;
			WMEconfig::appHandler_t appHandler;
			
			bool readWmefwdConfigFile(const char* ptr);
			bool parseAppConfig(const std::string s,WMEconfig::appConfig_t* pAppConfig);
		public:
			configWme(const char* ptr);
			~configWme(void);
						
			bool isConfigSucceed(void) const;
			WMEconfig::appHandler_t getAppHandler(void) const;
	};
};

#endif

