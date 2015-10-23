// Header file for gpio.c

#define DB_DIG_IN_TYPE              289 
#define DB_DIG_OUT_TYPE             290
#define DB_DIG_IN_VAR               DB_DIG_IN_TYPE
#define DB_DIG_OUT_VAR              DB_DIG_OUT_TYPE

#define VERBOSE		1
#define USE_DATABASE	2

#define GPIO_DIG_IN_0	1<<0
#define GPIO_DIG_IN_1	1<<1
#define GPIO_DIG_IN_2	1<<2
#define GPIO_DIG_IN_3	1<<3
#define GPIO_DIG_IN_4	1<<4
#define GPIO_DIG_IN_5	1<<5
#define GPIO_DIG_IN_6	1<<6
#define GPIO_DIG_IN_7	1<<7


typedef struct{
	timestamp_t ts;
	unsigned char dig_in;
} gpio_dig_in_t;

/** Macro to vonvert GPIO dig input
*/

#define GPIO_DIG_IN(val, mask) ((val) & (mask)) ? (1):(0)	/// Return 1 if the condition is TRUE, 0 if the condition if FALSE

extern void print_gpio_dig_in (gpio_dig_in_t signal_in);
