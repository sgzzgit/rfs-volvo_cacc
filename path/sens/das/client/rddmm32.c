/**\file
 * 
 * rddmm32.c - client for das_man compiled with dmm32.o
 *
 * 
 *  Copyright (c) 2009   Regents of the University of California
 *
*/

#include <sys_os.h>
#include <local.h>
#include <sys_rt.h>
#include <das_clt.h>
#include <sys_das.h>

/* Page 1 control (dig I/O) */
#define PORT_A          0x0c
#define PORT_B          0x0d
#define PORT_C          0x0e
#define DIGIO_CONTROL   0x0f

#define PORT_CFG_MODE		0x80
#define PORT_A_DIR_OUT		0x00
#define PORT_A_DIR_IN		0x10
#define PORT_B_DIR_OUT		0x00
#define PORT_B_DIR_IN		0x02
#define PORT_C_HI_DIR_OUT	0x00
#define PORT_C_HI_DIR_IN	0x08
#define PORT_C_LO_DIR_OUT	0x00
#define PORT_C_LO_DIR_IN	0x01

jmp_buf exit_env;

static char *usage= "-d domain -v verbose -w ver verbose -o output mask (1 trace, 2 DB, 4 MySQL, 8 init DB vars, can OR) -n db_number (660 default) -p # inputs to scan";

static void sig_hand(int sig);

static int sig_list[] = 
{
        SIGINT,
        SIGQUIT,
        SIGTERM,
        ERROR,
};

static void sig_hand(int code) {
	longjmp(exit_env, code);
}

int main(int argc, char *argv[]) {

        char *ppath = "/dev/dmm32";
        unsigned long sample_rate = 50;
        char pscan[] = {0,1,2};
        int num_scan = 3;
        int index;
        int ctr = 0;
	int err;
	float pbuf[DAS_MAX_CHANNEL];
	long port_A_data;
	das_typ *pdas = NULL;
        int verbose = 0;
        int veryverbose = 0;
        int option;
	unsigned char outchar = 0;

	while ((option = getopt(argc, argv, "vc:wp:")) != EOF) {
		switch(option) {
		case 'c':
			outchar = atoi(optarg);     
			break;
		case 'v':
			verbose = 1;
			break;
		case 'w':
			verbose = 1;
			veryverbose = 1;
			break;
		case 'p':
			num_scan = atoi(optarg);
			break;
		default:
			printf("Usage: %s %s\n",argv[0], usage);
			exit(EXIT_FAILURE);
			break;
		}
	}

        if (setjmp(exit_env) != 0) {
		if (pdas != NULL) {
			printf("Exiting rddmm32, %d interrupts\n", ctr);
			das_done(pdas);
		}
                exit(EXIT_SUCCESS);
        } else 
                sig_ign(sig_list, sig_hand);

        if ((pdas = das_init(ppath, sample_rate, pscan, num_scan, 0)) 
		== NULL) {
                printf("das_init failed\n");
                exit(EXIT_FAILURE);
	}
	printf("%s: das_init succeeded. pdas %#x fd %d ",
			argv[0], (int)pdas, pdas->fd); 
	printf("sizeof(pdas->volts) %d pdas->result_size %d sizeof(pbuf) %d\n", 
		sizeof(pdas->volts), pdas->result_size, sizeof(pbuf));

	if ((err = DAS_AD_ENQUEUE(pdas)) != EOK) 
		printf("%s :das_ad_enqueue failed error %d\n", argv[0], err);

	port_A_data = PORT_CFG_MODE | PORT_A_DIR_IN | PORT_B_DIR_OUT |
			PORT_C_LO_DIR_IN | PORT_C_HI_DIR_IN;
	printf("%s: port_A_data %#x\n", argv[0], (unsigned int)port_A_data);

	if ((err = DAS_DIGITAL_DIR(pdas, DIGIO_CONTROL, 
			port_A_data)) != EOK) 
		printf("%s: das_digital_dir failed error %d\n", argv[0], err);

	while(1) {	
		if (das_ad_pulse(pdas)) {
			++ctr;
			if (verbose) {
				if ((ctr%50) == 0) {
					for(index = 0; index < num_scan;
						 index++){ 
						printf("%- 3.2f ",
							 pdas->volts[index]);
					}
					printf("\n");
					fflush(stdout);
				}
			}

			if (DAS_DIGITAL_IN(pdas, PORT_A, &port_A_data) == EOK) {
				if (veryverbose) 
					printf("%s: port A input %#x\n", 
						argv[0], 
						(unsigned int)port_A_data);
			}

			if (DAS_DIGITAL_OUT(pdas, PORT_B, outchar, 0, 
					NULL, NULL) == EOK) {
				if (veryverbose) 
					printf("port B output %#x\n", outchar);
			}
		}
	}
}
