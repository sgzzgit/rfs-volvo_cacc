/**
 * @file libwme.h
   @brief this library contains the APIs that are used to
           transmit/recv WSMP messages and/or WSAs.
           Link with -lwme to use this library.
 */
#ifndef __LIBWME_H__
#define __LIBWME_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <savariWAVE_api.h>
/**
 * SUCCESS status of the API in this library
 */
#define SUCCESS 0
/**
 * FAIL Status of the API in this library
 */
#define FAIL -1
/**
 * PSID length
 */
#define PSID_LEN 4
/**
 * SSI length
 */
#define SSI_LEN 16

typedef int savari_socket_desc_t;
/**
 * a handler returned from the wme_init
 */
typedef savari_socket_desc_t savari_wme_handler_t;
/**
 * struct savariwme_reg_req - used to register the application with wme stack
 */
struct savariwme_reg_req {
    /**
     * should be either service channel(SC) or continuous channel(CC)
     */
    int channel;
    /**
     * source mac address
     */
    uint8_t srcmacaddr[SAVARI1609_IEEE80211_ADDR_LEN];
    /**
     * destination macaddress to which WSAs should be sent
     */
    int destmacaddr[SAVARI1609_IEEE80211_ADDR_LEN];
    /**
     * psid - provider service identifier
     */
    uint32_t psid;
    /**
     * the number of WSAs transmitted for 5sec.
     */
    int repeatrate;
    /**
     * priority of Provider/User
     */
    int priority;
    /**
     * type of user application request.\n\n
        SAVARI1609_USER_AUTOACCESS_ONMATCH (Switch Between 178 and SCH after
        receiving Matching WSA from RSE\n\n

        SAVARI1609_USER_AUTOACCESS_UNCOND (Start Switching between 178 and SCH
        Without Waiting for Matching WSA from RSEs)\n\n

        SAVARI1609_USER_AUTOACCESS_NOSCHACCESS(CCH Only Mode. No Switching)
        Only applicable if channel_access is ALTERNATING\n\n
     */
    int request_type;
    /**
     * SAVARI1609_USER_AUTOACCESS_ONMATCH/UNCOND indicate a request for
        continuous operation on the channel for
        extended_access number of SCH intervals
     */
    int extended_access;
    /**
     * Provider channel switching mode\n\n
        One of SAVARI1609_CHANNEL_ACCESS_CONTINUOUS(non channel switching, stay on channel),\n\n
        SAVARI1609_CHANNEL_ACCESS_ALTERNATING(forced/conditional switching between 178 and channel)
     */
    int channel_access;
    /**
     * this indicates the device should immediately switch to SCH, rather than waiting for the next SCH interval (0/1)
     */
    int immediate_access;
    /**
     * secured (SAVARI1609_WSA_SECURED) or unsecured (SAVARI1609_WSA_UNSECURED) WSA
     */
    int wsatype;
    /**
     * provider service context
     */
    char psc[32];     
    /**
     * provider service context length
     */
    int psc_length;
    /**
     * index to the associated MIB table or internal datastructure.
     * Must be unique for a give PSID and psc combination
     */
    int local_service_index;
    int ipservice;
    /**
     * service IPv6 address; memset to 0 if not used
     */
       struct in6_addr service_ipv6addr;
    /**
     * port on which service is provided; memset to 0 if not used
     */
    int service_port;
    /**
     * for doing registration of first radio set it to 0 and for the
     * second radio set it to 1.
     */
    int secondradio;
#if 0 // SSI is not needed any more
	/**
	 * SSI is required by cycur for certificate look up.
     * This is not a 1609.3 standard parameter.
	 */
	//char ssi[SSI_LEN];
#endif
}__attribute__((__packed__));

/**
 * savariwme_rx_indication - informs the application about a
                             WSMP/WSMPS reception
 */
struct savariwme_rx_indication {
    /**
     * WAVE version number
     */
    int version;
    /**
     * timestamp at which packet got received
     */
    int tstamp;
    /**
     * plcp length
     */
    int plcp_length;
    /**
     * transmitted power
     */
    int txpower;
    /**
     * datarate
     */
    int datarate;
    /**
     * received mac
     */
    uint8_t rx_mac[SAVARI1609_IEEE80211_ADDR_LEN];
    /**
     * received bufferlen
     */
    uint32_t rx_buf_length;
    /**
     * received WSMP supplement
     */
    uint32_t rx_supp;
    /**
     * channel on which the message was received
     */
    int channel;
    /**
     * Provider Service Identifier
     */
    uint8_t psid[PSID_LEN];
    /**
     * prirority at which the packet received
     */
    int priority;
    /**
     * received signalstrength indication
     */
    int rssi;
    /**
     * number of received bytes
     */
    int num_rx;
    /**
     * received buffer
     */
    uint8_t rx_buf[SAVARI1609_MAXLINE];
}__attribute__((__packed__));

/**
 * struct savari1609Wra - WRA information
 */
struct savari1609Wra {
    /**
     * router lifetime
     */
    uint16_t  lifetime;
	/**
	 * ipv6addr indicates IPv6 subnet prefix of the link
	 */
    struct in6_addr ipv6addr;
	/**
	 * prefixlen indicates the IPv6 subnet prefix of the link. (RFC 3513)
	 */
    uint8_t prefixlen;
    /**
     * Default gateway is 128 bit IPv6 address of a router that provides internet connectivity to subnet
     */
    struct in6_addr default_gw;
	/**
	 * Macaddress of the default gateway.
	 */
    uint8_t gw_macaddr[SAVARI1609_IEEE80211_ADDR_LEN];
    /**
     * Primary DNS is the 128 bit IPv6 address that can provide DNS lookup for the subnet devices.
     */
    struct in6_addr primarydns;
    /**
     * Secondary DNS is the 128 bit IPv6 address of an alternate device that can provide DNS lookup for the subnet devices.
     */
    struct in6_addr secondarydns;
} __attribute__((__packed__));

/**
 * struct savariwme_tx_req - this structure is used to transmit
                             wsmp/wsmps over the air.
                             the configuration parameters will be
                             read from this structure and using
                             those, the packet will be transmitted
 */
struct savariwme_tx_req {
    /**
     * channel of transmission of WSMs/WSMPs.
     */
    int channel;
    /** 
     * Proivder Service Identifier
     */
    uint32_t psid;
    /**
     * Qos for Packet
     */
    int priority;
    /** 
     * datarate
     */
    int datarate;
    /**
     * transmit power
     */
    int txpower;
    /** 
     * mac address
     */
    uint8_t mac[SAVARI1609_IEEE80211_ADDR_LEN];
    /** 
     * indicates the time at which the message is no longer valid
     */
    int expiry_time;
    /** 
     * WAVE element id
        set to WAVE_ELEMID_WSMP for WSMP
        set to WAVE_ELEMID_WSMPS for WSMPS
     */
    int element_id;
    /**
     * tx buffer length
     */
    int tx_length;
    /**
     * sup_enable when 1 reads and transmits safetysupp
        when 0 it doesn't
     */
    int supp_enable;
    /**
     * WSM safety supplement
     */
    uint32_t safetysupp;
}__attribute__((__packed__));

/**
 * struct savariwme_cbs - a set of callbacks associated with the application
                          about the indication of WSMs/commands etc.
 */
struct savariwme_cbs {
/**
 * wme_provider_confirm - indicates to the application about the
                          provider registration Confirmation
  \param ctx - context of the application
  \param conf_result_indication - result of the wme_register_provider. 0 - success non-zero : failure
 */
    void (*wme_provider_confirm)(void *ctx, int conf_result_indication);
/**
 * wme_user_confirm - indicates to the application about the user registration 
   \                - confirmation
   \param ctx - context of the application
   \param conf_result_indication - result of the wme_register_user. 0 - success non-zero : failure
 */
    void (*wme_user_confirm)(void *ctx, int conf_result_indication);
/**
 * wme_cch_confirm - confirm message about the cch registration
   \param ctx - context of the application
   \param conf_result_indication - result of the cch registration. 0 - success non-zero : failure
 */
    void (*wme_cch_confirm)(void *ctx, int conf_result_indication);
/**
 * wme_wsm_indication - indicates to the application about the WSM reception.
   \param ctx - application ctx.
   \param ind - rx indication buffer about the rxdatalen, psid, datarate etc.
   \par Description -
             This function indicates to the application about the
             WSM packet matching packet based on PSID. and returns in the buffer
             ,ind->rx_buf.
 */
    void (*wme_wsm_indication)(void *ctx, struct savariwme_rx_indication *ind);
/**
 * wme_cmd - response for the wme command issued by the application
   \param ctx - context of the application
   \param cmd_indication - indication from the library about the command
   \par Description  -
       The function gives a response about a wme command sent from user
 */
    void (*wme_cmd)(void *ctx, int cmd_indication);

	void (*get_wsa_cnt)(void *ctx, uint32_t wsa_cnt);
};
/**
 * wme_init - initialises and connects to the wme stack
   \param serverip - serverip , the wme stack ip to connect to, default 127.0.0.1
   \param iface - on which interface the app wants to connect to the stack (ath0/ath1)
   \par Description -
             The wme_init initialises and connects to the wme stack
             and returns a savari_wme_handler_t handler. this handler is further
             used to transmit and receive messages to/from the daemon.
             the messages may consist of a set of confirmations, or WSM data.
   \return - returns a handler of savari_wme_handler_t, and FAIL on failure
 */ 
int
wme_init(char *serverip, char *iface);
/**
 * wme_register_user - register the application as user
   \param handler - handler returned from the wme_init
   \param wme_req - the wme_req structure from the application
   \par Description -
             The wme_register_user registers the application in user mode
             to the stack for the purpose of sending /receiving 
             WSMs or joining a service. the stack recognises the application using psids.
   \return - returns SUCCESS on success and FAIL on failure.   
 */
int
wme_register_user(savari_wme_handler_t handler,
            struct savariwme_reg_req *wme_req);
/**
 * wme_unregister_user - unregister the user from the wme stack
   \param handler - handler returned from the wme_init
   \param wme_req - the wme_req structure from the wme_init
   \par Description -
             The wme_unregister_user unregisters the user application
             from the stack and stops receiving the WSMs or commands on
             behalf of the application.
 */
void
wme_unregister_user(savari_wme_handler_t handler,
            struct savariwme_reg_req *wme_req);
/**
 * wme_register_cch_request - registers a CCH access request
   \param handler - a handler returned from the wme_init
   \param intvl - CCH interval
   \param priority - priority  
   \par Description -
             This function registers a cch request.
   \return - returns SUCCESS on success and FAIL on failure.
 */
int
wme_register_cch_request(savari_wme_handler_t handler,
                    int intvl, int priority);
/**
 * wme_unregister_cch_request - unregisters CCH access request
 * \param handler - a handler returned from the wme_init
   \param intvl - CCH interval
   \param priority - priority
   \par Description -
             This function unregisters the cch request
   \return - returns SUCCESS on success and FAIL on failure.
 */
int
wme_unregister_cch_request(savari_wme_handler_t handler,
                    int intvl , int priority);
/**
 * wme_user_service_confirm - confirm user application to the 1609.3 stack
   \param handler - a handler returned from the wme_init
   \param action - an action to specify whether wanted to do tx/rx or not.
   \param wme_req - a pointer to the savariwme_reg_req structure, passed from
                    the application at the time of wme_register_user
   \par Description -
                       This function should be called by user application
                      when it receives user confirm from the stack.
   \return - returns SUCCESS on success and FAIL on failure.
 */
void wme_user_service_confirm(savari_wme_handler_t handler, int action,
                    struct savariwme_reg_req *wme_req);
/**
 * wme_register_provider - registers the provider application
   \param handler - a handler returned from the wme_init
   \param wme_req - a pointer to the savariwme_reg_req structure passed from
                    the application
   \par Description -
                      This function registers the application as a provider and
                      makes it capable of transmitting/receiving WSMs and transmitting WSAs.
   \return - returns SUCCESS on success and FAIL on failure.
 */
int wme_register_provider(savari_wme_handler_t handler,
            struct savariwme_reg_req *wme_req);
/**
 * wme_unregister_provider - unregisters the provider
   \param handler - handler returned from the wme_init
   \param wme_req - a savariwme_reg_req structure passed from the application
                    at the time of registering using the API
                    wme_register_provider
   \par Description -
             This function unregisters the provider application and
             the stack stops transmitting the WSMs and WSAs on behalf of the
             application.
 */
void wme_unregister_provider(savari_wme_handler_t handler,
                struct savariwme_reg_req *wme_req);
/**
 * wme_provider_service_confirm - confirm provider application to the 1609.3 stack
   \param handler - a handler returned from the wme_init
   \param action - a action to specify whether wanted to do tx/rx or not.
   \param wme_req - a pointer to the savariwme_reg_req structure, passed from
                    the application at the time of wme_register_user
   \par Description -
                       This function should be called by provider application
                      when it receives provider confirm from the stack.
   \return - returns SUCCESS on success and FAIL on failure.
 */
void wme_provider_service_confirm(savari_wme_handler_t handler, int action,
                    struct savariwme_reg_req *wme_req);
/**
 * wme_wsm_tx - transmits the WSMs 
 * \param handler - the handler returned from the wme_init
   \param wme_wsm_tx - a pointer to the tx structure containing the transmission
                       parameters, such as channel, datarate, txpower, and 
                       transmission data length etc.
   \param tx_buffer - a pointer to the tx buffer from the application
   \par Description - 
                      This function gets the tx_buffer from the application and
                      then it transmits the message with the control parameters specified in
                      the wme_wsm_tx to the 1609.3 stack.
   \return - returns SUCCESS on success and FAIL on failure. 
 */
int
wme_wsm_tx(savari_wme_handler_t handler, struct savariwme_tx_req *wme_wsm_tx,
                                    uint8_t *tx_buffer);
/**
 * wme_rx - 1609.3 stack's callback routine from applicaiton
   \param handler - the handler returned from the wme_init
   \param handle - this is a structure containing a set of function pointers.
   \param ctx - the context passed from the application
   \par Description -
            This function should be called by the application whenever there is
            data available on handler returned by wme_init(). Stack will then parse
            that data and calls applications callbacks registered using savariwme_cbs.
        \n   for a application provider confirmation calls wme_provider_confirm
        \n   for a application user confirmation calls wme_user_confirm
        \n   for a cch confirmation calls wme_cch_confirm
        \n   for a WSM receivd. calls wme_wsm_indication
        \n   for a wme cmd responses, calls wme_cmd
   \return - this function returns SUCCESS on success and FAIL on failure
 */  
int
wme_rx(savari_wme_handler_t handler, struct savariwme_cbs *handle, void *ctx);
/**
 * wme_convert_psid_be - converts the psid to bigendian
   \param psid - psid
   \par Description -
                This function returns PSID in bigendian form.
   \return - returns the converted psid.
 */
uint32_t
wme_convert_psid_be(uint32_t psid);
/**
 * wme_getpsidlen - gets the psid length
   \param psid - an uint8_t pointer.
   \par Description -
                wme_getpsidlen returns the psid length of the given psid.
   \return - returns the length of psid on success, and 0 on failure.
 */
int wme_getpsidlen (uint8_t *psid);
/**
 * wme_process_wme_cmd - Sends stack specific commands
   \param handler - a handler returned from the wme_init
   \param cmd - command sent from the application (GET_WME_ERROR/CLEAR_WME_ERROR)
   \par Description -
        This function processes the GET/CLEAR WME command specified by the
        application. It returns or clears PSID mismatch errors.
 */
int
wme_process_wme_cmd(savari_wme_handler_t handler, int cmd);
/**
 * wme_set_gpslocation - sets the gps location information to the 1609.3 stack
   \param handler - a handler returned from the wme_init
   \param gpsinfo -
        The gpsdata passed by the user application. Application may get this
        by querying the gps stack, refer to libgpsapi on
        how to query the gps stack , by using those apis.
   \par Description -
             This function sets the gps location speicified in the
             gpsinfo structure to the 1609.3 stack for the purpose of
             transmitting the location information in the 1609.3 WSA header. 
   \return - returns SUCCESS on success and FAIL on failure.
 */

int
wme_set_gpslocation(savari_wme_handler_t handler,
                    struct savari16093gpsinfo *gpsinfo);
/**
 * wme_set_wrainfo - sets the WRA information to the 1609.3stack 
   \param handler - a handler returned from the wme_init
   \param wra - the wra information passed by the application
   \par Description -
            This function sets the WRA information passed from the
            application to the 1609.3 stack for further transmissions
            in WSAs.
   \return - returns SUCCESS on success and FAIL on failure.
 */
int
wme_set_wrainfo(savari_wme_handler_t handler,
                    struct savari1609Wra *wra);
/**
 * wme_set_wsaintvl - sets the wsa interval
   \param handler - a handler returned from the wme_init
   \param wsa_intvl - WSA interval 
   \par Description -
                     This function sets the wsa interval to transmit WSAs.
   \return - returns SUCCESS on success and FAIL on failure. 
 */
int
wme_set_wsaintvl(savari_wme_handler_t handler, int wsa_intvl);
/**
 * wme_set_wsatxpower -sets the txpower of a WSA
   \param handler - a handler returned from the wme_init
   \param txpower - transmit power of a WSA
   \par Description -
                     This function sets the WSA txpower.
   \return - returns SUCCESS on success and FAIL on failure.
 */
int
wme_set_wsatxpower(savari_wme_handler_t handler, int txpower);
/**
 * wme_set_advertiserid - sets the advertiser id
   \param handler- a handler returned from the wme_init
   \param adv_id - advertiser id from the application to set 
   \par Description -
                     This function sets the advertiser id of the WSA.
   \return - returns SUCCESS on success and FAIL on failure.
 */
int
wme_set_advertiserid(savari_wme_handler_t handler, char* adv_id);

/**
 * wme_deinit - deinitialise and disassociate the application from 1609.3 stack
   \param handler - a handler returned from the wme_init
 */
void
wme_deinit(savari_wme_handler_t handler);

#ifdef __cplusplus
}
#endif

#endif
