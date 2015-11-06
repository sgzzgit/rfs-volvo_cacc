/**\file
 * Translates J1939 PDU structures into database variables.  
 * Page numbers refer to standalone SAE J1939 documents, 
 * are wrong for the "purple book" that has all bound together.
 *
 * Copyright (c) 2005 Regents of the University of California
 *
 */
#include "std_jbus_extended.h"

int j1939_debug = 0;	/* main routines can set to turn on debug */

/**
 * Converts from pdu format used to read from network to database pdu format.
 * For general debugging: print using this format, isearch for individual
 * fields or sort and count PDU lines
 */
void
pdu_to_pdu(struct j1939_pdu *pdu_in, void *pdbv)
{
	int i;
	j1939_pdu_typ *pdu_out = (j1939_pdu_typ *)pdbv;
	pdu_out->priority = pdu_in->priority;
	pdu_out->pdu_format = pdu_in->pdu_format;
	pdu_out->pdu_specific = pdu_in->pdu_specific;
	pdu_out->src_address = pdu_in->src_address;
	pdu_out->numbytes = pdu_in->numbytes;
	for (i = 0; i < 8; i++)
		 pdu_out->data_field[i] = pdu_in->data_field[i];
}	

/**
 * Prints an uninterpreted J1939 Protocol Data Unit.
 */
void
print_pdu(void *pdbv, FILE *fp, int numeric) 
{
	int i;

	j1939_pdu_typ *pdu = (j1939_pdu_typ *) pdbv;
	fprintf(fp, "PDU ");
	print_timestamp(fp, &pdu->timestamp);
	if (numeric) {
		fprintf(fp, "%d %d %d %d %d ", pdu->priority, pdu->pdu_format,
			pdu->pdu_specific, pdu->src_address, pdu->numbytes);
		for (i = 0; i < 8; i++)
			fprintf(fp, "%d ", pdu->data_field[i]);
		fprintf(fp,"\n");
	} else {
		fprintf(fp, "Priority: 0x%2x\n", pdu->priority);
		fprintf(fp, "PF:       0x%2x\n", pdu->pdu_format);
		fprintf(fp, "PS:       0x%2x\n", pdu->pdu_specific);
		fprintf(fp, "Source:   0x%2x\n", pdu->src_address);
		fprintf(fp, "No. of bytes %d\n", pdu->numbytes);
		for (i = 0; i < 8; i++)
			fprintf(fp, "\t0x%2x", pdu->data_field[i]);
		fprintf(fp, "\n");
	}
}


/**
 * Engine/Retarder Torque Modes documented in J1939-71, p. 34
 */
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

/** ERC1 (Electronic Retarder Controller #1) documented in J1939 - 71, p 150 */
void
pdu_to_erc1 (struct j1939_pdu *pdu, void *pdbv)
{
	j1939_erc1_typ *erc1 = (j1939_erc1_typ *)pdbv;
	unsigned char byte;

	/* two-bit fields in data byte 0 indicate status of switch,
         * enabled or disabled.
	 */

	byte = (unsigned int) pdu->data_field[0];
	erc1->ERC1ERRetarderEnableShiftAssistSw = BITS87(byte);
	erc1->ERC1ERRtdrEnablBrakeAssistSwitch = BITS65(byte);
	erc1->ERC1ERRetarderTorqueMode = LONIBBLE(byte);

	erc1->ERC1ERActualEngineRetPercentTrq = 
		percent_m125_to_p125(pdu->data_field[1]);

	erc1->ERC1ERIntendedRtdrPercentTorque = 
		percent_m125_to_p125(pdu->data_field[2]);

	erc1->ERC1ERRetarderRqingBrakeLight = BITS43(pdu->data_field[3]);

	erc1->ERC1ERSrcAddrOfCtrlDvcFrRtdrCtrl = pdu->data_field[4];

	erc1->ERC1ERDrvrsDmandRtdrPerctTorque = pdu->data_field[5];

	erc1->ERC1ERRetarderSelectionNonEng = pdu->data_field[6];

	erc1->ERC1ERActlMxAvlbRtdrtPerctTorque = pdu->data_field[7];
	printf("ERC1 Raw %#02hhx %hhd %#02hhx %hhd %#02hhx %hhd %#02hhx %hhd %#02hhx %hhd %#02hhx %hhd %#02hhx %hhd %#02hhx\n",
		pdu->data_field[0],
		pdu->data_field[1],
		pdu->data_field[1],
		pdu->data_field[2],
		pdu->data_field[2],
		pdu->data_field[3],
		pdu->data_field[3],
		pdu->data_field[4],
		pdu->data_field[4],
		pdu->data_field[5],
		pdu->data_field[5],
		pdu->data_field[6],
		pdu->data_field[6],
		pdu->data_field[7],
		pdu->data_field[7]
		);
}

void 
print_erc1(void *pdbv , FILE  *fp, int numeric)

{
	j1939_erc1_typ *erc1 = (j1939_erc1_typ *)pdbv;
	fprintf(fp, "ERC1 ");
	print_timestamp(fp, &erc1->timestamp);
	if (numeric) {
		fprintf(fp, "%d ", erc1->ERC1ERRetarderEnableShiftAssistSw);
		fprintf(fp, "%d ", erc1->ERC1ERRtdrEnablBrakeAssistSwitch);
		fprintf(fp, "%d ", erc1->ERC1ERRetarderTorqueMode);
		fprintf(fp, "%.2f ", erc1->ERC1ERActualEngineRetPercentTrq);
		fprintf(fp, "%.2f ", erc1->ERC1ERIntendedRtdrPercentTorque);
		fprintf(fp, "%d ", erc1->ERC1ERRetarderRqingBrakeLight);
	 	fprintf(fp, "%d ", erc1->ERC1ERSrcAddrOfCtrlDvcFrRtdrCtrl);
		fprintf(fp, "%hhd ", erc1->ERC1ERDrvrsDmandRtdrPerctTorque);
		fprintf(fp, "%d ", erc1->ERC1ERRetarderSelectionNonEng);
		fprintf(fp, "%hhd ", erc1->ERC1ERActlMxAvlbRtdrtPerctTorque);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp, "Enable shift assist status %d\n",
			 erc1->ERC1ERRetarderEnableShiftAssistSw);
		fprintf(fp, "Enable brake assist status %d\n",
			 erc1->ERC1ERRtdrEnablBrakeAssistSwitch);
		fprintf(fp, "Engine retarder torque mode %d\n",
			 erc1->ERC1ERRetarderTorqueMode);
		fprintf(fp, "Actual retarder percent torque %.2f\n",
			 erc1->ERC1ERActualEngineRetPercentTrq);
		fprintf(fp, "Intended retarder percent torque %.2f\n",
			 erc1->ERC1ERIntendedRtdrPercentTorque);
		fprintf(fp, "Retarder requesting brake light %d\n",
			 erc1->ERC1ERRetarderRqingBrakeLight);
		fprintf(fp, "Source address %d (0x%0x)\n", erc1->ERC1ERSrcAddrOfCtrlDvcFrRtdrCtrl,
			erc1->ERC1ERSrcAddrOfCtrlDvcFrRtdrCtrl);
		fprintf(fp, "Drivers demand retarder percent torque %d ", erc1->ERC1ERDrvrsDmandRtdrPerctTorque);
		fprintf(fp, "Retarder selection Non-eng %d ", erc1->ERC1ERRetarderSelectionNonEng);
		fprintf(fp, "Actual maximum available retarder percent torque %d ", erc1->ERC1ERActlMxAvlbRtdrtPerctTorque);
	}
}

/** EBC1 (Electronic Brake Controller #1) documented in J1939 - 71, p 151 */
void
pdu_to_ebc1 (struct j1939_pdu *pdu, void *pdbv)
{
	j1939_ebc1_typ *ebc1 = (j1939_ebc1_typ *)pdbv;
	unsigned char byte;

	byte = (unsigned int) pdu->data_field[0];
	ebc1->EBSBrakeSwitch = BITS87(byte);
	ebc1->EBC1_AntiLockBrakingActive = BITS65(byte);
	ebc1->EBC1_ASRBrakeControlActive = BITS43(byte);
	ebc1->EBC1_ASREngineControlActive = BITS21(byte);

	ebc1->EBC1_BrakePedalPosition = 
		percent_0_to_100(pdu->data_field[1]);

	byte = (unsigned int) pdu->data_field[2];
	ebc1->traction_control_override_switch_status = BITS87(byte);
	ebc1->asr_hill_holder_switch_status = BITS65(byte);
//#####	ebc1->EBC1_ASROffroadSwitch = BITS43(byte);

	byte = (unsigned int) pdu->data_field[3];
	ebc1->EBC1_RemoteAccelEnableSwitch = BITS87(byte);
	ebc1->auxiliary_engine_shutdown_switch_status = BITS65(byte);
	ebc1->engine_derate_switch_status = BITS43(byte);
	ebc1->accelerator_interlock_switch_status = BITS21(byte);

	ebc1->EBC1_EngRetarderSelection = 
		percent_0_to_100(pdu->data_field[4]);

	byte = (unsigned int) pdu->data_field[5];
	ebc1->ABSEBSAmberWarningSignal = BITS65(byte);
	ebc1->EBC1_EBSRedWarningSignal = BITS43(byte);
	ebc1->EBC1_ABSFullyOperational = BITS21(byte);

	ebc1->EBC1_SrcAddrssOfCtrllngDvcFrBrkCntrl = pdu->data_field[6];
	ebc1->total_brake_demand = brake_demand(pdu->data_field[7]);
}

void 
print_ebc1(void *pdbv, FILE  *fp, int numeric)

{
	j1939_ebc1_typ *ebc1 = (j1939_ebc1_typ *)pdbv;
	fprintf(fp, "EBC1 ");
	print_timestamp(fp, &ebc1->timestamp);
	if (numeric) {
		fprintf(fp, "%d ", ebc1->EBSBrakeSwitch);
		fprintf(fp, "%d ", ebc1->EBC1_AntiLockBrakingActive);
		fprintf(fp, "%d ", ebc1->EBC1_ASRBrakeControlActive);
		fprintf(fp, "%d ", ebc1->EBC1_ASREngineControlActive);
		fprintf(fp, "%.2f ", ebc1->EBC1_BrakePedalPosition);
		fprintf(fp, "%d ",
			 ebc1->traction_control_override_switch_status);
		fprintf(fp, "%d ", ebc1->asr_hill_holder_switch_status);
		fprintf(fp, "%d ", ebc1->EBC1_ASROffroadSwitch);
		fprintf(fp, "%d ",
			 ebc1->EBC1_RemoteAccelEnableSwitch);
		fprintf(fp, "%d ",
			 ebc1->auxiliary_engine_shutdown_switch_status);
		fprintf(fp, "%d ", ebc1->engine_derate_switch_status);
		fprintf(fp, "%d ", ebc1->accelerator_interlock_switch_status);
		fprintf(fp, "%.2f ",
			 ebc1->EBC1_EngRetarderSelection);
		fprintf(fp, "%d ", ebc1->ABSEBSAmberWarningSignal);
		fprintf(fp, "%d ", ebc1->EBC1_EBSRedWarningSignal);
		fprintf(fp, "%d ", ebc1->EBC1_ABSFullyOperational);

	 	fprintf(fp, "%d ", ebc1->EBC1_SrcAddrssOfCtrllngDvcFrBrkCntrl);
	 	fprintf(fp, "%.3f ", ebc1->total_brake_demand);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp, "EBS brake switch status %d \n",
			 ebc1->EBSBrakeSwitch);
		fprintf(fp, "ABS active status %d \n",
			 ebc1->EBC1_AntiLockBrakingActive);
		fprintf(fp, "ASR brake control status%d \n",
			 ebc1->EBC1_ASRBrakeControlActive);
		fprintf(fp, "ASR engine control active status %d \n",
			 ebc1->EBC1_ASREngineControlActive);
		fprintf(fp, "Brake pedal position %.2f\n ",
			 ebc1->EBC1_BrakePedalPosition);
		fprintf(fp, "Traction contorl override switch status %d \n",
			 ebc1->traction_control_override_switch_status);
		fprintf(fp, "Hill holder switch status %d \n",
			 ebc1->asr_hill_holder_switch_status);
		fprintf(fp, "ASR off road switch status %d \n",
			 ebc1->EBC1_ASROffroadSwitch);
		fprintf(fp, "Remote accelerator enable switch status %d \n",
			 ebc1->EBC1_RemoteAccelEnableSwitch);
		fprintf(fp, "Auxiliary engine shutdown switch status %d \n",
			 ebc1->auxiliary_engine_shutdown_switch_status);
		fprintf(fp, "Engine derate switch status %d \n",
			 ebc1->engine_derate_switch_status);
		fprintf(fp, "Accelerator interlock switch status %d \n",
			 ebc1->accelerator_interlock_switch_status);
		fprintf(fp, "Percent engine retarder torque selected %.2f \n",
			 ebc1->EBC1_EngRetarderSelection);
		fprintf(fp, "ABS/EBS amber warning state %d \n",
			 ebc1->ABSEBSAmberWarningSignal);
		fprintf(fp, "EBS red warning state %d \n",
			 ebc1->EBC1_EBSRedWarningSignal);
		fprintf(fp, "ABS fully operational %d \n",
			 ebc1->EBC1_ABSFullyOperational);
		fprintf(fp, "Source address %d (0x%0x)\n",
			 ebc1->EBC1_SrcAddrssOfCtrllngDvcFrBrkCntrl, ebc1->EBC1_SrcAddrssOfCtrllngDvcFrBrkCntrl);
		fprintf(fp, "Total brake demand %.3f\n",
			ebc1->total_brake_demand);
	}
}


/** ETC1 (Electronic Retarder Controller #1) documented in J1939 - 71, p 150 */
void
pdu_to_etc1 (struct j1939_pdu *pdu, void *pdbv)
{ 
	j1939_etc1_typ *etc1 = (j1939_etc1_typ *)pdbv;
	unsigned char byte;
	unsigned short two_bytes;

	/* two-bit fields in data byte 0 indicate status of switch,
         * enabled or disabled.
	 */

	byte = (unsigned int) pdu->data_field[0];
	etc1->ETC1_TransmissionShiftInProcess = BITS65(byte);
	etc1->ETC1_TorqueConverterLockupEngaged = BITS43(byte);
	etc1->ETC1_TransmissionDrivelineEngaged = BITS21(byte);

	two_bytes = TWOBYTES(pdu->data_field[2], pdu->data_field[1]);
	etc1->ETC1_TransmissionOutputShaftSpeed = speed_in_rpm_2byte(two_bytes);

	etc1->ETC1_PercentClutchSlip = percent_0_to_100(pdu->data_field[3]);

	byte = pdu->data_field[4];
	etc1->ETC1_ProgressiveShiftDisable = BITS43(byte);
	etc1->ETC1_MomentaryEngineOverspeedEnable = BITS21(byte);

	two_bytes = TWOBYTES(pdu->data_field[6], pdu->data_field[5]);
	etc1->ETC1_TransInputShaftSpeed = speed_in_rpm_2byte(two_bytes);

	etc1->ETC1_SrcAddrssOfCtrllngDvcFrTrnsCtrl = pdu->data_field[7];
}

void 
print_etc1(void *pdbv, FILE  *fp, int numeric)

{
	j1939_etc1_typ *etc1 = (j1939_etc1_typ *)pdbv;
	fprintf(fp, "ETC1 ");
	print_timestamp(fp, &etc1->timestamp);
	if (numeric) {
		fprintf(fp, "%d %d %d %.2f",
			etc1->ETC1_TransmissionShiftInProcess,	/* 2 */
			etc1->ETC1_TorqueConverterLockupEngaged,	/* 3 */
			etc1->ETC1_TransmissionDrivelineEngaged,	/* 4 */
			etc1->ETC1_TransmissionOutputShaftSpeed);	/* 5 */
		fprintf(fp, " %.2f %d %d %.2f %d\n",
			etc1->ETC1_PercentClutchSlip,	/* 6 */
			etc1->ETC1_ProgressiveShiftDisable,	/* 7 */
			etc1->ETC1_MomentaryEngineOverspeedEnable,	/* 8 */
			etc1->ETC1_TransInputShaftSpeed,	/* 9 */
			etc1->ETC1_SrcAddrssOfCtrllngDvcFrTrnsCtrl);		/* 10 */			
		fprintf(fp, "\n");	
	}
	else {
		fprintf(fp, "Shift in progress %d\n",
			 etc1->ETC1_TransmissionShiftInProcess);
		fprintf(fp, "Torque converter lockup engaged %d\n",
			 etc1->ETC1_TorqueConverterLockupEngaged);
		fprintf(fp, "Driveline engaged %d\n",
			 etc1->ETC1_TransmissionDrivelineEngaged);
		fprintf(fp, "Output shaft speed %.2f\n",
			 etc1->ETC1_TransmissionOutputShaftSpeed);
		fprintf(fp, "Percent clutch slip %.2f\n",
			 etc1->ETC1_PercentClutchSlip);
		fprintf(fp, "Progressive shift disable %d\n",
			 etc1->ETC1_ProgressiveShiftDisable);
		fprintf(fp, "Momentary engine overspeed enable %d\n",
			 etc1->ETC1_MomentaryEngineOverspeedEnable);
		fprintf(fp, "Input shaft speed %.2f\n",
			 etc1->ETC1_TransInputShaftSpeed);
		fprintf(fp, "Source address %d (0x%0x)\n", etc1->ETC1_SrcAddrssOfCtrllngDvcFrTrnsCtrl,
			etc1->ETC1_SrcAddrssOfCtrllngDvcFrTrnsCtrl);
	}
}



/** EEC1 (Electronic Engine Controller #1) documented in J1939 - 71, p152 */
void
pdu_to_eec1 (struct j1939_pdu *pdu, void *pdbv)
{
	j1939_eec1_typ *eec1 = (j1939_eec1_typ *)pdbv;
	unsigned short two_bytes;

	eec1->EEC1_EngineTorqueMode = LONIBBLE(pdu->data_field[0]);
	eec1->EEC1_DrvrDemandEngPercentTorque =
		 percent_m125_to_p125(pdu->data_field[1]); /* max 125% */
	eec1->EEC1_ActualEnginePercentTorque =
		 percent_m125_to_p125(pdu->data_field[2]); /* max 125% */
	eec1->EEC1_EngDemandPercentTorque =
		 percent_m125_to_p125(pdu->data_field[7]); /* max 125% */
	two_bytes = TWOBYTES(pdu->data_field[4], pdu->data_field[3]);
	eec1->EEC1_EngineSpeed = speed_in_rpm_2byte(two_bytes);
	eec1->source_address = pdu->data_field[5];
}

void 
print_eec1(void *pdbv, FILE  *fp, int numeric)

{
	j1939_eec1_typ *eec1 = (j1939_eec1_typ *)pdbv;
	fprintf(fp, "EEC1 ");
	print_timestamp(fp, &eec1->timestamp);
	if (numeric) {
		fprintf(fp," %d", eec1->EEC1_EngineTorqueMode);
		fprintf(fp," %.2f", eec1->EEC1_DrvrDemandEngPercentTorque);
		fprintf(fp," %.2f", eec1->EEC1_ActualEnginePercentTorque);
		fprintf(fp," %.2f", eec1->EEC1_EngDemandPercentTorque);
		fprintf(fp," %.3f", eec1->EEC1_EngineSpeed);
		fprintf(fp," %d", eec1->source_address);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp,"Engine retarder torque mode %d\n",
			 eec1->EEC1_EngineTorqueMode);
		fprintf(fp,"Driver's demand percent torque %.2f\n",
			 eec1->EEC1_DrvrDemandEngPercentTorque);
		fprintf(fp,"Actual engine percent torque %.2f\n",
			 eec1->EEC1_ActualEnginePercentTorque);
		fprintf(fp,"Engine speed (rpm) %.3f\n", eec1->EEC1_EngineSpeed);
		fprintf(fp,"Source address engine control device %d\n",
			 eec1->source_address);
	}

}

/** EEC2 (Electronic Engine Controller #2) documented in J1939 - 71, p152 */
void
pdu_to_eec2 (struct j1939_pdu *pdu, void *pdbv)
{
	j1939_eec2_typ *eec2 = (j1939_eec2_typ *)pdbv;
	unsigned char byte;

	/* two-bit fields in data byte 0 indicate active/inactive */
	/* 00 active, 01 not active for road speed limit, see p. 140 */ 

	byte = (unsigned int) pdu->data_field[0];
	eec2->EEC2_AccelPedal2LowIdleSwitch = BITS87(byte);
	eec2->EEC2_RoadSpeedLimitStatus = BITS65(byte);
	eec2->EEC2_AccelPedalKickdownSwitch = BITS43(byte);
	eec2->EEC2_AccelPedal1LowIdleSwitch = BITS21(byte);

	eec2->EEC2_AccelPedalPos1 =
		 percent_0_to_100(pdu->data_field[1]);

	eec2->EEC2_AccelPedalPos2 =
		 percent_0_to_100(pdu->data_field[4]);

	eec2->EEC2_ActMaxAvailEngPercentTorque =
		 percent_0_to_100(pdu->data_field[6]);

	eec2->EEC2_EnginePercentLoadAtCurrentSpd = 
		percent_0_to_250(pdu->data_field[2]);	/* max 125% */

}

void 
print_eec2(void *pdbv, FILE  *fp, int numeric)

{
	j1939_eec2_typ *eec2 = (j1939_eec2_typ *)pdbv;
	fprintf(fp, "EEC2 ");
	print_timestamp(fp, &eec2->timestamp);
	if (numeric){
		fprintf(fp," %d", eec2->EEC2_RoadSpeedLimitStatus);
		fprintf(fp," %d", eec2->EEC2_AccelPedalKickdownSwitch);
		fprintf(fp," %d", eec2->EEC2_AccelPedal1LowIdleSwitch);
		fprintf(fp," %d", eec2->EEC2_AccelPedal2LowIdleSwitch);
		fprintf(fp," %.2f", eec2->EEC2_ActMaxAvailEngPercentTorque);
		fprintf(fp," %.2f", eec2->EEC2_AccelPedalPos1);
		fprintf(fp," %.2f", eec2->EEC2_AccelPedalPos2);
		fprintf(fp," %.2f", eec2->EEC2_EnginePercentLoadAtCurrentSpd);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp,"Road speed limit %d\n", eec2->EEC2_RoadSpeedLimitStatus);
		fprintf(fp,"Kickpedal active %d\n", eec2->EEC2_AccelPedalKickdownSwitch);
		fprintf(fp,"Low idle 1 %d\n", eec2->EEC2_AccelPedal1LowIdleSwitch);
		fprintf(fp,"Low idle 2 %d\n", eec2->EEC2_AccelPedal2LowIdleSwitch);
		fprintf(fp,"AP1 position %.2f\n", eec2->EEC2_AccelPedalPos1);
		fprintf(fp,"AP2 position %.2f\n", eec2->EEC2_AccelPedalPos2);
		fprintf(fp,"Percent load %.2f\n", eec2->EEC2_EnginePercentLoadAtCurrentSpd);
		fprintf(fp,"Percent torque %.2f\n", eec2->EEC2_ActMaxAvailEngPercentTorque);
	}
}

/** ETC2 (Electronic Transmission Controller #3) documented in J1939 - 71, p152
*/
void
pdu_to_etc2 (struct j1939_pdu *pdu, void *pdbv)
{
	j1939_etc2_typ *etc2 = (j1939_etc2_typ *)pdbv;
	unsigned short two_bytes;

	etc2->ETC2_TransmissionSelectedGear =
		 gear_m125_to_p125(pdu->data_field[0]);

	two_bytes = TWOBYTES(pdu->data_field[2], pdu->data_field[1]);
	etc2->ETC2_TransmissionActualGearRatio = gear_ratio(two_bytes);	

	etc2->ETC2_TransmissionCurrentGear =
		 gear_m125_to_p125(pdu->data_field[3]);

	two_bytes = TWOBYTES(pdu->data_field[5], pdu->data_field[4]);
	etc2->ETC2_TransmissionRangeSelected = two_bytes;

	two_bytes = TWOBYTES(pdu->data_field[7], pdu->data_field[6]);
	etc2->ETC2_TransmissionRangeAttained = two_bytes;
	printf("ETC2 Raw %#02hhx %#02hhx %#02hhx %#02hhx %#02hhx %#02hhx %#02hhx %#02hhx\n",
		pdu->data_field[0],
		pdu->data_field[1],
		pdu->data_field[2],
		pdu->data_field[3],
		pdu->data_field[4],
		pdu->data_field[5],
		pdu->data_field[6],
		pdu->data_field[7]
		);

}

void 
print_etc2(void *pdbv, FILE  *fp, int numeric)

{
	j1939_etc2_typ *etc2 = (j1939_etc2_typ *)pdbv;
	fprintf(fp, "ETC2 ");
	print_timestamp(fp, &etc2->timestamp);
	if (numeric){
		fprintf(fp," %d", etc2->ETC2_TransmissionSelectedGear);
		fprintf(fp," %.2f", etc2->ETC2_TransmissionActualGearRatio);
		fprintf(fp," %d", etc2->ETC2_TransmissionCurrentGear);
		fprintf(fp," %d", etc2->ETC2_TransmissionRangeSelected);
		fprintf(fp," %d", etc2->ETC2_TransmissionRangeAttained);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp,"Selected gear %d\n", etc2->ETC2_TransmissionSelectedGear);
		fprintf(fp,"Actual gear ratio %.2f\n",
			 etc2->ETC2_TransmissionActualGearRatio);
		fprintf(fp,"Current gear %d\n", etc2->ETC2_TransmissionCurrentGear);
		fprintf(fp,"Transmission requested range %d\n",
			etc2->ETC2_TransmissionRangeSelected);
		fprintf(fp,"Transmission current range %d\n",
			etc2->ETC2_TransmissionRangeAttained);
	}
}

/** ETC2_E (Electronic Tr_eansmission Controller #3) documented in J1939 - 71, p152
*/
void
pdu_to_etc2_e (struct j1939_pdu *pdu, void *pdbv)
{
	j1939_etc2_e_typ *etc2_e = (j1939_etc2_e_typ *)pdbv;
	unsigned short two_bytes;

	etc2_e->ETC2_E_TransmissionSelectedGear =
		 gear_m125_to_p125(pdu->data_field[0]);

	two_bytes = TWOBYTES(pdu->data_field[2], pdu->data_field[1]);
	etc2_e->ETC2_E_TransmissionActualGearRatio = gear_ratio(two_bytes);	

	etc2_e->ETC2_E_TransmissionCurrentGear =
		 gear_m125_to_p125(pdu->data_field[3]);

	two_bytes = TWOBYTES(pdu->data_field[5], pdu->data_field[4]);
	etc2_e->ETC2_E_TransmissionRangeSelected = two_bytes;

	two_bytes = TWOBYTES(pdu->data_field[7], pdu->data_field[6]);
	etc2_e->ETC2_E_TransmissionRangeAttained = two_bytes;
	printf("ETC2_E Raw %#02hhx %#02hhx %#02hhx %#02hhx %#02hhx %#02hhx %#02hhx %#02hhx\n",
		pdu->data_field[0],
		pdu->data_field[1],
		pdu->data_field[2],
		pdu->data_field[3],
		pdu->data_field[4],
		pdu->data_field[5],
		pdu->data_field[6],
		pdu->data_field[7]
		);

}

void 
print_etc2_e(void *pdbv, FILE  *fp, int numeric)

{
	j1939_etc2_e_typ *etc2_e = (j1939_etc2_e_typ *)pdbv;
	fprintf(fp, "ETC2_E ");
	print_timestamp(fp, &etc2_e->timestamp);
	if (numeric){
		fprintf(fp," %d", etc2_e->ETC2_E_TransmissionSelectedGear);
		fprintf(fp," %.2f", etc2_e->ETC2_E_TransmissionActualGearRatio);
		fprintf(fp," %d", etc2_e->ETC2_E_TransmissionCurrentGear);
		fprintf(fp," %d", etc2_e->ETC2_E_TransmissionRangeSelected);
		fprintf(fp," %d", etc2_e->ETC2_E_TransmissionRangeAttained);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp,"Selected gear %d\n", etc2_e->ETC2_E_TransmissionSelectedGear);
		fprintf(fp,"Actual gear ratio %.2f\n",
			 etc2_e->ETC2_E_TransmissionActualGearRatio);
		fprintf(fp,"Current gear %d\n", etc2_e->ETC2_E_TransmissionCurrentGear);
		fprintf(fp,"Transmission requested range %d\n",
			etc2_e->ETC2_E_TransmissionRangeSelected);
		fprintf(fp,"Transmission current range %d\n",
			etc2_e->ETC2_E_TransmissionRangeAttained);
	}
}
/** TURBO (Turbocharger) documented in J1939 - 71, p153
*/
void
pdu_to_turbo (struct j1939_pdu *pdu, void *pdbv)
{
	j1939_turbo_typ *turbo = (j1939_turbo_typ *)pdbv;
	unsigned short two_bytes;

	turbo->turbocharger_lube_oil_pressure =
		 pressure_0_to_1000kpa(pdu->data_field[0]);

	two_bytes = TWOBYTES(pdu->data_field[2], pdu->data_field[1]);

	turbo->turbocharger_speed = rotor_speed_in_rpm(two_bytes);	
}

void 
print_turbo(void *pdbv, FILE  *fp, int numeric)

{
	j1939_turbo_typ *turbo = (j1939_turbo_typ *)pdbv;
	fprintf(fp, "TURBO ");
	print_timestamp(fp, &turbo->timestamp);
	if (numeric){
		fprintf(fp," %.2f", turbo->turbocharger_lube_oil_pressure);
		fprintf(fp," %.2f", turbo->turbocharger_speed);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp,"Turbocharger lube oil pressure %.2f",
			 turbo->turbocharger_lube_oil_pressure);
		fprintf(fp,"Turbocharger speed %.2f",
			 turbo->turbocharger_speed);
		fprintf(fp, "\n");	
	}
}

/** EEC3 (Electronic Engine Controller #3) documented in J1939 - 71, p154 */
void
pdu_to_eec3 (struct j1939_pdu *pdu, void *pdbv)
{
	unsigned short two_bytes;

	j1939_eec3_typ *eec3 = (j1939_eec3_typ *)pdbv;

	eec3->EEC3_NominalFrictionPercentTorque =
		 percent_m125_to_p125(pdu->data_field[0]);

	two_bytes = TWOBYTES(pdu->data_field[2], pdu->data_field[1]);
	eec3->EEC3_EngsDesiredOperatingSpeed = 0.125 * two_bytes;

	eec3->EEC3_EstEngPrsticLossesPercentTorque =
		 percent_m125_to_p125(pdu->data_field[4]);
}

void 
print_eec3(void *pdbv, FILE  *fp, int numeric)

{
	j1939_eec3_typ *eec3 = (j1939_eec3_typ *)pdbv;
	fprintf(fp, "EEC3 ");
	print_timestamp(fp, &eec3->timestamp);
	if (numeric){
		fprintf(fp," %.2f", eec3->EEC3_NominalFrictionPercentTorque);
		fprintf(fp," %.2f", eec3->EEC3_EstEngPrsticLossesPercentTorque);
		fprintf(fp," %.2f", eec3->EEC3_EngsDesiredOperatingSpeed);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp,"Nominal friction percent torque %.2f\n",
			 eec3->EEC3_NominalFrictionPercentTorque);
		fprintf(fp,"Estimated engine power loss as a percentage of torque %.3f\n",
			 eec3->EEC3_EstEngPrsticLossesPercentTorque);
		fprintf(fp,"Engine desired operating speed %.3f\n",
			 eec3->EEC3_EngsDesiredOperatingSpeed);
	}
}

/** VD (Vehicle Distance) documented in J1939 - 71, p154 */
void
pdu_to_vd (struct j1939_pdu *pdu, void *pdbv)
{
	j1939_vd_typ *vd = (j1939_vd_typ *)pdbv;
	unsigned int four_bytes;

	four_bytes = FOURBYTES(pdu->data_field[3], pdu->data_field[2],
			pdu->data_field[2], pdu->data_field[0]);

	vd->trip_distance = distance_in_km(four_bytes); 

	four_bytes = FOURBYTES(pdu->data_field[7], pdu->data_field[6],
			pdu->data_field[5], pdu->data_field[4]);
	vd->total_vehicle_distance = distance_in_km(four_bytes);
}

void 
print_vd(void *pdbv, FILE  *fp, int numeric)
{
	j1939_vd_typ *vd = (j1939_vd_typ *)pdbv;
	fprintf(fp, "VD ");
	print_timestamp(fp, &vd->timestamp);
	if (numeric){
		fprintf(fp," %.2f", vd->trip_distance);
		fprintf(fp," %.2f", vd->total_vehicle_distance);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp,"Trip distance (km) %.2f\n",
			 vd->trip_distance);
		fprintf(fp,"Total vehicle distance (km) %.2f\n",
			 vd->total_vehicle_distance);
	}
}

/** ETEMP (Engine Temperature) documented in J1939 - 71, p160 */
void
pdu_to_etemp (struct j1939_pdu *pdu, void *pdbv)
{
	j1939_etemp_typ *etemp = (j1939_etemp_typ *)pdbv;
	unsigned short two_bytes;

	etemp->engine_coolant_temperature =
		temp_m40_to_p210(pdu->data_field[0]);	

	etemp->fuel_temperature =
		temp_m40_to_p210(pdu->data_field[1]);	

	two_bytes = TWOBYTES(pdu->data_field[3], pdu->data_field[2]);
	etemp->engine_oil_temperature =
		temp_m273_to_p1735(two_bytes);	

	two_bytes = TWOBYTES(pdu->data_field[5], pdu->data_field[4]);
	etemp->turbo_oil_temperature =
		temp_m273_to_p1735(two_bytes);	

	etemp->engine_intercooler_temperature =
		temp_m40_to_p210(pdu->data_field[6]);	

	etemp->engine_intercooler_thermostat_opening =
		percent_0_to_100(pdu->data_field[7]);
}

void 
print_etemp(void *pdbv, FILE  *fp, int numeric)

{
	j1939_etemp_typ *etemp = (j1939_etemp_typ *)pdbv;
	fprintf(fp, "ETEMP ");
	print_timestamp(fp, &etemp->timestamp);
	if (numeric){
		fprintf(fp," %.3f", etemp->engine_coolant_temperature);
		fprintf(fp," %.3f", etemp->fuel_temperature);
		fprintf(fp," %.3f", etemp->engine_oil_temperature);
		fprintf(fp," %.3f", etemp->turbo_oil_temperature);
		fprintf(fp," %.3f", etemp->engine_intercooler_temperature);
		fprintf(fp," %.3f",
			etemp->engine_intercooler_thermostat_opening);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp,"Engine coolant temperature %.3f\n",
			etemp->engine_coolant_temperature);
		fprintf(fp,"Fuel temperature %.3f\n",
			etemp->fuel_temperature);
		fprintf(fp,"Engine oil temperature %.3f\n",
			etemp->engine_oil_temperature);
		fprintf(fp,"Turbo oil temperature %.3f\n",
			etemp->turbo_oil_temperature);
		fprintf(fp,"Engine intercooler temperature %.3f\n",
			etemp->engine_intercooler_temperature);
		fprintf(fp,"Engine intercooler thermostat opening %.3f\n",
			etemp->engine_intercooler_thermostat_opening);
	}
}

/** PTO (Power Takeoff) documented in J1939 - 71, p160 */
void
pdu_to_pto (struct j1939_pdu *pdu, void *pdbv)
{
	j1939_pto_typ *pto = (j1939_pto_typ *)pdbv;
	unsigned short two_bytes;
	unsigned char byte;

	pto->pto_oil_temperature =
		temp_m40_to_p210(pdu->data_field[0]);	

	two_bytes = TWOBYTES(pdu->data_field[2], pdu->data_field[1]);
	pto->pto_speed =
		speed_in_rpm_2byte(two_bytes);	

	two_bytes = TWOBYTES(pdu->data_field[4], pdu->data_field[3]);
	pto->pto_set_speed =
		speed_in_rpm_2byte(two_bytes);	

	byte = pdu->data_field[5];
	pto->remote_pto_variable_speed_status = BITS65(byte);
	pto->remote_pto_preprogrammed_status = BITS43(byte);
	pto->pto_enable_switch_status = BITS21(byte);

	byte = pdu->data_field[6];
	pto->pto_accelerate_switch_status = BITS87(byte);
	pto->pto_resume_switch_status = BITS65(byte);
	pto->pto_coast_accelerate_switch_status = BITS43(byte);
	pto->pto_set_switch_status = BITS21(byte);
}

void 
print_pto(void *pdbv, FILE  *fp, int numeric)
{
	j1939_pto_typ *pto = (j1939_pto_typ *)pdbv;
	fprintf(fp, "PTO ");
	print_timestamp(fp, &pto->timestamp);
	if (numeric){
		fprintf(fp," %.3f", pto->pto_oil_temperature);
		fprintf(fp," %.3f", pto->pto_speed);
		fprintf(fp," %.3f", pto->pto_set_speed);
		fprintf(fp," %d", pto->remote_pto_variable_speed_status);
		fprintf(fp," %d", pto->remote_pto_preprogrammed_status);
		fprintf(fp," %d", pto->pto_enable_switch_status);
		fprintf(fp," %d", pto->pto_accelerate_switch_status);
		fprintf(fp," %d", pto->pto_resume_switch_status);
		fprintf(fp," %d", pto->pto_coast_accelerate_switch_status);
		fprintf(fp," %d", pto->pto_set_switch_status);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp,"PTO oil temperature %.3f\n",
			 pto->pto_oil_temperature);
		fprintf(fp,"PTO speed %.3f\n",
			 pto->pto_speed);
		fprintf(fp,"PTO set speed %.3f\n",
			 pto->pto_set_speed);
		fprintf(fp,"Remote PTO variable speed control switch %d\n",
			 pto->remote_pto_variable_speed_status);
		fprintf(fp,
			"Remote PTO preprogrammed speed control switch %d\n",
			 pto->remote_pto_preprogrammed_status);
		fprintf(fp,"PTO enable switch %d\n",
			 pto->pto_enable_switch_status);
		fprintf(fp,"PTO accelerate switch %d\n",
			 pto->pto_accelerate_switch_status);
		fprintf(fp,"PTO resume switch %d\n",
			 pto->pto_resume_switch_status);
		fprintf(fp,"PTO coast decelerate switch %d\n",
			 pto->pto_coast_accelerate_switch_status);
		fprintf(fp,"PTO set switch %d\n",
			 pto->pto_set_switch_status);
	}
}

/** CCVS (Cruise Control/Vehicle Speed) documented in J1939 - 71, p162 */
void
pdu_to_ccvs (struct j1939_pdu *pdu, void *pdbv)
{
	j1939_ccvs_typ *ccvs = (j1939_ccvs_typ *)pdbv;
	unsigned char byte;
	unsigned short two_bytes;

	byte =  pdu->data_field[0];
	ccvs->CCVS_ParkBrakeReleaseInhibitRq = BITS87(byte);
	ccvs->CCVS_CruiseCtrlPauseSwitch = BITS65(byte);
	ccvs->CCVS_ParkingBrakeSwitch = BITS43(byte);
	ccvs->CCVS_TwoSpeedAxleSwitch = BITS21(byte);

	two_bytes = TWOBYTES(pdu->data_field[2], pdu->data_field[1]);
	ccvs->CCVS_VehicleSpeed = wheel_based_mps(two_bytes);	

	byte = pdu->data_field[3];
	ccvs->CCVS_ClutchSwitch = BITS87(byte);
	ccvs->CCVS_BrakeSwitch = BITS65(byte);
	ccvs->CCVS_CruiseControlEnableSwitch = BITS43(byte);
	ccvs->CCVS_CruiseControlActive = BITS21(byte);

	byte = pdu->data_field[4];
	ccvs->CCVS_CruiseControlAccelerateSwitch = BITS87(byte);
	ccvs->CCVS_CruiseControlResumeSwitch = BITS65(byte);
	ccvs->CCVS_CruiseControlCoastSwitch = BITS43(byte);
	ccvs->CCVS_CruiseControlSetSwitch = BITS21(byte);

	ccvs->CCVS_CruiseControlSetSpeed =
		 cruise_control_set_meters_per_sec(pdu->data_field[5]);
	
	byte = pdu->data_field[6];
	ccvs->CCVS_CruiseControlState = HINIBBLE(byte) >> 1;
	ccvs->CCVS_PtoState = byte & 0x1f;
	
	byte = pdu->data_field[7];
	ccvs->CCVS_EngShutdownOverrideSwitch = BITS87(byte);
	ccvs->CCVS_EngTestModeSwitch = BITS65(byte);
	ccvs->CCVS_EngIdleDecrementSwitch = BITS43(byte);
	ccvs->CCVS_EngIdleIncrementSwitch = BITS21(byte);
}

void 
print_ccvs(void *pdbv, FILE  *fp, int numeric) 
{
	j1939_ccvs_typ *ccvs = (j1939_ccvs_typ *)pdbv;
	fprintf(fp, "CCVS ");
	print_timestamp(fp, &ccvs->timestamp);
	if (numeric){
		fprintf(fp," %d", ccvs->CCVS_ParkBrakeReleaseInhibitRq );
		fprintf(fp," %d", ccvs->CCVS_ParkingBrakeSwitch);
		fprintf(fp," %d", ccvs->CCVS_TwoSpeedAxleSwitch);
		fprintf(fp," %.3f", ccvs->CCVS_VehicleSpeed);
		fprintf(fp," %d", ccvs->CCVS_ClutchSwitch);
		fprintf(fp," %d", ccvs->CCVS_BrakeSwitch);
		fprintf(fp," %d", ccvs->CCVS_CruiseCtrlPauseSwitch);
		fprintf(fp," %d", ccvs->CCVS_CruiseControlEnableSwitch);
		fprintf(fp," %d", ccvs->CCVS_CruiseControlActive);
		fprintf(fp," %d", ccvs->CCVS_CruiseControlAccelerateSwitch);
		fprintf(fp," %d", ccvs->CCVS_CruiseControlResumeSwitch);
		fprintf(fp," %d", ccvs->CCVS_CruiseControlCoastSwitch);
		fprintf(fp," %d", ccvs->CCVS_CruiseControlSetSwitch);
		fprintf(fp," %.3f", ccvs->CCVS_CruiseControlSetSpeed);
		fprintf(fp," %d", ccvs->CCVS_CruiseControlState);
		fprintf(fp," %d", ccvs->CCVS_PtoState);
		fprintf(fp," %d", ccvs->CCVS_EngShutdownOverrideSwitch);
		fprintf(fp," %d", ccvs->CCVS_EngTestModeSwitch);
		fprintf(fp," %d", ccvs->CCVS_EngIdleDecrementSwitch);
		fprintf(fp," %d", ccvs->CCVS_EngIdleIncrementSwitch);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp,"Parking brake %d\n", ccvs->CCVS_ParkingBrakeSwitch);
		fprintf(fp,"Parking brake inhibit %d\n", ccvs->CCVS_ParkBrakeReleaseInhibitRq);
		fprintf(fp,"Two speed axle switch %d\n", ccvs->CCVS_TwoSpeedAxleSwitch);
		fprintf(fp,"Vehicle speed (meters/sec) %.3f\n",
			ccvs->CCVS_VehicleSpeed);
		fprintf(fp,"Clutch switch %d\n", ccvs->CCVS_ClutchSwitch);
		fprintf(fp,"Brake switch %d\n", ccvs->CCVS_BrakeSwitch);
		fprintf(fp,"Cruise control pause %d\n", ccvs->CCVS_CruiseCtrlPauseSwitch);
		fprintf(fp,"Cruise control enable %d\n", ccvs->CCVS_CruiseControlEnableSwitch);
		fprintf(fp,"Cruise control active %d\n", ccvs->CCVS_CruiseControlActive);
		fprintf(fp,"Cruise control accelerate %d\n",
			ccvs->CCVS_CruiseControlAccelerateSwitch);
		fprintf(fp,"Cruise control resume %d\n", ccvs->CCVS_CruiseControlResumeSwitch);
		fprintf(fp,"Cruise control coast %d\n", ccvs->CCVS_CruiseControlCoastSwitch);
		fprintf(fp,"Cruise control set %d\n", ccvs->CCVS_CruiseControlSetSwitch);
		fprintf(fp,"Cruise control set speed %.3f\n", 
			ccvs->CCVS_CruiseControlSetSpeed);
		fprintf(fp,"Cruise control state %d\n", ccvs->CCVS_CruiseControlState);
		fprintf(fp,"PTO state %d\n", ccvs->CCVS_PtoState);
		fprintf(fp,"Engine shutdown override %d\n",
			 ccvs->CCVS_EngShutdownOverrideSwitch);
		fprintf(fp,"Engine test mode %d\n", ccvs->CCVS_EngTestModeSwitch);
		fprintf(fp,"Idle decrement %d\n", ccvs->CCVS_EngIdleDecrementSwitch);
		fprintf(fp,"Idle increment %d\n", ccvs->CCVS_EngIdleIncrementSwitch);
	}
}

/** LFE (Fuel Economy) documented in J1939 - 71, p162 */
void
pdu_to_lfe (struct j1939_pdu *pdu, void *pdbv)
{
	j1939_lfe_typ *lfe = (j1939_lfe_typ *)pdbv;
	unsigned short two_bytes;

	two_bytes = TWOBYTES(pdu->data_field[1], pdu->data_field[0]);
	lfe->LFE_EngineFuelRate = fuel_rate_cm3_per_sec(two_bytes);	

	two_bytes = TWOBYTES(pdu->data_field[3], pdu->data_field[2]);
	lfe->LFE_EngineInstantaneousFuelEconomy =
		fuel_economy_meters_per_cm3(two_bytes);	

	two_bytes = TWOBYTES(pdu->data_field[5], pdu->data_field[4]);
	lfe->LFE_EngineAverageFuelEconomy =
		fuel_economy_meters_per_cm3(two_bytes);	

	lfe->LFE_EngineThrottleValve1Position = percent_0_to_100(pdu->data_field[6]);
}

void 
print_lfe(void *pdbv, FILE  *fp, int numeric)
{
	j1939_lfe_typ *lfe = (j1939_lfe_typ *)pdbv;
	fprintf(fp, "LFE ");
	print_timestamp(fp, &lfe->timestamp);
	if (numeric){
		fprintf(fp," %.3f", lfe->LFE_EngineFuelRate);
		fprintf(fp," %.3f", lfe->LFE_EngineInstantaneousFuelEconomy);
		fprintf(fp," %.3f", lfe->LFE_EngineAverageFuelEconomy);
		fprintf(fp," %.3f", lfe->LFE_EngineThrottleValve1Position);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp,"Fuel rate (cm3/sec) %.3f\n", lfe->LFE_EngineFuelRate);
		fprintf(fp,"Instantaneous fuel economy (m/cm3) %.3f\n",
			 lfe->LFE_EngineInstantaneousFuelEconomy);
		fprintf(fp,"Average fuel economy (m/cm3) %.3f\n",
			 lfe->LFE_EngineAverageFuelEconomy);
		fprintf(fp,"Throttle position (percent) %.3f\n",
			 lfe->LFE_EngineThrottleValve1Position);
	}
}

/** AMBC (Ambient Conditions) documented in J1939 - 71, p163 */
void
pdu_to_ambc (struct j1939_pdu *pdu, void *pdbv)
{
	j1939_ambc_typ *ambc = (j1939_ambc_typ *)pdbv;
	unsigned short two_bytes;

	ambc->barometric_pressure =
		pressure_0_to_125kpa(pdu->data_field[0]);

	two_bytes = TWOBYTES(pdu->data_field[2], pdu->data_field[1]);
	ambc->cab_interior_temperature = 
		temp_m273_to_p1735(two_bytes);	

	two_bytes = TWOBYTES(pdu->data_field[4], pdu->data_field[3]);
	ambc->ambient_air_temperature = 
		temp_m273_to_p1735(two_bytes);	

	ambc->air_inlet_temperature =
		temp_m40_to_p210(pdu->data_field[5]);	

	two_bytes = TWOBYTES(pdu->data_field[7], pdu->data_field[6]);
	ambc->road_surface_temperature = 
		temp_m273_to_p1735(two_bytes);	
}

void 
print_ambc(void *pdbv, FILE  *fp, int numeric)
{
	j1939_ambc_typ *ambc = (j1939_ambc_typ *)pdbv;
	fprintf(fp, "AMBC ");
	print_timestamp(fp, &ambc->timestamp);
	if (numeric){
		fprintf(fp," %.3f", ambc->barometric_pressure);
		fprintf(fp," %.3f", ambc->cab_interior_temperature);
		fprintf(fp," %.3f", ambc->ambient_air_temperature);
		fprintf(fp," %.3f", ambc->air_inlet_temperature);
		fprintf(fp," %.3f", ambc->road_surface_temperature);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp,"Barometric pressure %.3f\n", 
			ambc->barometric_pressure);
		fprintf(fp,"Cab interior temperature %.3f\n", 
			ambc->cab_interior_temperature);
		fprintf(fp,"Ambient air temperature %.3f\n", 
			ambc->ambient_air_temperature);
		fprintf(fp,"Air inlet temperature %.3f\n", 
			ambc->air_inlet_temperature);
		fprintf(fp,"Road surface temperature %.3f\n", 
			ambc->road_surface_temperature);
	}
}

/** IEC (Inlet/Exhaust Conditions) documented in J1939 - 71, p164 */
void
pdu_to_iec (struct j1939_pdu *pdu, void *pdbv)
{
	j1939_iec_typ *iec = (j1939_iec_typ *)pdbv;
	unsigned short two_bytes;

	iec->particulate_trap_inlet_pressure =
		pressure_0_to_125kpa(pdu->data_field[0]);

	iec->boost_pressure =
		pressure_0_to_500kpa(pdu->data_field[1]);

	iec->intake_manifold_temperature =
		temp_m40_to_p210(pdu->data_field[2]);	

	iec->air_inlet_pressure =
		pressure_0_to_500kpa(pdu->data_field[3]);	

	iec->air_filter_differential_pressure =
		pressure_0_to_12kpa(pdu->data_field[4]);	

	two_bytes = TWOBYTES(pdu->data_field[6], pdu->data_field[5]);
	iec->exhaust_gas_temperature = 
		temp_m273_to_p1735(two_bytes);	

	iec->coolant_filter_differential_pressure =
		pressure_0_to_125kpa(pdu->data_field[7]);	
}

void 
print_iec(void *pdbv, FILE  *fp, int numeric)
{
	j1939_iec_typ *iec = (j1939_iec_typ *)pdbv;
	fprintf(fp, "IEC ");
	print_timestamp(fp, &iec->timestamp);
	if (numeric){
		fprintf(fp," %.3f", iec->particulate_trap_inlet_pressure);
		fprintf(fp," %.3f", iec->boost_pressure);
		fprintf(fp," %.3f", iec->intake_manifold_temperature);
		fprintf(fp," %.3f", iec->air_inlet_pressure);
		fprintf(fp," %.3f", iec->air_filter_differential_pressure);
		fprintf(fp," %.3f", iec->exhaust_gas_temperature);
		fprintf(fp," %.3f",
			 iec->coolant_filter_differential_pressure);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp,"Particulate trap inlet pressure %.3f\n",
			iec->particulate_trap_inlet_pressure);
		fprintf(fp,"Boost pressure %.3f\n",
			iec->boost_pressure);
		fprintf(fp,"Intake manifold temperature %.3f\n",
			iec->intake_manifold_temperature);
		fprintf(fp,"Air inlet pressure %.3f\n",
			iec->air_inlet_pressure);
		fprintf(fp,"Air filter differential pressure %.3f\n",
			iec->air_filter_differential_pressure);
		fprintf(fp,"Exhaust gas temperature %.3f\n",
			iec->exhaust_gas_temperature);
		fprintf(fp,"Coolant filter differential pressure %.3f\n",
			 iec->coolant_filter_differential_pressure);
	}
}

/** VEP (Vehicle Electrical Power) documented in J1939 - 71, p164 */
void
pdu_to_vep (struct j1939_pdu *pdu, void *pdbv)
{
	j1939_vep_typ *vep = (j1939_vep_typ *)pdbv;
	unsigned short two_bytes;

	vep->net_battery_current =
		current_m125_to_p125amp(pdu->data_field[0]);

	vep->alternator_current =
		current_0_to_250amp(pdu->data_field[1]);

	two_bytes = TWOBYTES(pdu->data_field[3], pdu->data_field[2]);
	vep->alternator_potential = voltage(two_bytes);	

	two_bytes = TWOBYTES(pdu->data_field[5], pdu->data_field[4]);
	vep->electrical_potential = voltage(two_bytes);	

	two_bytes = TWOBYTES(pdu->data_field[7], pdu->data_field[6]);
	vep->battery_potential_switched = voltage(two_bytes);	
}

void 
print_vep(void *pdbv, FILE  *fp, int numeric)
{
	j1939_vep_typ *vep = (j1939_vep_typ *)pdbv;
	fprintf(fp, "VEP ");
	print_timestamp(fp, &vep->timestamp);
	if (numeric){
		fprintf(fp," %.3f", vep->net_battery_current);
		fprintf(fp," %.3f", vep->alternator_current);
		fprintf(fp," %.3f", vep->alternator_potential);
		fprintf(fp," %.3f", vep->electrical_potential);
		fprintf(fp," %.3f", vep->battery_potential_switched);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp,"Net battery current %.3f\n",
			vep->net_battery_current);
		fprintf(fp," Alternator current %.3f\n",
			vep->alternator_current);
		fprintf(fp," Alternator potential %.3f\n",
			vep->alternator_potential);
		fprintf(fp," Electrical potential %.3f\n",
			vep->electrical_potential);
		fprintf(fp," Battery potential %.3f\n",
			vep->battery_potential_switched);
	}
}

/** TF (Transmission Fluids) documented in J1939 - 71, p164 */
void
pdu_to_tf (struct j1939_pdu *pdu, void *pdbv)
{
	j1939_tf_typ *tf = (j1939_tf_typ *)pdbv;
	unsigned short two_bytes;

	tf->clutch_pressure =
		pressure_0_to_4000kpa(pdu->data_field[0]);

	tf->transmission_oil_level =
		percent_0_to_100(pdu->data_field[1]);

	tf->transmission_filter_differential_pressure =
		pressure_0_to_500kpa(pdu->data_field[2]);

	tf->transmission_oil_pressure =
		pressure_0_to_4000kpa(pdu->data_field[3]);

	two_bytes = TWOBYTES(pdu->data_field[5], pdu->data_field[4]);
	tf->transmission_oil_temperature = 
		temp_m273_to_p1735(two_bytes);	
}

void 
print_tf(void *pdbv, FILE  *fp, int numeric)
{
	j1939_tf_typ *tf = (j1939_tf_typ *)pdbv;
	fprintf(fp, "TF ");
	print_timestamp(fp, &tf->timestamp);
	if (numeric){
		fprintf(fp," %.3f", tf->clutch_pressure);
		fprintf(fp," %.3f", tf->transmission_oil_level);
		fprintf(fp," %.3f", tf->transmission_filter_differential_pressure);
		fprintf(fp," %.3f", tf->transmission_oil_pressure);
		fprintf(fp," %.3f", tf->transmission_oil_temperature);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp,"Clutch pressure %.3f\n",
			tf->clutch_pressure);
		fprintf(fp,"Transmission oil level %.3f\n",
			tf->transmission_oil_level);
		fprintf(fp,"Filter differential pressure %.3f\n",
			tf->transmission_filter_differential_pressure);
		fprintf(fp,"Transmission oil pressure %.3f\n",
			tf->transmission_oil_pressure);
		fprintf(fp,"Transmission oil temperature %.3f\n",
			tf->transmission_oil_temperature);
	}
}

/** RF (Retarder Fluids) documented in J1939 - 71, p165 */
void
pdu_to_rf (struct j1939_pdu *pdu, void *pdbv)
{
	j1939_rf_typ *rf = (j1939_rf_typ *)pdbv;
	rf->hydraulic_retarder_pressure =
		pressure_0_to_4000kpa(pdu->data_field[0]);

	rf->hydraulic_retarder_oil_temperature = 
		temp_m40_to_p210(pdu->data_field[1]);	
}

void 
print_rf(void *pdbv, FILE  *fp, int numeric)
{
	j1939_rf_typ *rf = (j1939_rf_typ *)pdbv;
	fprintf(fp, "RF ");
	print_timestamp(fp, &rf->timestamp);
	if (numeric){
		fprintf(fp," %.3f", rf->hydraulic_retarder_pressure);
		fprintf(fp," %.3f", rf->hydraulic_retarder_oil_temperature);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp,"Hydraulic retarder pressure %.3f\n",
			rf->hydraulic_retarder_pressure);
		fprintf(fp,"Hydraulic retarder temperature %.3f\n",
			rf->hydraulic_retarder_oil_temperature);
	}
}

/** HRVD (High resolution vehicle distance) documented in J1939 - 71, p170 */
void
pdu_to_hrvd (struct j1939_pdu *pdu, void *pdbv)
{
	j1939_hrvd_typ *hrvd = (j1939_hrvd_typ *)pdbv;
	unsigned int four_bytes;

	four_bytes = FOURBYTES(pdu->data_field[3], pdu->data_field[2],
			pdu->data_field[2], pdu->data_field[0]);

	hrvd->vehicle_distance = hr_distance_in_km(four_bytes);

	four_bytes = FOURBYTES(pdu->data_field[7], pdu->data_field[6],
			pdu->data_field[5], pdu->data_field[4]);

	hrvd->trip_distance = hr_distance_in_km(four_bytes);

}

void 
print_hrvd(void *pdbv, FILE  *fp, int numeric)
{
	j1939_hrvd_typ *hrvd = (j1939_hrvd_typ *)pdbv;
	fprintf(fp, "HRVD ");
	print_timestamp(fp, &hrvd->timestamp);
	if (numeric){
		fprintf(fp," %.3f", hrvd->vehicle_distance);
		fprintf(fp," %.3f", hrvd->trip_distance);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp,"Vehicle distance %.3f\n",
			hrvd->vehicle_distance);
		fprintf(fp,"Trip distance %.3f\n",
			hrvd->trip_distance);
	}
}

/** EBC2 (Electronic Brake Controller 2) documented in J1939 - 71, p171 */
void
pdu_to_ebc2 (struct j1939_pdu *pdu, void *pdbv)
{
	j1939_ebc2_typ *ebc2 = (j1939_ebc2_typ *)pdbv;
	unsigned short two_bytes;

	two_bytes = TWOBYTES(pdu->data_field[1], pdu->data_field[0]);
	ebc2->EBC2_FrontAxleSpeed = wheel_based_mps(two_bytes);
	ebc2->EBC2_RelativeSpeedFrontAxleLeftWheel =
		wheel_based_mps_relative(pdu->data_field[2]);
	ebc2->EBC2_RlativeSpeedFrontAxleRightWheel =
		wheel_based_mps_relative(pdu->data_field[3]);
	ebc2->EBC2_RelativeSpeedRearAxle1LeftWheel =
		wheel_based_mps_relative(pdu->data_field[4]);
	ebc2->EBC2_RlativeSpeedRearAxle1RightWheel =
		wheel_based_mps_relative(pdu->data_field[5]);
	ebc2->EBC2_RelativeSpeedRearAxle2LeftWheel =
		wheel_based_mps_relative(pdu->data_field[6]);
	ebc2->EBC2_RlativeSpeedRearAxle2RightWheel =
		wheel_based_mps_relative(pdu->data_field[7]);
}

void 
print_ebc2(void *pdbv, FILE  *fp, int numeric)

{
	j1939_ebc2_typ *ebc2 = (j1939_ebc2_typ *)pdbv;
	fprintf(fp, "EBC2 ");
	print_timestamp(fp, &ebc2->timestamp);
	if (numeric){
		fprintf(fp," %.3f", ebc2->EBC2_FrontAxleSpeed);
		fprintf(fp," %.3f", ebc2->EBC2_RelativeSpeedFrontAxleLeftWheel);
		fprintf(fp," %.3f", ebc2->EBC2_RlativeSpeedFrontAxleRightWheel);
		fprintf(fp," %.3f", ebc2->EBC2_RelativeSpeedRearAxle1LeftWheel);
		fprintf(fp," %.3f", ebc2->EBC2_RlativeSpeedRearAxle1RightWheel);
		fprintf(fp," %.3f", ebc2->EBC2_RelativeSpeedRearAxle2LeftWheel);
		fprintf(fp," %.3f", ebc2->EBC2_RlativeSpeedRearAxle2RightWheel);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp,"Front axle speed %.3f\n",
			ebc2->EBC2_FrontAxleSpeed);
		fprintf(fp,"Front left wheel relative speed %.3f\n",
			ebc2->EBC2_RelativeSpeedFrontAxleLeftWheel);
		fprintf(fp,"Front right wheel relative speed %.3f\n",
			ebc2->EBC2_RlativeSpeedFrontAxleRightWheel);
		fprintf(fp,"Rear 1 left wheel relative speed %.3f\n",
			ebc2->EBC2_RelativeSpeedRearAxle1LeftWheel);
		fprintf(fp,"Rear 1 left wheel relative speed %.3f\n",
			ebc2->EBC2_RlativeSpeedRearAxle1RightWheel);
		fprintf(fp,"Rear 2 left wheel relative speed %.3f\n",
			ebc2->EBC2_RelativeSpeedRearAxle2LeftWheel);
		fprintf(fp,"Rear 2 left wheel relative speed %.3f\n",
			ebc2->EBC2_RlativeSpeedRearAxle2RightWheel);
	}
}

/** EBC5 (Electronic Brake Controller 5) */
void
pdu_to_ebc5 (struct j1939_pdu *pdu, void *pdbv)
{
	j1939_ebc5_typ *ebc5 = (j1939_ebc5_typ *)pdbv;
	unsigned char byte;

	ebc5->EBC5_HaltBrakeMode = (pdu->data_field[0] >> 2) & (0xFF >> 5);
	ebc5->EBC5_FoundationBrakeUse = BITS21(pdu->data_field[1]);
	ebc5->EBC5_XBRActiveControlMode = HINIBBLE(pdu->data_field[1]);
	byte = pdu->data_field[2];
	ebc5->EBC5_XBRAccelerationLimit = 0.1 * percent_m125_to_p125(byte);
}

void
print_ebc5(void *pdbv, FILE  *fp, int numeric)

{
	j1939_ebc5_typ *ebc5 = (j1939_ebc5_typ *)pdbv;
	fprintf(fp, "EBC5 ");                                 
	print_timestamp(fp, &ebc5->timestamp);                
	if (numeric){                                         
		fprintf(fp," %d", ebc5->EBC5_HaltBrakeMode ); 
		fprintf(fp," %d", ebc5->EBC5_FoundationBrakeUse );
		fprintf(fp," %d", ebc5->EBC5_XBRActiveControlMode );
		fprintf(fp," %.3f", ebc5->EBC5_XBRAccelerationLimit );
		fprintf(fp, "\n");                            
	} else {                                              
		fprintf(fp,"Halt Brake Mode %d\n",            
			ebc5->EBC5_HaltBrakeMode );           
		fprintf(fp,"Foundation Brake Use %d\n",       
			ebc5->EBC5_FoundationBrakeUse);       
		fprintf(fp,"EBC5_XBRActiveControlMode %d\n",  
			ebc5->EBC5_XBRActiveControlMode);     
		fprintf(fp,"EBC5_XBRAccelerationLimit %.3f\n",
			ebc5->EBC5_XBRAccelerationLimit);     
	}                                                     
}                                                             

/** VDC2(Steering Wheel Angle and Acceleration) */
void
pdu_to_vdc2(struct j1939_pdu *pdu, void *pdbv)
{
	j1939_vdc2_typ *vdc2 = (j1939_vdc2_typ *)pdbv;
	unsigned short two_bytes;

	vdc2->VDC2_SteeringWheelTurnCounter = (pdu->data_field[2] & 0x3F) - 32;
	two_bytes = TWOBYTES(pdu->data_field[4], pdu->data_field[3]);
	vdc2->VDC2_YawRate = (two_bytes * 0.000122) - 3.92;
	two_bytes = TWOBYTES(pdu->data_field[6], pdu->data_field[5]);
	vdc2->VDC2_LateralAcceleration = (two_bytes * 0.000488281) - 12.5;
	vdc2->VDC2_LongitudinalAcceleration = (pdu->data_field[7] * 0.1) - 12.5;
}

void 
print_vdc2(void *pdbv, FILE  *fp, int numeric)
{
	j1939_vdc2_typ *vdc2 = (j1939_vdc2_typ *)pdbv;
	fprintf(fp, "VDC2 ");
	print_timestamp(fp, &vdc2->timestamp);
	if (numeric){
		fprintf(fp," %d", vdc2->VDC2_SteeringWheelTurnCounter);
		fprintf(fp," %.3f", vdc2->VDC2_YawRate);
		fprintf(fp," %.3f", vdc2->VDC2_LateralAcceleration);
		fprintf(fp," %.3f", vdc2->VDC2_LongitudinalAcceleration);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp,"Steering Wheel Turn Counter %d\n",
			vdc2->VDC2_SteeringWheelTurnCounter);
		fprintf(fp,"Yaw Rate %.3f\n",
			vdc2->VDC2_YawRate);
		fprintf(fp,"Lateral Acceleration %.3f\n",
			vdc2->VDC2_LateralAcceleration);
		fprintf(fp,"Longitudinal Acceleration %.3f\n",
			vdc2->VDC2_LongitudinalAcceleration);
	}
}
/*
void
pdu_to_vdc2(struct j1939_pdu *pdu, void *pdbv)
{
	j1939_vdc2_typ *vdc2 = (j1939_vdc2_typ *)pdbv;
	unsigned short two_bytes;

	vdc2->VDC2_SteeringWheelTurnCounter = (pdu->data_field[2] & 0x3F) - 32;
	two_bytes = TWOBYTES(pdu->data_field[4], pdu->data_field[3]);
	vdc2->VDC2_YawRate = (two_bytes * 0.000122) - 3.92;
	two_bytes = TWOBYTES(pdu->data_field[6], pdu->data_field[5]);
	vdc2->VDC2_LateralAcceleration = (two_bytes * 0.000488281) - 12.5;
	vdc2->VDC2_LongitudinalAcceleration = (pdu->data_field[7] * 0.1) - 12.5;
	printf("VDC2 Raw %#02hhx %hhd %#02hhx %hhd %#02hhx %hhd %#02hhx %hhd %#02hhx %hhd %#02hhx\n",
		pdu->data_field[2],
		pdu->data_field[3],
		pdu->data_field[3],
		pdu->data_field[4],
		pdu->data_field[4],
		pdu->data_field[5],
		pdu->data_field[5],
		pdu->data_field[6],
		pdu->data_field[6],
		pdu->data_field[7],
		pdu->data_field[7]
		);
}

void 
print_vdc2(void *pdbv, FILE  *fp, int numeric)
{
	j1939_vdc2_typ *vdc2 = (j1939_vdc2_typ *)pdbv;
	fprintf(fp, "VDC2 ");
	print_timestamp(fp, &vdc2->timestamp);
	if (numeric){
		fprintf(fp," %d", vdc2->VDC2_SteeringWheelTurnCounter);
		fprintf(fp," %.3f", vdc2->VDC2_YawRate);
		fprintf(fp," %.3f", vdc2->VDC2_LateralAcceleration);
		fprintf(fp," %.3f", vdc2->VDC2_LongitudinalAcceleration);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp,"Steering Wheel Turn Counter %d\n",
			vdc2->VDC2_SteeringWheelTurnCounter);
		fprintf(fp,"Yaw Rate %.3f\n",
			vdc2->VDC2_YawRate);
		fprintf(fp,"Lateral Acceleration %.3f\n",
			vdc2->VDC2_LateralAcceleration);
		fprintf(fp,"Longitudinal Acceleration %.3f\n",
			vdc2->VDC2_LongitudinalAcceleration);
	}
}
*/
/** PDU VP_X_TGW (Latitude & Longitude) */
void
pdu_to_vp_x(struct j1939_pdu *pdu, void *pdbv)
{
	j1939_vp_x_typ *vp_x = (j1939_vp_x_typ *)pdbv;

	vp_x->VP_X_TGW_Latitude_BB1_X_TGW = 
		( pdu->data_field[0] +
		 (pdu->data_field[1] << 8) +
		 (pdu->data_field[2] << 16) +
		 (pdu->data_field[3] << 24) ) / 1000000.0;

	vp_x->VP_X_TGW_Longitude_BB1_X_TGW = 
		( pdu->data_field[4] +
		 (pdu->data_field[5] << 8) +
		 (pdu->data_field[6] << 16) +
		 (pdu->data_field[7] << 24) ) / 1000000.0;
	printf("VP_X Raw %#02hhx %#02hhx %#02hhx %#02hhx %#02hhx %#02hhx %#02hhx %#02hhx\n",
		pdu->data_field[0],
		pdu->data_field[1],
		pdu->data_field[2],
		pdu->data_field[3],
		pdu->data_field[4],
		pdu->data_field[5],
		pdu->data_field[6],
		pdu->data_field[7]
		);
}

void 
print_vp_x(void *pdbv, FILE  *fp, int numeric)
{
	j1939_vp_x_typ *vp_x = (j1939_vp_x_typ *)pdbv;
	fprintf(fp, "VP_X_TGW ");
	print_timestamp(fp, &vp_x->timestamp);
	if (numeric){
		fprintf(fp," %.3f", vp_x->VP_X_TGW_Latitude_BB1_X_TGW); 
		fprintf(fp," %.3f", vp_x->VP_X_TGW_Longitude_BB1_X_TGW); 
		fprintf(fp, "\n");	
	} else {
		fprintf(fp,"Latitude %.3f\n",
			vp_x->VP_X_TGW_Latitude_BB1_X_TGW); 
		fprintf(fp,"Longitude %.3f\n",
			vp_x->VP_X_TGW_Longitude_BB1_X_TGW); 
	}
}

/** CAN1 */
void
pdu_to_can1(struct j1939_pdu *pdu, void *pdbv)
{
	j1939_can1_typ *can1 = (j1939_can1_typ *)pdbv;

	can1->CAN1_StdData = pdu->data_field[0];
	can1->CAN1_ExtData = pdu->data_field[1];
	can1->CAN1_BusLoad = pdu->data_field[2];
}

void 
print_can1(void *pdbv, FILE  *fp, int numeric)

{
	j1939_can1_typ *can1 = (j1939_can1_typ *)pdbv;
	fprintf(fp, "CAN1 ");
	print_timestamp(fp, &can1->timestamp);
	if (numeric){
		fprintf(fp," %d", can1->CAN1_StdData);
		fprintf(fp," %d", can1->CAN1_ExtData);
		fprintf(fp," %d", can1->CAN1_BusLoad);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp,"CAN1 StdData %d\n",
			can1->CAN1_StdData);
		fprintf(fp,"CAN1 ExtData %d\n",
			can1->CAN1_ExtData);
		fprintf(fp,"CAN1 BusLoad %d\n",
			can1->CAN1_BusLoad);
	}
}

/** CAN2 */
void
pdu_to_can2(struct j1939_pdu *pdu, void *pdbv)
{
	j1939_can2_typ *can2 = (j1939_can2_typ *)pdbv;

	can2->CAN2_BusLoad = pdu->data_field[2];
}

void 
print_can2(void *pdbv, FILE  *fp, int numeric)

{
	j1939_can2_typ *can2 = (j1939_can2_typ *)pdbv;
	fprintf(fp, "CAN2 ");
	print_timestamp(fp, &can2->timestamp);
	if (numeric){
		fprintf(fp," %d", can2->CAN2_BusLoad);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp,"CAN2 BusLoad %d\n",
			can2->CAN2_BusLoad);
	}
}

/** MVS_X_E */
void
pdu_to_mvs_x_e(struct j1939_pdu *pdu, void *pdbv)
{
	j1939_mvs_x_e_typ *mvs_x_e = (j1939_mvs_x_e_typ *)pdbv;

	mvs_x_e->MVS_X_E_AppliedVehicleSpeedLimit_BB1_X_E = pdu->data_field[7];
}

void 
print_mvs_x_e(void *pdbv, FILE  *fp, int numeric)

{
	j1939_mvs_x_e_typ *mvs_x_e = (j1939_mvs_x_e_typ *)pdbv;
	fprintf(fp, "MVS_X_E ");
	print_timestamp(fp, &mvs_x_e->timestamp);
	if (numeric){
		fprintf(fp," %.3f", mvs_x_e->MVS_X_E_AppliedVehicleSpeedLimit_BB1_X_E);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp,"MVS_X_E %.3f\n",
			mvs_x_e->MVS_X_E_AppliedVehicleSpeedLimit_BB1_X_E);
	}
}
/** RCFG (Retarder Configuration) documented in J1939 - 71, p155 */
void
pdu_to_rcfg (struct j1939_pdu *pdu, void *pdbv)
{
	j1939_rcfg_typ *rcfg = (j1939_rcfg_typ *)pdbv;
	unsigned short two_bytes;
	unsigned char data[21];	/* to hold data bytes from 3 packets */
	int i, j;

	for (i = 0; i < 3; i++)
	{
		unsigned char *p = &pdu[i].data_field[0]; 
		for (j = 0; j < 7; j++)
			data[i*7+j] = p[j+1]; /* first byte is sequence no. */
		
	}
	rcfg->retarder_location = HINIBBLE(data[0]);
	rcfg->retarder_type = LONIBBLE(data[0]);
	rcfg->retarder_control_steps = data[1];
	for (i = 0; i < 4; i++){
		two_bytes = TWOBYTES(data[3*i+3], data[3*i+2]);
		rcfg->retarder_speed[i] = speed_in_rpm_2byte(two_bytes);
		rcfg->percent_torque[i] = percent_m125_to_p125(data[3*i + 1]);
	}
	two_bytes = TWOBYTES(data[15], data[14]);
	rcfg->retarder_speed[4] = speed_in_rpm_2byte(two_bytes);
	two_bytes = TWOBYTES(data[17], data[16]);
	rcfg->reference_retarder_torque = torque_in_nm(two_bytes);
	rcfg->percent_torque[4] = percent_m125_to_p125(data[18]);
}

void 
print_rcfg(void *pdbv, FILE  *fp, int numeric)

{
	j1939_rcfg_typ *rcfg = (j1939_rcfg_typ *)pdbv;
	int i;
	fprintf(fp, "RCFG ");
	print_timestamp(fp, &rcfg->timestamp);
	if (numeric){
		fprintf(fp, " %x", rcfg->receive_status);
		fprintf(fp, " %d %d %d", rcfg->retarder_location,
			 rcfg->retarder_type,
			 rcfg->retarder_control_steps);
		for (i = 0; i < 5; i++)
			fprintf(fp, " %7.2f", rcfg->retarder_speed[i]);
		for (i = 0; i < 5; i++)
			fprintf(fp, " %7.2f", rcfg->percent_torque[i]);

		fprintf(fp, " %7.2f", rcfg->reference_retarder_torque);
		fprintf(fp, "\n");
	} else {
		fprintf(fp, "Retarder configuration received mask 0x%x \n",
			 rcfg->receive_status);
		fprintf(fp, "Retarder location 0x%x, type 0x%x, control %d\n",
			 rcfg->retarder_location, rcfg->retarder_type,
			 rcfg->retarder_control_steps);

		fprintf(fp, "Retarder speed ");
		for (i = 0; i < 5; i++)
			fprintf(fp, " %7.2f", rcfg->retarder_speed[i]);
		
		fprintf(fp, "\nPercent torque ");
		for (i = 0; i < 5; i++)
			fprintf(fp, " %7.2f", rcfg->percent_torque[i]);

		fprintf(fp, "\nReference retarder torque %7.2f\n",
			 rcfg->reference_retarder_torque);
	}
}

/** ENGCFG (Engine Configuration) documented in J1939 - 71, p156 */
void
pdu_to_ecfg (struct j1939_pdu *pdu, void *pdbv)
{
	j1939_ecfg_typ *ecfg = (j1939_ecfg_typ *)pdbv;
	unsigned short two_bytes;
	unsigned char data[28];	/* to hold data bytes from 4 packets */
	int i, j;

	for (i = 0; i < 4; i++)
	{
		unsigned char *p = &pdu[i].data_field[0]; 
		for (j = 0; j < 7; j++)
			data[i*7+j] = p[j+1]; /* first byte is sequence no. */

	}
	for (i = 0; i < 5; i++){
		two_bytes = TWOBYTES(data[3*i+1], data[3*i]);
		ecfg->engine_speed[i] = speed_in_rpm_2byte(two_bytes);
		ecfg->percent_torque[i] = percent_m125_to_p125(data[3*i + 2]);
	}

	two_bytes = TWOBYTES(data[16], data[15]);
	ecfg->engine_speed[5] = speed_in_rpm_2byte(two_bytes);
	two_bytes = TWOBYTES(data[18], data[17]);
	ecfg->gain_endspeed_governor = gain_in_kp(two_bytes);
	two_bytes = TWOBYTES(data[20], data[19]);
	ecfg->reference_engine_torque = torque_in_nm(two_bytes);
	two_bytes = TWOBYTES(data[22], data[21]);
	ecfg->engine_speed[6] = speed_in_rpm_2byte(two_bytes);
        ecfg->max_momentary_overide_time = time_0_to_25sec(data[23]); 
        ecfg->speed_control_lower_limit = speed_in_rpm_1byte(data[24]); 
        ecfg->speed_control_upper_limit = speed_in_rpm_1byte(data[25]); 
        ecfg->torque_control_lower_limit = percent_m125_to_p125(data[26]); 
        ecfg->torque_control_upper_limit = percent_m125_to_p125(data[27]); 
}

void 
print_ecfg(void *pdbv, FILE  *fp, int numeric)

{
	j1939_ecfg_typ *ecfg = (j1939_ecfg_typ *)pdbv;
	int i;
	fprintf(fp, "ECFG ");
	print_timestamp(fp, &ecfg->timestamp);
	if (numeric){
		fprintf(fp, " %x ", ecfg->receive_status);
		for (i = 0; i < 7; i++)
			fprintf(fp, " %7.2f", ecfg->engine_speed[i]);
		for (i = 0; i < 5; i++)
			fprintf(fp, " %7.2f", ecfg->percent_torque[i]);
		fprintf(fp, " %7.2f", ecfg->gain_endspeed_governor);
		fprintf(fp, " %7.2f", ecfg->reference_engine_torque);
		fprintf(fp, " %7.2f", ecfg->max_momentary_overide_time);
		fprintf(fp, " %7.2f", ecfg->speed_control_lower_limit);
		fprintf(fp, " %7.2f", ecfg->speed_control_upper_limit);
		fprintf(fp, " %7.2f", ecfg->torque_control_lower_limit);
		fprintf(fp, " %7.2f", ecfg->torque_control_upper_limit);
		fprintf(fp, "\n");
	} else {
		fprintf(fp, "Engine configuration received mask 0x%x \n", ecfg->receive_status);
		fprintf(fp, "Engine speed ");
		for (i = 0; i < 7; i++)
			fprintf(fp, " %7.2f", ecfg->engine_speed[i]);
		
		fprintf(fp, "\nPercent torque ");
		for (i = 0; i < 5; i++)
			fprintf(fp, " %7.2f", ecfg->percent_torque[i]);
		fprintf(fp, "\nGain endspeed governor %7.2f\n", ecfg->gain_endspeed_governor);
		fprintf(fp, "Reference engine torque %7.2f\n", ecfg->reference_engine_torque);
		fprintf(fp, "Max Momentary Override Time %7.2f\n", 
			ecfg->max_momentary_overide_time);
		fprintf(fp, "Speed Control Lower Limit %7.2f\n", ecfg->speed_control_lower_limit);
		fprintf(fp, "Speed Control Upper Limit %7.2f\n", ecfg->speed_control_upper_limit);
		fprintf(fp, "Torque Control Lower Limit %7.2f\n",
				 ecfg->torque_control_lower_limit);
		fprintf(fp, "Torque Control Upper Limit %7.2f\n",
				 ecfg->torque_control_upper_limit);
	}
}

/** TSC (Torque/Speed Control Command) documented in J1939 - 71 
 * Since this is a command, conversion routine translates database
 * variable to PDU for transmission.
 */
 
void tsc1_to_pdu (struct j1939_pdu *pdu, void * pdbv)
{
	j1939_tsc1_typ *tsc1 = (j1939_tsc1_typ  *)pdbv;
	int i;
	int requested_speed;

        pdu->priority = 3; /* high PDU priority */
        pdu->R = 0;
        pdu->DP = 0;
        pdu->pdu_format = 0;	
        pdu->pdu_specific = tsc1->destination_address;

	/* pretend to be the transmission */
        pdu->src_address = tsc1->src_address; 

        pdu->numbytes = 8;
        pdu->data_field[0] = 
			BITS21(tsc1->EnOvrdCtrlM) |
			BITS21(tsc1->EnRSpdCtrlC) << 2 |
			BITS21(tsc1->EnOvrdCtrlMPr) << 4;
	requested_speed = code_engine_speed(tsc1->EnRSpdSpdLm);
        pdu->data_field[1] = LOBYTE(requested_speed);
        pdu->data_field[2] = HIBYTE(requested_speed);
	pdu->data_field[3] =
		code_percent_m125_to_p125(tsc1->EnRTrqTrqLm);
        for (i = 4; i < 8; i++)
                pdu->data_field[i] = 0xff;
}

/** reverse also needed for logging to database */
void
pdu_to_tsc1 (struct j1939_pdu *pdu, void *pdbv)
{

	j1939_tsc1_typ *tsc1 = (j1939_tsc1_typ *)pdbv;
	short data;
	tsc1->src_address = pdu->src_address;
        tsc1->EnOvrdCtrlM = BITS21(pdu->data_field[0]);
        tsc1->EnRSpdCtrlC = BITS43(pdu->data_field[0]);
        tsc1->EnOvrdCtrlMPr = BITS65(pdu->data_field[0]);
       	data = TWOBYTES(pdu->data_field[2], pdu->data_field[1]);
        tsc1->EnRSpdSpdLm = speed_in_rpm_2byte(data);
        tsc1->EnRTrqTrqLm = percent_m125_to_p125(pdu->data_field[3]);
        tsc1->destination_address = pdu->pdu_specific; /* engine or retarder */
}

void 
print_tsc1(void *pdbv, FILE  *fp, int numeric)
{
	j1939_tsc1_typ *tsc1 = (j1939_tsc1_typ *)pdbv;
	fprintf(fp, "TSC1_E_T ");
	print_timestamp(fp, &tsc1->timestamp);
	if (numeric) {
		fprintf(fp, "%d ", tsc1->destination_address);
		fprintf(fp, "%d ", tsc1->src_address);
		fprintf(fp, "%d ", tsc1->EnOvrdCtrlMPr);
		fprintf(fp, "%d ", tsc1->EnRSpdCtrlC);
		fprintf(fp, "%d ", tsc1->EnOvrdCtrlM);
		fprintf(fp, "%.3f ", tsc1->EnRSpdSpdLm);
		fprintf(fp, "%.3f ", tsc1->EnRTrqTrqLm);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp, "Destination %d \n", tsc1->destination_address);
		fprintf(fp, "Source address %d\n", tsc1->src_address);
		fprintf(fp, "Override Control Mode priority %d\n",
			 tsc1->EnOvrdCtrlMPr);
		fprintf(fp, "Requested speed control conditions %d\n",
			 tsc1->EnRSpdCtrlC);
		fprintf(fp, "Override control mode %d\n",
			 tsc1->EnOvrdCtrlM);
		fprintf(fp, "Requested speed/speed limit  %.3f\n",
			 tsc1->EnRSpdSpdLm);
		fprintf(fp, "Requested torque/torque limit %.3f\n",
			 tsc1->EnRTrqTrqLm);
	}
}

/**FD (Fan Drive) J1939-71, sec 5.3.58 */
void
pdu_to_fd (struct j1939_pdu *pdu, void *pdbv)
{
	j1939_fd_typ *fd = (j1939_fd_typ *)pdbv;
        fd->estimated_percent_fan_speed = percent_0_to_100(pdu->data_field[0]);
        fd->fan_drive_state = LONIBBLE(pdu->data_field[1]);
}

void 
print_fd(void *pdbv, FILE  *fp, int numeric)

{
	j1939_fd_typ *fd = (j1939_fd_typ *)pdbv;
	fprintf(fp, "FD ");
	print_timestamp(fp, &fd->timestamp);
	if (numeric) {
		fprintf(fp, "%.3f ", fd->estimated_percent_fan_speed);
		fprintf(fp, "%d ", fd->fan_drive_state);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp, "Estimated percent fan speed %.3f\n",
			 fd->estimated_percent_fan_speed);
		fprintf(fp, "Fan drive state %d\n",
			 fd->fan_drive_state);
	}
}


/** EXAC (External Acceleration Command) WABCO proprietary
 * Since this is a command, conversion routine translates database
 * variable to PDU for transmission.
 
 
void exac_to_pdu (struct j1939_pdu *pdu, void *pdbv) 
{
	j1939_exac_typ *exac = (j1939_exac_typ *) pdbv;
	short requested_deceleration;

        pdu->priority = 3;  high PDU priority 
        pdu->R = 0;
        pdu->DP = 0;
        pdu->pdu_format = 0;	
        pdu->pdu_specific = J1939_ADDR_BRAKE;

	pretend to be adaptive cruise control
        pdu->src_address = exac->src_address; 

        pdu->numbytes = 8;
        pdu->data_field[0] = 
			0xf0 |		 bits 5-8 undefined 
			BITS21(exac->ebs_override_control_mode_priority)  << 2 |
			BITS21(exac->external_deceleration_control_mode); 
	requested_deceleration = deceleration_to_short(
					exac->requested_deceleration_to_ebs);

#ifdef DEBUG_BRAKE
	printf("requested_deceleration %d (0x%2x)\n",
		requested_deceleration, requested_deceleration);
#endif
        pdu->data_field[1] = LOBYTE(requested_deceleration);
        pdu->data_field[2] = HIBYTE(requested_deceleration);
        pdu->data_field[3] = 
			0xf0 |		 bits 5-8 undefined 
			BITS21(exac->edc_override_control_mode_priority) << 2 |
			BITS21(exac->override_control_modes); 
        pdu->data_field[4] =
		code_percent_m125_to_p125(exac->requested_torque_to_edc);
        pdu->data_field[5] = 
			LONIBBLE(exac->alive_signal) << 4 |
			LONIBBLE(exac->acc_internal_status); 
	pdu->data_field[6] = exac->undefined;

	pdu->data_field[7] = pdu->data_field[0] + pdu->data_field[1] +
			pdu->data_field[2] + pdu->data_field[3] +
			pdu->data_field[4] + pdu->data_field[5] +
			pdu->data_field[6] + 1;

}
*/

/** VOLVO XBR (External Brake Command) VOLVO proprietary
 * Since this is a command, conversion routine translates database
 * variable to PDU for transmission.
 */
 
void volvo_xbr_to_pdu (struct j1939_pdu *pdu, void *pdbv) 
{
	j1939_volvo_xbr_typ *volvo_xbr = (j1939_volvo_xbr_typ *) pdbv;
	unsigned short requested_deceleration;
	unsigned int checksum;
	unsigned const int id_sum = 0x0C + 0xEF + 0x0B + 0x2A;
	unsigned const int customer_key = 5;
	static unsigned char counter = 0;
	int i;

        pdu->priority = 3; /* high PDU priority */
        pdu->R = 0;
        pdu->DP = 0;
        pdu->pdu_format = 0xef;	
        pdu->pdu_specific = J1939_ADDR_BRAKE;
	if(++counter >= 16)
		counter = 0;

	/* pretend to be adaptive cruise control */
        pdu->src_address = 0x2A; 
//	volvo_xbr->XBRPriority = 3;


        pdu->numbytes = 8;
        pdu->data_field[2] = 
			0xc0 |		/* bits 7-8 undefined */
			BITS21(volvo_xbr->XBREBIMode) |
			(BITS21(volvo_xbr->XBRPriority)  << 2) |
			(BITS21(volvo_xbr->XBRControlMode)  << 4);
	if(volvo_xbr->XBRControlMode == 0)
		volvo_xbr->ExternalAccelerationDemand = 10;
	requested_deceleration = ((unsigned short)((volvo_xbr->ExternalAccelerationDemand + 15.687) / 0.0004882812)) & 0xFFFF;
//	0x557F = -5.00
#ifdef DEBUG_BRAKE
	printf("requested_deceleration %.3f m/s/s = (%#0hx)\n",
		volvo_xbr->ExternalAccelerationDemand, requested_deceleration);
#endif
        pdu->data_field[0] = LOBYTE(requested_deceleration);
        pdu->data_field[1] = HIBYTE(requested_deceleration);
        pdu->data_field[3] = volvo_xbr->XBRUrgency; 
        pdu->data_field[3] = 100; 
        pdu->data_field[4] = volvo_xbr->spare1;
        pdu->data_field[5] = volvo_xbr->spare2;
	pdu->data_field[6] = volvo_xbr->spare3;

printf("Calling volvo_xbr_to_pdu: mode %d msg ctr %#hhx id_sum %#x cust_key %#x ",
	volvo_xbr->XBRControlMode,
	counter,
	id_sum,
	customer_key
);
	checksum = pdu->data_field[0] + pdu->data_field[1] +
			pdu->data_field[2] + pdu->data_field[3] +
			pdu->data_field[4] + pdu->data_field[5] +
			pdu->data_field[6] + 
			counter +
			id_sum + customer_key;

printf("CS1 %#x ", checksum);
	checksum &= 0xFF;
printf("CS2 %#x ", checksum);
	checksum = (((checksum & 0xF0) >> 4) + (checksum & 0x0F)) & 0x0F;
printf("CS3 %#x msg ctr %#hhx ", checksum, counter);
	pdu->data_field[7] = ((checksum << 4) & 0xF0) + counter;

for(i=0; i<8; i++)
	printf("D[%d] %#hhx ", i, pdu->data_field[i]);
printf("\n");
}

void volvo_xbr_warn_to_pdu (struct j1939_pdu *pdu, void *pdbv) 
{
	j1939_volvo_xbr_warn_typ *volvo_xbr_warn = (j1939_volvo_xbr_warn_typ *) pdbv;
	int i;
//CFF10FE is wrong
        pdu->priority = 6; /* low PDU priority */
        pdu->R = 0;
        pdu->DP = 0;
        pdu->pdu_format = 0xff;	
        pdu->pdu_specific = 0x10;
        pdu->src_address = (0x2A & 0xFF); 

        pdu->numbytes = 8;

        pdu->data_field[0] = 0xFF;
        pdu->data_field[1] = 0xFF;
        pdu->data_field[2] = 0xFF;
        pdu->data_field[3] = 0xFF;
        pdu->data_field[4] = 0xFF;
        pdu->data_field[5] = 0xFF;
        pdu->data_field[6] = 0x31;
        pdu->data_field[7] = 0xFF;
printf("Calling volvo_xbr_warn_to_pdu: ");
for(i=0; i<8; i++)
	printf("%#hhx ", pdu->data_field[i]);
printf("\n");
}
/** reverse also needed for logging to database */
void
pdu_to_exac (struct j1939_pdu *pdu, void *pdbv)
{
	j1939_exac_typ *exac = (j1939_exac_typ *)pdbv;
	exac->ebs_override_control_mode_priority = BITS43(pdu->data_field[0]);
	exac->external_deceleration_control_mode = BITS21(pdu->data_field[0]);
	exac->requested_deceleration_to_ebs = short_to_deceleration(
			TWOBYTES(pdu->data_field[2], pdu->data_field[1]));
	exac->edc_override_control_mode_priority = BITS43(pdu->data_field[3]);
	exac->override_control_modes = BITS21(pdu->data_field[3]);
	exac->requested_torque_to_edc = percent_m125_to_p125(pdu->data_field[4]);
	exac->alive_signal = HINIBBLE(pdu->data_field[5]);
	exac->acc_internal_status = LONIBBLE(pdu->data_field[5]);
	exac->undefined = pdu->data_field[6];
	exac->checksum = pdu->data_field[7];
}


/** Volvo brake data */
void
pdu_to_volvo_xbr(struct j1939_pdu *pdu, void *pdbv)
{

	j1939_volvo_xbr_typ *volvo_xbr= (j1939_volvo_xbr_typ *)pdbv;
	unsigned short data;
	unsigned int byte;

       	data = TWOBYTES(pdu->data_field[1], pdu->data_field[0]);
	volvo_xbr->ExternalAccelerationDemand = (data * 0.000488281) - 15.687;
	byte = (unsigned int) pdu->data_field[2];
	volvo_xbr->XBREBIMode = BITS21(byte);
	volvo_xbr->XBRPriority = BITS43(byte);
	volvo_xbr->XBRControlMode = BITS65(byte);
	volvo_xbr->XBRUrgency = pdu->data_field[3];
	volvo_xbr->spare1 = pdu->data_field[4];
	volvo_xbr->spare2 = pdu->data_field[5];
	volvo_xbr->spare3 = pdu->data_field[6];
	volvo_xbr->XBRMessageCounter = LONIBBLE(pdu->data_field[7]);
	volvo_xbr->XBRMessageChecksum = HINIBBLE(pdu->data_field[7]) >> 4;
}

/** Volvo brake warning */
void
pdu_to_volvo_xbr_warn(struct j1939_pdu *pdu, void *pdbv)
{

	j1939_volvo_xbr_warn_typ *volvo_xbr_warn = (j1939_volvo_xbr_warn_typ *)pdbv;

	volvo_xbr_warn->byte1= pdu->data_field[0];
	volvo_xbr_warn->byte2= pdu->data_field[1];
	volvo_xbr_warn->byte3= pdu->data_field[2];
	volvo_xbr_warn->byte4= pdu->data_field[3];
	volvo_xbr_warn->byte5= pdu->data_field[4];
	volvo_xbr_warn->byte6= pdu->data_field[5];
	volvo_xbr_warn->byte7= pdu->data_field[6];
	volvo_xbr_warn->byte8= pdu->data_field[7];
}

void 
print_volvo_xbr_warn(void *pdbv, FILE  *fp, int numeric)
{
	j1939_volvo_xbr_warn_typ *volvo_xbr_warn = (j1939_volvo_xbr_warn_typ *)pdbv;
	fprintf(fp, "VOLVO XBR WARNING ");
	print_timestamp(fp, &volvo_xbr_warn->timestamp);
	fprintf(fp, "%hhx ", volvo_xbr_warn->byte8);
	fprintf(fp, "%hhx ", volvo_xbr_warn->byte7);
	fprintf(fp, "%hhx ", volvo_xbr_warn->byte6);
	fprintf(fp, "%hhx ", volvo_xbr_warn->byte5);
	fprintf(fp, "%hhx ", volvo_xbr_warn->byte4);
	fprintf(fp, "%hhx ", volvo_xbr_warn->byte3);
	fprintf(fp, "%hhx ", volvo_xbr_warn->byte2);
	fprintf(fp, "%hhx ", volvo_xbr_warn->byte1);
	fprintf(fp,  "\n");	
}

void 
print_volvo_xbr(void *pdbv, FILE  *fp, int numeric)
{
	j1939_volvo_xbr_typ *volvo_xbr = (j1939_volvo_xbr_typ *)pdbv;
	fprintf(fp, "VOLVO XBR ");
	print_timestamp(fp, &volvo_xbr->timestamp);
	if (numeric) {
		fprintf(fp, "%.3f ", volvo_xbr->ExternalAccelerationDemand);
		fprintf(fp, "%d ", volvo_xbr->XBREBIMode);
		fprintf(fp, "%d ", volvo_xbr->XBRPriority);
		fprintf(fp, "%d ", volvo_xbr->XBRControlMode);
		fprintf(fp, "%d ", volvo_xbr->XBRUrgency);
		fprintf(fp, "%d ", volvo_xbr->spare1);
		fprintf(fp, "%d ", volvo_xbr->spare2);
		fprintf(fp, "%d ", volvo_xbr->spare3);
		fprintf(fp, "%d ", volvo_xbr->XBRMessageCounter);
		fprintf(fp, "%d ", volvo_xbr->XBRMessageChecksum);
		fprintf(fp,  "\n");	
	} else {
		fprintf(fp, "XBR External Acceleration Demand %.3f\n",
			volvo_xbr->ExternalAccelerationDemand);
		fprintf(fp, "XBR EBI Mode %d\n",
			volvo_xbr->XBREBIMode);
		fprintf(fp, "XBR Priority %d\n",
			volvo_xbr->XBRPriority);
		fprintf(fp, "XBR Control Mode %d\n",
			volvo_xbr->XBRControlMode);
		fprintf(fp, "XBR Urgency %d\n",
			volvo_xbr->XBRUrgency);
		fprintf(fp, "XBR Message Counter %d\n",
			volvo_xbr->XBRMessageCounter);
		fprintf(fp, "Undefined %d %d %d", volvo_xbr->spare1, volvo_xbr->spare2, volvo_xbr->spare3);
		fprintf(fp, "Checksum %d ", volvo_xbr->XBRMessageChecksum);
		fprintf(fp,  "\n");	
	}
}

void 
print_exac(void *pdbv, FILE  *fp, int numeric)

{
	j1939_exac_typ *exac = (j1939_exac_typ *)pdbv;
	fprintf(fp, "EXAC ");
	print_timestamp(fp, &exac->timestamp);
	if (numeric) {
		fprintf(fp, "%d ", exac->ebs_override_control_mode_priority);
		fprintf(fp, "%d ", exac->external_deceleration_control_mode );
		fprintf(fp, "%.3f ", exac->requested_deceleration_to_ebs );
		fprintf(fp, "%d ", exac->edc_override_control_mode_priority );
		fprintf(fp, "%d ", exac->override_control_modes );
		fprintf(fp, "%.3f ", exac->requested_torque_to_edc );
		fprintf(fp, "%d ", exac->alive_signal );
		fprintf(fp, "%d ", exac->acc_internal_status );
		fprintf(fp, "%d ", exac->undefined );
		fprintf(fp, "%d ", exac->checksum );
		fprintf(fp,  "\n");	
	} else {
		fprintf(fp, "EBS override control mode priority %d\n",
			exac->ebs_override_control_mode_priority);
		fprintf(fp, "External deceleration control mode %d \n",
			exac->external_deceleration_control_mode);
		fprintf(fp, "Requested deceleration to EBS %.3f \n",
			exac->requested_deceleration_to_ebs);
		fprintf(fp, "EDC override control mode priority %d \n",
			exac->edc_override_control_mode_priority);
		fprintf(fp, "Override control mode %d \n",
			exac->override_control_modes);
		fprintf(fp, "Requested torque to EDC %.3f \n",
			exac->requested_torque_to_edc);
		fprintf(fp, "Alive signal %d \n",
			exac->alive_signal);
		fprintf(fp, "ACC internal status %d \n",
			exac->acc_internal_status );
		fprintf(fp, "Undefined %d ", exac->undefined );
		fprintf(fp, "Checksum %d ", exac->checksum );
		fprintf(fp,  "\n");	
	}
}

/** Volvo target data */
void
pdu_to_volvo_target(struct j1939_pdu *pdu, void *pdbv)
{

	j1939_volvo_target_typ *volvo_target = (j1939_volvo_target_typ *)pdbv;
	unsigned short data;
       	data = TWOBYTES(pdu->data_field[1], pdu->data_field[0]);
	volvo_target->TargetDist = data * 0.01;
       	data = TWOBYTES(pdu->data_field[3], pdu->data_field[2]);
	volvo_target->TargetVel = data * 0.01;
       	data = TWOBYTES(pdu->data_field[5], pdu->data_field[4]);
	volvo_target->TargetAcc = (data * 0.01) - 327.68;
	volvo_target->TargetAvailable = pdu->data_field[6] & 1;
}

/** Volvo self data */
void
pdu_to_volvo_ego(struct j1939_pdu *pdu, void *pdbv)
{

	j1939_volvo_ego_typ *volvo_ego = (j1939_volvo_ego_typ *)pdbv;
	unsigned short data;
       	data = TWOBYTES(pdu->data_field[1], pdu->data_field[0]);
	volvo_ego->EgoVel = data * 0.01;
       	data = TWOBYTES(pdu->data_field[3], pdu->data_field[2]);
	volvo_ego->EgoAcc = (data * 0.01) - 327.68;
}

void 
print_volvo_target(void *pdbv, FILE  *fp, int numeric)
{
	j1939_volvo_target_typ *volvo_target = (j1939_volvo_target_typ *)pdbv;
	fprintf(fp, "VOLVO_TARGET: ");
	print_timestamp(fp, &volvo_target->timestamp);
	if (numeric) {
		fprintf(fp, "%.3f ", volvo_target->TargetDist);
		fprintf(fp, "%.3f ", volvo_target->TargetVel);
		fprintf(fp, "%.3f ", volvo_target->TargetAcc);
		fprintf(fp, "%d ", volvo_target->TargetAvailable);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp, "Volvo target available %d\n",
			 volvo_target->TargetAvailable);
		fprintf(fp, "Volvo target distance %.3f\n",
			 volvo_target->TargetDist);
		fprintf(fp, "Volvo target velocity %.3f\n",
			 volvo_target->TargetVel);
		fprintf(fp, "Volvo target acceleration %.3f\n",
			 volvo_target->TargetVel);
	}
}

void 
print_volvo_ego(void *pdbv, FILE  *fp, int numeric)
{
	j1939_volvo_ego_typ *volvo_ego = (j1939_volvo_ego_typ *)pdbv;
	fprintf(fp, "VOLVO_EGO: ");
	print_timestamp(fp, &volvo_ego->timestamp);
	if (numeric) {
		fprintf(fp, "%.3f ", volvo_ego->EgoVel);
		fprintf(fp, "%.3f ", volvo_ego->EgoAcc);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp, "Volvo ego velocity %.3f\n",
			 volvo_ego->EgoVel);
		fprintf(fp, "Volvo ego acceleration %.3f\n",
			 volvo_ego->EgoVel);
	}
}


void
pdu_to_ebc_acc (struct j1939_pdu *pdu, void *pdbv)
{
	j1939_ebc_acc_typ *ebc_acc = (j1939_ebc_acc_typ *)pdbv;
	ebc_acc->vehicle_mass = mass_0_to_100t(pdu->data_field[0]);
	ebc_acc->road_slope = percent_m25_to_p25(pdu->data_field[1]);
}

void 
print_ebc_acc(void *pdbv, FILE  *fp, int numeric)

{
	j1939_ebc_acc_typ *ebc_acc = (j1939_ebc_acc_typ *)pdbv;
	fprintf(fp, "EBC_ACC ");
	print_timestamp(fp, &ebc_acc->timestamp);
	if (numeric) {
		fprintf(fp, " %.3f\n", ebc_acc->vehicle_mass);
		fprintf(fp, " %.3f\n", ebc_acc->road_slope);
	} else {
		fprintf(fp, "Vehicle mass %.3f\n",
			 ebc_acc->vehicle_mass);
		fprintf(fp, "Road slope %.3f\n",
			 ebc_acc->road_slope);
	}
}

void
pdu_to_gfi2 (struct j1939_pdu *pdu, void *pdbv)
{
	j1939_gfi2_typ *gfi2 = (j1939_gfi2_typ *)pdbv;
	gfi2->fuel_valve1_position = pdu->data_field[2];
}

void 
print_gfi2(void *pdbv, FILE  *fp, int numeric)

{
	j1939_gfi2_typ *gfi2 = (j1939_gfi2_typ *)pdbv;
	fprintf(fp, "GFI2 ");
	print_timestamp(fp, &gfi2->timestamp);
	if (numeric)
		fprintf(fp, " %.3f\n", gfi2->fuel_valve1_position);
	else
		fprintf(fp, "Fuel valve 1 position %.3f\n",
			 gfi2->fuel_valve1_position);
}

void
pdu_to_ei (struct j1939_pdu *pdu, void *pdbv)
{
	j1939_ei_typ *ei = (j1939_ei_typ *)pdbv;
	short data;
	ei->pre_filter_oil_pressure = pressure_0_to_1000kpa(pdu->data_field[0]);
	data = TWOBYTES(pdu->data_field[2], pdu->data_field[1]);
	ei->exhaust_gas_pressure = pressure_m250_to_p252kpa(data);
	ei->rack_position = percent_0_to_100(pdu->data_field[3]);
	data = TWOBYTES(pdu->data_field[5], pdu->data_field[4]);
	ei->natural_gas_mass_flow = mass_flow(data);
	data = TWOBYTES(pdu->data_field[7], pdu->data_field[6]);
	ei->instantaneous_estimated_brake_power = power_in_kw(data);
}

void 
print_ei(void *pdbv, FILE  *fp, int numeric)

{
	j1939_ei_typ *ei = (j1939_ei_typ *)pdbv;
	fprintf(fp, "EI ");
	print_timestamp(fp, &ei->timestamp);
	if (numeric){
		fprintf(fp, " %.3f\n", ei->pre_filter_oil_pressure);
		fprintf(fp, " %.3f\n", ei->exhaust_gas_pressure);
		fprintf(fp, " %.3f\n", ei->rack_position);
		fprintf(fp, " %.3f\n", ei->natural_gas_mass_flow);
		fprintf(fp, " %.3f\n", ei->instantaneous_estimated_brake_power);
	} else {
		fprintf(fp, "Pre-filter oil pressure %.3f\n",
			 ei->pre_filter_oil_pressure);
		fprintf(fp, "Exhaust gas pressure %.3f\n",
			 ei->exhaust_gas_pressure);
		fprintf(fp, "Rack position %.3f\n",
			 ei->rack_position);
		fprintf(fp, "Natural gas mass flow %.3f\n",
			 ei->natural_gas_mass_flow);
		fprintf(fp, "Instantaneous estimated brake power %.3f\n",
			 ei->instantaneous_estimated_brake_power);
	}
}
