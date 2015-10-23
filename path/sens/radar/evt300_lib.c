/**\file 
 *
 *	evt300 routines for path_serial_lib
 *
 *  	Copyright (c) 1999-2007   Regents of the University of California
 *
 *	In this directory they are also linked directly into
 *	the evt300 binary, which reads only from one radar.
 *
 *	They may be used by other programs that read a
 *	radar and take some action, or by programs that
 *	read sensor data from more than one serial port.
 *	In the latter application a multithreaded program
 *	should be used, because the ser_driver_read routine
 *	blocks if no data is coming in from a radar. 
 */
#include <sys_os.h>
#include <local.h>
#include "sys_list.h"
#include "sys_buff.h"
#include "data_log.h"
#include <timestamp.h>
#include "evt300.h"

/** Converts message buffer to structure for data server or trace
 *  Should be called only after pbuf has been verified to contain
 *  a correct EVT300 radar target meesage.
 */
void evt300_message_buff_to_radar(evt300_mess_typ *pmsg,  
		evt300_radar_typ *pevt)
{
	get_current_timestamp(&pevt->ts);
	pevt->mess_ID = pmsg->msgID;
	pevt->target_count = pmsg->targ_count;

	pevt->target[0].id = pmsg->target_1_id;
	pevt->target[0].range = pmsg->target_1_range;
	pevt->target[0].rate = pmsg->target_1_rate;
	pevt->target[0].azimuth = pmsg->target_1_azimuth;
	pevt->target[0].mag = pmsg->target_1_mag;
	pevt->target[0].lock = pmsg->target_1_lock;
	pevt->target[1].id = pmsg->target_2_id;
	pevt->target[1].range = pmsg->target_2_range;
	pevt->target[1].rate = pmsg->target_2_rate;
	pevt->target[1].azimuth = pmsg->target_2_azimuth;
	pevt->target[1].mag = pmsg->target_2_mag;
	pevt->target[1].lock = pmsg->target_2_lock;
	pevt->target[2].id = pmsg->target_3_id;
	pevt->target[2].range = pmsg->target_3_range;
	pevt->target[2].rate = pmsg->target_3_rate;
	pevt->target[2].azimuth = pmsg->target_3_azimuth;
	pevt->target[2].mag = pmsg->target_3_mag;
	pevt->target[2].lock = pmsg->target_3_lock;
	pevt->target[3].id = pmsg->target_4_id;
	pevt->target[3].range = pmsg->target_4_range;
	pevt->target[3].rate = pmsg->target_4_rate;
	pevt->target[3].azimuth = pmsg->target_4_azimuth;
	pevt->target[3].mag = pmsg->target_4_mag;
	pevt->target[3].lock = pmsg->target_4_lock;
	pevt->target[4].id = pmsg->target_5_id;
	pevt->target[4].range = pmsg->target_5_range;
	pevt->target[4].rate = pmsg->target_5_rate;
	pevt->target[4].azimuth = pmsg->target_5_azimuth;
	pevt->target[4].mag = pmsg->target_5_mag;
	pevt->target[4].lock = pmsg->target_5_lock;
	pevt->target[5].id = pmsg->target_6_id;
	pevt->target[5].range = pmsg->target_6_range;
	pevt->target[5].rate = pmsg->target_6_rate;
	pevt->target[5].azimuth = pmsg->target_6_azimuth;
	pevt->target[5].mag = pmsg->target_6_mag;
	pevt->target[5].lock = pmsg->target_6_lock;
	pevt->target[6].id = pmsg->target_7_id;
	pevt->target[6].range = pmsg->target_7_range;
	pevt->target[6].rate = pmsg->target_7_rate;
	pevt->target[6].azimuth = pmsg->target_7_azimuth;
	pevt->target[6].mag = pmsg->target_7_mag;
	pevt->target[6].lock = pmsg->target_7_lock;
	return;
}

static char evt300_checksum(char *pdata)
{
	char sum = 0;
	int i;
	char zero =0;	// in order to have 8 bit value 

	for(i=0; i< sizeof(evt300_mess_typ) - 1; i++)
		sum += pdata[i];
	sum = zero - sum;

	return (sum);
}
/** 
 *	evt300_ser_driver_read returns TRUE if a message of type 82 or 89
 *	with a correct number of targets and a correct checksum
 *	has been found, and fills in the evt300_radar_typ structure.
 *	Returns FALSE if a message is found with bad number of targets
 *	or a checksum error.
 *	FAILS TO RETURN if input stream blocks or never finds a
 *	82 or 89	
 */
bool_typ evt300_ser_driver_read(int fpin, evt300_radar_typ *pevt,
		evt300_radar_info_t *pinfo) 
{
	evt300_mess_union_typ msgbuf;
	char *pdata = (char *) &msgbuf.gen_mess.data[0]; 
			/// signed for checksum calc
	int i, ii, index;
	char csum;
	unsigned char dummy;

	/* Read from serial port. */
	/* Blocking read is used, so control doesn't return unless data is
	 * available.  Keep reading until the beginning of a FE target
	 * message (type 82 or 89) is detected. */

	memset(&pdata[0], 0x0, sizeof(evt300_mess_typ));
	memset(pevt, 0x0, sizeof(evt300_radar_typ));

	/* 
	 * Since (hopefully) once we're synchronized with the radar we'll
	 * stay that way the byte we just read should have been the first
	 * byte of a new message.  If it was a 34 or 36, read enough bytes
	 * for that type of message.  Otherwise print out message type 
	 * in verbose mode (to figure out what's happening).
	 */
	while (pdata[0] != FE_TARGET_REPORT_MESSAGE &&
		pdata[0] != FE_TARGET_REPORT_2_MESSAGE) {
		evt300_msg_info_t *pm;	/// use to get message type info  
		unsigned char msg_id;	/// unsigned for array indexing

		read (fpin, &pdata[0], 1);
		msg_id = pdata[0];
		pm = &evt300_msg_info[msg_id]; 

		switch (msg_id) {
		case FE_TARGET_REPORT_MESSAGE:
	        case FE_TARGET_REPORT_2_MESSAGE:
			if (pinfo->verbose)
				printf("%s %d\n", pm->msg_name, msg_id);
			break;
	        case FE_CRITICAL_TARGET_MESSAGE:
		case FE_VEHICLE_DATA_MESSAGE:
		case DDU_DATA_REQUEST_MESSAGE:
		/* Expected other message types: skip over*/
			for (i = 0; i <pm->num_bytes; i++) /// read data
				read (fpin, &dummy, 1);
			read (fpin, &dummy, 1);	/// read checksum
			if (pinfo->verbose)
				printf("%s %d\n", pm->msg_name, msg_id);
			break;
		default:
			if (pinfo->verbose) {
				printf("%s %d\n", pm->msg_name, msg_id);
			}
			break;
	        }
	}

	/* FE target message.  Now read next two bytes which are FFT
 	 * frame number and number of targets (0-7). */
	read (fpin, &msgbuf.evt300_mess.FFTframe, 1);
	read (fpin, &msgbuf.evt300_mess.targ_count, 1);

	/** If target count is not 0-7 this is not really a target message.
	 * Take an error exit. */
	if (msgbuf.evt300_mess.targ_count > EVT300_MAX_TARGETS) {
		if (pinfo->verbose)
			printf("frame %d target count %d\n",
				 msgbuf.evt300_mess.FFTframe,
				 msgbuf.evt300_mess.targ_count);
		pinfo->error_count++;
		return (FALSE);
	}

	/** Now read data for as many targets as exist. */
	if (msgbuf.evt300_mess.targ_count > 0) {
		index = 3;	// target data begins in fourth byte
	        for (i=0; i<msgbuf.evt300_mess.targ_count; i++) {
			/* Read 8 bytes per target. */
			for (ii=0; ii<8; ii++) {
				read (fpin, &pdata[index], 1);
				index++;
			}
		}
	}

	/* Now read the checksum.  Always save it in last byte regardless
	 * of message type or how many targets are in a FE target message.
	 * This will work just fine, even though the message is variable sized,
	 * since the entire buffer was zeroed before reading any data. */
	read (fpin, &pdata[sizeof(evt300_mess_typ) - 1], 1);

	csum = evt300_checksum(pdata);
	if(csum != pdata[sizeof(evt300_mess_typ) - 1]) {
		if (pinfo->verbose) {
			timestamp_t stamp;
			int j;
			get_current_timestamp(&stamp);
			print_timestamp(stdout, &stamp);
			printf("checksum calc %d read %d, radar %s\n",
				 csum, pdata[sizeof(evt300_mess_typ)-1],
			  	 pinfo->id);
			for (j = 0; j < sizeof(evt300_mess_typ); j++) {
				if (j % 16 == 0) printf("\n");
				printf(" %02x", pdata[j]);
			}
			printf("\n");
			fflush(stdout);
		}
		pinfo->error_count++;
		return (FALSE);
	} else {
		evt300_message_buff_to_radar(&msgbuf.evt300_mess, pevt);
		return (TRUE);
	}
}

/** Currently prints a line in the same format that has been used by
 * the project's "wrfiles" routines, with two additional columns
 */
void evt300_print_radar(FILE *f_radar, evt300_radar_typ *pradar, 
			timestamp_t write_time, evt300_radar_info_t *pinfo)
{
	int i;

	print_timestamp(f_radar, &write_time);
	fprintf(f_radar, " ");
	print_timestamp(f_radar, &pradar->ts);
	fprintf(f_radar, " ");

	for (i = 0; i < EVT300_MAX_TARGETS; i++)  
		fprintf(f_radar, " %hhu %hd %hd %hhd %hhu %hhu",
			pradar->target[i].id,
			pradar->target[i].range,
			pradar->target[i].rate,
	       		pradar->target[i].azimuth, 
			pradar->target[i].mag,
			pradar->target[i].lock);
	// Additional columns: target count, error count so far
	fprintf(f_radar, " %d", pradar->mess_ID);	
	fprintf(f_radar, " %d", pradar->target_count);
	if (pinfo != NULL)
		fprintf(f_radar, " %d\n", pinfo->error_count);
	return;
}

/** Takes a line in a string buffer in the same format that has been 
 * used by "wrfiles" routines, placing the data back in an 
 * evt300_radar_typ variable.
 */
int evt300_sscan_radar(char *strbuf, evt300_radar_typ *pradar, 
			timestamp_t *pwrite_time, evt300_radar_info_t *pinfo)
{
        char tmpbuf[MAX_LINE_LEN];
	int i;
        int cur = 0;    // current position in input string
        int cc = 0;     // character count for field
        int retval = 0; // from each scanf, should always be 1

	cc = get_data_log_next_field(&strbuf[cur], tmpbuf, MAX_LINE_LEN);
        retval += str2timestamp(tmpbuf, pwrite_time);
        cur += cc;
	cc = get_data_log_next_field(&strbuf[cur], tmpbuf, MAX_LINE_LEN);
        retval += str2timestamp(tmpbuf, &pradar->ts);
        cur += cc;

	for (i = 0; i < EVT300_MAX_TARGETS; i++) {  
		evt300_target_t *ptarget = &pradar->target[i];
		cc = get_data_log_next_field(&strbuf[cur],tmpbuf,MAX_LINE_LEN);
		retval += (sscanf(tmpbuf, "%hhu", &ptarget->id)); 
		cur += cc;
		cc = get_data_log_next_field(&strbuf[cur],tmpbuf,MAX_LINE_LEN);
		retval += (sscanf(tmpbuf, "%hd", &ptarget->range)); 
		cur += cc;
		cc = get_data_log_next_field(&strbuf[cur],tmpbuf,MAX_LINE_LEN);
		retval += (sscanf(tmpbuf, "%hd", &ptarget->rate)); 
		cur += cc;
		cc = get_data_log_next_field(&strbuf[cur],tmpbuf,MAX_LINE_LEN);
		retval += (sscanf(tmpbuf, "%hhd", &ptarget->azimuth)); 
		cur += cc;
		cc = get_data_log_next_field(&strbuf[cur],tmpbuf,MAX_LINE_LEN);
		retval += (sscanf(tmpbuf, "%hhu", &ptarget->mag)); 
		cur += cc;
		cc = get_data_log_next_field(&strbuf[cur],tmpbuf,MAX_LINE_LEN);
		retval += (sscanf(tmpbuf, "%hhu", &ptarget->lock)); 
		cur += cc;
	}
	// Additional columns: target count, error count so far
	cc = get_data_log_next_field(&strbuf[cur],tmpbuf,MAX_LINE_LEN);
	retval += (sscanf(tmpbuf, "%hhu", &pradar->mess_ID)); 
	cur += cc;
	cc = get_data_log_next_field(&strbuf[cur],tmpbuf,MAX_LINE_LEN);
	retval += (sscanf(tmpbuf, "%hhd", &pradar->target_count)); 
	cur += cc;
	// last field is optional
	cc = get_data_log_next_field(&strbuf[cur],tmpbuf,MAX_LINE_LEN);
	if (cc > 0)
		retval += (sscanf(tmpbuf, "%d", &pinfo->error_count)); 
	return retval;
}


/** Puts the line in a string that can be saved in memory and
 *  printed later using buff_init, buff_add and buff_done from
 *  local/sys_buff.c. Returns number of characters written.
 */
int evt300_sprint_radar(char *evt300_buffer, evt300_radar_typ *pradar, 
			timestamp_t write_time, evt300_radar_info_t *pinfo)
{
	int i;
	int cc = 0;		// character count

	cc += (sprint_timestamp(evt300_buffer+cc, &write_time));
	cc += (sprintf(evt300_buffer+cc, " "));
	cc += (sprint_timestamp(evt300_buffer + cc, &pradar->ts));
	cc += (sprintf(evt300_buffer + cc, " "));

	for (i = 0; i < EVT300_MAX_TARGETS; i++)  
		cc += (sprintf(evt300_buffer + cc, " %d %d %d %d %d %d",
			pradar->target[i].id,
			pradar->target[i].range,
			pradar->target[i].rate,
	       		pradar->target[i].azimuth, 
			pradar->target[i].mag,
			pradar->target[i].lock));
	// Additional columns: target count, error count so far
	cc += (sprintf(evt300_buffer + cc, " %d", pradar->mess_ID));	
	cc += (sprintf(evt300_buffer + cc, " %d", pradar->target_count));
	if (pinfo != NULL)
		cc += (sprintf(evt300_buffer + cc, " %d\n", 
			pinfo->error_count));
	return cc;
}

/** stub for Jeff to fill in
 */
void evt300_mysql_save_radar(evt300_radar_typ *pradar,
	 evt300_radar_info_t *pinfo)
{
}
