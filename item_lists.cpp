/*
 * User can store current inventory contents into a named list. When you
 * are near an open storage, you can fetch the listed items out of
 * storage driectly from the preview window.  The preview window also
 * provides a way to recored inventory configurations for set tasks such
 * as making steel bars at a mine. It was hoped that you would be able
 * to fetch the complete list of items in one go just by selecting the
 * list from the menu.  However, due to concerns with macroing, this may
 * never be allowed.
 *
 * Author bluap/pjbroad December 2009
 * 
 * TODO New features
 * 		Option to rename a list?
 *
 */

#ifdef CONTEXT_MENUS

#include <assert.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

//needed for notepad.h
#include <SDL.h>
#include "text.h"
#include "notepad.h"

#include "asc.h"
#include "context_menu.h"
#include "errors.h"
#include "font.h"
#include "gl_init.h"
#include "hud.h"
#include "init.h"
#include "io/elpathwrapper.h"
#include "items.h"
#include "item_lists.h"
#ifdef NEW_SOUND
#include "sound.h"
#endif
#include "storage.h"

// TODO Move these strings to translate module
const char *item_list_format_error = "Format error while reading item list.";
const char *item_list_save_error_str = "Failed to save the item category file.";
const char *item_list_learn_cat_str = "Note: storage categories need to be learnt by selecting each category.";
const char *item_list_cat_format_error_str = "Format error reading item categories.";
const char *item_list_version_error_str = "Item lists file is not compatible with client version.";
const char *item_list_empty_list_str = "No point saving an empty list.";
const char *cm_item_list_menu_str = "Save a new list\nDisable list preview\n--\nDelete a list\n--\nReload item lists file";
const char *cm_item_list_empty_str = "Empty";
const char *cm_item_list_selected_str = "Edit quantity\n--\nDelete";
const char *item_list_name_str = "Enter list name";
const char *item_list_preview_title = "List preview";
const char *item_list_use_help_str = "Use quantity - right-click";
const char *item_list_pickup_help_str = "Pick up - left-click";
const char *item_list_edit_help_str = "Edit menu - ctrl+right-click";
const char *item_list_add_help_str = "Add to list - ctrl+left-click";
const char *item_list_quantity_str = "Quantity";
const char *item_list_magic_str = "Magical interference caused the list preview to close O.O";


// proto types
static void quantity_input_handler(const char *input_text, void *);

namespace ItemLists
{
	//	A class for an individual item list.
	//
	class List
	{
		public:
			bool set(std::string save_name);
			void fetch(void) const;
			const std::string & get_name(void) const { return name; }
			size_t get_num_items(void) const { return image_ids.size(); }
			int get_image_id(size_t i) const { assert(i<image_ids.size()); return image_ids[i]; }
			Uint16 get_item_id(size_t i) const { assert(i<item_ids.size()); return item_ids[i]; }
			int get_quantity(size_t i) const  { assert(i<quantities.size()); return quantities[i]; }
			void set_quantity(size_t item, int quantity) { assert(item<quantities.size()); quantities[item] = quantity; }
			void write(std::ostream & out) const;
			bool read(std::istream & in);
			void del(size_t item_index);
			void add(int image_id, Uint16 id, int quantity);
		private:
			std::string name;
			std::vector<int> image_ids;
			std::vector<int> quantities;
			std::vector<Uint16> item_ids;
	};
	
	
	// TODO Fetch the items from the server.
	//
	void List::fetch(void) const
	{
		// do nothing for now
		// std::string message = "Fetch \"" + name + "\": Not yet supported:(";
		// LOG_TO_CONSOLE(c_green1, message.c_str());
	}
	
	
	// Set the name, ids and quantities for a new list.
	// If there is nothing in the inventory, then return
	// value is false, the caller should delete the object.
	//
	bool List::set(std::string save_name)
	{
		name = save_name;
		for (size_t i=0; i<ITEM_NUM_ITEMS-ITEM_NUM_WEAR; i++)
			if (item_list[i].quantity > 0)
			{
#ifdef ITEM_UID
				bool stacked_item = false;
				if (item_list[i].id != unset_item_uid)
					for (size_t j=0; j<item_ids.size(); j++)
						if (item_list[i].id == item_ids[j])
						{
							quantities[j]++;
							stacked_item = true;
							break;
						}
				if (!stacked_item)
				{
					image_ids.push_back(item_list[i].image_id);
					item_ids.push_back(item_list[i].id);
					quantities.push_back(item_list[i].quantity);
				}
#else
				image_ids.push_back(item_list[i].image_id);
				item_ids.push_back(unset_item_uid);
				quantities.push_back(item_list[i].quantity);
#endif
			}
		if (quantities.empty())
			return false;
		return true;
	}
	
	
	//	Write the list to the specified stream.
	//  The name, ids and quantities are on separate lines.
	//
	void List::write(std::ostream & out) const
	{
		out << name << std::endl;
		for (size_t i=0; out && i<image_ids.size(); i++)
			out << image_ids[i] << " ";
		out << std::endl;
		for (size_t i=0; out && i<quantities.size(); i++)
			out << quantities[i] << " ";
		out << std::endl;
		for (size_t i=0; out && i<item_ids.size(); i++)
			out << item_ids[i] << " ";
		out << std::endl;
	}
	
	
	//	Read an itemlist form the specified input stream
	//	If an error occurs the function will return false;
	// 	the caller should delete the object.
	//
	bool List::read(std::istream & in)
	{
		std::string name_line, image_id_line, cnt_line, item_uid_line;

		// each part is on a separate line, but allow empty lines
		while (getline(in, name_line) && name_line.empty());
		while (getline(in, image_id_line) && image_id_line.empty());
		while (getline(in, cnt_line) && cnt_line.empty());
		while (getline(in, item_uid_line) && item_uid_line.empty());

		// mop up extra lines at the end of the file silently
		if (name_line.empty())
			return false;

		// a name without data is not a list!
		if (image_id_line.empty() || cnt_line.empty() || item_uid_line.empty())
		{
			LOG_ERROR("%s: %s [%s]\n", __FILE__, item_list_format_error, name_line.c_str() );
			return false;
		}

		// read each image id value
		std::istringstream ss(image_id_line);
		int value = 0;
		while (ss >> value)
			image_ids.push_back(value);

		// read each quantity value
		ss.clear();
		ss.str(cnt_line);
		value = 0;
		while (ss >> value)
			quantities.push_back(value);

		// read each item uid value
		ss.clear();
		ss.str(item_uid_line);
		Uint16 ui_value = 0;
		while (ss >> ui_value)
			item_ids.push_back(ui_value);

		// don't use a list with unequal or empty data sets
		if ((quantities.size() != image_ids.size()) || (quantities.size() != item_ids.size()) || quantities.empty())
		{
			LOG_ERROR("%s: %s name=[%s] #id=%d #cnts=%d #uid=%d\n", __FILE__, item_list_format_error, name_line.c_str(), image_ids.size(), quantities.size(), item_ids.size() );
			return false;
		}

		// read list just fine
		name = name_line;
		return true;
	}


	// Delete the specified item from the list
	//
	void List::del(size_t item_index)
	{
		assert(item_index<quantities.size());
		image_ids.erase( image_ids.begin()+item_index );
		quantities.erase( quantities.begin()+item_index );
		item_ids.erase( item_ids.begin()+item_index );
	}


	//	Add a new item to a list or increase quantity if already in the list
	void List::add(int image_id, Uint16 id, int quantity)
	{
#ifdef NEW_SOUND
		add_sound_object(get_index_for_sound_type_name("Drop Item"), 0, 0, 1);
#endif // NEW_SOUND

		// Add to quanity if already in list

		// if new item has proper ID, try to use it
		if (id != unset_item_uid)
		{
			// match only against items with the same id
			for (size_t i=0; i<quantities.size(); ++i)
				if (item_ids[i] == id)
				{
					quantities[i] += quantity;
					return;
				}
			// or with items that don't have a proper id but do match the image
			for (size_t i=0; i<quantities.size(); ++i)
				if ((item_ids[i] == unset_item_uid) && (image_ids[i] == image_id))
				{
					quantities[i] += quantity;
					return;
				}
		}
		// if the new items does not have a proper id, just look for a matching image id
		else
		{
			for (size_t i=0; i<quantities.size(); ++i)
				if (image_ids[i] == image_id)
				{
					quantities[i] += quantity;
					return;
				}
		}

		// otherwise add new item
		image_ids.push_back(image_id);
		item_ids.push_back(id);
		quantities.push_back(quantity);
	}


	//	Store and lookup categories for objects.
	//
	class Category_Maps
	{
		public:
			Category_Maps(void) : must_save(false) {}
			void update(int image_id, Uint16 item_id, int cat_id);
			bool have_image_id(int image_id) const
				{ return cat_by_image_id.find(image_id) != cat_by_image_id.end(); }
			bool have_item_id(Uint16 item_id) const
				{ return cat_by_item_id.find(item_id) != cat_by_item_id.end(); }
			int get_cat(int image_id, Uint16 item_id);
			void save(void);
			void load(void);
		private:
			static const char *filename;
			std::map<int, int> cat_by_image_id;
			std::map<Uint16, int> cat_by_item_id;
			bool must_save;
			struct IDS { public: std::vector<int> images; std::vector<Uint16> items; };
	};

	const char * Category_Maps::filename = "item_categories.txt";

	//	If we don't already have the objects category, store it now.
	//
	void Category_Maps::update(int image_id, Uint16 item_id, int cat_id)
	{
		if ((item_id != unset_item_uid) && !have_item_id(item_id))
		{
			//std::cout << "Storing item id " << item_id << " cat " << cat_id << std::endl;
			cat_by_item_id[item_id] = cat_id;
			must_save = true;
		}
		if (!have_image_id(image_id))
		{
			//std::cout << "Storing image id " << image_id << " cat " << cat_id << std::endl;
			cat_by_image_id[image_id] = cat_id;
			must_save = true;
		}
	}


	//	Return the category for an item/image id, or -1 for not found.
	//
	int Category_Maps::get_cat(int image_id, Uint16 item_id)
	{
		if ((item_id != unset_item_uid) && have_item_id(item_id))
		{
			//std::cout << "Retrieving by item id " << item_id << std::endl;
			return cat_by_item_id[item_id];
		}
		else if (have_image_id(image_id))
		{
			//std::cout << "Retrieving by image id " << image_id << std::endl;
			return cat_by_image_id[image_id];
		}
		else
			return -1;
	}


	//	Save the object/category mappings.
	//	Grouped for each category to save space and allow easy image/item id mixing
	//
	void Category_Maps::save(void)
	{
		if (!must_save)
			return;

		std::string fullpath = get_path_config() + std::string(filename);
		std::ofstream out(fullpath.c_str());
		if (!out)
		{
			LOG_ERROR("%s: %s [%s]\n", __FILE__, item_list_save_error_str, fullpath.c_str() );
			LOG_TO_CONSOLE(c_red2, item_list_save_error_str);
			return;
		}

		// store the ids grouped by category
		std::map<int,IDS> ids_in_cat;
		for (std::map<int,int>::const_iterator i=cat_by_image_id.begin(); i!=cat_by_image_id.end(); ++i)
			ids_in_cat[i->second].images.push_back(i->first);
		for (std::map<Uint16,int>::const_iterator i=cat_by_item_id.begin(); i!=cat_by_item_id.end(); ++i)
			ids_in_cat[i->second].items.push_back(i->first);

		// step though each category, writing ids
		for (std::map<int,IDS>::const_iterator i=ids_in_cat.begin(); i!=ids_in_cat.end(); ++i)
		{
			// write category id
			out << i->first << std::endl;
			// write the number of image ids, then the values
			out << i->second.images.size() << " ";
			for (std::vector<int>::const_iterator j=i->second.images.begin(); j!=i->second.images.end(); ++j)
				out << *j << " ";
			out << std::endl;
			// write the number of item ids, then the values
			out << i->second.items.size() << " ";
			for (std::vector<Uint16>::const_iterator j=i->second.items.begin(); j!=i->second.items.end(); ++j)
				out << *j << " ";
			out << std::endl << std::endl;
		}
		
		must_save = false;
	}


	//	Load the object/category mappings.
	//
	void Category_Maps::load(void)
	{
		cat_by_image_id.clear();
		cat_by_item_id.clear();

		std::string fullpath = get_path_config() + std::string(filename);
		std::ifstream in(fullpath.c_str());
		if (!in)
			return;

		while (!in.eof())
		{
			// read the info, image_id and item_id lines
			std::string info_line, image_id_line, item_id_line;
			while (getline(in, info_line) && info_line.empty());
			getline(in, image_id_line);
			getline(in, item_id_line);
			if (info_line.empty())
				break;

			// read the category 
			std::istringstream ss(info_line);
			int category = -1;
			ss >> category;			

			// read and count the image id values and store in the map
			ss.clear();
			ss.str(image_id_line);
			int value = 0;
			int actual_num_image_ids = 0;
			int expected_num_image_ids = 0;
			ss >> expected_num_image_ids;
			while (ss >> value)
			{
				cat_by_image_id[value] = category;
				actual_num_image_ids++;
			}

			// read and count the item id values and store in the map
			ss.clear();
			ss.str(item_id_line);
			Uint16 ui_value = 0;
			int actual_num_item_ids = 0;
			int expected_num_item_ids = 0;
			ss >> expected_num_item_ids;
			while (ss >> ui_value)
			{
				cat_by_item_id[ui_value] = category;
				actual_num_item_ids++;
			}

			// check for format errors and end now if something detected
			if ((category<0) || (actual_num_image_ids != expected_num_image_ids) || (actual_num_item_ids != expected_num_item_ids))
			{
				LOG_TO_CONSOLE(c_red2, item_list_cat_format_error_str);
				LOG_ERROR("%s: %s cat=%d expected/actual image=%d/%d item %d/%d\n",
					__FILE__, item_list_cat_format_error_str, category,
					expected_num_image_ids, actual_num_image_ids,
					expected_num_item_ids, actual_num_item_ids );
				break;
			}
		}
		
		must_save = false;
	}


	//	Simple wrapper around the quantity input dialogue
	//
	class Quantity_Input
	{
		public:
			Quantity_Input(void) { init_ipu(&ipu, -1, 200, -1, 10, 1, NULL, quantity_input_handler); }
			size_t get_list(void) const { return list; };
			size_t get_item(void) const { return item; };
			void open(int parent_id, int mx, int my, size_t list, size_t item);
			~Quantity_Input(void) { close_ipu(&ipu); }
			void close(void) { if (get_show_window(ipu.popup_win)) clear_popup_window(&ipu); }
		private:
			INPUT_POPUP ipu;
			size_t list;
			size_t item;
	};


	//	Open the input quantity window
	//
	void Quantity_Input::open(int parent_id, int mx, int my, size_t list, size_t item)
	{
		this->list = list;
		this->item = item;
		ipu.x = mx;
		ipu.y = my;
		ipu.parent = parent_id;
		ipu.data = static_cast<void *>(this);
		display_popup_win(&ipu, item_list_quantity_str);
	}


	//	Class to contain a collection of item lists.
	//
	class List_Container
	{
		public:
			List_Container(void) : previewed(0) {}
			void load(void);
			void save(void);
			bool add(const char *name);
			void del(size_t list_index);
			size_t get_previewed(void) const { return previewed; }
			size_t size(void) const { return saved_item_lists.size(); }
			bool valid_preview(void) const { return previewed < size(); }
			void change_preview(Uint32 flags);
			void get_menu(std::string &str) const
				{ for (size_t i=0; i<size(); ++i) str += saved_item_lists[i].get_name() + "\n"; }
			bool set_previewed(size_t new_previewed)
				{ if (new_previewed >= size()) return false; previewed = new_previewed; return true; }
			void set_quantity(size_t item, int quantity)
				{ assert(valid_preview()); return saved_item_lists[previewed].set_quantity(item, quantity); }
			void del_item(size_t i)
				{ assert(valid_preview()); saved_item_lists[previewed].del(i); }
			void add_item(int image_id, Uint16 id, int quantity)
				{ assert(valid_preview()); saved_item_lists[previewed].add(image_id, id, quantity); }
			const List & get_list(void) const
				{ assert(valid_preview()); return saved_item_lists[previewed]; }
			void fetch(size_t i) const
				{ if (i<size()) saved_item_lists[i].fetch(); }
			void sort_list(void)
				{ std::sort( saved_item_lists.begin(), saved_item_lists.end(), List_Container::sort_compare); };
		private:
			std::vector<List> saved_item_lists;
			static int FILE_REVISION;
			size_t previewed;
			static const char * filename;
			static bool sort_compare(const List &a, const List &b);
	};

	int List_Container::FILE_REVISION = 2;
	const char * List_Container::filename = "item_lists.txt";

	//  Save the item lists to a file in players config directory
	//
	void List_Container::save(void)
	{
		std::string fullpath = get_path_config() + std::string(filename);
		std::ofstream out(fullpath.c_str());
		if (!out)
		{
			LOG_ERROR("%s: %s [%s]\n", __FILE__, item_list_save_error_str, fullpath.c_str() );
			LOG_TO_CONSOLE(c_red2, item_list_save_error_str);
			return;
		}
		out << FILE_REVISION << std::endl << std::endl;
		for (size_t i=0; i<saved_item_lists.size(); ++i)
		{
			saved_item_lists[i].write(out);
			out << std::endl;
		}
		out.close();
	}


	//  Load the item lists from the file in players config directory
	//
	void List_Container::load(void)
	{
		saved_item_lists.clear();
		std::string fullpath = get_path_config() + std::string(filename);
		std::ifstream in(fullpath.c_str());
		if (!in)
			return;
		int revision;
		in >> revision;
		if (revision != FILE_REVISION)
		{
			LOG_ERROR("%s: %s [%s]\n", __FILE__, item_list_version_error_str, fullpath.c_str() );
			return;
		}
		while (!in.eof())
		{
			saved_item_lists.push_back(ItemLists::List());
			if (!saved_item_lists.back().read(in))
				saved_item_lists.pop_back();
		}
		in.close();
	}


	//	Add a new list 
	//
	bool List_Container::add(const char *name)
	{
		saved_item_lists.push_back(List());
		if (saved_item_lists.back().set(name))
			return true;
		saved_item_lists.pop_back();
		return false;
	}


	//	Delete the specified list
	//
	void List_Container::del(size_t list_index)
	{
		assert(list_index < size());
		saved_item_lists.erase(saved_item_lists.begin()+list_index);
	}


	//	Change the current previewed list by mouse wheel
	//
	void List_Container::change_preview(Uint32 flags)
	{
		if ((flags & ELW_WHEEL_UP ) && previewed > 0)
			previewed--;
		else if ((flags & ELW_WHEEL_DOWN ) && previewed+1 < size())
			previewed++;
	}


	// Used by the sort algorithm to alphabetically compare two list names, case insensitive
	//
	bool List_Container::sort_compare(const List &a, const List &b)
	{
		std::string alower(a.get_name());
		std::transform(alower.begin(), alower.end(), alower.begin(), tolower);
		std::string blower(b.get_name());
		std::transform(blower.begin(), blower.end(), blower.begin(), tolower);
		return alower < blower;
	} 


	//	Class for static objects to avoid destructor issues
	//  Will be created after and destructed before global statics
	class Vars
	{
		public:
			static Quantity_Input * quantity_input(void)
				{ static Quantity_Input qi; return &qi; }
			static Category_Maps * cat_maps(void)
				{ static Category_Maps cm; return &cm; }
			static List_Container * lists(void)
				{ static List_Container lc; return &lc; }
	};

	
} // end ItemLists namespace



static INPUT_POPUP ipu_item_list_name;
static int delete_item_list = 0;
static int preview_win = -1;
static std::vector<const char *>help_str;
static int last_quantity_selected = 0;
static const int preview_grid_size = 33;
static size_t selected_item_number = static_cast<size_t>(1);
static const enum { OPTION_SAVE = 0, OPTION_PREVIEW, OPT_SEP01,
	OPTION_DELETE, OPT_SEP02, OPTION_RELOAD } cm_opts = OPTION_SAVE;
static size_t cm_selected_item_menu = CM_INIT_VALUE;
static Uint32 last_mod_time = 0;
Uint32 il_pickup_fail_time = 0;


//  When lists are added/removed, update the list in the menu.
//
static void update_list_window()
{
	ItemLists::Vars::lists()->sort_list();
	std::string menu_string;
	ItemLists::Vars::lists()->get_menu(menu_string);	
	if (menu_string.empty())
	{
		cm_set(cm_item_list_but, cm_item_list_empty_str, cm_item_list_handler);
		cm_grey_line(cm_item_list_but, 0, 1);
	}
	else
		cm_set(cm_item_list_but, menu_string.c_str(), cm_item_list_handler);
}


//  Draw the preview window, name, item image_ids + quantities & help text
//
static int display_preview_handler(window_info *win)
{
	if (!ItemLists::Vars::lists()->valid_preview())
		return 1;

	if (last_mod_time && abs(last_mod_time - SDL_GetTicks()) > 5000)
	{
		ItemLists::Vars::lists()->save();
		last_mod_time = 0;
	}

	glEnable(GL_TEXTURE_2D);

	// draw the images
	glColor3f(1.0f,1.0f,1.0f);
	for(size_t i=0; i<ItemLists::Vars::lists()->get_list().get_num_items(); i++)
	{
		int x_start, x_end, y_start, y_end;
		x_start = preview_grid_size * (i%6) + 1;
		x_end = x_start + preview_grid_size - 1;
		y_start = preview_grid_size * (i/6);
		y_end = y_start + preview_grid_size - 1;
		draw_item(ItemLists::Vars::lists()->get_list().get_image_id(i), x_start, y_start, preview_grid_size);
	}

	// draw the list name
	draw_string_small(4, preview_grid_size*6 + 4, (unsigned char*)ItemLists::Vars::lists()->get_list().get_name().c_str(), 1);

	// draw mouse over window help text
	if (show_help_text)
	{
		for (size_t i=0; i<help_str.size(); ++i)
			show_help(help_str[i], 0, static_cast<int>(0.5 + win->len_y + 10 + SMALL_FONT_Y_LEN * i));
		help_str.clear();
	}

	glDisable(GL_TEXTURE_2D);

	// draw the item grid
	glColor3f(0.77f,0.57f,0.39f);
	rendergrid(6, 6, 0, 0, preview_grid_size, preview_grid_size);

	// if an object is selected, draw a green grid around it
	if ((quantities.selected == ITEM_EDIT_QUANT) && (selected_item_number < ItemLists::Vars::lists()->get_list().get_num_items()))
	{
		int x_start = selected_item_number%6 * preview_grid_size;
		int y_start = static_cast<int>(selected_item_number/6) * preview_grid_size;
		if ((SDL_GetTicks() - il_pickup_fail_time) < 250)
			glColor3f(0.8f,0.2f,0.2f);
		else
			glColor3f(0.0f, 1.0f, 0.3f);
		rendergrid(1, 1, x_start, y_start, preview_grid_size, preview_grid_size);
		rendergrid(1, 1, x_start-1, y_start-1, preview_grid_size+2, preview_grid_size+2);
	}
	
	glEnable(GL_TEXTURE_2D);

	// draw the quantities over everything else so they always show
	glColor3f(1.0f,1.0f,1.0f);
	char str[80];
	for(size_t i=0; i<ItemLists::Vars::lists()->get_list().get_num_items(); i++)
	{
		int x_start, y_start, y_end;
		x_start = preview_grid_size * (i%6) + 1;
		y_start = preview_grid_size * (i/6);
		y_end = y_start + preview_grid_size - 1;
		safe_snprintf(str, sizeof(str), "%i", ItemLists::Vars::lists()->get_list().get_quantity(i));
		draw_string_small_shadowed(x_start, (i&1)?(y_end-15):(y_end-27), (unsigned char*)str, 1,1.0f,1.0f,1.0f, 0.0f, 0.0f, 0.0f);
	}

#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	
	return 1;
}


//	Switch back to a previous item quantity on the main window.
//
static void restore_inventory_quantity(void)
{
	if (quantities.selected == ITEM_EDIT_QUANT)
	{
		quantities.selected = last_quantity_selected;
		item_quantity=quantities.quantity[quantities.selected].val;
	}
	selected_item_number = static_cast<size_t>(-1);
}


//	Get the item number of the object under the mouse
//
static size_t get_preview_item_number(int mx, int my)
{
	size_t num_items = ItemLists::Vars::lists()->get_list().get_num_items();
	if ((my >= preview_grid_size*6) && (mx >= preview_grid_size*6))
		return num_items;
	size_t list_index = 6 * static_cast<int>(my/preview_grid_size) + mx/preview_grid_size;
	if (list_index < num_items)
		return list_index;
	return num_items;
}


//	Handle mouse clicks in the preview window
//
static int click_preview_handler(window_info *win, int mx, int my, Uint32 flags)
{
	if (my < 0) // don't respond here to title bar being clicked
		return 0;

	size_t last_selected = selected_item_number;

	// hide and clear any quantity input widow
	ItemLists::Vars::quantity_input()->close();

	size_t num_items = ItemLists::Vars::lists()->get_list().get_num_items();
	bool was_dragging = ((storage_item_dragged != -1) || (item_dragged != -1));

	// If dragging item and ctrl+left-click on window, add item to list
	if ((flags & ELW_LEFT_MOUSE) && (flags & ELW_CTRL) && was_dragging)
	{
#ifdef ITEM_UID
		if (storage_item_dragged != -1)
			ItemLists::Vars::lists()->add_item(storage_items[storage_item_dragged].image_id, storage_items[storage_item_dragged].id, item_quantity);
		else if (item_dragged != -1)
			ItemLists::Vars::lists()->add_item(item_list[item_dragged].image_id, item_list[item_dragged].id, item_quantity);
#else
		if (storage_item_dragged != -1)
			ItemLists::Vars::lists()->add_item(storage_items[storage_item_dragged].image_id, unset_item_uid, item_quantity);
		else if (item_dragged != -1)
			ItemLists::Vars::lists()->add_item(item_list[item_dragged].image_id, unset_item_uid, item_quantity);
#endif
		last_mod_time = SDL_GetTicks();
		return 1;
	}

	// ctrl+right-click on a selected item opens the edit menu
	if ((flags & ELW_RIGHT_MOUSE) && (flags & ELW_CTRL) && (get_preview_item_number(mx, my)<num_items))
	{
		cm_show_direct(cm_selected_item_menu, win->window_id, -1);
		storage_item_dragged = item_dragged = -1;
		return 1;
	}

	// always reset the quanitity selection on other mouse click
	restore_inventory_quantity();

	// wheel mouse up/down scrolls though lists - other clicks use the list
	if ((flags & ELW_WHEEL_UP ) || (flags & ELW_WHEEL_DOWN ))
		ItemLists::Vars::lists()->change_preview(flags);

	// see if we can use the item quantity or take items from storage
	else if ((flags & ELW_RIGHT_MOUSE) || (flags & ELW_LEFT_MOUSE))
	{
		size_t new_selected = get_preview_item_number(mx, my);
		if ((new_selected!=last_selected) && (new_selected < num_items))
		{
			selected_item_number = new_selected;
			last_quantity_selected = quantities.selected;
			quantities.selected = ITEM_EDIT_QUANT;
			item_quantity = quantities.quantity[ITEM_EDIT_QUANT].val = ItemLists::Vars::lists()->get_list().get_quantity(selected_item_number);
#ifdef NEW_SOUND
			if (flags & ELW_RIGHT_MOUSE)
				add_sound_object(get_index_for_sound_type_name("Button Click"), 0, 0, 1);
#endif // NEW_SOUND
			if (flags & ELW_LEFT_MOUSE)
			{
				// randomly close the preview window
				if (!(SDL_GetTicks() & 63))
				{
					hide_window(preview_win);
					set_shown_string(c_red2, item_list_magic_str);
					return 0;
				}
				storage_item_dragged = item_dragged = -1;
				int image_id = ItemLists::Vars::lists()->get_list().get_image_id(selected_item_number);
				Uint16 item_id = ItemLists::Vars::lists()->get_list().get_item_id(selected_item_number);
				int cat_id = ItemLists::Vars::cat_maps()->get_cat(image_id, item_id);
				if (cat_id != -1)
					pickup_storage_item(image_id, item_id, cat_id);
				else
				{
#ifdef NEW_SOUND
					add_sound_object(get_index_for_sound_type_name("alert1"), 0, 0, 1);
#endif // NEW_SOUND
					il_pickup_fail_time = SDL_GetTicks();
					static bool first_fail = true;
					if (first_fail)
					{
						first_fail = false;
						LOG_TO_CONSOLE(c_red1, item_list_learn_cat_str);
					}
				}
			}
		}
		else
			storage_item_dragged = item_dragged = -1;
	}

	return 1;
}


//	Record mouse over the preview window so the draw handler can show help text
//
static int mouseover_preview_handler(window_info *win, int mx, int my)
{
	if ((my < 0) || (cm_window_shown()!=CM_INIT_VALUE))
		return 0;
	size_t item_number = get_preview_item_number(mx, my);
	if (item_number < ItemLists::Vars::lists()->get_list().get_num_items())
	{
		help_str.push_back(item_list_pickup_help_str);
		help_str.push_back(item_list_use_help_str);
		help_str.push_back(item_list_edit_help_str);
	}
	if ((storage_item_dragged != -1) || (item_dragged != -1))
		help_str.push_back(item_list_add_help_str);

	return 0;
}


//  Called when the preview window is hidden, undo any quantity setting, do pending save.
//
static int hide_preview_handler(window_info *win)
{
	restore_inventory_quantity();
	if (last_mod_time)
	{
		ItemLists::Vars::lists()->save();
		last_mod_time = 0;
	}
	return 1;
}


//	Once a new quantity has been entered, set the value in the list
//
static void quantity_input_handler(const char *input_text, void *data)
{
	ItemLists::Quantity_Input *input = static_cast<ItemLists::Quantity_Input *>(data);

	if ((ItemLists::Vars::lists()->get_previewed() != input->get_list()) ||
		(input->get_item() >= ItemLists::Vars::lists()->get_list().get_num_items()))
		return;
	int quantity;
	std::istringstream ss(input_text);
	ss >> quantity;
	if (quantity > 0)
	{
		ItemLists::Vars::lists()->set_quantity(input->get_item(), quantity);
		ItemLists::Vars::lists()->save();
	}
}


//	Selected item context menu (in preview list) option handler
//
static int cm_selected_item_handler(window_info *win, int widget_id, int mx, int my, int option)
{
	size_t item_under_mouse = get_preview_item_number(mx, my);
	if (!ItemLists::Vars::lists()->valid_preview() || (item_under_mouse>=ItemLists::Vars::lists()->get_list().get_num_items()))
		return 0;

	// edit the quanity
	if (option == 0)
	{
		ItemLists::Vars::quantity_input()->open(win->window_id, mx, my, ItemLists::Vars::lists()->get_previewed(), item_under_mouse);
		return 1;
	}

	// delete item, removing whole list if its now empty.  Save lists in any case.
	else if (option == 2)
	{
		ItemLists::Vars::lists()->del_item(item_under_mouse);
		if (ItemLists::Vars::lists()->get_list().get_num_items()==0)
		{
			hide_window(preview_win);
			ItemLists::Vars::lists()->del(ItemLists::Vars::lists()->get_previewed());
			update_list_window();			
		}
		ItemLists::Vars::lists()->save();
		return 1;		
	}
	
	return 0;	
}


//	Create the preview window or just show it.
//
static void show_preview(window_info *win)
{
	if (preview_win <= 0 )
	{
		preview_win = create_window(item_list_preview_title, win->window_id, 0, win->len_x + 5, 0, preview_grid_size*6 + ELW_BOX_SIZE + 3, preview_grid_size*6 + ELW_BOX_SIZE, ELW_WIN_DEFAULT);
		set_window_handler(preview_win, ELW_HANDLER_DISPLAY, (int (*)())&display_preview_handler );
		set_window_handler(preview_win, ELW_HANDLER_CLICK, (int (*)())&click_preview_handler );
		set_window_handler(preview_win, ELW_HANDLER_MOUSEOVER, (int (*)())&mouseover_preview_handler );
		set_window_handler(preview_win, ELW_HANDLER_HIDE, (int (*)())&hide_preview_handler );
		cm_selected_item_menu = cm_create(cm_item_list_selected_str, cm_selected_item_handler);
	}
	else
	{
		show_window(preview_win);
		ItemLists::Vars::quantity_input()->close();
	}
	move_window(preview_win, win->window_id, 0, win->pos_x + win->len_x + 5, win->pos_y + preview_grid_size*(SDL_GetTicks() & 1));
}




//	The handler for when a list window (context menu) line is selected
//
static int list_window_handler(window_info *win, int widget_id, int mx, int my, int option)
{
	size_t list_index = static_cast<size_t>(option);
	if (list_index >= ItemLists::Vars::lists()->size())
		return 0;

	if (delete_item_list)
	{
		ItemLists::Vars::lists()->del(list_index);
		update_list_window();
		ItemLists::Vars::lists()->save();
	}
	else if (!disable_item_list_preview)
	{
		if (ItemLists::Vars::lists()->set_previewed(list_index))
			show_preview(win);
		else
			return 0;
	}
	else
		ItemLists::Vars::lists()->fetch(list_index);
		
	return 1;
}


//	Enter name input callback - when OK selected
//
static void name_input_handler(const char *input_text, void *data)
{
	// if sucessful, update the list menu and save
	if (ItemLists::Vars::lists()->add(input_text))
	{
		update_list_window();
		ItemLists::Vars::lists()->save();
	}
	// else delete the entry - could be because no items
	else
		LOG_TO_CONSOLE(c_red1, item_list_empty_list_str);

	// The new list option will have been disabled so re-enable it
	cm_grey_line(cm_item_list_options_but, OPTION_SAVE, 0);
}


//	Enter name input callback - when cancel selected
//
static void name_cancel_handler(void *data)
{
	// The new list option will have been disabled so re-enable it
	cm_grey_line(cm_item_list_options_but, OPTION_SAVE, 0);
}


//	Execute an option from the item list button context menu
//
static int options_handler(window_info *win, int widget_id, int mx, int my, int option)
{
	// make sure the preview window is hidden
	if (preview_win >= 0)
		hide_window(preview_win);
	
	if (option == OPTION_SAVE)
	{
		// create an input window for the name and let the callback save the list
		cm_grey_line(cm_item_list_options_but, OPTION_SAVE, 1);
		close_ipu(&ipu_item_list_name);
		init_ipu(&ipu_item_list_name, items_win, 310, 100, 26, 1, name_cancel_handler, name_input_handler);
		ipu_item_list_name.x = win->len_x + 10;
		ipu_item_list_name.y = (win->len_y - ipu_item_list_name.popup_y_len) / 2;
		display_popup_win(&ipu_item_list_name, item_list_name_str );
		return 1;
	}
	else if (option == OPTION_DELETE)
	{
		show_item_list_menu = delete_item_list = 1;
		return 1;		
	}
	else if (option == OPTION_RELOAD)
	{
		ItemLists::Vars::lists()->load();
		update_list_window();
		return 1;
	}
	
	return 0;
}


//	Open the items list menu, ready for normal use or to delete entries
//
static void show_window(bool is_delete)
{
	if (preview_win >= 0)
		hide_window(preview_win);

	if (is_delete)
	{
		// highligh is red for danger - delete
		cm_set_colour(cm_item_list_but, CM_HIGHLIGHT_TOP, 0.1f, 0.0f, 0.0f);
		cm_set_colour(cm_item_list_but, CM_HIGHLIGHT_BOTTOM, 1.0f, 0.0f, 0.0f);
		cm_show_direct(cm_item_list_but, items_win, -1);
		show_item_list_menu = 0;
	}
	else
	{
		// otherwise the default (need to make this a CM function)
		delete_item_list = 0;
		cm_set_colour(cm_item_list_but, CM_HIGHLIGHT_TOP, 0.11f, 0.11f, 0.11f);
		cm_set_colour(cm_item_list_but, CM_HIGHLIGHT_BOTTOM, 0.77f, 0.57f, 0.39f);
		cm_show_direct(cm_item_list_but, items_win, -1);
	}
}



extern "C"
{
	int show_item_list_menu = 0;
	int disable_item_list_preview = 0;
	size_t cm_item_list_but = CM_INIT_VALUE;
	size_t cm_item_list_options_but = CM_INIT_VALUE;

	int cm_item_list_handler(window_info *win, int widget_id, int mx, int my, int option)
		{ return list_window_handler(win, widget_id, mx, my, option); }

	void cm_item_list_pre_show_handler(window_info *win, int widget_id, int mx, int my, window_info *cm_win)
	{
		int offset = 5 + int( 0.5 + 0.8 * DEFAULT_FONT_Y_LEN + 3);
		int new_y_pos = win->cur_y + offset * (SDL_GetTicks() & 1);
		if (new_y_pos + cm_win->len_y > window_height)
			new_y_pos = window_height - cm_win->len_y - offset * (SDL_GetTicks() & 1);
		move_window(cm_win->window_id, -1, 0, win->cur_x + win->len_x + 2, new_y_pos);
	}

	void show_items_list_window(int is_delete)
		{ show_window((is_delete == 1) ?true :false); }
		
	int cm_item_list_options_handler(window_info *win, int widget_id, int mx, int my, int option)
		{ return options_handler(win, widget_id,  mx,  my, option); }

	void setup_item_list_menus(void)
	{
		cm_item_list_but = cm_create(cm_item_list_empty_str, cm_item_list_handler);
		cm_set_pre_show_handler(cm_item_list_but, cm_item_list_pre_show_handler);
		cm_grey_line(cm_item_list_but, 0, 1);
		
		cm_item_list_options_but = cm_create(cm_item_list_menu_str, cm_item_list_options_handler);
		cm_bool_line(cm_item_list_options_but, OPTION_PREVIEW, &disable_item_list_preview, NULL);

		/* always use preview for now */
		disable_item_list_preview = 0;
		cm_grey_line(cm_item_list_options_but, OPTION_PREVIEW, 1);

		ItemLists::Vars::lists()->load();
		ItemLists::Vars::cat_maps()->load();
		update_list_window();
		init_ipu(&ipu_item_list_name, -1, -1, -1, 1, 1, NULL, NULL);
	}

	void update_category_maps(int image_id, Uint16 item_id, int cat_id)
		{ ItemLists::Vars::cat_maps()->update(image_id, item_id, cat_id); }

	void save_category_maps(void)
		{ ItemLists::Vars::cat_maps()->save(); }
}

#endif
