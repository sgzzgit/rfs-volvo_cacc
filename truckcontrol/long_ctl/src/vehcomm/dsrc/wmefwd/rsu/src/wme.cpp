#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/select.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "savariWAVE_api.h"
#include "libwme.h"
#include "libeloop.h"
#include <wmefwd.h>

#define MAX_LEN 1024

savari_wme_handler_t handler;

void savari_user_confirm(void *ctx, int conf_result_indication);
void savari_provider_confirm(void *ctx, int confirm);
void savari_wsm_indication(void *ctx, struct savariwme_rx_indication *ind);
void savari_rx(int sock, void *eloop_ctx, void *sock_ctx);
void transmit(int sock, void *eloop_data, void *user_ctx);

struct savariwme_cbs wme_cbs;
struct savariwme_tx_req wmetx;
struct savariwme_reg_req wmereq;
int type = 0;
static int dir;

int wme_main ()
{
	uint8_t mac[6] = {0xFF,0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; //Broadcast MAC Address
	int channel = config_data.channel;
	char *iface = config_data.iface;
	int psid = config_data.PSID;
	char *psc = config_data.psc;
	int psc_length = strlen(psc);
	int request = 0;
	int priority = 0x01;
	int datarate = 6;
	int power = 15;
	int supp_enable = 0; 
	int supplicant = 0;
	int ret;

	handler = wme_init("::1", iface);

	if (handler < 0) {
		perror ("socket to wme failed\n");
		exit(-1);
	}

	memcpy(wmereq.psc,psc,psc_length);
	wmereq.channel = channel;
	wmereq.priority = priority;
	wmereq.psid = wme_convert_psid_be(psid);
	wmereq.request_type = request;
	wmereq.immediate_access = 0;
	wmereq.psc_length = psc_length;
	memcpy(wmereq.destmacaddr, mac, sizeof(wmereq.destmacaddr));

	if (config_data.mode == CONTINUOUS) {
		wmereq.channel_access = SAVARI1609_CHANNEL_ACCESS_CONTINUOUS;
	    wmereq.request_type = SAVARI1609_USER_AUTOACCESS_ONMATCH;
		wmereq.extended_access = 0xFFFF;
	}
	else if (config_data.mode == ALTERNATING) {
	    wmereq.request_type = SAVARI1609_USER_AUTOACCESS_UNCOND;
		wmereq.channel_access = SAVARI1609_CHANNEL_ACCESS_ALTERNATING;
		wmereq.extended_access = 0;
    }
    dir = config_data.direction;
	ret = wme_register_provider(handler, &wmereq);
	wme_cbs.wme_provider_confirm = savari_provider_confirm;
	if (ret < 0) {
		perror ("registering device");
		exit(-1);
	}
	
	ret = eloop_init(0);
	if (ret < 0) {
		perror ("eloop_init");
		exit(-1);
	}
	eloop_register_signal_terminate(end_application, NULL);

	if (config_data.direction == INCOMING) {
		log ("Preparing to receive incoming messages\n");
		wme_cbs.wme_wsm_indication = savari_wsm_indication;
	}
	else if (config_data.direction == OUTGOING) {
		log ("Preparing to send outgoing messages\n");
		memset(&wmetx, 0, sizeof(struct savariwme_tx_req));
		wmetx.supp_enable = supp_enable;
		wmetx.channel = channel;
		wmetx.psid = wmereq.psid;
		wmetx.priority = priority;
		wmetx.datarate = datarate;
		wmetx.txpower = power;
		wmetx.tx_length = config_data.length;
		memcpy(wmetx.mac, mac, SAVARI1609_IEEE80211_ADDR_LEN);
		wmetx.expiry_time = 0;
		wmetx.element_id = WAVE_ELEMID_WSMP;
		if (supp_enable)
			wmetx.safetysupp = supplicant;
	}

	eloop_register_read_sock(handler, savari_rx, 0, (void*) &wmereq);
	eloop_run();
	eloop_destroy();

    return 0;
}

void savari_user_confirm(void *ctx, int confirm)
{
	struct savariwme_reg_req *wme_req = (struct savariwme_reg_req *)ctx;

	if (confirm == SAVARI1609_RC_ACCEPTED) {
		wme_user_service_confirm(handler, SAVARI1609_ACTION_ADD, wme_req);
		eloop_register_read_sock (sockfd, transmit, NULL, NULL);
	}  else {
		log ("Service user registration failed\n");
		exit (-2);
	}

}

void savari_provider_confirm(void *ctx, int confirm)
{
	struct savariwme_reg_req *wme_req = (struct savariwme_reg_req *)ctx;

	if (confirm == SAVARI1609_RC_ACCEPTED) {
		wme_provider_service_confirm(handler, SAVARI1609_ACTION_ADD, wme_req);
		eloop_register_read_sock (sockfd, transmit, NULL, NULL);
	} else {
		log ("Service provider registration failed\n");
		exit (-2);
	}
}

void savari_wsm_indication(void *ctx, struct savariwme_rx_indication *rxind)
{
	setSocketData (rxind->rx_buf);
}

void savari_rx(int sock, void *eloop_ctx, void *sock_ctx)
{
	wme_rx(handler, &wme_cbs, &wmereq);
}

void transmit(int sock, void *eloop_data, void *user_ctx)
{
	static uint8_t *buf = NULL;
	if (buf == NULL) buf = (uint8_t *) malloc (config_data.length);
	memset (buf, 0, config_data.length);
	getSocketData (buf);
	if (wme_wsm_tx(handler, &wmetx, buf) == FAIL) { 
		log ("WME Sending failed!\n");
	}
}

void end_application(int sig, void *eloop_ctx, void *signal_ctx) {
	if(type == 0) {
		wme_unregister_provider(handler, &wmereq);
	} else if(type == 1){
		wme_unregister_user(handler, &wmereq);
	}
	eloop_unregister_read_sock(handler);
	eloop_unregister_read_sock(sockfd);
	eloop_terminate();
	log ("Ending application\n");
	exit(0);
}
