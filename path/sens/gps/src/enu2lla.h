#ifndef ENU2LLA_H
#define ENU2LLA_H

#ifndef PI
#define PI           3.1415926535897930L
#endif

#define WGS84_A 6378137.0L
#define WGS84_E 8.1819190842622e-2L
#define WGS84_E_SQR 6.6943799901414e-3L

int lla2ecef(double latitude, double longitude, double altitude, 
	     double *x, double *y, double *z);

int ecef2lla(double x, double y, double z, 
	     double *latitude, double *longitude, double *altitude);

int enu2lla(double dE, double dN, double dU,
	    double lat_ref, double long_ref, double alt_ref,
	    double *latitude, double *longitude, double *altitude);

#endif /* ENU2LLA_H */
