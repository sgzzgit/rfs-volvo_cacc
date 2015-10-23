/*\\file
 *	
 *	Function to do a 2D transform (rotation, translation and
 *	scaling) from one (X,Y) coordinate system to another on
 *	arrays of x and y coordinates.
 *
 *	For use with GPS coordinates, first use path_gps_latlong2xy
 *	from the library in built in path/sens/gps/src to convert to
 *	x-axis East, y-axis North, meter coordinates.
 *
 *	Copyright (c) 2006   Regents of the University of California
 *
 */
#include <sys_os.h>

/**
 *	x0 and y0 are the coordinates in the old coordinate
 *	system of the new origin; angle is in degrees from the
 *	old x-axis to the new x-axis. 
 *	'n' is the size of the arrays, which must be allocated in
 *	the calling scope.
 */ 
void xy2dtransform(double x0, double y0, double angle, double scale,
	double *x, double *y, double *newx, double *newy, int n)
{
	double theta = ((2.0 * M_PI)/360.0) * angle;
	double stheta = sin(theta);
	double ctheta = cos(theta);
	int i;
	for (i = 0; i < n; i++) {
		newx[i] = scale*(stheta*(y[i]-y0)+ctheta*(x[i]-x0));
		newy[i] = scale*(ctheta*(y[i]-y0)-stheta*(x[i]-x0));
	}
}
		

