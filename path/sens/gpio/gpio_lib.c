/**\file gpio_lib.c
* Copyright (c) 2009 Regents of the University of California
*
* Process to check gpio digital input.
*
*/

#include <sys_os.h>	/// In path/local, OS-dependent difinitions
#include <sys_rt.h>	/// In path/local, "real-time" definitions
#include <timestamp.h>	/// In path/local, for timestamps hh:mm:ss.sss
#include "gpio.h"


void print_gpio_dig_in (gpio_dig_in_t signal_in){

	print_timestamp(stdout, &signal_in.ts);

	printf("%d ", GPIO_DIG_IN(signal_in.dig_in, GPIO_DIG_IN_0 ));
	printf("%d ", GPIO_DIG_IN(signal_in.dig_in, GPIO_DIG_IN_1 ));
	printf("%d ", GPIO_DIG_IN(signal_in.dig_in, GPIO_DIG_IN_2 ));
	printf("%d ", GPIO_DIG_IN(signal_in.dig_in, GPIO_DIG_IN_3 ));
	printf("%d ", GPIO_DIG_IN(signal_in.dig_in, GPIO_DIG_IN_4 ));
	printf("%d ", GPIO_DIG_IN(signal_in.dig_in, GPIO_DIG_IN_5 ));
	printf("%d ", GPIO_DIG_IN(signal_in.dig_in, GPIO_DIG_IN_6 ));
	printf("%d ", GPIO_DIG_IN(signal_in.dig_in, GPIO_DIG_IN_7 ));

	printf("\n");

}
