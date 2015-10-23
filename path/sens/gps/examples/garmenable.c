/* garmenable.c -  taken from code John wrote (OBMS project?)
**		   to initialize a Garmen GPS device
**		Compiles enough to print the following help message

lnx/garmenable enables and disables the GPS messages output by the
Garmin GPS18-5Hz.  The following examples illustrate its usage:

lnx/garmenable -e -s GPGGA > /dev/ttyS4 Enable GPGGA on serial port 4
lnx/garmenable -d -s GPGGA > /dev/ttyS4 Disable GPGGA on serial port 4
lnx/garmenable -d > /dev/ttyS4          Disable all ouput from serial port 4
lnx/garmenable -e > /dev/ttyS4          Enable all sentences ouput from
                                        serial port 4
lnx/garmenable -r > /dev/ttyS4          Restore factory default sentence
                                        output on serial port 4

When enabling or disabling one sentence type,
 the -s switch is NECESSARY. If it's not there,
 the sentence type will be ignored, and you will enable
 or disable ALL input.

** but not actually tested in the current version.
** 
*/

#include <sys_os.h>
#include <local.h>
#include "timestamp.h"
#include "path_gps_lib.h"

void help_message(char *progname)
{
	printf("\n%s enables and disables the ",progname);
	printf("GPS messages output by the\nGarmin ");
	printf("GPS18-5Hz.  The following examples illustrate its usage:\n\n");
	printf("%s -e -s GPGGA > /dev/ttyS4\tEnable GPGGA on serial port 4\n",
		 progname);
	printf("%s -d -s GPGGA > /dev/ttyS4\tDisable GPGGA on serial port 4\n",
		 progname);
	printf("%s -d > /dev/ttyS4\t\tDisable all ouput from serial port 4\n",
		 progname);
	printf("%s -e > /dev/ttyS4\t\tEnable all sentences ouput from\n", progname);
	printf("\t\t\t\t\tserial port 4\n");
	printf("%s -r > /dev/ttyS4\t\tRestore factory default sentence\n", progname);
	printf("\t\t\t\t\toutput on serial port 4\n\n");
	printf("When enabling or disabling one sentence type,\n");
	printf(" the -s switch is NECESSARY. If it's not there,\n");
	printf(" the sentence type will be ignored, and you will enable\n");
	printf(" or disable ALL input.\n\n");
}

void do_usage(char *progname)
{
	printf("Usage: %s <-e(nable),-d(isable),-r(estore)>", progname);
	printf("<-s GPS message> > <port>\n");
}

int main (int argc, char *argv[]) {

	char *flag = "";
	char *sent_type = "";
	char msg[100] = "$PGRMO," ;
	int option;
	int restore = 0;

	while(( option = getopt(argc, argv, "dehrs:")) != EOF) {

		switch(option) {
		case 'd':
			flag = "0";
			break;
		case 'e':
			flag = "1";
			break;
		case 's':
			sent_type = strdup(optarg);
			break;
		case 'r':
			restore = 1;
			break;
		case 'h':
			help_message(argv[0]);
			exit(EXIT_SUCCESS);
			break;
		default:
			do_usage(argv[0]);
			exit(EXIT_FAILURE);
		}
	}
	if(restore == 1) {
		flag = "4";
		sent_type = "";
		fprintf(stderr, 
			"Resetting sentence output to factory defaults\n");
	} else if(strcmp(flag,"") == 0) {
		do_usage(argv[0]);
		exit(EXIT_FAILURE);
	} else if(strcmp(sent_type,"") == 0) {
		if(strcmp(flag,"0") == 0){ 
			flag = "2";
			fprintf(stderr, "Disabling all input\n");
		} else {
			flag = "3";
			fprintf(stderr, "Enabling all input\n");
		}
	}	
	strcat(msg,sent_type);
	strcat(msg, ",");
	strcat(msg,flag);
	strcat(msg, "*");

//	printf("%s%X\n", msg, gps_checksum(msg) );
//	fprintf(stderr,"%s%X\n", msg, checksum(msg) );
}

