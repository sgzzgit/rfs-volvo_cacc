/**\file
 *
 *	laipdu.h Declarations for laipdu.c
 *		
 */
#ifndef LAIPDU_H
#define LAIPDU_H

/* Reads of data from Jbus are formatted into a single database variable,
 * that is then placed in the database in a uniform way for all reads
 * by the rdj1939 program that calls these functions.
 *
 * Sends of data to the Jbus may come take fields from more than one
 * database variable in some situations, so the formatting function
 * does the database reads.
 */

extern void pdu_to_lai_ctrlstat (struct j1939_pdu *pdu, void *pdbv);
extern void lai_ctrlstat_to_pdu(db_clt_typ *pclt, struct j1939_pdu *pdu,
	void *pdbv_info, void *pitem_info, void *psend_info);
extern void print_lai_ctrlstat(void *pdbv , FILE  *fp, int numeric);

extern void pdu_to_lai_stin (struct j1939_pdu *pdu, void *pdbv);
extern void lai_stin_to_pdu(db_clt_typ *pclt, struct j1939_pdu *pdu,
	void *pdbv_info, void *pitem_info, void *psend_info);
extern void print_lai_stin(void *pdbv , FILE  *fp, int numeric);

extern void pdu_to_lai_stout (struct j1939_pdu *pdu, void *pdbv);
extern void lai_stout_to_pdu(db_clt_typ *pclt, struct j1939_pdu *pdu,
	void *pdbv_info, void *pitem_info, void *psend_info);
extern void print_lai_stout(void *pdbv , FILE  *fp, int numeric);

extern void pdu_to_lai_latout (struct j1939_pdu *pdu, void *pdbv);
extern void lai_latout_to_pdu(db_clt_typ *pclt, struct j1939_pdu *pdu,
	void *pdbv_info, void *pitem_info, void *psend_info);
extern void print_lai_latout(void *pdbv , FILE  *fp, int numeric);

extern void pdu_to_lai_sigstat (struct j1939_pdu *pdu, void *pdbv);
extern void lai_sigstat_to_pdu(db_clt_typ *pclt, struct j1939_pdu *pdu,
	void *pdbv_info, void *pitem_info, void *psend_info);
extern void print_lai_sigstat(void *pdbv , FILE  *fp, int numeric);

extern void pdu_to_lai_sigout (struct j1939_pdu *pdu, void *pdbv);
extern void lai_sigout_to_pdu(db_clt_typ *pclt, struct j1939_pdu *pdu,
	void *pdbv_info, void *pitem_info, void *psend_info);
extern void print_lai_sigout(void *pdbv , FILE  *fp, int numeric);

extern void pdu_to_lai_gyro (struct j1939_pdu *pdu, void *pdbv);
extern void lai_gyro_to_pdu(db_clt_typ *pclt, struct j1939_pdu *pdu,
	void *pdbv_info, void *pitem_info, void *psend_info);
extern void print_lai_gyro(void *pdbv , FILE  *fp, int numeric);

extern void pdu_to_lai_dvimon (struct j1939_pdu *pdu, void *pdbv);
extern void lai_dvimon_to_pdu(db_clt_typ *pclt, struct j1939_pdu *pdu,
	void *pdbv_info, void *pitem_info, void *psend_info);
extern void print_lai_dvimon(void *pdbv , FILE  *fp, int numeric);

extern void pdu_to_lai_latsens (struct j1939_pdu *pdu, void *pdbv);
extern void lai_latsens_to_pdu(db_clt_typ *pclt, struct j1939_pdu *pdu,
	void *pdbv_info, void *pitem_info, void *psend_info);
extern void print_lai_latsens(void *pdbv , FILE  *fp, int numeric);

extern void pdu_to_lai_longin1 (struct j1939_pdu *pdu, void *pdbv);
extern void lai_longin1_to_pdu(db_clt_typ *pclt, struct j1939_pdu *pdu,
	void *pdbv_info, void *pitem_info, void *psend_info);
extern void print_lai_longin1(void *pdbv , FILE  *fp, int numeric);

extern void pdu_to_lai_longin2 (struct j1939_pdu *pdu, void *pdbv);
extern void lai_longin2_to_pdu(db_clt_typ *pclt, struct j1939_pdu *pdu,
	void *pdbv_info, void *pitem_info, void *psend_info);
extern void print_lai_longin2(void *pdbv , FILE  *fp, int numeric);

extern void pdu_to_lai_dvistat (struct j1939_pdu *pdu, void *pdbv);
extern void lai_dvistat_to_pdu(db_clt_typ *pclt, struct j1939_pdu *pdu,
	void *pdbv_info, void *pitem_info, void *psend_info);
extern void print_lai_dvistat(void *pdbv , FILE  *fp, int numeric);

extern void pdu_to_lai_dvisnd (struct j1939_pdu *pdu, void *pdbv);
extern void lai_dvisnd_to_pdu(db_clt_typ *pclt, struct j1939_pdu *pdu,
	void *pdbv_info, void *pitem_info, void *psend_info);
extern void print_lai_dvisnd(void *pdbv , FILE  *fp, int numeric);

extern void pdu_to_lai_dvipos (struct j1939_pdu *pdu, void *pdbv);
extern void lai_dvipos_to_pdu(db_clt_typ *pclt, struct j1939_pdu *pdu,
	void *pdbv_info, void *pitem_info, void *psend_info);
extern void print_lai_dvipos(void *pdbv , FILE  *fp, int numeric);

extern void pdu_to_lai_ctrlout (struct j1939_pdu *pdu, void *pdbv);
extern void lai_ctrlout_to_pdu(db_clt_typ *pclt, struct j1939_pdu *pdu,
	void *pdbv_info, void *pitem_info, void *psend_info);
extern void print_lai_ctrlout(void *pdbv , FILE  *fp, int numeric);

extern void pdu_to_lai_latheart (struct j1939_pdu *pdu, void *pdbv);
extern void lai_latheart_to_pdu(db_clt_typ *pclt, struct j1939_pdu *pdu,
	void *pdbv_info, void *pitem_info, void *psend_info);
extern void print_lai_latheart(void *pdbv , FILE  *fp, int numeric);

extern void pdu_to_lai_longout (struct j1939_pdu *pdu, void *pdbv);
extern void lai_longout_to_pdu(db_clt_typ *pclt, struct j1939_pdu *pdu,
	void *pdbv_info, void *pitem_info, void *psend_info);
extern void print_lai_longout(void *pdbv , FILE  *fp, int numeric);

#endif
