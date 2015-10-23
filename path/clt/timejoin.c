/**\file
 *
 * timejoin 
 *
 * Reads two column-formatted files, each with hr:min:sec:millisec timestamps
 * in column 1. Writes a third file in which a row is created for
 * each timestamp that occurs in either file, and all columns from
 * both files appear in the each output row. Optionally a row can
 * be created only for the timestamps appearing in one or the other file 
 *
 * All input files are assumed to have extension .tmj, and each
 * file must have the same number of columns in every row.
 *
 * Pre-process and post-process the files with awk to get what
 * you really want.
 *
 * Copyright (c) 2008   Regents of the University of California
 */

#include "sys_os.h"
#include "local.h"
#include "sys_rt.h"
#include "timestamp.h"

/* maximum number of characters in file names, and in rows of data 
 */
#define NAME_LENGTH	80
#define ROW_LENGTH	1024

/* signal handling set-up */
static int sig_list[] =
{
        SIGINT,
        SIGQUIT,
        SIGTERM,
        ERROR,
};

static jmp_buf env;

static void sig_hand(int sig)
{
        longjmp(env, 1);
}

int main(int argc,char **argv)
{
	int ch;				// used with getopt
	int verbose = 0;		// print verbose to stdout
	int which = 0;			// chooses preferred file
	FILE *fp1 = NULL;		// first input file
	FILE *fp2 = NULL;		// second input file 
	FILE *fpout = stdout;		// output file 
	char in1_name[NAME_LENGTH];	// 1st file name, including extension 
	char in2_name[NAME_LENGTH];	// 2nd file name, including extension 
	char out_name[NAME_LENGTH];	// name of output file, set from header
	char *in1_str = "f1";		// name without extension,
	char *in2_str = "f2";		// can be changed from command line 
	int out_to_file = 0;		// by default use stdout
	char *out_str = NULL;		// set on command line to use file
	timestamp_t ts1;		// timestamp, file 1
	timestamp_t ts2;		// timestamp, file 2
	timestamp_t new_ts1;		// used to read in timestamp, file 1
	timestamp_t new_ts2;		// used to read in timestamp, file 2
	char in_buf1[ROW_LENGTH];	// for current row read, file 1
	char in_buf2[ROW_LENGTH];	// for current row read, file 2
	char save_buf1[ROW_LENGTH];	// for saved row, file 1	
	char save_buf2[ROW_LENGTH];	// for saved row, file 2
	int exit_code;			// argument to longjmp 

        while ((ch = getopt(argc, argv, "f:g:o:vw:")) != EOF) {
                switch (ch) {
		case 'f':
			in1_str = strdup(optarg);
			break;
		case 'g':
			in2_str = strdup(optarg);
			break;
		case 'o':
			out_str = strdup(optarg);
			out_to_file = 1;
			break;
                case 'v': verbose = 1; 
			break;
                case 'w': which = atoi(optarg); 
			break;
		default:
			printf("Usage: %s -v <verbose>\n", argv[0]);
			exit(EXIT_FAILURE);
			break;
		}
	}
	// set buffers to 0 so it is safe to use strlen
	memset(in_buf1, 0, ROW_LENGTH);
	memset(in_buf2, 0, ROW_LENGTH);
	memset(save_buf1, 0, ROW_LENGTH);
	memset(save_buf2, 0, ROW_LENGTH);

	snprintf(in1_name, NAME_LENGTH, "%s.tmj", in1_str); 
	if (!(fp1 = fopen(in1_name,"r"))) {
		printf("Unable to open %s for reading\n", in1_name);
		exit (EXIT_FAILURE);
	}
	snprintf(in2_name, NAME_LENGTH, "%s.tmj", in2_str); 
	if (!(fp2 = fopen(in2_name,"r"))) {
		printf("Unable to open %s for reading\n", in2_name);
		exit (EXIT_FAILURE);
	}
	if (out_to_file) { 
		strncpy(out_name, out_str, NAME_LENGTH); 
		if (!(fpout = fopen(out_name,"w"))) {
			printf("Unable to open %s for writing\n", out_name);
			exit (EXIT_FAILURE);
		}
	}
				
	if (verbose){ 
		printf("file 1 %s, file 2 %s, out file %s\n", 
			in1_name, in2_name, out_to_file?out_name:"stdout");
		fflush(stdout);
	}

        /* Exit code after signal or longjmp  */
        if ((exit_code = setjmp(env)) != 0) {
		printf("exiting with code %d\n", exit_code); 
		fclose(fp1);
		fclose(fp2);
		if (fpout)
			fclose(fpout);
                exit(EXIT_SUCCESS);
        } else
                sig_ign(sig_list, sig_hand);

	// Read first line of both files to initialize timestamps and buffers	
	if (!(fgets(in_buf1, ROW_LENGTH-1, fp1)))
		longjmp(env,1); // error reading first line of file 1
	if (verbose) {
		printf("in1: %s\n", in_buf1);
		fflush(stdout);
	}
	if (!str2timestamp(in_buf1, &new_ts1))
		longjmp(env,3);	// no timestamp on first line of file 1
	strncpy(save_buf1, in_buf1, ROW_LENGTH-1);
	save_buf1[strlen(in_buf1)-2] = 0;	// get rid of EOL

	if (!(fgets(in_buf2, ROW_LENGTH-1, fp2)))
		longjmp(env,2); // error reading first line of file 2
	if (verbose) {
		printf("in2: %s\n", in_buf2);
		fflush(stdout);
	}
	if (!str2timestamp(in_buf2, &new_ts2))
		longjmp(env,4);	// no timestamp on first line of file 4
	strncpy(save_buf2, in_buf2, ROW_LENGTH-1);
	save_buf2[strlen(in_buf2)-2] = 0;	// get rid of EOL

	fprintf(fpout, "%s %s\n", save_buf1, save_buf2); 
	fflush(fpout);
	/// Loop exits on end of file or error processing header
	while (1){
		int do_print = 1;
		ts1 = new_ts1;
		ts2 = new_ts2;
		if (ts2_is_later_than_ts1(&ts1, &ts2)) {
			if (verbose)
				printf("ts2 later, in1: ");
			if (!(fgets(in_buf1, ROW_LENGTH-1, fp1)))
				longjmp(env,1);
			if (!str2timestamp(in_buf1, &new_ts1)) {
				if (verbose) 
					printf("bad fp1 row, no timestamp\n");
				continue; 
			}
			strncpy(save_buf1, in_buf1, ROW_LENGTH-1);
			save_buf1[strlen(in_buf1)-2] = 0;// get rid of EOL
			if (verbose) {
				printf("%s\n", save_buf1);
				fflush(stdout);
			}
		} else { 
			if (verbose)
				printf("ts1 later, in2: ");
			if (!(fgets(in_buf2, ROW_LENGTH-1, fp2)))
				longjmp(env,2);
			if (!str2timestamp(in_buf2, &new_ts2)) {
				if (verbose) 
					printf("bad fp2 row, no timestamp\n");
				continue; 
			}
			strncpy(save_buf2, in_buf2, ROW_LENGTH-1);
			save_buf2[strlen(in_buf2)-2] = 0;// get rid of EOL
			if (verbose) {
				printf("%s\n", save_buf2);
				fflush(stdout);
			}
		}
		if ((which == 1) && !ts2_is_later_than_ts1(&ts1, &new_ts1)) {
			if (verbose) printf("which 1: don't print\n"); 	
			do_print = 0;	 // don't print this row
		}
	
		if ((which == 2) && !ts2_is_later_than_ts1(&ts2, &new_ts2)){ 
			if (verbose) printf("which 2: don't print\n"); 	
			do_print = 0;	 // don't print this row
		}
		if (do_print) 
			fprintf(fpout, "%s %s\n", save_buf1, save_buf2);	
		fflush(fpout);
	}
}
