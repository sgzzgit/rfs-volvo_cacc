#ifndef _DATASTRUCTS_H
#define _DATASTRUCTS_H
#include <stdint.h>

typedef struct
{
    uint8_t serial;
    uint8_t type;
    uint8_t configuration;
} sensor_config;

typedef struct
{
    uint8_t operation_code;
    uint8_t fault_message;
    uint16_t sensor_health[3];
    uint8_t output_type;
} sensor_state;

typedef struct
{
    uint8_t heartbeat;
} message_status;

typedef struct
{
    uint16_t lateral_pos[6];
    uint16_t time_stamp[6];
    uint8_t polarity[6];
    uint8_t track_number[3];
    uint16_t estimated_speed;
    uint16_t real_speed;
    uint8_t missing_magnet_flag[3];
} position_data_normal;

typedef struct
{
    uint32_t magnetic_strengths[30];
    uint32_t vehicle_speed;
} position_data_raw;

typedef struct
{
    uint32_t magnetic_strengths[30];
} position_data_calibration;

typedef struct
{
    uint8_t id;
    uint8_t status;
} control_computer_status;

typedef struct
{
    uint8_t command;
    uint8_t target;
} system_command;

typedef struct
{
    uint32_t vehicle_speed;
    uint32_t magnet_information;
} data_inputs;

typedef struct
{
    uint16_t id;
    uint8_t operation_state;
    uint8_t heartbeat;
    uint8_t fault_message;
} HMI_state;

typedef struct
{
    uint8_t state;
    uint16_t devices;
} HMI_device_state;

typedef struct
{
    uint32_t recommended_action;
} HMI_optional_data;

typedef struct
{
    uint8_t id;
    uint8_t state;
    uint8_t heartbeat;
    uint32_t fault_message;
} CC_state;

typedef struct
{
    uint8_t controller_state;
    uint8_t transition_state;
    uint8_t coordination_state;
    uint8_t reserved_state;
} CC_operation_state;

typedef struct
{
    uint16_t steering_command;
} CC_optional_data;

#endif
