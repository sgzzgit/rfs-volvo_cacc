/**\file
 *
 * Function declarations must be in a header file that is also included
 * by the files that define the functions, so that the compiler can
 * check if the types of the arguments are correct.
 */
#ifndef LONG_TRK_FUNC_H
#define LONG_TRK_FUNC_H 

extern int coording(float, float, control_state_typ*, sens_read_typ*, control_config_typ*, switch_typ*, f_mode_comm_typ*, vehicle_info_typ*, pltn_info_typ*, manager_cmd_typ*);
extern int maneuver(float, float, float, float *,float *, float *, road_info_typ*, control_config_typ* , control_state_typ* , sens_read_typ*, switch_typ*, vehicle_info_typ* , fault_index_typ *, /*int,*/ int*, manager_cmd_typ*, pltn_info_typ*);         
extern int dvi(float, switch_typ*, control_state_typ*, vehicle_info_typ*, int*);
extern int control(float, float, int*, jbus_read_typ*, manager_cmd_typ*, control_config_typ*, control_state_typ*, switch_typ*, vehicle_info_typ*,con_output_typ*);    
extern int cacc(float, int*, jbus_read_typ*, manager_cmd_typ*, control_config_typ*, control_state_typ*, switch_typ*, vehicle_info_typ*,con_output_typ*);    
extern void v_flt( float, float, /*int,*/ float, float, float, float*, fault_index_typ*);                   
extern float gear_tbl(float);
extern void est_dist( float, sens_read_typ*, control_state_typ*, int*, int*, fault_index_typ*, manager_cmd_typ*);
extern void mag_dist( float,  float, sens_read_typ*, control_state_typ*, float *, float *, float *);
extern int ref_dist(float, int*, sens_read_typ*, vehicle_info_typ*, manager_cmd_typ*, fault_index_typ*, control_state_typ*, control_config_typ*, pltn_info_typ*);  
//extern int ref_dist(float, int*, vehicle_info_typ*, manager_cmd_typ*, fault_index_typ*, control_state_typ*, control_config_typ*, pltn_info_typ*);
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
extern int cacc_comm(float, float *, float, control_state_typ*, vehicle_info_typ*, comm_info_typ*, fault_index_typ*, 
				veh_comm_packet_t*, veh_comm_packet_t*, pltn_info_typ*);
extern int read_sw(long_vehicle_state *, switch_typ*);
extern int read_jbus(float, float,long_vehicle_state *,long_params *,jbus_read_typ *, sens_read_typ*, switch_typ *, vehicle_info_typ*);
//extern int read_sens();
//extern int veh_pos(control_config_typ*, comm_info_typ*, vehicle_info_typ*, pltn_info_typ*);
extern int config_sw(int *pread_sw, int *pmanu_auto_sw,
        unsigned short *phandshake_start, int *pread_sw_old, switch_typ *sw_pt,
        long_vehicle_state *pv, vehicle_info_typ* vehicle_info_pt,
        veh_comm_packet_t *comm_receive_pt, unsigned short *pprt1_sw, fault_index_typ* f_ind_pt);

//extern int set_init_leds(db_clt_typ*, unsigned short*, unsigned short*, /*unsigned short*,*/
//        switch_typ*, vehicle_info_typ*,veh_comm_packet_t*, control_state_typ*,manager_cmd_typ*, pltn_info_typ*);

//extern int set_time_sync(float *, float *,float *, vehicle_info_typ*, pltn_info_typ*);
extern int actuate(float, long_output_typ*, con_output_typ*, control_state_typ*, long_params*, long_output_typ*, 
				          manager_cmd_typ*, switch_typ*, jbus_read_typ*, control_config_typ*, fault_index_typ*);
extern int tq_we(float, float*);
extern int max_i(int , int);
extern int min_i(int , int);
extern float max_f(float , float);
extern float min_f(float , float);



#endif
