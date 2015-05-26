/* FILE
 *   mkprof.c
 *	        Makes profiles, either of step inputs or from recorded
 *		data with manual control, that are read by sndprof to
 *		set the longitudinal output variable.
 *
 *	
 *	Usage: mkprof 
 *		  -c field to increment -i step time
 *		  -v starting value -d step increase
 *		  -u upper bound (time for constant torque
 *	  or: mkprof <trace_data 
 *
 */

#include <sys_os.h>
#include "sys/timeb.h"
#include "timestamp.h"
#include "jbus_extended.h"
#include "j1939.h"
#include "j1939pdu_extended.h"

/* makes a longitudinal command file from data recorded during a manual run
 * that has been processed using awk or matlab to have the indicated
 * column values */

void from_manual()
{
	char buffer[80];
	int hour, min;
	float sec, spd, eng_trq, rtdr_trq, trq, eng_spd, fuel, decel;
	double time, time_base;
	int first_time = 1;
	while (fgets(buffer, 80, stdin)){
		sscanf(buffer, "%d %d %f %f %f %f %f %f %f", &hour, &min, &sec,
			 &spd, &eng_spd, &eng_trq, &rtdr_trq, &fuel, &decel);
		if (first_time){
			time = 0.0;
			time_base = 3600.0*hour + 60.0*min + sec;
			first_time = 0;
		} else
			time = (3600.0*hour + 60.0*min + sec) - time_base;

		if (rtdr_trq < 0.0)
			trq = MAX_RETARDER_TORQUE * rtdr_trq;
		else 
			trq = MAX_ENGINE_TORQUE * eng_trq; 

		trq /= 100.0;

		printf("%.3lf %.6f %.6f %.6f %.6f %6f\n",
			time, spd, eng_spd, trq, fuel, decel);
	}
}

/* creates step inputs for either speed or torque */

void step_input(int field_to_increment, float start_val, float step_increase,
		 float step_time, float upper_bound)
{
	double time, spd, eng_spd, trq, fuel, decel;
	double total_time;
	
	/* initialize to idle values, only field being stepped will change */ 
	spd = 0.0;
	eng_spd = 600.0;	/* spd and trq from truck samples, idle */
	trq = 450.0;	 
	fuel = 1.4;
	decel = 0.0;		
	

	if (step_increase == 0.0) 
		total_time = upper_bound;
	else
		total_time =
			 (upper_bound-start_val)/step_increase * step_time;

	switch (field_to_increment) {
	case 1:	eng_spd = start_val;
		break; 
	case 2: trq = start_val;
		break;
	case 3: decel = start_val;
		break;
	default: printf("unrecognized field to increment\n");
		return;
	}

	for (time = 0.0; time < total_time; time += step_time) {
		printf("%.3lf %.2f %.2f %.2f %.2f %.2f\n",
				 time, spd, eng_spd, trq, fuel, decel);
		if (step_increase > 0.0) {
			switch (field_to_increment) {
			case 1:	eng_spd += step_increase;
				if (eng_spd >upper_bound)	/* rounding error? */
					time = total_time + 1;
				break; 
			case 2: trq += step_increase;
				if (trq >upper_bound)	/* rounding error? */
					time = total_time + 1;
				break;
			case 3: decel += step_increase;
				if (decel >upper_bound)	/* rounding error? */
					time = total_time + 1;
				break;
			default: printf("unrecognized field to increment\n");
				break;
			} 
		}
	}
}

int
main(int argc, char **argv)

{
	int ch;
	int field_to_increment = 0;
	float step_time, upper_bound, start_val, step_increase;

        while ((ch = getopt(argc, argv, "c:d:hi:u:v:")) != EOF) {
                switch (ch) {
		case 'c': field_to_increment = atoi(optarg);
			  break;
		case 'd': step_increase = atof(optarg);
			  fprintf(stderr, "step_increase %.3f\n", step_increase);
			  break;
		case 'i': step_time = atof(optarg);
			  fprintf(stderr, "step_time %.3f\n", step_time);
			  break;
		case 'u': upper_bound = atof(optarg);
			  fprintf(stderr, "upper_bound %.3f\n", upper_bound);
			  break;
		case 'v': start_val = atof(optarg);
			  fprintf(stderr, "start_val %.3f\n", start_val);
			  break;
		case 'h':
                default:  printf( "Usage: %s ", argv[0]);
			  printf("-c field to increment -i step time\n ");
			  printf("\t\t-v starting value -d step increase");
			  printf("-u upper bound (time for constant torque\n");
			  printf(" or: %s <trace\n", argv[0]);
			  exit(1);
		}
	}
	/* if no field specified, create profile from stdin */
	if (field_to_increment)
		step_input(field_to_increment, start_val, step_increase,
		 step_time, upper_bound);
	else
		from_manual();		
	return 0;
}
