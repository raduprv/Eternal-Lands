/*
	Rewrite of quest log with new context menu features.
 
	Author bluap/pjbroad Feb 2010
*/

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
#include "interface.h"
#include "io/elpathwrapper.h"
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


//	Used to track requests for a quest title from the server
//
class Quest_Title_Request
{
	public:
		Quest_Title_Request(Uint16 req_id) : id(req_id), requested(false) {}
		Uint16 get_id(void) const { return id; }
		void request(void);
		bool been_requested(void) const { return requested; }
		bool is_too_old(void) const { return (abs(SDL_GetTicks() - request_time) > 5000); }
	private:
		Uint16 id;
		Uint32 request_time;
		bool requested;
};


//	Needed to sort quests with "Show all" first.
//
class QuestCompare {
	public:
		bool operator()(const Quest x, const Quest y) const
		{
			if (x.get_id() == Quest::UNSET_ID)
				return true;
			else if (y.get_id() == Quest::UNSET_ID)
				return false;
			else
				return x.get_id() < y.get_id();
		}
};


//	A list of Quests
//
class Quest_List
{
	public:
		Quest_List(void) : save_needed(false), iter_set(false), max_title(0),
			selected_id(Quest::UNSET_ID), highlighted_id(Quest::UNSET_ID),
			win_id(-1), scroll_id(0),
			mouseover_y(-1), clicked(false), spacer(3),
			linesep(static_cast<int>(SMALL_FONT_Y_LEN)+2*3),
			font_x(static_cast<int>(SMALL_FONT_X_LEN)), cm_id(CM_INIT_VALUE), 
			no_auto_open(0), hide_completed(0), list_left_of_entries(0), quest_completed(0), number_shown(0) {}
		void add(Uint16 id);
		void set_requested_title(const char* title);
		void showall(void);
		void load(void);
		void save(void);
		void set_completed(Uint16 id, bool is_complete);
		bool get_completed(Uint16 id) const;
		void toggle_completed(Uint16 id) { set_completed(id, !get_completed(id)); }
		size_t num_shown(void) const { return number_shown; }
		const Quest * get_first_quest(int offset);
		const Quest * get_next_quest(void);
		void set_selected(Uint16 id) { selected_id = id; }
		Uint16 get_selected(void) const { return selected_id; }
		size_t get_max_title(void) const { return max_title; }
		void open_window(void);
		int get_win_id(void) const { return win_id; }
		int get_scroll_id(void) const  { return scroll_id; }
		void scroll_to_selected(void);
		int get_mouseover_y(void) const { return mouseover_y; }
		void set_mouseover_y(int val) { mouseover_y = val; }
		bool has_mouseover(void) const { return mouseover_y != -1; }
		void clear_mouseover(void) { mouseover_y = -1; }
		bool was_clicked(void) const { return clicked; }
		bool set_clicked(bool val) { return clicked = val; }
		int get_spacer(void) const { return spacer; }
		int get_linesep(void) const { return linesep; }
		int get_font_x(void) const { return font_x; }
		bool auto_open_window(void) const { return !no_auto_open; }
		void recalc_num_shown(void);
		unsigned int get_options(void) const;
		void set_options(unsigned int options);
		void set_highlighted(Uint16 id) { highlighted_id = id; }
		Uint16 get_highlighted(void) const { return highlighted_id; }
		void clear_highlighted(void) { set_highlighted(Quest::UNSET_ID); }
		void cm_pre_show_handler(void);
		bool cm_active(void) const { return ((cm_id != CM_INIT_VALUE) && (cm_window_shown() == cm_id)); }
		void check_title_requests(void);
	private:
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
		const int spacer;
		const int linesep;
		const int font_x;
		size_t cm_id;
		// use logical sense so zero/false value is off
		int no_auto_open;
		int hide_completed;
		int list_left_of_entries;
		int quest_completed;
		size_t number_shown;
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

	std::string username = std::string(username_str);
	std::transform(username.begin(), username.end(), username.begin(), tolower);
	list_filename = std::string(get_path_config()) + "quest_" + username + ".list";
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



//	A single entry for the questlog.
class Quest_Entry
{
	public:
		Quest_Entry(void) : deleted(false), quest_id(Quest::UNSET_ID) {}
		void set(const std::string & the_text);
		void set(const std::string & the_text, const std::string & the_npc);
		const std::vector<std::string> & get_lines(void) const;
		void save(std::ofstream & out) const;
		bool contains_string(const char *text_to_find) const;
		const std::string & get_npc(void) const { return npc; }
		Uint16 get_charsum(void) const { return charsum; }
		void set_id(Uint16 id) { quest_id = id; }
		Uint16 get_id(void) const { return quest_id; }
		void set_deleted(bool is_deleted) { deleted = is_deleted; update_displayed_npc_name(); }
		bool get_deleted(void) const { return deleted; }
		const std::string & get_disp_npc(void) const { return disp_npc; };
	private:
		void set_lines(const std::string & the_text);
		void update_displayed_npc_name(void);
		bool deleted;
		std::vector<std::string> lines;
		static std::vector<std::string> deleted_line;
		static const std::string npc_spacer;
		std::string npc;
		std::string disp_npc;
		Uint16 quest_id;
		Uint16 charsum;
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
static std::vector<Quest_Entry> quest_entries;
static std::vector<size_t> active_entries;
static std::set<size_t> selected_entries;
static std::vector<Shown_Entry> shown_entries;
static std::map<std::string, int> filter_map;
static int quest_scroll_id = 14;
static std::string filename;
static size_t current_line = 0;
static bool need_to_save = false;
static bool mouse_over_questlog = false;
static Uint16 next_entry_quest_id = Quest::UNSET_ID;
static Quest_List questlist;
static enum { QLFLT_NONE=0, QLFLT_QUEST, QLFLT_NPC, QLFLT_SEL } active_filter = QLFLT_NONE;
static const int qlwinwidth = 580;
static const int qlwinheight = 350;
static const int qlborder = 5;
static size_t cm_questlog_id = CM_INIT_VALUE;
static size_t cm_questlog_over_entry = static_cast<size_t>(-1);
enum {	CMQL_SHOWALL=0, CMQL_QUESTFILTER, CMQL_NPCFILTER, CMQL_NPCSHOWNONE,
		CMQL_JUSTTHISNPC, CMQL_JUSTTHISQUEST, CMQL_S01,
		CMQL_COPY, CMQL_COPYALL, CMQL_FIND, CMQL_ADD, CMQL_S02,
		CMQL_SEL, CMQL_UNSEL, CMQL_SELALL, CMQL_UNSELALL, CMQL_SHOWSEL, CMQL_S03,
		CMQL_DELETE, CMQL_UNDEL, CMQL_S04, CMQL_DEDUPE, CMQL_S05, CMQL_SAVE };
enum {	CMQL_COMPLETED=0, CMQL_ADDSEL, CMQL_S11, CMQL_HIDECOMPLETED, CMQL_NOAUTOOPEN,  CMQL_LISTLEFTOFENTRIES };
static std::string adding_npc;
static size_t adding_insert_pos = 0;
static bool prompt_for_add_text = false;
static INPUT_POPUP ipu_questlog;
static int current_action = -1;

std::vector<std::string> Quest_Entry::deleted_line;
const std::string Quest_Entry::npc_spacer = ": ";

//	Create a fixed vector to use if deleted and a raw npc name / deleted string.
// 
void Quest_Entry::update_displayed_npc_name(void)
{
	if (deleted_line.empty())
		deleted_line.push_back(static_cast<char>(to_color_char(c_grey2)) + std::string(questlog_deleted_str));
	if (deleted)
		disp_npc = std::string(questlog_deleted_str);
	else
		disp_npc = std::string(npc + npc_spacer.substr(0, npc_spacer.size()-1));
}


//	Return the lines of an entry.
//
const std::vector<std::string> & Quest_Entry::get_lines(void) const
{
	if (!deleted)
		return lines;
	return deleted_line;
}


//	Write a quest entry as a single line to the output stream.
//
void Quest_Entry::save(std::ofstream & out) const
{
	for (std::vector<std::string>::size_type i=0; i<lines.size(); i++)
	{
		out.write(lines[i].c_str(), lines[i].size());
		if (i<lines.size()-1)
			out.put(' ');
		else if (quest_id == Quest::UNSET_ID)
			out.put('\n');
	}
	if (quest_id != Quest::UNSET_ID)
		out << "<<" << quest_id << ">>" << std::endl;
}


//	Store entry as a group of lines that fit into the window width.
//
void Quest_Entry::set_lines(const std::string & the_text)
{
	int chars_per_line = static_cast<int>((qlwinwidth - 2*qlborder - ELW_BOX_SIZE) / SMALL_FONT_X_LEN);
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

	// strip any line breaks from the end of the line
	while (!text.empty() && (*(text.end()-1) == '\r'))
		text.erase(text.end()-1, text.end());

	// look for and <<number>> at the end of the text, its the quest id
	std::string::size_type close = text.rfind(">>");
	if (close == text.size()-2)
	{
		std::string::size_type open = text.rfind("<<");
		if (open != std::string::npos)
		{
			std::string id_str = text.substr(open+2,close-open-2);
			if ((id_str.size()>0) && (id_str.find_first_not_of("0123456789") == std::string::npos))
			{
				quest_id = static_cast<Uint16>(atoi(id_str.c_str()));
				questlist.add(quest_id);
				text.erase(open,close);
			}
		}
	}

	// for matching purposes calculate the sum of the characters, if it wraps, fine
	charsum = 0;
	for (std::string::size_type i=0; i<text.size(); i++)
		charsum += text[i];

	// divide text into lines that fit within the window width
	for (std::string::size_type i=0; i<text.size(); i++)
	{
		if (is_color(text[i]))
			continue;
		if (text[i] == ' ')
			last_space = i;
		if (col >= chars_per_line)
		{
			if (last_space<=start)
			{
				lines.push_back(text.substr(start, chars_per_line));
				start += chars_per_line;
			}
			else
			{
				lines.push_back(text.substr(start, last_space-start));
				start = last_space+1;
			}
			col = i - start;
		}
		col++;
	}

	// catch the last line
	if (start < text.size())
		lines.push_back(text.substr(start, text.size()-start));

	update_displayed_npc_name();

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
void Quest_Entry::set(const std::string & the_text_const)
{
	if (the_text_const.empty())
		return;
	std::string the_text = the_text_const;

	// find any quest id - a mistake!
	// adding at the start was a mistake as it messes things up for the old client
	// moved to end of the text but keep this check for now for the early adopters!
	if (the_text[0] == '<')
	{
		std::string::size_type id_end = the_text.find_first_of('>', 1);
		if (id_end != std::string::npos)
		{
			std::string id_str = the_text.substr(1,id_end-1);
			quest_id = static_cast<Uint16>(atoi(id_str.c_str()));
			the_text.erase(0, id_end+1);
		}
		// make sure we save to the new format
		need_to_save = true;
	}

	// find and npc name
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
		bool useit = false;
		switch (active_filter)
		{
			case QLFLT_NONE: useit = true; break;
			case QLFLT_NPC: if (filter_map[quest_entries[entry].get_npc()]) useit = true; break;
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
		if (!entry->get_deleted())
			entry->save(out);
	need_to_save = false;

	LOG_DEBUG("Wrote questlog to file '%s'", filename.c_str());
}


//	Reset the filters and active lists so that all entries are shown.
//
void show_all_entries(void)
{
	active_filter = QLFLT_NONE;
	// set all NPC entries
	for (std::map<std::string,int>::iterator i = filter_map.begin(); i != filter_map.end(); ++i)
		i->second = 1;
	// clean a quest filter
	questlist.set_selected(Quest::UNSET_ID);
	rebuild_active_entries((current_line < active_entries.size()) ?active_entries[current_line] :0);
}


//	Draw a simple line
//
static void draw_underline(int startx, int starty, int endx, int endy)
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


//	Draw a context menu like hightlight using the supplied coords.
//
void draw_highlight(int topleftx, int toplefty, int widthx, int widthy, size_t col)
{
	float colours[2][2][3] = { { {0.11f, 0.11f, 0.11f }, {0.77f, 0.57f, 0.39f} },
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
static int quest_filter_win = -1;


//	Make sure the window size is fits the rows/cols nicely.
//
static int resize_quest_filter_handler(window_info *win, int new_width, int new_height)
{
		// let the width lead
		npc_name_cols = static_cast<int>(win->len_x / max_npc_name_x);
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
		set_window_scroll_len(win->window_id, static_cast<int>(npc_name_rows*max_npc_name_y-win->len_y));
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
		int new_width = static_cast<int>(2*npc_name_border + npc_name_cols * max_npc_name_x + ELW_BOX_SIZE);
		int new_rows = static_cast<int>((win->len_y+max_npc_name_y/2)/max_npc_name_y);
		int max_rows = static_cast<int>((filter_map.size() + npc_name_cols - 1) / npc_name_cols);
		resizing = 0;
		resize_window (win->window_id, new_width, static_cast<int>(((new_rows > max_rows) ?max_rows :new_rows)*max_npc_name_y));
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
			draw_highlight(posx, posy, static_cast<int>(0.5+max_npc_name_x), static_cast<int>(0.5+max_npc_name_y), 0);

		// set the colour and position for the box and text
		if ((active_filter != QLFLT_NPC) && (active_filter != QLFLT_NONE))
			glColor3f(0.7f, 0.7f, 0.7f);
		else
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
			do_click_sound();
			i->second ^= 1;
			active_filter = QLFLT_NPC;
			questlist.set_selected(Quest::UNSET_ID);
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
			set_window_scroll_pos(win->window_id, static_cast<int>(static_cast<int>(line/npc_name_cols)*max_npc_name_y));
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
	int yoffset = get_window_scroll_pos(win->window_id);
	if ((my >= yoffset) && (mx >= 0) && (mx < (npc_name_cols * max_npc_name_x)))
		quest_filter_active_npc_name = static_cast<int>(my / max_npc_name_y) * npc_name_cols + static_cast<int>(mx / max_npc_name_x);
	return 0; // make sure we get a arrow cursor
}


//	Open/Create the NPC list filter window.
//
static void open_filter_window(void)
{
	if (quest_filter_win < 0)
	{
		window_info *win = &windows_list.window[questlog_win];
		int min_x = static_cast<int>(2*npc_name_border + min_npc_name_cols * max_npc_name_x + ELW_BOX_SIZE);
		int min_y = static_cast<int>(min_npc_name_rows * max_npc_name_y);
		quest_filter_win = create_window(questlog_npc_filter_title_str, questlog_win, 0, win->len_x + 10, 0,
			min_x, static_cast<int>(static_cast<int>(win->len_y/max_npc_name_y)*max_npc_name_y),
			ELW_SCROLLABLE|ELW_RESIZEABLE|ELW_WIN_DEFAULT);
		set_window_handler(quest_filter_win, ELW_HANDLER_DISPLAY, (int (*)())&display_quest_filter_handler );
		set_window_handler(quest_filter_win, ELW_HANDLER_CLICK, (int (*)())&click_quest_filter_handler );
		set_window_handler(quest_filter_win, ELW_HANDLER_KEYPRESS, (int (*)())&keypress_quest_filter_handler );
		set_window_handler(quest_filter_win, ELW_HANDLER_MOUSEOVER, (int (*)())&mouseover_quest_filter_handler );
		set_window_handler(quest_filter_win, ELW_HANDLER_RESIZE, (int (*)())&resize_quest_filter_handler );
		set_window_min_size(quest_filter_win, min_x, min_y);
		set_window_scroll_inc(quest_filter_win, static_cast<int>(max_npc_name_y));
		resize_quest_filter_handler(&windows_list.window[quest_filter_win], -1, -1);
	}
	else
		show_window(quest_filter_win);
	set_window_scroll_pos(quest_filter_win, 0);
}


//	Display the quest list, highlighing any selected and one with mouse over.
//
static int display_questlist_handler(window_info *win)
{
	const size_t used_x = 4*questlist.get_spacer() + ELW_BOX_SIZE;
	const size_t disp_lines = win->len_y / questlist.get_linesep();
	const size_t disp_chars = (win->len_x - used_x) / questlist.get_font_x();

	// if resizing wait until we stop
	static Uint8 resizing = 0;
	if (win->resized)
		resizing = 1;
	// once we stop, snap the window size to fix nicely
	else if (resizing)
	{
		size_t to_show = (disp_lines>questlist.num_shown()) ?questlist.num_shown() :disp_lines;
		size_t newy = static_cast<size_t>(0.5 + to_show * questlist.get_linesep());
		to_show = (disp_chars>questlist.get_max_title()) ?questlist.get_max_title() :disp_chars;
		size_t newx = static_cast<size_t>(0.5+questlist.get_font_x()*to_show+used_x);
		resizing = 0;
		resize_window (win->window_id, newx, newy);
	}

	// only show help and clear the highlighted quest if the context window is closed
	if (cm_window_shown() == CM_INIT_VALUE)
	{
		questlist.clear_highlighted();
		if (show_help_text && questlist.has_mouseover())
			show_help(questlog_cm_help_str, 0, win->len_y + 10);
	}

	// get the top line and then loop drawing all quests we can display
	vscrollbar_set_bar_len(win->window_id, questlist.get_scroll_id(), (questlist.num_shown()>disp_lines) ?questlist.num_shown()-disp_lines :0);
	int offset = (questlist.num_shown()>disp_lines) ?vscrollbar_get_pos(win->window_id, questlist.get_scroll_id()) :0;
	const Quest* thequest  = questlist.get_first_quest(offset);
	int posy = questlist.get_spacer();
	while ((thequest != 0) && (posy+questlist.get_linesep()-questlist.get_spacer() <= win->len_y))
	{
		const int hl_x = static_cast<int>(win->len_x - 2*questlist.get_spacer() - ELW_BOX_SIZE);
		// is this the quest the mouse is over?
		if (questlist.has_mouseover() && (cm_window_shown() == CM_INIT_VALUE) &&
			(posy+questlist.get_linesep()-questlist.get_spacer() > questlist.get_mouseover_y()))
		{
			questlist.set_highlighted(thequest->get_id());
			// draw highlight over active name
			draw_highlight(questlist.get_spacer(), posy-questlist.get_spacer(), hl_x, questlist.get_linesep(), 0);
			// if clicked, update the filter
			if (questlist.was_clicked())
			{
				if (thequest->get_id() == Quest::UNSET_ID)
				{
					show_all_entries();
					active_filter = QLFLT_QUEST;
				}
				else
				{
					active_filter = QLFLT_QUEST;
					questlist.set_selected(thequest->get_id());
					rebuild_active_entries(quest_entries.size()-1);
				}
				do_click_sound();
			}
			questlist.clear_mouseover();
		}
		// is this the selected title?
		if ((active_filter == QLFLT_QUEST) && (thequest->get_id() == questlist.get_selected()))
			draw_highlight(questlist.get_spacer(), posy-questlist.get_spacer(), hl_x, questlist.get_linesep(), 1);
		if (questlist.cm_active() && (questlist.get_highlighted() == thequest->get_id()))
			glColor3f(0.77f, 0.57f, 0.39f);
		// display comleted quests less prominently
		else if (thequest->get_completed())
			glColor3f(0.6f,0.6f,0.6f);
		else
			glColor3f(1.0f,1.0f,1.0f);
		// display the title, truncating if its too long for the window width
		if (thequest->get_title().size() > disp_chars)
		{
			std::string todisp = thequest->get_title().substr(0,disp_chars);
			draw_string_small(2*questlist.get_spacer(), posy, (const unsigned char*)todisp.c_str(), 1);
		}
		else if (thequest->get_title().empty())
			draw_string_small(2*questlist.get_spacer(), posy, (const unsigned char*)"???", 1);
		else
			draw_string_small(2*questlist.get_spacer(), posy, (const unsigned char*)thequest->get_title().c_str(), 1);
		thequest = questlist.get_next_quest();
		posy += questlist.get_linesep();
	}
	// reset mouse over and clicked
	questlist.clear_mouseover();
	questlist.set_clicked(false);
	return 1;
}


//	Mouse left-click select an quest, mouse wheel scroll window.
//
static int click_questlist_handler(window_info *win, int mx, int my, Uint32 flags)
{
	if (flags&ELW_WHEEL_UP)
		vscrollbar_scroll_up(win->window_id, questlist.get_scroll_id());
	else if(flags&ELW_WHEEL_DOWN)
		vscrollbar_scroll_down(win->window_id, questlist.get_scroll_id());
	else if(flags&ELW_LEFT_MOUSE)
		questlist.set_clicked(true);
	return 1;
}


//	Save the y position so the display handler knows which to highlight/select.
//
static int mouseover_questlist_handler(window_info *win, int mx, int my)
{
	if (my>=0)
		questlist.set_mouseover_y(my);
	return 0;
}


//	Handle window resize, keeping the scroll handler in place.
//
static int resize_questlist_handler(window_info *win, int new_width, int new_height)
{
	widget_move(win->window_id, questlist.get_scroll_id(), win->len_x-ELW_BOX_SIZE, ELW_BOX_SIZE);
	widget_resize(win->window_id, questlist.get_scroll_id(), ELW_BOX_SIZE, win->len_y-2*ELW_BOX_SIZE);
	return 0;
}


//	Handle option selection for the quest list context menu 
static int cm_questlist_handler(window_info *win, int widget_id, int mx, int my, int option)
{
	
	switch (option)
	{
		case CMQL_COMPLETED: questlist.toggle_completed(questlist.get_highlighted()); break;
		case CMQL_ADDSEL:
			for (std::set<size_t>::const_iterator i=selected_entries.begin(); i!=selected_entries.end(); ++i)
				quest_entries[*i].set_id(questlist.get_highlighted());
			need_to_save = true;
			rebuild_active_entries((current_line < active_entries.size()) ?active_entries[current_line] :0);
			break;
		case CMQL_HIDECOMPLETED: questlist.recalc_num_shown(); break;
	}
	return 1;
}


//	Adjust things just before the quest list context menu is opened
//
void Quest_List::cm_pre_show_handler(void)
{
	quest_completed = (get_completed(highlighted_id)) ?1 :0;
	cm_grey_line(cm_id, CMQL_COMPLETED, (highlighted_id == Quest::UNSET_ID) ?1 :0);
	cm_grey_line(cm_id, CMQL_ADDSEL, selected_entries.empty() ?1 :0);
}


//	Adjust things just before the quest list context menu is opened
//
static void cm_questlist_pre_show_handler(window_info *win, int widget_id, int mx, int my, window_info *cm_win)
{
	questlist.cm_pre_show_handler();
}


//	Scroll to show the currently selected quest in the quest list window.
//
void Quest_List::scroll_to_selected(void)
{
	Uint16 setected_id = questlist.get_selected();
	if (setected_id == Quest::UNSET_ID)
		return;
	const Quest* thequest = questlist.get_first_quest(0);
	for (int line_num = 0; thequest != 0; line_num++)
	{
		if (thequest->get_id() == setected_id)
		{
			vscrollbar_set_pos(questlist.get_win_id(), questlist.get_scroll_id(), line_num);
			break;
		}
		thequest = questlist.get_next_quest();
	}
}


//	Create or open the quest list window.
//
void Quest_List::open_window(void)
{
	if (win_id < 0)
	{
		window_info *win = &windows_list.window[questlog_win];
		const int min_size_x = font_x*15+ELW_BOX_SIZE+4*spacer;
		const int min_size_y = 5*linesep;
		const int size_x = font_x*get_max_title()+ELW_BOX_SIZE+4*spacer;
		int size_y, pos_x, pos_y;
		if (list_left_of_entries)
		{
			pos_x = -(size_x + 10);
			pos_y = 0; 
			size_y = linesep * static_cast<int>(win->len_y / linesep);
		}
		else
		{
			pos_x = (win->len_x-size_x)/2;
			pos_y = static_cast<int>(win->len_y+SMALL_FONT_Y_LEN+10+ELW_TITLE_HEIGHT);
			size_y = min_size_y;
		}
		win_id = create_window(questlist_filter_title_str, questlog_win, 0, pos_x, pos_y, size_x, size_y, ELW_WIN_DEFAULT|ELW_RESIZEABLE);
		set_window_handler(win_id, ELW_HANDLER_DISPLAY, (int (*)())&display_questlist_handler );
		set_window_handler(win_id, ELW_HANDLER_CLICK, (int (*)())&click_questlist_handler );
		set_window_handler(win_id, ELW_HANDLER_MOUSEOVER, (int (*)())&mouseover_questlist_handler );
		set_window_handler(win_id, ELW_HANDLER_RESIZE, (int (*)())&resize_questlist_handler );
		scroll_id = vscrollbar_add_extended(win_id, scroll_id, NULL, 
			size_x-ELW_BOX_SIZE, ELW_BOX_SIZE, ELW_BOX_SIZE, size_y-2*ELW_BOX_SIZE, 0,
			1.0, 0.77f, 0.57f, 0.39f, 0, 1, quests.size()-1);
		set_window_min_size(win_id, min_size_x, min_size_y);

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
	close_ipu(&ipu_questlog);
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
	close_ipu(&ipu_questlog);
	init_ipu(&ipu_questlog, questlog_win, 400, -1, 1024, 5, questlog_input_cancel_handler, questlog_add_text_input_handler);
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
	questlist.set_selected(Quest::UNSET_ID);
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
		do_alert1_sound();
}

//	Prompt for text to find.  The dialogue will not close when "OK"
//	pressed but will call the callback.  Use "Cancel" to close.
//
static void find_in_entry(window_info *win)
{
	if (current_action != -1)
		return;
	current_action = CMQL_FIND;
	close_ipu(&ipu_questlog);
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
	quest_entries[active_entries[entry]].set_deleted(false);
	need_to_save = true;
}


//	Mark the current entry as not deleted.
//
static void delete_entry(size_t entry)
{
	quest_entries[active_entries[entry]].set_deleted(true);
	need_to_save = true;
}

//	Look for matching entries and delete (mark as deleted) all but first
//
void delete_duplicates(void)
{
	int deleted_count = 0;
	LOG_TO_CONSOLE(c_green1, questlog_deldupe_start_str);
	std::multimap<Uint16,std::string> mm;

	for (std::vector<Quest_Entry>::iterator entry=quest_entries.begin(); entry!=quest_entries.end(); ++entry)
	{
		if (!entry->get_deleted())
		{
			// get the full text for the entry
			std::string the_text;
			for (std::vector<std::string>::const_iterator line = entry->get_lines().begin(); line != entry->get_lines().end(); ++line)
				the_text += *line;

			// see if the charsum has already been seen
			std::multimap<Uint16,std::string>::const_iterator iter = mm.find(entry->get_charsum());
			if (iter != mm.end())
			{
				// we have a charsum match so check the text, there may be more than one with this charsum
				std::multimap<Uint16,std::string>::const_iterator last = mm.upper_bound(entry->get_charsum());
				for ( ; iter != last; ++iter)
				{
					// if the text and the charsum match, mark the entry as deleted
					if (iter->second == the_text)
					{
						entry->set_deleted(true);
						need_to_save = true;
						deleted_count++;
						break;
					}
				}
			}

			// save - as either the charsum did not match a previous entry or the charsum did but the text didn't
			if (!entry->get_deleted())
    			mm.insert(std::pair<Uint16,std::string>(entry->get_charsum(), the_text));
		}
	}

	char message[80];
	safe_snprintf(message, sizeof(message), questlog_deldupe_end_str, mm.size(), deleted_count);
	LOG_TO_CONSOLE(c_green1, message);
}


//	Enable/disable menu options as required....
//
static void cm_questlog_pre_show_handler(window_info *win, int widget_id, int mx, int my, window_info *cm_win)
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
	bool nfw_open = get_show_window(quest_filter_win);
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
	cm_grey_line(cm_questlog_id, CMQL_SAVE, (need_to_save) ?0 :1);
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
		case CMQL_SHOWALL: show_all_entries(); break;
		case CMQL_QUESTFILTER: questlist.open_window(); break;
		case CMQL_NPCFILTER: open_filter_window(); break;
		case CMQL_JUSTTHISQUEST:
			if (over_entry < active_entries.size())
			{
				Uint16 the_id = quest_entries[active_entries[over_entry]].get_id();
				if (the_id != Quest::UNSET_ID)
				{
					active_filter = QLFLT_QUEST;
					questlist.set_selected(the_id);
					rebuild_active_entries(quest_entries.size()-1);
					questlist.scroll_to_selected();
				}
			}
			break;
		case CMQL_JUSTTHISNPC:
		case CMQL_NPCSHOWNONE:
			questlist.set_selected(Quest::UNSET_ID);
			active_filter = QLFLT_NPC;
			for (std::map<std::string,int>::iterator i = filter_map.begin(); i != filter_map.end(); ++i)
			{
				i->second = 0;
				if ((option == CMQL_JUSTTHISNPC) && (i->first == quest_entries[active_entries[over_entry]].get_npc()))
					i->second = 1;
			}
			rebuild_active_entries((current_line < active_entries.size()) ?active_entries[current_line] :0);
			open_filter_window();
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
				show_all_entries();
			selected_entries.clear();
			break;
		case CMQL_SHOWSEL:
			active_filter = QLFLT_SEL;
			rebuild_active_entries((current_line < active_entries.size()) ?active_entries[current_line] :0);
			break;
		case CMQL_DELETE: if (over_entry < active_entries.size()) delete_entry(over_entry); break;
		case CMQL_UNDEL: if (over_entry < active_entries.size()) undelete_entry(over_entry); break;
		case CMQL_DEDUPE: delete_duplicates(); break;
		case CMQL_SAVE: save_questlog(); break;
	}
	return 1;
}


//	Draw the window contents.
//
static int display_questlog_handler(window_info *win)
{
	// If required, call the next stage of a entry input.
	if (prompt_for_add_text)
		questlog_add_text_input(win);

	questlist.check_title_requests();

	if (cm_window_shown() == CM_INIT_VALUE)
	{
		cm_questlog_over_entry = active_entries.size();
		if (show_help_text && mouse_over_questlog && (current_action == -1))
		{
			show_help(questlog_cm_help_str, 0, win->len_y + 10);
			mouse_over_questlog = false;
		}
	}

	int questlog_y = qlborder;
	shown_entries.clear();
	for (std::vector<Quest_Entry>::size_type entry = current_line; entry<active_entries.size(); entry++)
	{
		int start_y = questlog_y;
		const std::vector<std::string> &lines = quest_entries[active_entries[entry]].get_lines();
		glColor3f(1.0f, 1.0f, 1.0f);
		for (std::vector<std::string>::const_iterator line = lines.begin(); line != lines.end(); ++line)
		{
			draw_string_small (qlborder+gx_adjust, questlog_y+gy_adjust, reinterpret_cast<const unsigned char *>(line->c_str()), 1);
			questlog_y += static_cast<int>(SMALL_FONT_Y_LEN);
			if (questlog_y+qlborder > qlwinheight - SMALL_FONT_Y_LEN)
				break;
		}
		glColor3f(0.7f, 0.7f, 1.0f);
		if ((cm_questlog_over_entry < active_entries.size()) && (entry == cm_questlog_over_entry))
		{
			glColor3f(0.77f, 0.57f, 0.39f);
			draw_string_small(qlborder+gx_adjust, start_y+gy_adjust,
				reinterpret_cast<const unsigned char *>(quest_entries[active_entries[entry]].get_disp_npc().c_str()), 1);
		}
		if (selected_entries.find(active_entries[entry]) != selected_entries.end())
			draw_underline(qlborder, start_y+static_cast<int>(SMALL_FONT_Y_LEN),
				qlborder+static_cast<int>(quest_entries[active_entries[entry]].get_disp_npc().size() * SMALL_FONT_X_LEN),
				start_y+static_cast<int>(SMALL_FONT_Y_LEN));
		shown_entries.push_back(Shown_Entry(entry, start_y, questlog_y));
		questlog_y += qlborder;
		if (questlog_y+qlborder > qlwinheight - SMALL_FONT_Y_LEN)
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


static int keypress_questlog_handler(window_info *win, int mx, int my, Uint32 key, Uint32 unikey)
{
	char keychar = tolower(static_cast<char>(unikey));
	if ((key == K_MARKFILTER) || (keychar=='/'))
	{
		find_in_entry(win);
		return 1;
	}
	return 0;
}
	

static int mouseover_questlog_handler(window_info *win, int mx, int my)
{
	if (my>=0)
		mouse_over_questlog = true;
	return 0;
}


static int show_questlog_handler(window_info *win)
{
	if (questlist.auto_open_window() && !get_show_window(questlist.get_win_id()))
		questlist.open_window();
	return 0;
}


//	Add a new entry to the end of the quest log.  The entry will either
//	have been read from the file or about to be appended to the end.
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

	if (waiting_for_questlog_entry())
	{
		quest_entries.back().set_id(next_entry_quest_id);
		clear_waiting_for_questlog_entry();
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
extern "C" void load_questlog()
{
	questlist.load();
	
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

	LOG_DEBUG("Read questlog from file '%s'", filename.c_str());

	rebuild_active_entries(quest_entries.size()-1);
}


//	The questlog will only need saving if we add, delete or undelete.
//
extern "C" void unload_questlog()
{
	save_questlog();
	questlist.save();
}


//	Create the questlog (possibly) inside a tabbed window.
//
extern "C" void fill_questlog_win ()
{
	set_window_handler(questlog_win, ELW_HANDLER_DISPLAY, (int (*)())display_questlog_handler);
	set_window_handler(questlog_win, ELW_HANDLER_CLICK, (int (*)())questlog_click);
	set_window_handler(questlog_win, ELW_HANDLER_MOUSEOVER, (int (*)())&mouseover_questlog_handler );
	set_window_handler(questlog_win, ELW_HANDLER_KEYPRESS, (int (*)())&keypress_questlog_handler );
	set_window_handler(questlog_win, ELW_HANDLER_SHOW, (int (*)())&show_questlog_handler );

	size_t last_entry = active_entries.size()-1;
	quest_scroll_id = vscrollbar_add_extended (questlog_win, quest_scroll_id, NULL, qlwinwidth - 20, ELW_BOX_SIZE, 20, qlwinheight - ELW_BOX_SIZE, 0, 1.0, 0.77f, 0.57f, 0.39f, last_entry, 1, last_entry);
	goto_questlog_entry(last_entry);
	
	widget_set_OnClick (questlog_win, quest_scroll_id, (int (*)())questlog_scroll_click);
	widget_set_OnDrag (questlog_win, quest_scroll_id, (int (*)())questlog_scroll_drag);

	if (!cm_valid(cm_questlog_id))
	{
		cm_questlog_id = cm_create(cm_questlog_menu_str, cm_quest_handler);
		cm_add_window(cm_questlog_id, questlog_win);
		cm_set_pre_show_handler(cm_questlog_id, cm_questlog_pre_show_handler);
		init_ipu(&ipu_questlog, -1, -1, -1, 1, 1, NULL, NULL);
	}
}


//	Create (or show existing) a stand alone quest log window.
//
extern "C" void display_questlog()
{
	if (questlog_win < 0)
	{
		int our_root_win = -1;
		if (!windows_on_top) {
			our_root_win = game_root_win;
		}
		questlog_win = create_window(tab_questlog,our_root_win, 0, questlog_menu_x, questlog_menu_y, qlwinwidth, qlwinheight, ELW_WIN_DEFAULT);
		fill_questlog_win ();
	}
	else
		show_window(questlog_win);
	if (questlist.auto_open_window() && !get_show_window(questlist.get_win_id()))
		questlist.open_window();
}


extern "C" void set_next_quest_entry_id(Uint16 id)
{
	//char buf[80];
	//safe_snprintf(buf, 80, "Received NEXT_NPC_MESSAGE_IS_QUEST with id=%d", id);
	//LOG_TO_CONSOLE(c_green1, buf);
	//if (waiting_for_questlog_entry())
	//	LOG_TO_CONSOLE(c_red2, "Previous NEXT_NPC_MESSAGE_IS_QUEST was unused");
	next_entry_quest_id = id;
	questlist.add(id);
}


// Return true if a NEXT_NPC_MESSAGE_IS_QUEST message has been received
// but has not yet been used.
//
extern "C" int waiting_for_questlog_entry(void)
{
	if (next_entry_quest_id != Quest::UNSET_ID)
		return 1;
	else
		return 0;
}


// Make sure we are not expecting a new questlog entry
//
extern "C" void clear_waiting_for_questlog_entry(void)
{
	next_entry_quest_id = Quest::UNSET_ID;
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


extern "C"
{
	int questlog_win=-1;
	int questlog_menu_x=150;
	int questlog_menu_y=70;
}
