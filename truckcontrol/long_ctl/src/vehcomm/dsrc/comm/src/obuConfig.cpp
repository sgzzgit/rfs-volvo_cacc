#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <cstring>

#include "obuConfig.h"
#include "socketUtils.h"

using namespace std;

OBUconfig::configObu::configObu(const char* ptr1,const char* ptr2)
{
	// default value, can be changed when readSocketConfigFile
	configSucceed = false;
	mSocketConfig.i_logInterval = 15; 		
	mSocketConfig.i_dsrcTimeoutSec = 2;		
	mVinConfig.isPrioEligible = false;
	mVinConfig.vehType = 0;
	mVinConfig.prioLevel = 15;
	
	mpWMElistenAddr = new socketUtils::socketAddress_t;
	mpWMEsendAddr = new socketUtils::socketAddress_t;
	
	bool isSocketConfigSucceed = readSocketConfigFile(ptr1);
	bool isVinConfigSucceed = true;
	if (ptr2 != NULL)
		isVinConfigSucceed = readVinConfigFile(ptr2);
	bool isSetupAddrSucceed = seupSocketAddress();
	if (!isSocketConfigSucceed || !isVinConfigSucceed || !isSetupAddrSucceed)
		configSucceed = false;
	else
		configSucceed = true;
}

OBUconfig::configObu::~configObu(void)
{
	delete mpWMElistenAddr;
	delete mpWMEsendAddr;
}

bool OBUconfig::configObu::readSocketConfigFile(const char* ptr1)
{
	ifstream IS_F(ptr1);
  if (!IS_F.is_open())
	{
		cerr << "OBUconfig::readSocketConfigFile failed open: " << ptr1 << endl;
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
		}
	}
	IS_F.close();
	if (mSocketConfig.s_nmapFile.empty() 
		|| mSocketConfig.s_logPath.empty() 
		|| mSocketConfig.s_wmeListenSocket.empty() 
		|| mSocketConfig.s_wmeSendSocket.empty())
		return false;
	else
		return true;
}

bool OBUconfig::configObu::readVinConfigFile(const char* ptr2)
{
	int prioEligible = 0;
	int vehType = -1;
	int prioLevel = -1;
	ifstream IS_F(ptr2);
  if (!IS_F.is_open())
	{
		cerr << "OBUconfig::readVinConfigFile failed open: " << ptr2 << endl;
		return false;
	}
	istringstream iss;
	string line,s;
  while (std::getline(IS_F,line))
  {
		if (!line.empty())
		{
			if (line.find("isPrioEligible") == 0)
			{
				iss.str(line);
				iss >> std::skipws >> s >> s >> prioEligible;
				if (prioEligible == 1)
					mVinConfig.isPrioEligible = true;
				else
					mVinConfig.isPrioEligible = false;
				iss.clear();
			}
			else if (line.find("codeWord") == 0)
			{
				iss.str(line);
				iss >> std::skipws >> s >> s >> mVinConfig.codeWord;
				iss.clear();
			}			
			else if (line.find("vehId") == 0)
			{
				iss.str(line);
				iss >> std::skipws >> s >> s >> mVinConfig.vehId;
				iss.clear();
			}			
			else if (line.find("vehLen") == 0)
			{
				iss.str(line);
				iss >> std::skipws >> s >> s >> mVinConfig.vehLen;
				iss.clear();
			}			
			else if (line.find("vehWidth") == 0)
			{
				iss.str(line);
				iss >> std::skipws >> s >> s >> mVinConfig.vehWidth;
				iss.clear();
			}			      
			else if (line.find("vehName") == 0)
			{
				iss.str(line);
				iss >> std::skipws >> s >> s >> mVinConfig.name;
				iss.clear();
			}
			else if (line.find("vehVin") == 0)
			{
				iss.str(line);
				iss >> std::skipws >> s >> s >> mVinConfig.vin;
				iss.clear();
			}
			else if (line.find("vehOwner") == 0)
			{
				iss.str(line);
				iss >> std::skipws >> s >> s >> mVinConfig.ownerCode;
				iss.clear();
			}
			else if (line.find("vehType") == 0)
			{
				iss.str(line);
				iss >> std::skipws >> s >> s >> vehType;
				if (vehType >=0 && vehType < 0x0F)
					mVinConfig.vehType = static_cast<uint8_t>(vehType);
				else
					vehType = -1;
				iss.clear();
			}
			else if (line.find("priorityLevel") == 0)
			{
				iss.str(line);
				iss >> std::skipws >> s >> s >> prioLevel;
				if (prioLevel >=0 && prioLevel < 0x0F)
					mVinConfig.prioLevel = static_cast<uint8_t>(prioLevel);
				else
					prioLevel = -1;
				iss.clear();
			}
		}
	}
	IS_F.close();
	if (mVinConfig.name.empty() 
		|| mVinConfig.vin.empty() 
		|| mVinConfig.ownerCode.empty() 
		|| vehType == -1
		|| (mVinConfig.isPrioEligible && prioLevel == -1))
		return false;
	else
		return true;
}				

bool OBUconfig::configObu::seupSocketAddress(void)
{
	bool isWMElistenSucceed = socketUtils::setSocketAddress(mSocketConfig.s_wmeListenSocket.c_str(),(*mpWMElistenAddr));
	bool isWMEsendSucceed = socketUtils::setSocketAddress(mSocketConfig.s_wmeSendSocket.c_str(),(*mpWMEsendAddr));
	if (!isWMElistenSucceed || !isWMEsendSucceed)
		return false;
	else
		return true;
}

bool OBUconfig::configObu::isConfigSucceed(void) const
{
	return configSucceed;
}

string OBUconfig::configObu::getNmapFileName(void) const
{
	return mSocketConfig.s_nmapFile;
}

string OBUconfig::configObu::getLogPath(void) const
{
	return mSocketConfig.s_logPath;
}

int OBUconfig::configObu::getLogInterval(void) const
{
	return mSocketConfig.i_logInterval;
}

int OBUconfig::configObu::getDsrcTimeOut(void) const
{
	return mSocketConfig.i_dsrcTimeoutSec;
}

struct OBUconfig::VinConfig OBUconfig::configObu::getVinConfig(void) const
{
	return mVinConfig;
}

socketUtils::socketAddress_t OBUconfig::configObu::getSocketAddr(const OBUconfig::socketHandleEnum_t::socketHandle type) const
{
	socketUtils::socketAddress_t p;
  switch(type)
  {
  case OBUconfig::socketHandleEnum_t::WMELISTEN:
		p = *mpWMElistenAddr;
    break;
   default:
		p = *mpWMEsendAddr;
  }
	return (p);	
}
