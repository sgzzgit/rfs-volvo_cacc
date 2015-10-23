/**\file 
 *  
 *         SMS radar client routines
 *
 *         Access PATH data server variables written
 *         by smsparse.c (also in this directory) and
 *         provide the data in an easier to use format. 
 *    
 *         Copyright (c) 2009   Regents of the University of California
 */      
#include <sys_os.h>
#include "sys_rt.h"
#include "local.h"
#include "timestamp.h"
#include "db_clt.h"
#include "db_utils.h"
#include "smsparse.h"
#include "sms_lib.h"
#include "sys_buff.h"
#include "data_log.h"

/**
 *   Returns the cycle counter for the list
 */
static int get_list_from_db(db_clt_typ *pclt, sms_object_list_t *plist,
		objlist_typ *pdb)
{
	db_clt_read(pclt, DB_SMS_OBJLIST_VAR, sizeof (objlist_typ), pdb);

	// Initialize reverse lookup array to invalid array index 255
	memset(&plist->reverse_lookup[0], SMS_INVALID_INDEX, SMS_MAX_OBJ); 

	plist->smstime = pdb->ts;
	plist->num_obj = pdb->num_obj; 
	plist->t_scan = pdb->t_scan; 
	plist->mode = pdb->mode; 
	plist->sms_cycle_counter = pdb->cyc_cnt;
	return (plist->sms_cycle_counter);
}

/**
 *   Returns TRUE if cycle_counter on the DB VAR matches that
 *   passed in from the list for every object, otherwise FALSE
 */
static bool_typ get_objs_from_db(db_clt_typ *pclt, sms_object_list_t *plist,
		objlist_typ *pdb, int cycle_counter)
{
	int i;
	for (i = 0; i < pdb->num_obj; i++) {
		int obj_id = pdb->arr[i];
		int db_var = DB_VAR_SMS_OBJ(obj_id);
		int db_index = DB_VAR_SMS_OBJ_INDEX(obj_id);
		smsobjarr_typ db_sms_arr;
		smsobj_typ *pdbobj;

		db_clt_read(pclt, db_var, sizeof(db_sms_arr), &db_sms_arr);
		if (db_sms_arr.cyc_cnt != cycle_counter)
			return FALSE;

		pdbobj = &db_sms_arr.object[db_index];
		plist->obj[i].xrange = pdbobj->xrange;
		plist->obj[i].yrange = pdbobj->yrange;
		plist->obj[i].xvelocity = pdbobj->xvel;
		plist->obj[i].yvelocity = pdbobj->yvel;
		plist->obj[i].object_length = pdbobj->length;
		plist->obj[i].object_id = pdbobj->obj;
		plist->reverse_lookup[obj_id] = i;
	}
	return TRUE;
}

/** Accesses data server as required to get SMS object records stored there
 * Stores a set of objects all with the same tracking timestamp 
 * in an sms_object_list_t structure, with some additional information
 * about the object list, including the number of tracked objects.
 */
void sms_get_object_list(db_clt_typ *pclt, sms_object_list_t *plist)
{
	objlist_typ sms_db_list;
	objlist_typ *pdb = &sms_db_list;
	int cycle_counter = 0;
	
	// Keep trying until cycle_counter is consistent
	while (TRUE) {
		memset(plist, 0, sizeof(sms_object_list_t));
		memset(pdb, 0, sizeof(objlist_typ));
		cycle_counter = get_list_from_db(pclt, plist, pdb);
		if (get_objs_from_db(pclt, plist, pdb, cycle_counter))
			return;
		else
			continue;
	}
}

/** Takes a pointer to sms_object_list_t that has been filled in by
 * sms_get_object_list and uses it to fill in an array of tracked
 * objects, up to the maximum size. Returns the number of tracked 
 * objects actually filled in
 */
int sms_get_object_array(sms_object_list_t *plist, sms_object_t *parray,
         int max_size)
{
	int i;
	for (i = 0; i < plist->num_obj && i < max_size; i++) {
		parray[i] = plist->obj[i];
	}
	return i;
}

/** Returns tracked object with a particular obj_id from 
 * sms_object_list_t structure; returns TRUE if an object
 * with the supplied object_id is present in the list, and
 * fills in structure pointed to by pobj with the SMS object data. 
 */
bool_typ sms_get_object_by_id(sms_object_list_t *plist,
                 sms_object_t *pobj, unsigned char object_id)
{
	int location = plist->reverse_lookup[object_id]; 
	if (location == SMS_INVALID_INDEX)
		return 0;
	else {
		*pobj = plist->obj[location];
		return 1;
	}
}

static sms_object_t empty_obj = {999.0, 999.0, 999.0, 999.0, 99.0, 255};

/**
 * 	Used for logging, returns number of characters in buffer
 *	If you want to print status only, set num_to_print to 0
 */
int sms_sprint_list(char *sms_buffer, sms_object_list_t *plist,
		 timestamp_t write_time, int num_to_print)
{
	sms_object_t *pobjs = &plist->obj[0];
	int i;
	int cc = 0;
        cc += (sprint_timestamp(sms_buffer+cc, &write_time));
        cc += (sprintf(sms_buffer+cc, " "));
        cc += (sprint_timestamp(sms_buffer + cc, &plist->smstime));
        cc += (sprintf(sms_buffer + cc, " "));
 	cc += (sprintf(sms_buffer + cc, "%hhd ", plist->t_scan));
 	cc += (sprintf(sms_buffer + cc, "%hhd ", plist->mode));
 	cc += (sprintf(sms_buffer + cc, "%d ", plist->sms_cycle_counter));
 	cc += (sprintf(sms_buffer + cc, "%hhu ", plist->num_obj));
	for (i = 0; i < plist->num_obj && i < num_to_print; i++) {
		cc += (sprintf(sms_buffer + cc, "%hhu ", pobjs[i].object_id));
		cc += (sprintf(sms_buffer+cc, "%.1f ", pobjs[i].xrange));
		cc += (sprintf(sms_buffer+cc, "%.1f ", pobjs[i].yrange));
		cc += (sprintf(sms_buffer+cc, "%.1f ", pobjs[i].xvelocity));
		cc += (sprintf(sms_buffer+cc, "%.1f ", pobjs[i].yvelocity));
		cc += (sprintf(sms_buffer+cc, "%.1f ", pobjs[i].object_length));
	}
	for (i = plist->num_obj; i < num_to_print; i++) {
		cc += (sprintf(sms_buffer + cc, "%hhu ",empty_obj.object_id));
		cc += (sprintf(sms_buffer+cc, "%.1f ", empty_obj.xrange));
		cc += (sprintf(sms_buffer+cc, "%.1f ", empty_obj.yrange));
		cc += (sprintf(sms_buffer+cc, "%.1f ", empty_obj.xvelocity));
		cc += (sprintf(sms_buffer+cc, "%.1f ", empty_obj.yvelocity));
		cc += (sprintf(sms_buffer+cc, "%.1f ", empty_obj.object_length));
	}
	return cc;
}


/**
 * 	Used to read in log files for post-analysis and replay
 *	sms_buffer contains the line, data from line is put into
 *	sms_object_list_t, time that line was logged is returned
 *	in pwrite_time, num_to_print is used in case there was
 *	a limit on the number of objects printed.
 */
int sms_sscan_list(char *sms_buffer, sms_object_list_t *plist,
		 timestamp_t *pwrite_time, int num_to_print)
{
	char tmpbuf[MAX_LINE_LEN];
	sms_object_t *pobjs = &plist->obj[0];
	int i;
	int cur = 0;	// current position in input string
	int cc = 0;	// character count for field
	int retval = 0;	// from each scanf, should always be 1
	cc = get_data_log_next_field(&sms_buffer[cur], tmpbuf, MAX_LINE_LEN);	
	retval += str2timestamp(tmpbuf, pwrite_time);
	cur += cc;
	cc = get_data_log_next_field(&sms_buffer[cur], tmpbuf, MAX_LINE_LEN);	
	retval += str2timestamp(tmpbuf, &plist->smstime);
	cur += cc;
	cc = get_data_log_next_field(&sms_buffer[cur], tmpbuf, MAX_LINE_LEN);	
 	retval += (sscanf(tmpbuf, "%hhd ", &plist->t_scan));
	cur += cc;
	cc = get_data_log_next_field(&sms_buffer[cur], tmpbuf, MAX_LINE_LEN);	
 	retval += (sscanf(tmpbuf, "%hhd ", &plist->mode));
	cur += cc;
	cc = get_data_log_next_field(&sms_buffer[cur], tmpbuf, MAX_LINE_LEN);	
 	retval += (sscanf(tmpbuf, "%u ", &plist->sms_cycle_counter));
	cur += cc;
	cc = get_data_log_next_field(&sms_buffer[cur], tmpbuf, MAX_LINE_LEN);	
 	retval += (sscanf(tmpbuf, "%hhu ", &plist->num_obj));
	for (i = 0; i < plist->num_obj && i < num_to_print; i++) {
		cur += cc;
		cc = get_data_log_next_field(&sms_buffer[cur], tmpbuf,
			 MAX_LINE_LEN);	
		retval += (sscanf(tmpbuf, "%hhu ", &pobjs[i].object_id));
		cur += cc;
		cc = get_data_log_next_field(&sms_buffer[cur], tmpbuf,
			 MAX_LINE_LEN);	
		retval += (sscanf(tmpbuf, "%f ", &pobjs[i].xrange));
		cur += cc;
		cc = get_data_log_next_field(&sms_buffer[cur], tmpbuf,
			 MAX_LINE_LEN);	
		retval += (sscanf(tmpbuf, "%f ", &pobjs[i].yrange));
		cur += cc;
		cc = get_data_log_next_field(&sms_buffer[cur], tmpbuf,
			 MAX_LINE_LEN);	
		retval += (sscanf(tmpbuf, "%f ", &pobjs[i].xvelocity));
		cur += cc;
		cc = get_data_log_next_field(&sms_buffer[cur], tmpbuf,
			 MAX_LINE_LEN);	
		retval += (sscanf(tmpbuf, "%f ", &pobjs[i].yvelocity));
		cur += cc;
		cc = get_data_log_next_field(&sms_buffer[cur], tmpbuf,
			 MAX_LINE_LEN);	
		retval += (sscanf(tmpbuf, "%f ", &pobjs[i].object_length));
	}
	return retval;	// total number of columns read
}
