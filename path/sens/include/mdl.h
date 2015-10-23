/**\file	
 *	mdl.h
 *
 *  MDL IML15OHR lidar
 */

#ifndef MDL_LIDAR_H
#define MDL_LIDAR_H


/* \file:  mdl.h 
 *
 * Copyright 2007 Regents of the University of California
 *
 * Contains structure and database variable definitions for mdl
 * Also contains definitions of message types
 */

#define DB_MDL_LIDAR_TYPE 660
#define DB_MDL_LIDAR_VAR DB_MDL_LIDAR_TYPE  //make sure it is not in conflict with others

#define MDL_MAX_BYTES 134

/** Structure used by parsing code. MDL lidar send binary data over the RS232 serial port. 
 *  mdl_msg_typ defines the field of the binary data as the message comes from lidar. And 
 *  the lidar can only tracks one target as well as collecting only the range parameter.
**/
  
typedef struct {

	timestamp_t ts;
	float range;

} mdl_lidar_typ; 

void mdl_print( mdl_lidar_typ *pmdl_lidar) ;
bool_typ mdl_ser_driver_read(unsigned short *prange, int debug);

#endif
