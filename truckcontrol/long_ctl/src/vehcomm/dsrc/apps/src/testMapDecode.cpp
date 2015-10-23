#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <vector>

#include "AsnJ2735Lib.h"

using namespace std;

void do_usage(const char* progname)
{
	cout << "Usage" << progname << endl;
	cout << "\t-n nmap file" << endl;	
	cout << "\t-v turn on verbose" << endl;
	exit (1);
}

int main(int argc, char** argv) 
{
	int option;
	char* nmap = NULL;
	bool verbose = false;
	
	// getopt
	while ((option = getopt(argc, argv, "vn:")) != EOF) 
	{
		switch(option) 
		{
		case 'n':
			nmap = strdup(optarg);
			break;		
		case 'v':
			verbose = true;
			break;
		default:
			do_usage(argv[0]);
			break;
		}
	}
	if (nmap == NULL)
		do_usage(argv[0]);
	
	AsnJ2735Lib asnJ2735Lib(nmap);
	if (asnJ2735Lib.getMapVersion() == 0)
	{
		cerr << argv[0] << ": failed open nmap file: " << nmap << endl;
	}
	
	unsigned int intId[20];
	string intName;
	char payload[5000];
	size_t numBytes;
	
	Mapdata_element_t mapdata;
	
	size_t intNums = asnJ2735Lib.getIntersectionIdList(&intId[0],sizeof(intId)/sizeof(intId[0]));
	for (size_t i=0;i<intNums;i++)
	{
		intName = asnJ2735Lib.getIntersectionNameById(intId[i]);
		if (intName.empty())
		{
			cerr << argv[0] << ": failed to find intersectionName for Id " << intId[i] << endl;
			continue;
		}		
		numBytes = asnJ2735Lib.get_mapdata_payload(intName.c_str(), payload, sizeof(payload),false);
		if (numBytes == 0)
		{
			cerr << argv[0] << ": failed to get mapdata payload for " << intName << endl;
			continue;
		}	
		asnJ2735Lib.decode_mapdata_payload(payload,numBytes,&mapdata,NULL);
		
		if (verbose)
		{
			cout << "mapVersion " << static_cast<int>(mapdata.mapVersion) << ", intersectionId " << mapdata.id << ", bytes " << numBytes << endl;
			asnJ2735Lib.payloadHex_fprintf(payload,numBytes,stdout);
			cout << endl;
			numBytes = asnJ2735Lib.get_mapdata_payload(intName.c_str(), payload, sizeof(payload),true);
			fprintf(stdout,"%s\n\n",payload);
		}
	}
	
	return 0;
}
