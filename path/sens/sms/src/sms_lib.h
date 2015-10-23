/**\file
 * 	Header file to be included by client programs
 * 	using sms_lib to get SMS data from the PATH data server.
 *
 * 	Copyright (c) 2009   Regents of the University of California
 *
 */
#ifndef SMS_LIB_H
#define SMS_LIB_H

// No more than 6r objects per object list.
// Also tracking IDs are between 0 and 63.
#define SMS_MAX_OBJ  64
// Used to initialize reverse lookup array with invalid value
#define SMS_INVALID_INDEX 255

// Use these macros to make sure object data is valid.
#define SMS_DEFINED_RANGE(z) ((z) > -819.3 || (z) < 819.2)
#define SMS_DEFINED_VELOCITY(z) ((z) > -102.5 || (z) < 102.4)

/** Parsed data for an object tracked by the SMS radar
 *  xrange and yrange are from the calibrated 0,0 of the
 *  SMS intersection
 */
typedef struct {
	float   xrange;		// -819.2 to +819.1 meters
	float   yrange;		// -819.2 to +819.1 meters
	float   xvelocity;	// -102.4 to +104.3 m/sec
	float   yvelocity;	// -102.4 to +104.3 m/sec
	float   object_length;	// 0 represents a point track
				// !=0 represents an object
				// values 0-51 
	unsigned char object_id;	// from the SMS tracking filter, 0-63
} IS_PACKED sms_object_t;


/** Structure containing all tracked objects in a given object list
 *  returned from the SMS radar. 
 */
typedef struct {
        timestamp_t smstime;
        unsigned char num_obj;
        char t_scan;
        char mode;
        unsigned int sms_cycle_counter;
        sms_object_t obj[SMS_MAX_OBJ];
        unsigned char reverse_lookup[SMS_MAX_OBJ];
} IS_PACKED sms_object_list_t;

/** Accesses data server as required to get SMS object records stored there
 * Stores a set of objects all with the same tracking timestamp 
 * in an sms_object_list_t structure, with some additional information
 * about the object list, including the number of tracked objects.
 */
extern void sms_get_object_list(db_clt_typ *pclt, sms_object_list_t *plist);

/** Takes a pointer to sms_object_list_t that has been filled in by
 * sms_get_object_list and uses it to fill in an array of tracked
 * objects, up to the maximum size. Returns the number of tracked 
 * objects actually filled in
 */
extern int sms_get_object_array(sms_object_list_t *plist, sms_object_t *parray,
	int max_size);

/** Returns tracked object with a particular obj_id from 
 * sms_object_list_t structure; returns TRUE if an object
 * with the supplied object_id is present in the list, and
 * fills in structure pointed to by pobj with the SMS object data. 
 */
extern bool_typ sms_get_object_by_id(sms_object_list_t *plist,
	sms_object_t *pobj, unsigned char object_id);

/** Prints "num_to_print" radar track objects
 * Some may be empty, if list contains fewer thanr equested.
 */
extern int sms_sprint_list(char *sms_buffer, sms_object_list_t *plist,
                 timestamp_t write_time, int num_to_print);

/**      Used to read in log files for post-analysis and replay
 *      sms_buffer contains the line, data from line is put into
 *      sms_object_list_t, time that line was logged is returned
 *      in pwrite_time, num_to_print is used in case there was
 *      a limit on the number of objects printed.
 */
extern int sms_sscan_list(char *sms_buffer, sms_object_list_t *plist,
                 timestamp_t *pwrite_time, int num_to_print);

#endif  //ifndef SMS_LIB_H
