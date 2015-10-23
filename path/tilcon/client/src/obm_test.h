/**\file
 *
 *	Header file for Tilcon test code for On-Board Monitor project
 */

typedef struct {
	int radar_status;	// 0 or 1
} obm_radar_typ;

typedef struct {
	double meter_val;	// 0 to 100
	int meter_fill;		// TRT_RGB values
} obm_meter_typ;

