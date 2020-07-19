/*
	Display windows showing players achievement icons and text.

	Author bluap/pjbroad (an unusually chilly) November 2010
*/

#include <iostream>
#include <sstream>
#include <vector>
#include <list>

#include "actors.h"
#include "asc.h"
#include "achievements.h"
#include "context_menu.h"
#include "elwindows.h"
#include "errors.h"
#include "elconfig.h"
#include "interface.h"
#include "gamewin.h"
#include "gl_init.h"
#include "hud.h"
#include "init.h"
#include "sound.h"
#include "text.h"
#include "textures.h"
#include "io/elfilewrapper.h"
#include "image_loading.h"

/*
 *	TO DO
 * 		fully comment
 * 		fix issue with mouse over open causing other windows to flicker
 * 		check create window status and warn if out of slots - fix handling elsewhere
*/

using namespace eternal_lands;

//	A class to hold a single achievement object.
//
class Achievement
{
	public:
		Achievement(int achievement_id, int image_id, const char *title, const char *text);
		void show(std::ostream & out) const;
		const ustring& get_text() const { return lines; }
		const std::string & get_title(void) const { return title; }
		size_t get_id(void) const { return image_id; }
		static const size_t npos;
		void prepare(int win_x, float zoom, int border, bool force=false);
		size_t get_num_lines() const { return nr_lines; }
	private:
		ustring text;
		std::string title;
		size_t achievement_id;
		size_t image_id;
		bool prepared;
		ustring lines;
		size_t nr_lines;
};


const size_t Achievement::npos = static_cast<size_t>(-1);


//	Construct a new achievement object
//
Achievement::Achievement(int achievement_id, int image_id, const char *title, const char *text)
 : prepared(false)
{
	this->achievement_id = achievement_id;
	this->image_id = image_id;
	this->title = title;
	this->text = (const unsigned char*)text;
}


//	Show achievement object data
//
void Achievement::show(std::ostream & out) const
{
	out << "ID=" << achievement_id << " IMG=" << image_id;
	out << " Title=[" << title << "] Text=["
		<< reinterpret_cast<const char*>(text.c_str()) << "]";
}


//	Process the main text into lines that fix into the pop-up window
//
void Achievement::prepare(int win_x, float zoom, int border, bool force)
{
	if (!prepared || force)
	{
		TextDrawOptions options = TextDrawOptions().set_max_width(win_x - 2 * border)
			.set_zoom(zoom);
		std::tie(lines, nr_lines) = FontManager::get_instance().reset_soft_breaks(
			FontManager::Category::UI_FONT, text, options);
		prepared = true;
	}
}


//	A single achievement window, handles all the rendering and interation
//
class Achievements_Window
{
	public:
		Achievements_Window(void) : win_mouse_x(-1), win_mouse_y(-1), clicked(false), ctrl_clicked(false), main_win_id(-1),
			child_win_id(-1), logical_rows(0), physical_rows(0), first(0), last_over(Achievement::npos) {}
		~Achievements_Window(void);
		void set_achievements(const std::vector<Uint32> & data);
		void set_name(const std::string & name);
		const std::string & get_name(void) const { return their_name; }
		void open(int win_pos_x, int win_pos_y);
		void open_child(void);
		int display_handler(window_info *win);
		void ui_scale_handler(window_info *win);
		int font_change_handler(window_info *win, FontManager::Category cat);
		bool shown(void) const { return get_show_window(main_win_id); }
		void hide(void) const { hide_window(main_win_id); }
		void set_mouse_over(int mx, int my) { win_mouse_x = mx; win_mouse_y = my; }
		void window_clicked(void) { clicked = true; }
		void window_ctrl_clicked(void)  { ctrl_clicked = true; }
		int get_window_id(void) const { return main_win_id; }
	private:
		std::vector<size_t> their_achievements;
		std::string their_name;
		int win_mouse_x, win_mouse_y;
		bool clicked;
		bool ctrl_clicked;
		int main_win_id, child_win_id;
		int logical_rows, physical_rows;
		size_t first;
		size_t last_over;
};



//	Controlling class, holds collections of achievement objects & windows and
//	the common properties and functions.  Reads the xml file and creates the
//	achievement objects.  Handles requests for new windows, creating the window
//	objects.
//
class Achievements_System
{
	public:
		Achievements_System(void);
		~Achievements_System(void);
		const Achievement * achievement(size_t index) const;
		void prepare_details(size_t index, bool force=false);
		void new_data(const Uint32 *data, size_t word_count);
		void new_name(const char *name, int len);
		void requested(int mouse_pos_x, int mouse_pos_y, int control_used) { win_pos_x = mouse_pos_x; win_pos_y = mouse_pos_y; this->control_used = control_used; }
		static Achievements_System * get_instance(void);
		void show(void) const;
		int texture(size_t index) const;
		bool hide_all(void) const;
		int get_size(void) const { return size; }
		void set_current_scale(float scale) { current_scale = scale; }
		float get_current_scale(void) const { return current_scale; }
		int get_font_width(const std::string& str) const
		{
			return get_string_width_zoom(reinterpret_cast<const unsigned char*>(str.c_str()),
				UI_FONT, current_scale * DEFAULT_SMALL_RATIO);
		}
		int get_font_y() const
		{
			return get_line_height(UI_FONT, current_scale * DEFAULT_SMALL_RATIO);
		}
		int get_display(void) const { return static_cast<int>(0.5 + display * current_scale); }
		int get_y_win_offset(void) const { return static_cast<int>(0.5 + y_win_offset * current_scale); }
		int get_per_row(void) const { return per_row; }
		int get_max_rows(void) const { return max_rows; }
		int get_border(void) const { return static_cast<int>(0.5 + border * current_scale); }
		int main_win_x(void) const { return per_row * get_display() + 3 * get_border(); }
		int get_child_win_x(void) const;
		int get_child_win_y(void) const { return get_font_y() * (1 + max_detail_lines) + 2 * get_border(); }
		const std::string & get_prev(void) const { return prev; }
		const std::string & get_next(void) const { return next; }
		const std::string & get_close(void) const { return close; }
		const char * get_close_help(void) const { return close_help.c_str(); }
		const char * get_prev_help(void) const { return prev_help.c_str(); }
		const char * get_no_prev_help(void) const { return no_prev_help.c_str(); }
		const char * get_next_help(void) const { return next_help.c_str(); }
		const char * get_no_next_help(void) const { return no_next_help.c_str(); }
		void rewrap_achievement_texts()
		{
			max_detail_lines = 2;
			for (size_t i = 0; i < achievements.size(); ++i)
				prepare_details(i, true);
		}
	private:
		void get_int_props(const xmlNodePtr cur, int *props_p[], const char *props_s[], size_t num);
		void get_string_props(const xmlNodePtr cur, std::string * strings_p[], const char* strings[], size_t count);
		std::vector<Achievement *> achievements;
		std::list<Achievements_Window *> windows;
		std::vector<Uint32> last_data;
		std::vector<int> textures;
		int size, display, y_win_offset;
		int per_row, min_rows, max_rows, border;
		std::string prev, next, close;
		std::string too_many, xml_fail, texture_fail;
		std::string close_help, no_next_help, no_prev_help, next_help, prev_help;
		int widest_title_id;
		size_t max_detail_lines;
		size_t max_windows;
		int win_pos_x, win_pos_y;
		bool control_used;
		float current_scale;
};


//	Work out if the specified point of the specified window is on top of the window stack.
//	Method to traverse layers copied from elwindow.c this could probably be a general window function.
//
bool is_window_coord_top(int window_id, int coord_x, int coord_y)
{
	bool have_seen_window = false;
	int id = 0;
	int i;
	while (1)
	{
		int next_id = 9999;
		for (i = 0; i < windows_list.num_windows; i++)
		{
			// only look at displayed windows
			if (windows_list.window[i].displayed > 0)
			{
				// at this level?
				if (windows_list.window[i].order == id)
				{
					if (i == window_id)
						have_seen_window = true;
					else if (have_seen_window)
					{
						if (mouse_in_window(i, coord_x, coord_y))
							return false;
					}
				}
				// try to find the next level
				else if (windows_list.window[i].order > id && windows_list.window[i].order < next_id)
					next_id = windows_list.window[i].order;
			}
		}
		if (next_id >= 9999)
			break;
		else
			id = next_id;
	}
	return true;
}


//	Read the achievements xml file and create all the achievement objects,
//	the global settings and the textures.
//
Achievements_System::Achievements_System(void)
 : size(32), display(32), y_win_offset(10), per_row(5), min_rows(1), max_rows(12), border(2),
	prev("[<]"), next("[>]"), close("[close]"),
	too_many("Too many achievement windows open already"),
	xml_fail("Failed to load achievement data"),
	texture_fail("Failed to load achievement texture"),
	close_help("Close or +ctrl close all"),
	no_next_help("No more pages"),
	no_prev_help("No previous page"),
	next_help("Next page"),
	prev_help("Previous page"),
	widest_title_id(0),
	max_detail_lines(2), max_windows(15), win_pos_x(100), win_pos_y(50), control_used(false), current_scale(1.0)
{
	xmlDocPtr doc;
	xmlNodePtr cur;
	char const *error_prefix = "Reading xml: ";

	std::ostringstream langpath;
	langpath << "languages/" << lang << "/achievements.xml";
	if ((doc = xmlReadFile(langpath.str().c_str(), NULL, 0)) == NULL)
	{
		const char *path = "languages/en/achievements.xml";
		if ((doc = xmlReadFile(path, NULL, 0)) == NULL)
		{
			LOG_ERROR("%sCan't open file [%s]\n", error_prefix, path );
			return;
		}
	}

	if ((cur = xmlDocGetRootElement (doc)) == NULL)
	{
		LOG_ERROR("%sEmpty xml document\n", error_prefix );
		xmlFreeDoc(doc);
		return;
	}

	if (xmlStrcasecmp (cur->name, (const xmlChar *) "achievements_system"))
	{
		LOG_ERROR("%sNot achievements system.\n", error_prefix );
		xmlFreeDoc(doc);
		return;
	}

	int max_title_width = 0;
	for (cur = cur->xmlChildrenNode; cur; cur = cur->next)
	{
		if (!xmlStrcasecmp(cur->name, (const xmlChar *)"achievement"))
		{
			char *text = (char*)(cur->children ? cur->children->content : NULL);
			char *title = (char*)xmlGetProp(cur, (xmlChar *)"title");

			int achievement_id = -1, image_id = -1;
			int *props_p[2] = { &achievement_id, &image_id };
			const char *props_s[2] =  { "achievement_id", "image_id" };
			get_int_props(cur, props_p, props_s, 2);

			if ((text == NULL) || (achievement_id < 0) || (image_id < 0) || (title == NULL))
			{
				LOG_WARNING("%sInvalid achievements node\n", error_prefix );
				continue;
			}

			char *proc_title = 0;
			char *proc_text = 0;
			MY_XMLSTRCPY(&proc_title, title);
			MY_XMLSTRCPY(&proc_text, text);
			xmlFree(title);

			if ((achievement_id < 0) || (achievement_id >= MAX_ACHIEVEMENTS))
				LOG_ERROR("%sInvalid achievement id=%lu\n", error_prefix, achievement_id );
			else
			{
				achievements.resize(achievement_id+1, 0);
				if (achievements[achievement_id])
					LOG_ERROR("%sDuplicate achievement id=%lu\n", error_prefix, achievement_id );
				else
				{
					achievements[achievement_id] = new Achievement(achievement_id, image_id, proc_title, proc_text);
					int title_width = get_font_width(achievements[achievement_id]->get_title());
					if (title_width > max_title_width)
					{
						max_title_width = title_width;
						widest_title_id = achievement_id;
					}
				}
			}

			if (proc_title) free(proc_title);
			if (proc_text) free(proc_text);
		}
		else if (!xmlStrcasecmp(cur->name, (const xmlChar *)"image_settings"))
		{
			int *props_p[2] = { &size, &display };
			const char * props_s[2] =  { "size", "display" };
			get_int_props(cur, props_p, props_s, 2);
		}
		else if (!xmlStrcasecmp(cur->name, (const xmlChar *)"window_settings"))
		{
			int *props_p[4] = { &per_row, &min_rows, &max_rows, &border };
			const char *props_s[4] =  { "per_row", "min_rows", "max_rows", "border" };
			get_int_props(cur, props_p, props_s, 4);
			std::string * ctrl_p[3] = { &prev, &next, &close };
			const char* ctrl_s[3] = { "prev", "next", "close" };
			get_string_props(cur, ctrl_p, ctrl_s, 3);
		}
		else if (!xmlStrcasecmp(cur->name, (const xmlChar *)"strings"))
		{
			const size_t count = 8;
			std::string * strings_p[count] = { &too_many, &xml_fail, &texture_fail,
				&close_help, &no_next_help, &no_prev_help, &next_help, &prev_help };
			const char* strings_s[count] = { "too_many", "xml_fail", "texture_fail",
				"close_help", "no_next_help", "no_prev_help", "next_help", "prev_help" };
			get_string_props(cur, strings_p, strings_s, count);
		}
		else if (!xmlStrcasecmp(cur->name, (const xmlChar *)"texture"))
		{
			char *path = (char*)(cur->children ? cur->children->content : NULL);
			char buffer[1024];

			if (check_image_name(path, sizeof(buffer), buffer) == 1)
			{
				textures.push_back(load_texture_cached(buffer, tt_gui));
			}
		}
	}
	xmlFreeDoc(doc);
}


//	Destroy all the achievement objects and windows.
//
Achievements_System::~Achievements_System(void)
{
	for (std::list<Achievements_Window *>::iterator i = windows.begin(); i!= windows.end(); ++i)
		delete *i;
	for (std::vector<Achievement *>::iterator i = achievements.begin(); i!= achievements.end(); ++i)
		delete *i;
}


//	Return an achievement object.
//
const Achievement * Achievements_System::achievement(size_t index) const
{
	if (index < achievements.size())
		return achievements[index];
	else
		return 0;
}


//	Return a texture.
//
int Achievements_System::texture(size_t index) const
{
	if (index<textures.size())
		return textures[index];
	else
		return -1;
}


//	Return the width of the popup window
int Achievements_System::get_child_win_x(void) const
{
	int proposed = get_font_width(achievements[widest_title_id]->get_title()) + 2 * get_border();
	return std::max(proposed, main_win_x());
}


//	Save the data for the next window
//
void Achievements_System::new_data(const Uint32 *data, size_t word_count)
{
	last_data.resize(word_count);
	for (size_t i=0; i<word_count; ++i)
		last_data[i] = data[i];
}


//	Now we have the data and a name, create a new window.
//
void Achievements_System::new_name(const char *player_name, int len)
{
	if (achievements.empty())
	{
		LOG_TO_CONSOLE(c_red1, xml_fail.c_str());
		return;
	}
	if (textures.empty())
	{
		LOG_TO_CONSOLE(c_red1, texture_fail.c_str());
		return;
	}

	if (achievements_ctrl_click && !control_used)
	{
		last_data.clear();
		return;
	}

	std::string name;
	if (player_name && (len > 0))
	{
		char *tmp = new char[len+1];
		safe_strncpy2(tmp, player_name, len+1, len);
		name = tmp;

		// check for already open window for player and delete it
		for (std::list<Achievements_Window *>::iterator i = windows.begin(); i!= windows.end(); ++i)
			if (*i && (*i)->get_name() == name)
			{
				delete *i;
				*i = 0;
				break;
			}

		delete [] tmp;
	}

	// remove any closed windows
	for (std::list<Achievements_Window *>::iterator i = windows.begin(); i!= windows.end(); ++i)
		if (*i && !(*i)->shown())
		{
			delete *i;
			*i = 0;
		}
	windows.remove(0);

	if (last_data.empty())
		return;

	// limit the number of windows so that we don't run out for everyone else
	if (windows.size() < max_windows)
	{
		windows.push_back(new Achievements_Window);
		windows.back()->set_achievements(last_data);
		windows.back()->set_name(name);
		windows.back()->open(win_pos_x, win_pos_y);
	}
	else
		LOG_TO_CONSOLE(c_red1, too_many.c_str());

	last_data.clear();
}


//	Ctrl-click close on any window closes them all, hides now but destroys later
//
bool Achievements_System::hide_all(void) const
{
	bool return_value = false;
	for (std::list<Achievements_Window *>::const_iterator i = windows.begin(); i!= windows.end(); ++i)
	{
		if ((*i)->shown())
			return_value = true;
		(*i)->hide();
	}
	return return_value;
}


//	Show global properties read form the xml file.
//
void Achievements_System::show(void) const
{
	std::cout << "image props size=" << size << " display=" << get_display() << std::endl;
	std::cout << "window props per_row=" << per_row << " min_rows=" << min_rows << " max_rows=" << max_rows << " border=" << get_border() << std::endl;
	std::cout << "window strings prev=" << prev << " next=" << next << " close=" << close << std::endl;
}


//	The evil singelton mechanism.
//
Achievements_System * Achievements_System::get_instance(void)
{
	static Achievements_System as;
	static SDL_threadID creation_thread = SDL_ThreadID();
	if (SDL_ThreadID() != creation_thread)
		std::cerr << __FUNCTION__ << ": Achievements system called by non-creator thread." << std::endl;
	return &as;
}


//	A helper function for reading xml integer properties
//
void Achievements_System::get_int_props(const xmlNodePtr cur, int *props_p[], const char *props_s[], size_t num)
{
	for (size_t i=0; i<num; ++i)
	{
		char *prop = (char*)xmlGetProp(cur, (xmlChar *)props_s[i]);
		if (prop == NULL)
			continue;
		int val=atoi(prop);
		if (val>=0)
			*props_p[i] = val;
		xmlFree(prop);
	}
}


//	A helper function for reading xml string properties
//
void Achievements_System::get_string_props(const xmlNodePtr cur, std::string * strings_p[], const char* strings[], size_t count)
{
	for (size_t i=0; i<count; ++i)
	{
		char *tmp = (char*)xmlGetProp(cur, (xmlChar *)strings[i]);
		char *parsed = 0;
		if (!tmp)
			continue;
		MY_XMLSTRCPY(&parsed, tmp);
		if (parsed)
		{
			*(strings_p[i]) = parsed;
			free(parsed);
		}
		xmlFree(tmp);
	}
}


//	First time we're asked to display some achievement details, prepare for the window size etc.
//
void Achievements_System::prepare_details(size_t index, bool force)
{
	if ((index >= achievements.size()) || !achievements[index])
		return;
	achievements[index]->prepare(get_child_win_x(), get_current_scale() * DEFAULT_SMALL_RATIO,
		get_border(), force);
	if (achievements[index]->get_num_lines() > max_detail_lines)
		max_detail_lines = achievements[index]->get_num_lines();
}


//	Destory achievement windows, done when hidden and garbage collect next time we create one.
//
Achievements_Window::~Achievements_Window(void)
{
	if (main_win_id >= 0)
		destroy_window(main_win_id);
	if (child_win_id >= 0)
		destroy_window(child_win_id);
}


//	When the mouse is over an achievement, display the details window.
//
static int achievements_child_display_handler(window_info *win)
{
	if (!win || !win->data)
		return 0;
	size_t index = *reinterpret_cast<size_t *>(win->data);
	Achievements_System *as = Achievements_System::get_instance();

	as->prepare_details(index);

	const Achievement * achievement = as->achievement(index);
	if (achievement)
	{
		glColor3f(0.77f, 0.57f, 0.39f);
		draw_string_small_zoomed_centered(win->len_x/2, as->get_border(),
			reinterpret_cast<const unsigned char *>(achievement->get_title().c_str()), 1, as->get_current_scale());

		glColor3f(1.0f, 1.0f, 1.0f);
		draw_string_small_zoomed(as->get_border(), as->get_font_y(),
			achievement->get_text().c_str(), achievement->get_num_lines(), as->get_current_scale());
	}
	else
	{
		glColor3f(0.77f, 0.57f, 0.39f);
		std::ostringstream buf;
		buf << "Undefined " << index;
		draw_string_small_zoomed_centered(win->len_x/2, as->get_border(),
			reinterpret_cast<const unsigned char *>(buf.str().c_str()), 1, as->get_current_scale());
	}

	return 1;
}


//	When the mouse is over an achievement, create a window to display the details.
//
void Achievements_Window::open_child(void)
{
	window_info *parent = &windows_list.window[main_win_id];
	if (child_win_id < 0)
	{
		child_win_id = create_window("child", parent->window_id, 0, 0, 0, 0, 0,
			ELW_USE_BACKGROUND|ELW_USE_BORDER|ELW_SHOW|ELW_ALPHA_BORDER|ELW_SWITCHABLE_OPAQUE);
		set_window_custom_scale(child_win_id, MW_ACHIEVE);
		set_window_handler(child_win_id, ELW_HANDLER_DISPLAY, (int (*)())&achievements_child_display_handler );
	}
	else
		show_window(child_win_id);

	Achievements_System *as = Achievements_System::get_instance();
	window_info *win = &windows_list.window[child_win_id];
	win->data = reinterpret_cast<void *>(&last_over);
	win->opaque = parent->opaque;
	resize_window(win->window_id, as->get_child_win_x(), as->get_child_win_y());
	move_window(win->window_id, parent->window_id, 0,
		parent->pos_x + (parent->len_x - as->get_child_win_x()) / 2, parent->pos_y  + parent->len_y + as->get_y_win_offset());
}


//	Display the main window.  All the players achievemnt icons and the controls.
//	Handle the mouse over events for the icons and the controls.
//
int Achievements_Window::display_handler(window_info *win)
{
	Achievements_System *as = Achievements_System::get_instance();

	int icon_per = (256 / as->get_size());
	int icon_per_texture = icon_per * icon_per;
	bool another_page = false;

	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0f,1.0f,1.0f);

	for (size_t i=first; i<their_achievements.size(); ++i)
	{
		size_t shown_num = i-first;

		if ((static_cast<int>(shown_num)/as->get_per_row()) >= physical_rows)
		{
			another_page = true;
			break;
		}

		int texture = -1;
		const Achievement * achievement = as->achievement(their_achievements[i]);
		if (achievement)
			texture = as->texture(achievement->get_id() / icon_per_texture);

		if (texture >= 0)
		{
			int cur_item = achievement->get_id() % icon_per_texture;

			float u_start = 1.0f/static_cast<float>(icon_per) * (cur_item % icon_per);
			float u_end = u_start + static_cast<float>(as->get_size())/256;
			float v_start = 1.0f/static_cast<float>(icon_per) * (cur_item / icon_per);
			float v_end = v_start + static_cast<float>(as->get_size()) / 256;
			int start_x = as->get_border() + as->get_display() * (shown_num % as->get_per_row());
			int start_y = as->get_border() + as->get_display() * (shown_num / as->get_per_row());

			bind_texture(texture);
			glEnable(GL_ALPHA_TEST);
			glAlphaFunc(GL_GREATER, 0.05f);
			glBegin(GL_QUADS);
			draw_2d_thing( u_start, v_start, u_end, v_end,
				start_x, start_y, start_x+as->get_display(), start_y+as->get_display() );
			glEnd();
			glDisable(GL_ALPHA_TEST);
		}
		else
		{
			int pos_x = as->get_border() + (shown_num % as->get_per_row()) * as->get_display()
				+ as->get_display()/2;
			int pos_y = as->get_border() + (shown_num / as->get_per_row()) * as->get_display()
				+ (as->get_display() - win->default_font_len_y) / 2;
			draw_string_zoomed_centered(pos_x, pos_y, (const unsigned char *)"?", 1, win->current_scale);
		}
	}

	size_t now_over = Achievement::npos;

	if ((cm_window_shown() == CM_INIT_VALUE) && (win_mouse_x > as->get_border()) && (win_mouse_y > as->get_border()))
	{
		int row = (win_mouse_y - as->get_border()) / as->get_display();
		int col = (win_mouse_x - as->get_border()) / as->get_display();
		if ((row >= 0) && (row < physical_rows) && (col >= 0) && (col < as->get_per_row()))
		{
			size_t current_index = first + row * as->get_per_row() + col;
			if (current_index < their_achievements.size())
				now_over = their_achievements[current_index];
		}
	}

	if (now_over != last_over)
	{
		if (last_over != Achievement::npos)
			hide_window(child_win_id);
		last_over = now_over;
		if (last_over != Achievement::npos)
			open_child();
	}

	int prev_start = as->get_border();
	int prev_end = prev_start + as->get_font_width(as->get_prev());
	int next_start = prev_end + 2 * as->get_border();
	int next_end = next_start + as->get_font_width(as->get_next());
	int close_start = win->len_x
		- (as->get_border() + as->get_font_width(as->get_close()));
	int close_end = win->len_x - as->get_border();

	bool over_controls = (win_mouse_y > (win->len_y - (as->get_font_y() + as->get_border())));
	bool over_close = (over_controls && (win_mouse_x > close_start) && (win_mouse_x < close_end));
	bool over_prev = (over_controls && (win_mouse_x > prev_start) && (win_mouse_x < prev_end));
	bool over_next = (over_controls && (win_mouse_x > next_start) && (win_mouse_x < next_end));

	float active_colour[3] = { 1.0f, 1.0f, 1.0f };
	float inactive_colour[3] =  { 0.5f, 0.5f, 0.5f };
	float mouse_over_colour[3] = { 1.0f, 0.5f, 0.0f };

	glColor3fv((first) ?((over_prev) ?mouse_over_colour :active_colour) :inactive_colour);
	draw_string_small_zoomed(prev_start, win->len_y - (as->get_font_y() + as->get_border()),
		reinterpret_cast<const unsigned char *>(as->get_prev().c_str()), 1, as->get_current_scale());

	glColor3fv((another_page) ?((over_next) ?mouse_over_colour :active_colour) :inactive_colour);
	draw_string_small_zoomed(next_start, win->len_y - (as->get_font_y() + as->get_border()),
		reinterpret_cast<const unsigned char *>(as->get_next().c_str()), 1, as->get_current_scale());

	glColor3fv((over_close) ?mouse_over_colour :active_colour);
	draw_string_small_zoomed(close_start, win->len_y - (as->get_font_y() + as->get_border()),
		reinterpret_cast<const unsigned char *>(as->get_close().c_str()), 1, as->get_current_scale());

	if (over_close && ctrl_clicked)
		as->hide_all();
	if (over_close && clicked)
		hide_window(main_win_id);
	else if (over_prev && first && clicked)
		first -= physical_rows * as->get_per_row();
	else if (over_next && another_page && clicked)
		first += physical_rows * as->get_per_row();

	if (clicked && (over_prev || over_next))
		do_click_sound();
	if ((ctrl_clicked || clicked) && over_close)
		do_window_close_sound();

	if (over_controls && show_help_text)
	{
		if (over_close)
			show_help(as->get_close_help(), 0, win->len_y + as->get_y_win_offset(), win->current_scale);
		else if (over_prev)
			show_help((first)?as->get_prev_help() :as->get_no_prev_help(), 0, win->len_y + as->get_y_win_offset(), win->current_scale);
		else if (over_next)
			show_help((another_page)?as->get_next_help() :as->get_no_next_help(), 0, win->len_y + as->get_y_win_offset(), win->current_scale);
	}

	win_mouse_x = win_mouse_y = -1;
	ctrl_clicked = clicked = false;

	return 1;
}


//	A common display handler callback that used my all the windows and calls the specific handler.
//
static int achievements_display_handler(window_info *win)
{
	if (!win || !win->data)
		return 0;
	Achievements_Window *object = reinterpret_cast<Achievements_Window *>(win->data);
	return object->display_handler(win);
}


//	A common display handler callback for mouse over activity.
//
static int achievements_mouseover_handler(window_info *win, int mx, int my)
{
	if (!win || !win->data)
		return 0;
	if (!is_window_coord_top(win->window_id, mouse_x, mouse_y))
		return 0;
	Achievements_Window *object = reinterpret_cast<Achievements_Window *>(win->data);
	object->set_mouse_over(mx, my);
	return 0;
}


//	A common display handler callback for keypress activity.
//
int achievements_keypress_handler(window_info *win, int mx, int my, SDL_Keycode key_code, Uint32 key_unicode, Uint16 key_mod)
{
	if (!win)
		return 0;
	if (key_code == SDLK_ESCAPE) // close window if Escape pressed
	{
		do_window_close_sound();
		if (key_mod & KMOD_CTRL)
			Achievements_System::get_instance()->hide_all();
		else
			hide_window(win->window_id);
		return 1;
	}
	return 0;
}


//	A common display handler callback for mouse click activity.
//
static int achievements_click_handler(window_info *win, int mx, int my, Uint32 flags)
{
	if (!win || !win->data)
		return 0;
	if (my < 0)
		return 0;
	Achievements_Window *object = reinterpret_cast<Achievements_Window *>(win->data);
	if ((flags & ELW_LEFT_MOUSE) && (flags & KMOD_CTRL))
		object->window_ctrl_clicked();
	else if (flags & ELW_LEFT_MOUSE)
		object->window_clicked();
	return 1;
}


//	Called when UI scaling changes
void Achievements_Window::ui_scale_handler(window_info *win)
{
	Achievements_System *as = Achievements_System::get_instance();
	as->set_current_scale(win->current_scale);
	resize_window(win->window_id, as->main_win_x(), physical_rows * as->get_display() + as->get_font_y() + 2 * as->get_border());
}
static int achievements_ui_scale_handler(window_info *win)
{
	Achievements_Window *object = reinterpret_cast<Achievements_Window *>(win->data);
	object->ui_scale_handler(win);
	return 1;
}

int Achievements_Window::font_change_handler(window_info *win, FontManager::Category cat)
{
	if (cat != FontManager::Category::UI_FONT)
		return 0;

	Achievements_System *as = Achievements_System::get_instance();
	resize_window(win->window_id, as->main_win_x(), physical_rows * as->get_display() + as->get_font_y() + 2 * as->get_border());
	as->rewrap_achievement_texts();

	return 1;
}
static int change_achievements_font_handler(window_info *win, font_cat cat)
{
	Achievements_Window *object = reinterpret_cast<Achievements_Window*>(win->data);
	return object->font_change_handler(win, cat);
}

//	When creating a new window, strip any guild from the playe name.
//
void Achievements_Window::set_name(const std::string & name)
{
	if (name.empty())
		their_name = "<?>";
	else
	{
		their_name = name;
		size_t space_pos = their_name.find(" ");
		if (space_pos != std::string::npos)
			their_name = their_name.substr(0, space_pos);
	}
}


//	Process the achievement bits and save ids for later display.
//
void Achievements_Window::set_achievements(const std::vector<Uint32> & data)
{
	// bit position w1(lsb) ....w5(msb) map to achievement id 0...159
	for (size_t i=0; i<data.size(); ++i)
	{
		Uint32 word = data[i];
		for (size_t j=0; j<sizeof(Uint32)*8; ++j)
		{
			if (word & 1)
				their_achievements.push_back(i*sizeof(Uint32)*8+j);
			word >>= 1;
		}
	}
}


//	Create a new achievement window.
//
void Achievements_Window::open(int win_pos_x, int win_pos_y)
{
	if (main_win_id >= 0)
	{
		show_window(main_win_id);
		return;
	}

	Achievements_System *as = Achievements_System::get_instance();

	logical_rows = (their_achievements.size() + (as->get_per_row() - 1)) / as->get_per_row();
	physical_rows = (logical_rows > as->get_max_rows()) ?as->get_max_rows() :logical_rows;

	main_win_id = create_window(their_name.c_str(), -1, 0, win_pos_x, win_pos_y, 0, 0,
		ELW_USE_UISCALE|ELW_TITLE_BAR|ELW_DRAGGABLE|ELW_USE_BACKGROUND|ELW_USE_BORDER|ELW_SHOW|ELW_TITLE_NAME|ELW_ALPHA_BORDER|ELW_SWITCHABLE_OPAQUE);
	set_window_custom_scale(main_win_id, MW_ACHIEVE);
	set_window_handler(main_win_id, ELW_HANDLER_DISPLAY, (int (*)())&achievements_display_handler );
	set_window_handler(main_win_id, ELW_HANDLER_CLICK, (int (*)())&achievements_click_handler );
	set_window_handler(main_win_id, ELW_HANDLER_MOUSEOVER, (int (*)())&achievements_mouseover_handler );
	set_window_handler(main_win_id, ELW_HANDLER_KEYPRESS, (int (*)())&achievements_keypress_handler );
	set_window_handler(main_win_id, ELW_HANDLER_UI_SCALE, (int (*)())&achievements_ui_scale_handler );
	set_window_handler(main_win_id, ELW_HANDLER_FONT_CHANGE, (int (*)())&change_achievements_font_handler);

	window_info *win = &windows_list.window[main_win_id];
	win->data = reinterpret_cast<void *>(this);
	ui_scale_handler(win);
}

int achievements_ctrl_click = 0;

//	We have a new player name from the server.
//
extern "C" void achievements_player_name(const char *name, int len)
{
	Achievements_System::get_instance()->new_name(name, len);
}


//	We have new achievement data from the server.
//
extern "C" void achievements_data(Uint32 *data, size_t word_count)
{
	Achievements_System::get_instance()->new_data(data, word_count);
}


//	We may be about to get some achievement data, remember the mouse position.
//
extern "C" void achievements_requested(int mouse_pos_x, int mouse_pos_y, int control_used)
{
	Achievements_System::get_instance()->requested(mouse_pos_x, mouse_pos_y, control_used);
}


//	Close any open windows, they will get destroyed later.
//	If no windows are open return 0, otherwise return 1.
//
extern "C" int achievements_close_all(void)
{
	return (Achievements_System::get_instance()->hide_all()) ?1: 0;
}
