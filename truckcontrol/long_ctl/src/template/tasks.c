/**\ 
 *	 tasks.c		
 *
 * User defined function to run tasks
 *
 * This template is kind of a hack originally devised by Paul Kretz
 * to help control engineers with initialization and data gathering
 * tasks for their controllers. 
 *
 */

#include "long_ctl.h"
#include "tasks.h"

/**
 * print_params prints long_params and private_params structures, 
 * for documentation or debugging. 
 */
void print_params(FILE *fp, long_params *pparams, private_params *ppparams)
{
	fprintf(fp, "avcs_cfg_file %s\n", pparams->avcs_cfg_file);
	fprintf(fp, "long_cfg_file %s\n", pparams->long_cfg_file);
	fprintf(fp, "double %f\n", ppparams->num_double);
	fprintf(fp, "integer %d\n", ppparams->num_integer);
	fprintf(fp, "string %s\n", ppparams->string);

}

static long_private private;

/**
 * init_tasks
 * Initialization at the beginning of a run of the controller
 */
int init_tasks(db_clt_typ *pclt, long_ctrl *pctrl, long_output_typ *pcmd)
{
	long_params *pparams = &pctrl->params;
	cbuff_typ *pbuff = &pctrl->buff;

	private_params *ppparams = &private.params;

	FILE *pfin;		/* file pointer for initialization files */

	/* Read from long_ctl specific ini file */
	if ((pfin = get_ini_section(pparams->long_cfg_file, "template")) == NULL) {
		fprintf(stderr, "Cannot get ini file %s, section template\n",
		 pparams->long_cfg_file); 
		return 0;
	}

	/* Examples of how to read from long_cfg ini file */
//	ppparams->num_integer = get_ini_long(pfin, "integer", 30000);
//	ppparams->num_double = get_ini_double( pfin, "double", 1L );
//	pst = get_ini_string( pfin, "string", NULL );
//	strcpy(ppparams->string,pst);

	/* Initialize remaining long_private structure */
	private.execution_state = IDLE;
	private.gather_data = 30000;

	pctrl->plong_private = &private;

	/* Initialize data buffer */
	init_circular_buffer(pbuff, private.gather_data, sizeof(buffer_item));

	ftime(&pctrl->start_time);

	print_params(stdout, pparams, ppparams);

	return 1;
}

/**
 * run_tasks
 * Whatever you want to do: in this case, run open-loop control for
 * system identification
 */
int run_tasks(db_clt_typ *pclt, long_ctrl *pctrl, long_output_typ *pcmd)
{
	long_vehicle_state *pv = &pctrl->vehicle_state;
	long_params *pparams = &pctrl->params;
	cbuff_typ *pbuff = &pctrl->buff;
	buffer_item *pdata = (buffer_item *) pbuff->data_array;
	long_private *ppriv = (long_private *) pctrl->plong_private;
	private_params *ppparams = &ppriv->params;

	/* copy unchanging parameters to local variables for easy reference */
	int current_state = ppriv->execution_state;
	int gather_data = ppriv->gather_data;
 
	/* local variables */
	float delta_t = 0;
	struct timeb current_time;
	buffer_item current_item;
	
	if (!long_read_vehicle_state(pclt, pctrl))
		return 0;

	/* Update time elapsed since beginning of open loop control */
	ftime (&current_time);
	delta_t = TIMEB_SUBTRACT(&pctrl->start_time, &current_time) / 1000.0; 

	switch (current_state) {
	case IDLE:
		if (delta_t >= 1){
			ppriv->execution_state = RUN;
		}
		break;

	case RUN:
		break;	

	default:
		break;
	}


	/* add current info to buffer if gathering data */
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
		pdata[index] = current_item;
	}	
	return 1;			 
}
				
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

	private_params *ppparams = &private.params;

	FILE * fp;
	int i;
	int current_index;
	char fname[80];

	sprintf(fname, "%s", pparams->data_file);
	if ((fp = long_get_output_stream (fname)) == NULL) {
		fprintf(stderr, " %s get output stream error\n",
			fname);
		return 0;
	}

	if ((pdata == NULL) || (pbuff->data_count == 0))
		return 1;

	current_index = pbuff->data_start;
	
	for (i = 0; i < pbuff->data_count; i++) {
		buffer_item *p = &pdata[current_index]; 
		long_vehicle_state *pv = &p->state;
		long_private *ppriv = &p->priv;
		long_output_typ *pcmd = &p->cmd;

		fprintf(fp, "%.3f ", p->timestamp);
		fprintf(fp, "%.3f ", pv->vehicle_speed);
		fprintf(fp, "%.3f ", pv->engine_speed);
		fprintf(fp, "%.3f ", pv->engine_torque);
		fprintf(fp, "%.3f ", pv->brake_pedal_position);
		fprintf(fp, "%.3f ", pv->front_axle_speed);
		fprintf(fp, "%.3f ", pv->input_shaft_speed);
		fprintf(fp, "%.3f ", pv->output_shaft_speed);
		fprintf(fp, "%d ", pv->shift_in_progress);
		fprintf(fp, "%d ", pv->lockup_engaged);

		fprintf(fp, "%d ", pv->driveline_engaged);
		fprintf(fp, "%d ", pv->current_gear);
		fprintf(fp, "%d ", pv->selected_gear);
		fprintf(fp, "%.3f ", pv->actual_gear_ratio);
		fprintf(fp, "%.3f ", pv->accelerator_pedal_position);
		fprintf(fp, "%.3f ", pv->rear1_left_wheel_relative);
		fprintf(fp, "%.3f ", pv->rear1_right_wheel_relative);
		fprintf(fp, "%d ", pv->fan_drive_state);
		fprintf(fp, "%.3f ", pv->fuel_rate);
		fprintf(fp, "%.3f ", pv->nominal_friction_torque);

		fprintf(fp, "%.3f ", pv->actual_retarder_percent_torque);
		fprintf(fp, "%.3f ", pv->boost_pressure);
		fprintf(fp, "%.3f ", pv->estimated_percent_fan_speed);
		fprintf(fp, "%.3f ", pv->percent_load_current_speed);
		fprintf(fp, "%d ", pv->engine_mode);
		fprintf(fp, "%.3f ", pv->fuel_flow_rate1);
		fprintf(fp, "%.3f ", pv->fuel_flow_rate2);
		fprintf(fp, "%.3f ", pv->fuel_valve1_position);
		fprintf(fp, "%.3f ", pv->fuel_valve2_position);
		fprintf(fp, "%.3f ", pv->exhaust_gas_pressure);

		fprintf(fp, "%.3f ", pv->rack_position);
		fprintf(fp, "%.3f ", pv->natural_gas_mass_flow);
		fprintf(fp, "%.3f ", pv->instantaneous_brake_power);
		fprintf(fp, "%d ", ppriv->execution_state);
		fprintf(fp, "%f ", pcmd->acc_pedal_control); 
		fprintf(fp, "%.3f ", pcmd->engine_speed); 
		fprintf(fp, "%.3f ", pcmd->engine_torque); 
		fprintf(fp, "%d ", pcmd->engine_command_mode); 
		fprintf(fp, "\n");
		current_index++;
		if (current_index == pbuff->data_size)
			current_index = 0;
	}
	return 1;
}
