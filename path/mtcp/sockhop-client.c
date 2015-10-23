#include <sys/types.h>

#ifndef WIN32
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include "mtcp.h"

/** \file sockhop-client.c
 *
 * Library file for client access to sockhop message server.
 *
 * See header file, sockhop-client.h, for details.
 * 
 */

int sockhop_connect(char *addr, int port)
{
    int                 sock;
    socklen_t           socklen;
    int                 result;
    union {
        struct sockaddr_in  in;
        struct sockaddr_un  un;
    } servaddr;

    memset((char *)&servaddr, 0, sizeof(servaddr));

    if (port) { // Use tcp.
        servaddr.in.sin_family = AF_INET;
        servaddr.in.sin_port = htons(port);
        inet_aton(addr, &(servaddr.in.sin_addr));
        sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        socklen = sizeof(servaddr.in);
    }
    else {
        servaddr.un.sun_family = AF_UNIX;
        strncpy(servaddr.un.sun_path, addr, 100); //##?
        sock = socket(AF_UNIX, SOCK_STREAM, 0);
        socklen = strlen(servaddr.un.sun_path) + sizeof(servaddr.un.sun_family);
    }

    if (sock <= 0)
        return sock;

    result = connect(sock, (struct sockaddr *)&servaddr, socklen);
    
    if (result)
        return result;
    
    return sock;
}

int sockhop_connect_topic(char *addr, int port, char *topic)
{
    int                 sock;
    int                 result;
    mtcp_state          mts;
    
    sock = sockhop_connect(addr, port);
    if (sock < 0)
        return sock;
    
    mtcp_init_state(&mts, sock);
    result = mtcp_send_message(&mts, topic, strlen(topic));
    if (result < 0)
        return result;
    
    return sock;
}

int sockhop_recv_message(int sock, void *buf, size_t len)
{
    mtcp_state          mts;
    
    mtcp_init_state(&mts, sock);
    return mtcp_recv_message(&mts, buf, len);
}

int sockhop_send_message(int sock, void *buf, size_t len)
{
    mtcp_state          mts;
    
    mtcp_init_state(&mts, sock);
    return mtcp_send_message(&mts, buf, len);
}
