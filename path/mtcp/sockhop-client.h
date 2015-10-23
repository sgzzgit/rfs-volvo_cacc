/** \file sockhop-client.h
 *
 * Library file for client access to sockhop message server. There are three
 * access modes for clients:
 *
 *  - subscribe  - client receives all new messages on a topic
 *  - publish    - client sends new messages on a topic
 *  - read       - client sends request for most recent message on a topic, and
 *                receives the message.
 *
 * Clients call one of the following function to initiate access, depending
 * on the type of access:
 *
 *   sockhop_connect:       connect to sockhop server
 *   sockhop_connect_topic: connect to sockhop server and register interest
 *                          in a topic
 * 
 * For publish or subscribe, use sockhop_connect_topic(). All further
 * access will be limited to that topic.
 *
 * For read access, use sockhop_connect() directly, and specify the topic
 * for each request.
 *
 * The client then needs to call one function per message sent or received:
 *
 *   sockhop_send_message: for publisher
 *   sockhop_recv_message: for subscriber
 *
 * For read access, alternate between sockhop_send_message() to send the
 * topic, and sockhop_recv_message() to receive the message on the topic.
 *
 * Note that read access doesn't block if there is no message on the topic.
 * However, subscribe access does block in that case. See sockhop_recv_message()
 * for details.
 *
 * Note that no state (aside from the socket file descriptor) needs to be
 * maintained by the client. This means that the non-blocking functionality
 * of mtcp cannot be used through this API. However, mtcp can be used
 * directly with a sockhop socket if this functionality is desired.
 */

/** \brief connect to a server, before reading
 *
 * \param addr          address of server, either file path or IP (see below)
 * \param port          port, or 0 (see below)
 * \return              sock fd, or -1 as in socket(), connect(), and send()
 *
 * Initialize a socket for publishing, subscribing, or reading, depending on the
 * address of the server. The server has a different address for each mode.
 *
 * For pub and sub, the caller must send a topic string after connecting.
 * Use sockhop_connect_topic() to do this in one call.
 *
 * For read access, the caller alternates sending topic strings, and receiving
 * the associated values.
 * 
 * If the port is 0, treats addr as path to Unix socket. If port>0, treats
 * addr as INET, e.g. "127.0.0.1".
 *
 * Limitation: topic must be null-terminated str (the sockhop server
 * supports arbitrary data in the topic, but this should not be used).
 *
 */
int sockhop_connect(char *addr, int port);

/** \brief connect to a server a given topic, before publising or subscribing
 *
 * \param addr          address of server, either file path or IP (see below)
 * \param port          port, or 0 (see below)
 * \param topic         topic string
 * \return              sock fd, or -1 as in socket(), connect(), and send()
 *
 * Initialize a socket for publishing or subscribing, depending on the
 * address of the server. The server has a different address for each mode.
 * 
 * If the port is 0, treats addr as path to Unix socket. If port>0, treats
 * addr as INET, e.g. "127.0.0.1".
 *
 * Limitation: topic must be null-terminated str (the sockhop server
 * supports arbitrary data in the topic, but this should not be used).
 *
 */
int sockhop_connect_topic(char *addr, int port, char *topic);

/** \brief receive a message on a subscribed topic
 *
 * \param sock          file descriptor of socket returned by sockhop_connect
 * \param buf           as in recv()
 * \param len           as in recv()
 * \return              as in recv(), but see below
 *
 * Receives a message from a socket (in either subscribe mode or read mode),
 * and copy it to the buffer, returning the length of the data.
 *
 * On failure, return value is -1 and errno is left as set by recv(), with
 * two additional possible errors, EMSGSIZE, indicating that the message is
 * too long for buf, and ENOMSG, indicating that no data has been written to
 * the topic yet. (ENOMSG only applies in the case of _read_ access, not
 * _subscribe_ access, which blocks when no message is available.)
 *
 * Return value of 0 means peer has closed its socket. Positive return value
 * is the length of the message copied into buf.
 */
int sockhop_recv_message(int sock, void *buf, size_t len);

/** \brief send a message on a subscribed topic
 *
 * \param sock          file descriptor of socket returned by sockhop_connect
 * \param buf           as in recv()
 * \param len           as in recv()
 * \return              as in recv(), but see below
 *
 * Send a message to a socket (in either publishe mode or read mode) from
 * data stored in the buffer.
 *
 * On failure, return value is -1 and errno is left as set by recv().
 * Return value of 0 means peer has closed its socket. Positive return value
 * is the length of the message copied into buf.
 */
int sockhop_send_message(int sock, void *buf, size_t len);
