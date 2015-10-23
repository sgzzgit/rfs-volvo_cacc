#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <sys/time.h>

#include "MapDataStruct.h"
#include "geoUtils.h"

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
		
	NmapData::MapDataStruct mapData(nmap);
	if (mapData.getMapVersion() == 0)
	{
		cerr << argv[0] << ": failed open nmap file " << nmap << endl;
		exit (1);
	}

	if (verbose)
	{
		mapData.nmap_fprintf(stdout);
		mapData.nmapApproachLaneNums_fprintf(stdout);
		mapData.nmapPahseLaneSet_fprintf(stdout);
		mapData.nmapConnectTo_fprintf(stdout);
		mapData.nmapWaypoints_fprintf(stdout);
		mapData.nmapPolygonConvexity_fprintf(stdout);
		mapData.nmapIntersectionPolygon_fprintf(stdout);
		mapData.nmapEgressNoConnectTo_fprintf(stdout);
	}
	
	GeoUtils::connectedVehicle_t testTrace;
	testTrace.reset();
	testTrace.id = 1;
	testTrace.ts = 0;
	testTrace.geoPoint.latitude = 37.42308800;
	testTrace.geoPoint.longitude = -122.14235000;
	testTrace.geoPoint.elevation = -18.6;	
	testTrace.motionState.speed = 0.022222;
	testTrace.motionState.heading = 0.0;
	
	GeoUtils::vehicleTracking_t cvTrackingState;
	bool isVehicleInMap;
	
	isVehicleInMap = mapData.locateVehicleInMap(testTrace,cvTrackingState);
	if (!isVehicleInMap)
	{
		cout << "Not in map" << endl;
	}
	else
	{
		cout << "In map: " << "status = " << cvTrackingState.intsectionTrackingState.vehicleIntersectionStatus;
		cout << ", intersectionIndex = " << cvTrackingState.intsectionTrackingState.intersectionIndex + 1;
		if (cvTrackingState.intsectionTrackingState.vehicleIntersectionStatus != GeoUtils::vehicleInMap_enum_t::INSIDE_INTERSECTION_BOX)
		{
			cout << ", approachIndex = " << cvTrackingState.intsectionTrackingState.approachIndex + 1;
			cout << ", approachIndex = " << cvTrackingState.intsectionTrackingState.approachIndex + 1;
			cout << ", laneIndex = " << cvTrackingState.intsectionTrackingState.laneIndex + 1;
			cout << " t = " << cvTrackingState.laneProj.proj2segment.t;
			cout << " d = " << cvTrackingState.laneProj.proj2segment.d << endl;
		}
	}

	return 0;
}
