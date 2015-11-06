/**\file
 * Definitions of macros and constants needed to receive, convert
 * and scale the fields in  J1939 Protocol Data Units (frames) 
 * and store the variables corresponding to a Parameter Group
 * in the database
 *
 * Copyright (c) 2005 Regents of the University of California
 */ 

#ifndef J1939PDU_H
#define J1939PDU_H
/* J1939 values for discrete (status) parameters (see J1939-71, p. 8) */

#define JSTATUS_DISABLED	0
#define JSTATUS_ENABLED		1
#define JSTATUS_ERROR		2
#define JSTATUS_NOT_AVAILABLE	3

/* Out of range values for J1939 byte signal ranges; these are translated
 * to negative float values of the same magnitude when encountered as, e.g.
 * acclerator_pedal_position
 */
#define JBYTE_PARAMETER_SPECIFIC	251
#define JBYTE_RESERVED1			252
#define JBYTE_RESERVED2			253
#define JBYTE_ERROR			254
#define JBYTE_NOT_AVAILABLE		255
typedef unsigned char JSTATUS;
typedef unsigned char JBYTE;


#ifdef USE_TIMEB

/* compare to timeb values, return 1 iff a >= b && a.millitm > b.millitm */

#define TIMEB_COMP(a, b)	(( (a).time > (b).time) || \
	 (((a).time == (b).time) && ((a).millitm > (b).millitm) ))

/* return difference in milliseconds (t2 - t1), t2, t1 pointers to timeb */
#define TIMEB_SUBTRACT(t1,t2)  (((t2)->time * 1000 + (t2)->millitm) - \
                        ((t1)->time * 1000 + (t1)->millitm))
#endif

extern int check_timestamp(timestamp_t *raw);

/* Engine/Retarder Torque Modes documented in J1939-71, p. 34 */
#define ERTM_LOW_IDLE	0
#define ERTM_AP_OP	1
#define ERTM_CC		2
#define ERTM_PTO	3
#define ERTM_ROAD_SPD	4
#define ERTM_ASR	5
#define ERTM_TRANSM	6
#define ERTM_ABS	7
#define ERTM_TORQUE_LMT	8
#define ERTM_HI_SPD	9
#define ERTM_BRAKE	10
#define ERTM_RMT_ACC	11
#define ERTM_OTHER	14
#define ERTM_NA		15

/* PDU Specific Global Address */

#define GLOBAL	0xff

/* Electronic Control Module (ECM) addresses on J1939 network */

/* Engine */
#define J1939_ADDR_ENGINE		0
/* Transmission */
#define J1939_ADDR_TRANS		3
/* Brakes */
#define J1939_ADDR_BRAKE		11
/* Engine retarder (jake brake) */
#define J1939_ADDR_ENG_RTDR		15
/* Transmission retarder */
#define J1939_ADDR_TR_RTDR		16
/* Cruise Control, Alison transmission retarder listens */
#define J1939_ADDR_CC			17
/* Steering Controller */ 
#define J1939_ADDR_STEERING		19
/* Adaptive Cruise Control, WABCO EBS listens */
#define J1939_ADDR_ACC			42
/* Experimental Use */ 
#define J1939_ADDR_EXP			252

/* Parameter group numbers for supported messages */

#define PDU     0x00ff  /* (0, 255) sample undefined parameter group number */
#define TSC1    0x0000  /* (0, 0) Torque Speed Control 1, destination 0, engine */
#define TSC1_E_T 0x0000  /* (0, 0) Torque Speed Control 2, destination 0 (engine), source 3 (transmission) */ 
#define TSC1_E_ACC 0x0000  /* (0, 0) Torque Speed Control 2, destination 0 (engine), source 42 (Adaptive Cruise Control, ACC) */
#define TSC1_ER_T 0x000f  /* (0, 15) Torque Speed Control 2, destination 15 (engine retarder), source 3 (transmission)*/
#define TSC1_ER_V 0x000f  /* (0, 15) Torque Speed Control 2, destination 15 (engine retarder), source 17 (cruise control) */
#define TSC1_ER_ACC 0x000f  /* (0, 15) Torque Speed Control 2, destination 15 (engine retarder), source 42 (Adaptive Cruise Control, ACC) */
#define EXAC    0x000b  /* (0, 11) EXAC (WABCO proprietary) */
#define RQST    0xea00  /* (234, 0) request transmission of a particular PGN */
#define ERC1    0xf000  /* (240, 0) electronic retarder controller 1, source 15 (engine retarder) */
#define ERC1_ER 0xf000  /* (240, 0) electronic retarder controller 1, source 15 (engine retarder) */
#define EBC1    0xf001  /* (240, 1) electronic brake controller 1 */
#define ETC1    0xf002  /* (240, 2) electronic transmission controller 1 */
#define EEC2    0xf003  /* (240, 3) electronic engine controller 2 */
#define EEC1    0xf004  /* (240, 4) electronic engine controller 1 */
#define ETC2    0xf005  /* (240, 5) electronic transmission controller 2, source 3 (transmission) */
#define ETC2_E  0xf005  /* (240, 5) electronic transmission controller 2, source 0 (engine) */
#define VDC2    0xf009  /* (240, 9) Contains information which relates to the vehicle's movement */
//					** Note: Updaterate from EBS is 20ms.
//					** Updaterate according to SAE J1939-71 shall 
//					** be 10ms.
#define EBC5	0xfdc4  /* (253, 196) ?????????????????????????*/
#define MVS_X_E 0xfde5  /* (253, 229) Maximum Vehicle Speed Limit Status - Reports the possible maximum vehicle speed limits, one through seven, and the applied maximum vehicle speed limit*/
#define VOLVO_TARGET 0xfe33  /* (254, 51) Volvo target data */ 
#define VOLVO_EGO 0xfe34  /* (254, 52) Volvo self data */ 
#define GFI2    0xfe81  /* (254, 129) gaseous fuel information 2 */ 
#define EI      0xfe92  /* (254, 146) engine information */
#define FD      0xfebd  /* (254, 189) fan drive */
#define EEC4    0xfebe  /* (254, 190) electronic engine controller 4 */
#define EBC2    0xfebf  /* (254, 191) electronic brake controller 2 */
#define EBC2WSI 0xfebf  /* (254, 191) electronic brake controller 2, source 11 (brake) */
#define HRVD    0xfec1  /* (254, 193) high resolution vehicle distance */
#define ERC2    0xfec2  /* (254, 194) electronic retarder controller 2 */
#define EBC1    0xf001  /* (240, 1) electronic brake controller 1 */
#define TURBO   0xfedd  /* (254, 221) turbocharger */
#define EEC3    0xfedf  /* (254, 223) electronic engine controller 3 */
#define VD      0xfee0  /* (254, 224) vehicle distance */
#define RCFG    0xfee1  /* (254, 225) retarder configuration */
#define TCFG    0xfee2  /* (254, 226) transmission configuration */
#define ECFG    0xfee3  /* (254, 227) engine configuration */
#define ETEMP   0xfeee  /* (254, 238) engine temperature */
#define PTO     0xfef0  /* (254, 240) power takeoff */
#define CCVS    0xfef1  /* (254, 241) cruise control vehicle speed */
#define LFE     0xfef2  /* (254, 242) fuel economy */
#define VP_X    0xfef3  /* (254, 243) Vehicle Position */
#define AMBC    0xfef5  /* (254, 245) ambient conditions */
#define IEC     0xfef6  /* (254, 246) inlet/exhaust conditions */
#define VEP     0xfef7  /* (254, 247) vehicle electric power */
#define TF      0xfef8  /* (254, 248) transmission fluids */
#define RF      0xfefb  /* (254, 251) retarder fluids */
#define VOLVO_XBR_WARN 0xff10  /* (255, 16) XBR Bendix proprietary warning message */
#define VP39	0xff27  /* (255, 39)  Status of the buttons in the steering wheel. With key in 'Radio'-position: sent on state changes. With key in 'Ignition'-position: sent periodically and on state changes */
#define EBC_ACC 0xff80  /* (255, 128) EBC to ACC WABCO proprietary */
#define TPDT    0xebff  /* (235, 255) transfer protocol data transfer, global */
#define TPCM    0xecff  /* (236, 255) transfer protocol connection message, global */
#define VOLVO_XBR 0xef0b  /* (239, 11) XBR Bendix proprietary brake control */

/* Values for the TSC1 override control mode priority field (J1939 standard) */
#define TSC_HIGHEST	0
#define TSC_HIGH	1
#define TSC_MEDIUM	2
#define TSC_LOW		3

/* Values for the TSC1 (Torque override control mode field (J1939 standard) */
#define TSC_OVERRIDE_DISABLED	0
#define TSC_SPEED_CONTROL	1
#define TSC_TORQUE_CONTROL	2
#define TSC_SPEED_TORQUE_LIMIT	3

/* Values for the XBR external_deceleration_control_mode field */

#define XBR_NOT_ACTIVE	0
#define XBR_ACTIVE 	2

/* Default reference values used for percent torque; taken from trucks */
#define MAX_ENGINE_TORQUE 	1966
#define MAX_RETARDER_TORQUE	1476

extern void tsc1_to_pdu(struct j1939_pdu *pdu, void *pdbv);
extern void exac_to_pdu(struct j1939_pdu *pdu, void *pdbv);
extern void volvo_xbr_to_pdu(struct j1939_pdu *pdu, void *pdbv);
extern void volvo_xbr_warn_to_pdu(struct j1939_pdu *pdu, void *pdbv);
extern void pdu_to_pdu (struct j1939_pdu *pdu_in, void *pdbv_out);

extern void pdu_to_erc1 (struct j1939_pdu *pdu, void *pdbv);
extern void pdu_to_ebc1 (struct j1939_pdu *pdu, void *pdbv);
extern void pdu_to_etc1 (struct j1939_pdu *pdu, void *pdbv);
extern void pdu_to_eec1 (struct j1939_pdu *pdu, void *pdbv);
extern void pdu_to_eec2 (struct j1939_pdu *pdu, void *pdbv);
extern void pdu_to_etc2 (struct j1939_pdu *pdu, void *pdbv);
extern void pdu_to_etc2_e (struct j1939_pdu *pdu, void *pdbv);
extern void pdu_to_turbo (struct j1939_pdu *pdu, void *pdbv);
extern void pdu_to_eec3 (struct j1939_pdu *pdu, void *pdbv);
extern void pdu_to_vd (struct j1939_pdu *pdu, void *pdbv);
extern void pdu_to_etemp (struct j1939_pdu *pdu, void *pdbv);
extern void pdu_to_pto (struct j1939_pdu *pdu, void *pdbv);
extern void pdu_to_ccvs (struct j1939_pdu *pdu, void *pdbv);
extern void pdu_to_lfe (struct j1939_pdu *pdu, void *pdbv);
extern void pdu_to_ambc (struct j1939_pdu *pdu, void *pdbv);
extern void pdu_to_iec (struct j1939_pdu *pdu, void *pdbv);
extern void pdu_to_vep (struct j1939_pdu *pdu, void *pdbv);
extern void pdu_to_tf (struct j1939_pdu *pdu, void *pdbv);
extern void pdu_to_rf (struct j1939_pdu *pdu, void *pdbv);
extern void pdu_to_hrvd (struct j1939_pdu *pdu, void *pdbv);
extern void pdu_to_ebc2 (struct j1939_pdu *pdu, void *pdbv);
extern void pdu_to_rcfg (struct j1939_pdu *pdu, void *pdbv);
extern void pdu_to_ecfg (struct j1939_pdu *pdu, void *pdbv);
extern void pdu_to_tsc1 (struct j1939_pdu *pdu, void *pdbv);
extern void pdu_to_fd (struct j1939_pdu *pdu, void *pdbv);
extern void pdu_to_exac (struct j1939_pdu *pdu, void *pdbv);
extern void pdu_to_ebc_acc (struct j1939_pdu *pdu, void *pdbv);
extern void pdu_to_gfi2 (struct j1939_pdu *pdu, void *pdbv);
extern void pdu_to_ei (struct j1939_pdu *pdu, void *pdbv);
extern void pdu_to_vp_x (struct j1939_pdu *pdu, void *pdbv);
extern void pdu_to_vdc2 (struct j1939_pdu *pdu, void *pdbv);
extern void pdu_to_mvs_x_e (struct j1939_pdu *pdu, void *pdbv);
extern void pdu_to_ebc5 (struct j1939_pdu *pdu, void *pdbv);
extern void pdu_to_volvo_target (struct j1939_pdu *pdu, void *pdbv);
extern void pdu_to_volvo_ego (struct j1939_pdu *pdu, void *pdbv);
extern void pdu_to_volvo_xbr_warn(struct j1939_pdu *pdu, void *pdbv);
extern void pdu_to_volvo_xbr(struct j1939_pdu *pdu, void *pdbv);


extern void print_pdu (void *pdbv, FILE *fp, int numeric);

extern void print_erc1 (void *pdbv, FILE *fp, int numeric);
extern void print_ebc1 (void *pdbv, FILE *fp, int numeric);
extern void print_etc1 (void *pdbv, FILE *fp, int numeric);
extern void print_eec1 (void *pdbv, FILE *fp, int numeric);
extern void print_eec2 (void *pdbv, FILE *fp, int numeric);
extern void print_etc2 (void *pdbv, FILE *fp, int numeric);
extern void print_etc2_e (void *pdbv, FILE *fp, int numeric);
extern void print_turbo (void *pdbv, FILE *fp, int numeric);
extern void print_eec3 (void *pdbv, FILE *fp, int numeric);
extern void print_vd (void *pdbv, FILE *fp, int numeric);
extern void print_etemp (void *pdbv, FILE *fp, int numeric);
extern void print_pto (void *pdbv, FILE *fp, int numeric);
extern void print_ccvs (void *pdbv, FILE *fp, int numeric);
extern void print_lfe (void *pdbv, FILE *fp, int numeric);
extern void print_ambc (void *pdbv, FILE *fp, int numeric);
extern void print_iec (void *pdbv, FILE *fp, int numeric);
extern void print_vep (void *pdbv, FILE *fp, int numeric);
extern void print_tf (void *pdbv, FILE *fp, int numeric);
extern void print_rf (void *pdbv, FILE *fp, int numeric);
extern void print_hrvd (void *pdbv, FILE *fp, int numeric);
extern void print_ebc2 (void *pdbv, FILE *fp, int numeric);
extern void print_rcfg (void *pdbv, FILE *fp, int numeric);
extern void print_ecfg (void *pdbv, FILE *fp, int numeric);
extern void print_tsc1 (void *pdbv, FILE *fp, int numeric);
extern void print_fd (void *pdbv, FILE *fp, int numeric);
extern void print_exac (void *pdbv, FILE *fp, int numeric);
extern void print_ebc_acc (void *pdbv, FILE *fp, int numeric);
extern void print_gfi2 (void *pdbv, FILE *fp, int numeric);
extern void print_ei (void *pdbv, FILE *fp, int numeric);
extern void print_vp_x (void *pdbv, FILE *fp, int numeric);
extern void print_vdc2 (void *pdbv, FILE *fp, int numeric);
extern void print_mvs_x_e (void *pdbv, FILE *fp, int numeric);
extern void print_ebc5 (void *pdbv, FILE *fp, int numeric);
extern void print_volvo_target(void *pdbv, FILE *fp, int numeric);
extern void print_volvo_ego(void *pdbv, FILE *fp, int numeric);
extern void print_volvo_xbr_warn(void *pdbv, FILE *fp, int numeric);
extern void print_volvo_xbr(void *pdbv, FILE *fp, int numeric);

#endif
