/*
 * jbussend.h	Header file for jbussend.c 
 *
 * Copyright (c) 2002   Regents of the University of California
 *
 */
#include <sys_os.h>
#include <local.h>
#include <j1939scale.h>


#define DB_VOLVO_GPS_CAN_TYPE		2345
#define DB_VOLVO_GPS_CAN_VAR		DB_VOLVO_GPS_CAN_TYPE
#define DB_ENGINE_DEBUG_TYPE		2346
#define DB_ENGINE_DEBUG_VAR		DB_ENGINE_DEBUG_TYPE
#define DB_ENGINE_RETARDER_DEBUG_TYPE	2347
#define DB_ENGINE_RETARDER_DEBUG_VAR	DB_ENGINE_RETARDER_DEBUG_TYPE

typedef struct {
        unsigned int sof: 1; //Start-of-frame = 0
        unsigned int gps_position_pdu: 11; // =0x0010, 0x0011, 0x0012
        unsigned int rtr: 1 ; //Remote transmission request=0
        unsigned int extension: 1; //Extension bit=0
        unsigned int reserved: 1; //Reserved bit=0
        unsigned int dlc: 4;    //Data length code=8
        unsigned int latitude : 31;     //Max=124.748, Min=-90.000, resolution=1E-07, offset=-90.000
        unsigned int pos_data_valid: 1;
        unsigned int longitude : 32; ;  //Max=249.497, Min=-180.000, resolution=1E-07, offset=-180.000
        unsigned int crc: 15;           //Cyclic redundancy check. Calculated
        unsigned int crcd: 1;           //CRC delimiter=1
        unsigned int ack_slot: 1;       //Acknowledge slot=1
        unsigned int ack_delimiter: 1;  //Acknowledge delimiter=1
        unsigned int eof:7;             //End-of-frame=0x07
} IS_PACKED gps_position_t;

typedef struct {
        unsigned short gps_time_pdu; // =0x0013, 0x0014, 0x0015
        unsigned int year :12;  //Max=4095, Min=0, resolution=1, offset=0
        unsigned int month :4;  //Max=15, Min=0, resolution=1, offset=0
        unsigned int day :9;    //Max=511, Min=0, resolution=1, offset=0
        unsigned int hour :5;   //Max=31, Min=0, resolution=1, offset=0
        unsigned int minute :6; //Max=63, Min=0, resolution=1, offset=0
        unsigned int second :6; //Max=63, resolution=1, offset=0
//      unsigned int split_second :10, :12;     //Max=102.3, Min=0, resolution=0.1, offset=0
        unsigned int split_second :10, :4;      //Max=102.3, Min=0, resolution=0.1, offset=0
        unsigned int id :8;     //74, 75, or 76
} IS_PACKED gps_time_t;

unsigned long LAT_MULTIPLIER=10E7;
unsigned long LAT_MIN   =0;
unsigned long LAT_MAX   =(unsigned int)0x7FFFFFFF;
unsigned long LAT_OFFSET=-90;

unsigned long LONGITUDE_MULTIPLIER=10E7;
unsigned long LONGITUDE_MIN=0;
unsigned long LONGITUDE_MAX=(unsigned int)0xFFFFFFFF;
unsigned long LONGITUDE_OFFSET=-180;

unsigned long POS_DATA_IS_VALID=0x10000000;
unsigned long POS_DATA_INVALID=0x7FFFFFFF;

typedef struct {
        unsigned short can_debug_pdu; // =0x0016 for engine, 0x0017 for engine retarder
        unsigned int EnOvrdCtrlM :2;
        unsigned int EnRSpdCtrlC :2;
        unsigned int EnOvrdCtrlMPr :2;
        unsigned int new_mode :2;
        unsigned int engine_torque :8;
        unsigned int last_engine_torque :8;
        unsigned int EnRTrqTrqLm :8;
        unsigned int state_change_counter :2;
        unsigned int ret :1, :5;
} IS_PACKED can_debug_t;

