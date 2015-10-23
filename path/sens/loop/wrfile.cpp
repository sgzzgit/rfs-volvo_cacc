/**\file
 * 
 * read.c    Process to read the virtual loop updated data from database 
 *
 *
 * Copyright (c) 2003   Regents of the University of California
 *
 *
 */

#include	<sys_os.h>

extern "C"{
	#include "local.h"
	#include "db_clt.h"
	#include "sys_rt.h"
        #include "db_utils.h"

}

#include  "cv.h"
#include  "highgui.h"
#include  "conf.h"

#include  "vloop.h"
#include  <iostream>
#include  <fstream>
#include  <string>
//#include  <math>
//#include  <time>

#include  "Indicator.h"
#include  "LoopBufList.h"
using namespace std;

static int sig_list[]=
{
	SIGINT,
	SIGQUIT,
	SIGTERM,
	SIGALRM,	// for timer
	ERROR
};
static jmp_buf exit_env;

static void sig_hand( int code )
{
	if (code == SIGALRM)
		return;
	else
		longjmp( exit_env, code );
}

int var = DB_VIRTUAL_LOOP_VAR;
int var2 = DB_LOOP_VAR;

static db_clt_typ *database_init(char *pclient, char *phost, char *pserv,int xport);
static void veh_done( db_clt_typ *pclt );
static void sig_hand( int code );

double loop0_ref = 0.000047;
double loop1_ref = 0.000131;
 
// Display
static void UpdateDisplay(int laneNum,  VIRTUAL_LOOPS_TYP *vloop, LOOPS_TYP *loop );

IplImage **g_pDisplayImage;
#define WND_WIDTH  400/MAX_VIRTUAL_LOOPS
#define WMARGIN 25
#define HMARGIN 50
#define HBAR_WIDTH 150
#define HBAR_HEIGHT 100
#define HBAR_MARGIN 10
#define CAPTION_HEIGHT 20

static char **g_wndname;
//static CvIndicatorHistoryBar g_Virtual_Loop_Bar0(HBAR_WIDTH, HBAR_HEIGHT,30,1.5,0,0);
//static CvIndicatorHistoryBar g_Virtual_Loop_Bar1(HBAR_WIDTH, HBAR_HEIGHT,30,1.5,0,0);
//static CvIndicatorHistoryBar g_Loop_Bar0(HBAR_WIDTH, HBAR_HEIGHT,30,0.15*loop0_ref,0,60);
//static CvIndicatorHistoryBar g_Loop_Bar1(HBAR_WIDTH, HBAR_HEIGHT,30,0.15*loop1_ref,0,60);

int main( int argc, char *argv[] )
{
        //  
        int opt;
	int NumOfLane;
	while((opt = getopt(argc,argv,"n:"))!=-1)
	{
		switch (opt)
		{
			case 'n':
        			NumOfLane = atoi(optarg);
				break;
			default:
				fprintf(stderr, "Usage: %s -n [int] ", argv[0]);
                                exit(EXIT_FAILURE);
		}
	}                	 	

	db_clt_typ *pclt=NULL;
	char hostname[MAXHOSTNAMELEN+1];
	VIRTUAL_LOOPS_TYP vloop_typ;
        LOOPS_TYP loop_typ;
	int recv_type;
	trig_info_typ trig_info;

	FILE *fp;
        FILE *report1, *report2;   //, *report3, *report4;
        time_t ts_int;
        double ts_msec;
        struct tm *ts;
        char *buf;
        LoopBufList Mylist[NumOfLane];
        g_wndname = new char* [NumOfLane];
        g_pDisplayImage = new IplImage* [NumOfLane];
	for ( int i = 0 ; i<NumOfLane; i++)  
	{
	 	g_wndname[i] = new char[20];
                strcpy(g_wndname[i],"Display For Lane ");
                g_wndname[i][17] = '0'+i;
                g_wndname[i][18] = '\0';		
		g_pDisplayImage[i] = cvCreateImage(cvSize(400,350),IPL_DEPTH_8U,3);
	}
 
        int stay_alive = 0;
	get_local_name(hostname, MAXHOSTNAMELEN);
	pclt = database_init(argv[0], hostname, DEFAULT_SERVICE,COMM_PSX_XPORT);

	if (pclt == NULL)
        {
	    fprintf(stderr, "Database initialization error in read\n");
	    veh_done( pclt );
	    exit( EXIT_FAILURE );
        }
	else
	{
 		puts("login sucsess");
        	fp = fopen("report.txt","w");
        }

	if( setjmp( exit_env ) != 0 )
	{
	    	veh_done( pclt );
		fclose(fp);
                for( int i = 0; i<NumOfLane;i++)
		{
                	Mylist[i].DestroyList();
			if (g_wndname[i])
				delete []g_wndname[i];
  			if (g_pDisplayImage[i])
				cvReleaseImage(&g_pDisplayImage[i]);
		}	
		exit( EXIT_SUCCESS );
	}
	else
		sig_ign( sig_list, sig_hand );


	cvStartWindowThread();
	for ( int i = 0; i<NumOfLane; i++)
       		 cvNamedWindow(g_wndname[i], CV_WINDOW_AUTOSIZE);

	for( ;; )
	{
	    /* Now wait for a trigger. */
	    for( int i = 0; i<NumOfLane; i++)
             	cvShowImage(g_wndname[i],g_pDisplayImage[i]);
            recv_type= clt_ipc_receive(pclt, &trig_info, sizeof(trig_info));
	    printf("Trigger received\n");
	    
       	    if (recv_type == DB_TIMER)
            {
		printf("received timer alarm\n");

            }
	    else if(DB_TRIG_VAR(&trig_info) == var2)  // receve a loop package
	    {
		if (db_clt_read( pclt, DB_LOOP_TYPE, sizeof(loop_typ), &loop_typ) == FALSE)
			printf("clt_read(DB_LOOP_TYPE) fails\n");
		else
		{
			printf("receive loop info\n");
			int LaneNum = loop_typ.laneNum;
               	        Mylist[LaneNum].InsertNode(loop_typ);
                        printf("the length of Mylist[%d] is : %d\n", LaneNum,Mylist[LaneNum].GetLength());
                       // printf("the received loop value is %lf\n",loop_typ.Inductance[0]);
                       // Mylist.PrintData(Mylist.GetLength()-1);
                }

	    }
	    else if(DB_TRIG_VAR(&trig_info) == var)    //receive a vloop package 
            {
		/* Read DB_VIRTUAL_LOOP_TYPE and print on screen */
/*
		if( db_clt_read( pclt, DB_VIRTUAL_LOOP_TYPE, sizeof(vloop_typ), &vloop_typ) == FALSE) 
		{
			printf("clt_read( DB_VIRTUAL_LOOP_TYPE).\n" );
		}
		else
		{
                        printf("receive virtual loop info\n");

                        int index = Mylist.LocateElem(vloop_typ.ts);
	
                       // printf("the length of Mylist is : %d\n", Mylist.GetLength());
               	       // Mylist.PrintData(index);
   	   	        if (!Mylist.GetNodeData(index,loop_typ))
				printf("can't get loop info at position %d\n", index);
			//printf("vloop timestamp: %lf, loop timestamp: %lf\n",vloop_typ.ts, loop_typ.ts);                                                 
                        //Display
                        UpdateDisplay(&vloop_typ, &loop_typ);
                        printf("loop1 inductance: %f, ref induc: %f\n",loop_typ.Inductance[0],loop0_ref-loop_typ.Inductance[0]);
                        printf("loop2 inductance: %f, ref induc: %f\n",loop_typ.Inductance[1],loop1_ref-loop_typ.Inductance[1]);
                        cvShowImage(g_wndname,g_pDisplayImage);

                        //Record virtual loop info
			fprintf(fp,"%lf", vloop_typ.ts);
			for (int ii=0; ii<MAX_VIRTUAL_LOOPS; ii++)
			{
				fprintf(fp," %d", vloop_typ.is_on[ii]);
                        }
                        //Record loop info
			fprintf(fp," %lf", loop_typ.ts);
			for (int ii=0; ii<MAX_LOOPS; ii++)
			{
				fprintf(fp," %f", loop_typ.Inductance[ii]);
                        }
			fprintf(fp, "\n" );
		}
*/
            }
	    else
		printf("Unknown trigger, recv_type %d\n", recv_type);
        }
	
	if (stay_alive) while (1);
	longjmp( exit_env, 1);
}


static db_clt_typ *database_init(char *pclient, char *phost, char *pserv,int xport)
{
	db_clt_typ *pclt;

	if( (pclt = clt_login( pclient, phost, pserv, xport)) == NULL )
	{
	    return( NULL );
	}
   //	else if (clt_trig_set( pclt,var ,var )      // this is video trig; Commented off on 09/05/08  this if for running loop only
//			 == FALSE ) 
//	{
//	    printf("trig setting error 1\n");
//	    clt_logout( pclt );
//	    return( NULL );
//	}
        else if (clt_trig_set( pclt,var2,var2)==FALSE ) // this is for loop;
	{
	    printf("trig setting error 2\n");
	    clt_logout( pclt );	
	    return (NULL);
	}
	else
	    return( pclt );
}


static void veh_done( db_clt_typ *pclt)
{

	if( pclt != NULL )
	    {
	    clt_trig_unset( pclt,var,var );
            clt_trig_unset( pclt,var2,var2);
	    clt_logout( pclt );
	    }

}


static void UpdateDisplay(VIRTUAL_LOOPS_TYP *vloop, LOOPS_TYP *loop)
{
/*
	//clean up image
	CvPoint pt1 = {0,0};
 	CvPoint pt2 = {g_pDisplayImage->width-1, g_pDisplayImage->height-1};

	cvRectangle(g_pDisplayImage,pt1,pt2,cvScalar(255,255,255),CV_FILLED);
	//int loop_on = vloop->is_on[0];
        // caption

        char caption_str[256];

        // display indicators
        CvPoint p_virtual_channel[4];
        CvPoint p_loop_channel[4];

	for (int i=0; i<MAX_VIRTUAL_LOOPS; i++)
	{
        	p_virtual_channel[i] = cvPoint(i*WND_WIDTH+WMARGIN,HMARGIN) ;
		p_loop_channel[i] = cvPoint(i*WND_WIDTH+WMARGIN, g_pDisplayImage->height-HMARGIN-HBAR_HEIGHT);
        	//CvPoint p_caption = { p_dvi_left.x, HMARGIN };
	}
       	
        g_Virtual_Loop_Bar0.Display(g_pDisplayImage, p_virtual_channel[0], vloop->is_on[0]);
        g_Virtual_Loop_Bar1.Display(g_pDisplayImage, p_virtual_channel[1], vloop->is_on[1]);
//        g_Virtual_Loop_Bar2.Display(g_pDisplayImage, p_virtual_channel[2], vloop->is_on[2]);
//        g_Virtual_Loop_Bar3.Display(g_pDisplayImage, p_virtual_channel[3], vloop->is_on[3]);

        g_Loop_Bar0.Display(g_pDisplayImage, p_loop_channel[0], loop0_ref-loop->Inductance[0]);
        g_Loop_Bar1.Display(g_pDisplayImage, p_loop_channel[1], loop1_ref-loop->Inductance[1]);
//        g_Loop_Bar2.Display(g_pDisplayImage, p_loop_channel[2], loop->Inductance[2]);
//        g_Loop_Bar3.Display(g_pDisplayImage, p_loop_channel[3], loop->Inductance[3]);
        
	// display 
        cvNamedWindow(g_wndname,CV_WINDOW_AUTOSIZE);
        cvShowImage(g_wndname, g_pDisplayImage);
*/ 
}
        




