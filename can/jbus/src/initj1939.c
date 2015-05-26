/* Initializes a B&B J1939 STB converter by reading a stream of messages
 * from standard input and sending this as a stream of external messages 
 * to the converter to set the protocol bits in the buffer's slots. 
 *
 * static char rcsid[] = "$Id"
 *
 * "$Log"
 *
 */

#include "std_jbus_extended.h"

#undef DEBUG

int 
main (int argc, char **argv)
{
	char buffer[80];
	struct j1939_pdu pdu;		/* Protocol Data Unit */
	int dummyR, dummyDP;		/* for sscanf */
	int data[8];			/* for sscanf */
	int n = 0;
	int state_code;
	int slot_number;
	int fd;
	int i;
	int ch;
	char *filename;

	fd = STDOUT_FILENO;
        while ((ch = getopt(argc, argv, "cf:")) != EOF) {
                switch (ch) {
                case 'f': filename = optarg;
			  fd = open(filename, O_WRONLY);
			  if (fd < 0) {
				fprintf(stderr, "open error, %s fd %d\n", filename, fd);
				exit(1);
		          }
                          break;
                default: printf(
			"Usage: %s [-f output file name]\n",
					 argv[0]);
			  exit(1);
                          break;
                }
        }

	/* message to place in converter's buffer is given on stdin */
	while (fgets(buffer, 80, stdin)){
		sscanf(buffer,
			 "%i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i", 
			&state_code,
			&pdu.priority, &dummyR, &dummyDP,
			&pdu.pdu_format, &pdu.pdu_specific,
			&pdu.src_address, &pdu.numbytes,
			&data[0], &data[1], &data[2], &data[3],
			&data[4], &data[5], &data[6], &data[7]);
		pdu.R = (unsigned char) (dummyR & 0xff);	
		pdu.DP = (unsigned char) (dummyDP & 0xff);	
		for (i = 0; i < 8; i++)
			pdu.data_field[i] = (unsigned char) data[i] & 0xff;
		slot_number = n % 15 + 1;
		send_stb_external(fd, &pdu, state_code, slot_number);
#ifdef DEBUG
		fprintf(stderr, "slot %d, PF %d PS %d SRC %d\n",
			slot_number, pdu.pdu_format, pdu.pdu_specific,
			pdu.src_address);
#endif
		n++;
	}
	close(fd);
	return 0;
}			
