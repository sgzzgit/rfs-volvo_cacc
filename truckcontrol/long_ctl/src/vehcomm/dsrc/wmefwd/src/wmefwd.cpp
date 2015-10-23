#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <vector>
#include <getopt.h>
#include <iomanip> 			// setw, setfill

#include "libeloop.h"	  // eloop_* functions

#include "wmefwd.h"
#include "wmeconfig.h"
#include "socketUtils.h"
#include "wmeUtils.h"
#include "timeUtils.h"

#ifdef RSU
#include "savariWAVE_api.h"
#endif

using namespace std;

WMEconfig::appHandler_t appHandle;
std::map<uint32_t,size_t> wmePSIDmap; /// key: network byte psid, value: index in applications
std::vector<savariWmeHandler_t> savariHandle;
savari_wme_handler_t handler[2];
map<uint32_t,size_t>::iterator ite;
wmeUtils::wsmp_t wsmpHeader;
int fd_listen;
int fd_send;
uint8_t buf[2000];	
ssize_t bytesReceived;
ssize_t bytesSend;

string s_logfilename;
ofstream OS_log;

timeUtils::fullTimeStamp_t fullTimeStamp;
  
struct savariwme_cbs wme_cbs;
bool verbose = false;
bool logFile = false;

void do_usage(const char* progname)
{
	cerr << progname << "Usage: " << endl;
	cerr << "\t-c wmefwdConfig file" << endl;
	cerr << "\t-f turn logfile" << endl;
	cerr << "\t-v turn on verbose" << endl;
	exit (1);
}

int main (int argc, char** argv)
{
	int option;
	char* f_wmefwdConfig = NULL;
	/// getopt
	while ((option = getopt(argc, argv, "c:fv")) != EOF) 
	{
		switch(option) 
		{
		case 'c':
			f_wmefwdConfig = strdup(optarg);
			break;
    case 'f':
      logFile = true;
      break;
		case 'v':
			verbose = true;
			break;
		default:
			do_usage(argv[0]);
			break;
		}
	}
	if (f_wmefwdConfig == NULL)
		do_usage(argv[0]);
	
	/* ----------- preparing -------------------------------------*/	
	/// instance WMEconfig::configWme to read configuration files
	WMEconfig::configWme s_appConfig(f_wmefwdConfig);
	if (!s_appConfig.isConfigSucceed())
	{
		cerr << "Failed construct WMEconfig::configWme, filename " << f_wmefwdConfig << endl;
		delete f_wmefwdConfig;
		exit(1);
	}
	delete f_wmefwdConfig;
	appHandle = s_appConfig.getAppHandler();	
  if (appHandle.applications.empty())
  {
    cout << "appHandle: no applications" << endl;
    exit(1);
  }
	if (verbose)
	{
		cout << "appHandle " << appHandle.applications.size() << " applications, map size " << appHandle.appPSIDmap.size() << endl;		
		for (ite = appHandle.appPSIDmap.begin(); ite != appHandle.appPSIDmap.end(); ite++)
		{
			cout << "Application " << appHandle.applications[ite->second].s_name;
			cout << " psid: " << "0x" << hex << uppercase << ite->first << dec;
      cout << " network byte order psid: " << "0x" << hex << uppercase << wme_convert_psid_be(ite->first) << dec;
			cout << ", application index: " << ite->second << endl;
		}
	}
  	
  /// open log file
  s_logfilename = appHandle.s_logFilePath + std::string("/wmefwd.log");
  if (logFile)
  {
    OS_log.open(s_logfilename.c_str(), std::ofstream::out | std::ofstream::app);
    if (!OS_log.is_open())
    {
      cerr << "Failed open logfile " << s_logfilename << endl;
      exit(1);      
    }
  }
  
	/// open socket
	fd_listen = socketUtils::server(*(appHandle.mpListenAddr),false);
	if (fd_listen < 0)
	{
		cerr << "Failed create listen socket" << endl;
    if (logFile)
      OS_log.close();
		exit (1);
	}
	else if (verbose)
		cout << "Listen on " << appHandle.mpListenAddr->name << ":" << appHandle.mpListenAddr->port << endl;
		
	fd_send = socketUtils::client(*(appHandle.mpSendAddr),false);
	if (fd_send < 0)
	{
		cerr << "Failed create send socket" << endl;
    if (logFile)
      OS_log.close();
		close(fd_listen);
		exit (1);
	}
	else if (verbose)
		cout << "Send to " << appHandle.mpSendAddr->name << ":" << appHandle.mpSendAddr->port << endl;
		
	/// fill structure savariWmeHandler_t
	uint8_t broadcast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	savariHandle.resize(appHandle.applications.size());
	for (size_t i = 0; i < appHandle.applications.size(); i++)
	{
		savariHandle[i].isRegistered = false;
		savariHandle[i].isConfirmed = false;		
    savariHandle[i].handler = 0;
    
		memset(&(savariHandle[i].wmereq),0,sizeof(struct savariwme_reg_req));
		savariHandle[i].wmereq.channel = appHandle.applications[i].channel;
		memcpy(savariHandle[i].wmereq.destmacaddr, broadcast_mac, SAVARI1609_IEEE80211_ADDR_LEN);
		savariHandle[i].wmereq.psid = wme_convert_psid_be(appHandle.applications[i].psid);
    wmePSIDmap[savariHandle[i].wmereq.psid] = i;
		savariHandle[i].wmereq.priority = appHandle.applications[i].priority;
#ifdef RSU		
		if (appHandle.applications[i].e_channelMode == wmeUtils::channelmode_enum_t::CONTINUOUS)
		{			
			savariHandle[i].wmereq.channel_access = SAVARI1609_CHANNEL_ACCESS_CONTINUOUS;
      /* type of user application request:
          SAVARI1609_USER_AUTOACCESS_ONMATCH (Switch Between 178 and SCH after receiving Matching WSA from RSE)
          SAVARI1609_USER_AUTOACCESS_UNCOND (Start Switching between 178 and SCH without Waiting for Matching WSA from RSEs)
          SAVARI1609_USER_AUTOACCESS_NOSCHACCESS(CCH Only Mode. No Switching). Only applicable if channel_access is ALTERNATING
      */
			savariHandle[i].wmereq.request_type = SAVARI1609_USER_AUTOACCESS_ONMATCH;
			savariHandle[i].wmereq.extended_access = 0xFFFF;
		}
		else
		{
			savariHandle[i].wmereq.channel_access = SAVARI1609_CHANNEL_ACCESS_ALTERNATING;
			savariHandle[i].wmereq.request_type = SAVARI1609_USER_AUTOACCESS_UNCOND;
			savariHandle[i].wmereq.extended_access = 0;
		}	
#else
		savariHandle[i].wmereq.request_type = LIBWME_USER_AUTOACCESS_ONMATCH;
		savariHandle[i].wmereq.extended_access = 0xFFFF;
#endif		
		savariHandle[i].wmereq.immediate_access = 0;
		savariHandle[i].wmereq.psc_length = static_cast<int>(appHandle.applications[i].s_psc.size() + 1);
		memcpy(savariHandle[i].wmereq.psc, appHandle.applications[i].s_psc.c_str(), savariHandle[i].wmereq.psc_length);
		
		memset(&(savariHandle[i].wmetx),0, sizeof(struct savariwme_tx_req));
		savariHandle[i].wmetx.channel = appHandle.applications[i].channel;
		savariHandle[i].wmetx.psid = savariHandle[i].wmereq.psid;
		savariHandle[i].wmetx.priority = appHandle.applications[i].priority;
		savariHandle[i].wmetx.datarate = 6; //3Mbps
		savariHandle[i].wmetx.txpower = 15; //in dbM
		memcpy(savariHandle[i].wmetx.mac, broadcast_mac, SAVARI1609_IEEE80211_ADDR_LEN);
		savariHandle[i].wmetx.expiry_time = 0;
		savariHandle[i].wmetx.element_id = WAVE_ELEMID_WSMP;
		savariHandle[i].wmetx.tx_length = 0;
		savariHandle[i].wmetx.supp_enable = 0;		
  }  
  
  /// set up WME callback functions
  wme_cbs.wme_user_confirm = &savari_user_confirm;
  wme_cbs.wme_provider_confirm = &savari_provider_confirm;	
	wme_cbs.wme_wsm_indication = &savari_wsm_indication;
  
  /// connect to WME
  for (int i=0;i<2;i++)
  {
    handler[i] = wme_init("::1", (char*)wmeUtils::iface[i]);
    if (handler[i] < 0)
    {
      cerr << "Failed wme_init for interface " << wmeUtils::iface [i] << endl;
      if (logFile)
        OS_log.close();
      close(fd_listen);
      close(fd_send);
    }
    else if (verbose)
    {
      cout << "wme_init succeed for interface " << wmeUtils::iface[i] << endl;
    }
  }
  
	/// initial event loop
	if (eloop_init(0) < 0)
	{
		cerr << "Failed eloop_init" << endl;
    if (logFile)
      OS_log.close();
		close(fd_listen);
		close(fd_send);
		exit(1);
	}
  
	/* ----------- intercepts signals -------------------------------------*/
	if (setjmp(exit_env) != 0) 
	{
		cerr << "Received user termination signal, quit!" << endl;
    if (logFile)
    {
      timeUtils::getFullTimeStamp(fullTimeStamp);
      OS_log << timeUtils::getTimestampStr(fullTimeStamp.localDateTimeStamp);
      OS_log << ": Received user termination signal, quit!" << endl;
    }
    close_all();
	}
	else
		sig_ign( sig_list, sig_hand );  
 
  /// register applications to WME
  int ret;
	for (size_t i = 0; i < appHandle.applications.size(); i++)
	{
    if (appHandle.applications[i].e_registerType == WMEconfig::appConfigEnum_t::USER)
      ret = wme_register_user(handler[appHandle.applications[i].e_iface],&savariHandle[i].wmereq);
    else
      ret = wme_register_provider(handler[appHandle.applications[i].e_iface],&savariHandle[i].wmereq);
    
    if (ret < 0)
    {
      cerr << "Failed wme_register for " << appHandle.applications[i].s_name << endl;
      close_all();
    }
    else
    {
      savariHandle[i].handler = handler[appHandle.applications[i].e_iface];
      savariHandle[i].isRegistered = true;
      if (verbose)
      {
        cout << "wme_register succeed for " << appHandle.applications[i].s_name << endl;
      }
    }
  }
 
  /// WME eloop_register_read_sock 
	for (size_t i = 0; i < appHandle.applications.size(); i++)
	{
    ret = eloop_register_read_sock(savariHandle[i].handler,savari_rx,0,&(savariHandle[i].wmereq));
    if (ret < 0)
    {
      cerr << "Failed eloop_register_read_sock for " << appHandle.applications[i].s_name << endl;
      close_all();
    }
    else if (verbose)
      cout << "eloop_register_read_sock succeed for " << appHandle.applications[i].s_name << endl;
  }
  
  /// fd_listen eloop_register_read_sock
  ret = eloop_register_read_sock(fd_listen,wsm_tx,0,0);
  if (ret < 0)
  {
    cerr << "Failed eloop_register_read_sock for listening socket" << endl;
    close_all();
  }
  else if (verbose)
  {
    cout << "eloop_register_read_sock succeed for listening socket" << endl;
  }

	if (verbose)
	{
		cout << "start eloop_run waiting for data ..." << endl;
	}
  if (logFile)
  {
    timeUtils::getFullTimeStamp(fullTimeStamp);
    OS_log << timeUtils::getTimestampStr(fullTimeStamp.localDateTimeStamp);
    OS_log << ": Start eloop_run waiting for data ..." << endl;
  }
	eloop_run();
	
	/// should not be here
  if (logFile)
  {
    timeUtils::getFullTimeStamp(fullTimeStamp);
    OS_log << timeUtils::getTimestampStr(fullTimeStamp.localDateTimeStamp);
    OS_log << ": Exit from eloop_run!" << endl;
  }  
  close_all();
	return 0;
}

/* savari_user_confirm - Invoked by the WME layer to indicate the status of wme_register_user.
 * \param ctx - application ctx
 * \param conf_result_indication - registration status
 */
void savari_user_confirm(void *ctx, int conf_result_indication)
{
	struct savariwme_reg_req *wme_req = (struct savariwme_reg_req *)ctx;
	ite = wmePSIDmap.find(wme_req->psid);
  if (ite == wmePSIDmap.end())
  {
    cerr << "Failed savari_user_confirm, can't find psid " << "0x" << hex << uppercase << wme_req->psid << dec << " in wmePSIDmap" << endl;
    close_all();
  }
#ifdef RSU	
	if (conf_result_indication == SAVARI1609_RC_ACCEPTED)
	{
		savariHandle[ite->second].isConfirmed = true;
		wme_user_service_confirm(savariHandle[ite->second].handler, SAVARI1609_ACTION_ADD, wme_req);
		if (verbose)
			cout << "wme_user_service_confirm for " << appHandle.applications[ite->second].s_name << endl;
	}
#else
	if (conf_result_indication == LIBWME_RC_ACCEPTED)
	{
		savariHandle[ite->second].isConfirmed = true;	
		wme_user_service_confirm(savariHandle[ite->second].handler, LIBWME_ACTION_ADD, wme_req);
		if (verbose)
			cout << "wme_user_service_confirm for " << appHandle.applications[ite->second].s_name << endl;
	}
#endif  
	else
	{	
    cerr << "Failed savari_user_confirm for " << appHandle.applications[ite->second].s_name << endl;
    cerr << ", code " << conf_result_indication << endl;
    close_all();
  }
}

void savari_provider_confirm(void *ctx, int conf_result_indication)
{
	struct savariwme_reg_req *wme_req = (struct savariwme_reg_req *)ctx;
	ite = wmePSIDmap.find(wme_req->psid);
  if (ite == wmePSIDmap.end())
  {
    cerr << "Failed savari_provider_confirm, can't find psid " << "0x" << hex << uppercase << wme_req->psid << dec << " in wmePSIDmap" << endl;
    close_all();
  }
#ifdef RSU	
	if (conf_result_indication == SAVARI1609_RC_ACCEPTED)
	{
		savariHandle[ite->second].isConfirmed = true;
		wme_provider_service_confirm(savariHandle[ite->second].handler, SAVARI1609_ACTION_ADD, wme_req);
		if (verbose)
			cout << "wme_provider_service_confirm for " << appHandle.applications[ite->second].s_name << endl;
	}
#else
	if (conf_result_indication == LIBWME_RC_ACCEPTED)
	{
		savariHandle[ite->second].isConfirmed = true;	
		wme_provider_service_confirm(savariHandle[ite->second].handler, LIBWME_ACTION_ADD, wme_req);
		if (verbose)
			cout << "wme_provider_service_confirm for " << appHandle.applications[ite->second].s_name << endl;
	}
#endif  
	else
	{	
    cerr << "Failed wme_provider_service_confirm for " << appHandle.applications[ite->second].s_name << endl;
    cerr << ", code " << conf_result_indication << endl;
    close_all();
	}
}

/* savari_rx - Thread that waits for data on the savari_wme_handler returned by the wme_init. 
 * When data is available wme_rx invokes registered callbacks (wme_user_confirm, wme_provider_confirm & wme_wsm_indication) */
void savari_rx(int sock_t, void *eloop_data, void *user_data)
{
  wme_rx(sock_t,&wme_cbs,user_data);
}

/* savari_wsm_indication - Invoked by the WME layer indicating the application about the WSM packet matching based on psid.
 * \param ctx - application ctx
 * \param rxind - rx indication buffer */
void savari_wsm_indication(void *ctx, struct savariwme_rx_indication *rxind)
{
	/// received over the air WSM packet
	struct savariwme_reg_req *wme_req = (struct savariwme_reg_req *)ctx;
	if (verbose)
	{
		cout << "Received wme_req psid: 0x" << hex << uppercase << wme_req->psid << dec << endl;   
    cout << "savariwme_rx_indication psid: " << "0x" << hex << uppercase << static_cast<int>(rxind->psid[0]) << " ";
    cout << static_cast<int>(rxind->psid[1]) << " " << static_cast<int>(rxind->psid[2]) << " " << static_cast<int>(rxind->psid[3]) << dec << endl;
	}  
  
	ite = wmePSIDmap.find(wme_req->psid);
	if (ite != wmePSIDmap.end() && appHandle.applications[ite->second].e_direction != WMEconfig::appConfigEnum_t::TX)
	{
		/// encode wsmp header
		wsmpHeader.version = static_cast<uint8_t>(rxind->version);
		wsmpHeader.psid = appHandle.applications[ite->second].psid;
		wsmpHeader.channel = static_cast<uint8_t>(rxind->channel);
		wsmpHeader.rate = static_cast<uint8_t>(rxind->datarate);
		wsmpHeader.txpower = static_cast<uint8_t>(rxind->txpower);
		wsmpHeader.txlength = static_cast<uint16_t>(rxind->num_rx);
		wsmpHeader.waveId = WAVE_ELEMID_WSMP;
		int headerLen = wmeUtils::encodewsm(&wsmpHeader,buf,(int)sizeof(buf));
		memcpy(&buf[headerLen],&rxind->rx_buf[0],rxind->num_rx);
		bytesSend = static_cast<size_t>(headerLen) + static_cast<size_t>(rxind->num_rx);    
		socketUtils::sendall(fd_send,(char*)buf,bytesSend);
    if (verbose)
		{
			cout << "sent wme to application, psid 0x" << hex << uppercase << wsmpHeader.psid << dec << endl;
		}
	}	
}

void wsm_tx(int sock_t, void *eloop_data, void *user_data)
{
	/// received packet on eth0 interface, going to broadcast
	bytesReceived = recv(sock_t,buf,sizeof(buf),0);
	if (verbose)
	{
		cout << "receive on eth0 " << bytesReceived << "bytes" << endl;
	}
  
	if ((long)bytesReceived > (long)(sizeof(wmeUtils::mmitss_udp_header_t)) && buf[0] == static_cast<uint8_t>(0xFF) && buf[1] == static_cast<uint8_t>(0xFF))
	{
		uint8_t	msgid = buf[2];
		map<uint8_t,uint32_t>::const_iterator it = wmeUtils::mmitssMsgid2psidMap.find(msgid);
		if (it != wmeUtils::mmitssMsgid2psidMap.end())
		{
			ite = appHandle.appPSIDmap.find(it->second);
			if (ite != appHandle.appPSIDmap.end() && appHandle.applications[ite->second].isEnabled
				&& appHandle.applications[ite->second].e_direction != WMEconfig::appConfigEnum_t::RX
				&& savariHandle[ite->second].isRegistered)
			{
				savariHandle[ite->second].wmetx.tx_length = static_cast<int>(bytesReceived - sizeof(wmeUtils::mmitss_udp_header_t));
				if (wme_wsm_tx(savariHandle[ite->second].handler,&(savariHandle[ite->second].wmetx),&buf[sizeof(wmeUtils::mmitss_udp_header_t)]) < 0)
				{
					cerr << "Failed wme_wsm_tx for " << appHandle.applications[ite->second].s_name;					
				}		
				else if(verbose)
        {
					cout << "Send msgid 0x" << hex << uppercase << static_cast<int>(msgid) << dec << ", application " << appHandle.applications[ite->second].s_name;
          cout << " to wme psid 0x" << hex << uppercase << it->second << dec << " for broadcast" << endl;
        }
			}
    }
		else
			cerr << "Failed finding msgid 0x" << hex << uppercase << static_cast<int>(msgid) << " in mmitssMsgid2psidMap" << endl;
	}
	else
  {
		cerr << "Wrong mmitss header, reject to broadcast: 0x" << hex << uppercase << static_cast<int>(buf[0]);
    cerr << " " <<  static_cast<int>(buf[1]) << " " << static_cast<int>(buf[2]) << dec << endl;
  }
}

void close_all(void)
{
  for (size_t i = 0; i < savariHandle.size(); i++)
  {
    if (savariHandle[i].isRegistered)
    {
      if (appHandle.applications[i].e_registerType == WMEconfig::appConfigEnum_t::USER)
        wme_unregister_user(savariHandle[i].handler,&(savariHandle[i].wmereq));
      else
        wme_unregister_provider(savariHandle[i].handler,&(savariHandle[i].wmereq));
    }
  }
  eloop_unregister_read_sock(handler[0]);
  eloop_unregister_read_sock(handler[1]);
  eloop_unregister_read_sock(fd_listen);
  eloop_terminate();
  sleep(1);    
  eloop_destroy();
  close(fd_listen);
  close(fd_send);    
  if (logFile)
  {
    timeUtils::getFullTimeStamp(fullTimeStamp);
    OS_log << timeUtils::getTimestampStr(fullTimeStamp.localDateTimeStamp);
    OS_log << ": close_all()" << endl;
    OS_log.close();
  }
  exit (0);
}
