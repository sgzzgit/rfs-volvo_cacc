/**\file
 *
 *	Functions in this directory support data logging usingfiles with
 *	unique names automatically created and keyed to the time.
 *
 *	Also supports table definition of variables to be logged
 *	and replay programs.
 *
 *      Copyright (c) 2008-9   Regents of the University of California
 *
 *	Generalization of the code written by Paul Kretz and
 *	used in wrfiles.c in many projects. Installed in local
 *	directory by Sue Dickey.
 *
 */
#include "sys_os.h"
#include "sys_list.h"
#include "data_log.h"
#include "timestamp.h"

/* The following function will open a data file in the directory
 * indicated by the "prefix" parameter using the nameing convention
 *
 * 	<prefix>MMDDSSS.<suffix> 
 *
 * where <prefix> will indicate an absolute path name from the current
 * directory if it begins with /, otherwise a relative path name from
 * the current directory.
 *
 * In the above names, MM is replaced by a 2-digit month code, DD is
 * replaced by a 2-digit day code, and SSS is replaced by a 3-digit
 * serial code.  Serial codes for a given day start at 000 and proceed
 * to 999.  If more than 999 files are written on the same date, the number
 * will be reset to 000 and an attempt to find a number with no file is
 * made.  If no serial number is available, no file will be opened so
 * no data will be recorded.  
 *
 * If the value in old_fileday does not match the current date, the
 * serial number (file_serialno) will be reset to zero.  This will happen
 * the first time open_file is called (since old_fileday was deliberately
 * initialized to 99, an invalid day) and if the system ever runs through
 * a day change at midnight.
 *
 * The monthdayserialnum MMDDSSS is returned as a string so that if multiple
 * files with different prefixes are to be written at the same time,
 * the same suffix can be used.
 *
 * A return value of 1 indicates open succeeded, 0 indicates a problem
 */
int open_data_log(FILE ** pf_data, /// pass in file pointer to be set
		 char* prefix, 	   /// e.g., "a" or "e"
		 char* suffix,	   /// e.g., ".dat"
		 double *pstart_time,	/// will be set by open_data_log
		 int *pold_fileday,     /// initialize bad day value, e.g. 99
		 int *pcounter, 	/// will be set to serial number 
		 char *monthdayserialnum /// if not NULL, string  of form
					  /// e.g. "0131001" will be copied out 
					  /// for use in opening other files
		)
{
	int old_fileday = *pold_fileday; /// set illegal value on first call
	int file_serialno = *pcounter;	/// will be set or incremented
	int status;
	struct timespec now;
	struct timeb timeptr_raw;
	struct tm time_converted;
	char filename[80];
	char temp[80];
	bool_typ file_exists = TRUE;

	/* Get date information. */
	ftime (&timeptr_raw);
	//_localtime (&timeptr_raw.time, &time_converted);
	localtime_r (&timeptr_raw.time, &time_converted);

	/* If old_fileday is not the same as time_converted.tm_mday, reset
	 * the serial number to zero.  This will happen first time through, or
	 * if we run through midnight. */
	if (old_fileday != time_converted.tm_mday)
		file_serialno = 0;
	old_fileday = time_converted.tm_mday;

	while(file_exists == TRUE) {
		sprintf(temp, "%2.2d%2.2d%3.3d",
			time_converted.tm_mon+1, time_converted.tm_mday,
			file_serialno);
		sprintf(filename, "%s%s%s", prefix, temp, suffix);
		*pf_data = fopen(filename, "r");
		if (*pf_data == NULL)
			file_exists = FALSE;
		else {
			fclose(*pf_data);
			file_serialno++;
		}
	}

	/* We've found a valid serial number (and filename for radar file is
	 * still stored in the variable filename).  Now open all the necessary
	 * files. */
	*pf_data = fopen(filename, "w");
	if (*pf_data == NULL) {
		perror("open_data_log");
		return 0;
	}

	/* Set start_time on successful open */

	status = clock_gettime( CLOCK_REALTIME, &now );
	*pstart_time = now.tv_sec + ((double) now.tv_nsec/ 1000000000L);
	file_serialno++;

	*pcounter = file_serialno;	

	if (monthdayserialnum != NULL)
		strncpy(monthdayserialnum, temp, 80);

	return 1;
}
    
/** Determine the time since files were opened. 
 *  If the current time is more than the allotted time for file
 *  collection, close the current set of files and open a new file.
 *  pstart_time should originally be set by the first call to open_data_log. 
 *  The return value can be used to open a set of files with open
 *  and close synchronized to the first.
 */

int reopen_data_log(FILE ** pf_data, int file_time, char *prefix, char *suffix,
			double *pstart_time, 
			int *pold_fileday, int *pcounter,
			char *monthdayserialnum, buff_typ *pbuf)
{
	struct timespec now;
	double curr_time;
	int status;
	status = clock_gettime( CLOCK_REALTIME, &now );
	curr_time = now.tv_sec +
                        ((double) now.tv_nsec/1000000000L) - *pstart_time;

	if ( curr_time >= file_time * 60.0 ) {
		if (pbuf != NULL)
			buff_done(pbuf);
                fclose(*pf_data);
		open_data_log(pf_data, prefix, suffix, pstart_time,
			pold_fileday, pcounter, monthdayserialnum);
		return 1;
	}
	return 0;
}

/**
 *	The following function uses an entry from an array of
 *	data_log_column_spec_t to format the data into a string
 *	Returns the number of characters put in the string.
 */
int sprint_data_log_column_entry(char *strbuf, data_log_column_spec_t *pentry)
{
	int cc = 0;
	timestamp_t ts;

	switch (pentry->base_type) {
	case BASE_CHAR:
		cc += (sprintf(strbuf, pentry->format_string,
			*((char *) pentry->field_pointer)));
		break;
	case BASE_SHORT:
		cc += (sprintf(strbuf, pentry->format_string,
			*((short *) pentry->field_pointer)));
		break;
	case BASE_INT:
	case BASE_HEX_INT:
		cc += (sprintf(strbuf, pentry->format_string,
			*((int *) pentry->field_pointer)));
		break;
	case BASE_FLOAT:
		cc += (sprintf(strbuf, pentry->format_string,
			*((float *) pentry->field_pointer)));
		break;
	case BASE_DOUBLE:
		cc += (sprintf(strbuf, pentry->format_string,
			*((double *) pentry->field_pointer)));
		break;
	case BASE_STRING:
		cc += (sprintf(strbuf, pentry->format_string,
			(char *) pentry->field_pointer));
		break;
	case BASE_TIMESTAMP:
		ts = *((timestamp_t *) pentry->field_pointer); 
		if (strncmp("HH:MM:SS.SSS ", pentry->format_string, 13) == 0) {
			timestamp2str(&ts, strbuf);
			cc += (sprintf(strbuf, "%s ", strbuf)); 
		} else {
			// print as double according to format string
			cc += (sprintf(strbuf, pentry->format_string,
				TS_TO_SEC(&ts)));
		}
		break;
	default:
		fprintf(stderr, "unrecognized BASE %d\n", pentry->base_type);
		break;
	}
	return cc;
}
		 
/**
 *	The following function uses an entry from an array of
 *	data_log_column_spec_t to scan strbuf and fill in the
 *	variable pointed to by the field pointer. 
 *
 *	Returns the return value from sccanf on strbuf.
 */
int sscan_data_log_column_entry(char *strbuf, data_log_column_spec_t *pentry)
{
	int retval = 0;
	timestamp_t *pts;
	double secs_since_midnight = 0.0;	// seconds since midnight
	int millisecs;	// used in converting double secs since midnight to ts

	switch (pentry->base_type) {
	case BASE_CHAR:
		retval =(sscanf(strbuf, pentry->format_string,
			((char *) pentry->field_pointer)));
		break;
	case BASE_SHORT:
		retval =(sscanf(strbuf, pentry->format_string,
			((short *) pentry->field_pointer)));
		break;
	case BASE_INT:
		retval =(sscanf(strbuf, pentry->format_string,
			((int *) pentry->field_pointer)));
		break;
	// if printed as 0x%08x, read as %i
	case BASE_HEX_INT:
		retval =(sscanf(strbuf, "%i", 
			((int *) pentry->field_pointer)));
		break;
	// for float and doubles, too hard to match x.y specs
	case BASE_FLOAT:
		retval =(sscanf(strbuf, "%f", 
			((float *) pentry->field_pointer)));
		break;
	case BASE_DOUBLE:
		retval =(sscanf(strbuf, "%lf", 
			((double *) pentry->field_pointer)));
		break;
	case BASE_STRING:
		retval =(sscanf(strbuf, pentry->format_string,
			(char *) pentry->field_pointer));
		break;
	case BASE_TIMESTAMP:
		pts = ((timestamp_t *) pentry->field_pointer); 
		memset(pts, 0, sizeof(timestamp_t));
		if (strncmp("HH:MM:SS.SSS ", pentry->format_string, 13) == 0) {
			// fill in timestamp_t from hr:min:ss.sss
			retval = str2timestamp(strbuf, pts);
		} else {
			// fill in timestamp_t from double secs since midnight
			
			retval = sscanf(strbuf, "%lf", &secs_since_midnight); 
			if (retval == 1) {	// successful scanf
				millisecs = 1000*secs_since_midnight;
				ms_to_ts(millisecs, pts);
			}	
		}
		break;
	default:
		fprintf(stderr, "unrecognized BASE %d\n", pentry->base_type);
		break;
	}
	return retval;
}
		 
/**
 *	Copies space-separated field into tmpbuf, null-terminating.
 *	Returns number of characters traversed in line buffer. 
 *	Returns -1 if max_length is exceeded without finding ' ' or \0
 *
 *	Leading spaces are trimmed from tmpbuf.
 */ 
int get_data_log_next_field(char *linebuf, char *tmpbuf, int max_length) 
{
	int count = 0;
	int tmp_count = 0;

	// First skip over spaces to first non-space
	while (linebuf[count] == ' ') {
		count++;
		if (count > max_length)
			return(-1);
	}
	while (linebuf[count] != ' ') {
		tmpbuf[tmp_count] = linebuf[count];
		if (linebuf[count] == '\0')	
			return count;
		count++;
		tmp_count++;
		if (count > max_length)
			return(-1);
	}		
	tmpbuf[tmp_count] = '\0';
	return count;	// characters traversed, not necessarly string length
}

/**
 *	This function takes a buffer representing a line of fields
 *	from a data log and places it back in the appropriate variables
 *	according to the specified table. Useful for replay and
 *	data analysis.
 */

int get_data_log_line(char *linebuf, data_log_column_spec_t *ptable, int size)
{
	char tmpbuf[MAX_LINE_LEN];	// individual fields in logs <132 chars
	int i;	// entry in specification table
	int cur = 0;		// current character in linebuf 
	for (i = 0; i < size; i++) {
		int cc;		// number of characters put in tmpbuf
		data_log_column_spec_t *pentry = &ptable[i];
		cc = get_data_log_next_field(&linebuf[cur], 
				&tmpbuf[0], MAX_LINE_LEN); 
		if (cc <= 0) {
			printf("Data log read error: entry %d %s\n", i,
				pentry->format_string);
			return i;
		}
		if (sscan_data_log_column_entry(&tmpbuf[0], pentry) != 1) {
			printf("Data log conversion error: entry %d %s\n", i,
				pentry->format_string);
			return i;
		}
		cur += cc;
	}	
	return i;
}		

/** This function is identical to open_data_log except that it has
 *	an additional 'infix' string between MMDD and SSS.
 *	open_data_log not rewritten for backward compatibility.
 *	'monthday' and 'serialnum' are returned separately as
 *	strings to allow the opening of other files with the 'infix' between.
 */
int open_data_log_infix(FILE ** pf_data, /// pass in file pointer to be set
		 char* prefix, 	   /// e.g., "a" or "e"
		 char* suffix,	   /// e.g., ".dat"
		 double *pstart_time,	/// will be set by open_data_log
		 int *pold_fileday,     /// initialize bad day value, e.g. 99
		 int *pcounter, 	/// will be set to serial number 
		 char *monthday,	/// if not NULL, "MMDD" will fill this
		 char *serialnum,       /// if not NULL, "SSS"  will fill this 
		 char *infix	// inserted betwee MMDD and SSS
		)
{
	int old_fileday = *pold_fileday; /// set illegal value on first call
	int file_serialno = *pcounter;	/// will be set or incremented
	int status;
	struct timespec now;
	struct timeb timeptr_raw;
	struct tm time_converted;
	char filename[80];
	char temp1[80];
	char temp2[80];
	bool_typ file_exists = TRUE;

	/* Get date information. */
	ftime (&timeptr_raw);
	//_localtime (&timeptr_raw.time, &time_converted);
	localtime_r (&timeptr_raw.time, &time_converted);

	/* If old_fileday is not the same as time_converted.tm_mday, reset
	 * the serial number to zero.  This will happen first time through, or
	 * if we run through midnight. */
	if (old_fileday != time_converted.tm_mday)
		file_serialno = 0;
	old_fileday = time_converted.tm_mday;

	while(file_exists == TRUE) {
		snprintf(temp1, 80, "%2.2d%2.2d",
			time_converted.tm_mon+1, time_converted.tm_mday);
		snprintf(temp2, 80, "%3.3d", file_serialno);
		snprintf(filename, 80, "%s%s%s%s%s", 
				prefix, temp1, infix, temp2, suffix);

		*pf_data = fopen(filename, "r");
		if (*pf_data == NULL)
			file_exists = FALSE;
		else {
			fclose(*pf_data);
			file_serialno++;
		}
	}

	/* We've found a valid serial number (and filename for radar file is
	 * still stored in the variable filename).  Now open all the necessary
	 * files. */
	*pf_data = fopen(filename, "w");
	if (*pf_data == NULL) {
		perror("open_data_log_infix");
		return 0;
	}

	/* Set start_time on successful open */

	status = clock_gettime( CLOCK_REALTIME, &now );
	*pstart_time = now.tv_sec + ((double) now.tv_nsec/ 1000000000L);
	file_serialno++;

	*pcounter = file_serialno;	

	if (monthday != NULL)
		strncpy(monthday, temp1, 80);
	if (serialnum != NULL)
		strncpy(serialnum, temp2, 80);

	return 1;
}
    
/** Determine the time since files were opened. 
 *  If the current time is more than the allotted time for file
 *  collection, close the current set of files and open a new file.
 *  pstart_time should originally be set by the first call to open_data_log. 
 *  The return value can be used to open a set of files with open
 *  and close synchronized to the first.
 */

int reopen_data_log_infix(FILE ** pf_data, int file_time, 
			char *prefix, char *suffix,
			double *pstart_time, 
			int *pold_fileday, int *pcounter,
			char *monthday, char* serialnum, char *infix,
				 buff_typ *pbuf)
{
	struct timespec now;
	double curr_time;
	int status;
	status = clock_gettime( CLOCK_REALTIME, &now );
	curr_time = now.tv_sec +
                        ((double) now.tv_nsec/1000000000L) - *pstart_time;

	if (curr_time >= file_time * 60.0) {
		if (pbuf != NULL)
			buff_done(pbuf);
                fclose(*pf_data);
		open_data_log_infix(pf_data, prefix, suffix, pstart_time,
			pold_fileday, pcounter, monthday, serialnum, infix);
		return 1;
	}
	return 0;
}

/** This function is identical to open_data_log except that it has
 *	an additional 'infix' string between MMDD and SSS.
 *	open_data_log not rewritten for backward compatibility.
 *	'monthday' and 'serialnum' are returned separately as
 *	strings to allow the opening of other files with the 'infix' between.
 */
int open_data_log_infix_force_fileno(FILE ** pf_data, /// pass in file pointer to be set
		 char* prefix, 	   /// e.g., "a" or "e"
		 char* suffix,	   /// e.g., ".dat"
		 double *pstart_time,	/// will be set by open_data_log
		 int *pold_fileday,     /// initialize bad day value, e.g. 99
		 int *pcounter, 	/// will be set to serial number 
		 char *monthday,	/// if not NULL, "MMDD" will fill this
		 char *serialnum,       /// if not NULL, "SSS"  will fill this 
		 char *infix	// inserted betwee MMDD and SSS
		)
{
	int old_fileday = *pold_fileday; /// set illegal value on first call
	int file_serialno = *pcounter;	/// will be set or incremented
	int status;
	struct timespec now;
	struct timeb timeptr_raw;
	struct tm time_converted;
	char filename[80];
	char temp1[80];
	char temp2[80];
	bool_typ file_exists = TRUE;

	/* Get date information. */
	ftime (&timeptr_raw);
	//_localtime (&timeptr_raw.time, &time_converted);
	localtime_r (&timeptr_raw.time, &time_converted);

	/* If old_fileday is not the same as time_converted.tm_mday, reset
	 * the serial number to zero.  This will happen first time through, or
	 * if we run through midnight. */
	if (old_fileday != time_converted.tm_mday)
		file_serialno = 0;
	old_fileday = time_converted.tm_mday;

	snprintf(temp1, 80, "%2.2d%2.2d",
		time_converted.tm_mon+1, time_converted.tm_mday);
	snprintf(temp2, 80, "%3.3d", file_serialno);
	snprintf(filename, 80, "%s%s%s%s%s", 
			prefix, temp1, infix, temp2, suffix);

	*pf_data = fopen(filename, "a");
	if (*pf_data == NULL) {
		perror("open_data_log_infix");
		return 0;
	}

	/* Set start_time on successful open */

	status = clock_gettime( CLOCK_REALTIME, &now );
	*pstart_time = now.tv_sec + ((double) now.tv_nsec/ 1000000000L);

	*pcounter = file_serialno;	

	if (monthday != NULL)
		strncpy(monthday, temp1, 80);
	if (serialnum != NULL)
		strncpy(serialnum, temp2, 80);

	return 1;
}

void save_to_spec (FILE *fout, timestamp_t timestamp,
                        int use_memory, buff_typ *pbuff,
                        int num_columns, data_log_column_spec_t *spec)
{
        int i;
        int cnt = 0;
        char char_buf[MAX_LOG_LINE_LEN];
        for (i = 0; i < num_columns; i++)
                cnt += (sprint_data_log_column_entry(
                                char_buf + cnt, &spec[i]));
        cnt += (sprintf(char_buf + cnt, "\n"));
        if (use_memory)
                buff_add(pbuff, char_buf, cnt);
        else
                fprintf(fout, "%s", char_buf);
}

