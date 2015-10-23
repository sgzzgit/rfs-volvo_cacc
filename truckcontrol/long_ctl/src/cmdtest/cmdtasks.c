/* FILE
 *   cmdtasks.c
 * 
 * Example using longitudinal shared code for brake testing.
 *	Runs a variety of simple tests of acceleration and
 *	braking actuation. Parsing of the configuration file and parameters
 *	in the function init_tasks determines which test is done, and
 *	what the parameters of the test are. The kinds of tests that can
 *	be done:
 *
 *	1) commanding of speed or torque, using J1939 bus, to a constant
 *		value (on systems with no J1939 torque/speed control,
 *		the accelerator pedal command voltage is scaled depending
 *		on the commanded engine torque)
 *	2) simple control to reach a specified vehicle speed, maintain
 *		that speed for a specified distance, then brake
 *
 * Three types of braking can be done: air brakes, engine retarder (jake brake)
 * and transmission retarder.
 * Control has the following phases:
 *      - INITIAL_OPEN_LOOP Torque command is increased until desired
 * vehicle speed is reached.
 *      - CLOSED_LOOP_TORQUE A simple P-type controller is used to maintain
 * vehicle speed for the desired distance by adjusting the torque command.
 *      - CLOSED_LOOP_BRAKING
 *
 *
 */

#include "long_ctl.h"
#include "cmdtasks.h"


/* for now, define voltage to send when no acceleration is required as 0.0 */
#define ACC_OFF_VOLTAGE 0.0

/**
 * trq_to_acc_voltage 
 * Computes a voltage to send as the acc pedal control from
 * the desired engine torque 
 */ 

float trq_to_acc_voltage(long_ctrl *pctrl, float engine_torque)
{
	float idle_torque = 400.0;	/* idle torque, param later? */
	float max_torque = pctrl->params.engine_reference_torque;
	float trq_range = max_torque - idle_torque;
	float vlow = 1.0;	/* active voltage lower and upper bounds */
	float vhigh = 3.0;
	float vrange = vhigh - vlow;
	float trq_val = engine_torque - idle_torque;
	if (trq_val < 0) trq_val = 0;
	return (vlow + (trq_val/trq_range) * vrange);
}

/**
 * sinusoidal
 * Returns a floating point value as a function of the current time,
 * period, and maximum value. 
 */

static float sinusoidal(float current_time, float period, float max_value)
{
	return (max_value *
		fabs(0.5 * sin(current_time * 15.0 * PI/period +0.5*PI)-0.5));  
}

/**
 * brake_tasks
 * Utility function used in braking phase
 */
int brake_tasks(long_ctrl *pctrl, long_output_typ *pcmd)
{
	long_params *pparams = &pctrl->params;
	long_private *ppriv = pctrl->plong_private;
	private_params *pcparams = &ppriv->cmd_params;
	long_vehicle_state *pstate = &pctrl->vehicle_state;
	float delta_t = pparams->ctrl_interval * 0.001; /* in seconds */
	float retarder_reference_torque = pparams->retarder_reference_torque;
	int brk_task_type = pcparams->brk_task_type;
	float ebs_deceleration = pcparams->ebs_deceleration;
	float max_deceleration = pcparams->ebs_deceleration; /* for sinusoidal */
	float max_braking_time = pcparams->max_braking_time;
	float engine_retarder_torque = pcparams->engine_retarder_torque;
	float trans_retarder_value = pcparams->trans_retarder_value;

	float current_speed = pstate->EBC2_FrontAxleSpeed;

	/* used for EBS_SINUSOIDAL, scaled as function of velocity when
	 * braking begins */

	float stop_period;
	
	ppriv->ebs_brk_t += delta_t;                                                                       // braking time

	switch (brk_task_type) {
	case EBS_NOBRAKE:
		ppriv->cmd_ebs_dcc = 0.0;
		ppriv->cmd_rtdr_trq = 0.0;
		ppriv->cmd_trtdr_val = 0.0;
		pcmd->engine_retarder_command_mode = TSC_OVERRIDE_DISABLED;
		pcmd->trans_retarder_command_mode = TSC_OVERRIDE_DISABLED;
		pcmd->brake_command_mode = XBR_NOT_ACTIVE;
		break;
	case EBS_CONSTANT:
		if (ppriv->ebs_brk_t  < max_braking_time)
		    ppriv->cmd_ebs_dcc = ebs_deceleration;
		else
		    ppriv->cmd_ebs_dcc = 0.0;
		if (current_speed < LOW_VEHICLE_SPEED) 
			ppriv->cmd_ebs_dcc = MAX_EBS_DECELERATION;
		pcmd->brake_command_mode = XBR_ACTIVE;
		if (ppriv->ebs_brk_t > max_braking_time)
			return 0;
		else
			return 1;
		break;
	case EBS_SINUSOIDAL:
		stop_period = 2.0 * (ppriv->v_init_brk/max_deceleration);

		if (ppriv->ebs_brk_t < stop_period) {
			ppriv->cmd_ebs_dcc = sinusoidal(ppriv->ebs_brk_t,
						stop_period, max_deceleration);
			pcmd->brake_command_mode = XBR_ACTIVE;
			return 1;
		}
		else {
			ppriv->cmd_ebs_dcc = MAX_EBS_DECELERATION;
			return 0;
		}
		
		break;
	case ENG_RTDR_CONSTANT:
		pcmd->brake_command_mode = XBR_NOT_ACTIVE;
		pcmd->trans_retarder_command_mode = TSC_OVERRIDE_DISABLED;
		pcmd->engine_retarder_command_mode = TSC_TORQUE_CONTROL;
		if (ppriv->ebs_brk_t  < max_braking_time)
		    ppriv->cmd_rtdr_trq = engine_retarder_torque;
		else
		    ppriv->cmd_rtdr_trq = 0.0;;
		break;
	case ENG_RTDR_SINUSOIDAL:
		pcmd->brake_command_mode = XBR_NOT_ACTIVE;
		pcmd->trans_retarder_command_mode = TSC_OVERRIDE_DISABLED;
		pcmd->engine_retarder_command_mode = TSC_TORQUE_CONTROL;

		stop_period = (retarder_reference_torque/engine_retarder_torque)
				 * (ppriv->v_init_brk);

		if (ppriv->ebs_brk_t < stop_period) {
			ppriv->cmd_rtdr_trq = sinusoidal(ppriv->ebs_brk_t,
					stop_period, engine_retarder_torque);
			pcmd->engine_retarder_command_mode = TSC_TORQUE_CONTROL;
			return 1;
		}
		else {
			ppriv->cmd_rtdr_trq = retarder_reference_torque;
			return 0;
		}
		break;
	case TRANS_RTDR_CONSTANT:
		pcmd->brake_command_mode = XBR_NOT_ACTIVE;
		pcmd->engine_retarder_command_mode = TSC_OVERRIDE_DISABLED;
		pcmd->trans_retarder_command_mode = TSC_TORQUE_CONTROL;
		if (ppriv->ebs_brk_t  < max_braking_time)
		    ppriv->cmd_trtdr_val = trans_retarder_value;
		else
		    ppriv->cmd_trtdr_val = 0.0;;
		break;
	case TRANS_RTDR_SINUSOIDAL:
		pcmd->brake_command_mode = XBR_NOT_ACTIVE;
		pcmd->engine_retarder_command_mode = TSC_OVERRIDE_DISABLED;
		pcmd->trans_retarder_command_mode = TSC_TORQUE_CONTROL;
		stop_period = (100/trans_retarder_value) * (ppriv->v_init_brk);

		if (ppriv->ebs_brk_t < stop_period) {
			ppriv->cmd_trtdr_val = sinusoidal(ppriv->ebs_brk_t,
					stop_period, trans_retarder_value);
			return 1;
		}
		else {
			ppriv->cmd_trtdr_val = trans_retarder_value;
			return 0;
		}
		break;
	default:
		fprintf(stderr, "Unknown braking type\n");
		break;
	}                                                                                                        
	return 1;
}

/**
 * print_params prints long_params and private_params structures, 
 * for documentation or debugging. 
 */
void print_params(FILE *fp, long_params *pparams, private_params *pcparams)
{
	fprintf(fp, "avcs_cfg_file %s\n", pparams->avcs_cfg_file);
	fprintf(fp, "long_cfg_file %s\n", pparams->long_cfg_file);
	fprintf(fp, "data_file %s\n", pparams->data_file);

	fprintf(fp, "gather_data %d\n", pcparams->gather_data);

	fprintf(fp, "max_iterations %d\n", pparams->max_iterations);
	fprintf(fp, "max_engine_speed %.3f\n", pparams->max_engine_speed);
	fprintf(fp, "engine_reference_torque %.3f\n", pparams->engine_reference_torque);
	fprintf(fp, "retarder_reference_torque %.3f\n", pparams->retarder_reference_torque);

	fprintf(fp, "cmd_test %d\n", pparams->cmd_test);
	fprintf(fp, "acc_task_type %d\n", pcparams->acc_task_type);
	fprintf(fp, "brk_task_type %d\n", pcparams->brk_task_type);
	fprintf(fp, "engine_command_mode %d\n",	pcparams->engine_command_mode);
	fprintf(fp, "engine_speed %.3f\n", pcparams->engine_speed);
	fprintf(fp, "engine_torque %.3f\n", pcparams->engine_torque);
	fprintf(fp, "desired_vehicle_speed %.3f\n", pcparams->desired_vehicle_speed);
	fprintf(fp, "init_torque %.3f\n", pcparams->init_torque);
	fprintf(fp, "delta_torque %.3f\n", pcparams->delta_torque);
	fprintf(fp, "trq_time_limit %.3f\n", pcparams->trq_time_limit);
	fprintf(fp, "stop_distance %.3f\n", pcparams->stop_distance);
	fprintf(fp, "ctrl_interval %d\n", pparams->ctrl_interval);
	fprintf(fp, "control_gain %.3f\n", pcparams->control_gain);
	fprintf(fp, "max_braking_time %.3f\n", pcparams->max_braking_time);
	fprintf(fp, "ebs_deceleration %.3f\n", pcparams->ebs_deceleration);
	fprintf(fp, "engine_retarder_torque %.3f\n", pcparams->engine_retarder_torque);
	fprintf(fp, "trans_retarder_value %.3f\n", pcparams->trans_retarder_value);
}

static long_private cmd_private;

/**
 * init_tasks
 * Initialization at the beginning of a run of the controller
 */

int init_tasks(db_clt_typ *pclt, long_ctrl *pctrl, long_output_typ *pcmd)
{
	long_params *pparams = &pctrl->params;
	cbuff_typ *pbuff = &pctrl->buff;

	private_params *pcparams = &cmd_private.cmd_params;
  
	char buffer[MAX_LINE_LEN+1];
	char *ini_file;  	/* initialization file name */ 
	FILE *pfin;		/* file pointer for initialization files */

printf("cmdtasks.c:init_tasks:Got to 1\n");
	/* Read from long_ctl specific ini file */
	ini_file = pparams->long_cfg_file;
  
	/* Read from CommandTestType section */
	if ((pfin = get_ini_section(ini_file, "CommandTestType")) == NULL) {
		printf("CMDTASKS.C1:Cannot get ini file %s, section %s\n",
		   ini_file, buffer); 
		fflush(stdout);
		return 0;
	}
printf("cmdtasks.c:init_tasks:Got to 2\n");
	pcparams->acc_task_type = get_ini_long(pfin, "AccTaskType", 0);
	pcparams->brk_task_type = get_ini_long(pfin, "BrakingTaskType", 0);
	pcparams->gather_data = get_ini_long(pfin, "GatherData", 30000);
	pcparams->trq_time_limit = 
		 get_ini_double(pfin, "TrqTimeLimit", 60000); /* ms */
	pcparams->desired_vehicle_speed =
		 get_ini_double(pfin, "VehicleSpeed", 10.0); /* m/s */
	pcparams->init_torque = 
		 get_ini_double(pfin, "InitTorque", 450.0); /* N-m */
	pcparams->delta_torque = 
		 get_ini_double(pfin, "DeltaTorque", 30.0); /* N-m */
	pcparams->initial_reduction_trq = 
		 get_ini_double(pfin, "InitialReductionTrq", 0.6); /* fraction */
	pcparams->stop_distance =    
		 get_ini_double(pfin, "StopDistance", 150.0); 
	pcparams->control_gain = 
		 get_ini_double(pfin, "ControlGain", 10.0); 
	pcparams->initial_stop_time = 
		 get_ini_double(pfin, "InitialStopTime", 10000.0); 

printf("cmdtasks.c:init_tasks:Got to 3\n");

	if (pcparams->acc_task_type == 0)  {
		printf("invalid task type: acc %d, brk %d\n",
			pcparams->acc_task_type, pcparams->brk_task_type);
		fflush(stdout);
		return 0;
	}
		
	/* Read parameters from CommandTest#n section for particular test.
	 * Parameters not set in config file will get default value.
	 */
	sprintf(buffer, "%s%d", "CommandTest", pparams->cmd_test);
  
	if ((pfin = get_ini_section(ini_file, buffer)) == NULL) {
		printf("CMDTASKS.C2:Cannot get ini file %s, section %s\n",
		   ini_file, buffer); 
		fflush(stdout);
		return 0;
	}
printf("cmdtasks.c:init_tasks:Got to 4\n");
	pcparams->engine_command_mode =
		 get_ini_long(pfin, "EngineCommandMode", TSC_OVERRIDE_DISABLED);
	pcparams->engine_speed = get_ini_double(pfin, "EngineSpeed", 800.0);
	pcparams->engine_torque = get_ini_double(pfin, "EngineTorque", 400.0);
	pcparams->max_braking_time = get_ini_double(pfin, "MaxBrakingTime", 0);
	pcparams->ebs_deceleration = get_ini_double(pfin, "EBSDeceleration", 0);
	pcparams->engine_retarder_torque =
		get_ini_double(pfin, "EngineRetarderTorque", 0);
	pcparams->trans_retarder_value =
		get_ini_double(pfin, "TransRetarderValue", 0);

	/* Initialize long_private structure */
        cmd_private.cmd_eng_trq = pcparams->init_torque; 
        cmd_private.cmd_ebs_dcc = 0.0;
	cmd_private.cmd_rtdr_trq = 0.0;
	cmd_private.cmd_trtdr_val = 0.0;
        cmd_private.run_dist = 0.0;   
        cmd_private.trq_cmd_t = 0.0; 
        cmd_private.v_init_brk = 0.0;    
        cmd_private.trq_fixed = 0.0;    
	cmd_private.ebs_brk_t = 0.0;
	if (pcparams->acc_task_type == CONSTANT_COMMAND) {
		cmd_private.execution_state = ISSUE_CONSTANT_COMMAND; 
	} else if (pcparams->acc_task_type == VARY_TORQUE) {
		cmd_private.execution_state = DO_VARY_TORQUE; 
		printf("DO_VARY_TORQUE\n");
	} else {
		cmd_private.execution_state = INITIAL_FULL_STOP;    
	}
	pctrl->plong_private = &cmd_private;

	pcmd->engine_command_mode = TSC_OVERRIDE_DISABLED;
	pcmd->brake_command_mode = XBR_NOT_ACTIVE;
	pcmd->engine_retarder_command_mode = TSC_OVERRIDE_DISABLED;
	pcmd->trans_retarder_command_mode = TSC_OVERRIDE_DISABLED;

printf("cmdtasks.c:init_tasks:Got to 5\n");
	/* Initialize data buffer */
	init_circular_buffer(pbuff, pcparams->gather_data, sizeof(buffer_item));

	ftime(&pctrl->start_time);

printf("cmdtasks.c:init_tasks:Got to 6\n");
	print_params(stdout, pparams, pcparams);

	return 1;
}
/**
 * run_tasks
 * Whatever you want to do: in this case, run simple control to
 * test J1939 send commands.
 */

int run_tasks(db_clt_typ *pclt, long_ctrl *pctrl, long_output_typ *pcmd)
{
        long_vehicle_state *pv = &pctrl->vehicle_state;
        long_params *pparams = &pctrl->params;
	cbuff_typ *pbuff = &pctrl->buff;
	buffer_item *pdata = (buffer_item *) pbuff->data_array;
	long_private *ppriv = (long_private *) pctrl->plong_private;
	private_params *pcparams = &ppriv->cmd_params;

	/* copy execution state for reference after update */
	int current_state = ppriv->execution_state;

	/* copy unchanging parameters to local variables for easy reference */
	float engine_reference_torque = pparams->engine_reference_torque;
	float desired_vehicle_speed = pcparams->desired_vehicle_speed; 
	float delta_torque = pcparams->delta_torque;
	float trq_time_limit = pcparams->trq_time_limit;
	int ctrl_interval = pparams->ctrl_interval; /* in milliseconds */
	float delta_t = ctrl_interval * 0.001; /* in seconds */
	float stop_distance = pcparams->stop_distance;
	int gather_data = pcparams->gather_data;

	/* local variables */
        float current_speed;
	buffer_item current_item;
        
        current_speed = pv->EBC2_FrontAxleSpeed; 

        /* Update distance traveled based on speed during control interval */
        
        ppriv->run_dist += delta_t * current_speed; /* meters/s */
        
	/* Update time elapsed since beginning of torque control */
        ppriv->trq_cmd_t += ctrl_interval;

        /* Revise torque command value upwards until desired vehicle 
	 * speed is reached.
	 * After desired speed is reached, maintain speed until run distance
	 * or time limit is exceeded, then decelerate using method
	 * specified.
         */

	switch (current_state) {
	case ISSUE_CONSTANT_COMMAND:
		pcmd->engine_command_mode = pcparams->engine_command_mode;
		pcmd->engine_speed = pcparams->engine_speed;
		pcmd->engine_torque = pcparams->engine_torque;
		if ((ppriv->run_dist >= stop_distance) ||
			 (ppriv->trq_cmd_t >= trq_time_limit)) { 
			ppriv->execution_state = CLOSED_LOOP_BRAKING;
			ppriv->v_init_brk = current_speed;
		}
		break;	
	case DO_VARY_TORQUE:
		pcmd->engine_command_mode = pcparams->engine_command_mode;
		ppriv->cmd_eng_trq = sinusoidal(ppriv->trq_cmd_t,
						(trq_time_limit), 
					pcparams->engine_torque) + 300;
#ifdef DEBUG
		printf("time %f, period %f, max %f, torque %f\n",
			ppriv->trq_cmd_t,
			(trq_time_limit),
			pcparams->engine_torque,
			ppriv->cmd_eng_trq);
#endif
		pcmd->engine_torque = ppriv->cmd_eng_trq;
		if ((ppriv->run_dist >= stop_distance) ||
			 (ppriv->trq_cmd_t >= trq_time_limit)) { 
			ppriv->execution_state = CLOSED_LOOP_BRAKING;
			ppriv->v_init_brk = current_speed;
		}
		break;	
	case INITIAL_FULL_STOP:
		pcmd->engine_command_mode = TSC_OVERRIDE_DISABLED;
		pcmd->engine_retarder_command_mode = TSC_OVERRIDE_DISABLED;
		pcmd->trans_retarder_command_mode = TSC_OVERRIDE_DISABLED;
		pcmd->brake_command_mode = XBR_ACTIVE; 
		ppriv->cmd_ebs_dcc = MAX_EBS_DECELERATION;

		if (ppriv->trq_cmd_t  > pcparams->initial_stop_time) {
                        ppriv->execution_state = INITIAL_OPEN_LOOP;
		}
		break;

	case INITIAL_OPEN_LOOP:
		pcmd->engine_command_mode = TSC_TORQUE_CONTROL;
		pcmd->engine_retarder_command_mode = TSC_OVERRIDE_DISABLED;
		pcmd->trans_retarder_command_mode = TSC_OVERRIDE_DISABLED;
		pcmd->brake_command_mode = XBR_NOT_ACTIVE; 
		ppriv->cmd_ebs_dcc = 0.0;

		if (current_speed < desired_vehicle_speed)
                        ppriv->cmd_eng_trq += delta_torque * delta_t;
                else {
                        ppriv->execution_state = CLOSED_LOOP_TORQUE;
			ppriv->trq_fixed = pcparams->initial_reduction_trq
						 * ppriv->cmd_eng_trq;
		}

		if (ppriv->run_dist >= stop_distance) {
			ppriv->execution_state = CLOSED_LOOP_BRAKING;
			ppriv->v_init_brk = current_speed;
		}

		break;
	case CLOSED_LOOP_TORQUE:
        /*   A simple P-Type closed loop control by  XYLU on Oct. 29 02 */
#ifdef DEBUG_CLOSED_LOOP
		printf("closed loop: control gain %.3f\n", pcparams->control_gain);
		printf("closed loop: current speed %.3f\n", current_speed);
		printf("closed loop: desired vehicle speed %.3f\n",
				 desired_vehicle_speed);
#endif
                ppriv->cmd_eng_trq = ppriv->trq_fixed  
				 + pcparams->control_gain *
				   (desired_vehicle_speed - current_speed);

		pcmd->engine_command_mode = TSC_TORQUE_CONTROL;
		pcmd->engine_retarder_command_mode = TSC_OVERRIDE_DISABLED;
		pcmd->brake_command_mode = XBR_NOT_ACTIVE;	
		ppriv->cmd_ebs_dcc = 0.0;

		if (ppriv->run_dist >= stop_distance) { 
			ppriv->execution_state = CLOSED_LOOP_BRAKING;
			ppriv->v_init_brk = current_speed;
		}

		break;
	case CLOSED_LOOP_BRAKING:
                pcmd->engine_command_mode = TSC_OVERRIDE_DISABLED;
		
		if (!brake_tasks(pctrl, pcmd))
			return 0;
		                
		break;
	}

	/* check limits and adjust command values */
        if (ppriv->cmd_eng_trq > engine_reference_torque)
                ppriv->cmd_eng_trq = engine_reference_torque;

        pcmd->engine_torque = ppriv->cmd_eng_trq;

	if (pcmd->engine_command_mode == TSC_OVERRIDE_DISABLED)
		pcmd->acc_pedal_control = ACC_OFF_VOLTAGE;
	else
		pcmd->acc_pedal_control = 
			trq_to_acc_voltage(pctrl, pcmd->engine_torque);


	/* EBS command expects negative deceleration */
        pcmd->ebs_deceleration = -ppriv->cmd_ebs_dcc;

	/* Engine retarder command expects negative torque */
	pcmd->engine_retarder_torque = -ppriv->cmd_rtdr_trq;

	/* Transmission retarder command expects negative percentage */
	pcmd->trans_retarder_value = -ppriv->cmd_trtdr_val;

	if (gather_data) {
		int index = get_circular_index(pbuff);
		struct timeb start_time = pctrl->start_time;
		struct timeb current_time;
		ftime(&current_time);
		current_item.state = *pv;
		current_item.priv = *ppriv;
		current_item.cmd = *pcmd;
		current_item.timestamp =
			 TIMEB_SUBTRACT(&start_time, &current_time);
#if 0
		sprintf(current_item.adhoc, "%.3f", pcparams->max_braking_time);
#endif
		pdata[index] = current_item;
	}	
        return 1;       
}
        

static long_output_typ inactive_ctrl = {
        600.0,  /* engine speed, truck idle, don't care since disabled */
        400.0,  /* engine torque, truck idle, don't care since disabled */
          0.0,  /* retarder torque, don't care since disabled */
        TSC_OVERRIDE_DISABLED,  /* engine command mode */
        TSC_OVERRIDE_DISABLED,  /* engine retarder command mode */
         0.0,   /* accelerator pedal voltage -- not used by jbussend */
         0.0,   /* ebs deceleration */
        XBR_NOT_ACTIVE,        /* brake command mode */
         0.0,   /* percent of maximum transmission retarder activation */
        TSC_OVERRIDE_DISABLED,  /* transmission retarder command mode */
};

/**
 * exit_tasks
 * Performed before logging out of database.
 * Sends deactivate messages and prints out any stored data.
 * Or any other clean-up.
 */
int exit_tasks(db_clt_typ *pclt, long_ctrl *pctrl, long_output_typ *pcmd)
{
	cbuff_typ *pbuff = &pctrl->buff;
	long_params *pparams = &pctrl->params;
	buffer_item *pdata = (buffer_item *) pbuff->data_array;

//	private_params *pcparams = &cmd_private.cmd_params;

        FILE * fp;
	int i;
	int current_index;
	char fname[80];

	/* Deactivate all control, will be written by main program */
	*pcmd = inactive_ctrl;

	sprintf(fname, "%s%d", pparams->data_file, pparams->cmd_test);
        if ((fp = long_get_output_stream (fname)) == NULL) {
                fprintf(stderr, " %s get output stream error\n",
				 fname);
                return 0;
        }
	printf("in exit_task, pbuff->data_count %d, pdata %08x\n",
			pbuff->data_count, (unsigned int)pdata);

	if ((pdata == NULL) || (pbuff->data_count == 0))
		return 1;

	current_index = pbuff->data_start;
	
	for (i = 0; i < pbuff->data_count; i++) {
		buffer_item *p = &pdata[current_index]; 
		long_vehicle_state *pv = &p->state;
		long_private *ppriv = &p->priv;
		long_output_typ *pcmd = &p->cmd;

		fprintf(fp, "%.3f ", p->timestamp);
        	fprintf(fp, "%.3f ", pv->CCVS_VehicleSpeed);
        	fprintf(fp, "%.3f ", pv->EEC1_EngineSpeed);
//        	fprintf(fp, "%.3f ", pv->engine_torque);
//        	fprintf(fp, "%d ", pv->engine_mode);
        	fprintf(fp, "%.3f ", pv->ERC1ERActualEngineRetPercentTrq);
        	fprintf(fp, "%d ", pv->ERC1ERRetarderTorqueMode);
        	fprintf(fp, "%d ", pv->ABSEBSAmberWarningSignal);
        	fprintf(fp, "%d ", pv->EBSBrakeSwitch);
        	fprintf(fp, "%.3f ", pv->EBC1_BrakePedalPosition);
        	fprintf(fp, "%.3f ", pv->EEC2_AccelPedalPos1);
        	fprintf(fp, "%.3f ", pv->EBC2_FrontAxleSpeed);
        	fprintf(fp, "%.3f ", pv->ETC1_TransInputShaftSpeed);
        	fprintf(fp, "%.3f ", pv->ETC1_TransmissionOutputShaftSpeed);
        	fprintf(fp, "%d ", pv->ETC1_TransmissionShiftInProcess);
        	fprintf(fp, "%d ", pv->ETC1_TorqueConverterLockupEngaged);
        	fprintf(fp, "%d ", pv->ETC1_TransmissionDrivelineEngaged);
        	fprintf(fp, "%d ", pv->ETC2_TransmissionCurrentGear);
        	fprintf(fp, "%.3f ", ppriv->cmd_eng_trq);
        	fprintf(fp, "%.3f ", ppriv->cmd_ebs_dcc);
        	fprintf(fp, "%.3f ", ppriv->cmd_rtdr_trq);
        	fprintf(fp, "%.3f ", ppriv->cmd_trtdr_val);
        	fprintf(fp, "%.3f ", ppriv->run_dist);
        	fprintf(fp, "%.3f ", ppriv->trq_cmd_t/1000.0);
        	fprintf(fp, "%.3f ", ppriv->v_init_brk);
        	fprintf(fp, "%.3f ", ppriv->ebs_brk_t);
        	fprintf(fp, "%.3f ", ppriv->trq_fixed);
        	fprintf(fp, "%d ", ppriv->execution_state);
		fprintf(fp, "%.3f ", pcmd->engine_torque); 
		fprintf(fp, "%d ", pcmd->engine_command_mode); 
		fprintf(fp, "%.3f ", pcmd->ebs_deceleration); 
		fprintf(fp, "%d ", pcmd->brake_command_mode); 
		fprintf(fp, "%.3f ", pcmd->engine_retarder_torque); 
		fprintf(fp, "%d ", pcmd->engine_retarder_command_mode); 
		fprintf(fp, "%.3f ", pcmd->trans_retarder_value); 
		fprintf(fp, "%d ", pcmd->trans_retarder_command_mode); 
		fprintf(fp, "%.3f ", pcmd->engine_speed); 
		fprintf(fp, "%.3f ", pv->accelerator_pedal_position);
		fprintf(fp, "%.3f ", pv->acc_pedal_voltage); 
		fprintf(fp, "%.3f ", pv->trans_retarder_value); 
		fprintf(fp, "%f ", pv->trans_retarder_mode); 
		fprintf(fp, "%.3f ", pv->coolant_load_increase); 
		fprintf(fp, "%d ", pv->trans_retarder_source); 
		fprintf(fp, "%.3f ", pv->boost_pressure);
		fprintf(fp, "%.3f ", pv->LFE_EngineFuelRate);
		fprintf(fp, "%.3f ", pv->EBC2_RlativeSpeedFrontAxleRightWheel);
		fprintf(fp, "%.3f ", pv->EBC2_RelativeSpeedFrontAxleLeftWheel);
		fprintf(fp, "%.3f ", pv->EBC2_RlativeSpeedRearAxle1RightWheel);
		fprintf(fp, "%.3f ", pv->EBC2_RelativeSpeedRearAxle1LeftWheel);
		fprintf(fp, "\n");
		current_index++;
		if (current_index == pbuff->data_size)
			current_index = 0;
	}
	return 1;
}
