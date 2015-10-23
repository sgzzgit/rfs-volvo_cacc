/**\file
 *
 * Function declarations must be in a header file that is also included
 * by the files that define the functions, so that the compiler can
 * check if the types of the arguments are correct.
 */
#ifndef LONG_TRK_FUNC_H
#define LONG_TRK_FUNC_H 

extern int coording(float, int, float, control_state_typ*, control_config_typ*, f_mode_comm_typ*, vehicle_info_typ*, manager_cmd_typ*);
extern int maneuver(float, float, float, float *,float *, float *, float, control_config_typ* , control_state_typ* , vehicle_info_typ* , f_mode_comm_typ , int, int*);
extern int dvi(float, switch_typ*, control_state_typ*, vehicle_info_typ*, int*);
extern int control(float, float, int, int*, sens_read_typ*, control_config_typ*, control_state_typ*, switch_typ*, con_output_typ*);    
extern void v_flt( float, float, int, float, float, float, float*, fault_index_typ*);                   
extern float gear_tbl(float);
extern void rad_dist( float, control_state_typ*, int*, int*, fault_index_typ*, int);
extern int vrd_flt(float, evt300_radar_typ*, evrd_out_typ*);
extern int vrd_flt1(float, evt300_radar_typ*, evrd_out_typ*);
extern int ldr_flt(float, float*, float*, ldr_out_typ*);
extern int ldr_flt1(float, float*, float*, ldr_out_typ*);
extern void mag_dist( float,  float, control_state_typ*, float *, float *, float *);
extern int ref_dist(float, float, int*, vehicle_info_typ*, fault_index_typ*, control_state_typ*, control_config_typ*);  
extern int mdl_flt(float, mdl_out_typ, control_state_typ* );            
extern float eng_map( float, float, int, int, int*);  
extern float pich_flt(float);
extern float acc_flt(float);
extern float sig_flt1(float);
extern float sig_flt2(float);
extern void filt_4(float, float, float, float, float, float *, float *);
extern int rdswitch();
extern float svg13(float);
extern float svg25(float);
extern float svg31(float);
extern float trq_to_acc_voltage(long_ctrl *, float);
extern void ref_ini(control_config_typ*, float *, float *, float *);
extern int comm(unsigned short *phandshake_start, int *pvehicle_id,
	float *plocal_time, float *pglobal_time,
	unsigned short *pcomm_prt_sw, unsigned short *pcomm_prt1_sw, 
	unsigned short *psynchrn_sw, unsigned short *pglobal_t_sw, float *pdt, 
	float *ptime_filter, control_state_typ *con_state_pt, 
	vehicle_info_typ *vehicle_info_pt, comm_info_typ *comm_info_pt, 
	fault_index_typ *f_index_pt, unsigned short *pcomm_err_bound, 
	veh_comm_packet_t *comm_receive_pt, veh_comm_packet_t *comm_send_pt);
extern int read_sens(float *gear, int *selected_gear, int *fan_drive_state, 
	float *actual_gear_ratio, float *percent_load_v, float *v1, float *v2, 
	float *v3, float *fl_axle_diff, float *fr_axle_diff, 
	float *rl_axle_diff, float *rr_axle_diff, float *acl, int *eng_mode, 
	float *we, float *fan_spd_est, float *eng_tq, float *nominal_fric_tq, 
	float *fuel_m1, float *fuel_m2, float *fuel_val1_pos, 
	float *fuel_val2_pos, float *acc_pedal_pos, int *jk_mode, float *jk_tq, 
	float *jk_percent_tq, int *ebs_brake_switch, 
	int *abs_ebs_amber_warning_state, float *inst_brk_power, 
	float *brk_pedal_pos, float *brk_demand, float *trans_rtd_mode, 
	float *trans_rtd_value, float *trans_rtd_volt, float *time_filter, 
	float *we_old, long_vehicle_state *pv,
        long_params *pparams, sens_read_typ* sens_read_pt);
extern int config_sw(int *pread_sw, int *pmanu_auto_sw,
        unsigned short *phandshake_start, int *pread_sw_old, switch_typ *sw_pt,
        long_vehicle_state *pv, vehicle_info_typ* vehicle_info_pt,
        veh_comm_packet_t *comm_receive_pt, unsigned short *pprt1_sw, fault_index_typ* f_ind_pt);
extern int process_sigs(float *pdt, float *prun_dist, float *pv, float *pacl,
        float *pacl_old, float *lid_hi_rg, float *lid_hi_rt, int *maneuver_id,
        int *pre_maneuver_id, const int *pradar_sw,
        vehicle_info_typ* vehicle_info_pt, control_config_typ *config,
        control_state_typ* con_state_pt, evrd_out_typ* evrd_out_pt,
        ldr_out_typ* ldr_out_pt, evt300_radar_typ *pvor_radar,
        fault_index_typ* f_index_pt, mdl_lidar_typ *pmdl_lidar, 
	long_params *pparams);

extern int set_init_leds(db_clt_typ *pclt, unsigned short *pprt1_sw,
        unsigned short *pprt_buff, unsigned short *phandshake_start,
        switch_typ *sw_pt, vehicle_info_typ* vehicle_info_pt,
        veh_comm_packet_t *comm_receive_pt);
extern int set_time_sync(float *pt_ctrl, float *pdt,
        float *ptime_filter, int *pvehicle_pip,
        vehicle_info_typ* vehicle_info_pt);
extern int actuate(long_output_typ *pcmd, float *pengine_reference_torque,
        int *pvehicle_id, float *pt_ctrl, int *pmaneuver_des, int *maneuver_id,
        con_output_typ* con_output_pt, control_state_typ* con_state_pt,
        control_config_typ* config, control_config_typ* config_pt,
        long_params *pparams, float *pminimum_torque,
        long_output_typ *inactive_ctrl);
extern int max_i(int , int);
extern int min_i(int , int);
extern float max_f(float , float);
extern float min_f(float , float);



#endif
