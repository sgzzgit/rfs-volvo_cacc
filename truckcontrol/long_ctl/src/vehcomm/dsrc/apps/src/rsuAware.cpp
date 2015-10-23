/* rsuAware.cpp
 *	MMITSS RSU main process, tasks include
 *		1. receives on the air WME messages forwarded by wmefwd, including 
 *				BSM, SRM from OBUs and SPaT and Map from nearby RSUs
 *		2. receives its own SPaT send by TCI
 *		3. decodes WME messages
 *		4. tracks OBU locations on the map and identifies those are on an approach at the intersection
 *		5. tracks SRM and associates with BSM
 *		6. sends soft-calls to TCI 
 *		7. sends ART to wmefwd to be forwarded to on the air messages
*/

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <bitset>  
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "AsnJ2735Lib.h"
#include "comm.h"
#include "socketUtils.h"
#include "rsuAware.h"

using namespace std;

void do_usage(const char* progname)
{
	cout << "Usage" << progname << endl;
	cout << "\t-n intersection name" << endl;
	cout << "\t-s socket.config" << endl;
	cout << "\t-f turn on log files" << endl;
	cout << "\t-v turn on verbose" << endl;
	exit (1);
}

int main(int argc, char** argv) 
{
	int option;
	char*	f_intName = NULL;
	char* f_socketConfig = NULL;
  volatile bool sendSoftcall = false;
	volatile bool logFile = false;
	volatile bool verbose = false;
	
	// getopt
	while ((option = getopt(argc, argv, "s:n:cfv")) != EOF) 
	{
		switch(option) 
		{
		case 's':
			f_socketConfig = strdup(optarg);
			break;
		case 'n':
			f_intName = strdup(optarg);
			break;
    case 'c':
      sendSoftcall = true;
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
	if (f_socketConfig == NULL || f_intName == NULL)
		do_usage(argv[0]);

	/* ----------- preparation -------------------------------------*/	
	/// instance RSUconfig::configRsu to read configuration files
	RSUconfig::configRsu s_rsuConfig(f_socketConfig);
	if (!s_rsuConfig.isConfigSucceed())
	{
		cerr << argv[0] << ": failed construct RSUconfig::configRsu";
		cerr << ", f_socketConfig = " << f_socketConfig << endl;
		delete f_socketConfig;
		exit(1);
	}
	delete f_socketConfig;

	/// store intersectionName
	string intersectionName = f_intName;
	delete f_intName;

	/// instance AsnJ2735Lib to read nmap
	AsnJ2735Lib asnJ2735Lib(s_rsuConfig.getNmapFileName().c_str());
	if (asnJ2735Lib.getMapVersion() == 0)
	{
		cerr << argv[0] << ": failed open nmap file = " << s_rsuConfig.getNmapFileName() << endl;
		exit (1);
	}
	
	/// get intersectionId
	uint32_t intersectionId = asnJ2735Lib.getIntersectionIdByName(intersectionName.c_str());
	if (intersectionId == 0)
	{
		cerr << argv[0] << ": failed getIntersectionIdByName: " << intersectionName << endl;
		exit (1);
	}

	/// get index of this intersection
	int8_t intersectionIndex = static_cast<int8_t>(asnJ2735Lib.getIndexByIntersectionId(intersectionId));
	if (intersectionIndex < 0)
	{
		cerr << argv[0] << ": failed getIndexByIntersectionId: " << intersectionId << endl;
		exit (1);
	}
	
	/// timestamp
	timeUtils::fullTimeStamp_t fullTimeStamp;
	timeUtils::getFullTimeStamp(fullTimeStamp);

  /// open display log file
	string logFilePath = s_rsuConfig.getLogPath();
  string s_DisplayLogFileName = logFilePath + std::string("/display.log");
  ofstream OS_Display(s_DisplayLogFileName.c_str());
  if (!OS_Display.is_open())
  {
    cerr << argv[0] << ": failed open display log file" << endl;
    exit(1);
  }
  
	/// open log files
	long long logFileInterval = static_cast<long long>(s_rsuConfig.getLogInterval()) * 60 * 1000LL;
	ofstream OS_Aware;		// log tracked bsm locationAware & signalAware
	ofstream OS_CvStatus;	// simple log of changing of cv status	
	ofstream OS_Wme;			// simple log of received WMEs (BSM & SRM from OBUs, its own SPaT,  and SPaT & Map from nearby intersections)
	ofstream OS_Payload;	// log inbound and outbound payloads for offline debugging
	ofstream OS_Art;			// log ART
	ofstream OS_Softcall;	// log Soft-call	
	ofstream OS_PedCall;	// log pedestrian SRM sent from the cloud server
	ofstream OS_Ownspat;	// log pedestrian SRM sent from the cloud server	
	string s_AwreLogFileName;
	string s_CvStatusLogFileName;	
	string s_WmeLogFileName;
	string s_PayloadLogFileName;	
	string s_ArtLogFileName;
	string s_SoftcallLogFileName;
	string s_PedcallLogFileName;
	string s_OwnspatLogFileName;
	volatile unsigned long l_awreLogRows = 0;
	volatile unsigned long l_cvstatusLogRows = 0;	
	volatile unsigned long l_wmeLogRows = 0;
	volatile unsigned long l_payloadLogRows = 0;	
	volatile unsigned long l_artLogRows = 0;
	volatile unsigned long l_softcallLogRows = 0;	
	volatile unsigned long l_pedcallLogRows = 0;		
	volatile unsigned long l_ownspatLogRows = 0;			
	volatile long long logfile_tms_minute = fullTimeStamp.tms_minute;
	if (logFile)
	{
		if (!openLogFile(OS_Aware,s_AwreLogFileName,logFilePath,fullTimeStamp.localDateTimeStamp,intersectionName,"aware"))
		{
			cerr << argv[0] << ": failed open aware log file" << endl;
      OS_Display << "Failed open aware log file" << endl;
      OS_Display.close();
			exit(1);
		}
		l_awreLogRows = 0;
		
		if (!openLogFile(OS_CvStatus,s_CvStatusLogFileName,logFilePath,fullTimeStamp.localDateTimeStamp,intersectionName,"cvstatus"))
		{
			cerr << argv[0] << ": failed open cvstatus log file" << endl;
      OS_Display << "Failed open cvstatus log file" << endl;
      OS_Display.close();
			OS_Aware.close();
			exit(1);
		}
		l_cvstatusLogRows = 0;
		
		if (!openLogFile(OS_Art,s_ArtLogFileName,logFilePath,fullTimeStamp.localDateTimeStamp,intersectionName,"art"))
		{
			cerr << argv[0] << ": failed open art log file" << endl;
      OS_Display << "Failed open art log file" << endl;
      OS_Display.close();
			OS_Aware.close();
			OS_CvStatus.close();
			exit(1);
		}
		l_artLogRows = 0;
		
		if (!openLogFile(OS_Wme,s_WmeLogFileName,logFilePath,fullTimeStamp.localDateTimeStamp,intersectionName,"wme"))
		{
			cerr << argv[0] << ": failed open wme log file" << endl;
      OS_Display << "Failed open wme log file" << endl;
      OS_Display.close();
			OS_Aware.close();
			OS_CvStatus.close();
			OS_Art.close();
			exit(1);
		}
		l_wmeLogRows = 0;
		
		if (!openLogFile(OS_Payload,s_PayloadLogFileName,logFilePath,fullTimeStamp.localDateTimeStamp,intersectionName,"payload"))
		{
			cerr << argv[0] << ": failed open payload log file" << endl;
      OS_Display << "Failed open payload log file" << endl;
      OS_Display.close();
			OS_Aware.close();
			OS_CvStatus.close();
			OS_Art.close();
			OS_Wme.close();			
			exit(1);
		}
		l_payloadLogRows = 0;
		
		if (!openLogFile(OS_Softcall,s_SoftcallLogFileName,logFilePath,fullTimeStamp.localDateTimeStamp,intersectionName,"softcall"))
		{
			cerr << argv[0] << ": failed open softcall log file" << endl;
      OS_Display << "Failed open softcall log file" << endl;
      OS_Display.close();      
			OS_Aware.close();
			OS_CvStatus.close();
			OS_Art.close();
			OS_Wme.close();	
			OS_Payload.close();
			exit(1);
		}
		l_softcallLogRows = 0;
		
		if (!openLogFile(OS_PedCall,s_PedcallLogFileName,logFilePath,fullTimeStamp.localDateTimeStamp,intersectionName,"pedcall"))
		{
			cerr << argv[0] << ": failed open pedcall log file" << endl;
      OS_Display << "Failed open pedcall log file" << endl;
      OS_Display.close();      
			OS_Aware.close();
			OS_CvStatus.close();
			OS_Art.close();
			OS_Wme.close();	
			OS_Payload.close();
			OS_Softcall.close();
			exit(1);
		}
		l_pedcallLogRows = 0;
		
		if (!openLogFile(OS_Ownspat,s_OwnspatLogFileName,logFilePath,fullTimeStamp.localDateTimeStamp,intersectionName,"spat"))
		{
			cerr << argv[0] << ": failed open ownspat log file" << endl;
      OS_Display << "Failed open ownspat log file" << endl;
      OS_Display.close();            
			OS_Aware.close();
			OS_CvStatus.close();
			OS_Art.close();
			OS_Wme.close();	
			OS_Payload.close();
			OS_Softcall.close();
			OS_PedCall.close();
			exit(1);
		}
		l_ownspatLogRows = 0;    
		
		logfile_tms_minute = fullTimeStamp.tms_minute;
	}
  
	/// open sockets
	int fd_wmeListen = socketUtils::server(s_rsuConfig.getSocketAddr(RSUconfig::socketHandleEnum_t::WMELISTEN),true);
	if (fd_wmeListen < 0)
	{
		cerr << argv[0] << ": failed create WMELISTEN socket" << endl;
    OS_Display << "Failed create WMELISTEN socket" << endl;
    OS_Display.close();
		if (logFile)
		{
			OS_Aware.close();
			OS_CvStatus.close();
			OS_Art.close();
			OS_Wme.close();	
			OS_Payload.close();
			OS_Softcall.close();
			OS_PedCall.close();
			OS_Ownspat.close();
		}
		exit(1);
	}
	
	int fd_wmeSend = socketUtils::client(s_rsuConfig.getSocketAddr(RSUconfig::socketHandleEnum_t::WMESEND),false);
	if (fd_wmeSend < 0)
	{
		cerr << argv[0] << ": failed create WMESEND socket" << endl;
    OS_Display << "Failed create WMESEND socket" << endl;
    OS_Display.close();
		close(fd_wmeListen);
		if (logFile)
		{
			OS_Aware.close();
			OS_CvStatus.close();
			OS_Art.close();
			OS_Wme.close();	
			OS_Payload.close();
			OS_Softcall.close();
			OS_PedCall.close();
			OS_Ownspat.close();
		}
		exit(1);
	}
	
	int fd_cloudListen = socketUtils::server(s_rsuConfig.getSocketAddr(RSUconfig::socketHandleEnum_t::CLOUDLISTEN),true);
	if (fd_cloudListen < 0)
	{
		cerr << argv[0] << ": failed create CLOUDLISTEN socket" << endl;
    OS_Display << "Failed create CLOUDLISTEN socket" << endl;
    OS_Display.close();    
		close(fd_wmeListen);
		close(fd_wmeSend);
		if (logFile)
		{
			OS_Aware.close();
			OS_CvStatus.close();
			OS_Art.close();
			OS_Wme.close();	
			OS_Payload.close();
			OS_Softcall.close();
			OS_PedCall.close();
			OS_Ownspat.close();
		}
		exit(1);
	}
	
	int fd_tciSend = socketUtils::client(s_rsuConfig.getSocketAddr(RSUconfig::socketHandleEnum_t::TCISEND),false);
	if (fd_tciSend < 0)
	{
		cerr << argv[0] << ": failed create TCISEND socket" << endl;
    OS_Display << "Failed create TCISEND socket" << endl;
    OS_Display.close();        
		close(fd_wmeListen);
		close(fd_wmeSend);
		close(fd_cloudListen);
		if (logFile)
		{
			OS_Aware.close();
			OS_CvStatus.close();
			OS_Art.close();
			OS_Wme.close();	
			OS_Payload.close();
			OS_Softcall.close();
			OS_PedCall.close();
			OS_Ownspat.close();
		}
		exit(1);
	}

	int fd_tciListen = socketUtils::server(s_rsuConfig.getSocketAddr(RSUconfig::socketHandleEnum_t::TCILISTEN),true);
	if (fd_tciListen < 0)
	{
		cerr << argv[0] << ": failed create TCILISTEN socket" << endl;
    OS_Display << "Failed create TCILISTEN socket" << endl;
    OS_Display.close();            
		close(fd_wmeListen);
		close(fd_wmeSend);
		close(fd_cloudListen);
		close(fd_tciSend);
		if (logFile)
		{
			OS_Aware.close();
			OS_CvStatus.close();
			OS_Art.close();
			OS_Wme.close();	
			OS_Payload.close();
			OS_Softcall.close();
			OS_PedCall.close();
			OS_Ownspat.close();
		}
		exit(1);
	}
  
	/* ----------- intercepts signals -------------------------------------*/
	if (setjmp(exit_env) != 0) 
	{
		cout << argv[0] << ": received user termination signal, quit!" << endl;
    OS_Display << "Received user termination signal, quit!" << endl;
    OS_Display.close();
		close(fd_wmeListen);
		close(fd_wmeSend);
		close(fd_cloudListen);
		close(fd_tciSend);
		close(fd_tciListen);
		if (logFile)
		{
			OS_Aware.close();
			OS_CvStatus.close();
			OS_Art.close();
			OS_Wme.close();	
			OS_Payload.close();
			OS_Softcall.close();
			OS_PedCall.close();
			OS_Ownspat.close();
		}
		return (0);
	}
	else
		sig_ign( sig_list, sig_hand );
		
	/* ----------- local variables -------------------------------------*/
	/// control parameters
	const double 		stopSpeed = 0.2; 			// in m/s
	const double		stopDist = 5.0;				// in meters, no need to re-do mapping when speed is lower than stopSpeed & distance travelled is smaller than stopDist 
	const useconds_t usleepms = 5000;			// in microseconds
	const long long artInterval = 1000LL;	// in milliseconds
	const long long vehPhaseCallInterval = 1000LL; 	// in milliseconds
	long long timeOutms = static_cast<long long>(s_rsuConfig.getDsrcTimeOut()) * 1000LL;										// in milliseconds
	uint16_t maxTime2goPhaseCall = static_cast<uint16_t>(s_rsuConfig.getMaxTime2goPhaseCall() * 10); 				// in deciseconds
	uint16_t maxTime2changePhaseExt = static_cast<uint16_t>(s_rsuConfig.getMaxTime2changePhaseExt() * 10); 	// in deciseconds
	uint16_t maxTime2phaseExt = static_cast<uint16_t>(s_rsuConfig.getMaxTime2phaseExt() * 10); 							// in deciseconds
	uint16_t maxGreenExtenstion = static_cast<uint16_t>(s_rsuConfig.getPhaseExtEvery() * 10);								// in deciseconds
	uint16_t phaseExtEvery = static_cast<uint16_t>(s_rsuConfig.getPhaseExtEvery()); 												// in deciseconds
	uint8_t	 syncPhase = static_cast<uint8_t>(s_rsuConfig.getSyncPhase());
	bitset<NEMAPHASES> coordinatedPhases(s_rsuConfig.getCoordinatedPhases());
    
	/// socket buffer
	char buf[MAXDSRCMSGSIZE];	
	ssize_t bytesReceived;
	ssize_t bytesSend;

	/// dsrc message decoding
	MSG_UNION_t	pkt_union;	
	BSM_List_t 	bsmIn;
	SRM_List_t	srmIn;
	SPAT_List_t spatIn;
	MAP_List_t 	mapIn;
	ART_List_t	artIn;

	/// maps to store latest inbound dsrc messages (SPaT, Map, ART from RSUs & BSM from other cars)
	map<uint32_t,BSM_List_t> 	bsmList;		// key vehId
	map<uint32_t,SRM_List_t>	srmList;		// key requested intersectionId
	map<uint32_t,SPAT_List_t> spatList;		// key intersectionId
	map<uint32_t,MAP_List_t> 	mapList;		// key intersectionId
	map<uint32_t,ART_List_t> 	artList;		// key intersectionId
	
	/// for tracking vehicle on map, locationAware, signalAware, phase call/extension status
	map<uint32_t,cvStatusAware_t> vehList;	// key vehId
	map<uint32_t,cvStatusAware_t>::iterator itVehList;
	cvStatusAware_t cvStatusAware;			
	GeoUtils::connectedVehicle_t cvIn;	
	GeoUtils::connectedVehicle_t cv;
	
	/// for tracking MMITSS signal control status data at this intersection
	mmitssSignalControlStatus_t signalControlStatus;
	signalControlStatus.reset();
	map<uint32_t,priorityRequestTableEntry_t>::iterator itPrsList;
	map<uint32_t,uint8_t>::iterator itPhaseExtVidMap;
	priorityRequestTableEntry_t prsTableEntry;

	/// structure to send soft-calls
	softcall_t s_softcall;
	bitset<NEMAPHASES> phases2call;

	/// for sending ART
	ART_element_t art2send;
	
	/// for tracking signal cycle count
	uint32_t prevSyncPhaseState = phasestate_enum_t::UNKNOWN;
	uint8_t cycleCtn = 0;
  
	/// for redirect cout to into ofstream
	streambuf * strm_buffer = cout.rdbuf();
		
	ownspat_t ownSpat;
  ownSpat.msgid = wmeUtils::msgid_spat_internal;
  SPAT_element_t rawSpat;
  
	while(1)
	{
		timeUtils::getFullTimeStamp(fullTimeStamp);
		/// check whether there is SRM comes in from the cloud server
		bytesReceived = recv(fd_cloudListen,buf,sizeof(buf),0);		
		if (bytesReceived > 0)
		{
			/// this should be SRM payload
			if (asnJ2735Lib.decode_payload(buf,bytesReceived,&pkt_union,NULL))
			{
				/// decoding succeeded, log payload
				cout.rdbuf(OS_Payload.rdbuf());
				logPayload(fullTimeStamp.localDateTimeStamp,buf,bytesReceived,pkt_union.type);
				cout.rdbuf(strm_buffer);
				l_payloadLogRows++;
				
				if (pkt_union.type == msg_enum_t::SRM_PKT && pkt_union.srm.signalRequest_element.id == intersectionId)
				{        
					uint8_t pedRequestPhase = asnJ2735Lib.getControlPhaseByLaneId(pkt_union.srm.signalRequest_element.id,pkt_union.srm.signalRequest_element.inLaneId);
          if (pedRequestPhase >= 1 && pedRequestPhase <= NEMAPHASES)
          {
            OS_Display << "Received pedestrian BSM call on intersection " << intersectionName << ", phase " << static_cast<int>(pedRequestPhase) << endl;
            if (verbose)
            {
              cout << timeUtils::getTimestampStr(fullTimeStamp.localDateTimeStamp) << ",";
              cout << " received pedestrian BSM call on intersection " << intersectionName << ", phase " << static_cast<int>(pedRequestPhase) << endl;
            }
            if (logFile)
            {
              OS_PedCall << timeUtils::getTimestampStr(fullTimeStamp.localDateTimeStamp) << ",";
              OS_PedCall << " received pedestrian BSM call on intersection " << intersectionName << ", phase " << static_cast<int>(pedRequestPhase) << endl;
              l_pedcallLogRows++;
            }
						/// this is a ped phase call request, send soft-call right away
						s_softcall.reset();
						s_softcall.ms_since_midnight = fullTimeStamp.ms_since_midnight;
						s_softcall.softcallPhase = static_cast<unsigned char>(pedRequestPhase);
						s_softcall.softcallObj = static_cast<unsigned char>(softcall_enum_t::PED);
						s_softcall.softcallType = static_cast<unsigned char>(softcall_enum_t::CALL);
						s_softcall.softcallNum = 1;
            if (sendSoftcall)
              socketUtils::sendall(fd_tciSend,(char*)&s_softcall,sizeof(s_softcall));
            OS_Display << "*************************************************" << endl;
            OS_Display << "***Sending pedestrian phase call on phase " << static_cast<int>(pedRequestPhase) << "**********" << endl;
            OS_Display << "*************************************************" << endl;
						if (verbose)
						{
							logSoftcall(fullTimeStamp.localDateTimeStamp,s_softcall,"soft priority cancel call");
						}
						if (logFile)
						{
							cout.rdbuf(OS_Softcall.rdbuf());
							logSoftcall(fullTimeStamp.localDateTimeStamp,s_softcall,"soft priority cancel call");
							cout.rdbuf(strm_buffer);
							l_softcallLogRows++;
						}
					}
				}
			}
		}
		
		timeUtils::getFullTimeStamp(fullTimeStamp);
		/// check whether there is raw spat comes
		bytesReceived = recv(fd_tciListen,&(rawSpat),sizeof(SPAT_element_t),0);
		if (bytesReceived > 0)
		{	
      /// fill in intersection id as rawSpat used intersection name as the computer host name
      rawSpat.id = intersectionId;
      /// signal status in rawSpat is in Battelle's format while decoded SPAT_element_t converted
      /// signal state to ASN.1 format. Encode and decode rawSpat to have the input being consistent
      bytesSend = asnJ2735Lib.encode_spat_payload(&rawSpat, buf, sizeof(buf),false);
      asnJ2735Lib.decode_spat_payload(buf,bytesSend,&rawSpat,NULL);
      memcpy(&(ownSpat.spat),&(rawSpat),sizeof(SPAT_element_t));      
      /// log payload
      cout.rdbuf(OS_Payload.rdbuf());
      logPayload(fullTimeStamp.localDateTimeStamp,buf,bytesSend,msg_enum_t::SPAT_PKT);
      cout.rdbuf(strm_buffer);
      l_payloadLogRows++;      
			if (verbose)
			{
				cout << timeUtils::getTimestampStr(fullTimeStamp.localDateTimeStamp);
				cout << " received SPaT, msgId " << static_cast<int>(ownSpat.msgid) << endl;
			}		
			if (logFile)
			{
				OS_Ownspat << timeUtils::getTimestampStr(fullTimeStamp.localDateTimeStamp);
				OS_Ownspat << " received SPaT, msgId " << static_cast<int>(ownSpat.msgid) << endl;
				l_ownspatLogRows++;
			}		
	
			if (ownSpat.msgid == wmeUtils::msgid_spat_internal)
			{
				/// update spatList
        spatIn.tms = fullTimeStamp.tms;
				memcpy(&(spatIn.spatMsg),&(ownSpat.spat),sizeof(SPAT_element_t));									
				updateSpatList(spatList,spatIn);
				/// update cycleCtn
				if (prevSyncPhaseState == phasestate_enum_t::UNKNOWN)
					prevSyncPhaseState = ownSpat.spat.phaseState[syncPhase-1].currState;
				if (ownSpat.spat.phaseState[syncPhase-1].currState != prevSyncPhaseState)
				{
					if (ownSpat.spat.phaseState[syncPhase-1].currState == phasestate_enum_t::RED)
          {
						cycleCtn = (uint8_t)((cycleCtn + 1) % 128);
            OS_Display << "cycleCtn = " << static_cast<int>(cycleCtn) << endl;
          }
					prevSyncPhaseState = ownSpat.spat.phaseState[syncPhase-1].currState;
				}	
			}
		}
		
		/// check whether there is dsrc msgs comes in (BSM & SRM from OBUS, SPAT, MAP, ART from nearby intersections)
		bytesReceived = recv(fd_wmeListen,buf,sizeof(buf),0);		
		if (bytesReceived > 0)
		{
			/// decode payload
			if (asnJ2735Lib.decode_payload(buf,bytesReceived,&pkt_union,NULL))
			{
				/// decoding succeeded, log payload
        if (pkt_union.type != msg_enum_t::MAP_PKT)
        {
          cout.rdbuf(OS_Payload.rdbuf());
          logPayload(fullTimeStamp.localDateTimeStamp,buf,bytesReceived,pkt_union.type);
          cout.rdbuf(strm_buffer);
          l_payloadLogRows++;
        }
				/// update tracking lists
				switch(pkt_union.type)
				{
				case msg_enum_t::BSM_PKT:
          bsmIn.tms = fullTimeStamp.tms;
					memcpy(&(bsmIn.bsmMsg),&(pkt_union.bsm),sizeof(BSM_element_t));
					updateBsmList(bsmList,bsmIn);
					if (logFile)
					{
						OS_Wme << timeUtils::getTimestampStr(fullTimeStamp.localDateTimeStamp);
						OS_Wme << " received BSM from vehicle: " << bsmIn.bsmMsg.bolb1_element.id;
						OS_Wme	<< ", msgCnt: " << static_cast<int>(bsmIn.bsmMsg.bolb1_element.msgCnt);
						OS_Wme	<< endl;
						l_wmeLogRows++;
					}
					break;
				case msg_enum_t::SRM_PKT:
					asnJ2735Lib.decode_bsmblob1_payload((u_char*)pkt_union.srm.BSMblob,&(srmIn.srmBlob1));
          srmIn.tms = fullTimeStamp.tms;
					memcpy(&(srmIn.srmMsg),&(pkt_union.srm),sizeof(SRM_element_t));
					srmIn.isCancel = false;
					if (srmIn.srmMsg.signalRequest_element.requestedAction == priorityrequest_enum_t::CANCELPRIORITY 
						|| srmIn.srmMsg.signalRequest_element.requestedAction == priorityrequest_enum_t::CANCELPREEMP)
					{
						srmIn.isCancel = true;
					}					
					srmIn.requestedPhase = asnJ2735Lib.getControlPhaseByLaneId(srmIn.srmMsg.signalRequest_element.id,srmIn.srmMsg.signalRequest_element.inLaneId);
					srmIn.requestedServiceStartTms = fullTimeStamp.tms_midnight + static_cast<long long>(srmIn.srmMsg.timeOfService.sec)
						+ (static_cast<long long>(srmIn.srmMsg.timeOfService.hour) * 3600LL + static_cast<long long>(srmIn.srmMsg.timeOfService.min) * 60LL) * 1000LL;
					srmIn.requestedServiceEndTms = fullTimeStamp.tms_midnight + static_cast<long long>(srmIn.srmMsg.endOfService.sec)
						+ (static_cast<long long>(srmIn.srmMsg.endOfService.hour) * 3600LL + static_cast<long long>(srmIn.srmMsg.endOfService.min) * 60LL) * 1000LL;					
					updateSrmList(srmList,srmIn);	
					if (logFile)
					{
						OS_Wme << timeUtils::getTimestampStr(fullTimeStamp.localDateTimeStamp);
						OS_Wme << " received SRM from vehicle: " << srmIn.srmBlob1.id;
						OS_Wme	<< ", msgCnt: " << static_cast<int>(srmIn.srmMsg.msgCnt);
						OS_Wme << ", request for intersection: " << asnJ2735Lib.getIntersectionNameById(srmIn.srmMsg.signalRequest_element.id);
						OS_Wme << " on phase: " << static_cast<int>(srmIn.requestedPhase);
						OS_Wme << " action: " << static_cast<int>(srmIn.srmMsg.signalRequest_element.requestedAction);
						OS_Wme << endl;
						l_wmeLogRows++;
					}
					break;
				case msg_enum_t::SPAT_PKT:
          spatIn.tms = fullTimeStamp.tms;
					memcpy(&(spatIn.spatMsg),&(pkt_union.spat),sizeof(SPAT_element_t));					
					updateSpatList(spatList,spatIn);
					if (logFile)
					{
						OS_Wme << timeUtils::getTimestampStr(fullTimeStamp.localDateTimeStamp);
						OS_Wme << " received SPaT from intersection: " << asnJ2735Lib.getIntersectionNameById(spatIn.spatMsg.id);
						OS_Wme << endl;
						l_wmeLogRows++;
					}
					break;	
				case msg_enum_t::MAP_PKT:
					mapIn.tms = fullTimeStamp.tms;
					memcpy(&(mapIn.mapMsg),&(pkt_union.mapdata),sizeof(Mapdata_element_t));
					mapList[pkt_union.mapdata.id] = mapIn;	// insert if new or overwrite existing entry
					if (logFile)
					{
						OS_Wme << timeUtils::getTimestampStr(fullTimeStamp.localDateTimeStamp);
						OS_Wme << " received Map from intersection: " << asnJ2735Lib.getIntersectionNameById(mapIn.mapMsg.id);
						OS_Wme	<< ", mapVersion: " << static_cast<int>(mapIn.mapMsg.mapVersion);
						OS_Wme << endl;
						l_wmeLogRows++;
					}					
					break;
				case msg_enum_t::ART_PKT:
          artIn.tms = fullTimeStamp.tms;
					memcpy(&(artIn.artMsg),&(pkt_union.art),sizeof(ART_element_t));	
					updateArtList(artList,artIn);					
					break;					
				default:
					/// should not be here
					break;
				}
				
				/// deal with BSM messages to update vehList
				if (pkt_union.type == msg_enum_t::BSM_PKT)
				{
					/// get cvIn geo location from bms (id,ts,geoPoint,motionState)
					cvIn.reset();						
					cvIn.id = pkt_union.bsm.bolb1_element.id;
          cvIn.ts = fullTimeStamp.tms;
					cvIn.geoPoint.latitude = pkt_union.bsm.bolb1_element.lat;
					cvIn.geoPoint.longitude = pkt_union.bsm.bolb1_element.lon;
					cvIn.geoPoint.elevation = pkt_union.bsm.bolb1_element.elev;
					cvIn.motionState.speed = pkt_union.bsm.bolb1_element.speed;
					cvIn.motionState.heading = pkt_union.bsm.bolb1_element.heading;
					/// get last cvStatusAware (map mapping result, priority request and soft-call status)
					itVehList = vehList.find(cvIn.id);
					if (itVehList != vehList.end())
					{
						// get cvStatus from vehList
						cvStatusAware = itVehList->second;
					}
					else
					{
						// add to vehList
						cvStatusAware.reset();
						cvStatusAware.cvStatus.push_back(cvIn);
						vehList[cvIn.id] = cvStatusAware;
						if (logFile)
						{
							// log vehicle state
							OS_CvStatus << getTimestampStr(fullTimeStamp.localDateTimeStamp);
							OS_CvStatus << " add vehicle " << cvIn.id << " to vehList";
							OS_CvStatus	<< endl; 
							l_cvstatusLogRows++;
						}
            OS_Display << "Adding vehicle " << cvIn.id << " on vehList" << endl;
					}
					/// get latest map matching result
					cv = cvStatusAware.cvStatus.back();
					/// determine whether it's an outdated BSM (it's UDP)					
					bool proceed;
					if (cvIn.ts < cv.ts)
						proceed = false;	// outdated cvIn
					else
						proceed = true;
					/// process new BSM
					if (proceed)
					{
						/// get distance travelled from the last geoPoint
						double distTravelled = GeoUtils::distlla2lla(cvIn.geoPoint,cv.geoPoint);
						/// check whether need to locate the vehicle on map
						bool doMapping = true;
						if (cvIn.motionState.speed  < stopSpeed && fabs(distTravelled) < stopDist)
						{
							doMapping = false;
						}
						/// update cvIn geo location (id, ts, geoPoint & motionState) to cv
						cv.id = cvIn.id;
						cv.ts = cvIn.ts;
						/// keep same geoPoint & motionState if not doing mapping
						if(doMapping)
						{
							cv.geoPoint = cvIn.geoPoint;
							cv.motionState = cvIn.motionState;
						}					
						/// do mapping to get current cv mapping result (isVehicleInMap & vehicleTrackingState)
						if (!doMapping)
						{
							// keep the same mapping info. 
							// Note that cv.ts has been updated but cv.geoPoint & cv.motionState remained as the last record,
							//	to ensure working for the case that vehicle is moving in geoPoint but speed is low (seen this on OBU pcap file)
							cvIn.isVehicleInMap = cv.isVehicleInMap;
							cvIn.vehicleTrackingState = cv.vehicleTrackingState;
							cvIn.vehicleLocationAware = cv.vehicleLocationAware;
						}
						else
						{
							// call locateVehicleInMap
							// 	cvIn & cv have the same ts, geoPoint & motionState but different isVehicleInMap & vehicleTrackingState
							//	cv consonants the last mapping result & cvIn is going to get the current mapping result
							cvIn.isVehicleInMap = asnJ2735Lib.locateVehicleInMap(cv,cvIn.vehicleTrackingState); 	
							// convert mapping result cvIn to locationAware & signalAware
							asnJ2735Lib.updateLocationAware(cvIn.vehicleTrackingState,cvIn.vehicleLocationAware);
						}
            updateSignalAware(spatList,cvIn.vehicleLocationAware,cvIn.vehicleSignalAware);
						/// log tracking output (not payload)
#if 0            
            OS_Display << "vid=" << cvIn.id << ",";
            OS_Display << "status=" <<static_cast<int>(cvIn.vehicleTrackingState.intsectionTrackingState.vehicleIntersectionStatus) << ",";
            OS_Display << "id=" << cvIn.vehicleLocationAware.intersectionId << ",";
            OS_Display << "lane=" << static_cast<int>(cvIn.vehicleLocationAware.laneId) << ",";
            OS_Display << "dist=" << static_cast<int>(cvIn.vehicleLocationAware.dist2go.distLong) << ",";
            OS_Display << "sigState=" << static_cast<int>(cv.vehicleSignalAware.currState) << ",";
            OS_Display << "time2change=" << static_cast<int>(cv.vehicleSignalAware.timeToChange) << ",";
            OS_Display << "nextState=" << static_cast<int>(cv.vehicleSignalAware.nextState) << endl;
#endif            
						if (verbose)
						{
							logAware(fullTimeStamp.localDateTimeStamp,pkt_union.bsm.bolb1_element.msgCnt,cvIn);
						}			
						if (logFile)
						{
							cout.rdbuf(OS_Aware.rdbuf());
							logAware(fullTimeStamp.localDateTimeStamp,pkt_union.bsm.bolb1_element.msgCnt,cvIn);
							cout.rdbuf(strm_buffer);
							l_awreLogRows++;
						}
            
            if (!cvStatusAware.isOnApproach && cvIn.isVehicleInMap // in one of the intersection map
							&& cvIn.vehicleTrackingState.intsectionTrackingState.intersectionIndex == intersectionIndex // at this intersection						
							&& cvIn.vehicleTrackingState.intsectionTrackingState.vehicleIntersectionStatus == GeoUtils::vehicleInMap_enum_t::ON_APPROACH)
            {
              OS_Display << " Vehicle " << cvIn.id << " is approaching " << endl;
            } 
						/// update vehList with the current mapping result			
						if (cvStatusAware.isOnApproach) 
						{
							/// previous on an ingress of this intersection, possible current vehicleIntersectionStatus:
							///		case 1: on the same ingress (ON_APPROACH or moved into AT_INTERSECTION_BOX)
							if (cvIn.isVehicleInMap // in one of the intersection map 
								&& cvIn.vehicleTrackingState.intsectionTrackingState.intersectionIndex == intersectionIndex // at this intersection
								&& (cvIn.vehicleTrackingState.intsectionTrackingState.vehicleIntersectionStatus == GeoUtils::vehicleInMap_enum_t::ON_APPROACH
									|| cvIn.vehicleTrackingState.intsectionTrackingState.vehicleIntersectionStatus == GeoUtils::vehicleInMap_enum_t::AT_INTERSECTION_BOX)
								&& cvIn.vehicleTrackingState.intsectionTrackingState.approachIndex == cv.vehicleTrackingState.intsectionTrackingState.approachIndex) // on the same ingress
							{
								// vehicleIntersectionStatus remains the same (treat AT_INTERSECTION_BOX as part of ON_APPROACH)
								if (logFile)
								{
									OS_CvStatus << getTimestampStr(fullTimeStamp.localDateTimeStamp);
									OS_CvStatus << " vehicle " << cvIn.id << " at " << intersectionName;
									OS_CvStatus << " on approach " << static_cast<int>(cvIn.vehicleTrackingState.intsectionTrackingState.approachIndex + 1);
									OS_CvStatus << ", dist2stopbar = " << cvIn.vehicleLocationAware.dist2go.distLong;
									OS_CvStatus << ", control phase = " << static_cast<int>(cvIn.vehicleLocationAware.controlPhase);
									OS_CvStatus << ", status = (" << static_cast<int>(cvIn.vehicleSignalAware.currState) << ",";
									OS_CvStatus << cvIn.vehicleSignalAware.timeToChange << ",";
									OS_CvStatus << static_cast<int>(cvIn.vehicleSignalAware.stateConfidence) << ",";
									OS_CvStatus << static_cast<int>(cvIn.vehicleSignalAware.nextState) << ",";
									OS_CvStatus << cvIn.vehicleSignalAware.clearanceIntv << ",";
									OS_CvStatus << static_cast<int>(cvIn.vehicleSignalAware.yellStateConfidence) << ")";
									OS_CvStatus << endl;
									l_cvstatusLogRows++;
								}
								// update vehList
								cvStatusAware.cvStatus.push_back(cvIn);
								vehList[cvIn.id] = cvStatusAware;
							}
							///	case 2: still at this intersection but changed approach (moves to egress)
							else if (cvIn.isVehicleInMap // in one of the intersection map
								&& cvIn.vehicleTrackingState.intsectionTrackingState.intersectionIndex == intersectionIndex // at this intersection
								&& cvIn.vehicleTrackingState.intsectionTrackingState.vehicleIntersectionStatus == GeoUtils::vehicleInMap_enum_t::ON_EGRESS
								&& cvIn.vehicleTrackingState.intsectionTrackingState.approachIndex != cv.vehicleTrackingState.intsectionTrackingState.approachIndex) // changed approach
							{
								cvStatusAware.isOnApproach = false;
								if (logFile)
								{
									// log vehicleIntersectionStatus change (later on need to form travel time message to perform observer)
									OS_CvStatus << getTimestampStr(fullTimeStamp.localDateTimeStamp);
									OS_CvStatus << " vehicle " << cvIn.id << " at " << intersectionName;
									OS_CvStatus << " leaved approach " << static_cast<int>(cv.vehicleTrackingState.intsectionTrackingState.approachIndex + 1);
									OS_CvStatus << " onto approach " << static_cast<int>(cvIn.vehicleTrackingState.intsectionTrackingState.approachIndex + 1);
									OS_CvStatus << ", dist2stopbar = " << cvIn.vehicleLocationAware.dist2go.distLong;
									OS_CvStatus << ", control phase = " << static_cast<int>(cvIn.vehicleLocationAware.controlPhase);
									OS_CvStatus << ", status = (" << static_cast<int>(cvIn.vehicleSignalAware.currState) << ",";
									OS_CvStatus << cvIn.vehicleSignalAware.timeToChange << ",";
									OS_CvStatus << static_cast<int>(cvIn.vehicleSignalAware.stateConfidence) << ",";
									OS_CvStatus << static_cast<int>(cvIn.vehicleSignalAware.nextState) << ",";
									OS_CvStatus << cvIn.vehicleSignalAware.clearanceIntv << ",";
									OS_CvStatus << static_cast<int>(cvIn.vehicleSignalAware.yellStateConfidence) << ")";
									OS_CvStatus << endl;
									l_cvstatusLogRows++;
								}
								// update vehList
								cvStatusAware.cvStatus.clear();
								cvStatusAware.cvStatus.push_back(cvIn);
								vehList[cvIn.id] = cvStatusAware;
							}
							/// case 3: changed intersection (moved to ingress of the next intersection)
							else if (cvIn.isVehicleInMap // in one of the intersection map
								&& cvIn.vehicleTrackingState.intsectionTrackingState.intersectionIndex != intersectionIndex) // changed intersection
							{
								cvStatusAware.isOnApproach = false;
								if (logFile)
								{							
									// log vehicleIntersectionStatus change (later on need to form travel time message to perform observer)
									OS_CvStatus << getTimestampStr(fullTimeStamp.localDateTimeStamp);					
									OS_CvStatus << " vehicle " << cvIn.id << " at " << intersectionName;
									OS_CvStatus << " leaved approach " << static_cast<int>(cv.vehicleTrackingState.intsectionTrackingState.approachIndex + 1);
									OS_CvStatus << " onto intersection " << asnJ2735Lib.getIntersectionNameByIndex(cvIn.vehicleTrackingState.intsectionTrackingState.intersectionIndex);
									OS_CvStatus << endl;
									l_cvstatusLogRows++;
								}
								// update vehList
								cvStatusAware.cvStatus.clear();
								cvStatusAware.cvStatus.push_back(cvIn);
								vehList[cvIn.id] = cvStatusAware;
							}
							/// case 4: NOT_IN_MAP (moved outside of the map)
							else if (!cvIn.isVehicleInMap)
							{
								cvStatusAware.isOnApproach = false;
								if (logFile)
								{							
									// log vehicleIntersectionStatus change (later on need to form travel time message to perform observer)
									OS_CvStatus << getTimestampStr(fullTimeStamp.localDateTimeStamp);						
									OS_CvStatus << " vehicle " << cvIn.id << " at " << intersectionName;
									OS_CvStatus << " leaved approach " << static_cast<int>(cv.vehicleTrackingState.intsectionTrackingState.approachIndex + 1);
									OS_CvStatus << " and moves outside the map";
									OS_CvStatus << endl;
									l_cvstatusLogRows++;
								}
								// update vehList
								cvStatusAware.cvStatus.clear();
								cvStatusAware.cvStatus.push_back(cvIn);
								vehList[cvIn.id] = cvStatusAware;
							}
						}
						///	previous not on any ingress of this intersection, current on an ingress of this intersection
						else if (cvIn.isVehicleInMap // in one of the intersection map
							&& cvIn.vehicleTrackingState.intsectionTrackingState.intersectionIndex == intersectionIndex // at this intersection						
							&& cvIn.vehicleTrackingState.intsectionTrackingState.vehicleIntersectionStatus == GeoUtils::vehicleInMap_enum_t::ON_APPROACH) // on an ingress of this intersection
						{
							// vehicle first enters an approach of this intersection
							cvStatusAware.isOnApproach = true;
							if (logFile)
							{							
								// log vehicleIntersectionStatus change
								OS_CvStatus << getTimestampStr(fullTimeStamp.localDateTimeStamp);						
								OS_CvStatus << " vehicle " << cvIn.id << " enters " << intersectionName;
								OS_CvStatus << " from approach " << static_cast<int>(cvIn.vehicleTrackingState.intsectionTrackingState.approachIndex + 1);
								OS_CvStatus << " on lane " << static_cast<int>(cvIn.vehicleTrackingState.intsectionTrackingState.laneIndex + 1);
								OS_CvStatus << ", dist2stopbar = " << cvIn.vehicleLocationAware.dist2go.distLong;
								OS_CvStatus << ", control phase = " << static_cast<int>(cvIn.vehicleLocationAware.controlPhase);
								OS_CvStatus << ", status = (" << static_cast<int>(cvIn.vehicleSignalAware.currState) << ",";
								OS_CvStatus << cvIn.vehicleSignalAware.timeToChange << ",";
								OS_CvStatus << static_cast<int>(cvIn.vehicleSignalAware.stateConfidence) << ",";
								OS_CvStatus << static_cast<int>(cvIn.vehicleSignalAware.nextState) << ",";
								OS_CvStatus << cvIn.vehicleSignalAware.clearanceIntv << ",";
								OS_CvStatus << static_cast<int>(cvIn.vehicleSignalAware.yellStateConfidence) << ")";
								OS_CvStatus	<< endl;
								l_cvstatusLogRows++;
							}
							// update vehList
							cvStatusAware.cvStatus.clear();
							cvStatusAware.cvStatus.push_back(cvIn);
							vehList[cvIn.id] = cvStatusAware;
						}
						///	both previous and current are not on any ingress of this intersection. 
						else
						{
							// only log when cvstatus changed
							if (logFile && !(cvIn.vehicleTrackingState.intsectionTrackingState == cv.vehicleTrackingState.intsectionTrackingState))
							{
								OS_CvStatus << getTimestampStr(fullTimeStamp.localDateTimeStamp);		
								OS_CvStatus << " vehicle " << cvIn.id << " changed intersection status:";
								OS_CvStatus << " vehicleIntersectionStatus = " << static_cast<int>(cvIn.vehicleTrackingState.intsectionTrackingState.vehicleIntersectionStatus);
								OS_CvStatus << " intersectionIndex = " << static_cast<int>(cvIn.vehicleTrackingState.intsectionTrackingState.intersectionIndex + 1);
								OS_CvStatus << " approachIndex = " << static_cast<int>(cvIn.vehicleTrackingState.intsectionTrackingState.approachIndex + 1);
								OS_CvStatus << " laneIndex = " << static_cast<int>(cvIn.vehicleTrackingState.intsectionTrackingState.laneIndex + 1);
								OS_CvStatus	<< endl;
								l_cvstatusLogRows++;
							}
							// update vehList
							cvStatusAware.cvStatus.clear();
							cvStatusAware.cvStatus.push_back(cvIn);
							vehList[cvIn.id] = cvStatusAware;
						}
					}          
				}
				
				/// deal with SRM messages that request priority at this intersection to update prsList
				if (pkt_union.type == msg_enum_t::SRM_PKT && srmIn.srmMsg.signalRequest_element.id == intersectionId)
				{          
					// srmIn maintains the latest SRM data
					itPrsList = signalControlStatus.prsList.find(srmIn.srmBlob1.id);
					if (itPrsList == signalControlStatus.prsList.end())
					{
						// this is a new SRM
						prsTableEntry.requestVehId = srmIn.srmBlob1.id;
						prsTableEntry.msgCnt = srmIn.srmMsg.msgCnt;
						prsTableEntry.requestUpdateStatus = priorityRequestServer_enum_t::INITIATE;
						prsTableEntry.initiatedTms = srmIn.tms;
						prsTableEntry.receivedTms = prsTableEntry.initiatedTms;
						prsTableEntry.requestedServiceStartTms = srmIn.requestedServiceStartTms;
						prsTableEntry.requestedServiceEndTms = srmIn.requestedServiceEndTms;
						prsTableEntry.requestPhase = srmIn.requestedPhase;
						prsTableEntry.requestedAction = srmIn.srmMsg.signalRequest_element.requestedAction;
						prsTableEntry.requestInLaneId = srmIn.srmMsg.signalRequest_element.inLaneId;
						prsTableEntry.requestOutLaneId = srmIn.srmMsg.signalRequest_element.outLaneId;
						prsTableEntry.requestVehicleClassType = srmIn.srmMsg.signalRequest_element.NTCIPVehicleclass.NTCIPvehicleClass_type;
						prsTableEntry.requestVehicleClassLevel = srmIn.srmMsg.signalRequest_element.NTCIPVehicleclass.NTCIPvehicleClass_level;
						prsTableEntry.requestVehicleTransitStatus = srmIn.srmMsg.transitStatus;
						memcpy(&(prsTableEntry.requestTimeOfService),&(srmIn.srmMsg.timeOfService),sizeof(DTime_element_t));
						memcpy(&(prsTableEntry.requestEndOfService),&(srmIn.srmMsg.endOfService),sizeof(DTime_element_t));
						// determine requestStatus
						itVehList = vehList.find(prsTableEntry.requestVehId);         
            if (itVehList == vehList.end() 						// not on vehList
							|| !(itVehList->second.isOnApproach)		// not on approach of this intersection
//							|| prsTableEntry.requestedServiceEndTms < fullTimeStamp.tms // requestedServiceEndTms expired
							|| srmIn.isCancel												// cancel of nothing
							|| srmIn.srmMsg.signalRequest_element.requestedAction != priorityrequest_enum_t::REQUESTPRIORITY) // not request priority 
						{
							prsTableEntry.requestStatus = priorityrequest_enum_t::NOTVALID;
						}
						else if (!coordinatedPhases.test(prsTableEntry.requestPhase - 1))			// request on non-coordinated phase
							prsTableEntry.requestStatus = priorityrequest_enum_t::REJECTED;
						else if (itVehList->second.cvStatus.back().vehicleSignalAware.currState == phasestate_enum_t::GREEN // request phase in green
							&& getTime2goDeci(itVehList->second.cvStatus.back().vehicleLocationAware.dist2go.distLong,
								itVehList->second.cvStatus.back().motionState.speed) < itVehList->second.cvStatus.back().vehicleSignalAware.timeToChange)	// can pass in green
							prsTableEntry.requestStatus = priorityrequest_enum_t::NOTNEEDED;
						else
							prsTableEntry.requestStatus = priorityrequest_enum_t::QUEUED;
						// add to prsList
						signalControlStatus.prsList[srmIn.srmBlob1.id] = prsTableEntry;
            if(!srmIn.isCancel)
            {
              OS_Display << "received SRM request from vehicle " << srmIn.srmBlob1.id << " on phase " << static_cast<int>(srmIn.requestedPhase) << endl;
              OS_Display << "adding SRM to PRS list" << endl;          
            }
					}
					else if (itPrsList->second.msgCnt == srmIn.srmMsg.msgCnt)
					{
						itPrsList->second.requestUpdateStatus = priorityRequestServer_enum_t::MAINTAIN;
					}
					else
					{
						// this is an update of existing SRM
						itPrsList->second.msgCnt = srmIn.srmMsg.msgCnt;
						itPrsList->second.requestUpdateStatus = priorityRequestServer_enum_t::UPDATE;
						itPrsList->second.receivedTms = srmIn.tms;
						itPrsList->second.requestedServiceStartTms = srmIn.requestedServiceStartTms;
						itPrsList->second.requestedServiceEndTms = srmIn.requestedServiceEndTms;
						itPrsList->second.requestPhase = srmIn.requestedPhase;
						itPrsList->second.requestedAction = srmIn.srmMsg.signalRequest_element.requestedAction;
						itPrsList->second.requestInLaneId = srmIn.srmMsg.signalRequest_element.inLaneId;
						itPrsList->second.requestOutLaneId = srmIn.srmMsg.signalRequest_element.outLaneId;
						itPrsList->second.requestVehicleClassType = srmIn.srmMsg.signalRequest_element.NTCIPVehicleclass.NTCIPvehicleClass_type;
						itPrsList->second.requestVehicleClassLevel = srmIn.srmMsg.signalRequest_element.NTCIPVehicleclass.NTCIPvehicleClass_level;
						itPrsList->second.requestVehicleTransitStatus = srmIn.srmMsg.transitStatus;
						memcpy(&(itPrsList->second.requestTimeOfService),&(srmIn.srmMsg.timeOfService),sizeof(DTime_element_t));
						memcpy(&(itPrsList->second.requestEndOfService),&(srmIn.srmMsg.endOfService),sizeof(DTime_element_t));
						// determine requestStatus
						priorityrequest_enum_t::requeststatus prevRequestStatus = itPrsList->second.requestStatus;
						itVehList = vehList.find(itPrsList->second.requestVehId);
						switch(prevRequestStatus)
						{
						case priorityrequest_enum_t::ACTIVE:
							// it was the cause of the ongoing signal priority treatment, possible next requestStatus: COMPLETED or CANCELLED
							if (itVehList == vehList.end() || !(itVehList->second.isOnApproach))	// moved out from the approach
								itPrsList->second.requestStatus = priorityrequest_enum_t::COMPLETED;
							else if (srmIn.isCancel)																							// vehicle requested cancel
								itPrsList->second.requestStatus = priorityrequest_enum_t::CANCELLED;
							break;
						default:
							// for other cases, next requestStatus is determined the same as INITIATE state
							if (itVehList == vehList.end() 						// not on vehList
								|| !(itVehList->second.isOnApproach)		// not on approach of this intersection
//								|| itPrsList->second.requestedServiceEndTms < fullTimeStamp.tms // requestedServiceEndTms expired
								|| srmIn.isCancel												// cancel of nothing
								|| srmIn.srmMsg.signalRequest_element.requestedAction != priorityrequest_enum_t::REQUESTPRIORITY) // not request priority 
							{
								itPrsList->second.requestStatus = priorityrequest_enum_t::NOTVALID;
							}
							else if (!coordinatedPhases.test(itPrsList->second.requestPhase - 1))			// request on non-coordinated phase
								itPrsList->second.requestStatus = priorityrequest_enum_t::REJECTED;
							else if (itVehList->second.cvStatus.back().vehicleSignalAware.currState == phasestate_enum_t::GREEN // request phase in green
								&& getTime2goDeci(itVehList->second.cvStatus.back().vehicleLocationAware.dist2go.distLong,
									itVehList->second.cvStatus.back().motionState.speed) < itVehList->second.cvStatus.back().vehicleSignalAware.timeToChange)	// can pass in green
								itPrsList->second.requestStatus = priorityrequest_enum_t::NOTNEEDED;
							else
								itPrsList->second.requestStatus = priorityrequest_enum_t::QUEUED;
						}
            if(srmIn.isCancel)
            {
              OS_Display << "received cancel SRM request from vehicle " << srmIn.srmBlob1.id << endl;
            }
					}
				}
			}	// end if decode_payload
		} // end if bytesReceived
				
		/// check signal priority call: check cancel first
		if (signalControlStatus.prsAction != priorityRequestServer_enum_t::NONE)
		{
			/// get SPaT info for this intersection		
			spatIn = getSpatFromList(spatList,intersectionId);
			if (spatIn.tms > 0)
			{
				phases2call.reset();		
				/// get vehicle info for the priority vehicle
				itVehList = vehList.find(signalControlStatus.priorityVehicleId);
				if ((signalControlStatus.prsAction == priorityRequestServer_enum_t::EARLYGREEN
							&& spatIn.spatMsg.phaseState[signalControlStatus.callPhase - 1].currState == phasestate_enum_t::GREEN) 	// phase turned green
					|| (signalControlStatus.prsAction == priorityRequestServer_enum_t::GREENEXTENSION
							&& spatIn.spatMsg.phaseState[signalControlStatus.callPhase - 1].currState != phasestate_enum_t::GREEN)	// green expired
					|| (itVehList == vehList.end() || !itVehList->second.isOnApproach))																					// priority vehicle passed
				{
					phases2call.set(signalControlStatus.callPhase - 1,1);					
					signalControlStatus.prsAction = priorityRequestServer_enum_t::NONE;
					signalControlStatus.callPhase = 0;
					signalControlStatus.priorityVehicleId = 0;
					itPrsList = signalControlStatus.prsList.find(signalControlStatus.priorityVehicleId);
					if (itPrsList != signalControlStatus.prsList.end())
					{
						itPrsList->second.requestStatus = priorityrequest_enum_t::COMPLETED;
					}
				}	
				if (phases2call.any())
				{
					/// send cancel priority call
					timeUtils::getFullTimeStamp(fullTimeStamp);		
					s_softcall.reset();
					s_softcall.ms_since_midnight = fullTimeStamp.ms_since_midnight;
					s_softcall.softcallPhase = static_cast<unsigned char>(phases2call.to_ulong());
					s_softcall.softcallObj = static_cast<unsigned char>(softcall_enum_t::PRIORITY);
					s_softcall.softcallType = static_cast<unsigned char>(softcall_enum_t::CANCEL);
					s_softcall.softcallNum = 1;
          if (sendSoftcall)
            socketUtils::sendall(fd_tciSend,(char*)&s_softcall,sizeof(s_softcall));
          OS_Display << "*************************************************" << endl;
          OS_Display << "***Sending cancel priority call on phase " << phases2call.to_string() << "**********" << endl;
          OS_Display << "*************************************************" << endl;
					if (verbose)
					{
						logSoftcall(fullTimeStamp.localDateTimeStamp,s_softcall,"soft priority cancel call");
					}
					if (logFile)
					{
						cout.rdbuf(OS_Softcall.rdbuf());
						logSoftcall(fullTimeStamp.localDateTimeStamp,s_softcall,"soft priority cancel call");
						cout.rdbuf(strm_buffer);
						l_softcallLogRows++;
					}
				}
			}
		}
    		
		/// check signal priority call: check whether need to initiate priority treatment
		///		Note that no new treatment will be initiated if the signal is in an active priority treatment,
		///		in the field, there is additional constrain of how many cycles per priority treatment, which is 
		///		not considered here as that needs local cycle clock info
		if (signalControlStatus.prsAction == priorityRequestServer_enum_t::NONE && cycleCtn != signalControlStatus.priorityCycleCnt)
		{
			/// get SPaT info for this intersection		
			spatIn = getSpatFromList(spatList,intersectionId);
			if (spatIn.tms > 0)
			{
				timeUtils::getFullTimeStamp(fullTimeStamp);
				phases2call.reset();
				softcall_enum_t::callType priorityCall = softcall_enum_t::UNKNOWN;
				for (itPrsList = signalControlStatus.prsList.begin(); itPrsList != signalControlStatus.prsList.end();++itPrsList)
				{
					if (itPrsList->second.requestStatus != priorityrequest_enum_t::QUEUED)
						continue;
					itVehList = vehList.find(itPrsList->first);
					if (itVehList == vehList.end() || !(itVehList->second.isOnApproach))
						continue;
          if (itVehList->second.cvStatus.back().motionState.speed < 2.0)
            continue;                   // not initiate priority request for stopped vehicle
					uint16_t time2go = getTime2goDeci(itVehList->second.cvStatus.back().vehicleLocationAware.dist2go.distLong,
						itVehList->second.cvStatus.back().motionState.speed);										                // in deciseconds           
					if (spatIn.spatMsg.phaseState[syncPhase-1].currState == phasestate_enum_t::GREEN					// sync phase in green
						&& spatIn.spatMsg.phaseState[syncPhase-1].timeToChange < maxTime2changePhaseExt					// time2change within maxTime2changePhaseExt
						&& time2go > spatIn.spatMsg.phaseState[syncPhase-1].timeToChange 
						&& time2go < spatIn.spatMsg.phaseState[syncPhase-1].timeToChange + maxGreenExtenstion)	// time2go within maxGreenExtenstion
					{
						priorityCall = softcall_enum_t::EXTENSION;
						itPrsList->second.requestStatus = priorityrequest_enum_t::ACTIVE;
						signalControlStatus.prsAction = priorityRequestServer_enum_t::GREENEXTENSION;
						signalControlStatus.callPhase = syncPhase;
						signalControlStatus.priorityVehicleId = itPrsList->first;
						signalControlStatus.priorityCycleCnt = cycleCtn;
						signalControlStatus.priorityTms = fullTimeStamp.tms; 
						break;
					}
					else if (spatIn.spatMsg.phaseState[syncPhase-1].currState == phasestate_enum_t::YELLOW		// sync phase in yellow
						&& time2go > spatIn.spatMsg.phaseState[syncPhase-1].timeToChange)									      // can't pass within yellow
					{
						priorityCall = softcall_enum_t::CALL;
						itPrsList->second.requestStatus = priorityrequest_enum_t::ACTIVE;
						signalControlStatus.prsAction = priorityRequestServer_enum_t::EARLYGREEN;
						signalControlStatus.callPhase = syncPhase;
						signalControlStatus.priorityVehicleId = itPrsList->first;
						signalControlStatus.priorityCycleCnt = cycleCtn;
						signalControlStatus.priorityTms = fullTimeStamp.tms; 
						break;
					}
					else if (spatIn.spatMsg.phaseState[syncPhase-1].currState == phasestate_enum_t::RED				// sync phase in red
						&& time2go > spatIn.spatMsg.phaseState[syncPhase-1].timeToChange)												// can't pass within timeToChange
					{
						priorityCall = softcall_enum_t::CALL;
						itPrsList->second.requestStatus = priorityrequest_enum_t::ACTIVE;
						signalControlStatus.prsAction = priorityRequestServer_enum_t::EARLYGREEN;
						signalControlStatus.callPhase = syncPhase;
						signalControlStatus.priorityVehicleId = itPrsList->first;
						signalControlStatus.priorityCycleCnt = cycleCtn;
						signalControlStatus.priorityTms = fullTimeStamp.tms; 
						break;
					}
				}
				if (priorityCall != softcall_enum_t::UNKNOWN)
					phases2call.set(syncPhase-1,1);
					
				if (phases2call.any())
				{
					/// send soft vehicle phase call request
					s_softcall.reset();
					s_softcall.ms_since_midnight = fullTimeStamp.ms_since_midnight;
					s_softcall.softcallPhase = static_cast<unsigned char>(phases2call.to_ulong());
					s_softcall.softcallObj = static_cast<unsigned char>(softcall_enum_t::PRIORITY);
					s_softcall.softcallType = static_cast<unsigned char>(priorityCall);
					if (priorityCall == softcall_enum_t::CALL)
						s_softcall.softcallNum = 1;
					else
						s_softcall.softcallNum = 0;					
          if (sendSoftcall)  
            socketUtils::sendall(fd_tciSend,(char*)&s_softcall,sizeof(s_softcall));
          OS_Display << "*************************************************" << endl;
          OS_Display << "***Sending priority call type " << static_cast<int>(priorityCall) << " on phase " << phases2call.to_string() << "**********" << endl;
          OS_Display << " color: " << static_cast<int>(spatIn.spatMsg.phaseState[syncPhase-1].currState);
          OS_Display << " time2change: " << static_cast<int>(spatIn.spatMsg.phaseState[syncPhase-1].timeToChange) << endl;
          OS_Display << "*************************************************" << endl;
					if (verbose)
					{
						logSoftcall(fullTimeStamp.localDateTimeStamp,s_softcall,"soft priority call");
					}
					if (logFile)
					{
						cout.rdbuf(OS_Softcall.rdbuf());
						logSoftcall(fullTimeStamp.localDateTimeStamp,s_softcall,"soft priority call");
						cout.rdbuf(strm_buffer);
						l_softcallLogRows++;
					}
				}
			}
		}
		
		/// check whether need to send ART
		timeUtils::getFullTimeStamp(fullTimeStamp);		
		if (fullTimeStamp.tms > signalControlStatus.artTms + 	artInterval && !signalControlStatus.prsList.empty())
		{
			spatIn = getSpatFromList(spatList,intersectionId);
			if (spatIn.tms > 0)
			{
				// fill art2send structure
				signalControlStatus.artMsgCnt = (uint8_t)((signalControlStatus.artMsgCnt + 1) % 128);
				signalControlStatus.artTms = fullTimeStamp.tms;
				art2send.msgCnt = signalControlStatus.artMsgCnt;
				art2send.id = intersectionId;
				art2send.status = spatIn.spatMsg.status;
				art2send.ms = static_cast<uint16_t>(fullTimeStamp.localDateTimeStamp.timeStamp.sec * 1000 + fullTimeStamp.localDateTimeStamp.timeStamp.millisec);
				art2send.priorityCause = signalControlStatus.priorityVehicleId;
				art2send.requestNums = static_cast<uint8_t>(signalControlStatus.prsList.size());
				int i = 0;
				long long etaOffset;
				for (itPrsList = signalControlStatus.prsList.begin(); itPrsList != signalControlStatus.prsList.end();++itPrsList)
				{
					art2send.request[i].id = itPrsList->first;
					art2send.request[i].inLaneId = itPrsList->second.requestInLaneId;
					art2send.request[i].outLaneId = itPrsList->second.requestOutLaneId;
					art2send.request[i].contolPhase = itPrsList->second.requestPhase;
					art2send.request[i].classType = itPrsList->second.requestVehicleClassType;
					art2send.request[i].classLevel = itPrsList->second.requestVehicleClassLevel;
					art2send.request[i].requestStatus = static_cast<uint8_t>(itPrsList->second.requestStatus);
					art2send.request[i].transitStatus = itPrsList->second.requestVehicleTransitStatus;
					memcpy(&(art2send.request[i].timeOfService),&(itPrsList->second.requestTimeOfService),sizeof(DTime_element_t));
					memcpy(&(art2send.request[i].endOfService),&(itPrsList->second.requestEndOfService),sizeof(DTime_element_t));
					etaOffset = (itPrsList->second.requestedServiceEndTms - fullTimeStamp.tms)/100;			// in deciseconds
					if (etaOffset < 0)
						etaOffset = 0;
					art2send.request[i].etaOffset = static_cast<uint16_t>(etaOffset);
					i++;
				}
				// encode ART
				buf[0] = static_cast<char>(0xFF);
				buf[1] = static_cast<char>(0xFF);
				buf[2] = static_cast<char>(wmeUtils::msgid_art);
				memcpy(&(buf[3]),&(fullTimeStamp.ms_since_midnight),4);
				bytesSend = asnJ2735Lib.encode_art_payload(&art2send,&buf[sizeof(wmeUtils::mmitss_udp_header_t)],sizeof(buf),false);
				// log payload
				cout.rdbuf(OS_Payload.rdbuf());
				logPayload(fullTimeStamp.localDateTimeStamp,&buf[sizeof(wmeUtils::mmitss_udp_header_t)],(size_t)bytesSend,msg_enum_t::ART_PKT);
				cout.rdbuf(strm_buffer);
				l_payloadLogRows++;
				// send ART to RSU for broadcast
				socketUtils::sendall(fd_wmeSend,buf,bytesSend+sizeof(wmeUtils::mmitss_udp_header_t));
				if (verbose)
				{
					logART(fullTimeStamp.localDateTimeStamp,buf,(size_t)bytesSend,signalControlStatus.priorityCycleCnt,art2send.priorityCause,art2send.requestNums);
				}
				if (logFile)
				{
					cout.rdbuf(OS_Art.rdbuf());
					logART(fullTimeStamp.localDateTimeStamp,buf,(size_t)bytesSend,signalControlStatus.priorityCycleCnt,art2send.priorityCause,art2send.requestNums);
					cout.rdbuf(strm_buffer);
					l_artLogRows++;
				}
			}
		}
		
		/// check vehicle phase call (all phases including coordinated phases, can be sent when signal priority control is active)
		phases2call.reset();
		timeUtils::getFullTimeStamp(fullTimeStamp);		
		for (itVehList = vehList.begin(); itVehList != vehList.end(); ++itVehList)
		{
			if (itVehList->second.isOnApproach && !itVehList->second.isPhaseCalled // at an approach of this intersection and has not been called before
        && itVehList->second.cvStatus.back().motionState.speed > 2.0)        // vehicle is not stopped
			{
				uint8_t controlPhase = (uint8_t)(itVehList->second.cvStatus.back().vehicleLocationAware.controlPhase - 1);
				uint16_t time2go = getTime2goDeci(itVehList->second.cvStatus.back().vehicleLocationAware.dist2go.distLong,
					itVehList->second.cvStatus.back().motionState.speed);																					// in deciseconds
				if (itVehList->second.cvStatus.back().vehicleSignalAware.currState != phasestate_enum_t::GREEN 	// phase not Green
          && !coordinatedPhases.test(controlPhase)																// not a coordinated phase
					&& time2go < maxTime2goPhaseCall																															// time2go within maxTime2goPhaseCall requirement		
					&& fullTimeStamp.tms - signalControlStatus.phaseCallTms[controlPhase] > vehPhaseCallInterval)	// within vehPhaseCallInterval requirement
				{
					// phase is not green and within the maximum bound for soft phase call
					phases2call.set(controlPhase,1);
					itVehList->second.isPhaseCalled = true;
					signalControlStatus.phaseCallTms[controlPhase] = fullTimeStamp.tms;
				}
			}
		}	
		if (phases2call.any())
		{
			/// send soft vehicle phase call request
			s_softcall.reset();
			s_softcall.ms_since_midnight = fullTimeStamp.ms_since_midnight;
			s_softcall.softcallPhase = static_cast<unsigned char>(phases2call.to_ulong());
			s_softcall.softcallObj = static_cast<unsigned char>(softcall_enum_t::VEH);
			s_softcall.softcallType = static_cast<unsigned char>(softcall_enum_t::CALL);
			s_softcall.softcallNum = 1;
      if (sendSoftcall)
        socketUtils::sendall(fd_tciSend,(char*)&s_softcall,sizeof(s_softcall));
      OS_Display << "*************************************************" << endl;
      OS_Display << "***Sending vehicle phase call on phase " << phases2call.to_string() << "**********" << endl;
      OS_Display << "*************************************************" << endl;
			if (verbose)
			{
				logSoftcall(fullTimeStamp.localDateTimeStamp,s_softcall,"soft vehicle phase call");
			}
			if (logFile)
			{
				cout.rdbuf(OS_Softcall.rdbuf());
				logSoftcall(fullTimeStamp.localDateTimeStamp,s_softcall,"soft vehicle phase call");
				cout.rdbuf(strm_buffer);
				l_softcallLogRows++;
			}
		}
		
		/// check vehicle green extension call: check cancel of active call first
		spatIn = getSpatFromList(spatList,intersectionId);	// get SPaT info for this intersection		
		if (spatIn.tms > 0)
		{
			phases2call.reset();
			for (int i = 0; i < NEMAPHASES; i++)
			{
				/// check phase that has been called for extension
				if (signalControlStatus.phaseExtStatus[i].callAction == softcall_enum_t::CALLED)
				{
					/// loop through callVehicles, remove vehicles that has passed the intersection
					for (itPhaseExtVidMap = signalControlStatus.phaseExtStatus[i].callVehicles.begin(); itPhaseExtVidMap != signalControlStatus.phaseExtStatus[i].callVehicles.end();)
					{
						itVehList = vehList.find(itPhaseExtVidMap->first);
						if (itVehList == vehList.end() || !itVehList->second.isOnApproach)
							signalControlStatus.phaseExtStatus[i].callVehicles.erase(itPhaseExtVidMap++);
						else
							++itPhaseExtVidMap;
					}				
					if (spatIn.spatMsg.phaseState[i].currState == phasestate_enum_t::GREEN 	// phase still in green
						&& signalControlStatus.phaseExtStatus[i].callVehicles.empty())				// no vehicle needs extension any more
					{
						/// needs to send soft cancel call
						phases2call.set(i,1);
						signalControlStatus.phaseExtStatus[i].callAction = softcall_enum_t::CANCELLED;
					}
					else if (spatIn.spatMsg.phaseState[i].currState != phasestate_enum_t::GREEN)	// phase no longer in green
					{
						/// phase changed from green, got to cancel regardless whether the vehicles passed the intersection or not
						/// no need to send soft cancel call but send it anyway
						phases2call.set(i,1);
						signalControlStatus.phaseExtStatus[i].callAction = softcall_enum_t::NOACTION;
						signalControlStatus.phaseExtStatus[i].callVehicles.clear();
					}
				}
			}
			if (phases2call.any())
			{
				/// send cancel soft vehicle phase call request
				timeUtils::getFullTimeStamp(fullTimeStamp);						
				s_softcall.reset();
				s_softcall.ms_since_midnight = fullTimeStamp.ms_since_midnight;
				s_softcall.softcallPhase = static_cast<unsigned char>(phases2call.to_ulong());
				s_softcall.softcallObj = static_cast<unsigned char>(softcall_enum_t::VEH);
				s_softcall.softcallType = static_cast<unsigned char>(softcall_enum_t::CANCEL);
				s_softcall.softcallNum = 1;
        if (sendSoftcall)
          socketUtils::sendall(fd_tciSend,(char*)&s_softcall,sizeof(s_softcall));
        OS_Display << "*************************************************" << endl;
        OS_Display << "***Sending cancel vehicle phase extension on phase " << phases2call.to_string() << "**********" << endl;
        OS_Display << "*************************************************" << endl;
				if (verbose)
				{
					logSoftcall(fullTimeStamp.localDateTimeStamp,s_softcall,"soft cancel phase extension");
				}
				if (logFile)
				{
					cout.rdbuf(OS_Softcall.rdbuf());
					logSoftcall(fullTimeStamp.localDateTimeStamp,s_softcall,"soft cancel phase extension");
					cout.rdbuf(strm_buffer);
					l_softcallLogRows++;
				}
			}
		}
		
		/// check vehicle green extension call: check whether need to initiate a call
		///		(non-coordination phases only as coordinated phase green extension involving signal priority green extension treatment)
		///		Note that vehicle green extension can only be initiated when signal priority control is not active, as it may conflict with priority treatment
		if (signalControlStatus.prsAction == priorityRequestServer_enum_t::NONE)
		{
			timeUtils::getFullTimeStamp(fullTimeStamp);		
			phases2call.reset();
			uint8_t maxCallNums = 0;
			for (itVehList = vehList.begin(); itVehList != vehList.end(); ++itVehList)
			{
				if (itVehList->second.isOnApproach && !itVehList->second.isExtensionCalled	// vehicle on an approach of this intersection
          && itVehList->second.cvStatus.back().motionState.speed > 2.0)             // vehicle is not stopped
				{
					uint8_t controlPhase = (uint8_t)(itVehList->second.cvStatus.back().vehicleLocationAware.controlPhase - 1);
					uint16_t time2go = getTime2goDeci(itVehList->second.cvStatus.back().vehicleLocationAware.dist2go.distLong,
						itVehList->second.cvStatus.back().motionState.speed);										// in deciseconds
					if (signalControlStatus.phaseExtStatus[controlPhase].callAction == softcall_enum_t::NOACTION	// no multiple extension calls on one green phase
						&& !coordinatedPhases.test(controlPhase)																// not a coordinated phase
						&& itVehList->second.cvStatus.back().vehicleSignalAware.currState == phasestate_enum_t::GREEN // phase in green
						&& itVehList->second.cvStatus.back().vehicleSignalAware.timeToChange < maxTime2changePhaseExt // time2change within maxTime2changePhaseExt
						&& time2go > itVehList->second.cvStatus.back().vehicleSignalAware.timeToChange 								// extension time within maxTime2phaseExt
						&& time2go < itVehList->second.cvStatus.back().vehicleSignalAware.timeToChange + maxTime2phaseExt)
					{
						phases2call.set(controlPhase,1);
						itVehList->second.isExtensionCalled = true;
						signalControlStatus.phaseExtStatus[controlPhase].callAction = softcall_enum_t::CALLED;
						signalControlStatus.phaseExtStatus[controlPhase].callTms = fullTimeStamp.tms;
						uint8_t callNums = (uint8_t)((time2go - itVehList->second.cvStatus.back().vehicleSignalAware.timeToChange) / phaseExtEvery);
						if (callNums == 0)
							callNums++;
						signalControlStatus.phaseExtStatus[controlPhase].callVehicles[itVehList->second.cvStatus.back().id] = callNums;			
						if (callNums > maxCallNums)
							maxCallNums = callNums;
					}
				}
			}
			if (phases2call.any())
			{
				/// send soft vehicle phase extension request
				s_softcall.reset();
				s_softcall.ms_since_midnight = fullTimeStamp.ms_since_midnight;
				s_softcall.softcallPhase = static_cast<unsigned char>(phases2call.to_ulong());
				s_softcall.softcallObj = static_cast<unsigned char>(softcall_enum_t::VEH);
				s_softcall.softcallType = static_cast<unsigned char>(softcall_enum_t::EXTENSION);
				s_softcall.softcallNum = maxCallNums;
        if (sendSoftcall)
          socketUtils::sendall(fd_tciSend,(char*)&s_softcall,sizeof(s_softcall));
        OS_Display << "*************************************************" << endl;
        OS_Display << "***Sending vehicle phase extension on phase " << phases2call.to_string() << "**********" << endl;
        OS_Display << "*************************************************" << endl;
				if (verbose)
				{
					logSoftcall(fullTimeStamp.localDateTimeStamp,s_softcall,"soft vehicular phase extension");
				}
				if (logFile)
				{
					cout.rdbuf(OS_Softcall.rdbuf());
					logSoftcall(fullTimeStamp.localDateTimeStamp,s_softcall,"soft vehicular phase extension");
					cout.rdbuf(strm_buffer);
					l_softcallLogRows++;
				}
			}
		}

		/// update lists (remove entry if it's outdated by timeOutms milliseconds)
		timeUtils::getFullTimeStamp(fullTimeStamp);		
		cleanBsmList(bsmList,fullTimeStamp.tms,timeOutms);
		cleanSrmList(srmList,fullTimeStamp.tms,timeOutms);
		cleanSpatList(spatList,fullTimeStamp.tms,timeOutms);
		cleanMapList(mapList,fullTimeStamp.tms,timeOutms);
		cleanArtList(artList,fullTimeStamp.tms,timeOutms);

    cleanVehList(vehList,fullTimeStamp.tms,timeOutms);
		cleanPrsList(signalControlStatus.prsList,fullTimeStamp.tms,timeOutms);
    
		/// check reopen log files
		timeUtils::getFullTimeStamp(fullTimeStamp);		
		if (logFile && fullTimeStamp.tms_minute > logfile_tms_minute + logFileInterval)
		{
			// close and delete the log file if it's empty
			OS_Aware.close();
			if (l_awreLogRows == 0)
				remove(s_AwreLogFileName.c_str());
			OS_CvStatus.close();
			if (l_cvstatusLogRows == 0)
				remove(s_CvStatusLogFileName.c_str());
			OS_Art.close();
			if (l_artLogRows == 0)
				remove(s_ArtLogFileName.c_str());
			OS_Wme.close();
			if (l_wmeLogRows == 0)
				remove(s_WmeLogFileName.c_str());	
			OS_Payload.close();
			if (l_payloadLogRows == 0)
				remove(s_PayloadLogFileName.c_str());	
			OS_Softcall.close();
			if (l_softcallLogRows == 0)
				remove(s_SoftcallLogFileName.c_str());		
			OS_PedCall.close();
			if (l_pedcallLogRows == 0)
				remove(s_PedcallLogFileName.c_str());		
			OS_Ownspat.close();
			if (l_ownspatLogRows == 0)
				remove(s_OwnspatLogFileName.c_str());		
			// re-open log files
			openLogFile(OS_Aware,s_AwreLogFileName,logFilePath,fullTimeStamp.localDateTimeStamp,intersectionName,"aware");
			l_awreLogRows = 0;
			openLogFile(OS_CvStatus,s_CvStatusLogFileName,logFilePath,fullTimeStamp.localDateTimeStamp,intersectionName,"cvstatus");
			l_cvstatusLogRows = 0;			
			openLogFile(OS_Art,s_ArtLogFileName,logFilePath,fullTimeStamp.localDateTimeStamp,intersectionName,"art");
			l_artLogRows = 0;
			openLogFile(OS_Wme,s_WmeLogFileName,logFilePath,fullTimeStamp.localDateTimeStamp,intersectionName,"wme");
			l_wmeLogRows = 0;
			openLogFile(OS_Payload,s_PayloadLogFileName,logFilePath,fullTimeStamp.localDateTimeStamp,intersectionName,"payload");
			l_payloadLogRows = 0;
			openLogFile(OS_Softcall,s_SoftcallLogFileName,logFilePath,fullTimeStamp.localDateTimeStamp,intersectionName,"softcall");
			l_softcallLogRows = 0;
			openLogFile(OS_PedCall,s_PedcallLogFileName,logFilePath,fullTimeStamp.localDateTimeStamp,intersectionName,"pedcall");
			l_pedcallLogRows = 0;
			openLogFile(OS_Ownspat,s_OwnspatLogFileName,logFilePath,fullTimeStamp.localDateTimeStamp,intersectionName,"spat");
			l_ownspatLogRows = 0;
				
			logfile_tms_minute = fullTimeStamp.tms_minute;
		}
		
		// sleep 
		usleep(usleepms);
	}
}

void cleanVehList(map<uint32_t,cvStatusAware_t>& list,const long long tms,const long long timeout)
{
	for(map<uint32_t,cvStatusAware_t>::iterator it = list.begin(); it != list.end();) 
	{
		if (tms > it->second.cvStatus.back().ts + timeout)
		{
			list.erase(it++);
		}
		else
			++it;
	}
}

void cleanPrsList(map<uint32_t,priorityRequestTableEntry_t>& list,const long long tms,const long long timeout)
{
	for(map<uint32_t,priorityRequestTableEntry_t>::iterator it = list.begin(); it != list.end();) 
	{
		if (tms > it->second.receivedTms + timeout)
		{
			list.erase(it++);
		}
		else
			++it;
	}
}

void logSoftcall(const timeUtils::dateTimeStamp_t& ts,const softcall_t& s_softcall,const char* callTye)
{
	bitset<NEMAPHASES> callPhase(s_softcall.softcallPhase);
	cout << timeUtils::getTimestampStr(ts) << " sent " << callTye << ": (";
	cout << callPhase.to_string() << ",";
	cout << static_cast<int>(s_softcall.softcallObj) << ",";
	cout << static_cast<int>(s_softcall.softcallType) << ",";
	cout << static_cast<int>(s_softcall.softcallNum) << ")";
	cout << endl;
}

void logART(const timeUtils::dateTimeStamp_t& ts,const char* buf,const size_t& size,
	const uint8_t& priorityCycleCnt,const uint32_t& priorityCause,const uint8_t& requestNums) 
{
	cout << timeUtils::getTimestampStr(ts) << ",";
	cout << " cycleCnt = " << static_cast<int>(priorityCycleCnt) << ",";
	cout << " priorityVehId = " << priorityCause << ";";
	cout << " requestNums = " << static_cast<int>(requestNums) << endl;
	cout << "ART payload = ";
	hexPrint(buf,size);
}
