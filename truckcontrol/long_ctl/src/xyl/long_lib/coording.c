/***********************************************************************************************

    coording()

    To determine   mng_cmd_pt-> man_des  based on cnfg_pt: task and run values;
    Only run on the leader car or in coordinastion layer
    Its output to be sent to other vehicles by communication

    But for preliminary tests, it is run on the leader car only.
    All the simulation code should be inserted here.
    
    Begin 03/24/03
    Tested for truck joining and splitting on 10_31_03 at CRO for 25[mph]
    

    Maneuver start afetr 5[s] cruising                                                                  12_02_03
    Other task have been added to the case of  site=NVD;                            03_28_11
    test_site has been removed for general use;                                              04_12_11

    By XY_Lu 



***********************************************************************************************/

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "veh_long.h"
#include "coording.h"

int coording(float delta_t,  float track_len, control_state_typ* con_st_pt, control_config_typ* cnfg_pt,
              f_mode_comm_typ* f_mode_cmm_pt, vehicle_info_typ* veh_info_pt, manager_cmd_typ* mng_cmd_pt)

{
    
     static float cruise_t=0.0, splt_tmp_t=0.0, jn_tmp_t=0.0, man_t=0.0;
     

     static float global_time=0.0, split_t=0.0, join_t=0.0, temp=0.0;
     static unsigned short j=0, split_sw=1, join_sw=1,  split_t_sw=1, join_t_sw=1, f_manage_2_sw=1, f_manage_3_sw=1, man_t_sw=0;
     //static int split_sw=1, join_sw=1,  split_t_sw=1, join_t_sw=1, f_manage_2_sw=1, f_manage_3_sw=1;                 // Only do once per run
     static int man_des_29_sw=1;
     
     
     global_time += delta_t;         // for synchronization of all the vehicles     
     mng_cmd_pt-> global_t = global_time;
      

      // Check if get hand-shaking
  
     if ((mng_cmd_pt-> f_manage_index != 2) && (mng_cmd_pt-> f_manage_index != 3))
     {
           if (veh_info_pt-> veh_id == 1)
              {              
                 if (global_time<=3.0*delta_t)   
                    mng_cmd_pt-> man_des = 0;
                 else if (global_time <= t_wait)               // Get hand-shake info if more than one vehicle
                    mng_cmd_pt-> man_des = 1;
                 else if ((veh_info_pt-> run_dist) <= (mng_cmd_pt-> stop_dist) )
                     {
                          if (con_st_pt-> lead_v < cnfg_pt-> max_spd-v_threshold)                        
                            mng_cmd_pt-> man_des = 3;                                   
                         else if (con_st_pt-> lead_v > cnfg_pt-> max_spd+v_threshold)                                                 
                            mng_cmd_pt-> man_des = 27;  
                         else
                         {
	                         if ((veh_info_pt-> man_id1 == 3) ||(veh_info_pt-> man_id1 == 27)  )
                         	 	mng_cmd_pt-> man_des = 7;   
                     	}
                     }
                 else //if( (veh_info_pt-> run_dist) > (mng_cmd_pt-> stop_dist) )
                    mng_cmd_pt-> man_des = 29;
              }
           else //if (veh_info_pt-> veh_id >= 2)
              {
                 if (global_time<=3.0*delta_t)   
                    mng_cmd_pt-> man_des = 0;
                 if (global_time <= t_wait)               // Get hand-shake info if more than one vehicle
                    mng_cmd_pt-> man_des = 1;
                 else if ((veh_info_pt-> run_dist) <= (mng_cmd_pt-> stop_dist) )   // This part needs refining       // Dec. 01, 03
                     {
                           if (con_st_pt-> lead_v < cnfg_pt-> max_spd-v_threshold)  
                                {                      
                                	mng_cmd_pt-> man_des = 3;     
                                	cruise_t =0.0;
                            	}                              
                           else if (con_st_pt-> lead_v > cnfg_pt-> max_spd+v_threshold)   
                               {                                              
                               	    mng_cmd_pt-> man_des = 27;  
                               	    cruise_t =0.0;
                           	    }
                           else
                               	{
	                         		if ((veh_info_pt-> man_id1 == 3) || (veh_info_pt-> man_id1 == 27)  )
                         	 			mng_cmd_pt-> man_des = 7;   
                     	        }
                     	 if (veh_info_pt->  man_id1 == 7)   
                     	 {
                         	  cruise_t += delta_t;
                         	  man_t_sw =1;
                     	  }
                     	  if (  man_t_sw == 1)
                     	  	man_t+= delta_t;
                     	  
                         	
                         if (cnfg_pt-> task == 3)                                     // Splitting;  
                            {
                               if (cruise_t > CRUISE_T && split_sw == 1)       
                               {                                 
                                  mng_cmd_pt-> man_des = 8;   // tested for 8, 6     
                                  split_sw =0;   
                                  //con_st_pt-> des_f_dist=DES_FOLLOW_DIST+SPLIT_DIST;                        
                               }   
                               if ( mng_cmd_pt-> man_des == 8)
                               		con_st_pt-> des_f_dist=DES_FOLLOW_DIST+con_st_pt-> man_dist_var3-con_st_pt-> man_dist_var2;                  // added on 04/16/11
                               if  ((mng_cmd_pt-> man_des == 8) && (veh_info_pt-> man_id2 == 1)) 
                               {
	                               mng_cmd_pt-> man_des=7;
	                               con_st_pt-> des_f_dist=DES_FOLLOW_DIST+SPLIT_DIST;
	                               cruise_t =0.0;
	                               veh_info_pt-> man_id2 =0;
                               }              
                            }
                         if (cnfg_pt-> task == 4)                                     // Joining
                            {
                               if ((cruise_t > CRUISE_T) && (join_sw == 1))     
                               {  
                                  mng_cmd_pt-> man_des = 6;
                                  join_sw = 0;
                                  //con_st_pt-> des_f_dist=DES_FOLLOW_DIST-JOIN_DIST;
                               }
                                if ( mng_cmd_pt-> man_des == 6)
                               		con_st_pt-> des_f_dist=DES_FOLLOW_DIST+con_st_pt-> man_dist_var3-con_st_pt-> man_dist_var2;                  // added on 04/16/11
                              if  ((mng_cmd_pt-> man_des  == 6) && (veh_info_pt-> man_id2 == 1))   // finished the maneuver and return to cruise
                               {
	                               mng_cmd_pt-> man_des=7;
	                               con_st_pt-> des_f_dist=DES_FOLLOW_DIST-JOIN_DIST;
	                               cruise_t =0.0;
                               }     
                            }
                          if (cnfg_pt-> task == 5)                                     // Splitting & Joining
                            {
                               if ((cruise_t > CRUISE_T) && (split_sw == 1) )      
                               {                                 
                                  mng_cmd_pt-> man_des = 8;   // tested for 8, 6     
                                  split_sw =0; 
                                  //con_st_pt-> des_f_dist=DES_FOLLOW_DIST+SPLIT_DIST;  
                               }   
                                if ( mng_cmd_pt-> man_des == 8)
                               		con_st_pt-> des_f_dist=DES_FOLLOW_DIST+con_st_pt-> man_dist_var3-con_st_pt-> man_dist_var2;                  // added on 04/16/11
                               if  ((mng_cmd_pt-> man_des == 8) && (veh_info_pt-> man_id2 == 1)) 
                               {
	                               mng_cmd_pt-> man_des=7;
	                               con_st_pt-> des_f_dist=DES_FOLLOW_DIST+SPLIT_DIST;
	                               cruise_t =0.0;
                               }     
                               if ((cruise_t > 3.0*CRUISE_T) && (join_sw == 1))     
                               {  
                                  mng_cmd_pt-> man_des = 6;
                                  join_sw = 0;
                                  //con_st_pt-> des_f_dist=DES_FOLLOW_DIST-JOIN_DIST;
                               }
                                if ( mng_cmd_pt-> man_des == 6)
                               		con_st_pt-> des_f_dist=DES_FOLLOW_DIST+SPLIT_DIST+con_st_pt-> man_dist_var3-con_st_pt-> man_dist_var2;                  // added on 04/16/11
                              if  ((mng_cmd_pt-> man_des  == 6) && (veh_info_pt-> man_id2 == 1))   // finished the maneuver and return to cruise
                               {
	                               mng_cmd_pt-> man_des=7;
	                               con_st_pt-> des_f_dist=DES_FOLLOW_DIST+SPLIT_DIST-JOIN_DIST;
	                               cruise_t =0.0;	   
                               }    
                            }
                           if (cnfg_pt-> task == 6)                                              // Splitting individually;  
                            {
	                            // time based
	                            if (man_t  < CRUISE_T) 
	                            ;
	                            else if (man_t < CRUISE_T+SPLIT_T)
	                            {
		                            if (veh_info_pt-> veh_id == 2)   
		                            {
			                            mng_cmd_pt-> man_des=7;
	                             	   	con_st_pt-> des_f_dist=DES_FOLLOW_DIST; 
                             	   	}     
	                                if (veh_info_pt-> veh_id == 3)   
	                                {
		                                if (split_sw == 1)       
                               			{                                 
                                  			mng_cmd_pt-> man_des = 8;                   // tested for 8, 6     
                                  			split_sw =0;      
	                             			//con_st_pt-> des_f_dist=DES_FOLLOW_DIST+2.0*SPLIT_DIST;
                               			}   
                               			if ( mng_cmd_pt-> man_des == 8)
                               				con_st_pt-> des_f_dist=DES_FOLLOW_DIST+con_st_pt-> man_dist_var3;                  // added on 04/16/11
                             		}	
                             		//printf("veh 2: task 6: step 1\n");	                            
	                            }
	                            else if (man_t < CRUISE_T+2.0*SPLIT_T)
	                            {
		                            //printf("man_t=%f\n", man_t);
		                            if (veh_info_pt-> veh_id == 2)   
		                            {
			                            if (split_sw == 1)       
                               			{                                 
                                  			mng_cmd_pt-> man_des = 8;                   // tested for 8, 6     
                                  			split_sw =0;      
	                             			//con_st_pt-> des_f_dist=DES_FOLLOW_DIST+SPLIT_DIST;
                               			}   
                               			if ( mng_cmd_pt-> man_des == 8)
                               				con_st_pt-> des_f_dist=DES_FOLLOW_DIST+ con_st_pt-> man_dist_var2;                  // added on 04/16/11
                             	   	}     
	                                if (veh_info_pt-> veh_id == 3)   
	                                {
		                                 //mng_cmd_pt-> man_des = 7;                     // here should be speed control only, keep to be man_des==8; it will not split since con_st_pt-> des_f_dist is shorter;
		                                 split_t += delta_t;                          				
                                         temp=split_t*pi/SPLIT_T;     
	                             	     con_st_pt-> des_f_dist=DES_FOLLOW_DIST+2.0*SPLIT_DIST -  SPLIT_DIST*(0.5-0.5*cos(temp));   //con_st_pt-> man_dist_var2;  
	                             	     //printf("veh 3 dist_var= %f\n",con_st_pt-> man_dist_var2 );
                             		}	
                             			//printf("veh 2: task 6: step 2\n");	           	                  
	                            }
	                            else // (man_t > CRUISE_T+2.0*SPLIT_T)     // both vehicles return to cruise following
	                            {
		                            if  ((mng_cmd_pt-> man_des == 8) && (veh_info_pt-> man_id2 == 1))     // at this point, it should be true for both vehicles
                               		{
	                               		mng_cmd_pt-> man_des=7;
	                               		con_st_pt-> des_f_dist=DES_FOLLOW_DIST+SPLIT_DIST;
	                               		//man_t =0.0;
	                              		veh_info_pt-> man_id2 =0;
	                              		split_t=0.0;
	                              		//printf("veh 2: task 6: step 3\n");	   
	                                }
	                            }
	                            
                            }
                            
                         if (cnfg_pt-> task == 7)                                     // Joining individually
                            {
	                            if (man_t  < CRUISE_T) 
	                            ;
	                            else if (man_t < CRUISE_T+JOIN_T)
	                            {
		                            if (veh_info_pt-> veh_id == 2)   
	                                {
		                                if (join_sw == 1)       
                               			{                                 
                                  			mng_cmd_pt-> man_des = 6;                   //   
                                  			join_sw =0;      
	                             			//con_st_pt-> des_f_dist=DES_FOLLOW_DIST-JOIN_DIST;
                               			}   
                               			 if ( mng_cmd_pt-> man_des == 6)
                               				con_st_pt-> des_f_dist=DES_FOLLOW_DIST+con_st_pt-> man_dist_var2;                  // added on 04/16/11
                             		}	
                             		if (veh_info_pt-> veh_id == 3)   
		                            {
			                            mng_cmd_pt-> man_des=7;
			                            join_t += delta_t;                          				
                                        temp=join_t*pi/JOIN_T;     
	                             	   	con_st_pt-> des_f_dist=DES_FOLLOW_DIST +  JOIN_DIST*(0.5-0.5*cos(temp)) ; 
                             	   	}     
                             		//printf("veh 2: task 6: step 1\n");	                            
	                            }
	                            else if (man_t < CRUISE_T+2.0*JOIN_T)
	                            {
		                            //printf("man_t=%f\n", man_t);
		                            if (veh_info_pt-> veh_id == 2)   
	                                {
		                                 mng_cmd_pt-> man_des = 7;                     // here should be speed control only, keep to be man_des==8; it will not split since con_st_pt-> des_f_dist is shorter;
	                             	     con_st_pt-> des_f_dist=DES_FOLLOW_DIST-JOIN_DIST;
                             		}	
                             		if (veh_info_pt-> veh_id == 3)   
		                            {
			                            if (join_sw == 1)       
                               			{                                 
                                  			mng_cmd_pt-> man_des = 6;                   // tested for 8, 6     
                                  			join_sw =0;      
	                             			//con_st_pt-> des_f_dist=DES_FOLLOW_DIST-JOIN_DIST;
                               			}  
                               			 if ( mng_cmd_pt-> man_des == 6)
                               				con_st_pt-> des_f_dist=DES_FOLLOW_DIST+JOIN_DIST + con_st_pt-> man_dist_var3;                  // added on 04/16/11 
                             	   	}     
                             			//printf("veh 2: task 7: step 2\n");	           	                  
	                            }
	                            else
	                            {
		                            if  ((mng_cmd_pt-> man_des == 6) && (veh_info_pt-> man_id2 == 1))     // at this point, it should be true for both vehicles
                               		{
	                               		mng_cmd_pt-> man_des=7;
	                               		con_st_pt-> des_f_dist=DES_FOLLOW_DIST-JOIN_DIST;
	                              		veh_info_pt-> man_id2 =0;
	                              		  join_t =0.0;
	                              		//printf("veh 2: task 7: step 3\n");	   
	                                }
	                            }
	                           
                            }
                          if (cnfg_pt-> task == 8)                                     // Splitting & Joining Individually
                            {
                               // Splitting
                                if (man_t  < CRUISE_T) 
	                            ;
	                            else if (man_t < CRUISE_T+SPLIT_T)
	                            {
		                            if (veh_info_pt-> veh_id == 2)   
		                            {
			                            mng_cmd_pt-> man_des=7;
	                             	   	con_st_pt-> des_f_dist=DES_FOLLOW_DIST; 
                             	   	}     
	                                if (veh_info_pt-> veh_id == 3)   
	                                {
		                                if (split_sw == 1)       
                               			{                                 
                                  			mng_cmd_pt-> man_des = 8;                   // tested for 8, 6     
                                  			split_sw =0;      
	                             			//con_st_pt-> des_f_dist=DES_FOLLOW_DIST+2.0*SPLIT_DIST;
                               			}  
                               			 if ( mng_cmd_pt-> man_des == 8)
                               				con_st_pt-> des_f_dist=DES_FOLLOW_DIST+con_st_pt-> man_dist_var3;                  // added on 04/16/11  
                             		}	
                             		//printf("veh 2: task 6: step 1\n");	                            
	                            }
	                            else if (man_t < CRUISE_T+2.0*SPLIT_T)
	                            {
		                            //printf("man_t=%f\n", man_t);
		                            if (veh_info_pt-> veh_id == 2)   
		                            {
			                            if (split_sw == 1)       
                               			{                                 
                                  			mng_cmd_pt-> man_des = 8;                   // tested for 8, 6     
                                  			split_sw =0;      
	                             			//con_st_pt-> des_f_dist=DES_FOLLOW_DIST+SPLIT_DIST;
                               			} 
                               			 if ( mng_cmd_pt-> man_des == 8)
                               				con_st_pt-> des_f_dist=DES_FOLLOW_DIST+con_st_pt-> man_dist_var2;                  // added on 04/16/11  
                             	   	}     
	                                if (veh_info_pt-> veh_id == 3)   
	                                {
		                                 //mng_cmd_pt-> man_des = 7;                     // here should be speed control only, keep to be man_des==8; it will not split since con_st_pt-> des_f_dist is shorter;
	                             	    // con_st_pt-> des_f_dist=DES_FOLLOW_DIST+2.0*SPLIT_DIST - con_st_pt-> man_dist_var2;
	                             	    split_t += delta_t;                          				
                                        temp=split_t*pi/SPLIT_T;     
	                             	    con_st_pt-> des_f_dist=DES_FOLLOW_DIST+2.0*SPLIT_DIST -  SPLIT_DIST*(0.5-0.5*cos(temp));   //con_st_pt-> man_dist_var2;  
                             		}	
                             			//printf("veh 2: task 6: step 2\n");	           	                  
	                            }
	                            else if (man_t < 2.0*CRUISE_T+2.0*SPLIT_T)
	                            {
		                            if  ((mng_cmd_pt-> man_des == 8) && (veh_info_pt-> man_id2 == 1))     // at this point, it should be true for both vehicles
                               		{
	                               		mng_cmd_pt-> man_des=7;
	                               		con_st_pt-> des_f_dist=DES_FOLLOW_DIST+SPLIT_DIST;
	                               		//man_t =0.0;
	                              		veh_info_pt-> man_id2 =0;
	                              		//printf("veh 2: task 6: step 3\n");	   
	                                }
	                            }
	                           // Joining
	                            else if (man_t < 2.0*CRUISE_T+2.0*SPLIT_T+JOIN_T)
	                            {
		                            if (veh_info_pt-> veh_id == 2)   
	                                {
		                                if (join_sw == 1)       
                               			{                                 
                                  			mng_cmd_pt-> man_des = 6;                   //   
                                  			join_sw =0;      
	                             			//con_st_pt-> des_f_dist=DES_FOLLOW_DIST+SPLIT_DIST-JOIN_DIST;
                               			}   
                               			 if ( mng_cmd_pt-> man_des == 6)
                               				con_st_pt-> des_f_dist=DES_FOLLOW_DIST+SPLIT_DIST+con_st_pt-> man_dist_var2;                  // added on 04/16/11 
                             		}	
                             		if (veh_info_pt-> veh_id == 3)   
		                            {
			                            mng_cmd_pt-> man_des=7;
	                             	   	//con_st_pt-> des_f_dist=DES_FOLLOW_DIST+SPLIT_DIST-con_st_pt-> man_dist_var2; 
	                             	   	join_t += delta_t;                          				
                                        temp=join_t*pi/JOIN_T;     
	                             	   	con_st_pt-> des_f_dist=DES_FOLLOW_DIST + SPLIT_DIST + JOIN_DIST*(0.5-0.5*cos(temp)) ; 
                             	   	}     
                             		//printf("veh 2: task 6: step 1\n");	                            
	                            }
	                            else if (man_t < 2.0*CRUISE_T+2.0*SPLIT_T+2.0*JOIN_T)
	                            {
		                            //printf("man_t=%f\n", man_t);
		                            if (veh_info_pt-> veh_id == 2)   
	                                {
		                                 mng_cmd_pt-> man_des = 7;                     // here should be speed control only, keep to be man_des==8; it will not split since con_st_pt-> des_f_dist is shorter;
	                             	     con_st_pt-> des_f_dist=DES_FOLLOW_DIST+SPLIT_DIST-JOIN_DIST;
                             		}	
                             		if (veh_info_pt-> veh_id == 3)   
		                            {
			                            if (join_sw == 1)       
                               			{                                 
                                  			mng_cmd_pt-> man_des = 6;                   // tested for 8, 6     
                                  			join_sw =0;      
	                             			//con_st_pt-> des_f_dist=DES_FOLLOW_DIST+SPLIT_DIST-JOIN_DIST;
                               			}   
                               			 if ( mng_cmd_pt-> man_des == 6)
                               				con_st_pt-> des_f_dist=DES_FOLLOW_DIST+SPLIT_DIST+JOIN_DIST+con_st_pt-> man_dist_var3;                  // added on 04/16/11 
                             	   	}
                         	   	}    
                             	else
	                            {
		                            if  ((mng_cmd_pt-> man_des == 6) && (veh_info_pt-> man_id2 == 1))     // at this point, it should be true for both vehicles
                               		{
	                               		mng_cmd_pt-> man_des=7;
	                               		con_st_pt-> des_f_dist=DES_FOLLOW_DIST+SPLIT_DIST-JOIN_DIST;
	                              		veh_info_pt-> man_id2 =0;
	                              		split_t =0.0;
	                              		join_t =0.0;
	                              		//printf("veh 2: task 7: step 3\n");	   
	                                }
	                            } 
                            }  // task 8 end
                            
                     }
                 else //if( (veh_info_pt-> run_dist) > (mng_cmd_pt-> stop_dist) )
                 {
                    mng_cmd_pt-> man_des = 29;
                    if (man_des_29_sw == 1)
                    {
                        con_st_pt-> des_f_dist=con_st_pt-> des_f_dist+SPLIT_DIST;
                        man_des_29_sw =0;
                   }
                }
             }   // End for veh > 1
   }
   else   /// Emergency handling for NVD all vehicles
   {      
             if ((mng_cmd_pt-> f_manage_index == 2) && (f_manage_2_sw ==1) && (mng_cmd_pt-> man_des != 29)) 
              {
	              mng_cmd_pt-> man_des = 13;
	              if ((mng_cmd_pt-> man_des  == 13) && (veh_info_pt-> man_id2 == 1))
	              {
	              	  mng_cmd_pt-> man_des = 7;
	              	  //con_st_pt-> des_f_dist=DES_FOLLOW_DIST+E_SPLIT_DIST;
	              	  if (veh_info_pt-> veh_id == 2)   
	              	  		con_st_pt-> des_f_dist=DES_FOLLOW_DIST+con_st_pt-> man_dist_var2;  
	              	  if (veh_info_pt-> veh_id == 3)   
	              	  		con_st_pt-> des_f_dist=DES_FOLLOW_DIST+con_st_pt-> man_dist_var3-con_st_pt-> man_dist_var2;  
	              	  f_manage_2_sw =0;
	              	  veh_info_pt-> man_id2 =0;
              	  }
              }
             if( ( mng_cmd_pt-> man_des ==7) && (veh_info_pt-> run_dist) > (mng_cmd_pt-> stop_dist) )
             {
                  mng_cmd_pt-> man_des = 29;                 
                   if (man_des_29_sw == 1)
                    {
                        con_st_pt-> des_f_dist=con_st_pt-> des_f_dist+SPLIT_DIST;
                        man_des_29_sw =0;
                   }
             }
             if ((mng_cmd_pt-> f_manage_index == 3) && (f_manage_3_sw ==1))
              {
	              mng_cmd_pt-> man_des = 13;
	              if ((mng_cmd_pt-> man_des  == 13) && (veh_info_pt-> man_id2 == 1)) 
	              {
		              //con_st_pt-> des_f_dist=DES_FOLLOW_DIST+E_SPLIT_DIST;
	              	  mng_cmd_pt-> man_des = 29;
	                 if (man_des_29_sw == 1)
                       {
                        con_st_pt-> des_f_dist=con_st_pt-> des_f_dist+SPLIT_DIST;
                        man_des_29_sw =0;
                      }
	              	   f_manage_3_sw =0;
	              	   veh_info_pt-> man_id2 =0;
              	  }
              }
  }
     
  return 1;

}                

