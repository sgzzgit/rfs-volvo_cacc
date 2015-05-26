#include <sys_os.h>
#include <devctl.h>
#include <sys/neutrino.h>
#include "local.h"
#include "das_clt.h"
#include "can_defs.h"
#include "can_client.h"
#include "dachs_sja/include/candef.h"   // Steinhoff include files
#include "dachs_sja/include/canstr.h"
#include "dachs_sja/include/canglob.h"

typedef struct {
        canhdl_t hdl;   // Steinhoff handle
        int channel_id; // from QNX6 Channel Create
        int connect_id; // from QNX6 Connect Attach
        char *can_str;  //CAN channel number as string
        struct sigevent pulse_event;    // signal event structure
} can_dev_handle_t; 

#undef DO_TRACE
#define MAX_MSG_BUF 1000

/** Read information from the CAN card; return -1 if error encountered,
 *  otherwise number of bytes in the data segment.
 *  Blocking read, includes MsgReceive
 */
int can_read(int fd, unsigned long *id, char *extended, void *data, 
                                unsigned char size) {
        can_dev_handle_t *phdl = (can_dev_handle_t *) fd; 
        int chid = phdl->channel_id;
        struct _pulse pulse;
        struct can_object rmsg;
        int byte_count;
        int rcvid;
        short resp;

        memset(&pulse, 0, sizeof(pulse));

        // a pulse will be triggerd after a frame or more has been received
        rcvid = MsgReceivePulse(chid, &pulse, sizeof(struct _pulse), NULL);         
        if (rcvid == -1) {
                perror("can_read");
                return -1;
        } else {
                resp = CanRead (phdl->hdl, &rmsg, NULL);  // read the next frame
                if(resp == 0) {                                         
#ifdef DO_TRACE
                        printf("can_read: %04x %d %04x %hhd", 
                                CanGetStatus(phdl->hdl, &st), 
                                rmsg.frame_inf.inf.DLC,
                                rmsg.id, rmsg.frame_inf.inf.FF);
                        fflush(stdout);
#endif
                        byte_count = rmsg.frame_inf.inf.DLC;
                        if (id != NULL)
                                *id = (unsigned long)rmsg.id;
                        if (extended != NULL)
                                *extended = rmsg.frame_inf.inf.FF; 
                        memcpy(data, rmsg.data,
                                size > byte_count ? byte_count : size);
                        return(byte_count);
                }else {                                    
#ifdef DO_TRACE
                        printf("can_read: %hd %04x\n", resp,
                                CanGetStatus(phdl->hdl, &st));
#endif
                        return (-1);
                }
        }
}

int can_write(int fd, unsigned long id, char extended, void *data, 
                                unsigned char size) {
        can_dev_handle_t *phdl = (can_dev_handle_t *) fd; 
        struct can_object msg;          // Steinhoff CAN message type

#ifdef DO_TRACE
        printf("can_write: extended %hhd size %hhu\n", extended, size);
#endif
        msg.frame_inf.inf.DLC = size > 8 ? 8 : size;
        msg.id = id;
        if (extended) 
                msg.frame_inf.inf.FF = ExtFF;
        else
                msg.frame_inf.inf.FF = StdFF;
        msg.frame_inf.inf.RTR = 0;
        memcpy(msg.data, data, msg.frame_inf.inf.DLC);

#ifdef DO_TRACE
        printf("can_write: msg.frame_inf.octet 0x%02hhx\n", msg.frame_inf.octet);
#endif
        return (CanWrite(phdl->hdl, &msg));
}

/** We never actually used this function on QNX4, so it was
 *  untested at PATH with QNX4.
 *  Steinhoff API does not seem to inlude an equivalent
 *      function, don't know if it is supported on the SJA1000.
 *
 *      Internal to the driver, set as part of open for read
 */
int can_set_filter(int fd, unsigned long id, unsigned long mask)
{
        can_filter_t filter_data;
        filter_data.id = id;

        return(1);
}

/* Enable pulses from the CAN driver to the client process.
 * Pulses are waited for with MsgReceive or IP_Receive.
 * channel_id can be one obtained from DB_CHANNEL(pclt) on
 * a the pointer returned by the database clt_login.
 *
 *      Internal to the driver, done as part of open for read
 */
int can_arm(int fd, int channel_id)
{
        can_dev_handle_t *phdl = (can_dev_handle_t *) fd; 

        if ((phdl->connect_id = 
                ConnectAttach(0, 0, channel_id, _NTO_SIDE_CHANNEL, 0)) == -1) {
                        printf("can_open: ConnectAttach failed\n");
                        ChannelDestroy(channel_id);
                        return (-1);
        }
        // initialize notification event
        SIGEV_PULSE_INIT(&phdl->pulse_event, phdl->connect_id, getprio(0), 
                        _PULSE_CODE_MINAVAIL, 0);
        return (0);
}

/* Call this function to empty the queue of messages in
 * the CAN driver. 
 * With Steinoff driver just deregisters and reregisters
 * the rd pulse. NOTE: this does not empty queue, we need
 * to look at this (Sue, August 2009)
 *
 *      Internal to the driver, done as part of open for read
 */
int can_empty_q(int fd)
{
        can_dev_handle_t *phdl = (can_dev_handle_t *) fd; 

        DeRegRdPulse(phdl->hdl);                                        
        RegRdPulse(phdl->hdl, &phdl->pulse_event);      

        return (0);
}

/* Wrapper for the open call; some drivers are not file structured
 * and different calls will be needed to "open"
 *
 * Returns "fd" that will be used in all subsequent calls
 * Filename string and flags contain any information needed to do the open
 * For Steinhoff driver, "filename" is just string with channel number.
 */
int can_open(char *filename, int flags)
{
        int chid;               // from QNX6 ChannelCreate
        int can_channel;        // for multichannel boards
        struct status st;

        // CA PATH CAN library handle
        can_dev_handle_t *phdl = (can_dev_handle_t *) 
                                malloc(sizeof(can_dev_handle_t));
        if (flags == O_RDONLY) {
                // create notification channel
                if ((chid = ChannelCreate(0)) == -1) { 
                        printf("can_open: ChannelCreate failed\n");
                        return (-1);
                }
                if (can_arm((int) phdl, chid) == -1)
                        return (-1); 
        }
        can_channel = atoi(filename);
        switch (can_channel) {
                case 1: 
                case 2: 
                case 3: 
                case 4: 
                        printf("Valid channel number %d\n", can_channel);
                        break;
                default:
                        printf("Bad channel number %d, filename %s\n", 
                                can_channel, filename);
                        break;
        }
        if (ConnectDriver (can_channel, "CANDRV", &phdl->hdl) < 0) {
                printf ("can_open: can't connect the CAN driver.\n");
                return (0);
        }  
                                                                          
        printf ("\ncan_open: status: %04x\n", CanGetStatus(phdl->hdl, &st));  
 
        if (flags == O_RDONLY) {
                can_empty_q((int) phdl);
        }
        phdl->channel_id = chid;
        phdl->can_str = filename;
        return ((int) phdl);
}

/* Wrapper for the close call; some drivers are not file structured
 * and may require disconnect functions to be called.
 */
int can_close(int *pfd)
{
        can_dev_handle_t **pphdl = (can_dev_handle_t **) pfd;
        can_dev_handle_t *phdl = *pphdl; 

        if (phdl == NULL)
                return -1;

        DeRegRdPulse(phdl->hdl);

        free(phdl);

        *pphdl = NULL;  // to avoid free errors if can_close is called twice

        return 0;
}

/*
 * Call to print configuration information
 * So far only prints timing information.
 * Returns 1 if successful, 0 if fail.
 */

int can_print_config(FILE *fpout, int fd)
{
        can_dev_handle_t *phdl = (can_dev_handle_t *) fd; 
        struct config sja_conf; // structure used by DACHS CanGetConfig
        short retval = CanGetConfig(phdl->hdl, &sja_conf);
		int i = 0;
		int j = 0;
        if (retval != ERR_OK) {
                fprintf(fpout, "Error return from DACHS CanGetConfig %hd\n",
                         retval);
                return 0;
		} else if (retval == ERR_DEVCTL) {
				fprintf(fpout, "Connection broken from DACHS CanGetConfig %hd\n", 
						 retval);
				return 0;
		} else if (retval == ERR_NOT_CON) {
				fprintf(fpout, "Driver not opened from DACHS CanGetConfig %hd\n", 
						 retval);
				return 0;
		} else {
                fprintf(fpout, "BTR0: 0x%02hhx\n", sja_conf.ubtr0.oct);
                fprintf(fpout, "BTR1: 0x%02hhx\n", sja_conf.ubtr1.oct);

				fprintf(fpout, "BTR0.brp: 0x%02hhx\n", sja_conf.ubtr0.BTR0.brp);
				fprintf(fpout, "BTR0.sjw: 0x%02hhx\n", sja_conf.ubtr0.BTR0.sjw);
				fprintf(fpout, "BTR1.tseg1: 0x%02hhx\n", sja_conf.ubtr1.BTR1.tseg1);
				fprintf(fpout, "BTR1.tseg2: 0x%02hhx\n", sja_conf.ubtr1.BTR1.tseg2);
				fprintf(fpout, "BTR1.sam: 0x%02hhx\n", sja_conf.ubtr1.BTR1.sam);

				fprintf(fpout, "Listen Only Mode (if bit set to 1); LOM: 0x%02hhx\n", sja_conf.LOM);
				fprintf(fpout, "Self Test Mode (if bit set to 1); STM: 0x%02hhx\n", sja_conf.STM);
				fprintf(fpout, "Acceptance Filter Mode; AFM: 0x%02hhx\n", sja_conf.AFM);
				fprintf(fpout, "Error Warning Limit(default 96); EWL: 0x%02hhx\n", sja_conf.EWL);
				fprintf(fpout, "Receive Error Counter; RXERR: 0x%02hhx\n", sja_conf.RXERR);
				fprintf(fpout, "Transmit Error Counter; TXERR: 0x%02hhx\n", sja_conf.TXERR);

				while (i < 4)
				{
					fprintf(fpout, "Current Acceptance Code (returns 4 values); ACR: 0x%02hhx\n",(int)sja_conf.ACR[i]);
					i++;
				}
			
				while (j < 4)
				{
					fprintf(fpout, "Current Mask Value (returns 4 values); AMR: 0x%02hhx\n",(int)sja_conf.AMR[j]);
					j++;
				}

                return 1;
        }
}

/*
 * Call to print status information
 * Returns 1 if successful, 0 if fail.
 */
int can_print_status(FILE *fpout, int fd)
{	
		can_dev_handle_t *phdl = (can_dev_handle_t *) fd;
		struct status sja_status;	// structure used by DACHS CanGetStatus
		short retval = CanGetStatus(phdl->hdl, &sja_status);
		if (retval == ERR_NOT_CON) {
			fprintf(fpout, "Driver not opened from DACHS CanGetStatus %hd\n",
					 retval);
			return 0;
		} else if (retval == ERR_DEVCTL) {
			fprintf(fpout, "Connection broken from DACHS CanGetStatus %hd\n",
					 retval);
			return 0;
		} else {

			fprintf(fpout, "Status Register; STR: 0x%02hhx\n", sja_status.STR);
			fprintf(fpout, "ErrState: 0x%02hhx\n", sja_status.ErrState);
			fprintf(fpout, "OverRunState: 0x%02hhx\n", sja_status.OverRunState);
			fprintf(fpout, "WakeUpState: 0x%02hhx\n", sja_status.WakeUpState);
			fprintf(fpout, "ErrPassiveState: 0x%02hhx\n", sja_status.ErrPassiveState);
			fprintf(fpout, "ArbitLostState: 0x%02hhx\n", sja_status.ArbitLostState);
			fprintf(fpout, "BusErrState: 0x%02hhx\n", sja_status.BusErrState);
			fprintf(fpout, "Number of LostFrames: 0x%02hhu\n", sja_status.LostFrames);

			return 1;
			
		}
		
	
}



/* Call to change configuration.
 * So far only changes bit speed with BTR0 and BTR1
 * Returns 1 if successful, 0 if fail.
 */
int can_set_config(int fd, int bitspeed) 
{
        can_dev_handle_t *phdl = (can_dev_handle_t *) fd; 
        struct config sja_conf; // structure used by DACHS CanGetConfig
        short retval; 

        if (phdl == NULL) {
                fprintf(stderr, "can_set_config: NULL handle for CAN driver\n");
                return 0;
        }

        if ((retval = CanGetConfig(phdl->hdl, &sja_conf)) != ERR_OK) {
                fprintf(stderr, "Steinhoff CanGetConfig failed\n");
                return 0;
        }

        switch (bitspeed) {     // in Kbs
        case 250: sja_conf.ubtr0.oct = 0x1;
                sja_conf.ubtr1.oct = 0x1c;
                break;
        case 500: sja_conf.ubtr0.oct = 0x0;
                sja_conf.ubtr1.oct = 0x1c;
                break;
        default:
                fprintf(stderr, "Unknown bit speed\n");
                return 0;
        }

        printf("bitspeed %d: setting btr0 0x%hhx btr1 0x%hhx\n",
                bitspeed,sja_conf.ubtr0.oct, sja_conf.ubtr1.oct);

        if ((retval = CanSetConfig(phdl->hdl, &sja_conf)) != ERR_OK) {
                fprintf(stderr, "Steinhoff CanSetConfig failed\n");
                return 0;
        }
        return 1;
}

/**
 *	Stubs for compile compatibility with PATH CAN.
 */
can_err_count_t can_clear_errs()
{
	can_err_count_t errs;
	memset(&errs, 0, sizeof(can_err_count_t));
	return (errs);
}

can_err_count_t can_get_errs()
{
	can_err_count_t errs;
	memset(&errs, 0, sizeof(can_err_count_t));
	return (errs);
}
