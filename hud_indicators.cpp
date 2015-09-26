//	Status Indicators
//
//		Simple HUD window to show the status of things
//
//	TODO change list
//		Possibly use a coloured light or preferably an icon for
//			the indicators rather than a simple character.
//		Make movable and save state, allow reset too.
//		Enable/disable individual indicators.
//		Use context menu?
//
//		Author bluap/pjbroad Jan 2014
//

#include <vector>
#include <string>
#include <cstring>
#include <sstream>

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


namespace Indicators
{
	//	Constants used by the window.
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

	//	A class to hold the state for an individual, basic indicator.
	//	Has a simple on / of state and no action.
	//
	class Basic_Indicator
	{
		public:
			Basic_Indicator(const char *the_strings, int (*ctrl)(void), int no);
			virtual void do_draw(void);
			virtual void do_action(void) const { do_alert1_sound(); }
			virtual bool is_over(int mx);
			virtual void show_tooltip(void) const;
			virtual ~Basic_Indicator(void) { }
		protected:
			std::string on_tooltip;
			std::string off_tooltip;
			int (*cntr_func)(void);
			int y_pos;
		private:
			std::string disp;
			int x_pos;
			bool mouse_over;
	};


	//	A class to hold the state for an individual, basic indicator with a parsed action.
	//	Has a simple on/off state and an action string that is passed to the input parser.
	//
	class Parse_Action_Indicator: public Basic_Indicator
	{
		public:
			Parse_Action_Indicator(const char *the_strings, int (*ctrl)(void), int no, const char *the_action)
				: Basic_Indicator(the_strings, ctrl, no), action(the_action) {}
			virtual void do_action(void) const;
			virtual ~Parse_Action_Indicator(void) { }
		private:
			std::string action;
	};


	//	A class to hold the state for an individual, basic indicator with value and a click action function.
	//	Has a value rather than on/off state displayed in the tool-tip, the value is typically cleared by the action.
	//
	class Value_Indicator : public Basic_Indicator
	{
		public:
			Value_Indicator(const char *the_strings, int (*ctrl)(void), int no, void (*action)(void))
				: Basic_Indicator(the_strings, ctrl, no), action_function(action) {}
			virtual void do_action(void) const;
			virtual void show_tooltip(void) const;
			virtual ~Value_Indicator(void) { }
		private:
			void (*action_function)(void);
	};


	//	Class for the collection of indicators, and the window state and action methods.
	//
	class Indicators_Container
	{
		public:
			Indicators_Container(void)
				: indicators_win(-1) {}
			void init(void);
			void destroy(void);
			void show(void) { if (indicators_win >= 0) show_window (indicators_win); }
			void hide(void) { if (indicators_win >= 0) hide_window (indicators_win); }
			void toggle(int show);
			void draw(void);
			void tooltip(int mx);
			void click(int mx, Uint32 flags);
			~Indicators_Container(void) { destroy(); }
		private:
			std::vector<Basic_Indicator *> indicators;
			int indicators_win;
	};


	//	Construct the indicator deriving the strings from the "||" separated string passed.
	//
	Basic_Indicator::Basic_Indicator(const char *the_strings, int (*ctrl)(void), int no)
		: on_tooltip("Unset"), off_tooltip("Unset"), cntr_func(ctrl), y_pos(Vars::border()), disp("*"), mouse_over(false)
	{
		x_pos = static_cast<int>(Vars::border() + no * (Vars::zoom() * Vars::font_x() + Vars::space()) + 0.5);

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


	//	Simply draw the single character, highlighted if the status is true.
	//
	void Basic_Indicator::do_draw(void)
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


	//	Show the tool-tip for the current status.
	//
	void Basic_Indicator::show_tooltip(void) const
	{
		const std::string tooltip = ((cntr_func && cntr_func()) ?on_tooltip : off_tooltip);
		show_help(tooltip.c_str(), -static_cast<int>(Vars::border() + SMALL_FONT_X_LEN * (1 + tooltip.size()) + 0.5), y_pos );
	}


	//	If mouse is over, set for this frame and return true.
	//
	bool Basic_Indicator::is_over(int mx)
	{
		mouse_over = ((mx > x_pos && (mx < x_pos + Vars::zoom() * Vars::font_x())));
		return mouse_over;
	}


	//	If an action string is defined, execute using the standard command line parser.
	//
	void Parse_Action_Indicator::do_action(void) const
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


	//	Execute the click action, typically this clears the value.
	//
	void Value_Indicator::do_action(void) const
	{
		if (!action_function)
		{
			do_alert1_sound();
			return;
		}
		do_click_sound();
		action_function();
	}


	//	Show the tool-tip that explains the status and includes the current value if non-zero.
	//
	void Value_Indicator::show_tooltip(void) const
	{
		std::ostringstream ss("");
		int value = (cntr_func) ?cntr_func() :0;
		if (value > 0)
			ss << on_tooltip << " [" << value << "]";
		else
			ss << off_tooltip;
		show_help(ss.str().c_str(), -static_cast<int>(Vars::border() + SMALL_FONT_X_LEN * (1 + ss.str().size()) + 0.5), y_pos );
	}


	//	The indicators instance.
	static Indicators_Container container;


	//	Window callback functions.
	//
	static int display_indicators_handler(window_info *win) { container.draw(); return 1; }
	static int mouseover_indicators_handler(window_info *win, int mx, int my) { container.tooltip(mx); return 0; }
	static int click_indicators_handler(window_info *win, int mx, int my, Uint32 flags) { container.click(mx, flags); return 1; }


	//	Initialise the indicators, create or re-initialise the window.
	//
	void Indicators_Container::init(void)
	{
		if (indicators.empty())
		{
			indicators.reserve(4);
			indicators.push_back(new Parse_Action_Indicator(day_indicator_str, today_is_special_day, indicators.size(), "#day"));
			indicators.push_back(new Basic_Indicator(harvest_indicator_str, now_harvesting, indicators.size()));
			indicators.push_back(new Basic_Indicator(poison_indicator_str, we_are_poisoned, indicators.size()));
			indicators.push_back(new Value_Indicator(messages_indicator_str, get_seen_pm_count, indicators.size(), clear_seen_pm_count));
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


	//	Delete the indicators and destroy the window.
	//
	void Indicators_Container::destroy(void)
	{
		std::vector<Basic_Indicator *>::iterator  i;
		for (i=indicators.begin(); i<indicators.end(); ++i)
			delete (*i);
		indicators.clear();
		destroy_window(indicators_win);
		indicators_win = -1;
	}


	//	Called if the state changed in the configuration window.
	//
	void Indicators_Container::toggle(int show)
	{
		if (show)
		{
			if (indicators_win < 0)
				init();
			else
				show_window(indicators_win);
		}
		else
			hide_window(indicators_win);
	}


	//	Draw all the indicators.
	//
	void Indicators_Container::draw(void)
	{
		std::vector<Basic_Indicator *>::iterator i;
		for (i=indicators.begin(); i<indicators.end(); ++i)
			(*i)->do_draw();
	}


	//	If the mouse is over an indicator, draw the tool-tip.
	//
	void Indicators_Container::tooltip(int mx)
	{
		std::vector<Basic_Indicator *>::iterator  i;
		for (i=indicators.begin(); i<indicators.end(); ++i)
			if ((*i)->is_over(mx))
				(*i)->show_tooltip();
	}


	//	If click an indicator, execute the action.
	//
	void Indicators_Container::click(int mx, Uint32 flags)
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
			std::vector<Basic_Indicator *>::iterator  i;
			for (i=indicators.begin(); i<indicators.end(); ++i)
				if ((*i)->is_over(mx))
					(*i)->do_action();
		}
	}

} // end namespace


//	Variables and functions accessible from rest of client
//
extern "C"
{
	int show_hud_indicators = 1;
	void init_hud_indicators(void) { if (show_hud_indicators) Indicators::container.init(); }
	void destroy_hud_indicators(void) { Indicators::container.destroy(); }
	void show_hud_indicators_window(void) { Indicators::container.show(); }
	void hide_hud_indicators_window(void) { Indicators::container.hide(); }
	void toggle_hud_indicators_window(int *show) { *show = !*show; Indicators::container.toggle(*show); }
}
