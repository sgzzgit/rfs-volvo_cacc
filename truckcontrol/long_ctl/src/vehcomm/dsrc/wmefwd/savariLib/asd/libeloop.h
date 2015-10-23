/**
 * @file libeloop.h
 * @brief This library defines an event loop interface that supports processing
 * events from registered timeouts (i.e., do something after N seconds), sockets
 * (e.g., a new packet available for reading), and signals. This library's
 * implementation is based on using select() and sockets. This is
 * suitable for most UNIX/POSIX systems.
 * See sample program for details
 * \n
 * Link with -leloop.
 */

#ifndef ELOOP_H
#define ELOOP_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * ELOOP_ALL_CTX - eloop_cancel_timeout() magic number to match all timeouts
 */
#define ELOOP_ALL_CTX (void *) -1

/**
 * eloop socket event type for eloop_register_sock()
 */
typedef enum {
        /**
         * Socket has data available for reading
         */
	EVENT_TYPE_READ = 0,
        /**
         * Socket has room for new data to be written
         */
	EVENT_TYPE_WRITE,
        /**
         * An exception has been reported
         */
	EVENT_TYPE_EXCEPTION
} eloop_event_type;

/**
 * @brief eloop socket event callback type
 * \param sock File descriptor number for the socket
 * \param eloop_ctx Registered callback context data (eloop_data)
 * \param sock_ctx Registered callback context data (user_data)
 */
typedef void (*eloop_sock_handler)(int sock, void *eloop_ctx, void *sock_ctx);

/**
 * @brief eloop generic event callback type
 * \param eloop_ctx Registered callback context data (eloop_data)
 * \param sock_ctx Registered callback context data (user_data)
 */
typedef void (*eloop_event_handler)(void *eloop_data, void *user_ctx);

/**
 * @brief eloop timeout event callback type
 * \param eloop_ctx Registered callback context data (eloop_data)
 * \param sock_ctx Registered callback context data (user_data)
 */
typedef void (*eloop_timeout_handler)(void *eloop_data, void *user_ctx);

/**
 * @brief eloop signal event callback type
 * \param sig Signal number
 * \param eloop_ctx Registered callback context data (global user_data from
 * eloop_init() call)
 * \param signal_ctx Registered callback context data (user_data from
 * eloop_register_signal(), eloop_register_signal_terminate(), or
 * eloop_register_signal_reconfig() call)
 */
typedef void (*eloop_signal_handler)(int sig, void *eloop_ctx,
				     void *signal_ctx);

/**
 * @brief Initialize global event loop data
 *
   This function must be called before any other
   eloop_* function. user_data can be used to configure a global (to the process
   ) pointer that will be passed as eloop_ctx parameter to signal handlers.
 * \param user_data: Pointer to global data passed
					 as eloop_ctx to signal handlers
 *
 * \return Returns 0 on success, -1 on failure
 */
int eloop_init(void *user_data);

/**
 * @brief Register handler for read events
 *
 * Register a read socket notifier for the given file descriptor. The handler
 * function will be called whenever data is available for reading from the
 * socket. The handler function is responsible for clearing the event after
 * having processed it in order to avoid eloop from calling the handler again
 * for the same event.

 * \param sock File descriptor number for the socket
 * \param handler Callback function to be called when data
				   is available for reading
 * \param eloop_data Callback context data (eloop_ctx)
 * \param user_data Callback context data (sock_ctx)
 * \return Returns 0 on success, -1 on failure
 */
int eloop_register_read_sock(int sock, eloop_sock_handler handler,
			     void *eloop_data, void *user_data);

/**
 * @brief Unregister handler for read events
 *
 * Unregister a read socket notifier that was previously registered with
 * eloop_register_read_sock().
 * \param sock File descriptor number for the socket
 */
void eloop_unregister_read_sock(int sock);

/**
 * @brief Register handler for socket events
 *
 * Register an event notifier for the given socket's file descriptor. The
 * handler function will be called whenever the that event is triggered for the
 * socket. The handler function is responsible for clearing the event after
 * having processed it in order to avoid eloop from calling the handler again
 * for the same event.

 * \param sock File descriptor number for the socket
 * \param type Type of event to wait for
 * \param handler Callback function to be called when the event is triggered
 * \param eloop_data Callback context data (eloop_ctx)
 * \param user_data Callback context data (sock_ctx)
 * \return Returns 0 on success, -1 on failure
 */
int eloop_register_sock(int sock, eloop_event_type type,
			eloop_sock_handler handler,
			void *eloop_data, void *user_data);

/**
 * @brief Unregister handler for socket events
 *
 * Unregister a socket event notifier that was previously registered with
 * eloop_register_sock().
 * \param sock File descriptor number for the socket
 * \param type Type of event for which sock was registered
 */
void eloop_unregister_sock(int sock, eloop_event_type type);

/**
 * @brief Register handler for generic events
 *
 * Register an event handler for the given event. This function is used to
 * register eloop implementation specific events which are mainly targetted for
 * operating system specific code (driver interface and l2_packet) since the
 * portable code will not be able to use such an OS-specific call.
   \n The handler function will be called whenever the event is triggered.
   The handler function is responsible for clearing the event after
   having processed it in order to avoid eloop from calling the handler again
   for the same event.
 * \n
 * In case of Windows implementation (eloop_win.c), event pointer is of HANDLE
 * type, i.e., void*. The callers are likely to have 'HANDLE h' type variable,
 * and they would call this function with eloop_register_event(h, sizeof(h),
 * ...).

 * \param event Event to wait (eloop implementation specific)
 * \param event_size Size of event data
 * \param handler Callback function to be called when event is triggered
 * \param eloop_data Callback context data (eloop_data)
 * \param user_data Callback context data (user_data)
 * \return Returns 0 on success, -1 on failure
 */
int eloop_register_event(void *event, size_t event_size,
			 eloop_event_handler handler,
			 void *eloop_data, void *user_data);

/**
 * @brief Unregister handler for a generic event
 *
 * Unregister a generic event notifier that was previously registered with
 * eloop_register_event().
 * \param event Event to cancel (eloop implementation specific)
 * \param event_size Size of event data
 */
void eloop_unregister_event(void *event, size_t event_size);

/**
 * @brief Register timeout
 *
 * Register a timeout that will cause the handler function to be called after
 * given time.
 * \param secs Number of seconds to the timeout
 * \param usecs Number of microseconds to the timeout
 * \param handler Callback function to be called when timeout occurs
 * \param eloop_data Callback context data (eloop_ctx)
 * \param user_data Callback context data (sock_ctx)
 * \return Returns 0 on success, -1 on failure
 */
int eloop_register_timeout(unsigned int secs, unsigned int usecs,
			   eloop_timeout_handler handler,
			   void *eloop_data, void *user_data);

/**
 * @brief Cancel timeouts
 *
 * Cancel matching <handler,eloop_data,user_data> timeouts registered with
 * eloop_register_timeout(). ELOOP_ALL_CTX can be used as a wildcard for
 * cancelling all timeouts regardless of eloop_data/user_data.
 * \param handler Matching callback function
 * \param eloop_data Matching eloop_data or %ELOOP_ALL_CTX to match all
 * \param user_data Matching user_data or %ELOOP_ALL_CTX to match all
 * \return Returns Number of cancelled timeouts
 */
int eloop_cancel_timeout(eloop_timeout_handler handler,
			 void *eloop_data, void *user_data);

/**
 * @brief Register handler for signals
 *
 * Register a callback function that will be called when a signal is received.
 * The callback function is actually called only after the system signal
 * handler has returned. This means that the normal limits for sighandlers
 * (i.e., only "safe functions" allowed) do not apply for the registered
 * callback.
 * \n
 * Signals are 'global' events and there is no local eloop_data pointer like
 * with other handlers. The global user_data pointer registered with
 * eloop_init() will be used as eloop_ctx for signal handlers.
 *
 * \param sig Signal number (e.g., SIGHUP)
 * \param handler Callback function to be called when the signal is received
 * \param user_data Callback context data (signal_ctx)
 * \return Returns 0 on success, -1 on failure
 */
int eloop_register_signal(int sig, eloop_signal_handler handler,
			  void *user_data);

/**
 * @brief Register handler for terminate signals
 *
 * Register a callback function that will be called when a process termination
 * signal is received. The callback function is actually called only after the
 * system signal handler has returned. This means that the normal limits for
 * sighandlers (i.e., only "safe functions" allowed) do not apply for the
 * registered callback.
 * \n
 * Signals are 'global' events and there is no local eloop_data pointer like
 * with other handlers. The global user_data pointer registered with
 * eloop_init() will be used as eloop_ctx for signal handlers.
 * \n
 * This function is a more portable version of eloop_register_signal() since
 * the knowledge of exact details of the signals is hidden in eloop
 * implementation. In case of operating systems using signal(), this function
 * registers handlers for SIGINT and SIGTERM.

 * \param handler Callback function to be called when the signal is received
 * \param user_data Callback context data (signal_ctx)
 * \return Returns 0 on success, -1 on failure
 */
int eloop_register_signal_terminate(eloop_signal_handler handler,
				    void *user_data);

/**
 * @brief Register handler for reconfig signals
 *
 * Register a callback function that will be called when a reconfiguration /
 * hangup signal is received. The callback function is actually called only
 * after the system signal handler has returned. This means that the normal
 * limits for sighandlers (i.e., only "safe functions" allowed) do not apply
 * for the registered callback.
 * \n
 * Signals are 'global' events and there is no local eloop_data pointer like
 * with other handlers. The global user_data pointer registered with
 * eloop_init() will be used as eloop_ctx for signal handlers.
 * \n
 * This function is a more portable version of eloop_register_signal() since
 * the knowledge of exact details of the signals is hidden in eloop
 * implementation. In case of operating systems using signal(), this function
 * registers a handler for SIGHUP.

 * \param handler Callback function to be called when the signal is received
 * \param user_data Callback context data (signal_ctx)
 * \return Returns 0 on success, -1 on failure
 */
int eloop_register_signal_reconfig(eloop_signal_handler handler,
				   void *user_data);

/**
 * @brief Start the event loop
 *
 * Start the event loop and continue running as long as there are any
 * registered event handlers. This function is run after event loop has been
 * initialized with event_init() and one or more events have been registered.
 */
void eloop_run(void);

/**
 * @brief Terminate event loop
 *
 * Terminate event loop even if there are registered events. This can be used
 * to request the program to be terminated cleanly.
 */
void eloop_terminate(void);

/**
 * @brief Free any resources allocated for the event loop
 *
 * After calling eloop_destroy(), other eloop_* functions must not be called
 * before re-running eloop_init().
 */
void eloop_destroy(void);

/**
 * @brief Check whether event loop has been terminated
 *
 * This function can be used to check whether eloop_terminate() has been called
 * to request termination of the event loop. This is normally used to abort
 * operations that may still be queued to be run when eloop_terminate() was
 * called.
 * \return Returns 1 = event loop terminate, 0 = event loop still running
 */
int eloop_terminated(void);

/**
 * @brief Wait for a single reader
 *
 * Do a blocking wait for a single read socket.
 * \param sock File descriptor number for the socket
 */
void eloop_wait_for_read_sock(int sock);

/**
 * @brief Get global user data
 * \return Returns user_data pointer that was registered with eloop_init()
 */
void * eloop_get_user_data(void);

#ifdef __cplusplus
}
#endif
#endif /* ELOOP_H */
