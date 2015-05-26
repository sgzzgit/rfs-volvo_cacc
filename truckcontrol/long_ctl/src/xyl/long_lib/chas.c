/****************************************************************************
 

*****************************************************************************/
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include <local.h>
#include <sys_ini.h>
#include <sys_list.h>
#include <sys_buff.h>
#include <sys_rt.h>

#include "track.h"
#include "veh_iotr.h"
#include "db_comm.h"
#include "db_clt.h"
#include "clt_vars.h"


#define MAX_BUFFER_SIZE    10000
#define MAX_THROTTLE       4.5
#define MIN_THROTTLE       1.25
#define MAX_BRAKE          3.0
#define MIN_BRAKE          1.2
#define pi                 3.14159
#define ON                 1
#define OFF                0

extern bool_typ verbose_flag;

db_clt_typ *pclt=NULL;       /* Database client pointer */

/* The following defines the experimental profile for the leading vehicle 
and the follower. */

/* static float time; */
static float start_time;
static float now;
static buff_typ *pbuff;

/* this routine is for obtaining desired truck velocity, whose breaking
   points are installed in realtime.ini. The desired velocity is computed
   by interpolation between those breaking points  */



bool_typ spd_init( char *pname, char *psect, char *argv[])
{
      
    char hostname[MAXHOSTNAMELEN+1]; 
    now=get_sec_clock();
    start_time=get_sec_clock();

    if( ( pbuff = buff_init( stdout, MAX_BUFFER_SIZE ) ) == NULL) 
    return(FALSE);

        /* Log in to the database (shared global memory).  Default to the
         * current host. */
#ifdef __QNX4__
        sprintf( hostname, "%lu", getnid() );
#else
        get_local_host(hostname, MAXHOSTNAMELEN);
#endif
    if (( pclt = clt_login( argv[0], hostname, DEFAULT_SERVICE,
              COMM_OS_XPORT)) == NULL )
         {
            printf("Database initialization error in demo_pid\n");
            return( FALSE );
         }
   
    return(TRUE);
}    



bool_typ spd_ctrl( long_input_typ const *pinput, track_control_typ const *ptrack,
                   long_output_typ *poutput)
{
    const float max_tst_spd = 30.0;                 // Step 5.0
    const float we_step=15.0;
    const float fuel_step=0.1, brk_step=0.02;
    static float time_step=0.0, dt=0.0, time_flt=0.0;
    static float v=0.0, lead_vel=0.0, vel=0.0, acl=0.0;
    static float fuel=0.0, w=0.0, we=0.0, pm=0.0, eng_temp=0.0;
    static float jk_so1=0.0, jk_so2=0.0, jk_sw1= 0.0, jk_sw2= 0.0; 
    float brake=0.0, bp=0.0;
    static int gear=0, fuel_sw=ON;
    static float gear_flt=0.0;
    char buffer[MAX_LINE_LEN+1];   
    float control=0.0;
    static float ui=0.0; /* integral velocity error for integral term */
    static float ud=0.0; /* derivative term state */
    int marker_counter=0;     /* Relative marker number */
    int marker_number=0;      /* Absolute marker number */
    static int demo_flag=0;
    db_data_typ db_data;
    marker_pos_typ *pmarker_pos;
    long_input2_typ *pinput2;
 
        /* Retireve information from the database. */
    if ( clt_read( pclt, DB_MARKER_POS_VAR, DB_MARKER_POS_TYPE,
                       &db_data ) == FALSE )
    fprintf( stderr, "clt_read( DB_MARKER_POS_VAR ) in demo_pid\n");
    pmarker_pos = (marker_pos_typ *) db_data.value.user;

    if ( clt_read( pclt, DB_LONG_INPUT2_VAR, DB_LONG_INPUT2_TYPE,
                       &db_data ) == FALSE )
    fprintf( stderr, "clt_read(DB_LONG_INPUT2_VAR) in demo_pid\n");
    pinput2 = (long_input2_typ *) db_data.value.user;
 

    /* to obtain the time interval between two sampling times, now is
       used to install last sampling time point */

    time_step = get_sec_clock() - now;  //Real time step instead of 0.02 is used
//    now=get_sec_clock();
    time_flt += time_step;

          /**************************************/
          /*      Read in sensor measurement    */
  
          /**************************************/

    time=get_sec_clock()-start_time;

//    vel=(pinput->speed1+pinput->speed2+pinput->speed3+pinput->speed4)*1.22/4.0;
   vel= pinput->speed3*1.22;    // Old
    v = vel;            //speed_filter(vel);   
    
    acl = (pinput->long_acc1+pinput->long_acc2)/2.0;
    pm = pinput->eng_press;
    eng_temp = pinput->eng_temp;
    dt = pinput->interval;
    bp =(pinput->brake_LF+pinput->brake_RF+pinput->brake_LD+pinput->brake_RD+
        pinput->brake_RB+pinput->brake_LB+pinput->brake_LTF+pinput->brake_RTF+
        +pinput->brake_LTR+pinput->brake_RTR)/10.0;   
    w = pinput->eng_rpm;         // Does this make any sense?
    we = pinput2-> eng_speed2;   /*Engine speed [rpm]*/
    jk_so1= pinput2-> jake_sol1; /*Jake solenoid 1 (2 cylinder version)*/
    jk_so2= pinput2-> jake_sol2; /*Jake solenoid 2 (4 cylinder version)*/
    jk_sw1= pinput2-> jake_sw1;  /*Jake switch 1 (2 cylinder version)*/
    jk_sw2= pinput2-> jake_sw2;  /*Jake switch 2 (4 cylinder version)*/
    marker_number = pmarker_pos->marker_number;
    marker_counter = pmarker_pos->marker_counter;

   

    

               /****************************/
               /**       Maneuver         **/
               /****************************/

    // trajectory();                 or   truck_ref_spd();    to be here.

               /****************************/
               /**       Controller       **/
               /****************************/

    //control= open_loop(ui, ud, &fuel, &brake);

    if (time<0.3)              //time_step)
       fuel=MIN_THROTTLE;
      
    if (v<max_tst_spd && fuel_sw == ON)   
       fuel += fuel_step;
     
    if (v>=max_tst_spd  || fuel>=MAX_THROTTLE)
       fuel_sw = OFF;
    if (fuel_sw == OFF)  
       fuel=MIN_THROTTLE;
    brake=MIN_BRAKE;             // 0.0;
 //  fuel=2.0;
   
               /******************************/
               /**   Output to actuatros    **/
               /******************************/

    poutput->throttle=fuel;
    poutput->brake_front=brake;
    poutput->brake_rear=brake;
    poutput->brake_left=brake;
    poutput->brake_right=brake;
    

    if(demo_flag<=0)
    {
      demo_flag = 5;
      if(verbose_flag==TRUE){
        sprintf(buffer,
      "%6.4f %6.3f %6.3f %6.3f %6.3f %6.3f %6.3f %6.3f %6.3f %6.3f %5i %6.3f %6.3f %6.3f %6.3f %6.3f %6.3f %5i %5i %6.3f %6.3f\n",
             time,              //1       Define in file scope globally 
             v,                     //2
             vel,                   //3
             pinput->speed3*1.22,   //4
             we,                    //5
             pm,                    //6
             pinput->long_acc1,     //7
             pinput->long_acc2,     //8
             acl,                   //9
             fuel,                  //10
             gear,                  //11
             brake,                 //12              
             bp,                    //13
             jk_so1,                //14
             jk_so2,                //15
             jk_sw1,                //16
             jk_sw2,                //17
             marker_counter,        //18
             marker_number,          //19
             dt,                    //20
             time_step              //21
             );
       buff_add(pbuff,buffer,strlen(buffer));
       }
   }else demo_flag--;
   return(TRUE);
}
                     
/* this subroutine is called when the speed controller(longitudinal controller
   is turned off. buff_done is used to save the buffer to disk */
bool_typ spd_done( void)
{
      bool_typ status;
      status = buff_done (pbuff);

      /* Log out from the database. */
      if (pclt != NULL)
          clt_logout( pclt );

      return( status);
}



/*********************     Sub-modulars    ************************/



/*****************
double speed_filter(double speed)
{
    static float Y[3];
    static float X[3];
    X[2]=speed;
    Y[2]=0.1483*X[0]+0.0553*X[1]+0.1483*X[2]+1.0301*Y[1]-0.3820*Y[0];
    X[0]=X[1];
    X[1]=X[2];
    Y[0]=Y[1];
    Y[1]=Y[2];
    return Y[2];
}
***********************/
float control_filter(float cntrl_input)
{
    static float Y[3];
    static float X[3];
    X[2]=cntrl_input;
    Y[2]=0.1483*X[0]+0.0553*X[1]+0.1483*X[2]+1.0301*Y[1]-0.3820*Y[0];
    X[0]=X[1];
    X[1]=X[2];
    Y[0]=Y[1];
    Y[1]=Y[2];
    return Y[2];
}


