/*\file
 *	For debugging connections and message formatting;
 *	echos bytes received from file, serial port or UDP socket
 *	to output file.
 *
 *	Default is to receive from any address on port 6056.
 *
 */ 

#include <sys_os.h>
#include "udp_utils.h"
#include <stdio.h>
#include <sqlite3.h>
#include <time.h>

#define     SQL_DB      "COSW.db"
#define     SQL_TABLE   " COSW "


typedef struct cosw_msg {
	unsigned char data[4];
} __attribute__((packed)) cosw_msg_t;

#define UDP_PACKET_SIZE sizeof(cosw_msg_t)


static sqlite3 *apdb;

static int sql_exec_callback(void *NotUsed, int argc, char **argv,
                                    char **azColName){
  int i;
  for(i=0; i<argc; i++){
    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }
  printf("\n");
  return 0;
}

int sql_exec(char *cmd)
{
  char *zErrMsg = 0;
  int rc;

  rc = sqlite3_exec(apdb, cmd, sql_exec_callback, 0, &zErrMsg);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }
  return 0;
}

int sql_init(char *dbname)
{
  char *zErrMsg = 0;
  int rc;

  rc = sqlite3_open(dbname ? dbname : SQL_DB, &apdb);
  if( rc ){
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(apdb));
    sqlite3_close(apdb);
    return -1;
  }

  sql_exec("DROP TABLE" SQL_TABLE ";");

  sql_exec("CREATE TABLE" SQL_TABLE 
             "(Safe int, Unsafe int, Distance int, TOD int);");
  return 0;
}

void sql_insert_msg(cosw_msg_t *msg)
{
    char cmd[256];
    time_t timeval;

    time(&timeval);

    snprintf(cmd, sizeof(cmd), "INSERT INTO"  SQL_TABLE
       "(Safe, Unsafe, Distance, TOD) VALUES(\'%u\', \'%u\', \'%u\', \'%u\');",
	msg->data[0], msg->data[1], msg->data[2], timeval);

    printf("Executing %s\n", cmd);
    sql_exec(cmd);
}

void do_usage(char *progname)
{
	fprintf(stderr, "Usage %s:\n", progname);
	fprintf(stderr, "-f input name ");
	fprintf(stderr, "-n output line length ");
	fprintf(stderr, "-o output name ");
	fprintf(stderr, "-p port ");
	fprintf(stderr, "-s serial (not socket) ");
	fprintf(stderr, "-v verbose ");
	fprintf(stderr, "\n");
	exit(1);
}

int main (int argc, char **argv)
{
	int option;		/// for getopt
	int fdin;		/// input file or socket descriptor
	int fdout;		/// output file descriptor
	char *finname = NULL;	/// set for serial port or single socket source 
	char *foutname = "stream.out";	/// default output file name
	int port = 6056;		/// default port 
	unsigned char buf[UDP_PACKET_SIZE];	/// input buffer 
	int do_serial = 0;	/// by default, receive from socket
	struct sockaddr_in src_addr;	/// filled in by recvfrom
	int bytes_received;	/// returned from recvfrom
	int verbose = 0;	/// echo formatted output to stdout
	int count = 0;		/// character count for formatting
	int linelength = 16;	/// default byte codes per line
	int i;			/// local counter for for loops
	unsigned int socklen;	/// holds sockaddr_in size for recvfrom call

        while ( (option = getopt( argc, argv, "f:o:p:sv" )) != EOF ) {
                switch( option ) {
                        case 'f':
                                finname = strdup(optarg);
                                break;
			case 'n':
				linelength = atoi(optarg);
				break;
                        case 'o':
                                foutname = strdup(optarg);
                                break;
                        case 'p':
                                port = atoi(optarg);
				break;
                        case 's':
                                do_serial = 1;
				break;
                        case 'v':
                                verbose = 1;
				break;
                        default:
				do_usage(argv[0]);
                                break;
                }
        }
	if (do_serial) {
		if (finname == NULL) 
			do_usage(argv[0]);
		if ((fdin = open(finname, O_RDONLY)) < 0) {
			perror("serial open");
			do_usage(argv[0]);
		}
	} else {
		if (finname == NULL) 
			fdin = udp_allow_all(port);
		else
			// finname must be IP numeric string x.x.x.x 
			fdin = udp_allow_from(inet_addr(finname), port);

		if (fdin < 0) 
			do_usage(argv[0]);
			
		
		fprintf(stderr, "Receive Port: %d\n", port);
	}

	if (finname == NULL)
		fprintf(stderr, "Input: any IP\n", finname);
	else
		fprintf(stderr, "Input: %s\n", finname);

	fdout = open(foutname, O_WRONLY | O_CREAT);
	fprintf(stderr, "Output: %s\n", foutname);

	socklen = sizeof(src_addr);
	memset(&src_addr, 0, socklen);

	if ( sql_init(NULL) ) return -1;

	while (1) {
		if (do_serial) {
			read(fdin, buf, 1);
			write(fdout, buf, 1);
			if (verbose) 
				printf("%02hhx ", buf[0]);
			count++;
			if (count % linelength == 0)
				printf("\n");
		} else { 
#if 1
			bytes_received = recvfrom(fdin, buf,
				 UDP_PACKET_SIZE, 0,
                                (struct sockaddr *) &src_addr, &socklen);

			if (bytes_received  < 0) {
				perror("recvfrom failed\n");
				break;
			}
			write(fdout, buf, bytes_received);
#else
			{
				cosw_msg_t *msg = (cosw_msg_t *)buf;
				sleep(1);
				msg->data[0] = 1;
				msg->data[1] = 2;
				msg->data[2] = 3;
			}		 	
#endif

			if (verbose) 
				for (i = 0; i < bytes_received; i++) {
					printf("%02hhx ", buf[i]);
					fflush(stdout);
					count++;
					if (count % linelength == 0)
						printf("\n");
				}
			/* expect 3 Byte packets, convert each ASCII byte value to int,
				1st byte - "Safe"
				2nd byte - "Unsafe"
				3rd byte - "Distance"
			   SQL table name - COSW (curve overspeed warning)
			   SQL file name - COSW.db
			*/
			sql_insert_msg((cosw_msg_t *)buf);
                }
	}
	fprintf(stderr, "%s exiting on error\n", argv[0]);
}
