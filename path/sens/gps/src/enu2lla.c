#include <math.h>
#include "matrix.h"
#include "enu2lla.h"

/* function [x,y,z]=lla2ecef(lat,lon,alt)
 *
 * LLA2ECEF - convert latitude, longitude, and altitude to
 *            earth-centered, earth-fixed (ECEF) cartesian
 *
 * USAGE:
 * [x,y,z] = lla2ecef(lat,lon,alt)
 *
 * x = ECEF X-coordinate (m)
 * y = ECEF Y-coordinate (m)
 * z = ECEF Z-coordinate (m)
 * lat = geodetic latitude (radians)
 * lon = longitude (radians)
 * alt = height above WGS84 ellipsoid (m)
 *
 * Notes: This function assumes the WGS84 model.
 *        Latitude is customary geodetic (not geocentric).
 *
 * Source: "Department of Defense World Geodetic System 1984"
 *         Page 4-4
 *         National Imagery and Mapping Agency
 *         Last updated June, 2004
 *         NIMA TR8350.2
 *
 * Michael Kleder, July 2005
 */
int lla2ecef(double latitude, double longitude, double altitude, 
	     double *x, double *y, double *z)
{
  /* intermediate calculation (prime vertical radius of curvature)
   * N = a ./ sqrt(1 - e^2 .* sin(lat).^2);
   */

  double N = WGS84_A / sqrt(1 - (WGS84_E_SQR * (sin(latitude)*sin(latitude))));

  /* results: */
  *x = (N+altitude) * cos(latitude) * cos(longitude);
  *y = (N+altitude) * cos(latitude) * sin(longitude);
  *z = ((1-WGS84_E_SQR) * N + altitude) * sin(latitude);

  return 1;
}

/* function [lat,lon,alt] = ecef2lla(x,y,z)
 *
 * ECEF2LLA - convert earth-centered earth-fixed (ECEF)
 *            cartesian coordinates to latitude, longitude,
 *            and altitude
 *
 * USAGE:
 * [lat,lon,alt] = ecef2lla(x,y,z)
 *
 * lat = geodetic latitude (radians)
 * lon = longitude (radians)
 * alt = height above WGS84 ellipsoid (m)
 * x = ECEF X-coordinate (m)
 * y = ECEF Y-coordinate (m)
 * z = ECEF Z-coordinate (m)
 *
 * Notes: (1) This function assumes the WGS84 model.
 *        (2) Latitude is customary geodetic (not geocentric).
 *        (3) Inputs may be scalars, vectors, or matrices of the same
 *            size and shape. Outputs will have that same size and shape.
 *        (4) Tested but no warranty; use at your own risk.
 *        (5) Michael Kleder, April 2006
 */
int ecef2lla(double x, double y, double z, 
	     double *latitude, double *longitude, double *altitude)
{
  /* calculations: */

  /* b   = sqrt(a^2*(1-e^2)); */
  double b   = sqrt(WGS84_A*WGS84_A * (1 - WGS84_E_SQR));
  /* ep  = sqrt((a^2-b^2)/b^2); */
  double ep  = sqrt(((WGS84_A*WGS84_A - b*b)/(b*b)));
  /* p   = sqrt(x.^2+y.^2); */
  double p  = sqrt(x*x + y*y); 
  /* th  = atan2(a*z,b*p); */
  double th  = atan2(WGS84_A*z, b*p);
  /* lon = atan2(y,x); */
  double lon = atan2(y, x);
  /* lat = atan2((z+ep^2.*b.*sin(th).^3),(p-e^2.*a.*cos(th).^3)); */
  double lat = atan2((z+ep*ep * b * pow(sin(th),3)),
		     (p - WGS84_E_SQR * WGS84_A * pow(cos(th),3)));
  /* N   = a./sqrt(1-e^2.*sin(lat).^2); */
  double N = WGS84_A / sqrt(1 - (WGS84_E_SQR * (sin(lat)*sin(lat))));

  /* alt = p./cos(lat)-N; */
  double alt = p / cos(lat) - N;

  /* return lon in range [-pi,pi) */
  if (-PI > lon) {
    *longitude = lon + 2*PI;
  } else if ( PI <= lon) {
    *longitude = lon - 2*PI;
  } else {
    *longitude = lon;
  }

  /* correct for numerical instability in altitude near exact poles:
   * (after this correction, error is about 2 millimeters, which is about
   * the same as the numerical precision of the overall function)
   *
   *  k=abs(x)<1 & abs(y)<1;
   *  alt(k) = abs(z(k))-b;
   *
   * if x, y not near 0, k is false
   * if both x, y near 0, k is true 
   * This overrides values of altitude vector when k is true
   */

  *altitude = alt;
  if ((fabs(x) < 1.0) && (fabs(y) < 1.0)) {
    *altitude = fabs(z) - b;
  }
  
  *latitude = lat;

  return 1;
}

/* function [lat,long,alt] = enu2lla(dE,dN,dU,ref_lla)
 *
 * This function convert positions from ENU (east,north,up)coordinate
 * relative to a reference point to LLA (latitude,longitude,altitude) for
 * WGS84 model.
 *
 * Inputs:
 * dE,dN,dU: relative positions (East,North,Up) in meter.
 *
 * optional input:
 * ref_lla: (latitude,longitude,altitude)of the reference point in
 * (degree,degree,meter) using WGS84 model.
 *
 * If reference point is not provided, use default value at the
 * differential base station.
 * default reference:
 * lat_ref = 37+ 54.8767679/60; (deg)
 * long_ref = 122 + 20.0304416/60; (deg)
 * alt_ref =  10.2266; -22.9248 (m)
 * REF_GPS_RFS = [122.000000+20.0304416/60, 37.000000+54.8767679/60, 10.2266];
 *
 * Ouputs:
 * lat = geodetic latitude (degree)
 * long = longitude (degree)
 * alt = height above WGS84 ellipsoid (m)
 */
int enu2lla(double dE, double dN, double dU,
	    double lat_ref, double long_ref, double alt_ref,
	    double *latitude, double *longitude, double *altitude)
{

  double **matrix_trans;
  double **matrix_temp;
  double x, y, z;
  /* change units: */
  double lat_0 = lat_ref * PI/180.0;   /* unit: degree-> radian */
  double long_0 = long_ref * PI/180.0; /* unit: degree-> radian */

  /* get the reference position in ECEF coordinate */
  double x0, y0, z0;
  int status = lla2ecef(lat_0, long_0, alt_ref, &x0, &y0, &z0);

  if (1 != status) return status;
  
  /* Construct transformation matrix 
   * matrix_temp = [ -sin(long_0),cos(long_0),0;
   *                 -sin(lat_0)*cos(long_0), -sin(lat_0)*sin(long_0), cos(lat_0);
   *                 cos(lat_0)*cos(long_0), cos(lat_0)*sin(long_0), sin(lat_0)
   *               ];
   *
   * matrix_trans = inv(matrix_temp);
   */

  matrix_temp = zero3();

  matrix_temp[0][0] = -sin(long_0);
  matrix_temp[0][1] = cos(long_0);

  matrix_temp[1][0] = -sin(lat_0)*cos(long_0);
  matrix_temp[1][1] = -sin(lat_0)*sin(long_0);
  matrix_temp[1][2] = cos(lat_0);

  matrix_temp[2][0] = cos(lat_0)*cos(long_0);
  matrix_temp[2][1] = cos(lat_0)*sin(long_0);
  matrix_temp[2][2] = sin(lat_0);

  matrix_trans = inverse3(matrix_temp);
  free_mat3(matrix_temp);

  /* transform positions from ENU to ECEF
   *        temp_ECEF = matrix_trans* [dE(i);dN(i);dU(i)];
   *        x(i) = temp_ECEF(1);
   *        y(i) = temp_ECEF(2);
   *        z(i) = temp_ECEF(3);
   */

  x = matrix_trans[0][0] * dE + 
             matrix_trans[0][1] * dN +
             matrix_trans[0][2] * dU;
  y = matrix_trans[1][0] * dE + 
             matrix_trans[1][1] * dN +
             matrix_trans[1][2] * dU;
  z = matrix_trans[2][0] * dE + 
             matrix_trans[2][1] * dN +
             matrix_trans[2][2] * dU;

  free_mat3(matrix_trans);

  /* positions in ECEF coordinate (in meters) */
  x = x + x0;
  y = y + y0;
  z = z + z0;

  /* transform from ECEF to LLA (WGS84)*/
  status = ecef2lla(x,y,z,latitude,longitude,altitude);
  if (1 != status ) return status;

  /* change units of lati. and long. (deg--> radian) */
  *latitude = *latitude * 180.0/PI;
  *longitude = *longitude * 180.0/PI;

  return 1;
}
