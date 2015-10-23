/**\file 
 *
 * macaddrget.c     
 *              Reads a line from stdin that represents a beacon MAC addr
 *		Saves the 5 most recent unique.
 *		Writes all 5 on stdout if over a second has elapsed since
 *		the last write, and zeroes the array.
 *
 *  Copyright (c) 2008   Regents of the University of California
 *
 *  The signals SIGINT, SIGQUIT, and SIGTERM are trapped, and cause the
 *  process to terminate. SIGALRM is used for Posix timers on Linux.
 *
 */


#include <sys_os.h>
#include <sys_rt.h>
#include <local.h>
#include <timestamp.h>

jmp_buf env;
static void sig_hand(int sig);

static int sig_list[] = {
	SIGINT,
	SIGQUIT,
	SIGTERM,
	SIGALRM,
	ERROR,
};

#define MAX_ADDR_STR	16

typedef struct {
	char mac_addr_str[MAX_ADDR_STR];
	int filled;
} mac_rec_t;

#define MAX_SAVED 5

mac_rec_t mac_addr[MAX_SAVED];

int main(int argc, char *argv[])
{
	int status;
	char buf[80];
	timestamp_t ts, new_ts;
	int millisec;	/// difference between new_ts and ts
	int verbose = 0;	/// if 1, print extra info for debugging
	int interval = 1000;	/// milliseconds between saves
	int new_addr;		/// TRUE if not already saved
	int num_saved = 0;
	int next_store = 0;
	int fgets_error = 0;
	int i;
	int option;

	/* Read and interpret any user switches. */
	while ((option = getopt(argc, argv, "i:v")) != EOF) {
		switch(option) {
		case 'i':
			interval = atoi(optarg);
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			fprintf(stderr, "Usage %s: \n", argv[0]); 
			fprintf(stderr, " -i  min millisec between saves \n ");
			fprintf(stderr, " -v  (verbose) \n ");
			fprintf(stderr, "\n");
			exit(EXIT_FAILURE);
			break;
		}
	}
	if (setjmp(env) != 0) {
		exit(EXIT_SUCCESS);
	} else
               sig_ign(sig_list, sig_hand);

	for (i = 0; i < MAX_SAVED; i++) {
		memset(&mac_addr[i], 0, sizeof(mac_rec_t));
	}
	get_current_timestamp(&ts);
	while (1) {
		if (!fgets(buf, 80, stdin)) {
			perror("fget failed");
			fgets_error++;
			if (fgets_error >10) {
				printf("exiting: multiple fgets fail\n");
			}
		}	
		buf[MAX_ADDR_STR-2] = 0;	// get rid of EOL
		for (i = 0; i < MAX_ADDR_STR-2; i++) 
			if (buf[i] == ' ') buf[i] = ':';
		new_addr = TRUE;
		for (i = 0; i < MAX_SAVED; i++) {
			char *str= &mac_addr[i].mac_addr_str[0];
			if (mac_addr[i].filled) {
				if (verbose)
					printf("comparing %s %s \n", str, buf);
				 if (strncmp(str, buf, 16) == 0)
					new_addr = FALSE;
			}
		}
		if (new_addr) {
			char *dest= &mac_addr[next_store].mac_addr_str[0];
			mac_addr[next_store].filled = TRUE;
			strncpy(dest, &buf[0], MAX_ADDR_STR);
			next_store++;
			if (next_store == MAX_SAVED) next_store = 0;
			if (num_saved < MAX_SAVED) num_saved++;
		}	
		get_current_timestamp(&new_ts);
		time_diff(&ts, &new_ts, &millisec);
		if (millisec > interval) {
			print_timestamp(stdout, &ts);
			printf(" ");
			print_timestamp(stdout, &new_ts);
			printf(" ");
			ts = new_ts;
			printf("%d ", num_saved);
			num_saved = 0;
			for (i = 0; i < MAX_SAVED; i++) {
				if (mac_addr[i].filled)
					printf("%s ",mac_addr[i].mac_addr_str); 
				else
					printf("0000:0000:0000 ");
				mac_addr[i].filled = FALSE;
			}
			printf("\n");
		}
	}
}

static void sig_hand(int sig)
{
	longjmp(env, 1);
}
