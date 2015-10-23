#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#ifndef WIN32
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#endif
#include <errno.h>
#include "mtcp.h"

// ## This test can't run on windows as-is, because it uses fork.

typedef struct {
    int     port;
    size_t  msglen;     // length of message
    int     count;      // number of messages
    int     delay_ms;   // delay between messages
    int     blocking;   // use blocking IO?
    // ## timing params (for sleep, timeout)
} test_param;

void test_mtcp(test_param *ptp);
void do_server(struct sockaddr_in *pservaddr, pid_t child, test_param *ptp);
void do_client(struct sockaddr_in *pservaddr, test_param *ptp);
void receive_messages(int session, test_param *ptp);
void send_messages(int session, test_param *ptp);
char *make_message(size_t mglen);
int test_message(char *ref, size_t reflen, char *msg, size_t msglen);

int main(int argc, char *argv[])
{
    test_param  tp;

#ifdef DEBUG
    fprintf(stderr, "DEBUG mode\n");
#endif
    
    tp.port = 6060;
    
    tp.count = 3;
    tp.delay_ms = 10;
    
    for (tp.blocking = 1; tp.blocking >= 0; tp.blocking--) {
        tp.msglen = 1000;
        tp.port++;
        test_mtcp(&tp);

        tp.msglen = 64*1000;
        tp.port++;
        test_mtcp(&tp);
    }
    
    return 0;
}

void test_mtcp(test_param *ptp)
{
    pid_t child;
    struct sockaddr_in servaddr;
    
    fprintf(stderr, "test_mtcp: started with port=%d, msglen=%d, count=%d, delay_ms=%d,\n",
            ptp->port, ptp->msglen, ptp->count, ptp->delay_ms);
    fprintf(stderr, "test_mtcp:              blocking=%d\n",
            ptp->blocking);
    
    memset((char *)&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(ptp->port);
    inet_aton("127.0.0.1", &(servaddr.sin_addr));
    
    if ((child=fork())) {
        if (child < 0) {
            perror("in test_mtcp: fork failed");
            exit(1);
        }
        
        do_server(&servaddr, child, ptp);
    }
    else {
        do_client(&servaddr, ptp);
        exit(0);
    }

    fprintf(stderr, "test_mtcp: finished\n");
}

void do_server(struct sockaddr_in *pservaddr, pid_t child, test_param *ptp)
{
    int result;
    int server;
    int session;
    struct sockaddr_in peeraddr;
    socklen_t peeraddrlen = 0;

    server = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server <= 0) {
        perror("in do_server: socket() failed");
        exit(1);
    }

    result = bind(server, (struct sockaddr *)pservaddr, sizeof(*pservaddr));
    if (result) {
        perror("in do_server: bind");
        exit(1);
    }

    result = listen(server, 1);
    if (result) {
        perror("in do_server: listen");
        exit(1);
    }

    session = accept(server, (struct sockaddr *)&peeraddr, &peeraddrlen);
    if (session < 0) {
        perror("in do_server: accept");
        exit(1);
    }

    // set session to be non-blocking
    if (!ptp->blocking) {
        int flags;

        flags = fcntl(session, F_GETFL, 0);

        if (flags == -1) {
            perror("in do_server: fcntl: can't get socket flags");
            exit(1);
        }

        fcntl(session, F_SETFL, flags | O_NONBLOCK);
    }

    receive_messages(session, ptp);

    waitpid(child, &result, 0);

    close(server);
}

void do_client(struct sockaddr_in *pservaddr, test_param *ptp)
{
    int sender;
    int result;

    sender = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sender <= 0) {
        perror("in do_client: socket() failed");
        exit(1);
    }

    usleep(100 * 1000); // ## needed?

    result = connect(sender, (struct sockaddr *)pservaddr, sizeof(*pservaddr));
    if (result) {
        perror("in do_client: connect");
        exit(1);
    }
    
    // set sender to be non-blocking
    if (!ptp->blocking) {
        int flags;

        flags = fcntl(sender, F_GETFL, 0);

        if (flags == -1) {
            perror("in do_server: fcntl: can't get socket flags");
            exit(1);
        }

        fcntl(sender, F_SETFL, flags | O_NONBLOCK);
        // ## OK to do this after connect?
        // ## if do it before, then check for EINPROGRESS
    }
    
    usleep(1000 * 1000); // stresses non-blocking case
    
    send_messages(sender, ptp);

    close(sender);
}

char *make_message(size_t msglen)
{
    char *msg = malloc(msglen);
    int i;
    
    for (i = 0; i < msglen; i++) {
        msg[i] = i % 26 + 65;
    }
    
    return msg;
}

int test_message(char *ref, size_t reflen, char *msg, size_t msglen)
{
    char            output[100];

    if (msglen != reflen) {
        fprintf(stderr, "receive_messages: got %d bytes, expecting %d\n",
            msglen, reflen);
        return 1;
    }
    else if (strncmp(ref, msg, msglen)) {
        snprintf(output, msglen < 100 ? msglen : 100, "%s", msg);
        fprintf(stderr, "receive_messages: received message differs\n");
        fprintf(stderr, "first 100 bytes:%s\n", output);
        return 1;
    }
    
    return 0;
}

void receive_messages(int session, test_param *ptp)
{
    mtcp_state      mts;
    char            *buf;
    char            *msg;
    int             result;
    fd_set          readfds, writefds, exceptfds;
    struct timeval  timeout;
    int             i;
    
    msg = make_message(ptp->msglen);
    buf = malloc(ptp->msglen);

    for (i = 0; i < ptp->count; i++) {
        mtcp_init_state(&mts, session);

        while (1) {
            if (!ptp->blocking) {
                // just block using select, since we have nothing else to do
                FD_ZERO(&readfds); FD_ZERO(&writefds); FD_ZERO(&exceptfds);
                FD_SET(session, &readfds);

                timeout.tv_sec = 0;
                timeout.tv_usec = 500*1000;

                result = select(session+1, &readfds, &writefds, &exceptfds, &timeout);
                if (result < 0) {
                    perror("select in receive_messages");
                    exit(1);
                }
                if (result == 0) {
                    fprintf(stderr, "receive_messages: timeout in select(): trying again\n");
                    continue;
                }
            }

            result = mtcp_recv_message(&mts, buf, ptp->msglen);

            if (result < 0) {
                if (!ptp->blocking && errno == EAGAIN) {
                    mtcp_state  *pmts = &mts;
                    
                    // This can still happen, even though select() succeeded,
                    // because mtcp_recv_message may not have gotten the whole msg.
                    fprintf(stderr, "receive_messages: partial data (%d of %d): trying again\n",
                            pmts->bufpos, pmts->msglen ? pmts->msglen : 4);
                    continue;
                }

                perror("receive_messages");
                goto done;
            }
            if (result == 0) {
                fprintf(stderr, "receive_messages: client closed session\n");
                goto done;
            }

            if (test_message(msg, ptp->msglen, buf, result)) {
                goto done;
            }
            
            break;
        }
    }

    fprintf(stderr, "test_mtcp: OK!\n");

done:    
    free(buf);
    free(msg);
}

void send_messages(int session, test_param *ptp)
{
    mtcp_state      mts;
    char            *msg;
    int             result;
    int             i;

    msg = make_message(ptp->msglen);
    
    for (i = 0; i < ptp->count; i++) {
        mtcp_init_state(&mts, session);

again:
        result = mtcp_send_message(&mts, msg, ptp->msglen);

        if (result < 0) {
            if (!ptp->blocking && errno == EAGAIN) {
                fprintf(stderr, "send_messages: partial data (%d of %d): trying again\n",
                        mts.bufpos, ptp->msglen);
                goto again;
            }
            perror("send_messages");
            goto done;
        }

        if (result < ptp->msglen) {
            fprintf(stderr, "send_messages: incomplete send: %d of %d bytes\n",
                    result, ptp->msglen);
            goto done;
        }
        usleep(ptp->delay_ms*1000);
    }
    
done:
    free(msg);
}
