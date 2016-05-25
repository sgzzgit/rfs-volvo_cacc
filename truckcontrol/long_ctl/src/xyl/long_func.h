/**\file
 *
 * Function declarations must be in a header file that is also included
 * by the files that define the functions, so that the compiler can
 * check if the types of the arguments are correct.
 */
#ifndef LONG_TRK_FUNC_H
#define LONG_TRK_FUNC_H 

extern int coording(float, float, control_state_typ*, control_config_typ*, switch_typ*, f_mode_comm_typ*, vehicle_info_typ*, pltn_info_typ*, manager_cmd_typ*);
extern int maneuver(float, float, float, float *,float *, float *, road_info_typ*, control_config_typ* , control_state_typ* , vehicle_info_typ* , fault_index_typ *, int, int*, manager_cmd_typ*, pltn_info_typ*);         
extern int dvi(float, switch_typ*, control_state_typ*, vehicle_info_typ*, int*);
extern int control(float, float, int, int*, jbus_read_typ*, manager_cmd_typ*, control_config_typ*, control_state_typ*, switch_typ*, con_output_typ*);    
extern void v_flt( float, float, int, float, float, float, float*, fault_index_typ*);                   
extern float gear_tbl(float);
extern void est_dist( float, control_state_typ*, int*, int*, fault_index_typ*, int);
extern void mag_dist( float,  float, control_state_typ*, float *, float *, float *);
extern int ref_dist(float, float, int*, vehicle_info_typ*, fault_index_typ*, control_state_typ*, control_config_typ*, pltn_info_typ*);  
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
	veh_comm_packet_t *comm_receive_pt, veh_comm_packet_t *comm_send_pt, pltn_info_typ* tmp_pt);
extern int read_sw(long_vehicle_state *, switch_typ*);
extern int read_jbus(float,long_vehicle_state *,long_params *,jbus_read_typ *, switch_typ *);
//extern int read_sens();
//extern int veh_pos(control_config_typ*, comm_info_typ*, vehicle_info_typ*, pltn_info_typ*);
extern int config_sw(int *pread_sw, int *pmanu_auto_sw,
        unsigned short *phandshake_start, int *pread_sw_old, switch_typ *sw_pt,
        long_vehicle_state *pv, vehicle_info_typ* vehicle_info_pt,
        veh_comm_packet_t *comm_receive_pt, unsigned short *pprt1_sw, fault_index_typ* f_ind_pt);
/*extern int process_sigs(float *pdt, float *prun_dist, float *pv, float *pacl,
        float *pacl_old, float *lid_hi_rg, float *lid_hi_rt, int *maneuver_id,
        int *pre_maneuver_id, const int *pradar_sw,
        vehicle_info_typ* vehicle_info_pt, control_config_typ *config,
        control_state_typ* con_state_pt, //evrd_out_typ* evrd_out_pt,
        //ldr_out_typ* ldr_out_pt, evt300_radar_typ *pvor_radar,
        fault_index_typ* f_index_pt, //mdl_lidar_typ *pmdl_lidar, 
	long_params *pparams, float global_time);*/

extern int set_init_leds(db_clt_typ *pclt, unsigned short *pprt1_sw,
        unsigned short *pprt_buff, unsigned short *phandshake_start,
        switch_typ *sw_pt, vehicle_info_typ* vehicle_info_pt,
        veh_comm_packet_t *comm_receive_pt, control_state_typ *con_state_pt,
	manager_cmd_typ *manager_pt, pltn_info_typ* pltn_inf_pt);

extern int set_time_sync(float *pt_ctrl, float *pdt,
        float *ptime_filter, int *pvehicle_pip,
        vehicle_info_typ* vehicle_info_pt, pltn_info_typ*);
extern int actuate(long_output_typ *pcmd, float *pengine_reference_torque,
        int *pvehicle_id, float *pt_ctrl, int *pmaneuver_des, int *maneuver_id,
        con_output_typ* con_output_pt, control_state_typ* con_state_pt,
        control_config_typ* config, control_config_typ* config_pt,
        long_params *pparams, float *pminimum_torque,
        long_output_typ *inactive_ctrl, manager_cmd_typ * manager_cmd_pt, switch_typ *sw_pt);
extern int max_i(int , int);
extern int min_i(int , int);
extern float max_f(float , float);
extern float min_f(float , float);



#endif
