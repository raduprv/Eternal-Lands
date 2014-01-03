//	Status Indicators
//
//		Simple HUD window to show the status of multiple client states
//		Harvesting, Special Day and Poison status are initial provided.
//
//	TODO change list
//		Possibly use a coloured light or preferably an icon for
//			the indicators rather than a simple character.
//		Make movable and save state, allow reset too.  Use context menu?
// 
//		Author bluap/pjbroad Jan 2014
//

#include <vector>
#include <string>
#include <cstring>

#include "asc.h"
#include "chat.h"
#include "counters.h"
#include "font.h"
#include "elwindows.h"
#include "gamewin.h"
#include "gl_init.h"
#include "hud.h"
#include "sound.h"
#include "spells.h"
#include "text.h"
#include "translate.h"


static void init_indicators(void);

extern "C"
{
	int indicators_win = -1;
	int show_indicators = 1;
	void init_hud_indicators(void) { init_indicators(); }
}


//	Constants used by the window
//
class Vars
{
	public:
		static const float zoom(void) { return 1.0; }
		static const int space(void) { return 10; }
		static const int border(void) { return 2; }
		static const float font_x(void) { return DEFAULT_FONT_X_LEN; }
		static const float font_y(void) { return DEFAULT_FONT_Y_LEN; }
		static const int y_len(void) { return static_cast<int>(border() + zoom() * font_y() + 0.5); }
};


//	A class to hold the state for an individual indicator
//
class Indicator
{
	public:
		Indicator(const char *the_strings, int (*ctrl)(void), int no, const char *the_action);
		void do_draw(void);
		void do_action(void) const;
		bool is_over(int mx);
		void show_tooltip(void) const;
	private:
		std::string disp;
		std::string on_tooltip;
		std::string off_tooltip;
		std::string action;
		int (*cntr_func)(void);
		int x_pos;
		int y_pos;
		bool mouse_over;
};


//	Construct the indicator deriving the strings from the "||" separated string passed
//
Indicator::Indicator(const char *the_strings, int (*ctrl)(void), int no, const char *the_action)
	: disp("*"), on_tooltip("Unset"), off_tooltip("Unset"), cntr_func(ctrl), y_pos(Vars::border()), mouse_over(false)
{
	x_pos = static_cast<int>(Vars::border() + no * (Vars::zoom() * Vars::font_x() + Vars::space()) + 0.5);

	if (the_action)
		action = the_action;

	if (the_strings)
	{
		std::string line_text(the_strings);
		std::string::size_type from_index = 0;
		std::string::size_type to_index = 0;
		std::string delim = "||";
		std::string::size_type len = 0;
		std::vector<std::string> fields;
		while ((to_index = line_text.find(delim, from_index)) != std::string::npos)
		{
			if ((len = to_index-from_index) > 0)
				fields.push_back(line_text.substr(from_index, len));
			from_index = to_index + delim.size();
		}
		if ((len = line_text.size()-from_index) > 0)
			fields.push_back(line_text.substr(from_index, len));
		if (fields.size() == 3)
		{
			disp = fields[0].substr(0, 1);
			on_tooltip = fields[1];
			off_tooltip = fields[2];
		}
	}
}


//	Simply draw the single character, highlighted if the status is true
//
void Indicator::do_draw(void)
{
	if (mouse_over)
		glColor3f(1.0f,1.0f,1.0f);
	else if (cntr_func && cntr_func())
		glColor3f(0.99f,0.87f,0.65f);
	else
		glColor3f(0.40f,0.30f,0.20f);
	draw_string_zoomed(x_pos, y_pos, (const unsigned char*)disp.c_str(), 1, Vars::zoom());
	mouse_over = false;
}


//	If an action string is defined, execute it as a command line
//
void Indicator::do_action(void) const
{
	if (action.empty())
	{
		do_alert1_sound();
		return;
	}
	size_t command_len = action.size() + 1;
	do_click_sound();
	char temp[command_len];
	safe_strncpy(temp, action.c_str(), command_len);
	parse_input(temp, strlen(temp));
}


//	Show the tooltip that explains the status
//
void Indicator::show_tooltip(void) const
{
	const char *tooltip = ((cntr_func && cntr_func()) ?on_tooltip.c_str() : off_tooltip.c_str());
	show_help(tooltip, -static_cast<int>(Vars::border() + SMALL_FONT_X_LEN * (1 + strlen(tooltip)) + 0.5), y_pos );
}


//	If mouse is over, set for this frame and return true
//
bool Indicator::is_over(int mx)
{
	mouse_over = ((mx > x_pos && (mx < x_pos + Vars::zoom() * Vars::font_x())));
	return mouse_over;
}


//	The indicators
static std::vector<Indicator> indicators;


//	Window display callback, display the indicators
//
static int display_indicators_handler(window_info *win)
{
	std::vector<Indicator>::iterator  i;
	for (i=indicators.begin(); i<indicators.end(); ++i)
		i->do_draw();
	return 1;
}


//	Window mouse over callback, show tooltip if over an indicator
//
static int mouseover_indicators_handler(window_info *win, int mx, int my)
{
	std::vector<Indicator>::iterator  i;
	for (i=indicators.begin(); i<indicators.end(); ++i)
		if (i->is_over(mx))
			i->show_tooltip();
	return 0;
}


//	Window mouse click callback, execute action if over an indicator
//
static int click_indicators_handler(window_info *win, int mx, int my, Uint32 flags)
{
	// TODO move window code, finish/abandon.... 
	/*if (flags&ELW_CTRL)
	{
		if (win->flags&ELW_TITLE_BAR)
			win->flags &= ~(ELW_TITLE_BAR|ELW_DRAGGABLE);
		else
			win->flags |= ELW_TITLE_BAR|ELW_DRAGGABLE;
		return 1;
	}*/
	if (flags&ELW_LEFT_MOUSE)
	{
		std::vector<Indicator>::iterator  i;
		for (i=indicators.begin(); i<indicators.end(); ++i)
			if (i->is_over(mx))
				i->do_action();
	}
	return 1;
}


//	Initialise the indicators, create or re-init the window
//
static void init_indicators(void)
{
	if (indicators.empty())
	{
		indicators.reserve(3);
		indicators.push_back(Indicator(day_indicator_str, today_is_special_day, indicators.size(), "#day"));
		indicators.push_back(Indicator(harvest_indicator_str, now_harvesting, indicators.size(), 0));
		indicators.push_back(Indicator(poison_indicator_str, we_are_poisoned, indicators.size(), 0));
	}

	int x_len = static_cast<int>(Vars::font_x() * indicators.size() * Vars::zoom() +
		2 * Vars::border() + Vars::space() * (indicators.size()-1) + 0.5);
	int y_len = static_cast<int>(Vars::border() + Vars::zoom() * Vars::font_y() + 0.5);
	int x_pos = window_width - HUD_MARGIN_X - x_len;
	int y_pos = window_height - y_len;

	if(indicators_win < 0)
	{
		indicators_win = create_window("Indicators", -1, 0, x_pos, y_pos, x_len, y_len, ELW_SHOW);
		set_window_handler(indicators_win, ELW_HANDLER_DISPLAY, (int (*)())&display_indicators_handler);
		set_window_handler(indicators_win, ELW_HANDLER_MOUSEOVER, (int (*)())&mouseover_indicators_handler);
		set_window_handler(indicators_win, ELW_HANDLER_CLICK, (int (*)())&click_indicators_handler);
	}
	else
		init_window(indicators_win, -1, 0, x_pos, y_pos, x_len, y_len);
}
