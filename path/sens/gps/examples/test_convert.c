#include <stdio.h>
#include "sys_os.h"
#include "sys_rt.h"
#include "local.h"
#include "timestamp.h"
#include "path_gps_lib.h"

#define LAT_180    37.914613L
#define LONG_180 -122.333841L
#define ALT_180    10.2266

int main(int argc, char **argv)
{
   double lat, lon;
   double x, y, u;
   path_gps_point_t origin;
   int counter = 0;

   origin.latitude = LAT_180;
   origin.longitude = LONG_180;
   origin.altitude = ALT_180;

   while (EOF != fscanf(stdin, "%lg %lg", &lat, &lon))
   {
      path_gps_point_t gpsin;
      path_gps_point_t gpsout;
      gpsin.latitude = lat;
      gpsin.longitude = lon;

      printf("INPUT: %.9f %.9f\n", lat, lon);

      path_gps_latlong2xy(&gpsin, origin, &y, &x, 1, GPS_ELLIPSOID_EARTH);
      u = 0.0;

      printf("PATH XY: %.9f %.9f %.9f\n", x, y, u);

      path_gps_xy2latlong(&x, &y, origin, &gpsout, 1);
      
      printf("XY2LATLONG OUTPUT: %.9f %.9f\n", gpsout.latitude,
	     gpsout.longitude);
      counter++;
   }
   return counter;
}
