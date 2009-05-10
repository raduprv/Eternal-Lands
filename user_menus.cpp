/*
Uses the Context Menu system to implement user configured command menus.

Files with a .menu name extension found in the users config directory are
considered user menu files. Each .menu file defines a new menu displayed 
in a standard EL window container. The window is always on top and can be
moved around by the optional title bar. A standard right-click context menu
controls window options. These options include a run-time reload function so you
can update the menus without restarting the client. Left clicking a particular
menu name opens the menu.  The window options and position are saved
between sessions in the el.cfg file.  The use of user menus is enabled/disabled
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


Author bluap/pjbroad May 2009
*/

// 	An example:
/*
Example
Show Server Stats||#stats
Show Invasion Monsters Count||#il
Show Day, Date & Time||#day||#date||#time
||
Say local hi||hi
Stats On||%show_stats_in_hud=1
Stats Off||%show_stats_in_hud=0
Channel Hi||@hi
||
Open Readonly Storage||#sto
||
BBC News||#open_url http://news.bbc.co.uk/
*/


/* To Do:
	- Add input prompt, e.g. "....||#knowledge <filter>" would prompt
		for "filter" then substitude in the executed command
	- Add option for key presses (from keys.ini file)
	- Add a "pause for no output" #command ?
	- build in delay between command to avoid spamming (Ent req)
	- block use of #suicide #reset #killme #change_pass - may be store in file
	- make the menu prettier
	- tidy up how the container window creates its context menu
	- restore context help text??
	- move litteral strings to translation module
	- virtical/horizontal option
	- option auto hide down to icon
	- option hide/show from clicking an icon
	- split Container class, possibly into window/container classes
	- option for global menus (in data_dir)
*/

#if defined(CONTEXT_MENUS)

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#ifdef WINDOWS
#include "io.h"
#else
#include <glob.h>
#endif

#include "asc.h"
#include "chat.h"
#include "context_menu.h"
#include "elconfig.h"
#include "elwindows.h"
#include "errors.h"
#include "font.h"
#include "gl_init.h"
#include "io/elpathwrapper.h"
#include "user_menus.h"

namespace UserMenus
{
	//
	//	A single menu line, menu name and assiociated commands extracted 
	//	from the string passed the to the constructor.
	//
	class Line
	{
		public:
			Line(std::string line_text);
			const std::string & get_text() const { return text; }
			const std::vector<std::string> & get_commands() const { return commands; }
		private:
			std::string text;
			std::vector<std::string> commands;
	};


	//
	//	A single user menu, constructed from a menu file.  Contains
	//	one or more Line objects.
	//
	class Menu
	{
		public:
			Menu(const std::string file_name);
			~Menu();
			const std::string & get_name() const { return menu_name; }
			int get_name_width() const { return menu_name_width; }
			size_t get_cm_id() const { return cm_menu_id; }
			int action_commands(int option);
		private:
			size_t cm_menu_id;
			int menu_name_width;
			std::string menu_name;
			std::vector<Line *> lines;
	};


	//
	//	A singleton class for the user menu window, contains the
	//	menu objects and implements the window callbacks.
	//
	class Container
	{
		public:
			~Container(void);
			void open_window();
			void close_window() { if (win_id >= 0) hide_window(win_id); }
			void set_options(int win_x, int win_y, int options);
			void get_options(int *win_x, int *win_y, int *options);
			static Container * get_instance();
			static int do_option_handler(window_info *win, int widget_id, int mx, int my, int option) { return get_instance()->do_option(widget_id, option); }
			static void pre_show_handler(window_info *win, int widget_id, int mx, int my, window_info *cm_win) { get_instance()->pre_show(win, widget_id, mx, my, cm_win); }

		protected:
			Container(void);

		private:
			int win_id;
			int win_width;
			size_t current_mouseover_menu;
			bool queue_reload;
			size_t context_id;
			bool window_used;
			int title_on;
			int border_on;
			int use_small_font;
			int win_x_pos;
			int win_y_pos;
			std::vector<Menu *> menus;
			static const int name_sep;
			static const int window_pad;
			static const char *no_menus;

			void reload();
			void recalc_win_width();
			int display(window_info *win);
			int do_option(size_t active_menu, int option);
			void pre_show(window_info *win, int widget_id, int mx, int my, window_info *cm_win);
			int click(window_info *win, int mx, Uint32 flags);
			size_t get_mouse_over_menu(int mx);
			void delete_menus(void);
			int context(window_info *win, int widget_id, int mx, int my, int option);
			void set_win_flag(Uint32 *flags, Uint32 flag, int state);
			void mouseover(int mx) { current_mouseover_menu = get_mouse_over_menu(mx); }
			int get_height(void) const { return static_cast<int>(((use_small_font)?SMALL_FONT_Y_LEN:DEFAULT_FONT_Y_LEN)+2*window_pad +0.5); }

			static int display_handler(window_info *win) { return get_instance()->display(win); }
			static int mouseover_handler(window_info *win, int mx, int my) { get_instance()->mouseover(mx); return 0; }
			static int click_handler(window_info *win, int mx, int my, Uint32 flags) { return get_instance()->click(win, mx, flags); }
			static int context_handler(window_info *win, int widget_id, int mx, int my, int option){ return get_instance()->context(win, widget_id, mx, my, option); }
	};




	//
	// construct a menu line from a text string
	//
	Line::Line(std::string line_text)
	{
		std::string::size_type from_index = 0;
		std::string::size_type to_index = 0;
		std::string delim = "||";
		std::string::size_type len = 0;

		// parse the line extracting the fields separated by the delimitor
		while ((to_index = line_text.find(delim, from_index)) != std::string::npos)
		{
			if ((len = to_index-from_index) > 0)
				commands.push_back(line_text.substr(from_index, len));
			from_index = to_index + delim.size();
		}
		if ((len = line_text.size()-from_index) > 0)
			commands.push_back(line_text.substr(from_index, len));

		// line with no fields are treated as context menu separators
		if (commands.empty())
		{
			text = "--";
			return;
		}

		// a line must always have at least two fields, the text and a command
		if (commands.size() == 1)
		{
			text = "<Error: invalid line>";
			commands.clear();
			return;
		}

		// the first field is the menu text, remaining feilds are the assiociated commands
		text = commands[0];
		commands.erase(commands.begin());

	} // end Line::Line()


	//
	//	Construct the Menu given a filepath and name.
	//
	Menu::Menu(const std::string file_name)
		: cm_menu_id(CM_INIT_VALUE), menu_name_width(0)
	{
		std::ifstream in(file_name.c_str());
		if (!in)
		{
			LOG_ERROR("%s: Failed to open [%s]\n", __FILE__, file_name.c_str() );
			return;
		}

		// th first line is the menu name, a menu without a name is not a menu
		getline(in, menu_name);
		if (menu_name.empty())
		{
			LOG_ERROR("%s: Failed while reading [%s]\n", __FILE__, file_name.c_str() );
			in.close();
			return;
		}
		menu_name_width = get_string_width((const unsigned char*)menu_name.c_str());

		// read each line after the menu name line, creating a menu Line object for each
		std::string line;
		while (getline(in, line))
		{
			if (!line.empty())
				lines.push_back(new Line(line));
		}
		in.close();

		// no lines, no menu
		if (lines.empty())
			return;

		// create a context menu for the menu
		std::string menu_text;
		for (size_t i=0; i<lines.size(); i++)
			menu_text += lines[i]->get_text() + "\n";
		cm_menu_id = cm_create(menu_text.c_str(), Container::do_option_handler);
		cm_set_pre_show_handler(cm_menu_id, Container::pre_show_handler);

	} // end Menu()


	//
	//	action the selection menu option 
	//
	int Menu::action_commands(int option)
	{
		if (lines[option]->get_commands().empty())
			return 1;

		for (size_t i=0; i<lines[option]->get_commands().size(); i++)
		{
			size_t command_len = lines[option]->get_commands()[i].size();
			char temp[command_len+1];
			safe_strncpy(temp, lines[option]->get_commands()[i].c_str(), command_len+1);
			parse_input(temp, strlen(temp));
		}
		return 1;
	}

	//
	// destruct a Menu, insuring the lines are deleted
	//
	Menu::~Menu()
	{
		if (cm_valid(cm_menu_id))
			cm_destroy(cm_menu_id);
		for (size_t i=0; i<lines.size(); i++)
			delete lines[i];
		lines.clear();
	}




	//	pixels between menu text
	const int Container::name_sep = 10;

	// 	pixels around the window edge
	const int Container::window_pad = 4;

	//	displayed if no user menus are found
	const char *Container::no_menus = "No User Menus";


	//
	//	constructor for Container, just initialises attributes
	//
	Container::Container(void) : win_id(-1), win_width(0), current_mouseover_menu(0), 
		queue_reload(false), context_id(CM_INIT_VALUE), window_used(false), title_on(1),
		border_on(1), use_small_font(0), win_x_pos(100), win_y_pos(100)
	{
	}


	//
	//	destroy the window, menus and lines
	//
	Container::~Container(void)
	{
		delete_menus();
		if (cm_valid(context_id))
			cm_destroy(context_id);
		destroy_window(win_id);
	}


	//
	//	create the window for the user menus, or display it if already created
	//
	void Container::open_window()
	{
		if (win_id >= 0)
		{
			show_window(win_id);
			select_window(win_id);
			return;
		}

		Uint32 win_flags = ELW_DRAGGABLE|ELW_USE_BACKGROUND|ELW_SHOW|ELW_TITLE_NAME|ELW_ALPHA_BORDER|ELW_SWITCHABLE_OPAQUE;
		
		set_win_flag(&win_flags, ELW_TITLE_BAR, title_on);
		set_win_flag(&win_flags, ELW_USE_BORDER, border_on);
		window_used = true;

		reload();

		win_id = create_window("User Menus", -1, 0, win_x_pos, win_y_pos,
			win_width, get_height(), win_flags);
			
		set_window_handler(win_id, ELW_HANDLER_DISPLAY, (int (*)())&display_handler );
		set_window_handler(win_id, ELW_HANDLER_MOUSEOVER, (int (*)())&mouseover_handler );
		set_window_handler(win_id, ELW_HANDLER_CLICK, (int (*)())&click_handler );

		// the build-in context menu is only available if we have a title so always recreate

		if (cm_valid(windows_list.window[win_id].cm_id))
		{
			cm_destroy(windows_list.window[win_id].cm_id);
			windows_list.window[win_id].cm_id = CM_INIT_VALUE;
		}

		context_id = cm_create(cm_title_menu_str, NULL);
		cm_bool_line(context_id, 1, &windows_list.window[win_id].opaque, NULL);
		cm_bool_line(context_id, 2, &windows_on_top, "windows_on_top");
		cm_add(context_id, "--\nShow Title\nDraw Border\nSmall Font\n--\nReload Menus", context_handler);
		cm_add_window(context_id, win_id);

		cm_bool_line(context_id, ELW_CM_MENU_LEN+1, &title_on, NULL);
		cm_bool_line(context_id, ELW_CM_MENU_LEN+2, &border_on, NULL);
		cm_bool_line(context_id, ELW_CM_MENU_LEN+3, &use_small_font, NULL);

	} // Container::open_window()


	//
	//	return current window position and option values, normally for saving in el.cfg
	//
	void Container::get_options(int *win_x, int *win_y, int *options)
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
	}


	//
	//	set window position and option, normally from el.cfg
	//
	void Container::set_options(int win_x, int win_y, int options)
	{
		if ((window_used = options & 1))
		{
			title_on = (options >> 1) & 1;
			border_on = (options >> 2) & 1;
			use_small_font = (options >> 3) & 1;
			win_x_pos = win_x;
			win_y_pos = win_y;
		}
	}


	//
	// the window display callback
	//
	int Container::display(window_info *win)
	{
		if (queue_reload)
			reload();

		// a reload may have changed the window size
		if ((win_width != win->len_x) || (get_height() != win->len_y))
			resize_window (win->window_id, win_width, get_height());

		// enable the title bar if the window ends up off screen - resolution change perhaps
		if ((win->cur_x + 20 > window_width) || (win->cur_y + 10 > window_height))
		{
			win->flags |= ELW_TITLE_BAR;
			title_on = 1;
		}

		int curr_x = window_pad;

		// if there are no menus, fill the window with a suitable message
		if (menus.empty())
		{
			glColor3f(0.40f,0.30f,0.20f);
			if (use_small_font)
				draw_string_small(curr_x, window_pad, (const unsigned char *)no_menus, 1 );
			else
				draw_string(curr_x, window_pad, (const unsigned char *)no_menus, 1 );
			return 1;
		}

		size_t open_menu = menus.size();
		size_t open_cm = cm_window_shown();
		if (open_cm != CM_INIT_VALUE)
		{
			for (size_t i=0; i<menus.size(); i++)
				if (menus[i]->get_cm_id() == open_cm)
					open_menu = i;
		}

		// draw the menu name text, hightlighting ne if the mouse is over it
		for (size_t i=0; i<menus.size(); i++)
		{
			if (open_menu == i)
				glColor3f(1.0f,1.0f,1.0f);
			else if (current_mouseover_menu == i)
				glColor3f(0.77f,0.57f,0.39f);
			else
				glColor3f(0.40f,0.30f,0.20f);
			if (use_small_font)
				draw_string_small(curr_x, window_pad, (const unsigned char *)menus[i]->get_name().c_str(), 1);
			else
				draw_string(curr_x, window_pad, (const unsigned char *)menus[i]->get_name().c_str(), 1);
			curr_x += ((use_small_font)?DEFAULT_SMALL_RATIO:1) * menus[i]->get_name_width() + name_sep;
		}

		// make sure next time we will not highlight a menu name if the mouse is not in the window
		current_mouseover_menu = menus.size();

		return 1;

	} // end Container::display()


	//
	// open the menu if the name is clicked
	//
	int Container::click(window_info *win, int mx, Uint32 flags)
	{
		if ((flags & ELW_LEFT_MOUSE) &&
			((current_mouseover_menu = get_mouse_over_menu(mx)) < menus.size()) &&
			cm_valid(menus[current_mouseover_menu]->get_cm_id()))
		{
			cm_show_direct(menus[current_mouseover_menu]->get_cm_id(), win_id, current_mouseover_menu);
			return 1;
		}
		return 0;
	}

	//
	// common callback fuction for context menu, line selection
	//
	int Container::do_option(size_t active_menu, int option)
	{
		if (active_menu < menus.size())
			return menus[active_menu]->action_commands(option);
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
		int x_offset = window_pad;				
		for (size_t i=0; i<curr_menu && i<menus.size(); i++)
			x_offset += ((use_small_font)?DEFAULT_SMALL_RATIO:1) * menus[i]->get_name_width() + name_sep;
		
		// see what fits x: position under the menu window, or hard at the right or hard at the left
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
	Container * Container::get_instance()
	{
		static Container um;
		return &um;
	}


	//
	// load, or reload, the menu files
	//
	void Container::reload()
	{
		// if a context menu is currently showing, do not reload yet
		if (cm_window_shown() != CM_INIT_VALUE)
		{
			queue_reload = true;
			return;
		}
		queue_reload = false;
		
		delete_menus();
		std::string glob_path = std::string(get_path_config()) + "*.menu";
		std::vector<std::string> filelist;

		// find all the menu files and build a list of path+filenames for later
#ifdef WINDOWS
		struct _finddata_t c_file;
		long hFile;
		if ((hFile = _findfirst(glob_path.c_str(), &c_file)) != -1L)
		{
			do
				filelist.push_back(std::string(get_path_config()) + std::string(c_file.name));
			while (_findnext(hFile, &c_file) == 0);	
			_findclose(hFile);
		}
#else 	// phew! it's a real operating system
		glob_t glob_res;
		if (glob(glob_path.c_str(), 0, NULL, &glob_res)==0)
		{
			for (size_t i=0; i<glob_res.gl_pathc; i++)
				filelist.push_back(std::string(glob_res.gl_pathv[i]));
			globfree(&glob_res);
		}
#endif

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
		recalc_win_width();

	} // end Container::reload()


	//
	//	Calculates the window width, which changes with the zoom
	//
	void Container::recalc_win_width()
	{
		// if there are no menus, use the size of the message for the window width
		if (menus.empty())
		{
			win_width = 2 * window_pad + ((use_small_font)?DEFAULT_SMALL_RATIO:1) * get_string_width((const unsigned char*)no_menus);
			return;
		}

		// otherwise, calculate the width from the widths of all the menus names
		win_width = 2 * window_pad + name_sep * (menus.size() - 1);
		for (size_t i=0; i<menus.size(); i++)		
			win_width += ((use_small_font)?DEFAULT_SMALL_RATIO:1) * menus[i]->get_name_width();
	}


	//
	//	If the mouse is over a menu name and there are no context windows
	//	open then return the menu index, otherwise return the number of menus.
	//
	//	Side effects:
	//	If one of the menus is open and the mouse is over a different menu name,
	//	the current menu is closed and the one with the mouse over, opened.
	//
	size_t Container::get_mouse_over_menu(int mx)
	{
		// if the mouse is over a menu name, get the menus[] index 
		size_t mouse_over = menus.size();
		int win_width = window_pad;
		for (size_t i=0; i<menus.size(); i++)
		{
			win_width += ((use_small_font)?DEFAULT_SMALL_RATIO:1) * menus[i]->get_name_width() + name_sep;
			if (mx < win_width-name_sep/2)
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
	
		// a context menu is open, if it is one of our menus and the mouse is open another
		// close the current menu and open the one user the mouse
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
	//	handler window content menu options
	//
	int Container::context(window_info *win, int widget_id, int mx, int my, int option)
	{
		if (cm_title_handler(win, widget_id, mx, my, option))
			return 1;

		switch (option)
		{
			case ELW_CM_MENU_LEN+1:
			{
				set_win_flag(&win->flags, ELW_TITLE_BAR, title_on);
				if (win->cur_y == ELW_TITLE_HEIGHT)
					move_window(win->window_id, -1, 0, win->cur_x, 0);
				else if (win->cur_y == 0)
					move_window(win->window_id, -1, 0, win->cur_x, ELW_TITLE_HEIGHT);
				break;
			}
			case ELW_CM_MENU_LEN+2: set_win_flag(&win->flags, ELW_USE_BORDER, border_on); break;
			case ELW_CM_MENU_LEN+3: recalc_win_width(); break;
			case ELW_CM_MENU_LEN+5: reload(); break;
		}

		return 1;
	}

} // end UserMenus namespace


//
//	External Interface functions
//
extern "C"
{
	int enable_user_menus = 1;

	void set_options_user_menus(int win_x, int win_y, int options)
	{
		UserMenus::Container::get_instance()->set_options(win_x, win_y, options);
	}

	void get_options_user_menus(int *win_x, int *win_y, int *options)
	{
		UserMenus::Container::get_instance()->get_options(win_x, win_y, options);
	}

	void toggle_user_menus(int *enable)
	{
		*enable = !*enable;
		if (*enable)
			UserMenus::Container::get_instance()->open_window();
		else
			UserMenus::Container::get_instance()->close_window();
	}

	void display_user_menus(void)
	{
		UserMenus::Container::get_instance()->open_window();
	}
}

#endif // CONTEXT_MENUS
