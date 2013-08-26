#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#include <float.h>
#include <errno.h>
//For stat() etc.. below
#include <sys/types.h>
#include <sys/stat.h>
#ifndef _MSC_VER
 #include <unistd.h>
#endif //_MSC_VER
#include "user_menus.h"

#ifdef MAP_EDITOR
 #include "map_editor/global.h"
 #include "map_editor/browser.h"
 #include "map_editor/interface.h"
 #include "load_gl_extensions.h"
#else
 #include "achievements.h"
 #include "alphamap.h"
 #include "bags.h"
 #include "buddy.h"
 #include "chat.h"
 #include "console.h"
 #include "counters.h"
 #include "dialogues.h"
 #include "draw_scene.h"
 #include "errors.h"
 #include "elwindows.h"
 #include "filter.h"
 #include "gamewin.h"
 #include "gl_init.h"
 #include "hud.h"
 #include "init.h"
 #include "interface.h"
 #include "items.h"
 #include "item_info.h"
 #include "manufacture.h"
 #include "map.h"
 #include "mapwin.h"
 #include "missiles.h"
 #include "multiplayer.h"
 #include "new_character.h"
 #include "openingwin.h"
 #include "particles.h"
 #include "pm_log.h"
 #include "questlog.h"
 #include "reflection.h"
 #include "serverpopup.h"
 #include "session.h"
 #include "shadows.h"
 #include "sound.h"
 #include "spells.h"
 #include "stats.h"
 #include "storage.h"
 #include "tabs.h"
 #include "trade.h"
 #include "trade_log.h"
 #include "weather.h"
 #include "minimap.h"
 #ifdef NEW_ALPHA
  #include "3d_objects.h"
 #endif
 #include "io/elpathwrapper.h"
 #include "notepad.h"
 #include "sky.h"
 #ifdef OSX
  #include "events.h"
 #endif // OSX
#endif

#include "asc.h"
#include "elconfig.h"
#include "text.h"
#include "consolewin.h"
#include "queue.h"
#include "url.h"
#include "widgets.h"

#include "eye_candy_wrapper.h"
#include "sendvideoinfo.h"
#include "actor_init.h"
#include "io/elpathwrapper.h"
#ifdef	NEW_TEXTURES
 #include "textures.h"
#endif	/* NEW_TEXTURES */
#ifdef	FSAA
 #include "fsaa/fsaa.h"
#endif	/* FSAA */
#ifdef	CUSTOM_UPDATE
 #include "custom_update.h"
#endif	/* CUSTOM_UPDATE */

typedef	float (*float_min_max_func)();
typedef	int (*int_min_max_func)();

// Defines for config variables
#define CONTROLS	0
#define HUD		1
#define CHAT		2
#define FONT 		3
#define SERVER		4
#define AUDIO		5
#define VIDEO		6
#define GFX		7
#define CAMERA		8
#define TROUBLESHOOT	9


#ifdef DEBUG
 #define DEBUGTAB	10
 #define MAX_TABS	11
#else
 #define MAX_TABS	10
#endif

#define CHECKBOX_SIZE		15
#define SPACING			5	//Space between widgets and labels and lines
#define LONG_DESC_SPACE		50	//Space to give to the long descriptions
#define MAX_LONG_DESC_LINES	3	//How many lines of text we can fit in LONG_DESC_SPACE

typedef char input_line[256];

/*!
 * var_struct stores the data for a single configuration entry.
 */
typedef struct
{
	option_type type; /*!< type of the variable */
	char	*name; /*!< name of the variable */
	int 	nlen; /*!< length of the \a name */
	char 	*shortname; /*!< shortname of the variable */
	int 	snlen; /*!< length of the \a shortname */
	void 	(*func)(); /*!< routine to execute when this variable is selected. */
	void 	*var; /*!< data for this variable */
	int 	len; /*!< length of the variable */
	int	saved;
//	char 	*message; /*!< In case you want a message to be written when a setting is changed */
	dichar display;
	struct {
		int tab_id; /*!< The tab ID in which we find this option */
		int label_id; /*!< The label ID associated with this option */
		int widget_id; /*!< Widget ID for things like checkboxes */
	} widgets;
	queue_t *queue; /*!< Queue that holds info for certain widget types. */
} var_struct;

/*!
 * a list of variables of type \see var_struct
 */
struct variables
{
	int no; /*!< current number of allocated \see var_struct in \a var */
	var_struct * var[200]; /*!< fixed array of \a no \see var_struct structures */
} our_vars= {0,{NULL}};

int write_ini_on_exit= 1;
// Window Handling
int elconfig_win= -1;
int force_elconfig_win_ontop = 0;
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
int elconfig_menu_x_len= 620;
int elconfig_menu_y_len= 430;

int windows_on_top= 0;
static int options_set= 0;
int shadow_map_size_multi= 0;
#ifdef	FSAA
 int fsaa_index = 0;
#endif	/* FSAA */

/* temporary variables for fine graphic positions asjustmeet */
int gx_adjust = 0;
int gy_adjust = 0;

int you_sit= 0;
int sit_lock= 0;
int use_keypress_dialogue_boxes = 0, use_full_dialogue_window = 0;
int use_alpha_banner = 0;
int show_fps= 1;
int render_skeleton= 0;
int render_mesh= 1;
int render_bones_id = 0;
int render_bones_orientation = 0;
#ifdef NEW_CURSOR
 int big_cursors = 0;
 int sdl_cursors = 0;
 float pointer_size = 1.0;
#endif // NEW_CURSOR
float water_tiles_extension = 200.0;
int show_game_seconds = 0;
int skybox_update_delay = 10;
int skybox_local_weather = 0;
#ifdef OSX	// for probelem with rounded buttons on Intel graphics
 int square_buttons = 0;
#endif
#ifdef	NEW_TEXTURES
 int small_actor_texture_cache = 0;
#endif	/* NEW_TEXTURES */

int video_info_sent = 0;

#ifdef DEBUG
 int enable_client_aiming = 0;
#endif // DEBUG

#ifdef ELC
static void consolidate_rotate_chat_log_status(void);
#endif

void options_loaded(void)
{
	size_t i;
	// find any options not marked as saved, excluding ones we marked on purpose
	for (i= 0; i < our_vars.no; i++)
		if ((!our_vars.var[i]->saved) && (our_vars.var[i]->type!=OPT_BOOL_INI) && (our_vars.var[i]->type!=OPT_INT_INI))
			our_vars.var[i]->saved = 1;
	options_set = 1;
#ifdef ELC
	get_rotate_chat_log();
	consolidate_rotate_chat_log_status();
#endif
}


int int_zero_func()
{
	return 0;
}

float float_zero_func()
{
	return 0.0f;
}

int int_one_func()
{
	return 1;
}

float float_one_func()
{
	return 1.0f;
}

static __inline__ void check_option_var(char* name);

static __inline__ void destroy_shadow_mapping()
{
	if (gl_extensions_loaded)
	{
		CHECK_GL_ERRORS();
		if (have_extension(ext_framebuffer_object))
		{
#ifndef MAP_EDITOR
			free_shadow_framebuffer();
#endif //MAP_EDITOR
		}
		else
		{
			if (depth_map_id != 0)
			{
				glDeleteTextures(1, &depth_map_id);
				depth_map_id = 0;
			}
		}
		CHECK_GL_ERRORS();
	}
}

static __inline__ void destroy_fbos()
{
	if (gl_extensions_loaded)
	{
		CHECK_GL_ERRORS();
		if (have_extension(ext_framebuffer_object))
		{
#ifndef MAP_EDITOR
			destroy_shadow_mapping();
			free_reflection_framebuffer();
#endif //MAP_EDITOR
		}
		CHECK_GL_ERRORS();
	}
}

static __inline__ void build_fbos()
{
	if (gl_extensions_loaded)
	{
#ifndef MAP_EDITOR
		if (have_extension(ext_framebuffer_object) && use_frame_buffer)
		{
			if ((water_shader_quality > 0) && show_reflection)
			{
				make_reflection_framebuffer(window_width, window_height);
			}
		}
#endif // MAP_EDITOR
		check_option_var("shadow_map_size");
	}
}

static __inline__ void update_fbos()
{
	destroy_fbos();
	build_fbos();
}

void change_var(int * var)
{
	*var= !*var;
}

#ifndef MAP_EDITOR
static void change_show_action_bar(int * var)
{
	*var= !*var;
	if (stats_bar_win >= 0)
		init_stats_display();
}

void change_minimap_scale(float * var, float * value)
{
	int shown = 0;
	*var= *value;
	if (minimap_win>=0)
	{
		shown = get_show_window(minimap_win);
		minimap_win_x = windows_list.window[minimap_win].cur_x;
		minimap_win_y = windows_list.window[minimap_win].cur_y;
		destroy_window(minimap_win);
		minimap_win = -1;
	}
	if (shown)
		display_minimap();
}

void change_sky_var(int * var)
{
	*var= !*var;
	skybox_update_colors();
}

void change_use_animation_program(int * var)
{
	if (*var)
	{
		if (gl_extensions_loaded && have_extension(arb_vertex_buffer_object) &&
			have_extension(arb_vertex_program))
		{
			unload_vertex_programs();
		}
		*var = 0;
	}
	else
	{
		if (!gl_extensions_loaded)
		{
			*var = 1;
		}
		else
		{
			if (have_extension(arb_vertex_buffer_object) &&
				have_extension(arb_vertex_program))
			{
				*var = load_vertex_programs();
			}
		}
	}
	if (gl_extensions_loaded)
	{
		if (use_animation_program)
		{
			LOG_TO_CONSOLE(c_green2, "Using vertex program for actor animation.");
		}
		else
		{
			LOG_TO_CONSOLE(c_red1, "Not using vertex program for actor animation.");
		}
	}
}
#endif //MAP_EDITOR

#ifndef MAP_EDITOR
void change_min_ec_framerate(float * var, float * value)
{
	if(*value >= 0) {
		if (*value < max_ec_framerate) {
			*var = *value;
		} else if (!max_ec_framerate) {
			*var = *value;
		} else {
			*var = max_ec_framerate - 1;
	  	}
	} else {
		*var= 0;
	}
}

void change_max_ec_framerate(float * var, float * value)
{
	if(*value >= 1) {
		if (*value > min_ec_framerate) {
			*var = *value;
		} else if (!min_ec_framerate) {
			*var = *value;
	  	} else {
			*var = min_ec_framerate + 1;
	  	}
	} else {
		*var= 1;
	}
}
#endif //!MAP_EDITOR

void change_int(int * var, int value)
{
	if(value>=0) *var= value;
}

void change_signed_int(int * var, int value)
{
	*var= value;
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
	/* Commented by Schmurk: if we can define bounds for parameters, why testing
	 * if the value is over 0 here? */
	//if(*value >= 0) {
	*var= *value;
	//} else {
	// *var= 0;
	//}
}

void change_string(char * var, char * str, int len)
{
	while(*str && len--){
		*var++= *str++;
	}
	*var= 0;
}

#ifdef ELC

/*
 * The chat logs are created very early on in the client start up, before the
 * el.ini file is read at least. Because of this, a simple el.ini file variable
 * can not be used to enabled/disable rotating chat log files. I suppose the init
 * code could be changed but rather do that and unleash all kinds of grief, use a
 * simple flag file when the chat logs are opened.  The el.ini file setting now
 * becomes the creater/remover of that file.
 */
static int rotate_chat_log_config_var = 0;
static int rotate_chat_log = -1;
static const char* rotate_chat_log_flag_file = "rotate_chat_log_enabled";

/* get the current value depending on if the flag file exists */
int get_rotate_chat_log(void)
{
	if (rotate_chat_log == -1)
		rotate_chat_log = (file_exists_config(rotate_chat_log_flag_file)==1) ?1: 0;
	return rotate_chat_log;
}

/* create or delete the flag file to reflect the el.ini file rotate chat log value */
static void change_rotate_chat_log(int *value)
{
	*value = !*value;

	/* create the flag file if we are switching on chat log rotate */
	if (*value && (file_exists_config(rotate_chat_log_flag_file)!=1))
	{
		FILE *fp = open_file_config(rotate_chat_log_flag_file,"w");
		if ((fp == NULL) || (fclose(fp) != 0))
			LOG_ERROR("%s: Failed to create [%s] [%s]\n", __FILE__, rotate_chat_log_flag_file, strerror(errno) );
		else
			LOG_TO_CONSOLE(c_green2, rotate_chat_log_restart_str);
	}
	/* remove the flag file if we are switching off chat log rotate */
	else if (!(*value) && (file_exists_config(rotate_chat_log_flag_file)==1))
	{
		const char *config_dir = get_path_config();
		char *name_buf = NULL;
		if (config_dir != NULL)
		{
			size_t buf_len = strlen(config_dir) + strlen(rotate_chat_log_flag_file) + 1;
			name_buf = (char *)malloc(buf_len);
			if (name_buf != NULL)
			{
				safe_strncpy(name_buf, config_dir, buf_len);
				safe_strcat(name_buf, rotate_chat_log_flag_file, buf_len);
				if (unlink(name_buf) != 0)
					LOG_ERROR("%s: Failed to remove [%s] [%s]\n", __FILE__, rotate_chat_log_flag_file, strerror(errno) );
				else
					LOG_TO_CONSOLE(c_green2, rotate_chat_log_restart_str);
				free(name_buf);
			}
		}
	}
}

/* called after the el.in file has been read, so we can consolidate the rotate chat log status */
static void consolidate_rotate_chat_log_status(void)
{
	/* it is too late to use a newly set rotate log value, but we can set the el.ini flag if rotating is on */
	if ((rotate_chat_log==1) && !rotate_chat_log_config_var)
	{
		rotate_chat_log_config_var = 1;
		set_var_unsaved("rotate_chat_log", INI_FILE_VAR);
	}
}

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

#ifdef	NEW_TEXTURES
void update_max_actor_texture_handles()
{
	if (poor_man == 1)
	{
		if (small_actor_texture_cache == 1)
		{
			max_actor_texture_handles = 1;
		}
		else
		{
			max_actor_texture_handles = 4;
		}
	}
	else
	{
		if (small_actor_texture_cache == 1)
		{
			max_actor_texture_handles = 16;
		}
		else
		{
			max_actor_texture_handles = 32;
		}
	}
}
#endif	/* NEW_TEXTURES */

void change_poor_man(int *poor_man)
{
	*poor_man= !*poor_man;
#ifdef	NEW_TEXTURES
	unload_texture_cache();
	update_max_actor_texture_handles();
#endif	/* NEW_TEXTURES */
	if(*poor_man) {
		show_reflection= 0;
		shadows_on= 0;
		clouds_shadows= 0;
		use_shadow_mapping= 0;
#ifndef MAP_EDITOR2
		special_effects= 0;
		use_eye_candy = 0;
		use_fog= 0;
		show_weather = 0;
#endif
#ifndef MAP_EDITOR
		use_frame_buffer= 0;
#endif
		update_fbos();
		skybox_show_clouds = 0;
		skybox_show_sun = 0;
		skybox_show_moons = 0;
		skybox_show_stars = 0;
	}
}

void change_compiled_vertex_array(int *value)
{
	if (*value) {
		*value= 0;
	}
	else if (!gl_extensions_loaded || have_extension(ext_compiled_vertex_array))
	{
		// don't check if we have hardware support when OpenGL
		// extensions are not initialized yet.
		*value= 1;
	}
	else LOG_TO_CONSOLE(c_green2,disabled_compiled_vertex_arrays);
}

void change_vertex_buffers(int *value)
{
	if (*value) {
		*value= 0;
	}
	else if (!gl_extensions_loaded || have_extension(arb_vertex_buffer_object))
	{
		// don't check if we have hardware support when OpenGL
		// extensions are not initialized yet.
		*value= 1;
	}
//	else LOG_TO_CONSOLE(c_green2,disabled_vertex_buffers);
}

void change_clouds_shadows(int *value)
{
	if (*value) {
		*value= 0;
	}
	else if (!gl_extensions_loaded || (get_texture_units() >= 2))
	{
		// don't check if we have hardware support when OpenGL
		// extensions are not initialized yet.
		*value= 1;
	}
//	else LOG_TO_CONSOLE(c_green2,disabled_clouds_shadows);
}

#ifdef	NEW_TEXTURES
void change_small_actor_texture_cache(int *value)
{
	if (*value)
	{
		*value = 0;
	}
	else
	{
		*value = 1;
	}

	update_max_actor_texture_handles();
}

void change_eye_candy(int *value)
{
	if (*value)
	{
		*value = 0;
	}
	else if (!gl_extensions_loaded || ((get_texture_units() >= 2) &&
		supports_gl_version(1, 5)))
	{
		// don't check if we have hardware support when OpenGL
		// extensions are not initialized yet.
		*value = 1;
	}
}
#else	/* NEW_TEXTURES */
void change_mipmaps(int *value)
{
	if (*value) {
		*value= 0;
	}
	else if (!gl_extensions_loaded || have_extension(sgis_generate_mipmap))
	{
		// don't check if we have hardware support when OpenGL
		// extensions are not initialized yet.
		*value= 1;
	}
//	else LOG_TO_CONSOLE(c_green2,disabled_mipmaps);
}
#endif	/* NEW_TEXTURES */

void change_point_particles(int *value)
{
	if (*value) {
		*value= 0;
	}
	else if (!gl_extensions_loaded || have_extension(arb_point_sprite))
	{
		// don't check if we have hardware support when OpenGL
		// extensions are not initialized yet.
		*value= 1;
	}
	else
	{
		LOG_TO_CONSOLE(c_green2, disabled_point_particles);
	}

#ifndef	NEW_TEXTURES
	ec_set_draw_method();
#endif	/* NEW_TEXTURES */
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

void change_new_selection(int *value)
{
	if (*value)
	{
		*value = 0;
	}
	else
	{
		if (gl_extensions_loaded)
		{
			if ((supports_gl_version(1, 3) ||
				have_extension(arb_texture_env_combine)) &&
				(get_texture_units() > 1) && (bpp == 32))
			{
				*value = 1;
			}
		}
		else
		{
			*value = 1;
		}
	}
}

int switch_video(int mode, int full_screen)
{
	int win_width,
		win_height,
		win_bpp;

	int index = mode - 1;

	int flags = SDL_OPENGL;
	if (full_screen)
		flags |= SDL_FULLSCREEN;

	if(mode == 0 && !full_screen) {
		win_width = video_user_width;
		win_height = video_user_height;
		win_bpp = bpp;
	} else if(index < 0 || index >= video_modes_count) {
		//warn about this error
		LOG_TO_CONSOLE(c_red2,invalid_video_mode);
		return 0;
	} else {
		/* Check if the video mode is supported. */
		win_width = video_modes[index].width;
		win_height = video_modes[index].height;
		win_bpp = video_modes[index].bpp;
	}

#ifndef LINUX
	LOG_TO_CONSOLE(c_green2, video_restart_str);
	video_mode=mode;
	set_var_unsaved("video_mode", INI_FILE_VAR);
	return 1;
#endif

	destroy_fbos();

	if (!SDL_VideoModeOK(win_width, win_height, win_bpp, flags)) {
		LOG_TO_CONSOLE(c_red2, invalid_video_mode);
		return 0;
	} else {
		set_new_video_mode(full_screen, mode);
#ifndef MAP_EDITOR2
		if(items_win >= 0) {
			windows_list.window[items_win].show_handler(&windows_list.window[items_win]);
		}
#endif
	}
	build_fbos();
	return 1;
}
void switch_vidmode(int *pointer, int mode)
{
	if(!video_mode_set) {
		/* Video isn't ready yet, just remember the mode */
		video_mode= mode;
	} else {
		switch_video(mode, full_screen);
	}
}

void toggle_full_screen_mode(int * fs)
{
	if(!video_mode_set) {
		*fs= !*fs;
	} else {
		toggle_full_screen();
	}
}

#ifdef NEW_CURSOR
void change_sdl_cursor(int * fs)
{
	if(!*fs) {
		SDL_ShowCursor(1);
	} else {
		SDL_ShowCursor(0);
	}
	*fs = !*fs;
}
#endif // NEW_CURSOR

void toggle_follow_cam(int * fc)
{
	last_kludge=camera_kludge;
	if (*fc)
		hold_camera=rz;
	else
		hold_camera+=camera_kludge;
	change_var(fc);
}

void toggle_follow_cam_behind(int * fc)
{
	if (*fc)
	{
		last_kludge = camera_kludge;
		hold_camera += camera_kludge;
	}
	else
	{
		last_kludge = -rz;
	}
	change_var(fc);
}

void toggle_ext_cam(int * ec)
{
	change_var(ec);
	if (*ec)
	{
		isometric = 0;
		if (video_mode_set)
		{
			resize_root_window();
			set_all_intersect_update_needed(main_bbox_tree);
		}
	}
}

void change_tilt_float(float * var, float * value)
{
	*var= *value;
    if (rx > -min_tilt_angle) rx = -min_tilt_angle;
    else if (rx < -max_tilt_angle) rx = -max_tilt_angle;
}

void change_shadow_map_size(int *pointer, int value)
{
	const int array[10]= {256, 512, 768, 1024, 1280, 1536, 1792, 2048, 3072, 4096};
	int index, size, i, max_size, error;
	char error_str[1024];

	if (value >= array[0])
	{
		index = 0;
		for (i = 0; i < 10; i++)
		{
			/* Check if we can set the multiselect widget to this */
			if (array[i] == value)
			{
				index = i;
				break;
			}
		}
	}
	else
	{
		index = min2i(max2i(0, value), 9);
	}

	size = array[index];

	if (gl_extensions_loaded && use_shadow_mapping)
	{
		error= 0;

		if (use_frame_buffer)
		{
			glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE_EXT, &max_size);
		}
		else
		{
			max_size = min2i(window_width, window_height);
		}

		if (size > max_size)
		{
			while ((size > max_size) && (index > 0))
			{
				index--;
				size = array[index];
			}
			error= 1;
		}

		if (!(have_extension(arb_texture_non_power_of_two) || supports_gl_version(2, 0)))
		{
			switch (index)
			{
				case 2:
					index = 1;
					error = 1;
					break;
				case 4:
				case 5:
				case 6:
					index = 3;
					error = 1;
					break;
				case 8:
					index = 7;
					error = 1;
					break;
			}
			size = array[index];
		}
		if (error == 1)
		{
			memset(error_str, 0, sizeof(error_str));
			safe_snprintf(error_str, sizeof(error_str),
				shadow_map_size_not_supported_str, size);
			LOG_TO_CONSOLE(c_yellow2, error_str);
		}
		shadow_map_size = size;

		destroy_shadow_mapping();
		if (have_extension(ext_framebuffer_object) && use_frame_buffer)
		{
			make_shadow_framebuffer();
		}
	}
	else
	{
		shadow_map_size = size;
	}

	if (pointer != NULL)
	{
		*pointer= index;
	}
}

#ifdef	FSAA
void change_fsaa(int *pointer, int value)
{
	unsigned int i, index, fsaa_value;

	index = 0;
	fsaa_value = 0;

	if (value > 0)
	{
		for (i = 1; i < get_fsaa_mode_count(); i++)
		{
			if (get_fsaa_mode(i))
			{
				index++;
				fsaa_value = i;
			}
			if (value == index)
			{
				break;
			}
		}
	}

	fsaa = fsaa_value;

	if (pointer != 0)
	{
		*pointer = index;
	}

	LOG_TO_CONSOLE(c_green2, video_restart_str);
}
#endif	/* FSAA */

#ifdef CUSTOM_UPDATE
void change_custom_update(int *var)
{
	*var = !*var;

	if (*var)
	{
		start_custom_update();
	}
}

void change_custom_clothing(int *var)
{
	*var = !*var;
#ifdef	NEW_TEXTURES
	unload_actor_texture_cache();
#endif	/* NEW_TEXTURES */	    
}
#endif    //CUSTOM_UPDATE

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

void set_buff_icon_size(int *pointer, int value)
{
	/* The value is actually set in the widget code so attempting so controlling the
		range here does not work.  Instead, use the built in max/min code of the widget.
		We still need to set the value here for the initial read from the config file. */
	*pointer = value;
	/* Turn off icons when the size is zero (or at least low). */
	view_buffs = (value < 5) ?0: 1;
}

void change_dark_channeltext(int *dct, int value)
{
	*dct = value;
	if (*dct == 1)
		set_text_message_color (&input_text_line, 0.6f, 0.6f, 0.6f);
	else if (*dct == 2)
		set_text_message_color (&input_text_line, 0.16f, 0.16f, 0.16f);
	else
		set_text_message_color (&input_text_line, 1.0f, 1.0f, 1.0f);
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
			display_chat();
		}
	}
	else if (chat_win >= 0)
	{
		hide_window (chat_win);
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
			console_font_resize(*value);
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

void change_note_zoom (float *dest, float *value)
{
	if (*value < 0.0f)
		return;
	*dest = *value;
	if (notepad_win >= 0)
		notepad_win_update_zoom ();
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
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
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
	if (video_mode_set)
	{
		resize_root_window ();
		set_all_intersect_update_needed (main_bbox_tree);
	}
}

void change_projection_bool(int *pointer) {
	change_var(pointer);
	if (video_mode_set)
	{
		resize_root_window ();
		if (pointer == &isometric && isometric) ext_cam = 0;
		set_all_intersect_update_needed (main_bbox_tree);
	}
}

void change_gamma(float *pointer, float *value)
{
	*pointer= *value;
	if(video_mode_set && !disable_gamma_adjust) {
		SDL_SetGamma(*value, *value, *value);
	}
}

#ifndef MAP_EDITOR2
void change_windows_on_top(int *var)
{
	int winid_list[] = { storage_win, manufacture_win, items_win, buddy_win, ground_items_win,
						 sigil_win, elconfig_win, tab_stats_win, tab_info_win,
						 minimap_win, questlog_win, trade_win, range_win };
	int i;

	*var=!*var;
	if (*var)
	{
		for (i=0; i<sizeof(winid_list)/sizeof(int); i++)
		{
			if (winid_list[i] >= 0)
			{
				window_info *win = &windows_list.window[winid_list[i]];
				/* Change the root windows */
				move_window(winid_list[i], -1, 0, win->pos_x, win->pos_y );
				/* Display any open windows */
				if (win->displayed != 0 || win->reinstate != 0)
					show_window(winid_list[i]);
			}
		}
	}
	else
	{
		// Change the root windows
		for (i=0; i<sizeof(winid_list)/sizeof(int); i++)
			if (winid_list[i] >= 0)
			{
				window_info *win = &windows_list.window[winid_list[i]];
				move_window(winid_list[i], game_root_win, 0, win->pos_x, win->pos_y );
			}

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
		if (!gl_extensions_loaded || ((get_texture_units() >= 3) &&
			have_extension(arb_shadow) && have_extension(arb_texture_env_combine)))
		{
			*sm= 1;
		}
		else
		{
			LOG_TO_CONSOLE (c_red1, disabled_shadow_mapping);
		}
	}
	update_fbos();
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

#endif // ELC

void change_reflection(int *rf)
{
	*rf= !*rf;
	update_fbos();
}

#ifndef MAP_EDITOR
void change_frame_buffer(int *fb)
{
	if (*fb)
	{
		*fb= 0;
	}
	else
	{
		if (!gl_extensions_loaded || have_extension(ext_framebuffer_object))
		{
			*fb= 1;
		}
		else
		{
			LOG_TO_CONSOLE (c_red1, disabled_framebuffer);
		}
	}
	update_fbos();
}
#endif

void change_shadows(int *sh)
{
	*sh= !*sh;
	update_fbos();
}

#ifndef MAP_EDITOR
int int_max_water_shader_quality()
{
	if (gl_extensions_loaded)
	{
		return get_max_supported_water_shader_quality();
	}
	else
	{
		return 2;
	}
}

void change_water_shader_quality(int *wsq, int value)
{
	if (gl_extensions_loaded)
	{
		*wsq = min2i(max2i(value, 0), get_max_supported_water_shader_quality());
	}
	else
	{
		*wsq = value;
	}
	update_fbos();
}
#endif

#ifdef MAP_EDITOR

void set_auto_save_interval (int *save_time, int time)
{
	if(time>0) {
		*save_time= time*60000;
	} else {
		*save_time= 0;
	}
}

void switch_vidmode(int *pointer, int mode)
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

int find_var (const char *str, var_name_type type)
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


int set_var_unsaved(const char *str, var_name_type type)
{
	int var_index;

	var_index = find_var(str, type);

	if (var_index == -1)
	{
		LOG_ERROR("Can't find var '%s', type %d", str, type);
		return 0;
	}
	our_vars.var[var_index]->saved = 0;
	return 1;
}

int toggle_OPT_BOOL_by_name(const char *str)
{
	int var_index = find_var(str, OPT_BOOL);
	if ((var_index == -1) || (our_vars.var[var_index]->type != OPT_BOOL))
	{
		fprintf(stderr, "%s(): Invalid OPT_BOOL '%s'\n", __FUNCTION__, str);
		return 0;
	}
	our_vars.var[var_index]->func(our_vars.var[var_index]->var);
	our_vars.var[var_index]->saved= 0;
	return 1;
}

#ifdef	ELC
// Find an OPT_INT widget and set its's value
// Other types might be useful but I just needed an OPT_INT this time.
int set_var_OPT_INT(const char *str, int new_value)
{
	int var_index;

	var_index = find_var(str, INI_FILE_VAR);

	if ((var_index != -1) && (our_vars.var[var_index]->type == OPT_INT))
	{
		int tab_win_id = elconfig_tabs[our_vars.var[var_index]->widgets.tab_id].tab;
		int widget_id = our_vars.var[var_index]->widgets.widget_id;
		// This bit belongs in the widgets module
		widget_list *widget = widget_find(tab_win_id, widget_id);
		our_vars.var[var_index]->saved = 0;
		if(widget != NULL && widget->widget_info != NULL)
		{
			spinbutton *button = widget->widget_info;
			*(int *)button->data = new_value;
			safe_snprintf(button->input_buffer, sizeof(button->input_buffer), "%i", *(int *)button->data);
			return 1;
		}

		return 0;
	}

	LOG_ERROR("Can't find var '%s', type 'OPT_INT'", str);
	return 0;
}
#endif

void change_language(const char *new_lang)
{
	int var_index;

	LOG_DEBUG("Language changed, was [%s] now [%s]\n",  lang, new_lang);
	/* guard against being the same string */
	if (strcmp(lang, new_lang) != 0)
		safe_strncpy(lang, new_lang, sizeof(lang));

	var_index = find_var("language", INI_FILE_VAR);

	if (var_index != -1)
	{
		our_vars.var[var_index]->saved= 0;
	}
	else
	{
		LOG_ERROR("Can't find var '%s', type 'OPT_STRING'", "language");
	}
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
		LOG_ERROR("Can't find var '%s', type 'IN_GAME_VAR'", name);
		return;
	}

	switch (our_vars.var[i]->type)
	{
		case OPT_INT:
		case OPT_MULTI:
		case OPT_MULTI_H:
		case OPT_INT_F:
		case OPT_INT_INI:
			value_i= *((int*)our_vars.var[i]->var);
			our_vars.var[i]->func (our_vars.var[i]->var, value_i);
			break;
		case OPT_BOOL:
		case OPT_BOOL_INI:
			value_i= *((int*)our_vars.var[i]->var);
			if (value_i == 0) *((int*)our_vars.var[i]->var)= 1;
			else *((int*)our_vars.var[i]->var)= 0;
			our_vars.var[i]->func (our_vars.var[i]->var);
			break;
		case OPT_STRING:
		case OPT_PASSWORD:
			value_s= (char*)our_vars.var[i]->var;
			our_vars.var[i]->func (our_vars.var[i]->var, value_s, our_vars.var[i]->len);
			break;
		case OPT_FLOAT:
		case OPT_FLOAT_F:
			value_f= *((float*)our_vars.var[i]->var);
			our_vars.var[i]->func (our_vars.var[i]->var, value_f);
			break;
	}
}

void check_options()
{
	check_option_var("use_compiled_vertex_array");
	check_option_var("use_vertex_buffers");
	check_option_var("clouds_shadows");
#ifdef	NEW_TEXTURES
	check_option_var("small_actor_texture_cache");
	check_option_var("use_eye_candy");
#else	/* NEW_TEXTURES */
	check_option_var("use_mipmaps");
#endif	/* NEW_TEXTURES */
	check_option_var("use_point_particles");
	check_option_var("use_frame_buffer");
	check_option_var("use_shadow_mapping");
	check_option_var("shadow_map_size");
	check_option_var("water_shader_quality");
	check_option_var("use_animation_program");
}

int check_var (char *str, var_name_type type)
{
	int i, *p;
	char *ptr= str;
	float foo;

	i= find_var (str, type);
	if (i < 0)
	{
		LOG_WARNING("Can't find var '%s', type %d", str, type);
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
				safe_snprintf (str, sizeof(str), "Reached newline without an ending \" in %s", our_vars.var[i]->name);
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
		case OPT_INT:
		case OPT_MULTI:
		case OPT_MULTI_H:
		case OPT_INT_F:
			our_vars.var[i]->func ( our_vars.var[i]->var, atoi (ptr) );
			return 1;
		case OPT_BOOL_INI:
		case OPT_INT_INI:
			// Needed, because var is never changed through widget
			our_vars.var[i]->saved= 0;
		case OPT_BOOL:
		{
			int new_val;
			p= our_vars.var[i]->var;
			if (ptr && ptr[0]=='!')
				new_val = !*p;
			else
				new_val = atoi (ptr);
			if ((new_val>0) != *p)
				our_vars.var[i]->func (our_vars.var[i]->var); //only call if value has changed
			return 1;
		}
		case OPT_STRING:
		case OPT_PASSWORD:
			our_vars.var[i]->func (our_vars.var[i]->var, ptr, our_vars.var[i]->len);
			return 1;
		case OPT_FLOAT:
		case OPT_FLOAT_F:
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
			case OPT_INT:
			case OPT_FLOAT:
			case OPT_FLOAT_F:
			case OPT_INT_F:
				queue_destroy(our_vars.var[i]->queue);
				break;
			case OPT_MULTI:
			case OPT_MULTI_H:
				if(our_vars.var[i]->queue != NULL) {
					while(!queue_isempty(our_vars.var[i]->queue)) {
						//We don't free() because it's not allocated.
						queue_pop(our_vars.var[i]->queue);
					}
					queue_destroy(our_vars.var[i]->queue);
				}
				break;
			default:
				/* do nothing */ ;
		}
		free(our_vars.var[i]);
	}
	our_vars.no=0;
}

void add_var(option_type type, char * name, char * shortname, void * var, void * func, float def, char * short_desc, char * long_desc, int tab_id, ...)
{
	int *integer=var;
	float *f=var;
	int no=our_vars.no++;
	char *pointer;
	float *tmp_f;
	point *tmp_i;
	int_min_max_func *i_func;
	float_min_max_func *f_func;
	va_list ap;

	our_vars.var[no]=(var_struct*)calloc(1,sizeof(var_struct));
	switch(our_vars.var[no]->type=type)
	{
		case OPT_MULTI:
		case OPT_MULTI_H:
			queue_initialise(&our_vars.var[no]->queue);
			va_start(ap, tab_id);
			while((pointer= va_arg(ap, char *)) != NULL) {
				queue_push(our_vars.var[no]->queue, pointer);
			}
			va_end(ap);
			*integer= (int)def;
		break;
		case OPT_INT:
		case OPT_INT_INI:
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
		case OPT_BOOL:
		case OPT_BOOL_INI:
			*integer=(int)def;
			break;
		case OPT_STRING:
		case OPT_PASSWORD:
			our_vars.var[no]->len=(int)def;
			break;
		case OPT_FLOAT:
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
		case OPT_FLOAT_F:
			queue_initialise(&our_vars.var[no]->queue);
			va_start(ap, tab_id);
			//Min
			f_func = calloc(1, sizeof(*f_func));
			*f_func = va_arg(ap, float_min_max_func);
			queue_push(our_vars.var[no]->queue, f_func);
			//Max
			f_func = calloc(1, sizeof(*f_func));
			*f_func = va_arg(ap, float_min_max_func);
			queue_push(our_vars.var[no]->queue, f_func);
			//Interval
			tmp_f = calloc(1,sizeof(*tmp_f));
			*tmp_f = va_arg(ap, double);
			queue_push(our_vars.var[no]->queue, (void *)tmp_f);
			va_end(ap);
			*f = def;
			break;
		case OPT_INT_F:
			queue_initialise(&our_vars.var[no]->queue);
			va_start(ap, tab_id);
			//Min
			i_func = calloc(1, sizeof(*i_func));
			*i_func = va_arg(ap, int_min_max_func);
			queue_push(our_vars.var[no]->queue, i_func);
			//Max
			i_func = calloc(1, sizeof(*i_func));
			*i_func = va_arg(ap, int_min_max_func);
			queue_push(our_vars.var[no]->queue, i_func);
			va_end(ap);
			*integer = (int)def;
			break;
	}
	our_vars.var[no]->var=var;
	our_vars.var[no]->func=func;
	our_vars.var[no]->name=name;
	our_vars.var[no]->shortname=shortname;
	our_vars.var[no]->nlen=strlen(our_vars.var[no]->name);
	our_vars.var[no]->snlen=strlen(our_vars.var[no]->shortname);
	our_vars.var[no]->saved= 0;
#ifdef ELC
	add_options_distringid(name, &our_vars.var[no]->display, short_desc, long_desc);
#endif //ELC
	our_vars.var[no]->widgets.tab_id= tab_id;
}

void add_multi_option(char * name, char * str)
{
	int var_index;

	var_index = find_var(name, INI_FILE_VAR);

	if (var_index == -1)
	{
		LOG_ERROR("Can't find var '%s', type 'INI_FILE_VAR'", name);
	}
	else
	{
		queue_push(our_vars.var[var_index]->queue, str);
	}
}


//ELC specific variables
#ifdef ELC
static void init_ELC_vars(void)
{
	int i;

	// CONTROLS TAB
	add_var(OPT_BOOL,"sit_lock","sl",&sit_lock,change_var,0,"Sit Lock","Enable this to prevent your character from moving by accident when you are sitting.",CONTROLS);
	add_var(OPT_BOOL,"always_pathfinding", "alwayspathfinding", &always_pathfinding, change_var, 0, "Extend the range of the walk cursor", "Extends the range of the walk cursor to as far as you can see.  Using this option, movement may be slightly less responsive on larger maps.", CONTROLS);
	add_var(OPT_BOOL,"use_floating_messages", "floating", &floatingmessages_enabled, change_var, 1, "Floating Messages", "Toggles the use of floating experience messages and other graphical enhancements", CONTROLS);
	add_var(OPT_BOOL,"floating_session_counters", "floatingsessioncounters", &floating_session_counters, change_var, 0, "Floating Session Counters", "Toggles the display of floating session counters.  Configure each type using the context menu of the counter category.", CONTROLS);
	add_var(OPT_BOOL,"use_keypress_dialog_boxes", "keypressdialogues", &use_keypress_dialogue_boxes, change_var, 0, "Keypresses in dialogue boxes", "Toggles the ability to press a key to select a menu option in dialogue boxes (eg The Wraith)", CONTROLS);
	add_var(OPT_BOOL,"use_full_dialogue_window", "keypressdialoguesfullwindow", &use_full_dialogue_window, change_var, 0, "Keypresses allowed anywhere in dialogue boxes", "If set, the above will work anywhere in the Dialogue Window, if unset only on the NPC's face", CONTROLS);
	add_var(OPT_BOOL,"use_cursor_on_animal", "useanimal", &include_use_cursor_on_animals, change_var, 0, "For animals, right click includes use cursor", "Toggles inclusion of the use cursor when right clicking on animals, useful for your summoned creatures.  Even when this option is off, you can still click the use icon.", CONTROLS);
	add_var(OPT_BOOL,"disable_double_click", "disabledoubleclick", &disable_double_click, change_var, 0, "Disable double-click button safety", "Some buttons are protected from mis-click by requiring you to double-click them.  This option disables that protection.", CONTROLS);
	add_var(OPT_BOOL,"auto_disable_ranging_lock", "adrl", &auto_disable_ranging_lock, change_var, 1, "Auto-disable Ranging Lock when under attack", "Automatically disable Ranging Lock when the char is under attack and Ranging Lock is enabled", CONTROLS);
	add_var(OPT_BOOL,"achievements_ctrl_click", "achievementsctrlclick", &achievements_ctrl_click, change_var, 0, "Control click required to view achievements", "To view a players achievements, you click on them with the eye cursor.  With this option enabled, you must use Ctrl+click.", CONTROLS);
	add_var(OPT_INT,"mouse_limit","lmouse",&mouse_limit,change_int,15,"Mouse Limit","You can increase the mouse sensitivity and cursor changing by adjusting this number to lower numbers, but usually the FPS will drop as well!",CONTROLS,1,INT_MAX);
#ifdef OSX
	add_var(OPT_BOOL,"osx_right_mouse_cam","osxrightmousecam", &osx_right_mouse_cam, change_var,0,"Rotate Camera with right mouse button", "Allows to rotate the camera by pressing the right mouse button and dragging the cursor", CONTROLS);
	add_var(OPT_BOOL,"emulate_3_button_mouse","emulate3buttonmouse", &emulate3buttonmouse, change_var,0,"Emulate a 3 Button Mouse", "If you have a 1 Button Mouse you can use <apple> click to emulate a rightclick. Needs client restart.", CONTROLS);
#endif // OSX
#ifdef NEW_CURSOR
	add_var(OPT_BOOL,"sdl_cursors","sdl_cursors", &sdl_cursors, change_sdl_cursor,1,"Old Style Pointers", "Use default SDL cursor.", CONTROLS);
	add_var(OPT_BOOL,"big_cursors","big_cursors", &big_cursors, change_var,0,"Big Pointers", "Use 32x32 graphics for pointer. Only works with SDL cursor turned off.", CONTROLS);
	add_var(OPT_FLOAT,"pointer_size","pointer_size", &pointer_size, change_float,1.0,"Pointer Size", "Scale the pointer. 1.0 is 1:1 scale with pointer graphic. Only works with SDL cursor turned off.", CONTROLS,0.25,4.0,0.05);
#endif // NEW_CURSOR
	add_var(OPT_BOOL_INI,"enable_trade_log", "enabletradelog", &enable_trade_log, change_var, 0, "Enable trade log", "Enable logging of all successful trades. (Experimental)", CONTROLS);
	add_var(OPT_MULTI,"trade_log_mode","tradelogmode",&trade_log_mode,change_int, TRADE_LOG_NONE,"Trade log","Set how successful trades are logged.",CONTROLS,"Do not log trades", "Log only to console", "Log only to file", "Log to console and file", NULL);
	// CONTROLS TAB


	// HUD TAB
	add_var(OPT_BOOL,"show_fps","fps",&show_fps,change_var,1,"Show FPS","Show the current frames per second in the corner of the window",HUD);
	add_var(OPT_BOOL,"view_analog_clock","analog",&view_analog_clock,change_var,1,"Analog Clock","Toggle the analog clock",HUD);
	add_var(OPT_BOOL,"view_digital_clock","digit",&view_digital_clock,change_var,1,"Digital Clock","Toggle the digital clock",HUD);
	add_var(OPT_BOOL,"view_knowledge_bar","knowledge_bar",&view_knowledge_bar,change_var,1,"Knowledge Bar","Toggle the knowledge bar",HUD);
	add_var(OPT_BOOL,"view_hud_timer","timer",&view_hud_timer,change_var,1,"Countdown/Stopwatch Timer","Toggle the countdown/stopwatch timer.  Shift-left-click to toggle mode. Left-click to start/stop. Mouse wheel to reset, up/down to change countdown start time (+ctrl/alt to change step).",HUD);
	add_var(OPT_BOOL,"show_game_seconds","show_game_seconds",&show_game_seconds,change_var,0,"Show Game Seconds","Show seconds on the digital clock. Note: the seconds displayed are computed on client side and synchronized with the server at each new minute.",HUD);
	add_var(OPT_BOOL,"show_stats_in_hud","sstats",&show_stats_in_hud,change_var,0,"Stats In HUD","Toggle showing stats in the HUD",HUD);
	add_var(OPT_BOOL,"show_statbars_in_hud","sstatbars",&show_statbars_in_hud,change_var,0,"StatBars In HUD","Toggle showing statbars in the HUD. Needs Stats in HUD",HUD);
	add_var(OPT_BOOL,"show_action_bar","ssactionbar",&show_action_bar,change_show_action_bar,0,"Action Points Bar in HUD","Show the current action points level in a stats bar on the bottom HUD.",HUD);
	add_var(OPT_BOOL,"logo_click_to_url","logoclick",&logo_click_to_url,change_var,0,"Logo Click To URL","Toggle clicking the LOGO opening a browser window",HUD);
	add_var(OPT_STRING,"logo_link", "logolink", LOGO_URL_LINK, change_string, 128, "Logo Link", "URL when clicking the logo", HUD);
	add_var(OPT_BOOL,"show_help_text","shelp",&show_help_text,change_var,1,"Help Text","Enable tooltips.",HUD);
	add_var(OPT_BOOL,"show_item_desc_text","showitemdesctext",&show_item_desc_text,change_var,1,"Item Description Text","Enable item description tooltips. Needs item_info.txt file.",HUD);
	add_var(OPT_BOOL,"use_alpha_border", "aborder", &use_alpha_border, change_var, 1,"Alpha Border","Toggle the use of alpha borders",HUD);	//ADVVID);
	add_var(OPT_BOOL,"use_alpha_banner", "abanner", &use_alpha_banner, change_var, 0,"Alpha Behind Name/Health Text","Toggle the use of an alpha background to name/health banners",HUD);
	add_var(OPT_BOOL,"cm_banner_disabled", "cmbanner", &cm_banner_disabled, change_var, 0,"Disable Name/Health Text Context Menu","Disable the context menu on your players name/health banner.",HUD);
	add_var(OPT_BOOL, "windows_on_top", "wot", &windows_on_top, change_windows_on_top, 0, "Windows On Top","Allows the Manufacture, Storage and Inventory windows to appear above the map and console.", HUD);
	add_var(OPT_BOOL,"opaque_window_backgrounds", "opaquewin", &opaque_window_backgrounds, change_var, 0,"Use Opaque Window Backgrounds","Toggle the current state of all windows between transparent and opaque background. Use CTRL+D to toggle the current state of an individual window.",HUD);
	add_var(OPT_SPECINT, "buff_icon_size","bufficonsize", &buff_icon_size, set_buff_icon_size, 32, "Buff Icon Size","The size of the icons of the active buffs.  Icons are not displayed when size set to zero.",HUD,0,48);
	add_var(OPT_BOOL,"relocate_quickbar", "requick", &quickbar_relocatable, change_quickbar_relocatable, 0,"Relocate Quickbar","Set whether you can move the quickbar",HUD);
	add_var(OPT_INT,"num_quickbar_slots","numqbslots",&num_quickbar_slots,change_int,6,"Number Of Quickbar Slots","Set the number of quickbar slots (both inventory & spells) displayed. May be automatically reduced for low resolutions",HUD,1,MAX_QUICKBAR_SLOTS);
	add_var(OPT_INT,"max_food_level","maxfoodlevel",&max_food_level,change_int,45,"Maximum Food Level", "Set the maximum value displayed by the food level bar.",HUD,10,200);
	add_var(OPT_INT,"wanted_num_recipe_entries","wantednumrecipeentries",&wanted_num_recipe_entries,change_num_recipe_entries,10,"Number of recipe entries", "Sets the number of entries available for the manufacturing window stored recipes.",HUD,4,max_num_recipe_entries);
	add_var(OPT_INT,"exp_log_threshold","explogthreshold",&exp_log_threshold,change_int,5000,"Log exp gain to console", "If you gain experience of this value or over, then a console message will be written.  Set the value to zero to disable completely.",HUD,0,INT_MAX);
	add_var(OPT_STRING,"npc_mark_template","npcmarktemplate",npc_mark_str,change_string,sizeof(npc_mark_str)-1,"NPC map mark template","The template used when setting a map mark from the NPC dialogue (right click name). The %s is substituted for the NPC name.",HUD);
	add_var(OPT_BOOL,"3d_map_markers","3dmarks",&marks_3d,change_3d_marks,1,"Enable 3D Map Markers","Shows user map markers in the game window",HUD);
	add_var(OPT_BOOL,"item_window_on_drop","itemdrop",&item_window_on_drop,change_var,1,"Item Window On Drop","Toggle whether the item window shows when you drop items",HUD);
	add_var(OPT_FLOAT,"minimap_scale", "minimapscale", &minimap_size_coefficient, change_minimap_scale, 0.7, "Minimap Scale", "Adjust the overall size of the minimap", HUD, 0.5, 1.5, 0.1);
	add_var(OPT_BOOL,"rotate_minimap","rotateminimap",&rotate_minimap,change_var,1,"Rotate Minimap","Toggle whether the minimap should rotate.",HUD);
	add_var(OPT_BOOL,"pin_minimap","pinminimap",&pin_minimap,change_var,0,"Pin Minimap","Toggle whether the minimap ignores close-all-windows.",HUD);
	add_var(OPT_BOOL, "continent_map_boundaries", "cmb", &show_continent_map_boundaries, change_var, 1, "Map Boundaries On Continent Map", "Show map boundaries on the continent map", HUD);
	add_var(OPT_BOOL,"enable_user_menus", "user_menus", &enable_user_menus, toggle_user_menus, 0, "Enable User Menus","Create .menu files in your config directory.  First line is the menu name. After that, each line is a command using the format \"Menus Text||command\".  Prompt for input using \"command text <prompt text>\". A line can include multiple commands.",HUD);
	add_var(OPT_BOOL,"console_scrollbar_enabled", "console_scrollbar", &console_scrollbar_enabled, toggle_console_scrollbar, 1, "Show Console Scrollbar","If enabled, a scrollbar will be shown in the console window.",HUD);
#if !defined(WINDOWS) && !defined(OSX)
	add_var(OPT_BOOL,"use_clipboard","uclb",&use_clipboard, change_var, 1, "Use Clipboard For Pasting", "Use CLIPBOARD for pasting (as e.g. GNOME does) or use PRIMARY cutbuffer (as xterm does)",HUD);
#endif

	add_var(OPT_BOOL,"show_poison_count", "poison_count", &show_poison_count, change_var, 0, "Show Food Poison Count", "Displays on the poison drop icon, the number of times you have been food poisoned since last being free of poison.",HUD);
	// instance mode options
	add_var(OPT_BOOL,"use_view_mode_instance","instance_mode",&view_mode_instance, change_var, 0, "Use instance mode banners", "Shows only your and mobs banners, adds mana bar to your banner.",HUD);
	add_var(OPT_FLOAT,"instance_mode_banner_height","instance_mode_bheight",&view_mode_instance_banner_height,change_float,5.0f,"Your instance banner height","Sets how high the banner is located above your character",HUD,1.0,12.0,0.2);
	add_var(OPT_BOOL,"im_creature_view_names","im_cnm",&im_creature_view_names, change_var, 0, "Creatures instance banners - names", "Show creature names when using instance mode",HUD);
	add_var(OPT_BOOL,"im_creature_view_hp","im_chp",&im_creature_view_hp, change_var, 0, "Creatures instance banners - health numbers", "Show creature health numbers when using instance mode",HUD);
	add_var(OPT_BOOL,"im_creature_view_hp_bar","im_chpbar",&im_creature_view_hp_bar, change_var, 0, "Creatures instance banners - health bars", "Show creature health bars when using instance mode",HUD);
	add_var(OPT_BOOL,"im_creature_banner_bg","im_cbbg",&im_creature_banner_bg, change_var, 0, "Creatures instance banners - background", "Show creatures banners background when using instance mode",HUD);
	add_var(OPT_BOOL,"im_other_player_view_names","im_opnm",&im_other_player_view_names, change_var, 0, "Other players instance banners - names", "Show other players names when using instance mode",HUD);
	add_var(OPT_BOOL,"im_other_player_view_hp","im_ophp",&im_other_player_view_hp, change_var, 0, "Other players instance banners - health numbers", "Show other players health numbers when using instance mode",HUD);
	add_var(OPT_BOOL,"im_other_player_view_hp_bar","im_ophpbar",&im_other_player_view_hp_bar, change_var, 0, "Other players instance banners - health bars", "Show other players health bars when using instance mode",HUD);
	add_var(OPT_BOOL,"im_other_player_banner_bg","im_opbbg",&im_other_player_banner_bg, change_var, 0, "Other players instance banners - background", "Show other players banners background when using instance mode",HUD);
	add_var(OPT_BOOL,"im_other_player_show_banner_on_damage","im_opbdmg",&im_other_player_show_banner_on_damage, change_var, 0, "Other players instance banners - show hp on damage", "Show other players banners for a while if player gets hit when using instance mode",HUD);
	// HUD TAB


	// CHAT TAB
	add_var(OPT_MULTI,"windowed_chat", "winchat", &use_windowed_chat, change_windowed_chat, 1, "Chat Display Style", "How do you want your chat to be displayed?", CHAT, "Old behavior", "Tabbed chat", "Chat window", NULL);
	add_var(OPT_BOOL,"local_chat_separate", "locsep", &local_chat_separate, change_separate_flag, 0, "Separate Local Chat", "Should local chat be separate?", CHAT);
	// The forces that be want PMs always global, so that they're less likely to be ignored
	//add_var (OPT_BOOL, "personal_chat_separate", "pmsep", &personal_chat_separate, change_separate_flag, 0, "Separate Personal Chat", "Should personal chat be separate?", CHAT);
	add_var(OPT_BOOL,"guild_chat_separate", "gmsep", &guild_chat_separate, change_separate_flag, 1, "Separate Guild Chat", "Should guild chat be separate?", CHAT);
	add_var(OPT_BOOL,"server_chat_separate", "scsep", &server_chat_separate, change_separate_flag, 0, "Separate Server Messages", "Should the messages from the server be separate?", CHAT);
	add_var(OPT_BOOL,"mod_chat_separate", "modsep", &mod_chat_separate, change_separate_flag, 0, "Separate Moderator Chat", "Should moderator chat be separated from the rest?", CHAT);
	add_var(OPT_BOOL,"highlight_tab_on_nick", "highlight", &highlight_tab_on_nick, change_var, 1, "Highlight Tabs On Name", "Should tabs be highlighted when someone mentions your name?", CHAT);
	add_var(OPT_BOOL,"emote_filter", "emote_filter", &emote_filter, change_var, 1, "Emotes filter", "Do not display lines of text in local chat containing emotes only", CHAT);
	add_var(OPT_BOOL,"summoning_filter", "summ_filter", &summoning_filter, change_var, 0, "Summoning filter", "Do not display lines of text in local chat containing summoning messages", CHAT);
	add_var(OPT_INT,"time_warning_hour","warn_h",&time_warn_h,change_int,-1,"Time warning for new hour","If set to -1, there will be no warning given. Otherwise, you will get a notification in console this many minutes before the new hour",CHAT, -1, 30);
	add_var(OPT_INT,"time_warning_sun","warn_s",&time_warn_s,change_int,-1,"Time warning for dawn/dusk","If set to -1, there will be no warning given. Otherwise, you will get a notification in console this many minutes before sunrise/sunset",CHAT, -1, 30);
	add_var(OPT_INT,"time_warning_day","warn_d",&time_warn_d,change_int,-1,"Time warning for new #day","If set to -1, there will be no warning given. Otherwise, you will get a notification in console this many minutes before the new day",CHAT, -1, 30);
	add_var(OPT_SPECINT,"auto_afk_time","afkt",&afk_time_conf,set_afk_time,5,"AFK Time","The idle time in minutes before the AFK auto message",CHAT,0,INT_MAX);
	add_var(OPT_STRING,"afk_message","afkm",afk_message,change_string,127,"AFK Message","Set the AFK message",CHAT);
	add_var(OPT_BOOL, "afk_local", "afkl", &afk_local, change_var, 0, "Save Local Chat Messages When AFK", "When you go AFK, local chat messages are counted and saved as well as PMs", CHAT);
#ifdef NEW_SOUND
	add_var(OPT_BOOL, "afk_snd_warning", "afks", &afk_snd_warning, change_var, 0, "Play AFK Message Sound", "When you go AFK, a sound is played when you receive a message or trade request", CHAT);
#endif	//NEW_SOUND
	add_var(OPT_BOOL,"use_global_ignores","gign",&use_global_ignores,change_var,1,"Global Ignores","Global ignores is a list with people that are well known for being nasty, so we put them into a list (global_ignores.txt). Enable this to load that list on startup.",CHAT);
	add_var(OPT_BOOL,"save_ignores","sign",&save_ignores,change_var,1,"Save Ignores","Toggle saving of the local ignores list on exit.",CHAT);
	add_var(OPT_BOOL, "use_global_filters", "gfil", &use_global_filters, change_global_filters, 1, "Global Filter", "Toggle the use of global text filters.", CHAT);
	/* add_var(OPT_STRING,"text_filter_replace","trepl",text_filter_replace,change_string,127,"Text Filter","The word to replace bad text with",CHAT); */
	add_var(OPT_BOOL,"caps_filter","caps",&caps_filter,change_var,1,"Caps Filter","Toggle the caps filter",CHAT);
	add_var(OPT_BOOL,"show_timestamp","timestamp",&show_timestamp,change_var,0,"Show Time Stamps","Toggle time stamps for chat messages",CHAT);
	add_var(OPT_MULTI_H,"dark_channeltext","dark_channeltext",&dark_channeltext,change_dark_channeltext,0,"Channel Text Color","Display the channel text in a darker color for better reading on bright maps ('Dark' may be unreadable in F1 screen)",CHAT, "Normal", "Medium", "Dark");
	// CHAT TAB


	// FONT TAB
	add_var(OPT_FLOAT,"name_text_size","nsize",&name_zoom,change_float,1,"Name Text Size","Set the size of the players name text",FONT,0.0,2.0,0.01);
	add_var(OPT_FLOAT,"chat_text_size","csize",&chat_zoom,change_chat_zoom,1,"Chat Text Size","Sets the size of the normal text",FONT,0.0,FLT_MAX,0.01);
	add_var(OPT_FLOAT,"note_text_size", "notesize", &note_zoom, change_note_zoom, 0.8, "Notepad Text Size","Sets the size of the text in the notepad", FONT, 0.0, FLT_MAX, 0.01);
	add_var(OPT_FLOAT,"mapmark_text_size", "marksize", &mapmark_zoom, change_float, 0.3, "Mapmark Text Size","Sets the size of the mapmark text", FONT, 0.0, FLT_MAX, 0.01);
	add_var(OPT_MULTI,"name_font","nfont",&name_font,change_int,0,"Name Font","Change the type of font used for the name",FONT, NULL);
	add_var(OPT_MULTI,"chat_font","cfont",&chat_font,change_int,0,"Chat Font","Set the type of font used for normal text",FONT, NULL);
	// FONT TAB


	// SERVER TAB
	add_var(OPT_STRING,"username","u",username_str,change_string,MAX_USERNAME_LENGTH,"Username","Your user name here",SERVER);
	add_var(OPT_PASSWORD,"password","p",password_str,change_string,MAX_USERNAME_LENGTH,"Password","Put your password here",SERVER);
	add_var(OPT_MULTI,"log_chat","log",&log_chat,change_int,LOG_SERVER,"Log Messages","Log messages from the server (chat, harvesting events, GMs, etc)",SERVER,"Do not log chat", "Log chat only", "Log server messages", "Log server to srv_log.txt", NULL);
	add_var(OPT_BOOL,"rotate_chat_log","rclog",&rotate_chat_log_config_var,change_rotate_chat_log,0,"Rotate Chat Log File","Tag the chat/server message log files with year and month. You will still need to manage deletion of the old files. Requires a client restart.",SERVER);
	add_var(OPT_BOOL,"buddy_log_notice", "buddy_log_notice", &buddy_log_notice, change_var, 1, "Log Buddy Sign On/Off", "Toggle whether to display notices when people on your buddy list log on or off", SERVER);
	add_var(OPT_STRING,"language","lang",lang,change_string,8,"Language","Wah?",SERVER);
	add_var(OPT_STRING,"browser","b",browser_name,change_string,70,"Browser","Location of your web browser (Windows users leave blank to use default browser)",SERVER);
	add_var(OPT_BOOL,"write_ini_on_exit", "wini", &write_ini_on_exit, change_var, 1,"Save INI","Save options when you quit",SERVER);
	add_var(OPT_STRING,"data_dir","dir",datadir,change_dir_name,90,"Data Directory","Place were we keep our data. Can only be changed with a Client restart.",SERVER);
	add_var(OPT_BOOL,"serverpopup","spu",&use_server_pop_win,change_var,1,"Use Special Text Window","Toggles whether server messages from channel 255 are displayed in a pop up window.",SERVER);
	/* Note: We don't take any action on the already-running thread, as that wouldn't necessarily be good. */
	add_var(OPT_BOOL,"autoupdate","aup",&auto_update,change_var,1,"Automatic Updates","Toggles whether updates are automatically downloaded.",SERVER);
#ifdef CUSTOM_UPDATE
	add_var(OPT_BOOL,"customupdate","cup",&custom_update,change_custom_update,1,"Custom Looks Updates","Toggles whether custom look updates are automatically downloaded.",SERVER);
	add_var(OPT_BOOL,"showcustomclothing","scc",&custom_clothing,change_custom_clothing,1,"Show Custom clothing","Toggles whether custom clothing is shown.",SERVER);
#endif	//CUSTOM_UPDATE
	// SERVER TAB


	// AUDIO TAB
#ifdef NEW_SOUND
	add_var(OPT_BOOL,"disable_sound", "nosound", &no_sound, disable_sound, 0, "Disable Sound & Music System", "Disable all of the sound effects and music processing", AUDIO);
	add_var(OPT_STRING,"sound_device", "snddev", sound_device, change_string, 30, "Sound Device", "Device used for playing sounds & music", AUDIO);
	add_var(OPT_BOOL,"enable_sound", "sound", &sound_on, toggle_sounds, 0, "Enable Sound Effects", "Turn sound effects on/off", AUDIO);
	add_var(OPT_FLOAT,"sound_gain", "sgain", &sound_gain, change_sound_level, 1, "Overall Sound Effects Volume", "Adjust the overall sound effects volume", AUDIO, 0.0, 1.0, 0.1);
	add_var(OPT_FLOAT,"crowd_gain", "crgain", &crowd_gain, change_sound_level, 1, "Crowd Sounds Volume", "Adjust the crowd sound effects volume", AUDIO, 0.0, 1.0, 0.1);
	add_var(OPT_FLOAT,"enviro_gain", "envgain", &enviro_gain, change_sound_level, 1, "Environmental Sounds Volume", "Adjust the environmental sound effects volume", AUDIO, 0.0, 1.0, 0.1);
	add_var(OPT_FLOAT,"actor_gain", "again", &actor_gain, change_sound_level, 1, "Character Sounds Volume", "Adjust the sound effects volume for fighting, magic and other character sounds", AUDIO, 0.0, 1.0, 0.1);
	add_var(OPT_FLOAT,"walking_gain", "wgain", &walking_gain, change_sound_level, 1, "Walking Sounds Volume", "Adjust the walking sound effects volume", AUDIO, 0.0, 1.0, 0.1);
	add_var(OPT_FLOAT,"gamewin_gain", "gwgain", &gamewin_gain, change_sound_level, 1, "Item and Inventory Sounds Volume", "Adjust the item and inventory sound effects volume", AUDIO, 0.0, 1.0, 0.1);
	add_var(OPT_FLOAT,"client_gain", "clgain", &client_gain, change_sound_level, 1, "Misc Client Sounds Volume", "Adjust the client sound effects volume (warnings, hud/button clicks)", AUDIO, 0.0, 1.0, 0.1);
	add_var(OPT_FLOAT,"warn_gain", "wrngain", &warnings_gain, change_sound_level, 1, "Text Warning Sounds Volume", "Adjust the user configured text warning sound effects volume", AUDIO, 0.0, 1.0, 0.1);
	add_var(OPT_BOOL,"enable_music","music",&music_on,toggle_music,0,"Enable Music","Turn music on/off",AUDIO);
	add_var(OPT_FLOAT,"music_gain","mgain",&music_gain,change_sound_level,1,"Music Volume","Adjust the music volume",AUDIO,0.0,1.0,0.1);
#endif	//NEW_SOUND
	// AUDIO TAB


	// VIDEO TAB
	add_var(OPT_BOOL,"full_screen","fs",&full_screen,toggle_full_screen_mode,0,"Full Screen","Changes between full screen and windowed mode",VIDEO);
	add_var(OPT_MULTI,"video_mode","vid",&video_mode,switch_vidmode,4,"Video Mode","The video mode you wish to use",VIDEO, "Userdefined", NULL);
	for (i = 0; i < video_modes_count; i++)
	{
		static char str[100];
		safe_snprintf(str, sizeof(str), "%dx%dx%d", video_modes[i].width, video_modes[i].height, video_modes[i].bpp);
		if (video_modes[i].name)
			free(video_modes[i].name);
		video_modes[i].name = strdup(str);
		add_multi_option("video_mode", video_modes[i].name);
	}
	add_var(OPT_BOOL, "disable_window_adjustment", "nowindowadjustment", &disable_window_adjustment, change_var, 0, "Disable window size adjustment","Disables the window size adjustment on video mode changes in windowed mode.", VIDEO);
	add_var(OPT_INT,"video_width","width",&video_user_width,change_int, 640,"Userdefined width","Userdefined window width",VIDEO, 640,INT_MAX);
	add_var(OPT_INT,"video_height","height",&video_user_height,change_int, 480,"Userdefined height","Userdefined window height",VIDEO, 480,INT_MAX);
	add_var(OPT_INT,"limit_fps","lfps",&limit_fps,change_int,0,"Limit FPS","Limit the frame rate to reduce load on the system",VIDEO,0,INT_MAX);
	add_var(OPT_FLOAT,"gamma","g",&gamma_var,change_gamma,1,"Gamma","How bright your display should be.",VIDEO,0.10,3.00,0.05);
	add_var(OPT_BOOL,"disable_gamma_adjust","dga",&disable_gamma_adjust,change_var,0,"Disable Gamma Adjustment","Stop the client from adjusting the display gamma.",VIDEO);
#ifdef ANTI_ALIAS
	add_var(OPT_BOOL,"anti_alias", "aa", &anti_alias, change_aa, 0, "Toggle Anti-Aliasing", "Anti-aliasing makes edges look smoother", VIDEO);
#endif //ANTI_ALIAS
#ifdef	FSAA
	add_var(OPT_MULTI_H, "anti_aliasing", "fsaa", &fsaa_index, change_fsaa, 0, "Anti-Aliasing", "Full Scene Anti-Aliasing", VIDEO, get_fsaa_mode_str(0), 0);
	for (i = 1; i < get_fsaa_mode_count(); i++)
	{
		if (get_fsaa_mode(i) == 1)
		{
			add_multi_option("anti_aliasing", get_fsaa_mode_str(i));
		}
	}
#endif	/* FSAA */
	add_var (OPT_BOOL, "use_frame_buffer", "fb", &use_frame_buffer, change_frame_buffer, 0, "Toggle Frame Buffer Support", "Toggle frame buffer support. Used for reflection and shadow mapping.", VIDEO);
	add_var(OPT_INT_F,"water_shader_quality","water_shader_quality",&water_shader_quality,change_water_shader_quality,1,"  water shader quality","Defines what shader is used for water rendering. Higher values are slower but look better. Needs \"toggle frame buffer support\" to be turned on.",VIDEO, int_zero_func, int_max_water_shader_quality);
#ifdef	NEW_TEXTURES
	add_var(OPT_BOOL,"small_actor_texture_cache","small_actor_tc",&small_actor_texture_cache,change_small_actor_texture_cache,0,"Small actor texture cache","A small Actor texture cache uses less video memory, but actor loading can be slower.",VIDEO);
#else	/* NEW_TEXTURES */
	add_var(OPT_BOOL,"use_mipmaps","mm",&use_mipmaps,change_mipmaps,0,"Mipmaps","Mipmaps is a texture effect that blurs the texture a bit - it may look smoother and better, or it may look worse depending on your graphics driver settings and the like.",VIDEO);
#endif	/* NEW_TEXTURES */
	add_var(OPT_BOOL,"use_vertex_buffers","vbo",&use_vertex_buffers,change_vertex_buffers,0,"Vertex Buffer Objects","Toggle the use of the vertex buffer objects, restart required to activate it",VIDEO);
	add_var(OPT_BOOL, "use_animation_program", "uap", &use_animation_program, change_use_animation_program, 1, "Use animation program", "Use GL_ARB_vertex_program for actor animation", VIDEO);
	add_var(OPT_BOOL_INI, "video_info_sent", "svi", &video_info_sent, change_var, 0, "Video info sent", "Video information are sent to the server (like OpenGL version and OpenGL extentions)", VIDEO);
	// VIDEO TAB


	// GFX TAB
	add_var(OPT_BOOL,"shadows_on","shad",&shadows_on,change_shadows,0,"Shadows","Toggles the shadows", GFX);
	add_var(OPT_BOOL,"use_shadow_mapping", "sm", &use_shadow_mapping, change_shadow_mapping, 0, "Shadow Mapping", "If you want to use some better quality shadows, enable this. It will use more resources, but look prettier.", GFX);
	add_var(OPT_MULTI,"shadow_map_size","smsize",&shadow_map_size_multi,change_shadow_map_size,1024,"Shadow Map Size","This parameter determines the quality of the shadow maps. You should as minimum set it to 512.",GFX,"256","512","768","1024","1280","1536","1792","2048","3072","4096",NULL);
	add_var(OPT_BOOL,"no_adjust_shadows","noadj",&no_adjust_shadows,change_var,0,"Don't Adjust Shadows","If enabled, tell the engine not to disable the shadows if the frame rate is too low.",GFX);
	add_var(OPT_BOOL,"clouds_shadows","cshad",&clouds_shadows,change_clouds_shadows,1,"Cloud Shadows","The clouds shadows are projected on the ground, and the game looks nicer with them on.",GFX);
	add_var(OPT_BOOL,"show_reflection","refl",&show_reflection,change_reflection,1,"Show Reflections","Toggle the reflections",GFX);
	add_var(OPT_BOOL,"render_fog","fog",&use_fog,change_var,1,"Render Fog","Toggles fog rendering.",GFX);
	add_var(OPT_BOOL,"show_weather","weather",&show_weather,change_var,1,"Show Weather Effects","Toggles thunder, lightning and rain effects.",GFX);
	add_var(OPT_BOOL,"skybox_show_sky","sky", &skybox_show_sky, change_sky_var,1,"Show Sky", "Enable the sky box.", GFX);
/* 	add_var(OPT_BOOL,"reflect_sky","reflect_sky", &reflect_sky, change_var,1,"Reflect Sky", "Sky Performance Option. Disable these from top to bottom until you're happy", GFX); */
	add_var(OPT_BOOL,"skybox_show_clouds","sky_clouds", &skybox_show_clouds, change_sky_var,1,"Show Clouds", "Sky Performance Option. Disable these from top to bottom until you're happy", GFX);
/*	add_var(OPT_BOOL,"horizon_fog","horizon_fog", &skybox_show_horizon_fog, change_sky_var,1,"Show Horizon Fog", "Sky Performance Option. Disable these from top to bottom until you're happy", GFX); */
	add_var(OPT_BOOL,"skybox_show_sun","sky_sun", &skybox_show_sun, change_sky_var,1,"Show Sun", "Sky Performance Option. Disable these from top to bottom until you're happy", GFX);
	add_var(OPT_BOOL,"skybox_show_moons","sky_moons", &skybox_show_moons, change_sky_var,1,"Show Moons", "Sky Performance Option. Disable these from top to bottom until you're happy", GFX);
	add_var(OPT_BOOL,"skybox_show_stars","sky_stars", &skybox_show_stars, change_sky_var,1,"Show Stars", "Sky Performance Option. Disable these from top to bottom until you're happy", GFX);
	add_var(OPT_INT,"skybox_update_delay","skybox_update_delay", &skybox_update_delay, change_int, skybox_update_delay, "Sky Update Delay", "Specifies the delay in seconds between 2 updates of the sky and the environment. A value of 0 corresponds to an update at every frame.", GFX, 0, 60);
	add_var(OPT_INT,"particles_percentage","pp",&particles_percentage,change_particles_percentage,100,"Particle Percentage","If you experience a significant slowdown when particles are nearby, you should consider lowering this number.",GFX,0,100);
	add_var(OPT_BOOL,"special_effects", "sfx", &special_effects, change_var, 1, "Toggle Special Effects", "Special spell effects", GFX);
#ifdef	NEW_TEXTURES
	add_var(OPT_BOOL,"use_eye_candy", "ec", &use_eye_candy, change_eye_candy, 1, "Enable Eye Candy", "Toggles most visual effects, like spells' and harvesting events'. Needs OpenGL 1.5", GFX);
#else	/* NEW_TEXTURES */
	add_var(OPT_BOOL,"use_eye_candy", "ec", &use_eye_candy, change_var, 1, "Enable Eye Candy", "Toggles most visual effects, like spells' and harvesting events'", GFX);
#endif	/* NEW_TEXTURES */
	add_var(OPT_BOOL,"enable_blood","eb",&enable_blood,change_var,0,"Enable Blood","Enable blood special effects during combat.",GFX);
	add_var(OPT_BOOL,"use_harvesting_eye_candy","uharvec",&use_harvesting_eye_candy,change_var,0,"Enable harvesting effect","This effect shows that you're harvesting. Only you can see it!",GFX);
	add_var(OPT_BOOL,"use_lamp_halo","ulh",&use_lamp_halo,change_var,0,"Use Lamp Halos","Enable halos for torches, candles, etc.",GFX);
	add_var(OPT_BOOL,"use_fancy_smoke","ufs",&use_fancy_smoke,change_var,0,"Use Fancy Smoke","If your system has performance problems around chimney smoke, turn this option off.",GFX);
	add_var(OPT_FLOAT,"max_ec_framerate","ecmaxf",&max_ec_framerate,change_max_ec_framerate,45,"Max Effects Framerate","If your framerate is above this amount, eye candy will use maximum detail.",GFX,2.0,FLT_MAX,1.0);
	add_var(OPT_FLOAT,"min_ec_framerate","ecminf",&min_ec_framerate,change_min_ec_framerate,15,"Min Effects Framerate","If your framerate is below this amount, eye candy will use minimum detail.",GFX,1.0,FLT_MAX,1.0);
	add_var(OPT_INT,"light_columns_threshold","lct",&light_columns_threshold,change_int,5,"Light columns threshold","If your framerate is below this amount, you will not get columns of light around teleportation effects (useful for slow systems).",GFX, 0, INT_MAX);
	add_var(OPT_INT,"max_idle_cycles_per_second","micps",&max_idle_cycles_per_second,change_int,40,"Max Idle Cycles Per Second","The eye candy 'idle' function, which moves particles around, will run no more than this often.  If your CPU is your limiting factor, lowering this can give you a higher framerate.  Raising it gives smoother particle motion (up to the limit of your framerate).",GFX, 1, INT_MAX);
#ifdef	NEW_ALPHA
	add_var(OPT_BOOL,"use_3d_alpha_blend","3dalpha",&use_3d_alpha_blend,change_var,1,"3D Alpha Blending","Toggle the use of the alpha blending on 3D objects",GFX);
#endif	//NEW_ALPHA
	// GFX TAB


	// CAMERA TAB
	add_var(OPT_FLOAT,"far_plane", "far_plane", &far_plane, change_projection_float, 100.0, "Maximum Viewing Distance", "Adjusts how far you can see.", CAMERA, 40.0, 200.0, 1.0);
	add_var(OPT_FLOAT,"far_reflection_plane", "far_reflection_plane", &far_reflection_plane, change_projection_float, 100.0, "Maximum Reflection Distance", "Adjusts how far the reflections are displayed.", CAMERA, 0.0, 200.0, 1.0);
	add_var(OPT_FLOAT,"max_zoom_level","maxzoomlevel",&max_zoom_level,change_float,max_zoom_level,"Maximum Camera Zoom Out","Sets the maxiumum value that the camera can zoom out",CAMERA,4.0,8.0,0.5);
#ifndef OSX
	add_var(OPT_FLOAT,"perspective", "perspective", &perspective, change_projection_float, 0.15f, "Perspective", "The degree of perspective distortion. Change if your view looks odd.", CAMERA, 0.01, 0.80, 0.01);
#else // OSX
	add_var(OPT_FLOAT,"perspective", "perspective", &perspective, change_projection_float_init, 0.15f, "Perspective", "The degree of perspective distortion. Change if your view looks odd.", CAMERA, 0.01, 0.80, 0.01);
#endif // OSX
#ifndef OSX
	add_var(OPT_BOOL,"isometric" ,"isometric", &isometric, change_projection_bool, 1, "Use Isometric View", "Toggle the use of isometric (instead of perspective) view", CAMERA);
#else // OSX
	add_var(OPT_BOOL,"isometric" ,"isometric", &isometric, change_projection_bool_init, 1, "Use Isometric View, restart required", "Toggle the use of isometric (instead of perspective) view", CAMERA);
#endif // OSX
	add_var(OPT_BOOL,"follow_cam","folcam", &fol_cam, toggle_follow_cam,0,"Follow Camera", "Causes the camera to stay fixed relative to YOU and not the world", CAMERA);
	add_var(OPT_BOOL,"fol_cam_behind","fol_cam_behind", &fol_cam_behind, toggle_follow_cam_behind,0,"Keep the camera behind the char", "Causes the camera to stay behind you while walking (works only in follow camera mode)", CAMERA);
	add_var(OPT_BOOL,"extended_cam","extcam", &ext_cam, toggle_ext_cam,0,"Extended Camera", "Camera range of motion extended and adjusted to allow overhead and first person style camera.", CAMERA);
	add_var(OPT_BOOL,"ext_cam_auto_zoom","autozoom", &ext_cam_auto_zoom, change_var,0,"Auto zoom", "Allows the camera to zoom automatically when getting close to the ground (works only in extended camera mode and with a max tilt angle over 90.0).", CAMERA);
	add_var(OPT_FLOAT,"normal_camera_rotation_speed","nrot",&normal_camera_rotation_speed,change_float,15,"Camera Rotation Speed","Set the speed the camera rotates",CAMERA,1.0,FLT_MAX,0.5);
	add_var(OPT_FLOAT,"fine_camera_rotation_speed","frot",&fine_camera_rotation_speed,change_float,1,"Fine Rotation Speed","Set the fine camera rotation speed (when holding shift+arrow key)",CAMERA,1.0,FLT_MAX,0.5);
	add_var(OPT_FLOAT,"normal_camera_deceleration","ncd",&normal_camera_deceleration,change_float,normal_camera_deceleration,"Camera Rotation Deceleration","Set the camera rotation deceleration",CAMERA,0.01,1.0,0.01);
	add_var(OPT_FLOAT,"min_tilt_angle","min_tilt_angle", &min_tilt_angle, change_tilt_float,30.0,"Minimum tilt angle", "Minimum angle that the camera can reach when raising it (works only in extended camera mode).", CAMERA, 20.0, 45.0, 1.0);
	add_var(OPT_FLOAT,"max_tilt_angle","max_tilt_angle", &max_tilt_angle, change_tilt_float,90.0,"Maximum tilt angle", "Maximum angle that the camera can reach when lowering it (works only in extended camera mode).", CAMERA, 60.0, 150.0, 1.0);
	add_var(OPT_FLOAT,"follow_strength","f_strn",&fol_strn,change_float,0.1,"Follow Camera Snapiness","Adjust how responsive the follow camera is. 0 is stopped, 1 is fastest. Use the three numbers below to tweak the feel. Try them one at a time, then mix them to find a ratio you like.",CAMERA,0.0,1.00,0.01);
	add_var(OPT_FLOAT,"const_speed","f_con",&fol_con,change_float,7,"Constant Speed","The basic rate that the camera rotates at to keep up with you.",CAMERA,0.0,10.00,1.0);
	add_var(OPT_FLOAT,"lin_speed","f_lin",&fol_lin,change_float,1,"Linear Decel.","A hit of speed that drops off as the camera gets near its set point.",CAMERA,0.0,10.00,1.0);
	add_var(OPT_FLOAT,"quad_speed","f_quad",&fol_quad,change_float,1,"Quadratic Decel.","A hit of speed that drops off faster as it nears the set point.",CAMERA,0.0,10.00,1.0);
	// CAMERA TAB


	// TROUBLESHOOT TAB
	add_var(OPT_BOOL,"shadows_on","shad",&shadows_on,change_shadows,0,"Shadow Bug","Some video cards have trouble with the shadows. Uncheck this if everything you see is white.", TROUBLESHOOT);
	// Grum: attempt to work around bug in Ati linux drivers.
	add_var(OPT_BOOL,"ati_click_workaround", "atibug", &ati_click_workaround, change_var, 0, "ATI Bug", "If you are using an ATI graphics card and don't move when you click, try this option to work around a bug in their drivers.", TROUBLESHOOT);
	add_var (OPT_BOOL,"use_old_clicker", "oldmclick", &use_old_clicker, change_var, 0, "Mouse Bug", "Unrelated to ATI graphics cards, if clicking to walk doesn't move you, try toggling this option.", TROUBLESHOOT);
	add_var(OPT_BOOL,"use_new_selection", "uns", &use_new_selection, change_new_selection, 1, "New selection", "Using new selection can give you a higher framerate.  However, if your cursor does not change when over characters or items, try disabling this option.", TROUBLESHOOT);
	add_var(OPT_BOOL,"use_compiled_vertex_array","cva",&use_compiled_vertex_array,change_compiled_vertex_array,1,"Compiled Vertex Array","Some systems will not support the new compiled vertex array in EL. Disable this if some 3D objects do not display correctly.",TROUBLESHOOT);
	add_var(OPT_BOOL,"use_draw_range_elements","dre",&use_draw_range_elements,change_var,1,"Draw Range Elements","Disable this if objects appear partially stretched.",TROUBLESHOOT);
	add_var(OPT_BOOL,"use_point_particles","upp",&use_point_particles,change_point_particles,1,"Point Particles","Some systems will not support the new point based particles in EL. Disable this if your client complains about not having the point based particles extension.",TROUBLESHOOT);
#ifndef	NEW_TEXTURES
	add_var(OPT_BOOL,"transparency_resolution_fix","trf",&transparency_resolution_fix,change_var,0,"Transparency Resolution Fix","Use this if your video card or driver has problems with rendering highly blended effects, like teleportation.",TROUBLESHOOT);
#endif	/* NEW_TEXTURES */
	add_var(OPT_INT, "gx_adjust","gxa", &gx_adjust, change_signed_int, 0, "Adjust graphics X","Fine adjustment for text/line positioning - X direction.",TROUBLESHOOT, -3,3);
	add_var(OPT_INT, "gy_adjust","gxa", &gy_adjust, change_signed_int, 0, "Adjust graphics Y","Fine adjustment for text/line positioning - Y direction.",TROUBLESHOOT, -3,3);
#ifdef OSX
	add_var(OPT_BOOL, "square_buttons", "sqbutt",&square_buttons,change_var,1,"Square Buttons","Use square buttons rather than rounded",TROUBLESHOOT);
#endif
	add_var(OPT_BOOL, "use_animation_program", "uap", &use_animation_program, change_use_animation_program, 1, "Use animation program", "Use GL_ARB_vertex_program for actor animation", TROUBLESHOOT);
	add_var(OPT_BOOL,"poor_man","poor",&poor_man,change_poor_man,0,"Poor Man","If the game is running very slow for you, toggle this setting.",TROUBLESHOOT);
	// TROUBLESHOOT TAB

	// DEBUGTAB TAB
#ifdef DEBUG
	add_var(OPT_FLOAT,"sunny_sky_bias","sunny_sky_bias", &skybox_sunny_sky_bias, change_float,0.0,"Sunny sky bias", "Change the radius of the sun effect on the sky.", DEBUGTAB, -1.0, 1.0, 0.01);
	add_var(OPT_FLOAT,"sunny_clouds_bias","sunny_clouds_bias", &skybox_sunny_clouds_bias, change_float,-0.1,"Sunny clouds bias", "Change the radius of the sun effect on the clouds.", DEBUGTAB, -1.0, 1.0, 0.01);
	add_var(OPT_FLOAT,"sunny_fog_bias","sunny_fog_bias", &skybox_sunny_fog_bias, change_float,0.0,"Sunny fog bias", "Change the radius of the sun effect on the fog.", DEBUGTAB, -1.0, 1.0, 0.01);
	add_var(OPT_FLOAT,"water_tiles_extension","wt_ext", &water_tiles_extension, change_float,200.0,"Water tiles extension", "Extends the water tiles upto the specified distance.", DEBUGTAB, 0.0, 1000.0, 1.0);
	add_var(OPT_BOOL,"enable_client_aiming","eca",&enable_client_aiming,change_var,0,"Enable client aiming","Allow to aim at something by holding CTRL key. This aim is only done on client side and is used only for debugging purposes. Warning: enabling this code can produce server resyncs or locks when playing with missiles...",DEBUGTAB);
	add_var(OPT_BOOL,"render_skeleton","rskel",&render_skeleton,change_var,0,"Render Skeleton", "Render the Cal3d skeletons.", DEBUGTAB);
	add_var(OPT_BOOL,"render_mesh","rmesh",&render_mesh,change_var,1,"Render Mesh", "Render the meshes", DEBUGTAB);
	add_var(OPT_BOOL,"render_bones_id","rbid",&render_bones_id,change_var,0,"Render bones ID", "Render the bones ID", DEBUGTAB);
	add_var(OPT_BOOL,"render_bones_orientation","rbor",&render_bones_orientation,change_var,0,"Render bones orientation", "Render the bones orientation", DEBUGTAB);
	add_var(OPT_FLOAT,"near_plane", "near_plane", &near_plane, change_projection_float, 0.1, "Minimum Viewing Distance", "Adjusts how near you can see.", DEBUGTAB, 0.1, 10.0, 0.1);
	add_var(OPT_BOOL,"skybox_local_weather","skybox_local_weather", &skybox_local_weather, change_var,0,"Local Weather", "Show local weather areas on the sky. It allows to see distant weather but can reduce performance.", DEBUGTAB);
#endif // DEBUG
	// DEBUGTAB TAB

} /* end init_ELC_vars() */
#endif // def ELC



void init_vars()
{
#ifdef ELC
	init_ELC_vars();
	
#else
	// NOTE !!!!
	// some repeated in init_ELC_vars() so that we can control the order showin in the tabs

	//Global vars...
	// Only possible to do at startup - this could of course be changed by using a special function for this purpose. I just don't see why you'd want to change the directory whilst running the game...
	add_var(OPT_STRING,"data_dir","dir",datadir,change_dir_name,90,"Data Directory","Place were we keep our data. Can only be changed with a Client restart.",SERVER);
	add_var(OPT_INT,"limit_fps","lfps",&limit_fps,change_int,0,"Limit FPS","Limit the frame rate to reduce load on the system",VIDEO,0,INT_MAX);
#ifdef MAP_EDITOR
	add_var(OPT_INT,"video_mode","vid",&video_mode,switch_vidmode,4,"Video Mode","The video mode you wish to use",VIDEO,1,7);
	add_var(OPT_BOOL,"close_browser_on_select","cbos",&close_browser_on_select, change_var, 0,"Close Browser","Close the browser on select",HUD);
	add_var(OPT_BOOL,"show_position_on_minimap","spos",&show_position_on_minimap, change_var, 0,"Show Pos","Show position on the minimap",HUD);
	add_var(OPT_SPECINT,"auto_save","asv",&auto_save_time, set_auto_save_interval, 0,"Auto Save","Auto Save",HUD,0,INT_MAX);
	add_var(OPT_BOOL,"show_grid","sgrid",&view_grid, change_var, 0, "Show Grid", "Show grid",HUD);
#endif
#ifndef MAP_EDITOR
	add_var (OPT_BOOL, "use_frame_buffer", "fb", &use_frame_buffer, change_frame_buffer, 0, "Toggle Frame Buffer Support", "Toggle frame buffer support. Used for reflection and shadow mapping.", VIDEO);
	add_var(OPT_BOOL, "use_animation_program", "uap", &use_animation_program, change_use_animation_program, 1, "Use animation program", "Use GL_ARB_vertex_program for actor animation", VIDEO);
#endif //MAP_EDITOR
#ifdef OSX
	add_var(OPT_BOOL, "square_buttons", "sqbutt",&square_buttons,change_var,1,"Square Buttons","Use square buttons rather than rounded",HUD);
#endif

#endif // ELC

}

void write_var (FILE *fout, int ivar)
{
	if (fout == NULL) return;

	switch (our_vars.var[ivar]->type)
	{
		case OPT_INT:
		case OPT_MULTI:
		case OPT_MULTI_H:
		case OPT_BOOL:
		case OPT_INT_F:
		case OPT_BOOL_INI:
		case OPT_INT_INI:
		{
			int *p= our_vars.var[ivar]->var;
			fprintf (fout, "#%s= %d\n", our_vars.var[ivar]->name, *p);
			break;
		}
		case OPT_STRING:
			if (strcmp (our_vars.var[ivar]->name, "password") == 0)
				// Do not write the password to the file. If the user really wants it
				// s/he should edit the file.
				fprintf (fout, "#%s= \"\"\n", our_vars.var[ivar]->name);
			else
				fprintf (fout, "#%s= \"%s\"\n", our_vars.var[ivar]->name, (char *)our_vars.var[ivar]->var);
			break;
		case OPT_PASSWORD:
			// Do not write the password to the file. If the user really wants it
			// s/he should edit the file.
			fprintf (fout, "#%s= \"\"\n", our_vars.var[ivar]->name);
			break;
		case OPT_FLOAT:
		case OPT_FLOAT_F:
		{
			float *g= our_vars.var[ivar]->var;
			fprintf (fout, "#%s= %g\n", our_vars.var[ivar]->name, *g);
			break;
		}
	}
	our_vars.var[ivar]->saved= 1;	// keep only one copy of this setting
}


int read_el_ini ()
{
	input_line line;
#ifdef MAP_EDITOR
	FILE *fin= open_file_config("mapedit.ini", "r");
#else
	FILE *fin= open_file_config("el.ini", "r");
#endif //MAP_EDITOR

	if (fin == NULL){
		LOG_ERROR("%s: %s \"el.ini\": %s\n", reg_error_str, cant_open_file, strerror(errno));
		return 0;
	}


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
#if !defined(WINDOWS)
	int fd;
	struct stat statbuff;
#endif // !WINDOWS
	int nlines= 0, maxlines= 0, iline, ivar;
	input_line *cont= NULL;
	input_line last_line;
	FILE *file;
	short *written = NULL;

	// Prevent duplicate entries by remembering which we have written
	written = calloc(our_vars.no, sizeof(short));

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

	// Consolidate changes for any items featured more than once - on different tabs for example.
	for (ivar= 0; ivar < our_vars.no; ivar++)
	{
		if (!our_vars.var[ivar]->saved)
		{
			size_t i;
			for (i=0; i<our_vars.no; i++)
				if (strcmp(our_vars.var[ivar]->name, our_vars.var[i]->name) == 0)
					if (our_vars.var[i]->saved)
						our_vars.var[i]->saved = 0;
		}
	}

	// read the ini file
	file = open_file_config("el.ini", "r");
	if(file == NULL){
		LOG_ERROR("%s: %s \"el.ini\": %s\n", reg_error_str, cant_open_file, strerror(errno));
	} else {
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
	file = open_file_config("el.ini", "w");
	if(file == NULL){
		LOG_ERROR("%s: %s \"el.ini\": %s\n", reg_error_str, cant_open_file, strerror(errno));
		return 0;
	}

	strcpy(last_line, "");
	for (iline= 0; iline < nlines; iline++)
	{
		if (cont[iline][0] != '#')
		{
			if (strcmp(cont[iline], last_line) != 0)
				fprintf (file, "%s", cont[iline]);
		}
		else
		{
			ivar= find_var (&(cont[iline][1]), INI_FILE_VAR);
			if (ivar >= 0 && written[ivar])
				continue;
			if (ivar < 0 || our_vars.var[ivar]->saved)
				fprintf (file, "%s", cont[iline]);
			else
				write_var (file, ivar);
			if (ivar >= 0)
				written[ivar] = 1;
		}
		strcpy(last_line, cont[iline]);
	}

	// now write all variables that still haven't been saved yet
	for (ivar= 0; ivar < our_vars.no; ivar++)
	{
		// check if we already wrote a var with the same name
		int check_var= find_var (our_vars.var[ivar]->name, INI_FILE_VAR);
		if (check_var >= 0 && written[check_var])
			continue;
		if (!our_vars.var[ivar]->saved)
		{
			fprintf (file, "\n");
			write_var (file, ivar);
			written[ivar] = 1;
		}
	}
#if !defined(WINDOWS)
	fd = fileno (file);
	fstat (fd, &statbuff);
	/* Set perms to 600 on el_ini if they are anything else */
	if (statbuff.st_mode != (S_IRUSR|S_IWUSR)){
		fchmod (fd, S_IRUSR|S_IWUSR);
	}
#endif // !WINDOWS

	fclose (file);
	free (written);
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
		// Update the widgets in case an option changed
		// Actually that's ony the OPT_MULTI type widgets, the others are
		// taken care of by their widget handlers
		if ((our_vars.var[i]->type == OPT_MULTI) || (our_vars.var[i]->type == OPT_MULTI_H))
			multiselect_set_selected (elconfig_tabs[our_vars.var[i]->widgets.tab_id].tab, our_vars.var[i]->widgets.widget_id, *(int *)our_vars.var[i]->var);
	}

	// Draw the long description of an option
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
	put_small_text_in_box(our_vars.var[i]->display.desc, strlen((char*)our_vars.var[i]->display.desc),
								elconfig_menu_x_len-TAB_MARGIN*2, (char*)elconf_description_buffer);
	return 1;
}

int onclick_label_handler(widget_list *widget, int mx, int my, Uint32 flags)
{
	int i;
	var_struct *option= NULL;

	if(!(flags&(ELW_LEFT_MOUSE|ELW_RIGHT_MOUSE)))
		return 0;

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

	if (option->type == OPT_BOOL)
	{
		option->func(option->var);
		option->saved= 0;
	}

	do_click_sound();
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

	if (option->type == OPT_BOOL)
	{
		int *var= option->var;
		*var= !*var;
		option->func(var);
		option->saved= 0;
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
	int x; //Used for the position of multiselect buttons
	void *min, *max; //For the spinbuttons
	float *interval;
	int_min_max_func *i_min_func;
	int_min_max_func *i_max_func;
	float_min_max_func *f_min_func;
	float_min_max_func *f_max_func;

	for(i= 0; i < MAX_TABS; i++) {
		//Set default values
		elconfig_tabs[i].x= TAB_MARGIN;
		elconfig_tabs[i].y= TAB_MARGIN;
	}

	for(i= 0; i < our_vars.no; i++) {
		tab_id= our_vars.var[i]->widgets.tab_id;
		switch(our_vars.var[i]->type) {
			case OPT_BOOL_INI:
			case OPT_INT_INI:
				// This variable should not be settable
				// through the window, so don't try to add it,
				// and more importantly, don't try to compute
				// its height
				continue;
			case OPT_BOOL:
				//Add checkbox
				widget_id= checkbox_add_extended(elconfig_tabs[tab_id].tab, elconfig_free_widget_id++, NULL,
											elconfig_tabs[tab_id].x, elconfig_tabs[tab_id].y, CHECKBOX_SIZE, CHECKBOX_SIZE, 0, 1.0, 0.77f, 0.59f, 0.39f, our_vars.var[i]->var);
				//Add label for the checkbox
				label_id= label_add(elconfig_tabs[tab_id].tab, NULL, (char*)our_vars.var[i]->display.str, elconfig_tabs[tab_id].x+CHECKBOX_SIZE+SPACING, elconfig_tabs[tab_id].y);
				//Set handlers
				widget_set_OnClick(elconfig_tabs[tab_id].tab, label_id, onclick_label_handler);
				widget_set_OnClick(elconfig_tabs[tab_id].tab, widget_id, onclick_checkbox_handler);
			break;
			case OPT_INT:
				min= queue_pop(our_vars.var[i]->queue);
				max= queue_pop(our_vars.var[i]->queue);
				/* interval is always 1 */

				label_id= label_add_extended(elconfig_tabs[tab_id].tab, elconfig_free_widget_id++, NULL, elconfig_tabs[tab_id].x, elconfig_tabs[tab_id].y, 0, 1.0, 0.77f, 0.59f, 0.39f, (char*)our_vars.var[i]->display.str);
				widget_id= spinbutton_add(elconfig_tabs[tab_id].tab, NULL, elconfig_menu_x_len/4*3, elconfig_tabs[tab_id].y, 100, 20, SPIN_INT, our_vars.var[i]->var, *(int *)min, *(int *)max, 1.0);
				widget_set_OnKey(elconfig_tabs[tab_id].tab, widget_id, spinbutton_onkey_handler);
				widget_set_OnClick(elconfig_tabs[tab_id].tab, widget_id, spinbutton_onclick_handler);
				free(min);
				free(max);
				queue_destroy(our_vars.var[i]->queue);
				our_vars.var[i]->queue= NULL;
			break;
			case OPT_FLOAT:
				min= queue_pop(our_vars.var[i]->queue);
				max= queue_pop(our_vars.var[i]->queue);
				interval= (float *)queue_pop(our_vars.var[i]->queue);

				label_id= label_add_extended(elconfig_tabs[tab_id].tab, elconfig_free_widget_id++, NULL, elconfig_tabs[tab_id].x, elconfig_tabs[tab_id].y, 0, 1.0, 0.77f, 0.59f, 0.39f, (char*)our_vars.var[i]->display.str);

				widget_id= spinbutton_add(elconfig_tabs[tab_id].tab, NULL, elconfig_menu_x_len/4*3, elconfig_tabs[tab_id].y, 100, 20, SPIN_FLOAT, our_vars.var[i]->var, *(float *)min, *(float *)max, *interval);
				widget_set_OnKey(elconfig_tabs[tab_id].tab, widget_id, spinbutton_onkey_handler);
				widget_set_OnClick(elconfig_tabs[tab_id].tab, widget_id, spinbutton_onclick_handler);
				free(min);
				free(max);
				free(interval);
				queue_destroy(our_vars.var[i]->queue);
				our_vars.var[i]->queue= NULL;
			break;
			case OPT_STRING:

				label_id= label_add_extended(elconfig_tabs[tab_id].tab, elconfig_free_widget_id++, NULL, elconfig_tabs[tab_id].x, elconfig_tabs[tab_id].y, 0, 1.0, 0.77f, 0.59f, 0.39f, (char*)our_vars.var[i]->display.str);
				widget_id= pword_field_add_extended(elconfig_tabs[tab_id].tab, elconfig_free_widget_id++, NULL, elconfig_menu_x_len/5*2, elconfig_tabs[tab_id].y, 332, 20, P_TEXT, 1.0f, 0.77f, 0.59f, 0.39f, our_vars.var[i]->var, our_vars.var[i]->len);
				widget_set_OnKey (elconfig_tabs[tab_id].tab, widget_id, string_onkey_handler);
			break;
			case OPT_PASSWORD:
				// Grum: the client shouldn't store the password, so let's not add it to the configuration window
				//label_id= label_add_extended(elconfig_tabs[tab_id].tab, elconfig_free_widget_id++, NULL, elconfig_tabs[tab_id].x, elconfig_tabs[tab_id].y, 0, 0, 0, 1.0, 0.77f, 0.59f, 0.39f, our_vars.var[i]->display.str);
				//widget_id= pword_field_add_extended(elconfig_tabs[tab_id].tab, elconfig_free_widget_id++, NULL, elconfig_menu_x_len/2, elconfig_tabs[tab_id].y, 200, 20, P_NORMAL, 1.0f, 0.77f, 0.59f, 0.39f, our_vars.var[i]->var, our_vars.var[i]->len);
				//widget_set_OnKey (elconfig_tabs[tab_id].tab, widget_id, string_onkey_handler);
				continue;
			case OPT_MULTI:

				label_id= label_add_extended(elconfig_tabs[tab_id].tab, elconfig_free_widget_id++, NULL, elconfig_tabs[tab_id].x, elconfig_tabs[tab_id].y, 0, 1.0, 0.77f, 0.59f, 0.39f, (char*)our_vars.var[i]->display.str);
				widget_id= multiselect_add_extended(elconfig_tabs[tab_id].tab, elconfig_free_widget_id++, NULL, elconfig_tabs[tab_id].x+SPACING+get_string_width(our_vars.var[i]->display.str), elconfig_tabs[tab_id].y, 250, 80, 1.0f, 0.77f, 0.59f, 0.39f, 0.32f, 0.23f, 0.15f, 0);
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
			case OPT_FLOAT_F:
				f_min_func = queue_pop(our_vars.var[i]->queue);
				f_max_func = queue_pop(our_vars.var[i]->queue);
				interval= (float *)queue_pop(our_vars.var[i]->queue);

				label_id= label_add_extended(elconfig_tabs[tab_id].tab, elconfig_free_widget_id++, NULL, elconfig_tabs[tab_id].x, elconfig_tabs[tab_id].y, 0, 1.0, 0.77f, 0.59f, 0.39f, (char*)our_vars.var[i]->display.str);

				widget_id= spinbutton_add(elconfig_tabs[tab_id].tab, NULL, elconfig_menu_x_len/4*3, elconfig_tabs[tab_id].y, 100, 20, SPIN_FLOAT, our_vars.var[i]->var, (*f_min_func)(), (*f_max_func)(), *interval);
				widget_set_OnKey(elconfig_tabs[tab_id].tab, widget_id, spinbutton_onkey_handler);
				widget_set_OnClick(elconfig_tabs[tab_id].tab, widget_id, spinbutton_onclick_handler);
				free(f_min_func);
				free(f_max_func);
				free(interval);
				queue_destroy(our_vars.var[i]->queue);
				our_vars.var[i]->queue= NULL;
			break;
			case OPT_INT_F:
				i_min_func = queue_pop(our_vars.var[i]->queue);
				i_max_func = queue_pop(our_vars.var[i]->queue);
				/* interval is always 1 */

				label_id= label_add_extended(elconfig_tabs[tab_id].tab, elconfig_free_widget_id++, NULL, elconfig_tabs[tab_id].x, elconfig_tabs[tab_id].y, 0, 1.0, 0.77f, 0.59f, 0.39f, (char*)our_vars.var[i]->display.str);
				widget_id= spinbutton_add(elconfig_tabs[tab_id].tab, NULL, elconfig_menu_x_len/4*3, elconfig_tabs[tab_id].y, 100, 20, SPIN_INT, our_vars.var[i]->var, (*i_min_func)(), (*i_max_func)(), 1.0);
				widget_set_OnKey(elconfig_tabs[tab_id].tab, widget_id, spinbutton_onkey_handler);
				widget_set_OnClick(elconfig_tabs[tab_id].tab, widget_id, spinbutton_onclick_handler);
				free(i_min_func);
				free(i_max_func);
				queue_destroy(our_vars.var[i]->queue);
				our_vars.var[i]->queue= NULL;
			break;
			case OPT_MULTI_H:

				label_id= label_add_extended(elconfig_tabs[tab_id].tab, elconfig_free_widget_id++, NULL, elconfig_tabs[tab_id].x, elconfig_tabs[tab_id].y, 0, 1.0, 0.77f, 0.59f, 0.39f, (char*)our_vars.var[i]->display.str);
				widget_id= multiselect_add_extended(elconfig_tabs[tab_id].tab, elconfig_free_widget_id++, NULL, elconfig_tabs[tab_id].x+SPACING+get_string_width(our_vars.var[i]->display.str), elconfig_tabs[tab_id].y, 350, 80, 1.0f, 0.77f, 0.59f, 0.39f, 0.32f, 0.23f, 0.15f, 0);
				x = 0;
				for(y= 0; !queue_isempty(our_vars.var[i]->queue); y++) {
					char *label= queue_pop(our_vars.var[i]->queue);

					int radius = BUTTONRADIUS;
					float width_ratio = DEFAULT_FONT_X_LEN/12.0f;
					int width=0;
	
					width = 2 * radius+(get_string_width((unsigned char*)label)*width_ratio);

					multiselect_button_add_extended(elconfig_tabs[tab_id].tab, widget_id, x, 0, width, label, DEFAULT_SMALL_RATIO, y == *(int *)our_vars.var[i]->var);
					if (strlen(label) == 0)
					{
						y--;
					}
					else
					{
						x += width + SPACING;
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
		if (!force_elconfig_win_ontop && !windows_on_top) {
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
		int i;

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
		/* Pass ELW_SCROLLABLE as the final argument to tab_add() if you want
		 * to put more widgets in the tab than the size of the window allows.*/
		elconfig_tabs[CONTROLS].tab= tab_add(elconfig_win, elconfig_tab_collection_id, ttab_controls, 0, 0, ELW_SCROLLABLE);
		elconfig_tabs[HUD].tab= tab_add(elconfig_win, elconfig_tab_collection_id, ttab_hud, 0, 0, ELW_SCROLLABLE);
		elconfig_tabs[CHAT].tab= tab_add(elconfig_win, elconfig_tab_collection_id, ttab_chat, 0, 0, ELW_SCROLLABLE);
		elconfig_tabs[FONT].tab= tab_add(elconfig_win, elconfig_tab_collection_id, ttab_font, 0, 0, ELW_SCROLLABLE);
		elconfig_tabs[SERVER].tab= tab_add(elconfig_win, elconfig_tab_collection_id, ttab_server, 0, 0, ELW_SCROLLABLE);
		elconfig_tabs[AUDIO].tab= tab_add(elconfig_win, elconfig_tab_collection_id, ttab_audio, 0, 0, ELW_SCROLLABLE);
		elconfig_tabs[VIDEO].tab= tab_add(elconfig_win, elconfig_tab_collection_id, ttab_video, 0, 0, ELW_SCROLLABLE);
		elconfig_tabs[GFX].tab= tab_add(elconfig_win, elconfig_tab_collection_id, ttab_gfx, 0, 0, ELW_SCROLLABLE);
		elconfig_tabs[CAMERA].tab= tab_add(elconfig_win, elconfig_tab_collection_id, ttab_camera, 0, 0, ELW_SCROLLABLE);
		elconfig_tabs[TROUBLESHOOT].tab= tab_add(elconfig_win, elconfig_tab_collection_id, ttab_troubleshoot, 0, 0, ELW_SCROLLABLE);
#ifdef DEBUG
		elconfig_tabs[DEBUGTAB].tab= tab_add(elconfig_win, elconfig_tab_collection_id, "Debug", 0, 0, ELW_SCROLLABLE);
#endif
		elconfig_populate_tabs();

		/* configure scrolling for tabs */
		for (i=0; i<MAX_TABS; i++) {
			/* configure scrolling for any tabs that exceed the window length */
			if(elconfig_tabs[i].y > (widget_get_height(elconfig_win, elconfig_tab_collection_id)-TAB_TAG_HEIGHT)) {
				set_window_scroll_len(elconfig_tabs[i].tab, elconfig_tabs[i].y-widget_get_height(elconfig_win, elconfig_tab_collection_id)+TAB_TAG_HEIGHT);
				set_window_scroll_inc(elconfig_tabs[i].tab, TAB_TAG_HEIGHT);
			}
			/* otherwise disable scrolling */
			else {
				set_window_scroll_inc(elconfig_tabs[i].tab, 0);
				widget_set_flags(elconfig_tabs[i].tab, windows_list.window[elconfig_tabs[i].tab].scroll_id, WIDGET_DISABLED);
			}
		}
	}
	show_window(elconfig_win);
	select_window(elconfig_win);
}
#endif //ELC
