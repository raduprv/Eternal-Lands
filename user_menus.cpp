/*
Uses the Context Menu system to implement user configured command menus.

Files with a .menu name extension found in the users config directory are
considered user menu files. Each .menu file defines a new menu displayed
in a standard EL window container. The window is always on top and can be
moved around by the optional title bar. A standard right-click context menu
controls window options. These options include a run-time reload function so you
can update the menus without restarting the client. Left clicking a particular
menu name opens the menu.  The window options and position are saved
between sessions in the cfg file.  The use of user menus is enabled/disabled
by an option in the config window.

The .menu files are flat text files so they are easy to create and edit outside
the client. The first line in a .menu file is used as the menu name in the
container window. Each of the remaining lines are new lines for the menu. Lines
for the menus have an associated set of commands that are executed when
that line is selected. Valid commands are anything you can enter at the user
command prompt; #commands, text for chat channels & PM, %setting etc.  In
addition, commands can be URLs which will be directly opened in your browser.

Each menu line in a .menu file has two or more fields, the field separator is
"||". The first field is the text for the menu line, the remaining field or
fields are the associated commands.

Commands can prompt for user input. If the command contains text inside "<>",
e.g. "some text <more text> some other text" then the text inside the "<>" will
be used as a prompt and the "<...>" replaced by the text entered by the user.
You can include as many input prompts as you wish.


Author bluap/pjbroad May 2009
*/

// 	An example:
/*
Example
Show Server Stats||#stats
Show Invasion Monsters Count||#il
Show Day, Date & Time||#day||#date||#time
Show Day, Date & Time with 5 second delay||#user_menu_wait_time_ms 5000||#day||#date||#time||#user_menu_wait_time_ms
||
Say local hi||hi
Stats On||%show_stats_in_hud=1
Stats Off||%show_stats_in_hud=0
Channel Hi||@hi
||
Open Readonly Storage||#sto
Guild Info...||#guild_info <Guild Name>
||
Player Info...||#open_url http://game.eternal-lands.com/view_user.php?user=<Player Name>
BBC News||#open_url http://news.bbc.co.uk/
*/


/* To Do:
	- tidy up how the container window creates its context menu
	- restore context help text
	- split Container class, possibly into window/container classes
	- virtical/horizontal option
	- option auto hide down to icon
	- option hide/show from clicking an icon - handles at ends
	- make the menu prettier
	- use a subtle text fade once the mouse is no longer over
	- parameters should be separate (static ?) class or vars so do not create container usless needed
	- tear off windows - sounds a lot of work.....

*/


#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>
#ifdef WINDOWS
#include "io.h"
#else
#include <glob.h>
#endif

#include <SDL.h>
#include <SDL_thread.h>

#include "chat.h"
#include "command_queue.hpp"
#include "context_menu.h"
#include "elconfig.h"
#include "elwindows.h"
#include "errors.h"
#include "font.h"
#include "gamewin.h"
#include "gl_init.h"
#include "hud.h"
#include "hud_indicators.h"
#include "icon_window.h"
#include "init.h"
#include "io/elpathwrapper.h"
#ifdef JSON_FILES
#include "json_io.h"
#endif
#include "translate.h"
#include "user_menus.h"

namespace UserMenus
{
	//
	//	A single user menu, constructed from a menu file.  Contains
	//	one or more Line objects.
	//
	class Menu
	{
		public:
			Menu(const std::string &file_name);
			~Menu(void);
			const std::string & get_name(void) const { return menu_name; }
			int get_name_width(void) const { return menu_name_width; }
			int recalc_name_width(float zoom)
			{
				menu_name_width = eternal_lands::FontManager::get_instance()
					.line_width(UI_FONT, reinterpret_cast<const unsigned char*>(menu_name.c_str()),
						menu_name.length(), zoom);
				return menu_name_width;
			}
			size_t get_cm_id(void) const { return cm_menu_id; }
			void action(int option, CommandQueue::Queue &cq) const { lines[option]->action(cq); };
		private:
			size_t cm_menu_id;
			int menu_name_width;
			std::string menu_name;
			std::vector<CommandQueue::Line *> lines;
	};


	//
	//	A singleton class for the user menu window, contains the
	//	menu objects and implements the window callbacks.
	//
	class Container
	{
		public:
			void destroy(void);
			void open_window(void);
			void close_window(void) { command_queue.clear(); if (win_id >= 0) hide_window(win_id); }
			void set_options(int win_x, int win_y, int options);
			void get_options(int *win_x, int *win_y, int *options) const;
#ifdef JSON_FILES
			void read_options(const char *dict_name);
			void write_options(const char *dict_name) const;
#endif
			static Container * get_instance(void);
			static int action_handler(window_info *win, int widget_id, int mx, int my, int option) { return get_instance()->action(widget_id, option); }
			static void pre_show_handler(window_info *win, int widget_id, int mx, int my, window_info *cm_win) { get_instance()->pre_show(win, widget_id, mx, my, cm_win); }

		private:
			Container(void);
			int win_id;
			int win_width;
			size_t current_mouseover_menu;
			bool mouse_over_window;
			bool reload_menus;
			size_t context_id;
			bool window_used;
			int title_on;
			int background_on;
			int border_on;
			int use_small_font;
			int include_datadir;
			int just_echo;
			int win_x_pos;
			int win_y_pos;
			std::vector<Menu *> menus;
			CommandQueue::Queue command_queue;
			int name_sep;
			int window_x_pad, window_y_pad;
			int standard_window_position;

			void update_standard_window_position(window_info *win);
			void set_title_state(window_info *win, bool title_state) { title_on = (title_state) ?1:0; set_win_flag(&win->flags, ELW_TITLE_BAR, title_on); }
			void reload(window_info *win);
			void recalc_win_width(window_info *win);
			int display(window_info *win);
			int action(size_t active_menu, int option);
			void pre_show(window_info *win, int widget_id, int mx, int my, window_info *cm_win);
			int click(window_info *win, int mx, Uint32 flags);
			int ui_scale_changed(window_info *win);
			int font_changed(window_info *win, eternal_lands::FontManager::Category cat);
			size_t get_mouse_over_menu(window_info *win, int mx);
			void delete_menus(void);
			int context(window_info *win, int widget_id, int mx, int my, int option);
			void set_win_flag(Uint32 *flags, Uint32 flag, int state);
			void mouseover(window_info *win, int mx) { mouse_over_window = true; current_mouseover_menu = get_mouse_over_menu(win, mx); }
			int get_height(window_info *win) const { return ((use_small_font) ?win->small_font_len_y :win->default_font_len_y) + 2 * window_y_pad; }
			int calc_actual_width(window_info *win, int width) const
				{ return (int)(0.5 + (use_small_font ? win->current_scale_small : win->current_scale) * width); }

			static int display_handler(window_info *win) { return get_instance()->display(win); }
			static int mouseover_handler(window_info *win, int mx, int my) { get_instance()->mouseover(win, mx); return 0; }
			static int click_handler(window_info *win, int mx, int my, Uint32 flags) { return get_instance()->click(win, mx, flags); }
			static int ui_scale_handler(window_info *win) { return get_instance()->ui_scale_changed(win); }
			static int font_change_handler(window_info *win, font_cat cat) { return get_instance()->font_changed(win, cat); }
			static int context_handler(window_info *win, int widget_id, int mx, int my, int option){ return get_instance()->context(win, widget_id, mx, my, option); }

			enum { STND_POS_NONE = 0, STND_POS_TOP_LEFT, STND_POS_TOP_CENTRE, STND_POS_TOP_RIGHT, STND_POS_BOTTOM_LEFT, STND_POS_BOTTOM_CENTRE, STND_POS_BOTTON_RIGHT, STND_POS_LAST };
			enum { CM_MOVEWIN=1, CM_FIXPOS, CM_CHANGEPOS, CM_BACKGND, CM_BORDER, CM_FONT, CM_STANDMENU, CM_SEP1, CM_SHOWCMD, CM_SEP2, CM_RELOAD, CM_DISABLE };
	};


	//
	//	Construct the Menu given a filepath and name.
	//
	Menu::Menu(const std::string &file_name)
		: cm_menu_id(CM_INIT_VALUE), menu_name_width(0)
	{
		std::ifstream in(file_name.c_str());
		if (!in)
		{
			LOG_ERROR("%s: Failed to open [%s]\n", __FILE__, file_name.c_str() );
			return;
		}

		// the first line is the menu name, a menu without a name is not a menu
		getline(in, menu_name);
		if (menu_name.empty())
		{
			LOG_ERROR("%s: Failed while reading [%s]\n", __FILE__, file_name.c_str() );
			in.close();
			return;
		}
		menu_name_width = get_string_width_zoom((const unsigned char*)menu_name.c_str(),
			UI_FONT, 1.0);

		// read each line after the menu name line, creating a menu Line object for each
		std::string line;
		while (getline(in, line))
		{
			// lines starting with ## are ignored - like a comment
			if ((!line.empty()) && (line.substr(0,2) != "##"))
				lines.push_back(new CommandQueue::Line(line));
		}
		in.close();

		// no lines, no menu
		if (lines.empty())
			return;

		// create a context menu for the menu
		std::string menu_text;
		for (size_t i=0; i<lines.size(); i++)
			menu_text += lines[i]->get_text() + "\n";
		cm_menu_id = cm_create(menu_text.c_str(), Container::action_handler);
		cm_set_pre_show_handler(cm_menu_id, Container::pre_show_handler);

	} // end Menu()


	//
	// destruct a Menu, insuring the lines are deleted
	//
	Menu::~Menu(void)
	{
		if (cm_valid(cm_menu_id))
			cm_destroy(cm_menu_id);
		for (size_t i=0; i<lines.size(); i++)
			delete lines[i];
	}


	//
	//	constructor for Container, just initialises attributes
	//
	Container::Container(void) : win_id(-1), win_width(0), current_mouseover_menu(0), mouse_over_window(false),
		reload_menus(false), context_id(CM_INIT_VALUE), window_used(false), title_on(1), background_on(1),
		border_on(1), use_small_font(0), include_datadir(1), just_echo(0), win_x_pos(100),
		win_y_pos(100), name_sep(10), window_x_pad(8), window_y_pad(2), standard_window_position(STND_POS_NONE)
	{
	}


	//
	//	destroy the window, menus and lines
	//
	void Container::destroy(void)
	{
		delete_menus();
		if (cm_valid(context_id))
			cm_destroy(context_id);
		context_id = CM_INIT_VALUE;
		destroy_window(win_id);
	}


	//
	//	create the window for the user menus, or display it if already created
	//
	void Container::open_window(void)
	{
		if (win_id >= 0)
		{
			show_window(win_id);
			select_window(win_id);
			return;
		}

		Uint32 win_flags = ELW_USE_UISCALE|ELW_SHOW_LAST|ELW_DRAGGABLE|ELW_SHOW|ELW_TITLE_NAME|ELW_ALPHA_BORDER|ELW_SWITCHABLE_OPAQUE;

		set_win_flag(&win_flags, ELW_TITLE_BAR, title_on);
		set_win_flag(&win_flags, ELW_USE_BACKGROUND, background_on);
		set_win_flag(&win_flags, ELW_USE_BORDER, border_on);
		window_used = true;

		win_id = create_window(um_window_title_str, -1, 0, win_x_pos, win_y_pos, 0, 0, win_flags);
		if (win_id < 0  || win_id >= windows_list.num_windows)
			return;

		set_window_handler(win_id, ELW_HANDLER_DISPLAY, (int (*)())&display_handler );
		set_window_handler(win_id, ELW_HANDLER_MOUSEOVER, (int (*)())&mouseover_handler );
		set_window_handler(win_id, ELW_HANDLER_CLICK, (int (*)())&click_handler );
		set_window_handler(win_id, ELW_HANDLER_UI_SCALE, (int (*)())&ui_scale_handler );
		set_window_handler(win_id, ELW_HANDLER_FONT_CHANGE, (int (*)())&font_change_handler);

		// the build-in context menu is only available if we have a title so always recreate

		if (cm_valid(windows_list.window[win_id].cm_id))
		{
			cm_destroy(windows_list.window[win_id].cm_id);
			windows_list.window[win_id].cm_id = CM_INIT_VALUE;
		}

		context_id = cm_create(cm_title_menu_str, NULL);
		cm_bool_line(context_id, 1, &windows_list.window[win_id].opaque, NULL);
		cm_bool_line(context_id, 2, &windows_on_top, "windows_on_top");
		cm_add(context_id, cm_user_menu_str, context_handler);
		cm_add_window(context_id, win_id);

		cm_bool_line(context_id, ELW_CM_MENU_LEN+CM_MOVEWIN, &title_on, NULL);
		cm_bool_line(context_id, ELW_CM_MENU_LEN+CM_FIXPOS, &standard_window_position, NULL);
		cm_bool_line(context_id, ELW_CM_MENU_LEN+CM_BACKGND, &background_on, NULL);
		cm_bool_line(context_id, ELW_CM_MENU_LEN+CM_BORDER, &border_on, NULL);
		cm_bool_line(context_id, ELW_CM_MENU_LEN+CM_FONT, &use_small_font, NULL);
		cm_bool_line(context_id, ELW_CM_MENU_LEN+CM_STANDMENU, &include_datadir, NULL);
		cm_bool_line(context_id, ELW_CM_MENU_LEN+CM_SHOWCMD, &just_echo, NULL);

		ui_scale_changed(&windows_list.window[win_id]);
		reload(&windows_list.window[win_id]);

	} // Container::open_window()


	//
	//	return current window position and option values, normally for saving in the cfg file
	//
	void Container::get_options(int *win_x, int *win_y, int *options) const
	{
		if(win_id >= 0)
		{
			*win_x = windows_list.window[win_id].cur_x;
			*win_y = windows_list.window[win_id].cur_y;
		}
		else
		{
			*win_x = win_x_pos;
			*win_y = win_y_pos;
		}
		*options = (window_used) ?1: 0;
		*options |= title_on << 1;
		*options |= border_on << 2;
		*options |= use_small_font << 3;
		*options |= include_datadir << 4;
		*options |= (background_on ^ 1) << 5; // added later so will not get set for existing users but we want it on by default
		*options |= standard_window_position << 6; // three bits
	}


	//
	//	set window position and option, normally from the cfg file
	//
	void Container::set_options(int win_x, int win_y, int options)
	{
		if ((window_used = options & 1))
		{
			title_on = (options >> 1) & 1;
			border_on = (options >> 2) & 1;
			use_small_font = (options >> 3) & 1;
			include_datadir = (options >> 4) & 1;
			background_on = ((options >> 5) & 1) ^ 1;
			standard_window_position = (options >> 6) & 7; // three bits
			win_x_pos = win_x;
			win_y_pos = win_y;
		}
	}


#ifdef JSON_FILES
	//
	//	Write window position and option values, normally for saving in the cfg file
	//
	void Container::write_options(const char *dict_name) const
	{
		if(win_id >= 0)
		{
			json_cstate_set_int(dict_name, "pos_x", windows_list.window[win_id].cur_x);
			json_cstate_set_int(dict_name, "pos_y", windows_list.window[win_id].cur_y);
		}
		else
		{
			json_cstate_set_int(dict_name, "pos_x", win_x_pos);
			json_cstate_set_int(dict_name, "pos_y", win_y_pos);
		}
		json_cstate_set_bool(dict_name, "window_used", window_used);
		json_cstate_set_bool(dict_name, "title_on", title_on);
		json_cstate_set_bool(dict_name, "border_on", border_on);
		json_cstate_set_bool(dict_name, "use_small_font", use_small_font);
		json_cstate_set_bool(dict_name, "include_datadir", include_datadir);
		json_cstate_set_bool(dict_name, "background_off", background_on ^ 1);
		json_cstate_set_int(dict_name, "standard_window_position", standard_window_position);
	}


	//
	//	Read window position and option, normally from the cfg file
	//
	void Container::read_options(const char *dict_name)
	{
		if ((window_used = json_cstate_get_bool(dict_name, "window_used", 0)))
		{
			title_on = json_cstate_get_bool(dict_name, "title_on", 0);
			border_on = json_cstate_get_bool(dict_name, "border_on", 0);
			use_small_font = json_cstate_get_bool(dict_name, "use_small_font", 0);
			include_datadir = json_cstate_get_bool(dict_name, "include_datadir", 0);
			background_on = json_cstate_get_bool(dict_name, "background_off", 0) ^ 1;
			standard_window_position = json_cstate_get_int(dict_name, "standard_window_position", 0);
			win_x_pos = json_cstate_get_int(dict_name, "pos_x", 0);
			win_y_pos = json_cstate_get_int(dict_name, "pos_y", 0);
		}
	}
#endif


	//
	// the window display callback
	//
	int Container::display(window_info *win)
	{
		// if the menus need reloading, try it now
		if (reload_menus)
			reload(win);

		// update the command queue
		command_queue.process(just_echo);

		// a reload may have changed the window size
		if ((win_width != win->len_x) || (get_height(win) != win->len_y))
		{
			recalc_win_width(win);
			resize_window (win->window_id, win_width, get_height(win));
		}

		update_standard_window_position(win);

		// enable the title bar if the window ends up off screen - resolution change perhaps
		if ((win->cur_x + 20 > window_width) || (win->cur_y + 10 > window_height))
			set_title_state(win, true);

		int curr_x = window_x_pad;

		// if there are no menus, fill the window with a suitable message
		if (menus.empty())
		{
			if (mouse_over_window)
				glColor3fv(gui_color);
			else
				glColor3fv(gui_dull_color);
			if (use_small_font)
				draw_string_small_zoomed(curr_x, window_y_pad, (const unsigned char *)um_no_menus_str, 1, win->current_scale);
			else
				draw_string_zoomed(curr_x, window_y_pad, (const unsigned char *)um_no_menus_str, 1, win->current_scale );
			mouse_over_window = false;
			return 1;
		}

		// find the menu index of an already open menu window
		size_t open_menu = menus.size();
		size_t open_cm = cm_window_shown();
		if (open_cm != CM_INIT_VALUE)
		{
			for (size_t i=0; i<menus.size(); i++)
				if (menus[i]->get_cm_id() == open_cm)
					open_menu = i;
		}

		// draw the menu name text, hightlighting if it is open or if the mouse is over it
		for (size_t i=0; i<menus.size(); i++)
		{
			if ((current_mouseover_menu == i) || (open_menu == i))
				glColor3f(1.0f,1.0f,1.0f);
			else if (mouse_over_window)
				glColor3fv(gui_color);
			else
				glColor3fv(gui_dull_color);
			if (use_small_font)
				draw_string_small_zoomed(curr_x, window_y_pad, (const unsigned char *)menus[i]->get_name().c_str(), 1, win->current_scale);
			else
				draw_string_zoomed(curr_x, window_y_pad, (const unsigned char *)menus[i]->get_name().c_str(), 1, win->current_scale);
			curr_x += calc_actual_width(win, menus[i]->get_name_width()) + name_sep;
		}

		// make sure next time we will not highlight a menu name if the mouse is not in the window
		current_mouseover_menu = menus.size();
		mouse_over_window = false;

		return 1;

	} // end Container::display()


	//
	// open the menu if the name is clicked
	//
	int Container::click(window_info *win, int mx, Uint32 flags)
	{
		if ((flags & ELW_LEFT_MOUSE) &&
			((current_mouseover_menu = get_mouse_over_menu(win, mx)) < menus.size()) &&
			cm_valid(menus[current_mouseover_menu]->get_cm_id()))
		{
			cm_show_direct(menus[current_mouseover_menu]->get_cm_id(), win_id, current_mouseover_menu);
			return 1;
		}
		return 0;
	}


	//
	// Called when the UI is scaled
	//
	int Container::ui_scale_changed(window_info *win)
	{
		window_y_pad = static_cast<int>(0.5 + 2 * win->current_scale);
		window_x_pad = 4 * window_y_pad;
		name_sep = static_cast<int>(0.5 + 10 * win->current_scale);
		recalc_win_width(win);
		resize_window(win->window_id, win_width, get_height(win));
		return 1;
	}


	//
	// Called when font settings are changed
	//
	int Container::font_changed(window_info *win, eternal_lands::FontManager::Category cat)
	{
		if (cat != win->font_category)
			return 0;
		ui_scale_changed(win);
		return 1;
	}


	//
	// common callback fuction for context menu, line selection
	//
	int Container::action(size_t active_menu, int option)
	{
		if (active_menu < menus.size())
			menus[active_menu]->action(option, command_queue);
		return 1;
	}


	//
	//	adjust the menu window position to be more menu like
	//
	void Container::pre_show(window_info *win, int widget_id, int mx, int my, window_info *cm_win)
	{
		size_t curr_menu = (size_t)widget_id;
		if (win == NULL || cm_win == NULL || !(curr_menu<menus.size()))
		{
			std::cerr << __FUNCTION__ << ": Problem with input win=" << win << " widget=" << widget_id
			  << " mx=" << mx << " my=" << my << " cmwin=" << cm_win << std::endl;
			return;
		}

		// get the menu name x offset in the menu window
		int x_offset = window_x_pad;
		for (size_t i=0; i<curr_menu && i<menus.size(); i++)
			x_offset += calc_actual_width(win, menus[i]->get_name_width()) + name_sep;

		// see what fits x: position under the menu name, or hard at the right or hard at the left
		int new_x_pos = win->cur_x + x_offset;
		if (new_x_pos + cm_win->len_x > window_width)
			new_x_pos = window_width - cm_win->len_x;
		if (new_x_pos < 0)
			new_x_pos = 0;

		// see what fits y: position below the window menu, or position above the window menu
		int new_y_pos = win->cur_y;
		if (new_y_pos + win->len_y + cm_win->len_y > window_height)
			new_y_pos -= cm_win->len_y;
		else
			 new_y_pos += win->len_y;

   		move_window(cm_win->window_id, -1, 0, new_x_pos, new_y_pos);
	}


	//
	// the "evil" singleton mechanism
	//
	Container * Container::get_instance(void)
	{
		static Container um;
		static SDL_threadID creation_thread = SDL_ThreadID();
		if (SDL_ThreadID() != creation_thread)
			std::cerr << __FUNCTION__ << ": Danger W.R.! User menus call by non-creator thread." << std::endl;
		return &um;
	}


	//
	// load, or reload, the menu files
	//
	void Container::reload(window_info *win)
	{
		// if a context menu is currently showing, do not reload yet
		if (cm_window_shown() != CM_INIT_VALUE)
		{
			reload_menus = true;
			return;
		}
		reload_menus = false;

		delete_menus();

		std::vector<std::string> filelist;

		std::vector<std::string> search_paths;
		if (include_datadir)
			search_paths.push_back(std::string(datadir));
		search_paths.push_back(std::string(get_path_config()));

		for (size_t i=0; i<search_paths.size(); i++)
		{
			std::string glob_path = search_paths[i] + "*.menu";
			// find all the menu files and build a list of path+filenames for later
#ifdef WINDOWS
			struct _finddata_t c_file;
			long hFile;
			if ((hFile = _findfirst(glob_path.c_str(), &c_file)) != -1L)
			{
				do
					filelist.push_back(search_paths[i] + std::string(c_file.name));
				while (_findnext(hFile, &c_file) == 0);
				_findclose(hFile);
			}
#else 		// phew! it's a real operating system
			glob_t glob_res;
			if (glob(glob_path.c_str(), 0, NULL, &glob_res)==0)
			{
				for (size_t i=0; i<glob_res.gl_pathc; i++)
					filelist.push_back(std::string(glob_res.gl_pathv[i]));
				globfree(&glob_res);
			}
#endif
		}

		// process all the menu files, creating menu objects for each valid file
		for (size_t i=0; i<filelist.size(); i++)
		{
			Menu *umb = new Menu(filelist[i]);
			if (umb->get_name().empty())
				delete umb;
			else
				menus.push_back(umb);
		}

		current_mouseover_menu = menus.size();
		recalc_win_width(win);

	} // end Container::reload()


	//
	//	Calculates the window width, which changes with the zoom
	//
	void Container::recalc_win_width(window_info *win)
	{
		// if there are no menus, use the size of the message for the window width
		if (menus.empty())
		{
			win_width = 2 * window_x_pad + calc_actual_width(win, get_string_width_zoom((const unsigned char*)um_no_menus_str, win->font_category, 1.0));
			return;
		}

		// otherwise, calculate the width from the widths of all the menus names
		win_width = 2 * window_x_pad + name_sep * (menus.size() - 1);
		for (size_t i=0; i<menus.size(); i++)
			win_width += calc_actual_width(win, menus[i]->recalc_name_width(1.0));
	}


	//
	//	If the mouse is over a menu name and there are no context windows
	//	open then return the menu index, otherwise return the number of menus.
	//
	//	Side effects:
	//	If one of the menus is open and the mouse is over a different menu name,
	//	the current menu is closed and the one with the mouse over, opened.
	//
	size_t Container::get_mouse_over_menu(window_info *win, int mx)
	{
		// if the mouse is over a menu name, get the menus[] index
		size_t mouse_over = menus.size();
		int name_end_x = window_x_pad;
		for (size_t i=0; i<menus.size(); i++)
		{
			name_end_x += calc_actual_width(win, menus[i]->get_name_width()) + name_sep;
			if (mx < name_end_x-name_sep/2)
			{
				mouse_over = i;
				break;
			}
		}

		// if not over a menu name
		if (mouse_over == menus.size())
			return menus.size();

		// get the id of any open context menu, return with result of mouse index if none
		size_t open_cm = cm_window_shown();
		if (open_cm == CM_INIT_VALUE)
			return mouse_over;

		// a context menu is open, if it is one of our menus and the mouse is over another
		// close the current menu and open the one the mouse is over
		for (size_t i=0; i<menus.size(); i++)
			if (menus[i]->get_cm_id() == open_cm)
			{
				if (i != mouse_over)
				{
					cm_post_show_check(1);
					cm_show_direct(menus[mouse_over]->get_cm_id(), win_id, mouse_over);
				}
				break;
			}

		return menus.size();
	}


	//
	//  prepare for a menu reload or exit
	//
	void Container::delete_menus(void)
	{
		for (size_t i=0; i<menus.size(); i++)
			delete menus[i];
		menus.clear();
	}


	//
	//	change a window property bit flag
	//
	void Container::set_win_flag(Uint32 *flags, Uint32 flag, int state)
	{
		if (state)
			*flags |= flag;
		else
			*flags &= ~flag;
	}


	//
	//	Handle possitioning the window in one of the fixed locations.
	//
	void Container::update_standard_window_position(window_info *win)
	{
		if (title_on)
		{
			if (standard_window_position != STND_POS_NONE)
				standard_window_position = STND_POS_NONE;
			return;
		}

		int wanted_x = 0, wanted_y = 0, tmp_end = 0;

		switch (standard_window_position)
		{
			case STND_POS_TOP_LEFT:
				wanted_x = get_tabbed_chat_end_x() + window_x_pad;
				wanted_y = 0;
				break;
			case STND_POS_TOP_CENTRE:
				tmp_end = get_tabbed_chat_end_x();
				wanted_x = tmp_end + ( window_width - hud_x - get_fps_default_width() - tmp_end - win->len_x) / 2;
				wanted_y = 0;
				break;
			case STND_POS_TOP_RIGHT:
				wanted_x = window_width - hud_x - get_fps_default_width() - win->len_x - window_x_pad;
				wanted_y = 0;
				break;
			case STND_POS_BOTTOM_LEFT:
				wanted_x = window_x_pad + get_icons_win_active_len();
				wanted_y = window_height - win->len_y;
				break;
			case STND_POS_BOTTOM_CENTRE:
				tmp_end = get_icons_win_active_len();
				wanted_x = tmp_end + (window_width - hud_x - get_hud_indicators_default_width() - tmp_end - win->len_x) / 2;
				wanted_y = window_height - win->len_y;
				break;
			case STND_POS_BOTTON_RIGHT:
				wanted_x = window_width - hud_x - get_hud_indicators_default_width() - win->len_x - window_x_pad;
				wanted_y = window_height - win->len_y;
				break;
			default:
				return;
		}

		if ((win->cur_x != wanted_x) || (win->cur_y != wanted_y))
		{
			move_window(win->window_id, -1, 0, wanted_x, wanted_y);
			if ((win->cur_x != wanted_x) || (win->cur_y != wanted_y))
			{
				set_title_state(win, true);
				standard_window_position = STND_POS_NONE;
			}
		}
	}


	//
	//	handler window content menu options
	//
	int Container::context(window_info *win, int widget_id, int mx, int my, int option)
	{
		if (option<ELW_CM_MENU_LEN)
			return cm_title_handler(win, widget_id, mx, my, option);

		switch (option)
		{
			case ELW_CM_MENU_LEN+CM_MOVEWIN:
			{
				set_win_flag(&win->flags, ELW_TITLE_BAR, title_on);
				if (win->cur_y == win->title_height)
					move_window(win->window_id, -1, 0, win->cur_x, 0);
				else if (win->cur_y == 0)
					move_window(win->window_id, -1, 0, win->cur_x, win->title_height);
				break;
			}
			case ELW_CM_MENU_LEN+CM_FIXPOS:
				if (standard_window_position != STND_POS_NONE)
				{
					set_title_state(win, false);
					update_standard_window_position(win);
				}
				break;
			case ELW_CM_MENU_LEN+CM_CHANGEPOS:
				set_title_state(win, false);
				if (++standard_window_position >= STND_POS_LAST)
					standard_window_position = STND_POS_TOP_LEFT;
				break;
			case ELW_CM_MENU_LEN+CM_BACKGND: set_win_flag(&win->flags, ELW_USE_BACKGROUND, background_on); break;
			case ELW_CM_MENU_LEN+CM_BORDER: set_win_flag(&win->flags, ELW_USE_BORDER, border_on); break;
			case ELW_CM_MENU_LEN+CM_FONT: recalc_win_width(win); break;
			case ELW_CM_MENU_LEN+CM_STANDMENU: case ELW_CM_MENU_LEN+CM_RELOAD: reload(win); break;
			case ELW_CM_MENU_LEN+CM_DISABLE: toggle_user_menus(&enable_user_menus); break;
		}

		return 1;
	}

} // end UserMenus namespace


//
//	External Interface functions
//
extern "C"
{
	int enable_user_menus = 0;
	int ready_for_user_menus = 0;

	void set_options_user_menus(int win_x, int win_y, int options)
	{
		UserMenus::Container::get_instance()->set_options(win_x, win_y, options);
	}

	void get_options_user_menus(int *win_x, int *win_y, int *options)
	{
		UserMenus::Container::get_instance()->get_options(win_x, win_y, options);
	}

#ifdef JSON_FILES
	void read_options_user_menus(const char *dict_name)
	{
		UserMenus::Container::get_instance()->read_options(dict_name);
	}

	void write_options_user_menus(const char *dict_name)
	{
		UserMenus::Container::get_instance()->write_options(dict_name);
	}
#endif

	void toggle_user_menus(int *enable)
	{
		*enable = !*enable;
		set_var_unsaved("enable_user_menus", INI_FILE_VAR);
		if (!ready_for_user_menus)
			return;
		if (*enable)
			UserMenus::Container::get_instance()->open_window();
		else
			UserMenus::Container::get_instance()->close_window();
	}

	void display_user_menus(void)
	{
		if (ready_for_user_menus)
			UserMenus::Container::get_instance()->open_window();
	}

	void destroy_user_menus(void) { UserMenus::Container::get_instance()->destroy(); }
}
