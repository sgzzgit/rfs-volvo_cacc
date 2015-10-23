/**
 *	TCP client test program. 
 *	 
 *	Connects and optionally sends a string to a server,
 *	then calls recv and echos output until killed by a signal.
 *
 *	Can be used for checking gpsd operation, as well as
 *	many other TCP apps where a query or simple set-up message
 *	precedes reception.	
 *
 *	Either prints bytes returned from each recv call to stdout,
 *	with a newline between each group, or copies bytes exactly
 *	to a named file.
 *
 *	Can be killed with a signal (e.g. CTRL-C) and then prints
 *	the total number of bytes it received.
 */

#include <sys_os.h>
#include <sys_rt.h>
#include <timing.h>

jmp_buf exit_env;
static void sig_hand(int sig);

static int sig_list[] = {
        SIGINT,
        SIGQUIT,
        SIGTERM,
        SIGALRM,
        ERROR,
};

static void sig_hand( int code )
{
        if (code == SIGALRM)
                return;
        else
                longjmp( exit_env, code );
}

int main(int argc, char **argv)
{
	int sd;
	struct sockaddr_in serv_addr;
	unsigned short serv_port = 2947;	// default for gpsd
	char *serv_ip = "127.0.0.1";		// local interface
	char receive_buffer[256];	
	char *send_str = NULL;
	int fdout;			// use to save copy
	char *exact_copy = NULL;	// filename to save copy
	int perm = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
	int opt;
	int do_timing = 0;
	struct avcs_timing timing;
	int i;
	int byte_count = 0;
	int rcvd = 0;	// bytes received value returned from recv on socket 

	while ((opt = getopt(argc, argv, "a:c:p:q:t")) != -1)
        {
                switch (opt)
                {
                  case 'a':
                        serv_ip = strdup(optarg);
                        break;
                  case 'c':
                        exact_copy = strdup(optarg);
                        break;
                  case 'p':
                        serv_port = atoi(optarg);
                        break;
                  case 't':
                        do_timing = 1;
                        break;
                  case 'q':
                        send_str = strdup(optarg);
                        break;
		
                  default:
                        printf("Usage: tcp_client -a [server IP] ");
			printf("-t <turn on timing> ");
			printf("-q [string to send] -p [port]\n");
                        exit(1);
                }
        }
	if (send_str != NULL) {
		printf("Sending %s, to %s, port %d\n", send_str, 
			serv_ip, serv_port);
	}
	if ((sd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		printf("TCP socket create failed\n");
		exit(1);
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(serv_ip);
	serv_addr.sin_port = htons(serv_port);
	if (connect(sd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
		printf("TCP socket connect failed\n");
		perror(NULL);
		exit(1);
	}
	if (exact_copy != NULL) {
		if ((fdout = open(exact_copy,O_RDWR|O_CREAT, perm)) < 0) { 
			perror(argv[0]);
			exit (1);
		}
	}
	if (do_timing)
		avcs_start_timing(&timing);

	if (setjmp(exit_env) != 0) {
		avcs_end_timing(&timing);
		avcs_print_timing(stderr, &timing);
                shutdown(sd, SHUT_RDWR);
		printf("tcp_client exits, received %d bytes\n", byte_count); 
                exit(EXIT_SUCCESS);
        } else
               sig_ign(sig_list, sig_hand);

	if (send_str != NULL) {
		if ((i=send(sd, send_str, strlen(send_str), 0)) !=
				strlen(send_str)) {
			printf("sent wrong number of bytes %d for %s\n",
				 i, send_str);
			exit(1);
		} 
	}

	/** Receive and echo, with extra line feed after each receive
	 */
	while (1) {
		timing.exec_num++;
		if ((rcvd = recv(sd, receive_buffer, 255, 0)) <= 0) {
				printf("error receiving length from serv\n");
				exit(1);
		}
		receive_buffer[rcvd] = '\0';
		if (exact_copy) {
			if (write(fdout, receive_buffer, rcvd) < 0) {
				perror(argv[0]);
				exit (1);
			}
		} else {
			printf(receive_buffer);		
			printf("\n");
			fflush(stdout);
		}
		byte_count += rcvd;
	}
}
