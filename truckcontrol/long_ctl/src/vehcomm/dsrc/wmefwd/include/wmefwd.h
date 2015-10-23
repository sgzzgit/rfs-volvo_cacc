#ifndef _MMITSSWMEFWD_H
#define _MMITSSWMEFWD_H

#include <string>
#include <cstring>
#include <stdint.h>	// c++11 <cstdint>
#include <csetjmp>
#include <csignal>
#include <vector>
#include <map>
#include <sys/socket.h>
#include <netinet/in.h>

#include "libwme.h"	// savari_wme_handler_t, savariwme_cbs, savariwme_reg_req, savariwme_tx_req

/// interrupt signal
#define ERROR -1
static int sig_list[] = 
{
	/// list of signals for interruption, handled by sig_hand ()
	SIGABRT,
	SIGINT,			
	SIGQUIT,		
	SIGTERM,		
	SIGKILL,
	SIGSEGV,
	ERROR
};
static jmp_buf exit_env;
static void sig_hand( int code )
{
	longjmp( exit_env, code );
};
static void sig_ign(int *sigList, void sig_hand(int sig))
{
  int i = 0;
  while (sigList[i] != ERROR)
	{
    signal(sigList[i], sig_hand);
    i++;
  };
};

struct savariWmeHandler_t
{
	bool isRegistered;
	bool isConfirmed;
	savari_wme_handler_t handler;
	struct savariwme_reg_req wmereq;
	struct savariwme_tx_req wmetx;
};

void savari_user_confirm(void *ctx, int conf_result_indication);
void savari_provider_confirm(void *ctx, int conf_result_indication);
void savari_wsm_indication(void *ctx, struct savariwme_rx_indication *rxind);
void savari_rx(int sock_t, void *eloop_data, void *user_data);
void wsm_tx(int sock_t, void *eloop_data, void *user_data);
void savari_unregister_user(void);
void savari_unregister_provider(void);
void savari_eloop_register_read_sock(void);
unsigned long uchar2ulong(const uint8_t* ptr,const int size);
void close_all(void);

#endif
