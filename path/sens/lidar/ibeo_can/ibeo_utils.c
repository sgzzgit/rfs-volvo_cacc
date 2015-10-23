/**\file
 *
 *	ibeo_utils.c	
 *		
 *	Data and utility functions 
 *	for IBEO laser scanners.
 *
 *	Revised for compatibility with IBEO CAN message protocol Version 1.7.0
 *	by Sue Dickey in July 2008. Only list object definitions were updated,
 *	not information about instruction type. The IBEO Alasca is
 *	providing data over Ethernet in LONG format, so that is used.
 */
#include "ibeo.h"
#undef DO_TRACE

// Stores information about each instruction type
ibeo_instruction_t ibeo_instr[] = {
	{"Synchronization", IBEO_SYNC_ID, 4, 1000},
	{"Vehicle Velocity", IBEO_VELOCITY_ID, 2, 100},
	{"Yaw Rate", IBEO_YAWRATE_ID, 4, 100},
	{"Steering Angle", IBEO_STEERINGANGLE_ID, 4, 100},
	{"Calibration",IBEO_CALIBRATION_ID, 8, 0},
	{"Sensor Command",IBEO_DEFAULT_SENSOR_ID, 8, 0},
	{"Sensor Data",IBEO_DEFAULT_SENSOR_ID+1, 8, 0}
};

/* Parses list header and fills in structure, updates cycle
 * counter if this is the basic list header
 */
int ibeo_parse_list_header(ibeo_data_t *pcur, unsigned char *data,
		unsigned char *pcycle)
{
	ibeo_list_header_t *plh0 = &pcur->msg.lst;
	ibeo_list_ext1_t *plh1 = &pcur->msg.lst_ext1;
	ibeo_list_ext2_t *plh2 = &pcur->msg.lst_ext2;
	ibeo_parameter_data_t *pparam = &pcur->msg.param;

	switch (pcur->onum) {
	case 0: // list header
		plh0->object_style = data[1];
		plh0->object_count = GETFIELD(data[2], 7, 2);
		plh0->sensor_status = 0;	/// no longer supported 	
		plh0->calibration_flag = GETFIELD(data[2], 0, 0); 	
		plh0->cycle_counter = data[3];
		*pcycle = data[3];
		plh0->timestamp = (data[4] << 24) |
				  (data[5] << 16) |
				  (data[6] << 8) |
				  data[7];
		break;
	case 1: // list header, ext 1, environment info, 255 N/A for all fields
		plh1->sensor_dirty = data[1];	//0 OK, 1 clean 
		plh1->rain_detection = data[2]; //0 no rain, 1-254 rain points
		plh1->dirt_start = data[3];	// dirt start angle 0-180 
		plh1->dirt_end = data[4]; 	// dirt end angle 0-108
		break;
	case 2: // parameter reading // not supported in 1.7.0 version?
		pparam->parameter = data[1];
		pparam->data_type = data[2];
		pparam->data_value = (data[6] << 24) |
					(data[5] << 16) |
					(data[4] << 8) |
					data[3]; 
		break;
	case 3: // list header, ext 2 // not supported in 1.7.0 version?
		plh2->left_lane_offset = (GETFIELD(data[2], 4, 0) << 8) |
					data[1];
		plh2->right_lane_offset = (GETFIELD(data[4], 1, 0) << 11) |
					(data[3] << 3) |
					GETFIELD(data[2], 7, 5);
		plh2->heading_angle = (GETFIELD(data[5], 1, 0) << 6) |
					GETFIELD(data[4], 7, 2);
		plh2->curvature = (GETFIELD(data[6], 6, 0) << 6) |
					GETFIELD(data[5], 7, 2); 
		plh2->confidence = (GETFIELD(data[7], 3, 0) << 1) |
					GETFIELD(data[6], 7, 7);
		plh2->horizon = GETFIELD(data[7], 6, 4);
		break;
	default: // unknown object number setting for list header
		break;
	}
	return 1;
}

int ibeo_parse_obj_header(ibeo_data_t *pcur, unsigned char *data)
{
	ibeo_object_header_t *pobj = &pcur->msg.obj;
	pobj->tracking_number = data[1]; 	// constant object lifetime
	pobj->tracking_status = data[2]; 	// 0 known, 1 unknown, 255 N/A
	pobj->classification = data[3];		// see 1.7, chapter 7 
	pobj->point_count = data[4] + 1; 	// see 1.7, section 5.1.3
	pobj->position_x =  0;	// not part of object header now
	pobj->position_y = 0;	// not part of object header now 
	pobj->velocity_x = data[5];	// relative velocity, see 1.7 for units
	pobj->velocity_y = data[6];	
	pobj->velocity_x_ext = GETFIELD(data[7], 7,4);	// divide by 32, add 
	pobj->velocity_y_ext = GETFIELD(data[7], 3,0); 
	return 1;
}
 
/// This is called "Object deviation info" in IBEO CAN message protocol 1.7
int ibeo_parse_obj_ext1(ibeo_data_t *pcur, unsigned char *data)
{
	ibeo_object_ext1_t *pobj = &pcur->msg.obj_ext1;
	pobj->relative_moment = data[1]; 
	pobj->position_x_sigma = data[2]; 
	pobj->position_y_sigma = data[3]; 
	pobj->velocity_x_sigma = data[4]; 
	pobj->velocity_y_sigma = data[5]; 
	pobj->position_cor = data[6];
	pobj->velocity_cor = data[7];
	return 1;
}

/// This is called "Object classification and age" in 1.7
int ibeo_parse_obj_ext2(ibeo_data_t *pcur, unsigned char *data)
{
	ibeo_object_ext2_t *pobj = &pcur->msg.obj_ext2;
	pobj->height = data[1];
	pobj->height_sigma = data[2];
	pobj->class_certainty = data[3]; 
	pobj->class_age = data[4];	// count of tracking scans 
	pobj->object_age = (data[6] << 8) | data[5];
	return 1;
}

/// This is called "Extended Object Info" in 1.7
int ibeo_parse_obj_ext3(ibeo_data_t *pcur, unsigned char *data)
{
	ibeo_object_ext3_t *pobj = &pcur->msg.obj_ext3;
	pobj->info_type = data[1];
	switch (pobj->info_type) {
	case 0:
		pobj->info.collision_info.ttc = data[2] << 8 || data[3];
		pobj->info.collision_info.crash_probability = data[4];
		break;
	case 1:
		pobj->info.abs_velocity.x0 = data[2]; 
		pobj->info.abs_velocity.y0 = data[3]; 
		pobj->info.abs_velocity.x0_ext = GETFIELD(data[4],7,4); 
		pobj->info.abs_velocity.y0_ext = GETFIELD(data[4],3,0); 
		pobj->info.abs_velocity.x1 = data[5]; 
		pobj->info.abs_velocity.y1 = data[6]; 
		pobj->info.abs_velocity.x1_ext = GETFIELD(data[7],7,4); 
		pobj->info.abs_velocity.y1_ext = GETFIELD(data[7],3,0); 
		break;
	default:
		return 0;	/// unsupposrted info type 
		break;
	}
	return 1;
}

/* Structure holds two points, in order from left to right
 */
int ibeo_parse_obj_point(ibeo_data_t *pcur, unsigned char *data)
{
	ibeo_object_point_t *ppt = &pcur->msg.pt;
	ppt->point_number = GETFIELD(data[1], 7, 4);
	printf("DEBUG %d point_number\n", ppt->point_number);
	ppt->x0 = (GETFIELD(data[1], 3, 0) << 9) |
			(data[2] << 1) |
			GETFIELD(data[3], 7, 7);
	ppt->y0 = (GETFIELD(data[3], 6, 0) << 6 ) |
			GETFIELD(data[4], 7, 2); 
	ppt->x1 = (GETFIELD(data[4], 1, 0) << 11) |
			  (data[5] << 3) |
			 GETFIELD(data[6], 7, 5); 
	ppt->y1 =  (GETFIELD(data[6], 4, 0) << 8) | data[7];
	return 1;
}


/* Parses CAN ID and raw data and stores in current ibeo_data_t.
 * Updates running checksum, prints error message if incorrect
 * at end of message, and sets error flag in list end type..
 */
int ibeo_parse_message(ibeo_data_t *pcur,	// parsed data returned here
			can_std_id_t id,	// CAN ID 
			unsigned char *data,	// raw data
			unsigned char *pcycle,	//pointer to current cycle 
			int *pchk)		// pointer to running checksum
{
	int retval;
	ibeo_list_end_t *ple = &pcur->msg.end;
	unsigned int chk = *pchk;
	unsigned int end_check = 0;
	int i;
#ifdef DO_TRACE
	printf("ibeo_parse_message: start check %d(0x%x) ",
		chk, chk);
#endif

	if (id != IBEO_DATA_ID) {
		printf("Unexpected CAN ID 0x%x\n", id);
		return 0;
	}
	pcur->oid = GETFIELD(data[0], 2, 0);
	pcur->onum = GETFIELD(data[0], 7, 3);
	for (i = 0; i < 8; i++) {
		pcur->raw_data[i] = data[i];
	}
	switch (pcur->oid) {
	case IBEO_DATA_LIST_ID:	// list header
		retval = ibeo_parse_list_header(pcur, data, pcycle);
		break;
	case IBEO_DATA_OBJ_ID:	// object header
		retval = ibeo_parse_obj_header(pcur, data);
		break;
	case IBEO_DATA_EXT1_ID:	// object header, extension 1
		retval = ibeo_parse_obj_ext1(pcur, data);
		break;
	case IBEO_DATA_EXT2_ID:	// object header, extension 2
		retval = ibeo_parse_obj_ext2(pcur, data);
		break;
	case IBEO_DATA_EXT3_ID:	// object header, extension 3
		retval = ibeo_parse_obj_ext3(pcur, data);
		if (retval == 0)
			fprintf(stderr, "Unexpected info type in DATA_EXT3\n");
		retval = 1;	/// don't make this a fatal error
		break;
	case IBEO_DATA_PT_ID:	// object point
		retval = ibeo_parse_obj_point(pcur, data);
		break;
	}
	if (pcur->oid != IBEO_DATA_END_ID) {
		for (i = 0; i < 8; i++) 
			chk += data[i];
		*pchk = chk;		// update running checksum
	} else {
		end_check = (data[4] << 24) |
				 (data[5] << 16) |
				 (data[6] << 8) |
				 data[7]; 
		for (i = 0; i < 4; i++) 
			chk += data[i];
		ple->sensor_status = data[1];
		ple->sensor_id = data[2];	// now scan processing status
		ple->cycle_error = (*pcycle != data[3]);
		ple->checksum_error = (chk != end_check);
		*pchk = 0;
	}
#ifdef DO_TRACE
	printf(" oid %d new chk %d(0x%x) cycle %d\n",
			pcur->oid, *pchk, *pchk, *pcycle);
	if (pcur->oid == IBEO_DATA_END_ID)
		printf("end_check %d (0x%x) end cycle %d\n",
			 end_check, end_check, data[3]);  
#endif
	return retval;
}	

/// For IBEO velocity in meter/sec is scaled as v*1/2 + v_ext/32.
/// v == 254 means velocity > 62.5, will return 63.0
/// v == 0 means velocity < -63.5, will return -64.0
float ibeo_scale_velocity(unsigned char v, unsigned char v_ext)
{
	float fv;
	if (v == 255)
		fv = 1000;	// signals speed is unavailable 
	else
		fv = (v/2.0 - 64.0) + v_ext/32.0; 
	return (fv);
} 

/// For IBEO relative position in meters is scaled as p*0.05 -200
/// p == 8191 means unavailable 
float ibeo_scale_position(unsigned short p)
{
	float fp;
	if (p == 8191)
		fp = 1000;	// signals position is unavailable 
	else
		fp = p*0.05 - 200; 
	return (fp);
} 


/* Takes parsed data from current message and stores in the correct
 * field of the local structures that will be used to write the
 * database variables. Database variables will be written at
 * the end of each list.
 */

void ibeo_pack_data(ibeo_list_typ *plist, ibeo_obj_typ *obj_array,
			ibeo_data_t *pcur)
{
	int i = pcur->onum;	// object number
	ibeo_obj_typ *p = &obj_array[i];
	int npt = 0;		// point number

	switch (pcur->oid) {
	case IBEO_DATA_LIST_ID: // list header
		if (pcur->onum == 0) {
			plist->object_style = pcur->msg.lst.object_style;
			plist->object_count = pcur->msg.lst.object_count;
			plist->sensor_status = pcur->msg.lst.sensor_status;
			plist->calibration_flag = pcur->msg.lst.calibration_flag;
			plist->cycle_counter = pcur->msg.lst.cycle_counter;
			plist->timestamp = pcur->msg.lst.timestamp;
		}
		if (pcur->onum == 1) {
			plist->sensor_dirty = pcur->msg.lst_ext1.sensor_dirty;
			plist->rain_detection =
					 pcur->msg.lst_ext1.rain_detection;
			plist->dirt_start = 2.0 * pcur->msg.lst_ext1.dirt_start
						 - 180.0;
			plist->dirt_end = 2.0 * pcur->msg.lst_ext1.dirt_end;
		}
		break;
	case IBEO_DATA_OBJ_ID:  // object header
		p->object_number = i;
		p->tracking_number = pcur->msg.obj.tracking_number;	
		p->tracking_status = pcur->msg.obj.tracking_status;	
		p->classification = pcur->msg.obj.classification;	
		p->point_count = pcur->msg.obj.point_count;	
		printf("DATA_OBJ_ID %d: DEBUG point_count %d\n",
			i, pcur->msg.obj.point_count);
		p->velocity_x = ibeo_scale_velocity(pcur->msg.obj.velocity_x,	
					pcur->msg.obj.velocity_x_ext);	
		p->velocity_y = ibeo_scale_velocity(pcur->msg.obj.velocity_y,	
					pcur->msg.obj.velocity_y_ext);	
		break;
	case IBEO_DATA_EXT1_ID: // object header, extension 1
		p->relative_moment = 0.5 * pcur->msg.obj_ext1.relative_moment;
		p->position_x_sigma = 0.1 * pcur->msg.obj_ext1.position_x_sigma;
		p->position_y_sigma = 0.1 * pcur->msg.obj_ext1.position_y_sigma;
		p->velocity_x_sigma = 0.5 * pcur->msg.obj_ext1.velocity_x_sigma;
		p->velocity_y_sigma = 0.5 * pcur->msg.obj_ext1.velocity_y_sigma;
		p->position_cor = 0.01 * pcur->msg.obj_ext1.position_cor - 1.0;
		p->velocity_cor = 0.01 * pcur->msg.obj_ext1.velocity_cor - 1.0;
		break;
	case IBEO_DATA_EXT2_ID: // object header, extension 2
		p->height = 0.05 * pcur->msg.obj_ext2.height;
		p->height_sigma = 0.05 * pcur->msg.obj_ext2.height_sigma;
		p->class_certainty = pcur->msg.obj_ext2.class_certainty;
		p->class_age = pcur->msg.obj_ext2.class_age;
		p->object_age = pcur->msg.obj_ext2.object_age;
		break;
	case IBEO_DATA_EXT3_ID: // object header, extension 3
		if (pcur->msg.obj_ext3.info_type == IBEO_EXT3_COLLISION_INFO){
			ibeo_collision_info_t *pci = 
				&pcur->msg.obj_ext3.info.collision_info;
			p->ttc = pci->ttc; /// milliseconds
			p->crash_probability = pci->crash_probability;
		}
		// IBEO_EXT3_ABS_VELOCITY not yet used
		break;
	case IBEO_DATA_PT_ID:   // object point
		npt = pcur->msg.pt.point_number;
		printf("DEBUG: IBEO_DATA_PT_ID %d point_number\n", npt);
		p->point[npt].x = pcur->msg.pt.x0;
		p->point[npt].y = pcur->msg.pt.y0;
		p->point[npt+1].x = pcur->msg.pt.x1;
		p->point[npt+1].y = pcur->msg.pt.y1;
		break;
	}
}

void ibeo_update_database(db_clt_typ *pclt, ibeo_list_typ *plist,
				 ibeo_obj_typ *obj_array)
{
	int i;
	int db_num = DB_IBEO_LIST_VAR;
	if (clt_update(pclt, db_num, db_num, sizeof(ibeo_list_typ),
                       (void *) plist) == FALSE)
               printf("err: clt_update(%d)\n", db_num);
	for (i = 0; i < plist->object_count; i++){
		db_num = DB_IBEO_OBJ0_VAR + i;
		if (clt_update(pclt, db_num, db_num, sizeof(ibeo_obj_typ),
                       (void *) plist) == FALSE)
			printf("err: clt_update(%d)\n", db_num);
	}
}

/* Output a single line for each active object to the input file
 * Order of fields: (all on one line)
 * 
 *	Current timestamp	(system time of write to file)
 *	object_number		(from ibeo_obj_typ)
 *	cycle_counter		(from ibeo_list_typ)
 *	object_style		(from ibeo_list_typ)
 *	sensor_status		(from ibeo_list_typ)
 *	calibration_flag	(from ibeo_list_typ)
 *	object list timestamp	(from ibeo_list_typ)
 *	tracking_number		(from ibeo_obj_typ)
 *	tracking_status		(from ibeo_obj_typ)
 *	classification		(from ibeo_obj_typ)
 *	rel velocity_x		(from ibeo_obj_typ)
 *	rel velocity_y		(from ibeo_obj_typ)
 *	point_count		(from ibeo_obj_typ)
 *	16 x, y point coordinates (from ibeo_obj_typ)
 *				print point coordinates as -1 if invalid
 *
 */

void ibeo_print_database(FILE *fp, ibeo_list_typ *plist, 
					ibeo_obj_typ *obj_array,
					timestamp_t *pts)
{
	int i, j;

	for (i = 0; i < plist->object_count; i++) {
		ibeo_obj_typ *p = &obj_array[i];
		print_timestamp(fp, pts);
		print_timestamp(stdout, pts);

		fprintf(fp, " ");
		fprintf(fp,"%3hhu %3hhu %3hhu %3hhu %3hhu %3hhu %10u ",
			p->object_number, 
 			p->tracking_number,	
 			p->tracking_status,	
			plist->cycle_counter,		
 			plist->object_style,
 			plist->calibration_flag, // 1 calibrated, 0 not	
 			plist->timestamp); 	//in ms, wraps every 7 weeks
		fprintf(fp,"%3hhu %3hhu %.02f %.02f ",
			plist->sensor_dirty,		
			plist->rain_detection,		
			plist->dirt_start,		
			plist->dirt_end);		
		fprintf(fp, "%3hhu %6hu %3hhu %.02f %.02f %3hhu ",
			p->classification,		
			p->ttc,
			p->crash_probability,
			p->velocity_x,
			p->velocity_y,
			p->point_count);
		for (j = 0; j < 16; j++){
			unsigned short x, y;
			// 13 bit quantities before scaling
			// x, y values +/- 200.0 meters
			// after doing scaling, see spec
			printf("DEBUG: obj %d p->point_count %d j %d:", 
				p->object_number, p->point_count, j);
			if (j < p->point_count) {
				x = p->point[j].x;
				y = p->point[j].y;
			} else {
				x = 10000;	/// unavailable
				y = 10000;	/// unavailable
			}	
			fprintf(fp, "%hu %hu ", x, y);
			printf("%hu %hu \n", x, y);
		}
		fprintf(fp, "\n");
	}
}
