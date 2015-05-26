/* FILE
 *   profile.c
 *
 * Routines to read and write profile items.
 *
 * Copyright (c) 2002   Regents of the University of California
 *
 */

#include "profile.h"

/* following included file has both type definitions and functions that depend
 * on the particular fields being profiled.
 */

#if defined(VEHICLE_PROFILE)
#include "vehicle_profile.c"
#elif defined(LONG_OUT_PROFILE)
#include "long_out_profile.c"
#elif defined(JCMD_PROFILE)
#include "jcmd_profile.c"
#else
error("No profile item definition")
#endif
 
db_clt_typ *database_init_for_profile (char **argv, int create_dbvs)

{
	int i;
	db_clt_typ *pclt = open_local_database(argv);
	int dbvs_created = 0;

	if (create_dbvs) {
		for (i = 0; i < profile_num_dbvs; i++){ 
			int num = dbv_used[i].dbn;
			if (clt_create (pclt, num, num, dbv_used[i].size)){
				dbvs_created++;
			}
		}
		if (dbvs_created != profile_num_dbvs) {
			fprintf(stderr, "Database variable not created\n");
			fprintf(stderr, "Created by another process? \n");
		}
	}
	return pclt;
}

/* Waits until at least one of the database variables that are being
 * profiled has been written.
 */

int trig_profile(db_clt_typ *pclt)

{
        pid_t trig_pid;
        trig_info_typ trig_msg;
	int var_num;
	int i;
	int trigger_set = 0;
	int dbv_read = 0;

	for (i = 0; i < profile_num_dbvs; i++) {
		var_num = dbv_used[i].dbn;
		if (clt_trig_set(pclt, var_num, var_num))
			trigger_set++;
	}
	if (!trigger_set)
		return 0;
        while (!dbv_read) {
                trig_pid = clt_ipc_receive(0, &trig_msg, sizeof(trig_msg));
		for (i = 0; i < profile_num_dbvs; i++) {
			var_num = dbv_used[i].dbn;
			if (DB_TRIG_VAR(&trig_msg) == var_num) {
				dbv_read = 1;
				break;
			}
		}
        }
	return 1;
}

/* reads a profile file, returns pointer to array, sets number of items */

profile_item *read_profile (char *filename, int *pnumitems) 
{
	int numitems = 0;
	int count = 0;
	profile_item *profile_array;
	FILE* fp;
	char buffer[MAX_PROFILE_LINE_LENGTH];

	if ((fp = fopen(filename, "r")) == NULL) {
		fprintf(stderr, "File %s open error\n", filename);
		return NULL;
	}

	/* first line of file should contain number of lines */

	if (!(fgets(buffer, MAX_PROFILE_LINE_LENGTH, fp)
		&& (sscanf(buffer, "%d", &numitems) == 1))) {
		fprintf(stderr, "Item count not found in %s\n", filename);
		return NULL;
	}
	fprintf(stderr, "%d lines in file\n", numitems);
	fflush(stderr);

	profile_array = (profile_item *)
			 malloc(sizeof(profile_item) * (numitems));
	
        while (fgets(buffer, MAX_PROFILE_LINE_LENGTH, fp)) {
		read_profile_item (buffer, &profile_array[count]);
		count++;
		if (count == numitems) 
			break;
	}	

	/* set number of items to number actually read */

	*pnumitems = count;
		
	fclose(fp);

	return profile_array;
}

/* writes a profile file, from an array with the given number of items */

int write_profile (char *filename, int numitems, profile_item *profile_array) 
{
	int count;
	FILE* fp;
	char buffer[MAX_PROFILE_LINE_LENGTH];

	if ((fp = fopen(filename, "w")) == NULL) {
		fprintf(stderr, "Output file %s open error\n", filename);
		return 0;
	}

	/* first line of file should contain number of lines */
 	sprintf(buffer, "%d\n", numitems);
	fputs(buffer, fp);

	for (count = 0; count < numitems; count++) {
		write_profile_item (buffer, &profile_array[count]);
        	fputs(buffer, fp);
	}	

	fclose(fp);
	return 1;
}
