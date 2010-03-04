/*
 * User can store current inventory contents into a named list.  It is
 * hoped that the server will support fetching the listed items directly
 * from storage.  However, macroing concerns may prevent this.  IMHO,
 * the preview window provides a useful feature in itself, allow you to
 * recored inventory configurations for set tasks such as making steel
 * bars at a mine.
 *
 * Author bluap/pjbroad December 2009
 *
 * To Do:
 * 		Make list array a singleton object?
 * 		Preview Window
 * 			Add buttons Fetch/Delete/Rename etc
 */

#ifdef CONTEXT_MENUS

#include <assert.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

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
			const std::vector<int> & get_object_ids(void) const { return object_ids; }
			const std::vector<int> & get_quantities(void) const { return quantities; }
			void write(std::ostream & out) const;
			bool read(std::istream & in, bool temp_version);
		private:
			std::string name;
			std::vector<int> object_ids;
			std::vector<int> quantities;
			std::vector<Uint16> item_uid;
	};
	
	
	// Fetch the items from the server.
	//
	void List::fetch(void) const
	{
		// do nothing for now
		// std::string message = "Fetch \"" + name + "\": Not yet supported:(";
		// LOG_TO_CONSOLE(c_green1, message.c_str());
	}
	
	
	// Set the name, object_ids and quantities for a new list.
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
					for (size_t j=0; j<item_uid.size(); j++)
						if (item_list[i].id == item_uid[j])
						{
							quantities[j]++;
							stacked_item = true;
							break;
						}
				if (!stacked_item)
				{
					object_ids.push_back(item_list[i].image_id);
					quantities.push_back(item_list[i].quantity);
					item_uid.push_back(item_list[i].id);
				}
#else
				object_ids.push_back(item_list[i].image_id);
				quantities.push_back(item_list[i].quantity);
				item_uid.push_back(unset_item_uid);
#endif
			}
		if (quantities.empty())
			return false;
		return true;
	}
	
	
	//	Write the list to the specified stream.
	//  The name, object_ids and quantities are on separate lines.
	//
	void List::write(std::ostream & out) const
	{
		out << name << std::endl;
		for (size_t i=0; out && i<object_ids.size(); i++)
			out << object_ids[i] << " ";
		out << std::endl;
		for (size_t i=0; out && i<quantities.size(); i++)
			out << quantities[i] << " ";
		out << std::endl;
		for (size_t i=0; out && i<item_uid.size(); i++)
			out << item_uid[i] << " ";
		out << std::endl;
	}
	
	
	//	Read an itemlist form the specified input stream
	//	If an error occurs the function will return false;
	// 	the caller should delete the object.
	//
	bool List::read(std::istream & in, bool temp_version)
	{
		std::string name_line, id_line, cnt_line, item_uid_line;

		// each part is on a separate line, but allow empty lines
		while (getline(in, name_line) && name_line.empty());
		while (getline(in, id_line) && id_line.empty());
		while (getline(in, cnt_line) && cnt_line.empty());
		// temporary code just to maintain first file format...
		if (temp_version)
			item_uid_line = "XXX";
		else
			while (getline(in, item_uid_line) && item_uid_line.empty());

		// mop up extra lines at the end of the file silently
		if (name_line.empty())
			return false;

		// a name without data is not a list!
		if (id_line.empty() || cnt_line.empty() || item_uid_line.empty())
		{
			LOG_ERROR("%s: Failed reading item list name=[%s]\n", __FILE__, name_line.c_str() );
			return false;
		}

		// read each id value
		std::istringstream ss(id_line);
		int value = 0;
		while (ss >> value)
			object_ids.push_back(value);

		// read each quantity value
		ss.clear();
		ss.str(cnt_line);
		value = 0;
		while (ss >> value)
			quantities.push_back(value);

		// temporary code just to maintain first file format...
		if (temp_version)
			item_uid.resize(quantities.size(), unset_item_uid);
		// just do this next in the next version
		else
		{
			// read each item uid value
			ss.clear();
			ss.str(item_uid_line);
			Uint16 ui_value = 0;
			while (ss >> ui_value)
				item_uid.push_back(ui_value);
		}

		// don't use a list with unequal or empty data sets
		if ((quantities.size() != object_ids.size()) || (quantities.size() != item_uid.size()) || quantities.empty())
		{
			LOG_ERROR("%s: Failed reading item list name=[%s] #id=%d #cnts=%d #uid=%d\n", __FILE__, name_line.c_str(), object_ids.size(), quantities.size(), item_uid.size() );
			return false;
		}

		// read list just fine
		name = name_line;
		return true;
	}
	
} // end ItemLists namespace



static std::vector<ItemLists::List> saved_item_lists;
static float FILE_REVISION = 2;
static INPUT_POPUP ipu_item_list_name;
static int delete_item_list = 0;
static int preview_win = -1;
static size_t previewed_list = 0;
static const char * preview_help_str = NULL;
//static const char * preview_fetch_help_str = "Left-click to fetch all items";
static const char * preview_quantity_help_str = "Right-click to use item quantity";
static int last_quantity_selected = 0;
static const int preview_grid_size = 33;
static size_t selected_item_number = static_cast<size_t>(1);

static const int OPTION_SAVE = 0;
static const int OPTION_PREVIEW = 1;
// seperator value 2
static const int OPTION_DELETE = 3;
// seperator value 4
static const int OPTION_RELOAD = 5;


//  When lists are added/remove, update the list in the menu.
//
static void update_list_window()
{
	std::string menu_string;
	for (size_t i=0; i<saved_item_lists.size(); ++i)
		menu_string += saved_item_lists[i].get_name() + "\n";

	if (menu_string.empty())
	{
		cm_set(cm_item_list_but, "Empty", cm_item_list_handler);
		cm_grey_line(cm_item_list_but, 0, 1);
	}
	else
		cm_set(cm_item_list_but, menu_string.c_str(), cm_item_list_handler);
}


//  Save the item lists to a file in players config directory
//
static void save_item_lists(void)
{
	std::string fullpath = get_path_config() + std::string("item_lists.txt");
	std::ofstream out(fullpath.c_str());
	if (!out)
	{
		LOG_ERROR("%s: Failed to open item lists file for write [%s]\n", __FILE__, fullpath.c_str() );
		LOG_TO_CONSOLE(c_red1, "Failed to save the item lists file.");
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


//  Save the item lists to a file in players config directory
//
static void load_item_lists(void)
{
	std::string fullpath = get_path_config() + std::string("item_lists.txt");
	std::ifstream in(fullpath.c_str());
	if (!in)
	{
		LOG_ERROR("%s: Failed to open item lists file for read [%s]\n", __FILE__, fullpath.c_str() );
		return;
	}

	float revision;
	in >> revision;
	if ((revision != FILE_REVISION) && (revision != static_cast<float>(0.1)))
	{
		LOG_ERROR("%s: Item lists file is not compatible with client version [%s]\n", __FILE__, fullpath.c_str() );
		return;
	}
	
	saved_item_lists.clear();
	while (!in.eof())
	{
		saved_item_lists.push_back(ItemLists::List());
		if (!saved_item_lists.back().read(in, (revision == static_cast<float>(0.1))))
			saved_item_lists.pop_back();
	}
	update_list_window();

	in.close();
}




//  Draw the preview window, name, item image_ids + quantities & help text
//
static int display_preview_handler(window_info *win)
{
	size_t i;
	char str[80];

	if (previewed_list >= saved_item_lists.size())
		return 1;
	
	glEnable(GL_TEXTURE_2D);

	// draw the images and the quantities
	glColor3f(1.0f,1.0f,1.0f);
	for(i=0; i<saved_item_lists[previewed_list].get_object_ids().size(); i++)
	{
		int x_start, x_end, y_start, y_end;
		x_start = preview_grid_size * (i%6) + 1;
		x_end = x_start + preview_grid_size - 1;
		y_start = preview_grid_size * (i/6);
		y_end = y_start + preview_grid_size - 1;
		draw_item(saved_item_lists[previewed_list].get_object_ids()[i], x_start, y_start, preview_grid_size);

		safe_snprintf(str, sizeof(str), "%i", saved_item_lists[previewed_list].get_quantities()[i]);
		draw_string_small_shadowed(x_start, (i&1)?(y_end-15):(y_end-25), (unsigned char*)str, 1,1.0f,1.0f,1.0f, 0.0f, 0.0f, 0.0f);
	}

	// draw the list name
	draw_string_small(4, preview_grid_size*6 + 4, (unsigned char*)saved_item_lists[previewed_list].get_name().c_str(), 1);

	// draw mouse over window help text
	if (show_help_text && (preview_help_str != NULL))
	{
		show_help(preview_help_str, 0, win->len_y + 10);
		preview_help_str = NULL;
	}

	glDisable(GL_TEXTURE_2D);

	// draw the item grid
	glColor3f(0.77f,0.57f,0.39f);
	rendergrid(6, 6, 0, 0, preview_grid_size, preview_grid_size);

	// if an object is selected, draw a green grid around it
	if ((quantities.selected == ITEM_EDIT_QUANT) && (selected_item_number < saved_item_lists[previewed_list].get_quantities().size()))
	{
		int x_start = selected_item_number%6 * preview_grid_size;
		int y_start = static_cast<int>(selected_item_number/6) * preview_grid_size;
		glColor3f(0.0f, 1.0f, 0.3f);
		rendergrid(1, 1, x_start, y_start, preview_grid_size, preview_grid_size);
		rendergrid(1, 1, x_start-1, y_start-1, preview_grid_size+2, preview_grid_size+2);
	}
	
	glEnable(GL_TEXTURE_2D);
	
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	
	return 1;
}


//
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
	size_t num_items = saved_item_lists[previewed_list].get_quantities().size();
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

	// always reset the quanitity selection on mouse click
	restore_inventory_quantity();

	// wheel mouse up/down scrolls though lists - other clicks use the list
	if ((flags & ELW_WHEEL_UP ) && previewed_list > 0)
		previewed_list--;
	else if ((flags & ELW_WHEEL_DOWN ) && previewed_list+1 < saved_item_lists.size())
		previewed_list++;
	// left-click - fetch the items
	else if (flags & ELW_LEFT_MOUSE)
	{
		saved_item_lists[previewed_list].fetch();
		// for now don't hide.... hide_window(preview_win);
#ifdef NEW_SOUND
		add_sound_object(get_index_for_sound_type_name("Button Click"), 0, 0, 1);
#endif
	}
	// right-click see if we can use the item quantity
	else if (flags & ELW_RIGHT_MOUSE)
	{
		selected_item_number = get_preview_item_number(mx, my);
		if (selected_item_number < saved_item_lists[previewed_list].get_quantities().size())
		{
			last_quantity_selected = quantities.selected;
			quantities.selected = ITEM_EDIT_QUANT;
			item_quantity = quantities.quantity[ITEM_EDIT_QUANT].val = saved_item_lists[previewed_list].get_quantities()[selected_item_number];
		}		 
	}

	return 1;
}


//	Record mouse over the preview window so the draw handler can show help text
//
static int mouseover_preview_handler(window_info *win, int mx, int my)
{
	if (my < 0)
		return 0;
	size_t item_number = get_preview_item_number(mx, my);
	if ((item_number >= 0) && (item_number < saved_item_lists[previewed_list].get_quantities().size()))
		preview_help_str = preview_quantity_help_str;
/*	else
		preview_help_str = preview_fetch_help_str;*/
	return 0;
}


//  Called when the preview window is hidden, undo any quantity setting
//
static int hide_preview_handler(window_info *win)
{
	restore_inventory_quantity();
	return 1;
}


//	Create the preview window or just show it.
//
static void show_preview(window_info *win)
{
	if (preview_win <= 0 )
	{
		preview_win = create_window("List Preview", win->window_id, 0, win->len_x + 5, 0, preview_grid_size*6 + ELW_BOX_SIZE + 3, preview_grid_size*6 + ELW_BOX_SIZE, ELW_WIN_DEFAULT);
		set_window_handler(preview_win, ELW_HANDLER_DISPLAY, (int (*)())&display_preview_handler );
		set_window_handler(preview_win, ELW_HANDLER_CLICK, (int (*)())&click_preview_handler );
		set_window_handler(preview_win, ELW_HANDLER_MOUSEOVER, (int (*)())&mouseover_preview_handler );
		set_window_handler(preview_win, ELW_HANDLER_HIDE, (int (*)())&hide_preview_handler );
	}
	else
		show_window(preview_win);
}




//	The handler for when a list window (context menu) line is selected
//
static int list_window_handler(window_info *win, int widget_id, int mx, int my, int option)
{
	if (static_cast<size_t>(option) >= saved_item_lists.size())
		return 0;

	if (delete_item_list)
	{
		saved_item_lists.erase(saved_item_lists.begin()+option);
		update_list_window();
		save_item_lists();
	}
	else if (!disable_item_list_preview)
	{
		previewed_list = static_cast<size_t>(option);
		show_preview(win);
	}
	else
		saved_item_lists[option].fetch();
		
	return 1;
}


//	Enter name input callback - when OK selected
//
static void name_input_handler(const char *input_text)
{
	saved_item_lists.push_back(ItemLists::List());

	// if sucessful, update the list and save
	if (saved_item_lists.back().set(input_text))
	{
		update_list_window();
		save_item_lists();
	}
	// else delete the entry - could be because no items
	else
	{
		LOG_TO_CONSOLE(c_red1, "No point saving an empty list");
		saved_item_lists.pop_back();
	}

	// The new list option will have been disabled so re-enable it
	cm_grey_line(cm_item_list_options_but, OPTION_SAVE, 0);
}


//	Enter name input callback - when cancel selected
//
static void name_cancel_handler(void)
{
	// The new list option will have been disabled so re-enable it
	cm_grey_line(cm_item_list_options_but, OPTION_SAVE, 0);
}


//	Execute an option from the item list button context menu
//
static int options_handler(window_info *win, int widget_id, int mx, int my, int option)
{
	// make sure the preview window is hidden
	previewed_list = saved_item_lists.size();
	if (preview_win >= 0)
		hide_window(preview_win);
	
	if (option == OPTION_SAVE)
	{
		// create an input window for the name and let the callback save the list
		cm_grey_line(cm_item_list_options_but, OPTION_SAVE, 1);
		init_ipu(&ipu_item_list_name, items_win, 310, 100, 26, 1, name_cancel_handler, name_input_handler);
		ipu_item_list_name.x = win->len_x + 10;
		ipu_item_list_name.y = (win->len_y - ipu_item_list_name.popup_y_len) / 2;
		display_popup_win(&ipu_item_list_name, "Enter List Name");
		return 1;
	}
	else if (option == OPTION_DELETE)
	{
		show_item_list_menu = delete_item_list = 1;
		return 1;		
	}
	else if (option == OPTION_RELOAD)
	{
		load_item_lists();
		return 1;
	}
	
	return 0;
}


//	Open the items list menu, ready for normal use or to delete entries
//
static void show_window(bool is_delete)
{
	previewed_list = saved_item_lists.size();
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
		cm_item_list_but = cm_create("Empty", cm_item_list_handler);
		cm_set_pre_show_handler(cm_item_list_but, cm_item_list_pre_show_handler);
		cm_grey_line(cm_item_list_but, 0, 1);
		
		cm_item_list_options_but = cm_create("Save a new list\nDisable list preview\n--\nDelete a list\n--\nReload item lists file", cm_item_list_options_handler);
		cm_bool_line(cm_item_list_options_but, OPTION_PREVIEW, &disable_item_list_preview, NULL);
		cm_grey_line(cm_item_list_options_but, OPTION_PREVIEW, 1); /* always use preview for now */
		
		load_item_lists();
	}
}

#endif
