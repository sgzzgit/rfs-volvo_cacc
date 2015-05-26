/*********************************************************************************************************************
*       
*       to process GPS and postmile for selcting road grade
*        sw =1: using GPS; 2: using local postmile; 
*       
*       
*       
*       
*       
*       
*                                                                                                       Started on 04_17_2011, by xylu
*****************************************************************************************************************/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>                    
#include <string.h>
//#include <conio.h>
//#include  "path_gps_lib.h"
#include  "road_map.h"
#include "veh_long.h"
#include "coording.h"


int rd_grade(int sw, vehicle_info_typ* veh_info_pt , local_gps_typ * gps_pt, road_info_typ * rd_info_pt)
{
   double enu_x, enu_y;     
   int link_id=-1;
   int i;
   static int sect_sw[ROAD_PT]={1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
                                                     // loc_pstml [m]; Grade;	Latitude Start;	Longitude Start;	X Start;	 Y Start
const double map_tbl[ROAD_PT][6]={{0.0, -2.35, 39.362355, -117.373460, 467826.9978, 4357055.2580},
{250.0386, 	-3.08, 39.362686, -117.370645, 468069.6047, 4357090.99},
{977.6284, 	-1.40, 39.365932, -117.363304, 468703.5370, 4357448.6840},
{1157.5146, -0.49, 39.366738, -117.361491, 468860.0532, 4357537.4490},
{1178.1098, -1.64, 39.366829, -117.361283, 468877.98, 		4357547.52},
{1669.9811, -2.88, 39.369336, -117.356575, 469284.6915, 4357824.1490},
{1968.2897, -1.71, 39.370891, -117.353743, 469529.2886, 4357995.7360},
{2418.1661, -0.20, 39.373228, -117.349471, 469898.2614, 4358253.6180},
{2641.3344, -1.25, 39.374385, -117.347356, 470080.9681, 4358381.3960},
{7858.1951, -0.44, 39.401481, -117.297866, 474353.5978, 4361373.2640},
{7964.8718, -0.59, 39.402035, -117.296855, 474440.8752, 4361434.43},
{8252.8828, -0.40, 39.403531, -117.294124, 474676.5706, 4361599.6270},
{8616.9995, -0.53, 39.405422, -117.290670, 474974.6460, 4361808.5390},
{8656.0982, -0.36, 39.405626, -117.2903, 475006.5191, 4361831.05},
{9310.8003, -0.51, 39.409024, -117.284093, 475542.1385, 4362206.5150},
{9333.9699, -0.30, 39.409144, -117.283873, 475561.1221, 4362219.8140},
{11041.1189, -0.51, 39.417997, -117.267675, 476958.5429, 4363197.9930},
{11044.6587, -0.15, 39.418015, -117.267641, 476961.4527, 4363200.0130},
{16912.6817, -0.15, 39.4484175,-117.2119135,481764.3132,4366562.7835},      // manually added middle point
{22780.7047, 1.40, 39.478820, -117.156186, 486567.1737, 4369925.5540} };
   
   // Using postmile to determine; all in double
 if (sw == GRADE_POSTMILE)
 {
	  if (rd_info_pt-> direction == EB)
   	  {
   	  		for (i=1;i<ROAD_PT-1;i++)   // pre start from 0
   	  		{
	   	  		if ( (rd_info_pt-> postmile >= map_tbl[i][0]) && (rd_info_pt-> postmile < map_tbl[i+1][0]) )
	   	  		 rd_info_pt-> grade = atan((map_tbl[i][1])*0.01);   
	  		}
	  }  // EB end
   	  if (rd_info_pt-> direction == WB)
   	  {		
   	  		for (i=(ROAD_PT-1);i>1;i--)    // pre: i>0
   	  		{
	   	  		if ( (rd_info_pt-> postmile >= map_tbl[i-1][0]) && (rd_info_pt-> postmile < map_tbl[i][0]) )
	   	  			 rd_info_pt-> grade = -atan((map_tbl[i-1][1])*0.01);  
	  		}
	  }   // WB end
  }  
  
  if (sw == GRADE_GPS)
  {
    if (rd_info_pt-> direction == EB) 
    {
     for(i=1;i<ROAD_PT-1;i++)  // start from 0
        {
                if( ( (gps_pt-> latitude >= map_tbl[i][2] && gps_pt-> latitude < map_tbl[i+1][2])  ||  (gps_pt-> longitude >= map_tbl[i][3] && gps_pt-> longitude < map_tbl[i+1][3]) ) && (sect_sw[i] == 1))
                {
                        rd_info_pt-> grade = atan((map_tbl[i][1])*0.01);   
                        sect_sw[i]=0;
                }
        }
     }
   if (rd_info_pt-> direction == WB) 
    {
	  for(i=ROAD_PT-1;i > 1;i--)   // pre i>0
        {
             if( ( (gps_pt-> latitude >= map_tbl[i-1][2] && gps_pt-> latitude < map_tbl[i][2])  ||  (gps_pt-> longitude >= map_tbl[i-1][3] && gps_pt-> longitude < map_tbl[i][3]) ) && (sect_sw[i] == 1))
             {
                  rd_info_pt-> grade = -atan((map_tbl[i-1][1])*0.01);  
                  sect_sw[i]=0;
            }
        }
    }
  } 

   return 1;   
 }
           
 
 
