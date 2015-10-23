/**
 *	widget.h created June 5, 2003 by E. Vedar
 *
 *	For use with the Tilcon graphics libraries.
 *
 *	Structures used to hold information for widget display.	
 *
 *	Also includes useful color and other definitions for
 *	use with Tilcon graphics.
 */
#ifndef WIDGET_H
#define WIDGET_H

#include "tilcon/TRTAPI.h"

/* This first set of structure definitions are used in the code on the buses
 * that was implemented for demo2003
 */

struct chart_scroll_one_point {

	long num_pts;
	long num_yseries;
	long x_type;
	long x_size;
	double x_incs[1];
	double y0_values[1];
	double y1_values[1];

};

struct widget
{
	
	char*	ID;			/* Object ID */
	long	type;			/* Type of widget */
	long	current_value;		/* num read from db,color, text, etc */
	long	current_state;		/* Used for toggle buttons 0 = up, 1 = down */
					/* Also used for multi-state icons */
	long	previous_value;
	double	double_value;		/* Meters need a double value! */
	char	previous_string_value[512];
	char	string_value[512];
	void	(*data_setup)(struct widget*);/* Uses data from datahub 
						to adjust widget display*/
	int	hidden;			/* Shows if object will be shown on the GUI */

	int	active;			/* Used for TRT_edit_text to update smoothly */

	int	dim;			/* dim = 1 dimmed */ 

	int	changed;		/* if 1, widget attribute changed in last update */

	double	alarm_low_to_value;
	double	alarm_mid_to_value;
	double	alarm_high_to_value;

	long	color;
	char*	previous_sound_file;
	char*	sound_file;

	// Used mainly for TRT_anim_obj types
	double	x_pos;
	double	y_pos;

	// Used mostly for TRT_chart types
	TRT_VarLen chart_points;
	struct chart_scroll_one_point chart_update;

	// Used for blinking
	unsigned int blink_enabled;

	// Used for label text
	long	label_look;
};

/* Used for array used in widget initialization */
struct widget_init {
	char* ID;
	int    type;
	long   value;
	void   (*function)(struct widget*);
	int    hidden;
};

// Basic Color Definitions
#define AMBER                   TRT_RGB(255,190,0)
#define BLACK                   TRT_RGB(0,0,0)
#define BLUE255                 TRT_RGB(0,0,255)
#define GREEN255				TRT_RGB(0,255,0)
#define GREEN128				TRT_RGB(0,128,0)
#define MAROON					TRT_RGB(128,0,0)
#define RED255                  TRT_RGB(255,0,0)
#define WHITE                   TRT_RGB(255,255,255)
#define SPEEDOMETER_ORANGE      TRT_RGB(255,65,0)
#define TRANSPARENT				TRT_Transparent

// Common Greyscale Definitions
#define GREY32                  TRT_RGB(32,32,32)
#define GREY69                  TRT_RGB(69,69,69)
#define GREY162                 TRT_RGB(162,162,162)
#define GREY192                 TRT_RGB(192,192,192)


// Legacy Application Color Definitions
#define LATERAL_BLUE            TRT_RGB(0,0,255)
#define ACC_BROWN               TRT_RGB(87,32,0)
#define GAPMETER_INDICATOR_FILL_OFFWHITE                TRT_RGB(128,0,0)
#define BACKGROUND_GREY         TRT_RGB(162,162,162)
#define BUTTON_GREY             TRT_RGB(192,192,192)
#define DARK_GREY               TRT_RGB(69,69,69)


/** The following set of structure definitions are used with the functions
 *  in tilcon_utils.cpp that have been written in fall 2004 for the CCW
 *  and IDS projects.
 */

#define TILCON_MAX_TEXT_SIZE	511
#define TILCON_MAX_FMT_SIZE	64

/** Tilcon type TRT_multi_state */
typedef struct {
	int state;
} tilcon_multistate_t;

/** Tilcon meter TRT_meter */
typedef struct {
	double val;
	double precision;
	int indicator_fill;
} tilcon_meter_t;

typedef struct {
	char str[TILCON_MAX_TEXT_SIZE+1];
	int int_val;
	double dbl_val;
	double precision;
	char fmt[TILCON_MAX_FMT_SIZE];
	int font_color;
} tilcon_text_t;

/** Tilcon button type TRT_anim_object */
typedef struct {
	double x;	
	double y;
	double precision;
	double heading;
} tilcon_anim_t;

/** Tilcon button type TRT_anim_area */
typedef struct {
	double xmin;	
	double ymin;
	double xmax;
	double ymax;
	double precision;
} tilcon_area_t;

/** Tilcon button type TRT_button */
typedef struct {
	int pressed;
	char label[TILCON_MAX_TEXT_SIZE];
	int in_color;
	int out_color;
} tilcon_button_t;

/** Tilcon drawing object TRT_drawing */
typedef struct {
	int fill_color;
	int line_color;
} tilcon_drawing_t;

/** Tilcon meter object TRT_meter alarm attributes */
typedef struct {
	unsigned int low_color;
	unsigned int mid_color;
	unsigned int high_color;
	double start_at;
	double low_to;
	double mid_to;
	double high_to;
	double precision;
} tilcon_meter_alarm_t;
	
typedef union {
	tilcon_multistate_t mstate;
	tilcon_meter_t meter;
	tilcon_text_t txt;
	tilcon_anim_t anim;
	tilcon_area_t area;
	tilcon_button_t button;
	tilcon_drawing_t drawing;
	tilcon_meter_alarm_t meter_alarm;
} tilcon_data_t;

typedef struct {
	char*	id;	/* Object ID */
	long	type;	/* Use Tilcon object type to choose part of union type */
	int	hidden;	/* Shows if object will be shown on the GUI */
	int	dim;	/* dim = 1 dimmed */ 
	tilcon_data_t data;
} tilcon_widget_t;



#endif
