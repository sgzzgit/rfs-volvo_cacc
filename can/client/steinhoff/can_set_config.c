/**\file
 *
 * can_set_config.c
 *
 * Currently can only be used with the Steinhoff driver
 *
 * Usage for Steinhoff driver with SJA1000 chips:
 *		can_set_config  -p 1 -b 250 
 *
*/

#include <sys_os.h>
#include <sys/neutrino.h>
#include "local.h"
#include "sys_rt.h"
#include "timestamp.h"
#include "can_defs.h"
#include "can_client.h"

int main(int argc, char **argv)
{
	int fd;
        char *port = "1";	// use "1" to "4" for steinhoff
        int opt;
	int bitspeed;


        while ((opt = getopt(argc, argv, "b:p:")) != -1) {
                switch (opt) {
                  case 'b':
                        bitspeed = atoi(optarg);
                        break;
                  case 'p':
                        port = strdup(optarg);
                        break;
                  default:
                        printf("Usage: %s -p <port> -b <Kbps> \n", argv[0]);
                        exit(1);
                }
        }

	fd = can_open(port, O_RDONLY);

	if (fd == -1)
		exit(EXIT_FAILURE);	// error message printed by can_open 

	printf("%s: opened %s\n", argv[0], port); 
	fflush(stdout);

	printf("%s, device name %s, fd: %d\n", argv[0], port, fd);
	fflush(stdout);

	printf("Original configuration:\n");
	if (!can_print_config(stdout, fd))
		exit(EXIT_FAILURE);
	fflush(stdout);

	if (!can_set_config(fd, bitspeed))
		exit(EXIT_FAILURE);

	printf("Final configuration:\n");
	if (!can_print_config(stdout, fd))
		exit(EXIT_FAILURE);
	fflush(stdout);

	exit(EXIT_SUCCESS);
	
}

