/**\file
 *
 *	tilcon_utils.h	Header file for convenience functions for Tilcon code
 *			Once tested and refined, make libary.
 */

#ifndef TILCON_UTILS_H
#define TILCON_UTILS_H
#include "widget.h"

/** Used to call "set_tilcon" function specific to a program from
 *  the generic tilcon event handling code.
 */
typedef struct {
	void (*set_values)(pid_t cid, void *data);
	void *data;
} tilcon_callback_t;

/** When buttons are used, set up the data field of the tilcon_callback_t
 *  structure to point to this button event structure.
 */
typedef struct {
	int button_pressed;
	char button_id[TILCON_MAX_TEXT_SIZE];
} tilcon_event_t;
	

extern int tilcon_start_kernel(char *appname, char *progname, int flags);
extern int tilcon_init_app(pid_t cid, int timer_enable, char *working_dir,
                        char *main_window_file, char *main_window_id,
                        int flags, int debug_level);
extern void tilcon_message_box(pid_t cid, char *title, char *msg);
extern int tilcon_process_main_window(TRT_ReceiveData *rec_data,
		 char *main_window_id, tilcon_callback_t *pcallback);
extern int tilcon_process_notify(pid_t cid, TRT_ReceiveData *rec_data, long *full,
                int *errorcode, tilcon_callback_t *pcallback,
                char *main_window_id);
extern int tilcon_exit(pid_t cid, char *main_window_id);

extern void tilcon_set_multistate(pid_t cid, tilcon_widget_t *pw, int new_state);
extern void tilcon_set_drawing(pid_t cid, tilcon_widget_t *pw, int fill_color,
		                        int line_color);
extern void tilcon_set_meter(pid_t cid, tilcon_widget_t *pw, double new_val, int new_indicator_fill);
extern void tilcon_set_meter_alarm(pid_t cid, tilcon_widget_t *pw,
	unsigned int low_color, unsigned int mid_color, unsigned int high_color,
	double start_at, double low_to, double mid_to, double high_to);
extern void tilcon_set_text(pid_t cid, tilcon_widget_t *pw, char *new_str);
extern void tilcon_set_font_color(pid_t cid, tilcon_widget_t *pw, 
	unsigned int new_color);
extern void tilcon_set_int_text(pid_t cid, tilcon_widget_t *pw, int new_val);
extern void tilcon_set_dbl_text(pid_t cid, tilcon_widget_t *pw, double new_val);
extern void tilcon_set_anim(pid_t cid, tilcon_widget_t *pw, double x, double y);
extern void tilcon_set_anim_area(pid_t cid, tilcon_widget_t *pw, 
		double xmin, double ymin, double xmax, double ymax);
extern void tilcon_set_button(pid_t cid, tilcon_widget_t *pw,
                  char *label, int in_color, int out_color);
extern void tilcon_init_multistate(pid_t cid, tilcon_widget_t *pw, char *id, 
		int init_val);
extern void tilcon_init_drawing(pid_t cid, tilcon_widget_t *pw, char *id,
		                 int fill_color, int line_color);
extern void tilcon_init_meter(pid_t cid, tilcon_widget_t *pw, char *id,
	 double init_val, double precision, int init_indicator_fill);
extern void tilcon_init_meter_alarm(pid_t cid, tilcon_widget_t *pw, char *id,
	unsigned int low_color, unsigned int mid_color, unsigned int high_color,
	double start_at, double low_to, double mid_to, double high_to,
	double precision);
extern void tilcon_init_dbl_text(pid_t cid, tilcon_widget_t *pw, char *id,
		 double init_val, char *fmt, double precision);
extern void tilcon_init_int_text(pid_t cid, tilcon_widget_t *pw, char *id,
		 int init_val, char *fmt);
extern void tilcon_init_text(pid_t cid, tilcon_widget_t *pw, char *id,
		 char *init_val);
extern void tilcon_init_anim_area(pid_t cid, tilcon_widget_t *pw, char *id,
		double xmin, double ymin, double xmax, double ymax,
		double precision);
extern void tilcon_init_anim(pid_t cid, tilcon_widget_t *pw, char *id,
		 double x, double y, double precision, double heading); 
extern void tilcon_init_panel(pid_t cid, tilcon_widget_t *pw, char *id);
extern void tilcon_init_button(pid_t cid, tilcon_widget_t *pw, char *id,
                  char *label, int in_color, int out_color);
extern void tilcon_play_sound(char* sound_file);
extern void tilcon_rotate_anim(pid_t cid, tilcon_widget_t *pw, double heading);
extern void tilcon_hide(pid_t cid, tilcon_widget_t *pw);
extern void tilcon_unhide(pid_t cid, tilcon_widget_t *pw);
extern void tilcon_dim(pid_t cid, tilcon_widget_t *pw);
extern void tilcon_undim(pid_t cid, tilcon_widget_t *pw);

#endif
