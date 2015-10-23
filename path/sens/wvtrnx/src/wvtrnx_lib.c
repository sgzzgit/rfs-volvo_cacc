/**\file wvtrnx_lib.c
 *
 * Copyright (c) 2009 Regents of the University of California
 *
 * Process to check Wavetronix messages and put the values in the database.
 *
 */


#include <sys_os.h>	/// In path/local, OS-dependent definitions
#include <sys_rt.h>	/// In path/local, "real-time" definitions
#include <timestamp.h>	/// In path/local, for using hh:mm:ss.sss timestamps
#include "wvtrnx.h"


void print_wvtrnx(wvtrnx_msg_t w_msg){

	int i;

	print_timestamp(stdout, &w_msg.ts);

	for(i =0; i < WVTRNX_MAX_TRACKS; i++){
		
		printf("%d ", WVTRNX_STATUS(w_msg.tracks[i].status, 
					WVTRNX_STATUS_APPROACHING_SENSOR_MASK));
		printf("%d ", WVTRNX_STATUS(w_msg.tracks[i].status, 
					WVTRNX_STATUS_CORRECT_DIRECTION_MASK));
		printf("%d ", WVTRNX_STATUS(w_msg.tracks[i].status, 
					WVTRNX_STATUS_READY_TO_READ_MASK));
		printf("%d ", WVTRNX_STATUS(w_msg.tracks[i].status, 
					WVTRNX_STATUS_NEWLY_DISCOVERED_MASK));
		printf("%d ", WVTRNX_STATUS(w_msg.tracks[i].status, 
					WVTRNX_STATUS_ACTIVE_MASK)); 

		printf("%.2f ", WVTRNX_CVT_DISTANCE(w_msg.tracks[i].distance));
		printf("%.2f ", WVTRNX_CVT_SPEED(w_msg.tracks[i].speed));
				
	}
	printf("\n");
}

void fprint_wvtrnx(wvtrnx_msg_t w_msg, FILE *fo){

	int i;

	print_timestamp(stdout, &w_msg.ts);

	for(i =0; i < WVTRNX_MAX_TRACKS; i++){
		
		fprintf(fo, "%d ", WVTRNX_STATUS(w_msg.tracks[i].status, 
					WVTRNX_STATUS_APPROACHING_SENSOR_MASK));
		fprintf(fo, "%d ", WVTRNX_STATUS(w_msg.tracks[i].status, 
					WVTRNX_STATUS_CORRECT_DIRECTION_MASK));
		fprintf(fo, "%d ", WVTRNX_STATUS(w_msg.tracks[i].status, 
					WVTRNX_STATUS_READY_TO_READ_MASK));
		fprintf(fo, "%d ", WVTRNX_STATUS(w_msg.tracks[i].status, 
					WVTRNX_STATUS_NEWLY_DISCOVERED_MASK));
		fprintf(fo, "%d ", WVTRNX_STATUS(w_msg.tracks[i].status, 
					WVTRNX_STATUS_ACTIVE_MASK)); 

		fprintf(fo, "%.2f ", WVTRNX_CVT_DISTANCE(w_msg.tracks[i].distance));
		fprintf(fo, "%.2f ", WVTRNX_CVT_SPEED(w_msg.tracks[i].speed));
				
	}
}
