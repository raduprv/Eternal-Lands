/*
	Provide item description and emu lookup from a data file.

	Uses the file item_info.txt, stored in the datadir or updates
	directory which is read when first needed.

	Functions provide descriptions and emu for items based on their image
	and unique id.  If unique id are not enabled (#item_uid) and so only
	image id are used then the values may not be unique.  The get_item_count()
	function allows you to check for uniqueness.

	The description, emu and count functions all cached last result to speed
	up the look-up, otherwise, the return time is dependant on the position
	in the list (but still not that slow).

	Author bluap/pjbroad February 2013
*/
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <functional>
#include <algorithm>

#include "client_serv.h"
#include "elloggingwrapper.h"
#include "init.h"
#include "item_info.h"
#include "items.h"
#include "io/elpathwrapper.h"
#include "knowledge.h"
#include "translate.h"
#include "text.h"
#include "url.h"

namespace Item_Info
{
	//	Class for a single item, holding ids, emu and description
	//
	class Item
	{
		public:
			Item(const std::string &text);
			~Item(void) {}
			bool is_valid(void) const { return valid; }
			const std::string &get_description(void) const { return description; }
			int get_emu(void) const { return emu; }
			enum EQUIP_TYPE get_equip_type(void) const { return equip_type; }
			void set_equip_type(std::string &text);
			bool compare(Uint16 the_item_id, int the_image_id) const;
			void set_knowledge_reference(size_t index) { knowledge_reference = index; }
			size_t get_knowledge_reference(void) const { return knowledge_reference; }
		private:
			static void trim_text(std::string &text);
			Uint16 item_id;
			int image_id;
			int emu;
			std::string description;
			bool valid;
			enum EQUIP_TYPE equip_type;
			size_t knowledge_reference;
	};


	//	Helper function to trim text
	void Item::trim_text(std::string &the_string)
	{
		// thanks http://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
		the_string.erase(the_string.begin(), std::find_if(the_string.begin(), the_string.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
		the_string.erase(std::find_if(the_string.rbegin(), the_string.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), the_string.end());
	}


	//	Construct the item by parsing the line from the item_info.txt file
	//
	Item::Item(const std::string &text)
		: valid(false), equip_type(EQUIP_NONE), knowledge_reference(KNOWLEDGE_LIST_SIZE)
	{
		std::stringstream ss(text);
		std::vector<std::string> fields;
		std::string field;
		while(std::getline(ss, field, '|'))
			fields.push_back(field);
		if (fields.size() != 4)
			return;
		item_id = std::stoi(fields[0]);
		image_id = std::stoi(fields[1]);
		emu = std::stoi(fields[2]);
		description = fields[3];
		trim_text(description);
		if (description.empty())
			return;
		valid = true;
	}


	//	Return true of the item matches the ids, allowing for unset unique ids
	//
	bool Item::compare(Uint16 the_item_id, int the_image_id) const
	{
		if ((the_item_id == unset_item_uid) && (the_image_id == image_id))
			return true;
		if ((the_item_id == item_id) && (the_image_id == image_id))
			return true;
		if ((the_item_id == item_id) && (the_image_id == -1))
			return true;
		return false;
	}


	//	Set the item equipment type, converting the string to the EQUIP_TYPE type
	//
	void Item::set_equip_type(std::string &type_text)
	{
		typedef struct { const char *str; enum EQUIP_TYPE equip_type; } string_to_type;
		string_to_type table[] =
			{	{"HEAD", EQUIP_HEAD}, {"BODY", EQUIP_BODY}, {"LEGS", EQUIP_LEGS},
				{"FEET", EQUIP_FEET}, {"NECK", EQUIP_NECK}, {"RIGHT_HAND", EQUIP_RIGHT_HAND},
				{"LEFT_HAND", EQUIP_LEFT_HAND}, {"BOTH_HANDS", EQUIP_BOTH_HANDS}, {"CLOAK", EQUIP_CLOAK} };
		trim_text(type_text);
		std::transform(type_text.begin(), type_text.end(), type_text.begin(), toupper);
		if (type_text.empty())
			return;
		for (size_t i = 0; i < sizeof(table)/sizeof(string_to_type); i++)
			if (std::string(table[i].str) == type_text)
			{
				equip_type = table[i]. equip_type;
				return;
			}
	}


	//	Class to hold the list of items
	//
	class List
	{
		public:
			List(void) : load_tried(false), shown_help(false), last_item(0) {}
			~List(void);
			const std::string &get_description(Uint16 item_id, int image_id, bool return_basic);
			int get_emu(Uint16 item_id, int image_id);
			enum EQUIP_TYPE get_equip_type(Uint16 item_id, int image_id);
			int get_count(Uint16 item_id, int image_id);
			bool info_available(void) { if (!load_tried) load(); return !the_list.empty(); }
			void help_if_needed(void);
			void filter_by_description(Uint8 *storage_items_filter, const ground_item *storage_items, const char *filter_item_text, int no_storage);
		private:
			Item *get_item(Uint16 item_id, int image_id);
			void open_file(std::ifstream &in, const char *filename);
			void load(void);
			void load_extra(const char *filename, int version, void processor(List *object, std::vector<std::string> &fields));
			static void equip_type_processor(List *object, std::vector<std::string> &fields);
			static void knowledge_processor(List *object, std::vector<std::string> &fields);
			std::vector<Item *> the_list;
			std::string description_plus;
			static std::string empty_str;
			static const char *item_info_filename;
			bool load_tried, shown_help;
			Item *last_item;
			class Count
			{
				public:
					Count(void) : count(-1) {}
					bool matches(Uint16 item_id, int image_id) const
						{ return ((count >= 0) && (this->item_id == item_id) && (this->image_id == image_id)); }
					int get_count(void) const { return count; }
					void set(Uint16 item_id, int image_id, int count)
						{ this->item_id = item_id; this->image_id = image_id; this->count = count; }
				private:
					Uint16 item_id; int image_id; int count;
			};
			Count last_count;

	};


	std::string List::empty_str;
	const char * List::item_info_filename = "item_info.txt";


	//	Clean up memory on exit
	//
	List::~List(void)
	{
		for (size_t i=0; i<the_list.size(); ++i)
			delete the_list[i];
		the_list.clear();
	}


	//	Find and item by the ids
	//
	Item *List::get_item(Uint16 item_id, int image_id)
	{
		info_available();
		if (last_item && last_item->compare(item_id, image_id))
			return last_item;
		for (size_t i=0; i<the_list.size(); ++i)
			if (the_list[i]->compare(item_id, image_id))
			{
				last_item = the_list[i];
				return last_item;
			}
		return 0;
	}


	//	Get the description for the specified ids, or return an empty string
	//
	const std::string & List::get_description(Uint16 item_id, int image_id, bool return_basic)
	{
		Item *matching_item = get_item(item_id, image_id);
		if (matching_item)
		{
			if (!return_basic && matching_item->get_knowledge_reference() < KNOWLEDGE_LIST_SIZE)
			{
				// add an indication for books whether they are read, unread or being read (reading)
				description_plus = matching_item->get_description() + get_knowledge_state_tag(matching_item->get_knowledge_reference());
				return description_plus;
			}
			return matching_item->get_description();
		}
		return empty_str;
	}


	//	Get the emu for the specified ids, or return -1
	//
	int List::get_emu(Uint16 item_id, int image_id)
	{
		Item *matching_item = get_item(item_id, image_id);
		if (matching_item)
			return matching_item->get_emu();
		return -1;
	}


	//	Get the equipment type for the specified ids, or return EQUIP_NONE
	//
	enum EQUIP_TYPE List::get_equip_type(Uint16 item_id, int image_id)
	{
		Item *matching_item = get_item(item_id, image_id);
		if (matching_item)
			return matching_item->get_equip_type();
		return EQUIP_NONE;
	}


	//	Return the number of unique items matching the ids
	//
	int List::get_count(Uint16 item_id, int image_id)
	{
		info_available();
		if (last_count.matches(item_id, image_id))
			return last_count.get_count();
		int match_count = 0;
		for (size_t i=0; i<the_list.size(); ++i)
			if (the_list[i]->compare(item_id, image_id))
				match_count++;
		last_count.set(item_id, image_id, match_count);
		return last_count.get_count();
	}

	//	Match passed string against specified item descriptions and return details for matches
	//	Element in results array set to zero if their description matches the passed string
	//
	void List::filter_by_description(Uint8 *storage_items_filter, const ground_item *storage_items, const char *filter_item_text, int no_storage)
	{
		if (!info_available() || (no_storage<=0) || !storage_items_filter || !storage_items)
			return;
		std::string needle(filter_item_text);
		std::transform(needle.begin(), needle.end(), needle.begin(), tolower);
		for (size_t i=0; i<static_cast<size_t>(no_storage); i++)
		{
			storage_items_filter[i] = 1;
			Item *item = get_item(storage_items[i].id, storage_items[i].image_id);
			if (item)
			{
				std::string haystack(item->get_description());
				std::transform(haystack.begin(), haystack.end(), haystack.begin(), tolower);
				if (haystack.find(needle) != std::string::npos)
					storage_items_filter[i] = 0;
			}
		}
	}

	//	Read lines from the item_info.txt file and create item objects
	//
	void List::load(void)
	{
		load_tried = true;
		std::ifstream in;
		open_file(in, item_info_filename);
		if (!in)
			return;
		std::string line;
		while (std::getline(in, line))
		{
			Item *new_item = new Item(line);
			if (new_item->is_valid())
				the_list.push_back(new_item);
			else
				delete new_item;
		}
		load_extra("item_extra_info.txt", 1, equip_type_processor);
		load_extra("item_knowledge_info.txt", 1, knowledge_processor);
	}


	//	Common function to read lines from the additional information files.
	//
	void List::load_extra(const char *filename, int version, void processor(List *object, std::vector<std::string> &fields))
	{
		std::ifstream in;
		open_file(in, filename);
		if (!in)
			return;
		std::string line;
		bool version_checked = false;
		while (std::getline(in, line))
		{
			if (!version_checked)
			{
				std::stringstream ss(line);
				int value = 0;
				ss >> value;
				if (value != version)
				{
					LOG_ERROR("invalid version number [%d] for [%s]\n", value, filename);
					return;
				}
				version_checked = true;
			}
			if (line.size() && line[0] == '#')
				continue;
			std::stringstream ss(line);
			std::vector<std::string> fields;
			std::string field;
			while(std::getline(ss, field, '|'))
				fields.push_back(field);
			processor(this, fields);
		}
	}


	//	For equipable items, load the type from the extra info file
	//	and set the type the item.  This allows item if the same type
	//	to be swapped by double clicking the item.
	//
	void List::equip_type_processor(List *object, std::vector<std::string> &fields)
	{
		if (fields.size() != 4)
			return;
		Uint16 item_id = std::stoi(fields[0]);
		int image_id = std::stoi(fields[1]);
		Item *matching_item = object->get_item(item_id, image_id);
		if (matching_item)
			matching_item->set_equip_type(fields[2]);
	}


	//	For items that are knowledge (i.e. books), load the mapping 
	//	between the item id and the book id. The book id is added to the
	//	item data.  When the description of a book is requested, we can
	//	then include if the book has been read or not.
	//
	void List::knowledge_processor(List *object, std::vector<std::string> &fields)
	{
		if (fields.size() != 3)
			return;
		int item_id = std::stoi(fields[1]);
		if (item_id >= 0)
		{
			size_t book_id = std::stoi(fields[0]);
			Item *matching_item = object->get_item(item_id, -1);
			if (matching_item)
				matching_item->set_knowledge_reference(book_id);
		}
	}


	//	Helper function to open a file from update or data directory
	//
	void List::open_file(std::ifstream &in, const char *filename)
	{
		std::string fname = std::string(get_path_updates()) + filename;
		in.open(fname.c_str());
		if (!in)
		{
			fname = std::string(datadir) + filename;
			in.clear();
			in.open(fname.c_str());
		}
	}


	//	If the item_info file is missing or item_uid not enabled, show one time help
	//
	void List::help_if_needed(void)
	{
		if (shown_help)
			return;
		if (!info_available())
		{
			std::string message = std::string(item_info_load_failed_str) + ": " + std::string(item_info_filename);
			LOG_TO_CONSOLE(c_red1, message.c_str());
		}
		if (!item_uid_enabled)
			LOG_TO_CONSOLE(c_red1, item_uid_help_str);
		shown_help = true;
	}

} // end Item_Info namespace

// Holds all the item information
static Item_Info::List the_list;

// The external interface
extern "C"
{
	int show_item_desc_text = 1;
	const char *get_item_description(Uint16 item_id, int image_id) { return the_list.get_description(item_id, image_id, false).c_str(); }
	const char *get_basic_item_description(Uint16 item_id, int image_id) { return the_list.get_description(item_id, image_id, true).c_str(); }
	void filter_items_by_description(Uint8 *storage_items_filter, const ground_item *storage_items, const char *filter_item_text, int no_storage)
		{ the_list.filter_by_description(storage_items_filter, storage_items, filter_item_text, no_storage); }
	int get_item_emu(Uint16 item_id, int image_id) { return the_list.get_emu(item_id, image_id); }
	enum EQUIP_TYPE get_item_equip_type(Uint16 item_id, int image_id) { return the_list.get_equip_type(item_id, image_id); }
	int get_item_count(Uint16 item_id, int image_id) { return the_list.get_count(item_id, image_id); }
	int item_info_available(void) { return ((the_list.info_available()) ?1: 0); }
	void item_info_help_if_needed(void) { the_list.help_if_needed(); }
}
