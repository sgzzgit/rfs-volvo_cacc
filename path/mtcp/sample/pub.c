#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "sockhop-client.h"

/** \file pub.c
 *
 * Publisher example for sockhop message server.
 *
 * Publishes messages on a given topic.
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
        sock = sockhop_connect_topic("127.0.0.1", 30124, "foo");
    else {
        snprintf(buf, 100, "%s/pub-server.sock", getenv("TMPDIR"));
        sock = sockhop_connect_topic(buf, 0, "foo");
    }
    
    if (sock <= 0) {
        perror("in main: sockhop_connect_topic()");
        exit(1);
    }

    for (i=1; i<=10; i++) {
        sprintf(buf, "test from pub.c %d", i);
        result = sockhop_send_message(sock, buf, strlen(buf)); // assume null-term string
        if (result < 0) {
            perror("in main: sockhop_send_message()");
            exit(1);
        }
        usleep(500*1000);
    }
        
    return 0;
}
