/* FILE
 *   tasks.h
 *
 * Structures and includes for template project
 *
 */

#ifndef TASKS_H
#define TASKS_H

/**
 * valid states of execution
 */
enum ol_state{
  IDLE,
  RUN
};

/**
 * private_params 
 * Structure includes initialization conditions
 * and parameters for the current controller and the current
 * test. These will not change throughout the control, and are set
 * once at the beginning by reading the long_cfg_file initialization
 * file.
 */
#define MAX_PARAM_NAME 10

typedef struct{
  int num_integer;	    
  char string[MAX_PARAM_NAME]; /* filename of raw input command */
  float num_double;
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
  private_params params; /* private long_ctl parameters */

  int execution_state; /* current state of execution */
  int gather_data;     /* number of data points to collect */
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
//  char adhoc[80];
} buffer_item;

#endif /* TASKS_H */
