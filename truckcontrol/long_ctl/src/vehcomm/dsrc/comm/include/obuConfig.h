#ifndef _MMITSSOBUCONFIG_H
#define _MMITSSOBUCONFIG_H

#include <string>
#include <cstring>
#include <stdint.h>		// c++11 <cstdint>

// forward declaration
namespace socketUtils
{
	struct socketAddress_t;
};

namespace OBUconfig
{		
	struct SocketConfig
	{
		std::string s_nmapFile;
		std::string s_logPath;
		int					i_logInterval;
		int					i_dsrcTimeoutSec;		
		std::string s_wmeListenSocket;		      // from wmefwd
		std::string s_wmeSendSocket;            // to wmefwd
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
		enum socketHandle{WMELISTEN,WMESEND};
	};

	class configObu
	{
		private:			
			bool configSucceed;
			SocketConfig 	mSocketConfig;
			VinConfig			mVinConfig;
			socketUtils::socketAddress_t*	mpWMElistenAddr;
			socketUtils::socketAddress_t*	mpWMEsendAddr;

			bool readSocketConfigFile(const char* ptr1);
			bool readVinConfigFile(const char* ptr2);
			bool seupSocketAddress(void);
			
		public:
			configObu(const char* ptr1,const char* ptr2);
			~configObu(void);
						
			bool isConfigSucceed(void) const;
			std::string getNmapFileName(void) const;
			std::string getLogPath(void) const;
			int getLogInterval(void) const;
			int getDsrcTimeOut(void) const;			
			struct VinConfig getVinConfig(void) const;
			socketUtils::socketAddress_t getSocketAddr(const socketHandleEnum_t::socketHandle type) const;
	};
};

#endif

