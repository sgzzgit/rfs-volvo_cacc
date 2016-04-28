#ifndef LONG_COMM_H
#define LONG_COMM_H

/* This is used for the size of the array to hold 
 * veh_comm_packet_t received from other "trucks"
 */
#define MAX_PLATOON_SIZE	10

/** Use numbers from 480 to 489 for storing up to 10
 * veh_comm_packet_t variables in the DB for comm experiments.
 */
#define DB_COMM_BASE_TYPE 	480
#define DB_COMM_BASE_VAR 	480

/* Current longistudinal control code distinguishes messages
 * from the leader and from the preceding by their ID
 */
#define DB_COMM_LEAD_TRK_TYPE	496		/// veh_comm_packet_t 
#define DB_COMM_LEAD_TRK_VAR	DB_COMM_LEAD_TRK_TYPE
#define DB_COMM_SECOND_TRK_TYPE	497		/// veh_comm_packet_t 
#define DB_COMM_SECOND_TRK_VAR	DB_COMM_SECOND_TRK_TYPE
#define DB_COMM_THIRD_TRK_TYPE	498		/// veh_comm_packet_t 
#define DB_COMM_THIRD_TRK_VAR	DB_COMM_THIRD_TRK_TYPE
#define BSM_FLOAT_MULT		100.0
#define LONG_LAT_MULT		10000000.0
#define HEADING_MULT		80.0

/* Use this veh_comm_pkt_t variable to write the packet
 * that will be sent.
 */
#define DB_COMM_TX_TYPE		499		/// veh_comm_packet_t
#define DB_COMM_TX_VAR		DB_COMM_TX_TYPE

/* communications packet definition */
typedef struct {
	int node;		// Node number of packet origin
	timestamp_t rcv_ts;	// When message is received, from veh_recv
	timestamp_t ts;		// When message is sent, from veh_send
	float global_time;	// From long_ctl or trk_comm_mgr 
	float user_float;
	float user_float1;
	unsigned char user_ushort_1;
	unsigned char user_ushort_2;
	unsigned char my_pip;  // My position-in-platoon (i.e. 1, 2, or 3)
	unsigned char maneuver_id;
	unsigned char fault_mode;
	unsigned char maneuver_des_1;
	unsigned char maneuver_des_2;
	unsigned char pltn_size;
	char sequence_no;
	unsigned :0;		//Force int boundary for next 4 bit fields
	unsigned user_bit_1 : 1;
	unsigned user_bit_2 : 1;
	unsigned user_bit_3 : 1;
	unsigned user_bit_4 : 1;
	float acc_traj;		//Desired acceleration from profile (m/s^2)
	float vel_traj;		//Desired velocity from profile (m/s)
	float velocity;		//Current velocity (m/s)
	float accel;		//Current acceleration (m/s^2)
	float range;		//Range from *dar (m)
	float rate;		//Relative velocity from *dar (m/s)
	double longitude;
	double latitude;
	float heading;
	char object_id[GPS_OBJECT_ID_SIZE + 1];
} IS_PACKED veh_comm_packet_t;

typedef veh_comm_packet_t long_comm_pkt;

#endif 
