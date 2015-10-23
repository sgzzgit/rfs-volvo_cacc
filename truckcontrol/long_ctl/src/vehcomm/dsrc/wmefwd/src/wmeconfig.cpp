#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <cstring>
#include <map>
#include <netinet/in.h>

#include "wmeconfig.h"
#include "socketUtils.h"

using namespace std;

WMEconfig::configWme::configWme(const char* ptr)
{
	appHandler.mpListenAddr = new socketUtils::socketAddress_t;
	appHandler.mpSendAddr = new socketUtils::socketAddress_t;
	configSucceed = WMEconfig::configWme::readWmefwdConfigFile(ptr);
}

WMEconfig::configWme::~configWme(void)
{
	delete appHandler.mpListenAddr;
	delete appHandler.mpSendAddr;
}

bool WMEconfig::configWme::readWmefwdConfigFile(const char* ptr)
{
	ifstream IS_F(ptr);
  if (!IS_F.is_open())
	{
		cerr << "WMEconfig::readWmefwdConfigFile failed open: " << ptr << endl;
		return false;
	}	
	WMEconfig::appConfig_t* pAppConfig = new WMEconfig::appConfig_t;
	istringstream iss;
	string line,s;
  while (std::getline(IS_F,line))
  {
		if (!line.empty())
		{
			if (line.find("application") == 0)
			{
				iss.str(line);
				iss >> std::skipws >> s >> s;
				iss.clear();				
				if (!parseAppConfig(s,pAppConfig))
				{
					cerr << "WMEconfig::parseAppConfig failed: " << s << endl;
					return false;
				}
				if (pAppConfig->isEnabled)
					appHandler.applications.push_back(*pAppConfig);
			}
			else if (line.find("listenSocket") == 0)
			{
				iss.str(line);
				iss >> std::skipws >> s >> s;
				iss.clear();					
				if (!(socketUtils::setSocketAddress(s.c_str(),(*(appHandler.mpListenAddr)))))
				{
					cerr << "WMEconfig::setSocketAddress listen failed: " << s << endl;
					return false;
				}
			}
			else if (line.find("sendSocket") == 0)
			{
				iss.str(line);
				iss >> std::skipws >> s >> s;
				iss.clear();				
				if (!(socketUtils::setSocketAddress(s.c_str(),(*(appHandler.mpSendAddr)))))
				{
					cerr << "WMEconfig::setSocketAddress send failed: " << s << endl;
					return false;
				}
			}
      else if (line.find("logFilePath") == 0)
      {
				iss.str(line);
				iss >> std::skipws >> s >> appHandler.s_logFilePath;
				iss.clear();				
      }  
		}
	}
	IS_F.close();
	delete pAppConfig;
	for (size_t i = 0; i < appHandler.applications.size(); i++)
	{
		appHandler.appPSIDmap[appHandler.applications[i].psid] = i;
	}
	return true;
}

bool WMEconfig::configWme::parseAppConfig(const string str,WMEconfig::appConfig_t* pAppConfig)				
{
	// str in the formate of enable/disable : application name : wmeRegisterType : TxDirection : PSC 
	// e.g., 1:BSM:USER:ath1:0x20:172:2:TX_RX:CONT:MMITSS
	std::string in_str(str);	
	if (std::count(in_str.begin(),in_str.end(),':') != 4)
		return false;	
	std::istringstream iss(in_str);
	std::string s;
	std::getline(iss, s, ':'); // isEnabled	
	if (s.empty())
		return false;
  if (s.compare("1") == 0)
		pAppConfig->isEnabled = true;
  else if (s.compare("0") == 0)
		pAppConfig->isEnabled = false;
  else
    return false;
	std::getline(iss, s, ':'); // name
	if (s.empty())
		return false;
	pAppConfig->s_name = s;
	std::getline(iss, s, ':'); // registerType
	if (s.empty())
		return false;
  if (s.compare("USER") == 0)
    pAppConfig->e_registerType = appConfigEnum_t::USER;
  else if (s.compare("PROVIDER") == 0)
    pAppConfig->e_registerType = appConfigEnum_t::PROVIDER;
  else 
    return false;       
	std::getline(iss, s, ':'); // direction
	if (s.empty())
		return false;
	if (s.compare("TX") == 0)
		pAppConfig->e_direction = appConfigEnum_t::TX;
	else if (s.compare("RX") == 0)
		pAppConfig->e_direction = appConfigEnum_t::RX;
	else if (s.compare("TX_RX")	== 0)
		pAppConfig->e_direction = appConfigEnum_t::TX_RX;
	else
		return false;
	std::getline(iss, s, ':'); // psc
	if (s.empty())
		return false;
	pAppConfig->s_psc = s;
  
  /// now get channel confg from wmeUtils::wmwAppChannelMap
  std::map<std::string,wmeUtils::wme_channel_config_t>::const_iterator  ite = wmeUtils::wmwAppChannelMap.find(pAppConfig->s_name);
  if (ite == wmeUtils::wmwAppChannelMap.end())
    return false;
  pAppConfig->e_iface = ite->second.txIface;
  pAppConfig->e_channelMode = ite->second.txMode;
  pAppConfig->s_InterfaceName = ite->second.iface;
  pAppConfig->psid = ite->second.psid;
  pAppConfig->channel = ite->second.channel;
  pAppConfig->priority = ite->second.priority;
  /// tx_length is determined when WSM or application pkts arrived
	return true;
}

bool WMEconfig::configWme::isConfigSucceed(void) const
{
	return (configSucceed);
}

WMEconfig::appHandler_t WMEconfig::configWme::getAppHandler(void) const
{
	return (appHandler);
}
