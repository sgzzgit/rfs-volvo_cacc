/**\file
 *
 *	laipdu.c	Functions for receiving and sending Lane Assist Interface
 *			messages on the J1939 in-vehicle network and for
 *			printing each message type for debugging.
 */

#include <old_include/std_jbus.h>
#include <lai_vars.h>
#include <veh_bus.h>
#include "dvimsg.h"
#include "mag.h"

#undef DO_TRACE 

#ifndef __QNXNTO__
#define print_timestamp(a,b) j1939_print_timestamp(b, a, 1)
#endif

/* Reads of data from Jbus are formatted into a single database variable,
 * that is then placed in the database in a uniform way for all reads
 * by the rdj1939 program that calls these functions.
 *
 * Sends of data to the Jbus may take fields from more than one
 * database variable in some situations, so the formatting function
 * does the database reads.
 */

/* Function to set low level CAN bus header fields
 */
void set_pdu_header(struct j1939_pdu *pdu, j1939_dbv_info *pinfo, 
	j1939_send_item *pitem, j1939_send_info *sinfo)
{
	pdu->priority = pitem->priority;
	pdu->R = 0;
	pdu->DP = 0;
	pdu->pdu_format = BYTE1(pinfo->pgn);
	pdu->pdu_specific = BYTE0(pinfo->pgn);
	pdu->src_address = sinfo->src;
	pdu->numbytes = 8;
}

/** 	LAI_CTRLSTAT Lane Assist Interface Control Status received on Control Computer
  * 	pdu_to_lai_ctrl parses bytes obtained from read of Jbus into 
  *	type that will be updated to database..
  */
void pdu_to_lai_ctrlstat(struct j1939_pdu *pdu, void *pdbv)
{
	lai_ctrlstat_typ *ctrlstat = (lai_ctrlstat_typ *)pdbv;
        unsigned int four_bytes;

        four_bytes = FOURBYTES(pdu->data_field[3], pdu->data_field[2],
                        pdu->data_field[1], pdu->data_field[0]);

	ctrlstat->steer_angle = scale_m1440_to_p1440(four_bytes);
#ifdef DO_TRACE
	printf("pdu_to_lai_ctrlstat: %d %.3f\n", four_bytes, ctrlstat->steer_angle);
#endif
	ctrlstat->ready = pdu->data_field[4]; 
	ctrlstat->actuator_status = pdu->data_field[5]; 
	ctrlstat->fault_code = pdu->data_field[6]; 
}

/** 
 *	LAI_CTRLSTAT Lane Assist Interface Control Status sent from Sensor Computer 
 */
void lai_ctrlstat_to_pdu(db_clt_typ *pclt, struct j1939_pdu *pdu,
	void *pdbv_info, void *pitem_info, void *psend_info)
{
	j1939_dbv_info *pinfo = (j1939_dbv_info *) pdbv_info;
	j1939_send_info *sinfo = (j1939_send_info *) psend_info; 
	j1939_send_item *pitem = (j1939_send_item *) pitem_info; 
	lat_control_input_typ lat_ctrl;
	db_data_typ db_data;
	unsigned int steer_scaled;

	if (clt_read(pclt, DB_LAT_CONTROL_INPUT_VAR, DB_LAT_CONTROL_INPUT_TYPE,
                           &db_data) == FALSE) {
                fprintf(stderr, "clt_read(DB_LAT_CONTROL_INPUT_VAR)\n");
	}
	lat_ctrl = *((lat_control_input_typ *) db_data.value.user);
	steer_scaled = (lat_ctrl.steer_angle+1440.0) * 1000;
#ifdef DO_TRACE
printf("lai_ctrlstat_to_pdu: steer_scaled %d\n", steer_scaled);
#endif
	pdu->data_field[0] = BYTE0(steer_scaled);
	pdu->data_field[1] = BYTE1(steer_scaled);
	pdu->data_field[2] = BYTE2(steer_scaled);
	pdu->data_field[3] = BYTE3(steer_scaled);
	pdu->data_field[4] = lat_ctrl.ready;
	pdu->data_field[5] = lat_ctrl.actuator_status;
	pdu->data_field[6] = lat_ctrl.fault_code;
	pdu->data_field[7] = 0xff;
	set_pdu_header(pdu, pinfo, pitem, sinfo);
}

void print_lai_ctrlstat(void *pdbv , FILE  *fp, int numeric)
{
	lai_ctrlstat_typ *ctrlstat = (lai_ctrlstat_typ *)pdbv;
	fprintf(fp, "LAI CTRLSTAT: ");
	print_timestamp(fp, &ctrlstat->timestamp);
	if (numeric) {
		fprintf(fp, " %.3f ", ctrlstat->steer_angle);
		fprintf(fp, "%d ", ctrlstat->ready);
	 	fprintf(fp, "%d ", ctrlstat->actuator_status);
	 	fprintf(fp, "%d ", ctrlstat->fault_code);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp, " Steering angle %.3f\n",
			 ctrlstat->steer_angle);
		fprintf(fp, "Steering ready %d\n", ctrlstat->ready);
		fprintf(fp, "Steering actuator status %d\n", 
			ctrlstat->actuator_status);
		fprintf(fp, "Steering fault code %d\n", ctrlstat->fault_code);
	}
}

/* LAI_STIN
 */
void pdu_to_lai_stin(struct j1939_pdu *pdu, void *pdbv)
{
	lai_stin_typ *stin = (lai_stin_typ *)pdbv;
        unsigned int four_bytes, two_bytes;

        four_bytes = FOURBYTES(pdu->data_field[3], pdu->data_field[2],
                        pdu->data_field[1], pdu->data_field[0]);
        two_bytes = TWOBYTES(pdu->data_field[7], pdu->data_field[6]);

	stin->steer_angle = scale_m1440_to_p1440(four_bytes);
#ifdef DO_TRACE
	printf("pdu_to_lai_stin: %d %.3f\n", four_bytes, stin->steer_angle);
#endif
	stin->driver_status = pdu->data_field[4]; 
	stin->failure_code = pdu->data_field[5]; 
	stin->counta = two_bytes; 
}

void lai_stin_to_pdu(db_clt_typ *pclt, struct j1939_pdu *pdu,
	void *pdbv_info, void *pitem_info, void *psend_info)
{
	j1939_dbv_info *pinfo = (j1939_dbv_info *) pdbv_info;
	j1939_send_info *sinfo = (j1939_send_info *) psend_info; 
	j1939_send_item *pitem = (j1939_send_item *) pitem_info; 
	lat_steer_input_typ lat_steer_in;
	db_data_typ db_data;
	unsigned int steer_scaled;

	if (clt_read(pclt, DB_LAT_STEER_INPUT_VAR, DB_LAT_STEER_INPUT_TYPE,
                           &db_data) == FALSE) {
                fprintf(stderr, "clt_read(DB_LAT_STEER_INPUT_VAR)\n");
	}
	lat_steer_in = *((lat_steer_input_typ *) db_data.value.user);
	steer_scaled = (lat_steer_in.steer_angle+1440.0) * 1000;
#ifdef DO_TRACE
printf("lai_stin_to_pdu: steer_scaled %d\n", steer_scaled);
#endif
	pdu->data_field[0] = BYTE0(steer_scaled);
	pdu->data_field[1] = BYTE1(steer_scaled);
	pdu->data_field[2] = BYTE2(steer_scaled);
	pdu->data_field[3] = BYTE3(steer_scaled);
	pdu->data_field[4] = lat_steer_in.driver_status;
	pdu->data_field[5] = lat_steer_in.failure_code;
	pdu->data_field[6] = BYTE0(lat_steer_in.counta);
	pdu->data_field[7] = BYTE1(lat_steer_in.counta);
	set_pdu_header(pdu, pinfo, pitem, sinfo);
}

void print_lai_stin(void *pdbv , FILE  *fp, int numeric)
{
	lai_stin_typ *stin = (lai_stin_typ *)pdbv;
	fprintf(fp, "LAI STIN: ");
	print_timestamp(fp, &stin->timestamp);
	if (numeric) {
		fprintf(fp, " %.3f ", stin->steer_angle);
		fprintf(fp, "%d ", stin->driver_status);
	 	fprintf(fp, "%d ", stin->failure_code);
	 	fprintf(fp, "%d ", stin->counta);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp, " Steering angle %.2f\n", stin->steer_angle);
		fprintf(fp, "Driver status %d\n", stin->driver_status);
		fprintf(fp, "Failure code %d\n", stin->failure_code);
		fprintf(fp, "Counta %d\n", stin->counta);
	}
}

/* LAI_STOUT
 */
void pdu_to_lai_stout(struct j1939_pdu *pdu, void *pdbv)
{
	lai_stout_typ *stout = (lai_stout_typ *)pdbv;
        unsigned int two_bytes;

        two_bytes = TWOBYTES(pdu->data_field[1], pdu->data_field[0]);
	stout->torque = scale_10000_to_volt(two_bytes);
#ifdef DO_TRACE
	printf("pdu_to_lai_stout: %d %.3f\n", two_bytes, stout->torque);
#endif
	stout->mode = pdu->data_field[2]; 
}

void lai_stout_to_pdu(db_clt_typ *pclt, struct j1939_pdu *pdu,
	void *pdbv_info, void *pitem_info, void *psend_info)
{
	j1939_dbv_info *pinfo = (j1939_dbv_info *) pdbv_info;
	j1939_send_info *sinfo = (j1939_send_info *) psend_info; 
	j1939_send_item *pitem = (j1939_send_item *) pitem_info; 
	lat_steer_output_typ lat_steer_out;
	db_data_typ db_data;
	unsigned int torque_scaled;

	if (clt_read(pclt, DB_LAT_STEER_OUTPUT_VAR, DB_LAT_STEER_OUTPUT_TYPE,
                           &db_data) == FALSE) {
                fprintf(stderr, "clt_read(DB_LAT_STEER_OUTPUT_VAR)\n");
	}
	lat_steer_out = *((lat_steer_output_typ *) db_data.value.user);
	torque_scaled = lat_steer_out.torque * 10000;
#ifdef DO_TRACE
printf("lai_stout_to_pdu: torque_scaled %d\n", torque_scaled);
#endif
	pdu->data_field[0] = BYTE0(torque_scaled);
	pdu->data_field[1] = BYTE1(torque_scaled);
	pdu->data_field[2] = lat_steer_out.mode;
	pdu->data_field[3] = 0xff;
	pdu->data_field[4] = 0xff;
	pdu->data_field[5] = 0xff;
	pdu->data_field[6] = 0xff;
	pdu->data_field[7] = 0xff;
	set_pdu_header(pdu, pinfo, pitem, sinfo);
}

void print_lai_stout(void *pdbv , FILE  *fp, int numeric)
{
	lai_stout_typ *stout = (lai_stout_typ *)pdbv;
	fprintf(fp, "LAI STOUT: ");
	print_timestamp(fp, &stout->timestamp);
	if (numeric) {
		fprintf(fp, " %.2f ", stout->torque);
		fprintf(fp, "%d ", stout->mode);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp, " Torque %.2f\n", stout->torque);
		fprintf(fp, "Mode %d\n", stout->mode);
	}
}

/* LAI_LATOUT
 */
void pdu_to_lai_latout(struct j1939_pdu *pdu, void *pdbv)
{
	lai_latout_typ *latout = (lai_latout_typ *)pdbv;
        unsigned int two_bytes;

        two_bytes = TWOBYTES(pdu->data_field[1], pdu->data_field[0]);
	latout->torque = scale_10000_to_volt(two_bytes);
//printf("pdu_to_lai_latout: %d %.3f\n", two_bytes, latout->torque);
	latout->clutch = pdu->data_field[2]; 
}

void lai_latout_to_pdu(db_clt_typ *pclt, struct j1939_pdu *pdu,
	void *pdbv_info, void *pitem_info, void *psend_info)
{
	j1939_dbv_info *pinfo = (j1939_dbv_info *) pdbv_info;
	j1939_send_info *sinfo = (j1939_send_info *) psend_info; 
	j1939_send_item *pitem = (j1939_send_item *) pitem_info; 
	lat_output_typ lat_out;
	db_data_typ db_data;
	unsigned int torque_scaled;

	if (clt_read(pclt, DB_LAT_OUTPUT_VAR, DB_LAT_OUTPUT_TYPE,
                           &db_data) == FALSE) {
                fprintf(stderr, "clt_read(DB_LAT_OUTPUT_VAR)\n");
	}
	lat_out = *((lat_output_typ *) db_data.value.user);
	torque_scaled = lat_out.torque * 10000;
#ifdef DO_TRACE
printf("lai_latout_to_pdu: torque_scaled %d\n", torque_scaled);
#endif
	pdu->data_field[0] = BYTE0(torque_scaled);
	pdu->data_field[1] = BYTE1(torque_scaled);
	pdu->data_field[2] = lat_out.clutch;
	pdu->data_field[3] = 0xff;
	pdu->data_field[4] = 0xff;
	pdu->data_field[5] = 0xff;
	pdu->data_field[6] = 0xff;
	pdu->data_field[7] = 0xff;
	set_pdu_header(pdu, pinfo, pitem, sinfo);
}

void print_lai_latout(void *pdbv , FILE  *fp, int numeric)
{
	lai_latout_typ *latout = (lai_latout_typ *)pdbv;
	fprintf(fp, "LAI LATOUT: ");
	print_timestamp(fp, &latout->timestamp);
	if (numeric) {
		fprintf(fp, " %.2f ", latout->torque);
		fprintf(fp, "%d ", latout->clutch);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp, " Torque %.2f\n", latout->torque);
		fprintf(fp, "Clutch %d\n", latout->clutch);
	}
}

/* LAI_SIGSTAT
 */
void pdu_to_lai_sigstat(struct j1939_pdu *pdu, void *pdbv)
{
	lai_sigstat_typ *sigstat = (lai_sigstat_typ *)pdbv;
        unsigned int two_bytes1, two_bytes2;

        two_bytes1 = TWOBYTES(pdu->data_field[1], pdu->data_field[0]);
        two_bytes2 = TWOBYTES(pdu->data_field[3], pdu->data_field[2]);

	sigstat->mag_health = two_bytes1;
	sigstat->mag_dist = scale_by_100(two_bytes2);
	sigstat->f_sensor = pdu->data_field[4]; 
	sigstat->r_sensor = pdu->data_field[5]; 
}

void lai_sigstat_to_pdu(db_clt_typ *pclt, struct j1939_pdu *pdu,
	void *pdbv_info, void *pitem_info, void *psend_info)
{
	j1939_dbv_info *pinfo = (j1939_dbv_info *) pdbv_info;
	j1939_send_info *sinfo = (j1939_send_info *) psend_info; 
	j1939_send_item *pitem = (j1939_send_item *) pitem_info; 
	signalproc_output_typ sigproc_out;
	db_data_typ db_data;
	unsigned int mag_dist_scaled;

	if (clt_read(pclt, DB_SIGNALPROC_OUTPUT_VAR, DB_SIGNALPROC_OUTPUT_TYPE,
                           &db_data) == FALSE) {
                fprintf(stderr, "clt_read(DB_SIGNALPROC_OUTPUT_VAR)\n");
	}
	sigproc_out = *((signalproc_output_typ *) db_data.value.user);
	mag_dist_scaled = sigproc_out.mag_dist * 100;
	pdu->data_field[0] = BYTE0(sigproc_out.mag_health);
	pdu->data_field[1] = BYTE1(sigproc_out.mag_health);
	pdu->data_field[2] = BYTE0(mag_dist_scaled);
	pdu->data_field[3] = BYTE1(mag_dist_scaled);
	pdu->data_field[4] = sigproc_out.f_sensor;
	pdu->data_field[5] = sigproc_out.r_sensor;
	pdu->data_field[6] = 0xff;
	pdu->data_field[7] = 0xff;
	set_pdu_header(pdu, pinfo, pitem, sinfo);
}

void print_lai_sigstat(void *pdbv , FILE  *fp, int numeric)
{
	lai_sigstat_typ *sigstat = (lai_sigstat_typ *)pdbv;
	fprintf(fp, "LAI SIGSTAT: ");
	print_timestamp(fp, &sigstat->timestamp);
	if (numeric) {
		fprintf(fp, " %d ", sigstat->mag_health);
		fprintf(fp, "%.2f ", sigstat->mag_dist);
		fprintf(fp, "%d ", sigstat->f_sensor);
		fprintf(fp, "%d ", sigstat->r_sensor);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp, " Magnetomer health signal %d\n", sigstat->mag_health);
		fprintf(fp, "Magnet distance %.2f\n", sigstat->mag_dist);
		fprintf(fp, "Front sensor %d\n", sigstat->f_sensor);
		fprintf(fp, "Rear sensor %d\n", sigstat->r_sensor);
	}
}

/* LAI_SIGOUT
 */
void pdu_to_lai_sigout(struct j1939_pdu *pdu, void *pdbv)
{
	lai_sigout_typ *sigout = (lai_sigout_typ *)pdbv;
        unsigned int f_ymeas_bits;
        unsigned int f_ycar_bits;
        unsigned int r_ymeas_bits;
        unsigned int r_ycar_bits;

	f_ymeas_bits = pdu->data_field[1]  | ((pdu->data_field[2] & 0xf) << 8); 
//old	f_ycar_bits = pdu->data_field[3]  | ((pdu->data_field[2] & 0xf0) << 4); 
	f_ycar_bits = pdu->data_field[3]<<4  | ((pdu->data_field[2] & 0xf0) >> 4); 

	r_ymeas_bits = pdu->data_field[5]  | ((pdu->data_field[6] & 0xf) << 8); 
//old	r_ycar_bits = pdu->data_field[7]  | ((pdu->data_field[6] & 0xf0) << 4); 
	r_ycar_bits = pdu->data_field[7]<<4  | ((pdu->data_field[6] & 0xf0) >> 4); 

	sigout->delta_timer_obs = pdu->data_field[0];	//this is in msec/4 !!! 
	sigout->f_ymeas = scale_m2_to_p2(f_ymeas_bits);
	sigout->f_ycar = scale_m2_to_p2(f_ycar_bits);
	sigout->f_mark_flag = BITS21(pdu->data_field[4]); 
	sigout->f_polarity = (pdu->data_field[4] & (1 << 2)) >> 2; 
	sigout->r_mark_flag = (pdu->data_field[4] & 0x18) >> 3; 
	sigout->r_polarity = (pdu->data_field[4] & (1 << 5)) >> 5; 
	sigout->r_ymeas = scale_m2_to_p2(r_ymeas_bits);
	sigout->r_ycar = scale_m2_to_p2(r_ycar_bits);
}

void lai_sigout_to_pdu(db_clt_typ *pclt, struct j1939_pdu *pdu,
	void *pdbv_info, void *pitem_info, void *psend_info)
{
	j1939_dbv_info *pinfo = (j1939_dbv_info *) pdbv_info;
	j1939_send_info *sinfo = (j1939_send_info *) psend_info; 
	j1939_send_item *pitem = (j1939_send_item *) pitem_info; 
	signalproc_output_typ sigproc_out;
	db_data_typ db_data;
	unsigned int delta_timer_scaled;
	unsigned int f_ymeas_scaled, f_ycar_scaled;
	unsigned int r_ymeas_scaled, r_ycar_scaled;
	unsigned int fmark_flag_scaled, rmark_flag_scaled;

	if (clt_read(pclt, DB_SIGNALPROC_OUTPUT_VAR, DB_SIGNALPROC_OUTPUT_TYPE,
                           &db_data) == FALSE) {
                fprintf(stderr, "clt_read(DB_SIGNALPROC_OUTPUT_VAR)\n");
	}
	sigproc_out = *((signalproc_output_typ *) db_data.value.user);
	delta_timer_scaled = (sigproc_out.delta_timer_obs * 1000) / 4;
	f_ymeas_scaled = (sigproc_out.f_ymeas+2.0) * 1000;
	r_ymeas_scaled = (sigproc_out.r_ymeas+2.0) * 1000;
	f_ycar_scaled = (sigproc_out.f_ycar+2.0) * 1000;
	r_ycar_scaled = (sigproc_out.r_ycar+2.0) * 1000;

	// Why is one added to the mark flag fields?
	fmark_flag_scaled = sigproc_out.f_mark_flag + 1;
	rmark_flag_scaled = sigproc_out.r_mark_flag + 1;
	pdu->data_field[0] = delta_timer_scaled;
	pdu->data_field[1] = BYTE0(f_ymeas_scaled);
	pdu->data_field[2] = (f_ymeas_scaled & 0xf00)>>8 |
				 (f_ycar_scaled & 0xf)<<4;
	pdu->data_field[3] = (f_ycar_scaled & 0xff0)>>4;

	pdu->data_field[4] = fmark_flag_scaled | sigproc_out.f_polarity<<2
			   | rmark_flag_scaled<<3 | sigproc_out.r_polarity<<5;

	pdu->data_field[5] = BYTE0(r_ymeas_scaled);
	pdu->data_field[6] = (r_ymeas_scaled & 0xf00)>>8 | 
				(r_ycar_scaled & 0xf)<<4;
	pdu->data_field[7] = (r_ycar_scaled & 0xff0)>>4;
	set_pdu_header(pdu, pinfo, pitem, sinfo);
}

void print_lai_sigout(void *pdbv , FILE  *fp, int numeric)
{
	lai_sigout_typ *sigout = (lai_sigout_typ *)pdbv;
	fprintf(fp, "LAI SIGOUT: ");
	print_timestamp(fp, &sigout->timestamp);
	if (numeric) {
		fprintf(fp, " %d ", sigout->delta_timer_obs);
		fprintf(fp, "%.3f ", sigout->f_ymeas);
		fprintf(fp, "%.3f ", sigout->f_ycar);
		fprintf(fp, "%d ", sigout->f_mark_flag);
		fprintf(fp, "%d ", sigout->f_polarity);
		fprintf(fp, "%d ", sigout->r_mark_flag);
		fprintf(fp, "%d ", sigout->r_polarity);
		fprintf(fp, "%.3f ", sigout->r_ymeas);
		fprintf(fp, "%.3f ", sigout->r_ycar);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp, " Delta timer obs %d\n", sigout->delta_timer_obs);
		fprintf(fp, "Front ymeas %.3f\n", sigout->f_ymeas);
		fprintf(fp, "Front ycar %.3f\n", sigout->f_ycar);
		fprintf(fp, " Front mark flag %d ", sigout->f_mark_flag);
		fprintf(fp, " Front polarity %d ", sigout->f_polarity);
		fprintf(fp, " Rear mark flag %d ", sigout->r_mark_flag);
		fprintf(fp, " Rear polarity %d ", sigout->r_polarity);
		fprintf(fp, "Rear ymeas %.3f\n", sigout->r_ymeas);
		fprintf(fp, "Rear ycar %.3f\n", sigout->r_ycar);
	}
}

/* LAI_GYRO
 */
void pdu_to_lai_gyro(struct j1939_pdu *pdu, void *pdbv)
{
	lai_gyro_typ *gyro = (lai_gyro_typ *)pdbv;
        unsigned int four_bytes;

        four_bytes = FOURBYTES(pdu->data_field[3], pdu->data_field[2],
                        pdu->data_field[1], pdu->data_field[0]);
	gyro->gyro_rate = scale_m1_to_p1(four_bytes);
//printf("pdu_to_lai_gyro: %d %.5f\n", four_bytes, gyro->gyro_rate);
}

void lai_gyro_to_pdu(db_clt_typ *pclt, struct j1939_pdu *pdu,
	void *pdbv_info, void *pitem_info, void *psend_info)
{
	j1939_dbv_info *pinfo = (j1939_dbv_info *) pdbv_info;
	j1939_send_info *sinfo = (j1939_send_info *) psend_info; 
	j1939_send_item *pitem = (j1939_send_item *) pitem_info; 
	gyro_typ gyro;
	db_data_typ db_data;
	unsigned int gyro_scaled;

	if (clt_read(pclt, DB_GYRO_VAR, DB_GYRO_TYPE, &db_data) == FALSE) {
                fprintf(stderr, "clt_read(DB_GYRO_VAR)\n");
	}
	gyro = *((gyro_typ *) db_data.value.user);
	gyro_scaled = (gyro.gyro_rate + 1.0) * 100000;
	pdu->data_field[0] = BYTE0(gyro_scaled);
	pdu->data_field[1] = BYTE1(gyro_scaled);
	pdu->data_field[2] = BYTE2(gyro_scaled);
	pdu->data_field[3] = BYTE3(gyro_scaled);
	pdu->data_field[4] = 0xff;
	pdu->data_field[5] = 0xff;
	pdu->data_field[6] = 0xff;
	pdu->data_field[7] = 0xff;
	set_pdu_header(pdu, pinfo, pitem, sinfo);
}

void print_lai_gyro(void *pdbv , FILE  *fp, int numeric)
{
	lai_gyro_typ *gyro = (lai_gyro_typ *)pdbv;
	fprintf(fp, "LAI GYRO: ");
	print_timestamp(fp, &gyro->timestamp);
	if (numeric) {
		fprintf(fp, " %.5f ", gyro->gyro_rate);
		fprintf(fp, "\n");	
	} else 
		fprintf(fp, " Yaw rate %.4f\n", gyro->gyro_rate);
}

/* LAI_DVIMON
 */
void pdu_to_lai_dvimon(struct j1939_pdu *pdu, void *pdbv)
{
	lai_dvimon_typ *dvimon = (lai_dvimon_typ *)pdbv;

	dvimon->mode = pdu->data_field[0]; 
}

void lai_dvimon_to_pdu(db_clt_typ *pclt, struct j1939_pdu *pdu,
	void *pdbv_info, void *pitem_info, void *psend_info)
{
	j1939_dbv_info *pinfo = (j1939_dbv_info *) pdbv_info;
	j1939_send_info *sinfo = (j1939_send_info *) psend_info; 
	j1939_send_item *pitem = (j1939_send_item *) pitem_info; 
	dvi_monitor_typ dvi_mon;
	db_data_typ db_data;

	if (clt_read(pclt, DB_DVI_MONITOR_VAR, DB_DVI_MONITOR_TYPE, &db_data) == FALSE) {
                fprintf(stderr, "clt_read(DB_DVI_MONITOR_VAR)\n");
	}
	dvi_mon = *((dvi_monitor_typ *) db_data.value.user);
	pdu->data_field[0] = dvi_mon.mode;
	pdu->data_field[1] = 0xff;
	pdu->data_field[2] = 0xff;
	pdu->data_field[3] = 0xff;
	pdu->data_field[4] = 0xff;
	pdu->data_field[5] = 0xff;
	pdu->data_field[6] = 0xff;
	pdu->data_field[7] = 0xff;
	set_pdu_header(pdu, pinfo, pitem, sinfo);
}

void print_lai_dvimon(void *pdbv , FILE  *fp, int numeric)
{
	lai_dvimon_typ *dvimon = (lai_dvimon_typ *)pdbv;
	fprintf(fp, "LAI DVIMON: ");
	print_timestamp(fp, &dvimon->timestamp);
	if (numeric) {
		fprintf(fp, " %d ", dvimon->mode);
		fprintf(fp, "\n");	
	} else 
		fprintf(fp, " Mode %d\n", dvimon->mode);
}

/* LAI_LATSENS
 */
void pdu_to_lai_latsens(struct j1939_pdu *pdu, void *pdbv)
{
	lai_latsens_typ *latsens = (lai_latsens_typ *)pdbv;
        unsigned int two_bytes1, two_bytes2;

        two_bytes1 = TWOBYTES(pdu->data_field[1], pdu->data_field[0]);
        two_bytes2 = TWOBYTES(pdu->data_field[3], pdu->data_field[2]);

	latsens->lat_acc = scale_100_to_volt(two_bytes1);
	latsens->long_acc = scale_100_to_volt(two_bytes2);
	latsens->manual_trans = pdu->data_field[4] & 1; 
	latsens->auto_trans = (pdu->data_field[4] & (1 << 1)) >> 1; 
	latsens->auto_steer = (pdu->data_field[4] & (1 << 2)) >> 2; 
	latsens->auto_throt = (pdu->data_field[4] & (1 << 3)) >> 3; 
	latsens->auto_brake = (pdu->data_field[4] & (1 << 4)) >> 4; 
	latsens->dc1 = (pdu->data_field[4] & (1 << 5)) >> 5; 
	latsens->dc2 = (pdu->data_field[4] & (1 << 6)) >> 6; 
}

void lai_latsens_to_pdu(db_clt_typ *pclt, struct j1939_pdu *pdu,
	void *pdbv_info, void *pitem_info, void *psend_info)
{
	j1939_dbv_info *pinfo = (j1939_dbv_info *) pdbv_info;
	j1939_send_info *sinfo = (j1939_send_info *) psend_info; 
	j1939_send_item *pitem = (j1939_send_item *) pitem_info; 
	lat_input_sensors_typ lat_sens;
	db_data_typ db_data;
	unsigned int latacc_scaled, longacc_scaled;

	if (clt_read(pclt, DB_LAT_INPUT_SENSORS_VAR, DB_LAT_INPUT_SENSORS_TYPE, 
		&db_data) == FALSE) {
                fprintf(stderr, "clt_read(DB_LAT_INPUT_SENSORS_VAR)\n");
	}
	lat_sens = *((lat_input_sensors_typ *) db_data.value.user);
	latacc_scaled = lat_sens.lat_acc * 100;
	longacc_scaled = lat_sens.long_accel * 100;
	pdu->data_field[0] = BYTE0(latacc_scaled);
	pdu->data_field[1] = BYTE1(latacc_scaled);
	pdu->data_field[2] = BYTE0(longacc_scaled);
	pdu->data_field[3] = BYTE1(longacc_scaled);
	pdu->data_field[4] = lat_sens.auto_steer | lat_sens.manual_trans<<1
			   | lat_sens.auto_trans<<2 | lat_sens.auto_throt<<3
			   | lat_sens.auto_brake<<4 | lat_sens.dc1<<5 | lat_sens.dc2<<6;
	pdu->data_field[5] = 0xff;
	pdu->data_field[6] = 0xff;
	pdu->data_field[7] = 0xff;
	set_pdu_header(pdu, pinfo, pitem, sinfo);
}

void print_lai_latsens(void *pdbv , FILE  *fp, int numeric)
{
	lai_latsens_typ *latsens = (lai_latsens_typ *)pdbv;
	fprintf(fp, "LAI LATSENS: ");
	print_timestamp(fp, &latsens->timestamp);
	if (numeric) {
		fprintf(fp, " %.2f ", latsens->lat_acc);
		fprintf(fp, "%.2f ", latsens->long_acc);
		fprintf(fp, "%d ", latsens->manual_trans);
		fprintf(fp, "%d ", latsens->auto_trans);
		fprintf(fp, "%d ", latsens->auto_steer);
		fprintf(fp, "%d ", latsens->auto_throt);
		fprintf(fp, "%d ", latsens->auto_brake);
		fprintf(fp, "%d ", latsens->dc1);
		fprintf(fp, "%d ", latsens->dc2);
		fprintf(fp, "\n");	
	} else { 
		fprintf(fp, " Lateral acceleration %.2f\n", latsens->lat_acc);
		fprintf(fp, "Longitudinaal acceleration %.2f\n", latsens->long_acc);
		fprintf(fp, "Manual trans %d ", latsens->manual_trans);
		fprintf(fp, "Auto trans %d ", latsens->auto_trans);
		fprintf(fp, "Auto steer %d ", latsens->auto_steer);
		fprintf(fp, "Auto throt %d ", latsens->auto_throt);
		fprintf(fp, "Auto brake %d ", latsens->auto_brake);
		fprintf(fp, "DC1 %d ", latsens->dc1);
		fprintf(fp, "DC2 %d ", latsens->dc2);
	}
}

/* LAI_LONGIN1
 */
void pdu_to_lai_longin1(struct j1939_pdu *pdu, void *pdbv)
{
	lai_longin1_typ *longin1 = (lai_longin1_typ *)pdbv;
        unsigned int two_bytes1, two_bytes2, two_bytes3;

        two_bytes1 = TWOBYTES(pdu->data_field[1], pdu->data_field[0]);
        two_bytes2 = TWOBYTES(pdu->data_field[3], pdu->data_field[2]);
        two_bytes3 = TWOBYTES(pdu->data_field[5], pdu->data_field[4]);

	longin1->acc_pedal = scale_100_to_volt(two_bytes1);
	longin1->fb_axle = scale_100_to_volt(two_bytes2);
	longin1->rb_axle = scale_100_to_volt(two_bytes3);
#ifdef DO_TRACE
printf("pdu_to_lai_longin1: %d %.2f %d %.2f %d %.2f\n", two_bytes1, longin1->acc_pedal,
	two_bytes2, longin1->fb_axle, two_bytes3, longin1->rb_axle);
#endif
}

void lai_longin1_to_pdu(db_clt_typ *pclt, struct j1939_pdu *pdu,
	void *pdbv_info, void *pitem_info, void *psend_info)
{
	j1939_dbv_info *pinfo = (j1939_dbv_info *) pdbv_info;
	j1939_send_info *sinfo = (j1939_send_info *) psend_info; 
	j1939_send_item *pitem = (j1939_send_item *) pitem_info; 
	long_input_typ long_in;
	db_data_typ db_data;
	unsigned int acc_scaled, fb_axle_scaled, rb_axle_scaled;

	if (clt_read(pclt, DB_LONG_INPUT_VAR, DB_LONG_INPUT_TYPE, 
		&db_data) == FALSE) {
                fprintf(stderr, "clt_read(DB_LONG_INPUT_VAR)\n");
	}
	long_in = *((long_input_typ *) db_data.value.user);
	acc_scaled = long_in.acc_pedal * 100;
	fb_axle_scaled = long_in.fb_axle * 100;
	rb_axle_scaled = long_in.rb_axle * 100;
	pdu->data_field[0] = BYTE0(acc_scaled);
	pdu->data_field[1] = BYTE1(acc_scaled);
	pdu->data_field[2] = BYTE0(fb_axle_scaled);
	pdu->data_field[3] = BYTE1(fb_axle_scaled);
	pdu->data_field[4] = BYTE0(rb_axle_scaled);
	pdu->data_field[5] = BYTE1(rb_axle_scaled);
	pdu->data_field[6] = 0xff;
	pdu->data_field[7] = 0xff;
	set_pdu_header(pdu, pinfo, pitem, sinfo);
}

void print_lai_longin1(void *pdbv , FILE  *fp, int numeric)
{
	lai_longin1_typ *longin1 = (lai_longin1_typ *)pdbv;
	fprintf(fp, "LAI LONGIN1: ");
	print_timestamp(fp, &longin1->timestamp);
	if (numeric) {
		fprintf(fp, " %.2f ", longin1->acc_pedal);
		fprintf(fp, "%.2f ", longin1->fb_axle);
		fprintf(fp, "%.2f ", longin1->rb_axle);
		fprintf(fp, "\n");	
	} else { 
		fprintf(fp, " Accelerator pedal %.2f\n", longin1->acc_pedal);
		fprintf(fp, "Front brake axle pressure %.2f\n", longin1->fb_axle);
		fprintf(fp, "Rear brake axle pressure %.2f\n", longin1->rb_axle);
	}
}

/* LAI_LONGIN2
 */
void pdu_to_lai_longin2(struct j1939_pdu *pdu, void *pdbv)
{
	lai_longin2_typ *longin2 = (lai_longin2_typ *)pdbv;
        unsigned int two_bytes1, two_bytes2, two_bytes3, two_bytes4;

        two_bytes1 = TWOBYTES(pdu->data_field[1], pdu->data_field[0]);
        two_bytes2 = TWOBYTES(pdu->data_field[3], pdu->data_field[2]);
        two_bytes3 = TWOBYTES(pdu->data_field[5], pdu->data_field[4]);
        two_bytes4 = TWOBYTES(pdu->data_field[7], pdu->data_field[6]);

	longin2->fb_applied = scale_100_to_volt(two_bytes1);
	longin2->rb_applied = scale_100_to_volt(two_bytes2);
	longin2->fb_monitor = scale_100_to_volt(two_bytes3);
	longin2->rb_monitor = scale_100_to_volt(two_bytes4);
#ifdef DO_TRACE
printf("pdu_to_lai_longin2: %d %.2f %d %.2f %d %.2f\n", two_bytes1, 
	longin2->fb_applied, two_bytes2, longin2->rb_applied, two_bytes3,
	longin2->fb_monitor, two_bytes4, longin2->rb_monitor); 
#endif
}

void lai_longin2_to_pdu(db_clt_typ *pclt, struct j1939_pdu *pdu,
	void *pdbv_info, void *pitem_info, void *psend_info)
{
	j1939_dbv_info *pinfo = (j1939_dbv_info *) pdbv_info;
	j1939_send_info *sinfo = (j1939_send_info *) psend_info; 
	j1939_send_item *pitem = (j1939_send_item *) pitem_info; 
	long_input_typ long_in;
	db_data_typ db_data;
	unsigned int fb_applied_scaled, rb_applied_scaled;
	unsigned int fb_monitor_scaled, rb_monitor_scaled;

	if (clt_read(pclt, DB_LONG_INPUT_VAR, DB_LONG_INPUT_TYPE, 
		&db_data) == FALSE) {
                fprintf(stderr, "clt_read(DB_LONG_INPUT_VAR)\n");
	}
	long_in = *((long_input_typ *) db_data.value.user);
	fb_applied_scaled = long_in.fb_applied * 100;
	rb_applied_scaled = long_in.rb_applied * 100;
	fb_monitor_scaled = long_in.fb_monitor * 100;
	rb_monitor_scaled = long_in.rb_monitor * 100;
	pdu->data_field[0] = BYTE0(fb_applied_scaled);
	pdu->data_field[1] = BYTE1(fb_applied_scaled);
	pdu->data_field[2] = BYTE0(rb_applied_scaled);
	pdu->data_field[3] = BYTE1(rb_applied_scaled);
	pdu->data_field[4] = BYTE0(fb_monitor_scaled);
	pdu->data_field[5] = BYTE1(fb_monitor_scaled);
	pdu->data_field[6] = BYTE0(rb_monitor_scaled);
	pdu->data_field[7] = BYTE1(rb_monitor_scaled);
	set_pdu_header(pdu, pinfo, pitem, sinfo);
}

void print_lai_longin2(void *pdbv , FILE  *fp, int numeric)
{
	lai_longin2_typ *longin2 = (lai_longin2_typ *)pdbv;
	fprintf(fp, "LAI LONGIN2: ");
	print_timestamp(fp, &longin2->timestamp);
	if (numeric) {
		fprintf(fp, " %.2f ", longin2->fb_applied);
		fprintf(fp, "%.2f ", longin2->rb_applied);
		fprintf(fp, "%.2f ", longin2->fb_monitor);
		fprintf(fp, "%.2f ", longin2->rb_monitor);
		fprintf(fp, "\n");	
	} else { 
		fprintf(fp, " Front brake applied pressure %.2f\n", longin2->fb_applied);
		fprintf(fp, "Rear brake applied pressure %.2f\n", longin2->rb_applied);
		fprintf(fp, "Front brake monitor pressure %.2f\n", longin2->fb_monitor);
		fprintf(fp, "Rear brake monitor pressure %.2f\n", longin2->rb_monitor);
	}
}

/* LAI_DVISTAT
 */
void pdu_to_lai_dvistat(struct j1939_pdu *pdu, void *pdbv)
{
	lai_dvistat_typ *dvistat = (lai_dvistat_typ *)pdbv;

	/* substract 6 from steer_fault, no need to mask because in the lower bits */
	dvistat->sf_mag_id = pdu->data_field[0] - 6; 
	dvistat->overall_mode = pdu->data_field[1]; 
	dvistat->platoon_info = pdu->data_field[2]; 
	dvistat->lat_status1 = pdu->data_field[3]; 
	dvistat->lat_status2 = pdu->data_field[4]; 
	dvistat->lat_status3 = pdu->data_field[5]; 
	dvistat->lat_status4 = pdu->data_field[6]; 
	dvistat->red = (pdu->data_field[7] & 1); 
	dvistat->green = (pdu->data_field[7] & (1 << 1)) >> 1; 
	dvistat->blue = (pdu->data_field[7] & (1 << 2)) >> 2; 
	dvistat->amber = (pdu->data_field[7] & (1 << 3)) >> 3; 
}

void lai_dvistat_to_pdu(db_clt_typ *pclt, struct j1939_pdu *pdu,
	void *pdbv_info, void *pitem_info, void *psend_info)
{
	j1939_dbv_info *pinfo = (j1939_dbv_info *) pdbv_info;
	j1939_send_info *sinfo = (j1939_send_info *) psend_info; 
	j1939_send_item *pitem = (j1939_send_item *) pitem_info; 
	lat_dvi_output_typ dvi_out;
	dvi_leds_typ dvi_leds;
	db_data_typ db_data1, db_data2;
	unsigned int steer_fault_scaled;

	if (clt_read(pclt, DB_LAT_DVI_OUTPUT_VAR, DB_LAT_DVI_OUTPUT_TYPE, 
		&db_data1) == FALSE) {
                fprintf(stderr, "clt_read(DB_LAT_DVI_OUTPUT_VAR)\n");
	}
	dvi_out = *((lat_dvi_output_typ *) db_data1.value.user);

	steer_fault_scaled = dvi_out.steer_fault + 6;
//	pdu->data_field[0] = dvi_out.mag_status | dvi_out.lane_id<<4
//			   | dvi_out.steer_fault<<6;
	/* bits 0-3: steer_fault, bits 4-5: mag_status, bits 6-7: lane_id */
	pdu->data_field[0] = steer_fault_scaled | dvi_out.mag_status <<4 
			   | dvi_out.lane_id<<6;
//printf("lai_dvi_to_pdu: %d %d %d => %d\n",dvi_out.mag_status, dvi_out.lane_id,
//	steer_fault_scaled, pdu->data_field[0]);

	pdu->data_field[1] = dvi_out.overall_mode;
	pdu->data_field[2] = dvi_out.platoon_info;
	pdu->data_field[3] = dvi_out.lat_status1;
	pdu->data_field[4] = dvi_out.lat_status2;
	pdu->data_field[5] = dvi_out.lat_status3;
	pdu->data_field[6] = dvi_out.lat_status4;

	if (clt_read(pclt, DB_DVI_LEDS_VAR, DB_DVI_LEDS_TYPE, 
		&db_data2) == FALSE) {
                fprintf(stderr, "clt_read(DB_DVI_LEDS_VAR)\n");
	}
	dvi_leds = *((dvi_leds_typ *) db_data2.value.user);
	pdu->data_field[7] = dvi_leds.red | dvi_leds.green<<1
			   | dvi_leds.blue<<2 | dvi_leds.amber<<3;
	set_pdu_header(pdu, pinfo, pitem, sinfo);
}

void print_lai_dvistat(void *pdbv , FILE  *fp, int numeric)
{
	lai_dvistat_typ *dvistat = (lai_dvistat_typ *)pdbv;

	fprintf(fp, "LAI DVISTAT: ");
	print_timestamp(fp, &dvistat->timestamp);
	if (numeric) {
		fprintf(fp, " %d ", dvistat->sf_mag_id);
		fprintf(fp, "%d ", dvistat->overall_mode);
		fprintf(fp, "%d ", dvistat->platoon_info);
		fprintf(fp, "%d ", dvistat->lat_status1);
		fprintf(fp, "%d ", dvistat->lat_status2);
		fprintf(fp, "%d ", dvistat->lat_status3);
		fprintf(fp, "%d ", dvistat->lat_status4);
		fprintf(fp, "%d ", dvistat->red);
		fprintf(fp, "%d ", dvistat->green);
		fprintf(fp, "%d ", dvistat->blue);
		fprintf(fp, "%d ", dvistat->amber);
		fprintf(fp, "\n");	
	} else { 
		fprintf(fp, " Mag sf ID %d\n", dvistat->sf_mag_id);
		fprintf(fp, "Overall DVI mode %d\n", dvistat->overall_mode);
		fprintf(fp, "Platoon info %d\n", dvistat->platoon_info);
		fprintf(fp, "Lateral status 1 %d\n", dvistat->lat_status1);
		fprintf(fp, "Lateral status 2 %d\n", dvistat->lat_status2);
		fprintf(fp, "Lateral status 3 %d\n", dvistat->lat_status3);
		fprintf(fp, "Lateral status 4 %d\n", dvistat->lat_status4);
		fprintf(fp, "%d ", dvistat->red);
		fprintf(fp, "%d ", dvistat->green);
		fprintf(fp, "%d ", dvistat->blue);
		fprintf(fp, "%d ", dvistat->amber);
	}
}

/* LAI_DVISND
 */
void pdu_to_lai_dvisnd(struct j1939_pdu *pdu, void *pdbv)
{
	lai_dvisnd_typ *dvisnd = (lai_dvisnd_typ *)pdbv;
        unsigned int two_bytes1, two_bytes2, two_bytes3, two_bytes4;

	two_bytes1 = TWOBYTES(pdu->data_field[1], pdu->data_field[0]);
	two_bytes2 = TWOBYTES(pdu->data_field[3], pdu->data_field[2]);
	two_bytes3 = TWOBYTES(pdu->data_field[5], pdu->data_field[4]);
	two_bytes4 = TWOBYTES(pdu->data_field[7], pdu->data_field[6]);

	dvisnd->sound1 = scale_100_to_volt(two_bytes1); 
	dvisnd->sound2 = scale_100_to_volt(two_bytes2); 
	dvisnd->sound3 = scale_100_to_volt(two_bytes3); 
	dvisnd->sound4 = scale_100_to_volt(two_bytes4); 
}

void lai_dvisnd_to_pdu(db_clt_typ *pclt, struct j1939_pdu *pdu,
	void *pdbv_info, void *pitem_info, void *psend_info)
{
	j1939_dbv_info *pinfo = (j1939_dbv_info *) pdbv_info;
	j1939_send_info *sinfo = (j1939_send_info *) psend_info; 
	j1939_send_item *pitem = (j1939_send_item *) pitem_info; 
	dvi_leds_typ dvi_leds;
	db_data_typ db_data;
	unsigned int sound1_scaled, sound2_scaled, sound3_scaled, sound4_scaled;

	if (clt_read(pclt, DB_DVI_LEDS_VAR, DB_DVI_LEDS_TYPE, 
		&db_data) == FALSE) {
                fprintf(stderr, "clt_read(DB_DVI_LEDS_VAR)\n");
	}
	dvi_leds = *((dvi_leds_typ *) db_data.value.user);
	sound1_scaled = dvi_leds.sound1 * 100;
	sound2_scaled = dvi_leds.sound2 * 100;
	sound3_scaled = dvi_leds.sound3 * 100;
	sound4_scaled = dvi_leds.sound4 * 100;
	pdu->data_field[0] = BYTE0(sound1_scaled);
	pdu->data_field[1] = BYTE1(sound1_scaled);
	pdu->data_field[2] = BYTE0(sound2_scaled);
	pdu->data_field[3] = BYTE1(sound2_scaled);
	pdu->data_field[4] = BYTE0(sound3_scaled);
	pdu->data_field[5] = BYTE1(sound3_scaled);
	pdu->data_field[6] = BYTE0(sound4_scaled);
	pdu->data_field[7] = BYTE1(sound4_scaled);
	set_pdu_header(pdu, pinfo, pitem, sinfo);
}

void print_lai_dvisnd(void *pdbv , FILE  *fp, int numeric)
{
	lai_dvisnd_typ *dvisnd = (lai_dvisnd_typ *)pdbv;

	fprintf(fp, "LAI DVISND: ");
	print_timestamp(fp, &dvisnd->timestamp);
	if (numeric) {
		fprintf(fp, " %.2f ", dvisnd->sound1);
		fprintf(fp, "%.2f ", dvisnd->sound2);
		fprintf(fp, "%.2f ", dvisnd->sound3);
		fprintf(fp, "%.2f ", dvisnd->sound4);
		fprintf(fp, "\n");	
	} else { 
		fprintf(fp, " Sound1 %.2f\n", dvisnd->sound1);
		fprintf(fp, "Sound2 %.2f\n", dvisnd->sound2);
		fprintf(fp, "Sound3 %.2f\n", dvisnd->sound3);
		fprintf(fp, "Sound4 %.2f\n", dvisnd->sound4);
	}
}

/* LAI_DVIPOS
 */
void pdu_to_lai_dvipos(struct j1939_pdu *pdu, void *pdbv)
{
	lai_dvipos_typ *dvipos = (lai_dvipos_typ *)pdbv;
        unsigned int two_bytes1, two_bytes2, two_bytes3, two_bytes4;

	two_bytes1 = TWOBYTES(pdu->data_field[1], pdu->data_field[0]);
	two_bytes2 = TWOBYTES(pdu->data_field[3], pdu->data_field[2]);
	two_bytes3 = TWOBYTES(pdu->data_field[5], pdu->data_field[4]);
	two_bytes4 = TWOBYTES(pdu->data_field[7], pdu->data_field[6]);

	dvipos->lat_pos = scale_m200_to_p200(two_bytes1); 
	dvipos->long_pos = two_bytes2; 
	dvipos->distance_end = two_bytes3; 
	dvipos->lat_est = scale_m720_to_p720(two_bytes4); 
#ifdef DO_TRACE 
printf("pdu_to_lai_dvipos: %d %d %d %d %d %d %d %d\n", two_bytes1, 
	dvipos->lat_pos, two_bytes2, dvipos->long_pos, two_bytes3,
	dvipos->distance_end, two_bytes4, dvipos->lat_est); 
#endif
}

void lai_dvipos_to_pdu(db_clt_typ *pclt, struct j1939_pdu *pdu,
	void *pdbv_info, void *pitem_info, void *psend_info)
{
	j1939_dbv_info *pinfo = (j1939_dbv_info *) pdbv_info;
	j1939_send_info *sinfo = (j1939_send_info *) psend_info; 
	j1939_send_item *pitem = (j1939_send_item *) pitem_info; 
	lat_dvi_output_typ dvi_out;
	db_data_typ db_data;
	unsigned int lat_pos_scaled, lat_est_scaled;

	if (clt_read(pclt, DB_LAT_DVI_OUTPUT_VAR, DB_LAT_DVI_OUTPUT_TYPE, 
		&db_data) == FALSE) {
                fprintf(stderr, "clt_read(DB_LAT_DVI_OUTPUT_VAR)\n");
	}
	dvi_out = *((lat_dvi_output_typ *) db_data.value.user);
	lat_pos_scaled = dvi_out.lat_pos + 200; 
	lat_est_scaled = dvi_out.lat_est + 720; 
	pdu->data_field[0] = BYTE0(lat_pos_scaled);
	pdu->data_field[1] = BYTE1(lat_pos_scaled);
	pdu->data_field[2] = BYTE0(dvi_out.long_pos);
	pdu->data_field[3] = BYTE1(dvi_out.long_pos);
	pdu->data_field[4] = BYTE0(dvi_out.distance_to_end);
	pdu->data_field[5] = BYTE1(dvi_out.distance_to_end);
	pdu->data_field[6] = BYTE0(lat_est_scaled);
	pdu->data_field[7] = BYTE1(lat_est_scaled);
	set_pdu_header(pdu, pinfo, pitem, sinfo);
}

void print_lai_dvipos(void *pdbv , FILE  *fp, int numeric)
{
	lai_dvipos_typ *dvipos = (lai_dvipos_typ *)pdbv;

	fprintf(fp, "LAI DVIPOS: ");
	print_timestamp(fp, &dvipos->timestamp);
	if (numeric) {
		fprintf(fp, " %d ", dvipos->lat_pos);
		fprintf(fp, "%d ", dvipos->long_pos);
		fprintf(fp, "%d ", dvipos->distance_end);
		fprintf(fp, "%d ", dvipos->lat_est);
		fprintf(fp, "\n");	
	} else { 
		fprintf(fp, " Lateral position offset %d\n", dvipos->lat_pos);
		fprintf(fp, "Longitudinal position %d\n", dvipos->long_pos);
		fprintf(fp, "Distance to end of magnets %d\n", dvipos->distance_end);
		fprintf(fp, "Lateral estimation in lane change %d\n", dvipos->lat_est);
	}
}

/* LAI_CTRLOUT
 */
void pdu_to_lai_ctrlout(struct j1939_pdu *pdu, void *pdbv)
{
	lai_ctrlout_typ *ctrlout = (lai_ctrlout_typ *)pdbv;
        unsigned int four_bytes;

        four_bytes = FOURBYTES(pdu->data_field[3], pdu->data_field[2],
                        pdu->data_field[1], pdu->data_field[0]);

	ctrlout->steer_ctrl = scale_m1440_to_p1440(four_bytes);
#ifdef DO_TRACE
	printf("pdu_to_lai_ctrlout: %d %.3f\n", four_bytes, ctrlout->steer_ctrl);
#endif
	ctrlout->actuator_mode = pdu->data_field[4]; 
	ctrlout->steer_mode = pdu->data_field[5]; 
}

void lai_ctrlout_to_pdu(db_clt_typ *pclt, struct j1939_pdu *pdu,
	void *pdbv_info, void *pitem_info, void *psend_info)
{
	j1939_dbv_info *pinfo = (j1939_dbv_info *) pdbv_info;
	j1939_send_info *sinfo = (j1939_send_info *) psend_info; 
	j1939_send_item *pitem = (j1939_send_item *) pitem_info; 
	lat_control_output_typ lat_ctrl;
	unsigned int steer_scaled;
	db_data_typ db_data;

	if (clt_read(pclt, DB_LAT_CONTROL_OUTPUT_VAR, DB_LAT_CONTROL_OUTPUT_TYPE,
                           &db_data) == FALSE) {
                fprintf(stderr, "clt_read(DB_LAT_CONTROL_OUTPUT_VAR)\n");
	}
	lat_ctrl = *((lat_control_output_typ *) db_data.value.user);
	steer_scaled = (lat_ctrl.steer_ctrl+1440.0) * 1000;
#ifdef DO_TRACE
printf("lai_ctrlout_to_pdu: steer_scaled %d\n", steer_scaled);
#endif
	pdu->data_field[0] = BYTE0(steer_scaled);
	pdu->data_field[1] = BYTE1(steer_scaled);
	pdu->data_field[2] = BYTE2(steer_scaled);
	pdu->data_field[3] = BYTE3(steer_scaled);
	pdu->data_field[4] = lat_ctrl.actuator_mode;
	pdu->data_field[5] = lat_ctrl.steer_mode;
	pdu->data_field[6] = 0xff;
	pdu->data_field[7] = 0xff;
	set_pdu_header(pdu, pinfo, pitem, sinfo);
}

void print_lai_ctrlout(void *pdbv , FILE  *fp, int numeric)
{
	lai_ctrlout_typ *ctrlout = (lai_ctrlout_typ *)pdbv;
	fprintf(fp, "LAI CTRLOUT: ");
	print_timestamp(fp, &ctrlout->timestamp);
	if (numeric) {
		fprintf(fp, " %.3f ", ctrlout->steer_ctrl);
		fprintf(fp, "%d ", ctrlout->actuator_mode);
	 	fprintf(fp, "%d ", ctrlout->steer_mode);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp, " Steering control %.3f\n", ctrlout->steer_ctrl);
		fprintf(fp, "Steering actuator mode %d\n", ctrlout->actuator_mode);
		fprintf(fp, "Steering mode %d\n", ctrlout->steer_mode);
	}
}

/* LAI_LATHEART
 */
void pdu_to_lai_latheart(struct j1939_pdu *pdu, void *pdbv)
{
	lai_latheart_typ *latheart = (lai_latheart_typ *)pdbv;

	latheart->is_active = pdu->data_field[0]; 
}

void lai_latheart_to_pdu(db_clt_typ *pclt, struct j1939_pdu *pdu,
	void *pdbv_info, void *pitem_info, void *psend_info)
{
	j1939_dbv_info *pinfo = (j1939_dbv_info *) pdbv_info;
	j1939_send_info *sinfo = (j1939_send_info *) psend_info; 
	j1939_send_item *pitem = (j1939_send_item *) pitem_info; 
	lat_heartbeat_output_typ lat_heart;
	db_data_typ db_data;

	if (clt_read(pclt, DB_LAT_HEARTBEAT_OUTPUT_VAR, DB_LAT_HEARTBEAT_OUTPUT_TYPE,
                           &db_data) == FALSE) {
                fprintf(stderr, "clt_read(DB_LAT_HEARTBEAT_OUTPUT_VAR)\n");
	}
	lat_heart = *((lat_heartbeat_output_typ *) db_data.value.user);
	pdu->data_field[0] = lat_heart.heartbeat;
	pdu->data_field[1] = 0xff;
	pdu->data_field[2] = 0xff;
	pdu->data_field[3] = 0xff;
	pdu->data_field[4] = 0xff;
	pdu->data_field[5] = 0xff;
	pdu->data_field[6] = 0xff;
	pdu->data_field[7] = 0xff;
	set_pdu_header(pdu, pinfo, pitem, sinfo);
}

void print_lai_latheart(void *pdbv , FILE  *fp, int numeric)
{
	lai_latheart_typ *latheart = (lai_latheart_typ *)pdbv;
	fprintf(fp, "LAI LATHEART: ");
	print_timestamp(fp, &latheart->timestamp);
	if (numeric) {
		fprintf(fp, " %d ", latheart->is_active);
		fprintf(fp, "\n");	
	} else 
		fprintf(fp, " Lateral heartbeat %d\n", latheart->is_active);
}

/* LAI_LONGOUT
 */
void pdu_to_lai_longout(struct j1939_pdu *pdu, void *pdbv)
{
	lai_longout_typ *longout = (lai_longout_typ *)pdbv;
        unsigned int two_bytes1, two_bytes2, two_bytes3;

	two_bytes1 = TWOBYTES(pdu->data_field[1], pdu->data_field[0]);
	two_bytes2 = TWOBYTES(pdu->data_field[3], pdu->data_field[2]);
	two_bytes3 = TWOBYTES(pdu->data_field[5], pdu->data_field[4]);

	longout->acc_pedal_ctrl = scale_100_to_volt(two_bytes1);
	longout->fb_ctrl = scale_100_to_volt(two_bytes2);
	longout->rb_ctrl = scale_100_to_volt(two_bytes3);
}

void lai_longout_to_pdu(db_clt_typ *pclt, struct j1939_pdu *pdu,
	void *pdbv_info, void *pitem_info, void *psend_info)
{
	j1939_dbv_info *pinfo = (j1939_dbv_info *) pdbv_info;
	j1939_send_info *sinfo = (j1939_send_info *) psend_info; 
	j1939_send_item *pitem = (j1939_send_item *) pitem_info; 
	long_output_typ long_out;
	db_data_typ db_data;
	unsigned int acc_scaled, fb_ctrl_scaled, rb_ctrl_scaled;

	if (clt_read(pclt, DB_LONG_OUTPUT_VAR, DB_LONG_OUTPUT_TYPE,
                           &db_data) == FALSE) {
                fprintf(stderr, "clt_read(DB_LONG_OUTPUT_VAR)\n");
	}
	long_out = *((long_output_typ *) db_data.value.user);
	acc_scaled = long_out.acc_pedal_control * 100;
	fb_ctrl_scaled = long_out.fb_control * 100;
	rb_ctrl_scaled = long_out.rb_control * 100;
	pdu->data_field[0] = BYTE0(acc_scaled);
	pdu->data_field[1] = BYTE1(acc_scaled);
	pdu->data_field[2] = BYTE0(fb_ctrl_scaled);
	pdu->data_field[3] = BYTE1(fb_ctrl_scaled);
	pdu->data_field[4] = BYTE0(rb_ctrl_scaled);
	pdu->data_field[5] = BYTE1(rb_ctrl_scaled);
	pdu->data_field[6] = 0xff;
	pdu->data_field[7] = 0xff;
	set_pdu_header(pdu, pinfo, pitem, sinfo);
}

void print_lai_longout(void *pdbv , FILE  *fp, int numeric)
{
	lai_longout_typ *longout = (lai_longout_typ *)pdbv;
	fprintf(fp, "LAI LONGOUT: ");
	print_timestamp(fp, &longout->timestamp);
	if (numeric) {
		fprintf(fp, " %.2f ", longout->acc_pedal_ctrl);
		fprintf(fp, "%.2f ", longout->fb_ctrl);
		fprintf(fp, "%.2f ", longout->rb_ctrl);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp, " Accelerator pedal command %.2f\n", longout->acc_pedal_ctrl);
		fprintf(fp, "Front brake command %.2f\n", longout->fb_ctrl);
		fprintf(fp, "Rear brake command %.2f\n", longout->rb_ctrl);
	}
}
