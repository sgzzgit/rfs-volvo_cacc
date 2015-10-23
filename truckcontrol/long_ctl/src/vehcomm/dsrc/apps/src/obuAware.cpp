/* obuAware.cpp
 *	MMITSS OBU main process, tasks include
 *		1. get gps_fixes from OBU gpsd, pack BSM and send to wmefwd for broadcast
 *		2. receives WME messages forwarded by wmefwd, including BSM from other OBUs, and SPaT, MAP and ART from RSUs
 *		3. decodes WME messages
 *		4. tracks OBU location on the map and associates intersection-lane with control phase,
 *				also automatically switches map when the OBU crossed an intersection
 *		5.	sends SRM (including cancel) to RSU if the OBU is priority eligible (determined by configuration file)
*/
 
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <bitset>  
#include <unistd.h>
#include <sys/time.h>

#include "AsnJ2735Lib.h"
#include "comm.h"
#include "socketUtils.h"
#include "obuAware.h"
#include "wmeUtils.h"

#include "libgps.h"

using namespace std;

static bool time2query = false;
static void set_alram_timer(struct itimerval *ptout_val)
{
	ptout_val->it_interval.tv_sec = 0;
	ptout_val->it_interval.tv_usec = 100000;
	ptout_val->it_value.tv_sec = 0; 
	ptout_val->it_value.tv_usec = 100000;   
};
static void timer_alarm_hand(int code)
{
  if (code == SIGALRM)
    time2query = true;
};

void do_usage(const char* progname)
{
	cout << "Usage" << progname << endl;
	cout << "\t-s socket.config" << endl;
	cout << "\t-c vin.config" << endl;
	cout << "\t-f turn on log files" << endl;
	cout << "\t-v turn on verbose" << endl;
	exit (1);
}

int main(int argc, char** argv) 
{
	int option;
	char* f_socketConfig = NULL;
	char* f_vinConfig = NULL;
	volatile bool logFile = false;
	volatile bool verbose = false;
	
	/// getopt
	while ((option = getopt(argc, argv, "s:c:fv")) != EOF) 
	{
		switch(option) 
		{
		case 's':
			f_socketConfig = strdup(optarg);
			break;
		case 'c':
			f_vinConfig = strdup(optarg);
			break;		
		case 'f':
			logFile = true;
			break;
		case 'v':
			verbose = true;
			break;
		default:
			do_usage(argv[0]);
		}
	}
	if (f_socketConfig == NULL || f_vinConfig == NULL)
	{
		do_usage(argv[0]);
	}
	
	/* ----------- preparation -------------------------------------*/
	/// instance OBUconfig::configObu to read configuration files
	OBUconfig::configObu s_obuConfig(f_socketConfig,f_vinConfig);
	if (!s_obuConfig.isConfigSucceed())
	{
		cerr << argv[0] << ": failed construct OBUconfig::configObu";
		cerr << ", f_socketConfig = " << f_socketConfig;
		cerr << ", f_vinConfig = " << f_vinConfig << endl;
		delete f_socketConfig;
		delete f_vinConfig;
		exit(1);
	}
	delete f_socketConfig;
	delete f_vinConfig;
	
	/// get OBU vin
	OBUconfig::VinConfig myVin = s_obuConfig.getVinConfig();
	
	/// instance AsnJ2735Lib to read nmap
	AsnJ2735Lib asnJ2735Lib(s_obuConfig.getNmapFileName().c_str());
	if (asnJ2735Lib.getMapVersion() == 0)
	{
		cerr << argv[0] << ": failed open nmap file = " << s_obuConfig.getNmapFileName() << endl;
		exit (1);
	}
	
	/// timestamp
	timeUtils::fullTimeStamp_t fullTimeStamp;
	timeUtils::getFullTimeStamp(fullTimeStamp);
  
	/// open log files
	string logFilePath = s_obuConfig.getLogPath();
  string s_DisplayLogFileName = logFilePath + std::string("/display.log");
  ofstream OS_Display(s_DisplayLogFileName.c_str(), std::ofstream::out | std::ofstream::app);
  if (!OS_Display.is_open())
  {
    cerr << argv[0] << ": failed open display log file" << endl;
    exit(1);
  }
	
	long long logFileInterval = static_cast<long long>(s_obuConfig.getLogInterval()) * 60 * 1000LL;
	ofstream OS_Aware;		// log tracked own bsm locationAware & signalAware
	ofstream OS_Srm;			// log SRM
	ofstream OS_Wme;			// simple log of received in the air DSRC messages (SPaT, MAP, ART from RSUs & BSM from other cars)
	ofstream OS_Payload;	// log inbound and outbound payloads for offline debugging
	string s_AwreLogFileName;
	string s_SrmLogFileName;
	string s_WmeLogFileName;
	string s_PayloadLogFileName;	
	volatile unsigned long l_awreLogRows = 0;
	volatile unsigned long l_srmLogRows = 0;
	volatile unsigned long l_wmeLogRows = 0;
	volatile unsigned long l_payloadLogRows = 0;	
	volatile long long logfile_tms_minute = fullTimeStamp.tms_minute;
	if (logFile)
	{
		if (!openLogFile(OS_Aware,s_AwreLogFileName,logFilePath,fullTimeStamp.localDateTimeStamp,myVin.name,"aware"))
		{
			cerr << argv[0] << ": failed open aware log file" << endl;
      OS_Display.close();
			exit(1);
		}
		l_awreLogRows = 0;
		
		if (!openLogFile(OS_Srm,s_SrmLogFileName,logFilePath,fullTimeStamp.localDateTimeStamp,myVin.name,"srm"))
		{
			cerr << argv[0] << ": failed open srm log file" << endl;
      OS_Display.close();
			OS_Aware.close();
			exit(1);
		}
		l_srmLogRows = 0;
		
		if (!openLogFile(OS_Wme,s_WmeLogFileName,logFilePath,fullTimeStamp.localDateTimeStamp,myVin.name,"wme"))
		{
			cerr << argv[0] << ": failed open wme log file" << endl;
      OS_Display.close();
			OS_Aware.close();
			OS_Srm.close();
			exit(1);
		}
		l_wmeLogRows = 0;
		
		if (!openLogFile(OS_Payload,s_PayloadLogFileName,logFilePath,fullTimeStamp.localDateTimeStamp,myVin.name,"payload"))
		{
			cerr << argv[0] << ": failed open payload log file" << endl;
      OS_Display.close();      
			OS_Aware.close();
			OS_Srm.close();
			OS_Wme.close();			
			exit(1);
		}
		l_payloadLogRows = 0;
		
		logfile_tms_minute = fullTimeStamp.tms_minute;
	}
	  
	/// open sockets
	int fd_wmeListen = socketUtils::server(s_obuConfig.getSocketAddr(OBUconfig::socketHandleEnum_t::WMELISTEN),true);
	if (fd_wmeListen < 0)
	{
		cerr << argv[0] << ": failed create WMELISTEN socket" << endl;
    OS_Display.close();    
		if (logFile)
		{
			OS_Aware.close();
			OS_Srm.close();
			OS_Wme.close();
			OS_Payload.close();
		}
		exit(1);
	}
  
	int fd_wmeSend = socketUtils::client(s_obuConfig.getSocketAddr(OBUconfig::socketHandleEnum_t::WMESEND),false);
	if (fd_wmeSend < 0)
	{
		cerr << argv[0] << ": failed create WMESEND socket" << endl;
    OS_Display.close();    
		close(fd_wmeListen);
		if (logFile)
		{
			OS_Aware.close();
			OS_Srm.close();
			OS_Wme.close();
			OS_Payload.close();
		}
		exit(1);
	}
  
  /// initiate GPS
  int fd_gps;  
  savari_gps_data_t gpsdata;
  struct gps_data_t *gps_handler;
  gps_handler = savari_gps_open(&fd_gps, 0);
  if (gps_handler == 0)
  {
    cerr << argv[0] << ": failed connect to GPS" << endl;
    OS_Display.close();    
		close(fd_wmeListen);
		close(fd_wmeSend);
		if (logFile)
		{
			OS_Aware.close();
			OS_Srm.close();
			OS_Wme.close();
			OS_Payload.close();
		}
		exit(1);
  }
  memset(&gpsdata, 0, sizeof(savari_gps_data_t));	

	/* ----------- intercepts signals -------------------------------------*/
	if (setjmp(exit_env) != 0) 
	{
		cout << argv[0] << ": received user termination signal, quit!" << endl;
    OS_Display.close();    
		close(fd_wmeListen);
		close(fd_wmeSend);
		if (logFile)
		{
			OS_Aware.close();
			OS_Srm.close();
			OS_Wme.close();
			OS_Payload.close();
		}
    savari_gps_close(gps_handler);    
		return (0);
	}
	else
		sig_ign( sig_list, sig_hand );
		
	/* ----------- gps -------------------------------------*/
  /// set up callback timer for every 100 milliseconds
  struct sigaction sa;	
  struct itimerval tout_val;	    
  memset(&sa,0,sizeof(sa));
  sa.sa_handler = &timer_alarm_hand;
  sigaction(SIGALRM,&sa,NULL);
  set_alram_timer(&tout_val);
  setitimer(ITIMER_REAL, &tout_val,0);  
    
	/* ----------- local variables -------------------------------------*/
	/// control parameters
	const double 		stopSpeed = 0.2; 			// in m/s
	const double		stopDist = 5.0;				// in meters, no need to re-do mapping when speed is lower than stopSpeed & distance travelled is smaller than stopDist 
	const long long srmInterval = 1000LL;	// in milliseconds
	const useconds_t usleepms = 10000;		// in microseconds
	long long timeOutms = static_cast<long long>(s_obuConfig.getDsrcTimeOut()) * 1000LL; // in milliseconds
	
	/// socket buffer
	char buf[MAXDSRCMSGSIZE];	
	ssize_t bytesReceived;
	ssize_t bytesSend;
		
	/// maps to store latest inbound dsrc messages (SPaT, Map, ART from RSUs & BSM from other cars)
	map<uint32_t,BSM_List_t> 	bsmList;		// key vehId
	map<uint32_t,SRM_List_t>	srmList;		// key requested intersectionId
	map<uint32_t,SPAT_List_t> spatList;		// key intersectionId
	map<uint32_t,MAP_List_t> 	mapList;		// key intersectionId
	map<uint32_t,ART_List_t> 	artList;		// key intersectionId
	  
	/// dsrc message decoding
	MSG_UNION_t	ownBsm_pkt_union;	
	MSG_UNION_t	dsrc_pkt_union;	
	BSM_List_t 	bsmIn;
	SRM_List_t	srmIn;
	SPAT_List_t spatIn;
	MAP_List_t 	mapIn;
	ART_List_t	artIn;
	
	/// for tracking own bsm on map
	GeoUtils::connectedVehicle_t cvIn;	// input
	GeoUtils::connectedVehicle_t cv;		// this vehicle
	cv.reset();
	
	/// for tracking own priority request
	SRM_element_t cvSRM;
	initialSRM(cvSRM,myVin);
	PriorityRequest_t cvRequest;
	cvRequest.reset();
	priorityRequestAction_enum_t::Action priorityRequestAction;
	
	/// for redirect cout to into ofstream
	streambuf * strm_buffer = cout.rdbuf();
	
  /// bsm encoding
  BSM_element_t ownbsm;
  memset(&ownbsm,0,sizeof(ownbsm));
  ownbsm.bolb1_element.msgCnt = 0; 
  ownbsm.bolb1_element.id = myVin.vehId;
  ownbsm.bolb1_element.vehLen = myVin.vehLen;
  ownbsm.bolb1_element.vehWidth = myVin.vehWidth;
  ownbsm.bolb1_element.steeringAngle = AsnJ2735Lib::msUnavailableElement;
  ownbsm.bolb1_element.transState = 7;
  ownbsm.bolb1_element.brakes = static_cast<uint16_t>(0x0800);
  ownbsm.bolb1_element.semiMajorAccuracy = AsnJ2735Lib::msUnavailableElement;
  ownbsm.bolb1_element.semiMinorAccuracy = AsnJ2735Lib::msUnavailableElement;
  ownbsm.bolb1_element.orientation = AsnJ2735Lib::msUnavailableElement;
  
	while(1)
	{				
    if (time2query)
    {
      /// time to savari_gps_read      
      if (savari_gps_read(&gpsdata,gps_handler) == SUCCESS)
      {
        if (isnan(gpsdata.latitude) == 0 && isnan(gpsdata.longitude) == 0 && isnan(gpsdata.altitude) == 0)
        {
          if (verbose)
            cout << "savari_gps_read" << endl;
          timeUtils::getFullTimeStamp(fullTimeStamp);      
          ownbsm.bolb1_element.msgCnt = (uint8_t)((ownbsm.bolb1_element.msgCnt + 1) % 128);
          ownbsm.bolb1_element.ms = static_cast<uint16_t>(gpsdata.dsecond);
          ownbsm.bolb1_element.lat = gpsdata.latitude;
          ownbsm.bolb1_element.lon = gpsdata.longitude;
          ownbsm.bolb1_element.elev = gpsdata.altitude;
          ownbsm.bolb1_element.speed = gpsdata.speed;    // in kmp
          ownbsm.bolb1_element.heading = gpsdata.heading;
          ownbsm.bolb1_element.accelLon = gpsdata.lonaccel;
          ownbsm.bolb1_element.accelLat = gpsdata.lataccel;
          ownbsm.bolb1_element.accelVert = gpsdata.vertaccel;
          ownbsm.bolb1_element.yawRate = gpsdata.yawrate;
          /// encode BSM
          buf[0] = static_cast<char>(0xFF);
          buf[1] = buf[0];
          buf[2] = static_cast<char>(wmeUtils::msgid_bsm);
          memcpy(&(buf[3]),&(fullTimeStamp.ms_since_midnight),4);          
          bytesSend = asnJ2735Lib.encode_bsm_payload(&ownbsm,&buf[sizeof(wmeUtils::mmitss_udp_header_t)],sizeof(buf)-sizeof(wmeUtils::mmitss_udp_header_t),false);
          if (bytesSend > 0)
          {
            socketUtils::sendall(fd_wmeSend,buf,bytesSend+sizeof(wmeUtils::mmitss_udp_header_t));
            if (verbose)
            {
              cout << "Send BSM payload to wmefwd, lat = " << ownbsm.bolb1_element.lat << ", lon = " << ownbsm.bolb1_element.lon << endl;
            }
            if (logFile)
            {
              /// log payload
              cout.rdbuf(OS_Payload.rdbuf());
              logPayload(fullTimeStamp.localDateTimeStamp,&buf[sizeof(wmeUtils::mmitss_udp_header_t)],bytesSend,msg_enum_t::BSM_PKT);
              cout.rdbuf(strm_buffer);
              l_payloadLogRows++;
            }
            ownBsm_pkt_union.type = msg_enum_t::BSM_PKT;
            memcpy(&(ownBsm_pkt_union.bsm),&ownbsm,sizeof(BSM_element_t));
          
            /// get cvIn geo location from bms (id,ts,geoPoint,motionState)
            cvIn.reset();
            cvIn.id = ownBsm_pkt_union.bsm.bolb1_element.id;
            cvIn.ts = fullTimeStamp.tms;
            cvIn.geoPoint.latitude = ownBsm_pkt_union.bsm.bolb1_element.lat;
            cvIn.geoPoint.longitude = ownBsm_pkt_union.bsm.bolb1_element.lon;
            cvIn.geoPoint.elevation = ownBsm_pkt_union.bsm.bolb1_element.elev;
            cvIn.motionState.speed = ownBsm_pkt_union.bsm.bolb1_element.speed;
            cvIn.motionState.heading = ownBsm_pkt_union.bsm.bolb1_element.heading;
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
              /// check whether need to locate vehicle on map
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
              if (verbose)
              {
                logAware(fullTimeStamp.localDateTimeStamp,ownBsm_pkt_union.bsm.bolb1_element.msgCnt,cvIn);
              }			
              if (logFile)
              {
                cout.rdbuf(OS_Aware.rdbuf());
                logAware(fullTimeStamp.localDateTimeStamp,ownBsm_pkt_union.bsm.bolb1_element.msgCnt,cvIn);
                cout.rdbuf(strm_buffer);
                l_awreLogRows++;
              }
              // set cvIn to cv and continue;
              cv.isVehicleInMap = cvIn.isVehicleInMap;
              cv.vehicleTrackingState = cvIn.vehicleTrackingState;	
              cv.vehicleLocationAware = cvIn.vehicleLocationAware;
              cv.vehicleSignalAware = cvIn.vehicleSignalAware;
            }
          }
        }
      }
      time2query = false;
		}
    
		/// check whether there is inbound dsrc messages comes in (SPaT, MAP, ART from RSUs & BSM from other cars)
		timeUtils::getFullTimeStamp(fullTimeStamp);		
		bytesReceived = recv(fd_wmeListen,buf,sizeof(buf),0);
		if (bytesReceived > 0)
		{
      if(verbose)
      {
        cout << "received WME message" << endl;
      }
			// decode payload
			if (asnJ2735Lib.decode_payload(buf,bytesReceived,&dsrc_pkt_union,NULL))
			{
        if (dsrc_pkt_union.type != msg_enum_t::MAP_PKT)
        {
          // decoding succeeded, log payload
          cout.rdbuf(OS_Payload.rdbuf());
          logPayload(fullTimeStamp.localDateTimeStamp,buf,bytesReceived,dsrc_pkt_union.type);
          cout.rdbuf(strm_buffer);
          l_payloadLogRows++;
        }
				// update tracking lists
				switch(dsrc_pkt_union.type)
				{
				case msg_enum_t::BSM_PKT:
					// BSM from other cars, could be map-matched and tracked for other applications.
          bsmIn.tms = fullTimeStamp.tms;
					memcpy(&(bsmIn.bsmMsg),&(dsrc_pkt_union.bsm),sizeof(BSM_element_t));
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
					// SRM from other cars, decode SRM_element_t::BSMblob
					asnJ2735Lib.decode_bsmblob1_payload((u_char*)dsrc_pkt_union.srm.BSMblob,&(srmIn.srmBlob1));
          srmIn.tms = fullTimeStamp.tms;
					memcpy(&(srmIn.srmMsg),&(dsrc_pkt_union.srm),sizeof(SRM_element_t));
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
					memcpy(&(spatIn.spatMsg),&(dsrc_pkt_union.spat),sizeof(SPAT_element_t));
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
					memcpy(&(mapIn.mapMsg),&(dsrc_pkt_union.mapdata),sizeof(Mapdata_element_t));
					mapList[dsrc_pkt_union.mapdata.id] = mapIn;	// insert if new or overwrite existing entry
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
          OS_Display << "Received ART" << endl;
          artIn.tms = fullTimeStamp.tms;
					memcpy(&(artIn.artMsg),&(dsrc_pkt_union.art),sizeof(ART_element_t));	
					updateArtList(artList,artIn);					
					break;
				default:
					// shouldn't be here, do nothing
					break;
				}
			}
      else if (verbose)
      {
        cerr << "received wrong WME message" << endl;
      }
		}
		
		/// check whether need to send SRM
		timeUtils::getFullTimeStamp(fullTimeStamp);		
		if (myVin.isPrioEligible && fullTimeStamp.tms > cvRequest.requestedTms + srmInterval)
		{
			// determine priority request action
			priorityRequestAction = priorityRequestAction_enum_t::NONE;
			if (!cvRequest.isPriorityRequested)
			{
				if (cv.isVehicleInMap && cv.vehicleTrackingState.intsectionTrackingState.vehicleIntersectionStatus == GeoUtils::vehicleInMap_enum_t::ON_APPROACH)
				{
					// has not requested priority yet, now OBU is on an approach and needs to initiate SRM
					priorityRequestAction = priorityRequestAction_enum_t::INITIATE;
				}
				else
				{
					// no need to initiate priority request, do nothing
				}
			}
			else if (cv.isVehicleInMap && cv.vehicleTrackingState.intsectionTrackingState.vehicleIntersectionStatus != GeoUtils::vehicleInMap_enum_t::INSIDE_INTERSECTION_BOX
				&& cv.vehicleTrackingState.intsectionTrackingState.intersectionIndex == cvRequest.requestedIntersectionIndex
				&& cv.vehicleTrackingState.intsectionTrackingState.approachIndex == cvRequest.requestedApproachedIndex)
			{
				// remains on the same approach
				priorityRequestAction = priorityRequestAction_enum_t::KEEPGOING;
			}
			else
			{
				// changed approach or intersection
				priorityRequestAction = priorityRequestAction_enum_t::CANCEL;			
			}

			if (priorityRequestAction != priorityRequestAction_enum_t::NONE)
			{
				//	update cvRequest & cvSRM based on latest map matching result stored in cv and latest BSM bolb1 stored in ownBsm_pkt_union
				updatePriorityRequest(cvRequest,cv,fullTimeStamp.tms,priorityRequestAction);
				updateSRM(cvSRM,cvRequest,asnJ2735Lib,&(ownBsm_pkt_union.bsm.bolb1_element),priorityRequestAction);
				// encode SRM
				buf[0] = static_cast<char>(0xFF);
				buf[1] = static_cast<char>(0xFF);
				buf[2] = static_cast<char>(wmeUtils::msgid_srm);
				memcpy(&(buf[3]),&(fullTimeStamp.ms_since_midnight),4);
				bytesSend = asnJ2735Lib.encode_srm_payload(&cvSRM,&buf[sizeof(wmeUtils::mmitss_udp_header_t)],sizeof(buf) - sizeof(wmeUtils::mmitss_udp_header_t),false);
				// log payload
				cout.rdbuf(OS_Payload.rdbuf());
				logPayload(fullTimeStamp.localDateTimeStamp,&buf[sizeof(wmeUtils::mmitss_udp_header_t)],(size_t)bytesSend,msg_enum_t::SRM_PKT);
				cout.rdbuf(strm_buffer);
				l_payloadLogRows++;
				// send to OBU for broadcast
				socketUtils::sendall(fd_wmeSend,buf,bytesSend+sizeof(wmeUtils::mmitss_udp_header_t));
        OS_Display << "Send SRM " << static_cast<int>(priorityRequestAction) << endl;  // 1-start, 2- continue; 3- cancel
        cout.rdbuf(OS_Display.rdbuf());
        logSRM(fullTimeStamp.localDateTimeStamp,&buf[sizeof(wmeUtils::mmitss_udp_header_t)],(size_t)bytesSend,priorityRequestAction);						
        cout.rdbuf(strm_buffer);        
				if (verbose)
				{
					logSRM(fullTimeStamp.localDateTimeStamp,&buf[sizeof(wmeUtils::mmitss_udp_header_t)],(size_t)bytesSend,priorityRequestAction);
				}
				if (logFile)
				{
					cout.rdbuf(OS_Srm.rdbuf());
					logSRM(fullTimeStamp.localDateTimeStamp,&buf[sizeof(wmeUtils::mmitss_udp_header_t)],(size_t)bytesSend,priorityRequestAction);						
					cout.rdbuf(strm_buffer);
					l_srmLogRows++;
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
		/// reset cv if own BSM timeout expiries to make sure SRM can be cancelled 
		if (fullTimeStamp.tms > cv.ts + timeOutms)
			cv.reset();
		
		/// check reopen log files
		if (logFile && fullTimeStamp.tms_minute > logfile_tms_minute + logFileInterval)
		{
			// close and delete the log file if it's empty
			OS_Aware.close();
			if (l_awreLogRows == 0)
				remove(s_AwreLogFileName.c_str());
			OS_Srm.close();
			if (l_srmLogRows == 0)
				remove(s_SrmLogFileName.c_str());
			OS_Wme.close();
			if (l_wmeLogRows == 0)
				remove(s_WmeLogFileName.c_str());	
			OS_Payload.close();
			if (l_payloadLogRows == 0)
				remove(s_PayloadLogFileName.c_str());	
			// re-open log files
			openLogFile(OS_Aware,s_AwreLogFileName,logFilePath,fullTimeStamp.localDateTimeStamp,myVin.name,"aware");
			l_awreLogRows = 0;
			openLogFile(OS_Srm,s_SrmLogFileName,logFilePath,fullTimeStamp.localDateTimeStamp,myVin.name,"srm");
			l_srmLogRows = 0;
			openLogFile(OS_Wme,s_WmeLogFileName,logFilePath,fullTimeStamp.localDateTimeStamp,myVin.name,"wme");
			l_wmeLogRows = 0;
			openLogFile(OS_Payload,s_PayloadLogFileName,logFilePath,fullTimeStamp.localDateTimeStamp,myVin.name,"payload");
			l_payloadLogRows = 0;
			
			logfile_tms_minute = fullTimeStamp.tms_minute;
		}		

		/// sleep 
		usleep(usleepms);
	}
}

void initialSRM(SRM_element_t& cvSRM,OBUconfig::VinConfig& vehvin)
{
	cvSRM.msgCnt = 0;
	cvSRM.signalRequest_element.id = 0;
	cvSRM.signalRequest_element.requestedAction = priorityrequest_enum_t::UNKNOWREQUEST;
	cvSRM.signalRequest_element.inLaneId = 0;
	cvSRM.signalRequest_element.outLaneId = 0;
	cvSRM.signalRequest_element.NTCIPVehicleclass.NTCIPvehicleClass_type = vehvin.vehType;
	cvSRM.signalRequest_element.NTCIPVehicleclass.NTCIPvehicleClass_level = vehvin.prioLevel;
	strcpy(cvSRM.signalRequest_element.codeWord,vehvin.codeWord.c_str());
	cvSRM.timeOfService.hour = 0;
	cvSRM.timeOfService.min = 0;
	cvSRM.timeOfService.sec = 0;
	cvSRM.endOfService.hour = 0;
	cvSRM.endOfService.min = 0;
	cvSRM.endOfService.sec = 0;
	cvSRM.transitStatus = 0;
	strcpy(cvSRM.vehIdent_element.vehName,vehvin.name.c_str());
	strcpy(cvSRM.vehIdent_element.vehVin,vehvin.vin.c_str());
	strcpy(cvSRM.vehIdent_element.vehOwnerCode,vehvin.ownerCode.c_str());
	cvSRM.vehIdent_element.vehId = 0;
	cvSRM.vehIdent_element.vehType = vehvin.vehType;
	memset(cvSRM.BSMblob,0,BSMBLOB1SIZE);
	cvSRM.requestStatus = 0;
}

uint8_t getOutlaneId(const std::vector<GeoUtils::connectTo_t>& aConnectTo)
{
	uint8_t ret = 0;
	if (aConnectTo.empty())
		return (0);
	for (size_t i = 0; i < aConnectTo.size(); i++)
	{
		if (aConnectTo[i].laneManeuver == connectTo_enum_t::STRAIGHTAHEAD 
			|| aConnectTo[i].laneManeuver == connectTo_enum_t::STRAIGHT)
		{
			ret = aConnectTo[i].laneId;
			break;
		}
	}
	if (ret > 0)
		return (ret);
	for (size_t i = 0; i < aConnectTo.size(); i++)
	{
		if (aConnectTo[i].laneManeuver == connectTo_enum_t::LEFTTURN) 
		{
			ret = aConnectTo[i].laneId;
			break;
		}
	}
	if (ret > 0)
		return (ret);
	for (size_t i = 0; i < aConnectTo.size(); i++)
	{
		if (aConnectTo[i].laneManeuver == connectTo_enum_t::RIGHTTURN) 
		{
			ret = aConnectTo[i].laneId;
			break;
		}
	}
	if (ret > 0)
		return (ret);
	return (aConnectTo[0].laneId);
}

void updatePriorityRequest(PriorityRequest_t& request,const GeoUtils::connectedVehicle_t& cv,const long long tms,const priorityRequestAction_enum_t::Action action)
{
	request.requestedmsgCnt = (uint8_t)((request.requestedmsgCnt + 1) % 128);
	switch(action)
	{
	case priorityRequestAction_enum_t::CANCEL:
		request.isPriorityRequested = false;
		// obu may moved out the intersection, keep the info for the last priority request
		break;
	default:
		request.isPriorityRequested = true;
		// update request
		request.requestedTms = tms;
		request.requestedEta = tms + getTime2goLL(cv.vehicleLocationAware.dist2go.distLong,cv.motionState.speed);
		request.requestedIntersectionIndex = static_cast<uint8_t>(cv.vehicleTrackingState.intsectionTrackingState.intersectionIndex);
		request.requestedApproachedIndex = static_cast<uint8_t>(cv.vehicleTrackingState.intsectionTrackingState.approachIndex);
		request.requestedLaneIndex = static_cast<uint8_t>(cv.vehicleTrackingState.intsectionTrackingState.laneIndex);	
		request.requestedVehicleId = cv.id;
		request.requestedIntersectionId = cv.vehicleLocationAware.intersectionId;
		request.requestedInlaneId = cv.vehicleLocationAware.laneId;
		request.requestedOutlaneId = getOutlaneId(cv.vehicleLocationAware.connect2go);
		request.requestedControlPhase = cv.vehicleLocationAware.controlPhase;		
	}
}

void updateSRM(SRM_element_t& cvSRM,const PriorityRequest_t& request,const AsnJ2735Lib& lib,const BOLB1_element_t* ps_bsmblob1,const priorityRequestAction_enum_t::Action action)
{
	cvSRM.msgCnt = request.requestedmsgCnt;
	cvSRM.signalRequest_element.id = request.requestedIntersectionId;
	if (action == priorityRequestAction_enum_t::CANCEL)
	{
		cvSRM.signalRequest_element.requestedAction = static_cast<uint8_t>(priorityrequest_enum_t::CANCELPRIORITY);
	}
	else
	{
		cvSRM.signalRequest_element.requestedAction = static_cast<uint8_t>(priorityrequest_enum_t::REQUESTPRIORITY);
	}
	cvSRM.signalRequest_element.inLaneId = request.requestedInlaneId;
	cvSRM.signalRequest_element.outLaneId = request.requestedOutlaneId;	
	timeUtils::dateTimeStamp_t ts;
	timeUtils::timeStampFrom_time_t(static_cast<time_t>(request.requestedTms/1000LL),ts,true);
	dts2DTime(cvSRM.timeOfService,ts,static_cast<uint16_t>(request.requestedTms - (request.requestedTms/1000LL) * 1000LL));
	timeUtils::timeStampFrom_time_t(static_cast<time_t>(request.requestedEta/1000LL),ts,true);
	dts2DTime(cvSRM.endOfService,ts,static_cast<uint16_t>(request.requestedEta - (request.requestedTms/1000LL) * 1000LL));
	cvSRM.transitStatus = 0;
	cvSRM.vehIdent_element.vehId = request.requestedVehicleId;
	lib.encode_bsmblob1_payload(ps_bsmblob1,(u_char*)cvSRM.BSMblob);
	cvSRM.requestStatus = 0;
}

void dts2DTime(DTime_element_t& dt,const timeUtils::dateTimeStamp_t& ts,const uint16_t millitm)
{
	dt.hour = static_cast<uint8_t>(ts.timeStamp.hour);
	dt.min = static_cast<uint8_t>(ts.timeStamp.min);
	dt.sec = static_cast<uint16_t>(ts.timeStamp.sec * 1000 + millitm);
}

void logSRM(const timeUtils::dateTimeStamp_t& ts,const char* buf,const size_t size,const priorityRequestAction_enum_t::Action action)
{
	cout << timeUtils::getTimestampStr(ts) << ",";
	switch(action)
	{
	case priorityRequestAction_enum_t::INITIATE:
		cout << " initiated srm" << endl;
		break;
	case priorityRequestAction_enum_t::CANCEL:
		cout << " cancel srm" << endl;
		break;
	default:
		cout << " send srm" << endl;
	}
	cout << "srm payload = ";
	hexPrint(buf,size);
}
