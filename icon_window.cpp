/*
	Display icon window.

	Rewritten from the code previously contained in hud.c, primarily so
	the icons can be configured from an xml file at run-time.

	Author bluap/pjbroad December 2012 / January 2013
*/
#include <iostream>
#include <sstream>
#include <utility>
#include <cassert>
#include <algorithm>

#include "asc.h"
#include "actors.h"
#include "chat.h"
#include "context_menu.h"
#include "elwindows.h"
#include "elloggingwrapper.h"
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
#include "command_queue.hpp"

/* 
 * TODO		Add icon window position code - allowing the window to be repositioned
 * TODO		Look for xml files in user config directory
 * TODO		Give indication that command queue is busy?
 */

namespace IconWindow
{
	//	Common base class for all icon types.
	//
	class Virtual_Icon
	{
		public:
			virtual const char *get_help_message(void) const = 0;
			virtual void set_flash(Uint32 seconds) = 0;
			virtual void set_highlight(bool is_highlighted) = 0;
			virtual void update_highlight(void) = 0;
			virtual std::pair<float, float> get_uv(void) = 0;
			virtual void action(void) = 0;
			virtual void menu(void) = 0;
			virtual int cm_handler(size_t option) = 0;
			virtual ~Virtual_Icon(void) {};
	};


	//	Implements the basic icon function code
	//
	class Basic_Icon : public Virtual_Icon
	{
		public:
			Basic_Icon(int icon_id, int coloured_icon_id, const char * help_str, const std::vector<CommandQueue::Line> *lines = 0);
			virtual ~Basic_Icon(void) { if (cq) delete cq; if (cm_valid(cm_menu_id)) cm_destroy(cm_menu_id); }
			virtual const char *get_help_message(void) const { return help_message.c_str(); }
			virtual void set_flash(Uint32 seconds) { flashing = 4*seconds; }
			virtual void set_highlight(bool is_highlighted) { has_highlight = is_highlighted; }
			virtual void update_highlight(void) { has_highlight = false; if (cq) cq->process(); }
			virtual void action(void) { flashing = 0; do_icon_click_sound(); }
			virtual void menu(void) { if (cm_valid(cm_menu_id)) cm_show_direct(cm_menu_id, -1, -1); else action(); }
			virtual std::pair<float, float> get_uv(void);
			virtual int cm_handler(size_t option) { if (!cm_valid(cm_menu_id) || (option>=menu_lines.size())) return 0; menu_lines[option].action(*get_cq()); return 1; }
		protected:
			CommandQueue::Queue *get_cq(void) { if (!cq) cq = new CommandQueue::Queue(); return cq; }
		private:
			bool has_highlight;				// true if the icon is highlighted
			float u[2], v[2];				// icon image positions
			std::string help_message;		// icon help message
			Uint32 flashing;				// if non-zero, the number times left to flash
			Uint32 last_flash_change;		// if flashing, the time the flashing state last changed
			std::vector<CommandQueue::Line> menu_lines; 	// context menu #command lines
			CommandQueue::Queue *cq;		// Command queue for commands
			size_t cm_menu_id;				// if this icon has a context menu, this is the id, otherwise CM_INIT_VALUE
	};


	//	Implements multi icon, icons using a specifed control state variable.
	//
	class Multi_Icon : public Virtual_Icon
	{
		public:
			Multi_Icon(const char *control_name, std::vector<Virtual_Icon *> &the_icons)
				: control_var(0), icons(the_icons)
			{
				assert(!the_icons.empty());
				if (control_name && (strlen(control_name) > 0))
					for (size_t i=0; multi_icon_vars[i].name!=0; i++)
						if (strcmp(control_name, multi_icon_vars[i].name) == 0)
							control_var = multi_icon_vars[i].var;
			}
			~Multi_Icon(void)
			{
				for (size_t i=0; i<icons.size(); ++i)
					delete icons[i];
			}
			const char *get_help_message(void) const { return icons[get_index()]->get_help_message(); }
			void set_flash(Uint32 seconds) { icons[get_index()]->set_flash(seconds); }
			void set_highlight(bool is_highlighted) { icons[get_index()]->set_highlight(is_highlighted); }
			void update_highlight(void) { icons[get_index()]->update_highlight(); }
			std::pair<float, float> get_uv(void) { return icons[get_index()]->get_uv(); }
			void action(void) { icons[get_index()]->action(); }
			void menu(void) { icons[get_index()]->menu(); }
			int cm_handler(size_t option) { return icons[get_index()]->cm_handler(option); }
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


	// Add new multi icon control variables to this list
	const Multi_Icon::multi_icon_var Multi_Icon::multi_icon_vars[] = {
		{ "you_sit", &you_sit },
		{ 0, 0 } /* needed as terminator */ };


	//	Implements window open/close icons.
	//
	class Window_Icon : public Basic_Icon
	{
		public:
			Window_Icon(int icon_id, int coloured_icon_id, const char * help_str, const char * window_name, const std::vector<CommandQueue::Line> *lines = 0)
				: Basic_Icon(icon_id, coloured_icon_id, help_str, lines), window_id(0)
				{ window_id = get_winid(window_name); }
			void update_highlight(void)
			{
				Basic_Icon::update_highlight();
				if ( window_id &&  *window_id >= 0 && (windows_list.window[*window_id].displayed || windows_list.window[*window_id].reinstate) )
					Basic_Icon::set_highlight(true);
			}
			void action(void)
			{
				if (window_id)
					view_window(window_id, 0);
				Basic_Icon::action();
			}
			~Window_Icon(void) {}
		private:
			int *window_id;
	};


	//	Implements keypress icons.
	//
	class Keypress_Icon : public Basic_Icon
	{
		public:
			Keypress_Icon(int icon_id, int coloured_icon_id, const char * help_str, const char * the_key_name, const std::vector<CommandQueue::Line> *lines = 0)
				: Basic_Icon(icon_id, coloured_icon_id, help_str, lines)
			{
				if (the_key_name && (strlen(the_key_name)>0))
					key_name = std::string(the_key_name);
			}
			~Keypress_Icon(void) {}
			void action(void)
			{
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


	//	Implements cursor action mode icons.
	//
	class Actionmode_Icon : public Basic_Icon
	{
		public:
			Actionmode_Icon(int icon_id, int coloured_icon_id, const char * help_str, const char * action_name, const std::vector<CommandQueue::Line> *lines = 0)
				: Basic_Icon(icon_id, coloured_icon_id, help_str, lines), the_action_mode(ACTION_WALK)
			{
				if (action_name && (strlen(action_name) > 0))
					for (size_t i=0; icon_action_modes[i].name!=0; i++)
						if (strcmp(action_name, icon_action_modes[i].name) == 0)
							the_action_mode = icon_action_modes[i].mode;
			}
			~Actionmode_Icon(void) {}
			void update_highlight(void)
			{
				Basic_Icon::update_highlight();
				if (action_mode == the_action_mode)
					Basic_Icon::set_highlight(true);
			}
			void action(void)
			{
				switch_action_mode(&the_action_mode, 0);
				Basic_Icon::action();
			}
		private:
			int the_action_mode;
			typedef struct { const char *name; int mode; } icon_action_mode;
			static const icon_action_mode icon_action_modes[];
	};

	// Add new cursor action modes to this list
	const Actionmode_Icon::icon_action_mode Actionmode_Icon::icon_action_modes[] = {
		{ "walk", ACTION_WALK },
		{ "look", ACTION_LOOK },
		{ "use", ACTION_USE },
		{ "use_witem", ACTION_USE_WITEM },
		{ "trade", ACTION_TRADE },
		{ "attack", ACTION_ATTACK },
		{ 0, 0 } /* needed as terminator */ };


	//	Implements #command icons.
	//
	class Command_Icon : public Basic_Icon
	{
		public:
			Command_Icon(int icon_id, int coloured_icon_id, const char * help_str, const char * command, const std::vector<CommandQueue::Line> *lines = 0)
				: Basic_Icon(icon_id, coloured_icon_id, help_str, lines), command_text(command) {}
			void action(void)
			{
				if (!command_text.empty())
				{
					std::string temp("CommandIcon||");
					temp += command_text;
					CommandQueue::Line temp_line(temp);
					temp_line.action(*get_cq());
				}
				Basic_Icon::action();
			}
			~Command_Icon(void) {}
		private:
			std::string command_text;
	};


	//	A generation and container class for the icons
	//
	class Container
	{
		public:
			Container(void) : mouse_over_icon(-1), display_icon_size(32) {}
			~Container(void) { free_icons(); }
			size_t get_num_icons(void) const { return icon_list.size(); }
			bool empty(void) const { return icon_list.empty(); };
			void mouse_over(int icon_number) { mouse_over_icon = icon_number; }
			void draw_icons(void);
			void default_icons(icon_window_mode icon_mode);
			Virtual_Icon * icon_xml_factory(const xmlNodePtr cur);
			bool read_xml(icon_window_mode icon_mode);
			int get_icon_size(void) const { return display_icon_size; }
			static int cm_generic_handler(window_info *win, int widget_id, int mx, int my, int option)
			{
				Busy dummy;
				IconWindow::Virtual_Icon * the_icon = reinterpret_cast<IconWindow::Virtual_Icon *>(cm_get_data(cm_window_shown()));
				if (the_icon)
					return the_icon->cm_handler(option);
				return 0;
			}
			void action(size_t icon_number)
			{
				Busy dummy;
				if (icon_number < icon_list.size())
					icon_list[icon_number]->action();
			}
			void menu(size_t icon_number)
			{
				Busy dummy;
				if (icon_number < icon_list.size())
					icon_list[icon_number]->menu();
			}
			void free_icons(void)
			{
				if (busy)
				{
					const char *error_str = " : not freeing as doing action";
					std::cerr << __PRETTY_FUNCTION__ << error_str << std::endl;
					LOG_ERROR("%s%s", __PRETTY_FUNCTION__, error_str );
					return;
				}
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
			class Busy { public: Busy(void) { busy = true; } ~Busy(void) { busy = false; } };
			std::vector <Virtual_Icon *> icon_list;
			int mouse_over_icon;
			static bool busy;
			int display_icon_size;
	};

	bool Container::busy = false;

	// Constucture basic icon object
	//
	Basic_Icon::Basic_Icon(int icon_id, int coloured_icon_id, const char * help_str, const std::vector<CommandQueue::Line> *lines)
		: help_message(help_str), cq(0), cm_menu_id(CM_INIT_VALUE)
	{
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
		flashing = 0;
		last_flash_change = 0;

		// construct context menu if we need one
		if (lines)
		{
			std::copy(lines->begin(), lines->end(), back_inserter(menu_lines));
			std::string menu_text;
			for (size_t i=0; i<menu_lines.size(); i++)
				menu_text += menu_lines[i].get_text() + "\n";
			cm_menu_id = cm_create(menu_text.c_str(), Container::cm_generic_handler);
			cm_set_data(cm_menu_id, dynamic_cast<void *>(this));
		}
	}


	//	Return the uv values of the icon bitmap need for drawing
	//	Implements the plain/highlighted icon switch.
	//
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


	//	Draw the icons into the window
	//
	void Container::draw_icons(void)
	{
		Busy dummy;
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
			draw_2d_thing( uv.first, uv.second, uv.first+uoffset, uv.second+voffset, i*get_icon_size(), 0, i*get_icon_size()+(get_icon_size()-1), get_icon_size() );
		}
		glEnd();
		if (show_help_text && (mouse_over_icon >= 0) && ((size_t)mouse_over_icon < icon_list.size()))
			show_help(icon_list[mouse_over_icon]->get_help_message(), get_icon_size()*(mouse_over_icon+1)+2, 10);
		mouse_over_icon = -1;
	}


	// helper function for reading xml strings, perhaps make generally available?
	//
	bool get_xml_field_string(std::string &ret_string, const char *field_name, const xmlNodePtr cur)
	{
		char *tmp = (char*)xmlGetProp(cur, (xmlChar *)field_name);
		if (!tmp)
			return false;
		char *parsed = 0;
		MY_XMLSTRCPY(&parsed, tmp);
		xmlFree(tmp);
		if (!parsed)
			return false;
		ret_string = parsed;
		free(parsed);
		return true;
	}


	// helper function for reading xml ints, perhaps make generally available?
	//
	bool get_xml_field_int(int *ret_int, const char *field_name, const xmlNodePtr cur)
	{
		std::string tmpstr;
		int tmpint;
		get_xml_field_string(tmpstr, field_name, cur);
		if (tmpstr.empty())
			return false;
		std::stringstream ss(tmpstr);
		if( (ss >> tmpint).fail() )
			return false;
		*ret_int = tmpint;
		return true;
	}

	//	Construct a new icon object from the xml information
	//
	Virtual_Icon * Container::icon_xml_factory(const xmlNodePtr cur)
	{
		std::string the_type, help_name, help_text, param_name;
		const char *help_str;
		int image_id = -1, alt_image_id = -1;

		get_xml_field_string(the_type, "type", cur);
		get_xml_field_int(&image_id, "image_id", cur);
		get_xml_field_int(&alt_image_id, "alt_image_id", cur);
		get_xml_field_string(help_name, "help_name", cur);
		get_xml_field_string(help_text, "help_text", cur);
		get_xml_field_string(param_name, "param_name", cur);

		std::vector<CommandQueue::Line> *menu_lines_ptr = 0;
		std::vector<CommandQueue::Line> menu_lines;
		{
			char *text = (char*)(cur->children ? cur->children->content : NULL);
			char *parsed = 0;
			MY_XMLSTRCPY(&parsed, text);
			if (parsed)
			{
    			std::istringstream lines(parsed);
    			std::string line;    
    			while (std::getline(lines, line))
					if (!line.empty())
						menu_lines.push_back(CommandQueue::Line(line));
				free(parsed);
				if (!menu_lines.empty())
					menu_lines_ptr = &menu_lines;
			}
		}

		if (the_type.empty() || (image_id<0) || (alt_image_id<0) ||
			(help_name.empty() && help_text.empty()) || param_name.empty())
		{
			LOG_ERROR("icon window factory: xml field error type=[%s] image_id=[%d] alt_image_id=[%d] help_name=[%s] help_text=[%s] param_name=[%s]\n",
				the_type.c_str(), image_id, alt_image_id, help_name.c_str(), help_text.c_str(), param_name.c_str() );
			return 0;
		}

		if (!help_text.empty())
			help_str = help_text.c_str();
		else
			help_str = get_named_string("tooltips", help_name.c_str());

		if (the_type == "keypress")
			return new Keypress_Icon(image_id, alt_image_id, help_str, param_name.c_str(), menu_lines_ptr);
		else if (the_type == "window")
			return new Window_Icon(image_id, alt_image_id, help_str, param_name.c_str(), menu_lines_ptr);
		else if (the_type == "action_mode")
			return new Actionmode_Icon(image_id, alt_image_id, help_str, param_name.c_str(), menu_lines_ptr);
		else if (the_type == "#command")
			return new Command_Icon(image_id, alt_image_id, help_str, param_name.c_str(), menu_lines_ptr);
		return 0;
	}


	//	Read the icon xml file, constructing icon objects as we go.
	//
	bool Container::read_xml(icon_window_mode icon_mode)
	{
		char const *error_prefix = __PRETTY_FUNCTION__;
		std::string file_name;
		if (icon_mode == NEW_CHARACTER_ICONS)
			file_name = "new_character_icon_window.xml";
		else if (icon_mode == MAIN_WINDOW_ICONS)
			file_name = "main_icon_window.xml";
		else
		{
			LOG_ERROR("%s : invalid icon mode\n", error_prefix );
			return false;
		}

		// shame but xmlFileMatch gives an additional error message
		if (!el_file_exists_anywhere(file_name.c_str()))
			return false;

		xmlDocPtr doc;
		xmlNodePtr cur;

		if ((doc = xmlReadFile(file_name.c_str(), NULL, 0)) == NULL)
		{
			LOG_ERROR("%s : Can't open file [%s]\n", error_prefix, file_name.c_str() );
			return false;
		}

		if ((cur = xmlDocGetRootElement (doc)) == NULL)
		{
			LOG_ERROR("%s : Empty xml document\n", error_prefix );
			xmlFreeDoc(doc);
			return false;
		}

		if (xmlStrcasecmp (cur->name, (const xmlChar *) "icon_window"))
		{
			LOG_ERROR("%s : Not icon_window file\n", error_prefix );
			xmlFreeDoc(doc);
			return false;
		}

		for (cur = cur->xmlChildrenNode; cur; cur = cur->next)
		{
			if (!xmlStrcasecmp(cur->name, (const xmlChar *)"icon"))
			{
				Virtual_Icon *new_icon = icon_xml_factory(cur);
				if (new_icon)
					icon_list.push_back(new_icon);
			}
			else if (!xmlStrcasecmp(cur->name, (const xmlChar *)"multi_icon"))
			{
				std::string control_name;
				get_xml_field_string(control_name, "control_name", cur);
				if (control_name.empty())
					LOG_ERROR("%s : invalid multi icon control_name\n", error_prefix );
				else
				{
					std::vector<Virtual_Icon *> multi_icons;
					for (xmlNodePtr multi_cur = cur->xmlChildrenNode; multi_cur; multi_cur = multi_cur->next)
					{
						if (!xmlStrcasecmp(multi_cur->name, (const xmlChar *)"icon"))
						{
							Virtual_Icon *new_icon = icon_xml_factory(multi_cur);
							if (new_icon)
								multi_icons.push_back(new_icon);
						}
					}
					icon_list.push_back(new Multi_Icon(control_name.c_str(), multi_icons));
				}
			}
			else if (!xmlStrcasecmp(cur->name, (const xmlChar *)"image_settings"))
				get_xml_field_int(&display_icon_size, "display_size", cur);
		}
		xmlFreeDoc(doc);
		return true;
	}


	//	If the xml file is missing or does not pass, fall back to default icons
	//
	void Container::default_icons(icon_window_mode icon_mode)
	{
		if (icon_mode == NEW_CHARACTER_ICONS)
		{
#ifndef NEW_NEW_CHAR_WINDOW
			icon_list.push_back(new Window_Icon(0, 18, get_named_string("tooltips", "name_pass"), "name_pass"));
			icon_list.push_back(new Window_Icon(2, 20, get_named_string("tooltips", "customize"), "customize"));
#endif
			icon_list.push_back(new Window_Icon(39, 38, get_named_string("tooltips", "help"), "help"));
			icon_list.push_back(new Window_Icon(14, 34, get_named_string("tooltips", "opts"), "opts"));
			LOG_ERROR("%s : Using default new character icons\n", __PRETTY_FUNCTION__ );
		}
		else if (icon_mode == MAIN_WINDOW_ICONS)
		{
			icon_list.push_back(new Actionmode_Icon(0, 18, get_named_string("tooltips", "walk"), "walk"));
			std::vector<Virtual_Icon *> sit_stand_icons;
			sit_stand_icons.push_back(new Keypress_Icon(7, 25, get_named_string("tooltips", "sit"), "#K_SIT"));
			sit_stand_icons.push_back(new Keypress_Icon(8, 26, get_named_string("tooltips", "stand"), "#K_SIT"));
			icon_list.push_back(new Multi_Icon("you_sit", sit_stand_icons));
			icon_list.push_back(new Actionmode_Icon(2, 20, get_named_string("tooltips", "look"), "look"));
			icon_list.push_back(new Actionmode_Icon(15, 35, get_named_string("tooltips", "use"), "use"));
			icon_list.push_back(new Actionmode_Icon(47, 46, get_named_string("tooltips", "use_witem"), "use_witem"));
			icon_list.push_back(new Actionmode_Icon(4, 22, get_named_string("tooltips", "trade"), "trade"));
			icon_list.push_back(new Actionmode_Icon(5, 23, get_named_string("tooltips", "attack"), "attack"));
			icon_list.push_back(new Window_Icon(11, 29, get_named_string("tooltips", "invent"), "invent"));
			icon_list.push_back(new Window_Icon(9, 27, get_named_string("tooltips", "spell"), "spell"));
			icon_list.push_back(new Window_Icon(12, 32, get_named_string("tooltips", "manu"), "manu"));
			icon_list.push_back(new Window_Icon(45, 44, get_named_string("tooltips", "emotewin"), "emotewin"));
			icon_list.push_back(new Window_Icon(19, 21, get_named_string("tooltips", "quest"), "quest"));
			icon_list.push_back(new Window_Icon(36, 37, get_named_string("tooltips", "map"), "map"));
			icon_list.push_back(new Window_Icon(3, 6, get_named_string("tooltips", "info"), "info"));
			icon_list.push_back(new Window_Icon(10, 24, get_named_string("tooltips", "buddy"), "buddy"));
			icon_list.push_back(new Window_Icon(13, 33, get_named_string("tooltips", "stats"), "stats"));
			icon_list.push_back(new Window_Icon(1, 28, get_named_string("tooltips", "console"), "console"));
			icon_list.push_back(new Window_Icon(39, 38, get_named_string("tooltips", "help"), "help"));
			icon_list.push_back(new Window_Icon(14, 34, get_named_string("tooltips", "opts"), "opts"));
			LOG_ERROR("%s : Using default main icons\n", __PRETTY_FUNCTION__ );
		}
		else
			LOG_ERROR("%s : Invalid mode for defaul icons\n", __PRETTY_FUNCTION__ );
	}

}


//	The icon container object, does nothing much when constructed, frees the icons when looses scope.
static IconWindow::Container action_icons;

int	icons_win = -1;
static int reload_flag = false;
static icon_window_mode last_mode = (icon_window_mode)0;

//	Window callback for mouse over
static int	mouseover_icons_handler(window_info *win, int mx, int my)
{
	action_icons.mouse_over(mx/action_icons.get_icon_size());
	return 0;
}

//	Window callback for display
static int	display_icons_handler(window_info *win)
{
	if (reload_flag)
	{
		action_icons.free_icons();
		init_icon_window(last_mode);
		reload_flag = false;
	}
	action_icons.draw_icons();
	return 1;
}

//	Window callback mouse click
static int	click_icons_handler(window_info *win, int mx, int my, Uint32 flags)
{
	if ( (flags & ELW_MOUSE_BUTTON) == 0)
		return 0; // only handle mouse button clicks, not scroll wheels moves;
	if (flags & ELW_RIGHT_MOUSE)
		action_icons.menu(mx/action_icons.get_icon_size());
	else if (flags & ELW_LEFT_MOUSE)
		action_icons.action(mx/action_icons.get_icon_size());
	return 1;
}

//	Reload the icons from file
extern "C" int reload_icon_window(char *text, int len)
{
	reload_flag = true;
	return 1;
}

//	Used for hacking the hud stats bar displays, really just returns window width
extern "C" int get_icons_win_active_len(void)
{
	return action_icons.get_icon_size() * action_icons.get_num_icons();
}

//	Make the specified icon flash
extern "C" void flash_icon(const char* name, Uint32 seconds)
{
	action_icons.flash(name, seconds);
}

//	Create/display the icon window and create the icons as needed
extern "C" void init_icon_window(icon_window_mode icon_mode)
{
	if (!action_icons.empty() && (last_mode != icon_mode))
		action_icons.free_icons();
	last_mode = icon_mode;

	if (action_icons.empty())
	{
		action_icons.read_xml(icon_mode);
		if (action_icons.empty())
			action_icons.default_icons(icon_mode);
	}

	if(icons_win < 0)
	{
		icons_win= create_window("Icons", -1, 0, 0, window_height-action_icons.get_icon_size(), window_width-hud_x, action_icons.get_icon_size(), ELW_TITLE_NONE|ELW_SHOW_LAST);
		set_window_handler(icons_win, ELW_HANDLER_DISPLAY, (int (*)())&display_icons_handler);
		set_window_handler(icons_win, ELW_HANDLER_CLICK, (int (*)())&click_icons_handler);
		set_window_handler(icons_win, ELW_HANDLER_MOUSEOVER, (int (*)())&mouseover_icons_handler);
	}
	else
		move_window(icons_win, -1, 0, 0, window_height-action_icons.get_icon_size());

	resize_window(icons_win, action_icons.get_icon_size()*action_icons.get_num_icons(), action_icons.get_icon_size());
}
