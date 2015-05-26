/**\file 
 * Definitions for J1708 networks: J1587 and J1922 
 */ 

struct j1587_mid{
	int mid;		/// message ID -- source of message 
	int first_pid;		/// parameter ID of first parameter  
	int num_pids;		/// number of parameters in message 
};
 
/* Each active PID will have an associated function that converts
 * the data and writes it to any associated database variables.
 */
 
struct j1587_pid{
	int pid;		/// parameter ID 
	int num_chars;		/// number of characters in parameter 	
	void (*convert_param)(void *pclt, char *param);
				/// first byte of param is PID  
	int write_to_dbase;	/// 1 if write should occur, 0 otherwise 
}; 

#define J1587_MSG_LENGTH	21
extern struct j1587_mid msg_j1587[];
extern struct j1587_pid pid_j1587[];
