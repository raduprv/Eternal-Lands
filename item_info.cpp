/*
	Provide item description and emu lookup from a data file.

	Using the file from http://el.other-life.com/downloads/item_info.txt
	The file should be stored in the datadir or updates directory and is
	read when first needed.

	Functions provide descriptions and emu for items based on their image
	and unique id.  If unique id are not enable (#item_uid) and so only
	image id are used then the values may not be unique.  The get_item_count()
	function allows you to check for uniqueness.

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

#include "init.h"
#include "items.h"
#include "io/elpathwrapper.h"

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


	//	Return true of the item patches the ids, allowing for unset unique ids
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
			List(void) : load_tried(false) {}
			~List(void);
			const std::string &get_description(Uint16 item_id, int image_id);
			int get_emu(Uint16 item_id, int image_id);
			int get_count(Uint16 item_id, int image_id);
		private:
			void load(void);
			std::vector<Item *> the_list;
			static std::string empty_str;
			bool load_tried;
	};


	std::string List::empty_str;


	//	Glean up memory on exit
	//
	List::~List(void)
	{
		for (size_t i=0; i<the_list.size(); ++i)
			delete the_list[i];
		the_list.clear();
	}


	//	Get the description for the specified ids, or return an empty string
	//
	const std::string & List::get_description(Uint16 item_id, int image_id)
	{
		if (!load_tried)
			load();
		for (size_t i=0; i<the_list.size(); ++i)
			if (the_list[i]->compare(item_id, image_id))
				return the_list[i]->get_description();
		return empty_str;
	}


	//	Get the emu for the specified ids, or return -1
	//
	int List::get_emu(Uint16 item_id, int image_id)
	{
		if (!load_tried)
			load();
		for (size_t i=0; i<the_list.size(); ++i)
			if (the_list[i]->compare(item_id, image_id))
				return the_list[i]->get_emu();
		return -1;
	}


	//	Return the number of unique items matching the ids
	//
	int List::get_count(Uint16 item_id, int image_id)
	{
		if (!load_tried)
			load();
		size_t match_count = 0;
		for (size_t i=0; i<the_list.size(); ++i)
			if (the_list[i]->compare(item_id, image_id))
				match_count++;
		return match_count;
	}


	//	Read lines from the item_info.txt files and create item objects
	//
	void List::load(void)
	{
		load_tried = true;
		std::ifstream in;
		std::string fname = std::string(get_path_updates()) + std::string("item_info.txt");
		in.open(fname.c_str());
		if (!in)
		{
			fname = std::string(datadir) + std::string("item_info.txt");
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

} // end Item_Info namespace

static Item_Info::List the_list;

extern "C"
{
	const char *get_item_description(Uint16 item_id, int image_id) { return the_list.get_description(item_id, image_id).c_str(); }
	int get_item_emu(Uint16 item_id, int image_id) { return the_list.get_emu(item_id, image_id); }
	int get_item_count(Uint16 item_id, int image_id) { return the_list.get_count(item_id, image_id); }
}
