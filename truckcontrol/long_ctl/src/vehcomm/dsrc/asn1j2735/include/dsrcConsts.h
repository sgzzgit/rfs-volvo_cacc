#ifndef _DSRC_CONSTS_H
#define _DSRC_CONSTS_H

class DsrcConstants
{
	public:
		static const double ellipsoid_r = 6371008.7714;				//earth’s mean radius in meter
		static const double ellipsoid_a = 6378137.0; 					//Semi-major axis, in meters (WGS84)
		static const double ellipsoid_e = 0.0818191908426215; //First eccentricity (WGS84)
		static const double rad2degree = 57.2957795;
		static const double deca = 10.0;
		static const double hecto = 100.0;
		static const double damega = 10000000.0;
		
		DsrcConstants(void){};
		~DsrcConstants(void){};
		static double deg2rad(const double d)
		{
			return (d / rad2degree);
		};
		static double rad2deg(const double d)
		{
			return (d * rad2degree);
		};
		template<class T> 
		static double damega2unit(const T d)
		{
			return (static_cast<double>(d) / damega);		
		};
		template<class T> 
		static T unit2damega(const double d)
		{
			return (static_cast<T>(d * damega));
		};
		template<class T> 
		static double hecto2unit(const T d)
		{
			return (static_cast<double>(d) / hecto);
		};
		template<class T> 
		static T unit2hecto(const double d)
		{
			return (static_cast<T>(d * hecto));
		};		
		template<class T> 
		static double deca2unit(const T d)
		{
			return (static_cast<double>(d) / deca);
		}		
		template<class T> 
		static T unit2deca(const double d)
		{
			return (static_cast<T>(d * deca));
		}		
};

#endif
