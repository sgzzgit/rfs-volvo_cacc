#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <sys_os.h>
#include "can_client.h"
#include "msg_descriptors.h"
#include "msg_packer.h"
#include "sys_list.h"
#include "sys_rt.h"
#include "db_clt.h"


typedef struct
{
    int id;
    uint64_t data[10]; // assume 10 messages for now, same as input_gen
    int frequency; /* hertz */
    int period;    /* milliseconds */
    int lastsent;  /* milliseconds */
} msg_type;

uint64_t parse_msg_input(msg_descriptor_t* p, 
			 uint64_t** extra_msg_out, size_t* extra_count);
void print_data(uint64_t n)
{
    uint8_t* p = (uint8_t*)&n;
    printf("%.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X",
	   p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
}

int main(int argc, char* argv[])
{
    int i = 0;
    msg_type messages[4];
    unsigned num_msg = sizeof(messages) / sizeof(messages[0]);
    struct timespec timeread;
    struct timespec timedelay;
//    int fd = -1;
//    char* port = "/dev/can1";
    char line[1024];
    char* read = NULL;
    int linenum = 0;
    uint8_t extended = 0;
    int nmax = 0;
    int opt = 0;
    int verbose = 0;

    db_clt_typ *pclt;              /* Database client pointer */
    char hostname[MAXHOSTNAMELEN+1];
    char *domain = DEFAULT_SERVICE;
    int xport = COMM_OS_XPORT;	// Correct value must be in sys_os.h


    // parse the command-line args
    while ((opt = getopt(argc, argv, "e:n:p:v:z")) != -1)
    {
	printf ("opt = %d\n", opt);
	switch (opt)
	{
	case 'e':
	    extended = 1;
	    break;
	case 'n':
	    nmax = atoi(optarg);
	    break;
/*
	case 'p':
	    port = strdup(optarg);
	    break;
*/
	case 'v':
	    verbose = 1;
	    break;
	}
    }

    memset(messages, 0, sizeof(messages));
    for (i = 0; i < num_msg; ++i)
    {
	messages[i].frequency = i * 10 + 10; // 10Hz, 20Hz, 30Hz, etc...
	messages[i].period = 1000 / messages[i].frequency;
    }

    // read 10 lines of messages from stdin
    read = fgets(line, sizeof(line), stdin);
    while ((int)read != EOF && read != NULL)
    {
	char* token = strtok(line, " \t");
	int num_msg_incoming = atoi(token);
	int id;

	// check number of messages matches the number we can store
	if (num_msg_incoming != num_msg)
	{
	    fprintf(stderr, "number of messages don't match: %d, %d\n",
		    num_msg_incoming, num_msg);
	    continue;
	}

	
	for (i = 0; i < num_msg_incoming; i++)
	{
	    msg_descriptor_t* p = NULL;
	    uint64_t msg_raw = 0;
	    
	    token = strtok(NULL, " \t");
	    id = atoi(token);
	    p = &msg_descriptors[id / 100 - 1];

	    if (messages[i].id != p->identifier)
	    {
		if (messages[i].id == 0)
		    messages[i].id = p->identifier;
		else
		{
		    fprintf(stderr, "identifers don't match.");
		    continue;
		}
	    }

	    msg_raw = parse_msg_input(p, NULL, NULL);
	    messages[i].data[linenum] = msg_raw;

	    if (verbose)
	    {
		printf ("msg parsed: ");
		print_data(msg_raw);
		putchar('\n');
	    }
	}

	linenum++;
	read = fgets(line, sizeof(line), stdin);
    }

    puts("input data correctly parsed.");

    // initialize the timedelay structure
    timedelay.tv_sec = 0;
    timedelay.tv_nsec = 5000000;   /* 5 millsecs */

/*
// open can port
fd = can_open(port, O_WRONLY);
if (fd == -1)
{
fprintf (stderr, "error opening %s\n", port);
exit(1);
}
printf ("can port opened. fd = %d\n", fd);
*/

    /* Log in to the database (shared global memory).  Default to the
     * the current host. */
    get_local_name(hostname, MAXHOSTNAMELEN);

    if (( pclt = clt_login( argv[0], hostname, domain, xport)) == NULL )
    {
	printf("Database initialization error in db_writer.\n");
	exit (EXIT_FAILURE);
    }
    printf("pclt 0x%x, hostname %s\n", (int) pclt, hostname);

    for (i = 0; i < num_msg; i++)
    {
	int id = messages[i].id;
	if (clt_create( pclt, id, id, sizeof(messages[0].data[0])) == FALSE)
	{
	    printf("clt_create failed: %d\n", id);
	    exit (EXIT_FAILURE);
	}
    }
    printf("clt_create %d succeeded\n", 200);

    /* Lower priority of this process to 9, since this is not a
     * critical task. */
    if ( setprio( 0, 9) == ERROR )
    {
	fprintf( stderr, "Can't change priority of chgdii to 9.\n" );
	exit ( EXIT_FAILURE );
    }

    
    // keep on sending data until someone decides to press ctrl+C
    for (;;) 
    {
	unsigned now_ms;

	clock_gettime(CLOCK_REALTIME, &timeread);
	now_ms = timeread.tv_sec * 1000 + timeread.tv_nsec / 1000000;

	for (i = 0; i < num_msg; ++i)
	{
	    // see how long it has been since that message was last sent
	    unsigned long diff = now_ms - messages[i].lastsent;

	    // if longer than period, send again
	    if (diff >= messages[i].period)
	    {
		int rand_msg = rand() % 10;
		if (clt_update(pclt, messages[i].id, messages[i].id,
			       sizeof(messages[i].data[rand_msg]),
			       &messages[i].data[rand_msg]) == FALSE)
		{
		    fprintf(stderr, "clt_update( %d ) failed.\n",
			    messages[i].id);
		}
/*
		can_write(fd, 
			  messages[i].id, 
			  0, 
			  &messages[i].data[rand_msg], // pick random
			  8);
*/		
		if (verbose)
		{
		    printf ("msg written: i=%d, id=%d, msg_index=%d, data= ",
			    i, messages[i].id,
			    rand_msg);
		    print_data(messages[i].data[rand_msg]);
		    putchar('\n');
		}

		messages[i].lastsent = now_ms;
	    }
	}

	// sleep for 5 ms to avoid using all the cpu
//	nanosleep(&timedelay, NULL);
    }
    
    return 0;
}


uint64_t parse_msg_input(msg_descriptor_t* p, 
			 uint64_t** extra_msg_out, size_t* extra_count)
{
    uint64_t ret = 0;

    if (extra_count)
	*extra_count = 0;
    if (extra_msg_out)
	*extra_msg_out = NULL;

    // this is really repetitive and boring stuff
    // assume all arguments are in floats, handle conversation later
    switch (p->identifier)
    {
    case 100:
    {
	sensor_config sc;
	sc.serial = (uint8_t) atof(strtok(NULL, " \t"));
	sc.type = (uint8_t) atof(strtok(NULL, " \t"));
	sc.configuration = (uint8_t) atof(strtok(NULL, " \t"));
	pack_sensor_config(&sc, &ret);     
    }
	break;
    case 200:
    {
	sensor_state ss;
	ss.operation_code = (uint8_t) atof(strtok(NULL, " \t"));
	ss.fault_message = (uint8_t) atof(strtok(NULL, " \t"));
	ss.sensor_health[0] = (uint16_t) atof(strtok(NULL, " \t"));
	ss.sensor_health[1] = (uint16_t) atof(strtok(NULL, " \t"));
	ss.sensor_health[2] = (uint16_t) atof(strtok(NULL, " \t"));
	ss.output_type = (uint8_t) atof(strtok(NULL, " \t"));
	pack_sensor_state(&ss, &ret);
    }
	break;
    case 300:
    {
	message_status ms;
	ms.heartbeat = (uint8_t) atof(strtok(NULL, " \t"));
	pack_message_status(&ms, &ret);
    }
    break;
    case 400:
    {
	position_data_normal pdn;
	int k;
	uint64_t* extra_msg = NULL;
	size_t count = 0;

	for(k=0; k<6; k++)
	    pdn.lateral_pos[k] = (uint16_t) atof(strtok(NULL, " \t"));
	for(k=0; k<6; k++)
	    pdn.time_stamp[k] = (uint16_t) atof(strtok(NULL, " \t"));
	for(k=0; k<6; k++)
	    pdn.polarity[k] = (uint8_t) atof(strtok(NULL, " \t"));
	for(k=0; k<3; k++)
	    pdn.track_number[k] = (uint8_t) atof(strtok(NULL, " \t"));
	    
	pdn.estimated_speed = (uint16_t) atof(strtok(NULL, " \t"));
	pdn.real_speed = (uint16_t) atof(strtok(NULL, " \t"));
	
	for(k=0; k<3; k++)
	    pdn.missing_magnet_flag[k] = (uint8_t) atof(strtok(NULL, " \t"));
	
	pack_position_data_normal(&pdn, &extra_msg, &count);
	ret = extra_msg[count - 1];
	count--;  // the ret is the last message, so 1 less extra_msg

	if (extra_msg_out)
	    *extra_msg_out = extra_msg;
	else
	    free(extra_msg);
	if (extra_count)
	    *extra_count = count;
    }
    break;
    case 500:
    {
	position_data_raw pdr;
	int k;
	uint64_t* extra_msg = NULL;
	size_t count = 0;
	
	for(k=0; k<30; k++)
	    (uint32_t) pdr.magnetic_strengths[k] = atof(strtok(NULL, " \t"));
	
	pdr.vehicle_speed = (uint32_t) atof(strtok(NULL, " \t"));
	
	pack_position_data_raw(&pdr, &extra_msg, &count);
	ret = extra_msg[count - 1];
	count--;

	if (extra_msg_out)
	    *extra_msg_out = extra_msg;
	else
	    free(extra_msg);
	if (extra_count)
	    *extra_count = count;
    }
    break;
    case 600:
    {
	int k = 0;
	position_data_calibration pdc;
	uint64_t* extra_msg = NULL;
	size_t count = 0;
	
	for(k=0; k<30; k++)
	    pdc.magnetic_strengths[k] = (uint32_t) atof(strtok(NULL, " \t"));

	pack_position_data_calibration(&pdc, &extra_msg, &count);
	ret = extra_msg[count - 1];
	count--;

	if (extra_msg_out)
	    *extra_msg_out = extra_msg;
	else
	    free(extra_msg);
	if (extra_count)
	    *extra_count = count;
    }
    break;
    case 700:
    {
	control_computer_status ccs;
	ccs.id = (uint8_t) atof(strtok(NULL, " \t"));
	ccs.status = (uint8_t) atof(strtok(NULL, " \t"));

	pack_control_computer_status(&ccs, &ret);
    }
	break;
    case 800:
    {
	system_command sc;
	sc.command = (uint8_t) atof(strtok(NULL, " \t"));
	sc.target = (uint8_t) atof(strtok(NULL, " \t"));

	pack_system_command(&sc, &ret);
    }
	break;
    case 900:
    {
	data_inputs di;
	di.vehicle_speed = (uint32_t) atof(strtok(NULL, " \t"));
	di.magnet_information = (uint32_t) atof(strtok(NULL, " \t"));

	pack_data_inputs(&di, &ret);
    }
	break;
    case 1000:
    {
	HMI_state hmis;
	hmis.id = (uint16_t) atof(strtok(NULL, " \t"));
	hmis.operation_state = (uint8_t) atof(strtok(NULL, " \t"));
	hmis.heartbeat = (uint8_t) atof(strtok(NULL, " \t"));
	hmis.fault_message = (uint8_t) atof(strtok(NULL, " \t"));
	pack_HMI_state(&hmis, &ret);
    }
	break;
    case 1100:
    {
	HMI_device_state hmids;
	hmids.state = (uint8_t) atof(strtok(NULL, " \t"));
	hmids.devices = (uint16_t) atof(strtok(NULL, " \t"));
	pack_HMI_device_state(&hmids, &ret);
    }
	break;
    case 1200:
    {
	HMI_optional_data od;
	od.recommended_action = (uint32_t) atof(strtok(NULL, " \t"));
	pack_HMI_optional_data(&od, &ret);
    }
	break;
    case 1300:
    {
	CC_state ccs;
	ccs.id = (uint8_t) atof(strtok(NULL, " \t"));
	ccs.state = (uint8_t) atof(strtok(NULL, " \t"));
	ccs.heartbeat = (uint8_t) atof(strtok(NULL, " \t"));
	ccs.fault_message = (uint32_t) atof(strtok(NULL, " \t"));
	pack_CC_state(&ccs, &ret);
    }
	break;
    case 1400:
    {
	CC_operation_state ccos;
	ccos.controller_state = (uint8_t) atof(strtok(NULL, " \t"));
	ccos.transition_state = (uint8_t) atof(strtok(NULL, " \t"));
	ccos.coordination_state = (uint8_t) atof(strtok(NULL, " \t"));
	ccos.reserved_state = (uint8_t) atof(strtok(NULL, " \t"));
	pack_CC_operation_state(&ccos, &ret);
    }
	break;
    case 1500:
    {
	CC_optional_data ccod;
	ccod.steering_command = (uint16_t) atof(strtok(NULL, " \t"));
	pack_CC_optional_data(&ccod, &ret);
    }
	break;
    default:
	fprintf (stderr, "error: unrecognized msg identifer: %d\n",
		 p->identifier);
    }
    
    return ret;
}
