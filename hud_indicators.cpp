//	Status Indicators
//
//		Simple HUD window to show the status of things
//
//	TODO change list
//		Possibly use a coloured light or preferably an icon for
//		the indicators rather than simple characters.
//
//		Author bluap/pjbroad Jan 2014
//

#include <vector>
#include <string>
#include <iostream>
#include <cstring>
#include <sstream>
#include <utility>

#include "asc.h"
#include "chat.h"
#include "context_menu.h"
#include "counters.h"
#include "errors.h"
#include "font.h"
#include "elconfig.h"
#include "elwindows.h"
#include "gamewin.h"
#include "gl_init.h"
#include "hud.h"
#ifdef JSON_FILES
#include "json_io.h"
#endif
#include "sound.h"
#include "spells.h"
#include "text.h"
#include "translate.h"

using namespace eternal_lands;

namespace Indicators
{
	//	Constants used by the window.
	//
	class Vars
	{
		public:
			static float zoom(void) { return scale; }
			static int space(void) { return (int)(0.5 + scale * 5); }
			static int border(void) { return (int)(0.5 + scale * 2); }
			static float font_x(void)
			{
				return FontManager::get_instance()
					.max_width_spacing(FontManager::Category::UI_FONT);
			}
			static float font_y(void)
			{
				return FontManager::get_instance().line_height(FontManager::Category::UI_FONT);
			}
			static int y_len(void) { return static_cast<int>(border() + zoom() * font_y() + 0.5); }
			static void set_scale(float new_scale) { scale = new_scale; }
		private:
			static float scale;
	};

	float Vars::scale = 1.0;

	//	A class to hold the state for an individual, basic indicator.
	//	Has a simple on / of state and no action.
	//
	class Basic_Indicator
	{
		public:
			Basic_Indicator(const char *the_strings, int (*ctrl)(void), int (*unavailable)(void), int no);
			virtual void do_draw(int x_pos);
			virtual void do_action(void) const { do_alert1_sound(); }
			virtual void get_tooltip(std::string & tooltip) const;
			virtual const std::string & get_context_menu_str(void) const { return context_menu_str; }
			virtual int *get_active_var(void) { return &is_active; }
			virtual void set_active(bool new_active) { is_active = (new_active) ?1 :0; }
			virtual bool not_active(void) const { return (is_active==0); }
			virtual void set_over(void) { mouse_over = true; }
			virtual int get_width(void) const { return indicator_text.size() * static_cast<int>(Vars::font_x() * Vars::zoom()); }
			virtual ~Basic_Indicator(void) {}
		protected:
			std::string on_tooltip;
			std::string off_tooltip;
			std::string unavailable_tooltip;
			int (*cntr_func)(void);
			int (*unavailable_func)(void);
		private:
			std::string indicator_text;
			std::string context_menu_str;
			int is_active;
			bool mouse_over;
	};


	//	A class to hold the state for an individual, basic indicator with a parsed action.
	//	Has a simple on/off state and an action string that is passed to the input parser.
	//
	class Parse_Action_Indicator: public Basic_Indicator
	{
		public:
			Parse_Action_Indicator(const char *the_strings, int (*ctrl)(void), int (*unavailable)(void), int no, const char *the_action)
				: Basic_Indicator(the_strings, ctrl, unavailable, no), action(the_action) {}
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
			Value_Indicator(const char *the_strings, int (*ctrl)(void), int (*unavailable)(void), int no, void (*action)(void))
				: Basic_Indicator(the_strings, ctrl, unavailable, no), action_function(action) {}
			virtual void do_action(void) const;
			virtual void get_tooltip(std::string & tooltip) const;
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
				: indicators_win(-1), cm_menu_id(CM_INIT_VALUE),
					cm_relocatable(0), x_len(0), y_len(0), default_location(true),
					option_settings(0), position_settings(0), have_settings(false),
					background_on(0), border_on(0) {}
			void init(void);
			void destroy(void);
			void show(void) { if (indicators_win >= 0) show_window (indicators_win); }
			void hide(void) { if (indicators_win >= 0) hide_window (indicators_win); }
			void toggle(int show);
			void draw(void);
			void show_tooltip(window_info *win, int mx);
			void click(int mx, Uint32 flags);
			int cm_handler(window_info *win, int widget_id, int mx, int my, int option);
			void set_settings(unsigned int opts, unsigned int pos) { option_settings = opts; position_settings = pos; have_settings = true;}
			void get_settings(unsigned int *opts, unsigned int *pos) const;
#ifdef JSON_FILES
			void read_settings(const char *dict_name);
			void write_settings(const char *dict_name) const;
#endif
			void ui_scale_handler(window_info *win) { x_len = 0; y_len = 0; Vars::set_scale(win->current_scale); }
			int get_default_width(void);
		private:
			void set_win_flag(Uint32 flag, int state);
			void set_background(bool on) { background_on = on; set_win_flag(ELW_USE_BACKGROUND, background_on); }
			void set_border(bool on) { border_on = on; set_win_flag(ELW_USE_BORDER, border_on); }
			std::vector<Basic_Indicator *> indicators;
			int indicators_win;
			size_t cm_menu_id;
			int cm_relocatable;
			int x_len;
			int y_len;
			bool default_location;
			unsigned int option_settings;
			unsigned int position_settings;
			bool have_settings;
			int background_on;
			int border_on;
			std::vector<Basic_Indicator *>::iterator get_over(int mx);
			std::pair<int,int> get_default_location(void);
			void change_width(int new_x_len);
			enum {	CMHI_RELOC=ELW_CM_MENU_LEN+1, CMHI_BACKGROUND, CMHI_BORDER,
					CMHI_SPACE1, CMHI_RESET, CMHI_SPACE2, CMHI_INDBASE};
	};


	//	Construct the indicator deriving the strings from the "||" separated string passed.
	//
	Basic_Indicator::Basic_Indicator(const char *the_strings, int (*ctrl)(void), int (*unavailable)(void), int no)
		: on_tooltip("Unset"), off_tooltip("Unset"), unavailable_tooltip("Unset"), cntr_func(ctrl), unavailable_func(unavailable),
			indicator_text("*"), is_active(1), mouse_over(false)
	{
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
			if (fields.size() >= 4)
			{
				indicator_text = fields[0];
				on_tooltip = fields[1];
				off_tooltip = fields[2];
				context_menu_str = fields[3];
				if (fields.size() == 5)
					unavailable_tooltip = fields[4];
			}
		}
	}


	//	Simply draw the single character, highlighted if the status is true.
	//
	void Basic_Indicator::do_draw(int x_pos)
	{
		if (mouse_over)
			glColor3f(1.0f,1.0f,1.0f);
		else if (unavailable_func && unavailable_func())
			glColor3f(gui_dull_color[0]/2, gui_dull_color[1]/2, gui_dull_color[2]/2);
		else if (cntr_func && cntr_func())
			glColor3fv(gui_bright_color);
		else
			glColor3fv(gui_dull_color);
		draw_string_zoomed(x_pos, Vars::border(), (const unsigned char*)indicator_text.c_str(), 1, Vars::zoom());
		mouse_over = false;
	}

	void Basic_Indicator::get_tooltip(std::string & tooltip) const
	{
		if (unavailable_func && unavailable_func())
			tooltip = unavailable_tooltip;
		else
			tooltip = ((cntr_func && cntr_func()) ?on_tooltip : off_tooltip);
	}

	//	If an action string is defined, execute using the standard command line parser.
	//
	void Parse_Action_Indicator::do_action(void) const
	{
		if (action.empty())
		{
			Basic_Indicator::do_action();
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
			Basic_Indicator::do_action();
			return;
		}
		do_click_sound();
		action_function();
	}


	//	Show the tool-tip that explains the status and includes the current value if non-zero.
	//
	void Value_Indicator::get_tooltip(std::string & tooltip) const
	{
		if (unavailable_func && unavailable_func())
		{
			tooltip = unavailable_tooltip;
			return;
		}
		std::ostringstream ss("");
		int value = (cntr_func) ?cntr_func() :0;
		if (value > 0)
			ss << on_tooltip << " [" << value << "]";
		else
			ss << off_tooltip;
		tooltip = ss.str();
	}


	//	The indicators instance.
	static Indicators_Container container;


	//	Window callback functions.
	//
	static int display_indicators_handler(window_info *win) { container.draw(); return 1; }
	static int mouseover_indicators_handler(window_info *win, int mx, int my) { if (my>=0) container.show_tooltip(win, mx); return 0; }
	static int ui_scale_indicators_handler(window_info *win) { container.ui_scale_handler(win); return 1; }
	static int click_indicators_handler(window_info *win, int mx, int my, Uint32 flags) { if (my>=0) container.click(mx, flags); return 1; }
	static int cm_indicators_handler(window_info *win, int widget_id, int mx, int my, int option) { return container.cm_handler(win, widget_id, mx, my, option); }


	//	Initialise the indicators, create or re-initialise the window.
	//
	void Indicators_Container::init(void)
	{
		std::pair<int,int> loc = get_default_location();

		if (indicators.empty())
		{
			indicators.reserve(6);
			indicators.push_back(new Parse_Action_Indicator(day_indicator_str, today_is_special_day, 0, indicators.size(), "#day"));
			indicators.push_back(new Basic_Indicator(harvest_indicator_str, now_harvesting, 0, indicators.size()));
			indicators.push_back(new Basic_Indicator(poison_indicator_str, we_are_poisoned, 0, indicators.size()));
			indicators.push_back(new Value_Indicator(messages_indicator_str, get_seen_pm_count, 0, indicators.size(), clear_seen_pm_count));
			indicators.push_back(new Parse_Action_Indicator(ranginglock_indicator_str, ranging_lock_is_on, 0, indicators.size(), "#keypress #K_RANGINGLOCK"));
			indicators.push_back(new Parse_Action_Indicator(glowperk_indicator_str, glow_perk_is_active, glow_perk_is_unavailable, indicators.size(), "#glow"));
		}

		x_len = static_cast<int>(Vars::font_x() * indicators.size() * Vars::zoom() +
			2 * Vars::border() + 2 * Vars::space() * indicators.size() + 0.5);
		y_len = Vars::y_len();

		if (indicators_win < 0)
		{
			if (have_settings)
			{
				unsigned int flags = option_settings;
				default_location = !((flags >> 24) & 1);
				if (!default_location)
				{
					loc.first = static_cast<int>(position_settings & 0xFFFF);
					loc.second = static_cast<int>((position_settings >> 16) & 0xFFFF);
				}
				std::vector<Basic_Indicator *>::iterator i;
				for (i=indicators.begin(); i<indicators.end(); ++i)
				{
					(*i)->set_active(!static_cast<bool>(flags&1));
					flags >>= 1;
				}
			}
		}
		else if (!default_location)
		{
			loc.first = windows_list.window[indicators_win].cur_x;
			loc.second = windows_list.window[indicators_win].cur_y;
		}

		if ((loc.first > (window_width - x_len)) || (loc.second > (window_height - y_len)))
			loc = get_default_location();

		if (indicators_win < 0)
		{
			indicators_win = create_window("Indicators", -1, 0, loc.first, loc.second, x_len, y_len, ELW_USE_UISCALE|ELW_SHOW|ELW_ALPHA_BORDER|ELW_SWITCHABLE_OPAQUE);
			if (indicators_win < 0 || indicators_win >= windows_list.num_windows)
			{
				LOG_ERROR("%s: Failed to create indicators window\n", __FILE__ );
				return;
			}
			set_window_handler(indicators_win, ELW_HANDLER_DISPLAY, (int (*)())&display_indicators_handler);
			set_window_handler(indicators_win, ELW_HANDLER_MOUSEOVER, (int (*)())&mouseover_indicators_handler);
			set_window_handler(indicators_win, ELW_HANDLER_CLICK, (int (*)())&click_indicators_handler);
			set_window_handler(indicators_win, ELW_HANDLER_UI_SCALE, (int (*)())&ui_scale_indicators_handler);
			ui_scale_indicators_handler(&windows_list.window[indicators_win]);

			background_on = ((option_settings >> 25) & 1);
			border_on = ((option_settings >> 26) & 1);
			set_background(background_on);
			set_border(border_on);
		}
		else
			init_window(indicators_win, -1, 0, loc.first, loc.second, x_len, y_len);

		if (!cm_valid(cm_menu_id))
		{
			std::vector<Basic_Indicator *>::iterator i;
			int j;
			std::ostringstream cm_menu("");
			cm_menu << cm_indicators_str;
			for (i=indicators.begin(); i<indicators.end(); ++i)
				cm_menu << (*i)->get_context_menu_str() << std::endl;
			cm_menu_id = cm_create(cm_title_menu_str, NULL);
			cm_bool_line(cm_menu_id, 1, &windows_list.window[indicators_win].opaque, NULL);
			cm_bool_line(cm_menu_id, 2, &windows_on_top, "windows_on_top");
			cm_add(cm_menu_id, cm_menu.str().c_str(), cm_indicators_handler);
			cm_bool_line(cm_menu_id, CMHI_RELOC, &cm_relocatable, 0);
			cm_bool_line(cm_menu_id, CMHI_BACKGROUND, &background_on, 0);
			cm_bool_line(cm_menu_id, CMHI_BORDER, &border_on, 0);
			for (i=indicators.begin(), j=0; i<indicators.end(); ++i, j++)
				cm_bool_line(cm_menu_id, CMHI_INDBASE+j, (*i)->get_active_var(), 0);
			cm_add_window(cm_menu_id, indicators_win);
		}
	}


	//	Delete the indicators and destroy the window.
	//
	void Indicators_Container::destroy(void)
	{
		std::vector<Basic_Indicator *>::iterator i;
		for (i=indicators.begin(); i<indicators.end(); ++i)
			delete (*i);
		indicators.clear();
		destroy_window(indicators_win);
		indicators_win = -1;
		if (cm_valid(cm_menu_id))
			cm_destroy(cm_menu_id);
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
		int pos_x = Vars::border();
		bool have_active = false;
		std::vector<Basic_Indicator *>::iterator i = indicators.begin();
		for (;i<indicators.end(); ++i)
		{
			if ((*i)->not_active())
				continue;
			pos_x += Vars::space();
			(*i)->do_draw(pos_x);
			pos_x += (*i)->get_width() + Vars::space();
			have_active = true;
		}
		if (!have_active)
		{
			glColor3fv(gui_dull_color);
			draw_string_zoomed(pos_x, Vars::border(), (const unsigned char *)no_indicators_str, 1, Vars::zoom());
			pos_x += static_cast<int>(strlen(no_indicators_str) * Vars::zoom() * Vars::font_x() + 0.5);
		}
		change_width(pos_x + Vars::border());
	}


	//	If different, resize the window and move if in the default location.
	//
	void Indicators_Container::change_width(int new_x_len)
	{
		if (new_x_len != x_len)
		{
			x_len = new_x_len;
			y_len = Vars::y_len();
			resize_window (indicators_win, x_len, y_len);
			if (default_location)
			{
				std::pair<int,int> loc = get_default_location();
				move_window(indicators_win, -1, 0, loc.first, loc.second);
			}
		}
	}


	//	Return an iterator to the indicator under the mouse or .end()
	//
	std::vector<Basic_Indicator *>::iterator Indicators_Container::get_over(int mx)
	{
		std::vector<Basic_Indicator *>::iterator i = indicators.begin();
		int pos_x = Vars::border();
		for (; i<indicators.end(); ++i)
		{
			if ((*i)->not_active())
				continue;
			int width = (*i)->get_width() + 2 * Vars::space();
			if ((mx > pos_x) && (mx < (pos_x + width)))
			{
				(*i)->set_over();
				return i;
			}
			pos_x += width;
		}
		return indicators.end();
	}


	//	If the mouse is over an indicator, draw the tool-tip.
	//
	void Indicators_Container::show_tooltip(window_info *win, int mx)
	{
		std::vector<Basic_Indicator *>::iterator i = get_over(mx);
		if (win && (i < indicators.end()))
		{
			eternal_lands::FontManager &fmgr = eternal_lands::FontManager::get_instance();
			std::string tooltip("");
			(*i)->get_tooltip(tooltip);
			int width = fmgr.line_width(win->font_category, (const unsigned char*)tooltip.c_str(),
				tooltip.length(), win->current_scale_small);
			int x_offset = -(Vars::border() + width);
			if ((win->cur_x + x_offset) < 0)
				x_offset = win->len_x;
			show_help(tooltip.c_str(), x_offset, Vars::border(), win->current_scale);
		}
	}


	//	If click an indicator, execute the action.
	//
	void Indicators_Container::click(int mx, Uint32 flags)
	{
		if (flags&ELW_LEFT_MOUSE)
		{
			std::vector<Basic_Indicator *>::iterator i = get_over(mx);
			if (i < indicators.end())
				(*i)->do_action();
		}
	}


	//	Change a window property bit flag
	//
	void Indicators_Container::set_win_flag(Uint32 flag, int state)
	{
		if ((indicators_win > -1) && (indicators_win < windows_list.num_windows))
		{
			Uint32 *flags = &windows_list.window[indicators_win].flags;
			if (state)
				*flags |= flag;
			else
				*flags &= ~flag;
		}
	}


	//	The context menu callback function.
	//
	int Indicators_Container::cm_handler(window_info *win, int widget_id, int mx, int my, int option)
	{
		size_t index = static_cast<size_t>(option - CMHI_INDBASE);
		if (option < ELW_CM_MENU_LEN)
			return cm_title_handler(win, widget_id, mx, my, option);
		if (index < indicators.size())
			return 1;
		switch (option)
		{
			case CMHI_RELOC:
				if (win->flags & ELW_TITLE_BAR)
				{
					win->flags &= ~(ELW_TITLE_BAR|ELW_DRAGGABLE);
					cm_relocatable = 0;
				}
				else
				{
					win->flags |= ELW_TITLE_BAR|ELW_DRAGGABLE;
					cm_relocatable = 1;
					default_location = false;
					if (win->cur_y == 0)
						move_window(win->window_id, -1, 0, win->cur_x, win->title_height);
				}
				if (win->cur_y == win->title_height)
					move_window(win->window_id, -1, 0, win->cur_x, 0);
				else if (win->cur_y == 0)
					move_window(win->window_id, -1, 0, win->cur_x, win->title_height);
				break;
			case CMHI_BACKGROUND: set_background(background_on); break;
			case CMHI_BORDER: set_border(border_on); break;
			case CMHI_RESET:
				{
					std::pair<int,int> loc = get_default_location();
					move_window(indicators_win, -1, 0, loc.first, loc.second);
					win->flags &= ~(ELW_TITLE_BAR|ELW_DRAGGABLE);
					cm_relocatable = 0;
					default_location = true;
					set_background(false);
					set_border(false);
					break;
				}
			default:
				return 0;
		}
		return 1;
	}


	//	Get the width of the indicators window if enabled and in the default location, otherwise 0.
	//
	int Indicators_Container::get_default_width(void)
	{
		if (!get_show_window(indicators_win) || !default_location || indicators_win < 0)
			return 0;
		return windows_list.window[indicators_win].len_x;
	}


	//	Get the x,y location, nice and snug against the bottom and right border
	//
	std::pair<int,int> Indicators_Container::get_default_location(void)
	{
		std::pair<int,int> loc;
		loc.first = window_width - HUD_MARGIN_X - x_len;
		loc.second = window_height - y_len;
		return loc;
	}


	//	Called when saving client settings
	//
	void Indicators_Container::get_settings(unsigned int *opts, unsigned int *pos) const
	{
		unsigned int flags = 0;
		unsigned int shift = 0;
		unsigned int x = 0;
		unsigned int y = 0;

		if (indicators_win < 0)
		{
			*opts = option_settings;
			*pos = position_settings;
			return;
		}

		std::vector<Basic_Indicator *>::const_iterator i;
		for (i=indicators.begin(); i<indicators.end(); ++i, shift++)
			flags |= (((*i)->not_active()) ?1 :0) << shift;

		if (!default_location)
			flags |= 1 << 24;
		flags |= background_on << 25;
		flags |= border_on << 26;
		*opts = flags;

		x = static_cast<unsigned int>(windows_list.window[indicators_win].cur_x);
		y = static_cast<unsigned int>(windows_list.window[indicators_win].cur_y);
		*pos = x | (y<<16);
	}


#ifdef JSON_FILES
	//	Read the options from the client state file
	//
	void Indicators_Container::read_settings(const char *dict_name)
	{
		int pos_x = json_cstate_get_int(dict_name, "pos_x", 0);
		int pos_y = json_cstate_get_int(dict_name, "pos_y", 0);
		position_settings = (pos_x & 0xFFFF) | ((pos_y & 0xFFFF) << 16);

		option_settings = json_cstate_get_unsigned_int(dict_name, "disabled_flags", 0);
		option_settings |= json_cstate_get_bool(dict_name, "relocated", 0) << 24;
		option_settings |= json_cstate_get_bool(dict_name, "background_on", 0) << 25;
		option_settings |= json_cstate_get_bool(dict_name, "border_on", 0) << 26;
		have_settings = true;
	}


	//	Write the options from the client state file
	//
	void Indicators_Container::write_settings(const char *dict_name) const
	{
		if (indicators_win < 0)
		{
			json_cstate_set_int(dict_name, "pos_x", position_settings & 0xFFFF);
			json_cstate_set_int(dict_name, "pos_y", (position_settings >> 16 ) & 0xFFFF);
			json_cstate_set_unsigned_int(dict_name, "disabled_flags", option_settings & 0x00FFFFFF);
			json_cstate_set_bool(dict_name, "relocated", (option_settings >> 24) & 1);
			json_cstate_set_bool(dict_name, "background_on", (option_settings >> 25) & 1);
			json_cstate_set_bool(dict_name, "border_on", (option_settings >> 26) & 1);
			return;
		}

		unsigned int disabled_flags = 0, shift = 0;
		std::vector<Basic_Indicator *>::const_iterator  i;
		for (i=indicators.begin(); i<indicators.end(); ++i, shift++)
			disabled_flags |= (((*i)->not_active()) ?1 :0) << shift;
		json_cstate_set_unsigned_int(dict_name, "disabled_flags", disabled_flags);

		json_cstate_set_unsigned_int(dict_name, "pos_x", static_cast<unsigned int>(windows_list.window[indicators_win].cur_x));
		json_cstate_set_unsigned_int(dict_name, "pos_y", static_cast<unsigned int>(windows_list.window[indicators_win].cur_y));
		json_cstate_set_bool(dict_name, "relocated", (default_location) ?0: 1);
		json_cstate_set_bool(dict_name, "background_on", background_on);
		json_cstate_set_bool(dict_name, "border_on", border_on);
	}
#endif

} // end namespace


//	Variables and functions accessible from rest of client
//
extern "C"
{
	int show_hud_indicators = 1;
	void init_hud_indicators(void) { if (show_hud_indicators) Indicators::container.init(); }
	void destroy_hud_indicators(void) { Indicators::container.destroy(); }
	void show_hud_indicators_window(void) { if (show_hud_indicators) Indicators::container.show(); }
	void hide_hud_indicators_window(void) { Indicators::container.hide(); }
	void toggle_hud_indicators_window(int *show) { *show = !*show; Indicators::container.toggle(*show); }
	void set_settings_hud_indicators(unsigned int opts, unsigned int pos) { Indicators::container.set_settings(opts, pos); }
	void get_settings_hud_indicators(unsigned int *opts, unsigned int *pos) { Indicators::container.get_settings(opts, pos); }
#ifdef JSON_FILES
	void read_settings_hud_indicators(const char *dict_name) { Indicators::container.read_settings(dict_name); }
	void write_settings_hud_indicators(const char *dict_name) { Indicators::container.write_settings(dict_name); }
#endif
	int get_hud_indicators_default_width(void) { if (show_hud_indicators) return Indicators::container.get_default_width(); else return 0; }
}
