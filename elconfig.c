#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#include <float.h>

//For stat() etc.. below
#include <sys/types.h>
#include <sys/stat.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif //_MSC_VER

#ifdef MAP_EDITOR
	#include "../map_editor/global.h"
#elif defined(MAP_EDITOR2)
	#include "../map_editor2/global.h"
#else
	#include "global.h"
	#include "chat.h"
#endif
#include "elconfig.h"
#include "text.h"
#include "consolewin.h"
#include "queue.h"

#define SPECINT		INT //Multiple ints, non-default func				func(int*,int)
#define BOOL		1	// Change variable 								func(int*)
#define STRING		2	// Change string 								func(char*,char*)
#define FLOAT		3 	// Change float									func(float*,float*)
#define INT			4	// Change int									func(int*,int)
#define MULTI		5   // INT with multiselect widget
#define PASSWORD	6

// Defines for config variables
#define VIDEO		0
#define CONTROLS	1
#define AUDIO		2
#define HUD			3
#define SERVER		4
#define MISC		5
#define FONT 		6
#define CHAT		7
#define SPECIALVID	8

#define MAX_TABS 9

#define CHECKBOX_SIZE		15
#define SPACING				5	//Space between widgets and labels and lines
#define LONG_DESC_SPACE		50	//Space to give to the long descriptions
#define MAX_LONG_DESC_LINES	3	//How many lines of text we can fit in LONG_DESC_SPACE

typedef char input_line[256];

struct variables our_vars= {0,{NULL}};

int write_ini_on_exit= 1;
// Window Handling
int elconfig_win= -1;
int elconfig_tab_collection_id= 1;
int elconfig_free_widget_id= 2;
unsigned char elconf_description_buffer[400]= {0};
struct {
	Uint32	tab;
	Uint16	x;
	Uint16	y;
} elconfig_tabs[MAX_TABS];

int elconfig_menu_x= 10;
int elconfig_menu_y= 10;
int elconfig_menu_x_len= 520;
int elconfig_menu_y_len= 463;

int windows_on_top= 0;
int options_set= 0;
int compass_direction_checkbox= 1;
int shadow_map_size_multi= 0;

int you_sit= 0;
int sit_lock= 0;
int show_fps= 1;
int render_skeleton= 0;
int render_mesh= 1;

static __inline__ void check_option_var(char* name);

static __inline__ void update_fbo_and_shadow_mapping()
{
	check_option_var("shadow_map_size");
}

void change_var(int * var)
{
	*var= !*var;
}

void change_int(int * var, int value)
{
	if(value>=0) *var= value;
}

void change_float(float * var, float * value)
{
#ifdef	ELC
	if(var == &name_zoom){
		if(*value > 2.0){
			*value= 2.0;
		}
	}
#endif	//ELC
	if(*value >= 0) {
		*var= *value;
	} else {
		*var= 0;
	}
}

void change_string(char * var, char * str, int len)
{
	while(*str && len--){
		*var++= *str++;
	}
	*var= 0;
}

#ifdef ELC
void change_sound_level(float *var, float * value)
{
	if(*value >= 0.0f && *value <= 1.0f+0.00001) {
		*var= (float)*value;
	} else {
		*var=0;
	}
}

void change_password(char * passwd)
{
	int i= 0;
	char *str= password_str;

	while(*passwd) {
		*str++= *passwd++;
	}
	*str= 0;
	if(password_str[0]){	//We have a password
		for(; i < str-password_str; i++) {
			display_password_str[i]= '*';
		}
		display_password_str[i]=0;
	}
}

void change_poor_man(int *poor_man)
{
	*poor_man= !*poor_man;
	if(*poor_man) {
		show_reflection= 0;
		shadows_on= 0;
		clouds_shadows= 0;
		use_shadow_mapping= 0;
#ifndef MAP_EDITOR2
#ifdef SFX
		special_effects= 0;
#endif
		use_fog= 0;
#endif
#ifdef	TERRAIN
		use_normal_mapping= 0;
#endif
#ifdef	USE_FRAMEBUFFER
		use_frame_buffer= 0;
		update_fbo_and_shadow_mapping();
#else
		//...and the texture used for shadow mapping
		glDeleteTextures(1, &depth_map_id);
		depth_map_id= 0;
#endif
	}
#ifdef SFX
	ec_set_draw_detail();
#endif
}

#ifdef	NOT_USED
void change_vertex_array(int *pointer)
{
	*pointer= !*pointer;
	if(use_vertex_array) {
		LOG_TO_CONSOLE(c_green2,enabled_vertex_arrays);
	}
}
#endif


void change_compiled_vertex_array(int *value)
{
	if (*value) {
		*value= 0;
	}
	else if (!gl_extensions_loaded || have_compiled_vertex_array)
	{
		// don't check if we have hardware support when OpenGL
		// extensions are not initialized yet.
		*value= 1;
	}
	else LOG_TO_CONSOLE(c_green2,disabled_compiled_vertex_arrays);
}

void change_point_particles(int *value)
{
	if (*value) {
		*value= 0;
	}
	else if (!gl_extensions_loaded || have_point_sprite)
	{
		// don't check if we have hardware support when OpenGL
		// extensions are not initialized yet.
		*value= 1;
	}
	else
	{
		LOG_TO_CONSOLE(c_green2, disabled_point_particles);
	}
	
#ifdef SFX
	ec_set_draw_method();
#endif
}

void change_particles_percentage(int *pointer, int value)
{
	if(value>0 && value <=100) {
		particles_percentage= value;
	}
	else
	{
		particles_percentage= 0;
		LOG_TO_CONSOLE(c_green2, disabled_particles_str);
	}
}

void switch_vidmode(int *pointer, int mode)
{
	int win_width,
		win_height,
		win_bpp;

	int flags = SDL_OPENGL;

	if(mode>18 || mode<1) {
		//warn about this error
		LOG_TO_CONSOLE(c_red2,invalid_video_mode);
		return;
	} else if(!video_mode_set) {
		/* Video isn't ready yet, just remember the mode */
		video_mode= mode;
		return;
	}
	/* Check if the video mode is supported. */
	switch(mode) {
		case 1:
			win_width= 640;
			win_height= 480;
			win_bpp= 16;
		break;
		case 2:
			win_width= 640;
			win_height= 480;
			win_bpp= 32;
		break;
		case 3:
			win_width= 800;
			win_height= 600;
			win_bpp= 16;
		break;
		case 4:
			win_width= 800;
			win_height= 600;
			win_bpp= 32;
		break;
		case 5:
			win_width= 1024;
			win_height= 768;
			win_bpp= 16;
		break;
		case 6:
			win_width= 1024;
			win_height= 768;
			win_bpp= 32;
		break;
		case 7:
			win_width= 1152;
			win_height= 864;
			win_bpp= 16;
		break;
		case 8:
			win_width= 1152;
			win_height= 864;
			win_bpp= 32;
		break;
		case 9:
			win_width= 1280;
			win_height= 1024;
			win_bpp= 16;
		break;
		case 10:
			win_width= 1280;
			win_height= 1024;
			win_bpp= 32;
		break;
		case 11:
			win_width= 1600;
			win_height= 1200;
			win_bpp= 16;
		break;
		case 12:
			win_width= 1600;
			win_height= 1200;
			win_bpp= 32;
		break;
		case 13:
			win_width= 1280;
			win_height= 800;
			win_bpp= 16;
		break;
		case 14:
			win_width= 1280;
			win_height= 800;
			win_bpp= 32;
		break;
		case 15:
			win_width= 1440;
			win_height= 900;
			win_bpp= 16;
		break;
		case 16:
			win_width= 1440;
			win_height= 900;
			win_bpp= 32;
		break;
		case 17:
			win_width= 1680;
			win_height= 1050;
			win_bpp= 16;
		break;
		case 18:
			win_width= 1680;
			win_height= 1050;
			win_bpp= 32;
		break;
		default:
			win_width= 640;
			win_height= 480;
			win_bpp= 16;
		break;
	}

//#ifndef OSX
//	if(!SDL_VideoModeOK(win_width, win_height, win_bpp, SDL_OPENGL|SDL_FULLSCREEN)) {
//#else
	if(full_screen) flags |= SDL_FULLSCREEN;
	if(!SDL_VideoModeOK(win_width, win_height, win_bpp, flags)) {
//#endif
		LOG_TO_CONSOLE(c_red2, invalid_video_mode);
	} else {
		set_new_video_mode(full_screen, mode);
#ifndef MAP_EDITOR2
		if(items_win >= 0) {
			windows_list.window[items_win].show_handler(&windows_list.window[items_win]);
		}
#endif
	}
#ifdef	USE_FRAMEBUFFER
	update_fbo_and_shadow_mapping();
#endif
}

void toggle_full_screen_mode(int * fs)
{
	if(!video_mode_set) {
		*fs= !*fs;
	} else {
		toggle_full_screen();
	}
}

void change_shadow_map_size(int *pointer, int value)
{
	const int array[10]= {256, 512, 768, 1024, 1280, 1536, 1792, 2048, 3072, 4096};
	int index, size, i, max_size, error;
	char error_str[1024];

	if (value >= array[0]) {
		index= 0;
		for(i= 0; i < 10; i++) {
			/* Check if we can set the multiselect widget to this */
			if(array[i] == value) {
				index= i;
				break;
			}
		}
	} else {
		index= min2i(max2i(0, value), 9);
	}

	size= array[index];

	if (gl_extensions_loaded && use_shadow_mapping && have_arb_shadow)
	{
		error= 0;

#ifdef	USE_FRAMEBUFFER
		if (use_frame_buffer && have_framebuffer_object) glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE_EXT, &max_size);
		else max_size= min2i(window_width, window_height);
#else
		max_size= min2i(window_width, window_height);
#endif

		if (size > max_size) {
			while ((size > max_size) && (index > 0)) {
				index--;
				size= array[index];
			}
			error= 1;
		}

		if (!have_texture_non_power_of_two)
		{
			switch (index)
			{
				case 2:
					index= 1;
					error= 1;
					break;
				case 4:
				case 5:
				case 6:
					index= 3;
					error= 1;
					break;
				case 8:
					index= 7;
					error= 1;
					break;
			}
			size= array[index];
		}
		if (error == 1) {
			memset(error_str, 0, sizeof(error_str));
			snprintf(error_str, sizeof(error_str), shadow_map_size_not_supported_str, size);
			LOG_TO_CONSOLE(c_yellow2, error_str);
		}

		shadow_map_size= size;
#ifdef	USE_FRAMEBUFFER
		if (depth_map_id == 0) {
			glDeleteTextures(1, &depth_map_id);
			depth_map_id= 0;
		}
		if (gl_extensions_loaded && have_framebuffer_object)
		{
			if (use_frame_buffer && use_shadow_mapping && have_arb_shadow && shadows_on) {
				change_shadow_framebuffer_size();
			} else {
				free_shadow_framebuffer();
			}
			if (use_frame_buffer && show_reflection) {
				change_reflection_framebuffer_size(window_width, window_height);
			} else {
				free_reflection_framebuffer();
			}
		}
#else
		glDeleteTextures(1, &depth_map_id);
		depth_map_id= 0;
#endif
	}

	if (pointer != NULL) {
		*pointer= index;
	}
	shadow_map_size= size;
}

void change_compass_direction(int *dir)
{
	compass_direction= 1-2 * (*dir>0);
	*dir= !*dir;
}

#ifndef MAP_EDITOR2
void set_afk_time(int *pointer, int time)
{
	if(time > 0) {
		afk_time= time*60000;
		*pointer= time;
	} else {
		afk_time= 0;
		*pointer= 0;
	}
}

void change_windowed_chat (int *wc, int val)
{
	int old_wc= *wc;

	*wc= val;
	if (*wc == 1)
	{
		if (game_root_win >= 0)
		{
			display_tab_bar ();
		}
	}
	else if (tab_bar_win >= 0)
	{
		hide_window (tab_bar_win);
	}

	if (*wc == 2)
	{
		if (game_root_win >= 0)
		{
			window_info *win;
			display_chat();
			widget_move_win(input_widget->window_id, input_widget->id, chat_win);
			widget_set_flags(input_widget->window_id, input_widget->id, TEXT_FIELD_BORDER|TEXT_FIELD_EDITABLE|TEXT_FIELD_NO_KEYPRESS);
			win= &windows_list.window[chat_win];
			resize_chat_handler(win, win->len_x, win->len_y);
		}
	}
	else if (chat_win >= 0)
	{
		window_info *win;
		int target_win= game_root_win;
		hide_window (chat_win);
		if(get_show_window(game_root_win)) {
			target_win= game_root_win;
			if(input_text_line.len > 0) {
				widget_unset_flag(input_widget->window_id, input_widget->id, WIDGET_INVISIBLE);
			} else {
				widget_set_flags(input_widget->window_id, input_widget->id, WIDGET_INVISIBLE);
			}
		} else if(get_show_window(console_root_win)) {
			target_win= console_root_win;
		} else if(get_show_window(map_root_win)) {
			target_win= map_root_win;
		}
		win= &windows_list.window[target_win];
		widget_move_win(input_widget->window_id, input_widget->id, target_win);
		widget_resize (input_widget->window_id, input_widget->id, win->len_x-HUD_MARGIN_X, input_widget->len_y);
		widget_move (input_widget->window_id, input_widget->id, 0, win->len_y-input_widget->len_y-HUD_MARGIN_Y);
		widget_set_flags(input_widget->window_id, input_widget->id, INPUT_DEFAULT_FLAGS);
		if(target_win == console_root_win) {
			widget_unset_flag(input_widget->window_id, input_widget->id, WIDGET_CLICK_TRANSPARENT);
		}
	}

	if (old_wc != *wc && (old_wc == 1 || old_wc == 2) )
	{
		convert_tabs (*wc);
	}
}


void change_quickbar_relocatable (int *rel)
{
	*rel= !*rel;
	if (quickbar_win >= 0)
	{
		init_quickbar ();
	}
}

void change_chat_zoom(float *dest, float *value)
{
	if (*value < 0.0f) {
		return;
	}
	*dest= *value;
	if (opening_root_win >= 0 || console_root_win >= 0 || chat_win >= 0 || game_root_win >= 0) {
		if (opening_root_win >= 0) {
			opening_win_update_zoom();
		}
		if (console_root_win >= 0) {
			nr_console_lines= (int) (window_height - input_widget->len_y - CONSOLE_SEP_HEIGHT - hud_y - 10) / (18 * chat_zoom);
			widget_set_size(console_root_win, console_out_id, *value);
		}
		if (chat_win >= 0) {
			chat_win_update_zoom();
		}
	}
	if(input_widget != NULL) {
		text_field *tf= input_widget->widget_info;
		widget_set_size(input_widget->window_id, input_widget->id, *value);
		if(use_windowed_chat != 2) {
			widget_resize(input_widget->window_id, input_widget->id, input_widget->len_x, tf->y_space*2 + ceilf(DEFAULT_FONT_Y_LEN*input_widget->size*tf->nr_lines));
		}
	}
}

#endif
#endif // def ELC

void change_dir_name (char *var, const char *str, int len)
{
	int idx;

	for (idx= 0; idx < len && str[idx]; idx++) {
		var[idx]= str[idx];
	}
	if (var[idx-1] != '/') {
		var[idx++]= '/';
	}
	var[idx]= '\0';
}

#ifdef ANTI_ALIAS
void change_aa(int *pointer) {
	change_var(pointer);
	if (anti_alias) {
		glHint(GL_POINT_SMOOTH_HINT,   GL_NICEST);
		glHint(GL_LINE_SMOOTH_HINT,    GL_NICEST);
		glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
		glEnable(GL_POINT_SMOOTH);
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_POLYGON_SMOOTH);
	} else {
		glHint(GL_POINT_SMOOTH_HINT,   GL_FASTEST);
		glHint(GL_LINE_SMOOTH_HINT,    GL_FASTEST);
		glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);
		glDisable(GL_POINT_SMOOTH);
		glDisable(GL_LINE_SMOOTH);
		glDisable(GL_POLYGON_SMOOTH);
	}
}
#endif // ANTI_ALIAS
#ifdef ELC
#ifdef OSX
void change_projection_float_init(float * var, float * value) {
	change_float(var, value);
}

void change_projection_bool_init(int *pointer) {
	change_var(pointer);
}
#endif //OSX
void change_projection_float(float * var, float * value) {
	change_float(var, value);
	resize_root_window();
}

void change_projection_bool(int *pointer) {
	change_var(pointer);
	resize_root_window();
}

void change_gamma(float *pointer, float *value)
{
	*pointer= *value;
	if(video_mode_set) {
		SDL_SetGamma(*value, *value, *value);
	}
}

#ifndef MAP_EDITOR2
void change_windows_on_top(int *var)
{
	*var=!*var;
	if (*var) {
		// Change the root windows
		move_window(storage_win, -1, 0, storage_win_x, storage_win_y);
		move_window(manufacture_win, -1, 0, manufacture_menu_x, manufacture_menu_y);
		move_window(items_win, -1, 0, items_menu_x, items_menu_y);
		move_window(buddy_win, -1, 0, buddy_menu_x, buddy_menu_y);
		move_window(ground_items_win, -1, 0, ground_items_menu_x, ground_items_menu_y);
		move_window(sigil_win, -1, 0, sigil_menu_x, sigil_menu_y);
		move_window(elconfig_win, -1, 0, elconfig_menu_x, elconfig_menu_y);
		move_window(tab_stats_win, -1, 0, tab_stats_x, tab_stats_y);
		move_window(server_popup_win, -1, 0, server_popup_win_x, server_popup_win_y);
		// Display any open windows (checking they exist first)
		if (storage_win > 0) {
			if (windows_list.window[storage_win].displayed != 0 || windows_list.window[storage_win].reinstate != 0) {
				show_window(storage_win);
			}
		}
		if (manufacture_win > 0) {
			if (windows_list.window[manufacture_win].displayed != 0 || windows_list.window[manufacture_win].reinstate != 0) {
				show_window(manufacture_win);
			}
		}
		if (items_win > 0) {
			if (windows_list.window[items_win].displayed != 0 || windows_list.window[items_win].reinstate != 0) {
				show_window(items_win);
			}
		}
		if (buddy_win > 0) {
			if (windows_list.window[buddy_win].displayed != 0 || windows_list.window[buddy_win].reinstate != 0) {
				show_window(buddy_win);
			}
		}
		if (ground_items_win > 0) {
			if (windows_list.window[ground_items_win].displayed != 0 || windows_list.window[ground_items_win].reinstate != 0) {
				show_window(ground_items_win);
			}
		}
		if (sigil_win > 0) {
			if (windows_list.window[sigil_win].displayed != 0 || windows_list.window[sigil_win].reinstate != 0) {
				show_window(sigil_win);
			}
		}
		if (elconfig_win > 0) {
			if (windows_list.window[elconfig_win].displayed != 0 || windows_list.window[elconfig_win].reinstate != 0) {
				show_window(elconfig_win);
			}
		}
		if (tab_stats_win > 0) {
			if (windows_list.window[tab_stats_win].displayed != 0 || windows_list.window[tab_stats_win].reinstate != 0) {
				show_window(tab_stats_win);
			}
		}
		if (server_popup_win > 0) {
			if (windows_list.window[server_popup_win].displayed != 0 || windows_list.window[server_popup_win].reinstate != 0) {
				show_window(server_popup_win);
			}
		}
	} else {
		// Change the root windows
		move_window(storage_win, game_root_win, 0, storage_win_x, storage_win_y);
		move_window(manufacture_win, game_root_win, 0, manufacture_menu_x, manufacture_menu_y);
		move_window(items_win, game_root_win, 0, items_menu_x, items_menu_y);
		move_window(buddy_win, game_root_win, 0, buddy_menu_x, buddy_menu_y);
		move_window(ground_items_win, game_root_win, 0, ground_items_menu_x, ground_items_menu_y);
		move_window(sigil_win, game_root_win, 0, sigil_menu_x, sigil_menu_y);
		move_window(elconfig_win, game_root_win, 0, elconfig_menu_x, elconfig_menu_y);
		move_window(tab_stats_win, game_root_win, 0, tab_stats_x, tab_stats_y);
		move_window(server_popup_win, game_root_win, 0, server_popup_win_x, server_popup_win_y);
		// Hide all the windows if needed
		if (windows_list.window[game_root_win].displayed == 0) {
			hide_window(game_root_win);
		}
	}
}
#endif

#ifndef MAP_EDITOR2
void change_separate_flag(int * pointer) {
	change_var(pointer);

	if (chat_win >= 0) {
		update_chat_win_buffers();
	}
}
#endif

void change_shadow_mapping (int *sm)
{
	if (*sm)
	{
		*sm= 0;
	}
	else
	{
		// don't check if we have hardware support when OpenGL
		// extensions are not initialized yet.
		if (!gl_extensions_loaded || (have_multitexture >= 3 && have_arb_shadow))
		{
			*sm= 1;
		}
		else
		{
			LOG_TO_CONSOLE (c_red1, disabled_shadow_mapping);
		}
	}
#ifdef	USE_FRAMEBUFFER
	update_fbo_and_shadow_mapping();
#else
	glDeleteTextures(1, &depth_map_id);
	depth_map_id= 0;
#endif //USE_FRAMEBUFFER
}

#ifndef MAP_EDITOR2
void change_global_filters (int *use)
{
	*use= !*use;
	// load global filters when new value is true, but only when changed
	// in game, not on startup
	if (options_set && *use) {
		load_filters_list ("global_filters.txt", 0);
	}
}
#endif //ndef MAP_EDITOR2

#ifdef	TERRAIN
void change_normal_mapping(int *nm)
{
	if (*nm) {
		*nm= 0;
	} else {
		// don't check if we have hardware support when OpenGL
		// extensions are not initialized yet.
		if (!gl_extensions_loaded || (have_multitexture >= 4 && have_ogsl_vertex_shader && have_ogsl_pixel_shader)) {
			*nm= 1;
		} else {
			LOG_TO_CONSOLE (c_red1, disabled_normal_mapping);
		}
	}
}
#endif // TERRAIN
#endif // ELC

#ifdef	USE_FRAMEBUFFER
void change_reflection(int *rf)
{
	*rf= !*rf;
	update_fbo_and_shadow_mapping();
}

void change_frame_buffer(int *fb)
{
	if (*fb)
	{
		*fb= 0;
	}
	else
	{
		if (!gl_extensions_loaded || have_framebuffer_object)
		{
			*fb= 1;
		}
		else
		{
			LOG_TO_CONSOLE (c_red1, disabled_framebuffer);
		}
	}
	update_fbo_and_shadow_mapping();
}
#endif //USE_FRAMEBUFFER

void change_shadows(int *sh)
{
	*sh= !*sh;
#ifdef	USE_FRAMEBUFFER
	update_fbo_and_shadow_mapping();
#else
	//...and the texture used for shadow mapping
#ifndef OSX
	glDeleteTextures(1, &depth_map_id);
#endif //OSX
	depth_map_id= 0;
#endif
}

#ifdef MAP_EDITOR

void set_auto_save_interval (int *save_time, int time)
{
	if(time>0) {
		*save_time= time*60000;
	} else {
		*save_time= 0;
	}
}

void switch_vidmode(int mode)
{
	switch(mode)
		{
			case 1: window_width=640;
				window_height=480;
				return;
			case 2: window_width=780;
				window_height=550;
				return;
			case 3:
				window_width=990;
				window_height=720;
				return;
			case 4:
				window_width=1070;
				window_height=785;
				return;
			case 5:
				window_width=1250;
				window_height=990;
				return;
			case 6:
				window_width=1600;
				window_height=1200;
			case 7:
				window_width=1280;
				window_height=800;
			default:
				return;
		}
}

#endif

int find_var (char *str, var_name_type type)
{
	int i, isvar;

	for(i= 0; i < our_vars.no; i++)
	{
		if (type != COMMAND_LINE_SHORT_VAR)
			isvar= !strncmp(str, our_vars.var[i]->name, our_vars.var[i]->nlen);
		else
			isvar= !strncmp(str, our_vars.var[i]->shortname, our_vars.var[i]->snlen);
		if (isvar)
			return i;
	}
	return -1;
}

static __inline__ void check_option_var(char* name)
{
	int i;
	int value_i;
	float value_f;
	char* value_s;

	i= find_var(name, IN_GAME_VAR);
	if (i < 0)
	{
		return;
	}

	switch (our_vars.var[i]->type)
	{
		case INT:
		case MULTI:
			value_i= *((int*)our_vars.var[i]->var);
			our_vars.var[i]->func (our_vars.var[i]->var, value_i);
			break;
		case BOOL:
			value_i= *((int*)our_vars.var[i]->var);
			if (value_i == 0) *((int*)our_vars.var[i]->var)= 1;
			else *((int*)our_vars.var[i]->var)= 0;
			our_vars.var[i]->func (our_vars.var[i]->var);
			break;
		case STRING:
		case PASSWORD:
			value_s= (char*)our_vars.var[i]->var;
			our_vars.var[i]->func (our_vars.var[i]->var, value_s, our_vars.var[i]->len);
			break;
		case FLOAT:
			value_f= *((float*)our_vars.var[i]->var);
			our_vars.var[i]->func (our_vars.var[i]->var, value_f);
			break;
	}
}

void check_options()
{
	check_option_var("use_point_particles");
#ifdef	USE_FRAMEBUFFER
	check_option_var("use_frame_buffer");
#endif
	check_option_var("use_shadow_mapping");
	check_option_var("shadow_map_size");
#ifdef	TERRAIN
	check_option_var("use_normal_mapping");
#endif
}

int check_var (char *str, var_name_type type)
{
	int i, *p;
	char *ptr= str;
	float foo;

	i= find_var (str, type);
	if (i < 0)
	{
		return -1;
	}

	ptr += (type != COMMAND_LINE_SHORT_VAR) ? our_vars.var[i]->nlen : our_vars.var[i]->snlen;

	while (*ptr && (*ptr== ' ' || *ptr == '='))
		ptr++;	// go to the string occurence
	if (!*ptr || *ptr == 0x0d || *ptr == 0x0a)
		return -1;	// hmm, why would you do such a stupid thing?

	if (*ptr == '"')
	{
		//Accurate quoting
		char *tptr= ++ptr;
		while (*tptr && *tptr != '"')
		{
			if (*tptr == 0x0a || *tptr == 0x0d)
			{
#ifdef ELC
				char str[200];
				snprintf (str, sizeof(str), "Reached newline without an ending \" in %s", our_vars.var[i]->name);
				LOG_TO_CONSOLE(c_red2,str);
#endif // ELC
				break;
			}
			tptr++;
		}
		*tptr= 0;
	}
	else
	{
		// Strip it
		char our_string[200];
		char *tptr= our_string;
		while (*ptr && *ptr != 0x0a && *ptr != 0x0d)
		{
			if (*ptr != ' ')
				*tptr++= *ptr++; //Strip all spaces
			else
				ptr++;
		}
		*tptr= 0;
		ptr= our_string;
	}

	if (type == INI_FILE_VAR)
		our_vars.var[i]->saved= 1;
	else if (type == IN_GAME_VAR)
		// make sure in-game changes are stored in el.ini
		our_vars.var[i]->saved= 0;

	switch (our_vars.var[i]->type)
	{
		case INT:
		case MULTI:
			our_vars.var[i]->func ( our_vars.var[i]->var, atoi (ptr) );
			return 1;
		case BOOL:
			p= our_vars.var[i]->var;
			if ((atoi (ptr) > 0) != *p)
				our_vars.var[i]->func (our_vars.var[i]->var); //only call if value has changed
			return 1;
		case STRING:
		case PASSWORD:
			our_vars.var[i]->func (our_vars.var[i]->var, ptr, our_vars.var[i]->len);
			return 1;
		case FLOAT:
			foo= atof (ptr);
			our_vars.var[i]->func (our_vars.var[i]->var, &foo);
			return 1;
	}
	return -1;
}

void free_vars()
{
	int i;
	for(i= 0; i < our_vars.no; i++)
	{
		switch(our_vars.var[i]->type) {
			case INT:
			case FLOAT:
				queue_destroy(our_vars.var[i]->queue);
			break;
			case MULTI:
				if(our_vars.var[i]->queue != NULL) {
					while(!queue_isempty(our_vars.var[i]->queue)) {
						//We don't free() because it's not allocated.
						queue_pop(our_vars.var[i]->queue);
					}
					queue_destroy(our_vars.var[i]->queue);
				}
			break;
		}
#ifndef OPTIONS_I18N
		free(our_vars.var[i]->short_desc);
		free(our_vars.var[i]->long_desc);
#endif
		free(our_vars.var[i]);
	}
	our_vars.no=0;
}

void add_var(int type, char * name, char * shortname, void * var, void * func, float def, char * short_desc, char * long_desc, int tab_id, ...)
{
	int *integer=var;
	float *f=var;
	int no=our_vars.no++;
	char *pointer;
	float *tmp_f;
	point *tmp_i;
	va_list ap;

	our_vars.var[no]=(var_struct*)calloc(1,sizeof(var_struct));
	switch(our_vars.var[no]->type=type)
	{
		case MULTI:
			queue_initialise(&our_vars.var[no]->queue);
			va_start(ap, tab_id);
			while((pointer= va_arg(ap, char *)) != NULL) {
				queue_push(our_vars.var[no]->queue, pointer);
			}
			va_end(ap);
			*integer= (int)def;
		break;
		case INT:
			queue_initialise(&our_vars.var[no]->queue);
			va_start(ap, tab_id);
			//Min
			tmp_i= calloc(1,sizeof(*tmp_i));
			*tmp_i= va_arg(ap, point);
			queue_push(our_vars.var[no]->queue, tmp_i);
			//Max
			tmp_i= calloc(1,sizeof(*tmp_i));
			*tmp_i= va_arg(ap, point);
			queue_push(our_vars.var[no]->queue, tmp_i);
			va_end(ap);
			*integer= (int)def;
		break;
		case BOOL:
			*integer=(int)def;
			break;
		case STRING:
		case PASSWORD:
			our_vars.var[no]->len=(int)def;
			break;
		case FLOAT:
			queue_initialise(&our_vars.var[no]->queue);
			va_start(ap, tab_id);
			//Min
			tmp_f= calloc(1,sizeof(*tmp_f));
			*tmp_f= va_arg(ap, double);
			queue_push(our_vars.var[no]->queue, (void *)tmp_f);
			//Max
			tmp_f= calloc(1,sizeof(*tmp_f));
			*tmp_f= va_arg(ap, double);
			queue_push(our_vars.var[no]->queue, (void *)tmp_f);
			//Interval
			tmp_f= calloc(1,sizeof(*tmp_f));
			*tmp_f= va_arg(ap, double);
			queue_push(our_vars.var[no]->queue, (void *)tmp_f);
			va_end(ap);
			*f=def;
			break;
	}
	our_vars.var[no]->var=var;
	our_vars.var[no]->func=func;
	our_vars.var[no]->name=name;
	our_vars.var[no]->shortname=shortname;
	our_vars.var[no]->nlen=strlen(our_vars.var[no]->name);
	our_vars.var[no]->snlen=strlen(our_vars.var[no]->shortname);
	our_vars.var[no]->saved= 0;
#ifdef OPTIONS_I18N
	add_options_distringid(name, &our_vars.var[no]->display, short_desc, long_desc);
#else
	our_vars.var[no]->short_desc= malloc(strlen(short_desc)+1);
	strcpy(our_vars.var[no]->short_desc, short_desc);
	our_vars.var[no]->long_desc= malloc(strlen(long_desc)+1);
	strcpy(our_vars.var[no]->long_desc, long_desc);
#endif
	our_vars.var[no]->widgets.tab_id= tab_id;
}

void add_multi_option(char * name, char * str) {
	queue_push(our_vars.var[find_var (name, MULTI)]->queue, str);
}

void init_vars()
{
	//ELC specific variables
#ifdef ELC
	add_var(BOOL,"full_screen","fs",&full_screen,toggle_full_screen_mode,0,"Full Screen","Changes between full screen and windowed mode",VIDEO);
#ifndef MAP_EDITOR2
 #ifdef DEBUG
	add_var(BOOL,"render_skeleton","rskel",&render_skeleton,change_var,0,"Render Skeleton", "Render the Cal3d skeletons.", SPECIALVID);
	add_var(BOOL,"render_mesh","rmesh",&render_mesh,change_var,1,"Render Mesh", "Render the meshes", SPECIALVID);
 #endif//DEBUG
#endif //MAP_EDITOR2
	add_var(BOOL,"shadows_on","shad",&shadows_on,change_shadows,0,"Shadows","Toggles the shadows",VIDEO);
	add_var (BOOL, "use_shadow_mapping", "sm", &use_shadow_mapping, change_shadow_mapping, 0, "Shadow Mapping", "If you want to use some better quality shadows, enable this. It will use more resources, but look prettier.", VIDEO);
	add_var(MULTI,"shadow_map_size","smsize",&shadow_map_size_multi,change_shadow_map_size,1024,"Shadow Map Size","This parameter determines the quality of the shadow maps. You should as minimum set it to 512.",VIDEO,"256","512","768","1024","1280","1536","1792","2048","3072","4096",NULL);
#ifndef MAP_EDITOR2
	add_var(BOOL,"render_fog","fog",&use_fog,change_var,1,"Render Fog","Toggles fog rendering.",VIDEO);
#endif	//MAP_EDITOR2
	add_var(BOOL,"poor_man","poor",&poor_man,change_poor_man,0,"Poor Man","Toggles the poor man option for slower systems",VIDEO);
#ifdef	NEW_ALPHA
	add_var(BOOL,"use_3d_alpha_blend","3dalpha",&use_3d_alpha_blend,change_var,1,"3D Alpha Blending","Toggle the use of the alpha blending on 3D objects",VIDEO);
#endif	//NEW_ALPHA
#ifdef	USE_FRAMEBUFFER
	add_var(BOOL,"show_reflection","refl",&show_reflection,change_reflection,1,"Show Reflections","Toggle the reflections",VIDEO);
#else
	add_var(BOOL,"show_reflection","refl",&show_reflection,change_var,1,"Show Reflections","Toggle the reflections",VIDEO);
#endif
	add_var(BOOL,"no_adjust_shadows","noadj",&no_adjust_shadows,change_var,0,"Don't Adjust Shadows","If enabled, tell the engine not to disable the shadows if the frame rate is too low.",SPECIALVID);
	add_var(BOOL,"clouds_shadows","cshad",&clouds_shadows,change_var,1,"Cloud Shadows","The clouds shadows are projected on the ground, and the game looks nicer with them on.",SPECIALVID);
	add_var(BOOL,"show_fps","fps",&show_fps,change_var,1,"Show FPS","Show the current frames per second in the corner of the window",HUD);
	add_var(BOOL,"use_mipmaps","mm",&use_mipmaps,change_var,1,"Mipmaps","Mipmaps is a texture effect that blurs the texture a bit - it may look smoother and better, or it may look worse depending on your graphics driver settings and the like.",SPECIALVID);
	add_var(BOOL,"use_compiled_vertex_array","cva",&use_compiled_vertex_array,change_compiled_vertex_array,1,"Compiled Vertex Array","Some systems will not support the new compiled vertex array in EL. Disable this if some 3D objects do not display correctly.",SPECIALVID);
#ifndef MAP_EDITOR
#ifdef	NOT_USED
	add_var(BOOL,"use_vertex_array","vertex",&use_vertex_array,change_vertex_array,0,"Vertex Array","Toggle the use of the vertex array",SPECIALVID);
#endif
	add_var(BOOL,"use_vertex_buffers","vbo",&use_vertex_buffers,change_var,0,"Vertex Buffer objects","Toggle the use of the vertex buffer objects, restart required to activate it",SPECIALVID);

	add_var(INT,"mouse_limit","lmouse",&mouse_limit,change_int,15,"Mouse Limit","You can increase the mouse sensitivity and cursor changing by adjusting this number to lower numbers, but usually the FPS will drop as well!",CONTROLS,1,INT_MAX);
	add_var(BOOL,"use_point_particles","upp",&use_point_particles,change_point_particles,1,"Point Particles","Some systems will not support the new point based particles in EL. Disable this if your client complains about not having the point based particles extension.",SPECIALVID);
	add_var(INT,"particles_percentage","pp",&particles_percentage,change_particles_percentage,100,"Particle Percentage","If you experience a significant slowdown when particles are nearby, you should consider lowering this number.",SPECIALVID,0,100);
	add_var(BOOL,"enable_blood","eb",&enable_blood,change_int,0,"Enable Blood","Enable blood special effects during combat.",SPECIALVID);
 #ifdef	TERRAIN
	add_var (BOOL, "use_normal_mapping", "nm", &use_normal_mapping, change_normal_mapping, 0, "Normal Mapping", "If you want to use some better quality terrain, enable this. It will use more resources, but look prettier.", SPECIALVID);
 #endif // TERRAIN
#endif // ELC

	// Lachesis: this var is not used in the code.
	// add_var(INT,"click_speed","cspeed",&click_speed,change_int,300,"Click Speed","Set the mouse click speed",CONTROLS,0,INT_MAX);

	add_var(FLOAT,"normal_camera_rotation_speed","nrot",&normal_camera_rotation_speed,change_float,15,"Camera Rotation Speed","Set the speed the camera rotates",CONTROLS,1.0,FLT_MAX,0.5);
	add_var(FLOAT,"fine_camera_rotation_speed","frot",&fine_camera_rotation_speed,change_float,1,"Fine Rotation Speed","Set the fine camera rotation speed (when holding shift+arrow key)",CONTROLS,1.0,FLT_MAX,0.5);

#ifndef MAP_EDITOR2
	add_var(FLOAT,"name_text_size","nsize",&name_zoom,change_float,1,"Name Text Size","Set the size of the players name text",FONT,0.0,2.0,0.01);
#endif
#ifdef ELC
 #ifndef MAP_EDITOR2
	add_var(FLOAT,"chat_text_size","csize",&chat_zoom,change_chat_zoom,1,"Chat Text Size","Sets the size of the normal text",FONT,0.0,FLT_MAX,0.01);
 #endif	//MAP_EDITOR2
 #ifndef FONTS_FIX
	add_var(MULTI,"name_font","nfont",&name_font,change_int,0,"Name Font","Change the type of font used for the name",FONT,"Type 1", "Type 2", NULL);
	add_var(MULTI,"chat_font","cfont",&chat_font,change_int,0,"Chat Font","Set the type of font used for normal text",FONT,"Type 1", "Type 2", NULL);
 #else	//FONTS_FIX
	add_var(MULTI,"name_font","nfont",&name_font,change_int,0,"Name Font","Change the type of font used for the name",FONT, NULL);
	add_var(MULTI,"chat_font","cfont",&chat_font,change_int,0,"Chat Font","Set the type of font used for normal text",FONT, NULL);
 #endif	//FONTS_FIX
#else	//ELC
	add_var(INT,"name_font","nfont",&name_font,change_int,0,"Name Font","Change the type of font used for the name",FONT,1,3);
	add_var(INT,"chat_font","cfont",&chat_font,change_int,0,"Chat Font","Set the type of font used for normal text",FONT,1,3);
#endif	//ELC

#ifdef NEW_SOUND
	add_var(MULTI,"sounds","sounds",&sound_opts,change_sounds,3,"Sounds","Turn sound effects on/off",AUDIO, "No sound", "Environmental Only", "+General Character", "+Walking", NULL);
#else
	add_var(BOOL,"enable_sound","sound",&sound_on,toggle_sounds,0,"Enable Sound Effects","Turn sound effects on/off",AUDIO);
#endif	//NEW_SOUND
#ifndef NO_MUSIC
	add_var(BOOL,"enable_music","music",&music_on,toggle_music,0,"Enable Music","Turn music on/off",AUDIO);
#endif //NO_MUSIC
	add_var(FLOAT,"sound_gain","sgain",&sound_gain,change_sound_level,1,"Sound Gain","Adjust the sound effects volume",AUDIO,0.0,1.0,0.1);
#ifndef NO_MUSIC
	add_var(FLOAT,"music_gain","mgain",&music_gain,change_sound_level,1,"Music Gain","Adjust the music volume",AUDIO,0.0,1.0,0.1);
#endif //NO_MUSIC

#ifndef MAP_EDITOR2
	add_var(BOOL,"sit_lock","sl",&sit_lock,change_var,0,"Sit Lock","Enable this to prevent your character from moving by accident when you are sitting.",CONTROLS);

	add_var(BOOL,"item_window_on_drop","itemdrop",&item_window_on_drop,change_var,1,"Item Window On Drop","Toggle whether the item window shows when you drop items",CONTROLS);
	add_var (BOOL, "use_floating_messages", "floating", &floatingmessages_enabled, change_var, 1, "Floating Messages", "Toggles the use of floating experience messages and other graphical enhancements", CONTROLS);
#endif
	add_var(BOOL,"view_digital_clock","digit",&view_digital_clock,change_var,1,"Digital Clock","Toggle the digital clock",HUD);
#ifndef MAP_EDITOR2
	add_var(BOOL,"show_stats_in_hud","sstats",&show_stats_in_hud,change_var,0,"Stats In HUD","Toggle showing stats in the HUD",HUD);
	add_var(BOOL,"show_statbars_in_hud","sstatbars",&show_statbars_in_hud,change_var,0,"StatBars In HUD","Toggle showing statbars in the HUD. Needs Stats in HUD",HUD);
#endif
	add_var(BOOL,"show_help_text","shelp",&show_help_text,change_var,1,"Help Text","Enable tooltips.",HUD);
#ifndef MAP_EDITOR2
	add_var(BOOL, "relocate_quickbar", "requick", &quickbar_relocatable, change_quickbar_relocatable, 0,"Relocate Quickbar","Set whether you can move the quickbar",HUD);
#endif
	add_var(BOOL,"compass_north","comp",&compass_direction_checkbox,change_compass_direction,1,"Compass Direction","Set the compass direction for a static compass",HUD);
	add_var (BOOL, "use_alpha_border", "aborder", &use_alpha_border, change_var, 1,"Alpha Border","Toggle the use of alpha borders",HUD);	//SPECIALVID);

#ifndef MAP_EDITOR2
	add_var(SPECINT,"auto_afk_time","afkt",&afk_time_conf,set_afk_time,5,"AFK Time","The idle time in minutes before the AFK auto message",MISC,0,INT_MAX);
	add_var(STRING,"afk_message","afkm",afk_message,change_string,127,"AFK Message","Set the AFK message",MISC);
#ifdef AFK_FIX
	add_var(BOOL, "afk_local", "afkl", &afk_local,change_var, 0, "Save Local Chat Messages When AFK", "When you go AFK, local chat messages are counted and saved as well as PMs", MISC);
#endif //AFK_FIX
#endif

#ifndef MAP_EDITOR2
	add_var(BOOL,"use_global_ignores","gign",&use_global_ignores,change_var,1,"Global Ignores","Global ignores is a list with people that are well known for being nasty, so we put them into a list (global_ignores.txt). Enable this to load that list on startup.",MISC);
	add_var(BOOL,"save_ignores","sign",&save_ignores,change_var,1,"Save Ignores","Toggle saving of the local ignores list on exit.",MISC);
	add_var (BOOL, "use_global_filters", "gfil", &use_global_filters, change_global_filters, 1, "Global Filter", "Toggle the use of global text filters.", MISC);
	/* add_var(STRING,"text_filter_replace","trepl",text_filter_replace,change_string,127,"Text Filter","The word to replace bad text with",MISC); */
	add_var(BOOL,"caps_filter","caps",&caps_filter,change_var,1,"Caps filter","Toggle the caps filter",MISC);

	add_var(STRING,"server_address","sa",server_address,change_string,70,"Server Address","The address of the EL server",SERVER);
	add_var(INT,"server_port","sp",&port,change_int,2000,"Server Port","Where on the server to connect.",SERVER,1,65536);
	add_var(STRING,"username","u",username_str,change_string,16,"Username","Your user name here",SERVER);
	add_var(PASSWORD,"password","p",password_str,change_string,16,"Password","Put your password here",SERVER);

#endif
#ifndef MAP_EDITOR2
 #ifdef ELC
 	add_var(MULTI,"log_chat","log",&log_chat,change_int,2,"Log Messages","Log messages from the server (chat, harvesting events, GMs, etc)",SERVER,"Do not log chat", "Log chat only", "Log server messages", "Log server to srv_log.txt", NULL);
 #else
	add_var(INT,"log_chat","log",&log_chat,change_int,2,"Log Messages","Log messages from the server (harvesting events, GMs, etc)",SERVER);
 #endif //ELC
	add_var(BOOL,"serverpopup","spu",&use_server_pop_win,change_var,1,"Use special text window","Toggles whether server messages from channel 255 are displayed in a pop up window.",SERVER);
  #ifdef AUTO_UPDATE
     /* Note: We don't take any action on the already-running thread, as that wouldn't necessarily be good. */
	add_var(BOOL,"autoupdate","aup",&auto_update,change_var,1,"Automatic Updates","Toggles whether updates are automatically downloaded.",SERVER);
  #ifdef CUSTOM_UPDATE
	add_var(BOOL,"customupdate","cup",&custom_update,change_var,1,"Custom Looks Updates","Toggles whether custom look updates are automatically downloaded.",SERVER);
  #endif    //CUSTOM_UPDATE
  #endif    //AUTO_UPDATE
 	add_var(STRING,"language","lang",lang,change_string,8,"Language","Wah?",MISC);
 	add_var(STRING,"browser","b",browser_name,change_string,70,"Browser","Location of your web browser (Windows users leave blank to use default browser)",MISC);
#endif // MAP_EDITOR2

#ifndef MAP_EDITOR2
#ifdef ELC
	add_var (MULTI,"windowed_chat", "winchat", &use_windowed_chat, change_windowed_chat, 1, "Use Windowed Chat", "How do you want your chat to be displayed?", CHAT, "Old behavior", "Tabbed chat", "Chat window", NULL);
 #else
	add_var (INT,"windowed_chat", "winchat", &use_windowed_chat, change_windowed_chat, 1, "Use Windowed Chat", "0= Old behavior, 1= new behavior, 2=chat window", CHAT);
#endif //ELC
#endif
	add_var (BOOL, "write_ini_on_exit", "wini", &write_ini_on_exit, change_var, 1,"Save INI","Save options when you quit",MISC);
	// Grum: attempt to work around bug in Ati linux drivers.
	add_var (BOOL, "ati_click_workaround", "atibug", &ati_click_workaround, change_var, 0, "ATI Bug", "If you are using an ATI card and don't move when you click, try this option to work around a bug in their drivers", SPECIALVID);
#ifndef MAP_EDITOR2
	add_var (BOOL, "use_old_clicker", "oldmclick", &use_old_clicker, change_var, 0, "Mouse Bug", "If the above option doesn't move you when you click, try this one", SPECIALVID);
 #ifdef ELC
	add_var (BOOL, "local_chat_separate", "locsep", &local_chat_separate, change_separate_flag, 0, "Separate Local Chat", "Should local chat be separate?", CHAT);
	// The forces that be want PMs always global, so that they're less likely to be ignored
	//add_var (BOOL, "personal_chat_separate", "pmsep", &personal_chat_separate, change_separate_flag, 0, "Separate Personal Chat", "Should personal chat be separate?", CHAT);
	add_var (BOOL, "guild_chat_separate", "gmsep", &guild_chat_separate, change_separate_flag, 1, "Separate Guild chat", "Should guild chat be separate?", CHAT);
	add_var (BOOL, "server_chat_separate", "scsep", &server_chat_separate, change_separate_flag, 0, "Separate Server Messages", "Should the messages from the server be separate?", CHAT);
	add_var (BOOL, "mod_chat_separate", "modsep", &mod_chat_separate, change_separate_flag, 0, "Separate Moderator Chat", "Should moderator chat be separated from the rest?", CHAT);
  #else
	add_var (BOOL, "local_chat_separate", "locsep", &local_chat_separate, change_var, 0, "Separate Local Chat", "Should local chat be separate?", CHAT);
	//add_var (BOOL, "personal_chat_separate", "pmsep", &personal_chat_separate, change_var, 0, "Separate personal chat", "Should personal chat be separate?", CHAT);
	add_var (BOOL, "guild_chat_separate", "gmsep", &guild_chat_separate, change_var, 1, "Separate Guild Chat", "Should guild chat be separate?", CHAT);
	add_var (BOOL, "server_chat_separate", "scsep", &server_chat_separate, change_var, 0, "Separate Server Messages", "Should the messages from the server be separate?", CHAT);
	add_var (BOOL, "mod_chat_separate", "modsep", &mod_chat_separate, change_var, 0, "Separate Moderator Chat", "Should moderator chat be separated from the rest?", CHAT);
 #endif
	add_var (BOOL, "highlight_tab_on_nick", "highlight", &highlight_tab_on_nick, change_var, 1, "Highlight Tabs On Name", "Should tabs be highlighted when someone mentions your name?", CHAT);
#endif
#ifndef OSX
	add_var (BOOL, "isometric" ,"isometric", &isometric, change_projection_bool, 1, "Use Isometric View", "Toggle the use of isometric (instead of perspective) view", VIDEO);
	add_var (FLOAT, "perspective", "perspective", &perspective, change_projection_float, 0.15f, "Perspective", "The degree of perspective distortion. Change if your view looks odd.", SPECIALVID, 0.01, 0.80, 0.01);
	add_var (FLOAT, "near_plane", "near_plane", &near_plane, change_projection_float, 40, "Near Plane Distance", "The distance of the near clipping plane to your actor", SPECIALVID, 1.0, 60.0, 0.5);
#else
        add_var (BOOL, "isometric" ,"isometric", &isometric, change_projection_bool_init, 1, "Use Isometric View, restart required", "Toggle the use of isometric (instead of perspective) view", VIDEO);
	add_var (FLOAT, "perspective", "perspective", &perspective, change_projection_float_init, 0.15f, "Perspective", "The degree of perspective distortion. Change if your view looks odd.", SPECIALVID, 0.01, 0.80, 0.01);
	add_var (FLOAT, "near_plane", "near_plane", &near_plane, change_projection_float_init, 40, "Near Plane Distance", "The distance of the near clipping plane to your actor", SPECIALVID, 1.0, 60.0, 0.5);
#endif
 #ifdef ANTI_ALIAS
	add_var (BOOL, "anti_alias", "aa", &anti_alias, change_aa, 0, "Toggle Anti-Aliasing", "Anti-aliasing makes edges look smoother", SPECIALVID);
 #endif //ANTI_ALIAS
#ifdef SFX
	add_var (BOOL, "special_effects", "sfx", &special_effects, change_var, 1, "Toggle Special Effects", "Special spell effects", SPECIALVID);
#endif //SFX
#ifndef MAP_EDITOR2
	add_var (BOOL, "buddy_log_notice", "buddy_log_notice", &buddy_log_notice, change_var, 1, "Log Buddy Sign On/Off", "Toggle whether to display notices when people on your buddy list log on or off", MISC);
#endif
#endif // def ELC
#ifdef	USE_FRAMEBUFFER
	add_var (BOOL, "use_frame_buffer", "fb", &use_frame_buffer, change_frame_buffer, 0, "Toggle Frame Buffer Support", "Toggle frame buffer support. Used for reflection and shadow mapping.", SPECIALVID);
#endif // TERRAIN

	//Global vars...
	// Only possible to do at startup - this could of course be changed by using a special function for this purpose. I just don't see why you'd want to change the directory whilst running the game...
	add_var(STRING,"data_dir","dir",datadir,change_dir_name,90,"Data Directory","Place were we keep our data. Can only be changed with a Client restart.",MISC);
#ifdef ELC
	add_var(BOOL, "windows_on_top", "wot", &windows_on_top, change_windows_on_top, 0, "Windows On Top","Allows the Manufacture, Storage and Inventory windows to appear above the map and console.", MISC);
	add_var(MULTI,"video_mode","vid",&video_mode,switch_vidmode,4,"Video Mode","The video mode you wish to use",VIDEO, "", "640x480x16", "640x480x32", "800x600x16", "800x600x32", "1024x768x16", "1024x768x32", "1152x864x16", "1152x864x32", "1280x1024x16", "1280x1024x32", "1600x1200x16", "1600x1200x32", "1280x800x16", "1280x800x32", "1440x900x16", "1440x900x32", "1680x1050x16", "1680x1050x32", NULL);
#else
	add_var(SPECINT,"video_mode","vid",&video_mode,switch_vidmode,4,"Video Mode","The video mode you wish to use",VIDEO);
#endif //ELC
	add_var(INT,"limit_fps","lfps",&limit_fps,change_int,0,"Limit FPS","Limit the frame rate to reduce load on the system",VIDEO,0,INT_MAX);
#ifdef ELC
	add_var(FLOAT,"gamma","g",&gamma_var,change_gamma,1,"Gamma","How bright your display should be.",SPECIALVID,0.10,3.00,0.05);
#endif //ELC
#ifdef MAP_EDITOR
	add_var(BOOL,"close_browser_on_select","cbos",&close_browser_on_select, change_var, 0,"Close Browser","Close the browser on select",MISC);
	add_var(BOOL,"show_position_on_minimap","spos",&show_position_on_minimap, change_var, 0,"Show POS","Show position on the minimap",HUD);
	add_var(SPECINT,"auto_save","asv",&auto_save_time, set_auto_save_interval, 0,"Auto Save","Auto Save",MISC,0,INT_MAX);
	add_var(BOOL,"show_grid","sgrid",&view_grid, change_var, 0, "Show Grid", "Show grid",HUD);
#endif

#if defined ELC && ! defined MAP_EDITOR2
#if !defined(WINDOWS) && !defined(OSX)
	add_var(BOOL,"use_clipboard","uclb",&use_clipboard, change_var, 1, "Use Clipboard For Pasting", "Use CLIPBOARD for pasting (as e.g. GNOME does) or use PRIMARY cutbuffer (as xterm does)",MISC);
#endif
#endif
}

void write_var (FILE *fout, int ivar)
{
	if (fout == NULL) return;

	switch (our_vars.var[ivar]->type)
	{
		case INT:
		case MULTI:
		case BOOL:
		{
			int *p= our_vars.var[ivar]->var;
			fprintf (fout, "#%s= %d\n", our_vars.var[ivar]->name, *p);
			break;
		}
		case STRING:
			if (strcmp (our_vars.var[ivar]->name, "password") == 0)
				// Do not write the password to the file. If the user really wants it
				// s/he should edit the file.
				fprintf (fout, "#%s= \"\"\n", our_vars.var[ivar]->name);
			else
				fprintf (fout, "#%s= \"%s\"\n", our_vars.var[ivar]->name, (char *)our_vars.var[ivar]->var);
			break;
		case PASSWORD:
			// Do not write the password to the file. If the user really wants it
			// s/he should edit the file.
			fprintf (fout, "#%s= \"\"\n", our_vars.var[ivar]->name);
			break;
		case FLOAT:
		{
			float *g= our_vars.var[ivar]->var;
			fprintf (fout, "#%s= %g\n", our_vars.var[ivar]->name, *g);
			break;
		}
	}
	our_vars.var[ivar]->saved= 1;	// keep only one copy of this setting
}

FILE* open_el_ini (const char *mode)
{
#ifdef WINDOWS
	return my_fopen ("el.ini", mode);
#else
	char el_ini[256];
	char el_tmp[256];
	FILE *f;
	mode_t modes;

	snprintf (el_ini, sizeof (el_ini), "%s/el.ini", configdir);
	f= my_fopen (el_ini, mode);	// try local file first
	if (f == NULL)
	{
		FILE *f2;
		int flen;
		char *data;

		//OK, no local el.ini - copy the defaults
		snprintf (el_ini, sizeof (el_ini), "%s/el.ini", datadir);
		f= fopen(el_ini, mode);

		if(f == NULL)
		{
			return NULL;//Shit, no global el.ini either? Fortunately we'll write one on exit...
		}

		snprintf(el_tmp, sizeof (el_tmp), "%s/el.ini", configdir);
		f2= my_fopen (el_tmp, "w");

		if(f2 == NULL) {
			//Hmm... we cannot create a file in ~/.elc/
			fclose(f);
			return NULL;
		}

		//Copy the data from the global el.ini to the ~/.elc/el.ini

		fseek(f, 0, SEEK_END);
		flen=ftell(f);
		fseek(f, 0, SEEK_SET);

		data=calloc(flen+1, sizeof(char));

		fread(data, flen, sizeof(char), f);
		fwrite(data, flen, sizeof(char), f2);

		fclose(f);
		fclose(f2);
		free(data);

		//Now load it as read-only
		snprintf(el_ini, sizeof (el_ini), "%s/el.ini", configdir);
		f= my_fopen(el_ini, mode);
	}

	if(f) {
		struct stat statbuff;
		stat(el_ini,&statbuff);
		modes= statbuff.st_mode;
		/* Set perms to 600 on el_ini if they are anything else */
		if(((modes & S_IRWXU) == (S_IRUSR|S_IWUSR)) &&
		   ((modes & S_IRWXG) == (S_IRGRP|S_IWGRP|S_IXGRP)) &&
		   ((modes & S_IRWXO) == (S_IROTH|S_IWOTH|S_IXOTH))) {
			chmod(el_ini,S_IRUSR|S_IWUSR);
		}
	}

	return f;
#endif //WINDOWS
}

int read_el_ini ()
{
	input_line line;
	FILE *fin= open_el_ini ("r");

	if (fin == NULL) return 0;

	while ( fgets (line, sizeof (input_line), fin) )
	{
		if (line[0] == '#')
			check_var (&(line[1]), INI_FILE_VAR);	//check only for the long strings
	}

	fclose (fin);
	return 1;
}

int write_el_ini ()
{
	int nlines= 0, maxlines= 0, iline, ivar;
	input_line *cont= NULL;
	FILE *file;

	// first check if we need to change anything
	//
	// The advantage of skipping this check is that a new el.ini would be
	// created in the users $HOME/.elc for Unix users, even if nothing
	// changed. However, most of the time it's pointless to update an
	// unchanged file.
	for (ivar= 0; ivar < our_vars.no; ivar++)
	{
		if (!our_vars.var[ivar]->saved)
			break;
	}
	if (ivar >= our_vars.no)
		return 1; // nothing changed, no need to write

	// read the ini file
	file= open_el_ini ("r");
	if (file != NULL)
	{
		maxlines= 300;
	 	cont= malloc (maxlines * sizeof (input_line));
		while (fgets (cont[nlines], sizeof (input_line), file) != NULL)
		{
			if (++nlines >= maxlines)
			{
				maxlines *= 2;
				cont= realloc (cont, maxlines * sizeof (input_line));
			}
		}
		fclose (file);
	}

	// Now write the contents of the file, updating those variables that have been changed
	file= open_el_ini ("w");
	if (file == NULL)
	{
		return 0;
	}

	for (iline= 0; iline < nlines; iline++)
	{
		if (cont[iline][0] != '#')
		{
			fprintf (file, "%s", cont[iline]);
		}
		else
		{
			ivar= find_var (&(cont[iline][1]), 1);
			if (ivar < 0 || our_vars.var[ivar]->saved)
				fprintf (file, "%s", cont[iline]);
			else
				write_var (file, ivar);
		}
	}

	// now write all variables that still haven't been saved yet
	for (ivar= 0; ivar < our_vars.no; ivar++)
	{
		if (!our_vars.var[ivar]->saved)
		{
			fprintf (file, "\n");
			write_var (file, ivar);
		}
	}

	fclose (file);
	free (cont);
	return 1;
}

/* ------ ELConfig Window functions start here ------ */
#ifdef ELC
int display_elconfig_handler(window_info *win)
{
	int i;

	for(i= 0; i < our_vars.no; i++)
	{
		//Update the widgets in case an option changed
		switch(our_vars.var[i]->type)
		{
			case BOOL:
				//Nothing to do for BOOL. The checkbox widget takes care of that.
			break;
			case MULTI:
				multiselect_set_selected(elconfig_tabs[our_vars.var[i]->widgets.tab_id].tab, our_vars.var[i]->widgets.widget_id, *(int *)our_vars.var[i]->var);
			break;
			case INT:
				//Nothing to do here either.
			break;
		}
	}
	//Draw the long description of an option
	draw_string_small(TAB_MARGIN, elconfig_menu_y_len-LONG_DESC_SPACE, elconf_description_buffer, MAX_LONG_DESC_LINES);
	return 1;
}

int spinbutton_onkey_handler(widget_list *widget, int mx, int my, Uint32 key, Uint32 unikey)
{
	if(widget != NULL) {
		int i;
		spinbutton *button;

		if (!(key&ELW_ALT) && !(key&ELW_CTRL)) {
			for(i= 0; i < our_vars.no; i++) {
				if(our_vars.var[i]->widgets.widget_id == widget->id) {
					button= widget->widget_info;
					switch(button->type) {
						case SPIN_FLOAT:
							our_vars.var[i]->func(our_vars.var[i]->var, (float *)button->data);
						break;
						case SPIN_INT:
							our_vars.var[i]->func(our_vars.var[i]->var, *(int *)button->data);
						break;
					}
					our_vars.var[i]->saved= 0;
					return 0;
				}
			}
		}
	}
	return 0;
}

int spinbutton_onclick_handler(widget_list *widget, int mx, int my, Uint32 flags)
{
	if(widget != NULL) {
		int i;
		spinbutton *button;

		for(i= 0; i < our_vars.no; i++) {
			if(our_vars.var[i]->widgets.widget_id == widget->id) {
				button= widget->widget_info;
				switch(button->type) {
					case SPIN_FLOAT:
						our_vars.var[i]->func(our_vars.var[i]->var, (float *)button->data);
					break;
					case SPIN_INT:
						our_vars.var[i]->func(our_vars.var[i]->var, *(int *)button->data);
					break;
				}
				our_vars.var[i]->saved= 0;
				return 0;
			}
		}
	}
	return 0;
}

int multiselect_click_handler(widget_list *widget, int mx, int my, Uint32 flags)
{
	int i;
	if(flags&ELW_LEFT_MOUSE || flags&ELW_RIGHT_MOUSE) {
		for(i= 0; i < our_vars.no; i++) {
			if(our_vars.var[i]->widgets.widget_id == widget->id) {
				our_vars.var[i]->func ( our_vars.var[i]->var, multiselect_get_selected(elconfig_tabs[our_vars.var[i]->widgets.tab_id].tab, our_vars.var[i]->widgets.widget_id) );
				our_vars.var[i]->saved= 0;
				return 1;
			}
		}
	}
	return 0;
}

int mouseover_option_handler(widget_list *widget, int mx, int my)
{
	int i;

	//Find the label in our_vars
	for(i= 0; i < our_vars.no; i++) {
		if(our_vars.var[i]->widgets.label_id == widget->id || widget->id == our_vars.var[i]->widgets.widget_id) {
			break;
		}
	}
	if(i == our_vars.no) {
		//We didn't find anything, abort
		return 0;
	}
#ifdef OPTIONS_I18N
	put_small_text_in_box(our_vars.var[i]->display.desc, strlen(our_vars.var[i]->display.desc),
								elconfig_menu_x_len-TAB_MARGIN*2, elconf_description_buffer);
#else
	put_small_text_in_box(our_vars.var[i]->long_desc, strlen(our_vars.var[i]->long_desc),
								elconfig_menu_x_len-TAB_MARGIN*2, elconf_description_buffer);
#endif
	return 1;
}

int onclick_label_handler(widget_list *widget, int mx, int my, Uint32 flags)
{
	int i;
	var_struct *option= NULL;

	for(i= 0; i < our_vars.no; i++) {
		if(our_vars.var[i]->widgets.label_id == widget->id) {
			option= our_vars.var[i];
			break;
		}
	}

	if (option == NULL)
	{
		// option not found, not supposed to happen
		return 0;
	}

	switch(option->type) {
		case BOOL:
			option->func(option->var);
			option->saved= 0;
		break;
	}
	return 1;
}

int onclick_checkbox_handler(widget_list *widget, int mx, int my, Uint32 flags)
{
	int i;
	var_struct *option= NULL;

	for(i= 0; i < our_vars.no; i++) {
		if(our_vars.var[i]->widgets.widget_id == widget->id) {
			option= our_vars.var[i];
			break;
		}
	}
	switch(option->type) {
		case BOOL:
			{
				int *var= option->var;
				*var= !*var;
				option->func(var);
				option->saved= 0;
			}
		break;
	}
	return 1;
}

int string_onkey_handler(widget_list *widget)
{
	// dummy key handler that marks the appropriate variable as changed
	if(widget != NULL)
	{
		int i;

		for(i= 0; i < our_vars.no; i++)
		{
			if(our_vars.var[i]->widgets.widget_id == widget->id)
			{
				our_vars.var[i]->saved= 0;
				return 1;
			}
		}
	}

	return 0;
}

void elconfig_populate_tabs(void)
{
	int i;
	int tab_id; //temporary storage for the tab id
	int label_id=-1; //temporary storage for the label id
	int widget_id=-1; //temporary storage for the widget id
	int widget_height, label_height; //Used to calculate the y pos of the next option
	int y; //Used for the position of multiselect buttons
	void *min, *max; //For the spinbuttons
	float *interval;

	for(i= 0; i < MAX_TABS; i++) {
		//Set default values
		elconfig_tabs[i].x= 5;
		elconfig_tabs[i].y= 5;
	}

	for(i= 0; i < our_vars.no; i++) {
		tab_id= our_vars.var[i]->widgets.tab_id;
		switch(our_vars.var[i]->type) {
			case BOOL:
				//Add checkbox
				widget_id= checkbox_add_extended(elconfig_tabs[tab_id].tab, elconfig_free_widget_id++, NULL,
											elconfig_tabs[tab_id].x, elconfig_tabs[tab_id].y, CHECKBOX_SIZE, CHECKBOX_SIZE, 0, 1.0, 0.77f, 0.59f, 0.39f, our_vars.var[i]->var);
				//Add label for the checkbox
#ifdef OPTIONS_I18N
				label_id= label_add(elconfig_tabs[tab_id].tab, NULL, our_vars.var[i]->display.str, elconfig_tabs[tab_id].x+CHECKBOX_SIZE+SPACING, elconfig_tabs[tab_id].y);
#else
				label_id= label_add(elconfig_tabs[tab_id].tab, NULL, our_vars.var[i]->short_desc, elconfig_tabs[tab_id].x+CHECKBOX_SIZE+SPACING, elconfig_tabs[tab_id].y);
#endif
				//Set handlers
				widget_set_OnClick(elconfig_tabs[tab_id].tab, label_id, onclick_label_handler);
				widget_set_OnClick(elconfig_tabs[tab_id].tab, widget_id, onclick_checkbox_handler);
			break;
			case INT:
				min= queue_pop(our_vars.var[i]->queue);
				max= queue_pop(our_vars.var[i]->queue);
				/* interval is always 1 */

#ifdef OPTIONS_I18N
				label_id= label_add_extended(elconfig_tabs[tab_id].tab, elconfig_free_widget_id++, NULL, elconfig_tabs[tab_id].x, elconfig_tabs[tab_id].y, 0, 1.0, 0.77f, 0.59f, 0.39f, our_vars.var[i]->display.str);
#else
				label_id= label_add_extended(elconfig_tabs[tab_id].tab, elconfig_free_widget_id++, NULL, elconfig_tabs[tab_id].x, elconfig_tabs[tab_id].y, 0, 1.0, 0.77f, 0.59f, 0.39f, our_vars.var[i]->short_desc);
#endif
				widget_id= spinbutton_add(elconfig_tabs[tab_id].tab, NULL, elconfig_menu_x_len/2, elconfig_tabs[tab_id].y, 100, 20, SPIN_INT, our_vars.var[i]->var, *(int *)min, *(int *)max, 1.0);
				widget_set_OnKey(elconfig_tabs[tab_id].tab, widget_id, spinbutton_onkey_handler);
				widget_set_OnClick(elconfig_tabs[tab_id].tab, widget_id, spinbutton_onclick_handler);
				free(min);
				free(max);
				queue_destroy(our_vars.var[i]->queue);
				our_vars.var[i]->queue= NULL;
			break;
			case FLOAT:
				min= queue_pop(our_vars.var[i]->queue);
				max= queue_pop(our_vars.var[i]->queue);
				interval= (float *)queue_pop(our_vars.var[i]->queue);

#ifdef OPTIONS_I18N
				label_id= label_add_extended(elconfig_tabs[tab_id].tab, elconfig_free_widget_id++, NULL, elconfig_tabs[tab_id].x, elconfig_tabs[tab_id].y, 0, 1.0, 0.77f, 0.59f, 0.39f, our_vars.var[i]->display.str);
#else
				label_id= label_add_extended(elconfig_tabs[tab_id].tab, elconfig_free_widget_id++, NULL, elconfig_tabs[tab_id].x, elconfig_tabs[tab_id].y, 0, 1.0, 0.77f, 0.59f, 0.39f, our_vars.var[i]->short_desc);
#endif

				widget_id= spinbutton_add(elconfig_tabs[tab_id].tab, NULL, elconfig_menu_x_len/2, elconfig_tabs[tab_id].y, 100, 20, SPIN_FLOAT, our_vars.var[i]->var, *(float *)min, *(float *)max, *interval);
				widget_set_OnKey(elconfig_tabs[tab_id].tab, widget_id, spinbutton_onkey_handler);
				widget_set_OnClick(elconfig_tabs[tab_id].tab, widget_id, spinbutton_onclick_handler);
				free(min);
				free(max);
				free(interval);
				queue_destroy(our_vars.var[i]->queue);
				our_vars.var[i]->queue= NULL;
			break;
			case STRING:

#ifdef OPTIONS_I18N
				label_id= label_add_extended(elconfig_tabs[tab_id].tab, elconfig_free_widget_id++, NULL, elconfig_tabs[tab_id].x, elconfig_tabs[tab_id].y, 0, 1.0, 0.77f, 0.59f, 0.39f, our_vars.var[i]->display.str);
#else
				label_id= label_add_extended(elconfig_tabs[tab_id].tab, elconfig_free_widget_id++, NULL, elconfig_tabs[tab_id].x, elconfig_tabs[tab_id].y, 0, 1.0, 0.77f, 0.59f, 0.39f, our_vars.var[i]->short_desc);
#endif
				widget_id= pword_field_add_extended(elconfig_tabs[tab_id].tab, elconfig_free_widget_id++, NULL, elconfig_menu_x_len/3, elconfig_tabs[tab_id].y, 332, 20, P_TEXT, 1.0f, 0.77f, 0.59f, 0.39f, our_vars.var[i]->var, our_vars.var[i]->len);
				widget_set_OnKey (elconfig_tabs[tab_id].tab, widget_id, string_onkey_handler);
			break;
			case PASSWORD:
				// Grum: the client shouldn't store the password, so let's not add it to the configuration window
#ifdef OPTIONS_I18N
				//label_id= label_add_extended(elconfig_tabs[tab_id].tab, elconfig_free_widget_id++, NULL, elconfig_tabs[tab_id].x, elconfig_tabs[tab_id].y, 0, 0, 0, 1.0, 0.77f, 0.59f, 0.39f, our_vars.var[i]->display.str);
#else
				//label_id= label_add_extended(elconfig_tabs[tab_id].tab, elconfig_free_widget_id++, NULL, elconfig_tabs[tab_id].x, elconfig_tabs[tab_id].y, 0, 0, 0, 1.0, 0.77f, 0.59f, 0.39f, our_vars.var[i]->short_desc);
#endif
				//widget_id= pword_field_add_extended(elconfig_tabs[tab_id].tab, elconfig_free_widget_id++, NULL, elconfig_menu_x_len/2, elconfig_tabs[tab_id].y, 200, 20, P_NORMAL, 1.0f, 0.77f, 0.59f, 0.39f, our_vars.var[i]->var, our_vars.var[i]->len);
				//widget_set_OnKey (elconfig_tabs[tab_id].tab, widget_id, string_onkey_handler);
			break;
			case MULTI:

#ifdef OPTIONS_I18N
				label_id= label_add_extended(elconfig_tabs[tab_id].tab, elconfig_free_widget_id++, NULL, elconfig_tabs[tab_id].x, elconfig_tabs[tab_id].y, 0, 1.0, 0.77f, 0.59f, 0.39f, our_vars.var[i]->display.str);
				widget_id= multiselect_add_extended(elconfig_tabs[tab_id].tab, elconfig_free_widget_id++, NULL, elconfig_tabs[tab_id].x+SPACING+get_string_width(our_vars.var[i]->display.str), elconfig_tabs[tab_id].y, 250, 80, 1.0f, 0.77f, 0.59f, 0.39f, 0.32f, 0.23f, 0.15f, 0);
#else
				label_id= label_add_extended(elconfig_tabs[tab_id].tab, elconfig_free_widget_id++, NULL, elconfig_tabs[tab_id].x, elconfig_tabs[tab_id].y, 0, 1.0, 0.77f, 0.59f, 0.39f, our_vars.var[i]->short_desc);
				widget_id= multiselect_add_extended(elconfig_tabs[tab_id].tab, elconfig_free_widget_id++, NULL, elconfig_tabs[tab_id].x+SPACING+get_string_width(our_vars.var[i]->short_desc), elconfig_tabs[tab_id].y, 250, 80, 1.0f, 0.77f, 0.59f, 0.39f, 0.32f, 0.23f, 0.15f, 0);
#endif
				for(y= 0; !queue_isempty(our_vars.var[i]->queue); y++) {
					char *label= queue_pop(our_vars.var[i]->queue);
					int width= strlen(label) > 0 ? 0 : -1;

					multiselect_button_add_extended(elconfig_tabs[tab_id].tab, widget_id, 0, y*(22+SPACING), width, label, DEFAULT_SMALL_RATIO, y == *(int *)our_vars.var[i]->var);
					if(strlen(label) == 0) {
						y--;
					}
				}
				widget_set_OnClick(elconfig_tabs[tab_id].tab, widget_id, multiselect_click_handler);
				queue_destroy(our_vars.var[i]->queue);
				our_vars.var[i]->queue= NULL;
			break;
		}
		//Calculate y position of the next option.
		label_height= widget_find(elconfig_tabs[tab_id].tab, label_id)->len_y;
		widget_height= widget_find(elconfig_tabs[tab_id].tab, widget_id)->len_y;
		elconfig_tabs[tab_id].y += (widget_height > label_height ? widget_height : label_height)+SPACING;
		//Set IDs
		our_vars.var[i]->widgets.label_id= label_id;
		our_vars.var[i]->widgets.widget_id= widget_id;
		//Make the description print when the mouse is over a widget
		widget_set_OnMouseover(elconfig_tabs[tab_id].tab, label_id, mouseover_option_handler);
		widget_set_OnMouseover(elconfig_tabs[tab_id].tab, widget_id, mouseover_option_handler);
	}
}

// TODO: replace this hack by something clean.
int show_elconfig_handler(window_info * win) {
	int pwinx, pwiny; window_info *pwin;

	if (win->pos_id != -1) {
		pwin= &windows_list.window[win->pos_id];
		pwinx= pwin->cur_x;
		pwiny= pwin->cur_y;
	} else {
		pwinx= 0;
		pwiny= 0;
	}
#ifndef MAP_EDITOR2
	if (get_show_window(newchar_root_win)) {
		init_window(win->window_id, newchar_root_win, 0, win->pos_x - pwinx, win->pos_y - pwiny, win->len_x, win->len_y);
	} else {
		int our_root_win= -1;
		if (!windows_on_top) {
			our_root_win= game_root_win;
		}
		init_window(win->window_id, our_root_win, 0, win->pos_x - pwinx, win->pos_y - pwiny, win->len_x, win->len_y);
	}
#else
	init_window(win->window_id, game_root_win, 0, win->pos_x - pwinx, win->pos_y - pwiny, win->len_x, win->len_y);
#endif
	return 1;
}

void display_elconfig_win(void)
{
	if(elconfig_win < 0) {
		int our_root_win= -1;

		if (!windows_on_top) {
			our_root_win= game_root_win;
		}

		/* Set up the window */
		elconfig_win= create_window(win_configuration, our_root_win, 0, elconfig_menu_x, elconfig_menu_y, elconfig_menu_x_len, elconfig_menu_y_len, ELW_WIN_DEFAULT);
		set_window_color(elconfig_win, ELW_COLOR_BORDER, 0.77f, 0.59f, 0.39f, 0.0f);
		set_window_handler(elconfig_win, ELW_HANDLER_DISPLAY, &display_elconfig_handler );
		// TODO: replace this hack by something clean.
		set_window_handler(elconfig_win, ELW_HANDLER_SHOW, &show_elconfig_handler);
		/* Create tabs */
		elconfig_tab_collection_id= tab_collection_add_extended (elconfig_win, elconfig_tab_collection_id, NULL, TAB_MARGIN, TAB_MARGIN, elconfig_menu_x_len-TAB_MARGIN*2, elconfig_menu_y_len-TAB_MARGIN*2-LONG_DESC_SPACE, 0, 0.7, 0.77f, 0.57f, 0.39f, MAX_TABS, TAB_TAG_HEIGHT);
		elconfig_tabs[CONTROLS].tab= tab_add(elconfig_win, elconfig_tab_collection_id, ttab_controls, 0, 0);
		elconfig_tabs[AUDIO].tab= tab_add(elconfig_win, elconfig_tab_collection_id, ttab_audio, 0, 0);
		elconfig_tabs[HUD].tab= tab_add(elconfig_win, elconfig_tab_collection_id, ttab_hud, 0, 0);
		elconfig_tabs[SERVER].tab= tab_add(elconfig_win, elconfig_tab_collection_id, ttab_server, 0, 0);
		elconfig_tabs[MISC].tab= tab_add(elconfig_win, elconfig_tab_collection_id, ttab_misc, 0, 0);
		elconfig_tabs[FONT].tab= tab_add(elconfig_win, elconfig_tab_collection_id, ttab_font, 0, 0);
		elconfig_tabs[CHAT].tab= tab_add(elconfig_win, elconfig_tab_collection_id, ttab_chat, 0, 0);
		elconfig_tabs[VIDEO].tab= tab_add(elconfig_win, elconfig_tab_collection_id, ttab_video, 0, 0);
		elconfig_tabs[SPECIALVID].tab= tab_add(elconfig_win, elconfig_tab_collection_id, ttab_specialvideo, 0, 0);

		elconfig_populate_tabs();
	}
	show_window(elconfig_win);
	select_window(elconfig_win);
}
#endif //ELC
