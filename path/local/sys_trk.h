/**\file	
 *
 * Copyright (c) 2006   Regents of the University of California
 *
 */

#define DEFAULT_TRACK_FILE			"track.dat"

typedef struct
{
	dl_head_typ *phead;
	dl_node_typ *pcurrent;		/*	Node with current function.		*/
	double time_0;				/*	Time offset for this function.	*/
	float prev_value;			/*	Previous value, for derivative.	*/
	double prev_time;			/*	This is an absolute time.		*/
} trk_profile_typ;

extern trk_profile_typ *trk_init( char *pname, float init_value );
extern void trk_done( trk_profile_typ *ptrack );
extern bool_typ trk_run( trk_profile_typ *ptrack, float *pvalue, 
			float *pderiv );

static dl_head_typ *trk_file( char *pfilename );

#define MANUAL_VELOCITY				0.0

#define SINUSOID_TYPE				1
#define COSINE_TYPE					2
#define LINEAR_TYPE					3
#define QUADRATIC_TYPE				4
#define RAMP_TYPE					5
#define MANUAL_TYPE					6
#define SMOOTH_SIN_TYPE				7
#define SMOOTH_RAMP_TYPE				8

struct sinusoid_str
{
	float offset;
	float amplitude;
	float period;
	float phase;
};

struct linear_str
{
	float a0, a1;
};

struct ramp_str
{
	float start;
	float target;
};

struct quadratic_str
{
	float a0, a1, a2;
};

struct smooth_str
{
	float start;
	float target;
	float period;
	float phase;
};

typedef struct
{
	unsigned type;
	float duration;
	union
	{
		struct sinusoid_str sinusoid;
		struct linear_str linear;
		struct quadratic_str quadratic;
		struct ramp_str ramp;
		struct smooth_str smooth;
	} param;
} profile_typ;

