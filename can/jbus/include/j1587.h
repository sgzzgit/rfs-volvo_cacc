/**\file
 *	Header file for SAE J1587 support routines.
 */ 
	
#define J1587_MSG_MAX      21

/* out of range J1939 PGN value defined for use in j1939_dbv_info structure 
 */ 
#define J1587_PGN	999

#define J1587_DB_OFFSET 550

/** Structure used to store J1587 Parameters in database.
 */
 
struct j1587_pid{
        int pid;                /// parameter ID 
        int num_chars;          /// number of data characters following pid 
        void (*convert_param)(int *param, void *db_value);
                                /// first byte of param is PID  
	int db_num;		/// database variable number containing PID 
	void *dbfield;		/// pointer to field in local update variable 
	int field_type;		/// 0 int, 1 unsigned char, 2 float 
	int *changed_flag;	/// changed flag for this database variable 
};

struct j1587_update {
	void *dbv;		/// pointer to database variable 
	int db_num;		/// database variable number 
	int changed;		/// changed since last database write? 
};

#define TYPE_IS_INT	0
#define TYPE_IS_UCHAR	1
#define TYPE_IS_FLOAT	2
