/*	FILE
 *	static char rcsid[] = "$Id: sys_arma.h 625 2006-07-24 07:11:41Z dickey $";
 *
 * Copyright (c) 1996   Regents of the University of California
 *
 *	
 * Revision 1.3  1996/10/17  23:22:06  path
 * Add copyright notice
 *
 * Revision 1.2  1995/06/20  05:50:08  lchen
 * Add documentation.
 *
 * Revision 1.1  1994/11/16  22:18:01  lchen
 * Initial revision
 *
 */

typedef struct
{
	double coef_y1;
	double coef_x0;
	double coef_x1;
	int bound_type;
	double lower_bound;
	double upper_bound;
	double x1;
	double y1;
} arma_11_typ;

#define ARMA_BOUND_NONE				1	/*	No input or output restriction.	*/
#define ARMA_BOUND_INPUT_ABS		2	/*	Absolute restriction on input.	*/
#define ARMA_BOUND_INPUT_REL		3	/*	Relative restriction on input.	*/
#define ARMA_BOUND_OUTPUT_ABS		4	/*	Absolute restriction on output.	*/
#define ARMA_BOUND_OUTPUT_REL		5	/*	Relative restriction on output.	*/

arma_11_typ *arma_11_init( double coef_y1, double coef_x0, double coef_x1,
    int bound_type, double lower_bound, double upper_bound, double old_value );

void arma_11_set_value( arma_11_typ *pfilt, double value ); 
double arma_11_get_value( arma_11_typ *pfilt, double value ); 
bool_typ arma_11_done( arma_11_typ *pfilt );

