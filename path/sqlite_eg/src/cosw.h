/**
 *
 *	Header file for Curve Over Speed Warning structure
 */
#ifndef COSW_H
#define COSW_H

#ifndef MAX_CMD_STR
#define MAX_CMD_STR 256
#endif
/** roadside feature codes used in cosw_msg_t
 */
#define COSW_NO_FEATURE	0
#define COSW_STOP_SIGN	1
#define COSW_T_INTERSECTION	2
#define COSW_CURVE	3
#define COSW_INTERSECTION	4

/**
 *	If no roadside feature is present in the immediate approach, both	
 *	tha safe field  and the unsafe field will be 0.
 *	Otherwise one will be set to the roadside feature's value.
 *	The distance field is distance in meters from the host vehicel
 *	to the roadside feature in the safe or unsafe field.
 */
typedef struct cosw_msg {
	unsigned char safe;
	unsigned char unsafe;		 
	unsigned char distance;
	unsigned char reserved;		/// not used at this time
}  cosw_msg_t;

#endif
