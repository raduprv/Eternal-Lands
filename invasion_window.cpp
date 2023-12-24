/*
	Implement the invasion window.

	Author bluap/pjbroad August/December 2023

	Files are stored in the "invasion_lists" sub-directory of the users
	config. For example, $HOME/.elc/invasion_lists/ on Linux systems.
	Any file in that directory with a ".txt" extension will be loaded.

	Each file must contain the list name as the first line, which must 
	not start with a "#".  Of the remaining line, only those starting 
	with "#invasion" are interpreted as commands, anything else is 
	ignored.

	The invasion commands take these forms:
	#invasion <x> <y> <map> <monster> <number of mobs>
	#invasion_cap <x> <y> <map> <monster> <number of mobs> <cap>

	These commands should be formatted as valid to send to the server.

	The invasion window displays the list of files in the left pane,
	each line showing the list name from the first line of the file.
	The "reload" button is use to re-scan for and load files.

	The right pane shows the contents of the selected list file. Double
	clicking a line immediately sends that command to the server.
	Sending commands can be automated using the play/stop controls.

	To include as an icon, add this to main_icon_window.xml:
	<icon type="window" image_id="30" alt_image_id="31"
	  help_name="invasion" param_name="invasion"></icon>
	
	To Do:
		- Handle #command errors
		- Add/edit/detete commands
		- Add/delete files
		- Use map names
		- Use monster names
		- Monster count greater then 50
		- Replete command list option
*/

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#ifdef WINDOWS
#include "io.h"
#else
#include <glob.h>
#endif

#include "asc.h"
#include "chat.h"
#include "io/elpathwrapper.h"
#include "elwindows.h"
#include "errors.h"
#include "font.h"
#include "gamewin.h"
#ifdef JSON_FILES
#include "json_io.h"
#endif
#include "questlog.h" // for draw_highlight()
#include "text.h"
#include "sound.h"

namespace invasion_window
{
	//
	// Class for a single invasion command.
	//
	class Command
	{
		public:
			Command(unsigned int _x, unsigned int _y, unsigned int _map, std::string _name, unsigned int _count);
			Command(unsigned int _x, unsigned int _y, unsigned int _map, std::string _name, unsigned int _count, unsigned int _cap);
			Command(std::string &text);
			bool is_valid(void) const { return valid; };
			const std::string & get_text(void) const { return command_text; }
			void execute(void) const;
			static void log_parse_error(const std::string &text, const std::string &reason);
			static bool valid_x(unsigned int x) { return (x < 768); }
			static bool valid_y(unsigned int y) { return (y < 768); }
			static bool valid_map(unsigned int map) { return (map < 141); }
			static bool valid_name(std::string &name) { return !name.empty(); }
			static bool valid_count(unsigned int count) { return ((count > 0) && (count < 51)); }
			static bool valid_cap(unsigned int cap) { return ((cap > 0) && (cap < 180)); }
		private:
			void construct(void);
			std::string hash_command;
			unsigned int x;
			unsigned int y;
			unsigned int map;
			std::string name;
			unsigned int count;
			unsigned int cap;
			bool capped;
			bool valid;
			std::string command_text;
	};

	//	Log to console when lines do not parse correctly.
	//
	void Command::log_parse_error(const std::string &text, const std::string &reason)
	{
		std::string message = "Invasion list parsing failed: " + reason + " [" + text + "]";
		LOG_TO_CONSOLE(c_red1, message.c_str());
	}

	//	Commmon to all constructors, do some validity checking.
	//
	void Command::construct(void)
	{
		if (!valid_x(x))
			return;
		if (!valid_x(y))
			return;
		if (!valid_map(map))
			return;
		if (!valid_name(name))
			return;
		if (!valid_count(count))
			return;
		if (capped && !valid_cap(cap))
			return;
		command_text = hash_command + " " + std::to_string(x) + " " + std::to_string(y) + " " +
			std::to_string(map) + " " + name + " " + std::to_string(count);
		if (capped)
			command_text += " " + std::to_string(cap);
		valid = true;
	}

	// Create an invasion command from parameters.
	//
	Command::Command(unsigned int _x, unsigned int _y, unsigned int _map, std::string _name, unsigned int _count)
		: hash_command("#invasion"), x(_x), y(_y), map(_map), name(_name), count(_count), cap(0), capped(false), valid(false)
	{
		construct();
	}

	Command::Command(unsigned int _x, unsigned int _y, unsigned int _map, std::string _name, unsigned int _count, unsigned int _cap)
		: hash_command("#invasion_cap"), x(_x), y(_y), map(_map), name(_name), count(_count), cap(_cap), capped(true), valid(false)
	{
		construct();
	}

	// Create an invasion command from a text string.
	// #invasion <x> <y> <map> <monster> <number of mobs>
	//
	Command::Command(std::string &text)
		: x(0), y(0), map(0), name(""), count(0), cap(0), capped(false), valid(false)
	{
		std::istringstream ss(text);

		ss >> hash_command;
		if (hash_command == "#invasion")
			ss >> x >> y >> map >> name >> count;
		else if (hash_command == "#invasion_cap")
		{
			ss >> x >> y >> map >> name >> count >> cap;
			capped = true;
		}
		else
		{
			log_parse_error(text, "Not an #invasion(_cap) command");
			return;
		}

		if (ss.fail())
			log_parse_error(text, "Reading values");
		else
		{
			construct();
			if (!valid)
				log_parse_error(text, "Command validation");
		}
	}

	//	Run the command.
	//
	void Command::execute(void) const
	{
		if (is_valid())
		{
			size_t command_len = command_text.size() + 1;
			char temp[command_len];
			safe_strncpy(temp, command_text.c_str(), command_len);
			parse_input(temp, strlen(temp));
		}
	}


	//
	//	A class for a list of invasion commands.
	//
	class Command_List
	{
		public:
			Command_List(const char * file_name);
			~Command_List(void);
			void execute(size_t line) const { if (line < commands.size()) commands[line]->execute(); }
			const std::string & get_text(void) const { return list_name; }
			size_t size(void) const { return commands.size(); }
			const std::vector<Command *> & get(void) const { return commands; }
			bool is_valid(void) const { return valid; }
		private:
			std::vector<Command *> commands;
			std::string list_name;
			bool valid;
	};

	//	Load the command list form a file.
	//
	Command_List::Command_List(const char * file_name)
	 : valid(false)
	{
		std::ifstream in(file_name);
		if (!in)
		{
			LOG_ERROR("%s: Failed to open [%s]\n", __FILE__, file_name);
			return;
		}

		getline(in, list_name);
		if (list_name.empty() || (list_name.rfind("#", 0) == 0))
		{
			Command::log_parse_error(list_name, "Invalid list name");
			return;
		}

		int error_count = 0;
		std::string line;
		while (getline(in, line))
		{
			if (line.rfind("#invasion", 0) != 0)
				continue;
			Command *command = new Command(line);
			if (command->is_valid())
				commands.push_back(command);
			else
			{
				error_count++;
				delete command;
			}
		}
		in.close();

		valid = true;

		if (error_count)
			Command::log_parse_error(list_name, "Invalid lines: " + std::to_string(error_count));
	}

	//	Destruct the command list.
	//
	Command_List::~Command_List(void)
	{
		for (size_t i = 0; i < commands.size(); ++i)
			delete commands[i];
	}


	//
	//	A class to implement a scrolling list widget.
	//
	class List_Widget
	{
		public:
			List_Widget(void) :
				selected(0), under_mouse(-1), scroll_id(-1),
				start_x(0), width(0), start_y(0), height(0), line_hight(0), margin(0), win(0)
				{ }
			void init(window_info *_win, int widget_id);
			void update(int _start_x, int _width, int _start_y, int _height, int _line_hight, int scroll_offset_y, int _padding);
			bool in_list(int mx, int my) const;
			bool mouse_over(int mx, int my);
			void wheel_scroll(Uint32 flags);
			void set_bar_len(size_t num_items);
			size_t get_selected(void) const { return selected; }
			void set_selected(size_t _selected) { selected = _selected; make_selected_visable(); }
			int get_under_mouse(void) const { return under_mouse; }
			template <class TheType> void draw(TheType the_list, bool is_playing);
		private:
			void make_selected_visable(void);
			size_t selected;
			int under_mouse;
			int scroll_id;
			int start_x;
			int width;
			int start_y;
			int height;
			int line_hight;
			int margin;
			window_info *win;
	};

	//	Initialise the list.
	//
	void List_Widget::init(window_info *_win, int widget_id)
	{
		win = _win;
		scroll_id = vscrollbar_add_extended(win->window_id, widget_id, NULL, 0, 0, 0, 0, 0, 1.0, 0, 1, 0);
	}

	//	Update the sizes and position the scroll bar.
	//
	void List_Widget::update(int _start_x, int _width, int _start_y, int _height, int _line_hight, int scroll_offset_y, int _margin)
	{
		start_x = _start_x;
		width = _width;
		start_y = _start_y;
		height = _height;
		line_hight = _line_hight;
		margin = _margin;
		widget_resize(win->window_id, scroll_id, win->box_size, height - scroll_offset_y);
		widget_move(win->window_id, scroll_id, start_x + width + margin, start_y + scroll_offset_y);
	}

	//	Return true if the mouse is within the list.
	//
	bool List_Widget::in_list(int mx, int my) const
	{
		return ((mx > start_x) && (mx < (start_x + width)) &&
			(my > start_y) && (my < (start_y + height)));
	}

	//	Record the list entry if the mouse is over one.
	//
	bool List_Widget::mouse_over(int mx, int my)
	{
		under_mouse = -1;
		if (in_list(mx, my))
		{
			under_mouse = vscrollbar_get_pos(win->window_id, scroll_id) + (my - start_y) / line_hight;
			return true;
		}
		return false;
	}

	//	Scroll if using the mouse wheel over the list.
	//
	void List_Widget::wheel_scroll(Uint32 flags)
	{
		if (flags & ELW_WHEEL_UP)
			vscrollbar_scroll_up(win->window_id, scroll_id);
		else if(flags & ELW_WHEEL_DOWN)
			vscrollbar_scroll_down(win->window_id, scroll_id);
	}

	//	Set the scroll bar length based on number of list items.
	//
	void List_Widget::set_bar_len(size_t num_items)
	{
		if ((scroll_id > -1) && (line_hight > 0))
		{
			int len = num_items - height / line_hight;
			vscrollbar_set_bar_len(win->window_id, scroll_id, (len > 0) ?len : 0);
		}
	}

	// Display the list contents.
	//
	template <class TheType> void List_Widget::draw(TheType the_list, bool is_playing)
	{
		int pos_y = start_y;
		int top_entry = vscrollbar_get_pos(win->window_id, scroll_id);
		for (size_t i = top_entry; i < the_list.size(); ++i)
		{
			if (i == selected)
				draw_highlight(start_x, pos_y, width, line_hight, (is_playing) ?2: 1);
#ifndef ANDROID
			else if (i == get_under_mouse())
				draw_highlight(start_x, pos_y, width, line_hight, 0);
#endif
			glColor3f(1.0f, 1.0f, 1.0f);
			draw_string_zoomed_width_font(start_x, pos_y,
				reinterpret_cast<const unsigned char*>(the_list[i]->get_text().c_str()),
				width, 1, win->font_category, win->current_scale_small);
			pos_y += line_hight;
			if ((pos_y + line_hight) > (start_y + height))
				break;
		}
	}

	//	Make the selected line visible if not already.
	//
	void List_Widget::make_selected_visable(void)
	{
		if ((scroll_id < 0|| line_hight <= 0))
			return;
		int top_entry = vscrollbar_get_pos(win->window_id, scroll_id);
		int num_shown = height / line_hight;
		int new_pos;
		if (selected < top_entry)
			new_pos = selected;
		else if (selected >= (top_entry + num_shown))
			new_pos = selected - (num_shown - 1);
		else
			return;
		vscrollbar_set_pos(win->window_id, scroll_id, new_pos);
	}

	//
	//	A class to implement a labelled input widget.
	//
	class Labelled_Input_Widget
	{
		public:
			Labelled_Input_Widget(std::string _label, std::string _mouseover,
				unsigned int init_value, bool (*_validate_func)(unsigned int), size_t _buf_size, bool _have_checkbox) :
					label(_label), mouseover(_mouseover), validate_number(_validate_func), validate_string(0),
					last_input_number(init_value), is_string(false),
					label_id(-1), input_id(-1), checkbox_id(-1), window_id(-1),
					have_checkbox(_have_checkbox), checkbox_enabled(0),
					buf_size(_buf_size), last_input_buf(0), width(0), state(STATE_START) {}
			Labelled_Input_Widget(std::string _label, std::string _mouseover, std::string init_value,
				bool (*_validate_string)(std::string &), size_t _buf_size, bool _have_checkbox) :
					label(_label), mouseover(_mouseover), validate_number(0), validate_string(_validate_string),
					last_input_number(0), last_input_string(init_value),
					is_string(true), label_id(-1), input_id(-1),  checkbox_id(-1), window_id(-1),
					have_checkbox(_have_checkbox), checkbox_enabled(0),
					buf_size(_buf_size), last_input_buf(0), width(0), state(STATE_START) {}
			void destroy(int window_id, bool keep_buf = false);
			void create(window_info *win, int *new_widget_id, float space);
			void move(int window_id, int x, int y);
			void keypress(SDL_Keycode key_code);
			int get_input_id(void) const { return input_id; }
			float get_width(void) const { return width; }
			void set_init_value(unsigned int value) { last_input_number = value; }
			void set_init_value(std::string value) { last_input_string = value; }
			unsigned int get_number_value(void) const { return last_input_number; }
			bool get_checked(void) const { return checkbox_enabled == 1; }
			const std::string & get_string_value(void) const { return last_input_string; }
			std::string get_mouseover(void) const;
			bool is_valid(void) const { return (state == STATE_VALID); }
			void revalidate(void);
		private:
			std::string label;
			std::string mouseover;
			bool (*validate_number)(unsigned int);
			bool (*validate_string)(std::string &);
			unsigned int last_input_number;
			std::string last_input_string;
			bool is_string;
			int label_id;
			int input_id;
			int checkbox_id;
			int window_id;
			bool have_checkbox;
			int checkbox_enabled;
			size_t buf_size;
			unsigned char *last_input_buf;
			float width;
			enum WIDGET_STATE { STATE_START=0, STATE_VALID, STATE_INVALID, STATE_EDIT };
			void set_state(enum WIDGET_STATE new_state);
			enum WIDGET_STATE state;
	};

	// Destroy the label and input widgets, and free the buffer.
	//
	void Labelled_Input_Widget::destroy(int window_id, bool keep_buf)
	{
		if (label_id >= 0)
			widget_destroy(window_id, label_id);
		if (input_id >= 0)
			widget_destroy(window_id, input_id);
		if (checkbox_id >= 0)
			widget_destroy(window_id, checkbox_id);
		label_id = input_id = checkbox_id = window_id = -1;
		if (keep_buf)
			return;
		if (last_input_buf)
			delete [] last_input_buf;
		last_input_buf = 0;
	}

	// Pre-declare the keypress and mouseover functions
	static int common_input_keypress_handler(widget_list *widget, int mx, int my,
		SDL_Keycode key_code, Uint32 key_unicode, Uint16 key_mod);
	static int common_input_mouseover_handler(widget_list *widget, int mx, int my);

	// Destroy (if needed) and create the widgets.
	//
	void Labelled_Input_Widget::create(window_info *win, int *new_widget_id, float space)
	{
		destroy(win->window_id, true);

		window_id = win->window_id;

		label_id = label_add_extended(win->window_id, (*new_widget_id)++, NULL, 0, 0, 0, win->current_scale, label.c_str());
		widget_set_OnMouseover(win->window_id, label_id, (int (*)())&common_input_mouseover_handler);

		// the buffer if created once and populated by the initial value
		// this preserves any current edits
		if (!last_input_buf)
		{
			last_input_buf = new unsigned char[buf_size];
			std::string temp;
			if (is_string)
				temp = last_input_string;
			else
				temp = std::to_string(last_input_number);
			safe_strncpy2(reinterpret_cast<char *>(last_input_buf),
				temp.c_str(), sizeof(last_input_buf), temp.size());
		}

		float input_chars = (buf_size < 10) ?buf_size + 1 : 10;
		input_id = pword_field_add_extended(win->window_id, (*new_widget_id)++, NULL,
			0, 0, input_chars * get_max_digit_width_zoom(UI_FONT, win->current_scale),
			1.5 * win->default_font_len_y, P_TEXT, win->current_scale, last_input_buf, buf_size);
		widget_set_OnKey(win->window_id, input_id, (int (*)())common_input_keypress_handler);
		widget_set_OnMouseover(win->window_id, input_id, (int (*)())&common_input_mouseover_handler);

		width = widget_get_width(win->window_id, label_id) + space + widget_get_width(win->window_id, input_id);

		if (have_checkbox)
		{
			checkbox_id = checkbox_add_extended(win->window_id, (*new_widget_id)++, NULL,
				0, 0, win->box_size, win->box_size, 0, win->current_scale, &checkbox_enabled);
			width += widget_get_width(win->window_id, checkbox_id) + 4 * space;
		}

		// preserve any colour setting
		set_state(state);
	}

	// Move the label and input wigets as one, presurving the seperation space.
	//
	void Labelled_Input_Widget::move(int window_id, int x, int y)
	{
		float y_label_offset = (widget_get_height(window_id, input_id) - widget_get_height(window_id, label_id)) / 2;
		float checkbox_width = 0;
		float input_width = widget_get_width(window_id, input_id);
		float label_width = widget_get_width(window_id, label_id);
		float space = 0;
		if (have_checkbox)
		{
			checkbox_width = widget_get_width(window_id, checkbox_id);
			widget_move(window_id, checkbox_id, x, y + y_label_offset);
			space = (width - input_width - label_width - checkbox_width) / 5;
			x += checkbox_width + space * 4;
		}
		else
			space = width - input_width - label_width;
		widget_move(window_id, label_id, x, y + y_label_offset);
		widget_move(window_id, input_id, x + label_width + space, y);
	}

	// Callback from the common keypress handler.
	// When Return/Enter pressed, validate and update the value.
	//
	void Labelled_Input_Widget::keypress(SDL_Keycode key_code)
	{
		if ((key_code == SDLK_RETURN || key_code == SDLK_KP_ENTER))
		{
			std::string current = reinterpret_cast<char *>(last_input_buf);
			if (current.size())
			{
				if (is_string)
				{
					if (validate_string(current))
					{
						last_input_string = current;
						do_drag_item_sound();
						set_state(STATE_VALID);
						return;
					}
				}
				else
				{
					std::istringstream ss(current);
					unsigned int new_value = 0;
					ss >> new_value;
					if (!ss.fail() && validate_number(new_value))
					{
						last_input_number = new_value;
						do_drag_item_sound();
						set_state(STATE_VALID);
						return;
					}
				}
			}
			do_alert1_sound();
			set_state(STATE_INVALID);
		}
		else
			set_state(STATE_EDIT);
	}

	// Revalidate widget state.
	//
	void Labelled_Input_Widget::revalidate(void)
	{
		if (have_checkbox && !checkbox_enabled)
		{
			set_state(STATE_START);
			return;
		}
		if ((state == STATE_EDIT) || (state == STATE_INVALID))
			return;
		if ((is_string) ?validate_string(last_input_string) :validate_number(last_input_number))
			set_state(STATE_VALID);
		else
			set_state(STATE_INVALID);
	}

	// Set the colour of the wiget input field.
	//
	void Labelled_Input_Widget::set_state(enum WIDGET_STATE new_state)
	{
		state = new_state;
		if ((state == STATE_START) || (state == STATE_VALID))
			widget_set_color(window_id, input_id, gui_color[0], gui_color[1], gui_color[2]);
		else if (state == STATE_INVALID)
			widget_set_color(window_id, input_id, 255.0, 0.0, 0.0);
		else if (state == STATE_EDIT)
			widget_set_color(window_id, input_id, 255.0, 255.0, 0.0);
	}

	// Display a state aware mouseover message.
	//
	std::string Labelled_Input_Widget::get_mouseover(void) const
	{
		return mouseover + std::string(": ") +
			((state == STATE_VALID)
				? ((is_string) ?last_input_string :std::to_string(last_input_number))
				: std::string(": not validated"));
	}


	//
	//	Main class for invasion UI.
	//
	class Container
	{
		public:
			Container(void) :
				reload_button_id(-1), play_button_id(-1), stop_button_id(-1),
				launch_button_id(-1), add_button_id(-1), replace_button_id(-1),
				is_playing(false), last_play_execute(0),
				delay_input_widget(std::string("Delay:"), "Enter delay in seconds", def_delay, &Container::valid_delay, 4, false),
				x_coord_widget(std::string("X:"), "Enter X coord", 0, &Command::valid_x, 4, false),
				y_coord_widget(std::string("Y:"), "Enter Y coord", 0, &Command::valid_y, 4, false),
				map_widget(std::string("Map:"), "Enter map id", 0, &Command::valid_map, 4, false),
				monster_widget(std::string("Monster:"), "Enter monsters name", "", &Command::valid_name, 21, false),
				count_widget(std::string("Count:"), "Enter number of monsters", 0, &Command::valid_count, 4, false),
				cap_widget(std::string("Cap:"), "Enter player cap", 0, &Command::valid_cap, 4, true),
				input_widgets({ &delay_input_widget, &x_coord_widget, &y_coord_widget, &map_widget,
					&monster_widget, &count_widget, &cap_widget }),
				char_width(def_char_width), num_lines(def_num_lines), launch_command_valid(true) {}
			void init(void);
			void destroy(void);
			int display_window(window_info *win);
			int mouseover(window_info *win, int mx, int my);
			int click(window_info *win, int mx, int my, Uint32 flags);
			int ui_scale(window_info *win);
			void reload(void) { stop_play(); load_file_list(); }
			static bool valid_delay(unsigned int delay) { return ((delay >= min_delay) && (delay <= max_delay)); }
#ifdef JSON_FILES
			void save_win_state(void) const;
			void load_win_state(void);
#endif
			void start_play(void);
			void stop_play(void);
			void play_next(void);
			void launch(void);
			void launch_mouseover(void) { set_help((launch_command_valid) ?"Launch invasion" :"Last invasion command invalid"); }
			void update_play(void);
			void common_input_keypress(SDL_Keycode key_code, widget_list *widget);
			void common_input_mouseover(widget_list *widget);
			void set_help(const char *message) { help_text = message; }
			bool valid_file_selected(void) const
				{ return files_widget.get_selected() < commands_lists.size(); }
			bool valid_command_selected(void) const
				{ return valid_file_selected() &&
					(commands_widget.get_selected() < commands_lists[files_widget.get_selected()]->size()); }
			Command_List *get_selected_command_list(void)
				{ assert(valid_file_selected()); return commands_lists[files_widget.get_selected()]; }
		private:
			void load_file_list(void);
			void clear_commands_lists(void);
			std::vector<Command_List *> commands_lists;
			int reload_button_id;
			int play_button_id;
			int stop_button_id;
			int launch_button_id;
			int add_button_id;
			int replace_button_id;
			std::vector <int> button_ids;
			bool is_playing;
			Uint32 last_play_execute;
			List_Widget files_widget;
			List_Widget commands_widget;
			Labelled_Input_Widget delay_input_widget;
			Labelled_Input_Widget x_coord_widget;
			Labelled_Input_Widget y_coord_widget;
			Labelled_Input_Widget map_widget;
			Labelled_Input_Widget monster_widget;
			Labelled_Input_Widget count_widget;
			Labelled_Input_Widget cap_widget;
			std::vector <Labelled_Input_Widget *> input_widgets;
			std::string help_text;
			int char_width;
			float num_lines;
			bool launch_command_valid;
			static float min_num_lines, max_num_lines, def_num_lines;
			static int min_char_width, max_char_width, def_char_width;
			static int min_delay, max_delay, def_delay;
			static float min_scale, max_scale, def_scale;
	};

	//	The invasion window instance.
	static Container container;

	//	Limits and default for saved parameters
	float Container::min_num_lines = 5.0f, Container::max_num_lines = 40.0f, Container::def_num_lines = 10.0f;
	int Container::min_char_width = 60, Container::max_char_width = 180, Container::def_char_width = 60;
	int Container::min_delay = 1, Container::max_delay = 999, Container::def_delay = 10;
	float Container::min_scale = 0.25f, Container::max_scale = 3.0f, Container::def_scale = 1.0f;

	//	Window callback functions.
	//
	static int display_handler(window_info *win) { return container.display_window(win); }
	static int mouseover_handler(window_info *win, int mx, int my) { return container.mouseover(win, mx, my); }
	static int ui_scale_handler(window_info *win) { return container.ui_scale(win); }
	static int click_handler(window_info *win, int mx, int my, Uint32 flags) { return container.click(win, mx, my, flags); }
	static int list_name_reload_handler(widget_list *widget, int mx, int my) { container.reload(); return 0; }
	static int play_button_handler(widget_list *widget, int mx, int my) { container.start_play(); return 0; }
	static int stop_button_handler(widget_list *widget, int mx, int my) { container.stop_play(); return 0; }
	static int launch_button_handler(widget_list *widget, int mx, int my) { container.launch(); return 0; }
	static int add_button_handler(widget_list *widget, int mx, int my) { return 0; }
	static int replace_button_handler(widget_list *widget, int mx, int my) { return 0; }
	static int reload_mouseover_handler(widget_list *widget, int mx, int my) { container.set_help("Reload command files"); return 1; }
	static int play_mouseover_handler(widget_list *widget, int mx, int my) { container.set_help("Start command sequence"); return 1; }
	static int stop_mouseover_handler(widget_list *widget, int mx, int my) { container.set_help("Stop command sequence"); return 1; }
	static int launch_mouseover_handler(widget_list *widget, int mx, int my) { container.launch_mouseover(); return 1; }
	static int add_mouseover_handler(widget_list *widget, int mx, int my) { container.set_help("TBD: Append to current list"); return 1; }
	static int replace_mouseover_handler(widget_list *widget, int mx, int my) { container.set_help("TBD: Replace current list line"); return 1; }

	// Common keypress handler for labelled input widget.
	static int common_input_keypress_handler(widget_list *widget, int mx, int my, SDL_Keycode key_code, Uint32 key_unicode, Uint16 key_mod)
		{ container.common_input_keypress(key_code, widget); return 0; }

	// For a key press in a labelled input widget, find the objecting matching the widget id and call its handler.
	//
	void Container::common_input_keypress(SDL_Keycode key_code, widget_list *widget)
	{
		for (auto &i : input_widgets)
		{
			if (widget->id == i->get_input_id())
			{
				i->keypress(key_code);
				break;
			}
		}
	}

	// Common mouseover handler for labelled input widget.
	static int common_input_mouseover_handler(widget_list *widget, int mx, int my)
		{ container.common_input_mouseover(widget); return 1; }

	// When the mouse is over a labelled input widget, find the objecting matching the widget id and call its handler.
	//
	void Container::common_input_mouseover(widget_list *widget)
	{
		for (auto &i : input_widgets)
		{
			if (widget->id == i->get_input_id())
			{
				container.set_help(i->get_mouseover().c_str());
				break;
			}
		}
	}

	//	Init the invasion container, create or re-initialise the window.
	//
	void Container::init(void)
	{
		load_file_list();

		int win_id = get_id_MW(MW_INVASION);

		if (win_id < 0)
		{
			win_id = create_window("Invasion", (not_on_top_now(MW_INVASION) ?game_root_win : -1), 0,
				get_pos_x_MW(MW_INVASION), get_pos_y_MW(MW_INVASION), 0, 0,
				ELW_RESIZEABLE|ELW_USE_UISCALE|ELW_WIN_DEFAULT);
			if (win_id < 0 || win_id >= windows_list.num_windows)
			{
				LOG_ERROR("%s: Failed to create invasion window\n", __FILE__ );
				return;
			}
			set_id_MW(MW_INVASION, win_id);
			set_window_custom_scale(win_id, MW_INVASION);
			set_window_handler(win_id, ELW_HANDLER_DISPLAY, (int (*)())&display_handler);
			set_window_handler(win_id, ELW_HANDLER_MOUSEOVER, (int (*)())&mouseover_handler);
			set_window_handler(win_id, ELW_HANDLER_CLICK, (int (*)())&click_handler);
			set_window_handler(win_id, ELW_HANDLER_UI_SCALE, (int (*)())&ui_scale_handler);
			set_window_handler(win_id, ELW_HANDLER_FONT_CHANGE, (int (*)())&ui_scale_handler);

			int widget_id = 1;

			files_widget.init(&windows_list.window[win_id], widget_id++);

			reload_button_id = button_add_extended(win_id, widget_id++, NULL, 0, 0, 0, 0, 0, 1.0f, "Reload");
			widget_set_OnClick(win_id, reload_button_id, (int (*)())&list_name_reload_handler);
			widget_set_OnMouseover(win_id, reload_button_id, (int (*)())&reload_mouseover_handler);

			commands_widget.init(&windows_list.window[win_id], widget_id++);

			play_button_id = button_add_extended(win_id, widget_id++, NULL, 0, 0, 0, 0, 0, 1.0f, ">");
			widget_set_OnClick(win_id, play_button_id, (int (*)())&play_button_handler);
			widget_set_OnMouseover(win_id, play_button_id, (int (*)())&play_mouseover_handler);

			stop_button_id = button_add_extended(win_id, widget_id++, NULL, 0, 0, 0, 0, 0, 1.0f, "X");
			widget_set_OnClick(win_id, stop_button_id, (int (*)())&stop_button_handler);
			widget_set_OnMouseover(win_id, stop_button_id, (int (*)())&stop_mouseover_handler);

			launch_button_id = button_add_extended(win_id, widget_id++, NULL, 0, 0, 0, 0, 0, 1.0f, ">");
			widget_set_OnClick(win_id, launch_button_id, (int (*)())&launch_button_handler);
			widget_set_OnMouseover(win_id, launch_button_id, (int (*)())&launch_mouseover_handler);

			add_button_id = button_add_extended(win_id, widget_id++, NULL, 0, 0, 0, 0, 0, 1.0f, "+");
			widget_set_OnClick(win_id, add_button_id, (int (*)())&add_button_handler);
			widget_set_OnMouseover(win_id, add_button_id, (int (*)())&add_mouseover_handler);

			replace_button_id = button_add_extended(win_id, widget_id++, NULL, 0, 0, 0, 0, 0, 1.0f, "><");
			widget_set_OnClick(win_id, replace_button_id, (int (*)())&replace_button_handler);
			widget_set_OnMouseover(win_id, replace_button_id, (int (*)())&replace_mouseover_handler);

			button_ids = {reload_button_id, play_button_id, stop_button_id,
				launch_button_id, add_button_id, replace_button_id};

			ui_scale(&windows_list.window[win_id]);
			check_proportional_move(MW_INVASION);
		}
		else
		{
			show_window(win_id);
			select_window(win_id);
		}
	}

	//	Start auto execution of invasion commands.
	//
	void Container::start_play(void)
	{
		if (is_playing)
			return;
		if (valid_file_selected())
		{
			std::string tmp = "Started invasion sequence from " + get_selected_command_list()->get_text();
			LOG_TO_CONSOLE(c_green1, tmp.c_str());
			is_playing = true;
			play_next();
		}
	}

	//	Execute the next command and advance the queue.
	//
	void Container::play_next(void)
	{
		if (valid_command_selected())
		{
			get_selected_command_list()->execute(commands_widget.get_selected());

			size_t next = commands_widget.get_selected() + 1;
			if 	(next < get_selected_command_list()->size())
			{
				commands_widget.set_selected(next);
				last_play_execute = SDL_GetTicks();
				return;
			}
		}
		stop_play();
	}

	//	Stop auto execution of invasion commands.
	//
	void Container::stop_play(void)
	{
		if (!is_playing)
			return;
		do_alert1_sound();
		is_playing = false;
		last_play_execute = 0;
		LOG_TO_CONSOLE(c_green1, "Stopped invasion sequence");
	}

	//	Launch the current generator panel invasion.
	//
	void Container::launch(void)
	{
		std::vector <Labelled_Input_Widget *> widgets = { &x_coord_widget, &y_coord_widget,
			&map_widget, &monster_widget, &count_widget };
		if (cap_widget.get_checked())
			widgets.push_back(&cap_widget);

		for (auto &i : widgets)
			i->revalidate();

		for (auto &i : widgets)
			if (!i->is_valid())
			{
				launch_command_valid = false;
				do_alert1_sound();
				return;
			}

		Command *generated_command;
		if (cap_widget.get_checked())
			generated_command = new Command(x_coord_widget.get_number_value(), y_coord_widget.get_number_value(),
				map_widget.get_number_value(), monster_widget.get_string_value(), count_widget.get_number_value(),
				cap_widget.get_number_value());
		else
			generated_command = new Command(x_coord_widget.get_number_value(), y_coord_widget.get_number_value(),
				map_widget.get_number_value(), monster_widget.get_string_value(), count_widget.get_number_value());
		if (generated_command->is_valid())
		{
			launch_command_valid = true;
			generated_command->execute();
		}
		else
		{
			launch_command_valid = false;
			do_alert1_sound();
		}
		delete generated_command;
	}

	//	Clear the lists of invasion commands.
	//
	void Container::clear_commands_lists(void)
	{
		for (size_t i = 0; i < commands_lists.size(); ++i)
			delete commands_lists[i];
		commands_lists.clear();
	}

	//	Clean up the class.
	//
	void Container::destroy(void)
	{
		for (auto &i : input_widgets)
			i->destroy(get_id_MW(MW_INVASION));
		destroy_window(get_id_MW(MW_INVASION));
		set_id_MW(MW_INVASION, -1);
		clear_commands_lists();
	}

	//	Display the invasion window.
	//
	int Container::display_window(window_info *win)
	{
		if (win->resized)
		{
			static int last_x = -1, last_y = -1;
			if ((last_x != win->len_x) || (last_y != win->len_y))
			{
				last_x = win->len_x;
				last_y = win->len_y;
				ui_scale(win);
			}
		}

		files_widget.draw(commands_lists, false);

		if (valid_file_selected())
			commands_widget.draw(get_selected_command_list()->get(), is_playing);

		if (!help_text.empty())
		{
			show_help(help_text.c_str(), 0, win->len_y + win->small_font_len_y / 4, win->current_scale);
			help_text.clear();
		}

		return 1;
	}

	//	Called from the 0.5 second timer, check for play update.
	//
	void Container::update_play(void)
	{
		if ((is_playing) && ((last_play_execute + delay_input_widget.get_number_value() * 1000) < SDL_GetTicks()))
			play_next();
	}

	//	The mouse over callback for the window.
	//
	int Container::mouseover(window_info *win, int mx, int my)
	{
		if (files_widget.mouse_over(mx, my))
			set_help("Click to load command list, TBD: right-click for new/delete file.");
		if (commands_widget.mouse_over(mx, my))
			set_help("Double-click to execute command, TBD: right-click for edit/delete command.");
		return 0;
	}

	//	The mouse click callback for the window.
	//
	int Container::click(window_info *win, int mx, int my, Uint32 flags)
	{
		if (files_widget.in_list(mx, my))
		{
			if ((flags & ELW_LEFT_MOUSE) && (files_widget.get_under_mouse() < commands_lists.size()))
			{
				do_click_sound();
				stop_play();
				files_widget.set_selected(files_widget.get_under_mouse());
				commands_widget.set_selected(0);
				commands_widget.set_bar_len((valid_file_selected()) ?get_selected_command_list()->size(): 0);
			}
			files_widget.wheel_scroll(flags);
			return 1;
		}

		if (commands_widget.in_list(mx, my))
		{
			if ((flags & ELW_LEFT_MOUSE) && valid_file_selected() && (commands_widget.get_under_mouse() < get_selected_command_list()->size()))
			{
				static Uint32 last_click = 0;
				do_click_sound();
				commands_widget.set_selected(commands_widget.get_under_mouse());
				if (!safe_button_click(&last_click))
					return 1;
				if (!is_playing)
					get_selected_command_list()->execute(commands_widget.get_under_mouse());
			}
			commands_widget.wheel_scroll(flags);
			return 1;
		}

		return 1;
	}

	//	The UI scale callback for the window.
	//
	int Container::ui_scale(window_info *win)
	{
		float margin = win->small_font_len_y / 4;
		int new_widget_id = 101;

		// resize all the buttons
		for (auto i : button_ids)
			button_resize(win->window_id, i, 0, 0, win->current_scale);

		// recreate all the input widgets
		for (auto &i : input_widgets)
			i->create(win, &new_widget_id, margin / 2);

		float avg_char_width = get_avg_char_width_zoom(win->font_category, win->current_scale_small);

		float launch_width = widget_get_width(win->window_id, launch_button_id);
		float add_width = widget_get_width(win->window_id, add_button_id);
		float replace_width = widget_get_width(win->window_id, replace_button_id);

		float max_generate_input_width = 0;
		for (auto &i : input_widgets)
			if (i->get_input_id() != delay_input_widget.get_input_id())
				if (i->get_width() > max_generate_input_width)
					max_generate_input_width = i->get_width();
		float generator_panel_width = std::max(4 * margin + launch_width + add_width + replace_width,
			2 * margin + max_generate_input_width);

		// either use the last width of the two lists, or calculate new if the window is resizing
		if (win->resized)
			char_width = std::min(static_cast<float>(max_char_width), std::max(static_cast<float>(min_char_width),
				(win->len_x - generator_panel_width - 6 * margin - 3 * win->box_size) / avg_char_width));

		float name_list_width = avg_char_width * char_width / 3;
		float command_list_width = 2 * name_list_width;
		float win_x = 6 * margin + name_list_width + command_list_width + 3 * win->box_size + generator_panel_width;

		float button_height = widget_get_height(win->window_id, reload_button_id);

		// either use the last height of the two lists, or calculate new if the window is resizing
		if (win->resized)
			num_lines = std::min(max_num_lines, std::max(min_num_lines,
				(win->len_y - 3 * margin - button_height) / static_cast<int>(win->small_font_len_y * 1.1)));

		// keep the base of the lists aligned with the text, obsorbing any extra in the space to the buttons
		float list_height = static_cast<int>(num_lines) * static_cast<int>(win->small_font_len_y * 1.1);
		float win_y = std::max(num_lines * static_cast<int>(win->small_font_len_y * 1.1),
			(input_widgets.size() -1) * (button_height + margin) + margin);
		win_y += 3 * margin + button_height;

		// update the list of files
		files_widget.update(margin, name_list_width, margin, list_height, win->small_font_len_y * 1.1, 0, margin);
		files_widget.set_bar_len(commands_lists.size());

		// update the list of commands
		commands_widget.update(3 * margin + name_list_width + win->box_size,
			command_list_width, margin, list_height, win->small_font_len_y * 1.1, 0, margin);
		commands_widget.set_bar_len((valid_file_selected()) ?get_selected_command_list()->size(): 0);

		// all the buttons and the input field align at the top
		float button_y = win_y - button_height - margin;

		// reload button, x centred on names text
		float width = widget_get_width(win->window_id, reload_button_id);
		widget_move(win->window_id, reload_button_id, (name_list_width + 2 * margin - width) / 2, button_y);

		// recreate the delay widget
		delay_input_widget.create(win, &new_widget_id, margin / 2);

		// get the widths of the play, stop and delay widgets
		float play_width = widget_get_width(win->window_id, play_button_id);
		float stop_width = widget_get_width(win->window_id, stop_button_id);

		// play, stop and delay, x centred on the list text
		float total_with = 4 * margin + play_width + stop_width + delay_input_widget.get_width();
		float button_x = 2 * margin + name_list_width + win->box_size + (command_list_width + 2 * margin - total_with) / 2;
		widget_move(win->window_id, play_button_id, button_x, button_y);
		widget_move(win->window_id, stop_button_id, button_x + 2 * margin + play_width, button_y);
		delay_input_widget.move(win->window_id, button_x + 4 * margin + play_width + stop_width, button_y);

		// launch, add and replace, x centred on the generate panel, with gap for resize button
		float launch_x = 4 * margin + 2 * win->box_size + name_list_width + command_list_width +
			margin + (generator_panel_width - (margin * 4 + launch_width + add_width + replace_width)) / 2;
		widget_move(win->window_id, launch_button_id, launch_x, button_y);
		widget_move(win->window_id, add_button_id, launch_x + 2 * margin + launch_width, button_y);
		widget_move(win->window_id, replace_button_id, launch_x + 4 * margin + launch_width + add_width, button_y);

		// centre and right allign the generator panel input wigets in a column
		float right_offset = win_x - win->box_size - (generator_panel_width - max_generate_input_width) / 2 - margin;
		int y_line = 0;
		float y_space = button_height + margin;
		for (auto &i : input_widgets)
			if (i->get_input_id() != delay_input_widget.get_input_id())
				i->move(win->window_id, right_offset - i->get_width(), margin + y_line++ * y_space);

		resize_window(win->window_id, static_cast<int>(win_x + 0.5), static_cast<int>(win_y + 0.5));

		return 1;
	}

	//	Load the file list, all files in folder.
	//
	void Container::load_file_list(void)
	{
		std::string path = std::string(get_path_config_base()) + "invasion_lists/";
		std::string glob_path = path + "*.txt";
		std::vector<std::string> file_list;
#ifdef WINDOWS
		struct _finddata_t c_file;
		intptr_t hFile;
		if ((hFile = _findfirst(glob_path.c_str(), &c_file)) != static_cast<intptr_t>(-1))
		{
			do
				file_list.push_back(path + std::string(c_file.name));
			while (_findnext(hFile, &c_file) == 0);
			_findclose(hFile);
		}
#else	// phew! it's a real operating system
		glob_t glob_res;
		if (glob(glob_path.c_str(), 0, NULL, &glob_res)==0)
		{
			for (size_t i=0; i<glob_res.gl_pathc; i++)
				file_list.push_back(std::string(glob_res.gl_pathv[i]));
			globfree(&glob_res);
		}
#endif
		clear_commands_lists();
		for (size_t i = 0; i < file_list.size(); i++)
		{
			Command_List *new_list = new Command_List(file_list[i].c_str());
			if (new_list->is_valid())
				commands_lists.push_back(new_list);
			else
				delete new_list;
		}

		files_widget.set_selected(0);
		files_widget.set_bar_len(commands_lists.size());

		commands_widget.set_selected(0);
		commands_widget.set_bar_len((valid_file_selected()) ?get_selected_command_list()->size(): 0);
	}

#ifdef JSON_FILES
	//	Load the parameter from the client state file
	//
	void Container::load_win_state(void)
	{
		char window_dict_name[ELW_TITLE_SIZE];
		get_dict_name_WM(MW_INVASION, window_dict_name, sizeof(window_dict_name));
		*get_scale_WM(MW_INVASION) = std::min(max_scale, std::max(min_scale,
			json_cstate_get_float(window_dict_name, "scale", def_scale)));
		char_width = std::min(max_char_width, std::max(min_char_width,
			json_cstate_get_int(window_dict_name, "char_width", def_char_width)));
		num_lines = std::min(max_num_lines, std::max(min_num_lines,
			json_cstate_get_float(window_dict_name, "num_lines",def_num_lines)));
		delay_input_widget.set_init_value(std::min(max_delay, std::max(min_delay,
			json_cstate_get_int(window_dict_name, "delay_seconds", def_delay))));
	}

	//	Save the parameter to the client state file
	//
	void Container::save_win_state(void) const
	{
		char window_dict_name[ELW_TITLE_SIZE];
		get_dict_name_WM(MW_INVASION, window_dict_name, sizeof(window_dict_name));
		json_cstate_set_float(window_dict_name, "scale", *get_scale_WM(MW_INVASION));
		json_cstate_set_int(window_dict_name, "char_width", char_width);
		json_cstate_set_float(window_dict_name, "num_lines", static_cast<int>(num_lines));
		json_cstate_set_int(window_dict_name, "delay_seconds", delay_input_widget.get_number_value());
	}
#endif

} // end namespace invasion_window


extern "C"
{
	int command_invasion_window(char *text, int len)
	{
		invasion_window::container.init();
		return 1;
	}

	void display_invasion_win(void)
	{
		invasion_window::container.init();
	}

	void destroy_invasion_window(void)
	{
		invasion_window::container.destroy();
	}

	void update_play_invasion_window(void)
	{
		invasion_window::container.update_play();
	}

#ifdef JSON_FILES
	void save_invasion_window(void)
	{
		invasion_window::container.save_win_state();
	}

	void load_invasion_window(void)
	{
		invasion_window::container.load_win_state();
	}
#endif

}
