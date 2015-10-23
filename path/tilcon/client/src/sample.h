/**\file
 *
 *	Header file for Tilcon client test code 
 */

typedef struct {
	int radar_status;	// 0 or 1
} radar_typ;

typedef struct {
	int line_color;	
	int fill_color;	
} drawing_typ;

typedef struct {
	double meter_val;	// 0 to 100
	int meter_fill;		// TRT_RGB values
	int alarm_mode;		// fusses with alarm settings
} meter_typ;

