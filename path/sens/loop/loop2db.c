/**\file
 * Database variable update time test: test_update
 *
 * (C) Copyright University of California 2006.  All rights reserved.
 *
 * This program writes a variable to the database at a specified
 * interval, changing the value each cycle. For timing tests.
 *
 */

#include	<sys_os.h>
#include        "loop.h"
#include        "sys/timeb.h"
#include	"db_clt.h"
#include 	"sys_rt.h"
#include        "local.h"
#include	"timestamp.h"

#define MAX_BUF 256

// Forward declarations
static void request2response(int fpin, int fpout, char request[],
                             char response[], int response_length);
static void reset_unit(int fpin, int fpout);

static void request_firmware_version(int fpin, int fpout);
static void watch_inductance_forever(int fpin, int fpout, int var, db_clt_typ* pclt);


static int sig_list[]=
{
	SIGINT,
	SIGQUIT,
	SIGTERM,
	SIGALRM,	// for timer
	ERROR
};
static jmp_buf exit_env;
static void sig_hand( int code )
{
	if (code == SIGALRM)
		return;
	else
		longjmp( exit_env, code );
}

char *SERIAL_DEV_NAME="/dev/ttyUSB0";
Loops_TYP loop_typ;
time_t now;

int main (int argc, char** argv)
{
	char *domain=DEFAULT_SERVICE;
	char hostname[MAXHOSTNAMELEN+1];
	db_clt_typ *pclt;
	//new_type value;
	int opt;
	int var = DB_LOOP_VAR;           /* FIXIT */
        char* cmd = "inductance";
	posix_timer_typ *ptmr;
	int millisec = 1000;	/* 1 second interval */
	int numwrites = 60;	/* run for 60 seconds */
	int i;
	int chid;
	int xport = COMM_PSX_XPORT;
	int fpin, fpout;
        char ttyNum;
	int LaneNum;

        char SERIAL_DEV_NAME[13]="/dev/ttyUSB0";
	printf("begin main\n");
	while ((opt = getopt(argc, argv, "c:d:f:i:n:t:v:l:")) != -1)
	{
		switch (opt)
		{
                  case 'c':
			cmd = strdup(optarg);
                        printf("cmd=%s\n", cmd);
                        break;
		  case 'd':
			domain = strdup(optarg);
			break;
		  case 'i':
			//value.new_int = atoi(optarg);
			break;
		  case 'f':
			//value.new_double = atof(optarg);
			break;
		  case 'n':
			numwrites = atoi(optarg);
			break;
		  case 't':
			millisec = atoi(optarg);
			break;
		  case 'v':
			var = atoi(optarg);
			break;
		  case 'x':
			xport = atoi(optarg);
			break;
                  //case 's':
		  //      SERIAL_DEV_NAME = strdup(optarg);
                  //      break;
	          case 'l':
			ttyNum = optarg;
                        break;
   		  default:
			fprintf(stderr, "Usage: %s -d [domain] -v [var] ",argv[0]);
			fprintf(stderr, " -i [int] -f [float] -x [xport] \n");
			fprintf(stderr, "       -c [command] -s [serial port name] \n");
			exit(EXIT_FAILURE);
		}
	}

	if( ttyNum>='0' || ttyNum<='7')
	{
		strcpy(SERIAL_DEV_NAME[11],ttyNum);
		LaneNum = atoi(ttyNum);		
        }
	else
		printf("ttyNum is wrong\n");

	get_local_name(hostname, MAXHOSTNAMELEN);
	if ((pclt = clt_login(argv[0], hostname, domain, xport))
			== NULL)
	{
		printf("clt login %s %s %s %d failed\n",
			argv[0], hostname, domain, xport);
		exit(EXIT_FAILURE);
	}

	if (!clt_create(pclt, var, var, sizeof(loop_typ))) {  // FIXIT
		printf("clt create failed\n");
		clt_logout(pclt);
		exit(EXIT_FAILURE);
	}

	/* Catch the signals SIGINT, SIGQUIT, and SIGTERM.  If signal occurs,
	 * log out from database and exit. */
	if( setjmp( exit_env ) != 0 )
	{
	    /* Log out from the database. */
	    if (pclt != NULL)
            {
		clt_destroy( pclt, var, var);
	        clt_logout( pclt );
	    }
	    close(fpin);
	    close(fpout);
	    exit( EXIT_SUCCESS );
	}
	else
	{
		sig_ign( sig_list, sig_hand );
	}
	fpin = open_input(SERIAL_DEV_NAME);
	fpout = open_output(SERIAL_DEV_NAME);
#if 0
	for (i = 0; i < 10; i++) {
		char buf[80];
		int num_read;
		write(fpout, "abc\n", 5);
		num_read = read(fpin, buf, 8);
		if (num_read > 0) {
			buf[num_read] = 0;
			printf("%s\n", buf);
		}
	}
	longjmp(exit_env, 2);
#endif

	if (0 == strcmp(cmd, "inductance")) {
        		watch_inductance_forever(fpin, fpout, LaneNum, var, pclt);
		} else if (0 == strcmp(cmd, "reset")) {
			reset_unit(fpin, fpout);
		} else if (0 == strcmp(cmd, "firmware")) {
			request_firmware_version(fpin, fpout);
		} else {
			printf("Unrecognized cmd: %s\n", cmd);
		}
	longjmp( exit_env, 1 );
} // main

/*
 * The function, watch_inductance_forever, sits in a loop that
 * continuously reads a Canoga loop detector card for the
 *  purpose of writing calculated loop inductances into
 * the client database.
 *
 * The database variable is XX. The payload is an array of five
 * doubles. The first four are the inductances of the four
 * loops attached to the card; the fifth is a timestamp.
 */

const unsigned int DEFAULT_ADDRESS = 255;
const double N32_PERIOD = 1.0/32000000.0;
const double TWO_PI = 6.28318530717959;

int open_output(const char* name) {

  int h = open(name, O_WRONLY | O_NONBLOCK);
  //int h = open(name, O_WRONLY);
  if (h < 0) {
    printf("Error opening %s for output\n", name);
    exit(EXIT_FAILURE);
  }
  return h;

//  return 6;
} // open_output

int open_input(const char* name) {

  int h = open(name, O_RDONLY | O_NONBLOCK);
  //int h = open(name, O_RDONLY);
  if (h < 0) {
    printf("Error opening %s for input\n", name);
    exit(EXIT_FAILURE);
  }
  return h;

//  return 7;
} // open_input

unsigned int calculate_checksum(unsigned char message[], int significant_chars) {
  unsigned char checksum = 0;
  int i = 0;
  for (i=0; i<significant_chars; i++) {
    checksum ^= message[i];
  }
  return checksum;
} // calculate_checksum

void insert_address(unsigned char request[], unsigned char address) {
  unsigned char* pch = strchr(request, '@');
  if (pch) {
    *pch = address;
  }
} // insert_address

void insert_tail(unsigned char request[]) {
  // Fill the last three characters of the request with the checksum and '>'
  int significant_chars = strlen(request) - 3;
  unsigned char checksum = calculate_checksum(request, significant_chars);
  sprintf(&request[significant_chars], "%02X>", checksum);
} // insesrt_tail

unsigned int hex2ui(char* pch, int count) {
  // Convert the first 'count' characters of a hex string
  // starting at pch to an unsigned integer
  unsigned int result = 0;
  while (0 < count--) {
    result = result << 4;
    char ch = *pch++;
    if ('0' <= ch && ch <= '9') {
      result += ch - '0';
    } else if ('A' <= ch && ch <= 'F') {
      result += ch - 'A' + 10;
    } else if ('a' <= ch && ch <= 'f') {
      result += ch = 'a' + 10;
    } else {
      printf("Unexpected char '%c' in hex string\n", --*pch);
    }
  }
  return result;
} // hex2ui

double calc_inductance(unsigned int mHzPC, unsigned int loopPC) {
  if (loopPC == 0)
	return 0;
  double freq = 1.0/(mHzPC/loopPC*N32_PERIOD);
  double omega = TWO_PI*freq;
  return 1.39/((8.92e-08)*omega*omega) - 150.0e-06;
} // calc_inductance

void request2response(int fpin, int fpout, char request[],
                      char response[], int response_length) {
  //printf("request2response(request=%s, response_length=%d)\n", request, response_length);
  int num_read = 0;
  int total_read = 0;
  int countdown = 0;
  do {
    // Start over with a new prompt
    countdown = 110;                 // resume countdown to new prompt
    response[0] = 0;                 // erase any prior '<'
    response[response_length-1] = 0; // erase any prior '>'
    response[response_length] = 0;
    write(fpout, request, strlen(request));
    //printf("wrote: %s\n", request);
    // Read until getting the first character,
    // or the countdown limit expires
    do {
      read(fpin, response, 1);
      delay(1);
    } while ('<' != response[0] && 0 <= --countdown);
    if ('<' == response[0]) {
      // We have the start of a valid message
      total_read = 1;
      // Read the remaining characters, or the countdown limit expires
      do {
        num_read = read(fpin, &response[total_read], response_length - total_read);
        delay(2);
        if (0 < num_read) {
          total_read += num_read;
        }
      } while (total_read < response_length && 0 <= --countdown);
    }
    // Conclude this is a valid response only if last character is '>'
//printf("response=%s\n", response);
  } while ('>' != response[response_length-1]);
} // request2response

static void request_firmware_version(int fpin, int fpout) {
  printf("request_firmware_version\n");
  char request[] = "<08@BXX>";
  insert_address(request, DEFAULT_ADDRESS);
  insert_tail(request);
  const int FIRMWARE_RESPONSE_LENGTH = 18;
  unsigned char response[FIRMWARE_RESPONSE_LENGTH+1];
  response[FIRMWARE_RESPONSE_LENGTH] = 0;
  request2response(fpin, fpout, request, response, FIRMWARE_RESPONSE_LENGTH);
  printf("response=%s\n", response);
} // request_firmware_version

static void reset_unit(int fpin, int fpout) {
  printf("reset_unit\n");
  char request[] = "<08@GXX>";
  insert_address(request, DEFAULT_ADDRESS);
  insert_tail(request);
  const int RESET_RESPONSE_LENGTH = 8;
  unsigned char response[RESET_RESPONSE_LENGTH+1];
  response[RESET_RESPONSE_LENGTH] = 0;
  request2response(fpin, fpout, request, response, RESET_RESPONSE_LENGTH);
  printf("response=%s\n", response);
} // reset_unit

static void watch_inductance_forever(int fpin, int fpout, int lane, int var, db_clt_typ* pclt) {
  printf("watch_inductance_forever for lane %d\n",lane);
  char request[] = "<08@MXX>";
  insert_address(request, DEFAULT_ADDRESS);
  insert_tail(request);
  const int INDUCTANCE_RESPONSE_LENGTH = 56;
  unsigned char response[INDUCTANCE_RESPONSE_LENGTH+1];
  //char response[INDUCTANCE_RESPONSE_LENGTH+1];
  response[INDUCTANCE_RESPONSE_LENGTH] = 0;
  //char response[] = "<38@M000000000000000000000000000000000000000000000000XX>\0";
  
  //FILE* fp=fopen("loop.txt","w");

  for (;;) {
    request2response(fpin, fpout, request, response, INDUCTANCE_RESPONSE_LENGTH);
    //printf("response=%s\n", response);
    struct timeb ts;
    if (ftime(&ts)!=0)
	fprintf(stderr,"can't read timestamp\n");
    loop_typ.ts = (double)ts.time+ts.millitm/1000.0;
    loop_typ.LaneNum = lane;
    //fprintf(fp,"%lf ",loop_typ.ts);
    
    //channel_info* pchannel = (channel_info*) &response[5];
    unsigned char * pchannel = &response[5];
    int i;
    for (i=0; i<MAX_LOOPS_PER_LANE; i++) {
      unsigned int mHzPC = hex2ui(pchannel, 8);
      unsigned int loopPC = hex2ui(pchannel+8, 4);
      loop_typ.Inductance[i] = calc_inductance(mHzPC, loopPC);
      //fprintf(fp,"%d %f ",i+1,loop_typ.Inductance[i]);
      pchannel+=12;
    }
    //fprintf(fp,"\n");
    if (!clt_update(pclt, var, var, sizeof(loop_typ), &loop_typ)) {
      printf("clt_update failed\n");
    }
    delay(40);
  }
} // watch_inductance_forever
