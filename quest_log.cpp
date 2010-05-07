/*
	Rewrite of quest log with new context menu features.
 
	Author bluap/pjbroad Feb 2010
*/

#include <string>
#include <vector>
#include <map>
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
#include "gl_init.h"
#include "hud.h"
#include "init.h"
#include "interface.h"
#include "io/elpathwrapper.h"
#include "notepad.h"
#include "paste.h"
#include "questlog.h"
#ifdef NEW_SOUND
#include "sound.h"
#endif
#include "tabs.h"
#include "translate.h"

/*
 * TODO Possible changes...
 * 		Make file xml?
 * 		Remove save options - always save / timed save?
 * 		Replace entry containers with classes
 * 		md5sum each entry for matching to quest strands
 *		Add dedupe function
 * 			use md5sum for speed, then strcmp
 * 		Implement quest strands:
 * 			tag entry with quest strand(s)
 * 			filter by quest strand
 * 			colour code in some way?
 * 		Keypress handling - I keep trying / & ctrl-f for search....
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
		const std::string & get_npc(void) const { return npc; }
		bool deleted;
	private:
		void set_lines(const std::string & the_text);
		std::vector<std::string> lines;
		std::string npc;
};

//	Holds and tests position information for a shown entry.
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
static std::vector<size_t> active_entries;
static std::vector<Shown_Entry> shown_entries;
static std::map<std::string, int> filter_map;
static std::vector<std::string> deleted_entry;
static int quest_scroll_id = 14;
static std::string filename;
static size_t current_line = 0;
static bool need_to_save = false;
static bool mouse_over_questlog = false;
#ifdef CONTEXT_MENUS
static size_t cm_questlog_id = CM_INIT_VALUE;
enum {	CMQL_FILTER=0, CMQL_SHOWALL, CMQL_SHOWNONE, CMQL_S1, CMQL_COPY, CMQL_COPYALL, CMQL_FIND, CMQL_ADD, CMQL_S2, CMQL_DELETE, CMQL_UNDEL, CMQL_S3, CMQL_SAVE };
static std::string adding_npc;
static size_t adding_insert_pos = 0;
static bool prompt_for_add_text = false;
static INPUT_POPUP ipu_questlog;
static int current_action = -1;
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
	std::string text;
	char last_char = ' ';

	// make a copy of the string, replacing \n with spaces unless preceeded by a space
	text.reserve(the_text.size());
	for (std::string::size_type i=0; i<the_text.size(); i++)
	{
		if (the_text[i] == '\n')
		{
			if (last_char != ' ')
				text += last_char = ' ';
		}
		else
			text += last_char = the_text[i];
	}

	// divide text into lines that fit within the window width
	for (std::string::size_type i=0; i<text.size(); i++)
	{
		if (is_color(text[i]))
			continue;			
		if (text[i] == ' ')
			last_space = i;
		if (col >= chars_per_line)
		{
			lines.push_back(text.substr(start, last_space-start));
			start = last_space+1;
			col = i - start;
		}
		col++;
	}

	// catch the last line
	if (start < text.size())
		lines.push_back(text.substr(start, text.size()-start));

	// make sure latest entry is not filtered - shouldn't really use global !
	filter_map[npc] = 1;
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
	{
		npc = std::string("----");
		set_lines(static_cast<char>(to_color_char(NPC_NAME_COLOUR)) + npc + npc_spacer + the_text);
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
	for (std::vector<std::string>::size_type i=0; i<lines.size(); i++)
		fulltext += lines[i] + ' ';
	std::transform(fulltext.begin(), fulltext.end(), fulltext.begin(), tolower);
	if (fulltext.find(lowercase, 0) != std::string::npos)
		return true;
	return false;		
}




//	Set the length of the scrollbar depending on number entries given current filter.
//
static void set_scrollbar_len(void)
{
	if (questlog_win >= 0)
		vscrollbar_set_bar_len (questlog_win, quest_scroll_id,
			(active_entries.empty()) ?0 :active_entries.size()-1);
}


//	When entries are added or the filter changes, recreate the active entry list.
//
static void rebuild_active_entries(size_t desired_top_entry)
{
	size_t new_current_line = 0;
	active_entries.clear();
	for (std::vector<Quest_Entry>::size_type entry=0; entry<quest_entries.size(); ++entry)
	{
		if (entry == desired_top_entry)
			new_current_line = active_entries.size();
		if (filter_map[quest_entries[entry].get_npc()])
			active_entries.push_back(entry);
	}
	set_scrollbar_len();
	goto_questlog_entry(new_current_line);
}


//	If needed, save the complete questlog.
//
static void save_questlog(void)
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
		if (!entry->deleted)
			entry->save(out);
	need_to_save = false;
}


#ifdef CONTEXT_MENUS
// quest log filter window vars
static const int npc_name_space = 3;
static const int npc_name_border = 5;
static const int npc_name_box_size = 12;
static const float max_npc_name_x = npc_name_space*3+npc_name_box_size+(MAX_USERNAME_LENGTH) * SMALL_FONT_X_LEN;
static const float max_npc_name_y = SMALL_FONT_Y_LEN + 2*npc_name_space;
static const unsigned int min_npc_name_cols = 1;
static const unsigned int min_npc_name_rows = 10;
static unsigned int npc_name_cols = 0;
static unsigned int npc_name_rows = 0;
static size_t quest_filter_active_npc_name = static_cast<size_t>(-1);

//	Make sure the window size is fits the rows/cols nicely.
//
static int resize_quest_filter_handler(window_info *win, int new_width, int new_height)
{
		// let the width lead
		npc_name_cols = win->len_x / max_npc_name_x;
		if (npc_name_cols < min_npc_name_cols)
			npc_name_cols = min_npc_name_cols;
		// but maintain a minimum height
		npc_name_rows = (filter_map.size() + npc_name_cols - 1) / npc_name_cols;
		if (npc_name_rows <= min_npc_name_rows)
		{
			npc_name_rows = min_npc_name_rows;
			npc_name_cols = min_npc_name_cols;
			while (npc_name_cols*npc_name_rows < filter_map.size())
				npc_name_cols++;
		}
		set_window_scroll_len(win->window_id, npc_name_rows*max_npc_name_y-win->len_y);
		return 0;
}


//	Display handler for the quest log filter.
//
static int display_quest_filter_handler(window_info *win)
{
	static Uint8 resizing = 0;
	static size_t last_filter_size = static_cast<size_t>(-1);
	
	// if resizing wait until we stop
	if (win->resized)
		resizing = 1;

	// once we stop, snap the window to the new grid size
	else if (resizing)
	{
		int new_width = 2*npc_name_border + npc_name_cols * max_npc_name_x + ELW_BOX_SIZE;
		int new_rows = (win->len_y+max_npc_name_y/2)/max_npc_name_y;
		int max_rows = (filter_map.size() + npc_name_cols - 1) / npc_name_cols;
		resizing = 0;
		resize_window (win->window_id, new_width, ((new_rows > max_rows) ?max_rows :new_rows)*max_npc_name_y);
	}
	// spot new entries and make sure we resize
	else if (last_filter_size != filter_map.size())
		resize_quest_filter_handler(win, -1, -1);
	last_filter_size = filter_map.size();

	unsigned int row = 0;
	unsigned int col = 0;
	for (std::map<std::string,int>::const_iterator i = filter_map.begin(); i != filter_map.end(); ++i)
	{
		int posx = static_cast<int>(npc_name_border + col*max_npc_name_x + 0.5);
		int posy = static_cast<int>(row*max_npc_name_y + 0.5);
		
		// draw highlight over active name
		if ((col+row*npc_name_cols) == quest_filter_active_npc_name)
		{
			glDisable(GL_TEXTURE_2D);
			glBegin(GL_QUADS);
			glColor3f(0.11f, 0.11f, 0.11f);	
			glVertex2i(posx, posy);
			glColor3f(0.77f, 0.57f, 0.39f);
			glVertex2i(posx, posy + max_npc_name_y);
			glVertex2i(posx + max_npc_name_x, posy + max_npc_name_y);
			glColor3f(0.11f, 0.11f, 0.11f);	
			glVertex2i(posx + max_npc_name_x, posy);
			glEnd();
			glEnable(GL_TEXTURE_2D);
		}

		// set the colour and position for the box and text
		glColor3f(1.0f, 1.0f, 1.0f);
		posx += npc_name_space;
		posy += static_cast<int>(0.5 + (max_npc_name_y-npc_name_box_size)/2);

		// draw the on/off box
		glDisable(GL_TEXTURE_2D);
		glBegin( i->second ? GL_QUADS: GL_LINE_LOOP);
		glVertex2i(posx, posy);
		glVertex2i(posx + npc_name_box_size, posy);
		glVertex2i(posx + npc_name_box_size, posy + npc_name_box_size);
		glVertex2i(posx, posy + npc_name_box_size);
		glEnd();
		glEnable(GL_TEXTURE_2D);

		// draw the string
		draw_string_small(posx + npc_name_box_size + npc_name_space, posy, (unsigned char*)i->first.c_str(), 1);

		// control row and col values
		col++;
		if (col >= npc_name_cols)
		{
			col = 0;
			row++;
		}
	}

	// make sure the mouse over detection is fresh next time
	quest_filter_active_npc_name = static_cast<size_t>(-1);
	
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	return 1;
}


//	When a npc name is mouse clicked, toggle the filter value.
//
static int click_quest_filter_handler(window_info *win, int mx, int my, Uint32 flags)
{
	if ((my < 0) || (flags & ELW_WHEEL))
		return 0;
	int yoffset = get_window_scroll_pos(win->window_id);
	size_t index = static_cast<int>((my+yoffset) / max_npc_name_y) * npc_name_cols + static_cast<int>(mx / max_npc_name_x);
	if (index >= filter_map.size())
		return 0;
	size_t j = 0;
	for (std::map<std::string,int>::iterator i = filter_map.begin(); i != filter_map.end(); ++i, j++)
		if (j == index)
		{
#ifdef NEW_SOUND
			add_sound_object(get_index_for_sound_type_name("Button Click"), 0, 0, 1);
#endif			
			i->second ^= 1;
			rebuild_active_entries((current_line < active_entries.size()) ?active_entries[current_line] :0);
			break;
		}
	return 1;
}


//	Move the window scroll position to match the key pressed with the first character of the npc name.
//
static int keypress_quest_filter_handler(window_info *win, int mx, int my, Uint32 key, Uint32 unikey)
{
	char keychar = tolower(static_cast<char>(unikey));
	if ((key & ELW_CTRL) || (key & ELW_ALT) || (keychar<'a') || (keychar>'z'))
		return 0;
	size_t line = 0;
	for (std::map<std::string,int>::iterator i = filter_map.begin(); i != filter_map.end(); ++i, line++)
	{
		if (!i->first.empty() && (tolower(i->first[0]) == keychar))
		{
			set_window_scroll_pos(win->window_id, static_cast<int>(line/npc_name_cols)*max_npc_name_y);
			return 1;
		}
	}	
	return 1;
}


//	Record which name the mouse is over so it can be highlighted.
//
static int mouseover_quest_filter_handler(window_info *win, int mx, int my)
{
	mx -= npc_name_border;
	if ((my >= 0) && (mx >= 0) && (mx < (npc_name_cols * max_npc_name_x)))
		quest_filter_active_npc_name = static_cast<int>(my / max_npc_name_y) * npc_name_cols + static_cast<int>(mx / max_npc_name_x);
	return 0; // make sure we get a arrow cursor
}


//	Open/Create the NPC list filter window.
//
static void open_filter_window(void)
{
	static int quest_filter_win = -1;
	if (quest_filter_win < 0)
	{
		window_info *win = &windows_list.window[questlog_win];
		int min_x = 2*npc_name_border + min_npc_name_cols * max_npc_name_x + ELW_BOX_SIZE;
		int min_y = min_npc_name_rows * max_npc_name_y;
		quest_filter_win = create_window(questlog_add_npc_filter_str, questlog_win, -1, win->len_x + 10, 0,
			min_x, static_cast<int>(win->len_y/max_npc_name_y)*max_npc_name_y,
			ELW_SCROLLABLE|ELW_RESIZEABLE|ELW_WIN_DEFAULT);
		set_window_handler(quest_filter_win, ELW_HANDLER_DISPLAY, (int (*)())&display_quest_filter_handler );
		set_window_handler(quest_filter_win, ELW_HANDLER_CLICK, (int (*)())&click_quest_filter_handler );
		set_window_handler(quest_filter_win, ELW_HANDLER_KEYPRESS, (int (*)())&keypress_quest_filter_handler );
		set_window_handler(quest_filter_win, ELW_HANDLER_MOUSEOVER, (int (*)())&mouseover_quest_filter_handler );
		set_window_handler(quest_filter_win, ELW_HANDLER_RESIZE, (int (*)())&resize_quest_filter_handler );
		set_window_min_size(quest_filter_win, min_x, min_y);
		set_window_scroll_inc(quest_filter_win, max_npc_name_y);
		resize_quest_filter_handler(&windows_list.window[quest_filter_win], -1, -1);
	}
	else
		show_window(quest_filter_win);
	set_window_scroll_pos(quest_filter_win, 0);
}


//	Copy an entry to a string, stripped of special characters.
//
static void copy_one_entry(std::string &copy_str, size_t entry)
{
	for (std::vector<std::string>::const_iterator line = quest_entries[active_entries[entry]].get_lines().begin(); line != quest_entries[active_entries[entry]].get_lines().end(); ++line)
	{
		for (std::string::const_iterator i=line->begin(); i!=line->end(); ++i)
			if (!is_color(*i))
				copy_str += *i;
		copy_str += ' ';
	}
	if (copy_str.empty())
		return;
	copy_str[copy_str.size()-1] = '\n';
}


//	Copy all shown enties to the clipboard, stripped of special characters.
//
static void copy_all_entries(void)
{
	std::string copy_str;
	for (std::vector<size_t>::size_type i=0; i<active_entries.size(); ++i)
		copy_one_entry(copy_str, i);
	copy_to_clipboard(copy_str.c_str());
}


//	Copy an entry to the clipboard, stripped of special characters.
//
static void copy_entry(size_t entry)
{
	std::string copy_str;
	copy_one_entry(copy_str, entry);
	copy_to_clipboard(copy_str.c_str());
}


//	The input operation was cancalled, clear the busy flag.
//
static void questlog_input_cancel_handler(void *data)
{
	current_action = -1;
}


//  Prototypes for inputting a new entry - so we can order in sequence.
static void add_entry(window_info *win, size_t entry);
static void questlog_add_npc_input_handler(const char *input_text, void *data);
static void questlog_add_text_input(window_info *win);
static void questlog_add_text_input_handler(const char *input_text, void *data);

//	Start inputting a new entry, part 1 prompt for npc name.
//
static void add_entry(window_info *win, size_t entry)
{
	if (current_action != -1)
		return;
	current_action = CMQL_ADD;
	adding_insert_pos = (entry < active_entries.size()) ?active_entries[entry] :quest_entries.size();
	init_ipu(&ipu_questlog, questlog_win, -1, -1, MAX_USERNAME_LENGTH, 1, questlog_input_cancel_handler, questlog_add_npc_input_handler);
	ipu_questlog.x = (win->len_x - ipu_questlog.popup_x_len) / 2;
	ipu_questlog.y = (win->len_y - ipu_questlog.popup_y_len) / 2;
	display_popup_win(&ipu_questlog, questlog_add_npc_prompt_str);
}

//	Continue inputting a new entry, part 2 have npc so queue the input
//	for the body.  Can't call directly as reusing ipu_questlog and it
//	would get cleared on return from this function!
//
static void questlog_add_npc_input_handler(const char *input_text, void *data)
{
	adding_npc = std::string(input_text);
	prompt_for_add_text = true;
}

//	Continue inputting a new entry, part 3 prompt for entry main text.
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
static void questlog_add_text_input_handler(const char *input_text, void *data)
{
	current_action = -1;
	Quest_Entry ne;
	std::vector<Quest_Entry>::iterator e = quest_entries.insert(quest_entries.begin() + adding_insert_pos, ne);
	e->set(static_cast<char>(to_color_char(c_grey1)) + std::string(input_text), adding_npc);
	need_to_save = true;
	rebuild_active_entries(adding_insert_pos);
}


//	Find, searching from next entry to end then back to current.  Done
//	like this so we find multiple entries each time we hit ok.
//
static void questlog_find_input_handler(const char *input_text, void *data)
{
	for (std::vector<Quest_Entry>::size_type entry = current_line+1; entry<active_entries.size(); entry++)
		if (quest_entries[active_entries[entry]].contains_string(input_text))
		{
			goto_questlog_entry(entry);
			return;
		}
	for (std::vector<Quest_Entry>::size_type entry = 0; entry<=current_line && entry<active_entries.size(); entry++)
		if (quest_entries[active_entries[entry]].contains_string(input_text))
		{
			goto_questlog_entry(entry);
			return;
		}
#ifdef NEW_SOUND
		add_sound_object(get_index_for_sound_type_name("alert1"), 0, 0, 1);
#endif
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
	quest_entries[active_entries[entry]].deleted = false;
	need_to_save = true;
}


//	Mark the current entry as not deleted.
//
static void delete_entry(size_t entry)
{
	quest_entries[active_entries[entry]].deleted = true;
	need_to_save = true;
}


//	Enable/disable menu options as required....
//
static void cm_questlog_pre_show_handler(window_info *win, int widget_id, int mx, int my, window_info *cm_win)
{
	size_t over_entry = active_entries.size();
	for (std::vector<Shown_Entry>::const_iterator i=shown_entries.begin(); i!=shown_entries.end(); ++i)
		if (i->is_over(my))
		{
			over_entry = i->get_entry();
			break;
		}
	bool is_over_entry = (over_entry < active_entries.size());
	bool is_deleted = is_over_entry && quest_entries[active_entries[over_entry]].deleted;
	cm_grey_line(cm_questlog_id, CMQL_FILTER, quest_entries.empty() ?1 :0);
	cm_grey_line(cm_questlog_id, CMQL_SHOWALL, quest_entries.empty() ?1 :0);
	cm_grey_line(cm_questlog_id, CMQL_SHOWNONE, quest_entries.empty() ?1 :0);
	cm_grey_line(cm_questlog_id, CMQL_COPY, (is_over_entry && !is_deleted) ?0 :1);
	cm_grey_line(cm_questlog_id, CMQL_COPYALL, active_entries.empty() ?1 :0);
	cm_grey_line(cm_questlog_id, CMQL_FIND, (current_action == -1 && !active_entries.empty()) ?0 :1);
	cm_grey_line(cm_questlog_id, CMQL_ADD, (current_action == -1) ?0 :1);	
	cm_grey_line(cm_questlog_id, CMQL_DELETE, (is_over_entry && !is_deleted) ?0 :1);
	cm_grey_line(cm_questlog_id, CMQL_UNDEL, (is_over_entry && is_deleted) ?0 :1);
	cm_grey_line(cm_questlog_id, CMQL_SAVE, (need_to_save) ?0 :1);
	// propagate opacity from parent tab window
	if (tab_stats_win >-1 && tab_stats_win<windows_list.num_windows)
		cm_win->opaque = windows_list.window[tab_stats_win].opaque;
}


//	Call options selected from the context menu.
//
static int cm_quest_handler(window_info *win, int widget_id, int mx, int my, int option)
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
		case CMQL_FILTER: open_filter_window(); break;
		case CMQL_SHOWALL:
			for (std::map<std::string,int>::iterator i = filter_map.begin(); i != filter_map.end(); ++i)
				i->second = 1;
			rebuild_active_entries((current_line < active_entries.size()) ?active_entries[current_line] :0);
			break;
		case CMQL_SHOWNONE:
			for (std::map<std::string,int>::iterator i = filter_map.begin(); i != filter_map.end(); ++i)
				i->second = 0;
			rebuild_active_entries((current_line < active_entries.size()) ?active_entries[current_line] :0);
			open_filter_window();
			break;		
		case CMQL_COPY: if (over_entry < active_entries.size()) copy_entry(over_entry); break;
		case CMQL_COPYALL: copy_all_entries(); break;
		case CMQL_FIND: find_in_entry(win); break;
		case CMQL_ADD: add_entry(win, over_entry); break;
		case CMQL_DELETE: if (over_entry < active_entries.size()) delete_entry(over_entry); break;
		case CMQL_UNDEL: if (over_entry < active_entries.size()) undelete_entry(over_entry); break;
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

	if (show_help_text && mouse_over_questlog && (current_action == -1))
	{
		show_help(questlog_cm_help_str, 0, win->len_y + 10);
		mouse_over_questlog = false;
	}
#endif

	int questlog_y = 0, start_y = 0;
	shown_entries.clear();
	for (std::vector<Quest_Entry>::size_type entry = current_line; entry<active_entries.size(); entry++)
	{
		start_y = questlog_y;
		const std::vector<std::string> &lines = quest_entries[active_entries[entry]].get_lines();
		for (std::vector<std::string>::const_iterator line = lines.begin(); line != lines.end(); ++line)
		{
			draw_string_small (2, questlog_y, reinterpret_cast<const unsigned char *>(line->c_str()), 1);
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
//  Note:
//	Rebuilding the active_entry list must be done by the caller after this.
//
static void add_questlog_line(const char *t, const char *npcprefix)
{	
	quest_entries.push_back(Quest_Entry());
	if (strlen(npcprefix))
		quest_entries.back().set(std::string(t), std::string(npcprefix));
	else
		quest_entries.back().set(std::string(t));
}


extern "C" void goto_questlog_entry(int ln)
{
	if(ln <= 0)
		current_line = 0;
	else if (static_cast<size_t>(ln) >= active_entries.size())
		current_line = active_entries.size() - 1;
	else
		current_line = ln;
	vscrollbar_set_pos(questlog_win, quest_scroll_id, current_line);
}


//	Called when a new quest entry is received from the server.  Add to
//	the end of the entries and append to the questlog file immediately.
//
extern "C" void add_questlog (char *t, int len)
{
	t[len] = '\0';
	add_questlog_line(t, (const char*)npc_name);
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
}


//	Load the questlog, use the playname tag file but fall back to the
//	old file name if the new does not exist.  If reading from an old
//	named file, write the new file as we go.
//
extern "C" void load_questlog()
{
	// If the quest log is already loaded, just make sure we're saved.
	// This will take place when relogging after disconnection.
	if (!quest_entries.empty())
	{
		save_questlog();
		return;
	}
	
	std::string username = std::string(username_str);
	std::transform(username.begin(), username.end(), username.begin(), tolower);
	filename = std::string(get_path_config()) + "quest_" + username + ".log";

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
			add_questlog_line(line.c_str(), "");
			if (out)
				quest_entries.back().save(out);
		}
	}
	
	rebuild_active_entries(quest_entries.size()-1);
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
	size_t last_entry = active_entries.size()-1;

	set_window_handler(questlog_win, ELW_HANDLER_DISPLAY, (int (*)())display_questlog_handler);
	set_window_handler(questlog_win, ELW_HANDLER_CLICK, (int (*)())questlog_click);
	set_window_handler(questlog_win, ELW_HANDLER_MOUSEOVER, (int (*)())&mouseover_questlog_handler );

	quest_scroll_id = vscrollbar_add_extended (questlog_win, quest_scroll_id, NULL, STATS_TAB_WIDTH - 20, boxlen, 20, STATS_TAB_HEIGHT - boxlen, 0, 1.0, 0.77f, 0.57f, 0.39f, last_entry, 1, last_entry);
	goto_questlog_entry(last_entry);
	
	widget_set_OnClick (questlog_win, quest_scroll_id, (int (*)())questlog_scroll_click);
	widget_set_OnDrag (questlog_win, quest_scroll_id, (int (*)())questlog_scroll_drag);

#ifdef CONTEXT_MENUS
	if (!cm_valid(cm_questlog_id))
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
