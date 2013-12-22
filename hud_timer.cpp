/*
 *  The countdown / stopwatch timer code
 *
 * 	TODO
 * 		add #command start/stop/reset/mode/set
 * 		add to context menu, dynamic list of previous start values
 * 		move strings to translate module
 * 
 * 		Author bluap/pjbroad Aug 2013
 */

#include <stdlib.h>

#include "asc.h"
#include "context_menu.h"
#include "elconfig.h"
#include "font.h"
#include "hud.h"
#include "hud_timer.h"
#include "notepad.h"
#include "sound.h"

static struct
{
	int running;
	int current_value;
	int start_value;
	int mode_coundown;
	int mouse_over;
	const int max_value;
	const int height;
	size_t cm_id;
	int last_base_y_start;
	INPUT_POPUP *input;
} hud_timer = {0, 90, 90, 1, 0, 9*60+59, DEFAULT_FONT_Y_LEN, CM_INIT_VALUE, -1, NULL };

enum {	CMHT_MODE=0, CMHT_RUNSTATE, CMHT_SETTIME, CMHT_RESET, CMHT_SEP1, CMHT_HELP  };

static const char *hud_timer_cm_str = "Change Mode\nStart/Stop\nSet Time\nReset Time\n--\nShow Help";
static const char *hud_timer_popup_title = "Time (in seconds)";


/* return the height in pixels of the timer */
extern "C" int get_height_of_timer(void)
{
	return hud_timer.height;
}


/* note the mouse is over the timer */
extern "C" void set_mouse_over_timer(void)
{
	hud_timer.mouse_over = 1;
}


/* Called from the main thread 500 ms timer - implement the timer */
extern "C" void update_hud_timer(void)
{
	static int use_it = 0;
	if (hud_timer.running && use_it)
	{
		if (hud_timer.mode_coundown)
		{
			if ((--hud_timer.current_value) == 0)
				do_alert1_sound();
		}
		else
			hud_timer.current_value++;
		if (hud_timer.current_value > hud_timer.max_value)
			hud_timer.current_value = 0;
		else if (hud_timer.current_value < 0)
			hud_timer.current_value = 0;
	}
	use_it = !use_it;
}



/* set the start value for the countdown timer, stop running and reset */
static void set_timer_start(int new_start_value)
{
	if (!hud_timer.mode_coundown)
		return;
	hud_timer.running = 0;
	hud_timer.start_value = new_start_value;
	if (hud_timer.start_value < 0)
		hud_timer.start_value = 0;
	else if (hud_timer.start_value > hud_timer.max_value)
		hud_timer.start_value = hud_timer.max_value;
	hud_timer.current_value = hud_timer.start_value;
}


/* toggle the timer between countdown and stopwatch mode */
static void toggle_timer_mode(void)
{
	if (hud_timer.mode_coundown)
		hud_timer.mode_coundown = hud_timer.current_value = 0;
	else
	{
		hud_timer.mode_coundown = 1;
		hud_timer.current_value = hud_timer.start_value;
	}
	hud_timer.running = 0;
}


/* reset the timer to its initial value */
static void hud_timer_reset(void)
{
	hud_timer.current_value = (hud_timer.mode_coundown) ?hud_timer.start_value : 0;
}


/* toggle the timer running state */
static void hud_timer_toggle_running(void)
{
	hud_timer.running = !hud_timer.running;
}


/* callback for timer popup - new time setting */
static void set_timer_time(const char *text, void *data)
{
	if ((text != NULL) && (strlen(text)>0))
		set_timer_start(atoi(text));
}


/* change timer context menu options depending on state */
static void cm_timer_pre_show_handler(window_info *win, int widget_id, int mx, int my, window_info *cm_win)
{
	cm_grey_line(hud_timer.cm_id, CMHT_SETTIME, !hud_timer.mode_coundown);
}


/* implement the timer context menu options */
static int cm_timer_handler(window_info *win, int widget_id, int mx, int my, int option)
{
	switch (option)
	{
		case CMHT_MODE: toggle_timer_mode(); break;
		case CMHT_RUNSTATE: hud_timer_toggle_running(); break;
		case CMHT_SETTIME:
			{
				if (hud_timer.input == NULL)
					hud_timer.input = (INPUT_POPUP *)malloc(sizeof(INPUT_POPUP));
				else
					close_ipu(hud_timer.input);
				init_ipu(hud_timer.input, win->window_id, 220, -1, 4, 1, NULL, set_timer_time);
				hud_timer.input->x = -230;
				hud_timer.input->y = hud_timer.last_base_y_start;
				display_popup_win(hud_timer.input, hud_timer_popup_title);
			}
			break;
		case CMHT_RESET: hud_timer_reset(); break;
		case CMHT_HELP:
			{
				const char *desc = get_option_description("view_hud_timer", INI_FILE_VAR);
				if ((desc != NULL) && (*desc != '\0'))
					LOG_TO_CONSOLE(c_green1, desc);
			}
			break;
		default: return 0;
	}
	return 1;
}


/* create or destroy the timer context menu depending on if the timer is shown */
static void check_timer_cm_menu(window_info *win, int base_y_start)
{
	if (cm_valid(hud_timer.cm_id) && (!view_hud_timer || (hud_timer.last_base_y_start != base_y_start)))
	{
		cm_destroy(hud_timer.cm_id);
		hud_timer.cm_id = CM_INIT_VALUE;
	}
	if (view_hud_timer && !cm_valid(hud_timer.cm_id))
	{
		hud_timer.cm_id = cm_create(hud_timer_cm_str, cm_timer_handler);
		cm_add_region(hud_timer.cm_id, win->window_id, 0, base_y_start - hud_timer.height, win->len_x, hud_timer.height);
		cm_set_pre_show_handler(hud_timer.cm_id, cm_timer_pre_show_handler);
	}
	hud_timer.last_base_y_start = base_y_start;
}


/* if we have a popup window for the timer, destroy it, freeing the resources */
static void destroy_timer_popup(void)
{
	if (hud_timer.input != NULL)
	{
		close_ipu(hud_timer.input);
		free(hud_timer.input);
		hud_timer.input = NULL;
	}
}


/* display the current time for the hud timer, coloured by stopped or running */
extern "C" int display_timer(window_info *win, int base_y_start)
{
	char str[10];
	int x;
	check_timer_cm_menu(win, base_y_start);
	if ((hud_timer.input != NULL) && (!view_hud_timer || !get_show_window(hud_timer.input->popup_win)))
		destroy_timer_popup();
	if (!view_hud_timer)
		return 0;
	base_y_start -= hud_timer.height;
	safe_snprintf(str, sizeof(str), "%c%1d:%02d", ((hud_timer.mode_coundown) ?countdown_str[0] :stopwatch_str[0]), hud_timer.current_value/60, hud_timer.current_value%60);
	x= 3+(win->len_x - (get_string_width((unsigned char*)str)*11)/12)/2;
	if (hud_timer.running)
		draw_string_shadowed(x, 2 + base_y_start, (unsigned char*)str, 1,0.5f, 1.0f, 0.5f,0.0f,0.0f,0.0f);
	else
		draw_string_shadowed(x, 2 + base_y_start, (unsigned char*)str, 1,1.0f, 0.5f, 0.5f,0.0f,0.0f,0.0f);
	if (hud_timer.mouse_over)
	{
		char *use_str = ((hud_timer.mode_coundown) ?countdown_str:stopwatch_str);
		draw_string_small_shadowed(-(int)(SMALL_FONT_X_LEN*(strlen(use_str)+0.5)), base_y_start, (unsigned char*)use_str, 1,1.0f,1.0f,1.0f,0.0f,0.0f,0.0f);
		hud_timer.mouse_over = 0;
	}
	return hud_timer.height;
}


/* return true if the coords are over the hud timer */
extern "C" int mouse_is_over_timer(window_info *win, int mx, int my)
{
	if ((view_hud_timer) && ((my > (hud_timer.last_base_y_start - hud_timer.height)) && (my < hud_timer.last_base_y_start)))
		return 1;
	else
		return 0;
}


/*
 * Control the hud timer by various mouse clicks:
 * Shift+click changed mode Countdown / Stopwatch
 * For Countdown, mouse wheel up/down decrease/increase start time.
 * 		Defaul step 5, +ctrl 1, +alt 30
 * Left-click start/stop timer
 * Mouse wheel click - reset timer.
 */
extern "C" int mouse_click_timer(Uint32 flags)
{
	/* change countdown start */
	if (flags & (ELW_WHEEL_DOWN|ELW_WHEEL_UP))
	{
		int step = 5;
		if (!hud_timer.mode_coundown)
			return 1;
		if (flags & ELW_CTRL)
			step = 1;
		else if (flags & ELW_ALT)
			step = 30;
		if ((flags & ELW_WHEEL_UP)!=0)
			step *= -1;
		set_timer_start(hud_timer.start_value + step);
	}
	/* control mode */
	else if (flags & ELW_SHIFT)
	{
		toggle_timer_mode();
		do_window_close_sound();
	}
	else
	{
		/* reset */
		if (flags & ELW_MID_MOUSE)
			hud_timer_reset();
		/* start / stop */
		else if (flags & ELW_LEFT_MOUSE)
			hud_timer_toggle_running();
		do_click_sound();
	}
	return 1;
}


/* clean up timer resource usage */
extern "C" void destroy_timer(void)
{
	destroy_timer_popup();
	if (cm_valid(hud_timer.cm_id))
	{
		cm_destroy(hud_timer.cm_id);
		hud_timer.cm_id = CM_INIT_VALUE;
	}
}
