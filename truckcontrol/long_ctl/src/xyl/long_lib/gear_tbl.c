/*********************************************************
      gear_tbl()   
      
      Gear ratio based on CalTran Doc from Cummins    
      for new Freightliners
	 Error corrected: # gears 4 ==> 6 on 04_22_15
      
                        07/18/01            XYLU 
*********************************************************/

#include <stdio.h>
#include <math.h>

float gear_tbl(float b)
{
        float a;
        int i;
        const int N_gr=6;

        static float index[6] = {1.0,    2.0,     3.0,    4.0,   5.0,    6.0};      /* gear position */
        static float value[6] = {0.2849, 0.5246, 0.6998, 1.000, 1.3569, 1.5650};    /* gear ratio    */


        if( b < index[0] )
                a=value[0];
        else if( b > index[N_gr-1] )
                a=value[N_gr-1];
        else
        {
           for( i=0; i<N_gr; i++ )
			{
               if(fabs(b-index[i]) < 0.01)                  
					a = value[i];
			}
                
        }

		return a ;
}
