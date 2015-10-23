#include "mtcp.h"

#include <fcntl.h>
#ifdef WIN32
#include <winsock.h>
#else
#include <netinet/in.h>
#include <sys/socket.h>
#endif
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

static ssize_t recv_buf(int s, char *buf, size_t *bufpos, size_t len, int blocking);
static ssize_t send_buf(int s, const char *buf, size_t *bufpos, size_t len, int blocking);

void mtcp_init_state(mtcp_state *pmts, int s)
{
    pmts->sock                  = s;
    pmts->msglen                = 0;
    pmts->bufpos                = 0;
    *(uint32_t*)pmts->lenbuf    = 0;
    pmts->lenbufpos             = 0;
}

ssize_t mtcp_recv_message(mtcp_state *pmts, void *buf, size_t len)
{
    int         blocking;
    ssize_t     result;
    
    blocking = mtcp_get_block_state(pmts);
    if (blocking < 0) {
        return blocking;
    }
    
    // make sure we know how long the message is
    if (!pmts->msglen) {
        // try to recv the 32-bit length field
        result = recv_buf(pmts->sock, pmts->lenbuf, &pmts->lenbufpos, 4, blocking);
        if (result <= 0) {
            return result;
        }
        pmts->msglen = ntohl(*(uint32_t*)pmts->lenbuf);
    }
        
    if (pmts->msglen > len || pmts->msglen < 0) {
        errno = EMSGSIZE;
        return -1;
    }
    else if (pmts->msglen == 0) {
        errno = ENOMSG;
        return -1;
    }
    
    // try to recv the message itself
    result = recv_buf(pmts->sock, buf, &pmts->bufpos, pmts->msglen, blocking);
    return result;
}

ssize_t mtcp_send_message(mtcp_state *pmts, const void *buf, size_t len)
{
    int         blocking;
    ssize_t     result;
    
    blocking = mtcp_get_block_state(pmts);
    if (blocking < 0) {
        return blocking;
    }
    
    if (pmts->lenbufpos < 4) {
        if (pmts->lenbufpos == 0) {
            *(uint32_t*)pmts->lenbuf = htonl(len);
        }
        result = send_buf(pmts->sock, pmts->lenbuf, &pmts->lenbufpos, 4, blocking);
        if (result <= 0) {
            return result;
        }
    }
    
    result = send_buf(pmts->sock, buf, &pmts->bufpos, len, blocking);
    return result;
}

int mtcp_get_block_state(mtcp_state *pmts)
{
#ifdef WIN32
    pmts;
    return 1;
#else
    int flags;

    flags = fcntl(pmts->sock, F_GETFL, 0);

    if (flags == -1) {
        perror("mtcp_get_block_state: can't get socket flags");
        return(flags);
    }
    
    return !(flags & O_NONBLOCK);
#endif
}

static ssize_t recv_buf(int s, char *buf, size_t *bufpos, size_t len, int blocking)
{
    ssize_t     result;
    
#ifdef DEBUG
    if (*bufpos > len || *bufpos < 0) {
        fprintf(stderr, "recv_buf: *bufpos = %d, buflen = %d\n", *bufpos, len);
        exit(1);
    }
#endif

    while (*bufpos < len) {
        result = recv(s, buf + *bufpos, len - *bufpos, 0);
        if (result <= 0) {
            return result;  // closed normally, or error (including EAGAIN)
        }

        *bufpos += result;
        if (!blocking && *bufpos < len) {
            errno = EAGAIN;
            return -1;
        }
    }

    return *bufpos;
}

static ssize_t send_buf(int s, const char *buf, size_t *bufpos, size_t len, int blocking)
{
    ssize_t     result;
    
#ifdef DEBUG
    if (*bufpos > len || *bufpos < 0) {
        fprintf(stderr, "send_buf: *bufpos = %d, buflen = %d\n", *bufpos, len);
        exit(1);
    }
#endif

    result = send(s, buf + *bufpos, len - *bufpos, 0);
    if (result <= 0) {
        return result;  // closed normally, or error (including EAGAIN)
    }

    *bufpos += result;
    if (*bufpos < len) {
        if (blocking) {
#ifdef DEBUG
            fprintf(stderr, "send_buf: blocking send did not finish!\n");
            exit(1);
#endif
        }
        else {
            errno = EAGAIN;
            return -1;
        }
    }

    return *bufpos;
}
