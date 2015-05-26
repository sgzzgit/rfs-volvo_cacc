/**************************************************************************************************************************************

     maneuver()

     To generate reference trajectory for maneuvers to be performed
     DVI has higher priority to determine the maneuver to be performed.
     Only in automatic driving mode, trajecory planning is activated.
     maneuver_id[0] should be assgined here.

     Should use man_des to determine all the maneuvers instead of drive_mode    10_21_03
     For each maneuver, ref_v, ref_a, should be assigned to each vehicle        10_21_03
     Additional acc and spd for maneuvers w.r.t. to the preceding veh
       are independent of the fundamental acc and spd provided by tran_ref()    10_27_03
       
     fault_index_typ* f_index_ptis used forconditional ref_v and ref_a
                  determination                                                 09_04_10
    tran_ref()  cannot be used for variable max_speed trajectory planning;         03_25_11
    tran_ref() is replaced with vary_ref;                                                                           03_25_11
	Case 27 added for platoon deceleration;                                                                 03_25_11
	man_id[0] assignment moved here from trajectory generating;                        03_26_11   
	man_id[0] assignment is replaced with veh_info_pt-> man_id1;                       03_20_11      
	
	
****************************************************************************************************************************************/
 
 
#include <stdio.h>
#include <math.h>
#include "veh_long.h"
#include "coording.h"
#include "road_map.h"


#define pi 3.14159265



int maneuver(float delta_t, float t_ctrl, float t_flt, float* v_p, float* c, float* d, road_info_typ* rd_info_pt,
             control_config_typ* config_pt,  control_state_typ* con_st_pt, vehicle_info_typ* veh_info_pt, 
             fault_index_typ* f_index_pt, int man_des, int maneuver_id[2])

{

     //float reference_accel = 0.0, reference_speed = 0.0, reference_distance=0.0;      // Used to hold data to be passed out.
    
     static float x_ref[2]={0.0,0.0};    
     static short int man_des_local=0, i=0;
     static float v_t=0.0, acc_tmp=0.0;
     static int  split_sample_sw = 1, join_sample_sw=1, e_split_sample_sw = 1;

     static float temp=0.0,temp1=0.0,temp2=0.0, loc_join_dist=0.0, loc_split_dist=0.0;
     static float split_t=0.0, join_t=0.0,  e_split_t=0.0;
	 float max_acc_local=0.0;
	 
     /*--- Function Declaration ---*/

    //void gen_ref(float, float, int, float*, float*, float*,  control_config_typ*, float, int*, float*);                  
                            
     //void tran_ref(float, float, int, float*, float*, float*, control_config_typ*, float,
     //             control_state_typ*, int *, float*);
     void vary_ref(float, float, int, float*, float*, float*, control_config_typ*, float,
                  control_state_typ*, int *, float*);
     float spd_flt(float);
     float fuel_flt(float);              
	 float min(float , float );             


    
     /*--- Tansition Control: between manual and automatic ---*/

  //if (man_des !=0 && man_des != 1)
 if (t_ctrl > t_wait)	
     vary_ref(delta_t,t_flt,man_des,v_p,c,d,config_pt, (rd_info_pt-> grade), con_st_pt,maneuver_id,x_ref);

    
  if (x_ref[1] < 0.0)
         x_ref[1]=0.0;         
  maneuver_id[0] = man_des;                // This seems unnecessary if we use:   veh_info_pt-> man_id1; 
  veh_info_pt-> man_id1= man_des;

/*********************************************************
   Added for split join maneuver   04_01_11
********************************************************/
   
  for (i=0;i<N_pt-1;i++)
           {
                  if ((con_st_pt-> ref_v >= v_p[i]) && (con_st_pt-> ref_v < v_p[i+1]) )                                                   
                        max_acc_local = c[i]*(con_st_pt-> ref_v)+d[i] - G*sin(rd_info_pt-> grade);                                                                                                                  
           }
           
/*********************************************************
		Different man_des cases	
**********************************************************/
  switch (man_des)
     {
        case 0:
            con_st_pt-> lead_v=0.0;
            con_st_pt-> lead_a=0.0;
            con_st_pt-> pre_v=0.0;
            con_st_pt-> pre_a=0.0;                    
            con_st_pt-> ref_v=0.0;
            con_st_pt-> ref_a=0.0;  
            maneuver_id[0] = man_des; 
            veh_info_pt-> man_id1=man_des;     
            break;
        
        case 1:
             
             if (veh_info_pt-> veh_id ==1)
                 {
                    con_st_pt-> lead_v=x_ref[1];
                    con_st_pt-> lead_a=x_ref[0];
                    con_st_pt-> pre_v=x_ref[1];
                    con_st_pt-> pre_a=x_ref[0];                    
                    con_st_pt-> ref_v=x_ref[1];
                    con_st_pt-> ref_a=x_ref[0];
                 }
             else //if (veh_info_pt-> veh_id == 2)
                 {
                    con_st_pt-> lead_v=x_ref[1];
                    con_st_pt-> lead_a=x_ref[0];                                                   
                    con_st_pt-> ref_v=0.0;
                    con_st_pt-> ref_a=0.0;
                    con_st_pt-> man_dist_var2=0.0;       
                    con_st_pt-> man_dist_var3=0.0;        
                 }   
              maneuver_id[0] = man_des;   
              veh_info_pt-> man_id1=man_des;          
             break;
             
        case 2:
             if (t_ctrl >= t_wait)
                 {                                 
                     con_st_pt->auto_speed=con_st_pt-> spd;
                     con_st_pt->auto_acc=con_st_pt-> acc;
                 }
             if (veh_info_pt-> veh_id ==1)
                 {
                    con_st_pt-> lead_v=x_ref[1];
                    con_st_pt-> lead_a=x_ref[0];
                    con_st_pt-> pre_v=x_ref[1];
                    con_st_pt-> pre_a=x_ref[0];                    
                    con_st_pt-> ref_v=x_ref[1];
                    con_st_pt-> ref_a=x_ref[0];
                 }
             else if (veh_info_pt-> veh_id == 2)
                 {
                    con_st_pt-> lead_v=x_ref[1];
                    con_st_pt-> lead_a=x_ref[0];   
                    if (f_index_pt-> comm == 0) 
                    {                                                
                    	con_st_pt-> ref_v=con_st_pt-> pre_v;
                    	con_st_pt-> ref_a=con_st_pt-> pre_a;
                	}
                	else
                	{	
	                	con_st_pt-> ref_v=x_ref[1];
                		con_st_pt-> ref_a=x_ref[0];
                	}
                    con_st_pt-> man_dist_var2=0.0;       
                    con_st_pt-> man_dist_var3=0.0;             
                 } 
             else                                                   // pre_v  needs checking for multiple vehicles.
                 {
	                if (f_index_pt-> comm == 0)
	                {
                    	v_t += delta_t;            
                    	con_st_pt-> ref_a=con_st_pt-> lead_a;
                    	if (v_t < 5.0)  
                        	con_st_pt-> ref_v=con_st_pt-> lead_v;
                    	else if (v_t < 15.0)
                        	con_st_pt-> ref_v=con_st_pt-> lead_v + ((v_t-5.0)/10.0)*(con_st_pt-> pre_v - con_st_pt-> lead_v);
                    	else
                        	con_st_pt-> ref_v=con_st_pt-> pre_v;
                    }
                    else
                    {	
	                	con_st_pt-> ref_v=x_ref[1];
                		con_st_pt-> ref_a=x_ref[0];
                	}
                 }
              maneuver_id[0] = man_des;
              veh_info_pt-> man_id1=man_des;     
             break;
             
        case 3:  // Platoon acceleration
          
             if (veh_info_pt-> veh_id ==1)
                 {
                    con_st_pt-> lead_v=x_ref[1];
                    con_st_pt-> lead_a=x_ref[0];
                    con_st_pt-> pre_v=x_ref[1];
                    con_st_pt-> pre_a=x_ref[0];                    
                    con_st_pt-> ref_v=x_ref[1];
                    con_st_pt-> ref_a=x_ref[0];
                 }
             else if (veh_info_pt-> veh_id == 2)
                 {
                    con_st_pt-> lead_v=x_ref[1];
                    con_st_pt-> lead_a=x_ref[0];
                    if (f_index_pt-> comm == 0) 
                    {                                                
                    	con_st_pt-> ref_v=con_st_pt-> pre_v;
                    	con_st_pt-> ref_a=con_st_pt-> pre_a;
                	}
                	else
                	{	
	                	con_st_pt-> ref_v=x_ref[1];
                		con_st_pt-> ref_a=x_ref[0];
                	}
                   con_st_pt-> man_dist_var2=0.0;       
                    con_st_pt-> man_dist_var3=0.0;                 
                 } 
             else                                                   // pre_v  needs checking for multiple vehicles.
                 {
	                con_st_pt-> lead_v=x_ref[1];
                    con_st_pt-> lead_a=x_ref[0];
                    if (f_index_pt-> comm == 0)
	                {
                    	v_t += delta_t;            
                    	con_st_pt-> ref_a=con_st_pt-> lead_a;
                    	if (v_t < 5.0)  
                        	con_st_pt-> ref_v=con_st_pt-> lead_v;
                    	else if (v_t < 15.0)
                        	con_st_pt-> ref_v=con_st_pt-> lead_v + ((v_t-5.0)/10.0)*(con_st_pt-> pre_v - con_st_pt-> lead_v);
                    	else
                        	con_st_pt-> ref_v=con_st_pt-> pre_v;
                    }
                    else
                    {	
	                	con_st_pt-> ref_v=x_ref[1];
                		con_st_pt-> ref_a=x_ref[0];
                	}
                 }
              maneuver_id[0] = man_des;
              veh_info_pt-> man_id1=man_des;     
             break;
             
        case 4:  // Transition from manual to automatic
             if (veh_info_pt-> veh_id ==1)
                 {
                    con_st_pt-> lead_v=x_ref[1];
                    con_st_pt-> lead_a=x_ref[0];
                    con_st_pt-> pre_v=x_ref[1];
                    con_st_pt-> pre_a=x_ref[0];                    
                    con_st_pt-> ref_v=x_ref[1];
                    con_st_pt-> ref_a=x_ref[0];
                 }   
             maneuver_id[0] = man_des;
             veh_info_pt-> man_id1=man_des;     
             break;
             
        case 5:   // platoon forming
             break;

        case 6:   // joining under automatic control
             if (veh_info_pt-> veh_id ==1)
                 {
                    con_st_pt-> lead_v=x_ref[1];
                    con_st_pt-> lead_a=x_ref[0];
                    con_st_pt-> pre_v=x_ref[1];
                    con_st_pt-> pre_a=x_ref[0];                    
                    con_st_pt-> ref_v=x_ref[1];
                    con_st_pt-> ref_a=x_ref[0];
                 }
             else  //if (veh_info_pt-> veh_id == 2)
                 {
	                 if (join_sample_sw == 1) 
                     {
                          	join_sample_sw = 0;
                            veh_info_pt-> man_id2=0;                                                     // 03_30_11
                            maneuver_id[1]=0;
                     		if (con_st_pt-> front_range <= con_st_pt-> des_f_dist-JOIN_DIST-0.5)
                       		{
                           		con_st_pt-> ref_v=con_st_pt-> pre_v;
                           		con_st_pt-> ref_a=con_st_pt-> pre_a;
                           		con_st_pt-> man_dist_var2=0.0;       
                    			con_st_pt-> man_dist_var3=0.0;       
                         		fprintf(stderr," veh 2: Joining distance should be shorter than current distance!\n");
                          		break;
                       		}
                     		else
                     			loc_join_dist=JOIN_DIST; //-con_st_pt-> des_f_dist+con_st_pt-> front_range;
                     		join_sample_sw = 0;
                 	  }
                    con_st_pt-> lead_v=x_ref[1];
                    con_st_pt-> lead_a=x_ref[0];
                    if (join_t < JOIN_T)
                       {                              
                          join_t += delta_t;
                          temp=pi/JOIN_T;
                          temp1=temp*join_t;   //-pi;  
                          temp2=(float)(veh_info_pt-> veh_id-1)*JOIN_DIST;    // The initial distance should be sampled at the instant of starting joining   
                          con_st_pt-> ref_v=con_st_pt-> lead_v+0.5*temp2*temp*sin(temp1);
                          con_st_pt-> ref_a=con_st_pt-> lead_a+0.5*temp2*temp*temp*cos(temp1);
                          //acc_tmp+=(con_st_pt-> ref_a)*delta_t;
                          //con_st_pt-> ref_v=con_st_pt-> lead_v + min(acc_tmp, max_acc_local);                                         // working
                          if (veh_info_pt-> veh_id ==2)
                          {
                          con_st_pt-> man_dist_var2 = -temp2*(0.5-0.5*cos(temp1));
                          con_st_pt-> man_dist_var3 = -2.0*temp2*(0.5-0.5*cos(temp1));
                      	 }
                      	 if (veh_info_pt-> veh_id ==3)
                      	 {
	                      	 con_st_pt-> man_dist_var2 = -0.5*temp2*(0.5-0.5*cos(temp1));
                          	 con_st_pt-> man_dist_var3 = -temp2*(0.5-0.5*cos(temp1));
                      	 }
                          //con_st_pt-> des_f_dist=con_st_pt-> des_f_dist+(con_st_pt-> ref_v)*delta_t;
                       }
                    else
                       {
                          maneuver_id[1]=1;                                             // This seems unnecessary if we use:   veh_info_pt-> man_id2; 
                          veh_info_pt-> man_id2=1;                                                // To acknowledge finishing the maneuverl; 03_30_11          
                          
                          //con_st_pt-> des_f_dist = con_st_pt-> join_f_dist;        // After joining, reset the desired following dist
                          //con_st_pt-> temp_dist = con_st_pt-> des_f_dist;          //Added 12_03_03, No use
                          con_st_pt-> ref_v=con_st_pt-> pre_v;
                          con_st_pt-> ref_a=con_st_pt-> pre_a;
                         // con_st_pt-> man_dist_var = con_st_pt-> join_f_dist - con_st_pt-> des_f_dist;                          
                          //con_st_pt-> temp_dist = con_st_pt-> des_f_dist;    
                          
                       }  
                 } 
             //else;     
             maneuver_id[0] = man_des;   
             veh_info_pt-> man_id1=man_des;     
             break;

        case 7:   // platoon  cruise
         
             if (veh_info_pt-> veh_id ==1)
                 {
                    con_st_pt-> lead_v=x_ref[1];
                    con_st_pt-> lead_a=x_ref[0];
                    con_st_pt-> pre_v=x_ref[1];
                    con_st_pt-> pre_a=x_ref[0];                    
                    con_st_pt-> ref_v=x_ref[1];
                    con_st_pt-> ref_a=x_ref[0];
                 }             
             else       
             	 {
                    con_st_pt-> lead_v=x_ref[1];
                    con_st_pt-> lead_a=x_ref[0];
                    if (f_index_pt-> comm == 0) 
                    { 
                    	con_st_pt-> ref_v=con_st_pt-> pre_v;                    
                    	con_st_pt-> ref_a=con_st_pt-> lead_a;
                	}
                	else
                	{	
	                	con_st_pt-> ref_v=x_ref[1];
                		con_st_pt-> ref_a=x_ref[0];
                	}
                    con_st_pt-> man_dist_var2=0.0;       
                    con_st_pt-> man_dist_var3=0.0;               
                 } 
             maneuver_id[0] = man_des;
             veh_info_pt-> man_id1=man_des;  
             join_sample_sw =1;                   // to prepare for maneuver next step;
             split_sample_sw =1; 
             split_t = 0.0;
             join_t = 0.0; 
             veh_info_pt-> man_id2=0;
             e_split_t =0.0;   
             e_split_sample_sw =1;     
             break;

        case 8:   // splitting under automatic control

             if (veh_info_pt-> veh_id ==1)
                 {
                    con_st_pt-> lead_v=x_ref[1];
                    con_st_pt-> lead_a=x_ref[0];
                    con_st_pt-> pre_v=x_ref[1];
                    con_st_pt-> pre_a=x_ref[0];                    
                    con_st_pt-> ref_v=x_ref[1];
                    con_st_pt-> ref_a=x_ref[0];
                 }
             else //     veh_id>1    if (veh_info_pt-> veh_id == 2)
                 {
	                 if (split_sample_sw == 1) 
                     {
                          	split_sample_sw = 0;
                            veh_info_pt-> man_id2=0;                                                     // 03_30_11
                            maneuver_id[1]=0;
                     		if (con_st_pt-> front_range >= con_st_pt-> des_f_dist+SPLIT_DIST+0.5)
                       		{
                           		con_st_pt-> ref_v=con_st_pt-> pre_v;
                           		con_st_pt-> ref_a=con_st_pt-> pre_a;
                           		con_st_pt-> man_dist_var2=0.0;       
                                con_st_pt-> man_dist_var3=0.0;       
                         		fprintf(stderr," spliting distance should be longer than current distance!\n");
                          		break;
                       		}
                     		else
                     			loc_split_dist=SPLIT_DIST;  //+con_st_pt-> des_f_dist-con_st_pt-> front_range;
                     		split_sample_sw = 0;
                 	  }                                           
                    con_st_pt-> lead_v=x_ref[1];
                    con_st_pt-> lead_a=x_ref[0];
                    if (split_t <= SPLIT_T)
                       {                           
                          split_t += delta_t;
                          temp=pi/SPLIT_T;
                          temp1=temp*split_t;     
                          temp2=(float)(veh_info_pt-> veh_id-1)*loc_split_dist;    // The initial distance should be sampled at the instant of starting splitting.          
                          con_st_pt-> ref_v=con_st_pt-> lead_v-0.5*temp2*temp*sin(temp1);
                          con_st_pt-> ref_a=con_st_pt-> lead_a-0.5*temp2*temp*temp*cos(temp1);
                          //con_st_pt-> des_f_dist=con_st_pt-> des_f_dist+(con_st_pt-> ref_v)*delta_t;
                          if (veh_info_pt-> veh_id ==2)
                          {
                          con_st_pt-> man_dist_var2 = temp2*(0.5-0.5*cos(temp1));
                          con_st_pt-> man_dist_var3 = 2.0*temp2*(0.5-0.5*cos(temp1));
                      	 }
                      	 if (veh_info_pt-> veh_id ==3)
                      	 {
	                      	 con_st_pt-> man_dist_var2 = 0.5*temp2*(0.5-0.5*cos(temp1));
                          	 con_st_pt-> man_dist_var3 = temp2*(0.5-0.5*cos(temp1));
                      	 }
                       }
                    else
                       {
                           maneuver_id[1]=1;                                       // This seems unnecessary if we use:   veh_info_pt-> man_id2; 
                           veh_info_pt-> man_id2=1;                                                // To acknowledge finishing the maneuverl; 03_30_11          
                           //con_st_pt-> des_f_dist=con_st_pt-> split_f_dist;        // Reset desired foliwing distance after splitting
                           con_st_pt-> ref_v=con_st_pt-> pre_v;
                           con_st_pt-> ref_a=con_st_pt-> pre_a;
                           //con_st_pt-> man_dist_var = con_st_pt-> split_f_dist-con_st_pt-> des_f_dist;                           
                       }        
                 }      
             maneuver_id[0] = man_des;     
             veh_info_pt-> man_id1=man_des;   
            // printf("man_id1= %d\n", veh_info_pt-> man_id1);     
             break;  // end of case 8
             
         case 13:   // Emergency splitting under automatic control
                    con_st_pt-> lead_v=x_ref[1];
                    con_st_pt-> lead_a=x_ref[0];
                    con_st_pt-> pre_v=x_ref[1];
                    con_st_pt-> pre_a=x_ref[0];                    
                    con_st_pt-> ref_v=x_ref[1];
                    con_st_pt-> ref_a=x_ref[0];
                                        
                    if (e_split_t <= SPLIT_T)
                       {  
                          if (e_split_sample_sw == 1) 
                             {
                                e_split_sample_sw = 0;
                                veh_info_pt-> man_id2=0;  
                                maneuver_id[1]=0;   
                             }      
                          e_split_t += delta_t;
                          temp=pi/SPLIT_T;
                          temp1=temp*e_split_t;             
                          //temp2=(float)(veh_info_pt-> veh_id-1)*(1.5*SPLIT_FINAL_DIST -con_st_pt-> des_f_dist);    // The initial distance should be sampled at the instant of starting splitting.       
                          temp2=(float)(veh_info_pt-> veh_id-1)*E_SPLIT_DIST;                                                                     // The initial distance should be sampled at the instant of starting splitting.            
                          con_st_pt-> ref_v=con_st_pt-> ref_v-0.5*temp2*temp*sin(temp1);
                          con_st_pt-> ref_a=con_st_pt-> ref_a-0.5*temp2*temp*temp*cos(temp1);
                          //con_st_pt-> man_dist_var = temp2*(0.5-0.5*cos(temp1));
                         // con_st_pt-> des_f_dist=con_st_pt-> des_f_dist+(con_st_pt-> ref_v)*delta_t;
                          if (veh_info_pt-> veh_id ==2)
                          {
                          con_st_pt-> man_dist_var2 = temp2*(0.5-0.5*cos(temp1));
                          con_st_pt-> man_dist_var3 = 2.0*temp2*(0.5-0.5*cos(temp1));
                      	 }
                      	 if (veh_info_pt-> veh_id ==3)
                      	 {
	                      	 con_st_pt-> man_dist_var2 = 0.5*temp2*(0.5-0.5*cos(temp1));
                          	 con_st_pt-> man_dist_var3 = temp2*(0.5-0.5*cos(temp1));
                      	 }
                       }
                    else
                       {
                           maneuver_id[1]=1;                                        // This seems unnecessary if we use:   veh_info_pt-> man_id2; 
                           veh_info_pt-> man_id2=1;                          // To acknowledge finishing the maneuverl; 03_30_11                                      
                       }              
            maneuver_id[0] = man_des;     
            veh_info_pt-> man_id1=man_des;   
            // printf("man_id1= %d\n", veh_info_pt-> man_id1);     
             break;   // end of case 13
                 
         case 27:   // platoon  deceleration       
             if (veh_info_pt-> veh_id ==1)
                 {
                    con_st_pt-> lead_v=x_ref[1];
                    con_st_pt-> lead_a=x_ref[0];
                    con_st_pt-> pre_v=x_ref[1];
                    con_st_pt-> pre_a=x_ref[0];                    
                    con_st_pt-> ref_v=x_ref[1];
                    con_st_pt-> ref_a=x_ref[0];
                 }             
             else       
             	 {
                    con_st_pt-> lead_v=x_ref[1];
                    con_st_pt-> lead_a=x_ref[0];
                    if (f_index_pt-> comm == 0) 
                    { 
                    	con_st_pt-> ref_v=con_st_pt-> pre_v;                    
                    	con_st_pt-> ref_a=con_st_pt-> lead_a;
                	}
                	else
                	{	
	                	con_st_pt-> ref_v=x_ref[1];
                		con_st_pt-> ref_a=x_ref[0];
                	}
                    con_st_pt-> man_dist_var2=0.0;       
                    con_st_pt-> man_dist_var3=0.0;             
                 } 
             maneuver_id[0] = man_des; 
             veh_info_pt-> man_id1=man_des;                           // To acknowledge finishing the maneuver
             break;     // Case 27 end
             
        case 29:
           if (veh_info_pt-> veh_id ==1)
                 {
                    con_st_pt-> lead_v=x_ref[1];
                    con_st_pt-> lead_a=x_ref[0];
                    con_st_pt-> pre_v=x_ref[1];
                    con_st_pt-> pre_a=x_ref[0];                    
                    con_st_pt-> ref_v=x_ref[1];
                    con_st_pt-> ref_a=x_ref[0];
                 }
             else if (veh_info_pt-> veh_id >= 2)
                 {
                    con_st_pt-> lead_v=x_ref[1];
                    con_st_pt-> lead_a=x_ref[0];
                    if (f_index_pt-> comm == 0) 
                    {                                                
                    	con_st_pt-> ref_v=con_st_pt-> pre_v;
                    	con_st_pt-> ref_a=con_st_pt-> pre_a;
                	}
                	else
                	{	
	                	con_st_pt-> ref_v=x_ref[1];
                		con_st_pt-> ref_a=x_ref[0];
                	}
                    con_st_pt-> man_dist_var2=0.0;       
                    con_st_pt-> man_dist_var3=0.0;             
                 } 
             else;
			 maneuver_id[0] = man_des; 
			 veh_info_pt-> man_id1=man_des;
             break;              
                 
        case 45:
             con_st_pt-> manu_throt=fuel_flt(con_st_pt-> fuel_rt);
             //con_st_pt-> manu_spd_cmd=spd_cmd_filter();      // for new truck only
             //con_st_pt-> manu_tq_cmp=spd_cmd_filter();       // for new truck only
             con_st_pt-> manu_speed=spd_flt(con_st_pt-> spd);
             con_st_pt-> manu_acc=con_st_pt-> acc;             // Has been filtered
             maneuver_id[0] = man_des;   
             veh_info_pt-> man_id1=man_des;                           
             break;
           
      } // Switch END

      /**********************************/
      /*                                */
      /*--- Maneuvers in Fault Mode  ---*/
      /*                                */
      /**********************************/
                
      return 1;  
     
}





/*  Definition of man_des:

         *      0   : stay at rest with manual control
                
         *      1    : stay at rest to get automatic control ready
               
         *      2    : stay at rest with automatic lateral control ON
            
         *      3    : automatic control following a self-generated reference trajectory  - acceleration (used for variable max-speed)

         *      4    : transition from manual to automatic control (as a single agent)
        
         *      5    : platoon forming - following the previous speed but not the platoon desired distance (speed tracking)
 
         *      6    : automatc joining within a platoon (spd & dist control --> to desired distance)
         
         *      7    : automatic platooning (at cruise speed - no other maneuver)
         
         *      8    : splitting under automatic control (within a platoon) to a specified distance w.r.t. preceding vehicle

         *      9    : left-lane-changing to the end of a platoon

         *      10  : righ-lane-changing to the end of a platoon
         
         *      11  : virtual splitting to specified distance from curent distance on different lane
         
         *      12  : virtual joining to specified distance from curent distance on different lane
         
         *      13  : emergency splitting to longer inter_vehicle distance to stop (due to some incurable fault)
         
         *      14  : vehicle merging to a specified position from on-ramp

         *      15  : free left-lane-changing to the middle of two vehicles (free lane changing)  

         *      16  : free right-lane-changing to the middle of two vehicles (free lane changing)
         
         *      17  : 
         
         *      18  : collision avoidance by lane changing only
         
         *      19  : collision avoidance by lane changing and speed reduction
         
         *      20  : collision avoidance by emergency stop
         
         *      21  :
         *      22  : 
         *      23  :
         
         *      24  : 
         
         *      25  : 
         *      26  : 
         *      27  : automatic deceleration following designed speed profile  (used for variable max-speed)
         
         *      28  : lane departure - leaving AHS from off-ramp
         
         *      29  : automatic closed-loop decelerating to stop using specified profile
         
         *      30  : brake to stop in auto mode 
 
         *      31  : automatic open-loop decelerating to stop by applying specified deceleration cmd
 
         *      32  : brake to stop in manual mode
  
         *      33  : gradually splitting to longer desired distance but carry on platooning (due to some fault)
    
         *      34  : gradually splitting and reducing speed to stop (due to some fault)
  
         *      35  : Cruise Control (as a single agent - radar miss target or no target)
         *      40  : Adaptive Cruise Control (in a platoon with at least one radar but no communication )
         *      41  : Cooperative Adaptive Cruise Control (in a platoon with at least one radar and communication )         
         
         *      45  : manual control (including all the maneuvers)
 
*/
         

 /*  Definition of man_id (which is in agreement with man_des):
 
         *     Simplified on 03_29_11
         Only two values:
         (con_state_pt-> man_id1=man_des)  &&  (con_state_pt-> man_id1=0):    conducting th emaneuver speficied;      
         
         (con_state_pt-> man_id1=man_des)  &&  (con_state_pt-> man_id1=1):    the maneuver is finished in trajectory planning; waiting for new command;
         
*/

 

       


