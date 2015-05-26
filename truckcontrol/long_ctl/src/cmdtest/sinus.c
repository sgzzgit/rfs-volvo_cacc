#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define PI						3.14159265358979L

/**
 * sinusoidal
 * Returns a floating point value as a function of the current time,
 * period, and maximum value. 
 */

static float sinusoidal(float current_time, float period, float max_value)
{
	return (max_value *
		fabs(0.5 * sin(current_time * 15.0 * PI/period +0.5*PI)-0.5));  
}

int main(int argc, char **argv)
{
	float max_value = 300.0;
	float period = 4.0;
	float t;
	
	for (t = 0; t < 60; t+=1.0) {
		printf("%f %f\n", t, sinusoidal(t, period, max_value));
	}
} 	
