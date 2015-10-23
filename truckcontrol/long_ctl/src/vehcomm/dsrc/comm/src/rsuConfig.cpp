#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <cstring>

#include "rsuConfig.h"
#include "socketUtils.h"

using namespace std;

RSUconfig::configRsu::configRsu(const char* ptr)
{
	// default value, can be changed when readSocketConfigFile
	configSucceed = false;
	mSocketConfig.i_logInterval = 15; 		
	mSocketConfig.i_dsrcTimeoutSec = 2;		
	mSocketConfig.i_maxTime2goPhaseCall = 20;
	mSocketConfig.i_maxTime2changePhaseExt = 2;
	mSocketConfig.i_maxTime2phaseExt = 5;
	mSocketConfig.i_maxGreenExtenstion = 10;
	mSocketConfig.i_phaseExtEvery = 5;
	mSocketConfig.i_syncPhase = 2;
	mSocketConfig.s_coordinatedPhases = "00100010";
	mpWMElistenAddr 	= new socketUtils::socketAddress_t;
	mpWMEsendAddr 		= new socketUtils::socketAddress_t;
	mpCLOUDlistenAddr = new socketUtils::socketAddress_t;
	mpCLOUDsendAddr 	= new socketUtils::socketAddress_t;
	mpTCIlistenAddr		= new socketUtils::socketAddress_t;
	mpTCIsendAddr 		= new socketUtils::socketAddress_t;
  
	bool isSocketConfigSucceed = readSocketConfigFile(ptr);
	bool isSetupAddrSucceed = seupSocketAddress();
	if (!isSocketConfigSucceed || !isSetupAddrSucceed)
		configSucceed = false;
	else
		configSucceed = true;
}

RSUconfig::configRsu::~configRsu(void)
{
	delete mpWMElistenAddr;
	delete mpWMEsendAddr;
	delete mpCLOUDlistenAddr;
	delete mpCLOUDsendAddr;
	delete mpTCIlistenAddr;
	delete mpTCIsendAddr;
}

bool RSUconfig::configRsu::readSocketConfigFile(const char* ptr)
{
	ifstream IS_F(ptr);
  if (!IS_F.is_open())
	{
		cerr << "RSUconfig::readSocketConfigFile failed open: " << ptr << endl;
		return false;
	}
	istringstream iss;
	string line,s;
  while (std::getline(IS_F,line))
  {
		if (!line.empty())
		{
			if (line.find("nmapFile") == 0)
			{
				iss.str(line);
				iss >> std::skipws >> s >> s >> mSocketConfig.s_nmapFile;
				iss.clear();
			}
			else if (line.find("logPath") == 0)
			{
				iss.str(line);
				iss >> std::skipws >> s >> s >> mSocketConfig.s_logPath;
				iss.clear();
			}
			else if (line.find("logInterval") == 0)
			{
				iss.str(line);
				iss >> std::skipws >> s >> s >> mSocketConfig.i_logInterval;
				iss.clear();
			}
			else if (line.find("dsrcTimeout") == 0)
			{
				iss.str(line);
				iss >> std::skipws >> s >> s >> mSocketConfig.i_dsrcTimeoutSec;
				iss.clear();
			}
			else if (line.find("maxTime2goPhaseCall") == 0)
			{
				iss.str(line);
				iss >> std::skipws >> s >> s >> mSocketConfig.i_maxTime2goPhaseCall;
				iss.clear();
			}
			else if (line.find("maxTime2change4Ext") == 0)
			{
				iss.str(line);
				iss >> std::skipws >> s >> s >> mSocketConfig.i_maxTime2changePhaseExt;
				iss.clear();
			}
			else if (line.find("maxTime4phaseExt") == 0)
			{
				iss.str(line);
				iss >> std::skipws >> s >> s >> mSocketConfig.i_maxTime2phaseExt;
				iss.clear();
			}
			else if (line.find("softcallSendingRate") == 0)
			{
				iss.str(line);
				iss >> std::skipws >> s >> s >> mSocketConfig.i_phaseExtEvery;
				iss.clear();
			}
			else if (line.find("softcallSendingRate") == 0)
			{
				iss.str(line);
				iss >> std::skipws >> s >> s >> mSocketConfig.i_maxGreenExtenstion;
				iss.clear();
			}
			else if (line.find("syncPhase") == 0)
			{
				iss.str(line);
				iss >> std::skipws >> s >> s >> mSocketConfig.i_syncPhase;
				iss.clear();
			}			
			else if (line.find("coordinatedPhases") == 0)
			{
				iss.str(line);
				iss >> std::skipws >> s >> s >> mSocketConfig.s_coordinatedPhases;
				iss.clear();
			}
			else if (line.find("wmeListenSocket") == 0)
			{
				iss.str(line);
				iss >> std::skipws >> s >> s >> mSocketConfig.s_wmeListenSocket;
				iss.clear();
			}
			else if (line.find("wmeSendSocket") == 0)
			{
				iss.str(line);
				iss >> std::skipws >> s >> s >> mSocketConfig.s_wmeSendSocket;
				iss.clear();
			}
			else if (line.find("cloudListenSocket") == 0)
			{
				iss.str(line);
				iss >> std::skipws >> s >> s >> mSocketConfig.s_cloudListenSocket;
				iss.clear();
			}
			else if (line.find("cloudSendSocket") == 0)
			{
				iss.str(line);
				iss >> std::skipws >> s >> s >> mSocketConfig.s_cloudSendSocket;
				iss.clear();
			}
			else if (line.find("tciListenSocket") == 0)
			{
				iss.str(line);
				iss >> std::skipws >> s >> s >> mSocketConfig.s_tciListenSocket;
				iss.clear();
			}
			else if (line.find("tciSendSocket") == 0)
			{
				iss.str(line);
				iss >> std::skipws >> s >> s >> mSocketConfig.s_tciSendSocket;
				iss.clear();
			}
		}
	}
	IS_F.close();
	if (mSocketConfig.s_nmapFile.empty() 
		|| mSocketConfig.s_logPath.empty() 
		|| mSocketConfig.s_wmeListenSocket.empty() 
		|| mSocketConfig.s_wmeSendSocket.empty() 
		|| mSocketConfig.s_cloudListenSocket.empty()
		|| mSocketConfig.s_cloudSendSocket.empty()
		|| mSocketConfig.s_tciListenSocket.empty()
		|| mSocketConfig.s_tciSendSocket.empty())
		return false;
	else
		return true;
}

bool RSUconfig::configRsu::seupSocketAddress(void)
{
	if(!socketUtils::setSocketAddress(mSocketConfig.s_wmeListenSocket.c_str(),(*mpWMElistenAddr))
		|| !socketUtils::setSocketAddress(mSocketConfig.s_wmeSendSocket.c_str(),(*mpWMEsendAddr))
		|| !socketUtils::setSocketAddress(mSocketConfig.s_cloudListenSocket.c_str(),(*mpCLOUDlistenAddr))
		|| !socketUtils::setSocketAddress(mSocketConfig.s_cloudSendSocket.c_str(),(*mpCLOUDsendAddr))
		|| !socketUtils::setSocketAddress(mSocketConfig.s_tciListenSocket.c_str(),(*mpTCIlistenAddr))
		|| !socketUtils::setSocketAddress(mSocketConfig.s_tciSendSocket.c_str(),(*mpTCIsendAddr)))
		return false;
	else
		return true;
}

bool RSUconfig::configRsu::isConfigSucceed(void) const
{
	return configSucceed;
}

string RSUconfig::configRsu::getNmapFileName(void) const
{
	return mSocketConfig.s_nmapFile;
}

string RSUconfig::configRsu::getLogPath(void) const
{
	return mSocketConfig.s_logPath;
}

int RSUconfig::configRsu::getLogInterval(void) const
{
	return mSocketConfig.i_logInterval;
}

int RSUconfig::configRsu::getDsrcTimeOut(void) const
{
	return mSocketConfig.i_dsrcTimeoutSec;
}

int RSUconfig::configRsu::getMaxTime2goPhaseCall(void) const
{
	return mSocketConfig.i_maxTime2goPhaseCall;
}

int RSUconfig::configRsu::getMaxTime2changePhaseExt(void) const
{
	return mSocketConfig.i_maxTime2changePhaseExt;
}

int RSUconfig::configRsu::getMaxTime2phaseExt(void) const
{
	return mSocketConfig.i_maxTime2phaseExt;
}

int RSUconfig::configRsu::getMaxGreenExtenstion(void) const
{
	return mSocketConfig.i_maxGreenExtenstion;
}

int RSUconfig::configRsu::getPhaseExtEvery(void) const
{
	return mSocketConfig.i_phaseExtEvery;
}

int RSUconfig::configRsu::getSyncPhase(void) const
{
	return mSocketConfig.i_syncPhase;
}

std::string RSUconfig::configRsu::getCoordinatedPhases(void) const
{
	return mSocketConfig.s_coordinatedPhases;
}

socketUtils::socketAddress_t RSUconfig::configRsu::getSocketAddr(const RSUconfig::socketHandleEnum_t::socketHandle type) const
{
	socketUtils::socketAddress_t p;
	switch(type)
	{
	case RSUconfig::socketHandleEnum_t::WMELISTEN:
		p = *mpWMElistenAddr;
		break;
	case RSUconfig::socketHandleEnum_t::WMESEND:
		p = *mpWMEsendAddr;
		break;
	case RSUconfig::socketHandleEnum_t::CLOUDLISTEN:
		p = *mpCLOUDlistenAddr;
		break;
	case RSUconfig::socketHandleEnum_t::CLOUDSEND:
		p = *mpCLOUDsendAddr;
		break;
	case RSUconfig::socketHandleEnum_t::TCILISTEN:
		p = *mpTCIlistenAddr;
		break;
	default:
		p = *mpTCIsendAddr;
	}
	return (p);
}
