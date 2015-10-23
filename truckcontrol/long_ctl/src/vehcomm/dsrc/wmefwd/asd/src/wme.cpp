#include <cstdio>
#include <cstdlib>
#include <string>
#include <cstring>
#include <pthread.h>

#include <sys/types.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
			 
#include "libwme.h"
#include "libeloop.h"

#include "wmefwd.h"

#define MAX_LEN 10240

// Variables  - File Handlers

uint32_t wsm_counter = 0, rx_wsm_counter;

savari_wme_handler_t handler;

void savari_user_confirm(void *ctx, int conf_result_indication);
void savari_provider_confirm(void *ctx, int conf_result_indication);
void savari_wsm_indication(void *ctx, struct savariwme_rx_indication *ind);
void end_application(int signal, void *eloop, void *user);
void savari_rx(int sock_t, void *eloop_data, void *user_data);
void transmit(int sock, void *eloop_data, void *user_data);

struct savariwme_cbs wme_cbs;

struct savariwme_reg_req wmereq;
struct savariwme_tx_req wmetx;
int type = 1; //User = 1, provider = 0;

int wme_main ()
{
	uint8_t mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // Broadcast MAC Address
	int channel = config_data.channel;
	int psid = config_data.PSID;
	int priority = 0x01;
	// DataRate - DataRate Value to send to Driver
	//  3Mbps -> 6, 4.5Mbps -> 9, 6Mbps -> 12, 12Mbps -> 24, 18Mbps -> 36
	//  24Mbps -> 48, 27Mbps -> 54
	int datarate = 6; //3Mbps
	int power = 15; //Power in dbM
	int ret;
	char *psc = config_data.psc;
	int psc_length = strlen (psc);
	char *iface = config_data.iface;
	int supplicant = 0;
	int supp_enable = 0;

	/* The wme_init initializes and connects to the WME stack and returns a savari_wme_handler_t
	   handler. This handler is further used to transmit and receive messages to/from the daemon.
	   The messages may consist of a set of confirmations, or WSM data. */
	handler = wme_init("::1", iface);

	if (handler < 0) {
		fprintf (stderr, "socket to wme failed\n");
		return -1;
	}

	// Application can be registered in Either Continuous channel or Alternating Channel Modes
	//
	// For Registering in Continuous channel mode (Other than 178 channel)
	// For Channels Other than 178 (Control Channel)
	// wmereq.request_type = LIBWME_USER_AUTOACCESS_UNCOND;
	// wmereq.extended_access = 0xFFFF;
	// wmereq.channel = 176 (Continous Channel 176);
	//
	// If you want to Operate in Continouos Control Channel Mode (ie Channel 178)
	// wmereq.request_type =LIBWME_USER_AUTOACCESS_ONMATCH;
	// wmereq.extended_access = 0;
	// wmereq.channel = 0;
	//
	// For Registering in Alternating Channel Mode
	// wmereq.request_type = LIBWME_USER_AUTOACCESS_UNCOND;
	// wmereq.channel = 174; (Radio Switches channel between 178 and 174)
	// wmereq.exteneded_access = 0;

	memcpy(wmereq.psc, psc, psc_length);
	wmereq.channel = channel;
	wmereq.priority = priority;
	wmereq.psid = wme_convert_psid_be(psid);
	wmereq.request_type = LIBWME_USER_AUTOACCESS_ONMATCH;
	wmereq.extended_access = 0xFFFF;
	wmereq.immediate_access = 0;
	wmereq.psc_length = psc_length;
	memcpy(wmereq.destmacaddr, mac, sizeof(wmereq.destmacaddr));

	/* if (config_data.mode == CONTINUOUS) */
	/* 	wmereq.channel_access = LIBWME_CHANNEL_ACCESS_CONTINUOUS; */
	/* else if (config_data.mode == CONTINUOUS) */
	/* 	wmereq.channel_access = LIBWME_CHANNEL_ACCESS_ALTERNAING; */

	if (type == 0) {
		ret = wme_register_provider(handler, &wmereq);
		wme_cbs.wme_provider_confirm = savari_provider_confirm;
	}
	else if (type == 1) {
		ret = wme_register_user(handler, &wmereq);
		wme_cbs.wme_user_confirm = savari_user_confirm;
	}
	if (ret < 0) {
		perror ("wme provider");
		exit(-1);
	}

	eloop_init(0);
	if (ret < 0) {
		perror ("eloop_init");
		exit(-1);
	}

	eloop_register_signal_terminate (end_application, NULL);

	if (config_data.direction == INCOMING) {
		log ("Preparing to receive incoming messages\n");
		wme_cbs.wme_wsm_indication = savari_wsm_indication;
	}
	else if (config_data.direction == OUTGOING) {
		memset(&wmetx, 0, sizeof(struct savariwme_tx_req));
		wmetx.supp_enable = supp_enable;
		wmetx.psid = wmereq.psid;
		wmetx.priority = priority;
		wmetx.channel = channel;   // Channel in which this WSM should be transmitted
		wmetx.datarate = datarate; // Rate and power at which WSM should be transmitted
		wmetx.txpower = power;
		memcpy(wmetx.mac, mac, SAVARI1609_IEEE80211_ADDR_LEN);
		wmetx.expiry_time = 0;
		wmetx.element_id = WAVE_ELEMID_WSMP;
		wmetx.tx_length = config_data.length;
		if (supp_enable)
			wmetx.safetysupp = supplicant;
	}

	eloop_register_read_sock(handler, savari_rx, 0, &wmereq);
	eloop_run();
	eloop_destroy();

	return 0;
}

/* savari_user_confirm - Invoked by the WME layer to indicate the
 * status of wme_register_user.
 * \param ctx - application ctx
 * \param confirm - registration status, can be LIBWME_RC_ACCEPTED,
 * LIBWME_RC_INVALID_PARAMETERS or LIBWME_RC_UNSPECIFIED */

void savari_user_confirm(void *ctx, int confirm)
{
	struct savariwme_reg_req *wme_req = (struct savariwme_reg_req *)ctx;

	if (confirm == LIBWME_RC_ACCEPTED) {
		wme_user_service_confirm(handler, LIBWME_ACTION_ADD, wme_req);
		eloop_register_read_sock (sockfd, transmit, NULL, NULL);
	} else {
		log ("Service user registration failed\n");
		exit (-2);
	}
}

void savari_provider_confirm(void *ctx, int confirm)
{
	struct savariwme_reg_req *wme_req = (struct savariwme_reg_req *)ctx;

	if (confirm == LIBWME_RC_ACCEPTED) {
		wme_provider_service_confirm(handler, LIBWME_ACTION_ADD, wme_req);
		eloop_register_read_sock (sockfd, transmit, NULL, NULL);
	} else {
		log ("Service user registration failed\n");
		exit (-2);
	}
}

/* savari_wsm_indication - Invoked by the WME layer indicating the application
 * about the WSM packet matching based on PSID.
 * \param ctx - application ctx
 * \param rxind - rx indication buffer*/

void savari_wsm_indication(void *ctx, struct savariwme_rx_indication *rxind)
{
	if (config_data.direction != INCOMING) return;
	setSocketData (rxind->rx_buf);
}

/* savari_rx - Thread that waits for data on the handler returned
 * by the wme_init. When data is available wme_rx invokes registered
 * callbacks. */

void savari_rx(int sock_t, void *eloop_data, void *user_data)
{
	wme_rx(handler, &wme_cbs, user_data);
}

void transmit(int sock, void *eloop_data, void *user_data)
{
	static uint8_t *buf = NULL;
	if (buf == NULL) buf = (uint8_t *) malloc (config_data.length);
	memset (buf, 0, config_data.length);
	getSocketData (buf);
	if (wme_wsm_tx(handler, &wmetx, buf) == FAIL) {
		log ("WME Sending failed!\n");
	}
}

void end_application (int signal, void *eloop, void *user) {
	if (type == 0) {
		wme_unregister_provider(handler, &wmereq);
	} else if (type == 1) {
		wme_unregister_user(handler, &wmereq);
	}
	eloop_unregister_read_sock (handler);
	eloop_unregister_read_sock (sockfd);
	eloop_terminate();
	log ("Ending application\n");
	exit(0);
}
