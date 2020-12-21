/*
	Rewrite of quest log with new context menu features.

	Author bluap/pjbroad Feb 2010
*/

#include <numeric>
#include <string>
#include <vector>
#include <map>
#include <queue>
#include <set>
#include <cctype>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>

#include "asc.h"
#include "context_menu.h"
#include "dialogues.h"
#include "elconfig.h"
#include "elwindows.h"
#include "errors.h"
#include "gamewin.h"
#include "gl_init.h"
#include "hud.h"
#include "icon_window.h"
#include "init.h"
#include "loginwin.h"
#include "io/elpathwrapper.h"
#ifdef JSON_FILES
#include "json_io.h"
#endif
#include "multiplayer.h"
#include "notepad.h"
#include "paste.h"
#include "questlog.h"
#include "sound.h"
#include "translate.h"

/*
 * TODO Possible changes...
 * 		Make questlog file xml?
 * 		Remove save options - always save / timed save?
 * 		Replace entry containers with classes
 * 		Quest lists
 * 			Space title retrieve by a few seconds
 * 			Context menu
 * 				refresh list?
 * 				refresh highlighted title string?
 * 				copy highlighted title to clipboard
 * 				delete highlighted
 * 		Feature to set the quest index for quest entries
 * 			output md5sums with quest index
 * 			import md5sums with quest index and set matches
 */

using namespace eternal_lands;

//	An individual quest.
//
class Quest
{
	public:
		Quest(Uint16 id) : is_completed(false), title("") { this->id = id; }
		Quest(const std::string & line);
		const std::string &get_title(void) const { return title; }
		void set_title(const char* new_title) { title = new_title; }
		bool get_completed(void) const { return is_completed; }
		void set_completed(bool yes_complete) { is_completed = yes_complete; }
		Uint16 get_id(void) const { return id; }
		void write(std::ostream & out) const
			{ out << id << " " << is_completed << " " << title << std::endl; }
		static const Uint16 UNSET_ID;
	private:
		Uint16 id;
		bool is_completed;
		std::string title;
};

const Uint16 Quest::UNSET_ID = static_cast<Uint16>(-1);


//	Construct a quest object from a string - probably read from a file.
//
Quest::Quest(const std::string & line)
{
	std::istringstream ss(line);
	id = Quest::UNSET_ID;
	is_completed = false;
	ss >> id;
	ss >> is_completed;
	getline(ss, title);
	// trim any leading or trailing space from title
	std::string::size_type start = title.find_first_not_of(' ');
	std::string::size_type end = title.find_last_not_of(' ');
	if (start == std::string::npos)
		title = "";
	else
		title = title.substr(start,end-start+1);
}


//	Used to track requests for a quest title from the server
//
class Quest_Title_Request
{
	public:
		Quest_Title_Request(Uint16 req_id) : id(req_id), requested(false) {}
		Uint16 get_id(void) const { return id; }
		void request(void);
		bool been_requested(void) const { return requested; }
		bool is_too_old(void) const { return ((SDL_GetTicks() - request_time) > 5000); }
	private:
		Uint16 id;
		Uint32 request_time;
		bool requested;
};


//	A single entry for the questlog.
class Quest_Entry
{
	public:
		static int content_width;
		static float content_zoom;

		Quest_Entry(void) : deleted(false), quest_id(Quest::UNSET_ID), charsum(0) {}
		void set(const ustring& the_text);
		void set(const ustring& the_text, const ustring& the_npc);
		const std::vector<ustring> & get_lines(void) const;
		void save(std::ofstream & out) const;
		bool contains_string(const char *text_to_find) const;
		const ustring & get_npc(void) const { return npc; }
		Uint16 get_charsum(void) const { return charsum; }
		void set_id(Uint16 id) { quest_id = id; }
		Uint16 get_id(void) const { return quest_id; }
		void set_deleted(bool is_deleted) { deleted = is_deleted; update_displayed_npc_name(); }
		bool get_deleted(void) const { return deleted; }
		const ustring & get_disp_npc(void) const { return disp_npc; };

		/*!
		 * Recalculate the line breaks
		 *
		 * Recalculate the line breaks in the text for this entry after a font
		 * change or a change in the UI scale.
		 */
		void rewrap_lines();
private:
		static const unsigned char NPC_NAME_COLOUR;
		static const std::vector<ustring> deleted_line;
		static const ustring npc_spacer;

		bool deleted;
		std::vector<ustring> _lines;
		ustring npc;
		ustring disp_npc;
		Uint16 quest_id;
		Uint16 charsum;

		void set_lines(const ustring& the_text);
		void update_displayed_npc_name();
};

const unsigned char Quest_Entry::NPC_NAME_COLOUR = to_color_char(c_blue2);
const std::vector<ustring> Quest_Entry::deleted_line(1,
	to_color_char(c_grey2) + ustring(reinterpret_cast<const unsigned char*>(questlog_deleted_str)));
const ustring Quest_Entry::npc_spacer = reinterpret_cast<const unsigned char*>(": ");
// Will be reset from Questlog_Window::ui_scale_handler
int Quest_Entry::content_width = window_width;
float Quest_Entry::content_zoom = 1.0;

//	Ask the server for the title this quest.
//
void Quest_Title_Request::request(void)
{
	if (requested)
		return;
	Uint8 str[10];
	//char buf [80];
	//safe_snprintf(buf, 80, "Sending WHAT_QUEST_IS_THIS_ID with id=%d", id);
	//LOG_TO_CONSOLE(c_green2,buf);
	str[0]=WHAT_QUEST_IS_THIS_ID;
	*((Uint16 *)(str+1)) = SDL_SwapLE16((Uint16)id);
	my_tcp_send (my_socket, str, 3);
	request_time = SDL_GetTicks();
	requested = true;
}


//	Needed to sort quests with "Show all" first.
//
class QuestCompare {
	public:
		bool operator()(const Quest& x, const Quest& y) const
		{
			if (x.get_id() == Quest::UNSET_ID)
				return true;
			else if (y.get_id() == Quest::UNSET_ID)
				return false;
			else
				return x.get_id() < y.get_id();
		}
};


//	Class to hold and tests position information for a shown entry.
//
class Shown_Entry
{
	public:
		Shown_Entry(std::vector<std::string>::size_type entry_m, int start_y_m, int end_y_m)
			: entry(entry_m), start_y(start_y_m), end_y(end_y_m) {}
		std::vector<std::string>::size_type get_entry(void) const { return entry; }
		bool is_over(int pos_y) const { return ((pos_y > start_y) && (pos_y < end_y)); }
	private:
		std::vector<std::string>::size_type entry;
		int start_y, end_y;
};


static std::vector<Quest_Entry> quest_entries;
static std::vector<size_t> active_entries;
static std::set<size_t> selected_entries;
static std::vector<Shown_Entry> shown_entries;
static enum { QLFLT_NONE=0, QLFLT_QUEST, QLFLT_NPC, QLFLT_SEL } active_filter = QLFLT_NONE;


//	A list of Quests
//
class Quest_List
{
	public:
		Quest_List(void) : save_needed(false), iter_set(false), max_title(0),
			selected_id(Quest::UNSET_ID), highlighted_id(Quest::UNSET_ID),
			win_id(-1), scroll_id(0), mouseover_y(-1), clicked(false), cm_id(CM_INIT_VALUE),
			no_auto_open(0), hide_completed(0), list_left_of_entries(0), quest_completed(0), number_shown(0),
			spacer(0), linesep(0), next_entry_quest_id(Quest::UNSET_ID) {}
		void add(Uint16 id);
		void set_requested_title(const char* title);
		void load(void);
		void save(void);
		void set_completed(Uint16 id, bool is_complete);
		void set_selected(Uint16 id) { selected_id = id; }
		Uint16 get_selected(void) const { return selected_id; }
		void open_window(void);
		int get_win_id(void) const { return win_id; }
		void scroll_to_selected(void);
		void set_mouseover_y(int val) { mouseover_y = val; }
		void recalc_num_shown(void);
		unsigned int get_options(void) const;
		void set_options(unsigned int options);
#ifdef JSON_FILES
		void write_options(const char *dict_name) const;
		void read_options(const char *dict_name);
#endif
		void set_highlighted(Uint16 id) { highlighted_id = id; }
		Uint16 get_highlighted(void) const { return highlighted_id; }
		void clear_highlighted(void) { set_highlighted(Quest::UNSET_ID); }
		void cm_pre_show_handler(void);
		bool cm_active(void) const { return ((cm_id != CM_INIT_VALUE) && (cm_window_shown() == cm_id)); }
		void check_title_requests(void);
		void ui_scale_handler(window_info *win);
		int font_change_handler(window_info *win, FontManager::Category cat);
		void check_auto_open(void) { if (!no_auto_open && !get_show_window(get_win_id())) open_window(); }
		void display_handler(window_info *win);
		void click_handler(window_info *win, Uint32 flags);
		void cm_handler(int option);
		void resize_handler(window_info *win);
		Uint16 get_next_id(void) const { return next_entry_quest_id; }
		void set_next_id(Uint16 id) { next_entry_quest_id = id; }
		void clear_next_id(void) { next_entry_quest_id = Quest::UNSET_ID; }
		int waiting_for_entry(void) { return (next_entry_quest_id != Quest::UNSET_ID) ?1 :0; }
	private:
		static const std::string _empty_title_replacement;

		int get_scroll_id(void) const  { return scroll_id; }
		void showall(void);
		bool get_completed(Uint16 id) const;
		void toggle_completed(Uint16 id) { set_completed(id, !get_completed(id)); }
		size_t num_shown(void) const { return number_shown; }
		const Quest * get_first_quest(int offset);
		const Quest * get_next_quest(void);
		size_t get_max_title(void) const { return max_title; }
		/*!
		 * Return the maximum width of a quest title when drawn in the current font
		 * with scale \a zoom.
		 */
		int get_max_title_width(float zoom) const;
		int get_mouseover_y(void) const { return mouseover_y; }
		bool has_mouseover(void) const { return mouseover_y != -1; }
		void clear_mouseover(void) { mouseover_y = -1; }
		bool was_clicked(void) const { return clicked; }
		bool set_clicked(bool val) { return clicked = val; }
		std::map <Uint16,Quest,QuestCompare> quests;
		std::queue <Quest_Title_Request> title_requests;
		bool save_needed;
		std::string list_filename;
		std::map <Uint16,Quest,QuestCompare>::const_iterator iter;
		bool iter_set;
		size_t max_title;
		Uint16 selected_id;
		Uint16 highlighted_id;
		int win_id;
		int scroll_id;
		int mouseover_y;
		bool clicked;
		size_t cm_id;
		// use logical sense so zero/false value is off
		int no_auto_open;
		int hide_completed;
		int list_left_of_entries;
		int quest_completed;
		size_t number_shown;
		int spacer;
		int linesep;
		Uint16 next_entry_quest_id;
		enum {	CMQL_COMPLETED=0, CMQL_ADDSEL, CMQL_S11, CMQL_HIDECOMPLETED, CMQL_NOAUTOOPEN,  CMQL_LISTLEFTOFENTRIES };
};

const std::string Quest_List::_empty_title_replacement = "???";

static Quest_List questlist;

static int display_questlist_handler(window_info *win)
	{ questlist.display_handler(win); return 1; }
static int click_questlist_handler(window_info *win, int mx, int my, Uint32 flags)
	{ questlist.click_handler(win, flags); return 1; }
static int resize_questlist_handler(window_info *win, int new_width, int new_height)
	{ questlist.resize_handler(win); return 0; }
static int mouseover_questlist_handler(window_info *win, int mx, int my)
	{ if ((my>=0) && (mx<win->len_x-win->box_size)) questlist.set_mouseover_y(my); return 0; }
static int ui_scale_questlist_handler(window_info *win)
	{ questlist.ui_scale_handler(win); return 1; }
static int change_questlist_font_handler(window_info *win, font_cat cat)
	{ return questlist.font_change_handler(win, cat); }
static int cm_questlist_handler(window_info *win, int widget_id, int mx, int my, int option)
	{ questlist.cm_handler(option); return 1; }
static void cm_questlist_pre_show_handler(window_info *win, int widget_id, int mx, int my, window_info *cm_win)
	{ questlist.cm_pre_show_handler(); }


//	Container class for quest log
//
class Questlog_Container
{
	public:
		Questlog_Container(void) : need_to_save(false) {}
		void add_entry(const unsigned char *t, int len);
		void load(void);
		void save(void);
		void show_all_entries(void);
		void rebuild_active_entries(size_t desired_top_entry);
		void set_save(void) { need_to_save = true; }
		bool save_needed(void) const { return need_to_save; }
	private:
		void add_line(const ustring& t, const unsigned char *npcprefix);
		std::string filename;
		bool need_to_save;
};

static Questlog_Container questlog_container;


//	Class for main questlog window
//
class Questlog_Window
{
	public:
		Questlog_Window(void) : qlwinwidth(0), qlwinheight(0), qlborder(0), spacer(0), win_space(0), linesep(0),
		mouse_over_questlog(false), quest_scroll_id(14), current_action(-1), prompt_for_add_text(false),
		adding_insert_pos(0), cm_questlog_id(CM_INIT_VALUE), cm_questlog_over_entry(static_cast<size_t>(-1)),
		current_line(0) {}
		void open(void);
		int display_handler(window_info *win);
		int click_handler(window_info *win, Uint32 flags);
		int keypress_handler(window_info *win, SDL_Keycode key_code, Uint32 key_unicode, Uint16 key_mod);
		void mouseover_handler(int my);
		void ui_scale_handler(window_info *win);
		int font_change_handler(window_info *win, FontManager::Category cat);
		void scroll_click_handler(widget_list *widget);
		void scroll_drag_handler(widget_list *widget);
		void cm_handler(window_info *win, int my, int option);
		void cm_preshow_handler(int my);
		int get_win_space(void) const { return win_space; }
		void update_scrollbar_len(void)
			{ if (get_id_MW(MW_QUESTLOG) >= 0) vscrollbar_set_bar_len (get_id_MW(MW_QUESTLOG), quest_scroll_id, (active_entries.empty()) ?0 :active_entries.size()-1); }
		void add_npc_input_handler(const unsigned char *input_text, void *data);
		void add_text_input_handler(const unsigned char *input_text, void *data);
		void cancel_action(void) { current_action = -1; }
		void find_input_handler(const char *input_text, void *data);
		void goto_entry(int ln);
		size_t get_current_entry(void) const { return (current_line < active_entries.size()) ?active_entries[current_line] :0; }
	private:
		void find_in_entry(window_info *win);
		void delete_entry(size_t entry);
		void undelete_entry(size_t entry);
		void delete_duplicates(void);
		void copy_entry(size_t entry);
		void copy_all_entries(void);
		void copy_one_entry(std::string &copy_str, size_t entry);
		void add_entry(window_info *win, size_t entry);
		void add_text_input(window_info *win);
		void draw_underline(int startx, int starty, int endx, int endy);
		// set in ui_scale_handler()
		int qlwinwidth;
		int qlwinheight;
		int qlborder;
		int spacer;
		int win_space;
		int linesep;
		// others
		bool mouse_over_questlog;
		int quest_scroll_id;
		int current_action;
		bool prompt_for_add_text;
		size_t adding_insert_pos;
		ustring adding_npc;
		INPUT_POPUP ipu_questlog;
		size_t cm_questlog_id;
		size_t cm_questlog_over_entry;
		size_t current_line;
		enum {	CMQL_SHOWALL=0, CMQL_QUESTFILTER, CMQL_NPCFILTER, CMQL_NPCSHOWNONE,
				CMQL_JUSTTHISNPC, CMQL_JUSTTHISQUEST, CMQL_S01,
				CMQL_COPY, CMQL_COPYALL, CMQL_FIND, CMQL_ADD, CMQL_S02,
				CMQL_SEL, CMQL_UNSEL, CMQL_SELALL, CMQL_UNSELALL, CMQL_SHOWSEL, CMQL_S03,
				CMQL_DELETE, CMQL_UNDEL, CMQL_S04, CMQL_DEDUPE, CMQL_S05, CMQL_SAVE };
};

static Questlog_Window questlog_window;

static int display_questlog_handler(window_info *win) { return questlog_window.display_handler(win); }
static int questlog_click(window_info *win, int mx, int my, Uint32 flags) { return questlog_window.click_handler(win, flags); }
static int keypress_questlog_handler(window_info *win, int mx, int my, SDL_Keycode key_code, Uint32 key_unicode, Uint16 key_mod)
	{ return questlog_window.keypress_handler(win, key_code, key_unicode, key_mod); }
static int mouseover_questlog_handler(window_info *win, int mx, int my) { questlog_window.mouseover_handler(my); return 0; }
static int show_questlog_handler(window_info *win) { questlist.check_auto_open(); return 0; }
static int ui_scale_questlog_handler(window_info *win) { questlog_window.ui_scale_handler(win); return 1; }
static int change_questlog_font_handler(window_info *win, font_cat cat) { return questlog_window.font_change_handler(win, cat); }
static int questlog_scroll_click (widget_list *widget, int mx, int my, Uint32 flags) { questlog_window.scroll_click_handler(widget); return 1; }
static int questlog_scroll_drag (widget_list *widget, int mx, int my, Uint32 flags, int dx, int dy) { questlog_window.scroll_drag_handler(widget); return 1; }
static int cm_quest_handler(window_info *win, int widget_id, int mx, int my, int option) { questlog_window.cm_handler(win, my, option); return 1; }
static void cm_questlog_pre_show_handler(window_info *win, int widget_id, int mx, int my, window_info *cm_win) { questlog_window.cm_preshow_handler(my); }
static void questlog_input_cancel_handler(void *data) { questlog_window.cancel_action(); }
static void questlog_add_npc_input_handler(const char *input_text, void *data) { questlog_window.add_npc_input_handler(reinterpret_cast<const unsigned char*>(input_text), data); }
static void questlog_add_text_input_handler(const char *input_text, void *data) { questlog_window.add_text_input_handler(reinterpret_cast<const unsigned char*>(input_text), data); }
static void questlog_find_input_handler(const char *input_text, void *data) { questlog_window.find_input_handler(input_text, data); }


// A class to impliment the NPC filter window
//
class NPC_Filter
{
	public:
		NPC_Filter(void) : npc_name_space(0), npc_name_border(0), npc_name_box_size(0), max_npc_name_x(0), max_npc_name_y(0),
			min_npc_name_cols(1), min_npc_name_rows(10), npc_name_cols(0), npc_name_rows(0),
			npc_filter_active_npc_name(static_cast<size_t>(-1)), npc_filter_win(-1) {}
		void open_window(void);
		void resize_handler(window_info *win);
		void ui_scale_handler(window_info *win);
		int font_change_handler(window_info *win, FontManager::Category cat);
		void display_handler(window_info *win);
		int click_handler(window_info *win, int mx, int my, Uint32 flags);
		int keypress_handler(window_info *win, int mx, int my, SDL_Keycode key_code, Uint32 key_unicode, Uint16 key_mod);
		void mouseover_handler(window_info *win, int mx, int my);
		int get_win_id(void) const { return npc_filter_win; }
		bool is_set(const ustring& npc) { return (npc_filter_map[npc] == 1); }
		void set(const ustring& npc) { npc_filter_map[npc] = 1; }
		void set_all(void) { for (auto& i: npc_filter_map) i.second = 1; }
		void unset_all(void) { for (auto& i: npc_filter_map) i.second = 0; }
	private:
		// scaled
		int npc_name_space;
		int npc_name_border;
		int npc_name_box_size;
		float max_npc_name_x;
		float max_npc_name_y;
		// other
		const unsigned int min_npc_name_cols;
		const unsigned int min_npc_name_rows;
		unsigned int npc_name_cols;
		unsigned int npc_name_rows;
		size_t npc_filter_active_npc_name;
		int npc_filter_win;
		std::map<ustring, int> npc_filter_map;
};

static NPC_Filter npc_filter;

static int resize_npc_filter_handler(window_info *win, int new_width, int new_height)
	{ npc_filter.resize_handler(win); return 0; }
static int ui_scale_npc_filter_handler(window_info *win)
	{ npc_filter.ui_scale_handler(win); return 1; }
static int change_npc_filter_font_handler(window_info *win, font_cat cat)
	{ return npc_filter.font_change_handler(win, cat); }
static int display_npc_filter_handler(window_info *win)
	{ npc_filter.display_handler(win); return 1; }
static int click_npc_filter_handler(window_info *win, int mx, int my, Uint32 flags)
	{ return npc_filter.click_handler(win, mx, my, flags); }
static int keypress_npc_filter_handler(window_info *win, int mx, int my, SDL_Keycode key_code, Uint32 key_unicode, Uint16 key_mod)
	{ return npc_filter.keypress_handler(win, mx, my, key_code, key_unicode, key_mod); }
static int mouseover_npc_filter_handler(window_info *win, int mx, int my)
	{ npc_filter.mouseover_handler(win, mx, my); return 0; }


//	Draw a context menu like hightlight using the supplied coords.
//
void draw_highlight(int topleftx, int toplefty, int widthx, int widthy, size_t col)
{
	float colours[2][2][3] = { { {gui_invert_color[0], gui_invert_color[1], gui_invert_color[2]}, {gui_color[0], gui_color[1], gui_color[2]} },
							  { {0.11, 0.11f, 0.11f}, {0.33, 0.42f, 0.70f} } };
	if (col > 1)
		col = 0;
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glColor3fv(colours[col][0]);
	glVertex2i(topleftx, toplefty);
	glColor3fv(colours[col][1]);
	glVertex2i(topleftx, toplefty + widthy);
	glVertex2i(topleftx + widthx, toplefty + widthy);
	glColor3fv(colours[col][0]);
	glVertex2i(topleftx + widthx, toplefty);
	glEnd();
	glEnable(GL_TEXTURE_2D);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

// Calculate the maximum string width of a title in the list
int Quest_List::get_max_title_width(float zoom) const
{
	int max_width = 0;
	for (const auto& iter: quests)
	{
		const std::string& title = iter.second.get_title().empty()
			? _empty_title_replacement : iter.second.get_title();
		int width = FontManager::get_instance().line_width(UI_FONT,
			reinterpret_cast<const unsigned char*>(title.c_str()), title.size(), zoom);
		max_width = std::max(max_width, width);
	}
	return max_width;
}

//	Add any new quest object to the list and request the title.
//
void Quest_List::add(Uint16 id)
{
	if ((quests.find(id) != quests.end()) || (id == Quest::UNSET_ID))
		return;
	quests.insert( std::make_pair( id, Quest(id)) );
	save_needed = true;
	title_requests.push(id);
	recalc_num_shown();
	if (title_requests.size() == 1)
		title_requests.front().request();
}


//	Mark quest completed, making a quest object if unknown so far.
//
void Quest_List::set_completed(Uint16 id, bool is_complete)
{
	if (id == Quest::UNSET_ID)
		return;
	std::map<Uint16,Quest,QuestCompare>::iterator i = quests.find(id);
	if (i != quests.end())
		i->second.set_completed(is_complete);
	else
	{
		add(id);
		i = quests.find(id);
		if (i != quests.end())
			i->second.set_completed(is_complete);
	}
	recalc_num_shown();
	save_needed = true;
}


//	Return true if the specified quest is known and its marked complete
bool Quest_List::get_completed(Uint16 id) const
{
	if (id == Quest::UNSET_ID)
		return false;
	std::map<Uint16,Quest,QuestCompare>::const_iterator i = quests.find(id);
	if (i != quests.end())
		return i->second.get_completed();
	return false;
}


//	Have received a title so set the quest from the front of the request queue.
//
void Quest_List::set_requested_title(const char* title)
{
	if (title_requests.empty())
	{
		std::cerr << "Received title [" << title << "] but not requested" << std::endl;
		return;
	}
	std::map<Uint16,Quest,QuestCompare>::iterator i = quests.find(title_requests.front().get_id());
	if (i != quests.end())
	{
		i->second.set_title(title);
		if (i->second.get_title().size() > max_title)
			max_title = i->second.get_title().size();
		title_requests.pop();
		save_needed = true;
		if (!title_requests.empty())
			title_requests.front().request();
	}
}


//	Load quests from file, creating objects as needed.
//
void Quest_List::load(void)
{
	if (!quests.empty())
	{
		save();
		return;
	}

	// Add the "show all" entry
	Quest showall(Quest::UNSET_ID);
	showall.set_title(questlist_showall_str);
	quests.insert( std::make_pair( Quest::UNSET_ID, showall) );
	max_title = showall.get_title().size();

	list_filename = std::string(get_path_config()) + "quest_" + std::string(get_lowercase_username()) + ".list";
	recalc_num_shown();

	std::ifstream in(list_filename.c_str());
	if (!in)
		return;

	std::string line;
	while (getline(in, line))
	{
		Quest new_quest(line);
		if (new_quest.get_id() != Quest::UNSET_ID)
		{
			quests.insert( std::make_pair( new_quest.get_id(), new_quest) );
			std::map<Uint16,Quest,QuestCompare>::iterator i = quests.find(new_quest.get_id());
			if ((i != quests.end()))
			{
				if (i->second.get_title().empty())
				{
					title_requests.push(i->first);
					if (title_requests.size() == 1)
						title_requests.front().request();
				}
				else if (i->second.get_title().size() > max_title)
					max_title = i->second.get_title().size();
			}
		}
	}

	save_needed = false;
	recalc_num_shown();
}


//	Save the list of quests.
//
void Quest_List::save(void)
{
	if (!save_needed)
		return;
	std::ofstream out(list_filename.c_str(), std::ios_base::out | std::ios_base::trunc);
	if (!out)
	{
		std::string error_str = std::string(file_write_error_str) + ' ' + list_filename;
		LOG_TO_CONSOLE(c_red2, error_str.c_str());
		LOG_ERROR("%s: %s \"%s\"\n", reg_error_str, file_write_error_str, list_filename.c_str());
		return;
	}
	for (std::map<Uint16,Quest,QuestCompare>::const_iterator i=quests.begin(); i!=quests.end(); ++i)
		if (i->first != Quest::UNSET_ID)
			i->second.write(out);
	save_needed = false;
}


//	Used for debug, write the list of quests to std out.
//
void Quest_List::showall(void)
{
	for (std::map<Uint16,Quest,QuestCompare>::const_iterator i=quests.begin(); i!=quests.end(); ++i)
		i->second.write(std::cout);
}


//	Allow readonly access to quests, start from the first
//
const Quest * Quest_List::get_first_quest(int offset)
{
	if (quests.empty())
		return 0;
	iter = quests.begin();
	for (int i=0; i<offset; i++)
	{
		if (++iter == quests.end())
			return 0;
		if (hide_completed)
			while (iter->second.get_completed())
				if (++iter == quests.end())
					return 0;
	}
	iter_set = true;
	return &iter->second;
}


//	Allow readonly access to quests, continue with next
//
const Quest * Quest_List::get_next_quest(void)
{
	if (!iter_set)
		return get_first_quest(0);
	++iter;
	if (hide_completed)
		while (iter != quests.end() && iter->second.get_completed())
			++iter;
	if (iter != quests.end())
		return &iter->second;
	iter_set = false;
	return 0;
}


//	Recalulate the number of quests that can be shown, it varies if hiding is enabled.
//
void Quest_List::recalc_num_shown(void)
{
	if (hide_completed)
	{
		number_shown = 0;
		for (std::map<Uint16,Quest,QuestCompare>::const_iterator i=quests.begin(); i!=quests.end(); ++i)
			if (!i->second.get_completed())
				number_shown++;
	}
	else
		number_shown = quests.size();
}


//	Return all the options values as bits in the word.
//
unsigned int Quest_List::get_options(void) const
{
	unsigned int options = 0;
	options |= (no_auto_open) ?1: 0;
	options |= ((hide_completed) ?1: 0) << 1;
	options |= ((list_left_of_entries) ?1: 0) << 2;
	return options;
}


//	Get the options from the bits of the word.
//
void Quest_List::set_options(unsigned int options)
{
	no_auto_open = options & 1;
	hide_completed = (options >> 1) & 1;
	list_left_of_entries = (options >> 2) & 1;
}


#ifdef JSON_FILES
void Quest_List::write_options(const char *dict_name) const
{
	json_cstate_set_bool(dict_name, "no_auto_open", no_auto_open);
	json_cstate_set_bool(dict_name, "hide_completed", hide_completed);
	json_cstate_set_bool(dict_name, "list_left_of_entries", list_left_of_entries);
}


void Quest_List::read_options(const char *dict_name)
{
	no_auto_open = json_cstate_get_bool(dict_name, "no_auto_open", 0);
	hide_completed = json_cstate_get_bool(dict_name, "hide_completed", 0);
	list_left_of_entries = json_cstate_get_bool(dict_name, "list_left_of_entries", 0);
}
#endif


//	Check the title request queue, remove stalled requests, request new ones.
//
void Quest_List::check_title_requests(void)
{
	if (title_requests.empty())
		return;
	if (title_requests.front().been_requested())
	{
		if (title_requests.front().is_too_old())
			title_requests.pop();
	}
	else
		title_requests.front().request();
}


//	Display the quest list, highlighing any selected and one with mouse over.
//
void Quest_List::display_handler(window_info *win)
{
	const size_t used_x = 4*spacer + win->box_size;
	const size_t disp_lines = win->len_y / linesep;
	float zoom = win->current_scale_small;

	// if resizing wait until we stop
	static Uint8 resizing = 0;
	if (win->resized)
		resizing = 1;
	// once we stop, snap the window size to fix nicely
	else if (resizing)
	{
		size_t to_show = (disp_lines>num_shown()) ?num_shown() :disp_lines;
		size_t newy = static_cast<size_t>(to_show * linesep);
		size_t newx = std::min(win->len_x, int(used_x) + get_max_title_width(zoom));
		resize_window(win->window_id, newx, newy);
		resizing = 0;
	}

	// only show help and clear the highlighted quest if the context window is closed
	if (cm_window_shown() == CM_INIT_VALUE)
	{
		clear_highlighted();
		if (show_help_text && has_mouseover())
			show_help(questlog_cm_help_str, 0, win->len_y + questlog_window.get_win_space(), win->current_scale);
	}

	// get the top line and then loop drawing all quests we can display
	vscrollbar_set_bar_len(win->window_id, get_scroll_id(), (num_shown()>disp_lines) ?num_shown()-disp_lines :0);
	int offset = (num_shown()>disp_lines) ?vscrollbar_get_pos(win->window_id, get_scroll_id()) :0;
	const Quest* thequest = get_first_quest(offset);
	int posy = spacer;
	TextDrawOptions text_options = TextDrawOptions().set_max_width(win->len_x - used_x)
		.set_max_lines(1).set_zoom(zoom);
	while ((thequest != 0) && (posy+linesep-spacer <= win->len_y))
	{
		const int hl_x = win->len_x - 2*spacer - win->box_size;
		// is this the quest the mouse is over?
		if (has_mouseover() && (cm_window_shown() == CM_INIT_VALUE) &&
			(posy+linesep-spacer > get_mouseover_y()))
		{
			set_highlighted(thequest->get_id());
			// draw highlight over active name
			draw_highlight(spacer, posy-spacer, hl_x, linesep, 0);
			// if clicked, update the filter
			if (was_clicked())
			{
				if (thequest->get_id() == Quest::UNSET_ID)
				{
					questlog_container.show_all_entries();
					active_filter = QLFLT_QUEST;
				}
				else
				{
					active_filter = QLFLT_QUEST;
					set_selected(thequest->get_id());
					questlog_container.rebuild_active_entries(quest_entries.size()-1);
				}
				do_click_sound();
			}
			clear_mouseover();
		}
		// is this the selected title?
		if ((active_filter == QLFLT_QUEST) && (thequest->get_id() == get_selected()))
			draw_highlight(spacer, posy-spacer, hl_x, linesep, 1);
		if (cm_active() && (get_highlighted() == thequest->get_id()))
			glColor3fv(gui_color);
		// display comleted quests less prominently
		else if (thequest->get_completed())
			glColor3f(0.6f,0.6f,0.6f);
		else
			glColor3f(1.0f,1.0f,1.0f);

		// display the title, truncating if its too long for the window width
		const std::string& title = thequest->get_title().empty()
			? _empty_title_replacement : thequest->get_title();
		FontManager::get_instance().draw(UI_FONT,
			reinterpret_cast<const unsigned char*>(title.c_str()), title.size(),
			2*spacer, posy, text_options);

		thequest = get_next_quest();
		posy += linesep;
	}
	// reset mouse over and clicked
	clear_mouseover();
	set_clicked(false);
}


//	Mouse left-click select an quest, mouse wheel scroll window.
//
void Quest_List::click_handler(window_info *win, Uint32 flags)
{
	if (flags&ELW_WHEEL_UP)
		vscrollbar_scroll_up(win->window_id, get_scroll_id());
	else if(flags&ELW_WHEEL_DOWN)
		vscrollbar_scroll_down(win->window_id, get_scroll_id());
	else if(flags&ELW_LEFT_MOUSE)
		set_clicked(true);
}


//	Handle window resize, keeping the scroll handler in place.
//
void Quest_List::resize_handler(window_info *win)
{
	widget_move(win->window_id, get_scroll_id(), win->len_x-win->box_size, win->box_size);
	widget_resize(win->window_id, get_scroll_id(), win->box_size, win->len_y-2*win->box_size);
}


//	Handle option selection for the quest list context menu
void Quest_List::cm_handler(int option)
{
	switch (option)
	{
		case CMQL_COMPLETED: toggle_completed(get_highlighted()); break;
		case CMQL_ADDSEL:
			for (std::set<size_t>::const_iterator i=selected_entries.begin(); i!=selected_entries.end(); ++i)
				quest_entries[*i].set_id(get_highlighted());
			questlog_container.set_save();
			questlog_container.rebuild_active_entries(questlog_window.get_current_entry());
			break;
		case CMQL_HIDECOMPLETED: recalc_num_shown(); break;
	}
}


//	Adjust things just before the quest list context menu is opened
//
void Quest_List::cm_pre_show_handler(void)
{
	quest_completed = (get_completed(highlighted_id)) ?1 :0;
	cm_grey_line(cm_id, CMQL_COMPLETED, (highlighted_id == Quest::UNSET_ID) ?1 :0);
	cm_grey_line(cm_id, CMQL_ADDSEL, selected_entries.empty() ?1 :0);
}

//	Scroll to show the currently selected quest in the quest list window.
//
void Quest_List::scroll_to_selected(void)
{
	Uint16 setected_id = questlist.get_selected();
	if (setected_id == Quest::UNSET_ID)
		return;
	const Quest* thequest = get_first_quest(0);
	for (int line_num = 0; thequest != 0; line_num++)
	{
		if (thequest->get_id() == setected_id)
		{
			vscrollbar_set_pos(get_win_id(), get_scroll_id(), line_num);
			break;
		}
		thequest = questlist.get_next_quest();
	}
}


//	Resize the quest list window.
//
void Quest_List::ui_scale_handler(window_info *win)
{
	spacer = static_cast<int>(0.5 + 3 * win->current_scale);
	linesep = win->small_font_len_y + 2 * spacer;
	if ((win->pos_id >= 0) && (win->pos_id<windows_list.num_windows))
	{
		float zoom = win->current_scale_small;
		window_info *parent_win = &windows_list.window[win->pos_id];
		int max_size_x = get_max_title_width(zoom) + win->box_size + 4 * spacer;
		int min_size_x = 10 * FontManager::get_instance().max_width_spacing(UI_FONT, zoom)
			+ 4 * spacer;
		int min_size_y = 5 * linesep;
		int size_y = (list_left_of_entries) ?linesep * static_cast<int>(parent_win->len_y / linesep) : min_size_y;
		int pos_x = (list_left_of_entries) ?-(max_size_x + questlog_window.get_win_space()) :(parent_win->len_x - max_size_x) / 2;
		int pos_y = (list_left_of_entries) ?0 : parent_win->len_y + win->small_font_len_y + questlog_window.get_win_space() + parent_win->title_height;
		set_window_min_size(win_id, min_size_x, min_size_y);
		resize_window(win_id, max_size_x, size_y);
		move_window(win_id, win->pos_id, win->pos_loc, parent_win->pos_x + pos_x, parent_win->pos_y + pos_y);
	}
}

int Quest_List::font_change_handler(window_info *win, FontManager::Category cat)
{
	if (cat != UI_FONT)
		return 0;
	ui_scale_handler(win);
	return 1;
}

//	Create or open the quest list window.
//
void Quest_List::open_window(void)
{
	if (win_id < 0)
	{
		win_id = create_window(questlist_filter_title_str, get_id_MW(MW_QUESTLOG), 0, 0, 0, 0, 0, ELW_USE_UISCALE|ELW_WIN_DEFAULT|ELW_RESIZEABLE);
		if (win_id < 0 || win_id >= windows_list.num_windows)
			return;
		set_window_custom_scale(win_id, MW_QUESTLOG);
		set_window_handler(win_id, ELW_HANDLER_DISPLAY, (int (*)())&display_questlist_handler );
		set_window_handler(win_id, ELW_HANDLER_CLICK, (int (*)())&click_questlist_handler );
		set_window_handler(win_id, ELW_HANDLER_MOUSEOVER, (int (*)())&mouseover_questlist_handler );
		set_window_handler(win_id, ELW_HANDLER_RESIZE, (int (*)())&resize_questlist_handler );
		set_window_handler(win_id, ELW_HANDLER_UI_SCALE, (int (*)())&ui_scale_questlist_handler );
		set_window_handler(win_id, ELW_HANDLER_FONT_CHANGE, (int (*)())&change_questlist_font_handler);
		scroll_id = vscrollbar_add_extended(win_id, scroll_id, NULL, 0, 0, 0, 0, 0, 1.0, 0, 1, quests.size()-1);
		ui_scale_handler(&windows_list.window[win_id]);

		cm_id = cm_create(cm_questlist_menu_str, cm_questlist_handler);
		cm_set_pre_show_handler(cm_id, cm_questlist_pre_show_handler);
		cm_add_window(cm_id, win_id);

		cm_bool_line(cm_id, CMQL_COMPLETED, &quest_completed, NULL);
		cm_bool_line(cm_id, CMQL_NOAUTOOPEN, &no_auto_open, NULL);
		cm_bool_line(cm_id, CMQL_HIDECOMPLETED, &hide_completed, NULL);
		cm_bool_line(cm_id, CMQL_LISTLEFTOFENTRIES, &list_left_of_entries, NULL);
	}
	else
		show_window(win_id);
}


//	Create a fixed vector to use if deleted and a raw npc name / deleted string.
//
void Quest_Entry::update_displayed_npc_name()
{
	if (deleted)
		disp_npc = ustring(reinterpret_cast<const unsigned char*>(questlog_deleted_str));
	else
		disp_npc = ustring(npc + npc_spacer.substr(0, npc_spacer.size()-1));
}


//	Return the lines of an entry.
//
const std::vector<ustring>& Quest_Entry::get_lines() const
{
	return deleted ? deleted_line : _lines;
}


//	Write a quest entry as a single line to the output stream.
//
void Quest_Entry::save(std::ofstream & out) const
{
	for (std::vector<ustring>::size_type i=0; i<_lines.size(); i++)
	{
		out.write(reinterpret_cast<const char*>(_lines[i].c_str()), _lines[i].size());
		if (i<_lines.size()-1)
			out.put(' ');
		else if (quest_id == Quest::UNSET_ID)
			out.put('\n');
	}
	if (quest_id != Quest::UNSET_ID)
		out << "<<" << quest_id << ">>" << std::endl;
}


//	Store entry as a group of lines that fit into the window width.
void Quest_Entry::set_lines(const ustring& the_text)
{
	// make a copy of the string, replacing \n with spaces unless preceeded by a space
	ustring text;
	char last_char = ' ';
	text.reserve(the_text.size());
	for (unsigned char c: the_text)
	{
		if (c == '\n')
		{
			if (last_char != ' ')
				text += last_char = ' ';
		}
		else if (c != '\r')
		{
			text += last_char = c;
		}
	}

	// look for and <<number>> at the end of the text, its the quest id
	if (text.compare(text.length() - 2, 2, reinterpret_cast<const unsigned char*>(">>")) == 0)
	{
		std::string::size_type open = text.rfind(reinterpret_cast<const unsigned char*>("<<"));
		if (open != std::string::npos && open != text.length() - 4)
		{
			ustring id_str = text.substr(open + 2, text.length() - open - 4);
			if (id_str.find_first_not_of(reinterpret_cast<const unsigned char*>("0123456789")) == std::string::npos)
			{
				quest_id = static_cast<Uint16>(atoi(reinterpret_cast<const char*>(id_str.c_str())));
				questlist.add(quest_id);
				text.erase(open);
			}
		}
	}

	// for matching purposes calculate the sum of the characters, if it wraps, fine
	charsum = std::accumulate(text.begin(), text.end(), Uint16(0));

	_lines.assign(1, text);
	rewrap_lines();

	update_displayed_npc_name();

	// make sure latest entry is not filtered
	npc_filter.set(npc);
}

void Quest_Entry::rewrap_lines()
{
	ustring full_text;
	for (const auto& line: _lines)
		full_text += line;

	// divide text into lines that fit within the window width
	ustring wrapped_text;
	int nr_lines;
	TextDrawOptions options = TextDrawOptions().set_max_width(content_width)
		.set_zoom(content_zoom);
	std::tie(wrapped_text, nr_lines) = FontManager::get_instance().reset_soft_breaks(
		UI_FONT, full_text, options);

	_lines.clear();
	size_t off = 0;
	while (off < wrapped_text.length())
	{
		ustring::size_type end = wrapped_text.find('\r', off);
		if (end == ustring::npos)
		{
			_lines.push_back(wrapped_text.substr(off));
			break;
		}
		_lines.push_back(wrapped_text.substr(off, end-off));
		off = end + 1;
	}
}

//	Set a new entry, using the specified npc name.
//
void Quest_Entry::set(const ustring& the_text, const ustring& the_npc)
{
// 	npc.assign(the_npc.begin(), the_npc.end());
	npc = the_npc;
	set_lines(NPC_NAME_COLOUR + npc + npc_spacer + the_text);
}


//	Set a new entry, finding the npc name from the text.
//
void Quest_Entry::set(const ustring& the_text_const)
{
	if (the_text_const.empty())
		return;
	ustring the_text = the_text_const;

	// find any quest id - a mistake!
	// adding at the start was a mistake as it messes things up for the old client
	// moved to end of the text but keep this check for now for the early adopters!
	if (the_text[0] == '<')
	{
		std::string::size_type id_end = the_text.find_first_of('>', 1);
		if (id_end != std::string::npos)
		{
			ustring id_str = the_text.substr(1,id_end-1);
			quest_id = static_cast<Uint16>(atoi(reinterpret_cast<const char*>(id_str.c_str())));
			the_text.erase(0, id_end+1);
		}
		// make sure we save to the new format
		questlog_container.set_save();
	}

	// find and npc name
	if (the_text[0] == NPC_NAME_COLOUR)
	{
		std::string::size_type npc_end = the_text.find(npc_spacer, 1);
		if ((npc_end != std::string::npos) &&
		    (npc_end < MAX_USERNAME_LENGTH) &&
			is_color(the_text[npc_end + npc_spacer.size()]))
			npc = the_text.substr(1,npc_end-1);
	}

	if (npc.empty())
	{
		npc = ustring(reinterpret_cast<const unsigned char*>("----"));
		set_lines(NPC_NAME_COLOUR + npc + npc_spacer + the_text);
	}
	else
		set_lines(the_text);
}


//	Return true if the entry contains the specified text, case insensitive.
//
bool Quest_Entry::contains_string(const char *text_to_find) const
{
	if (deleted)
		return false;
	std::string lowercase(text_to_find);
	std::transform(lowercase.begin(), lowercase.end(), lowercase.begin(), tolower);
	std::string fulltext;
	for (const auto& line: _lines)
	{
		std::remove_copy_if(line.begin(), line.end(), std::back_inserter(fulltext), is_color);
		fulltext += ' ';
	}
	std::transform(fulltext.begin(), fulltext.end(), fulltext.begin(), tolower);
	if (fulltext.find(lowercase, 0) != std::string::npos)
		return true;
	return false;
}


//	Make sure the window size is fits the rows/cols nicely.
//
void NPC_Filter::resize_handler(window_info *win)
{
	// let the width lead
	npc_name_cols = static_cast<int>(win->len_x / max_npc_name_x);
	if (npc_name_cols < min_npc_name_cols)
		npc_name_cols = min_npc_name_cols;
	// but maintain a minimum height
	npc_name_rows = (npc_filter_map.size() + npc_name_cols - 1) / npc_name_cols;
	if (npc_name_rows <= min_npc_name_rows)
	{
		npc_name_rows = min_npc_name_rows;
		npc_name_cols = min_npc_name_cols;
		while (npc_name_cols*npc_name_rows < npc_filter_map.size())
			npc_name_cols++;
	}
	set_window_scroll_inc(win->window_id, static_cast<int>(max_npc_name_y));
	set_window_scroll_len(win->window_id, static_cast<int>(npc_name_rows*max_npc_name_y-win->len_y));
}


//	Called on creation and when scaling changes, set the starting size and position fo npc filter window
//
void NPC_Filter::ui_scale_handler(window_info *win)
{
	npc_name_space = static_cast<int>(0.5 + 3 * win->current_scale);
	npc_name_border = static_cast<int>(0.5 + 5 * win->current_scale);
	npc_name_box_size = static_cast<int>(0.5 + 12 * win->current_scale);
	max_npc_name_x = npc_name_space * 3 + npc_name_box_size + MAX_USERNAME_LENGTH * win->small_font_max_len_x;
	max_npc_name_y = std::max(win->small_font_len_y, npc_name_box_size) + 2 * npc_name_space;
	if ((win->pos_id >= 0) && (win->pos_id<windows_list.num_windows))
	{
		window_info *parent_win = &windows_list.window[win->pos_id];
		int size_x = static_cast<int>(2*npc_name_border + min_npc_name_cols * max_npc_name_x + win->box_size);
		int size_y = static_cast<int>(static_cast<int>(parent_win->len_y/max_npc_name_y) * max_npc_name_y);
		int min_size_x = static_cast<int>(2*npc_name_border + min_npc_name_cols * max_npc_name_x + win->box_size);
		int min_size_y = static_cast<int>(min_npc_name_rows * max_npc_name_y);
		int pos_x = parent_win->len_x + questlog_window.get_win_space();
		int pos_y = 0;
		set_window_min_size(win->window_id, min_size_x, min_size_y);
		resize_window(win->window_id, size_x, size_y);
		move_window(win->window_id, win->pos_id, win->pos_loc, parent_win->pos_x + pos_x, parent_win->pos_y + pos_y);
	}
}

int NPC_Filter::font_change_handler(window_info *win, FontManager::Category cat)
{
	if (cat != win->font_category)
		return 0;
	ui_scale_handler(win);
	return 1;
}

//	Display handler for the quest log filter.
//
void NPC_Filter::display_handler(window_info *win)
{
	FontManager& font_manager = FontManager::get_instance();
	static Uint8 resizing = 0;
	static size_t last_filter_size = static_cast<size_t>(-1);
	int line_height = win->small_font_len_y;

	// if resizing wait until we stop
	if (win->resized)
		resizing = 1;

	// once we stop, snap the window to the new grid size
	else if (resizing)
	{
		int new_width = static_cast<int>(2*npc_name_border + npc_name_cols * max_npc_name_x + win->box_size);
		int new_rows = static_cast<int>((win->len_y+max_npc_name_y/2)/max_npc_name_y);
		int max_rows = static_cast<int>((npc_filter_map.size() + npc_name_cols - 1) / npc_name_cols);
		resizing = 0;
		resize_window (win->window_id, new_width, static_cast<int>(((new_rows > max_rows) ?max_rows :new_rows)*max_npc_name_y));
	}
	// spot new entries and make sure we resize
	else if (last_filter_size != npc_filter_map.size())
		resize_npc_filter_handler(win, -1, -1);
	last_filter_size = npc_filter_map.size();

	unsigned int row = 0;
	unsigned int col = 0;
	TextDrawOptions options = TextDrawOptions().set_max_lines(1)
		.set_zoom(win->current_scale_small);
	for (const auto& i: npc_filter_map)
	{
		int posx = static_cast<int>(npc_name_border + col*max_npc_name_x + 0.5);
		int posy = static_cast<int>(row*max_npc_name_y + 0.5);
		int boxy = posy + static_cast<int>(std::round((max_npc_name_y-npc_name_box_size)/2));
		int texty = posy + static_cast<int>(std::round((max_npc_name_y-line_height)/2));

		// draw highlight over active name
		if ((col+row*npc_name_cols) == npc_filter_active_npc_name)
			draw_highlight(posx, posy, static_cast<int>(0.5+max_npc_name_x), static_cast<int>(0.5+max_npc_name_y), 0);

		// set the colour and position for the box and text
		if ((active_filter != QLFLT_NPC) && (active_filter != QLFLT_NONE))
			glColor3f(0.7f, 0.7f, 0.7f);
		else
			glColor3f(1.0f, 1.0f, 1.0f);
		posx += npc_name_space;

		// draw the on/off box
		glDisable(GL_TEXTURE_2D);
		glBegin( i.second ? GL_QUADS: GL_LINE_LOOP);
		glVertex2i(posx, boxy);
		glVertex2i(posx + npc_name_box_size, boxy);
		glVertex2i(posx + npc_name_box_size, boxy + npc_name_box_size);
		glVertex2i(posx, boxy + npc_name_box_size);
		glEnd();
		glEnable(GL_TEXTURE_2D);

		// draw the string
		font_manager.draw(win->font_category, i.first.c_str(), i.first.length(),
			posx + npc_name_box_size + npc_name_space, texty, options);

		// control row and col values
		col++;
		if (col >= npc_name_cols)
		{
			col = 0;
			row++;
		}
	}

	// make sure the mouse over detection is fresh next time
	npc_filter_active_npc_name = static_cast<size_t>(-1);

#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}


//	When a npc name is mouse clicked, toggle the filter value.
//
int NPC_Filter::click_handler(window_info *win, int mx, int my, Uint32 flags)
{
	if ((my < 0) || (flags & ELW_WHEEL))
		return 0;
	int yoffset = get_window_scroll_pos(win->window_id);
	size_t index = static_cast<int>((my+yoffset) / max_npc_name_y) * npc_name_cols + static_cast<int>(mx / max_npc_name_x);
	if (index >= npc_filter_map.size())
		return 0;
	size_t j = 0;
	for (std::map<ustring,int>::iterator i = npc_filter_map.begin(); i != npc_filter_map.end(); ++i, j++)
		if (j == index)
		{
			do_click_sound();
			i->second ^= 1;
			active_filter = QLFLT_NPC;
			questlist.set_selected(Quest::UNSET_ID);
			questlog_container.rebuild_active_entries(questlog_window.get_current_entry());
			break;
		}
	return 1;
}


//	Move the window scroll position to match the key pressed with the first character of the npc name.
//
int NPC_Filter::keypress_handler(window_info *win, int mx, int my, SDL_Keycode key_code, Uint32 key_unicode, Uint16 key_mod)
{
	char keychar = tolower(static_cast<char>(key_unicode));
	if ((key_mod & KMOD_CTRL) || (key_mod & KMOD_ALT) || (keychar<'a') || (keychar>'z'))
		return 0;
	size_t line = 0;
	for (std::map<ustring,int>::iterator i = npc_filter_map.begin(); i != npc_filter_map.end(); ++i, line++)
	{
		if (!i->first.empty() && (tolower(i->first[0]) == keychar))
		{
			set_window_scroll_pos(win->window_id, static_cast<int>(static_cast<int>(line/npc_name_cols)*max_npc_name_y));
			return 1;
		}
	}
	return 1;
}


//	Record which name the mouse is over so it can be highlighted.
//
void NPC_Filter::mouseover_handler(window_info *win, int mx, int my)
{
	mx -= npc_name_border;
	int yoffset = get_window_scroll_pos(win->window_id);
	if ((my >= yoffset) && (mx >= 0) && (mx < (npc_name_cols * max_npc_name_x)))
		npc_filter_active_npc_name = static_cast<int>(my / max_npc_name_y) * npc_name_cols + static_cast<int>(mx / max_npc_name_x);
}


//	Open/Create the npc filter window.
//
void NPC_Filter::open_window(void)
{
	if (npc_filter_win < 0)
	{
		npc_filter_win = create_window(questlog_npc_filter_title_str, get_id_MW(MW_QUESTLOG), 0, 0, 0, 0, 0, ELW_USE_UISCALE|ELW_SCROLLABLE|ELW_RESIZEABLE|ELW_WIN_DEFAULT);
		if (npc_filter_win < 0 && npc_filter_win >=  windows_list.num_windows)
			return;
		set_window_custom_scale(npc_filter_win, MW_QUESTLOG);
		set_window_handler(npc_filter_win, ELW_HANDLER_DISPLAY, (int (*)())&display_npc_filter_handler );
		set_window_handler(npc_filter_win, ELW_HANDLER_CLICK, (int (*)())&click_npc_filter_handler );
		set_window_handler(npc_filter_win, ELW_HANDLER_KEYPRESS, (int (*)())&keypress_npc_filter_handler );
		set_window_handler(npc_filter_win, ELW_HANDLER_MOUSEOVER, (int (*)())&mouseover_npc_filter_handler );
		set_window_handler(npc_filter_win, ELW_HANDLER_RESIZE, (int (*)())&resize_npc_filter_handler );
		set_window_handler(npc_filter_win, ELW_HANDLER_UI_SCALE, (int (*)())&ui_scale_npc_filter_handler );
		set_window_handler(npc_filter_win, ELW_HANDLER_FONT_CHANGE, (int (*)())&change_npc_filter_font_handler);
		ui_scale_npc_filter_handler(&windows_list.window[npc_filter_win]);
	}
	else
		show_window(npc_filter_win);
	set_window_scroll_pos(npc_filter_win, 0);
}


//	Draw a simple line
//
void Questlog_Window::draw_underline(int startx, int starty, int endx, int endy)
{
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_LINES);
	glVertex2i(startx, starty);
	glVertex2i(endx, endy);
	glEnd();
	glEnable(GL_TEXTURE_2D);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}


//	Copy an entry to a string, stripped of special characters.
//
void Questlog_Window::copy_one_entry(std::string &copy_str, size_t entry)
{
	for (const auto& line: quest_entries[active_entries[entry]].get_lines())
	{
		std::remove_copy_if(line.begin(), line.end(), std::back_inserter(copy_str), is_color);
		copy_str += ' ';
	}
	if (!copy_str.empty())
		copy_str[copy_str.size()-1] = '\n';
}


//	Copy all shown enties to the clipboard, stripped of special characters.
//
void Questlog_Window::copy_all_entries(void)
{
	std::string copy_str;
	for (std::vector<size_t>::size_type i=0; i<active_entries.size(); ++i)
		copy_one_entry(copy_str, i);
	copy_to_clipboard(copy_str.c_str());
}


//	Copy an entry to the clipboard, stripped of special characters.
//
void Questlog_Window::copy_entry(size_t entry)
{
	std::string copy_str;
	copy_one_entry(copy_str, entry);
	copy_to_clipboard(copy_str.c_str());
}


//	Start inputting a new entry, part 1 prompt for npc name.
//
void Questlog_Window::add_entry(window_info *win, size_t entry)
{
	if (current_action != -1)
		return;
	current_action = CMQL_ADD;
	adding_insert_pos = (entry < active_entries.size()) ?active_entries[entry] :quest_entries.size();
	close_ipu(&ipu_questlog);
	init_ipu(&ipu_questlog, win->window_id, MAX_USERNAME_LENGTH, 1, MAX_USERNAME_LENGTH + 1, questlog_input_cancel_handler, questlog_add_npc_input_handler);
	display_popup_win(&ipu_questlog, questlog_add_npc_prompt_str);
	centre_popup_window(&ipu_questlog);
}


//	Continue inputting a new entry, part 2 have npc so queue the input
//	for the body.  Can't call directly as reusing ipu_questlog and it
//	would get cleared on return from this function!
//
void Questlog_Window::add_npc_input_handler(const unsigned char *input_text, void *data)
{
	adding_npc = ustring(input_text);
	prompt_for_add_text = true;
}


//	Continue inputting a new entry, part 3 prompt for entry main text.
//
void Questlog_Window::add_text_input(window_info *win)
{
	prompt_for_add_text = false;
	close_ipu(&ipu_questlog);
	init_ipu(&ipu_questlog, win->window_id, 1024, 5, 40, questlog_input_cancel_handler, questlog_add_text_input_handler);
	ipu_questlog.allow_nonprint_chars = 1;
	display_popup_win(&ipu_questlog, questlog_add_text_prompt_str);
	centre_popup_window(&ipu_questlog);
}


//	Continue inputting a new entry, part 4 insert the entry.
//
void Questlog_Window::add_text_input_handler(const unsigned char *input_text, void *data)
{
	current_action = -1;
	Quest_Entry ne;
	std::vector<Quest_Entry>::iterator e = quest_entries.insert(quest_entries.begin() + adding_insert_pos, ne);
	e->set(to_color_char(c_grey1) + ustring(input_text), adding_npc);
	questlog_container.set_save();
	questlist.set_selected(Quest::UNSET_ID);
	questlog_container.rebuild_active_entries(adding_insert_pos);
}


//	Find, searching from next entry to end then back to current.  Done
//	like this so we find multiple entries each time we hit ok.
//
void Questlog_Window::find_input_handler(const char *input_text, void *data)
{
	for (std::vector<Quest_Entry>::size_type entry = current_line+1; entry<active_entries.size(); entry++)
		if (quest_entries[active_entries[entry]].contains_string(input_text))
		{
			goto_entry(entry);
			return;
		}
	for (std::vector<Quest_Entry>::size_type entry = 0; entry<=current_line && entry<active_entries.size(); entry++)
		if (quest_entries[active_entries[entry]].contains_string(input_text))
		{
			goto_entry(entry);
			return;
		}
	do_alert1_sound();
}


//	Prompt for text to find.  The dialogue will not close when "OK"
//	pressed but will call the callback.  Use "Cancel" to close.
//
void Questlog_Window::find_in_entry(window_info *win)
{
	if (current_action != -1)
		return;
	current_action = CMQL_FIND;
	close_ipu(&ipu_questlog);
	init_ipu(&ipu_questlog, win->window_id, 21, 1, 22, questlog_input_cancel_handler, questlog_find_input_handler);
	ipu_questlog.accept_do_not_close = 1;
	display_popup_win(&ipu_questlog, questlog_find_prompt_str);
	centre_popup_window(&ipu_questlog);
}


//	Mark the current entry as deleted.
//
void Questlog_Window::undelete_entry(size_t entry)
{
	quest_entries[active_entries[entry]].set_deleted(false);
	questlog_container.set_save();
}


//	Mark the current entry as not deleted.
//
void Questlog_Window::delete_entry(size_t entry)
{
	quest_entries[active_entries[entry]].set_deleted(true);
	questlog_container.set_save();
}


//	Look for matching entries and delete (mark as deleted) all but first
//
void Questlog_Window::delete_duplicates(void)
{
	int deleted_count = 0;
	LOG_TO_CONSOLE(c_green1, questlog_deldupe_start_str);
	std::multimap<Uint16, ustring> mm;

	for (std::vector<Quest_Entry>::iterator entry=quest_entries.begin(); entry!=quest_entries.end(); ++entry)
	{
		if (!entry->get_deleted())
		{
			// get the full text for the entry
			ustring the_text;
			for (const auto& line: entry->get_lines())
				the_text += line;

			// see if the charsum has already been seen
			std::multimap<Uint16, ustring>::const_iterator iter = mm.find(entry->get_charsum());
			if (iter != mm.end())
			{
				// we have a charsum match so check the text, there may be more than one with this charsum
				std::multimap<Uint16, ustring>::const_iterator last = mm.upper_bound(entry->get_charsum());
				for ( ; iter != last; ++iter)
				{
					// if the text and the charsum match, mark the entry as deleted
					if (iter->second == the_text)
					{
						entry->set_deleted(true);
						questlog_container.set_save();
						deleted_count++;
						break;
					}
				}
			}

			// save - as either the charsum did not match a previous entry or the charsum did but the text didn't
			if (!entry->get_deleted())
    			mm.insert(std::make_pair(entry->get_charsum(), the_text));
		}
	}

	char message[80];
	safe_snprintf(message, sizeof(message), questlog_deldupe_end_str, mm.size(), deleted_count);
	LOG_TO_CONSOLE(c_green1, message);
}


//	Enable/disable menu options as required....
//
void Questlog_Window::cm_preshow_handler(int my)
{
	cm_questlog_over_entry = active_entries.size();
	for (std::vector<Shown_Entry>::const_iterator i=shown_entries.begin(); i!=shown_entries.end(); ++i)
		if (i->is_over(my))
		{
			cm_questlog_over_entry = i->get_entry();
			break;
		}
	bool is_over_entry = (cm_questlog_over_entry < active_entries.size());
	bool is_deleted = is_over_entry && quest_entries[active_entries[cm_questlog_over_entry]].get_deleted();
	bool qlw_open = get_show_window(questlist.get_win_id());
	bool nfw_open = get_show_window(npc_filter.get_win_id());
	bool is_selected = is_over_entry && (selected_entries.find(active_entries[cm_questlog_over_entry]) != selected_entries.end());
	cm_grey_line(cm_questlog_id, CMQL_SHOWALL, quest_entries.empty() ?1 :0);
	cm_grey_line(cm_questlog_id, CMQL_QUESTFILTER, (qlw_open || quest_entries.empty()) ?1 :0);
	cm_grey_line(cm_questlog_id, CMQL_NPCFILTER, (nfw_open || quest_entries.empty()) ?1 :0);
	cm_grey_line(cm_questlog_id, CMQL_NPCSHOWNONE, (quest_entries.empty()) ?1 :0);
	cm_grey_line(cm_questlog_id, CMQL_JUSTTHISNPC, (is_over_entry) ?0 :1);
	cm_grey_line(cm_questlog_id, CMQL_JUSTTHISQUEST, (is_over_entry && (quest_entries[active_entries[cm_questlog_over_entry]].get_id() != Quest::UNSET_ID)) ?0 :1);
	cm_grey_line(cm_questlog_id, CMQL_COPY, (is_over_entry && !is_deleted) ?0 :1);
	cm_grey_line(cm_questlog_id, CMQL_COPYALL, active_entries.empty() ?1 :0);
	cm_grey_line(cm_questlog_id, CMQL_FIND, (current_action == -1 && !active_entries.empty()) ?0 :1);
	cm_grey_line(cm_questlog_id, CMQL_ADD, (current_action == -1) ?0 :1);
	cm_grey_line(cm_questlog_id, CMQL_SEL, (is_over_entry && !is_deleted && !is_selected) ?0 :1);
	cm_grey_line(cm_questlog_id, CMQL_UNSEL, (is_over_entry && is_selected) ?0 :1);
	cm_grey_line(cm_questlog_id, CMQL_SELALL, quest_entries.empty() ?1 :0);
	cm_grey_line(cm_questlog_id, CMQL_UNSELALL, selected_entries.empty() ?1 :0);
	cm_grey_line(cm_questlog_id, CMQL_SHOWSEL, selected_entries.empty() ?1 :0);
	cm_grey_line(cm_questlog_id, CMQL_DELETE, (is_over_entry && !is_deleted) ?0 :1);
	cm_grey_line(cm_questlog_id, CMQL_UNDEL, (is_over_entry && is_deleted) ?0 :1);
	cm_grey_line(cm_questlog_id, CMQL_DEDUPE, quest_entries.empty() ?1 :0);
	cm_grey_line(cm_questlog_id, CMQL_SAVE, (questlog_container.save_needed()) ?0 :1);
}


//	Call options selected from the context menu.
//
void Questlog_Window::cm_handler(window_info *win, int my, int option)
{
	size_t over_entry = active_entries.size();
	for (std::vector<Shown_Entry>::const_iterator i=shown_entries.begin(); i!=shown_entries.end(); ++i)
		if (i->is_over(my))
		{
			over_entry = i->get_entry();
			break;
		}
	switch (option)
	{
		case CMQL_SHOWALL: questlog_container.show_all_entries(); break;
		case CMQL_QUESTFILTER: questlist.open_window(); break;
		case CMQL_NPCFILTER: npc_filter.open_window(); break;
		case CMQL_JUSTTHISQUEST:
			if (over_entry < active_entries.size())
			{
				Uint16 the_id = quest_entries[active_entries[over_entry]].get_id();
				if (the_id != Quest::UNSET_ID)
				{
					active_filter = QLFLT_QUEST;
					questlist.set_selected(the_id);
					questlog_container.rebuild_active_entries(quest_entries.size()-1);
					questlist.scroll_to_selected();
				}
			}
			break;
		case CMQL_JUSTTHISNPC:
		case CMQL_NPCSHOWNONE:
			questlist.set_selected(Quest::UNSET_ID);
			active_filter = QLFLT_NPC;
			npc_filter.unset_all();
			if (option == CMQL_JUSTTHISNPC)
				npc_filter.set(quest_entries[active_entries[over_entry]].get_npc());
			questlog_container.rebuild_active_entries(get_current_entry());
			npc_filter.open_window();
			break;
		case CMQL_COPY: if (over_entry < active_entries.size()) copy_entry(over_entry); break;
		case CMQL_COPYALL: copy_all_entries(); break;
		case CMQL_FIND: find_in_entry(win); break;
		case CMQL_ADD: add_entry(win, over_entry); break;
		case CMQL_SEL: if (over_entry < active_entries.size()) selected_entries.insert(active_entries[over_entry]); break;
		case CMQL_UNSEL: if (over_entry < active_entries.size()) selected_entries.erase(active_entries[over_entry]); break;
		case CMQL_SELALL:
			for (std::vector<size_t>::const_iterator i=active_entries.begin(); i!=active_entries.end(); ++i)
				selected_entries.insert(*i);
			break;
		case CMQL_UNSELALL:
			if (active_filter == QLFLT_SEL)
				questlog_container.show_all_entries();
			selected_entries.clear();
			break;
		case CMQL_SHOWSEL:
			active_filter = QLFLT_SEL;
			questlog_container.rebuild_active_entries(get_current_entry());
			break;
		case CMQL_DELETE: if (over_entry < active_entries.size()) delete_entry(over_entry); break;
		case CMQL_UNDEL: if (over_entry < active_entries.size()) undelete_entry(over_entry); break;
		case CMQL_DEDUPE: delete_duplicates(); break;
		case CMQL_SAVE: questlog_container.save(); break;
	}
}


//	Update values that change with ui scaling
//
void Questlog_Window::ui_scale_handler(window_info *win)
{
	int content_width = 70 * 8 * win->current_scale;
	float zoom = win->current_scale_small;

	qlborder = static_cast<int>(0.5 + 5 * win->current_scale);
	spacer = static_cast<int>(0.5 + 3 * win->current_scale);
	win_space = static_cast<int>(0.5 + 10 * win->current_scale);
	linesep = win->small_font_len_y + 2 * spacer;
	qlwinwidth = content_width + 2 * qlborder + win->box_size;
	qlwinheight = 16 * linesep;

	widget_resize(win->window_id, quest_scroll_id, win->box_size, qlwinheight - win->box_size);
	widget_move(win->window_id, quest_scroll_id, qlwinwidth - win->box_size, win->box_size);
	resize_window(win->window_id, qlwinwidth, qlwinheight);

	if (questlist.get_win_id() >= 0 && questlist.get_win_id() < windows_list.num_windows)
		questlist.ui_scale_handler(&windows_list.window[questlist.get_win_id()]);

	if (npc_filter.get_win_id() >= 0 && npc_filter.get_win_id() < windows_list.num_windows)
		ui_scale_npc_filter_handler(&windows_list.window[npc_filter.get_win_id()]);

	Quest_Entry::content_width = content_width;
	Quest_Entry::content_zoom = zoom;
	for (auto& entry: quest_entries)
		entry.rewrap_lines();
}

int Questlog_Window::font_change_handler(window_info *win, FontManager::Category cat)
{
	if (cat != FontManager::Category::UI_FONT)
		return 0;
	ui_scale_handler(win);
	return 1;
}

//	Draw the window contents.
//
int Questlog_Window::display_handler(window_info *win)
{
	// If required, call the next stage of a entry input.
	if (prompt_for_add_text)
		add_text_input(win);

	questlist.check_title_requests();

	if (cm_window_shown() == CM_INIT_VALUE)
	{
		cm_questlog_over_entry = active_entries.size();
		if (show_help_text && mouse_over_questlog && (current_action == -1))
		{
			show_help(questlog_cm_help_str, 0, win->len_y + win_space, win->current_scale);
			mouse_over_questlog = false;
		}
	}

	TextDrawOptions options = TextDrawOptions().set_max_lines(1)
		.set_zoom(win->current_scale_small);
	FontManager& font_manager = FontManager::get_instance();
	int questlog_y = qlborder;
	shown_entries.clear();
	for (std::vector<Quest_Entry>::size_type entry = current_line; entry<active_entries.size(); entry++)
	{
		int start_y = questlog_y;
		const std::vector<ustring> &lines = quest_entries[active_entries[entry]].get_lines();
		glColor3f(1.0f, 1.0f, 1.0f);
		for (const auto& line: lines)
		{
			font_manager.draw(FontManager::Category::UI_FONT, line.c_str(), line.length(),
				qlborder, questlog_y, options);
			questlog_y += win->small_font_len_y;
			if (questlog_y+qlborder > qlwinheight - win->small_font_len_y)
				break;
		}
		glColor3f(0.7f, 0.7f, 1.0f);
		if ((cm_questlog_over_entry < active_entries.size()) && (entry == cm_questlog_over_entry))
		{
			glColor3fv(gui_color);
			const ustring& name = quest_entries[active_entries[entry]].get_disp_npc();
			font_manager.draw(FontManager::Category::UI_FONT, name.c_str(), name.length(),
				qlborder, start_y, options);
		}
		if (selected_entries.find(active_entries[entry]) != selected_entries.end())
		{
			const ustring& name = quest_entries[active_entries[entry]].get_disp_npc();
			int width = font_manager.line_width(UI_FONT, name.c_str(), name.length(),
				options.zoom());
			draw_underline(qlborder, start_y + win->small_font_len_y,
				qlborder + width, start_y + win->small_font_len_y);
		}
		shown_entries.push_back(Shown_Entry(entry, start_y, questlog_y));
		questlog_y += qlborder;
		if (questlog_y+qlborder > qlwinheight - win->small_font_len_y)
			return 1;
	}
	return 1;
}


//	Move to quest entry after scroll click
void Questlog_Window::scroll_click_handler(widget_list *widget)
{
	int scroll = vscrollbar_get_pos (get_id_MW(MW_QUESTLOG), widget->id);
	goto_entry(scroll);
}


//	Move to quest entry after scroll drag
void Questlog_Window::scroll_drag_handler(widget_list *widget)
{
	int scroll = vscrollbar_get_pos (get_id_MW(MW_QUESTLOG), widget->id);
	goto_entry(scroll);
}


//	Move to entry of click wheel button in window
int Questlog_Window::click_handler(window_info *win, Uint32 flags)
{
	int questlog_win = get_id_MW(MW_QUESTLOG);
	if(flags&ELW_WHEEL_UP) {
		vscrollbar_scroll_up(questlog_win, quest_scroll_id);
		goto_entry(vscrollbar_get_pos(questlog_win, quest_scroll_id));
		return 1;
	} else if(flags&ELW_WHEEL_DOWN) {
		vscrollbar_scroll_down(questlog_win, quest_scroll_id);
		goto_entry(vscrollbar_get_pos(questlog_win, quest_scroll_id));
		return 1;
	} else {
		return 0;
	}
}


// Keypress in window, check for find key
int Questlog_Window::keypress_handler(window_info *win, SDL_Keycode key_code, Uint32 key_unicode, Uint16 key_mod)
{
	char keychar = tolower(static_cast<char>(key_unicode));
	if (KEY_DEF_CMP(K_MARKFILTER, key_code, key_mod) || (keychar=='/'))
	{
		find_in_entry(win);
		return 1;
	}
	return 0;
}


//	Register mouse over window
void Questlog_Window::mouseover_handler(int my)
{
	if (my>=0)
		mouse_over_questlog = true;
}


//	Scroll to the specified line
void Questlog_Window::goto_entry(int ln)
{
	if(ln <= 0)
		current_line = 0;
	else if (static_cast<size_t>(ln) >= active_entries.size())
		current_line = active_entries.size() - 1;
	else
		current_line = ln;
	vscrollbar_set_pos(get_id_MW(MW_QUESTLOG), quest_scroll_id, current_line);
}


//	Create (or show existing) a stand alone quest log window.
//
void Questlog_Window::open(void)
{
	int questlog_win = get_id_MW(MW_QUESTLOG);

	if (questlog_win < 0)
	{
		questlog_win = create_window(tab_questlog, (not_on_top_now(MW_QUESTLOG) ?game_root_win : -1), 0,
			get_pos_x_MW(MW_QUESTLOG), get_pos_y_MW(MW_QUESTLOG), 0, 0, ELW_USE_UISCALE|ELW_WIN_DEFAULT);
		if (questlog_win < 0 || questlog_win >= windows_list.num_windows)
			return;
		set_id_MW(MW_QUESTLOG, questlog_win);
		set_window_custom_scale(questlog_win, MW_QUESTLOG);
		set_window_handler(questlog_win, ELW_HANDLER_DISPLAY, (int (*)())display_questlog_handler);
		set_window_handler(questlog_win, ELW_HANDLER_CLICK, (int (*)())questlog_click);
		set_window_handler(questlog_win, ELW_HANDLER_MOUSEOVER, (int (*)())&mouseover_questlog_handler );
		set_window_handler(questlog_win, ELW_HANDLER_KEYPRESS, (int (*)())&keypress_questlog_handler );
		set_window_handler(questlog_win, ELW_HANDLER_SHOW, (int (*)())&show_questlog_handler );
		set_window_handler(questlog_win, ELW_HANDLER_UI_SCALE, (int (*)())&ui_scale_questlog_handler );
		set_window_handler(questlog_win, ELW_HANDLER_FONT_CHANGE, (int (*)())&change_questlog_font_handler);

		window_info *win = &windows_list.window[questlog_win];
		ui_scale_questlog_handler(win);
		check_proportional_move(MW_QUESTLOG);

		size_t last_entry = active_entries.size()-1;
		quest_scroll_id = vscrollbar_add_extended (questlog_win, quest_scroll_id, NULL, qlwinwidth - win->box_size, win->box_size, win->box_size, qlwinheight - win->box_size, 0, 1.0, last_entry, 1, last_entry);
		goto_entry(last_entry);

		widget_set_OnClick (questlog_win, quest_scroll_id, (int (*)())questlog_scroll_click);
		widget_set_OnDrag (questlog_win, quest_scroll_id, (int (*)())questlog_scroll_drag);

		if (!cm_valid(cm_questlog_id))
		{
			cm_questlog_id = cm_create(cm_questlog_menu_str, cm_quest_handler);
			cm_add_window(cm_questlog_id, questlog_win);
			cm_set_pre_show_handler(cm_questlog_id, cm_questlog_pre_show_handler);
			init_ipu(&ipu_questlog, -1, 1, 1, 1, NULL, NULL);
		}
	}
	else
		show_window(questlog_win);

	questlist.check_auto_open();
}


//	When entries are added or the filter changes, recreate the active entry list.
//
void Questlog_Container::rebuild_active_entries(size_t desired_top_entry)
{
	size_t new_current_line = 0;
	active_entries.clear();
	for (std::vector<Quest_Entry>::size_type entry=0; entry<quest_entries.size(); ++entry)
	{
		if (entry == desired_top_entry)
			new_current_line = active_entries.size();
		bool useit = false;
		switch (active_filter)
		{
			case QLFLT_NONE: useit = true; break;
			case QLFLT_NPC: if (npc_filter.is_set(quest_entries[entry].get_npc())) useit = true; break;
			case QLFLT_QUEST:
				if ((questlist.get_selected() == Quest::UNSET_ID) ||
					(questlist.get_selected() == quest_entries[entry].get_id()))
					useit = true;
				break;
			case QLFLT_SEL:
				if (selected_entries.find(entry) != selected_entries.end())
					useit = true;
				break;
		}
		if (useit)
			active_entries.push_back(entry);
	}
	questlog_window.update_scrollbar_len();
	questlog_window.goto_entry(new_current_line);
}


//	If needed, save the complete questlog.
//
void Questlog_Container::save(void)
{
	if (!need_to_save)
		return;
	std::ofstream out(filename.c_str(), std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
	if (!out)
	{
		std::string error_str = std::string(file_write_error_str) + ' ' + filename;
		LOG_TO_CONSOLE(c_red2, error_str.c_str());
		LOG_ERROR("%s: %s \"%s\"\n", reg_error_str, file_write_error_str, filename.c_str());
		return;
	}
	for (std::vector<Quest_Entry>::const_iterator entry=quest_entries.begin(); entry!=quest_entries.end(); ++entry)
		if (!entry->get_deleted())
			entry->save(out);
	need_to_save = false;

	LOG_DEBUG("Wrote questlog to file '%s'", filename.c_str());
}


//	Reset the filters and active lists so that all entries are shown.
//
void Questlog_Container::show_all_entries(void)
{
	active_filter = QLFLT_NONE;
	// set all NPC entries
	npc_filter.set_all();
	// clean a quest filter
	questlist.set_selected(Quest::UNSET_ID);
	rebuild_active_entries(questlog_window.get_current_entry());
}


//	Add a new entry to the end of the quest log.  The entry will either
//	have been read from the file or about to be appended to the end.
//  Note:
//	Rebuilding the active_entry list must be done by the caller after this.
//
void Questlog_Container::add_line(const ustring& t, const unsigned char *npcprefix)
{
	quest_entries.push_back(Quest_Entry());
	if (*npcprefix)
		quest_entries.back().set(t, ustring(npcprefix));
	else
		quest_entries.back().set(t);
}


//	Called when a new quest entry is received from the server.  Add to
//	the end of the entries and append to the questlog file immediately.
//
void Questlog_Container::add_entry(const unsigned char *t, int len)
{
	add_line(ustring(t, len), (const unsigned char*)npc_name);

	if (questlist.waiting_for_entry())
	{
		quest_entries.back().set_id(questlist.get_next_id());
		questlist.clear_next_id();
	}

	if ((active_filter == QLFLT_QUEST) && (questlist.get_selected() != Quest::UNSET_ID))
		questlist.set_selected(quest_entries.back().get_id());
	rebuild_active_entries(quest_entries.size()-1);

	std::ofstream out(filename.c_str(), std::ios_base::out | std::ios_base::binary | std::ios_base::app);
	if (!out)
	{
		std::string error_str = std::string(file_write_error_str) + ' ' + filename;
		LOG_TO_CONSOLE(c_red2, error_str.c_str());
		LOG_ERROR("%s: %s \"%s\"\n", reg_error_str, file_write_error_str, filename.c_str());
		return;
	}
    quest_entries.back().save(out);
    flash_icon(tt_questlog, 5);
}


//	Load the questlog, use the playname tag file but fall back to the
//	old file name if the new does not exist.  If reading from an old
//	named file, write the new file as we go.
//
void Questlog_Container::load(void)
{
	questlist.load();

	// If the quest log is already loaded, just make sure we're saved.
	// This will take place when relogging after disconnection.
	if (!quest_entries.empty())
	{
		save();
		return;
	}

	filename = std::string(get_path_config()) + "quest_" + std::string(get_lowercase_username()) + ".log";

	std::ifstream in(filename.c_str(), std::ios_base::in | std::ios_base::binary);
	std::ofstream out;
	if (!in)
	{
		in.clear();
		in.open((std::string(get_path_config()) + "quest.log").c_str(), std::ios_base::in | std::ios_base::binary);
		if (!in)
			return;
		out.open(filename.c_str(), std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
	}

	std::string line;
	while (getline(in, line))
	{
		if (!line.empty())
		{
			add_line(ustring(line.begin(), line.end()),
				reinterpret_cast<const unsigned char*>(""));
			if (out)
				quest_entries.back().save(out);
		}
	}

	LOG_DEBUG("Read questlog from file '%s'", filename.c_str());

	rebuild_active_entries(quest_entries.size()-1);
}


extern "C" void add_questlog(const unsigned char *t, int len)
{
	questlog_container.add_entry(t, len);
}


extern "C" void load_questlog(void)
{
	questlog_container.load();
}

extern "C" void unload_questlog()
{
	questlog_container.save();
	questlist.save();
}

extern "C" void display_questlog(void)
{
	questlog_window.open();
}

extern "C" void set_next_quest_entry_id(Uint16 id)
{
	//char buf[80];
	//safe_snprintf(buf, 80, "Received NEXT_NPC_MESSAGE_IS_QUEST with id=%d", id);
	//LOG_TO_CONSOLE(c_green1, buf);
	//if (waiting_for_questlog_entry())
	//	LOG_TO_CONSOLE(c_red2, "Previous NEXT_NPC_MESSAGE_IS_QUEST was unused");
	questlist.set_next_id(id);
	questlist.add(id);
}


// Return true if a NEXT_NPC_MESSAGE_IS_QUEST message has been received
// but has not yet been used.
//
extern "C" int waiting_for_questlog_entry(void)
{
	return questlist.waiting_for_entry();
}


// Make sure we are not expecting a new questlog entry
//
extern "C" void clear_waiting_for_questlog_entry(void)
{
	questlist.clear_next_id();
}


extern "C" void set_quest_title(const char *data, int len)
{
	char buf[256];
	//LOG_TO_CONSOLE(c_green1, "Received HERE_IS_QUEST_ID");
	safe_strncpy2(buf, data, 255, len);
	//LOG_TO_CONSOLE(c_green1, buf);
	questlist.set_requested_title(buf);
}


extern "C" void set_quest_finished(Uint16 id)
{
	//char buf[80];
	//safe_snprintf(buf, 80, "Received QUEST_FINISHED with id=%d", id);
	//LOG_TO_CONSOLE(c_green1, buf);
	questlist.set_completed(id, true);
}


extern "C" unsigned int get_options_questlog(void)
{
	return questlist.get_options();
}


extern "C" void set_options_questlog(unsigned int cfg_options)
{
	questlist.set_options(cfg_options);
}


#ifdef JSON_FILES
extern "C" void write_options_questlog(const char *dict_name)
{
	questlist.write_options(dict_name);
}


extern "C" void read_options_questlog(const char *dict_name)
{
	questlist.read_options(dict_name);
}
#endif
