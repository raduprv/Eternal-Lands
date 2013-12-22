//
//  The countdown / stopwatch time
//
// 	TODO
// 		add #command start/stop/reset/mode/set
// 		add to context menu, dynamic list of previous start values
// 
//		Author bluap/pjbroad Aug/Dec 2013
//

#include <cstdlib>
#include <cmath>
#include <cstring>

#include "asc.h"
#include "context_menu.h"
#include "elconfig.h"
#include "font.h"
#include "hud.h"
#include "hud_timer.h"
#include "notepad.h"
#include "sound.h"
#include "translate.h"


// A simple countdown / stopwatch timer class
// Digital display in main HUD
// Mouse and context menu control
//
class Hud_Timer
{
	public:
		Hud_Timer(void) : running(false), use_tick(false), current_value(90),
			start_value(90), mode_coundown(true), mouse_over(false),
			max_value(9*60+59), height(static_cast<int>(ceil(DEFAULT_FONT_Y_LEN))), cm_id(CM_INIT_VALUE),
			last_base_y_start(-1), input(0) {}
		~Hud_Timer(void) { destroy(); }
		int get_height(void) const { return height; }
		void set_mouse_over(void) { mouse_over = true; }
		void update(void);
		void set_start(int new_start_value);
		void pre_cm_handler(void) { cm_grey_line(cm_id, CMHT_SETTIME, !mode_coundown); }
		int cm_handler(window_info *win, int option);
		int display(window_info *win, int base_y_start);
		int mouse_is_over(window_info *win, int mx, int my);
		int mouse_click(Uint32 flags);
		void destroy(void);
	private:
		void toggle_mode(void);
		void toggle_running(void) { running = !running; }
		void reset(void) { current_value = (mode_coundown) ?start_value : 0; }
		void check_cm_menu(window_info *win, int base_y_start);
		void destroy_popup(void);
		void destroy_cm(void);
		bool running;
		bool use_tick;
		int current_value;
		int start_value;
		bool mode_coundown;
		bool mouse_over;
		const int max_value;
		const int height;
		size_t cm_id;
		int last_base_y_start;
		INPUT_POPUP *input;
		enum {	CMHT_MODE=0, CMHT_RUNSTATE, CMHT_SETTIME, CMHT_RESET, CMHT_SEP1, CMHT_HELP  };
};

static Hud_Timer my_timer;


// callback for timer popup - new time setting
//
static void set_timer_time(const char *text, void *data)
{
	if (text && (strlen(text)>0))
		my_timer.set_start(atoi(text));
}


// change timer context menu options depending on state
//
static void cm_timer_pre_show_handler(window_info *win, int widget_id, int mx, int my, window_info *cm_win)
{
	my_timer.pre_cm_handler();
}


// implement the timer context menu options
//
static int cm_timer_handler(window_info *win, int widget_id, int mx, int my, int option)
{
	return my_timer.cm_handler(win, option);
}


// Called from the main thread 500 ms timer - implement the timer
//
void Hud_Timer::update(void)
{
	if (running && use_tick)
	{
		if (mode_coundown)
		{
			if ((--current_value) == 0)
				do_alert1_sound();
		}
		else
			current_value++;
		if (current_value > max_value)
			current_value = 0;
		else if (current_value < 0)
			current_value = 0;
	}
	use_tick = !use_tick;
}


// set the start value for the countdown timer, stop running and reset
//
void Hud_Timer::set_start(int new_start_value)
{
	if (!mode_coundown)
		return;
	running = false;
	start_value = new_start_value;
	if (start_value < 0)
		start_value = 0;
	else if (start_value > max_value)
		start_value = max_value;
	current_value = start_value;
}


// toggle the timer between countdown and stopwatch mode
//
void Hud_Timer::toggle_mode(void)
{
	mode_coundown = !mode_coundown;
	reset();
	running = false;
}


// handle context menu options
//
int Hud_Timer::cm_handler(window_info *win, int option)
{
	switch (option)
	{
		case CMHT_MODE: toggle_mode(); break;
		case CMHT_RUNSTATE: toggle_running(); break;
		case CMHT_SETTIME:
			{
				if (!input)
					input = new INPUT_POPUP;
				else
					close_ipu(input);
				init_ipu(input, win->window_id, 220, -1, 4, 1, 0, set_timer_time);
				input->x = -230;
				input->y = last_base_y_start;
				display_popup_win(input, hud_timer_popup_title_str);
			}
			break;
		case CMHT_RESET: reset(); break;
		case CMHT_HELP:
			{
				const char *desc = get_option_description("view_hud_timer", INI_FILE_VAR);
				if (desc && (strlen(desc) > 0))
					LOG_TO_CONSOLE(c_green1, desc);
			}
			break;
		default: return 0;
	}
	return 1;
}


// create or destroy the timer context menu depending on if the timer is shown
//
void Hud_Timer::check_cm_menu(window_info *win, int base_y_start)
{
	if (cm_valid(cm_id) && (!view_hud_timer || (last_base_y_start != base_y_start)))
		destroy_cm();
	if (view_hud_timer && !cm_valid(cm_id))
	{
		cm_id = cm_create(hud_timer_cm_str, cm_timer_handler);
		cm_add_region(cm_id, win->window_id, 0, base_y_start - height, win->len_x, height);
		cm_set_pre_show_handler(cm_id, cm_timer_pre_show_handler);
	}
	last_base_y_start = base_y_start;
}



// display the current time for the hud timer, coloured by stopped or running
//
int Hud_Timer::display(window_info *win, int base_y_start)
{
	char str[10];
	int x;
	check_cm_menu(win, base_y_start);
	if (input && (!view_hud_timer || !get_show_window(input->popup_win)))
		destroy_popup();
	if (!view_hud_timer)
		return 0;
	base_y_start -= height;
	safe_snprintf(str, sizeof(str), "%c%1d:%02d", ((mode_coundown) ?countdown_str[0] :stopwatch_str[0]), current_value/60, current_value%60);
	x= 3+(win->len_x - (get_string_width((unsigned char*)str)*11)/12)/2;
	if (running)
		draw_string_shadowed(x, 2 + base_y_start, (unsigned char*)str, 1,0.5f, 1.0f, 0.5f,0.0f,0.0f,0.0f);
	else
		draw_string_shadowed(x, 2 + base_y_start, (unsigned char*)str, 1,1.0f, 0.5f, 0.5f,0.0f,0.0f,0.0f);
	if (mouse_over)
	{
		char *use_str = ((mode_coundown) ?countdown_str:stopwatch_str);
		draw_string_small_shadowed(-(int)(SMALL_FONT_X_LEN*(strlen(use_str)+0.5)), base_y_start, (unsigned char*)use_str, 1,1.0f,1.0f,1.0f,0.0f,0.0f,0.0f);
		mouse_over = false;
	}
	return height;
}


// return true if the coords are over the hud timer
//
int Hud_Timer::mouse_is_over(window_info *win, int mx, int my)
{
	if ((view_hud_timer) && ((my > (last_base_y_start - height)) && (my < last_base_y_start)))
		return 1;
	else
		return 0;
}



// Control the hud timer by various mouse clicks:
// Shift+click changed mode Countdown / Stopwatch
// For Countdown, mouse wheel up/down decrease/increase start time.
//		Defaul step 5, +ctrl 1, +alt 30
// Left-click start/stop timer
// Mouse wheel click - reset timer.
//
int Hud_Timer::mouse_click(Uint32 flags)
{
	// change countdown start
	if (flags & (ELW_WHEEL_DOWN|ELW_WHEEL_UP))
	{
		int step = 5;
		if (!mode_coundown)
			return 1;
		if (flags & ELW_CTRL)
			step = 1;
		else if (flags & ELW_ALT)
			step = 30;
		if ((flags & ELW_WHEEL_UP)!=0)
			step *= -1;
		set_start(start_value + step);
	}
	// control mode
	else if (flags & ELW_SHIFT)
	{
		toggle_mode();
		do_window_close_sound();
	}
	else
	{
		// reset
		if (flags & ELW_MID_MOUSE)
			reset();
		// start / stop
		else if (flags & ELW_LEFT_MOUSE)
			toggle_running();
		do_click_sound();
	}
	return 1;
}


// clean up timer resource usage
//
void Hud_Timer::destroy(void)
{
	destroy_popup();
	destroy_cm();
}


// if we have a context menu, destroy it
void Hud_Timer::destroy_cm(void)
{
	if (cm_valid(cm_id))
	{
		cm_destroy(cm_id);
		cm_id = CM_INIT_VALUE;
	}
}


// if we have a popup window for the timer, destroy it
//
void Hud_Timer::destroy_popup(void)
{
	if (input)
	{
		close_ipu(input);
		delete input;
		input = 0;
	}
}


extern "C"
{
	// external interface from hud
	int get_height_of_timer(void) { return my_timer.get_height(); }
	void set_mouse_over_timer(void) { my_timer.set_mouse_over(); }
	int display_timer(window_info *win, int base_y_start) { return my_timer.display(win, base_y_start); }
	int mouse_is_over_timer(window_info *win, int mx, int my) { return my_timer.mouse_is_over(win, mx, my); }
	int mouse_click_timer(Uint32 flags) { return my_timer.mouse_click(flags); }
	void destroy_timer(void) { return my_timer.destroy(); }

	// external interface from main thread timer
	void update_hud_timer(void) { my_timer.update(); }
}
