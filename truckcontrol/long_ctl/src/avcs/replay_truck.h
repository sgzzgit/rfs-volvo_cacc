#include <data_log.h>
#include <path_gps_lib.h>
#include <long_comm.h>
#include <long_ctl.h>

extern data_log_column_spec_t file_spec[];
extern int num_file_columns;

extern db_var_spec_t db_vars_ac_rm[];
extern int num_ac_rm_vars;
extern long_vehicle_state my_pv;
extern veh_comm_packet_t pcomm_tx;
extern unsigned char comm_tx_user_bit_3;

extern timestamp_t timestamp;
