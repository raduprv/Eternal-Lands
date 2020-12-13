#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include "mapwin.h"
#include "asc.h"
#include "books.h"
#include "console.h"
#include "consolewin.h"
#include "chat.h"
#include "draw_scene.h"
#include "elwindows.h"
#include "events.h"
#include "gamewin.h"
#include "gl_init.h"
#include "hud.h"
#include "init.h"
#include "interface.h"
#include "map.h"
#include "missiles.h"
#include "new_character.h"
#include "pathfinder.h"
#include "textures.h"
#include "eye_candy_wrapper.h"
#include "special_effects.h"


static int showing_continent = 0;
#ifdef DEBUG_MAP_SOUND
extern int cur_tab_map;
#endif // DEBUG_MAP_SOUND

int adding_mark = 0;
int mark_x , mark_y;
int max_mark = 0;
marking marks[MAX_MARKINGS];

static int mouse_over_minimap = 0;

static int reload_tab_map = 0;

int temp_tile_map_size_x = 0;
int temp_tile_map_size_y = 0;
int max_temp_mark = 0;
marking temp_marks[MAX_USER_MARKS];

#define MARK_FILTER_MAX_LEN 40
int mark_filter_active = 0;
char mark_filter_text[MARK_FILTER_MAX_LEN] = "";

int curmark_r=0,curmark_g=255,curmark_b=0;

static int click_map_handler (window_info *win, int mx, int my, Uint32 flags)
{
	Uint32 ctrl_on = flags & KMOD_CTRL;
	Uint32 left_click = flags & ELW_LEFT_MOUSE;
	Uint32 right_click = flags & ELW_RIGHT_MOUSE;

	if (hud_click(win, mx, my, flags))
		return 1;

	if (left_click && mx > small_map_screen_x_left && mx < small_map_screen_x_right
		&& my > small_map_screen_y_top && my < small_map_screen_y_bottom)
	{
		showing_continent = !showing_continent;
		inspect_map_text = 0;
	}
	else if (!showing_continent && inspect_map_text == 0)
	{
		if (left_click)
		{
			pf_move_to_mouse_position ();
		}
		else if (right_click)
		{
			if (!ctrl_on)
				put_mark_on_map_on_mouse_position ();
			else
				delete_mark_on_map_on_mouse_position ();
		}
	}
	else if(showing_continent)
	{
		if (left_click)
		{
			int screen_map_width = main_map_screen_x_right - main_map_screen_x_left;
			int screen_map_height = main_map_screen_y_bottom - main_map_screen_y_top;

			int i;
			/* Convert mouse coordinates to map coordinates (stolen from pf_get_mouse_position()) */
			int m_px = ((mx-main_map_screen_x_left) * 512) / screen_map_width;
			int m_py = 512 - ((my-main_map_screen_y_top) * 512) / screen_map_height;

			/* Check if we clicked on a map */
			for(i = 0; continent_maps[i].name != NULL; i++) {
				if(continent_maps[i].cont == continent_maps[cur_map].cont) {
					if(m_px > continent_maps[i].x_start && m_px < continent_maps[i].x_end
						&& m_py > continent_maps[i].y_start && m_py < continent_maps[i].y_end)
					{
						/* Load this map's bmp */
						if(cur_map != i) {
							inspect_map_text = load_texture_cached(continent_maps[i].name, tt_image);
						}
#ifdef DEBUG_MAP_SOUND
						cur_tab_map = i;
#endif // DEBUG_MAP_SOUND
						load_marks_to_buffer(continent_maps[i].name, temp_marks, &max_temp_mark);
						get_tile_map_sizes(continent_maps[i].name, &temp_tile_map_size_x, &temp_tile_map_size_y);
						showing_continent = !showing_continent;
						break;
					}
				}
			}
		}
	}

	return 1;
}

static int display_map_handler (window_info * win)
{
	draw_hud_interface (win);
	Leave2DMode ();
	if(reload_tab_map && windows_list.window[win->window_id].displayed)
		switch_to_game_map(); //need to reload the BMP
	draw_game_map (!showing_continent, mouse_over_minimap);
	Enter2DMode ();
	CHECK_GL_ERRORS ();
	reload_tab_map = 0;

	display_handling_common(win);

	return 1;
}

static int mouseover_map_handler (window_info *win, int mx, int my)
{
	if (hud_mouse_over(win, mx, my))
		return 1;

	mouse_over_minimap = (mx > small_map_screen_x_left && mx < small_map_screen_x_right
		&& my > small_map_screen_y_top && my < small_map_screen_y_bottom);
	return mouse_over_minimap;
}

static int keypress_map_handler (window_info *win, int mx, int my, SDL_Keycode key_code, Uint32 key_unicode, Uint16 key_mod)
{
	if ((key_code == SDLK_RETURN || key_code == SDLK_KP_ENTER) && adding_mark && input_text_line.len > 0)
	{
		int i;

		// if text wrapping just keep the text until the wrap.
		for (i = 0; i < input_text_line.len; i++)
		{
			if (input_text_line.data[i] == '\n')
			{
				input_text_line.data[i] = '\0';
				break;
			}
		}

		put_mark_on_position(mark_x, mark_y, input_text_line.data);
		adding_mark = 0;
		clear_input_line ();
	}
	// does the user want to cancel a mapmark?
	else if (key_code == SDLK_ESCAPE && adding_mark)
	{
		adding_mark = 0;
		clear_input_line ();
	}
	// enable, disable or reset the mark filter
	else if (KEY_DEF_CMP(K_MARKFILTER, key_code, key_mod) || (mark_filter_active && (key_code == SDLK_ESCAPE)))
	{
		if (!mark_filter_active || (key_code == SDLK_ESCAPE))
			mark_filter_active ^= 1;
		memset(mark_filter_text, 0, sizeof(char)*MARK_FILTER_MAX_LEN);
	}
	// now try the keypress handler for all root windows
	else if ( keypress_root_common (key_code, key_unicode, key_mod) )
	{
		return 1;
	}
	else if (KEY_DEF_CMP(K_MAP, key_code, key_mod))
	{
		return_to_gamewin_common();
	}
	else if (mark_filter_active && !adding_mark)
	{
		return string_input(mark_filter_text, MARK_FILTER_MAX_LEN, key_code, key_unicode, key_mod);
	}
	else
	{
		if (key_unicode == '`' || KEY_DEF_CMP(K_CONSOLE, key_code, key_mod))
		{
			hide_window (win->window_id);
			show_window_MW(MW_CONSOLE);
		}
		else if ( !text_input_handler (key_code, key_unicode, key_mod) )
		{
			// nothing we can handle
			return 0;
		}
	}
	// we handled it, return 1 to let the window manager know
	return 1;
}

static int show_map_handler (window_info *win)
{
	close_book_window();
	hide_window(tab_bar_win);
	return 1;
}

static int resize_map_root_handler(window_info *win, int width, int height)
{
	if (get_show_window(win->window_id))
		init_hud_interface (HUD_INTERFACE_GAME);
	reload_tab_map = 1;
	return 1;
}

static int hide_map_handler (window_info * win)
{
	widget_unset_flags (input_widget->window_id, input_widget->id, WIDGET_INVISIBLE);
	return 1;
}

void create_map_root_window (int width, int height)
{
	int map_root_win = get_id_MW(MW_TABMAP);
	if (map_root_win < 0)
	{
		map_root_win = create_window ("Map", -1, -1, 0, 0, width, height, ELW_USE_UISCALE|ELW_TITLE_NONE|ELW_SHOW_LAST);
		set_id_MW(MW_TABMAP, map_root_win);

		set_window_handler (map_root_win, ELW_HANDLER_DISPLAY, &display_map_handler);
		set_window_handler (map_root_win, ELW_HANDLER_KEYPRESS, (int (*)())&keypress_map_handler);
		set_window_handler (map_root_win, ELW_HANDLER_CLICK, &click_map_handler);
		set_window_handler (map_root_win, ELW_HANDLER_MOUSEOVER, &mouseover_map_handler);
		set_window_handler (map_root_win, ELW_HANDLER_SHOW, &show_map_handler);
		set_window_handler (map_root_win, ELW_HANDLER_HIDE, &hide_map_handler);
		set_window_handler (map_root_win, ELW_HANDLER_RESIZE, &resize_map_root_handler);
	}
}



