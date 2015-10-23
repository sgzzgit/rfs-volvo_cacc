/**
 * Send time to remote system
 */
#include <sys_os.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <local.h>
#include <sys_rt.h>
#include <udp_utils.h>
#include "clock_set.h"

extern unsigned int src_ip_for_checksum;

int main(int argc, char **argv)

{
	int sd;
	struct sockaddr_in snd_addr;
	unsigned short snd_port = CLOCK_SET_PORT; // not used for WAVE
	char *snd_ip = "192.168.20.1";
	char *self_ip = "192.168.20.13";
	int num_sends = 0;	// continuous send
	int bytes_sent = 0;	// returned from sendto 
	int timer_interval = 100; // every 100 milliseconds
	int message_length = 256;// pad after formatted string if non-zero
	struct timespec tm_spec;
	int socket_option = 0;  // unicast socket
	int opt;
	int i;
	int chid;	// channel ID for timer
	posix_timer_typ *ptmr;	// pointer to timer
	int do_logging = FALSE;
	int tx_activate = 0;	// if 1, TX options activated and
				// channel cannot be changed
	int channel = 178;	// control channel default
	int power = 10;		// dbm power
	int rate = 2;		// (2 == 6Mbps for 10MHz channel) 
	int do_broadcast = 0;
#ifdef WAVE
	sWAVE_Tx_Option tx_option_pkt;
#endif

	while ((opt = getopt(argc, argv, "Aa:b:m:n:P:p:r:t:v")) != -1) {
                switch (opt)
                {
                  case 'a':
                        snd_ip = strdup(optarg);
                        break;
                  case 'b':
                        do_broadcast = 1;
                        break;
                  case 'm':
                        message_length = atoi(optarg);
                        break;
                  case 'n':
                        num_sends = atoi(optarg);
                        break;
                  case 'p':
                        snd_port = atoi(optarg);
                        break;
		  case 't':
			timer_interval = atoi(optarg);
			break;
		  case 'v':
			do_logging = TRUE; 
			break;
                  default:
                        printf("Usage: clock -a [dest IP] -b [broadcast]");
			printf(" -m [message length] -n [number of sends]");
			printf(" -s [self ip string] -t timer interval\n");
                        exit(1);
                }
        }
	if (clock_getres(CLOCK_REALTIME,&tm_spec) == -1) {
	      perror("clock get resolution");
	      exit(1);
	}
	printf("Resolution is %ld micro seconds.\n",tm_spec.tv_nsec/1000);

	if (do_broadcast){ 
		if ((sd = udp_broadcast()) < 0) exit(1);
	} else if ((sd = udp_unicast()) < 0) exit(1); 

	chid = ChannelCreate(0);

        if ((ptmr = timer_init(timer_interval, chid)) == NULL) {
               printf("timer_init failed\n");
               exit(1);
        }

	memset(&snd_addr, 0, sizeof(snd_addr));
	snd_addr.sin_family = AF_INET;
	snd_addr.sin_addr.s_addr = inet_addr(snd_ip);
	snd_addr.sin_port = htons(snd_port);

        printf("send address to ip 0x%x, port %d initialized\n",
                        ntohl(snd_addr.sin_addr.s_addr),
                        ntohs(snd_addr.sin_port));

	for (i = 0; i < num_sends || num_sends == 0; i++) {
		char snd_buffer[RBUFSIZE];
		if (clock_gettime(CLOCK_REALTIME,&tm_spec) == -1 ) {
			 perror("clock gettime");
			 exit(1);
		}
		sprintf(snd_buffer, "%d %d %u %lu", 
					i, CLOCK_SND_MSG_TYPE,
				tm_spec.tv_sec, tm_spec.tv_nsec); 

		if (message_length > 0) {
			int min = strlen(snd_buffer);
			int j;

			for (j = min;
				j < message_length-1 && j < RBUFSIZE-1; j++) {
				// insert space after formatted string
				if (j == min) 
					snd_buffer[j] = ' ';
				else
					snd_buffer[j] = j % 10 + '0';
			}
			snd_buffer[j] = '\0'; //mark end of string
		}

		bytes_sent = sendto(sd, snd_buffer, strlen(snd_buffer)+1, 0,
				(struct sockaddr *) &snd_addr,
				sizeof(snd_addr)); 
		if (bytes_sent < 0)
			perror("sendto:");
		else if (do_logging)
			printf("sent %d bytes, strlen %d\n", bytes_sent,
					 strlen(snd_buffer)+1);

		if (timer_interval > 0)
			TIMER_WAIT(ptmr);

	}
	fprintf(stderr, "%s completed\n", argv[0]);
	return 0;
}
