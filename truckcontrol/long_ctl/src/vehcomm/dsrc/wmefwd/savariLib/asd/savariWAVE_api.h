#ifndef __SAVARIWAVE_API_H__
#define __SAVARIWAVE_API_H__

#include <endian.h>
#include <byteswap.h>
#include <stdint.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <error.h>
#include <fcntl.h>

#define SAVARI1609_SERV_PORT                        6060
#define SAVARI1609_ATH1_SERV_PORT                   6061
#define SAVARI1609_WME_NUM_AC           4
#define SAVARI1609_OCTET_STRING_LENGTH              32
#define SAVARI1609_IEEE80211_ADDR_LEN               6
#define SAVARI1609_MAX_APPLICATION_BUFFER_LEN       2999
#define    SAVARI1609_PSID_MAX_LEN                     4
#define SAVARI1609_SSPERMISSION_MAX_LEN                255
#define    SAVARI1609_SSI_MAX_LEN                     16
#define SAVARI1609_WSA_TYPE_SECURED                 0x0b
#define SAVARI1609_MAXLINE                          2000
#define SAVARI1609_MAX_WSMP_DATA                    1400
#define MAX_SIZE_ELEMS                          20 * 1000
#define MAX_RSE_NODES                           80




typedef enum {
    WAVE_ELEMID_SERVICEINFO     = 1,
    WAVE_ELEMID_CHANNELINFO     = 2,
    WAVE_ELEMID_WRA             = 3,
    WAVE_ELEMID_TXPOWER         = 4,
    WAVE_ELEMID_2DLOC           = 5,
    WAVE_ELEMID_3DLOC           = 6,
    WAVE_ELEMID_ADVERTISERID    = 7,
    WAVE_ELEMID_PSC             = 8,
    WAVE_ELEMID_IPV6ADDR        = 9,
    WAVE_ELEMID_SERVICEPORT     = 10,
    WAVE_ELEMID_MACADDR         = 11,
    WAVE_ELEMID_EDCA            = 12,
    WAVE_ELEMID_SDNS            = 13,
    WAVE_ELEMID_GWMACADDR       = 14,
    WAVE_ELEMID_CHANNEL         = 15,
    WAVE_ELEMID_DATARATE        = 16,
    WAVE_ELEMID_REPEAT          = 17,
    WAVE_ELEMID_COUNTRY         = 18,
    WAVE_ELEMID_RCPI_THRESH      = 19,
    WAVE_ELEMID_WSACOUNT_THRESH     = 20,
    WAVE_ELEMID_CHANNELACCESS   = 21,
    WAVE_ELEMID_WSACOUNT_THRESH_INTERVAL     = 22,
    WAVE_ELEMID_WSMP            = 128,
    WAVE_ELEMID_WSMPS           = 129,
    WAVE_ELEMID_WSMPI           = 130
} WAVE_ELEMID;


enum {
    SAVARI1609_PROVIDER_SERVICE_REQUEST = 0,
    SAVARI1609_PROVIDER_SERVICE_CONFIRM = 1,
    SAVARI1609_USER_SERVICE_REQUEST     = 2,
    SAVARI1609_USER_SERVICE_CONFIRM        = 3,
    SAVARI1609_WSM_SERVICE_REQUEST        = 4,    
    SAVARI1609_WSM_SERVICE_CONFIRM        = 5,        
    SAVARI1609_CCH_SERVICE_REQUEST        = 6,
    SAVARI1609_CCH_SERVICE_CONFIRM        = 7,    
    SAVARI1609_MGMT_SERVICE_REQUEST        = 8,
    SAVARI1609_MGMT_SERVICE_CONFIRM        = 9,
    SAVARI1609_TA_SERVICE_REQUEST        = 10,
    SAVARI1609_TA_SERVICE_CONFIRM        = 11,
    SAVARI1609_NOTIFICATION_INDICATION    = 12,
    SAVARI1609_GET_REQUEST                = 13,
    SAVARI1609_GET_CONFIRM                = 14,
    SAVARI1609_SET_REQUEST                = 15,
    SAVARI1609_SET_CONFIRM                = 16,
    SAVARI1609_ADDRESS_CHANGE_REQUEST    = 17,
    SAVARI1609_ADDRESS_CHANGE_CONFIRM    = 18,
    SAVARI1609_WSMP_REQUEST                = 19,
    SAVARI1609_WSMP_CONFIRM                = 20,
    SAVARI1609_WSMP_INDICATION            = 21,
    SAVARI1609_GET_CMD                    = 22,
    SAVARI1609_SET_CMD                    = 23,
};

enum {
    GET_WME_ERROR = 1,
    CLEAR_WME_ERROR = 2,
    GET_MATCHED_RSE_INFO = 3,
    GET_CONNECTED_RSE_POSITION = 4,
    GET_RSE_INFO = 5,
    GET_MATCHED_RSE_COUNT = 6,
};
enum {
    SET_GPS_LOCATION_INFO       = 0,
    SET_WSA_TXPOWER             = 1,
    SET_ADVERTISER_ID           = 2,
    SET_WRA_INFO                = 3,
    SET_WSA_INTVL               = 4,
    SET_WSA_INACT               = 5,
    SET_WSA_VERIFY_INTVL        = 6,
    SET_DEBUG_MODE              = 7,
    SET_WSA_FORCE_INACT         = 8,
    SET_WSA_VALIDITY_DISTANCE   = 9,
    SET_WSA_VALIDITY_PERIOD     = 10,
};

typedef enum {
    SAVARI1609_RC_ACCEPTED,
    SAVARI1609_RC_INVALID_PARAMETERS,
    SAVARI1609_RC_UNSPECIFIED
} SAVARI1609_RESULT_CODE;

enum {
    REASONCODE_NONE,
    CONNECTED_ON_USER_REQUEST,
    DISCONNECTED_USR_REQUESTED,
    DISCONNECTED_CHAN_INACT,
    PARAMETER_MISMATCH,
    PICKED_OTHER_RSE,
};
struct savari1609ProviderServiceContext
{
    uint8_t length;
    char pscContents[SAVARI1609_OCTET_STRING_LENGTH];
}__attribute__((packed));

struct savari1609channelAccess
{
    uint8_t cch;
    uint8_t sch;
    uint8_t cchintvl;
    uint8_t schintvl;
}__attribute__((packed));

struct savari1609EdcaParams {
    u_int8_t    acm;       /* ACM parameter */
    u_int8_t    aifsn;     /* AIFSN parameters */
    u_int8_t    logcwmin;      /* cwmin in exponential form */
    u_int8_t    logcwmax;      /* cwmax in exponential form */
    u_int16_t   txopLimit;     /* txopLimit */
}__attribute__((packed));

struct savari1609AdvertiserID
{
    uint8_t length;
    char contents[SAVARI1609_OCTET_STRING_LENGTH];
}__attribute__((packed));

struct savari1609_3DLocation {
    int latitude;
    int longitude;
    int16_t elevation;
    uint8_t position_confidence:4;
    uint8_t elevation_confidence:4;
    uint32_t positional_accuracy;

}__attribute__((packed));

struct savari1609_2DLocation {
    int latitude;
    int longitude;
}__attribute__((packed));

struct savari1609UsrAvailableEntry {
    uint8_t wsatype;
    uint8_t resultcode;
    uint8_t waveversion;
    uint64_t    generationtime;
    uint64_t    lifetime;
    uint64_t    expectedtime;
    uint8_t srcmacaddr[SAVARI1609_IEEE80211_ADDR_LEN];
    uint8_t psid[SAVARI1609_PSID_MAX_LEN];
    uint8_t sspermissions[SAVARI1609_SSPERMISSION_MAX_LEN];
    struct savari1609ProviderServiceContext psc;
    uint8_t priority;
    struct in6_addr primarydns;
    struct in6_addr secondarydns;
    uint8_t gwmac[SAVARI1609_IEEE80211_ADDR_LEN];
    struct in6_addr dfl_gateway;
    struct in6_addr ipprefix;
    int prefix_len;
    struct in6_addr service_ipv6addr;
    uint16_t service_port;
    uint8_t macaddr[SAVARI1609_IEEE80211_ADDR_LEN];
    uint8_t rcpi;
    uint8_t rcpi_thresh;
    uint8_t wsacount_thresh;
    uint8_t wsacount_thresh_intvl;
    uint8_t regclass;
    uint8_t channel;
    uint8_t adaptable;
    uint8_t datarate;
    int8_t  txpower;
    uint8_t channel_access;
    struct savari1609AdvertiserID advertiserid;
    int txlatitude;
    int txlongitude;
    int txelevation;
    uint8_t txposition_confidence:4;
    uint8_t txelevation_confidence:4;
    uint32_t txposition_accuracy;
    uint8_t linkqual;
    uint8_t status;
    uint8_t flags;
    struct savari1609EdcaParams edcaparams[SAVARI1609_WME_NUM_AC];
    struct savari1609channelAccess chanaccess;
    int mask;
    struct savari1609_2DLocation loc2d;
    struct savari1609_3DLocation loc3d;
}__attribute__((packed));

struct savari1609AvailServices {
    uint8_t srcmacaddr[SAVARI1609_IEEE80211_ADDR_LEN];
    uint8_t psid[SAVARI1609_PSID_MAX_LEN];
    struct savari1609ProviderServiceContext psc;
    uint8_t priority;
    struct in6_addr primarydns;
    struct in6_addr secondarydns;
    uint8_t gwmac[SAVARI1609_IEEE80211_ADDR_LEN];
    struct in6_addr dfl_gateway;
    struct in6_addr ipprefix;
    int prefix_len;
    struct in6_addr service_ipv6addr;
    uint16_t service_port;
    uint8_t rcpi;
    uint8_t rcpi_thresh;
    uint8_t wsacount_thresh;
    uint8_t wsacount_thresh_intvl;
    uint8_t regclass;
    uint8_t channel;
    uint8_t adaptable;
    uint8_t datarate;
    int8_t  txpower;
    uint8_t channel_access;
    struct savari1609AdvertiserID advertiserid;
    int txlatitude;
    int txlongitude;
    int txelevation;
    uint8_t txposition_confidence:4;
    uint8_t txelevation_confidence:4;
    uint32_t txposition_accuracy;
    uint8_t linkqual;
    int mask;
    int wsatype;
    struct savari1609_2DLocation loc2d;
    struct savari1609_3DLocation loc3d;
}__attribute__((packed));

struct savari1609Avail_RSE_List {
    int n_services;
    struct savari1609AvailServices services[MAX_SIZE_ELEMS];
}__attribute__((packed));

struct savari1609WSAInfo_buf {
    uint8_t pending;
    uint8_t data[SAVARI1609_MAX_APPLICATION_BUFFER_LEN];
} __attribute__((packed));

struct savari1609ApplicationBuffer {
    uint8_t type;
    uint16_t length;
    uint8_t data[MAX_SIZE_ELEMS];
}__attribute__((packed));

struct savariWmeCmd {
    uint8_t cmd;
    uint16_t length;
    uint8_t data[0];
}__attribute__((packed));

struct savariWmeErrorStats {
    uint32_t psid_mismatch;
}__attribute__((packed));

struct savari1609RSELoc_info {
    int     mask;
    int     latitude;
    int     longitude;
    int     elevation;
    uint64_t    wsa_last_updated;
} __attribute__((packed));

struct rseNode {
    int     mask;
    uint8_t macaddr[SAVARI1609_IEEE80211_ADDR_LEN];
    int32_t latitude;
    int32_t longitude;
    int32_t elevation;
    int     rssi;
    int reason_code;
    int     verification_fail_cnt;
    int     wsa_count;
    int     verification_status;
    int verification_cnt;
    uint64_t wsa_last_update_tstamp_msecs;
} __attribute__((packed));

struct savari1609RSE_Node_Info {
    int n_nodes;
    struct rseNode rsenodes[MAX_RSE_NODES];
} __attribute__((packed));

struct savari1609RSE_info {
    struct savari1609RSELoc_info info;
    uint8_t rse_mac[SAVARI1609_IEEE80211_ADDR_LEN];
    int wsa_count;
    int sec_verification_failures;
    int rssi;
    int verification_status;
    int reason_code;
} __attribute__((packed));

#define SAVARI1609_WMECMD_HDR_LEN \
        (sizeof(struct savariWmeCmd) -  \
         sizeof(((struct savariWmeCmd *)0)->data))

#define SAVARI1609_APPLICATION_BUFFER_HDR_LEN \
        (sizeof(struct savari1609ApplicationBuffer) -  \
         sizeof(((struct savari1609ApplicationBuffer *)0)->data))
enum {
    SAVARI1609_CCH_INTERVAL = 0x1,
    SAVARI1609_SCH_INTERVAL = 0x10,
    SAVARI1609_UNKNOWN_INTERVAL = 0xFF,
};

enum {
    SAVARI1609_FALSE,
    SAVARI1609_TRUE
};

enum {
    SAVARI1609_CHANNEL_ACCESS_CONTINUOUS = 0,
    SAVARI1609_CHANNEL_ACCESS_ALTERNATING,
    SAVARI1609_CHANNEL_ACCESS_UNKNOWN = 0xFF,
} SAVARI1609_CHANNEL_ACCESS;

enum {
    SAVARI1609_ACTION_ADD,
    SAVARI1609_ACTION_DELETE,
    SAVARI1609_ACTION_CHANGE,
} SAVARI1609_ACTION;

enum {
    SAVARI1609_SRS_PENDING,
    SAVARI1609_SRS_SATISFIED,
    SAVARI1609_SRS_PARTIALLYSATISFIED,
} SAVARI1609_SERVICEREQUEST_STATUS;

enum {
    SAVARI1609_WSA_UNSECURED = 0,
    SAVARI1609_WSA_SECURED = 1,
} SAVARI1609_WSATYPE;

enum {
    SAVARI1609_USER_AUTOACCESS_ONMATCH,
    SAVARI1609_USER_AUTOACCESS_UNCOND,
    SAVARI1609_USER_NOSCHACCESS
} SAVARI1609_USERREQUEST;

enum {
    SAVARI1609_UA_AVAILABLE,
    SAVARI1609_UA_ACTIVE
} SAVARI1609_USERAVAILABLE_STATUS;

enum {
    SAVARI1609_UARC_SUCCESS,
    SAVARI1609_UARC_INVALID_INPUT,
    SAVARI1609_UARC_CERT_NOTFOUND,
    SAVARI1609_UARC_UNSUPPORTED_SIGNERTYPE,
    SAVARI1609_UARC_NOT_MOSTRECENT_WSA,
    SAVARI1609_UARC_COULDNOT_CONSTRUCTCHAIN,
    SAVARI1609_UARC_INCORRECT_CACERT_TYPE,
    SAVARI1609_UARC_INCORRECT_CACERT_SUBJECTTYPE,
    SAVARI1609_UARC_INCONSISTENT_PERMISSIONS,
    SAVARI1609_UARC_INCONSISTENT_GEOGRAPHIC_SCOPE,
    SAVARI1609_UARC_UNAUTH_GENERATION_LOCATION,
    SAVARI1609_UARC_UNAUTH_PSID_PRIORITY,
    SAVARI1609_UARC_REVOKED_CERTIFICATE,
    SAVARI1609_UARC_NOUPTODATE_CRL,
    SAVARI1609_UARC_UNAVAILABLE_PERMISSIONS,
    SAVARI1609_UARC_UNAVAILABLE_GEOGRAPHIC_SCOPE,
    SAVARI1609_UARC_CERTIFICATE_VERIFICATION_FAILED,
    SAVARI1609_UARC_MESSAGE_VERIFICATION_FAILED,
    SAVARI1609_UARC_OTHER
}SAVARI1609_USERAVAILABLE_RESULTCODE;

typedef enum {
    SAVARI1609_CHANNEL_ASSIGNED,
    SAVARI1609_NOCHANNEL_ASSIGNED,
    SAVARI1609_REQUEST_MATCH
} SAVARI1609_EVENT_TYPE;

typedef enum {
    SAVARI1609_UNSPECIFIED,
    SAVARI1609_REQUESTED,
    SAVARI1609_CHANNEL_UNAVAILABLE,
    SAVARI1609_SERVICE_COMPLETE,
    SAVARI1069_REQUEST_FRAME_SCHEDULED,
    SAVARI1609_PRIORITY_PREEMPTION,
    SAVARI1609_SECURITY_CREDENTIAL_FAILURE
} SAVARI1609_REASON_TYPE;

typedef enum {
    SAVARI1609_WSA_EXT_REPEATRATE = 0x0001,
    SAVARI1609_WSA_EXT_TXPOWER      = 0x0002,
    SAVARI1609_WSA_EXT_2DLOC = 0x0004,
    SAVARI1609_WSA_EXT_3DLOC = 0x0008,
    SAVARI1609_WSA_EXT_ADVERTISERID = 0x0010,
    SAVARI1609_WSA_EXT_COUNTRY    = 0x0020,
} SAVARI1609_WSA_EXT;

typedef enum {
    SAVARI1609_WSM_EXT_CHANNEL      = 0x0001,
    SAVARI1609_WSM_EXT_DATARATE      = 0x0002,
    SAVARI1609_WSM_EXT_TXPOWER      = 0x0004,
} SAVARI1609_WSM_EXT;

struct savari1609ChannelIdentifier 
{
    uint8_t country[3];
    uint8_t regclass;
    uint8_t channel;
}__attribute__((packed));

/* Application -> WME  */
struct savari1609ProviderServiceRequest {
    uint32_t    localserviceindex;
    uint8_t        action;
    uint8_t        destmacaddr[SAVARI1609_IEEE80211_ADDR_LEN];
    uint8_t        wsatype;
    uint8_t        psid[SAVARI1609_PSID_MAX_LEN];
    struct         savari1609ProviderServiceContext psc;
    uint8_t        priority;
    uint8_t        sspermissions[SAVARI1609_SSPERMISSION_MAX_LEN];
    // SSI is not needed any more
    //uint8_t        ssi[SAVARI1609_SSI_MAX_LEN];
    struct savari1609ChannelIdentifier    channelid;
    uint8_t        channel_access;
    uint8_t        repeatrate;
    uint8_t        ipservice;
    struct in6_addr service_ipv6addr;
    uint16_t     service_port;
    uint8_t        srcmacaddr[SAVARI1609_IEEE80211_ADDR_LEN];
    uint8_t        rcpi_thresh;
    uint8_t        wsacount_thresh;
    uint8_t        wsacount_thresh_intvl;
    uint32_t    wsa_header_extensions;    
    uint32_t    signature_lifetime;
    uint32_t    secondradio;
}__attribute__((packed));

struct savari1609UserServiceRequest {
    uint32_t    localserviceindex;
    uint8_t        action;
    uint8_t        request_type;
    uint8_t        psid[SAVARI1609_PSID_MAX_LEN];
    uint8_t        priority;
    uint8_t        wsatype;
    struct         savari1609ProviderServiceContext psc;
    struct savari1609ChannelIdentifier    channelid;
    uint8_t        srcmacaddr[SAVARI1609_IEEE80211_ADDR_LEN];
    struct         savari1609AdvertiserID advertiserid;
    uint8_t        linkqual;
    uint8_t        immediate_access;
    uint16_t    extended_access;
    uint32_t    secondradio;
}__attribute__((packed));


struct savari1609WSMServiceRequest {
    uint32_t    localserviceindex;
    uint8_t        action;
    uint8_t        psid[SAVARI1609_PSID_MAX_LEN];
}__attribute__((packed));

struct savari1609CchServiceRequest {
    uint32_t    localserviceindex;
    uint8_t        action;
    uint8_t        channel_interval;
    uint8_t        priority;
}__attribute__((packed));

struct savari1609MgmtServiceRequest {
    uint32_t    localserviceindex;
    uint8_t        action;
    uint8_t        destmacaddr[SAVARI1609_IEEE80211_ADDR_LEN];
    uint8_t        mgmtid;
    uint8_t        repeatrate;
    struct savari1609ChannelIdentifier    channelid;
    uint8_t        channel_interval;
    uint8_t        orgid[5];
    uint8_t        data[250];
    uint8_t        priority;
}__attribute__((packed));


struct savari1609TAServiceRequest {
    uint32_t    localserviceindex;
    uint8_t        action;
    uint8_t        destmacaddr[SAVARI1609_IEEE80211_ADDR_LEN];
    uint8_t        repeatrate;
    struct savari1609ChannelIdentifier    channelid;
    uint8_t        channel_interval;
    uint8_t        priority;
    uint8_t        tac[0];
}__attribute__((packed));

struct savari1609GetRequest {
    uint32_t    attribute;
}__attribute__((packed));

struct savari1609SetRequest {
    uint32_t    attribute;
}__attribute__((packed));

struct savari1609AddressChangeRequest {
    struct in6_addr service_ipv6addr;
}__attribute__((packed));

struct savari1609WsmRequest {
    struct savari1609ChannelIdentifier channelid;
    uint8_t        datarate;
    int8_t        txpower;
    uint8_t        psid[SAVARI1609_PSID_MAX_LEN];
    uint8_t        priority;
    uint64_t    wsmexpirytime;
    uint8_t        macaddr[SAVARI1609_IEEE80211_ADDR_LEN];
    uint32_t    header_ext;
    uint8_t        elemid;
    uint16_t    length;
    uint32_t     safetysupp;
    uint8_t        data[0];
}__attribute__((packed));


/* WME --> Client */
struct savari1609MgmtDataServiceIndication {
    uint8_t        srcmacaddr[SAVARI1609_IEEE80211_ADDR_LEN];
    uint8_t        mgmtid;
    uint8_t        rcpi;
    struct savari1609ChannelIdentifier    channelid;
    uint8_t        orgid[5];
    uint8_t        data[250];
}__attribute__((packed));

struct savari1609NotificationIndication {
    uint8_t    event;
    uint32_t    localserviceindex;
    uint8_t    reason;
}__attribute__((packed));

struct savari1609ServiceConfirm {
    uint32_t    localserviceindex;
    uint8_t        result;
}__attribute__((packed));


struct savari1609WsmIndication {
    uint8_t        version;
    uint8_t        channel;
    uint8_t     datarate;
    int8_t        txpower;
    uint8_t        psid[SAVARI1609_PSID_MAX_LEN];
    uint8_t        priority;
    int32_t     rssi;
    uint32_t    plcplen;
    uint32_t    dot11prate;
    uint32_t    ts_sec;
    uint32_t    ts_usec;
    uint16_t    length;
    uint8_t        macaddr[SAVARI1609_IEEE80211_ADDR_LEN];
    uint8_t     elemid;
    uint8_t        data[0];
}__attribute__((packed));


typedef enum {
    SAVARI1609_WME_ERR_NOT_ENOUGH_BUF  = -2,
    SAVARI1609_WME_ERR_NOT_ENOUGH_DATA = -3,
} SAVARI1609_WME_ERR;

struct savari16093gpsinfo {
    int latitude;
    int longitude;
    int elevation;
    double pos_confidence;
    double elev_confidence;
    int positional_accuracy;
}__attribute__((packed));

struct __savari1609Wra {
    uint16_t  lifetime;
    struct in6_addr ipv6addr;
    uint8_t prefixlen;
    struct in6_addr default_gw;
    uint8_t gw_macaddr[SAVARI1609_IEEE80211_ADDR_LEN];
    struct in6_addr primarydns;
    struct in6_addr secondarydns;
} __attribute__((packed));

#endif
