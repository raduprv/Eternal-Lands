#include <iostream>
#include <utility>
#include <cassert>

#include "asc.h"
#include "actors.h"
#include "elwindows.h"
#include "gamewin.h"
#include "gl_init.h"
#include "hud.h"
#include "interface.h"
#include "multiplayer.h"
#include "new_character.h"
#include "icon_window.h"
#include "init.h"
#include "textures.h"
#include "translate.h"
#include "sound.h"


/* ToDo
 *
 * 	Remove debug statments
 * 	Add comments
 * 	Add reading data files
 * 	Add reload #command or context option
 * 	Add icon size control
 *	Add icon window position code
 *
 */

namespace IconWindow
{
	class Virtual_Icon
	{
		public:
			virtual const char *get_help_message(void) const = 0;
			virtual void set_flash(Uint32 seconds) = 0;
			virtual void set_highlight(bool is_highlighted) = 0;
			virtual void update_highlight(void) = 0;
			virtual std::pair<float, float> get_uv(void) = 0;
			virtual void action(void) = 0;
			virtual ~Virtual_Icon(void) { std::cout << "destruct Virtual_Icon" << std::endl; };
	};


	class Basic_Icon : public Virtual_Icon
	{
		public:
			Basic_Icon(int icon_id, int coloured_icon_id, const char * help_name);
			virtual ~Basic_Icon(void) { std::cout << "destruct Basic_Icon" << std::endl; }
			virtual const char *get_help_message(void) const { return help_message; }
			virtual void set_flash(Uint32 seconds) { flashing = 4*seconds; }
			virtual void set_highlight(bool is_highlighted) { has_highlight = is_highlighted; }
			virtual void update_highlight(void) { has_highlight = false; }
			virtual void action(void) { flashing = 0; }
			virtual std::pair<float, float> get_uv(void);
		private:
			bool has_highlight;				// true if the icon is highlighted
			float u[2], v[2];				// icon image positions
			const char * help_message;		// icon help message
			Uint32 flashing;				// if non-zero, the number times left to flash
			Uint32 last_flash_change;		// if flashing, the time the flashing state last changed
	};


	class Multi_Icon : public Virtual_Icon
	{
		public:
			Multi_Icon(const char *control_name, std::vector<Virtual_Icon *> &the_icons)
				: control_var(0), icons(the_icons)
			{
				std::cout << "construct Multi_Icon size=" << icons.size() << std::endl;
				assert(!the_icons.empty());
				if (control_name && (strlen(control_name) > 0))
					for (size_t i=0; multi_icon_vars[i].name!=0; i++)
						if (strcmp(control_name, multi_icon_vars[i].name) == 0)
							control_var = multi_icon_vars[i].var;
			}
			~Multi_Icon(void)
			{
				std::cout << "destruct Multi_Icon" << std::endl;
				for (size_t i=0; i<icons.size(); ++i)
					delete icons[i];
			}
			const char *get_help_message(void) const { return icons[get_index()]->get_help_message(); }
			void set_flash(Uint32 seconds) { icons[get_index()]->set_flash(seconds); }
			void set_highlight(bool is_highlighted) { icons[get_index()]->set_highlight(is_highlighted); }
			void update_highlight(void) { icons[get_index()]->update_highlight(); }
			std::pair<float, float> get_uv(void) { return icons[get_index()]->get_uv(); }
			void action(void) { icons[get_index()]->action(); }
		private:
			size_t get_index(void) const
			{
				if (control_var && (*control_var>=0) && ((size_t)*control_var<icons.size()))
					return (size_t)*control_var;
				else
					return 0;
			}
			int *control_var;
			std::vector<Virtual_Icon *> icons;
			typedef struct { const char *name; int *var; } multi_icon_var;
			static const multi_icon_var multi_icon_vars[];
	};


	const Multi_Icon::multi_icon_var Multi_Icon::multi_icon_vars[] = {
		{ "you_sit", &you_sit },
		{ 0, 0 } /* needed as terminator */ };


	class Window_Icon : public Basic_Icon
	{
		public:
			Window_Icon(int icon_id, int coloured_icon_id, const char * help_name, const char * window_name)
				: Basic_Icon(icon_id, coloured_icon_id, help_name), window_id(0)
			{
				std::cout << "construct Window_Icon name=[" << ((window_name)?window_name : "") << "]" << std::endl;
				window_id = get_winid(window_name);
			}
			void update_highlight(void)
			{
				if ( window_id &&  *window_id >= 0 && (windows_list.window[*window_id].displayed || windows_list.window[*window_id].reinstate) )
					Basic_Icon::set_highlight(true);
				else
					Basic_Icon::set_highlight(false);
			}
			void action(void)
			{
				std::cout << "Window_Icon::action()" << std::endl;
				if (window_id)
					view_window(window_id, 0);
				Basic_Icon::action();
			}
			~Window_Icon(void) { std::cout << "destruct Window_Icon" << std::endl; }
		private:
			int *window_id;	
	};


	class Keypress_Icon : public Basic_Icon
	{
		public:
			Keypress_Icon(int icon_id, int coloured_icon_id, const char * help_name, const char * the_key_name)
				: Basic_Icon(icon_id, coloured_icon_id, help_name)
			{
				std::cout << "construct Keypress_Icon" << std::endl;
				if (the_key_name && (strlen(the_key_name)>0))
					key_name = std::string(the_key_name);
			}
			~Keypress_Icon(void) { std::cout << "destruct Keypress_Icon" << std::endl; }
			void action(void)
			{
				std::cout << "Keypress_Icon::action" << std::endl; 
				if (!key_name.empty())
				{
					Uint32 value = get_key_value(key_name.c_str());
					if (value)
						keypress_root_common(value, 0);
				}
				Basic_Icon::action();
			}
		private:
			std::string key_name;
	};


	class Actionmode_Icon : public Basic_Icon
	{
		public:
			Actionmode_Icon(int icon_id, int coloured_icon_id, const char * help_name, const char * action_name)
				: Basic_Icon(icon_id, coloured_icon_id, help_name), the_action_mode(ACTION_WALK)
			{
				std::cout << "construct Actionmode_Icon action=[" << ((action_name) ?action_name : "") << "]" << std::endl;
				if (action_name && (strlen(action_name) > 0))
					for (size_t i=0; icon_action_modes[i].name!=0; i++)
						if (strcmp(action_name, icon_action_modes[i].name) == 0)
							the_action_mode = icon_action_modes[i].mode;
			}
			~Actionmode_Icon(void) { std::cout << "destruct Actionmode_Icon" << std::endl; }
			void update_highlight(void)
			{
				Basic_Icon::set_highlight((action_mode == the_action_mode));
			}
			void action(void)
			{
				std::cout << "Actionmode_Icon::action()" << std::endl;
				switch_action_mode(&the_action_mode, 0);
				Basic_Icon::action();
			}
		private:
			int the_action_mode;
			typedef struct { const char *name; int mode; } icon_action_mode;
			static const icon_action_mode icon_action_modes[];
	};

	const Actionmode_Icon::icon_action_mode Actionmode_Icon::icon_action_modes[] = {
		{ "walk", ACTION_WALK },
		{ "look", ACTION_LOOK },
		{ "use", ACTION_USE },
		{ "use_witem", ACTION_USE_WITEM },
		{ "trade", ACTION_TRADE },
		{ "attack", ACTION_ATTACK },
		{ 0, 0 } /* needed as terminator */ };


	class Container
	{
		public:
			Container(void) : mouse_over_icon(-1) {}
			size_t get_num_icons(void) const { return icon_list.size(); }
			bool empty(void) const { return icon_list.empty(); };
			void mouse_over(int icon_number) { mouse_over_icon = icon_number; }
			void draw_icons(void);
			void init_new_character(void);
			void init_main(void);
			void action(size_t icon_number)
			{
				if (icon_number < icon_list.size())
				{
					icon_list[icon_number]->action();
					do_icon_click_sound();
				}
			}
			void free(void)
			{
				for (size_t i=0; i<icon_list.size(); ++i)
					delete icon_list[i];
				icon_list.clear();
			}
			void flash(const char* name, Uint32 seconds)
			{
				for (size_t i=0; i<icon_list.size(); ++i)
					if (strcmp(name, icon_list[i]->get_help_message()) == 0)
					{
						icon_list[i]->set_flash(seconds);
						break;
					}
			}
		private:
			std::vector <Virtual_Icon *> icon_list;
			int mouse_over_icon;
	};


	Basic_Icon::Basic_Icon(int icon_id, int coloured_icon_id, const char * help_name)
	{
		std::cout << "construct Basic_Icon" << std::endl;
		has_highlight = false;
#ifdef	NEW_TEXTURES
		u[0] = 32.0 * (float)(icon_id % 8)/256.0;
		u[1] = 32.0 * (float)(coloured_icon_id % 8)/256.0;
#else
		u[0] = 1.0 - (32.0 * (float)(icon_id % 8))/256.0;
		u[1] = 1.0 - (32.0 * (float)(coloured_icon_id % 8))/256.0;
#endif
		v[0] = 32.0 * (float)(icon_id >> 3)/256.0;
		v[1] = 32.0 * (float)(coloured_icon_id >> 3)/256.0;
		help_message = get_named_string("tooltips", help_name);
		flashing = 0;
	}


	std::pair<float, float> Basic_Icon::get_uv(void)
	{
		size_t index = (has_highlight)? 1: 0;
		if (flashing)
		{
			if (abs(SDL_GetTicks() - last_flash_change) > 250)
			{
				last_flash_change = SDL_GetTicks();
				flashing--;
			}
			index = flashing & 1;
		}
		return std::pair<float, float>(u[index], v[index]);
	}


	void Container::draw_icons(void)
	{
		for (size_t i=0; i<icon_list.size(); ++i)
			icon_list[i]->update_highlight();
		if ((mouse_over_icon >= 0) && ((size_t)mouse_over_icon < icon_list.size()))
			icon_list[mouse_over_icon]->set_highlight(true);
		float uoffset = 31.0/256.0, voffset = 31.0/256.0;
#ifdef	NEW_TEXTURES
		bind_texture(icons_text);
#else	/* NEW_TEXTURES */
		get_and_set_texture_id(icons_text);
		voffset *= -1;
#endif	/* NEW_TEXTURES */
		glColor3f(1.0f,1.0f,1.0f);
		glBegin(GL_QUADS);
		for (size_t i=0; i<icon_list.size(); ++i)
		{
			std::pair<float, float> uv = icon_list[i]->get_uv();
			draw_2d_thing( uv.first, uv.second, uv.first+uoffset, uv.second+voffset, i*32, 0, i*32+31, 32 );
		}
		glEnd();
		if (show_help_text && (mouse_over_icon >= 0) && ((size_t)mouse_over_icon < icon_list.size()))
			show_help(icon_list[mouse_over_icon]->get_help_message(), 32*(mouse_over_icon+1)+2, 10);
		mouse_over_icon = -1;
	}


	void Container::init_new_character(void)
	{
#ifndef NEW_NEW_CHAR_WINDOW
		icon_list.push_back(new Window_Icon(0, 18, "name_pass", "name_pass"));
		icon_list.push_back(new Window_Icon(2, 20, "customize", "customize"));
#endif
		icon_list.push_back(new Window_Icon(39, 38, "help", "help"));
		icon_list.push_back(new Window_Icon(14, 34, "opts", "opts"));
	}

	void Container::init_main(void)
	{
		icon_list.push_back(new Actionmode_Icon(0, 18, "walk", "walk"));

		std::vector<Virtual_Icon *> sit_stand_icons;
		sit_stand_icons.push_back(new Keypress_Icon(7, 25, "sit", "#K_SIT"));
		sit_stand_icons.push_back(new Keypress_Icon(8, 26, "stand", "#K_SIT"));
		icon_list.push_back(new Multi_Icon("you_sit", sit_stand_icons));

		icon_list.push_back(new Actionmode_Icon(2, 20, "look", "look"));
		icon_list.push_back(new Actionmode_Icon(15, 35, "use", "use"));
		icon_list.push_back(new Actionmode_Icon(47, 46, "use_witem", "use_witem"));
		icon_list.push_back(new Actionmode_Icon(4, 22, "trade", "trade"));
		icon_list.push_back(new Actionmode_Icon(5, 23, "attack", "attack"));

		icon_list.push_back(new Window_Icon(11, 29, "invent", "invent"));
		icon_list.push_back(new Window_Icon(9, 27, "spell", "spell"));
		icon_list.push_back(new Window_Icon(12, 32, "manu", "manu"));
		icon_list.push_back(new Window_Icon(45, 44, "emotewin", "emotewin"));
		icon_list.push_back(new Window_Icon(19, 21, "quest", "quest"));
		icon_list.push_back(new Window_Icon(36, 37, "map", "map"));
		icon_list.push_back(new Window_Icon(3, 6, "info", "info"));
		icon_list.push_back(new Window_Icon(10, 24, "buddy", "buddy"));
		icon_list.push_back(new Window_Icon(13, 33, "stats", "stats"));
		icon_list.push_back(new Window_Icon(1, 28, "console", "console"));
		icon_list.push_back(new Window_Icon(39, 38, "help", "help"));
		icon_list.push_back(new Window_Icon(14, 34, "opts", "opts"));

		icon_list.push_back(new Window_Icon(16, 17, "range", "range"));
		icon_list.push_back(new Window_Icon(30, 31, "minimap", "minimap"));

		icon_list.push_back(new Keypress_Icon(16, 17, 0, 0));
		icon_list.push_back(new Window_Icon(16, 17, 0, 0));
		icon_list.push_back(new Actionmode_Icon(16, 17, 0, 0));
	}

}

static IconWindow::Container action_icons;

static int	mouseover_icons_handler(window_info *win, int mx, int my)
{
	action_icons.mouse_over(mx/32);
	return 0;
}

static int	display_icons_handler(window_info *win)
{
	action_icons.draw_icons();
	return 1;
}

static int	click_icons_handler(window_info *win, int mx, int my, Uint32 flags)
{
	if ( (flags & ELW_MOUSE_BUTTON) == 0)
		return 0; // only handle mouse button clicks, not scroll wheels moves;
	action_icons.action(mx/32);
	return 1;
}


static void create_icon_window(void)
{
	if(icons_win < 0)
	{
		icons_win= create_window("Icons", -1, 0, 0, window_height-32, window_width-hud_x, 32, ELW_TITLE_NONE|ELW_SHOW_LAST);
		set_window_handler(icons_win, ELW_HANDLER_DISPLAY, (int (*)())&display_icons_handler);
		set_window_handler(icons_win, ELW_HANDLER_CLICK, (int (*)())&click_icons_handler);
		set_window_handler(icons_win, ELW_HANDLER_MOUSEOVER, (int (*)())&mouseover_icons_handler);
	}
	else
		move_window(icons_win, -1, 0, 0, window_height-32);
}

int	icons_win = -1;


extern "C" int get_icons_win_active_len(void)
{
	return 32 * action_icons.get_num_icons();
}

extern "C" void free_icons()
{
	action_icons.free();
}

extern "C" void flash_icon(const char* name, Uint32 seconds)
{
	action_icons.flash(name, seconds);
}

extern "C" void init_newchar_icons(void)
{
	if (newchar_root_win < 0)
		return;	/* wait until we have the root window to avoid hiding this one */

	create_icon_window();

	if (action_icons.empty())
		action_icons.init_new_character();

	resize_window(icons_win, 32*action_icons.get_num_icons(), 32);
}

extern "C" void init_peace_icons(void)
{
	create_icon_window();

	if (action_icons.empty())
		action_icons.init_main();

	resize_window(icons_win, 32*action_icons.get_num_icons(), 32);
}
