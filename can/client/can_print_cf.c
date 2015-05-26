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
        char *port = "/dev/can1";	// use "1" to "4" for steinhoff
        int opt;

        while ((opt = getopt(argc, argv, "p:")) != -1) {
                switch (opt) {
                  case 'p':
                        port = strdup(optarg);
                        break;
                  default:
                        printf("Usage: %s -p <port>\n", argv[0]);
                        exit(1);
                }
        }

	fd = can_open(port, O_RDONLY);

	if (fd == -1)
		exit(EXIT_FAILURE);	// error message printed by can_open 

	printf("program %s, device name %s, fd: %d\n", argv[0], port, fd);
	fflush(stdout);

	(void) can_print_config(stdout, fd);
	fflush(stdout);

	/* closing and exiting immediately after can_print_config	*/

	can_close(&fd);
	exit(EXIT_SUCCESS);
}
