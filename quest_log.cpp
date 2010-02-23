/*
	Rewrite of quest log with new context menu features.
 
	Author bluap/pjbroad Feb 2010
*/

#include <string>
#include <vector>
#include <cctype>
#include <algorithm>
#include <iostream>
#include <fstream>

#include "asc.h"
#ifdef CONTEXT_MENUS
#include "context_menu.h"
#endif
#include "dialogues.h"
#include "elwindows.h"
#include "errors.h"
#include "gamewin.h"
#include "hud.h"
#include "init.h"
#include "interface.h"
#include "io/elpathwrapper.h"
#include "notepad.h"
#include "paste.h"
#include "questlog.h"
#include "tabs.h"
#include "translate.h"

/*
 * To Do:
 *
 * 		Add Filters and/or quest titles.
 * 		Make file xml?
 */

//	A single entry for the questlog.
class Quest_Entry
{
	public:
		Quest_Entry(void) : deleted(false) {}
		void set(const std::string & the_text);
		void set(const std::string & the_text, const std::string & the_npc);
		const std::vector<std::string> & get_lines(void) const;
		void save(std::ofstream & out) const;
		bool contains_string(const char *text_to_find) const;
		bool deleted;
	private:
		void set_lines(const std::string & the_text);
		std::vector<std::string> lines;
		std::string npc;
};

//	Holds and tests position information for a show entry.
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


static const char NPC_NAME_COLOUR = c_blue2;
static const std::string npc_spacer = ": ";
static std::vector<Quest_Entry> quest_entries;
static std::vector<Shown_Entry> shown_entries;
static std::vector<std::string> deleted_entry;
static int quest_scroll_id = 14;
static std::string filename;
static size_t current_line = 0;
static bool need_to_save = false;
static bool mouse_over_questlog = false;
static int current_action = -1;
#ifdef CONTEXT_MENUS
static size_t cm_questlog_id = CM_INIT_VALUE;
enum {	CMQL_FILTER=0, CMQL_COPY, CMQL_FIND, CMQL_ADD, CMQL_S1, CMQL_DELETE, CMQL_UNDEL, CMQL_S2, CMQL_SAVE };
static std::string adding_npc;
static size_t adding_insert_pos = 0;
static bool prompt_for_add_text = false;
static INPUT_POPUP ipu_questlog;
#endif


//	Return the lines of an entry.
//
const std::vector<std::string> & Quest_Entry::get_lines(void) const
{
	if (!deleted)
		return lines;
	if (deleted_entry.empty())
		deleted_entry.push_back(static_cast<char>(to_color_char(c_grey2)) + std::string(questlog_deleted_str));
	return deleted_entry;
}


//	Write all lines as a single line to the output stream.
//
void Quest_Entry::save(std::ofstream & out) const
{
	for (std::vector<std::string>::size_type i=0; i<lines.size(); i++)
	{
		out.write(lines[i].c_str(), lines[i].size());
		if (i<lines.size()-1)
			out.put(' ');
		else
			out.put('\n');
	}
}


//	Store entry as a group of lines that fit into the window width.
//
void Quest_Entry::set_lines(const std::string & the_text)
{
	int chars_per_line = static_cast<int>((STATS_TAB_WIDTH - 25) / SMALL_FONT_X_LEN);
	int col = 0;
	std::string::size_type last_space = 0;
	std::string::size_type start = 0;
	std::string text = the_text;

	for (std::string::size_type i=0; i<text.size(); i++)
	{
		if (is_color(text[i]))
			continue;			
		if (text[i] == '\n')
			text[i] = ' ';
		else if (text[i] == ' ')
			last_space = i;
		if (col >= chars_per_line)
		{
			lines.push_back(text.substr(start, last_space-start));
			start = last_space+1;
			col = i - start;
		}
		col++;
	}

	if (start < text.size())
		lines.push_back(text.substr(start, text.size()-start));
}


//	Set a new entry, using the specified npc name.
//
void Quest_Entry::set(const std::string & the_text, const std::string & the_npc)
{
	npc = the_npc;
	set_lines(static_cast<char>(to_color_char(NPC_NAME_COLOUR)) + npc + npc_spacer + the_text);
}


//	Set a new entry, finding the npc name from the text.
//
void Quest_Entry::set(const std::string & the_text)
{
	if (the_text[0] == (char)to_color_char(NPC_NAME_COLOUR))
	{
		std::string::size_type npc_end = the_text.find(npc_spacer, 1);
		if ((npc_end != std::string::npos) &&
		    (npc_end < MAX_USERNAME_LENGTH) &&
			is_color(the_text[npc_end + npc_spacer.size()]))
			npc = the_text.substr(1,npc_end-1);
	}
	if (npc.empty())
		set_lines(static_cast<char>(to_color_char(NPC_NAME_COLOUR)) + std::string("----") + npc_spacer + the_text);
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
	for (std::vector<std::string>::size_type i=0; i<lines.size(); i++)
		fulltext += lines[i] + ' ';
	std::transform(fulltext.begin(), fulltext.end(), fulltext.begin(), tolower);
	if (fulltext.find(lowercase, 0) != std::string::npos)
		return true;
	return false;		
}




//	If needed, save the complete questlog.
//
static void save_questlog(void)
{
	if (!need_to_save)
		return;
	std::ofstream out(filename.c_str(), std::ios_base::binary);
	if (!out)
	{
		std::string error_str = std::string(file_write_error_str) + ' ' + filename;
		LOG_TO_CONSOLE(c_red2, error_str.c_str());
		LOG_ERROR("%s: %s \"%s\"\n", reg_error_str, file_write_error_str, filename.c_str());
		return;
	}
	for (std::vector<Quest_Entry>::size_type entry = 0; entry<quest_entries.size(); entry++)
		if (!quest_entries[entry].deleted)
			quest_entries[entry].save(out);
	need_to_save = false;
}


#ifdef CONTEXT_MENUS
//	Copy and entry to the clipboard, stripped of special characters.
//
static void copy_entry(size_t entry)
{
	std::string copy_str;
	for (std::vector<std::string>::size_type line = 0; line < quest_entries[entry].get_lines().size(); line++)
	{
		for (std::string::size_type i = 0; i < quest_entries[entry].get_lines()[line].size(); i++)
			if (!is_color(quest_entries[entry].get_lines()[line][i]))
				copy_str += quest_entries[entry].get_lines()[line][i];
		copy_str += ' ';
	}
	copy_str[copy_str.size()-1] = '\n';
	copy_to_clipboard(copy_str.c_str());
}


//	The input operation was cancalled, clear the busy flag.
//
static void questlog_input_cancel_handler(void)
{
	current_action = -1;
}


//  Prototypes for inputting a new entry - so we can order in sequence.
static void add_entry(window_info *win, size_t entry);
static void questlog_add_npc_input_handler(const char *input_text);
static void questlog_add_text_input(window_info *win);
static void questlog_add_text_input_handler(const char *input_text);

//	Start inputting a new entry, part 1 prompt for npc name.
//
static void add_entry(window_info *win, size_t entry)
{
	if (current_action != -1)
		return;
	current_action = CMQL_ADD;
	adding_insert_pos = entry;
	init_ipu(&ipu_questlog, questlog_win, -1, -1, MAX_USERNAME_LENGTH, 1, questlog_input_cancel_handler, questlog_add_npc_input_handler);
	ipu_questlog.x = (win->len_x - ipu_questlog.popup_x_len) / 2;
	ipu_questlog.y = (win->len_y - ipu_questlog.popup_y_len) / 2;
	display_popup_win(&ipu_questlog, questlog_add_npc_prompt_str);
}

//	Continue inputting a new entry, part 2 have npc so queue the input
//	for the body.  Can't call directly as reusing ipu_questlog and it
//	would get cleared on return from this function!
//
static void questlog_add_npc_input_handler(const char *input_text)
{
	adding_npc = std::string(input_text);
	prompt_for_add_text = true;
}

//	Continue inputting a new entry, part 3 prompt for npc main text.
//
static void questlog_add_text_input(window_info *win)
{
	prompt_for_add_text = false;
	init_ipu(&ipu_questlog, questlog_win, 400, -1, 256, 5, questlog_input_cancel_handler, questlog_add_text_input_handler);
	ipu_questlog.x = (win->len_x - ipu_questlog.popup_x_len) / 2;
	ipu_questlog.y = (win->len_y - ipu_questlog.popup_y_len) / 2;
	ipu_questlog.allow_nonprint_chars = 1;
	display_popup_win(&ipu_questlog, questlog_add_text_prompt_str);
}

//	Continue inputting a new entry, part 4 insert the entry.
//
static void questlog_add_text_input_handler(const char *input_text)
{
	current_action = -1;
	Quest_Entry ne;	
	std::vector<Quest_Entry>::iterator e = quest_entries.insert(quest_entries.begin() + adding_insert_pos, ne);
	e->set(static_cast<char>(to_color_char(c_grey1)) + std::string(input_text), adding_npc);
	need_to_save = true;
	if (questlog_win >= 0)
		vscrollbar_set_bar_len (questlog_win, quest_scroll_id, quest_entries.size()-1);
}


//	Find, searching from next entry to end then back to current.  Done
//	like this so we find multiple entries each time we hit look.
//
static void questlog_find_input_handler(const char *input_text)
{
	for (std::vector<Quest_Entry>::size_type entry = current_line+1; entry<quest_entries.size(); entry++)
		if (quest_entries[entry].contains_string(input_text))
		{
			goto_questlog_entry(entry);
			return;
		}
	for (std::vector<Quest_Entry>::size_type entry = 0; entry<quest_entries.size(); entry++)
		if (quest_entries[entry].contains_string(input_text))
		{
			goto_questlog_entry(entry);
			return;
		}
}

//	Prompt for text to find.  The dialogue will not close when "OK"
//	pressed but will call the callback.  Use "Cancel" to close.
//
static void find_in_entry(window_info *win)
{
	if (current_action != -1)
		return;
	current_action = CMQL_FILTER;
	init_ipu(&ipu_questlog, questlog_win, -1, -1, 21, 1, questlog_input_cancel_handler, questlog_find_input_handler);
	ipu_questlog.x = (win->len_x - ipu_questlog.popup_x_len) / 2;
	ipu_questlog.y = win->len_y + 20;
	ipu_questlog.accept_do_not_close = 1;
	display_popup_win(&ipu_questlog, questlog_find_prompt_str);
}


//	Mark the current entry as deleted.
//
static void undelete_entry(size_t entry)
{
	quest_entries[entry].deleted = false;
	need_to_save = true;
}


//	Mark the current entry as not deleted.
//
static void delete_entry(size_t entry)
{
	quest_entries[entry].deleted = true;
	need_to_save = true;
}


//	Enable/disable menu options as required....
//
static void cm_questlog_pre_show_handler(window_info *win, int widget_id, int mx, int my, window_info *cm_win)
{
	size_t over_entry = quest_entries.size();
	for (std::vector<Shown_Entry>::size_type i=0; i<shown_entries.size(); i++)
		if (shown_entries[i].is_over(my))
			over_entry = shown_entries[i].get_entry();
	bool is_over_entry = (over_entry < quest_entries.size());
	bool is_deleted = is_over_entry && quest_entries[over_entry].deleted;
	cm_grey_line(cm_questlog_id, CMQL_FILTER, 1);
	cm_grey_line(cm_questlog_id, CMQL_COPY, (is_over_entry && !is_deleted) ?0 :1);
	cm_grey_line(cm_questlog_id, CMQL_FIND, (current_action == -1) ?0 :1);
	cm_grey_line(cm_questlog_id, CMQL_ADD, (current_action == -1) ?0 :1);	
	cm_grey_line(cm_questlog_id, CMQL_DELETE, (is_over_entry && !is_deleted) ?0 :1);
	cm_grey_line(cm_questlog_id, CMQL_UNDEL, (is_over_entry && is_deleted) ?0 :1);
	cm_grey_line(cm_questlog_id, CMQL_SAVE, (need_to_save) ?0 :1);
}


//	Call options selected from the context menu.
//
static int cm_quest_handler(window_info *win, int widget_id, int mx, int my, int option)
{
	size_t over_entry = quest_entries.size();
	for (std::vector<Shown_Entry>::size_type i=0; i<shown_entries.size(); i++)
		if (shown_entries[i].is_over(my))
			over_entry = shown_entries[i].get_entry();
	switch (option)
	{
		case CMQL_FILTER: break;
		case CMQL_COPY: if (over_entry < quest_entries.size()) copy_entry(over_entry); break;
		case CMQL_FIND: find_in_entry(win); break;
		case CMQL_ADD: add_entry(win, over_entry); break;
		case CMQL_DELETE: if (over_entry < quest_entries.size()) delete_entry(over_entry); break;
		case CMQL_UNDEL: if (over_entry < quest_entries.size()) undelete_entry(over_entry); break;
		case CMQL_SAVE: save_questlog(); break;
	 }
	return 1;
}
#endif


//	Draw the window contents.
//
static int display_questlog_handler(window_info *win)
{
#ifdef CONTEXT_MENUS
	// If required, call the next stage of a entry input.
	if (prompt_for_add_text)
		questlog_add_text_input(win);
#endif

	if (show_help_text && mouse_over_questlog && (current_action == -1))
	{
		show_help(cm_title_help_str, 0, win->len_y + 10);
		mouse_over_questlog = false;
	}

	int questlog_y = 0, start_y = 0;
	shown_entries.clear();
	for (std::vector<Quest_Entry>::size_type entry = current_line; entry<quest_entries.size(); entry++)
	{
		start_y = questlog_y;
		for (std::vector<std::string>::size_type line = 0; line < quest_entries[entry].get_lines().size(); line++)
		{
			const char *t = quest_entries[entry].get_lines()[line].c_str();
			draw_string_small (2, questlog_y, reinterpret_cast<const unsigned char *>(t), 1);
			questlog_y += static_cast<int>(SMALL_FONT_Y_LEN);
			if (questlog_y > STATS_TAB_HEIGHT - SMALL_FONT_Y_LEN)
				break;
		}
		shown_entries.push_back(Shown_Entry(entry, start_y, questlog_y));
		questlog_y += 5;
		if (questlog_y > STATS_TAB_HEIGHT - SMALL_FONT_Y_LEN)
			return 1;
	}
	return 1;
}


static int questlog_scroll_click (widget_list *widget, int mx, int my, Uint32 flags)
{
	int scroll = vscrollbar_get_pos (questlog_win, widget->id);
	goto_questlog_entry (scroll);
	return 1;
}


static int questlog_scroll_drag (widget_list *widget, int mx, int my, Uint32 flags, int dx, int dy)
{
	int scroll = vscrollbar_get_pos (questlog_win, widget->id);
	goto_questlog_entry (scroll);
	return 1;
}


static int questlog_click(window_info *win, int mx, int my, Uint32 flags)
{
	if(flags&ELW_WHEEL_UP) {
		vscrollbar_scroll_up(questlog_win, quest_scroll_id);
		goto_questlog_entry(vscrollbar_get_pos(questlog_win, quest_scroll_id));
		return 1;
	} else if(flags&ELW_WHEEL_DOWN) {
		vscrollbar_scroll_down(questlog_win, quest_scroll_id);
		goto_questlog_entry(vscrollbar_get_pos(questlog_win, quest_scroll_id));
		return 1;
	} else {
		return 0;
	}
}


static int mouseover_questlog_handler(window_info *win, int mx, int my)
{
	mouse_over_questlog = true;
	return 1;
}


//	Add a new entry to the end of the quest log.  The entry will either
//	have been read from the file or already appended to the end.
//
static void add_questlog_line(const char *t, const char *npcprefix)
{	
	quest_entries.push_back(Quest_Entry());
	if (strlen(npcprefix))
		quest_entries.back().set(std::string(t), std::string(npcprefix));
	else
		quest_entries.back().set(std::string(t));
	if (questlog_win >= 0)
		vscrollbar_set_bar_len (questlog_win, quest_scroll_id, quest_entries.size()-1);
}


extern "C" void goto_questlog_entry(int ln)
{
	if(ln <= 0)
		current_line = 0;
	else if (static_cast<size_t>(ln) >= quest_entries.size())
		current_line = quest_entries.size() - 1;
	else
		current_line = ln;
	vscrollbar_set_pos(questlog_win, quest_scroll_id, current_line);
}


//	Called when a new quest entry is received from teh server.  Add the
//	the end of the entries and append to the questlog file immediately.
//
extern "C" void add_questlog (char *t, int len)
{
	t[len] = '\0';
	add_questlog_line(t, (const char*)npc_name);

	std::ofstream out(filename.c_str(), std::ios_base::binary | std::ios_base::app);
	if (!out)
	{
		std::string error_str = std::string(file_write_error_str) + ' ' + filename;
		LOG_TO_CONSOLE(c_red2, error_str.c_str());
		LOG_ERROR("%s: %s \"%s\"\n", reg_error_str, file_write_error_str, filename.c_str());
		return;
	}
    out.seekp( 0, std::ios::end );
    quest_entries.back().save(out);
}


//	Load the questlog, use the playname tag file but fall back to the
//	old file name if the new does not exist.  If reading from an old
//	named file, write the new file as we go.
//
extern "C" void load_questlog()
{
	std::string username = std::string(username_str);
	std::transform(username.begin(), username.end(), username.begin(), tolower);
	filename = std::string(get_path_config()) + "quest_" + username + ".log";

	std::ifstream in(filename.c_str(), std::ios_base::binary);
	std::ofstream out;
	if (!in)
	{
		in.clear();
		in.open((std::string(get_path_config()) + "quest.log").c_str(), std::ios_base::binary);
		if (!in)
			return;
		out.open(filename.c_str(), std::ios_base::binary);
	}

	std::string line;
	while (getline(in, line))
	{
		if (!line.empty())
		{
			add_questlog_line(line.c_str(), "");
			if (out)
				quest_entries.back().save(out);
		}
	}
}


//	The questlog will only need saving if we add, delete or undelete.
//
extern "C" void unload_questlog()
{
	save_questlog();
}


//	Create the questlog inside the tabbed window.
//
extern "C" void fill_questlog_win ()
{
	int boxlen = 0;
	size_t last_entry = quest_entries.size()-1;

	set_window_handler(questlog_win, ELW_HANDLER_DISPLAY, (int (*)())display_questlog_handler);
	set_window_handler(questlog_win, ELW_HANDLER_CLICK, (int (*)())questlog_click);
	set_window_handler(questlog_win, ELW_HANDLER_MOUSEOVER, (int (*)())&mouseover_questlog_handler );

	quest_scroll_id = vscrollbar_add_extended (questlog_win, quest_scroll_id, NULL, STATS_TAB_WIDTH - 20, boxlen, 20, STATS_TAB_HEIGHT - boxlen, 0, 1.0, 0.77f, 0.57f, 0.39f, last_entry, 1, last_entry);
	goto_questlog_entry(last_entry);
	
	widget_set_OnClick (questlog_win, quest_scroll_id, (int (*)())questlog_scroll_click);
	widget_set_OnDrag (questlog_win, quest_scroll_id, (int (*)())questlog_scroll_drag);

#ifdef CONTEXT_MENUS
	if (cm_questlog_id == CM_INIT_VALUE)
	{
		cm_questlog_id = cm_create(cm_questlog_menu_str, cm_quest_handler);
		cm_add_window(cm_questlog_id, questlog_win);
		cm_set_pre_show_handler(cm_questlog_id, cm_questlog_pre_show_handler);
	}
#endif
}


extern "C"
{
	int questlog_win=-1;
	// not used - remove (from init.c too) if we keep this code
	int questlog_menu_x=-1;
	int questlog_menu_y=-1;
}
