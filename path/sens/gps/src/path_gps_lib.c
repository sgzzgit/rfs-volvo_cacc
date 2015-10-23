/**\file
 *
 *	Shared functions for code that reads, writes or
 *	processes GPS data
 *
 *	Library and header files named "path_gps" to avoid
 *	confusion with other GPS packages.	
 *
 *	These functions were assembled from GPS code originally
 *	written by Paul Kretz, John Spring, Jeff Ko and student
 *	Bernard Liang by Sue Dickey, April 2008.
 */

#include <sys_os.h>
#include <math.h>
#include <local.h>
#include <timestamp.h>
#include <path_gps_lib.h>
#include <enu2lla.h>

// Turn on only for detailed debugging of checksum
#undef DO_TRACE
#undef TRACE_CHECKSUM
#undef TRACE_QUERY

int hexvalue(char c)
{
	switch (c) {
	case '0': return 0;
	case '1': return 1;
	case '2': return 2;
	case '3': return 3;
	case '4': return 4;
	case '5': return 5;
	case '6': return 6;
	case '7': return 7;
	case '8': return 8;
	case '9': return 9;
	case 'A':
	case 'a': return 10;
		  break;
	case 'B':
	case 'b': return 11;
		  break;
	case 'C':
	case 'c': return 12;
		  break;
	case 'D':
	case 'd': return 13;
		  break;
	case 'E':
	case 'e': return 14;
		  break;
	case 'F':
	case 'f': return 15;
		  break;
	default:
		break;
	}
	return 0;
}
int twohexdigit(char high, char low)
{
#ifdef TRACE_CHECKSUM
	printf("high %c %d low %c %d\n", high, hexvalue(high),
		 low, hexvalue(low));
#endif
	return (hexvalue(high)*16  + hexvalue(low));
}

/**
 *	This function expects to receive a legal NMEA 0183 sentence,
 *	beginning with a $ and with a * before the checksum. It computes
 *	the exclusive or of the characters between $ and *, and 0x03,
 *	and compares the result to the hexadecimal number in the two
 *	characters after the *. The funtion returns FALSE in case of
 *	bad formatting or bad checksum, and TRUE if the checksum is good.
 *
 */
int path_gps_good_checksum(char *buf) {

	int i = 1;		    /// begin at character after initial $
	unsigned char sum = 0x00;   /// initialize checksum 
	unsigned char sentence_sum;
	int retval = 0;

#ifdef DO_TRACE
	printf("entering path_gps_good_checksum\n");
	fflush(stdout);
#endif
	if (buf == NULL) return 0;
#ifdef DO_TRACE
	printf("0x%x %s\n", buf, buf); 
	fflush(stdout);
#endif
	if (buf[0] != '$') return 0;

	while ((buf[i] != '*') && (buf[i] != '\0') &&
			 i < MAX_GPS_SENTENCE-2){ 
		sum ^=  buf[i];
		i++;
	}
	if (buf[i] != '*')
		return 0;
	sentence_sum = twohexdigit(buf[i+1], buf[i+2]);

#ifdef TRACE_CHECKSUM
	printf("checksum checking: ");
	printf("%s 0x%08x", buf, buf);	
	printf("sum 0x%02x ", sum); 
	printf("sentence_sum 0x%02x\n", sentence_sum);	
#endif
	retval = (sum == sentence_sum);
#ifdef DO_TRACE
	printf("s 0x%x retval %d\n",  buf, retval); 
	fflush(stdout);
#endif
	return (retval);
}

/**
 *	Returns file pointer for serial or USB, or
 *	TCP socket descriptor if using gpsd. If using gpsd,
 *	sends 'r+' to get raw GPS sentences.
 *	"name" parameter will be device name or IP address string
 *
 *	File pointer is used instead of integer file descriptor 
 *	because we want to use fgets, which limits the length of
 *	input, rather than gets when actually reading, to fail
 *	more gracefully when, e.g., reading the wrong serial port.
 */

void path_gps_get_fd(char *name, FILE **pfp, 
		int use_gpsd, unsigned short port, int *psd)
{
	char *send_str = "r+";			//
	FILE *fptr; 
	int sd;
	struct sockaddr_in serv_addr;

	if (use_gpsd) { 
		if ((sd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
			printf("TCP socket create failed\n");
			exit(1);
		}
		memset(&serv_addr, 0, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = inet_addr(name);
		serv_addr.sin_port = htons(port);
		if (connect(sd, (struct sockaddr *) &serv_addr, 
					sizeof(serv_addr)) < 0){
			printf("TCP socket connect failed\n");
			perror(NULL);
			exit(1);
		}

		if (send(sd, send_str, strlen(send_str), 0) <= 0) {
			perror(send_str);
			exit(1);
		} 
	} else {
		/* Initialize serial or USB port. */  
		fptr = fopen(name, "r");
		if (fptr <= 0) {
			printf("Error opening %s for input\n", name);
			exit(1);
		}
	}
	*pfp = fptr;
	*psd = sd; 
}

/**
 *	This code makes the assumption that with gpsd in raw mode,
 *	getting more than one GPS sentence or a partial GPS sentence
 *	in a single call to recv is rare enough we can just drop sentences
 *	and go on.
 *
 *	When reading directly from a serial port, errors are likewise ignored.
 *
 *	This function is blocking, it is assumed if we don't get GPS
 *	data here there is nothing we can do about it!
 */

void path_gps_get_sentence(char *sentence, FILE *fp, int use_gpsd, int sd)
{
	int rcvd = 0;
	static int fgets_error = 0;
	char buf[MAX_GPS_SENTENCE];

#ifdef DO_TRACE
	printf("entering gps_get_sentence, sentence 0x%08x\n", sentence);
	fflush(stdout);
#endif
	while (1) {
		if (use_gpsd) {
			memset(buf, 0, MAX_GPS_SENTENCE);

			if ((rcvd = recv(sd, buf, 
					MAX_GPS_SENTENCE, 0)) <= 0) {
                                printf("Error receiving from gpsd\n");
                                continue;
			}
                } else {
			if (!fgets(buf, MAX_GPS_SENTENCE, fp)) {
				perror("fgets failed");
				fgets_error++;
				sleep(1);

				/// try a few times before quitting
				if (fgets_error > 10){ 
					printf("exiting: multiple fgets fail\n");
					exit(EXIT_FAILURE);
				} else
					continue;
			}
		}
#ifdef DO_TRACE
	printf("before check: buf %s 0x%08x, s 0x%08x\n", buf, buf, sentence);
	fflush(stdout);
#endif
		if (path_gps_good_checksum(&buf[0])) {
#ifdef DO_TRACE
	printf("after check: buf 0x%08x, s 0x%08x\n", buf, sentence);
	fflush(stdout);
#endif
			strncpy(&sentence[0], &buf[0], MAX_GPS_SENTENCE);
#ifdef DO_TRACE
	printf("after strncpy: buf %s 0x%08x\n, s %s 0x%08x\n", buf, buf,
		 sentence, sentence);
	fflush(stdout);
#endif
			return;
		}
	}
}
	 
/**
 *	Copies characters from current location until before the next
 *  	comma (or *) from 'sentence' into 'field_buf', null-terminating
 *	the string.
 *
 *	Returns index in sentence of character after first comma found.
 *
 *	field_buf[0] will be 0 if no characters between commas.
 */

int path_gps_get_field(int start, char *sentence, char *field_buf, int max)
{
	int i = 0;
	while ((sentence[start+i] != ',') && 
			(sentence[start+i] != '*') && i < max) {
		field_buf[i] = sentence[start+i];
		i++;
	}
	field_buf[i] = '\0';
	return start+i+1;
}

/**
 *	See gps_gga_typ definition in path_gps_lib.h for description
 *	of data fields.
 */
void path_gps_parse_gga(char *sentence, gps_gga_typ *p)
{
	int index = 7;  
	char buf[MAX_GPS_FIELD];

	// default 0 for fields set in path_gps_parse_sentence
	
	index = path_gps_get_field(index, sentence, buf, MAX_GPS_FIELD);
	if (buf[0])
		sscanf(buf, "%f", &p->utc_time);

	index = path_gps_get_field(index, sentence, buf, MAX_GPS_FIELD);
	if (buf[0])
		sscanf(buf, "%lf", &p->latitude);

	index = path_gps_get_field(index, sentence, buf, MAX_GPS_FIELD);
	if (buf[0] == 'S')
		p->latitude = -p->latitude;

	index = path_gps_get_field(index, sentence, buf, MAX_GPS_FIELD);
	if (buf[0])
		sscanf(buf, "%lf", &p->longitude);

	index = path_gps_get_field(index, sentence, buf, MAX_GPS_FIELD);
	if (buf[0] == 'W')
		p->longitude = -p->longitude;

	index = path_gps_get_field(index, sentence, buf, MAX_GPS_FIELD);
	p->pos = buf[0] ? atoi(buf) : 0;

	index = path_gps_get_field(index, sentence, buf, MAX_GPS_FIELD);
	p->num_sats = buf[0] ? atoi(buf) : 0;

	index = path_gps_get_field(index, sentence, buf, MAX_GPS_FIELD);
	if (buf[0])
		sscanf(buf, "%f", &p->hdop);

	index = path_gps_get_field(index, sentence, buf, MAX_GPS_FIELD);
	if (buf[0])
		sscanf(buf, "%f", &p->altitude);

	index = path_gps_get_field(index, sentence, buf, MAX_GPS_FIELD);
	if (buf[0] && (buf[0] != 'M'))
		printf("Unusual unit %c for altitude\n", buf[0]);

	index = path_gps_get_field(index, sentence, buf, MAX_GPS_FIELD);
	if (buf[0])
		sscanf(buf, "%f", &p->alt_offset);

	index = path_gps_get_field(index, sentence, buf, MAX_GPS_FIELD);
	if (buf[0] && (buf[0] != 'M'))
		printf("Unusual unit %c for alt_offset\n", buf[0]);

	index = path_gps_get_field(index, sentence, buf, MAX_GPS_FIELD);
	if (buf[0])
		sscanf(buf, "%f", &p->time_since_dgps);

	index = path_gps_get_field(index, sentence, buf, MAX_GPS_FIELD);
	p->dgps_station_id = buf[0] ? atoi(buf) : 0;
}

/**
 *	See gps_vtg_typ definition in path_gps_lib.h for description
 *	of data fields.
 */

void path_gps_parse_vtg(char *sentence, gps_vtg_typ *p)
{
	int index = 7;  
	char buf[MAX_GPS_FIELD];

	memset(p, 0, sizeof(gps_vtg_typ));	/// default 0

	index = path_gps_get_field(index, sentence, buf, MAX_GPS_FIELD);
	if (buf[0])
		sscanf(buf, "%f", &p->true_track);

	/// read the 'T'
	index = path_gps_get_field(index, sentence, buf, MAX_GPS_FIELD);
	if (buf[0] && (buf[0] != 'T'))
		printf("Got %c instead of T for true track\n", buf[0]);

	index = path_gps_get_field(index, sentence, buf, MAX_GPS_FIELD);
	if (buf[0])
		sscanf(buf, "%f", &p->magnetic_track);
	
	/// read the 'M'
	index = path_gps_get_field(index, sentence, buf, MAX_GPS_FIELD);
	if (buf[0] && (buf[0] != 'M'))
		printf("Got %c instead of M for magnetic track\n", buf[0]);

	index = path_gps_get_field(index, sentence, buf, MAX_GPS_FIELD);
	if (buf[0])
		sscanf(buf, "%f", &p->speed_knots);

	/// read the 'N'
	index = path_gps_get_field(index, sentence, buf, MAX_GPS_FIELD);
	if (buf[0] && (buf[0] != 'N'))
		printf("Got %c instead of N for knots\n", buf[0]);

	index = path_gps_get_field(index, sentence, buf, MAX_GPS_FIELD);
	if (buf[0])
		sscanf(buf, "%f", &p->speed_kph);

	/// read the 'K'
	index = path_gps_get_field(index, sentence, buf, MAX_GPS_FIELD);
	if (buf[0] && (buf[0] != 'K'))
		printf("Got %c instead of K for kph\n", buf[0]);
}

/**
 *	See gps_rmc_typ definition in path_gps_lib.h for description
 *	of data fields.
 */

void path_gps_parse_rmc(char *sentence, gps_rmc_typ *p)
{
	int index = 7;  
	char buf[MAX_GPS_FIELD];

	memset(p, 0, sizeof(gps_rmc_typ));	/// default 0
	index = path_gps_get_field(index, sentence, buf, MAX_GPS_FIELD);
	if (buf[0]) {
		sscanf(buf, "%f", &p->utc_time);
	}
	
	index = path_gps_get_field(index, sentence, buf, MAX_GPS_FIELD);
	if (buf[0])
		sscanf(buf, "%c", &p->nav_warning);

	index = path_gps_get_field(index, sentence, buf, MAX_GPS_FIELD);
	if (buf[0])
		sscanf(buf, "%lf", &p->latitude);

	index = path_gps_get_field(index, sentence, buf, MAX_GPS_FIELD);
	if (buf[0] == 'S')
		p->latitude = -p->latitude;

	index = path_gps_get_field(index, sentence, buf, MAX_GPS_FIELD);
	if (buf[0])
		sscanf(buf, "%lf", &p->longitude);

	index = path_gps_get_field(index, sentence, buf, MAX_GPS_FIELD);
	if (buf[0] == 'W')
		p->longitude = -p->longitude;

	index = path_gps_get_field(index, sentence, buf, MAX_GPS_FIELD);
	if (buf[0])
		sscanf(buf, "%f", &p->speed_knots);

	index = path_gps_get_field(index, sentence, buf, MAX_GPS_FIELD);
	if (buf[0])
		sscanf(buf, "%f", &p->true_track);

	index = path_gps_get_field(index, sentence, buf, MAX_GPS_FIELD);
	if (buf[0])
		sscanf(buf, "%d", &p->date_of_fix);

	index = path_gps_get_field(index, sentence, buf, MAX_GPS_FIELD);
	if (buf[0])
		sscanf(buf, "%f", &p->magnetic_variation);
}

void path_gps_print_data(FILE *fp, gps_data_typ *pg)
{
	print_timestamp(fp, &pg->ts);
	fprintf(fp, " %s ", pg->gps_id);
	if (strcmp("GPGGA", pg->gps_id) == 0) {
		gps_gga_typ *p = &pg->data.gga;
		fprintf(fp, "%.3f %.7f %.7f %d %d %.2f %.2f %.2f %.3f %d ",
			p->utc_time,
			p->latitude,
			p->longitude,		
			p->pos,
			p->num_sats,
			p->hdop,
			p->altitude,
			p->alt_offset,
			p->time_since_dgps,
			p->dgps_station_id);
	} else if (strcmp("GPVTG", pg->gps_id) == 0) {
		gps_vtg_typ *p = &pg->data.vtg;
		fprintf(fp, "%f %f %f %f ",
			p->true_track,
			p->magnetic_track,
			p->speed_knots,		
			p->speed_kph);
	} else if (strcmp("GPRMC", pg->gps_id) == 0) {
		gps_rmc_typ *p = &pg->data.rmc;
		fprintf(fp, "%.3f %c %.7f %.7f %f %f %d %f ",
			p->utc_time,
			p->nav_warning,
			p->latitude,
			p->longitude,		
			p->speed_knots,
			p->true_track,
			p->date_of_fix,
			p->magnetic_variation);
	}
}

static char *ggatbl_str = "ggatbl (utc_time, utc_ms, latitude, longitude, pos,num_sats, hdop, altitude, alt_offset, time_since_dgps, dgps_station_id, date_time)"; 

static char *vtgtbl_str = "vtgtbl (true_track, magnetic_track, speed_knots, speed_kph, date_time)";

static char *rmctbl_str = "rmctbl (utc_time, utc_ms, nav_warning, latitude, longitude, speed_knots, true_track, date_of_fix, magnetic_variation, date_time)";

/** Creates string to insert in sqlite3 tables with fixed names and format
 *  per NEMA code
 *  Later tbl_str may be a parameter.
 */
void path_gps_data_insert_str(char *cmd_str, gps_data_typ *pg)
{
	char tmp_str[MAX_CMD_STR];
	char *tbl_str = NULL;
	if (strcmp("GPGGA", pg->gps_id) == 0) {
		gps_gga_typ *p = &pg->data.gga;
		timestamp_t utc_ts = gpsutc2ts(p->utc_time);
		snprintf(tmp_str, MAX_CMD_STR, 
	"(\"%02d:%02d:%02d\",%d,%.7f,%.7f,%d,%d,%.2f,%.2f,%.2f,%.3f,%d,%s)",
			utc_ts.hour,
			utc_ts.min,
			utc_ts.sec,
			utc_ts.millisec,
			p->latitude,
			p->longitude,		
			p->pos,
			p->num_sats,
			p->hdop,
			p->altitude,
			p->alt_offset,
			p->time_since_dgps,
			p->dgps_station_id,
			"datetime('now')");
		tbl_str = ggatbl_str;
	} else if (strcmp("GPVTG", pg->gps_id) == 0) {
		gps_vtg_typ *p = &pg->data.vtg;
		snprintf(tmp_str, MAX_CMD_STR, "(%.6f,%.6f,%.6f,%.6f,%s)",
			p->true_track,
			p->magnetic_track,
			p->speed_knots,		
			p->speed_kph,
			"datetime('now')");
		tbl_str = vtgtbl_str;
	} else if (strcmp("GPRMC", pg->gps_id) == 0) {
		gps_rmc_typ *p = &pg->data.rmc;
		timestamp_t utc_ts = gpsutc2ts(p->utc_time);
		snprintf(tmp_str, MAX_CMD_STR, 
	"(\"%02d:%02d:%02d\",%d,\"%c\",%.7f,%.7f,%.6f,%.6f,%.6d,%.6f,%s)",
			utc_ts.hour,
			utc_ts.min,
			utc_ts.sec,
			utc_ts.millisec,
			p->nav_warning,
			p->latitude,
			p->longitude,		
			p->speed_knots,
			p->true_track,
			p->date_of_fix,
			p->magnetic_variation,
			"datetime('now')");
		tbl_str = rmctbl_str;
	}
	snprintf(cmd_str, MAX_CMD_STR, "insert into %s values %s",
		 tbl_str, tmp_str);
}

/**
 *	This function will operate on a sentence that has been
 *	checked by path_gps_good_checksum and thus begins with $,
 *	continue to *, and should have a five character identifier
 *	starting with the second character.
 */
void path_gps_parse_sentence(char *sentence, gps_data_typ *pg)
{
	int index = 1;	///beginning of ID field

	memset(pg, 0, sizeof(gps_data_typ));
	index = path_gps_get_field(index, sentence, pg->gps_id, 5); 

	get_current_timestamp(&pg->ts);

	if (strcmp("GPGGA", pg->gps_id) == 0)
		path_gps_parse_gga(sentence, &pg->data.gga);
	else if (strcmp("GPVTG", pg->gps_id) == 0)
		path_gps_parse_vtg(sentence, &pg->data.vtg);
	else if (strcmp("GPRMC", pg->gps_id) == 0)
		path_gps_parse_rmc(sentence, &pg->data.rmc);
}

/**
 *	This function converts an array of gps_data_typ, each
 *	element of which summarizes an NMEA sentence,
 *	into a path_gps_point_t.
 *	
 *	Currently GGA is used for lat/long, if present, otherwise
 *	RMC, VTG for speed, local_time and utc_time are from the
 *	latest message with those values. (Except that if some times
 *	are before and some are after midnight, the ones before
 *	midnight will be chosen; since the differences will normally
 *	be less than a second, this should not cause a serious problem.)
 */ 
int path_gps_get_point(gps_data_typ *gps_array, int array_size,
			path_gps_point_t *pp)
{
	gps_gga_typ gga;
	gps_vtg_typ vtg;
	gps_rmc_typ rmc;
	timestamp_t utc_time;
	timestamp_t local_time;
	int found_gga = 0;
	int found_vtg = 0;
	int found_rmc = 0;
	int i;

	/// iniitalize hour min sec timestamps to minimum value
	utc_time = local_time = gpsutc2ts(0.0);

	/// initialize temporary variables to 0
	memset(&gga, 0, sizeof(gps_gga_typ));
	memset(&vtg, 0, sizeof(gps_vtg_typ));
	memset(&rmc, 0, sizeof(gps_rmc_typ));
	for (i = 0; i < array_size; i++) {
		gps_data_typ *pd = &gps_array[i];
		if (ts2_is_later_than_ts1(&local_time, &pd->ts)) 
			local_time = pd->ts;	
		if (strcmp("GPGGA", pd->gps_id) == 0){ 
			gga = pd->data.gga;	
			found_gga = 1;
		} else if (strcmp("GPVTG", pd->gps_id) == 0){ 
			vtg = pd->data.vtg;	
			found_vtg = 1;
		} else if (strcmp("GPRMC", pd->gps_id) == 0){ 
			rmc = pd->data.rmc;	
			found_rmc = 1;
		}
	}
	/// make sure we have a source of lat/long
	/// speed and heading are optional
	if (!(found_gga || found_rmc))
		return 0;

	/// Initialize path_gps_point_t structure to 0
	memset(pp, 0, sizeof(path_gps_point_t));

	pp->local_time = local_time;
	if (found_gga) {	/// use GGA in preference to RMC
		pp->utc_time = gpsutc2ts(gga.utc_time);
		pp->latitude = path_gps2degree(gga.latitude);
		pp->longitude = path_gps2degree(gga.longitude);
		pp->pos = gga.pos;
		pp->num_sats = gga.num_sats;
		pp->hdop = gga.hdop;
		pp->altitude = gga.altitude;  /* only from GGA */
	} else {
		pp->utc_time = gpsutc2ts(rmc.utc_time);
		pp->latitude = path_gps2degree(rmc.latitude);
		pp->longitude = path_gps2degree(rmc.longitude);
	}
	if (found_vtg) {	/// use VTG in preference to RMC
		pp->speed = vtg.speed_knots * KNOTS_TO_METERS_PER_SEC;
		pp->heading = vtg.true_track;
	} else if (found_rmc) {
		pp->speed = rmc.speed_knots * KNOTS_TO_METERS_PER_SEC;
		pp->heading = rmc.true_track;
	}
        if (found_rmc) {  /* date only in RMC message */
	        pp->date = rmc.date_of_fix;
	}
	return 1;
}

/*
 *	For now this does IPv4 integer IDs, later we will need IPv6
 */

void path_gps_update_object_id(void *id, path_gps_point_t *pt, int size)
{
	int i;
	unsigned char *p = (unsigned char *) id;
	memset(&pt->object_id, 0, GPS_OBJECT_ID_SIZE);
	for (i = 0; i < size && i < GPS_OBJECT_ID_SIZE; i++)
		pt->object_id[i] = p[i];
}

/*
 *	For now just return 0, later actually check and set the
 *	mask.
 *	
 *	Later will need to create structure internal to message
 *	receive process that associates IP addresses and DB vars.
 */
int path_gps_update_pt_ref(path_gps_pt_ref_t *pt_ref, path_gps_point_t *pt)
{
	pt_ref->mask[0] &= 1;	// set bit 0
	return 0;
}

void path_gps_sprint_point(char* buf, path_gps_point_t *pp)
{
	sprintf(buf, "%.7f %.7f %.3f %.3f %d %2d %3f %.2f %3d", 
		pp->latitude, pp->longitude, pp->speed,	pp->heading, pp->pos,
		pp->num_sats, pp->hdop, pp->altitude, pp->sequence_no);
}

/**
 *	Prints a GPS point structure.
 *	No line feed in function in case combined with other structure
 *	prints on a single row.
 */
int path_gps_print_point(FILE *fp, path_gps_point_t *pp)
{
	int i;
	for (i = 0; i < GPS_OBJECT_ID_SIZE; i++)
		fprintf(fp, "%02hhx", pp->object_id[i]);
	fprintf(fp," ");
	print_timestamp(fp, &pp->local_time);
	fprintf(fp," ");
	print_timestamp(fp, &pp->utc_time);
	fprintf(fp," ");	
	fprintf(fp,"%6d ", pp->date);	
	fprintf(fp,"%.7f ", pp->latitude);	
	fprintf(fp,"%.7f ", pp->longitude);	
	fprintf(fp, "%.3f ", pp->speed);	
	fprintf(fp, "%.3f ", pp->heading);	
	fprintf(fp, "%d ", pp->pos);	
	fprintf(fp, "%2d ", pp->num_sats);	
	fprintf(fp, "%3f ", pp->hdop);
	fprintf(fp,"%.2f ", pp->altitude);	
	fprintf(fp, "%d ", pp->sequence_no);	/// added for heartbeat
	return 1;
}

static char *pttbl_str = "pttbl (object_id, local_time, local_ms, utc_time, utc_ms, latitude, longitude, speed, heading, pos, num_sats, hdop, sequence_no, date_time)";

/**
 *	Creates insertion string for gps point in sqlite table	
 */
void path_gps_point_insert_str(char *cmd_str, path_gps_point_t *pp)
{
	char tmp_str[MAX_CMD_STR];
	char id_str[2*GPS_OBJECT_ID_SIZE + 6]; ///"x'XXXXXXXXXXXX'"\0
	int i;
	
	id_str[0] = '"'; 
	id_str[1] = 'x'; 
	id_str[2] = '\''; 
	for (i = 0; i < GPS_OBJECT_ID_SIZE; i++) 
		snprintf(&id_str[2*i+3], 3, "%02x", pp->object_id[i]);
	id_str[2*GPS_OBJECT_ID_SIZE + 3] = '\'';
	id_str[2*GPS_OBJECT_ID_SIZE + 4] = '"';
	id_str[2*GPS_OBJECT_ID_SIZE + 5] = '\0';
	
	snprintf(tmp_str, MAX_CMD_STR, 
"(%s,\"%02d:%02d:%02d\",%d,\"%02d:%02d:%02d\",%d,%.7f,%.7f,%.6f,%.6f,%d,%d,%.6f,%d,%s)",
			id_str,
			pp->local_time.hour,
			pp->local_time.min,
			pp->local_time.sec,
			pp->local_time.millisec,
			pp->utc_time.hour,
			pp->utc_time.min,
			pp->utc_time.sec,
			pp->utc_time.millisec,
			pp->latitude,
			pp->longitude,		
			pp->speed,
			pp->heading,
			pp->pos,
			pp->num_sats,
			pp->hdop,
			pp->sequence_no,
			"datetime('now')");
	snprintf(cmd_str, MAX_CMD_STR, "insert into %s values %s",
		 pttbl_str, tmp_str);
}


/**
 *      Assumes one row from sqlite database used for gps_point type
 *      is returned and fills the structure
 */
int path_gps_point_fill_from_row(void *ptr, int argc, char **argv,
                char **az_col)
{
	char tsbuf[9];		//temporarily holds timestring
        path_gps_point_t *pp = (path_gps_point_t *) ptr;

#ifdef TRACE_QUERY
	{
		int i;
		printf("fill_from_row: ");
		for (i = 0; i < argc; i++) {
			printf("%s %s\n", az_col[i], argv[i]);
		}
		fflush(stdout);
	}
#endif
	
	strncpy(tsbuf, argv[0], 9);
	tsbuf[2] = tsbuf[5] = tsbuf[8] = '\0';	//end of string markers
	pp->local_time.hour = atoi(&tsbuf[0]);
	pp->local_time.min = atoi(&tsbuf[3]);
	pp->local_time.sec = atoi(&tsbuf[6]);
	pp->local_time.millisec = atoi(argv[1]);
	strncpy(tsbuf, argv[2], 9);
	tsbuf[2] = tsbuf[5] = tsbuf[8] = '\0';	
	pp->utc_time.hour = atoi(&tsbuf[0]);
	pp->utc_time.min = atoi(&tsbuf[3]);
	pp->utc_time.sec = atoi(&tsbuf[6]);
	pp->utc_time.millisec = atoi(argv[3]);
	pp->latitude = atof(argv[4]);
	pp->longitude = atof(argv[5]);		
	pp->speed = atof(argv[6]);
	pp->heading = atof(argv[7]);
	pp->pos = atoi(argv[8]); 
	pp->num_sats = atoi(argv[9]);
	pp->hdop = atof(argv[10]);
	pp->sequence_no = atof(argv[11]);
        return 0;
}


/**
 * 	The following set of functions were writen by undergrad
 * 	Bernard Liang in 2006, and used for analyzing the data from
 * 	the ARINC/gumstix project..
 */


#define METERS_PER_MILE 1609.344
#define FLOAT_TOLERANCE 0.000000000001  // 10^(-12)

static double haversine(double lat1, double lat2, double dLong);
static double vincenty(double lat1, double lat2, double dLong,
			double float_tolerance);

/*
 * Returns the distance between the points.
 * if GPS_UNITS_MILES is set, result is in miles, otherwise in meters.
 * if GPS_ELLIPSOID_EARTH is set, then the more accurate Vincenty
 * method will be used, otherwise the haversine method will be used, which
 * assumes a spherical Earth (which will of course be relatively inaccurate,
 * though the error is not vastly significant.
 * lat/long coordinates of the arguments are expected to be in degrees 
 */
double path_gps_get_distance(path_gps_point_t* point1,
			 path_gps_point_t* point2,
			 int flags, double float_tolerance)
{
	double factor = (flags & GPS_UNITS_MILES) ? (1 / METERS_PER_MILE) : 1;
	double dLong = path_deg2rad(point2->longitude) -
			 path_deg2rad(point1->longitude);
	double lat1 = path_deg2rad(point1->latitude);
	double lat2 = path_deg2rad(point2->latitude);
	/// Make sure degrees have absolute value < 360.0
	while (path_fcompare(fabs(dLong), 360.0, FLOAT_TOLERANCE) > 0) {
		dLong += (path_fcompare(dLong, 0, FLOAT_TOLERANCE) > 0) ?
			 -360.0 : 360.0;
	}
	if (flags & GPS_ELLIPSOID_EARTH) {
		return vincenty(lat1, lat2, dLong, float_tolerance) * factor;
	} else {
		return haversine(lat1, lat2, dLong) * factor;
	}
}

/*
 * Given the same arguments as gps_get_distance, returns the average speed
 * attained by the object between the two specified GPS data points.
 * If GPS_UNITS_MILES is set, then result is in miles per hour.
 * Otherwise, result is in meters per second.
 * Convention: if point2's time is after that of point1, then speed is positive.
 * NOTE: this is NOT based on the instantaneous speed reported by GPVTG
 * only on the latitude/longitude.
 */
double path_gps_get_speed(path_gps_point_t* point1, 
			path_gps_point_t* point2,
			int flags, double float_tolerance)
{
	int factor = (flags & GPS_UNITS_MILES) ? 3600 : 1;
	int diff;	/// time difference in milliseconds
	double delta;	/// time difference in desired units
	if (flags & GPS_USE_LOCALTIME) {
		time_diff(&point1->local_time, &point2->local_time, &diff);
	} else {
		time_diff(&point1->utc_time, &point2->utc_time, &diff);
	}
	delta = (diff * 1000.0)/factor;	/// convert to seconds or hours 
	return (path_gps_get_distance(point1, point2, flags, float_tolerance)
			 / delta);
}

/*
 * Finds the point on the trajectory of the path determined by
 * two GPS data points an object would be at that time, assuming
 * a constant speed.
 * GPS_USE_LOCALTIME specifies whether to use local time or
 * UTC time in the interpolation.
 * (Currently, the time is allowed to not be between the times of the two
 * data points, in which case an extrapolation will be performed.)
 * Returns 0 for error, 1 for success.
 */
int path_gps_interpolate_time(path_gps_point_t* point1,
	 path_gps_point_t* point2, path_gps_point_t* result,
	 timestamp_t* at_time, int flags, double float_tolerance)
{
	int timediff, timediff2;
	double offset;
	path_gps_point_t* upper;
	path_gps_point_t* lower;
	if (flags & GPS_USE_LOCALTIME) {
		if (!valid_timestamp(at_time)) return 0;
		time_diff(&(point1->local_time),
			 &(point2->local_time), &timediff);
	} else {
		if (!valid_timestamp(at_time)) return 0;
		time_diff(&(point1->utc_time), 
			&(point2->utc_time), &timediff);
	}
	upper = (timediff > 0) ? point2 : point1;
	lower = (timediff > 0) ? point1 : point2;

	if (flags & GPS_USE_LOCALTIME) {
		time_diff(&(lower->local_time), at_time, &timediff);
		time_diff(&(lower->local_time), &(upper->local_time),
			 &timediff2);
		offset = ((double) timediff) / timediff2;
	} else {
		time_diff(&(lower->utc_time), at_time, &timediff);
		time_diff(&(lower->utc_time), &(upper->utc_time),
			 &timediff2);
		offset = ((double) timediff) / timediff2;
	}
	return path_gps_interpolate_offset(lower, upper, result, offset,
						float_tolerance);
}

/*
 * Finds the point on the path between two points closest to a particular
 * point, and determines the time at which the object would have reached that
 * location, assuming a constant speed. Stores all this information in "result".
 * If GPS_USE_LOCALTIME is set, uses local time to determine the later time,
 * otherwise uses UTC time. (With realistic data, the setting of this flag is
 * not expected to matter.)
 * As for the "speed" field of result, stores the extrapolated speed assuming
 * constant acceleration.
 * WARNING: result->speed may be inconsistent with the rest of the data in
 * result!
 * Returns 0 for error, 1 for success.
 */
int path_gps_interpolate_point(path_gps_point_t* point1,
	path_gps_point_t* point2, path_gps_point_t* result,
	double desired_lat, double desired_long, int flags,
	double float_tolerance)
{
	double low_lat, low_long, hi_lat, hi_long;
	double numer, denom, offset;
	path_gps_point_t* upper;
	path_gps_point_t* lower;
	int timediff;	/// temporary variable, used as boolean as well as value
	if ((path_fcompare(fabs(desired_lat), 90.0, float_tolerance) > 0) ||
	    (path_fcompare(fabs(desired_long), 180.0, float_tolerance) > 0))
		 return 0;
	if (flags & GPS_USE_LOCALTIME) {
		time_diff(&(point1->local_time), &(point2->local_time),
			&timediff);
	} else {
		time_diff(&(point1->utc_time), &(point2->utc_time), &timediff);
	}
	timediff = (timediff >= 0);
	upper = timediff ? point2 : point1; 
	lower = timediff ? point1 : point2;
	low_lat = lower->latitude;
	low_long = lower->longitude;
	hi_lat = upper->latitude;
	hi_long = upper->longitude;
	if (fabs(hi_long - low_long) > 180.0) {
		if (hi_long > low_long) {
		    low_long += 360.0;
		} else {
		    hi_long += 360.0;
		}
	}

	/* Algorithm taken from
	 * http://astronomy.swin.edu.au/~pbourke/geometry/pointline
	 * As applied here the 180th meridian wraparound causes some problems,
	 * which I do not understand, but for which I have below attempted to
	 * compensate. 
	 */
	numer = ((desired_long - low_long) * (hi_long - low_long) +
	           (desired_lat - low_lat) * (hi_lat - low_lat));
	denom = ((hi_long - low_long) * (hi_long - low_long) +
	           (hi_lat - low_lat) * (hi_lat - low_lat));
	offset = (!path_fcompare(denom, 0.0, float_tolerance) ? 0.5 :
			 numer / denom);
	while (path_fcompare(fabs(offset), 90.0, float_tolerance) > 0) {
		offset += (path_fcompare(offset, 0.0, float_tolerance) > 0) ?
			-180.0 : 180.0;
	}
	return path_gps_interpolate_offset(lower, upper, result, offset,
					float_tolerance);
}

/*
 * Returns a GPS data set that is equivalent to a linear extrapolation of an
 * offset factor (intended to be between 0.0 and 1.0 but will accept other
 * values). Using 0.0 as an offset is equivalent to the "lower" data,
 * 1.0 offset is equivalent to upper, 0.5 is equivalent to the median 
 * data set (all values * averaged), and so on.
 * Returns 0 for error, 1 for success.
 */
int path_gps_interpolate_offset(path_gps_point_t* lower,
	 path_gps_point_t* upper, path_gps_point_t* result,
	 double offset, double float_tolerance)
{
	int status = 1;
	int timediff;
	double low_lat, low_long, hi_lat, hi_long;
	timestamp_t temp;

	low_lat = lower->latitude;
	low_long = lower->longitude;
	hi_lat = upper->latitude;
	hi_long = upper->longitude;
	if (fabs(hi_long - low_long) > 180.0) {
		if (hi_long > low_long) {
		    low_long += 360.0;
		} else {
		    hi_long += 360.0;
		}
	}

	result->longitude = low_long + offset * (hi_long - low_long);
	if (path_fcompare(fabs(result->longitude), 180.0, float_tolerance) > 0){
		result->longitude += (path_fcompare(result->longitude, 0.0,
					 float_tolerance) > 0) ? -360.0 : 360.0;
	}
	result->latitude = low_lat + offset * (hi_lat - low_lat);
	result->speed = lower->speed + offset * (upper->speed - lower->speed);

	time_diff(&(lower->local_time), &(upper->local_time), &timediff);
	timediff = (int) (timediff * offset);
	if (timediff >= 0) {
		ms_to_ts(timediff, &temp);
		status &= increment_timestamp(&(lower->local_time), &temp,
	                              &(result->local_time));
	} else {
		ms_to_ts(-1 * timediff, &temp);
		status &= decrement_timestamp(&(lower->local_time),
			 &temp, &(result->local_time));
	}
	time_diff(&(lower->utc_time), &(upper->utc_time), &timediff);
	timediff = (int) (timediff * offset);
	if (timediff >= 0) {
		ms_to_ts(timediff, &temp);
		status &= increment_timestamp(&(lower->utc_time), &temp,
				      &(result->utc_time));
	} else {
		ms_to_ts(-1 * timediff, &temp);
		status &= decrement_timestamp(&(lower->utc_time), &temp,
	                                  &(result->utc_time));
	}
	return status;
}

/*
 * Given a polygon defined by a set of lat/long points
 * and another lat/long in GPS format, returns 
 * 0 if the point is outside the region or on a boundary,
 * 1 if the point is within the region,
 * Uses a form of ray casting to solve the point-in-polygon problem. 
 * Motivation: Wikipedia; see http://en.wikipedia.org/wiki/Point_in_polygon
 * NOTE: does not (yet) account for extreme polar cases,
 * assuming we're working with relatively local distances.
 * NOTE: Currently, wraparound at the 180th meridian is not supported.
 * NOTE: the boundary and a distance of float_tolerance around it
 * is assumed to NOT be in the region.
 */
int path_gps_is_inside_polygon(path_gps_polygon_t* closed_region, 
	double latitude, double longitude, double float_tolerance)
{
	int i, j;  // index of "previous" and "next" vertices, respectively
	int result = 0;
	double upper, lower, temp, slope;
	double pt_lat = path_deg2rad(latitude);
	double pt_long = path_deg2rad(longitude);
	double i_lat, i_long, j_lat, j_long;
	if (closed_region->size < 3) return 0;
	for (i = 0; i < closed_region->size; i++) {
		j = (i == closed_region->size - 1) ? 0 : i + 1;
		i_lat = path_deg2rad(closed_region->vertex[i].latitude);
		i_long = path_deg2rad(closed_region->vertex[i].longitude);
		j_lat = path_deg2rad(closed_region->vertex[j].latitude);
		j_long = path_deg2rad(closed_region->vertex[j].longitude);

		if ((!path_fcompare(i_long, pt_long, float_tolerance)) && 
			(!path_fcompare(i_lat, pt_lat, float_tolerance))) 
			return 0;  /* the point lies "on" a vertex */

		if (path_fcompare(i_long, j_long, float_tolerance) == 0) {
		/// vertical edge
			if (!path_fcompare(i_long, pt_long, float_tolerance)) {
			/// vertical edge with same longitude as this pt
				if (path_fcompare(i_lat, j_lat,
						 float_tolerance) > 0) {
					lower = j_lat;
					upper = i_lat;
				} else {
					lower = i_lat;
			    		upper = j_lat;
				}
				if ((path_fcompare(lower, pt_lat,
						float_tolerance) > 0) ||
					(path_fcompare(upper, pt_lat, 
						float_tolerance) < 0))	
				 /// the ray crosses over this edge 
					continue; 
				else
				 /// the point lies "on" a vertical edge 
					return 0; 
			}
			continue;	/// look for non-vertical edge
		} else {
	    	/** find out where the edge intersects the poleward ray
		 * from the point
		 */
			if (path_fcompare(i_long, j_long, float_tolerance)
							 > 0) {
				lower = j_long;
				upper = i_long;
			} else {
				lower = i_long;
				upper = j_long;
			}
			if ((path_fcompare(lower, pt_long, float_tolerance) < 0)
				&& (path_fcompare(upper, pt_long,
					 float_tolerance) > 0)) {
				slope = (j_lat - i_lat) / (j_long -  i_long);
				temp = slope * (pt_long - i_long) + i_lat;
				if (!path_fcompare(temp, pt_lat,
							 float_tolerance)) {
					return 0;
				} else if (path_fcompare(temp, pt_lat,
						 float_tolerance) <= 0) {
					continue;
				} else {
				    result = !result;
				}
			}
		}
	}
	return result;
}

/*
 * Applies the haversine method, as described on 
 * http://www.movable-type.co.uk/scripts/LatLong.html
 * Arguments are in radians. Returns result in meters.
 */
static double haversine(double lat1, double lat2, double dLong)
{
	/* Quadratic mean radius of earth;
	 *  see http://en.wikipedia.org/wiki/Earth_radius
	 */
	double a, radius = 6372795.477598;
	a = sin((lat2-lat1)/2) * sin((lat2-lat1)/2) +
	cos(lat1) * cos(lat2) * sin(dLong/2) * sin(dLong/2);
	return radius * 2 * atan2(sqrt(a), sqrt(1-a));
}

/*
 * Applies the Vincenty method, as described on
 * http://www.movable-type.co.uk/scripts/LatLongVincenty.html
 * Assumes the WGS-84 datum, with
 * semi-major axis 6,378,137 m
 * semi-minor axis 6,356,752.3 m
 * Arguments are in radians. Returns result in meters.
 */
static double vincenty(double lat1, double lat2, double dLong, 
			double float_tolerance)
{
	double a = 6378137;
	double b = 6356752.3;
	double f = 1 / 298.257223563;
	double sigma, sinSigma, cosSigma, sinAlpha, cos2Alpha, cos2SigmaM, C;
	double U1, U2, lambda, lambdaP, u2, A, B, deltaSigma;
	U1 = atan((1-f)*tan(lat1));
	U2 = atan((1-f)*tan(lat2));
	lambda = dLong; lambdaP = 2*M_PI;
	while (fabs(lambda-lambdaP) > float_tolerance) {
		sinSigma = sqrt((cos(U2)*sin(lambda)) * (cos(U2)*sin(lambda)) +
	           (cos(U1)*sin(U2)-sin(U1)*cos(U2)*cos(lambda)) *
	           (cos(U1)*sin(U2)-sin(U1)*cos(U2)*cos(lambda)));
		if (!path_fcompare(0.0, sinSigma, float_tolerance))
			 return 0;
		cosSigma = sin(U1)*sin(U2) + cos(U1)*cos(U2)*cos(lambda);
		sigma = atan2(sinSigma, cosSigma);
		sinAlpha = cos(U1)*cos(U2)*sin(lambda)/sinSigma;
		cos2Alpha = 1 - sinAlpha * sinAlpha;
		if (!path_fcompare(0.0, cos2Alpha, float_tolerance))
			return fabs(a * dLong);
		cos2SigmaM = cosSigma - 2*sin(U1)*sin(U2)/cos2Alpha;
		C = f/16*cos2Alpha*(4+f*(4-3*cos2Alpha));
		lambdaP = lambda;
		lambda = dLong + (1-C) * f * sinAlpha *
			 (sigma + C*sinSigma*(cos2SigmaM+
				C*cosSigma*(-1+2*cos2SigmaM*cos2SigmaM)));
	}
	u2 = cos2Alpha * (a*a - b*b)/(b*b);
	A = 1 + u2/16384*(4096+u2*(-768+u2*(320-175*u2)));
	B = u2/1024*(256+u2*(-128+u2*(74-47*u2)));
	deltaSigma = 
		B*sinSigma*(cos2SigmaM+B/4*(cosSigma*
			(-1+ 2*cos2SigmaM*cos2SigmaM)- 
			B/6*cos2SigmaM*(-3+4*sinSigma*sinSigma)*
			(-3+4*cos2SigmaM*cos2SigmaM)));
	return b*A*(sigma-deltaSigma);
}

/// Following code originally developed by John Spring on QNX6
/// Currently seems a bit off on the Linux system I tested
int gps2localtime(int utc_date, float utc_time )
{

        time_t curr_local_time;
        time_t curr_utc_time;
        time_t gm_epoch;
        time_t local_epoch;
        struct tm local_time;
        extern long timezone;
        extern int daylight;

        struct timespec update_tp;
        struct tm gps_tm;
        float utc_time_frac;
        float utc_time_int;
        

        curr_local_time = time(NULL);

#ifdef DEBUG
        printf("Current local time = %d string = %s", \
		(int)curr_local_time, ctime( &curr_local_time));
#endif
        memset( &gps_tm, sizeof(struct tm), 0);
        gps_tm.tm_year = utc_date % 100 + 100;
        gps_tm.tm_mon = (utc_date / 100) % 100 - 1;
        gps_tm.tm_mday = utc_date / 10000;
        
        utc_time_frac = modff(utc_time, &utc_time_int);
        gps_tm.tm_hour = ((int)(utc_time_int) / 10000);
        gps_tm.tm_min = (( (int)(utc_time_int) / 100) % 100) ;
        gps_tm.tm_sec = (int)(utc_time_int) % 100;
        curr_utc_time = mktime(&gps_tm);

#ifdef DEBUG
        printf("Desired UTC time   = %d string = %s", \
		(int)curr_utc_time, ctime( &curr_utc_time));
#endif
        local_epoch = curr_utc_time;
        gm_epoch = curr_utc_time;
        
        localtime_r( &local_epoch, &local_time );               
#ifdef QNX6
        local_epoch = gm_epoch - 28800 + 3600 ;
#else
        local_epoch = gm_epoch - 28800 + (3600 * local_time.tm_isdst);
#endif
        update_tp.tv_sec = local_epoch;
        update_tp.tv_nsec = (int)utc_time_frac * 1000000000;
	if( clock_settime(CLOCK_REALTIME, &update_tp) != 0 ) {
                        perror( "ERROR:gps2systime" );
                        return -1;
	}
	printf("Changing system clock!\n");
        
#ifdef DEBUG
        printf("Final local time   = %d string = %sdst = %d\n", 
		(int)local_epoch, ctime( &local_epoch), local_time.tm_isdst);
        fflush(stdout);
#endif
        return 0;
}

/// Try using date command instead
/// To call this process must run as root
/// Can't get more accurate than a second this way
/// This version is Pacific time zone specific 
int clockset2gps(int utc_date, double utc_time )
{
	int month, day, hour, min, sec, year;
	char date_string[128];
        day = utc_date / 10000;
	month = (utc_date - day * 10000) / 100;
	year = (utc_date -day*10000 - month * 100);
        hour = (utc_time / 10000);	// Pacific time zone
        min = (utc_time - hour * 10000) / 100; 
        sec = (utc_time - hour * 10000 - min * 100); 

	hour -= 7;	/// Pacific time zone
	if (hour < 0){	/// UTC is next day
		hour += 24;
		day -= 1;
	}

	snprintf(date_string, 128, "date %02d%02d%02d%02d%s%02d.%02d",
			month, day, hour, min, "20", year, sec);
	system(date_string);
	return 1;
}

/**
 *	Given an input array of GPS points in degrees, and an origin point,
 *	fills in two output arrays of X,Y coordinates in meters, where
 * 	the Y axis points north, the X axis points east. 
 *	
 *	Flag GPS_ELLIPSOID_EARTH can be set to use the more accurate
 *	Vicenty method when computing the X and Y distances.
 */
 
void path_gps_latlong2xy(path_gps_point_t *pt, path_gps_point_t origin, 
			double *yout, double *xout, int n, int flags)
{
	int i;
	path_gps_point_t xproject, yproject; 
	for (i = 0; i < n; i++) {
		xproject.latitude = origin.latitude;
		xproject.longitude = pt[i].longitude;
		yproject.latitude = pt[i].latitude;
		yproject.longitude = origin.longitude;
		xout[i] = path_gps_get_distance(&origin, &xproject,
				flags, 0.0000000001);
		yout[i] = path_gps_get_distance(&origin, &yproject,
				flags, 0.0000000001);
		if (pt[i].latitude < origin.latitude)
			yout[i] = -yout[i]; 
		if (pt[i].longitude < origin.longitude)
			xout[i] = -xout[i]; 
	}
}

/**
 *	Given input array of X,Y coordinates in meters, and an origin
 *	point, fills in an output array of GPS points with Lat/Long in
 *	degrees.  The array of GPS points must already be allocated!
 *
 * 	The Y axis points north, the X axis points east. Note that
 * 	altitude is used in the underlying calculations, and is pulled
 * 	from the value in the origin gps point structure -- so be sure
 * 	to initialize that value sensibly (0.0 works fine).  Altitude
 * 	is _not_ included in the output structure.
 *	
 *	This conversion routine assumes the ellipsoid correction for
 *	GPS conversion, similar to that used when the flag
 *	GPS_ELLIPSOID_EARTH is set in path_gps_latlong2xy() above.
 *	This function is essentially the inverse of the that function,
 *	but only when the ellipsoid correction is applied.  There is
 *	no inverse for the above if ellipsiod correction was not used.
 */

void path_gps_xy2latlong(double *xin, double *yin, 
			 path_gps_point_t origin, 
			 path_gps_point_t *pts_out, int n)
{
        int i;
        double alt_ref = origin.altitude;
	for (i = 0; i < n; i++) {
	        double lat = 0.0;
		double lon = 0.0;
		double alt = 0.0;
		int status = enu2lla(xin[i], yin[i], alt_ref, 
				     origin.latitude, origin.longitude, 
				     alt_ref, &lat, &lon, &alt);
		if (1 != status) {
		        fprintf(stderr, "enu2lla failed.\n");
		}

		pts_out[i].latitude = lat;
		pts_out[i].longitude = lon;
	}
	return;
}
