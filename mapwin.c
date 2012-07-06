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
#include "gamewin.h"
#include "gl_init.h"
#include "global.h"
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


int map_root_win = -1;
int showing_continent = 0;
#ifdef DEBUG_MAP_SOUND
extern int cur_tab_map;
#endif // DEBUG_MAP_SOUND

int mouse_over_minimap = 0;

int reload_tab_map = 0;

int temp_tile_map_size_x = 0;
int temp_tile_map_size_y = 0;
int max_temp_mark = 0;
marking temp_marks[MAX_USER_MARKS];

#define MARK_FILTER_MAX_LEN 40
int mark_filter_active = 0;
char mark_filter_text[MARK_FILTER_MAX_LEN] = "";

int curmark_r=0,curmark_g=255,curmark_b=0;

int click_map_handler (window_info *win, int mx, int my, Uint32 flags)
{
	Uint32 ctrl_on = flags & ELW_CTRL;
	Uint32 left_click = flags & ELW_LEFT_MOUSE;
	Uint32 right_click = flags & ELW_RIGHT_MOUSE;
	float scale = (float) (win->len_x-hud_x) / 300.0f;

	if (left_click && mx > 0 && mx < 50*scale && my > 0 && my < 55*scale)
	{
		showing_continent = !showing_continent;
#ifdef	NEW_TEXTURES
		inspect_map_text = 0;
#else	/* NEW_TEXTURES */
		if(inspect_map_text != 0)
		{
			glDeleteTextures(1,&inspect_map_text);
			inspect_map_text = 0;
		}
#endif	/* NEW_TEXTURES */
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
			int min_mouse_x = (window_width-hud_x)/6;
			int min_mouse_y = 0;
			
			int max_mouse_x = min_mouse_x+((window_width-hud_x)/1.5);
			int max_mouse_y = window_height - hud_y;
			
			int screen_map_width = max_mouse_x - min_mouse_x;
			int screen_map_height = max_mouse_y - min_mouse_y;

			int i;
			/* Convert mouse coordinates to map coordinates (stolen from pf_get_mouse_position()) */
			int m_px = ((mx-min_mouse_x) * 512) / screen_map_width;
			int m_py = 512 - ((my * 512) / screen_map_height);

			/* Check if we clicked on a map */
			for(i = 0; continent_maps[i].name != NULL; i++) {
				if(continent_maps[i].cont == continent_maps[cur_map].cont) {
					if(m_px > continent_maps[i].x_start && m_px < continent_maps[i].x_end
						&& m_py > continent_maps[i].y_start && m_py < continent_maps[i].y_end)
					{
						/* Load this map's bmp */
						if(cur_map != i) {
#ifdef	NEW_TEXTURES
							inspect_map_text = load_texture_cached(continent_maps[i].name, tt_image);
#else	/* NEW_TEXTURES */
							texture_cache_struct tex;
							size_t name_len;
							my_strcp(tex.file_name,continent_maps[i].name);
							name_len = strlen(tex.file_name);
							sprintf(tex.file_name+name_len-3, "bmp");
							inspect_map_text = load_bmp8_fixed_alpha(&tex, 128);
#endif	/* NEW_TEXTURES */
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

int display_map_handler (window_info * win)
{
	// are we actively drawing things?
	if (SDL_GetAppState () & SDL_APPACTIVE)
	{
		draw_hud_interface ();
		Leave2DMode ();
		if(reload_tab_map && map_root_win >= 0 && windows_list.window[map_root_win].displayed){
			//need to reload the BMP
			switch_from_game_map();
			switch_to_game_map();
		}
		draw_game_map (!showing_continent, mouse_over_minimap);
		Enter2DMode ();
		CHECK_GL_ERRORS ();
		reload_tab_map = 0;
	}	

	if(special_effects){
		display_special_effects(0);
	}

	// remember the time stamp to improve FPS quality when switching modes
	next_fps_time=cur_time+1000;
	last_count=0;

	ec_idle();


	missiles_update();
    update_camera();

	draw_delay = 20;

	if ((input_widget!= NULL) && (input_widget->window_id != win->window_id))
		input_widget_move_to_win(win->window_id);

	return 1;
}

int mouseover_map_handler (window_info *win, int mx, int my)
{
	float scale = (float) (win->len_x-hud_x) / 300.0f;

	if (mx > 0 && mx < 50*scale && my > 0 && my < 55*scale)
	{
		mouse_over_minimap = 1;
	}
	else
	{
		mouse_over_minimap = 0;
	}
	return mouse_over_minimap;
}	

int keypress_map_handler (window_info *win, int mx, int my, Uint32 key, Uint32 unikey)
{
	Uint8 ch = key_to_char (unikey);

	if (ch == SDLK_RETURN && adding_mark && input_text_line.len > 0)
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
	else if (ch == SDLK_ESCAPE && adding_mark)
	{
		adding_mark = 0;
		clear_input_line ();
	}
	// enable, disable or reset the mark filter
	else if ((key == K_MARKFILTER) || (mark_filter_active && (ch == SDLK_ESCAPE)))
	{
		if (!mark_filter_active || (ch == SDLK_ESCAPE))
			mark_filter_active ^= 1;
		memset(mark_filter_text, 0, sizeof(char)*MARK_FILTER_MAX_LEN);
	}
	// now try the keypress handler for all root windows
	else if ( keypress_root_common (key, unikey) )
	{
		return 1;
	}
	else if (key == K_MAP)
	{
		if (keep_grabbing_mouse)
		{
			toggle_have_mouse();
			keep_grabbing_mouse=0;
		}
		switch_from_game_map ();
		hide_window (map_root_win);
		show_window (game_root_win);
		// Undo stupid quickbar hack
		if ( !get_show_window (quickbar_win) )
			show_window (quickbar_win);
	}
	else if (mark_filter_active && !adding_mark)
	{
		string_input(mark_filter_text, MARK_FILTER_MAX_LEN, ch);
	}
	else
	{
		reset_tab_completer();
		if (ch == '`' || key == K_CONSOLE)
		{
			switch_from_game_map ();
			hide_window (map_root_win);
			show_window (console_root_win);
		}
		else if ( !text_input_handler (key, unikey) )
		{
			// nothing we can handle
			return 0;
		}
	}
	// we handled it, return 1 to let the window manager know
	return 1;
}

int show_map_handler (window_info *win)
{
	hide_window(book_win);
	hide_window(paper_win);
	hide_window(color_race_win);
	hide_window(tab_bar_win);
	return 1;
}

int hide_map_handler (window_info * win)
{
	widget_unset_flags (input_widget->window_id, input_widget->id, WIDGET_INVISIBLE);
	return 1;
}

void create_map_root_window (int width, int height)
{
	if (map_root_win < 0)
	{
		map_root_win = create_window ("Map", -1, -1, 0, 0, width, height, ELW_TITLE_NONE|ELW_SHOW_LAST);
	
		set_window_handler (map_root_win, ELW_HANDLER_DISPLAY, &display_map_handler);
		set_window_handler (map_root_win, ELW_HANDLER_KEYPRESS, &keypress_map_handler);
		set_window_handler (map_root_win, ELW_HANDLER_CLICK, &click_map_handler);
		set_window_handler (map_root_win, ELW_HANDLER_MOUSEOVER, &mouseover_map_handler);
		set_window_handler (map_root_win, ELW_HANDLER_SHOW, &show_map_handler);
		set_window_handler (map_root_win, ELW_HANDLER_HIDE, &hide_map_handler);
	}
}



