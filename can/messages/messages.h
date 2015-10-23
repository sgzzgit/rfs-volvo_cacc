#ifndef MESSAGES_H
#define MESSAGES_H
#include <stdint.h>


typedef struct
{
    uint8_t front_bar;       // 1-bit 0/1 indicator
    uint8_t track_number;    // 1-bit indicates track 1/2
    int16_t peak_detection;  // 13-bit signed -4096mm -> 4095mm
    int16_t low_speed;       // 13-bit signed -4096mm -> 4095mm
    uint8_t track_detected;  // 1-bit
    uint8_t peak_sensor_id;  // 4-bit 0-9
    uint8_t track_polarity;  // 1-bit
    uint8_t message_counter; // 8-bit 0-255
    uint32_t time_counter;   // 24-bit 0-16777216ms (4.6hr)
} track_measurement_t;

typedef struct
{
    uint8_t bar_status;    // 4-bit
    uint8_t sensor_fault;  // 1-bit 0/1
    uint16_t fault_sensor; // 10-bit 0/1 for each sensor fault
    uint8_t degraded_mode; // 1-bit 0/1
    uint8_t can_reception_fault;  // 1-bit 0/1
    uint8_t secondary_fault_cond; // 4-bit
    uint8_t other_status;         // 4-bit
    uint8_t message_counter;      // 8-bit 0-255
    uint16_t vehicle_speed;       // 8-10 bits
} sensor_status_t;

typedef struct
{
    int16_t longitudinal;     // 16-bit signed -32768->32765
    int16_t lateral;          // 16-bit signed -32768->32765
    int16_t vertical;         // 16-bit signed -32768->32765
    uint8_t message_counter;  // 8-bit 0-255
    uint8_t speed;            // 4-bit?
} raw_data_t;

typedef struct
{
    uint8_t command;             // 4-bit
    uint8_t command_validation;  // 4-bit
    uint8_t command_addresses;   // 2-bit (F+R)
    uint16_t speed;              // 12-bit 0-4096 cm/sec
    uint8_t message_counter;     // 8-bit 0-255
} cc_sensor_command_t;

typedef struct
{
    /* health and states */
    uint8_t counter;
    uint8_t state;
    
    /* dvi */
    uint8_t amber_led, green_led, blue_led, red_led;
    uint8_t sound;
    uint8_t on_switch, off_switch, kill_switch;
    uint8_t audio;

    /* io and commands */
    uint8_t driver_request;
    uint8_t command2cc;
    uint8_t primary_cc;
    
    /* fault */
    uint8_t fault_CAN;
    uint8_t fault_RS232;
    uint8_t fault_dvi;
    uint8_t fault_yawr1;
    uint8_t fault_yawr2;
    uint8_t fault_gps;
    uint8_t fault_fbar;
    uint8_t fault_rbar;
    uint8_t fault_actuator;
    uint8_t fault_hmi2;
    uint8_t fault_cc1;
    uint8_t fault_cc2;
    uint8_t fault_from_peerhmi;
    uint8_t fault_from_cc1;
    uint8_t fault_from_cc2;
    uint8_t fault_ccstate_differ;
    uint8_t fault_cccommand_differ;
    uint8_t fault_hmistate_differ;
    uint8_t fault_hmicommand_differ;
    uint8_t fault_severity;
    uint8_t fault_TBD;
    
    /* state transition and handshaking */
    uint8_t next_state;
    uint8_t peerhmi_reboot;
} hmi_primary_t;

typedef struct
{
    /* control computer state machine */
    uint8_t transition_state;
    uint8_t mission_state;
    uint8_t controller_state;
    uint8_t controller_mode;
    
    /* control authority */
    uint8_t current_role;
    uint8_t requested_role;
    
    /* request and notifications for driver action */
    uint8_t notify_driver;
    
    /* command to steering actuator */
    uint8_t actuator_vibration;
    uint8_t actuator_mode;
    uint16_t steer_d;
    
    uint8_t counter;
    
    uint8_t fault_severity;
    uint8_t fault_fbar, 
	fault_rbar, 
	fault_gps, 
	fault_yawr1, 
	fault_speed, 
	fault_yawr2, 
	fault_actuator, 
	fault_hmi1, 
	fault_hmi2, 
	fault_selfcc, 
	fault_peercc;
    uint8_t fault_CAN1, 
	fault_CAN2, 
	fault_CAN3, 
	fault_CAN4, 
	fault_RS232_1, 
	fault_RS232_2, 
	fault_RS232_3, 
	fault_ethernet;
    uint8_t fault_hmi_differ, fault_cc_differ;
} cc_hmi_primary_t;

#endif
