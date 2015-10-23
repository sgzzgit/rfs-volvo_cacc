/* sendpcap.cpp
 *	Program which reads pcap file and sends DSRC payloads to obuAware or rsuAware to test their functionality 
 *		1. when work with obuAware, sends BSM payload to OBU ownBsmListenSocket and sends SPaT and Map to OBU inAirDsrcListenSocket,
 *			 	obuAware should send SRM to RSU inAirDsrcSendSocket. Check obuAware log file for sent SRM payloads. 
 *				ownBsmListenSocket, inAirDsrcListenSocket & inAirDsrcSendSocket are defined in obuSocket.config.
 *		2. when work with rsuAware, sends BSM, SPaT & Map to RSU inAirDsrcListenSocket. rsuAware should track BSM and output to log files.
 *				 inAirDsrcListenSocket is defined in rsuSocket.config
 *		3. when working with both obuAware & rsuAware, sends BSM, SPaT and Map to both OBU (ownBsmListenSocket & inAirDsrcListenSocket)
 *				and RSU (inAirDsrcListenSocket). obuAware should send SRM to RSU (inAirDsrcListenSocket), 
 *				rsuAware should send ART to OBU (inAirDsrcListenSocket) and soft-calls to TCI (tciSendSocket).
 *				This can be used to debugging osuAware & rsyAware using pcap file.			
*/

#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <unistd.h>

#include "AsnJ2735Lib.h"
#include "socketUtils.h"
#include "wmeUtils.h"
#include "pcapheader.h"
#include "obuConfig.h"
#include "rsuConfig.h"

using namespace std;

struct Action_enum_t
{
	enum Action {OBU,RSU,OBUANDRSU};
};

void do_usage(const char* progname)
{
	cout << "Usage" << progname << endl;
	cout << "\t-a action (obu, rsu, or obu_rsu)" << endl;
	cout << "\t-o obuSocket.config" << endl;	
	cout << "\t-r rsuSocket.config" << endl;	
	cout << "\t-f input .pcap filename" << endl;
	cout << "\t-h turn on wsmp header with payload" << endl;
	cout << "\t-v turn on verbose" << endl;
	exit (1);
}

int main(int argc, char** argv) 
{
	int option;
	char* action = NULL;
	char* f_obuSocketConfig = NULL;
	char* f_rsuSocketConfig = NULL;
	char* pcap = NULL;
	bool withWSMPheader = false;
	bool verbose = false;
		
	// getopt
	while ((option = getopt(argc, argv, "f:o:r:a:vh")) != EOF) 
	{
		switch(option) 
		{
		case 'a':
			action = strdup(optarg);
			break;		
		case 'o':
			f_obuSocketConfig = strdup(optarg);
			break;		
		case 'r':
			f_rsuSocketConfig = strdup(optarg);
			break;	
		case 'f':
			pcap = strdup(optarg);
			break;
		case 'h':
			withWSMPheader = true;
			break;
		case 'v':
			verbose = true;
			break;
		default:
			do_usage(argv[0]);
			break;
		}
	}
	if (action == NULL || f_obuSocketConfig == NULL || f_rsuSocketConfig == NULL || pcap == NULL)
	{
		do_usage(argv[0]);
	}	
	
	/* ----------- preparation -------------------------------------*/
	/// determine action
	Action_enum_t::Action myaction;
	if (strcmp(action,"obu") == 0)
		myaction = Action_enum_t::OBU;
	else if (strcmp(action,"rsu") == 0)
		myaction = Action_enum_t::RSU;
	else if (strcmp(action,"obu_rsu") == 0)
		myaction = Action_enum_t::OBUANDRSU;
	else if (strcmp(action,"rsu_obu") == 0)
		myaction = Action_enum_t::OBUANDRSU;
	else
	{
		cout << argv[0] << ": action: obu, rsu, or obu_rsu" << endl;
		exit (1);
	}
	delete action;
	
	/// read obuSocketConfig for sending payloads to obu
	OBUconfig::configObu s_obuConfig(f_obuSocketConfig,NULL);
	if (!s_obuConfig.isConfigSucceed())
	{
		cerr << argv[0] << ": failed construct OBUconfig::configObu";
		cerr << ", f_obuSocketConfig = " << f_obuSocketConfig << endl;
		delete f_obuSocketConfig;
		exit(1);
	}
	delete f_obuSocketConfig;
	/// read rsuSocketConfig for sending payloads to rsu
	RSUconfig::configRsu s_rsuConfig(f_rsuSocketConfig);
	if (!s_rsuConfig.isConfigSucceed())
	{
		cerr << argv[0] << ": failed construct RSUconfig::configRsu";
		cerr << ", f_rsuSocketConfig = " << f_rsuSocketConfig << endl;
		delete f_rsuSocketConfig;
		exit(1);
	}
	delete f_rsuSocketConfig;

	/// read nmap
	AsnJ2735Lib asnJ2735Lib(s_rsuConfig.getNmapFileName().c_str());
	if (asnJ2735Lib.getMapVersion() == 0)
	{
		cerr << argv[0] << ": failed open nmap file = " << s_rsuConfig.getNmapFileName() << endl;
		exit (1);
	}
		
	/// open pcap file
	pcap_t* handle; 
	char errbuf[PCAP_ERRBUF_SIZE]; 
	handle = pcap_open_offline(pcap, errbuf);
	if (handle == NULL)
	{ 
		cerr << argv[0] << ": failed open pcap file = " << pcap << endl;
		cerr << "Error msg: " << errbuf << endl;
		exit (1);
	}
	delete pcap;
	
	/// open sockets
	int fd_obuOwnBsm = socketUtils::client(s_obuConfig.getSocketAddr(OBUconfig::socketHandleEnum_t::BSMLISTEN),false);
	if (fd_obuOwnBsm < 0)
	{
		cerr << argv[0] << ": failed create socket to send to OBU OwnBsm listen port" << endl;
		pcap_close(handle);
		exit(1);
	}	
	int fd_obuDsrcBsm = socketUtils::client(s_obuConfig.getSocketAddr(OBUconfig::socketHandleEnum_t::WMELISTEN),false);
	if (fd_obuDsrcBsm < 0)
	{
		cerr << argv[0] << ": failed create socket to send to OBU dsrc listen port" << endl;
		close(fd_obuOwnBsm);
		pcap_close(handle);
		exit(1);
	}
	int fd_rsuDsrcBsm = socketUtils::client(s_rsuConfig.getSocketAddr(RSUconfig::socketHandleEnum_t::WMELISTEN),false);
	if (fd_rsuDsrcBsm < 0)
	{
		cerr << argv[0] << ": failed create socket to send to RSU dsrc listen port" << endl;
		close(fd_obuOwnBsm);
		close(fd_obuDsrcBsm);
		pcap_close(handle);
		exit(1);
	}
	
	/* ----------- local variables  -------------------------------------*/	
	const useconds_t usleepms = 10000;	// sleep 10 milliseconds between sending payloads
	
	/// pkt_union for decoding
	MSG_UNION_t	pkt_union;
	
	/// loop through frames
  struct pcap_pkthdr* 	hdr; 		// The header that pcap gives us
  const u_char* 				packet;	// The actual packet
	struct ether_header* 	eptr;		// The Ethernet header of a frame
	struct llc_snap_hdr*	lptr;		// The LLC/SNAP header of a frame
	int res;
	size_t offset;
	size_t headerLen,payloadLen;
	size_t sendOffset,sendSize;
	
	while(1)
	{
		res = pcap_next_ex(handle,&hdr,&packet); // return values: 1 - success, 0 - timeout, -1	- reading error, -2 - reached EOF
		if (res == -2)
			break;
		else if (res < 1)
			continue;
		/// check packet length
		if (hdr->caplen != hdr->len || hdr->len == 0)
			continue;
		/// start with the Ethernet II header
		eptr = (struct ether_header *) packet;
		offset = 0;
		if (ntohs(eptr->ether_type) == ETHER_TYPE_80211)
		{
			offset += sizeof(struct ether_header) + 26; // skip Ethernet II header (14 bytes) and 802.11 QoS header (26 bytes)
			/// get LLC/SNAP header
			lptr = (struct llc_snap_hdr *)(packet+offset); // point to an LLC/SNAP header structure
			if (ntohs(lptr->ethertype) == ETHER_TYPE_WSM)
			{
				/// this is wsmp
				offset += sizeof(struct llc_snap_hdr); // skip LLC/SNAP header (8 bytes)
				/// decode the wsmp pkt with wsmp header
				if (asnJ2735Lib.decode_payload((const char*)(packet+offset),(size_t)(hdr->len - offset),&pkt_union,NULL))
				{
					/// decoding succeeded, determine offset and size to send (with or without wsmp header)
					wmeUtils::getwsmheaderinfo((const uint8_t*)(packet+offset),(size_t)(hdr->len - offset),headerLen,payloadLen);		
					if (withWSMPheader)
					{
						sendOffset = offset;
						sendSize = (size_t)(hdr->len - offset);
					}
					else
					{
						sendOffset = offset + headerLen;
						sendSize = payloadLen;
					}
					/// sending payload based on action
					switch(pkt_union.type)
					{
					case msg_enum_t::BSM_PKT:
						switch(myaction)
						{
						case Action_enum_t::OBU:
							// only send to obu
							socketUtils::sendall(fd_obuOwnBsm,(const char*)(packet+sendOffset),sendSize);
							break;
						case Action_enum_t::RSU:
							// only send to rsu
							socketUtils::sendall(fd_rsuDsrcBsm,(const char*)(packet+sendOffset),sendSize);
							break;
						default:
							// send to OBU & RSU
							socketUtils::sendall(fd_obuOwnBsm,(const char*)(packet+sendOffset),sendSize);
							socketUtils::sendall(fd_rsuDsrcBsm,(const char*)(packet+sendOffset),sendSize);
							break;
						}	
						break;
					default: // other dsrc messages
						switch(myaction)
						{
						case Action_enum_t::OBU:
							socketUtils::sendall(fd_obuDsrcBsm,(const char*)(packet+sendOffset),sendSize);
							break;
						case Action_enum_t::RSU:
							socketUtils::sendall(fd_rsuDsrcBsm,(const char*)(packet+sendOffset),sendSize);
							break;
						default:
							socketUtils::sendall(fd_obuDsrcBsm,(const char*)(packet+sendOffset),sendSize);
							socketUtils::sendall(fd_rsuDsrcBsm,(const char*)(packet+sendOffset),sendSize);
							break;
						}	
						break;
					}
					if (verbose)
						cout << argv[0] << ": send " << pkt_union.type << " to " << myaction << endl;					
				}
			}
		}
		usleep(usleepms);
	}
	close(fd_obuOwnBsm);
	close(fd_obuDsrcBsm);
	close(fd_rsuDsrcBsm);
	pcap_close(handle);

  return 0; 
}
