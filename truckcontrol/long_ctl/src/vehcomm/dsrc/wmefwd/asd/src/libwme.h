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
/**
 * Advertiser id length
 */
#define ADVERTISER_LEN 127

typedef int savari_socket_desc_t;

typedef savari_socket_desc_t savari_wme_handler_t;

#ifndef SAVARI1609_IEEE80211_ADDR_LEN
#define SAVARI1609_IEEE80211_ADDR_LEN 6
#endif

#ifndef SAVARI1609_MAXLINE
#define SAVARI1609_MAXLINE 2000
#endif

#ifndef WAVE_ELEMID_WSMP
#define WAVE_ELEMID_WSMP 128
#endif

/**
 * @brief result codes when a confirmation
 *  comes from 1609.3, generally these codes
 *  will go into as arguments of wme_provider_confirm
 *  or wme_user_confirm callback functions
 */
typedef enum {
    /**
     * registration successful
     */
    LIBWME_RC_ACCEPTED,
    /**
     * Invalid parameters in registration
     */
    LIBWME_RC_INVALID_PARAMETERS,
    /**
     * Unknown/Unspecified error occured
     */
    LIBWME_RC_UNSPECIFIED
} LIBWME_RC_RESULT_CODE;

/**
 * @brief Requested channel access type
 *
 */
enum {
    /**
     * Provide the service to the application
     * when a matched service is advertised over WSA
     */
    LIBWME_USER_AUTOACCESS_ONMATCH,
    /**
     * Force 1609.3 protocol to switch between a
     * given service channel and default control channel(178)
     */
    LIBWME_USER_AUTOACCESS_UNCOND,
    /**
     * Wait for the WSA and match the given service irrespective
     * of the service channel
     */
    LIBWME_USER_NOSCHACCESS
} LIBWME_USERREQUEST;


/**
 * @brief enums for the registration confirmation
 *
 * These are used to whether join/delete/change a service
 */
enum {
    /**
     * Add the current user/provider to the service table
     */
    LIBWME_ACTION_ADD,
    /**
     * Remove the current user/provider from the service table
     */
    LIBWME_ACTION_DELETE,
    /**
     * Change the current user/provider in the service table
     */
    LIBWME_ACTION_CHANGE,
} LIBWME_USER_ACTION;
/**
 * @brief used to register the application with wme stack
 *
 * This structure must be filled up from the application in order to
 * register to the 1609.3 stack
 */
struct savariwme_reg_req {
    /**
     * should be either service channel(SC) or continuous channel(CC)
     *
     * The WAVE stack support channels ranging from 172 to 184
     */
    int channel;
    /**
     * source mac address
     */
    uint8_t srcmacaddr[SAVARI1609_IEEE80211_ADDR_LEN];
    /**
     * destination macaddress to which WSMP/WSAs should be sent
     */
    int destmacaddr[SAVARI1609_IEEE80211_ADDR_LEN];
    /**
     * psid - provider service identifier (Refer to 1609.3 document)
     *
     * PSID is used to differentiate between different safety/roadside applications
     * and messages
     */
    uint32_t psid;
    /**
     * the number of WSAs transmitted for 5sec. Ignore for WSMP traffic.
     */
    int repeatrate;
    /**
     * priority of Provider/User
     */
    int priority;
    /**
     * type of user application request.\n\n
        LIBWME_USER_AUTOACCESS_ONMATCH (Switch Between 178 and SCH after
        receiving Matching WSA from RSE\n\n

        LIBWME_USER_AUTOACCESS_UNCOND (Start Switching between 178 and SCH
        Without Waiting for a Matching WSA from RSEs)\n

        In case of LIBWME_USER_AUTOACCESS_UNCOND set extended_access to 0xffff for a
        prolonged continuous mode of operation\n\n

        LIBWME_USER_AUTOACCESS_NOSCHACCESS(CCH Only Mode. No Switching)
        Only applicable if channel_access is ALTERNATING\n\n
     */
    int request_type;
    /**
     * set to 0xffff for continuous access otherwise 0.
     */
    int extended_access;
    /**
     * Provider channel switching mode\n\n
        One of LIBWME_CHANNEL_ACCESS_CONTINUOUS(non channel switching, stay on channel),\n\n
        LIBWME_CHANNEL_ACCESS_ALTERNATING(forced/conditional switching between 178 and channel)
     */
    int channel_access;
    /**
     * this indicates the device should immediately switch to SCH, rather than waiting for the next SCH interval (0/1)
     */
    int immediate_access;
    /**
     * secured (SAVARI1609_WSA_SECURED) or unsecured (SAVARI1609_WSA_UNSECURED) WSA - ignore for WSMP
     */
    int wsatype;
    /**
     * provider service context - ignore for WSMP
     */
    char psc[32];     
    /**
     * provider service context length - ignore for WSMP
     */
    int psc_length;
    /**
     * index to the associated MIB table or internal datastructure.
     * Must be unique for a give PSID and psc combination - not used as of now
     */
    int local_service_index;
    int ipservice;
    /**
     * service IPv6 address; memset to 0 if not used - ignore for WSMP
     */
       struct in6_addr service_ipv6addr;
    /**
     * port on which service is provided; memset to 0 if not used - ignore for WSMP
     */
    int service_port;
    /**
     * for doing registration of first radio set it to 0 and for the
     * second radio set it to 1. - ignore for WSMP
     */
    int secondradio;
#if 0 // SSI is not needed any more
    /**
     * SSI is required by cycur for certificate look up.
     * This is not a 1609.3 standard parameter.
     */
    //char ssi[SSI_LEN];
#endif
    char advertiser_id[ADVERTISER_LEN];
    int linkquality;
}__attribute__((packed));


struct libwme_gpsinfo {
    int latitude;
    int longitude;
    int elevation;
    double pos_confidence;
    double elev_confidence;
    int positional_accuracy;
}__attribute__((packed));

/**
 * @brief  informs the application about a
                             WSMP/WSMPS reception

   This structure used to inform the application about the
   received WSMP(S) packets
 */
struct savariwme_rx_indication {
    /**
     * WAVE version number
     */
    int version;
    /**
     * timestamp at which the packet got received
     */
    uint64_t tstamp;
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
}__attribute__((packed));

/**
 * @brief WRA information - Ignore for WSMPs
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
} __attribute__((packed));

/**
 * @brief this structure is used as configuration for transmitting
                             WSMP/WSMPS over the air.

                             the configuration parameters will be
                             read from this structure and
                             the packet will be transmitted based on
                             the configuration.

                             any/all of the configurations can be modified
                             and given to the 1609.3 stack for subsequent transmission.
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
     *  when 1, reads and transmits safetysupp in the WSMP header
        when 0 it won't consider the safetysupp to transmit in the WSMP header
     */
    int supp_enable;
    /**
     * WSM safety supplement
     */
    uint32_t safetysupp;
}__attribute__((packed));

/**
 * @brief a set of callbacks associated with the application
                          about the indication of WSMs/commands etc.

   This is set to a list of function pointers, and they will be called from the
   library based on the communication protocol type between the caller and the 1609.3.

   The protocol sends a confirmation upon a call to wme_register_user(provider) request.

   the confirmation callback wme_user(provider)_confirm is called (if its a valid pointer)
   and the application can decide to transmit/receive WSMP.

   The protocol sends a WSMP decoded packet and fills into the savariwme_rx_indication and
   callback wme_wsm_indication is called.
 */
struct savariwme_cbs {
/**
 * @brief indicates to the application about the
                          provider registration Confirmation
  \param ctx - context of the application
  \param conf_result_indication - result of wme_register_provider. 0 - success non-zero : failure
 */
    void (*wme_provider_confirm)(void *ctx, int conf_result_indication);
/**
 * wme_user_confirm - indicates to the application about the user registration 
   \                - confirmation
   \param ctx - context of the application
   \param conf_result_indication - result of wme_register_user. 0 - success non-zero : failure
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

    void (*wme_get_wme_err) (void *ctx, int length, void *data, int cmd_indication);

    void (*wme_clear_wme_err) (void *ctx, int length, void *data, int cmd_indication);

    void (*wme_get_rse_info) (void *ctx, int length, void *data, int cmd_indication);

    void (*wme_get_avail_services) (void *ctx, int length, void *data, int cmd_indication);

    void (*wme_get_connected_rse_cnt) (void *ctx, int length, void *data, int cmd_indication);

    void (*wme_scanning) (void *ctx, int length, void *data);

    void (*wme_connected) (void *ctx, int length, void *data);

    void (*wme_disconnected) (void *ctx, int length, void *data);
};
/**
 * @brief initialises and connects to the 1609.3 stack
 *
   The wme_init initialises and connects to the 1609.3 stack
   and returns a savari_wme_handler_t handler. \n this handler is further
   used to transmit and receive messages to/from the 1609.3 stack. \n
   the messages may consist of a set of confirmations, or WSM data.
   \param serverip - serverip , the 1609.3 stack ip to connect to, default 127.0.0.1
   \param iface - on which interface the app wants to connect to the stack (ath0/ath1)
   \return - returns a handler of savari_wme_handler_t, and FAIL on failure
 */ 
int
wme_init(char *serverip, char *iface);
/**
 * @brief register the application as user
 *
   The wme_register_user registers the application in user mode
   to the stack for the purpose of sending /receiving 
   WSMs or joining a service.\n  the stack recognises the application using psids.

   \param handler - handler returned from wme_init
   \param wme_req - the wme_req structure from the application
   \par Description -
   \return - returns SUCCESS on success and FAIL on failure.   
 */
int
wme_register_user(savari_wme_handler_t handler,
            struct savariwme_reg_req *wme_req);
/**
 * @brief unregister the user from the 1609.3 stack
 *
   The wme_unregister_user unregisters the user application
   from the stack and stops receiving the WSMs or commands on
   behalf of the application.
   \param handler - handler returned from wme_init
   \param wme_req - the wme_req structure from wme_init
 */
void
wme_unregister_user(savari_wme_handler_t handler,
            struct savariwme_reg_req *wme_req);
/**
 * @brief registers a CCH access request
 *
   \param handler - a handler returned from wme_init
   \param intvl - CCH interval
   \param priority - priority  
   \return - returns SUCCESS on success and FAIL on failure.
 */
int
wme_register_cch_request(savari_wme_handler_t handler,
                    int intvl, int priority);
/**
 * @brief unregisters CCH access request
 * \param handler - a handler returned from wme_init
   \param intvl - CCH interval
   \param priority - priority
   \return - returns SUCCESS on success and FAIL on failure.
 */
int
wme_unregister_cch_request(savari_wme_handler_t handler,
                    int intvl , int priority);
/**
 * @brief confirm user application to transmit/receive WSMPs.
 *
   This function should be called by user application
   when it receives user confirm from the stack.
   \param handler - a handler returned from wme_init
   \param action - an action to specify whether wanted to do tx/rx or not.
                   action can be LIBWME_ACTION_ADD/LIBWME_ACTION_CHANGE/LIBWME_ACTION_DELETE
   \param wme_req - a pointer to the savariwme_reg_req structure, passed from
                    the application at the time of wme_register_user
   \return - returns SUCCESS on success and FAIL on failure.
 */
void wme_user_service_confirm(savari_wme_handler_t handler, int action,
                    struct savariwme_reg_req *wme_req);
/**
 * @brief registers the provider application
 *
   This function registers the application as a provider and
   makes it capable of transmitting/receiving WSMs and transmitting WSAs.
   \param handler - a handler returned from wme_init
   \param wme_req - a pointer to the savariwme_reg_req structure passed from
                    the application
   \return - returns SUCCESS on success and FAIL on failure.
 */
int wme_register_provider(savari_wme_handler_t handler,
            struct savariwme_reg_req *wme_req);
/**
 * @brief unregisters the provider
 *
   This function unregisters the provider application and
   the stack stops transmitting  the WSMs and WSAs on behalf of the
   application.

   \param handler - handler returned from wme_init
   \param wme_req - a savariwme_reg_req structure passed from the application
                    at the time of registering using the API
                    wme_register_provider
 */
void wme_unregister_provider(savari_wme_handler_t handler,
                struct savariwme_reg_req *wme_req);
/**
 * @brief confirm provider application to the 1609.3 stack
 *
   This function should be called by provider application
   when it receives provider confirm from the stack.
   \param handler - a handler returned from wme_init
   \param action - a action to specify whether wanted to do tx/rx or not.
   \param wme_req - a pointer to the savariwme_reg_req structure, passed from
                    the application at the time of wme_register_user
   \return - returns SUCCESS on success and FAIL on failure.
 */
void wme_provider_service_confirm(savari_wme_handler_t handler, int action,
                    struct savariwme_reg_req *wme_req);
/**
 * @brief transmits the WSMs
 *
   This function gets the tx_buffer from the application and
   then it transmits the message with the control parameters specified in
   the wme_wsm_tx to the 1609.3 stack.
 * \param handler - the handler returned from wme_init
   \param wme_wsm_tx - a pointer to the tx structure containing the transmission
                       parameters, such as channel, datarate, txpower, and 
                       transmission data length etc.
   \param tx_buffer - a pointer to the tx buffer from the application
   \return - returns SUCCESS on success and FAIL on failure. 
 */
int
wme_wsm_tx(savari_wme_handler_t handler, struct savariwme_tx_req *wme_wsm_tx,
                                    uint8_t *tx_buffer);
/**
 * @brief Receive notifications from the 1609.3 stack
 *
   This function should be called by the application whenever there is
   data available on wme_handler (return value from wme_init).

   This function, based on the received data will calls the corresponding
   callback with the application given context given in wme_rx.

   All the data operations are performed in context of wme_rx.
   \n   for an application provider confirmation, calls wme_provider_confirm
   \n   for an application user confirmation, calls wme_user_confirm
   \n   for a cch confirmation, calls wme_cch_confirm
   \n   when a WSM received, calls wme_wsm_indication
   \n   for a wme cmd responses, calls wme_cmd
   \param handler - the handler returned from the wme_init
   \param handle - this is a structure containing a set of function pointers.
   \param ctx - the context passed from the application
   \return - this function returns SUCCESS on success and FAIL on failure
 */  
int
wme_rx(savari_wme_handler_t handler, struct savariwme_cbs *handle, void *ctx);
/**
 * @brief converts the psid to bigendian
 *
 * The PSID must be converted to bigendian format before calling wme_register_provider(user) and
 * wme_wsm_tx.
   \param psid - psid to be converted to big endian format
   \return - returns the converted psid.
 */
uint32_t
wme_convert_psid_be(uint32_t psid);
/**
 * @brief gets the psid length
 *
   \param psid - an uint8_t pointer.
   \return - returns the length of psid on success, and 0 on failure.
 */
int wme_getpsidlen (uint8_t *psid);
/**
 * @brief Sends stack specific commands
 *
   This function processes the GET/CLEAR WME command specified by the
   application. It returns/clears PSID mismatch errors.
   \param handler - a handler returned from wme_init
   \param cmd - command sent from the application (GET_WME_ERROR/CLEAR_WME_ERROR)
 */
int
wme_process_wme_cmd(savari_wme_handler_t handler, int cmd, int subcmd);
/**
 * @brief sets the gps location information to the 1609.3 stack
 *
   This function sets the gps location speicified in the
   gpsinfo structure to the 1609.3 stack for the purpose of
   transmitting the location information in the 1609.3 WSA header. 
   \param handler - a handler returned from wme_init
   \param gpsinfo -
        The gpsdata passed by the user application. Application may get this
        by querying the gps stack, refer to libgpsapi on
        how to query the gps stack , by using the gps library apis.
   \return - returns SUCCESS on success and FAIL on failure.
 */

int
wme_set_gpslocation(savari_wme_handler_t handler,
                    struct libwme_gpsinfo *gpsinfo);
/**
 * @brief sets the WRA information to the 1609.3 stack 
 *
   This function sets the WRA information passed from the
   application to the 1609.3 stack for further transmissions
   in WSAs.
   \param handler - a handler returned from wme_init
   \param wra - the wra information passed by the application
   \return - returns SUCCESS on success and FAIL on failure.
 */

int
wme_set_inactivity_timer(savari_wme_handler_t handler,
                         int timer_value);

int
wme_set_wrainfo(savari_wme_handler_t handler,
                    struct savari1609Wra *wra);
/**
 * @brief sets the wsa interval
 *
   This function sets the wsa interval to transmit WSAs.
   \param handler - a handler returned from wme_init
   \param wsa_intvl - WSA interval 
   \return - returns SUCCESS on success and FAIL on failure. 
 */
int
wme_set_wsaintvl(savari_wme_handler_t handler, int wsa_intvl);
/**
 * @brief sets the txpower of a WSA
 *
   \param handler - a handler returned from wme_init
   \param txpower - transmit power of WSA
   \return - returns SUCCESS on success and FAIL on failure.
 */
int
wme_set_wsatxpower(savari_wme_handler_t handler, int txpower);
/**
 * @brief sets the advertiser id
 *
   This function sets the advertiser id of the WSA.
   \param handler- a handler returned from wme_init
   \param adv_id - advertiser id from the application to transmit in WSAs 
   \return - returns SUCCESS on success and FAIL on failure.
 */
int
wme_set_advertiserid(savari_wme_handler_t handler, char* adv_id);

/**
 * @brief deinitialise and disassociate the application from 1609.3 stack
   \param handler - a handler returned from wme_init
 */

void
wme_deinit(savari_wme_handler_t handler);

int
wme_send_wmectl(savari_wme_handler_t handler, int cmd, int subcmd, void *data, int len);

#ifdef __cplusplus
}
#endif

#endif
