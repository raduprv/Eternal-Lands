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
#include <algorithm>

#include <SDL/SDL_types.h>

#include "client_serv.h"
#include "init.h"
#include "items.h"
#include "io/elpathwrapper.h"
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
			const bool compare(Uint16 the_item_id, int the_image_id) const;
		private:
			Uint16 item_id;
			int image_id;
			int emu;
			std::string description;
			bool valid;
	};


	//	Construct the item by parsing the line from the item_info.txt file
	//
	Item::Item(const std::string &text)
		: valid(false)
	{
		std::stringstream ss(text);
		std::vector<std::string> fields;
		std::string field;
		while(std::getline(ss, field, '|'))
			fields.push_back(field);
		if (fields.size() != 4)
			return;
		item_id = atoi(fields[0].c_str());
		image_id = atoi(fields[1].c_str());
		emu = atoi(fields[2].c_str());
		description = fields[3];
		// thanks http://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
		description.erase(description.begin(), std::find_if(description.begin(), description.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
		description.erase(std::find_if(description.rbegin(), description.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), description.end());
		if (description.empty())
			return;
		valid = true;
	}


	//	Return true of the item matches the ids, allowing for unset unique ids
	//
	const bool Item::compare(Uint16 the_item_id, int the_image_id) const
	{
		if ((the_item_id == unset_item_uid) && (the_image_id == image_id))
			return true;
		if ((the_item_id == item_id) && (the_image_id == image_id))
			return true;
		return false;
	}


	//	Class to hold the list of items
	//
	class List
	{
		public:
			List(void) : load_tried(false), shown_help(false), last_item(0) {}
			~List(void);
			const std::string &get_description(Uint16 item_id, int image_id);
			int get_emu(Uint16 item_id, int image_id);
			int get_count(Uint16 item_id, int image_id);
			bool info_available(void) { if (!load_tried) load(); return !the_list.empty(); }
			void help_if_needed(void);
			void filter_by_description(Uint8 *storage_items_filter, const ground_item *storage_items, const char *filter_item_text, int no_storage);
		private:
			Item *get_item(Uint16 item_id, int image_id);
			void load(void);
			std::vector<Item *> the_list;
			static std::string empty_str;
			static std::string item_info_filename;
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
	std::string List::item_info_filename = "item_info.txt";


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
	const std::string & List::get_description(Uint16 item_id, int image_id)
	{
		Item *matching_item = get_item(item_id, image_id);
		if (matching_item)
			return matching_item->get_description();
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
		std::string fname = std::string(get_path_updates()) + item_info_filename;
		in.open(fname.c_str());
		if (!in)
		{
			fname = std::string(datadir) + item_info_filename;
			in.clear();
			in.open(fname.c_str());
			if (!in)
				return;
		}
		std::string line;
		while (std::getline(in, line))
		{
			Item *new_item = new Item(line);
			if (new_item->is_valid())
				the_list.push_back(new_item);
			else
				delete new_item;
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
			std::string message = "Could not load the item information file: " + item_info_filename;
			LOG_TO_CONSOLE(c_red1, message.c_str());
		}
		if (!item_uid_enabled)
			LOG_TO_CONSOLE(c_red1, "Use #item_uid (set to 1) to enable unique item information.");
		shown_help = true;
	}

} // end Item_Info namespace

// Holds all the item information
static Item_Info::List the_list;

// The external interface
extern "C"
{
	int show_item_desc_text = 1;
	const char *get_item_description(Uint16 item_id, int image_id) { return the_list.get_description(item_id, image_id).c_str(); }
	void filter_items_by_description(Uint8 *storage_items_filter, const ground_item *storage_items, const char *filter_item_text, int no_storage)
		{ the_list.filter_by_description(storage_items_filter, storage_items, filter_item_text, no_storage); }
	int get_item_emu(Uint16 item_id, int image_id) { return the_list.get_emu(item_id, image_id); }
	int get_item_count(Uint16 item_id, int image_id) { return the_list.get_count(item_id, image_id); }
	int item_info_available(void) { return ((the_list.info_available()) ?1: 0); }
	void item_info_help_if_needed(void) { the_list.help_if_needed(); }
}
