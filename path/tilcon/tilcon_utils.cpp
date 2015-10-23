/**\file
 *
 *      tilcon_utils.cpp  Header file for convenience functions for Tilcon code
 *                      Once tested and refined, make libary.
 */

#include <sys_os.h>
#include "tilcon/TRT_API.h"
#include "tilcon_utils.h"
#include "widget.h"

char LOCAL_PLATFORM = TRT_LNX_X11R6;	

/**
 * 	TRT_StartEx will (a) spawn the Tilcon Kernel if it isn't already 
 * 	running and (b) create an 'object space' in that kernel, which 
 * 	will hold the windows and objects that we load or create.
 * 	appname is the name of the "object space", progname is
 *	the name of the process, flags should always be 0 for now, returns
 *	the channel id to be used by all calls to the Tilcon API.
 */
int tilcon_start_kernel(char *appname, char *progname, int flags)
{
	static pid_t TRT_CID;
	TRT_StartData start_data;
	int errorcode = 0;
	int mask = 0;	// mask used for optional members, see docs 

	start_data.Os_Env = LOCAL_PLATFORM;
	start_data.AppName = appname;
	start_data.Userprog = progname;
	start_data.Flags = flags;	// see docs for more info

	errorcode = TRT_StartEx ( mask, &start_data);

	if (errorcode != 0) {
		printf("Tilcon kernel failed to start, error %d\n", errorcode);
		return (-1);
	}
	TRT_CID = start_data.TRT_CID;
	return TRT_CID;
}
	
static void null_callback(void *a, void * b)
{
	return;
}

void tilcon_message_box(pid_t cid, char *parent_window, char *title, char *msg)
{
	TRT_MessageBox(cid, parent_window, title,
	    msg, " ", "OK", " ",
	    TRT_MessageBox_FontHelvetica, 14,
	    TRT_MessageBox_Exclamation|TRT_MessageBox_Wait,
            0, null_callback, NULL);
}

/**
 *	Initialize Tilcon application structures and load window.
 *	Return -1 on any error, print message on error.
 *	Flags argument unused for now, later may want to enable/disable
 *	blinking, bubbles, etc.
 *	Debug level 3 is recommended for synchronous operation.
 */
int tilcon_init_app(pid_t cid, int timer_enable, char *working_dir,
			char *main_window_file, char *main_window_id,
			int flags, int debug_level)
{
	int errorcode = 0;
//printf("TILCON_INIT_APP: cid %ld working dir %s main_window_file %s main_window_id %s\n", (int) cid, working_dir, main_window_file, main_window_id);
	/** Turn off "alive" checking between kernel and app */
	if (errorcode = TRT_SysClockSetup(cid, SYS_TIMER_CLEANUP, FALSE, 0)){
		printf("TRT_SysClockSetup failed, error %d\n", errorcode);
		return -1;
	}
	/** Timer kick every 0.05 seconds; 0=off, 1=on */
	if (errorcode = TRT_TimerHintEnable(cid, timer_enable)){
		printf("TRT_TimerHintEnable failed, error %d\n", errorcode);
		return -1;
	}
        /** Enable blinking  (0.5 sec. intervals) */
	if (errorcode = TRT_BlinkOn(cid, TRUE, 10)) {
		printf("TRT_BlinkOn failed, error %d\n", errorcode);
		return -1;
	}
	/** Enable help bubbles (1 sec. delay) */
	if (errorcode = TRT_BubbleOn(cid, TRUE, 20)) { 
		printf("TRT_BubbleOn failed, error %d\n", errorcode);
		return -1;
	}
	/** Level 3 turns on minimal debugging/error reporting in the kernel 
	 * this also forces API to be synchronous.  Debug level 0 allows 
	 * commands to be queued within the kernel;  
	 */
	 if (errorcode = TRT_Debug(cid, debug_level)) {
		printf("TRT_Debug failed, error %d\n", errorcode);
		return -1;
	}
	/** Working directory is location of .twd file */
	if (errorcode = TRT_SetWorkingDir(cid, working_dir)) {
		printf("TRT_SetWorkingDir %s failed, error %d\n", 
			working_dir, errorcode);
		return -1;
	}
	
	/** main_window_file specifies .twd file (w/o extension */	
	if (errorcode = TRT_WindowLoad(cid, main_window_file)) {
		tilcon_message_box(cid, main_window_id, main_window_file,
				"Cannot load the Main Window!");
		printf("TRT_WindowLoad %s failed, error %d\n",
			 main_window_file, errorcode);
		return -1;
	}

	/** main_window_id set with user interface tool */
	if (errorcode = TRT_WindowDisplay(cid, main_window_id)) {
		tilcon_message_box(cid, main_window_id, main_window_file,
		"Cannot display the Main Window form! Wrong ID");
		return -1;
	}

	return 0;
}


/**
 * Code in this routine is entirely from the Tilcon example routines.
 * Most of this code is not relevant to non-interactive applications
 * of the kind being developed for embedded systems.
 *
 * For some of our programs we use the Tilcon built-in timer to 
 * to activate a callback to our own routine. For other routines we
 * place TRT_GetInput in non-blocking mode and use our own timer.
 *
 * Respond to the 'notification' of a Tilcon event.
 * Based on the contents of rec_data (ID, code, state, window, etc),
 * figure out what needs to be done.
 *
 * Set full to true/false when queue_full message occurs.
 * Set errorcode if an error occurs.
 * Return FALSE if main loop should exit and terminate.
 *
 * Some events occur independently of any window and are processed here.
 * Other events come from within an object within a window; it is
 * convenient to call different event handlers depending on the
 * window where the event came from.
 *
 * TRT_ReceiveData structure - This structure is filled with data 
 * received from the Tilcon Kernel describing the event that occurred.
 * See the file trtconst.h for a complete description of its components.
 *	
 * rec_data->code -  Identifies the type of object selected
 * rec_data->event -  type of event: focus gained, state changed, ...
 * rec_data->frame_window -  ID of the parent window
 * rec_data->frame_window_len -  length of frame_window, in bytes, with NULL
 * rec_data->ID	-  ID of the selected object
 * rec_data->state -  state/value of selected object
 *			(or window event subtype)
 */
int tilcon_process_notify(pid_t cid, TRT_ReceiveData *rec_data, long *full, 
		int *errorcode, tilcon_callback_t *pcallback,
		char *main_window_id)
{
	// check 'code' first.  For some events (e.g. timers),	//
	// the ID and frame_window members would be meaningless	//
	switch (rec_data->code) {
	case TRT_queue:		// queue full / no longer full //
		*full = rec_data->state;
		break;

	case TRT_timer_hint:		// Handle Timer kick //	
		pcallback->set_values(cid, pcallback->data);
		break;

	case TRT_window:	// Handle Window notifications //
		// a few 'window' events aren't associated with any one window //
		switch (rec_data->state) {
		case TRT_exit_all:
			return FALSE;
			break;

		case TRT_command_finished:
			break;

		default:
			tilcon_process_main_window(rec_data, main_window_id,
				pcallback);	
			break;
		}
	default:
		tilcon_process_main_window(rec_data, main_window_id,
				pcallback);	
		break;
	}

	return TRUE;
}

/**
 *	Process window notifications 
 *	Currently only one main window is actually handled.
 *	Only look for exit event.
 *
 *	Set errorcode if an error occurs.
 *
 *	Return FALSE if main loop should exit and terminate.
 */

int tilcon_process_main_window(TRT_ReceiveData *rec_data, 
		char *main_window_id, tilcon_callback_t *pcallback)
{
	/** If frame_window_len is <= 1, frame_window is 
	 *  a NULL or an empty string...  i.e. the event 
	 *  isn't associated with a particular window 
	 */
	if (rec_data->frame_window_len > 1) {
		// call the event handler for the appropriate window //
		if (strcmp(rec_data->frame_window, main_window_id) == 0) {
			switch (rec_data->code) {
			// Handle Window notifications 
			case TRT_window:
				/** if 'X' close button hit on this window,
				  * end program, because main window 
				  */
				if (rec_data->state == TRT_window_quit) 
					return FALSE;
				break;

			// Handle Button notifications 
			case TRT_button:
			// If button, must set up data field of callback	
				if (pcallback->data == NULL) {
					printf("Button press error\n");
				} else {
					tilcon_event_t *pdata = 
						(tilcon_event_t *)
						pcallback->data;
					strcpy(pdata->button_id,
							rec_data->ID);
					pdata->button_pressed = 1;
				}
					
				break;
			default:
				break;
			}
		}
	}
	return TRUE;
}


/** int tilcon_exit();
 * Close the connection with the kernel 
 */
int tilcon_exit(pid_t cid, char *main_window_id)
{
	long errorcode = 0;
printf("TILCON_EXIT: cid %ld main_window_id %s\n", cid, main_window_id);
	//Delete specified windows by providing the window ID //
	errorcode += TRT_WindowDelete (cid, main_window_id );
printf("TILCON_EXIT: Finished TRT_WindowDelete, next is TRT_Exit errorcode %ld\n", errorcode);
	// Close this channel to the Tilcon kernel //
	// The kernel shuts down automatically when no connections remain //
	errorcode += TRT_Exit (cid);
printf("TILCON_EXIT: Finished TRT_Exit. Returning error code %ld\n",errorcode);

	// return non-zero on error //
	return errorcode;
}

/** The following functions are used to handle setting Tilcon values
 *  only when the values have changed. Application programs can
 *  use these functions instead of directly setting TILCON values
 *  without worrying about whether the value has changed or not.
 */

/** Multistate icons have a different appearance depending
 *  value of integer state
 */

void tilcon_set_multistate(pid_t cid, tilcon_widget_t *pw, int new_state) 
{
	tilcon_multistate_t *pm = &pw->data.mstate;

	if (pw->type != TRT_multi_state) {
		printf("%s not multi_state\n", pw->id);
		return;	
	}

	if (pm->state == new_state) 
		return; // do nothing

	pm->state = new_state;
	TRT_SetValues(cid, pw->id, TRT_ATT_STATE, new_state,NULL);
}

/** Meter icons use a double value to change a needle or slider.
 *  needles never use the indicator fill and must set to TRT_Transparent
 *  this is a hack
 */ 
void tilcon_set_meter(pid_t cid, tilcon_widget_t *pw, double new_val, int new_indicator_fill)
{
	int changed = 0;
	tilcon_meter_t *pm = &pw->data.meter;
	double diff;

	if (pw->type != TRT_meter) {
		printf("%s not meter\n", pw->id);
		return;	
	}

	diff = fabs(pm->val - new_val);

	if (diff >= pm->precision) {	// change great enough
		pm->val = new_val;
		TRT_SetValues(cid, pw->id, TRT_ATT_VALUE, new_val,NULL);
	}
	if (new_indicator_fill == TRT_Transparent) //indicates a needle
		return;

	if (pm->indicator_fill != new_indicator_fill) {
		pm->indicator_fill = new_indicator_fill;
		TRT_SetValues(cid, pw->id, TRT_ATT_CURRENT_DRAW_COMPONENT, 
				TRT_CHANGE_INDICATOR, 
			TRT_ATT_FILL_COLOR, new_indicator_fill, NULL);
	}
}

/** The ID of button objects is returned by a button event.
 */ 
void tilcon_set_button(pid_t cid, tilcon_widget_t *pw, 
				char *label, int in_color, int out_color)
{
	int changed = 0;
	tilcon_button_t *pb = &pw->data.button;
	double diff;

	if (pw->type != TRT_button) {
		printf("%s not button\n", pw->id);
		return;	
	}
	if (strcmp(pb->label, label) != 0) { 
		strncpy(pb->label, label, TILCON_MAX_TEXT_SIZE);
		TRT_SetValues(cid, pw->id, TRT_ATT_TEXT, label,NULL);
	}

	if (pb->in_color != in_color) {
		pb->in_color = in_color;
		TRT_SetValues(cid, pw->id, TRT_ATT_BUTTON_IN_COLOR, 
			in_color, NULL);
	}
	if (pb->out_color != out_color) {
		pb->out_color = out_color;
		TRT_SetValues(cid, pw->id, TRT_ATT_BUTTON_IN_COLOR, 
			out_color, NULL);
	}
}

/**
 *  This text function only handles single-line TRT_message_text,
 *  not multiline TRT_edit_text.
 */  
void tilcon_set_text(pid_t cid, tilcon_widget_t *pw, char *new_str)
{
        tilcon_text_t *pt = &pw->data.txt;

        if (pw->type != TRT_message_text) {
                printf("%s not text\n", pw->id);
                return;
        }

	if (strcmp(pt->str, new_str) == 0) //strings match
                return;// do nothing

	strncpy(pt->str, new_str, TILCON_MAX_TEXT_SIZE);

        TRT_SetValues(cid, pw->id, TRT_ATT_TEXT, new_str,NULL);
}
/**
 *  This function can be used with any of our text fields, only
 *  affects font color.
 *
 */
void tilcon_set_font_color(pid_t cid, tilcon_widget_t *pw,
		 unsigned int new_color)
{
	tilcon_text_t *pt = &pw->data.txt;

        if (pw->type != TRT_message_text) {
                printf("%s not text\n", pw->id);
                return;
        }

	if (pt->font_color == new_color) // new color the same
		return;		//do nothing
	
	pt->font_color = new_color;

        TRT_SetValues(cid, pw->id, TRT_ATT_FONT_COLOR, new_color, NULL);
}

/**
 *  Convenience function for common case of integer text field 
 */  
void tilcon_set_int_text(pid_t cid, tilcon_widget_t *pw, int val)
{
        tilcon_text_t *pt = &pw->data.txt;
	char txt[32];

        if (pw->type != TRT_message_text) {
                printf("%s not text\n", pw->id);
                return;
        }

	if (pt->int_val == val)
                return;// do nothing

	pt->int_val = val;

	sprintf(txt, pt->fmt, val);

        TRT_SetValues(cid, pw->id, TRT_ATT_TEXT, txt, NULL);
}

/**
 *  Convenience function for common case of double text field 
 */  
void tilcon_set_dbl_text(pid_t cid, tilcon_widget_t *pw, double val)
{
        tilcon_text_t *pt = &pw->data.txt;
	double diff;
	char txt[32];

        if (pw->type != TRT_message_text) {
                printf("%s not text\n", pw->id);
                return;
        }
	diff = fabs(pt->dbl_val - val);

	if (diff < pt->precision)	// change not great enough
		return; // but do nothing in Tilcon

	pt->dbl_val = val;

	sprintf(txt, pt->fmt, val);

        TRT_SetValues(cid, pw->id, TRT_ATT_TEXT, txt, NULL);
}

#undef DO_TRACE_ANIM
/**
 *  Sets x and y coordinates of Tilcon animation object
 */
void tilcon_set_anim(pid_t cid, tilcon_widget_t *pw, double x, double y)
{
        tilcon_anim_t *pa = &pw->data.anim;
	double xdiff;
	double ydiff;

        if (pw->type != TRT_anim_object) {
                printf("%s not anim\n", pw->id);
                return;
        }
	xdiff = fabs(pa->x - x);
	ydiff = fabs(pa->y - y);
#ifdef DO_TRACE_ANIM
	printf("%s oldx %f newx %f oldy %f newy %f\n",
		pw->id, pa->x, x, pa->y, y);
#endif

	if (xdiff < pa->precision && ydiff < pa->precision)
                return;// do nothing
	
	pa->x = x;
	pa->y = y;
	

	TRT_SetValues(cid, pw->id,
		TRT_ATT_ANIMOBJ_POSITION_X, x,
		TRT_ATT_ANIMOBJ_POSITION_Y, y, NULL);
}

void tilcon_set_anim_area(pid_t cid, tilcon_widget_t *pw, double xmin, double ymin,
				double xmax, double ymax)
{
	tilcon_area_t *pa = &pw->data.area;
	if (fabs(xmin - pa->xmin) < pa->precision) 
		TRT_SetValues(cid, pw->id, TRT_ATT_ANIMAREA_MIN_X, xmin, NULL);
	if (fabs(ymin - pa->ymin) < pa->precision) 
		TRT_SetValues(cid, pw->id, TRT_ATT_ANIMAREA_MIN_Y, ymin, NULL);
	if (fabs(xmax - pa->xmax) < pa->precision) 
		TRT_SetValues(cid, pw->id, TRT_ATT_ANIMAREA_MAX_X, xmax, NULL);
	if (fabs(ymax - pa->ymax) < pa->precision) 
		TRT_SetValues(cid, pw->id, TRT_ATT_ANIMAREA_MAX_Y, ymax, NULL);
}

/** Meter icons use a double value to change a needle or slider.
 *  needles never use the indicator fill and must set to TRT_Transparent
 *  this is a hack
 */ 
void tilcon_set_drawing(pid_t cid, tilcon_widget_t *pw, int fill_color,
	       		int line_color)	
{
	int changed = 0;
	tilcon_drawing_t *pd = &pw->data.drawing;
	double diff;

	if (pw->type != TRT_drawing) {
		printf("%s not drawing\n", pw->id);
		return;	
	}

	if (pd->fill_color != fill_color) {
		pd->fill_color = fill_color;
		TRT_SetValues(cid, pw->id, TRT_ATT_CURRENT_DRAW_COMPONENT, 
				TRT_DRAWOBJECT,
			TRT_ATT_FILL_COLOR, fill_color, NULL);
	}
	if (pd->line_color != line_color) {
		pd->line_color = line_color;
		TRT_SetValues(cid, pw->id, TRT_ATT_CURRENT_DRAW_COMPONENT,
			TRT_DRAWOBJECT, TRT_ATT_LINE_COLOR, line_color, NULL);
	}
}

/** Note that either TRT_CHANGE_INDICATOR or TRT_CHANGE_METER_BODY can
 *  be used in a TRT_SetValues call with TRT_ATT_CURRENT_DRAW_COMPONENT
 *  to ensure that the endpoints and colors of meter alarm components
 *  are updated with the next redraw.
 */
void tilcon_set_double_att(pid_t cid, char *wid, unsigned int att_flag, 
		double *pold, double new_val, double precision)
{
	double diff;
	double old_val = *pold;
	diff = fabs(old_val - new_val);

	if (diff >= precision) {	// change great enough
		 *pold = new_val;
		TRT_SetValues(cid, wid, TRT_ATT_CURRENT_DRAW_COMPONENT,
			TRT_CHANGE_INDICATOR, att_flag, new_val, NULL);
	}
}

void tilcon_set_alarm_att(pid_t cid, char *wid, unsigned int att_flag, 
		unsigned int *pold, unsigned int new_val)
{
	double old_val = *pold;

	if (old_val != new_val) {	// changed
		 *pold = new_val;
		TRT_SetValues(cid, wid, TRT_ATT_CURRENT_DRAW_COMPONENT, 
				TRT_CHANGE_METER_BODY, 
				att_flag, new_val, NULL);
	}
}

/** Meter_alarm code is used for alarm icons that change color bands
 */ 
void tilcon_set_meter_alarm(pid_t cid, tilcon_widget_t *pw,
	unsigned int low_color, unsigned int mid_color, unsigned int high_color,
	double start_at, double low_to, double mid_to, double high_to)
{
	double precision = pw->data.meter_alarm.precision;
	tilcon_meter_alarm_t *pm = &pw->data.meter_alarm;

	if (pw->type != TRT_meter) {
		printf("%s not meter\n", pw->id);
		return;	
	}
	tilcon_set_double_att(cid, pw->id, TRT_ATT_ALARM_START_AT,
				&pm->start_at, start_at, precision);
	tilcon_set_double_att(cid, pw->id, TRT_ATT_ALARM_LOW_TO,
				&pm->low_to, low_to, precision);
	tilcon_set_double_att(cid, pw->id, TRT_ATT_ALARM_MID_TO,
				&pm->mid_to, mid_to, precision);
	tilcon_set_double_att(cid, pw->id, TRT_ATT_ALARM_HIGH_TO,
				&pm->high_to, high_to, precision);
	tilcon_set_alarm_att(cid, pw->id, TRT_ATT_ALARM_LOW_COLOR,
				&pm->low_color, low_color);
	tilcon_set_alarm_att(cid, pw->id, TRT_ATT_ALARM_MID_COLOR,
				&pm->mid_color, mid_color);
	tilcon_set_alarm_att(cid, pw->id, TRT_ATT_ALARM_HIGH_COLOR,
				&pm->high_color, high_color);
}

/** Function to hide widget, if not already hidden
 *  Set pw->hidden, if previously unhidden or not initialized 
 */
void tilcon_hide(pid_t cid, tilcon_widget_t *pw)
{
	if (pw->hidden == 1)
		return;

	TRT_Hide(cid, pw->id);
	pw->hidden = 1;
	return;
}

/** Function to unhide widget, if not already unhidden
 *  Set pw->hidden, if previously hidden or not initialized 
 */
void tilcon_unhide(pid_t cid, tilcon_widget_t *pw)
{
	if (pw->hidden == 0)
		return;

	TRT_Display(cid, pw->id);
	pw->hidden = 0;
}

/** Function to dim widget, if not already dimmed
 *  Set pw->dim, if previously undimmed or not initialized 
 */
void tilcon_dim(pid_t cid, tilcon_widget_t *pw)
{
	if (pw->dim == 1)
		return;

	TRT_Dim(cid, pw->id);
	pw->dim = 1;
	return;
}

/** Function to undim widget, if not already undimmed
 *  Set pw->dim, if previously dimmed or not initialized 
 */
void tilcon_undim(pid_t cid, tilcon_widget_t *pw)
{
	if (pw->dim == 0)
		return;

	TRT_Undim(cid, pw->id);
	pw->dim = 0;
}


/** Initialized multistate object
 */
void tilcon_init_multistate(pid_t cid, tilcon_widget_t *pw, char *id, 
		int init_val)
{
	pw->id = id;
	pw->hidden = -1;
	pw->dim = -1;
	pw->type = TRT_multi_state;

	// bitwise complement to ensure different from desired initialization
	pw->data.mstate.state = ~ (unsigned int) init_val;

	tilcon_set_multistate(cid, pw, init_val);
}

/** Initialize meter object
 */
void tilcon_init_meter(pid_t cid, tilcon_widget_t *pw, char *id,
		 double init_val, double precision, int init_indicator_fill)
{
	pw->id = id;
	pw->hidden = -1;
	pw->dim = -1;
	pw->type = TRT_meter;
	pw->data.meter.precision = precision;
	pw->data.meter.val = init_val - 2*precision;
	pw->data.meter.indicator_fill = ~init_indicator_fill;
	tilcon_set_meter(cid, pw, init_val, init_indicator_fill); 
}

/** Initialize button object
 */
void tilcon_init_button(pid_t cid, tilcon_widget_t *pw, char *id,
		  char *label, int in_color, int out_color)
{
	pw->id = id;
	pw->hidden = -1;
	pw->dim = -1;
	pw->type = TRT_button;
	pw->data.button.in_color = ~in_color;
	pw->data.button.out_color = ~out_color;
	tilcon_set_button(cid, pw, label, in_color, out_color); 
}

/** Initialize text object used to display double value
 */
void tilcon_init_dbl_text(pid_t cid, tilcon_widget_t *pw, char *id,
		 double init_val, char *fmt, double precision)
{
	pw->id = id;
	pw->hidden = -1;
	pw->dim = -1;
	pw->type = TRT_message_text;
	pw->data.txt.str[0] = '\0';
	strncpy(pw->data.txt.fmt, fmt, TILCON_MAX_FMT_SIZE);  
	pw->data.txt.precision = precision;
	pw->data.txt.dbl_val = init_val;
	/* don't set the string until really set */

}

/** Initialize text object used to display integer value
 */
void tilcon_init_int_text(pid_t cid, tilcon_widget_t *pw, char *id,
		 int init_val, char *fmt)
{
	pw->id = id;
	pw->hidden = -1;
	pw->dim = -1;
	pw->type = TRT_message_text;
	pw->data.txt.str[0] = '\0';
	strncpy(pw->data.txt.fmt, fmt, TILCON_MAX_FMT_SIZE);  
	pw->data.txt.int_val = init_val;
	/* don't set the string until really set */

}

/** Initialize text object 
 *  Color is left alone, but saved for future comparisons
 */
void tilcon_init_text(pid_t cid, tilcon_widget_t *pw, char *id,
		 char *init_val)
{
	pw->id = id;
	pw->hidden = -1;
	pw->dim = -1;
	pw->type = TRT_message_text;
	pw->data.txt.str[0] = '\0';
	/** set up color for use of tilcon_set_font_color */
	TRT_GetValues(cid, id, TRT_ATT_FONT_COLOR, &pw->data.txt.font_color);

	tilcon_set_text(cid, pw, init_val); 
}

/** Initialize animation area 
 */
void tilcon_init_anim_area(pid_t cid, tilcon_widget_t *pw, char *id,
		 double xmin, double ymin, double xmax, double ymax, 
		double precision)
{
	tilcon_area_t *pa = &pw->data.area;
	pw->id = id;
	pw->hidden = -1;
	pw->dim = -1;
	pw->type = TRT_anim_area;
	pa->xmin = -1;
	pa->ymin = -1;
	pa->xmax = -1;
	pa->ymax = -1;
	pa->precision = precision;
	tilcon_set_anim_area(cid, pw, xmin, ymin, xmax, ymax);
}

void tilcon_rotate_anim(pid_t cid, tilcon_widget_t *pw, double heading)

{
	tilcon_anim_t *pa = &pw->data.anim;
	double diff = fabs(pa->heading - heading);

	if (diff < pa->precision)	// change not great enough
		return; // but do nothing in Tilcon

	pa->heading = heading;

	TRT_AnimObjectRotation(cid,"Relative_Position" ,
		pw->id, pa->heading);
}

/** Initialize animation object
 */

void tilcon_init_anim(pid_t cid, tilcon_widget_t *pw, char *id,
		 double x, double y, double precision, double heading) 
{
	tilcon_anim_t *pa = &pw->data.anim;
	pw->id = id;
	pw->hidden = -1;
	pw->dim = -1;
	pw->type = TRT_anim_object;
	pa->x = -1;
	pa->y = -1;
	pa->precision = precision;
	pa->heading = 0.0;
	tilcon_set_anim(cid, pw, x, y); 
	tilcon_rotate_anim(cid, pw, heading);
}

/** Initialize drawing  object
 */
void tilcon_init_drawing(pid_t cid, tilcon_widget_t *pw, char *id,
		 int fill_color, int line_color)
{
	pw->id = id;
	pw->hidden = -1;
	pw->dim = -1;
	pw->type = TRT_drawing;
	pw->data.drawing.fill_color = -fill_color; 
	pw->data.drawing.line_color = -line_color;
	tilcon_set_drawing(cid, pw, fill_color, line_color); 
}

/** Initialize meter_alarm object
 */
void tilcon_init_meter_alarm(pid_t cid, tilcon_widget_t *pw, char *id,
	unsigned int low_color, unsigned int mid_color, unsigned int high_color,
	double start_at, double low_to, double mid_to, double high_to,
	double precision)
{
	pw->id = id;
	pw->hidden = -1;
	pw->dim = -1;
	pw->type = TRT_meter;
	pw->data.meter_alarm.low_color = -low_color; 
	pw->data.meter_alarm.mid_color = -mid_color;
	pw->data.meter_alarm.high_color = -high_color;
	pw->data.meter_alarm.start_at = start_at + 2 * precision;
	pw->data.meter_alarm.low_to = low_to + 2 * precision;
	pw->data.meter_alarm.mid_to = mid_to + 2 * precision;
	pw->data.meter_alarm.high_to = high_to + 2 * precision;
	pw->data.meter_alarm.precision = precision;
	tilcon_set_meter_alarm(cid, pw, low_color, mid_color, high_color,
		       start_at, low_to, mid_to, high_to);	
}

void tilcon_init_panel(pid_t cid, tilcon_widget_t *pw, char *id)
{
	pw->id = id;
	pw->dim = -1;
	pw->hidden = -1;
}

/** Function to play a .wav file
 * Until we learn about sound libraries on Linux, just run aplay.
 * This is not really a Tilcon function, but was put here for the convenience
 *  of user interface programs.
 *  May want to remove error checking to speed it up once things are working.
 */

void tilcon_play_sound(char* sound_file)
{
#if 0
	/// On QNX6 we used a library because command line was too slow
	/// see archive/local/play.c in the svn repository
	play_wav(sound_file);
#endif
	struct stat stat_info;
	char *sound_player = "/usr/bin/aplay"; //later make this a parameter?
	int retval = -1;
	char sound_command[80];
	if ((retval = stat(sound_player, &stat_info)) != 0) {
		printf("Cannot stat %s\n", sound_player);
		return;
	}
	if ((retval = stat(sound_file, &stat_info)) != 0) {
		printf("Cannot stat %s\n", sound_file);
		return;
	}
		
	sprintf(sound_command, "%s %s &", sound_player, sound_file);
	system(sound_command);
	return;
}
