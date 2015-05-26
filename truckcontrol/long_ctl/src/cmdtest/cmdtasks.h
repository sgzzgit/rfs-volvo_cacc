/* FILE
 *   cmdtasks.h
 *
 * Structures and includes for command testing control programs
 *
 */

#ifndef CMDTASKS_H
#define CMDTASKS_H

/**
 * private_params 
 * Structure includes initialization conditions
 * and parameters for the current controller and the current
 * test. These will not change throughout the control, and are set
 * once at the beginning by reading the long_cfg_file initialization
 * file.
 */
typedef struct{
  /* parameters for type of test */
  int gather_data;			/* > 0 gather data, 0 no */
  int cmd_test;		/* test number used to read long_ctl.ini */
  int acc_task_type;	/* type of acceleration control */
  int brk_task_type;	/* type of braking control*/
  /* parameters for issuing constant command acceleration control*/
  int engine_command_mode;
  float engine_speed;
  float engine_torque;
  /* parameters for simple P-type acceleration control */
  float desired_vehicle_speed; /* increase torque until reached */ 
  float init_torque;	/* initial torque value during open loop */
  float delta_torque;     /* used to change cmd_torque during open loop */
  float initial_reduction_trq;  /* cut down at start of vehicle speed maintenance */
  float trq_time_limit;   /* desired time to reach target vehicle speed */
  float stop_distance;    /* maximum distance to travel before braking */
  float control_gain;     /* used in control calculations */
  float initial_stop_time;	/* time to wait with air brake at max */
  /* parameters for braking control */
  float max_braking_time;		/* time limit to brake */ 
  float ebs_deceleration;	/* specified EBS deceleration parameter */
  float engine_retarder_torque;	/* specified engine retarder torque (N-m) */ 
  float trans_retarder_value;	/* specified transmission retarder value (%) */ 
} private_params;

/**
 * long_private
 * Structure specific to the controller used for command testing.
 * This structure includes values that will be updated during control
 * execution by run_tasks and used in later control calculations.
 * They are kept in a structure, rather than as static variables, to make
 * it easy to pass them to subroutines, as the control code becomes more
 * complex, and to pass them to debugging routines that check or display
 * their values.
 */

typedef struct {
  private_params cmd_params; /* private long_ctl parameters */
  float cmd_eng_trq;      /* engine torque commanded each cycle */
  float cmd_ebs_dcc;      /* air brake deceleration commanded */
  float cmd_rtdr_trq;	/* engine retarder torque commanded */ 
  float cmd_trtdr_val;	/* transmission retarder percent of maximum */ 
  float run_dist;         /* calculated distance traveled */
  float trq_cmd_t;        /* time elapsed issuing torque commands */
  float v_init_brk;       /* velocity when braking began */
  float trq_fixed;	/* torque at end of open loop */
  float ebs_brk_t;	/* time elapsed since braking began */
  int execution_state;    /* state of test */
} long_private;

/**
 * buffer_item
 * Type of data stored in long_data_buffer. This can be different
 * for each controller. 
 */

typedef struct {
  double timestamp;
  long_vehicle_state state;
  long_private priv;
  long_output_typ cmd;
  char adhoc[80];
} buffer_item;


/* constant definitions for cmdtest */
#define LOW_VEHICLE_SPEED 0.5
#define MAX_EBS_DECELERATION 1.99

/* Execution states */
#define ISSUE_CONSTANT_COMMAND	1
#define INITIAL_OPEN_LOOP       2
#define CLOSED_LOOP_TORQUE      3
#define CLOSED_LOOP_BRAKING     4
#define INITIAL_FULL_STOP	5
#define DO_VARY_TORQUE		6

/* Acceleration task types */
#define CONSTANT_COMMAND	1
#define SIMPLE_P_CONTROL	2
#define VARY_TORQUE		3

/* Braking task types */
#define EBS_NOBRAKE	0
#define EBS_CONSTANT    1
#define EBS_SINUSOIDAL  2
#define ENG_RTDR_CONSTANT	3
#define ENG_RTDR_SINUSOIDAL	4
#define TRANS_RTDR_CONSTANT	5
#define TRANS_RTDR_SINUSOIDAL	6


#endif /* CMDTASKS_H */
