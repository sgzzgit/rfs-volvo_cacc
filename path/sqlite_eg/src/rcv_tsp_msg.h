#ifndef _RCV_TSP_MSG_
#define _RCV_TSP_MSG_

typedef struct{
	char recorded_date[12]; 
	char recorded_time[10]; 
	char recorded_ms[4];	
	char rcv_date_str[12];
	char rcv_time_str[10];
	unsigned short rcv_ms;
	int intersection_id;
	int signal_state;
	int time_to_next;
	int reqst_bus_id;
	int reqst_type;
	int reqst_phase;
	int bus_time_saved;
	int canPassOrNot;
	int dosound;
} tsp_msg_typ;

int parse_tsp_msg(char* instr, tsp_msg_typ *pp);

#endif
