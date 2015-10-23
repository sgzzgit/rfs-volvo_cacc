/**
 * @file libradio.h
 * @brief        This library has APIs and structures to get/set interface params.
	\n		 Link with -lradio to use this library.
	\n	 this library can be used to access information form
			 a variety of interfaces.
			 this includes ethernet, dsrc, and wireless.
	\n		 so the following things can be done by using this APIs.
	\n		 getting /setting interface MAC,
	\n		 getting /setting CCH MAC of a dsrc device,
	\n		 setting SCH MAC of a dsrc device,
	\n		 getting /setting of 11p wmm parameters,
	\n		 getting /setting of dsrc channel (ex. 172, 174 e.t.c).
	\n		 getting /setting interface ip, and so on..
 */

#ifndef __RADIO_H__
#define __RADIO_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * the success status of an API in this lib
 */
#define SUCCESS 0
/**
 * the failed status of an API in this lib
 */
#define FAIL -1

#define MEGA 1e6
#define GIGA 1e9

#define AUTO_RATE -1

/**
 * returned by savari_iface_up, meaning the device is already up
 */
#define DEV_ALREADY_UP 1
/**
 * returned by savari_iface_down, meaning the device is already down
 */
#define DEV_ALREADY_DOWN 2

#define NOT_SUPP -2

/**
 * MAC Address length
 */
#define MACADDR_LEN 6

#define IPADDR_LEN 4

typedef int savari_radio_t;
typedef savari_radio_t savari_radio_handler_t;

typedef enum savari_dbg_level {
	LIBRADIO_INFO,
	LIBRADIO_DEBUG,
	LIBRADIO_ERROR,
} savari_dbg_levels_t;

typedef enum savari_debug_flags {
	SOCKET_FAIL,
	IOCTL_FAIL_GET_ETHMAC,
	IOCTL_FAIL_SET_ETHMAC,
	IOCTL_FAIL_GET_CCHMAC,
	IOCTL_FAIL_SET_CCHMAC,
	IOCTL_FAIL_SET_SCHMAC,
	IOCTL_FAIL_GET_WMMPARAMS,
	IOCTL_FAIL_SET_WMMPARAMS,
	IOCTL_FAIL_GET_CHANNEL,
	IOCTL_FAIL_SET_CHANNEL,
	IOCTL_FAIL_GET_SERVCHAN,
	IOCTL_FAIL_SET_SERVCHAN,
	IOCTL_FAIL_SET_MODE,
	IOCTL_FAIL_GET_TSF,
	IOCTL_FAIL_SET_TSF,
	IOCTL_FAIL_GET_CHANNEL_STATS,
	IOCTL_FAIL_GET_IPADDR,
	IOCTL_FAIL_SET_IPADDR,
	IOCTL_FAIL_GET_DATARATE,
	IOCTL_FAIL_SET_DATARATE,
	IOCTL_FAIL_GET_TXPOWER,
	IOCTL_FAIL_SET_TXPOWER,
	IOCTL_FAIL_SET_IFACE_DOWN,
	IOCTL_FAIL_SET_IFACE_UP,
	IOCTL_FAIL_GET_IFACE_FLAGS,
	IOCTL_FAIL_GET_NETMASK,
	OP_NOT_SUPP,
} savari_dbg_flags_t;

/**
 * savari_radio_11p_param_t - 11p wmm parameter structure
 */
typedef struct savari_11p_param {
	/**
	 * transmit power
	 */
	uint32_t	txpower;
	/**
	 * channel spacing
	 */
	uint32_t	channelspacing;
	/**
	 * datarate
	 */
	uint32_t	datarate;
	/**
	 * BE param - cwmin
	 */
	uint32_t    ac_be_cwmin;
	/**
	 * BE param - cwmax
	 */
	uint32_t    ac_be_cwmax;
	/**
	 * BE param - aifsn
	 */
	uint32_t    ac_be_aifsn;
	/**
	 * BE param - txoplimit
	 */
	uint32_t    ac_be_txoplimit;
	/**
	 * BK param - cwmin
	 */
	uint32_t    ac_bk_cwmin;
	/**
	 * BK param - cwmax
	 */
	uint32_t    ac_bk_cwmax;
	/**
	 * BK param - aifsn
	 */
	uint32_t    ac_bk_aifsn;
	/**
	 * BK param - txoplimit
	 */
	uint32_t    ac_bk_txoplimit;
	/**
	 * VI param - cwmin
	 */
	uint32_t    ac_vi_cwmin;
	/**
	 * VI param - cwmax
	 */
	uint32_t    ac_vi_cwmax;
	/**
	 * VI param - aifsn
	 */
	uint32_t    ac_vi_aifsn;
	/**
	 * VI param - txoplimit
	 */
	uint32_t    ac_vi_txoplimit;
	/**
	 * VO param - cwmin
	 */
	uint32_t    ac_vo_cwmin;
	/**
	 * VO param - cwmax
	 */
	uint32_t    ac_vo_cwmax;
	/**
	 * VO param - aifsn
	 */
	uint32_t    ac_vo_aifsn;
	/**
	 * VO param - txoplimit
	 */
	uint32_t    ac_vo_txoplimit;
} savari_radio_11p_param_t;

/**
 * savari_wmm_flags_t - to be used to get/set the wmm params
						on a particular channel
 */
typedef enum savari_wmm_flags {
	/**
	 * CCH wmm parameters
	 */
	CCH_WMM_PARAMS,
	/**
	 * SCH wmm parameters
	 */
	SCH_WMM_PARAMS,
} savari_wmm_flags_t;

/**
 * savari_chanmode_t - enumed value , channel modes
 */
typedef enum savari_chanmode {
	/**
	 * continous channel mode
	 */
	CONTINUOUS,
	/**
	 * alternating forced channel mode
	 */
	ALTERNATING_FORCED,
	/**
	 * alternating conditional channel mode
	 */
	ALTERNATING_CONDITIONAL,
} savari_chanmode_t;

/**
 * struct savari_stats - information about the channel stats
 */
struct savari_stats {
	/**
	 * CCH cyclic counter
	 */
	uint32_t cch_cc;
	/**
	 * CCH recv counter
	 */
	uint32_t cch_rc;
	/**
	 * CCH rx frame
	 */
	uint32_t cch_rf;
	/**
	 * CCH tx frame
	 */
	uint32_t cch_tf;
	/**
	 * SCH cyclic counter
	 */
	uint32_t sch_cc;
	/**
	 * SCH recv counter
	 */
	uint32_t sch_rc;
	/**
	 * SCH rx frame
	 */
	uint32_t sch_rf;
	/**
	 * SCH tx frame
	 */
	uint32_t sch_tf;
} __attribute__((__packed__));

void
LIBRADIO_PRINTF (savari_dbg_levels_t level, savari_dbg_flags_t flag);
/**
 * savari_get_eth_mac - gets interface mac
 * \param ifname - valid interface name
 * \param mac - MAC Address returned by library, must be valid pointer
				from application
 * \par Description -
			  This function gets the MAC Address of the interface, and
			  it fills the MAC address into the second argument, the mac pointer
			  should at least be 6 bytes in length.
 * \return SUCCESS on success, \n FAIL on failure.
 */
int savari_get_eth_mac(char *ifname, uint8_t *mac);
/**
 * savari_set_eth_mac - sets interface mac
 * \param ifname - valid interface name
 * \param mac - MAC Address to set for interface specified by ifname
 * \par Description -
			  This function sets the MAC Address of the interface, and
			  it fills the MAC Address into the second argument, the mac pointer
			  should at least be 6 bytes in length.
 * \return SUCCESS on success, \n FAIL on failure.
 */
int savari_set_eth_mac(char *ifname, uint8_t *mac);

/**
 * savari_get_cch_mac - gets interface Control Channel MAC
 * \param ifname - valid interface name
 * \param mac - MAC Address returned by library, must be valid pointer
				from application
 * \par Description -
			  This function gets the CCH MAC address of the interface,
			  and it fills the MAC Address into the second argument, the mac pointer
			  should at least be 6 bytes in length.
   \note 	  This function is driver
			  specific and works only for a Atheros DSRC based driver.
 * \return SUCCESS on success, \n FAIL on failure.
 */
int savari_get_cch_mac(char *ifname, uint8_t *mac);

/**
 * savari_set_cch_mac - sets interface Control Channel MAC
 * \param ifname - valid interface name
 * \param mac - MAC Address specified by the application to set for control channel. It must be valid pointer and valid MAC Address from the application
 * \par Description -
			  This function sets the CCH MAC address of the interface.
   \note	  This function is driver
			  specific and works only for a Atheros DSRC based driver.
 * \return SUCCESS on success, \n FAIL on failure.
 */
int savari_set_cch_mac(char *ifname, uint8_t *mac);

/**
 * savari_set_sch_mac - gets interface SCH MAC
 * \param ifname - valid interface name
 * \param mac - MAC Address specified by the application to set for service channel. It must be valid pointer
				, the MAC Address must be a valid MAC Address from application
 * \par Description -
			  This function gets the MAC Address of the interface.
   \note      This function is driver
			  specific and works only for a Atheros DSRC based driver.
 * \return SUCCESS on success, \n FAIL on failure.
 */
int savari_set_sch_mac(char *ifname, uint8_t *mac);

/**
 * savari_get_wmm_params - gets EDCA parameters of a specified ac on interface
 * \param ifname - valid interface name
 * \param param - EDCA params returned by library. Must be valid pointer
				from application
   \param flag - flag shall be one of CCH_WMM_PARAMS and SCH_WMM_PARAMS to retrieve CCH or SCH params respectively.
 * \par Description -
			  This function gets the wmm parameters for a specified  channel
			  from the interface
   \note	  This function is driver
			  specific and works only on Atheros DSRC based drivers.
 * \return SUCCESS on success, \n FAIL on failure.
 */
int savari_get_wmm_params(char *ifname, savari_radio_11p_param_t *param,
							savari_wmm_flags_t flag);

/**
 * savari_set_wmm_params - sets EDCA parameters of a
						   specified parameter in a specified ac on interface
 * \param ifname - valid interface name
 * \param param - EDCA params from the application. Must be valid pointer
    from application. All params (BE/BK/VI/VO) should be specified.
    For ex. to set aifsn of be the param->ac_be_aifsn, should be set.
 * \param flag - flag shall be set to one of CCH_WMM_PARAMS and SCH_WMM_PARAMS to set CCH or SCH parameters.
 * \par Description -
			  This function sets one of the wmm params
			  of a specified AC on the interface
   \note	  This function is driver
			  specific, and it works only on Atheros DSRC based drivers.
 * \return SUCCESS on success, \n FAIL on failure.
 */
int savari_set_wmm_params(char *ifname, savari_radio_11p_param_t *param,
							savari_wmm_flags_t flag);

/**
 * savari_get_channel - gets channel from interface
 * \param ifname - valid interface name
 * \param chan_num - chan_num returned by library, must be valid pointer
				from application
 * \par Description -
			  This function gets the channel number from the interface,
			  and it fills the channel into the second argument
 * \return SUCCESS on success, \n FAIL on failure.
 */
int savari_get_channel(char *ifname, int *chan_num);
/**
 * savari_set_channel - sets channel on interface
 * \param ifname - valid interface name
 * \param chan_num - valid chan_num (channel number) from the application
 * \par Description -
			  This function sets the channel to the interface.
			  The interface must be one of ath0/ath1.
 * \return SUCCESS on success, \n FAIL on failure.
 */
int savari_set_channel(char *ifname, int chan_num);

/**
 * savari_set_srvice_channel - sets the service channel on interface
 * \param ifname - valid interface name
 * \param channel - channel from the application
 * \par Description -
			  This function sets the service channel on the interface.
			  The interface must be dsrc interface.
   \note	  This function works only when
			  the drivver is Atheros DSRC based one.
 * \return SUCCESS on success, \n FAIL on failure.
 */
int savari_set_service_channel(char *ifname, int channel);

/**
 * savari_get_mode - gets channel mode
 * \param ifname - valid interface name
 * \param mode - mode returned by library. Valid pointer must be passed from application
 * \par Description -
			  This function gets the mode of the interface, and
			  it fills the mode into the second argument
   		\n	  the mode will be 
		\n    0 -> continuous mode
		\n	  1 -> alternating forced
		\n	  2 -> alternating conditional
   \note	  This function is driver
			  specific, and works only on Atheros DSRC based driver.
 * \return SUCCESS on success, \n FAIL on failure.
 */
int savari_get_mode(char *ifname, savari_chanmode_t *mode);
/**
 * savari_set_mode - sets channel mode 
 * \param ifname - valid interface name
 * \param mode - mode specified by the application of type savari_chanmode_t
 * \par Description -
			  This function sets the channel mode of the interface
		\n	  the mode should be
		\n	  0 -> continous mode
		\n	  1 -> alternating forced
		\n	  2 -> alternating conditional
   \note	  This function is driver
			  specific, and works only on Atheros DSRC based driver.
 * \return SUCCESS on success, \n FAIL on failure.
 */
int savari_set_mode(char *ifname, savari_chanmode_t mode);

/**
 * savari_get_tsf - gets MAC Timing Synchronization Function (TSF) timer from interface
 * \param ifname - valid interface name
 * \param value - value returned by library. Application must pass valid pointer.
 * \par Description -
			  This function gets the tsf timer from the interface, and
			  it fills the tsf timer into the second argument
   \note	  This function is driver
			  specific, and works only on Atheros DSRC based driver.
 * \return SUCCESS on success, \n FAIL on failure.
 */
int savari_get_tsf(char *ifname, uint32_t *value);
/**
 * savari_set_tsf - sets MAC Timing Synchronization Function (TSF) timer from interface
 * \param ifname - valid interface name
 * \param value - timer value from the application
 * \par Description -
			  This function sets the TSF timer to the specified value.
   \note	  This function is driver
			  specific, and works only on Atheros DSRc based driver.
 * \return SUCCESS on success, \n FAIL on failure.
 */
int savari_set_tsf(char *ifname, uint32_t value);

/**
 * savari_get_channel_stats - gets channel statistics from interface
 * \param ifname - valid interface name
 * \param stats - a valid struct savari_stats pointer variable
 * \par Description -
					  This function gets the channel statistics from the
					  interface.
			  		  Description is specified in savari_stats about
					  each and every member.
   \note	  this function is driver
			  specific, and works only on Atheros DSRc based driver.
 * \return SUCCESS on success, \n FAIL on failure.
 */
int savari_get_channel_stats(char *ifname, struct savari_stats *stats);

/**
 * savari_get_ipaddr - gets interface ipaddress
 * \param ifname - valid interface name
 * \param ipaddr - Ip address returned by library. Application must pass valid pointer. It is in IPv4 dotted notation.
 * \par Description -
			  This function gets the ip address of the interface, and
			  it fills the ip address into the second argument.
 * \return SUCCESS on success, \n FAIL on failure.
 */
int savari_get_ipaddr(char *ifname, char *ipaddr);
/**
 * savari_set_ipaddr - sets interface ipaddress
 * \param ifname - valid interface name
 * \param ipaddr - Ipaddress specified by the application. It is in IPv4 dotted notation.
 * \par Description -
			  This function sets the ip address of the interface, the
			  ip address must be a valid IPv4 address type.
 * \return SUCCESS on success, \n FAIL on failure.
 */
int savari_set_ipaddr(char *ifname, char *ipaddr);

/**
 * savari_get_datarate - gets interface datarate
 * \param ifname - valid interface name
 * \param rate - data rate returned by library. Application must pass valid pointer.
 * \par Description -
			  This function gets the data rate of the interface, and
			  it fills the rate into the second argument,
			  the interface must be a ath0/ath1 interface
 * \return SUCCESS on success, \n FAIL on failure.
 */
int savari_get_datarate(char *ifname, int *rate);
/**
 * savari_set_datarate - sets interface datarate
 * \param ifname - valid interface name
 * \param rate - data rate set by the application
 * \par Description -
					  This function sets the data rate of the interface, the
					  interface must be one of ath0/ath1 interface.
 * \return SUCCESS on success, \n FAIL on failure.
 */
int savari_set_datarate(char *ifname, int rate);

/**
 * savari_get_txpower - gets interface txpower
 * \param ifname - valid interface name
 * \param power - txpower returned by library. Application must pass valid pointer.
 * \par Description -
			  This function gets the txpower of the interface, and
			  it fills the txpower into the second argument. The interface
			  must be one of ath0/ath1 interface.
 * \return SUCCESS on success, \n FAIL on failure.
 */
int savari_get_txpower(char *ifname, int *power);
/**
 * savari_set_txpower - sets interface txpower
 * \param ifname - valid interface name
 * \param power - txpower set by the application
 * \par Description -
			  This function sets the txpower of the interface,
			  the interface must be one of ath0/ath1 interface.
 * \return SUCCESS on success, \n FAIL on failure.
 */
int savari_set_txpower(char *ifname, int power);

/**
 * savari_iface_up - sets interface up
 * \param ifname - valid interface name
 * \par Description -
			  This function sets the interface to up 
			  if its down.
 * \return    SUCCESS on success, \n FAIL on failure.
		   \n DEV_ALREADY_UP if the interface is up already and
			  the application wants to make it up again.
 */
int savari_iface_up(char *ifname);
/**
 * savari_iface_down - sets interface down
 * \param ifname - valid interface name
 * \par Description -
			  This function sets the interface to down
			  if its up.
 * \return   SUCCESS on success, \n FAIL on failure.
		  \n DEV_ALREADY_DOWN if the interface is down already and
			 the application wants to make it down again
 */
int savari_iface_down(char *ifname);

/**
 * savari_get_netmask - gets interface networkmask 
 * \param ifname - valid interface name
 * \param netmask - network mask returned by library. Application must pass valid pointer. It will be in dotted IPv4 notation.
 * \par Description -
			  This function gets the network mask of the interface, and
			  it fills the network mask into the second argument.
 * \return SUCCESS on success, \n FAIL on failure.
 */
int savari_get_netmask(char *ifname, char *netmask);

/**
 * savari_get_service_channel - get the service channel of a radio
 * \param ifname - interface name
 * \param channel - channel number
 * \par Description -
                      This function is used to get the service channel of
                      a  DSRC enabled radio. this function is driver specific
                      and only be used for atheros enabled DSRC chips, with
                      Savari DSRC software stack.
 * \return Returns - 0 on success -1 on failure
 */

int
savari_get_service_channel(char *ifname, int *channel);

#ifdef __cplusplus
}
#endif

#endif
