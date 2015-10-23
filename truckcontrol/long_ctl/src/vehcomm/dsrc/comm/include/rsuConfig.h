#ifndef _MMITSSRSUCONFIG_H
#define _MMITSSRSUCONFIG_H

#include <string>
#include <cstring>
#include <stdint.h>		// c++11 <cstdint>

// forward declaration
namespace socketUtils
{
	struct socketAddress_t;
};

namespace RSUconfig
{		
	struct SocketConfig
	{
		std::string s_nmapFile;
		std::string s_logPath;
		int					i_logInterval;							// interval of log file, in minutes
		int					i_dsrcTimeoutSec;						// interval of timeout for dsrc messages, in seconds
		int					i_maxTime2goPhaseCall;			// maximum travel time to stop-bar for allowing vehicular phase call, in seconds
		int					i_maxTime2changePhaseExt;		// maximum green phase time2change for considering vehicular extension requests, in seconds
		int					i_maxTime2phaseExt;					// maximum time a green phase can be extended, in seconds
		int					i_maxGreenExtenstion;				// maximum time a priority phase can be extended, in seconds
		int 				i_phaseExtEvery;						// soft-call frequency sending to the controller, in deciseconds
		int					i_syncPhase;
		std::string s_coordinatedPhases;
		std::string s_wmeListenSocket;		      // from wmefdw
		std::string s_wmeSendSocket;			      // to wmefwd
		std::string s_cloudListenSocket;				// from pedestrian cloud server
		std::string s_cloudSendSocket;					// to pedestrian cloud server
		std::string s_tciListenSocket;					// from traffic controller interface
		std::string s_tciSendSocket;						// to traffic controller interface
	};
			
	struct socketHandleEnum_t
	{
		enum socketHandle{WMELISTEN,WMESEND,CLOUDLISTEN,CLOUDSEND,TCILISTEN,TCISEND};
	};

	class configRsu
	{
		private:			
			bool configSucceed;
			SocketConfig 	mSocketConfig;
			socketUtils::socketAddress_t*	mpWMElistenAddr;			
			socketUtils::socketAddress_t*	mpWMEsendAddr;			
			socketUtils::socketAddress_t*	mpCLOUDlistenAddr;
			socketUtils::socketAddress_t*	mpCLOUDsendAddr;
			socketUtils::socketAddress_t*	mpTCIlistenAddr;
			socketUtils::socketAddress_t*	mpTCIsendAddr;
			
			bool readSocketConfigFile(const char* ptr);
			bool seupSocketAddress(void);
			
		public:
			configRsu(const char* ptr);
			~configRsu(void);
						
			bool isConfigSucceed(void) const;
			std::string getNmapFileName(void) const;
			std::string getLogPath(void) const;
			int getLogInterval(void) const;			
			int getDsrcTimeOut(void) const;
			int getMaxTime2goPhaseCall(void) const;
			int getMaxTime2changePhaseExt(void) const;
			int getMaxTime2phaseExt(void) const;
			int getMaxGreenExtenstion(void) const;
			int getPhaseExtEvery(void) const;
			int getSyncPhase(void) const;
			std::string getCoordinatedPhases(void) const;
			socketUtils::socketAddress_t getSocketAddr(const socketHandleEnum_t::socketHandle type) const;
	};
};

#endif

