#include <iostream>
#include <fstream>
#include <sstream>
#include <bitset>			// bitset
#include <utility>		// pair
#include <algorithm>	// reverse
#include <set>

#include "MapDataStruct.h"
#include "dsrcConsts.h"

using namespace std;

const string NmapData::maneuver_enum_t::msConnectManeuverCode[] 
	= {"uTurn","leftTurn","rightTurn","straightAhead","straight"};
const uint8_t NmapData::maneuver_enum_t::msConnectManeuverApproachIncrease[] 
	= {1, 3, 7, 5, 3}; // assume approach is ordered counter clockwise
	
NmapData::MapDataStruct::MapDataStruct(const char* ptr)
{
	mMapVersion = 0;
	// read nmaps
	readNmap(ptr);
	if (mMapVersion > 0)
	{
		// link upstream egress lane to downstream ingress lane (mpConnectTo for egress lanes)
		buildConnectToForEgressLane();
		// calculate local offsets and heading for way-points
		setLocalOffsetAndHeading();
		// build approach box
		buildPolygons();
		// build Phase2LaneSetMap
		buildPhase2LaneSetMap();
	}
	else
	{
		cerr << "Failed to read nmap file " << ptr << endl;
	}
}

NmapData::MapDataStruct::~MapDataStruct(void){}

uint8_t NmapData::MapDataStruct::getMapVersion(void) const
{
	return mMapVersion;
}
		
size_t NmapData::MapDataStruct::getIntersectionIdList(uint32_t* list,const size_t size) const
{
	size_t intersectionNums = mpIntersection.size();
	if(!list || size < intersectionNums)
	{
		return 0;
	}
	for (size_t i=0;i<intersectionNums;i++)
	{
		list[i] = mpIntersection[i].id;
	}
	return static_cast<int>(intersectionNums);
}

int NmapData::MapDataStruct::getIndexByIntersectionId(const uint32_t intersectionId) const
{
	int ret = -1;
	for (size_t i=0;i<mpIntersection.size();i++)
	{
		if (mpIntersection[i].id == intersectionId)
		{
			ret = static_cast<int>(i);
			break;
		}
	}
	return (ret);
}

vector<uint8_t> NmapData::MapDataStruct::getIndexesByIds(const uint32_t intersectionId,const uint8_t laneId) const
{
	vector<uint8_t> ret;
	unsigned long key = static_cast<unsigned long>((intersectionId << 8) + laneId);
	map<unsigned long, uint32_t>::const_iterator it = IndexMap.find(key);
	if (it != IndexMap.end())
	{
		uint32_t value = it->second;
		ret.push_back( ((value >> 16) & 0xFF) );
		ret.push_back( ((value >> 8) & 0xFF) );
		ret.push_back( (value & 0xFF) );
	}
	return ret;
}

uint32_t NmapData::MapDataStruct::getIntersectionIdByName(const char* intersectionName) const
{
	uint32_t ret = 0;
	string name(intersectionName);
	map< string, uint32_t >::const_iterator it = name2IdMap.find(name);
	if (it != name2IdMap.end())
	{
		ret = it->second;
	}
	return (ret);
}

string NmapData::MapDataStruct::getIntersectionNameByIndex(const uint8_t intersectionIndex) const
{
	return (mpIntersection[intersectionIndex].rsuId);
}

size_t NmapData::MapDataStruct::getIntersectionNameById(const uint32_t intersectionId,char* intersectionName,const size_t size) const
{
	string intName = getIntersectionNameById(intersectionId);
	if (intName.empty() || !intersectionName || intName.size()+1 > size)
	{
		return 0;
	}
	else
	{
		memcpy(intersectionName, intName.c_str(), intName.size() + 1);
		return (intName.size());
	}
}

string NmapData::MapDataStruct::getIntersectionNameById(const uint32_t intersectionId) const
{
	string intName;
	int index = getIndexByIntersectionId(intersectionId);
	if (index < 0)
	{
		return intName;
	}
	else
	{
		intName = mpIntersection[index].rsuId.c_str();
		return (intName);
	}
}

uint8_t NmapData::MapDataStruct::getControlPhaseByLaneId(const uint32_t intersectionId,const uint8_t laneId) const
{
	uint8_t ret = 0; // value of control phase should be (1..8)
	vector<uint8_t> inds = getIndexesByIds(intersectionId,laneId);
	if(!inds.empty())
	{
		ret = mpIntersection[inds[0]].mpApproaches[inds[1]].mpLanes[inds[2]].controlPhase;
	}
	return (ret);
}

vector<uint8_t> NmapData::MapDataStruct::getLaneSetByPhase(const uint32_t intersectionId,const uint8_t phase) const
{
	vector<uint8_t> ret;
	unsigned long key = static_cast<unsigned long>((intersectionId << 8) + phase);
	map< unsigned long, vector<uint8_t> >::const_iterator it = Phase2LaneSetMap.find(key);
	if (it != Phase2LaneSetMap.end())
	{
		ret = it->second;
	}
	return ret;
}

int NmapData::MapDataStruct::getLaneSetByPhase(const uint32_t intersectionId,const uint8_t phase,char* ptr) const
{
	vector<uint8_t> laneSet = getLaneSetByPhase(intersectionId,phase);
	for (size_t i=0; i< laneSet.size(); i++)
	{
		ptr[i] = (char)laneSet[i];
	}
	return static_cast<int>(laneSet.size());
}

vector<uint8_t> NmapData::MapDataStruct::getLaneSetByPedPhase(const uint32_t intersectionId,const uint8_t phase) const
{
	vector<uint8_t> ret;
	unsigned long key = static_cast<unsigned long>((intersectionId << 8) + phase);
	map< unsigned long, vector<uint8_t> >::const_iterator it = PedPhase2LaneSetMap.find(key);
	if (it != PedPhase2LaneSetMap.end())
	{
		ret = it->second;
	}
	return (ret);
}

int NmapData::MapDataStruct::getLaneSetByPedPhase(const uint32_t intersectionId,const uint8_t phase,char* ptr) const
{
	vector<uint8_t> laneSet = getLaneSetByPedPhase(intersectionId,phase);
	for (size_t i=0; i< laneSet.size(); i++)
	{
		ptr[i] = (char)laneSet[i];
	}
	return static_cast<int>(laneSet.size());
}
		
void NmapData::MapDataStruct::readNmap(const char* ptr)
{
	// open nmap file	
	ifstream IS_NMAP(ptr);
  if (!IS_NMAP.is_open())
	{
		cerr << "Failed open: " << ptr << endl;
		mMapVersion = 0;
		return;
	}
	// read nmap
	istringstream iss;
	string line,s;
	IntersectionStruct* pIntersection = 0;
	int intersectionCount = 0;
	uint8_t approachCount = 0;
	uint8_t laneCount = 0;
	unsigned short iTmp;
	unsigned short connectToApproachId,connectToLaneId;
	GeoUtils::geoPoint_t geoPoint;
	char* pTmp;

  while (std::getline(IS_NMAP,line))
  {
		if (!line.empty())
		{
			if (line.find("MAP_Version") == 0)
			{
				iss.str(line);
				iss >> std::skipws >> s >> iTmp;
				iss.clear();
				mMapVersion = static_cast<uint8_t>(iTmp);
			}	
			else if (line.find("MAP_Name") == 0)
			{
				// this is the beginning of an intersection nmap
				pIntersection = new IntersectionStruct;
				iss.str(line);
				iss >> std::skipws >> s >> pIntersection->name;
				iss.clear();
			}
			else if(line.find("RSU_ID") == 0)
			{
				iss.str(line);
				iss >> std::skipws >> s >> pIntersection->rsuId;
				iss.clear();
			}
			else if(line.find("IntersectionID") == 0)
			{
				iss.str(line);
				iss >> std::skipws >> s >> pIntersection->id;
				iss.clear();
				name2IdMap[pIntersection->rsuId] = pIntersection->id;
			}
			else if(line.find("Intersection_attributes") == 0)
			{
				iss.str(line);
				iss >> std::skipws >> s >> s;
				pIntersection->attributes = static_cast<uint8_t>(strtoul(s.c_str(),&pTmp,2));
				iss.clear();
			}
			else if(line.find("Reference_point") == 0)
			{
				iss.str(line);
				iss >> std::skipws >> s >> geoPoint.latitude >> geoPoint.longitude >> geoPoint.elevation;
				iss.clear();
				geoPoint.elevation /= DsrcConstants::deca;	// read-in elevation in decimetres 
				GeoUtils::geoPoint2geoRefPoint(geoPoint,pIntersection->geoRef);
				GeoUtils::setEnuCoord(geoPoint,pIntersection->enuCoord);
			}
			else if(line.find("No_Approach") == 0)
			{
				// the is beginning of ApproachStruct
				approachCount = 0;
				iss.str(line);
				int approachNums;
				iss >> std::skipws >> s >> approachNums;
				iss.clear();
				if (approachNums > 0)
					pIntersection->mpApproaches.resize(approachNums);
			}
			else if(line.find("Approach_type") == 0)
			{
				iss.str(line);
				iss >> std::skipws >> s >> iTmp;
				iss.clear();
				pIntersection->mpApproaches[approachCount].type = static_cast<maneuver_enum_t::approachType>(iTmp & 0xFF);
			}
			else if(line.find("Approach") == 0)
			{
				iss.str(line);
				iss >> std::skipws >> s >> iTmp;				
				iss.clear();
				pIntersection->mpApproaches[approachCount].id = static_cast<uint8_t>(iTmp & 0xFF);
			}
			else if(line.find("No_lane") == 0)
			{
				iss.str(line);
				int laneNums;
				iss >> std::skipws >> s >> laneNums;
				iss.clear();
				if (laneNums > 0)
				{
					pIntersection->mpApproaches[approachCount].mpLanes.resize(laneNums);
				}
				laneCount = 0;
				if (laneNums > 0)
				{
					// not to include faked approaches which has 0 lane
					if (pIntersection->mpApproaches[approachCount].type == maneuver_enum_t::APPROACH)
					{
						pIntersection->ingressIndex.push_back(approachCount);
					}
					else if (pIntersection->mpApproaches[approachCount].type == maneuver_enum_t::EGRESS)
					{
						pIntersection->egressIndex.push_back(approachCount);
					}
					else if (pIntersection->mpApproaches[approachCount].type == maneuver_enum_t::CROSSWALK)
					{
						pIntersection->crossWalkIndex.push_back(approachCount);
					}					
				}
			}
			else if(line.find("Lane_ID") == 0)
			{
				iss.str(line);
				iss >> std::skipws >> s >> iTmp;
				iss.clear();
				pIntersection->mpApproaches[approachCount].mpLanes[laneCount].id 
					= static_cast<uint8_t>(iTmp & 0xFF);
				IndexMap[(pIntersection->id << 8) + pIntersection->mpApproaches[approachCount].mpLanes[laneCount].id] 
					= static_cast<uint32_t>((intersectionCount << 16) + (approachCount << 8) + laneCount);
			}
			else if(line.find("Lane_type") == 0)
			{
				iss.str(line);
				iss >> std::skipws >> s >> iTmp;
				iss.clear();
				pIntersection->mpApproaches[approachCount].mpLanes[laneCount].type 
					= static_cast<maneuver_enum_t::laneType>(iTmp & 0xFF);		
			}
			else if(line.find("Lane_attributes") == 0)
			{
				iss.str(line);
				iss >> std::skipws >> s >> s;
				iss.clear();
				pIntersection->mpApproaches[approachCount].mpLanes[laneCount].attributes 
					= static_cast<uint16_t>(strtoul(s.c_str(),&pTmp,2));
			}
			else if(line.find("Lane_width") == 0)
			{
				iss.str(line);
				iss >> std::skipws >> s >> pIntersection->mpApproaches[approachCount].mpLanes[laneCount].width;
				iss.clear();
			}
			else if(line.find("Lane") == 0) 
			{
				iss.str(line);
				iss >> std::skipws >> s >> s >> iTmp;
				iss.clear();
				pIntersection->mpApproaches[approachCount].mpLanes[laneCount].controlPhase 
					= static_cast<uint8_t>(iTmp & 0xFF);				
			}		
			else if(line.find("No_nodes") == 0)
			{
				iss.str(line);
				int nodeNums;
				iss >> std::skipws >> s >> nodeNums;
				iss.clear();
				if (nodeNums > 0)
				{
					pIntersection->mpApproaches[approachCount].mpLanes[laneCount].mpNodes.resize(nodeNums);
				}
				// read nodeNums of nodes
				for (int i=0;i<nodeNums;i++)
				{
					std::getline(IS_NMAP,line);
					iss.str(line);
					iss >> std::skipws >> s >> geoPoint.latitude >> geoPoint.longitude>> iTmp;
					iss.clear();
					geoPoint.elevation = DsrcConstants::hecto2unit<int32_t>(pIntersection->geoRef.elevation);
					GeoUtils::geoPoint2geoRefPoint(geoPoint,pIntersection->mpApproaches[approachCount].mpLanes[laneCount].mpNodes[i].geoNode);
					if (iTmp == 1)
						pIntersection->mpApproaches[approachCount].mpLanes[laneCount].mpNodes[i].isBroadcast = true;
					else
						pIntersection->mpApproaches[approachCount].mpLanes[laneCount].mpNodes[i].isBroadcast = false;					
				}	
			}
			else if(line.find("No_Conn_lane") == 0)
			{
				iss.str(line);
				int connectionNums;
				iss >> std::skipws >> s >> connectionNums;
				iss.clear();
				if (connectionNums > 0)
				{
					pIntersection->mpApproaches[approachCount].mpLanes[laneCount].mpConnectTo.resize(connectionNums);
				}
				// connectionNums of connectTo
				for (int i = 0;i<connectionNums;i++)
				{
					std::getline(IS_NMAP,line);
					iss.str(line);
					iss >> std::skipws >> s >> iTmp;
					iss.clear();
					if (sscanf(s.c_str(),"%hu.%hu",&connectToApproachId,&connectToLaneId) != 2)
					{
						mMapVersion = 0;
						cerr << "Failed parse connectTo: " << s << endl;
						return;
					}
					pIntersection->mpApproaches[approachCount].mpLanes[laneCount].mpConnectTo[i].intersectionId = pIntersection->id;
					pIntersection->mpApproaches[approachCount].mpLanes[laneCount].mpConnectTo[i].laneId = 
						static_cast<uint8_t>(((connectToApproachId & 0x0F) << 4) + (connectToLaneId & 0x0F));
					pIntersection->mpApproaches[approachCount].mpLanes[laneCount].mpConnectTo[i].laneManeuver 
						= static_cast<connectTo_enum_t::maneuverType>(iTmp & 0xFF); 
				}
			}
			else if(line.find("end_lane") == 0)
			{
				laneCount ++;
			}
			else if(line.find("end_approach") == 0)
			{
				approachCount ++;
			}
			else if(line.find("end_map") == 0)
			{
				// this is the end of an intersection nmap
				// assign ConnectTo::laneId				
				for (size_t i = 0; i < pIntersection->mpApproaches.size(); i++)
				{
					for (size_t j = 0; j < pIntersection->mpApproaches[i].mpLanes.size(); j++)
					{
						for (size_t k = 0; k < pIntersection->mpApproaches[i].mpLanes[j].mpConnectTo.size(); k++)
						{
							iTmp = pIntersection->mpApproaches[i].mpLanes[j].mpConnectTo[k].laneId;
							connectToApproachId = (unsigned short)(iTmp >> 4);
							connectToLaneId = (iTmp & 0x0F);
							pIntersection->mpApproaches[i].mpLanes[j].mpConnectTo[k].laneId = 
								pIntersection->mpApproaches[connectToApproachId-1].mpLanes[connectToLaneId-1].id;
						}
					}
				}
				mpIntersection.push_back(*pIntersection);
				delete pIntersection;
				intersectionCount ++;
			}
		}
	}
  IS_NMAP.close();			
}

void NmapData::MapDataStruct::buildConnectToForEgressLane(void)
{
	/* this is particularly for CA ECR corridor where the 11 intersections are connected with each other.
		Intersection index in nmap file starts from 1 to 11 from ecr_stanford to ecr_charleston. Index 0 is RFS intersection (isolated)
		Northbound ECR is controlled by phase 2, ingress approach has index 2 and egress approach has index 7.
		Southbound ECR is controlled by phase 6, ingress approach has index 6 and egress approach has index 3.
		This function build connectTo from an upstream intersection's egress lane (STRAIGHTAHEAD) to its downstream ingress lane.
		Note that connectTo is not built for bypass lane (which does not enter the intersection box)
	*/
	size_t index;
	int egress;
	int ingress;
	int ingressLaneIndex;
	ConnectStruct connect2;
	
	for (int i = 0;i<2;i++)
	{
		if (i==0)
		{
			// Northbound
			index = 1;
			egress = 7;
			ingress = 2;
		}
		else
		{
			// Southbound
			index = mpIntersection.size() - 1;
			egress = 3;
			ingress = 6;
		}
		
		for (size_t egressIndex=0;egressIndex<mpIntersection[index].mpApproaches[egress].mpLanes.size();egressIndex++)
		{
			// j is the index counts from the central lane
			size_t j = (size_t)(mpIntersection[index].mpApproaches[egress].mpLanes.size() - 1 - egressIndex);					
			size_t intersectionIndex = index;
			size_t prev_intersectionIndex = index;
			while(1)
			{
				// find the ingress lane that connects with this egress lane (same intersection, defined by connectTo in nmap)
				ingressLaneIndex = -1;
				for (size_t ii=0;ii<mpIntersection[intersectionIndex].mpApproaches[ingress].mpLanes.size();ii++)
				{
					for (size_t jj=0;jj<mpIntersection[intersectionIndex].mpApproaches[ingress].mpLanes[ii].mpConnectTo.size();jj++)
					{
						if (mpIntersection[intersectionIndex].mpApproaches[ingress].mpLanes[ii].mpConnectTo[jj].intersectionId 
								== mpIntersection[intersectionIndex].id
							&& mpIntersection[intersectionIndex].mpApproaches[ingress].mpLanes[ii].mpConnectTo[jj].laneManeuver 
								== connectTo_enum_t::STRAIGHTAHEAD
							&& mpIntersection[intersectionIndex].mpApproaches[ingress].mpLanes[ii].mpConnectTo[jj].laneId ==
								mpIntersection[intersectionIndex].mpApproaches[egress].mpLanes[j].id)
						{
							ingressLaneIndex = static_cast<int>(ii);
							break;
						}
					}
					if (ingressLaneIndex >=0)
						break;
				}
				if (ingressLaneIndex < 0)
					break;
				
				// move to the upstream intersection egress 
				if (i==0) // Northbound
					intersectionIndex++;
				else // Southbound
					intersectionIndex--;
				// break while loop if reached the end of Northbound or Southbound 
				if (i==0 && intersectionIndex == mpIntersection.size())
					break;			
				if (i==1 && intersectionIndex ==0)
					break;
				if (mpIntersection[intersectionIndex].mpApproaches[egress].mpLanes.size() < egressIndex + 1)
					break;	
				// connect upstream intersection (intersectionIndex) egress lane 
				// to downstream intersection (prev_intersectionIndex) ingress lane (ingressLaneIndex) 
				// in the order of central lane to curb lane
				j = (size_t)(mpIntersection[intersectionIndex].mpApproaches[egress].mpLanes.size() - egressIndex - 1);
				connect2.intersectionId = mpIntersection[prev_intersectionIndex].id;
				connect2.laneId = mpIntersection[prev_intersectionIndex].mpApproaches[ingress].mpLanes[ingressLaneIndex].id;
				connect2.laneManeuver = connectTo_enum_t::STRAIGHT;
				mpIntersection[intersectionIndex].mpApproaches[egress].mpLanes[j].mpConnectTo.push_back(connect2);
				// push-back upstream intersection (intersectionIndex) egress lane way-points 
				// to downstream intersection (prev_intersectionIndex) ingress lane way-points
				for (size_t ii = 0; ii < mpIntersection[intersectionIndex].mpApproaches[egress].mpLanes[j].mpNodes.size();ii++)
				{
					// egress lane way-points in the order of upstream to downstream
					// ingress lane way-points have the opposite order
					size_t nodeIndex = (size_t)(mpIntersection[intersectionIndex].mpApproaches[egress].mpLanes[j].mpNodes.size()-1-ii);
					bool isWaypointIncluded = false;
					for (size_t jj = 0; jj < mpIntersection[prev_intersectionIndex].mpApproaches[ingress].mpLanes[ingressLaneIndex].mpNodes.size();jj++)
					{
						if (mpIntersection[prev_intersectionIndex].mpApproaches[ingress].mpLanes[ingressLaneIndex].mpNodes[jj].geoNode 
							== mpIntersection[intersectionIndex].mpApproaches[egress].mpLanes[j].mpNodes[nodeIndex].geoNode)
						{
							isWaypointIncluded = true;
							break;
						}
					}
					if (!isWaypointIncluded)
					{
						NodeStruct node = mpIntersection[intersectionIndex].mpApproaches[egress].mpLanes[j].mpNodes[nodeIndex];
						node.isBroadcast = false;
						node.geoNode.elevation = mpIntersection[intersectionIndex].geoRef.elevation;
						mpIntersection[prev_intersectionIndex].mpApproaches[ingress].mpLanes[ingressLaneIndex].mpNodes.push_back(node);
					}
				}
				// update downstream intersection			
				prev_intersectionIndex = intersectionIndex;
			}
		}
	}
	
	// manual add egress lane connectTo ingress lane for bypass lanes (not going through intersection box)
	// 1. ecr_page_mill lane 5.4.1 to ecr_portage_hansen lane 6.7.1 (ECR SB)
	// 2. ecr_portage_hansen lane 6.4.1 to ecr_matadero lane 7.7.1
	if (mpIntersection[4].mpApproaches[3].mpLanes[0].mpConnectTo.empty())
	{
		connect2.intersectionId = mpIntersection[5].id;
		connect2.laneId = mpIntersection[5].mpApproaches[6].mpLanes[0].id;
		connect2.laneManeuver = connectTo_enum_t::STRAIGHT;
		mpIntersection[4].mpApproaches[3].mpLanes[0].mpConnectTo.push_back(connect2);
	}
	if (mpIntersection[5].mpApproaches[3].mpLanes[0].mpConnectTo.empty())
	{
		connect2.intersectionId = mpIntersection[6].id;
		connect2.laneId = mpIntersection[6].mpApproaches[6].mpLanes[0].id;
		connect2.laneManeuver = connectTo_enum_t::STRAIGHT;
		mpIntersection[5].mpApproaches[3].mpLanes[0].mpConnectTo.push_back(connect2);
	}		
}

void NmapData::MapDataStruct::setLocalOffsetAndHeading(void)
{	
	uint32_t dTo1stNode;
	uint32_t radius;
	uint32_t ptLength;
	GeoUtils::point2D_t origin;
	origin.x	= 0;
	origin.y	= 0;	
	GeoUtils::projection_t proj2segment;
	
	// ptNode, dTo1stNode
	for (size_t i=0;i<mpIntersection.size();i++)
	{
		radius = 0;
		for (size_t j=0;j<mpIntersection[i].mpApproaches.size();j++)
		{
			for (size_t k=0;k<mpIntersection[i].mpApproaches[j].mpLanes.size();k++)
			{
				dTo1stNode = 0;
				for (size_t ii = 0;ii<mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes.size(); ii++)
				{
					GeoUtils::lla2enu(mpIntersection[i].enuCoord,mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes[ii].geoNode,
						mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes[ii].ptNode);
					if (ii == 0)
					{
						mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes[ii].dTo1stNode = 0;
					}
					else
					{
						dTo1stNode += mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes[ii].ptNode.distance2pt(mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes[ii-1].ptNode);
						mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes[ii].dTo1stNode = dTo1stNode;
					}					
					ptLength = mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes[ii].ptNode.length();
					if (ptLength > radius)
					{
						radius = ptLength;
					}
				}
				// reset mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes[0].dTo1stNode
				// For traffic lanes, project intersection ref point (i.e., origin) onto the closed segment on lane, 
				//	set distance from the closed way-point to the projected point as dTo1stNode for the closed way-point
				//	this is helpful for applications that crossing the stop-bar will be an event trigger, such as to cancel TSP request.
				//	GPS overshot at stop-bar could cause wrong cancel request. This projected distance to intersection center can be used
				//	to ensure the vehicle has crossed the stop-bar. 
				//	This value won't affect calculation of distance to the stop-bar.
				switch (mpIntersection[i].mpApproaches[j].type)
				{
				case maneuver_enum_t::APPROACH:
					GeoUtils::projectPt2Line(mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes[1].ptNode,
						mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes[0].ptNode,origin,proj2segment);
					mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes[0].dTo1stNode  = static_cast<uint32_t>(fabs((proj2segment.t -1)* proj2segment.length));	
					break;
				case maneuver_enum_t::EGRESS:
					GeoUtils::projectPt2Line(mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes[0].ptNode,
						mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes[1].ptNode,origin,proj2segment);
					mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes[0].dTo1stNode  = static_cast<uint32_t>(fabs((proj2segment.t) * proj2segment.length));	
					break;
				default:
					break;
				}
			}
			// get mindist2intsectionCentralLine
			if (mpIntersection[i].mpApproaches[j].mpLanes.empty())
				mpIntersection[i].mpApproaches[j].mindist2intsectionCentralLine = 2000;
			else
			{
				mpIntersection[i].mpApproaches[j].mindist2intsectionCentralLine = mpIntersection[i].mpApproaches[j].mpLanes[0].mpNodes[0].dTo1stNode;
				for (size_t k = 1;k<mpIntersection[i].mpApproaches[j].mpLanes.size(); k++)
				{
					if (mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes[0].dTo1stNode < mpIntersection[i].mpApproaches[j].mindist2intsectionCentralLine)
						mpIntersection[i].mpApproaches[j].mindist2intsectionCentralLine = mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes[0].dTo1stNode;
				}
			}		
		}
		mpIntersection[i].radius = radius;
	}
	
	// heading
	for (size_t i=0;i<mpIntersection.size();i++)
	{
		for (size_t j=0;j<mpIntersection[i].mpApproaches.size();j++)
		{
			if (mpIntersection[i].mpApproaches[j].type == maneuver_enum_t::APPROACH)
			{
				// order of node sequence on APPROACH starts at stop-bar towards upstream
				for (size_t k=0;k<mpIntersection[i].mpApproaches[j].mpLanes.size();k++)
				{
					for (size_t ii = 1; ii<mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes.size(); ii++)
					{
						mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes[ii].heading 
							= mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes[ii].ptNode.direction2pt(mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes[ii-1].ptNode);
					}
					mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes[0].heading = mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes[1].heading;
				}
			}
			else
			{
				// order of node sequence on EGRESS starts at cross-walk towards downstream
				for (size_t k=0;k<mpIntersection[i].mpApproaches[j].mpLanes.size();k++)
				{
					for (size_t ii = 0;ii < mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes.size() - 1; ii++)
					{
						mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes[ii].heading
							= mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes[ii].ptNode.direction2pt(mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes[ii+1].ptNode);
					}
					mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes[mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes.size() - 1].heading = 
						mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes[mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes.size() - 2].heading;
				}
			}
		}
	}				
}

void NmapData::MapDataStruct::buildPolygons(void)
{
	GeoUtils::point2D_t origin = {0,0};
	GeoUtils::point2D_t waypoint;
	GeoUtils::point2D_t p1 = origin;
	GeoUtils::point2D_t p2 = origin;
	vector<GeoUtils::point2D_t> farthestWaypoints;
	// each intersection has at most 4 cross-walks
	vector< pair<GeoUtils::point2D_t,GeoUtils::point2D_t> > nearestWayPointPair(4, make_pair(origin,origin));
	double alpha;				// way-point heading
	int32_t halfWidth;
	size_t indx;
	size_t crosswalkIndx;

	for (size_t i = 0; i < mpIntersection.size();i++)
	{
		// reset nearestWayPointPair
		for (int ii = 0; ii < 4; ii++)
		{
			// intersection polygon cannot cross origin
			nearestWayPointPair[ii].first = origin;
			nearestWayPointPair[ii].second = origin;
		}
		for (size_t j = 0; j < mpIntersection[i].mpApproaches.size(); j++)
		{
			// approach is in counter-clockwise, approach pairs (0,1)(2,3)(4,5)(6,7) are on the same cross-walk
			// build ApproachPolygon
			// reset farthestWaypoints
			farthestWaypoints.clear();
			if (mpIntersection[i].mpApproaches[j].type == maneuver_enum_t::CROSSWALK)
				continue;
			if (mpIntersection[i].mpApproaches[j].mpLanes.empty())
				continue;
			crosswalkIndx = j / 2;
			for (size_t k = 0; k < mpIntersection[i].mpApproaches[j].mpLanes.size(); k++)
			{
				// lane starts from curb lane to central lane
				halfWidth = static_cast<int32_t>(mpIntersection[i].mpApproaches[j].mpLanes[k].width * NmapData::laneWidthRatio);
				if (k == 0)
				{
					// add half lane width to the curb lane, away from traffic lane (right-side of heading)
					indx = 0; // nearest way-point
					alpha = DsrcConstants::deg2rad(DsrcConstants::deca2unit<uint16_t>(mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes[indx].heading));
					waypoint.x = mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes[indx].ptNode.x + static_cast<int32_t>(halfWidth * cos(alpha));
					waypoint.y = mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes[indx].ptNode.y - static_cast<int32_t>(halfWidth * sin(alpha));
					mpIntersection[i].mpApproaches[j].mpPolygon.push_back(waypoint);
					p1 = waypoint;
					
					indx = mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes.size() - 1; // farthest way-point
					alpha = DsrcConstants::deg2rad(DsrcConstants::deca2unit<uint16_t>(mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes[indx].heading));
					waypoint.x = mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes[indx].ptNode.x + static_cast<int32_t>(halfWidth * cos(alpha));
					waypoint.y = mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes[indx].ptNode.y - static_cast<int32_t>(halfWidth * sin(alpha));
					farthestWaypoints.push_back(waypoint);
				}
				if (k == mpIntersection[i].mpApproaches[j].mpLanes.size() - 1)
				{
					// add half lane width to the central lane, away from traffic lane (left-side of heading)
					// in case the approach only has one lane, the polygon is actually the lane polygon
					indx = 0;
					alpha = DsrcConstants::deg2rad(DsrcConstants::deca2unit<uint16_t>(mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes[indx].heading));
					waypoint.x = mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes[indx].ptNode.x - static_cast<int32_t>(halfWidth * cos(alpha));
					waypoint.y = mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes[indx].ptNode.y + static_cast<int32_t>(halfWidth * sin(alpha));
					mpIntersection[i].mpApproaches[j].mpPolygon.push_back(waypoint);
					p2 = waypoint;
					
					indx = mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes.size() - 1;
					alpha = DsrcConstants::deg2rad(DsrcConstants::deca2unit<uint16_t>(mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes[indx].heading));
					waypoint.x = mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes[indx].ptNode.x - static_cast<int32_t>(halfWidth * cos(alpha));
					waypoint.y = mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes[indx].ptNode.y + static_cast<int32_t>(halfWidth * sin(alpha));
					farthestWaypoints.push_back(waypoint);
				}
				if (k > 0 && k < mpIntersection[i].mpApproaches[j].mpLanes.size() - 1)
				{
					// keep middle lane nearest and furthest way-points
					indx = 0;					
					waypoint = mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes[indx].ptNode;
					mpIntersection[i].mpApproaches[j].mpPolygon.push_back(waypoint);
					
					indx = mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes.size() - 1;
					waypoint = mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes[indx].ptNode;
					farthestWaypoints.push_back(waypoint);
				}
			}
			if(nearestWayPointPair[crosswalkIndx].first == origin)
			{
				nearestWayPointPair[crosswalkIndx].first = p1;
				nearestWayPointPair[crosswalkIndx].second = p2;
			}
			else
			{
				nearestWayPointPair[crosswalkIndx].second = p1;
			}
			
			// add farthestWaypoints to ApproachPolygon
			reverse(farthestWaypoints.begin(),farthestWaypoints.end());
			mpIntersection[i].mpApproaches[j].mpPolygon.insert(mpIntersection[i].mpApproaches[j].mpPolygon.end(),
				farthestWaypoints.begin(),farthestWaypoints.end());
			if (GeoUtils::convexcave(mpIntersection[i].mpApproaches[j].mpPolygon,mpIntersection[i].mpApproaches[j].mpPolygon.size()) != GeoUtils::polygon_enum_t::CONVEX)
			{
				mpIntersection[i].mpApproaches[j].mpPolygon = GeoUtils::convexHullAndrew(mpIntersection[i].mpApproaches[j].mpPolygon);
			}
			// get polygon type
			mpIntersection[i].mpApproaches[j].mpPolygonType = GeoUtils::convexcave(mpIntersection[i].mpApproaches[j].mpPolygon,mpIntersection[i].mpApproaches[j].mpPolygon.size());
		}
		for (int ii = 0; ii< 4; ii++)
		{
			mpIntersection[i].mpPolygon.push_back(nearestWayPointPair[ii].first);
			mpIntersection[i].mpPolygon.push_back(nearestWayPointPair[ii].second);
		}
		if (GeoUtils::convexcave(mpIntersection[i].mpPolygon, mpIntersection[i].mpPolygon.size()) != GeoUtils::polygon_enum_t::CONVEX)
		{
			mpIntersection[i].mpPolygon = GeoUtils::convexHullAndrew(mpIntersection[i].mpPolygon);
		}		
		mpIntersection[i].mpPolygonType = GeoUtils::convexcave(mpIntersection[i].mpPolygon, mpIntersection[i].mpPolygon.size());
	}
}

void NmapData::MapDataStruct::buildPhase2LaneSetMap(void)
{	
	for (size_t i=0;i<mpIntersection.size();i++)
	{
		for (uint8_t phase = 1; phase < 9; phase++)
		{
			vector<uint8_t> phaseLaneSet;
			vector<uint8_t> PedPhaseLaneSet;
			for (size_t j=0;j<mpIntersection[i].mpApproaches.size();j++)
			{
				for (size_t k=0;k<mpIntersection[i].mpApproaches[j].mpLanes.size();k++)
				{
					if (mpIntersection[i].mpApproaches[j].mpLanes[k].controlPhase == phase)
					{
						if (mpIntersection[i].mpApproaches[j].mpLanes[k].type == maneuver_enum_t::CROSSWALKLANE)
						{	
							PedPhaseLaneSet.push_back(mpIntersection[i].mpApproaches[j].mpLanes[k].id);
						}
						else
						{
							phaseLaneSet.push_back(mpIntersection[i].mpApproaches[j].mpLanes[k].id);
						}
					}
					if (!phaseLaneSet.empty())
					{
						Phase2LaneSetMap[(mpIntersection[i].id << 8) + phase] = phaseLaneSet;
					}
					if (!PedPhaseLaneSet.empty())
					{
						PedPhase2LaneSetMap[(mpIntersection[i].id << 8) + phase] = PedPhaseLaneSet;
					}					
				}
			}
		}
	}
}

vector<uint8_t> NmapData::MapDataStruct::nearedIntersections(const GeoUtils::geoPoint_t& geoPoint) const
{
	vector<uint8_t> ret;
	
	for (size_t i=0;i<mpIntersection.size();i++)
	{
		if (isPointNearIntersection(static_cast<uint8_t>(i),geoPoint))
		{
			ret.push_back(static_cast<uint8_t>(i));
		}
	}
	return ret;
}

bool NmapData::MapDataStruct::isPointNearIntersection(const uint8_t intersectionIndex, const GeoUtils::geoPoint_t& geoPoint) const
{
	GeoUtils::point3D_t ptENU;
	GeoUtils::lla2enu(mpIntersection[intersectionIndex].enuCoord,geoPoint,ptENU);
	if ( sqrt(ptENU.x * ptENU.x + ptENU.y * ptENU.y) > DsrcConstants::hecto2unit<int32_t>(mpIntersection[intersectionIndex].radius) )
		return false;
	else
		return true;
}

bool NmapData::MapDataStruct::isPointInsideIntersectionBox(const uint8_t intersectionIndex, const GeoUtils::point2D_t& ptENU) const
{
	return (GeoUtils::isPointInsidePolygon(mpIntersection[intersectionIndex].mpPolygon,mpIntersection[intersectionIndex].mpPolygon.size(),ptENU));
}

vector<uint8_t> NmapData::MapDataStruct::onApproaches(const uint8_t intersectionIndex, const GeoUtils::point2D_t& ptENU) const
{
	// also do this when geoPoint is near the intersection (check first with isPointNearIntersection) 
	vector<uint8_t> ret;
	for (size_t i = 0; i < mpIntersection[intersectionIndex].mpApproaches.size(); i++)
	{
		if (mpIntersection[intersectionIndex].mpApproaches[i].mpPolygon.empty())
			continue;
		if (isPointOnApproach(intersectionIndex,static_cast<uint8_t>(i),ptENU))
		{
			ret.push_back(static_cast<uint8_t>(i));
		}
	}
	return (ret);
}

bool NmapData::MapDataStruct::isPointOnApproach(const uint8_t intersectionIndex, const uint8_t approachIndex, const GeoUtils::point2D_t& ptENU) const
{	
	// Approach polygon is convex. For point inside (or on the polygon edge) a convex polygon, 
	// it should be on the same side of all polygon edges when moving counter-clockwise of clockwise along vertices. 
	// Check side with cross product
	return (GeoUtils::isPointInsidePolygon(mpIntersection[intersectionIndex].mpApproaches[approachIndex].mpPolygon, 
		mpIntersection[intersectionIndex].mpApproaches[approachIndex].mpPolygon.size(),ptENU));
}

double NmapData::MapDataStruct::getHeadingDifference(const uint16_t nodeHeading,const double ptHeading) const
{
	// headingDiff in [-180..180];
	double d = ptHeading - DsrcConstants::deca2unit<uint16_t>(nodeHeading);
	if (d > 180.0)
		d -= 360.0;
	else if (d < -180.0)
		d += 360.0;
		
	return (d);	
}

double NmapData::MapDataStruct::getHeadingErrorBound(const double ptSpeed) const
{
	if (ptSpeed < lowSpeedThreshold)
		return (headingErrorBoundLowSpeed);
	else
		return (headingErrorBound);
}

int NmapData::MapDataStruct::getIdxByLocationType(const vector<GeoUtils::vehicleTracking_t>& aVehicleTrackingState,const size_t size) const
{
	int idx = -1;
	for (size_t i=0;i<size;i++)
	{
		if (aVehicleTrackingState[i].intsectionTrackingState.vehicleIntersectionStatus == GeoUtils::vehicleInMap_enum_t::ON_APPROACH)
		{
			idx = static_cast<int>(i);
			break;
		}
	}
	if (idx >=0)
		return (idx);
		
	for (size_t i=0;i<size;i++)
	{
		if (aVehicleTrackingState[i].intsectionTrackingState.vehicleIntersectionStatus == GeoUtils::vehicleInMap_enum_t::ON_EGRESS)
		{
			idx = static_cast<int>(i);
			break;
		}
	}
	if (idx >= 0)
		return (idx);
	
	for (size_t i=0;i<size;i++)
	{
		if (aVehicleTrackingState[i].intsectionTrackingState.vehicleIntersectionStatus == GeoUtils::vehicleInMap_enum_t::INSIDE_INTERSECTION_BOX)
		{
			idx = static_cast<int>(i);
			break;
		}
	}
	
	return (idx);		
}

int NmapData::MapDataStruct::getIdx4minimumLateralError(const vector<GeoUtils::vehicleTracking_t>& aApproachTrackingState,const size_t size) const
{
	int idx = -1;
	double dminimum = 1000.0;	// in centimetres
	double d;
	
	for (size_t i=0; i<size; i++)
	{
		d = abs(aApproachTrackingState[i].laneProj.proj2segment.d);
		if (d < dminimum)
		{
			idx = static_cast<int>(i);
			dminimum = d;
		}
	}
	
	return (idx);
}

int NmapData::MapDataStruct::getIdx4minimumLateralError(const vector<GeoUtils::laneTracking_t>& aLaneTrackingState,const size_t size) const
{
	int idx = -1;
	double dminimum = 1000.0;	// in centimetres
	double d;
	
	for (size_t i=0;i<size;i++)
	{
		if (aLaneTrackingState[i].vehicleLaneStatus != GeoUtils::vehicleInMap_enum_t::INSIDE)
			continue;
		d = abs(aLaneTrackingState[i].laneProj.proj2segment.d);
		if (d < dminimum)
		{
			idx = static_cast<int>(i);
			dminimum = d;
		}
	}
	
	return (idx);
}

int NmapData::MapDataStruct::getIdx4minimumLateralError(const vector<GeoUtils::laneProjection_t>& aProj2Lane,const size_t size,const uint16_t laneWidth) const
{
	int idx = -1;
	double dwidth = static_cast<double>(laneWidth) * NmapData::laneWidthRatio;
	double dminimum = static_cast<double>(laneWidth);
	double d;
	for (size_t i=0;i<size;i++)
	{
		d = abs(aProj2Lane[i].proj2segment.d);
		if (aProj2Lane[i].proj2segment.t >= 0.0 && aProj2Lane[i].proj2segment.t <= 1.0 && d <= dwidth && d < dminimum)
		{
			idx = static_cast<int>(i);
			dminimum = d;
		}
	}
	return (idx);
}

int NmapData::MapDataStruct::getIdx4specicalCase(const vector<GeoUtils::laneProjection_t>& aProj2Lane,const size_t size,const uint16_t laneWidth) const
{
	vector<int> indexes;
	double dwidth = static_cast<double>(laneWidth) * NmapData::laneWidthRatio;
	double dminimum = static_cast<double>(laneWidth);
	double d1,d2,d;
	for (size_t i=0;i<size-1;i++)
	{
		d1 = abs(aProj2Lane[i].proj2segment.d);
		d2 = abs(aProj2Lane[i+1].proj2segment.d);
		if (aProj2Lane[i].proj2segment.t > 1.0 && d1 <= dwidth 
			&& aProj2Lane[i+1].proj2segment.t < 0.0 && d2 <= dwidth)
		{
			if (d1 < d2)
				indexes.push_back(static_cast<int>(i));
			else
				indexes.push_back(static_cast<int>(i+1));
		}
	}
	int idx = -1;
	for (size_t i=0;i<indexes.size();i++)
	{
		d = abs(aProj2Lane[indexes[i]].proj2segment.d);
		if (d < dminimum)
		{
			idx = static_cast<int>(indexes[i]);
			dminimum = d;
		}
	}
	return (idx);		
}

GeoUtils::laneTracking_t NmapData::MapDataStruct::projectPt2Lane(const uint8_t intersectionIndex,const uint8_t approachIndex, const uint8_t laneIndex,
	const uint8_t nodeIndex,const uint8_t approachType,const GeoUtils::point2D_t& ptENU,const GeoUtils::motion_t& motionState) const
{
	vector<NmapData::NodeStruct> mpNodes = mpIntersection[intersectionIndex].mpApproaches[approachIndex].mpLanes[laneIndex].mpNodes;
	uint16_t laneWidth = mpIntersection[intersectionIndex].mpApproaches[approachIndex].mpLanes[laneIndex].width;
	double headingErrorBound = getHeadingErrorBound(motionState.speed);
	vector<GeoUtils::laneProjection_t> aProj2Lane;
	GeoUtils::laneProjection_t proj2lane;
	
	if (approachType == maneuver_enum_t::APPROACH)
	{
		for (int i=nodeIndex;i>=1;i--)
		{
			if (abs(getHeadingDifference(mpNodes[i].heading,motionState.heading)) > headingErrorBound)
			{
				continue;
			}
			proj2lane.nodeIndex = static_cast<uint8_t>(i);
			GeoUtils::projectPt2Line(mpNodes[i].ptNode,mpNodes[i-1].ptNode,ptENU,proj2lane.proj2segment);
			aProj2Lane.push_back(proj2lane);
		}
	}
	else
	{
		for (size_t i=nodeIndex;i<mpNodes.size()-1;i++)
		{
			if (abs(getHeadingDifference(mpNodes[i].heading,motionState.heading)) > headingErrorBound)
			{
				continue;
			}
			proj2lane.nodeIndex = static_cast<uint8_t>(i);
			GeoUtils::projectPt2Line(mpNodes[i].ptNode,mpNodes[i+1].ptNode,ptENU,proj2lane.proj2segment);
			aProj2Lane.push_back(proj2lane);
		}
	}
	
	GeoUtils::laneTracking_t laneTrackingState = {GeoUtils::vehicleInMap_enum_t::UNKNOWN,{0,{0.0,0.0,0.0}}};
	if (aProj2Lane.empty())
	{
		return (laneTrackingState);
	}
	if (aProj2Lane[0].proj2segment.t < 0)
	{
		laneTrackingState.vehicleLaneStatus = GeoUtils::vehicleInMap_enum_t::APPROACHING;
		laneTrackingState.laneProj = aProj2Lane[0];
		return (laneTrackingState);
	}
	if (aProj2Lane[aProj2Lane.size()-1].proj2segment.t > 1)
	{
		laneTrackingState.vehicleLaneStatus = GeoUtils::vehicleInMap_enum_t::LEAVING;
		laneTrackingState.laneProj = aProj2Lane[aProj2Lane.size()-1];
		return (laneTrackingState);
	}
	// get index for the lane segment that projection_t.t is between [0,1],
	// projection_t.d is within laneWidth * laneWidthRatio, and has the minimum projection_t.d
	// among all success projected segments (i.e., projection_t.t in [0,1] & abs(projection_t.d) <= laneWidth * laneWidthRatio)
	int idx = getIdx4minimumLateralError(aProj2Lane,aProj2Lane.size(),laneWidth);
	if (idx >= 0)
	{
		laneTrackingState.vehicleLaneStatus = GeoUtils::vehicleInMap_enum_t::INSIDE;
		laneTrackingState.laneProj = aProj2Lane[idx];
		return (laneTrackingState);
	}
	// special case: 
	idx = getIdx4specicalCase(aProj2Lane,aProj2Lane.size(),laneWidth);
	if (idx >= 0)
	{
		laneTrackingState.vehicleLaneStatus = GeoUtils::vehicleInMap_enum_t::INSIDE;
		laneTrackingState.laneProj = aProj2Lane[idx];
		return (laneTrackingState);
	}
	return (laneTrackingState);
}

bool NmapData::MapDataStruct::locateVehicleOnApproach(const uint8_t intersectionIndex,const uint8_t approachIndex,const GeoUtils::point2D_t& ptENU,
	const GeoUtils::motion_t& motionState,GeoUtils::vehicleTracking_t& vehicleTrackingState) const
{
	vehicleTrackingState.reset();
	vector<GeoUtils::laneTracking_t> aLaneTrackingState(mpIntersection[intersectionIndex].mpApproaches[approachIndex].mpLanes.size()); 
	// one record per lane regardless whether the vehicle is on lane or not		
	for (size_t i=0; i<mpIntersection[intersectionIndex].mpApproaches[approachIndex].mpLanes.size(); i++)
	{
		uint8_t startNodeIndex, approachType;
		if (mpIntersection[intersectionIndex].mpApproaches[approachIndex].type == maneuver_enum_t::APPROACH)
		{
			startNodeIndex = static_cast<uint8_t>(mpIntersection[intersectionIndex].mpApproaches[approachIndex].mpLanes[i].mpNodes.size() -1);
			approachType = maneuver_enum_t::APPROACH;
		}
		else
		{
			startNodeIndex = 0;
			approachType = maneuver_enum_t::EGRESS;
		}
		aLaneTrackingState[i] = projectPt2Lane(intersectionIndex,approachIndex,static_cast<uint8_t>(i),startNodeIndex,approachType,ptENU,motionState);
	}
	// when project to multiple lanes, find the lane with minimum distance away from it
	int idx = getIdx4minimumLateralError(aLaneTrackingState,aLaneTrackingState.size());
	if (idx >= 0)
	{
		if (mpIntersection[intersectionIndex].mpApproaches[approachIndex].type == maneuver_enum_t::APPROACH)
			vehicleTrackingState.intsectionTrackingState.vehicleIntersectionStatus = GeoUtils::vehicleInMap_enum_t::ON_APPROACH;
		else
			vehicleTrackingState.intsectionTrackingState.vehicleIntersectionStatus = GeoUtils::vehicleInMap_enum_t::ON_EGRESS;
		vehicleTrackingState.intsectionTrackingState.intersectionIndex = static_cast<int8_t>(intersectionIndex);
		vehicleTrackingState.intsectionTrackingState.approachIndex = static_cast<int8_t>(approachIndex);
		vehicleTrackingState.intsectionTrackingState.laneIndex = static_cast<int8_t>(idx);
		vehicleTrackingState.laneProj = aLaneTrackingState[idx].laneProj;
		return true;
	}
	else
		return false;
}
			
bool NmapData::MapDataStruct::isEgressConnect2Ingress(const vector<NmapData::ConnectStruct>& connectTo,const GeoUtils::geoPoint_t& geoPoint,
	const GeoUtils::motion_t& motionState,GeoUtils::vehicleTracking_t& vehicleTrackingState) const
{
	bool ret = false;
	if (!connectTo.empty())
	{
		// egress lane has at most one connectTo, get indexes (intersection, approach & lane) of connectTo ingress lane
		vector<uint8_t> connectToindex = getIndexesByIds(connectTo[0].intersectionId,connectTo[0].laneId);
		// convert geoPoint to ptENU at connectTo intersection
		GeoUtils::point2D_t ptENU;	
		GeoUtils::lla2enu(mpIntersection[connectToindex[0]].enuCoord,geoPoint,ptENU);
		// check whether ptENU is ON_APPROACH
		if (isPointOnApproach(static_cast<int>(connectToindex[0]),static_cast<int>(connectToindex[1]),ptENU) 
			&& locateVehicleOnApproach(static_cast<int>(connectToindex[0]),static_cast<int>(connectToindex[1]),ptENU,motionState,vehicleTrackingState))
		{
			ret = true;
		}
	}
	return (ret);
}	

double NmapData::MapDataStruct::getPtDist2egress(const uint8_t intersectionIndex,const uint8_t approachIndex,const GeoUtils::point2D_t& ptENU) const
{
	// project ptENU onto the closet lane segment on egress approach
	GeoUtils::projection_t proj2segment;
	double dminimum = 2000.0; // in centimetres
	double d;
	for (size_t i=0;i<mpIntersection[intersectionIndex].mpApproaches[approachIndex].mpLanes.size();i++)
	{
		// should not be empty, as this function is call with connectTo info
		GeoUtils::projectPt2Line(mpIntersection[intersectionIndex].mpApproaches[approachIndex].mpLanes[i].mpNodes[0].ptNode,
			mpIntersection[intersectionIndex].mpApproaches[approachIndex].mpLanes[i].mpNodes[1].ptNode,ptENU,proj2segment);
		d = proj2segment.t * proj2segment.length;
		if (d <= 0 && abs(d) < dminimum)
			dminimum = abs(d);
	}
	return (dminimum);
}

bool NmapData::MapDataStruct::locateVehicleInMap(const GeoUtils::connectedVehicle_t& cv,GeoUtils::vehicleTracking_t& cvTrackingState) const
{
	cvTrackingState.reset();
	GeoUtils::point2D_t ptENU;
	GeoUtils::vehicleTracking_t vehicleTrackingState;
	
	if (!cv.isVehicleInMap)
	{
		// find target intersections that geoPoint is on
		vector<uint8_t> intersectionList = nearedIntersections(cv.geoPoint);
		if (intersectionList.empty())
			return false;
		vector<GeoUtils::vehicleTracking_t> aVehicleTrackingState; // at most one record per intersection
		for (size_t i=0; i<intersectionList.size(); i++)
		{
			// convert cv.geoPoint to ptENU
			GeoUtils::lla2enu(mpIntersection[intersectionList[i]].enuCoord,cv.geoPoint,ptENU);
			// check whether ptENU is inside intersection box first
			if (isPointInsideIntersectionBox(intersectionList[i],ptENU))
			{
				vehicleTrackingState.reset();
				vehicleTrackingState.intsectionTrackingState.vehicleIntersectionStatus = GeoUtils::vehicleInMap_enum_t::INSIDE_INTERSECTION_BOX;
				vehicleTrackingState.intsectionTrackingState.intersectionIndex = static_cast<int8_t>(intersectionList[i]);
				aVehicleTrackingState.push_back(vehicleTrackingState);
				continue;
			}
			// find target approaches that ptENU is on (not-empty APPROACH/EGRESS only)
			vector<uint8_t> approachList = onApproaches(intersectionList[i],ptENU);
			if (approachList.empty())
				continue;
			vector<GeoUtils::vehicleTracking_t> aApproachTrackingState;	// at most one record per approach
			// find target lanes that ptENU is on
			for (size_t j=0; j<approachList.size(); j++)
			{
				if (locateVehicleOnApproach(intersectionList[i],approachList[j],ptENU,cv.motionState,vehicleTrackingState))
				{
					aApproachTrackingState.push_back(vehicleTrackingState);
				}
			}
			if (!aApproachTrackingState.empty())
			{
				// when project to multiple approaches, find the approach/lane with minimum distance away from it
				int idx = getIdx4minimumLateralError(aApproachTrackingState,aApproachTrackingState.size());
				if (idx >= 0)
				{
					aVehicleTrackingState.push_back(aApproachTrackingState[idx]);
				}
			}	
		}
		if (aVehicleTrackingState.empty())
			return false;
		// when ptENU in multiple intersections, find the intersection in the order of ON_APPROACH, ON_EGRESS, INSIDE_INTERSECTION_BOX
		int idx = getIdxByLocationType(aVehicleTrackingState,aVehicleTrackingState.size());
		if (idx < 0)
			return false;
		cvTrackingState = aVehicleTrackingState[idx];
		return true;
	}
	
	// vehicle was already in map, so intersectionIndex is known
	uint8_t intersectionIndex = cv.vehicleTrackingState.intsectionTrackingState.intersectionIndex;
	// convert cv.geoPoint to ptENU
	GeoUtils::lla2enu(mpIntersection[intersectionIndex].enuCoord,cv.geoPoint,ptENU);
	
	if (cv.vehicleTrackingState.intsectionTrackingState.vehicleIntersectionStatus == GeoUtils::vehicleInMap_enum_t::INSIDE_INTERSECTION_BOX)
	{
		// vehicle was initiated inside the intersection box, so approachIndex & laneIndex are unknown
		// next vehicleIntersectionStatus:
		//	1. remain INSIDE_INTERSECTION_BOX
		//	2. on one of the egress approach (ON_EGRESS)
		//	3. NOT_IN_MAP 
		//		GPS error could cause distance away from the ON_EGRESS lane center greater than half lane width (may need to loss the constraint by 3/4 of lane width)
		//		or due to the gap between intersection mpPolygon and approach mpPolygon. This is not a problem for vehicle initiated INSIDE_INTERSECTION_BOX
		//		as it needs to be reset to start tracking anyway
		
		// check whether vehicle remains INSIDE_INTERSECTION_BOX
		if (isPointInsideIntersectionBox(intersectionIndex,ptENU))
		{
			// no change on vehicleTracking_t, return
			cvTrackingState = cv.vehicleTrackingState;
			return true;
		}
		
		// vehicle not INSIDE_INTERSECTION_BOX, check whether it is ON_EGRESS
		int idx = -1;
		vector<GeoUtils::vehicleTracking_t> aApproachTrackingState;	
		vector<uint8_t> approachList = onApproaches(intersectionIndex,ptENU);
		if (!approachList.empty())
		{
			for (size_t j=0; j<approachList.size(); j++)
			{
				// check EGRESS only
				if (mpIntersection[intersectionIndex].mpApproaches[approachList[j]].type == maneuver_enum_t::EGRESS
					&& locateVehicleOnApproach(intersectionIndex,approachList[j],ptENU,cv.motionState,vehicleTrackingState))
				{
					aApproachTrackingState.push_back(vehicleTrackingState);
				}
			}
		}
		if (!aApproachTrackingState.empty())
		{
			// find the approach with minimum distance away from the center of the lane
			idx = getIdx4minimumLateralError(aApproachTrackingState,aApproachTrackingState.size());
		}
		if (idx < 0)
		{
			// vehicle is not INSIDE_INTERSECTION_BOX nor ON_EGRESS, reset tracking to NOT_IN_MAP
			return false;
		}
		
		// vehicle ON_EGRESS, check whether the vehicle is ON_APPROACH that the egress lane connects to
		if (isEgressConnect2Ingress(mpIntersection[aApproachTrackingState[idx].intsectionTrackingState.intersectionIndex].mpApproaches[aApproachTrackingState[idx].intsectionTrackingState.approachIndex].mpLanes[aApproachTrackingState[idx].intsectionTrackingState.laneIndex].mpConnectTo,cv.geoPoint,cv.motionState,vehicleTrackingState))
		{
			cvTrackingState = vehicleTrackingState;
			return true;
		}
		
		// vehicle remains ON_EGRESS (can't find connectTo ON_APPROACH)
		cvTrackingState = aApproachTrackingState[idx];
		return true;
	}
	else if (cv.vehicleTrackingState.intsectionTrackingState.vehicleIntersectionStatus == GeoUtils::vehicleInMap_enum_t::ON_APPROACH)
	{
		// vehicle was ON_APPROACH, so approachIndex & laneIndex are known.
		// next vehicleIntersectionStatus:
		//	1. remain ON_APPROACH
		//	2. enters intersection box (AT_INTERSECTION_BOX)
		//	3. enters egress lane (ON_EGRESS) - for bypass lanes. not likely for lanes going inside the intersection box.
		//		It would take at least takes 1 sec to cross the intersection box, while GPS updating rate is 10 Hz.
		//	4. NOT_IN_MAP
		//		due to GPS error (distance away from the lane center is greater than half lane width), 
		//		or due to the gap between intersection mpPolygon and approach mpPolygon. 
		//		For the second cause, the projected distance onto the closed lane segment is compare against gapDist. If it's less than gapDist,
		//		vehicle is set as AT_INTERSECTION_BOX, otherwise set as NOT_IN_MAP
		uint8_t approachIndex = static_cast<int>(cv.vehicleTrackingState.intsectionTrackingState.approachIndex);
		uint8_t laneIndex = static_cast<int>(cv.vehicleTrackingState.intsectionTrackingState.laneIndex);
		
		// check whether vehicle remains ON_APPROACH
		if (isPointOnApproach(intersectionIndex,approachIndex,ptENU) 
			&& locateVehicleOnApproach(intersectionIndex,approachIndex,ptENU,cv.motionState,vehicleTrackingState))
		{
			cvTrackingState = vehicleTrackingState;
			return true;
		}
		
		// vehicle is not ON_APPROACH, check whether it is ON_EGRESS of ON_APPROACH connectTo (every ON_APPROACH has connectTo at the same intersection)
		set<int> connectToApproachIndex;	// set of egress approach index that ON_APPROACH connects to
		for (size_t i=0;i<mpIntersection[intersectionIndex].mpApproaches[approachIndex].mpLanes[laneIndex].mpConnectTo.size();i++)
		{
			vector<uint8_t> connectToindex = getIndexesByIds(mpIntersection[intersectionIndex].mpApproaches[approachIndex].mpLanes[laneIndex].mpConnectTo[i].intersectionId,
				mpIntersection[intersectionIndex].mpApproaches[approachIndex].mpLanes[laneIndex].mpConnectTo[i].laneId);
			connectToApproachIndex.insert(static_cast<int>(connectToindex[1]));
		}
		int idx = -1;
		vector<GeoUtils::vehicleTracking_t> aApproachTrackingState;	
		vector<uint8_t> approachList = onApproaches(intersectionIndex,ptENU);
		if (!approachList.empty())
		{
			for (size_t j=0; j<approachList.size(); j++)
			{
				// check connectTo EGRESS only
				if (mpIntersection[intersectionIndex].mpApproaches[approachList[j]].type == maneuver_enum_t::EGRESS
					&& connectToApproachIndex.find(approachList[j]) != connectToApproachIndex.end()
					&& locateVehicleOnApproach(intersectionIndex,approachList[j],ptENU,cv.motionState,vehicleTrackingState))
				{
					aApproachTrackingState.push_back(vehicleTrackingState);
				}
			}
		}
		if (!aApproachTrackingState.empty())
		{
			// find the approach with minimum distance away from the center of the lane
			idx = getIdx4minimumLateralError(aApproachTrackingState,aApproachTrackingState.size());
		}
		if (idx >= 0)
		{
			// vehicle ON_EGRESS, check whether the vehicle is ON_APPROACH that the egress lane connects to.
			if (isEgressConnect2Ingress(mpIntersection[aApproachTrackingState[idx].intsectionTrackingState.intersectionIndex].mpApproaches[aApproachTrackingState[idx].intsectionTrackingState.approachIndex].mpLanes[aApproachTrackingState[idx].intsectionTrackingState.laneIndex].mpConnectTo,cv.geoPoint,cv.motionState,vehicleTrackingState))
			{
				cvTrackingState = vehicleTrackingState;
				return true;
			}		
			else
			{
				// vehicle remains ON_EGRESS (can't find connectTo ON_APPROACH)
				cvTrackingState = aApproachTrackingState[idx];
				return true;
			}
		}
		
		// vehicle is not ON_APPROACH nor ON_EGRESS of connectTo approaches
		//	project ptENU onto the closet lane segment on which the vehicle entered the intersection box
		GeoUtils::projection_t proj2segment;
		GeoUtils::projectPt2Line(mpIntersection[intersectionIndex].mpApproaches[approachIndex].mpLanes[laneIndex].mpNodes[1].ptNode,
			mpIntersection[intersectionIndex].mpApproaches[approachIndex].mpLanes[laneIndex].mpNodes[0].ptNode,ptENU,proj2segment);
		double distInto = proj2segment.t * proj2segment.length 
			- static_cast<double>(mpIntersection[intersectionIndex].mpApproaches[approachIndex].mpLanes[laneIndex].mpNodes[1].dTo1stNode);
		if (isPointInsideIntersectionBox(intersectionIndex,ptENU)	|| 
			abs(distInto) < mpIntersection[intersectionIndex].mpApproaches[approachIndex].mpLanes[laneIndex].mpNodes[0].dTo1stNode / 2.0)
		{
			// if inside intersection polygon or distance into is less than gapDist,
			//	set vehicleIntersectionStatus to AT_INTERSECTION_BOX
			cvTrackingState = cv.vehicleTrackingState;
			cvTrackingState.intsectionTrackingState.vehicleIntersectionStatus = GeoUtils::vehicleInMap_enum_t::AT_INTERSECTION_BOX;
			// 	update cvTrackingState.laneProj
			cvTrackingState.laneProj.nodeIndex = 1;
			cvTrackingState.laneProj.proj2segment = proj2segment;
			return true;
		}
		else
			return false;
	}
	else if (cv.vehicleTrackingState.intsectionTrackingState.vehicleIntersectionStatus == GeoUtils::vehicleInMap_enum_t::ON_EGRESS)
	{
		// vehicle was ON_EGRESS, so approachIndex & laneIndex are known.
		// next vehicleIntersectionStatus:
		//	1. switch to ON_APPROACH to the downstream intersection
		//	2. remain ON_EGRESS
		//	3. NOT_IN_MAP (finished ON_EGRESS and no downstream intersections)
		uint8_t approachIndex = static_cast<int>(cv.vehicleTrackingState.intsectionTrackingState.approachIndex);
		uint8_t laneIndex = static_cast<int>(cv.vehicleTrackingState.intsectionTrackingState.laneIndex);
		
		// vehicle ON_EGRESS, check whether the vehicle is ON_APPROACH that the egress lane connects to.
		if (isEgressConnect2Ingress(mpIntersection[intersectionIndex].mpApproaches[approachIndex].mpLanes[laneIndex].mpConnectTo,cv.geoPoint,cv.motionState,vehicleTrackingState))
		{		
			cvTrackingState = vehicleTrackingState;
			return true;
		}
		
		// vehicle not switching to ON_APPROACH, check whether it remains ON_EGRESS
		if (isPointOnApproach(intersectionIndex,approachIndex,ptENU) 
			&& locateVehicleOnApproach(intersectionIndex,approachIndex,ptENU,cv.motionState,vehicleTrackingState))
		{
			// remains ON_EGRESS
			cvTrackingState = vehicleTrackingState;
			return true;
		}
		else
			return false; // NOT_IN_MAP
	}
	else 
	{
		// cv.vehicleTrackingState.intsectionTrackingState.vehicleIntersectionStatus == GeoUtils::vehicleInMap_enum_t::AT_INTERSECTION_BOX
		// vehicle was AT_INTERSECTION_BOX, so approachIndex & laneIndex are known (vehicle used which, ON_APPROACH, to enter the intersection box)
		// next vehicleIntersectionStatus:
		//	1. remain AT_INTERSECTION_BOX
		//	2. on one of the egress approach (ON_EGRESS)
		// 	3. ON_APPROACH
		//		due to GPS overshooting when stopped near the stop-bar, need to give the chance for correction
		//	4. NOT_IN_MAP
		//		due to GPS error (distance away from the lane center is greater than half lane width),
		//		or due to the gap between intersection mpPolygon and approach mpPolygon. 
		//		Same method for case ON_APPROACH is used to check whether need to keep vehicle AT_INTERSECTION_BOX or not
		uint8_t approachIndex = static_cast<int>(cv.vehicleTrackingState.intsectionTrackingState.approachIndex);
		uint8_t laneIndex = static_cast<int>(cv.vehicleTrackingState.intsectionTrackingState.laneIndex);
		
		// check whether vehicle is ON_EGRESS of ON_APPROACH (prior to AT_INTERSECTION_BOX) connectTo
		// this is the same as case ON_APPROACH above
		set<int> connectToApproachIndex;	// set of egress approach index that ON_APPROACH connects to (should not be empty)
		for (size_t i=0;i<mpIntersection[intersectionIndex].mpApproaches[approachIndex].mpLanes[laneIndex].mpConnectTo.size();i++)
		{
			vector<uint8_t> connectToindex = getIndexesByIds(mpIntersection[intersectionIndex].mpApproaches[approachIndex].mpLanes[laneIndex].mpConnectTo[i].intersectionId,
				mpIntersection[intersectionIndex].mpApproaches[approachIndex].mpLanes[laneIndex].mpConnectTo[i].laneId);
			connectToApproachIndex.insert(static_cast<int>(connectToindex[1]));
		}
		int idx = -1;
		vector<GeoUtils::vehicleTracking_t> aApproachTrackingState;	
		vector<uint8_t> approachList = onApproaches(intersectionIndex,ptENU);
		vector<double> dist2egress;
		double d;
		if (!approachList.empty())
		{
			for (size_t j=0; j<approachList.size(); j++)
			{
				// check connectTo EGRESS only
				if (mpIntersection[intersectionIndex].mpApproaches[approachList[j]].type == maneuver_enum_t::EGRESS
					&& connectToApproachIndex.find(approachList[j]) != connectToApproachIndex.end())
				{
					if (locateVehicleOnApproach(intersectionIndex,approachList[j],ptENU,cv.motionState,vehicleTrackingState))
					{
						aApproachTrackingState.push_back(vehicleTrackingState);
					}
					d = getPtDist2egress(intersectionIndex,approachList[j],ptENU);
					if (d < mpIntersection[intersectionIndex].mpApproaches[approachList[j]].mindist2intsectionCentralLine / 2)
					{
					 dist2egress.push_back(d);
					}
				}
			}
		}
		if (!aApproachTrackingState.empty())
		{
			// find the approach with minimum distance away from the center of the lane
			idx = getIdx4minimumLateralError(aApproachTrackingState,aApproachTrackingState.size());
		}
		if (idx >= 0)
		{
			// vehicle ON_EGRESS, check whether the vehicle is ON_APPROACH that the egress lane connects to.
			if (isEgressConnect2Ingress(mpIntersection[aApproachTrackingState[idx].intsectionTrackingState.intersectionIndex].mpApproaches[aApproachTrackingState[idx].intsectionTrackingState.approachIndex].mpLanes[aApproachTrackingState[idx].intsectionTrackingState.laneIndex].mpConnectTo,cv.geoPoint,cv.motionState,vehicleTrackingState))
			{
				cvTrackingState = vehicleTrackingState;
				return true;
			}		
			else
			{
				// vehicle remains ON_EGRESS (can't find connectTo ON_APPROACH)
				cvTrackingState = aApproachTrackingState[idx];
				return true;
			}
		}		 
		
		// vehicle not ON_EGRESS, check whether ptENU is ON_APPROACH on the approach it enters the intersection box
		if (isPointOnApproach(intersectionIndex,approachIndex,ptENU) 
			&& locateVehicleOnApproach(intersectionIndex,approachIndex,ptENU,cv.motionState,vehicleTrackingState))
		{
			cvTrackingState = vehicleTrackingState;
			return true;
		}
		
		//	vehicle not ON_EGRESS nor ON_APPROACH, go to be AT_INTERSECTION_BOX
		//	project ptENU onto the closet lane segment on approaching lane
		GeoUtils::projection_t proj2segment;
		GeoUtils::projectPt2Line(mpIntersection[intersectionIndex].mpApproaches[approachIndex].mpLanes[laneIndex].mpNodes[1].ptNode,
			mpIntersection[intersectionIndex].mpApproaches[approachIndex].mpLanes[laneIndex].mpNodes[0].ptNode,ptENU,proj2segment);
		double distInto = proj2segment.t * proj2segment.length 
			- static_cast<double>(mpIntersection[intersectionIndex].mpApproaches[approachIndex].mpLanes[laneIndex].mpNodes[1].dTo1stNode);
		if (isPointInsideIntersectionBox(intersectionIndex,ptENU)	|| 
			abs(distInto) < mpIntersection[intersectionIndex].mpApproaches[approachIndex].mpLanes[laneIndex].mpNodes[0].dTo1stNode)
		{
			// if inside intersection polygon or distance into is less than gapDist,
			//	set vehicleIntersectionStatus to AT_INTERSECTION_BOX
			cvTrackingState = cv.vehicleTrackingState;
			cvTrackingState.intsectionTrackingState.vehicleIntersectionStatus = GeoUtils::vehicleInMap_enum_t::AT_INTERSECTION_BOX;
			// 	update cvTrackingState.laneProj
			cvTrackingState.laneProj.nodeIndex = 1;
			cvTrackingState.laneProj.proj2segment = proj2segment;
			return true;
		}
		
		// vehicle not near ON_APPROACH, check whether it's near ON_EGRESS
		if (!dist2egress.empty())
		{
			// set vehicleIntersectionStatus to AT_INTERSECTION_BOX
			cvTrackingState = cv.vehicleTrackingState;
			cvTrackingState.intsectionTrackingState.vehicleIntersectionStatus = GeoUtils::vehicleInMap_enum_t::AT_INTERSECTION_BOX;
			// 	update cvTrackingState.laneProj
			cvTrackingState.laneProj.nodeIndex = 1;
			GeoUtils::projectPt2Line(mpIntersection[intersectionIndex].mpApproaches[approachIndex].mpLanes[laneIndex].mpNodes[1].ptNode,
				mpIntersection[intersectionIndex].mpApproaches[approachIndex].mpLanes[laneIndex].mpNodes[0].ptNode,ptENU,cvTrackingState.laneProj.proj2segment);
			return true;
		}
		
		return false;
	}
}

void NmapData::MapDataStruct::updateLocationAware(const GeoUtils::vehicleTracking_t& vehicleTrackingState,
	GeoUtils::locationAware_t& vehicleLocationAware) const
{
	vehicleLocationAware.reset();
	switch(vehicleTrackingState.intsectionTrackingState.vehicleIntersectionStatus)
	{
	case GeoUtils::vehicleInMap_enum_t::NOT_IN_MAP:
		break;
	case GeoUtils::vehicleInMap_enum_t::INSIDE_INTERSECTION_BOX:
		vehicleLocationAware.intersectionId = mpIntersection[vehicleTrackingState.intsectionTrackingState.intersectionIndex].id;
		break;
	default:
		vehicleLocationAware.intersectionId = mpIntersection[vehicleTrackingState.intsectionTrackingState.intersectionIndex].id;
		vehicleLocationAware.laneId = mpIntersection[vehicleTrackingState.intsectionTrackingState.intersectionIndex].mpApproaches[vehicleTrackingState.intsectionTrackingState.approachIndex].mpLanes[vehicleTrackingState.intsectionTrackingState.laneIndex].id;
		vehicleLocationAware.controlPhase = mpIntersection[vehicleTrackingState.intsectionTrackingState.intersectionIndex].mpApproaches[vehicleTrackingState.intsectionTrackingState.approachIndex].mpLanes[vehicleTrackingState.intsectionTrackingState.laneIndex].controlPhase;
	}
	GeoUtils::point2D_t pt;
	getPtDist2D(vehicleTrackingState,pt);
	vehicleLocationAware.dist2go.distLong = DsrcConstants::hecto2unit<int32_t>(pt.x);
	vehicleLocationAware.dist2go.distLat = DsrcConstants::hecto2unit<int32_t>(pt.y);
	GeoUtils::connectTo_t connectTo;
	for (size_t i=0;i<mpIntersection[vehicleTrackingState.intsectionTrackingState.intersectionIndex].mpApproaches[vehicleTrackingState.intsectionTrackingState.approachIndex].mpLanes[vehicleTrackingState.intsectionTrackingState.laneIndex].mpConnectTo.size();i++)
	{
		connectTo.intersectionId = mpIntersection[vehicleTrackingState.intsectionTrackingState.intersectionIndex].mpApproaches[vehicleTrackingState.intsectionTrackingState.approachIndex].mpLanes[vehicleTrackingState.intsectionTrackingState.laneIndex].mpConnectTo[i].intersectionId;
		connectTo.laneId = mpIntersection[vehicleTrackingState.intsectionTrackingState.intersectionIndex].mpApproaches[vehicleTrackingState.intsectionTrackingState.approachIndex].mpLanes[vehicleTrackingState.intsectionTrackingState.laneIndex].mpConnectTo[i].laneId;
		connectTo.laneManeuver = mpIntersection[vehicleTrackingState.intsectionTrackingState.intersectionIndex].mpApproaches[vehicleTrackingState.intsectionTrackingState.approachIndex].mpLanes[vehicleTrackingState.intsectionTrackingState.laneIndex].mpConnectTo[i].laneManeuver;
		vehicleLocationAware.connect2go.push_back(connectTo);
	}
}

void NmapData::MapDataStruct::getPtDist2D(const GeoUtils::vehicleTracking_t& vehicleTrackingState, GeoUtils::point2D_t& pt) const
{
	if (vehicleTrackingState.intsectionTrackingState.vehicleIntersectionStatus == GeoUtils::vehicleInMap_enum_t::NOT_IN_MAP
		|| vehicleTrackingState.intsectionTrackingState.vehicleIntersectionStatus == GeoUtils::vehicleInMap_enum_t::INSIDE_INTERSECTION_BOX)
	{
		pt.x = 0;
		pt.y = 0;
		return;
	}
	
	int8_t intersectionIndex = vehicleTrackingState.intsectionTrackingState.intersectionIndex;
	int8_t approachIndex = vehicleTrackingState.intsectionTrackingState.approachIndex;
	int8_t laneIndex = vehicleTrackingState.intsectionTrackingState.laneIndex;
	int8_t nodeIndex = vehicleTrackingState.laneProj.nodeIndex;
	uint32_t nodeDistTo1stNode = mpIntersection[intersectionIndex].mpApproaches[approachIndex].mpLanes[laneIndex].mpNodes[nodeIndex].dTo1stNode;
	if (nodeIndex == 0)
		nodeDistTo1stNode = 0;	// first node dTo1stNode is distance to the intersection central line, not the stop-bar
	double ptIntoLine = vehicleTrackingState.laneProj.proj2segment.t * vehicleTrackingState.laneProj.proj2segment.length;
	pt.y = static_cast<int32_t>(vehicleTrackingState.laneProj.proj2segment.d);
	if (vehicleTrackingState.intsectionTrackingState.vehicleIntersectionStatus == GeoUtils::vehicleInMap_enum_t::ON_EGRESS)
	{
		// pt.x is distance downstream from the intersection box boundary, node index has the same direction of lane projection
		pt.x = static_cast<int32_t>((double)nodeDistTo1stNode + ptIntoLine);
	}
	else
	{
		// pt.x is distance to stop-bar, node index has opposite direction from lane projection  
		pt.x = static_cast<int32_t>((double)nodeDistTo1stNode - ptIntoLine);
		// positive pt.x ON_APPROACH, negative AT_INTERSECTION_BOX
	}
}

void NmapData::MapDataStruct::nmap_fprintf(FILE* fp) const
{
	GeoUtils::geoPoint_t geoPoint;
	
	fprintf(fp,"MAP_Version\t%u :: %zu Intersections\n",mMapVersion,mpIntersection.size());
	fprintf(fp,"--------------------------------------------------------------------------------\n");
	for (size_t i=0;i<mpIntersection.size();i++)
	{
		GeoUtils::geoRefPoint2geoPoint(mpIntersection[i].geoRef,geoPoint);
		fprintf(fp,"MAP_Name\t%s\n",mpIntersection[i].name.c_str());
		fprintf(fp,"RSU_ID\t%s\n",mpIntersection[i].rsuId.c_str());
		fprintf(fp,"IntersectionID\t%u\n",mpIntersection[i].id);
		bitset<8> b1(mpIntersection[i].attributes);
		fprintf(fp,"Intersection_attributes\t%s\n",b1.to_string().c_str());
		fprintf(fp,"Reference_point\t%.7f\t%.7f\t%.1f\t%.1f\n",
			geoPoint.latitude,geoPoint.longitude,geoPoint.elevation * DsrcConstants::deca,
			DsrcConstants::hecto2unit<uint32_t>(mpIntersection[i].radius));
		fprintf(fp,"No_Approach\t%zu\n",mpIntersection[i].mpApproaches.size());
		for (size_t j=0;j<mpIntersection[i].mpApproaches.size();j++)
		{
			fprintf(fp,"Approach\t%u\n",mpIntersection[i].mpApproaches[j].id);			
			fprintf(fp,"Approach_type\t%u\n",mpIntersection[i].mpApproaches[j].type);
			fprintf(fp,"No_lane\t%zu\n",mpIntersection[i].mpApproaches[j].mpLanes.size());
			for (size_t k=0;k<mpIntersection[i].mpApproaches[j].mpLanes.size();k++)
			{
				fprintf(fp,"Lane\t%u.%zu\t%u\n",
					mpIntersection[i].mpApproaches[j].id,k+1,
					mpIntersection[i].mpApproaches[j].mpLanes[k].controlPhase);
				fprintf(fp,"Lane_ID\t%u\n",mpIntersection[i].mpApproaches[j].mpLanes[k].id);				
				fprintf(fp,"Lane_type\t%u\n",mpIntersection[i].mpApproaches[j].mpLanes[k].type);
				bitset<16> b2(mpIntersection[i].mpApproaches[j].mpLanes[k].attributes);
				fprintf(fp,"Lane_attributes\t%s\n",b2.to_string().c_str());
				fprintf(fp,"Lane_width\t%u\n",mpIntersection[i].mpApproaches[j].mpLanes[k].width);
				fprintf(fp,"No_nodes\t%zu\n",mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes.size());
				for (size_t ii = 0;ii<mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes.size(); ii++)
				{
					int flag = 0;
					if (mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes[ii].isBroadcast)
					{
						flag = 1;
					}
					GeoUtils::geoRefPoint2geoPoint(mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes[ii].geoNode,geoPoint);
					fprintf(fp,"%u.%zu.%zu\t%.7f\t%.7f\t%d\t%.2f\t%.2f\t%.2f\t%.1f\n",
						mpIntersection[i].mpApproaches[j].id,k+1,ii+1,
						geoPoint.latitude,geoPoint.longitude,flag,
						DsrcConstants::hecto2unit<int32_t>(mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes[ii].ptNode.x),
						DsrcConstants::hecto2unit<int32_t>(mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes[ii].ptNode.y),
						DsrcConstants::hecto2unit<uint32_t>(mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes[ii].dTo1stNode),
						DsrcConstants::deca2unit<uint16_t>(mpIntersection[i].mpApproaches[j].mpLanes[k].mpNodes[ii].heading));
				}
				fprintf(fp,"No_Conn_lane\t%zu\n",mpIntersection[i].mpApproaches[j].mpLanes[k].mpConnectTo.size());
				for (size_t ii = 0;ii<mpIntersection[i].mpApproaches[j].mpLanes[k].mpConnectTo.size(); ii++)
				{
					vector<uint8_t> inds = getIndexesByIds(mpIntersection[i].mpApproaches[j].mpLanes[k].mpConnectTo[ii].intersectionId,
						mpIntersection[i].mpApproaches[j].mpLanes[k].mpConnectTo[ii].laneId);
					if (!inds.empty())
					{	
						fprintf(fp,"%u.%u.%u\t%u\n",
							inds[0]+1,mpIntersection[i].mpApproaches[inds[1]].id,inds[2]+1,
							mpIntersection[i].mpApproaches[j].mpLanes[k].mpConnectTo[ii].laneManeuver);
					}
				}
				fprintf(fp,"end_lane\n");
			}
			fprintf(fp,"end_approach\n");
		}
		fprintf(fp,"end_map\n");
		fprintf(fp,"--------------------------------------------------------------------------------\n");
	}
}	

void NmapData::MapDataStruct::nmapApproachLaneNums_fprintf(FILE* fp) const
{
	fprintf(fp,"MAP_Version\t%u :: %zu Intersections\n",mMapVersion,mpIntersection.size());
	fprintf(fp,"--------------------------------------------------------------------------------\n");
	for (size_t i=0;i<mpIntersection.size();i++)
	{
		fprintf(fp,"MAP_Name\t%s\n",mpIntersection[i].rsuId.c_str());
		for (size_t j=0;j<mpIntersection[i].mpApproaches.size();j++)
		{
			fprintf(fp,"Approach::LaneNums %u::%zu\n",
				mpIntersection[i].mpApproaches[j].id,
				mpIntersection[i].mpApproaches[j].mpLanes.size());
		}
		fprintf(fp,"ingressAproachNums::egressApproachNums::crossWalkNums::sum %zu::%zu::%zu::%zu\n",
			mpIntersection[i].ingressIndex.size(),mpIntersection[i].egressIndex.size(),mpIntersection[i].crossWalkIndex.size(),
			mpIntersection[i].ingressIndex.size()+mpIntersection[i].egressIndex.size()+mpIntersection[i].crossWalkIndex.size());
		fprintf(fp,"--------------------------------------------------------------------------------\n");	
	}
}
	
void NmapData::MapDataStruct::nmapPahseLaneSet_fprintf(FILE* fp) const
{
	char buffer[20];
	int len;
	fprintf(fp,"MAP_Version\t%u :: %zu Intersections\n",mMapVersion,mpIntersection.size());
	fprintf(fp,"--------------------------------------------------------------------------------\n");
	for (size_t i=0;i<mpIntersection.size();i++)
	{
		fprintf(fp,"MAP_Name\t%s\n",mpIntersection[i].rsuId.c_str());
		fprintf(fp,"Phase2LaneSet\n");
		for (uint8_t phase = 1; phase < 9; phase++)
		{
			len = getLaneSetByPhase(mpIntersection[i].id,phase,buffer);
			if (len > 0)
			{
				fprintf(fp,"Phase %d:",phase);
				for (int j = 0; j < len; j++)
				{
					vector<uint8_t> inds = getIndexesByIds(mpIntersection[i].id,buffer[j]);
					if (!inds.empty())
					{
						fprintf(fp," %u.%u",mpIntersection[inds[0]].mpApproaches[inds[1]].id,inds[2]+1);
					}
				}
				fprintf(fp,"\n");
			}
		}
		fprintf(fp,"PedPhase2LaneSet\n");
		for (uint8_t phase = 1; phase < 9; phase++)
		{
			len = getLaneSetByPedPhase(mpIntersection[i].id,phase,buffer);
			if (len > 0)
			{
				fprintf(fp,"PedPhase %d:",phase);
				for (int j = 0; j < len; j++)
				{
					vector<uint8_t> inds = getIndexesByIds(mpIntersection[i].id,buffer[j]);
					if (!inds.empty())
					{
						fprintf(fp," %u.%u",mpIntersection[inds[0]].mpApproaches[inds[1]].id,inds[2]+1);
					}
				}
				fprintf(fp,"\n");
			}
		}
		fprintf(fp,"--------------------------------------------------------------------------------\n");	
	}					
}

void NmapData::MapDataStruct::nmapConnectTo_fprintf(FILE* fp)	const
{
	fprintf(fp,"MAP_Version\t%u :: %zu Intersections\n",mMapVersion,mpIntersection.size());
	fprintf(fp,"--------------------------------------------------------------------------------\n");
	for (size_t i=0;i<mpIntersection.size();i++)
	{
		fprintf(fp,"MAP_Name\t%s From::To::Manurer\n",mpIntersection[i].rsuId.c_str());
		for (size_t j = 0; j < mpIntersection[i].mpApproaches.size(); j++)
		{
			for (size_t k = 0; k < mpIntersection[i].mpApproaches[j].mpLanes.size(); k++)
			{
				for (size_t ii = 0; ii < mpIntersection[i].mpApproaches[j].mpLanes[k].mpConnectTo.size(); ii++)
				{
					int bitNum = mpIntersection[i].mpApproaches[j].mpLanes[k].mpConnectTo[ii].laneManeuver-1;	
					vector<uint8_t> inds = getIndexesByIds(mpIntersection[i].mpApproaches[j].mpLanes[k].mpConnectTo[ii].intersectionId,
						mpIntersection[i].mpApproaches[j].mpLanes[k].mpConnectTo[ii].laneId);
					if (!inds.empty())
					{
						uint8_t movementNum = (uint8_t)(mpIntersection[i].mpApproaches[j].id
							+ maneuver_enum_t::msConnectManeuverApproachIncrease[bitNum]
							- mpIntersection[i].mpApproaches[inds[1]].id);
						movementNum %= 8;
						fprintf(fp,"%zu.%u.%zu To %u.%u.%u %s(%u)",
							i+1,mpIntersection[i].mpApproaches[j].id,k+1,
							inds[0]+1,mpIntersection[i].mpApproaches[inds[1]].id,inds[2]+1,	maneuver_enum_t::msConnectManeuverCode[bitNum].c_str(),
							mpIntersection[i].mpApproaches[j].mpLanes[k].mpConnectTo[ii].laneManeuver);
						if (movementNum != 0)
						{
							fprintf(fp," ConnectTo Approach WRONG! %u\n",movementNum);
						}
						else
						{
							fprintf(fp,"\n");
						}
					}
				}
			}
		}
		fprintf(fp,"--------------------------------------------------------------------------------\n");	
	}
}

void NmapData::MapDataStruct::nmapEgressNoConnectTo_fprintf(FILE* fp) const
{
	fprintf(fp,"MAP_Version\t%u :: %zu Intersections\n",mMapVersion,mpIntersection.size());
	fprintf(fp,"Intersections with egress lanes that has no connecTo info\n");	
	fprintf(fp,"--------------------------------------------------------------------------------\n");
	for (size_t i=0;i<mpIntersection.size();i++)
	{
		for (size_t j = 0; j < mpIntersection[i].mpApproaches.size(); j++)
		{
			if (mpIntersection[i].mpApproaches[j].type != maneuver_enum_t::EGRESS)
				continue;
			for (size_t k = 0; k < mpIntersection[i].mpApproaches[j].mpLanes.size(); k++)
			{
				if (mpIntersection[i].mpApproaches[j].mpLanes[k].mpConnectTo.empty())
				{
					fprintf(fp,"MAP_Name %s %zu.%u.%zu\n",
						mpIntersection[i].rsuId.c_str(),i+1,mpIntersection[i].mpApproaches[j].id,k+1);
				}
			}
		}
	}
}

void NmapData::MapDataStruct::nmapWaypoints_fprintf(FILE* fp) const
{
	fprintf(fp,"MAP_Version\t%u :: %zu Intersections\n",mMapVersion,mpIntersection.size());
	size_t index;
	size_t ingress;
	GeoUtils::geoPoint_t geoPoint;
	
	for (int i = 0;i<2;i++)
	{
		if (i==0)
		{
			// Northbound starts at Charleston
			index = mpIntersection.size() - 1;
			ingress = 2;
			fprintf(fp,"--------------------------------------------------------------------------------\n");
			fprintf(fp,"Northbound way-points:\n");
		}
		else
		{
			// Southbound starts at Stanford
			index = 1;
			ingress = 6;
			fprintf(fp,"--------------------------------------------------------------------------------\n");
			fprintf(fp,"Southbound way-points:\n");	
		}
		
		for (size_t j = 0; j < mpIntersection[index].mpApproaches[ingress].mpLanes.size(); j++)
		{
			fprintf(fp,"--------------------------------------------------------------------------------\n");
			fprintf(fp,"Staring %zu.%zu.%zu\n",index+1,ingress+1,j+1);	
			size_t intersectionIndex = index;
			size_t ingressIndex = ingress;
			size_t laneIndex = j;
			bool reachEnd = false;
			bool isStraightMovement = false;
			while(1)
			{
				for (size_t k = 0; k < mpIntersection[intersectionIndex].mpApproaches[ingressIndex].mpLanes[laneIndex].mpConnectTo.size(); k++)
				{
					if (mpIntersection[intersectionIndex].mpApproaches[ingressIndex].mpLanes[laneIndex].mpConnectTo[k].laneManeuver 
						== connectTo_enum_t::STRAIGHTAHEAD)
					{
						isStraightMovement = true;
						for (size_t ii = 0; ii < mpIntersection[intersectionIndex].mpApproaches[ingressIndex].mpLanes[laneIndex].mpNodes.size(); ii++)
						{
							// ingress way-points (towards upstream)
							size_t nodeIndex = (size_t)(mpIntersection[intersectionIndex].mpApproaches[ingressIndex].mpLanes[laneIndex].mpNodes.size()-1-ii);
							GeoUtils::geoRefPoint2geoPoint(mpIntersection[intersectionIndex].mpApproaches[ingressIndex].mpLanes[laneIndex].mpNodes[nodeIndex].geoNode,geoPoint);
							fprintf(fp,"%zu.%zu.%zu.%zu\t%.7f\t%.7f\n",
								intersectionIndex+1,ingressIndex+1,laneIndex+1,nodeIndex+1,geoPoint.latitude,geoPoint.longitude);						
						}
						// get egress lane (included in origin nmap)
						vector<uint8_t> indexes = getIndexesByIds(mpIntersection[intersectionIndex].mpApproaches[ingressIndex].mpLanes[laneIndex].mpConnectTo[k].intersectionId,
							mpIntersection[intersectionIndex].mpApproaches[ingressIndex].mpLanes[laneIndex].mpConnectTo[k].laneId);
						// get downstream ingress lane (generated by MapDataStruct::getConnectToForEgressLane
						if (mpIntersection[indexes[0]].mpApproaches[indexes[1]].mpLanes[indexes[2]].mpConnectTo.empty()
							|| mpIntersection[indexes[0]].mpApproaches[indexes[1]].mpLanes[indexes[2]].mpConnectTo[0].laneManeuver
								!= connectTo_enum_t::STRAIGHT)
						{
							reachEnd = true;		
							// last egress
							for (size_t ii = 0; ii < mpIntersection[indexes[0]].mpApproaches[indexes[1]].mpLanes[indexes[2]].mpNodes.size(); ii++)
							{
								// egress way-points (towards downstream)
								GeoUtils::geoRefPoint2geoPoint(mpIntersection[indexes[0]].mpApproaches[indexes[1]].mpLanes[indexes[2]].mpNodes[ii].geoNode,geoPoint);
								
								fprintf(fp,"%u.%u.%u.%zu\t%.7f\t%.7f\n",
									indexes[0]+1,indexes[1]+1,indexes[2]+1,ii+1,geoPoint.latitude,geoPoint.longitude);						
							}
						}
						else
						{
							// move to the downstream intersection (no need to print way-points here, will be printed on the next loop
							vector<uint8_t> inds = getIndexesByIds(mpIntersection[indexes[0]].mpApproaches[indexes[1]].mpLanes[indexes[2]].mpConnectTo[0].intersectionId,
								mpIntersection[indexes[0]].mpApproaches[indexes[1]].mpLanes[indexes[2]].mpConnectTo[0].laneId);
							intersectionIndex = inds[0];
							ingressIndex= inds[1];
							laneIndex = inds[2];
						}
						break;
					}
				}
				if (reachEnd || !isStraightMovement)
					break;
			}
		}
	}
}

void NmapData::MapDataStruct::nmapIntersectionPolygon_fprintf(FILE* fp) const
{
	fprintf(fp,"MAP_Version\t%u :: %zu Intersections\n",mMapVersion,mpIntersection.size());
	GeoUtils::geoPoint_t geoPoint;
	
	for (size_t i=0;i<mpIntersection.size();i++)
	{
		fprintf(fp,"--------------------------------------------------------------------------------\n");
		fprintf(fp,"Intersection %s mpPolygon type %u\n",
			mpIntersection[i].rsuId.c_str(),mpIntersection[i].mpPolygonType);
		for (size_t k=0;k<mpIntersection[i].mpPolygon.size();k++)
		{
			GeoUtils::enu2lla(mpIntersection[i].enuCoord,mpIntersection[i].mpPolygon[k],geoPoint);
			fprintf(fp,"%d,%d,%.8f,%.8f\n",mpIntersection[i].mpPolygon[k].x,mpIntersection[i].mpPolygon[k].y,
				geoPoint.latitude,geoPoint.longitude);
		}	
		for (size_t j=0;j<mpIntersection[i].mpApproaches.size();j++)
		{
			if (mpIntersection[i].mpApproaches[j].mpPolygon.empty())
				continue;
			fprintf(fp,"--------------------------------------------------------------------------------\n");
			fprintf(fp,"Intersection %s approach %zu mpPolygon type %u\n",
				mpIntersection[i].rsuId.c_str(),j+1,mpIntersection[i].mpApproaches[j].mpPolygonType);
			for (size_t k=0;k<mpIntersection[i].mpApproaches[j].mpPolygon.size();k++)
			{	
				GeoUtils::enu2lla(mpIntersection[i].enuCoord,mpIntersection[i].mpApproaches[j].mpPolygon[k],geoPoint);
				fprintf(fp,"%d,%d,%.8f,%.8f\n",mpIntersection[i].mpApproaches[j].mpPolygon[k].x,mpIntersection[i].mpApproaches[j].mpPolygon[k].y,
					geoPoint.latitude,geoPoint.longitude);
			}
		}
	}
}
		
void NmapData::MapDataStruct::nmapPolygonConvexity_fprintf(FILE* fp) const
{
	for (size_t i=0;i<mpIntersection.size();i++)
	{
		if (mpIntersection[i].mpPolygonType != GeoUtils::polygon_enum_t::CONVEX)
		{
			fprintf(fp,"Intersection %s box non-convex\n",mpIntersection[i].rsuId.c_str());
			for (size_t k=0;k<mpIntersection[i].mpPolygon.size();k++)
			{
				fprintf(fp,"%d,%d\n",mpIntersection[i].mpPolygon[k].x,
					mpIntersection[i].mpPolygon[k].y);
			}	
		}
		
		for (size_t j=0;j<mpIntersection[i].mpApproaches.size();j++)
		{
			if (mpIntersection[i].mpApproaches[j].mpPolygon.empty())
				continue;
			if (mpIntersection[i].mpApproaches[j].mpPolygonType != GeoUtils::polygon_enum_t::CONVEX)
			{
				fprintf(fp,"Intersection %s approach %zu box non-convex\n",mpIntersection[i].rsuId.c_str(), j+1);
				for (size_t k=0; k<mpIntersection[i].mpApproaches[j].mpPolygon.size();k++)
				{
					fprintf(fp,"%d,%d\n",mpIntersection[i].mpApproaches[j].mpPolygon[k].x,
						mpIntersection[i].mpApproaches[j].mpPolygon[k].y);
				}
			}
		}
	}
}
