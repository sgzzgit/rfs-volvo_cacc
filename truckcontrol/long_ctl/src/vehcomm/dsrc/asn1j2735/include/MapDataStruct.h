#ifndef _MAP_DATA_STRUCT_H
#define _MAP_DATA_STRUCT_H

#include <stdint.h>		// c++11 <cstdint>
#include <string>
#include <cstring>
#include <vector>
#include <map>

#include "dsrcConsts.h"
#include "geoUtils.h"

// forward declaration
class AsnJ2735Lib;

namespace NmapData
{
	static const double lowSpeedThreshold = 0.2;		// speed lower than this heading is questionable
	static const double headingErrorBound = 45.0;
	static const double headingErrorBoundLowSpeed = 200.0;
	static const double laneWidthRatio = 1.5;	
	
	struct maneuver_enum_t
	{
		enum approachType { APPROACH = 1, EGRESS, BARRIER, CROSSWALK };
		enum laneType {TRAFFICLANE = 1,CROSSWALKLANE = 4};
		static const uint8_t msConnectManeuverApproachIncrease[];
		static const std::string msConnectManeuverCode[];
	};
	
	struct ConnectStruct
	{
		uint32_t 	intersectionId;
		uint8_t 	laneId;
		connectTo_enum_t::maneuverType laneManeuver; // bit number in Maneuver Code
	};
	struct NodeStruct
	{
		// A lane can have variable node objects (at most 20)
		// Order of node sequence starts from near intersection to downstream (egress) or upstream (approach)
		GeoUtils::geoRefPoint_t geoNode;
		GeoUtils::point2D_t ptNode;		// in centimetre, reference to intersection ref point		
		uint32_t dTo1stNode;					// in centimetre, reference to the first node on the same lane
		uint16_t heading;							// in decidegree
		bool isBroadcast;
	};
	struct LaneStruct
	{
		// An approach can have variable lane objectives (at most 8)
		// Lanes under an approach can be controlled by different traffic lights 
		// (e.g., left-turn, through on one approach)
		// Order of lane sequence starts from the curb lane to central lane
		uint8_t 	id;
		maneuver_enum_t::laneType type;
		uint16_t	attributes;
		uint16_t	width;				// in centimetres
		uint8_t		controlPhase;
		std::vector<NmapData::ConnectStruct> mpConnectTo;
		std::vector<NmapData::NodeStruct> mpNodes;		
	};
	struct ApproachStruct
	{
		// Each intersection has 12 approach objects, 8 for driving lanes and 4 cross-walks. 
		// If approach is not exit, number of lanes is 0 on the approach
		// so approach id actually is the sequence id
		// Order of approach sequence starts from EB approach (not egress!) counter-clockwise to NB egress
		uint8_t id;
		maneuver_enum_t::approachType type;
		std::vector<NmapData::LaneStruct> mpLanes;
		std::vector<GeoUtils::point2D_t> mpPolygon;
		GeoUtils::polygon_enum_t::polygonType mpPolygonType;	
		uint32_t mindist2intsectionCentralLine;	// in centimetres
	};
	struct IntersectionStruct
	{
		std::string		name;
		std::string		rsuId;
		uint32_t			id;
		uint8_t				attributes;
		GeoUtils::geoRefPoint_t	geoRef;
		GeoUtils::enuCoord_t		enuCoord;
		uint32_t			radius;				// in centimetres
		std::vector<uint8_t> ingressIndex;
		std::vector<uint8_t> egressIndex;
		std::vector<uint8_t> crossWalkIndex;				
		std::vector<NmapData::ApproachStruct> mpApproaches;
		std::vector<GeoUtils::point2D_t> mpPolygon;				
		GeoUtils::polygon_enum_t::polygonType mpPolygonType;
		std::string mapPayload;
	};
		
	class MapDataStruct
	{
		friend class ::AsnJ2735Lib;
		
		private:
			std::vector<NmapData::IntersectionStruct> mpIntersection;
		
			uint8_t mMapVersion;		
			std::map< unsigned long, uint32_t > IndexMap;		// (intersectionId, laneId) -> (intersection, approach, lane)indexes in mpIntersection
			std::map< std::string, uint32_t > name2IdMap;		// intersectionName -> intersectionId
			std::map< unsigned long, std::vector<uint8_t> > Phase2LaneSetMap; 	// (intersectionId, phaseId) -> laneSet
			std::map< unsigned long, std::vector<uint8_t> > PedPhase2LaneSetMap;// (intersectionId, phaseId) -> laneSet		
			
			void readNmap(const char* ptr);
			void buildConnectToForEgressLane(void);
			void setLocalOffsetAndHeading(void);
			void buildPolygons(void);
			void buildPhase2LaneSetMap(void);
			std::vector<uint8_t> getLaneSetByPhase(const uint32_t intersectionId,const uint8_t phase) const;
			std::vector<uint8_t> getLaneSetByPedPhase(const uint32_t intersectionId,const uint8_t phase) const;
			int getLaneSetByPhase(const uint32_t intersectionId,const uint8_t phase,char* ptr) const;
			int getLaneSetByPedPhase(const uint32_t intersectionId,const uint8_t phase,char* ptr) const;			
			std::vector<uint8_t> getIndexesByIds(const uint32_t intersectionId,const uint8_t laneId) const;	// return (intersection, approach, lane)indexes in mpIntersection
			int	getIndexByIntersectionId(const uint32_t intersectionId) const;  
			
			std::vector<uint8_t> nearedIntersections(const GeoUtils::geoPoint_t& geoPoint) const;
			bool isPointNearIntersection(const uint8_t intersectionIndex, const GeoUtils::geoPoint_t& geoPoint) const;
			bool isPointInsideIntersectionBox(const uint8_t intersectionIndex, const GeoUtils::point2D_t& ptENU) const;		
			std::vector<uint8_t> onApproaches(const uint8_t intersectionIndex, const GeoUtils::point2D_t& ptENU) const;
			bool isPointOnApproach(const uint8_t intersectionIndex, const uint8_t approachIndex, const GeoUtils::point2D_t& ptENU) const;
			double getHeadingDifference(const uint16_t nodeHeading,const double ptHeading) const;				
			double getHeadingErrorBound(const double ptSpeed) const;
			bool locateVehicleOnApproach(const uint8_t intersectionIndex,const uint8_t approachIndex,const GeoUtils::point2D_t& ptENU,
				const GeoUtils::motion_t& motionState,GeoUtils::vehicleTracking_t& vehicleTrackingState) const;			
			GeoUtils::laneTracking_t projectPt2Lane(const uint8_t intersectionIndex,const uint8_t approachIndex, const uint8_t laneIndex,
				const uint8_t nodeIndex,const uint8_t approachType,const GeoUtils::point2D_t& ptENU,const GeoUtils::motion_t& motionState) const;
			int getIdx4minimumLateralError(const std::vector<GeoUtils::laneProjection_t>& aProj2Lane,const size_t size,const uint16_t laneWidth) const;
			int getIdx4specicalCase(const std::vector<GeoUtils::laneProjection_t>& aProj2Lane,const size_t size,const uint16_t laneWidth) const;
			int getIdx4minimumLateralError(const std::vector<GeoUtils::laneTracking_t>& aLaneTrackingState,const size_t size) const;
			int getIdx4minimumLateralError(const std::vector<GeoUtils::vehicleTracking_t>& aApproachTrackingState,const size_t size) const;
			int getIdxByLocationType(const std::vector<GeoUtils::vehicleTracking_t>& aVehicleTrackingState,const size_t size) const;
			bool isEgressConnect2Ingress(const std::vector<NmapData::ConnectStruct>& connectTo,const GeoUtils::geoPoint_t& geoPoint,
				const GeoUtils::motion_t& motionState,GeoUtils::vehicleTracking_t& vehicleTrackingState) const;
			void updateLocationAware(const GeoUtils::vehicleTracking_t& vehicleTrackingState,
				GeoUtils::locationAware_t& vehicleLocationAware) const;
			double getPtDist2egress(const uint8_t intersectionIndex,const uint8_t approachIndex,const GeoUtils::point2D_t& ptENU) const;
			
		public:		
			// constructor
			MapDataStruct(const char* ptr);
			// destructor
			~MapDataStruct(void);
			
			// get static map data elements
			uint8_t getMapVersion(void) const;		
			size_t getIntersectionIdList(uint32_t* list,const size_t size) const;		
			size_t getIntersectionNameById(const uint32_t intersectionId,char* intersectionName,const size_t size) const;		
			std::string getIntersectionNameById(const uint32_t intersectionId) const;
			uint32_t getIntersectionIdByName(const char* intersectionName) const;			// null terminated intersectionName
			uint8_t getControlPhaseByLaneId(const uint32_t intersectionId,const uint8_t laneId) const;	
			std::string getIntersectionNameByIndex(const uint8_t intersectionIndex) const;

			// map related with GPS-trace		
			bool locateVehicleInMap(const GeoUtils::connectedVehicle_t& cv,GeoUtils::vehicleTracking_t& cvTrackingState) const;
			void getPtDist2D(const GeoUtils::vehicleTracking_t& vehicleTrackingState, GeoUtils::point2D_t& pt) const;
			
			// print
			void nmap_fprintf(FILE* fp) const;
			void nmapApproachLaneNums_fprintf(FILE* fp) const;
			void nmapPahseLaneSet_fprintf(FILE* fp) const;
			void nmapConnectTo_fprintf(FILE* fp) const;	
			void nmapEgressNoConnectTo_fprintf(FILE* fp) const;
			void nmapWaypoints_fprintf(FILE* fp) const;
			void nmapIntersectionPolygon_fprintf(FILE* fp) const;			
			void nmapPolygonConvexity_fprintf(FILE* fp) const;			
	};
}
#endif
