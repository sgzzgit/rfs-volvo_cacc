#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "sockhop-client.h"

/** \file sub.c
 *
 * Subscriber example for sockhop message server.
 *
 * Suscribes to a given topic and prints each incoming message.
 */

int main(int argc, char *argv[])
{
    int                 sock;
    int                 result;
    int                 buflen          = 1000;
    char                buf[buflen];
    int                 use_tcp         = 0;
    
    if (use_tcp)
        sock = sockhop_connect_topic("127.0.0.1", 30123, "foo");
    else {
        snprintf(buf, 100, "%s/sub-server.sock", getenv("TMPDIR"));
        sock = sockhop_connect_topic(buf, 0, "foo");
    }
    
    if (sock < 0) {
        perror("in main: sockhop_connect_topic()");
        exit(1);
    }
    
    while (1) {
        result = sockhop_recv_message(sock, buf, buflen);
        
        if (result < 0) {
            if (errno == EMSGSIZE)
                printf("message was too long for buffer\n");
            else
                perror("in main: sockhop_recv_message()");
            exit(1);
        }
        
        if (result == 0) {
            printf("server closed\n");
            exit(0);
        }

        buf[result] = 0;
        printf("message: %s\n", buf); // assume null-term string
    }
        
    return 0;
}
