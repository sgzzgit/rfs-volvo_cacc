/***********************************************************************************

   control.c 
   
   Vehicle longitudinal control
   Started                                                     12_29_01
   Compiled and run on                                         02_20_02
   
   Using new engine data based on N14_435 from                 05_15_02
   Begine for New Freightliner                                 03_20_03
   con_sw[5] is used to switch tq, spd, jk, trdt, brk          03_27_03          
   max() min() functions have run_time problems
   successful run on                                           04_09_03
   add brake control logics on                                 04_10_03
   rate limt to tnet_a removed                                 04_13_03
   jerk management started                                     04_13_03
   preliminarilly tested at CRO                                04_29_03 - 05_01_03
   jk_tq is used as a function of engine speed                 08_08_03
   Use passed parameters to adjust, tested at CRO for 2 veh    10_02_03
   Add tnet_a bounding                                         10_20_03
   Add control logic for transition                            10_20_03
   Actuator switched off in long_trk if man_des=45             10_20_03
   maneuver_id[1]  should be assigned here accroding to
       control performance etc.                                10_21_03
   tnet_a_cal need calibration and used for control.           10_24_03
   jk_tmp has been changed from 1.1 ==> 2.4 to reduce
                  air barek effect                             12_01_03    
                             
                             X. Y. Lu

************************************************************************************/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "veh_long.h"

#define PI 3.14159265


#define JK_TRTD_EBS   1    //10_02_03
//#define JK_EBS  1

//#define EBS  1
  
int control( float step_t, float time_flt, int vehic_id, int manuv_id[2], sens_read_typ* sens_read_pt,  control_config_typ* config_pt_tmp,
           control_state_typ* con_state_pt_tmp, switch_typ* sw_pt_tmp, con_output_typ* output_pt_tmp)
{

    
    //*** Mdl
    static float fan_tq=0.0, alt_tq=0.0, comp_tq=0.0, steer_tq=0.0, cond_tq=0.0, w_p=600.0, w_t=20.0;
    static float we_temp=0.0, rst_total=0.0, temp0=0.0, temp1=0.0,temp2=0.0,temp7=0.0;
    static float  I_trans=0.0, I_trans_old=0.0, I_trans_flt=0.0;  
    static float  r_star=0.0;
    static float  r_starh=0.0;
    static float  eff_in=0.0;
    static float  beta_tmp=1.0;
    static float  Mass=8381.0, Min_mass=0.0;  //Old Freightliner 7727.0(tractor) + 10455.0 = 18182.0   [kg]
    static float  temp_t=0.0;
    static short int gear_sw=1, ff_sw_a=OFF, ff_sw_b=ON;
   
  
    const float  hr = 0.5113;                   // Wheel radius, Cummins data 
    const float  c_a =  3.1571;                 // Air drag coefficient based on theoreticla calculation
                                                              // Used on Aug 28 02, except the last runs
    static float  f_0=0.05175; //0.10175;  good for tractor only  //0.09175 tested on Aug 28 02 except the last runs
    //const float  f_0=0.08175; //Decreasing
                                                                                                                                  
    //const float  c_a = 2.571;                 // 04_07_03
    const float  rd = 0.216;                    // 1.0/4.63;  // Driveline ratio 
    const float  I_WHL=180.0;  //66.0, 125.0;   // 22.0 used for tractor only
    const float  I_drive=210.2807;         // SCAAN, [Nm/s^2]
    const float  I_ENG=5.091;               // SCAAN, [Nm/s^2]
    const float  w_idle=600.0;                // For old Freightliner
   // const float  pitch_coeff=0.3;
    
   // const float  Fr_0 = 1850.0;     //[N], Estimated from Old Freightliner; Not used in control

    const float f_1=0.01;
    const float f_2=0.001;
    const float f_b=100.0;
    const float f_f=10.0, ff_reset=200.0;
    static float f_f_hat=0.0;
    static float f_b_hat=0.0, f_b_tmp=0.0;
    
    static float delta_t=0.0,t_flt=0.0,gear_flt=0.0, tg=0.0, rg=0.0, we_flt=0.0, v=0.0, v_tmp=0.0, gshift_t=0.0;
    static float acl=0.0, Max_spd=0.0, Max_dcc=0.0, Max_v_tmp=0.0;
    static float pitch=0.0, rst_pitch=0.0, dist_m=0.0, engbrk_tq=0.0;
    static float following_dist=10.0, pm=0.0, MAX_BRK=0.0, INI_BRK=0.0;  
    static int veh_id=0, man_id=0;
    static float rst_pitch_old=0.0;
    
    static float tnet_min=0.0;
    static float tnet_des=0.0;
    
    static float Mass_est=0.0;
    static int adap_a=0, adap_b=0;
    static float tnet_a=400.0, tnet_a_old=400.0, tnet_b=0.0, tnet_a_buff=0.0, tnet_b_buff=0.0;
    float tnet_a_cal=0.0, mass_tmp=0.0;
                   
   

    //*** Upper
    static float temp_dist=0.0, temp_dist_d=0.0, distance=0.0; 
    static float s_1=0.0, s_2=0.0, s_3=0.0, s_1_d=0.0, s_1_old=0.0;  // usyn1=0.0;
    static float ff_gain = 1.2, ff_gain_old=2.0;
    static float temp5=0.0, temp5a=0.0, temp5b=0.0;
    static float s_1_flt=0.0, temp3=0.0, eps=0.0; 
    static float slid=0.0, slid_old=0.0, slid_dot=0.0;
    static float slid1=0.0, slid1_old=0.0, slid1_dot=0.0; 
    static float k1=0.0, k2=0.0; 
    static float usyn=0.0;
    


    //*** Lower_1r
    
    static int  Index[3]={0,0,0};
          

   //*** Lower_2


    static float jk_brk=0.0, brake=0.0, pb_s2_p=0.0;
    float jk_tmp=0.0;
    
      
    const float jk_brk_2=400.0, jk_brk_4=800.0, jk_brk_6=1200.0;    // used on Oct. 30,03               
       
   // const float jk_brk_2=0.0, jk_brk_4=0.0, jk_brk_6=0.0;      //Used on Dec. 01, 03
  
    //const float jk_brk_2=0.0, jk_brk_4=0.0, jk_brk_6=0.0;       // Use EBS and trtd only
   
       
    static float pb_des=0.0, pb_des_old=0.0, pb_temp=0.0;
    static float pb_des_0=0.0, pb_des_2=0.0, pb_des_4=0.0, pb_des_6=0.0; 
    static float alp_des=0.0, pm_des=0.0, alp_des_old=0.0, pm_old=0.0;
    static int brake_sw = OFF;
    static float trtd_des=0.0, max_trtd_tq=0.0;
    
   //static int brake_mode=0.0;
   // static float brake_t=0.0;
   // static int E_brake=0.0;
    const float brk_coeff = 0.00367;  //0.00333;             
   
    static switch_typ* sw_pt;
    static con_output_typ* output_pt;
    static control_state_typ* con_state_pt;
    static control_config_typ* config_pt;

    //*** Function declaration
     float eng_map( float, float, int, int, int*);  
     
     float pm_filt(float);
     float usyn_flt(float);
     float brk_flt(float);
    //float trtd_flt(float);
    //float svg3(float);
    //float svg3a(float);
    //float svg3b(float);
    //float sign(float);
     sw_pt = sw_pt_tmp;
     output_pt = output_pt_tmp;
     con_state_pt=con_state_pt_tmp;  
     config_pt=config_pt_tmp; 
     

     delta_t=step_t;
     t_flt=time_flt;
     veh_id = vehic_id;
     man_id = manuv_id[0];
     tg = sens_read_pt-> tg;
     gear_flt = sens_read_pt-> gear_flt;     
     rg = sens_read_pt-> rg;
     we_flt = sens_read_pt-> we_flt;
     pitch=sens_read_pt-> grade;
     pm=sens_read_pt-> pm;     
     v=con_state_pt-> spd;
     acl=con_state_pt-> acc;     
        
     dist_m=con_state_pt-> front_range;
     Max_spd=(config_pt-> max_spd)*0.447;
     Max_dcc=config_pt-> max_dcc; 
     MAX_BRK=con_state_pt-> max_brake*272.48;                // Value at pb_des
     INI_BRK=con_state_pt-> ini_brake;
     following_dist = con_state_pt-> des_f_dist; 
 
     
     following_dist = max(3.0, following_dist);              // Previous minimal distance is 4[m]            

     if (config_pt-> mass_sw == 0)
         {         
            Mass=8381.0;
            Min_mass=Mass;
         }                                     
     else if (config_pt-> mass_sw == 1)
         Mass=14061.0;                                       //31000[lb]
     else if (config_pt-> mass_sw == 2)
         Mass=22226.0;                                       //49000[lb]
     else if (config_pt-> mass_sw == 3)
         Mass=31794.6;                                       //70095[lb]
     else if (config_pt-> mass_sw == 4)                      //bl_ucr
         Mass=28853.0; 
     else if (config_pt-> mass_sw == 5)                      //gl_rent
         Mass=14828.0;
     else if (config_pt-> mass_sw == 6)                      //bl_rent_2
         Mass=15212.0; 
     else
         Mass=15000.0;

     mass_tmp= (Mass-8381.0)/23413.0;      
     f_0= 0.06175*(1.0-mass_tmp) + 0.05175*mass_tmp;
     
     we_temp=we_flt/1000.0;   
     r_star = rg*rd;
     r_starh = r_star*hr;

     //  Gear shift time bound, average value from data reading
     if ( (sens_read_pt-> gshift_sw == ON) && ((int)gear_flt == 1)) 
          gshift_t=1.56;
     else if ( (sens_read_pt-> gshift_sw == ON) && ((int)gear_flt == 2)) 
          gshift_t=1.56;
     else if ( (sens_read_pt-> gshift_sw == ON) && ((int)gear_flt == 3)) 
          gshift_t=1.8;
     else if ( (sens_read_pt-> gshift_sw == ON) && ((int)gear_flt == 4)) 
          gshift_t=1.6;
     else if ( (sens_read_pt-> gshift_sw == ON) && ((int)gear_flt == 5)) 
          gshift_t=1.6;
     else;

    
     s_2 = v-con_state_pt-> ref_v;               
     
/*     if (veh_id == 0 || veh_id ==1)                      // Used at CRO
        {
           temp_dist = following_dist;
           if( (man_id != 0) && (man_id != 30) )
               eps += s_2 * delta_t;
           distance = following_dist - eps;  
        }
     else        
        cntract(delta_t, rate_1, rate_2, distance, following_dist, &temp_dist, &temp_dist_d); 
     s_1 = -(distance - temp_dist);         
*/

     if (con_state_pt-> drive_mode == 3)
        s_1=0.0;
     else
        s_1 = -(con_state_pt-> front_range - con_state_pt-> temp_dist);
 
     //w_t = fabs(con_state_pt-> ref_v*9.5493/max((rg*rd*hr),0.05));    // For old freightliner
    // w_t = sens_read_pt-> w_t;                           // For new freightliner j_bus reading, 04_23_03
     w_t = sens_read_pt-> trans_out_we/max(rg,0.05);       // For new freightliner,               04_23_03

         

    /////*** Upper

     temp3 += s_2*delta_t;
     slid = temp3 + k1*s_2;
     if (t_flt<0.07)
        slid_old = slid;
     else
        slid = 0.3*slid_old + 0.7*slid;        //svg3(slid);
     slid_dot = (slid - slid_old)/delta_t;     
     slid_old=slid;   
     temp5a = slid_dot + k2*slid;  // + 0.3*sign(slid)*pow(fabs(SAT(0.15,slid)),2.0);      
     if (t_flt<0.07)
         {
           s_1_old = s_1;
           s_1_flt = s_1;
           slid1_old = slid1;
           rst_pitch_old=rst_pitch; 
         }
      else
         s_1_flt = 0.2*s_1_old + 0.8*s_1;      //svg3a(s_1);  //(0.3, 0.7)          
      s_1_d = (s_1_flt - s_1_old)/delta_t;
      slid1 = s_1 + k1*s_1_d;
      slid1 = 0.2*slid1_old + 0.8*slid1;       //svg3b(slid1); //(0.3, 0.7) 
      slid1_dot = (slid1 - slid1_old)/delta_t;
      s_1_old = s_1;                           //s_1_flt;
      slid1_old = slid1;
      temp5b = slid1 + k2*slid1_dot;  // + 0.3*sign(slid1)*pow(fabs(SAT(0.15,slid1)),2.0);
      temp5 = config_pt-> para1*0.85*temp5a + (1.0-config_pt-> para1*0.85)*temp5b;
      //ff_gain = max(ff_gain_old,max(20.0,(v-45.0)/5.0));
      Max_v_tmp=0.8*Max_spd;
      if (v < Max_v_tmp)
         ff_gain = config_pt-> para2*2.0 + 0.75*(Max_v_tmp-v)/Max_v_tmp;   //1.75; 2.00   //max(ff_gain_old,max(0.0,(v-45.0)/5.0));             //May 20,  02
      else
          ff_gain = config_pt-> para2*2.0 + 0.75;   //2.75; 2.00 better than 2.75 for grading down; 1.5 is better
      ff_gain_old=ff_gain;
   
      usyn= -(k1+k2)*s_2 - k1*k2*s_1 + con_state_pt-> ref_a;  
      //usyn=usyn_flt(usyn);
      

    /////*** Mdl
      
        
        if (sw_pt->fan_sw == ON)                                                        
           fan_tq=94.6*we_temp-41.5;
        else                                                             
           fan_tq=0.0;
        if (sw_pt->alt_sw == ON)
           alt_tq=4.5*we_temp-2.0;
        else
           alt_tq=0.0;
        if (sw_pt->comp_sw == ON)
            comp_tq=5.3*we_temp-2.3;
        else
           comp_tq=0.0;
        if (sw_pt->cond_sw == ON)           
           cond_tq=10.6*we_temp-4.7;
        else
           cond_tq=0.0;
        if (sw_pt->steer_sw == ON)           
           steer_tq=5.3*we_temp-2.3;
        else
           steer_tq=0.0;

       
        if (tg<0.04)
           {
              temp_t=0.0;
              I_trans_old=I_trans;
           }                   
        if ( fabs(gear_flt-1.0) <= 0.001)           
              I_trans=11.27;
        else if (fabs(gear_flt-2.0) <= 0.001)
              I_trans=2.97;                
        else if (fabs(gear_flt-3.0) <= 0.001)           
              I_trans=1.89;     
        else if (fabs(gear_flt-4.0) <= 0.001)           
              I_trans=1.46;
        else if (fabs(gear_flt-5.0) <= 0.001)           
              I_trans=0.86;         
        else if (fabs(gear_flt-6.0) <= 0.001)           
              I_trans=0.77;           
        else if (fabs(gear_flt+1.0) <= 0.001)           
              I_trans=16.27;         
        else;                                 
        if (temp_t <= 3.0)   
           {
               temp_t += delta_t;           
               I_trans_flt = I_trans_old*(3.0-temp_t)/3.0+(temp_t/3.0)*I_trans;
           }
        else
           I_trans_flt = I_trans;
           
      
                   
        
        //*****************
        
     
      
        //*** MASS estimation based on vehicle model

#ifdef MASS_ESTIMATION          
        
        if (t_flt < 20.0 && pre_v < 20.0)  
           {
              if (f_f_hat > 500.0)
                 Mass += f_f_hat*delta_t/20.0;              
                      else if (f_f_hat < -500.0)
                 Mass -= f_f_hat*delta_t/20.0;
              else;
              if (Mass < 7727.0)
                 Mass = 7727.0;
              else if (Mass > 18182.0)
                 Mass = 18182.0;
           }   

#endif


          
        eff_in = (Mass*hr*hr+I_WHL+0.7*I_drive)*r_star*r_star + I_ENG;                        
        beta_tmp = eff_in/r_starh + (I_trans_flt+0.3*I_drive)*rg/(rd*hr);           
         
        if (man_id == 30 || man_id == 0)
           rst_pitch = 0.0;           
        else if (man_id == 29)
           {   
              
               rst_pitch = (0.1*con_state_pt-> ref_v/Max_spd + 0.025)*(1.0 - 0.5*acl)*sin(pitch)*Mass*G*r_starh;     // Apr 17 02
              //rst_pitch = (0.45*con_state_pt-> ref_v/Max_spd + 0.25)*(1.0 - 0.5*acl)*sin(pitch)*Mass*G*r_starh;     // Apr 19 02
                                                   
              // rst_pitch = 0.5*(1.0 - 0.6*acl)*sin(pitch)*Mass*G*r_starh;              
              if (rst_pitch > rst_pitch_old + 1.0)     //2.0, 1.8
                 rst_pitch = rst_pitch_old + 1.0;
              if (rst_pitch < rst_pitch_old - 1.0) 
                 rst_pitch = rst_pitch_old - 1.0;
              rst_pitch_old = rst_pitch;              
           } 
        else //if (man_id == 3)
           {     
              rst_pitch = 0.45*(1.0 - acl)*sin(pitch)*Mass*G*r_starh;                         //   Apr 19 02
              //rst_pitch = 0.4*sin(pitch-0.1*acl)*Mass*G*r_starh;                             //   Apr 12 02
              if (rst_pitch > rst_pitch_old + 1.0)             
                 rst_pitch = rst_pitch_old + 1.0;
              if (rst_pitch < rst_pitch_old - 1.0) 
                 rst_pitch = rst_pitch_old - 1.0;
              rst_pitch_old = rst_pitch;
           } 
                                       
      //  else 
      //     {
      //        rst_pitch = 0.3*(1.0 - 0.2*acl)*sin(pitch)*Mass*G*r_starh;
      //        rst_pitch_old = rst_pitch;
      //     }
          
          
        v_tmp=v/30.0;
        rst_total = c_a*v*v*r_starh + f_0*Mass*r_starh                                                                 // May 13 02 & Aug 27-28                                                                        
        //rst_total = c_a*v*v*r_starh + (f_0+f_1*v_tmp + f_2*pow(v_tmp,4.0) )*Mass*r_starh      // Aug 28 02 in last runs
                         + (fan_tq+alt_tq+comp_tq+steer_tq+comp_tq);                             
        temp1 = beta_tmp*usyn + r_starh*f_f_hat + rst_total + rst_pitch;
          
       // engbrk_tq = beta_tmp*acl  + rst_total;                                                 
        engbrk_tq = beta_tmp*con_state_pt-> ref_a  + rst_total;                                                      //     Apr 24 02

      

          w_p=we_flt;


        //w_p = sens_read_pt-> we_flt;                     // 04_24_03               
        if (w_p > 2400.0)                                                    
           w_p = 2400.0;
        if (w_p < 600.0)
           w_p=600.0;

        //w_p = sens_read_pt-> w_p;                      // From J-1939 Bus, 04_23_03               
                            
        temp0 =  108.224*we_temp+59.3808;                   
        temp1 = temp1/(-0.9985*w_t/w_p + 1.8915)+ temp0;                         
        rst_total = rst_total + temp0;                                                                                                            
        Mass_est=Mass;
        tnet_min = rst_total;
        
        tnet_des=temp1;                            
        if (tnet_des > 1966.0)  // Changed on 08_08_03         
           tnet_des = 1966.0;
        if (tnet_des < -5000.0)
           tnet_des=-5000.0; 
                   
       
    
        /**********************************************************/
        /*                                                        */
        /*         Switching logic and lower level control        */
        /*                                                        */
        /**********************************************************/



        if ( tnet_des >= 0.0)           
           {                          
              output_pt-> con_sw_1=1;
              output_pt-> con_sw_2=1;
              adap_a = 1;                                                              
              adap_b = 0;            
              tnet_a = tnet_des;
              tnet_b = 0.0;
              output_pt-> con_sw_3=0;
              output_pt-> con_sw_4=0;
              output_pt-> con_sw_5=0;
           }
        else     
           {
              output_pt-> con_sw_1=0;
              output_pt-> con_sw_2=0;
              adap_a = 0;                                                            
              adap_b = 1;             
              tnet_a = tnet_min;
              tnet_b = tnet_des - tnet_min;    
           }
      


        /////*** Lower_1

       if( t_flt < 0.07 )
           pm_old = pm;          
       else
          {

            if( (t_flt <= 1.5) || (tg < 0.8) || (tg > 1.5) )
                {
                      if( pm - pm_old > 0.2 )
                            pm = pm_old + 0.2;
                      else if( pm - pm_old < -0.2 )
                            pm = pm_old - 0.2;
                }
            else
                {
                      if( pm - pm_old > 0.8 )
                            pm = pm_old + 0.8;
                      else if (pm - pm_old < -0.8)
                            pm = pm_old - 0.8;
                }
          }             
        pm_old=pm;
                
        // alp_des = eng_map(we_flt, tnet_a,1,3,Index);
        
                   
        if (con_state_pt-> ref_a > 0.08) 
           {                    
                alp_des = eng_map(we_flt, tnet_a,1,3,Index);   // 04/01/03                 
                pm_des = eng_map(we_flt,tnet_a,1,2,Index);
                tnet_a_cal = eng_map(we_flt, sens_read_pt-> fuel_m,3,1,Index);
           }  
        else
           {
                alp_des = eng_map(w_p, tnet_a,1,3,Index);
                alp_des = 0.8219*alp_des;                      // Based on run data; It is too high from engine mapping                
                pm_des = eng_map(w_p,tnet_a,1,2,Index);
                tnet_a_cal = eng_map(we_flt, sens_read_pt-> fuel_m,3,1,Index);
                tnet_a_cal=0.8219*tnet_a_cal;                // To see if needs such coefficient
           }                    
             //pm= pm_filt(pm);
        s_3=pm-pm_des;
           
        
 
     //   usyn1 = - 0.1*s_3;        //+2.0*pm_temp);
     //   if (t_flt <= 10.0)
     //     alp_des = alp_des + 0.3*usyn1*t_flt/10.0;
     //   else      
     //     alp_des = alp_des + (0.3+0.5*fabs(s_1))*usyn1;    
          
        /**********************************************************/
                 
        //if( (t_flt < 1.6) || (tg > 1.6) )                               //Tested for gear 1 ==> gear 2 at RFS
        if( (t_flt < 1.6) || (tg > gshift_t) )                            //2.0
           {
             tnet_a = 0.4*tnet_a + 0.6*tnet_a_old;                        //New                                  
             tnet_a_old = tnet_a;
           }
        else
           {                              
             temp2=0.35*sin(PI*2.0/2.4*tg);                              
             tnet_a = 0.4*(1.0-temp2)*tnet_a + 0.6*tnet_a_old;                                            
             tnet_a_old = tnet_a;  
           }
                   
        /**********************************************************/
                           
        if( (t_flt < 1.6) || (tg > 1.6) )                               //2.0
           {
             alp_des = 0.2*alp_des + 0.8*alp_des_old;                      //New                                  
             alp_des_old = alp_des;
           }
        else
           {                              
             temp1=0.3*sin(PI*2.0/2.4*tg);                              
             alp_des = 0.5*(alp_des-temp1) + 0.5*alp_des_old;                                            
             alp_des_old = alp_des;  
           }
                   
//#ifdef ENG_CONTROL_1
                                 
        alp_des = max(alp_des,0.0);
        alp_des = min(alp_des,100.0);               // BUG LINES  XY_Lu    04/01/03
        
//#endif    

        /////*** Lower_2

                 /*--- Brake Control ---*/
                                 




// Desired brake pressure

        if( t_flt < 0.07 )
            {
                brake_sw = -1;
                pb_des_0 = 0.0;
                pb_des_2 = 0.0;
                pb_des_4 = 0.0;
                pb_des_6 = 0.0;               
            }
            
     // Complicated logic with relay

     if (man_id != 29)
        {
     
           if( (brake_sw == 0) && (tnet_des > 30.0) )               //30 ,   50                                              
              brake_sw = -1;
           else if( (brake_sw == -1) && (tnet_des <= 20.0) )       //20  ,  40  
              brake_sw = 0;                                                          
           else if( (brake_sw == 1) && (tnet_des >= -60.0) )       //-40 , -50                                                     // Apr 24 02
              brake_sw = 0;
           else if ( (brake_sw == 0) && (tnet_des < -50.0) )        //-30 , -40                                                                                      
              brake_sw = 1;
           else;
       }
     else
        {
           if( (brake_sw == 0) && (tnet_des > 30.0) )               //30 ,   50                                              
              brake_sw = -1;
           else if( (brake_sw == -1) && (tnet_des <= 20.0) )       //20  ,  40  
              brake_sw = 0;                                                          
           else if( (brake_sw == 1) && (tnet_des >= -50.0) )       //-40 , -50                                                     // 05_01_03
              brake_sw = 0;
           else if ( (brake_sw == 0) && (tnet_des < -150.0) )        //-30 , -40                                                                                      
              brake_sw = 1;
           else;
        }   

        

        // Simple logic, No relay
           
    //    if (tnet_des > 30.0)
    //          brake_sw = -1;           
    //    else if (tnet_des > -50.0)                                                                                             // Apr 29 02
    //          brake_sw = 0;
    //    else                                                                                            
    //          brake_sw = 1;

     /*--- Transmission retarder ---*/

//#ifdef TRANSMISSION

       //Suppose transmission work between 10~80[mph]
        if (v > 5.0)  //7.0                                                    // transmission retarder torque
          max_trtd_tq = 1350.0*(v - 2.0)/(28.0*rg);      // A function of v or transmission output speed
        else
          max_trtd_tq = 0.0;

//#endif

 

        if ( tnet_b < 0.0 )
            {
                jk_tmp = 2.4*min(1.0,(we_flt-800.0)/1200.0)/Max_dcc;       //1.1. used on Oct. 30, 03 introduce idle spd, 10_01_03; 0.75
             
                //jk_brk_2 = jk_tmp*jk_brk_2;
                //jk_brk_4 = jk_tmp*jk_brk_4;
                //jk_brk_6 = jk_tmp*jk_brk_6;
                
              //pb_s2_p =   3.0*sign(s_2)*min(fabs(s_2),0.2)*272.48;
                pb_s2_p = 3.0*s_2*272.48;
                                
                //f_b_hat -= 0.0005* sign(s_2)*min(fabs(s_2),0.2)*272.48;                                 
                pb_des_0 = -f_b_hat*3.57*(tnet_b+rst_total)/beta_tmp + pb_s2_p;        // g-force to psi;   // Mar 31 02                                
               
               // pb_des_2 = (-f_b_hat*tnet_b + 0.80*beta_tmp/1.19)*3.57/beta_tmp;
               //pb_des_2 = -f_b_hat*3.57*(tnet_b + 300.0 - rst_total)/beta_tmp;                            // Apr 02 02 
               //pb_des_2 = -f_b_hat*3.57*(tnet_b + jk_brk_2/Max_dcc +rst_total)/beta_tmp  + pb_s2_p;
                pb_des_2 = -f_b_hat*3.57*(tnet_b + jk_tmp*jk_brk_2 +rst_total)/beta_tmp + pb_s2_p;          // Aug 08 03
                                         
               // pb_des_4 = (-f_b_hat*tnet_b + 0.95*beta_tmp/1.19)*3.57/beta_tmp;
               // pb_des_4 = -f_b_hat*3.57*(tnet_b + 400.0 + rst_total)/beta_tmp;
               //pb_des_4 = -f_b_hat*3.57*(tnet_b + jk_brk_4/Max_dcc +rst_total)/beta_tmp + pb_s2_p;                               
                pb_des_4 = -f_b_hat*3.57*(tnet_b + jk_tmp*jk_brk_4 +rst_total)/beta_tmp + pb_s2_p;
                 
               // pb_des_6 = (-f_b_hat*tnet_b + 1.10*beta_tmp/1.19)*3.57/beta_tmp;
               // pb_des_6 = -f_b_hat*3.57*(tnet_b + 500.0 + rst_total)/beta_tmp;
               //pb_des_6 = -f_b_hat*3.57*(tnet_b + jk_brk_6/Max_dcc +rst_total)/beta_tmp + pb_s2_p;
                pb_des_6 = -f_b_hat*3.57*(tnet_b + jk_tmp*jk_brk_6 +rst_total)/beta_tmp + pb_s2_p;                                                                                                      
            }
            
       
 


  // Brk control begin
  //  if (adap_b == 1)                                                                      // Original
  if (brake_sw == 1)
      {
       alp_des = 0.0;
            
       #ifdef JK_TRTD_EBS     
       if (man_id == 29)  
         {
            if ( we_flt < 640.0 || con_state_pt-> ref_v < 1.0)              // 04/15
               {                  
                  output_pt-> con_sw_1=0;
                  output_pt-> con_sw_2=0;
                  output_pt-> con_sw_5=1;
                  pb_des=pb_des_0;
               }
            else
               {               
                  if (-tnet_b < jk_tmp*jk_brk_2/Max_dcc + rst_total)           //0.9*beta_tmp/1.427)
                     {
                         if (-tnet_b < rst_total)
                            {
                               jk_brk = 0.0;      
                               trtd_des = 0.0;
                               output_pt-> con_sw_3=0;
                               output_pt-> con_sw_4=0;
                               output_pt-> con_sw_5=0;
                            }
                         else // (-tnet_b < max_trtd_tq + rst_total)           //0.9*beta_tmp/1.427)
                            {
                               jk_brk = 0.0;      
                               //trtd_des = tnet_b + rst_total;
                               trtd_des = pb_des_0;
                               output_pt-> con_sw_1=0;
                               output_pt-> con_sw_2=0;
                               output_pt-> con_sw_3=0;
                               output_pt-> con_sw_4=1;
                               output_pt-> con_sw_5=0;
                            }     
                   
                     }
                  else if (-tnet_b < jk_tmp*jk_brk_6/Max_dcc + rst_total + max_trtd_tq)
                     {    
                         if (-tnet_b < jk_tmp*jk_brk_4/Max_dcc + rst_total)     //1.1*beta_tmp/1.427)
                            {
                               jk_brk = jk_brk_2;
                               //trtd_des = tnet_b + rst_total+jk_brk_2/Max_dcc;     //04_30_03                         
                               trtd_des = pb_des_2;
                               output_pt-> con_sw_3=1;
                               output_pt-> con_sw_4=1;
                               output_pt-> con_sw_5=0;                                        
                            }
                         else if (-tnet_b < jk_tmp*jk_brk_6/Max_dcc + rst_total)     //1.1*beta_tmp/1.427)
                            {                  
                               jk_brk = jk_brk_4;
                               //trtd_des = tnet_b + rst_total+jk_brk_4/Max_dcc;     //04_30_03                          
                               trtd_des = pb_des_4;
                                                           output_pt-> con_sw_1=0;
                               output_pt-> con_sw_2=0;
                               output_pt-> con_sw_3=1;
                               output_pt-> con_sw_4=1;
                               output_pt-> con_sw_5=0;                                      
                            }
                         else
                            {
                               jk_brk = jk_brk_6;
                               //trtd_des = tnet_b + rst_total+jk_brk_6/Max_dcc;     //04_30_03                           
                               trtd_des = pb_des_6;
                               output_pt-> con_sw_1=0;
                               output_pt-> con_sw_2=0;
                               output_pt-> con_sw_3=1;
                               output_pt-> con_sw_4=1;
                               output_pt-> con_sw_5=0;                         
                            }
                     }
                  else
                     {
                         jk_brk = jk_brk_6;
                         trtd_des = -1300.0;   // max_trtd_tq;
                         output_pt-> con_sw_1=0;
                         output_pt-> con_sw_2=0;
                         output_pt-> con_sw_3=1;
                         output_pt-> con_sw_4=1;
                         output_pt-> con_sw_5=1;    
                         pb_des = pb_des_6 + max_trtd_tq;
                        
                     }

               }
                                
            
            brake=pb_des*brk_coeff;
            
            pb_des_old = pb_des;             // Transition to man_id == 10
            pb_temp = 0.0;
            
            if (brake > 28.0)
              brake=28.0;

         }   // (man_id == 29) END   
       #endif //JK_TRTD_EBS END      
                 
 

       #ifdef JK_EBS 
       if (man_id == 29) 
         {
            if ( we_flt < 640.0 || con_state_pt-> ref_v < 1.0)              // 04/15
               {                  
                  output_pt-> con_sw_1=0;
                  output_pt-> con_sw_2=0;
                  output_pt-> con_sw_5=1;
                  pb_des=pb_des_0;
               }
            else
               {               
                  if (-tnet_b < jk_tmp*jk_brk_2/Max_dcc + rst_total)           //0.9*beta_tmp/1.427)
                     {
                         if (-tnet_b < rst_total)
                            {
                               jk_brk = 0.0;      
                               trtd_des = 0.0;
                               output_pt-> con_sw_3=0;
                               output_pt-> con_sw_4=0;
                               output_pt-> con_sw_5=0;
                            }
                         else // (-tnet_b < max_trtd_tq + rst_total)           //0.9*beta_tmp/1.427)
                            {
                               jk_brk = 0.0;      
                               //pb_des = tnet_b + rst_total;
                               output_pt-> con_sw_1=0;
                               output_pt-> con_sw_2=0;
                               trtd_des = pb_des_0;
                               output_pt-> con_sw_3=0;
                               output_pt-> con_sw_4=0;
                               output_pt-> con_sw_5=1;
                            }     
                   
                     }
                  else if (-tnet_b < jk_tmp*jk_brk_6/Max_dcc + rst_total)
                     {    
                         if (-tnet_b < jk_tmp*jk_brk_4/Max_dcc + rst_total)     //1.1*beta_tmp/1.427)
                            {
                               jk_brk = jk_brk_2;
                                                      
                               pb_des = pb_des_2;
                               output_pt-> con_sw_1=0;
                               output_pt-> con_sw_2=0;
                               output_pt-> con_sw_3=1;
                               output_pt-> con_sw_4=0;
                               output_pt-> con_sw_5=1;                                        
                            }
                         else if (-tnet_b < jk_tmp*jk_brk_6/Max_dcc + rst_total)     //1.1*beta_tmp/1.427)
                            {                  
                               jk_brk = jk_brk_4;
                                                      
                               pb_des = pb_des_4;
                               output_pt-> con_sw_1=0;
                               output_pt-> con_sw_2=0;
                               output_pt-> con_sw_3=1;
                               output_pt-> con_sw_4=0;
                               output_pt-> con_sw_5=1;                                      
                            }
                         else
                            {
                               jk_brk = jk_brk_6;
                                                        
                               pb_des = pb_des_6;
                               output_pt-> con_sw_1=0;
                               output_pt-> con_sw_2=0;
                               output_pt-> con_sw_3=1;
                               output_pt-> con_sw_4=0;
                               output_pt-> con_sw_5=1;                         
                            }
                     }
                  else
                     {
                         jk_brk = jk_brk_6;
                         output_pt-> con_sw_1=0;
                         output_pt-> con_sw_2=0;
                         output_pt-> con_sw_3=1;
                         output_pt-> con_sw_4=0;
                         output_pt-> con_sw_5=1;    
                         pb_des = pb_des_6;
                        
                     }

               }
                                
            
            brake=pb_des*brk_coeff;
            
            pb_des_old = pb_des;             // Transition to man_id == 10
            pb_temp = 0.0;
            
            if (brake > 28.0)
              brake=28.0;

         }   // (man_id == 29) END
       #endif //JK_EBS END

       #ifdef EBS 
       if (man_id == 29) 
         {
            if ( we_flt < 640.0 || con_state_pt-> ref_v < 1.0)              // 04/15
               {  
                  output_pt-> con_sw_1=0;
                  output_pt-> con_sw_2=0;                
                  output_pt-> con_sw_1=0;
                  output_pt-> con_sw_2=0;
                  output_pt-> con_sw_5=1;
                  pb_des=pb_des_0;
               }
            else
               {               
                  
                         if (-tnet_b < rst_total)
                            {
                               jk_brk = 0.0;      
                               trtd_des = 0.0;                                                     
                               output_pt-> con_sw_3=0;
                               output_pt-> con_sw_4=0;
                               output_pt-> con_sw_5=0;
                            }
                         else // (-tnet_b < max_trtd_tq + rst_total)           //0.9*beta_tmp/1.427)
                            {
                               jk_brk = 0.0;      
                               pb_des = pb_des_0; 
                                                           output_pt-> con_sw_1=0;
                               output_pt-> con_sw_2=0;                              
                               output_pt-> con_sw_3=0;
                               output_pt-> con_sw_4=0;
                               output_pt-> con_sw_5=1;
                            }                        
                           
               }
                                            
            brake=pb_des*brk_coeff*1.8;      // Larger than other cases,1.5 for full, 1.8 for Empty, 05_02_03
            
            pb_des_old = pb_des;             // Transition to man_id == 10
            pb_temp = 0.0;
            
            if (brake > 40.0)
              brake=40.0;

         }   // (man_id == 29)
       #endif  //EBS END

       
       else if (man_id != 30)  // if (man_id == 1,3 or other maneuvers other than E_stop), all combination of brk system;
         {                      // Using transmission retarder and jake brake only.
                        
           if (-tnet_b < jk_tmp*jk_brk_2 + rst_total)                  
               {
                   if (-tnet_b < rst_total)
                      {
                         jk_brk = 0.0;      
                         trtd_des = 0.0;
                         output_pt-> con_sw_3=0;
                         output_pt-> con_sw_4=0;
                         output_pt-> con_sw_5=0;
                      }
                   else // (-tnet_b < max_trtd_tq + rst_total)
                      {
                         jk_brk = 0.0;      
                         trtd_des = pb_des_0 + rst_total;
                         output_pt-> con_sw_3=0;
                         output_pt-> con_sw_4=1;
                         output_pt-> con_sw_5=0;
                      }     
                   
               }        
            else if (-tnet_b < jk_tmp*jk_brk_4/Max_dcc + rst_total)   
               {
                   jk_brk = jk_brk_2;
                   //trtd_des = tnet_b + rst_total+jk_brk_2/Max_dcc;
                   trtd_des =pb_des_2;
                   output_pt-> con_sw_3=1;
                   output_pt-> con_sw_4=1;
                   output_pt-> con_sw_5=0;                            
            
               }
            else if (-tnet_b < jk_tmp*jk_brk_6/Max_dcc + rst_total)     //1.1*beta_tmp/1.427)
               {                  
                   jk_brk = jk_brk_4;
                  // trtd_des = tnet_b + rst_total+jk_brk_4/Max_dcc;
                   trtd_des =pb_des_4;
                   output_pt-> con_sw_3=1;
                   output_pt-> con_sw_4=1;
                   output_pt-> con_sw_5=0;                                      
               }
            else
               {
                   jk_brk = jk_brk_6;
                   //trtd_des = tnet_b + rst_total+jk_brk_6/Max_dcc;
                   trtd_des =pb_des_6;
                   output_pt-> con_sw_3=1;
                   output_pt-> con_sw_4=1;
                   output_pt-> con_sw_5=0;                         
               }           
         }
      else if (man_id == 30)         
         {
            output_pt-> con_sw_1=0;     //tq_comd_sw
            output_pt-> con_sw_2=0;     //spd_cmd_sw    
            output_pt-> con_sw_3=0;     //jk_cmd_sw
            output_pt-> con_sw_4=0;     //trtd_cmd_sw      
            output_pt-> con_sw_5=1;     //brk_cmd_sw 
            brake =30.0;
         }
      else;      
      //brake = brk_flt(brake);               // For DBG                                                           
      }
  else    //if (brake_sw ==0)
      {
          brake=INI_BRK;
          jk_brk = 0.0;
          trtd_des = 0.0;
          output_pt-> con_sw_3=0;
          output_pt-> con_sw_4=0;
          output_pt-> con_sw_5=0;   
      }
  
  // Braking System  Control End

#ifdef AIR_BRAKE
     
     if (man_id == 29)       
         {
                  if (con_state_pt-> ref_v > 0.5)  //&& (con_state_pt-> ref_v > 0.05) )
                           {                          
                               output_pt-> con_sw_1=1;     //tq_comd_sw
                               output_pt-> con_sw_2=1;     //spd_cmd_sw       
                               output_pt-> con_sw_3=1;     //jk_cmd_sw
                               output_pt-> con_sw_4=1;     //trtd_cmd_sw    
                               output_pt-> con_sw_5=0;
                           }
                  else
                           {
                  brake = brake + 4.0*(s_2);
                  tnet_a = 0.0;
                  output_pt-> con_sw_1=0;     //tq_comd_sw
                  output_pt-> con_sw_2=0;     //spd_cmd_sw
                  output_pt-> con_sw_3=1;     //jk_cmd_sw
                  output_pt-> con_sw_4=1;     //trtd_cmd_sw        
                  output_pt-> con_sw_5=1;     //brk_cmd_sw
                                  
                           }                
         }
     if (man_id == 30)          
         {
            output_pt-> con_sw_1=0;     //tq_comd_sw
            output_pt-> con_sw_2=0;     //spd_cmd_sw    
            output_pt-> con_sw_3=0;     //jk_cmd_sw
            output_pt-> con_sw_4=0;     //trtd_cmd_sw      
            output_pt-> con_sw_5=1;     //brk_cmd_sw 
            brake =30.0;
             }
                 
                 
     if ((man_id == 29 || man_id == 30) && ((we_flt < 808.0) || (con_state_pt-> ref_v < 2.0)) && (brake < 1.6*Max_dcc*Max_dcc) )
         {        
            brake = 1.6*Max_dcc*Max_dcc; 
            output_pt-> con_sw_5=1;     //brk_cmd_sw
            output_pt-> con_sw_1=0;
            output_pt-> con_sw_2=0;    
         }          
     if (man_id == 30)
         {            
            if(pb_temp < 2.0)
               pb_temp += delta_t;
            pb_des = (MAX_BRK-pb_des_old)*pb_temp/2.0 + pb_des_old; 
            brake=pb_des*brk_coeff;
            output_pt-> con_sw_5=1;     //brk_cmd_sw
            output_pt-> con_sw_1=0;
            output_pt-> con_sw_2=0;          
         }
       
#endif

     

      /**********************************

              Command management
              for transition control
              
      ***********************************/    
      
      //trtd_des=pm_filt(trtd_des);

      if (man_id == 45 || man_id == 4 )  // Transition
         {
             tnet_a=tnet_a_cal;
         }



      /*********************************

             Command limiter
             
      **********************************/

      if (trtd_des > 0.0)
          trtd_des = 0.0;
      
      if (manuv_id[0] == 7)
         {

            if (tnet_a > tnet_a_buff + 14.5)               // added on 10_20_03
                 tnet_a=tnet_a_buff + 14.5;
            if (tnet_a < tnet_a_buff - 14.5)
                 tnet_a=tnet_a_buff - 14.5;
            if (tnet_b > tnet_b_buff + 14.5)
                 tnet_b = tnet_b_buff + 14.5;
            if (tnet_b < tnet_b_buff - 14.5)
                 tnet_b = tnet_b_buff - 14.5;
         }
      else //if (manuv_id[0] != 3) 
         {

            if (tnet_a > tnet_a_buff + 16.5)               // added on 10_20_03
                 tnet_a=tnet_a_buff + 16.5;
            if (tnet_a < tnet_a_buff - 16.5)
                 tnet_a=tnet_a_buff - 16.5;
            if (tnet_b > tnet_b_buff + 20.5)
                 tnet_b = tnet_b_buff + 20.5;
            if (tnet_b < tnet_b_buff - 20.5)
                 tnet_b = tnet_b_buff - 20.5;
         }
      //else;

      tnet_a_buff = tnet_a;
      tnet_b_buff = tnet_b;
         
           
           
      output_pt-> y1 = tnet_a;          // 9                        
      output_pt-> y2 = alp_des;         // 10                       
      output_pt-> y3 = w_p;             // 11
      output_pt-> y4 = w_t;             // 12
      output_pt-> y5 = tnet_a_cal;      // 13
      output_pt-> y6 = tnet_b;          // 14
      output_pt-> y7 = pb_des;          // 15
      output_pt-> y8 = s_1;             // 16
      output_pt-> y9 = s_2;             // 17
      output_pt-> y10 = trtd_des/135.0; // 18
      output_pt-> y11 = pm_des;         // 19
      output_pt-> y12 = f_f_hat;        // 20
      output_pt-> y13 = f_b_hat;        // 21
      output_pt-> y14 = pb_s2_p;        // 22         
      output_pt-> y15 = engbrk_tq;      // 23   engine braking effect
      output_pt-> y16 = brake*0.5;      // 24                            After ECM changing on 08_01_03
      output_pt-> y17 = jk_brk;         // 25   jk_tq
      output_pt-> y18 = rst_total;      //      temp_dist;      //      transmission retarder tq
      output_pt-> y19 = jk_brk_2;       // 
      output_pt-> y20 = jk_brk_4;       //     
                 
   
 return 1;
           
}




float sign(float y)
 {
   static float x=0.0;

   x=y;
   if(x>0.0) 
     return 1.0;
   else if (x<0.0) 
     return -1.0;
   else
     return 0.0;
 }              
