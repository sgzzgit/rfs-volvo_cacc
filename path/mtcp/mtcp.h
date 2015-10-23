#ifndef __MTCP_H__
#define __MTCP_H__

#include <sys/types.h>

#ifdef WIN32
#include <windows.h>
typedef int ssize_t;
#define EMSGSIZE WSAEMSGSIZE
#endif

/*! \mainpage MTCP and Sockhop
 *
 * \section mtcp_sec MTCP
 *
 * MTCP (Messaging TCP) is a library for sending and receiving
 * datagrams over a stream socket (e.g., TCP or UNIX) using a very
 * simple protocol. The data may be of any length, with any contents
 * (binary data). Transmission has the same reliability
 * characteristics as TCP. Details are in mtcp.h and the doxygen
 * documentation.
 *
 * The Makefile handles build, test and documentation tasks. Briefly:
 *
 * \code
 * make      -- build everything
 * make test -- build everything and run tests
 * make docs -- build docs indexed in doc/html/index.html
 * \endcode
 *
 * The files mtcp.[ch] are enough to use mtcp. However, the protocol
 * can easily be used without these files. It is simply this:
 *
 * To send a message of N bytes, first send the number N encoded as
 * an unsigned 4 byte integer in network (big-endian) byte order.
 * Then send the N bytes of data. The receiver must first parse the
 * length and then read that many more bytes. (The length does not
 * include the length field itself. For example, to send one
 * character of data, the 4 bytes length field must contain the
 * number 1.)
 *
 * Note about porting: the implementation in mtcp.c is more
 * complicated than necessary for most MTCP use, because it handles
 * nonblocking sockets and winsock. Implementing the above protocol
 * without these cases is much simpler.
 *
 * See test-mtcp.c (particularly, receive_messages() and
 * send_messages()) for an another (more complex) example of usage.
 *
 * MTCP has worked on windows, but test_mtcp will not (it uses fork).
 *
 * \section sockhop_sec Sockhop
 *
 * Sockhop is a socket client/server protocol for publish/subscribe
 * IPC. The sockhop protocol is based on mtcp:
 *
 * - to subscribe, send the topic as a message, then receive data
 * messages
 *
 * - to publish, send the topic, then send data messages
 *
 * - to read the last value, send the topic, receive a data message,
 * and repeat
 *
 * Rather than use mtcp driectly, it is easier to use the interface
 * defined in sockhop-client.h. See the sample/ dir for sample
 * clients for the above uses. The server is currently implemented in
 * ruby--see the sockhop package.
 */

/** \file mtcp.h
 *
 * Message TCP (MTCP).
 *
 * Protocol based on TCP that provide a message (reliable datagram)
 * abstraction (MTCP)on top of the continuous data streams of TCP.
 *
 * open(), bind(), listen(), accept(), connect(), close() are all used
 * the same as usual to manage the socket.
 *
 * The new functions mtcp_recv_message() and mtcp_send_message() wrap around
 * recv() and send() to manage message discretization.
 */

/** \brief MTCP state
 *
 * State of current MTCP transmission. Only the socket file descriptor member
 * should be accessed directly by callers of the MTCP API. The struct should
 * be initialized with the mtcp_init_state() function, which takes the socket
 * as an argument. A clean mtcp_state should be used for each message. The
 * struct should be reinitialized with mtcp_init_state() between messages
 * (but not between several calls to send or recv a single message with a
 * non-blocking socket). Each thread should use a different struct.
 *
 * Note: non-blocking IO is \b not \b implemented on Windows (use threads).
 */
typedef struct {
    int             sock;
    size_t          msglen;
    size_t          bufpos;
    char            lenbuf[4];
    size_t          lenbufpos;
} mtcp_state;

/** \brief initialize MTCP state
 *
 * Should be called for each message sent. (If the socket is non-blocking,
 * do not reinitialize for repeated send/recv calls on the same message.)
 *
 * \param pmts      the mtcp_state to be used
 * \param s         the socket file descriptor that it will use.
 *
 * It is up to the caller to initialize the socket s itself (including the
 * connect() call, setsockopt() with O_NONBLOCK, etc.).
 */
extern void mtcp_init_state(mtcp_state *pmts, int s);

/** \brief receive a single message on a TCP socket using the MTCP protocol
 *
 * \param pmts          pointer to the MTCP params (struct given by caller)
 * \param buf           as in recv()
 * \param len           as in recv()
 * \return              as in recv(), but see below
 *
 * Arguments are as in the standard recv() (see 'man 2 recv'), except that the
 * socket argument is wrapped in a mtcp_state struct and that that no
 * flags argument is used. The same mtcp_state struct should be used for all 
 * mtcp_recv_message calls on this socket (but use a different one for send
 * calls).
 *
 * The difference is that the stream is chopped into discrete messages using
 * length fields (4 octets each, stored in network order). The value of length
 * field is the length of the subsequent message, exclusive of the length
 * field itself. Each call to mtcp_recv_message() returns at most one such
 * message, even if more data is available. If no complete message is
 * available, the call blocks until a complete message is available, unless
 * the socket was placed in non-blocking mode. The length field is not
 * accessible to callers of the MTCP API -- it is managed internally.
 *
 * The return value is as usual: 0 means peer has closed its socket, -1 means
 * an error, and a positive number is the length of the message copied into
 * buf. Error codes are in errno. Two additional error codes are generated,
 * beyond what recv() generates: EMSGSIZE is generated if the received message
 * has a length field that indicates that the message is too long for buf;
 * ENOMSG is generated if the received message has a zero length field.
 * As usual, EAGAIN in the non-blocking case indicates that the function
 * should be called again to finish the receive operation, using the same pmts.
 */
extern ssize_t mtcp_recv_message(mtcp_state *pmts, void *buf, size_t len);

/** \brief send a single message on a TCP socket using the MTCP protocol
 *
 * \param pmts          pointer to the MTCP params (struct given by caller)
 * \param buf           as in send()
 * \param len           as in send()
 * \return              as in send()
 *
 * Arguments are as in the standard send() (see 'man 2 send'), except that the
 * socket argument is wrapped in a mtcp_state struct and that that no
 * flags argument is used.
 *
 * The difference is that the stream is chopped into discrete messages using
 * length fields (as described at mtcp_recv_message()). Each call to
 * mtcp_send_message() sends at most one such message. The length field is not
 * accessible to callers of the MTCP API--it is managed internally.
 *
 * The return value is as usual: -1 means an error, and a positive number is
 * the length of the message sent. Error codes are in errno.
 * As usual, EAGAIN in the non-blocking case indicates that the function
 * should be called again to finish the send operation, using the same pmts.
 */
extern ssize_t mtcp_send_message(mtcp_state *pmts, const void *buf, size_t len);

/** \brief Get the blocking/non-blocking state of the socket.
 *
 * \param pmts          pointer to the MTCP params (struct given by caller)
 * \return             1 if blocking, 0 if non-blocking.
 */
int mtcp_get_block_state(mtcp_state *pmts);

#endif // __MTCP_H__
