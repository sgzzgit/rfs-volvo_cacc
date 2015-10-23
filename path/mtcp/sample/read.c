#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "sockhop-client.h"

/** \file read.c
 *
 * Read example for sockhop message server.
 *
 * Read the current value of a topic and prints each message.
 */

int main(int argc, char *argv[])
{
    int                 sock;
    int                 result;
    int                 buflen          = 1000;
    char                buf[buflen];
    int                 use_tcp         = 0;
    int                 i;
    
    if (use_tcp)
        sock = sockhop_connect("127.0.0.1", 30125);
    else {
        snprintf(buf, 100, "%s/read-server.sock", getenv("TMPDIR"));
        sock = sockhop_connect(buf, 0);
    }
    
    if (sock < 0) {
        perror("in main: sockhop_connect()");
        exit(1);
    }
    
    for (i=1; i<=10; i++) {
        result = sockhop_send_message(sock, "foo", strlen("foo"));
        if (result < 0) {
            perror("in main: sockhop_send_message()");
            exit(1);
        }

        result = sockhop_recv_message(sock, buf, buflen);
        if (result < 0) {
            if (errno == EMSGSIZE)
                printf("message was too long for buffer\n");
            else if (errno == ENOMSG) {
                printf("no data yet, waiting...\n");
                usleep(1000*1000);
                continue;
            }
            else
                perror("in main: mtcp_recv_message()");
            exit(1);
        }
        
        if (result == 0) {
            printf("server closed\n");
            exit(0);
        }

        buf[result] = 0;
        printf("message: %s\n", buf); // assume null-term string

        usleep(500*1000);
    }
        
    return 0;
}
