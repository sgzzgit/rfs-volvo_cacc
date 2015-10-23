/**\file
 *
 *	Process to read GPS stream and forward only RMC messages 
 *
 *      Copyright (c) 2008 Regents of the University of California
 *
 *	Author: Sue Dickey
 */
#include <sys_os.h>

static int sig_list[]=
{
	SIGINT,
	SIGQUIT,
	SIGTERM,
	SIGALRM,	// for timer
	(-1)
};
static jmp_buf exit_env;
static void sig_hand(int code)
{
	if (code == SIGALRM)
		return;
	else
		longjmp(exit_env, code);
}

#define BUFFER_SIZE 1440

/** Open UDP socket for sending to a unicast address
 *  and initialize sock_addrin structure
 */
int udp_init(char *ipaddr, short port, struct sockaddr_in *paddr)
{
        int sockfd;

        if ((sockfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
                perror("socket");
                return -1;
        }
        memset(paddr, 0, sizeof(struct sockaddr_in));
        paddr->sin_family = AF_INET;       // host byte order
        paddr->sin_port = htons(port);     // short, network byte order
        paddr->sin_addr.s_addr = inet_addr(ipaddr);
        memset(&(paddr->sin_zero), '\0', 8); // zero the rest of the struct

        return sockfd;
}

int main (int argc, char** argv)
{
	int opt;
	char *ip_addr = "192.168.1.2";
	int port;
	int verbose = 0;
	char in_buf[BUFFER_SIZE];
	int ret;
	int i;
	int sd = -1;	/// socket descriptor	
	struct sockaddr_in snd_addr;
	int bytes_sent;

	while ((opt = getopt(argc, argv, "a:u:v")) != -1) {
		switch (opt) {
		case 'a':
			ip_addr = strdup(optarg);
			break;
		case 'u':
			port = atoi(optarg);
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			printf( "Usage: %s ", argv[0]);
			printf("-a ip address -u udp port -v verbose\n");
			exit(1);
		}
	}
	if (verbose) {
		printf("%s %d", ip_addr, port);
		fflush(stdout);
	}
	
	sd = udp_init(ip_addr, port, &snd_addr);

	/** Upon signal or jmp from error condition, deactivate if active,
	 *  unregister and close api before exiting.
	 */ 
	if (setjmp(exit_env) != 0) {
		if (sd > 0)
			close(sd);
		fprintf(stderr, "Exiting %s, return value %d\n", argv[0], ret);
		exit(0);
	} else {
                int i = 0;
                while (sig_list[i] != -1) {
                        signal(sig_list[i], sig_hand);
                        i++;
                }
	}

	memset(in_buf, 0, BUFFER_SIZE);
	/// Safe to print in_buf and use strlen because full
	/// buffer was initialized to 0, final character always
	/// terminates string. 
	while (fgets(in_buf, BUFFER_SIZE-1, stdin) != NULL) {
		char tmpstr[7];
		memset(&tmpstr[0], 0, 7);
		memcpy(&tmpstr[0], &in_buf[0], 6);		
		if (strncmp(&tmpstr[0],"$GPRMC", 7) == 0) {
			if (verbose) 
				printf("%s\n", in_buf);
			bytes_sent = sendto(sd, in_buf,
                                        strlen(in_buf), 0,
                                        (struct sockaddr *) &snd_addr,
                                        sizeof(snd_addr));

                        if (bytes_sent < 0) {
                                perror("sendto error");
                                printf("port %d addr 0x%08x\n",
                                        ntohs(snd_addr.sin_port),
                                        ntohl(snd_addr.sin_addr.s_addr));
                                fflush(stdout);
			}
		}
		memset(in_buf, 0, BUFFER_SIZE);
	}
	longjmp(exit_env, 1);	 
}

