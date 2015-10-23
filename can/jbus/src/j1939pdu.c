/**\file
 * Translates J1939 PDU structures into database variables.  
 * Page numbers refer to standalone SAE J1939 documents, 
 * are wrong for the "purple book" that has all bound together.
 *
 * Copyright (c) 2005 Regents of the University of California
 *
 */
#include "std_jbus.h"

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
	erc1->enable_shift_assist_status = BITS87(byte);
	erc1->enable_brake_assist_status = BITS65(byte);
	erc1->engine_retarder_torque_mode = LONIBBLE(byte);
	erc1->actual_retarder_percent_torque = 
		percent_m125_to_p125(pdu->data_field[1]);
	erc1->intended_retarder_percent_torque = 
		percent_m125_to_p125(pdu->data_field[2]);
	erc1->coolant_load_increase = BITS21(pdu->data_field[3]);
	erc1->source_address = pdu->data_field[4];
}

void 
print_erc1(void *pdbv , FILE  *fp, int numeric)

{
	j1939_erc1_typ *erc1 = (j1939_erc1_typ *)pdbv;
	fprintf(fp, "ERC1 ");
	print_timestamp(fp, &erc1->timestamp);
	if (numeric) {
		fprintf(fp, "%d ", erc1->enable_shift_assist_status);
		fprintf(fp, "%d ", erc1->enable_brake_assist_status);
		fprintf(fp, "%d ", erc1->engine_retarder_torque_mode);
		fprintf(fp, "%.2f ", erc1->actual_retarder_percent_torque);
		fprintf(fp, "%.2f ", erc1->intended_retarder_percent_torque);
		fprintf(fp, "%d ", erc1->coolant_load_increase);
	 	fprintf(fp, "%d ", erc1->source_address);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp, "Enable shift assist status %d\n",
			 erc1->enable_shift_assist_status);
		fprintf(fp, "Enable brake assist status %d\n",
			 erc1->enable_brake_assist_status);
		fprintf(fp, "Engine retarder torque mode %d\n",
			 erc1->engine_retarder_torque_mode);
		fprintf(fp, "Actual retarder percent torque %.2f\n",
			 erc1->actual_retarder_percent_torque);
		fprintf(fp, "Intended retarder percent torque %.2f\n",
			 erc1->intended_retarder_percent_torque);
		fprintf(fp, "Coolant load increase %d\n",
			 erc1->coolant_load_increase);
		fprintf(fp, "Source address %d (0x%0x)\n", erc1->source_address,
			erc1->source_address);
	}
}

/** EBC1 (Electronic Brake Controller #1) documented in J1939 - 71, p 151 */
void
pdu_to_ebc1 (struct j1939_pdu *pdu, void *pdbv)
{
	j1939_ebc1_typ *ebc1 = (j1939_ebc1_typ *)pdbv;
	unsigned char byte;

	byte = (unsigned int) pdu->data_field[0];
	ebc1->ebs_brake_switch_status = BITS87(byte);
	ebc1->abs_active_status = BITS65(byte);
	ebc1->asr_brake_control_active_status = BITS43(byte);
	ebc1->asr_engine_control_active_status = BITS21(byte);

	ebc1->brake_pedal_position = 
		percent_0_to_100(pdu->data_field[1]);

	byte = (unsigned int) pdu->data_field[2];
	ebc1->traction_control_override_switch_status = BITS87(byte);
	ebc1->asr_hill_holder_switch_status = BITS65(byte);
	ebc1->asr_off_road_switch_status = BITS43(byte);
	ebc1->abs_off_road_switch_status = BITS21(byte);

	byte = (unsigned int) pdu->data_field[3];
	ebc1->remote_accelerator_enable_switch_status = BITS87(byte);
	ebc1->auxiliary_engine_shutdown_switch_status = BITS65(byte);
	ebc1->engine_derate_switch_status = BITS43(byte);
	ebc1->accelerator_interlock_switch_status = BITS21(byte);

	ebc1->percent_engine_retarder_torque_selected = 
		percent_0_to_100(pdu->data_field[4]);

	byte = (unsigned int) pdu->data_field[5];
	ebc1->abs_ebs_amber_warning_state = BITS65(byte);
	ebc1->ebs_red_warning_state = BITS43(byte);
	ebc1->abs_fully_operational = BITS21(byte);

	ebc1->source_address = pdu->data_field[6];
	ebc1->total_brake_demand = brake_demand(pdu->data_field[7]);
}

void 
print_ebc1(void *pdbv, FILE  *fp, int numeric)

{
	j1939_ebc1_typ *ebc1 = (j1939_ebc1_typ *)pdbv;
	fprintf(fp, "EBC1 ");
	print_timestamp(fp, &ebc1->timestamp);
	if (numeric) {
		fprintf(fp, "%d ", ebc1->ebs_brake_switch_status);
		fprintf(fp, "%d ", ebc1->abs_active_status);
		fprintf(fp, "%d ", ebc1->asr_brake_control_active_status);
		fprintf(fp, "%d ", ebc1->asr_engine_control_active_status);
		fprintf(fp, "%.2f ", ebc1->brake_pedal_position);
		fprintf(fp, "%d ",
			 ebc1->traction_control_override_switch_status);
		fprintf(fp, "%d ", ebc1->asr_hill_holder_switch_status);
		fprintf(fp, "%d ", ebc1->asr_off_road_switch_status);
		fprintf(fp, "%d ", ebc1->abs_off_road_switch_status);
		fprintf(fp, "%d ",
			 ebc1->remote_accelerator_enable_switch_status);
		fprintf(fp, "%d ",
			 ebc1->auxiliary_engine_shutdown_switch_status);
		fprintf(fp, "%d ", ebc1->engine_derate_switch_status);
		fprintf(fp, "%d ", ebc1->accelerator_interlock_switch_status);
		fprintf(fp, "%.2f ",
			 ebc1->percent_engine_retarder_torque_selected);
		fprintf(fp, "%d ", ebc1->abs_ebs_amber_warning_state);
		fprintf(fp, "%d ", ebc1->ebs_red_warning_state);
		fprintf(fp, "%d ", ebc1->abs_fully_operational);

	 	fprintf(fp, "%d ", ebc1->source_address);
	 	fprintf(fp, "%.3f ", ebc1->total_brake_demand);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp, "EBS brake switch status %d \n",
			 ebc1->ebs_brake_switch_status);
		fprintf(fp, "ABS active status %d \n",
			 ebc1->abs_active_status);
		fprintf(fp, "ASR brake control status%d \n",
			 ebc1->asr_brake_control_active_status);
		fprintf(fp, "ASR engine control active status %d \n",
			 ebc1->asr_engine_control_active_status);
		fprintf(fp, "Brake pedal position %.2f\n ",
			 ebc1->brake_pedal_position);
		fprintf(fp, "Traction contorl override switch status %d \n",
			 ebc1->traction_control_override_switch_status);
		fprintf(fp, "Hill holder switch status %d \n",
			 ebc1->asr_hill_holder_switch_status);
		fprintf(fp, "ASR off road switch status %d \n",
			 ebc1->asr_off_road_switch_status);
		fprintf(fp, "ABS off road switch status %d \n",
			 ebc1->abs_off_road_switch_status);
		fprintf(fp, "Remote accelerator enable switch status %d \n",
			 ebc1->remote_accelerator_enable_switch_status);
		fprintf(fp, "Auxiliary engine shutdown switch status %d \n",
			 ebc1->auxiliary_engine_shutdown_switch_status);
		fprintf(fp, "Engine derate switch status %d \n",
			 ebc1->engine_derate_switch_status);
		fprintf(fp, "Accelerator interlock switch status %d \n",
			 ebc1->accelerator_interlock_switch_status);
		fprintf(fp, "Percent engine retarder torque selected %.2f \n",
			 ebc1->percent_engine_retarder_torque_selected);
		fprintf(fp, "ABS/EBS amber warning state %d \n",
			 ebc1->abs_ebs_amber_warning_state);
		fprintf(fp, "EBS red warning state %d \n",
			 ebc1->ebs_red_warning_state);
		fprintf(fp, "ABS fully operational %d \n",
			 ebc1->abs_fully_operational);
		fprintf(fp, "Source address %d (0x%0x)\n",
			 ebc1->source_address, ebc1->source_address);
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
	etc1->shift_in_progress = BITS65(byte);
	etc1->torque_converter_lockup_engaged = BITS43(byte);
	etc1->driveline_engaged = BITS21(byte);
	two_bytes = TWOBYTES(pdu->data_field[2], pdu->data_field[1]);
	etc1->output_shaft_speed = speed_in_rpm_2byte(two_bytes);
	etc1->percent_clutch_slip = percent_0_to_100(pdu->data_field[3]);
	byte = pdu->data_field[4];
	etc1->progressive_shift_disable = BITS43(byte);
	etc1->momentary_engine_overspeed_enable = BITS21(byte);
	two_bytes = TWOBYTES(pdu->data_field[6], pdu->data_field[5]);

	etc1->input_shaft_speed = speed_in_rpm_2byte(two_bytes);
	etc1->source_address = pdu->data_field[7];
}

void 
print_etc1(void *pdbv, FILE  *fp, int numeric)

{
	j1939_etc1_typ *etc1 = (j1939_etc1_typ *)pdbv;
	fprintf(fp, "ETC1 ");
	print_timestamp(fp, &etc1->timestamp);
	if (numeric) {
		fprintf(fp, "%d %d %d %.2f",
			etc1->shift_in_progress,	/* 2 */
			etc1->torque_converter_lockup_engaged,	/* 3 */
			etc1->driveline_engaged,	/* 4 */
			etc1->output_shaft_speed);	/* 5 */
		fprintf(fp, " %.2f %d %d %.2f %d\n",
			etc1->percent_clutch_slip,	/* 6 */
			etc1->progressive_shift_disable,	/* 7 */
			etc1->momentary_engine_overspeed_enable,	/* 8 */
			etc1->input_shaft_speed,	/* 9 */
			etc1->source_address);		/* 10 */			
		fprintf(fp, "\n");	
	}
	else {
		fprintf(fp, "Shift in progress %d\n",
			 etc1->shift_in_progress);
		fprintf(fp, "Torque converter lockup engaged %d\n",
			 etc1->torque_converter_lockup_engaged);
		fprintf(fp, "Driveline engaged %d\n",
			 etc1->driveline_engaged);
		fprintf(fp, "Output shaft speed %.2f\n",
			 etc1->output_shaft_speed);
		fprintf(fp, "Percent clutch slip %.2f\n",
			 etc1->percent_clutch_slip);
		fprintf(fp, "Progressive shift disable %d\n",
			 etc1->progressive_shift_disable);
		fprintf(fp, "Momentary engine overspeed enable %d\n",
			 etc1->momentary_engine_overspeed_enable);
		fprintf(fp, "Input shaft speed %.2f\n",
			 etc1->input_shaft_speed);
		fprintf(fp, "Source address %d (0x%0x)\n", etc1->source_address,
			etc1->source_address);
	}
}



/** EEC1 (Electronic Engine Controller #1) documented in J1939 - 71, p152 */
void
pdu_to_eec1 (struct j1939_pdu *pdu, void *pdbv)
{
	j1939_eec1_typ *eec1 = (j1939_eec1_typ *)pdbv;
	unsigned short two_bytes;

	eec1->engine_retarder_torque_mode = LONIBBLE(pdu->data_field[0]);
	eec1->driver_demand_percent_torque =
		 percent_m125_to_p125(pdu->data_field[1]); /* max 125% */
	eec1->actual_engine_percent_torque =
		 percent_m125_to_p125(pdu->data_field[2]); /* max 125% */
	two_bytes = TWOBYTES(pdu->data_field[4], pdu->data_field[3]);
	eec1->engine_speed = speed_in_rpm_2byte(two_bytes);
	eec1->source_address = pdu->data_field[5];
}

void 
print_eec1(void *pdbv, FILE  *fp, int numeric)

{
	j1939_eec1_typ *eec1 = (j1939_eec1_typ *)pdbv;
	fprintf(fp, "EEC1 ");
	print_timestamp(fp, &eec1->timestamp);
	if (numeric) {
		fprintf(fp," %d", eec1->engine_retarder_torque_mode);
		fprintf(fp," %.2f", eec1->driver_demand_percent_torque);
		fprintf(fp," %.2f", eec1->actual_engine_percent_torque);
		fprintf(fp," %.3f", eec1->engine_speed);
		fprintf(fp," %d", eec1->source_address);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp,"Engine retarder torque mode %d\n",
			 eec1->engine_retarder_torque_mode);
		fprintf(fp,"Driver's demand percent torque %.2f\n",
			 eec1->driver_demand_percent_torque);
		fprintf(fp,"Actual engine percent torque %.2f\n",
			 eec1->actual_engine_percent_torque);
		fprintf(fp,"Engine speed (rpm) %.3f\n", eec1->engine_speed);
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
	eec2->road_speed_limit_inactive = BITS65(byte);
	eec2->kickdown_active = BITS43(byte);
	eec2->low_idle = BITS21(byte);

	eec2->accelerator_pedal_position =
		 percent_0_to_100(pdu->data_field[1]);

	eec2->percent_load_current_speed = 
		percent_0_to_250(pdu->data_field[2]);	/* max 125% */

	eec2->remote_accelerator_position =
		percent_0_to_100 (pdu->data_field[3]);	
}

void 
print_eec2(void *pdbv, FILE  *fp, int numeric)

{
	j1939_eec2_typ *eec2 = (j1939_eec2_typ *)pdbv;
	fprintf(fp, "EEC2 ");
	print_timestamp(fp, &eec2->timestamp);
	if (numeric){
		fprintf(fp," %d", eec2->road_speed_limit_inactive);
		fprintf(fp," %d", eec2->kickdown_active);
		fprintf(fp," %d", eec2->low_idle);
		fprintf(fp," %.2f", eec2->accelerator_pedal_position);
		fprintf(fp," %.2f", eec2->percent_load_current_speed);
		fprintf(fp," %.2f", eec2->remote_accelerator_position);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp,"Road speed limit %d\n", eec2->road_speed_limit_inactive);
		fprintf(fp,"Kickpedal active %d\n", eec2->kickdown_active);
		fprintf(fp,"Low idle %d\n", eec2->low_idle);
		fprintf(fp,"AP position %.2f\n", eec2->accelerator_pedal_position);
		fprintf(fp,"Percent load %.2f\n", eec2->percent_load_current_speed);
		fprintf(fp,"Remote accelerator %.2f\n",
			 eec2->remote_accelerator_position);
	}
}

/** ETC2 (Electronic Transmission Controller #3) documented in J1939 - 71, p152
*/
void
pdu_to_etc2 (struct j1939_pdu *pdu, void *pdbv)
{
	j1939_etc2_typ *etc2 = (j1939_etc2_typ *)pdbv;
	unsigned short two_bytes;

	etc2->selected_gear =
		 gear_m125_to_p125(pdu->data_field[0]);

	two_bytes = TWOBYTES(pdu->data_field[2], pdu->data_field[1]);

	etc2->actual_gear_ratio = gear_ratio(two_bytes);	
	etc2->current_gear =
		 gear_m125_to_p125(pdu->data_field[3]);

	etc2->transmission_requested_range[0] = pdu->data_field[4];
	etc2->transmission_requested_range[1] = pdu->data_field[5];
	etc2->transmission_current_range[0] = pdu->data_field[6];
	etc2->transmission_current_range[1] = pdu->data_field[7];
}

void 
print_etc2(void *pdbv, FILE  *fp, int numeric)

{
	j1939_etc2_typ *etc2 = (j1939_etc2_typ *)pdbv;
	fprintf(fp, "ETC2 ");
	print_timestamp(fp, &etc2->timestamp);
	if (numeric){
		fprintf(fp," %d", etc2->selected_gear);
		fprintf(fp," %.2f", etc2->actual_gear_ratio);
		fprintf(fp," %d", etc2->current_gear);
		fprintf(fp," 0x%x%x", etc2->transmission_requested_range[0],
			etc2->transmission_requested_range[1]);
		fprintf(fp," 0x%x%x", etc2->transmission_current_range[0],
			etc2->transmission_current_range[1]);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp,"Selected gear %d\n", etc2->selected_gear);
		fprintf(fp,"Actual gear ratio %.2f\n",
			 etc2->actual_gear_ratio);
		fprintf(fp,"Current gear %d\n", etc2->current_gear);
		fprintf(fp,"Transmission requested range 0x%x%x\n",
			etc2->transmission_requested_range[0],
			etc2->transmission_requested_range[1]);
		fprintf(fp,"Transmission current range 0x%x%x\n",
			etc2->transmission_current_range[0],
			etc2->transmission_current_range[1]);
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
	j1939_eec3_typ *eec3 = (j1939_eec3_typ *)pdbv;
	unsigned short two_bytes;

	eec3->nominal_friction_percent_torque =
		 percent_m125_to_p125(pdu->data_field[0]);

	two_bytes = TWOBYTES(pdu->data_field[2], pdu->data_field[1]);
	eec3->engine_desired_operating_speed = speed_in_rpm_2byte(two_bytes);	
	eec3->operating_speed_asymmetry_adjust = pdu->data_field[3];
}

void 
print_eec3(void *pdbv, FILE  *fp, int numeric)

{
	j1939_eec3_typ *eec3 = (j1939_eec3_typ *)pdbv;
	fprintf(fp, "EEC3 ");
	print_timestamp(fp, &eec3->timestamp);
	if (numeric){
		fprintf(fp," %.2f", eec3->nominal_friction_percent_torque);
		fprintf(fp," %.3f", eec3->engine_desired_operating_speed);
		fprintf(fp," %d", eec3->operating_speed_asymmetry_adjust);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp,"Nominal friction percent torque %.2f\n",
			 eec3->nominal_friction_percent_torque);
		fprintf(fp,"Engine desired operating speed (rpm) %.3f\n",
			 eec3->engine_desired_operating_speed);
		fprintf(fp,"Operating speed asymmetry adjustment %d\n",
			 eec3->operating_speed_asymmetry_adjust);
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
	ccvs->parking_brake = BITS43(byte);
	ccvs->two_speed_axle = BITS21(byte);

	two_bytes = TWOBYTES(pdu->data_field[2], pdu->data_field[1]);
	ccvs->wheel_based_vehicle_speed = wheel_based_mps(two_bytes);	

	byte = pdu->data_field[3];
	ccvs->clutch_switch = BITS87(byte);
	ccvs->brake_switch = BITS65(byte);
	ccvs->cruise_control_enable = BITS43(byte);
	ccvs->cruise_control_active = BITS21(byte);

	byte = pdu->data_field[4];
	ccvs->cruise_control_accelerate = BITS87(byte);
	ccvs->cruise_control_resume = BITS65(byte);
	ccvs->cruise_control_coast = BITS43(byte);
	ccvs->cruise_control_set = BITS21(byte);

	ccvs->cruise_control_set_speed =
		 cruise_control_set_meters_per_sec(pdu->data_field[5]);
	
	byte = pdu->data_field[6];
	ccvs->cruise_control_state = HINIBBLE(byte) >> 1;
	ccvs->pto_state = byte & 0x1f;
	
	byte = pdu->data_field[7];
	ccvs->engine_shutdown_override = BITS87(byte);
	ccvs->engine_test_mode = BITS65(byte);
	ccvs->idle_decrement = BITS43(byte);
	ccvs->idle_increment = BITS21(byte);
}

void 
print_ccvs(void *pdbv, FILE  *fp, int numeric) 
{
	j1939_ccvs_typ *ccvs = (j1939_ccvs_typ *)pdbv;
	fprintf(fp, "CCVS ");
	print_timestamp(fp, &ccvs->timestamp);
	if (numeric){
		fprintf(fp," %d", ccvs->parking_brake);
		fprintf(fp," %d", ccvs->two_speed_axle);
		fprintf(fp," %.3f", ccvs->wheel_based_vehicle_speed);
		fprintf(fp," %d", ccvs->clutch_switch);
		fprintf(fp," %d", ccvs->brake_switch);
		fprintf(fp," %d", ccvs->cruise_control_enable);
		fprintf(fp," %d", ccvs->cruise_control_active);
		fprintf(fp," %d", ccvs->cruise_control_accelerate);
		fprintf(fp," %d", ccvs->cruise_control_resume);
		fprintf(fp," %d", ccvs->cruise_control_coast);
		fprintf(fp," %d", ccvs->cruise_control_set);
		fprintf(fp," %.3f", ccvs->cruise_control_set_speed);
		fprintf(fp," %d", ccvs->cruise_control_state);
		fprintf(fp," %d", ccvs->pto_state);
		fprintf(fp," %d", ccvs->engine_shutdown_override);
		fprintf(fp," %d", ccvs->engine_test_mode);
		fprintf(fp," %d", ccvs->idle_decrement);
		fprintf(fp," %d", ccvs->idle_increment);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp,"Parking brake %d\n", ccvs->parking_brake);
		fprintf(fp,"Two speed axle switch %d\n", ccvs->two_speed_axle);
		fprintf(fp,"Wheel-based vehicle speed (meters/sec) %.3f\n",
			ccvs->wheel_based_vehicle_speed);
		fprintf(fp,"Clutch switch %d\n", ccvs->clutch_switch);
		fprintf(fp,"Brake switch %d\n", ccvs->brake_switch);
		fprintf(fp,"Cruise control enable %d\n", ccvs->cruise_control_enable);
		fprintf(fp,"Cruise control active %d\n", ccvs->cruise_control_active);
		fprintf(fp,"Cruise control accelerate %d\n",
			ccvs->cruise_control_accelerate);
		fprintf(fp,"Cruise control resume %d\n", ccvs->cruise_control_resume);
		fprintf(fp,"Cruise control coast %d\n", ccvs->cruise_control_coast);
		fprintf(fp,"Cruise control set %d\n", ccvs->cruise_control_set);
		fprintf(fp,"Cruise control set speed %.3f\n", 
			ccvs->cruise_control_set_speed);
		fprintf(fp,"Cruise control state %d\n", ccvs->cruise_control_state);
		fprintf(fp,"PTO state %d\n", ccvs->pto_state);
		fprintf(fp,"Engine shutdown override %d\n",
			 ccvs->engine_shutdown_override);
		fprintf(fp,"Engine test mode %d\n", ccvs->engine_test_mode);
		fprintf(fp,"Idle decrement %d\n", ccvs->idle_decrement);
		fprintf(fp,"Idle increment %d\n", ccvs->idle_increment);
	}
}

/** LFE (Fuel Economy) documented in J1939 - 71, p162 */
void
pdu_to_lfe (struct j1939_pdu *pdu, void *pdbv)
{
	j1939_lfe_typ *lfe = (j1939_lfe_typ *)pdbv;
	unsigned short two_bytes;

	two_bytes = TWOBYTES(pdu->data_field[1], pdu->data_field[0]);
	lfe->fuel_rate = fuel_rate_cm3_per_sec(two_bytes);	

	two_bytes = TWOBYTES(pdu->data_field[3], pdu->data_field[2]);
	lfe->instantaneous_fuel_economy =
		fuel_economy_meters_per_cm3(two_bytes);	

	two_bytes = TWOBYTES(pdu->data_field[5], pdu->data_field[4]);
	lfe->average_fuel_economy =
		fuel_economy_meters_per_cm3(two_bytes);	

	lfe->throttle_position = percent_0_to_100(pdu->data_field[6]);
}

void 
print_lfe(void *pdbv, FILE  *fp, int numeric)
{
	j1939_lfe_typ *lfe = (j1939_lfe_typ *)pdbv;
	fprintf(fp, "LFE ");
	print_timestamp(fp, &lfe->timestamp);
	if (numeric){
		fprintf(fp," %.3f", lfe->fuel_rate);
		fprintf(fp," %.3f", lfe->instantaneous_fuel_economy);
		fprintf(fp," %.3f", lfe->average_fuel_economy);
		fprintf(fp," %.3f", lfe->throttle_position);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp,"Fuel rate (cm3/sec) %.3f\n", lfe->fuel_rate);
		fprintf(fp,"Instantaneous fuel economy (m/cm3) %.3f\n",
			 lfe->instantaneous_fuel_economy);
		fprintf(fp,"Average fuel economy (m/cm3) %.3f\n",
			 lfe->average_fuel_economy);
		fprintf(fp,"Throttle position (percent) %.3f\n",
			 lfe->throttle_position);
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
	ebc2->front_axle_speed = wheel_based_mps(two_bytes);
	ebc2->front_left_wheel_relative =
		wheel_based_mps_relative(pdu->data_field[2]);
	ebc2->front_right_wheel_relative =
		wheel_based_mps_relative(pdu->data_field[3]);
	ebc2->rear1_left_wheel_relative =
		wheel_based_mps_relative(pdu->data_field[4]);
	ebc2->rear1_right_wheel_relative =
		wheel_based_mps_relative(pdu->data_field[5]);
	ebc2->rear2_left_wheel_relative =
		wheel_based_mps_relative(pdu->data_field[6]);
	ebc2->rear2_right_wheel_relative =
		wheel_based_mps_relative(pdu->data_field[7]);
}

void 
print_ebc2(void *pdbv, FILE  *fp, int numeric)

{
	j1939_ebc2_typ *ebc2 = (j1939_ebc2_typ *)pdbv;
	fprintf(fp, "EBC2 ");
	print_timestamp(fp, &ebc2->timestamp);
	if (numeric){
		fprintf(fp," %.3f", ebc2->front_axle_speed);
		fprintf(fp," %.3f", ebc2->front_left_wheel_relative);
		fprintf(fp," %.3f", ebc2->front_right_wheel_relative);
		fprintf(fp," %.3f", ebc2->rear1_left_wheel_relative);
		fprintf(fp," %.3f", ebc2->rear1_right_wheel_relative);
		fprintf(fp," %.3f", ebc2->rear2_left_wheel_relative);
		fprintf(fp," %.3f", ebc2->rear2_right_wheel_relative);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp,"Front axle speed %.3f\n",
			ebc2->front_axle_speed);
		fprintf(fp,"Front left wheel relative speed %.3f\n",
			ebc2->front_left_wheel_relative);
		fprintf(fp,"Front left wheel relative speed %.3f\n",
			ebc2->front_right_wheel_relative);
		fprintf(fp,"Rear 1 left wheel relative speed %.3f\n",
			ebc2->rear1_left_wheel_relative);
		fprintf(fp,"Rear 1 left wheel relative speed %.3f\n",
			ebc2->rear1_right_wheel_relative);
		fprintf(fp,"Rear 2 left wheel relative speed %.3f\n",
			ebc2->rear2_left_wheel_relative);
		fprintf(fp,"Rear 2 left wheel relative speed %.3f\n",
			ebc2->rear2_right_wheel_relative);
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
	j1939_tsc1_typ *tsc1 = (j1939_tsc1_typ *) pdbv;
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
        pdu->data_field[0] = BITS21(tsc1->override_control_mode_priority) << 4 |
			BITS21(tsc1->requested_speed_control_conditions) << 2 |
			BITS21(tsc1->override_control_modes);
	requested_speed = code_engine_speed(tsc1->requested_speed_or_limit);
        pdu->data_field[1] = LOBYTE(requested_speed);
        pdu->data_field[2] = HIBYTE(requested_speed);
	pdu->data_field[3] =
		code_percent_m125_to_p125(tsc1->requested_torque_or_limit);
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
        tsc1->override_control_mode_priority = BITS65(pdu->data_field[0]);
        tsc1->requested_speed_control_conditions = BITS43(pdu->data_field[0]);
        tsc1->override_control_modes = BITS21(pdu->data_field[0]);
       	data = TWOBYTES(pdu->data_field[2], pdu->data_field[1]);
        tsc1->requested_speed_or_limit = speed_in_rpm_2byte(data);
        tsc1->requested_torque_or_limit = percent_m125_to_p125(pdu->data_field[3]);
        tsc1->destination_address = pdu->pdu_specific; /* engine or retarder */
}

void 
print_tsc1(void *pdbv, FILE  *fp, int numeric)

{
	j1939_tsc1_typ *tsc1 = (j1939_tsc1_typ *)pdbv;
	fprintf(fp, "TSC1 ");
	print_timestamp(fp, &tsc1->timestamp);
	if (numeric) {
		fprintf(fp, "%d ", tsc1->destination_address);
		fprintf(fp, "%d ", tsc1->src_address);
		fprintf(fp, "%d ", tsc1->override_control_mode_priority);
		fprintf(fp, "%d ", tsc1->requested_speed_control_conditions);
		fprintf(fp, "%d ", tsc1->override_control_modes);
		fprintf(fp, "%.3f ", tsc1->requested_speed_or_limit);
		fprintf(fp, "%.3f ", tsc1->requested_torque_or_limit);
		fprintf(fp, "\n");	
	} else {
		fprintf(fp, "Destination %d \n", tsc1->destination_address);
		fprintf(fp, "Source address %d\n", tsc1->src_address);
		fprintf(fp, "Override Control Mode priority %d\n",
			 tsc1->override_control_mode_priority);
		fprintf(fp, "Requested speed control conditions %d\n",
			 tsc1->requested_speed_control_conditions);
		fprintf(fp, "Override control mode %d\n",
			 tsc1->override_control_modes);
		fprintf(fp, "Requested speed/speed limit  %.3f\n",
			 tsc1->requested_speed_or_limit);
		fprintf(fp, "Requested torque/torque limit %.3f\n",
			 tsc1->requested_torque_or_limit);
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
 */
 
void exac_to_pdu (struct j1939_pdu *pdu, void *pdbv) 
{
	j1939_exac_typ *exac = (j1939_exac_typ *) pdbv;
	short requested_deceleration;

        pdu->priority = 3; /* high PDU priority */
        pdu->R = 0;
        pdu->DP = 0;
        pdu->pdu_format = 0;	
        pdu->pdu_specific = J1939_ADDR_BRAKE;

	/* pretend to be adaptive cruise control */
        pdu->src_address = exac->src_address; 

        pdu->numbytes = 8;
        pdu->data_field[0] = 
			0xf0 |		/* bits 5-8 undefined */
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
			0xf0 |		/* bits 5-8 undefined */
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
