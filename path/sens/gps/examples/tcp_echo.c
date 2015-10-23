/**\file
 * 	TCP echo server, listens on port and writes output to file
 *
 *	Handles only one client on a port. For debugging.
 *
 *	Sue Dickey April 2008
 */

#include <sys_os.h>

/** Don't know what is really a good value for this.
 */
#define MAXPENDING 3

int main(int argc, char **argv)

{
	int serv_sd = -1;
	int client_sd;
	struct sockaddr_in serv_addr;
	struct sockaddr_in client_addr;
	unsigned short serv_port = 2946;
	char receive_buffer[1500];	
	int rcvd;		/// count received from a call to recv 
	int byte_count = 0;	/// count received in total from a client
	int fd;			/// file descriptor for ouput
	char *file_name = "port.out";
	int loop_count = 0;
	int perm = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
	int opt;
	int verbose = 0;

	while ((opt = getopt(argc, argv, "f:p:v")) != -1) {
                switch (opt) {
		case 'f':
			file_name = strdup(optarg);
			break;
		case 'p':
			serv_port = atoi(optarg);
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			printf("Usage: serv -p [port] ");
			exit(1);
                }
        }
	printf("Echoing port %d\n", serv_port);

	if ((fd = open(file_name, O_RDWR|O_CREAT, perm)) < 0) {
		perror(file_name);
		exit (1);
	}


	if ((serv_sd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		perror("TCP socket create failed");
		exit(1);
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(serv_port);
	if (verbose)
		printf("s_addr 0x%x, sin_port 0x%x\n", 
			serv_addr.sin_addr.s_addr,
			serv_addr.sin_port); 
	if (bind(serv_sd, (struct sockaddr *) &serv_addr,
					 sizeof(serv_addr)) < 0){
		perror("TCP socket bind failed");
		exit(1);
	}

	if (listen(serv_sd, MAXPENDING) < 0) {
		perror("TCP listen failed");
		exit(1);
	}

	for (;;) {
		unsigned int accept_length = sizeof(client_addr);
		printf("Waiting for client\n");
		if ((client_sd = accept(serv_sd,
					(struct sockaddr *) &client_addr,
					&accept_length)) < 0) {
			perror("Client accept failed");
			exit(1);
		}
		printf("Handling client %s\n", inet_ntoa(client_addr.sin_addr));

		while (1) {
			loop_count++;
			rcvd = recv(client_sd, 
				&receive_buffer[0],
				sizeof(receive_buffer), 0); 
			if (rcvd < 0) { 
				perror("Client receive error");
				break;;
			}
			if (verbose) 
				printf("Received %d total %d loop count %d\n",
					rcvd, byte_count, loop_count);
			byte_count += rcvd; 	
			write(fd, receive_buffer, rcvd);
		}
		close(client_sd);				
	}
}
