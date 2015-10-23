/*\\file
 *	
 *	Header file for function to do a 2D transform (rotation, translation and
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

extern void xy2dtransform(double x0, double y0, double angle, double scale,
	double *x, double *y, double *newx, double *newy, int n);
		

