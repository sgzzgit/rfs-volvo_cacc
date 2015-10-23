/** 
 * Type and function declarations for db_utils.c
 */

#ifndef DB_UTILS_H
#define DB_UTILS_H
/* Type used for creating tables of database variables and their size
 */
typedef struct {
	int id;
	int size;
} db_id_t;

extern db_clt_typ *db_list_init(char *prog, char * hostname, char *domain,
                        int xport, db_id_t *varlist, int numvars,
                        int *triglist, int numtrigs);

extern void db_list_done(db_clt_typ *pclt, db_id_t *varlist, int numvars,
	int *triglist, int numtrigs);
/* 
 * Wrappers for database read and write functions that
 * take advantage of static inline calls to shorten code by
 * hiding the error reporting. Return value still allows
 * additional error checking if desired.
 */
	
static inline int db_clt_read(db_clt_typ *pclt, int db_num,
				 int db_size, void *dbv)
{
	db_data_typ db_data;
	int retval;
	if ((retval = clt_read(pclt, db_num, db_num, &db_data)) == FALSE)
		fprintf(stderr, "clt_read(%d) err\n", db_num);
	
	memcpy(dbv, db_data.value.user, db_size);
	return retval;
}

static inline int db_clt_write(db_clt_typ *pclt, int db_num,
				 int db_size, void *dbv)
{
	int retval;
	if ((retval = clt_update(pclt, db_num, db_num, db_size, dbv))
	             == FALSE)
		fprintf(stderr, "clt_update(%d) err\n", db_num);
	return retval;
}
#endif
