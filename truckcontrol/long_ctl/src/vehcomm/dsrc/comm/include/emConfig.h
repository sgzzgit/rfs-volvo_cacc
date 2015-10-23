#ifndef _MMITSSEMCONFIG_H
#define _MMITSSEMCONFIG_H

#include <string>
#include <cstring>
#include <stdint.h>	// c++11 <cstdint>

// forward declaration
namespace socketUtils
{
	struct socketAddress_t;
};

namespace EMconfig
{		
	struct SocketConfig
	{
		std::string s_nmapFile;
		std::string s_logPath;
		int					i_dsrcTimeoutSec;		
		std::string s_wmeListenSocket;		   
		std::string s_wmeSendSocket;         
		std::string s_gpsfixListenSocket;    
		std::string s_awareSendSocket;       
	};
	
	struct VinConfig
	{
		bool isPrioEligible;
    uint32_t    vehId;
    double      vehLen;
    double      vehWidth;
		std::string codeWord;		// (1..16)
		std::string name;				// (1..63)
		std::string vin;				// (1..17)
		std::string ownerCode;	// (1..32)
		uint8_t vehType;				// (4 bits)(0..15) 
		uint8_t prioLevel;			// (4 bits)(0..15)
	};
		
	struct socketHandleEnum_t
	{
		enum socketHandle{WMELISTEN,WMESEND,GPSFIXLISTEN,AWARESEND};
	};

	class configEm
	{
		private:			
			bool configSucceed;
			SocketConfig 	mSocketConfig;
			VinConfig			mVinConfig;
			socketUtils::socketAddress_t*	mpWMElistenAddr;
			socketUtils::socketAddress_t*	mpWMEsendAddr;
			socketUtils::socketAddress_t*	mpGPSFIXlistenAddr;
			socketUtils::socketAddress_t*	mpAWAREsendAddr;
			
			bool readSocketConfigFile(const char* ptr1);
			bool readVinConfigFile(const char* ptr2);
			bool seupSocketAddress(void);
			
		public:
			configEm(const char* ptr1,const char* ptr2);
			~configEm(void);
						
			bool isConfigSucceed(void) const;
			std::string getNmapFileName(void) const;
			std::string getLogPath(void) const;
			int getDsrcTimeOut(void) const;						
			struct VinConfig getVinConfig(void) const;
			socketUtils::socketAddress_t getSocketAddr(const socketHandleEnum_t::socketHandle type) const;
	};
};

#endif

