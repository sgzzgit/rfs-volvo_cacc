#include <sys_os.h>
#include "local.h"
#include "db_clt.h"
#include "db_utils.h"

/* Function to create database variables and set triggers
 * from list of database variables and their sizes.
 */
db_clt_typ *db_list_init(char *prog, char *hostname, char *domain, int xport, 
			db_id_t *varlist, int numvars,
			int *triglist, int numtrigs)
{
	db_clt_typ *pclt;
	int create_error = 0;
	int trigger_error = 0;
	int i;

	if((pclt = clt_login(prog, hostname, domain, xport)) == NULL ) {
		fprintf(stderr, "%s: clt_login errors\n", prog);
		return(NULL);
	}
	for (i = 0; i < numvars; i++) {
		int id = varlist[i].id;
		int size = varlist[i].size;
		if(clt_create( pclt, id, id, size) == FALSE ) {
			create_error += 1;
			printf("create error, var %d size %d\n", id, size);
		}
	}
	if (create_error != 0) {
		fprintf(stderr, "%s: create errors %d\n", prog, create_error);
		clt_logout(pclt);
		return(NULL);
	}
	for (i = 0; i < numtrigs; i++) {
		if(clt_trig_set( pclt, triglist[i], triglist[i]) == FALSE ) 
			trigger_error += 1;
	}
	if (trigger_error != 0) {
		fprintf(stderr, "%s: trigger errors %d\n", prog, trigger_error);
		clt_logout(pclt);
		return(NULL);
	}
	return pclt;
}

// The list of variables you destroy may not be the same as the list
// you create, if another process will read the variable after you exit.
// However you should always unset triggers before your process exits.
//
void db_list_done(db_clt_typ *pclt, db_id_t *varlist, int numvars,
	int *triglist, int numtrigs)
{
	int i;

	if (pclt == NULL) return;

	for (i = 0; i < numtrigs; i++) 
		clt_trig_unset(pclt, triglist[i], triglist[i]);

	for (i = 0; i < numvars; i++) 
		clt_destroy(pclt, varlist[i].id, varlist[i].id);

	clt_logout(pclt);
}
