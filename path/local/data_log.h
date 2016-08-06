/**\file
 *
 *	Function definitions to support data logging usingfiles with
 *	unique names automatically created and keyed to the time.
 *
 *	Also supports table definition of variables to be logged
 *	and replay programs.
 *
 *      Copyright (c) 2008   Regents of the University of California
 *
 *	Generalization of the code written by Paul Kretz and
 *	used in wrfiles.c in many projects. Installed in local
 *	directory by Sue Dickey.
 *
 */
#ifndef DATA_LOG_H 
#define DATA_LOG_H

#include <timestamp.h>
#include <sys_buff.h>

typedef struct {
	int db_id_num;
	int size;
	void *var_pointer;
} db_var_spec_t;

/** Base type entries in data column specifications.
 *  Needed in order to correctly access the data type
 *  to be logged from a void * pointer.
 */
#define BASE_CHAR	1	
#define BASE_SHORT	2	
#define BASE_INT	3	
#define BASE_FLOAT	4	
#define BASE_DOUBLE	5	
#define BASE_STRING	6	// treat as null-terminated string
#define BASE_TIMESTAMP	7	// treat as timestamp_t
#define BASE_HEX_INT	8	// printed as HEX, read with %i 

/** Indication for how to use thiis array element during replay.
 *  For example, if only debugging Tilcon off-line, may want to emulate
 *  both fusion and communication rather than running the processes.
 *  However, if running fusion and communication processes, too, do
 *  not want to write their outputs to data server.
 *  During replay, device driver outputs will always be emulated.
 */
#define REPLAY_TIME	1	// determines when to add next line to DB
#define REPLAY_NO	2	// never write to DB 
#define REPLAY_USE	3	// always write to DB 
#define REPLAY_COMM	4	// write to DB when emulating communication
#define REPLAY_FUSION	5	// write to DB when emulating fusion

typedef struct {
	char *format_string;
	void *field_pointer;
	unsigned char base_type;
	unsigned char replay_use;
} data_log_column_spec_t;

extern int open_data_log(FILE ** pf_data, char* prefix, char *suffix,
			double *pstart_time, int *pold_fileday, 
			int *pcounter, char *monthdayserialnum);
extern int reopen_data_log(FILE ** pf_data, int file_time, char *prefix, 
			char *suffix, double *pstart_time, int *pold_fileday,
			int *pcounter, char *monthdayserialnum,
			buff_typ *pbuf);
extern int open_data_log_infix(FILE ** pf_data, char* prefix, char *suffix,
			double *pstart_time, int *pold_fileday, 
			int *pcounter, char *monthday, char * serialnum,
			char *infix);
extern int reopen_data_log_infix(FILE ** pf_data, int file_time, char *prefix, 
			char *suffix, double *pstart_time, int *pold_fileday,
			int *pcounter, char *monthday, char *serialnum,
			char *infix, buff_typ *pbuf);
extern int sprint_data_log_column_entry(char *strbuf, data_log_column_spec_t *pentry);

extern int sprint_data_log_column_entry(char *strbuf, data_log_column_spec_t *pentry);

extern int sscan_data_log_column_entry(char *strbuf, data_log_column_spec_t *pentry);
extern int get_data_log_next_field(char *linebuf, char *tmpbuf, int max_length); 
extern int get_data_log_line(char *linebuf, data_log_column_spec_t *ptable, int size);


// TO DO: better error handling on failed file open
static inline void open_another_file(FILE **pf_data, char *prefix,
                         char *id_string, char *suffix)
{
        char filename[80];      // used to hold names for opening files
        snprintf(filename, 80, "%s%s%s", prefix, id_string, suffix);
        *pf_data = fopen(filename, "w");
}

// TO DO: better error handling on failed file open
static inline void reopen_another_file(FILE **pf_data, char *prefix,
                         char *id_string, char *suffix, buff_typ *pbuf)
{
        char filename[80];      // used to hold names for opening files
	fclose(*pf_data);
	if (pbuf != NULL)
		buff_done(pbuf);
        snprintf(filename, 80, "%s%s%s", prefix, id_string, suffix);
        *pf_data = fopen(filename, "w");
}

void save_to_spec (FILE *fout, timestamp_t timestamp,
	int use_memory, buff_typ *pbuff,
	int num_columns, data_log_column_spec_t *spec);


#endif
