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
 #include "context_menu.h"
 #include "counters.h"
 #include "cursors.h"
 #include "dialogues.h"
 #include "draw_scene.h"
 #include "emotes.h"
 #include "errors.h"
 #include "elwindows.h"
 #include "filter.h"
 #include "gamewin.h"
 #include "gl_init.h"
 #include "hud.h"
 #include "hud_statsbar_window.h"
 #include "hud_indicators.h"
 #include "hud_misc_window.h"
 #include "hud_quickbar_window.h"
 #include "hud_quickspells_window.h"
 #include "icon_window.h"
 #include "init.h"
 #include "interface.h"
 #include "items.h"
 #include "item_info.h"
#ifdef JSON_FILES
 #include "json_io.h"
#endif
 #include "loginwin.h"
 #include "manufacture.h"
 #include "map.h"
 #include "mapwin.h"
 #include "missiles.h"
 #include "multiplayer.h"
 #include "new_character.h"
 #include "openingwin.h"
 #include "particles.h"
 #include "password_manager.h"
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
#ifndef MAP_EDITOR
#include "text.h"
#include "consolewin.h"
#endif // !MAP_EDITOR
#include "url.h"
#if !defined(MAP_EDITOR)
#include "widgets.h"
#endif

#include "eye_candy_wrapper.h"
#include "sendvideoinfo.h"
#ifndef MAP_EDITOR
#include "actor_init.h"
#endif
#include "io/elpathwrapper.h"
#include "textures.h"
#ifdef	FSAA
 #include "fsaa/fsaa.h"
#endif	/* FSAA */
#ifdef	CUSTOM_UPDATE
 #include "custom_update.h"
#endif	/* CUSTOM_UPDATE */

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

const char * ini_filename = "el.ini";

#ifdef ELC
static int CHECKBOX_SIZE = 0;
static int SPACING = 0;			//Space between widgets and labels and lines
#define MAX_LONG_DESC_LINES	3	//How many lines of text we can fit in LONG_DESC_SPACE
static int LONG_DESC_SPACE = 0;	//Space to give to the long descriptions
static int TAB_TAG_HEIGHT = 0;	// the height of the tab at the top of the window
// The config window is special, it is scaled on creation then frozen.
// This is because its too complex to resize and cannot simpely be destroyed and re-created.
static float elconf_scale = 0;
static float elconf_custom_scale = 1.0f;
static int recheck_window_scale = 0;
#define ELCONFIG_SCALED_VALUE(BASE) ((int)(0.5 + ((BASE) * elconf_scale)))
#endif

typedef char input_line[256];

/*!
 * Structure describing an option in a multi-elect widget. It contains the
 * label, which is the text on the button presented to the user, and optionally a
 * value. Using the value, options can also be sought by value rather than
 * by index alone, and hence we don't have to rely on a particular ordering
 * of elements, or on all elements being present.
 */
typedef struct
{
	//! The user-visible lable for the option
	const char* label;
	//! The optional value associated with this option
	const char* value;
} multi_element;

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
	float	default_val; /*!< the default value before the config file is read */
	float	config_file_val; /*!< the value after the config file is read */
	int 	len; /*!< length of the variable */
	int 	in_ini_file; /*!< true if the var was present when we read the ini file */
#ifdef JSON_FILES
	int 	character_override;  /*!< true if the var is in the list maintained per character, requires JSON file support */
#endif
	int	saved;
//	char 	*message; /*!< In case you want a message to be written when a setting is changed */
	dichar display;
	struct {
		int tab_id; /*!< The tab ID in which we find this option */
		int label_id; /*!< The label ID associated with this option */
		int widget_id; /*!< Widget ID for things like checkboxes */
	} widgets;
	union
	{
		struct { int min; int max; } imm;
		struct { int (*min)(); int (*max)(); } immf;
		struct { float min; float max; float interval; } fmmi;
		struct { float (*min)(); float (*max)(); float interval; } fmmif;
		struct { multi_element *elems; size_t count; } multi;
	} args; /*!< The various versions of additional arguments used by configuration variables  */
} var_struct;

/*!
 * a list of variables of type \see var_struct
 */
struct variables
{
	int no; /*!< current number of allocated \see var_struct in \a var */
	var_struct * * var; /*!< fixed array of \a no \see var_struct structures */
} our_vars= {0,NULL};

/*!
 * For some multi-select widgets (currently only the font selections, it seems),
 * not all possible options are available at the time the ini file is read. Rather than
 * blindly assuming the option will later be added, setting these options is
 * deferred to a later stage after the widget has been fully initialized.
 */
typedef struct
{
	//! The index of the multiselect variable in \see our_vars.
	int var_idx;
	//! The previously stored index of the selected option.
	int opt_idx;
	//! The previously stored value of the selected option, or NULL if not present.
	char* value;
} deferred_option;

//! The list of multi-select options that still need to be set
static deferred_option *defers = NULL;
//! The size of the \see defers array
static int defers_size = 0;
//! The number of elements currently used in the \see defers array
static int defers_count = 0;
//! Whether to still defer options
static int do_defer = 1;


int write_ini_on_exit= 1;
// Window Handling
int force_elconfig_win_ontop = 0;
#ifdef ELC
static int elconfig_tab_collection_id= 1;
static int elconfig_free_widget_id= 2;
static unsigned char elconf_description_buffer[400]= {0};
static int last_description_idx = -1;
#endif
struct {
	Uint32	tab;
	Uint16	x;
	Uint16	y;
} elconfig_tabs[MAX_TABS];

int windows_on_top= 0;
static int options_set= 0;
#ifdef ELC
static int delay_poor_man = 1;
static int shadow_map_size_multi= 0;
#endif
#ifdef	FSAA
static int fsaa_index = 0;
#endif	/* FSAA */

static float ui_scale = 1.0;
float get_global_scale(void) { return ui_scale; }

int you_sit= 0;
int sit_lock= 0;
int use_keypress_dialogue_boxes = 0, use_full_dialogue_window = 0;
int use_alpha_banner = 0;
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
int emulate3buttonmouse=0;
int square_buttons = 0;
#endif

int buddy_log_notice=1;
int video_mode_set=0;
int video_info_sent = 0;
int no_adjust_shadows=0;
int clouds_shadows=1;
int item_window_on_drop=1;
int mouse_limit=15;
int isometric=1;
int poor_man=0;
int max_fps = 0, limit_fps=0;
int special_effects=0;
char lang[10] = "en";
int auto_update= 1;
int clear_mod_keys_on_focus=1;

#ifdef  CUSTOM_UPDATE
int custom_update= 1;
int custom_clothing= 1;
#endif  //CUSTOM_UPDATE

#ifdef DEBUG
 int enable_client_aiming = 0;
#endif // DEBUG

#ifdef ANTI_ALIAS
int anti_alias=0;
#endif  //ANTI_ALIAS

#ifdef ELC
static int small_actor_texture_cache = 0;
static void consolidate_rotate_chat_log_status(void);
static int elconfig_menu_x_len= 0;
static int elconfig_menu_y_len= 0;
static int is_mouse_over_option = 0;

static int disable_auto_highdpi_scale = 0;
static int delay_update_highdpi_auto_scaling = 0;
// the elconfig local version of the font sizes, so we can auto scale if needed
static float local_ui_scale = 1.0f;
static float local_minimap_size_coefficient = 0.7f;
static size_t local_encyclopedia_font = 0;

#ifdef JSON_FILES
// Most of this code can be removed when json file use if the default of the only supported client

static int find_var (const char *str, var_name_type type);
static int use_json_user_files = 0;
static int ready_for_user_files = 0;

// Set when we initially login, before calling load functions
void set_ready_for_user_files(void)
{
	ready_for_user_files = 1;
}

// Returns true if we are using json files.
int get_use_json_user_files(void)
{
	static int first_time = 1;

	// If this is the first time we have run with the use_json_user_files
	// option then check if we should switch it on.  The option is false
	// by default.
	if (first_time && !use_json_user_files)
	{
		int i = find_var("use_json_user_files_v1", INI_FILE_VAR);

		first_time = 0;

		// should always be fine as add_var() done earlier
		if (i < 0)
			return 0;

		// if the option was not in the read ini file, check for previous use of json files or a clean install
		if (!our_vars.var[i]->in_ini_file)
		{
			char filename[256];
			struct stat json_file_stat, bin_file_stat;
			int json_stat_ret, bin_stat_ret;

			// Use the counters file as not replaceable and is always saved
			safe_snprintf(filename, sizeof(filename), "%scounters_%s.json", get_path_config(), get_lowercase_username());
			json_stat_ret = stat(filename, &json_file_stat);
			safe_snprintf(filename, sizeof(filename), "%scounters_%s.dat", get_path_config(), get_lowercase_username());
			bin_stat_ret = stat(filename, &bin_file_stat);

			// No bin file so switch on json usage even if no json file either - prevous or clean install
			if (bin_stat_ret == -1)
			{
				use_json_user_files = 1;
				USE_JSON_DEBUG("Setting use_json_user_files as no binary file exists");
			}
			// If there are both json and binary files, pick the most recently modified or the json if the same
			else if ((json_stat_ret == 0) && (json_file_stat.st_mtime >= bin_file_stat.st_mtime))
			{
				use_json_user_files = 1;
				USE_JSON_DEBUG("Setting use_json_user_files as json file newer")
			}
			else // only binary files to leave option disabled
			{
				USE_JSON_DEBUG("Leaving set to binary");
			}

			// Make sure the option is saved to file, to avoid having to check again next run
			set_var_unsaved("use_json_user_files_v1", INI_FILE_VAR);
		}
	}

	return use_json_user_files;
}
#endif // JSON_FILES

#endif // ELC

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

#ifdef ELC
static int int_zero_func(void)
{
	return 0;
}
#endif

static __inline__ void check_option_var(const char* name);

static __inline__ void destroy_shadow_mapping(void)
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

static __inline__ void destroy_fbos(void)
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

static __inline__ void build_fbos(void)
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

static __inline__ void update_fbos(void)
{
	destroy_fbos();
	build_fbos();
}

static void change_var(int * var)
{
	*var= !*var;
}

#ifndef MAP_EDITOR
static void change_cursor_scale_factor(int * var, int value)
{
	// check the range, an invalid setting in the ini file bypasses the bound checking of that var
	if ((value > 0) && (value <= max_cursor_scale_factor))
	{
		*var= value;
		if (current_cursor >= 0)
		{
			build_cursors();
			change_cursor(current_cursor);
		}
	}
}

static void change_show_action_bar(int * var)
{
	*var= !*var;
	if (stats_bar_win >= 0)
		init_stats_display();
}

static void change_minimap_scale(float * var, float * value)
{
	int minimap_win = get_id_MW(MW_MINIMAP);
	int shown = 0;
	float last_minimap_size_coefficient = minimap_size_coefficient;
	*var= *value;
	minimap_size_coefficient = ((disable_auto_highdpi_scale)) ? *var : get_highdpi_scale() * *var;
	if (last_minimap_size_coefficient != minimap_size_coefficient && minimap_win >= 0)
	{
		shown = get_show_window(minimap_win);
		set_pos_MW(MW_MINIMAP, windows_list.window[minimap_win].cur_x, windows_list.window[minimap_win].cur_y);
		destroy_window(minimap_win);
		set_id_MW(MW_MINIMAP, -1);
	}
	if (shown)
		display_minimap();
}

static void change_sky_var(int * var)
{
	*var= !*var;
	skybox_update_colors();
}

static void change_use_animation_program(int * var)
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
static void change_min_ec_framerate(float * var, float * value)
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

static void change_max_ec_framerate(float * var, float * value)
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

static void change_int(int * var, int value)
{
	if(value>=0) *var= value;
}

#ifdef ELC

static void change_float(float * var, float * value)
{
	*var= *value;
}

static void change_string(char* var, const char* str, int len)
{
	safe_strncpy(var, str, len);
}

static void change_ui_scale(float *var, float *value)
{
	*var= *value;
	ui_scale = ((disable_auto_highdpi_scale)) ? *var : get_highdpi_scale() * *var;
	HUD_MARGIN_X = (int)ceilf(ui_scale * 64.0);
	if (hud_x != 0)
		hud_x = HUD_MARGIN_X;
	HUD_MARGIN_Y = (int)ceilf(ui_scale * 49.0);
	if (hud_y != 0)
		hud_y = HUD_MARGIN_Y;

	update_windows_scale(ui_scale);

	if (input_widget != NULL)
		input_widget_move_to_win(input_widget->window_id);
}

static void change_elconf_win_scale_factor(float *var, float *value)
{
	*var= *value;
	if (get_id_MW(MW_CONFIG) >= 0)
		recheck_window_scale = 1;
}

static void change_win_scale_factor(float *var, float *value)
{
	*var= *value;
	update_windows_custom_scale(var);
}

static const float win_scale_min = 0.25f;
static const float win_scale_max = 3.0f;
static const float win_scale_step = 0.01f;

static var_struct * find_win_scale_factor(float *changed_window_custom_scale)
{
	if (changed_window_custom_scale != NULL)
	{
		size_t i;
		for (i = 0; i < our_vars.no; i++)
			if (our_vars.var[i]->var == changed_window_custom_scale)
				return our_vars.var[i];
	}
	return NULL;
}

void step_win_scale_factor(int increase, float *changed_window_custom_scale)
{
	if (changed_window_custom_scale != NULL)
	{
		var_struct * var = find_win_scale_factor(changed_window_custom_scale);
		float new_value = *changed_window_custom_scale + ((increase) ? win_scale_step : -win_scale_step);
		if (new_value >= win_scale_min && new_value <= win_scale_max)
		{
			*changed_window_custom_scale = new_value;
			update_windows_custom_scale(changed_window_custom_scale);
		}
		if (var != NULL)
			var->saved = 0;
	}
}

void limit_win_scale_to_default(float *changed_window_custom_scale)
{
	var_struct * var = find_win_scale_factor(changed_window_custom_scale);
	if ((var != NULL) && (*changed_window_custom_scale > var->default_val))
	{
		*changed_window_custom_scale = var->default_val;
		update_windows_custom_scale(changed_window_custom_scale);
		var->saved = 0;
	}
}

void reset_win_scale_factor(int set_default, float *changed_window_custom_scale)
{
	var_struct * var = find_win_scale_factor(changed_window_custom_scale);
	if (var != NULL)
	{
		*changed_window_custom_scale = (set_default) ? var->default_val : var->config_file_val;
		update_windows_custom_scale(changed_window_custom_scale);
		var->saved = 0;
	}
}

/*
 * The chat logs are created very early on in the client start up, before the
 * ini file is read at least. Because of this, a simple ini file variable
 * can not be used to enabled/disable rotating chat log files. I suppose the init
 * code could be changed but rather do that and unleash all kinds of grief, use a
 * simple flag file when the chat logs are opened.  The ini file setting now
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

/* create or delete the flag file to reflect the ini file rotate chat log value */
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
	/* it is too late to use a newly set rotate log value, but we can set the ini flag if rotating is on */
	if ((rotate_chat_log==1) && !rotate_chat_log_config_var)
	{
		rotate_chat_log_config_var = 1;
		set_var_unsaved("rotate_chat_log", INI_FILE_VAR);
	}
}

#ifdef NEW_SOUND
static void change_sound_level(float *var, float * value)
{
	if(*value >= 0.0f && *value <= 1.0f+0.00001) {
		*var= (float)*value;
	} else {
		*var=0;
	}
}
#endif

static void update_max_actor_texture_handles(void)
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

static void change_compiled_vertex_array(int *value)
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

static void change_vertex_buffers(int *value)
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

static void change_clouds_shadows(int *value)
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

static void change_small_actor_texture_cache(int *value)
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

static void change_eye_candy(int *value)
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

static void change_point_particles(int *value)
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
}

static void change_fps(int * var, int value)
{
	if(value>=0) max_fps = *var = value;
}

static void change_particles_percentage(int *pointer, int value)
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

static void change_new_selection(int *value)
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

static void switch_vidmode(int *pointer, int mode)
{
	if(!video_mode_set) {
		/* Video isn't ready yet, just remember the mode */
		video_mode= mode;
	} else {
		full_screen = 0;
		switch_video(mode, full_screen);
	}
}

static void toggle_full_screen_mode(int * fs)
{
	if(!video_mode_set) {
		*fs= !*fs;
	} else {
		toggle_full_screen();
	}
}

#ifdef NEW_CURSOR
static void change_sdl_cursor(int * fs)
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

static void toggle_follow_cam_behind(int * fc)
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

static void change_tilt_float(float * var, float * value)
{
	*var= *value;
    if (rx > -min_tilt_angle) rx = -min_tilt_angle;
    else if (rx < -max_tilt_angle) rx = -max_tilt_angle;
}

static void change_shadow_map_size(int *pointer, int value)
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
static void change_fsaa(int *pointer, int value)
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
}
#endif	/* FSAA */

#ifdef CUSTOM_UPDATE
static void change_custom_update(int *var)
{
	*var = !*var;

	if (*var)
	{
		start_custom_update();
	}
}

static void change_custom_clothing(int *var)
{
	*var = !*var;
	unload_actor_texture_cache();
}
#endif    //CUSTOM_UPDATE

#ifndef MAP_EDITOR2
static void set_afk_time(int *pointer, int time)
{
	if(time > 0) {
		afk_time= time*60000;
		*pointer= time;
	} else {
		afk_time= 0;
		*pointer= 0;
	}
}

static void set_buff_icon_size(int *pointer, int value)
{
	/* The value is actually set in the widget code so attempting so controlling the
		range here does not work.  Instead, use the built in max/min code of the widget.
		We still need to set the value here for the initial read from the config file. */
	*pointer = value;
	/* Turn off icons when the size is zero (or at least low). */
	view_buffs = (value < 5) ?0: 1;
}

#ifdef JSON_FILES
static void change_use_json_user_files(int *var)
{
	*var= !*var;
	if (ready_for_user_files)
	{
		if (*var) // character options have no non-json equlivavnt
			load_character_options();
		save_local_data();
		LOG_TO_CONSOLE(c_green1, local_only_save_str);
	}
	else
		USE_JSON_DEBUG("Not ready for user files");
}
#endif

static void change_dark_channeltext(int *dct, int value)
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
	else if (get_id_MW(MW_CHAT) >= 0)
	{
		hide_window (get_id_MW(MW_CHAT));
	}

	if (old_wc != *wc && (old_wc == 1 || old_wc == 2) )
	{
		convert_tabs (*wc);
	}

	enable_chat_shown();
}

static void change_enable_chat_show_hide(int * var)
{
	*var= !*var;
	enable_chat_shown();
}

static void change_max_chat_lines(int * var, int value)
{
	if(value>=0) *var= value;
	enable_chat_shown();
}

static void change_quickbar_relocatable (int *rel)
{
	*rel= !*rel;
	if (get_id_MW(MW_QUICKBAR) >= 0)
	{
		init_quickbar ();
	}
}

static void change_quickspells_relocatable (int *rel)
{
	*rel= !*rel;
	if (get_id_MW(MW_QUICKSPELLS) >= 0)
	{
		init_quickspell ();
	}
}

static void change_font(size_t *var, int value)
{
	if (value >= 0)
	{
		*var = value;
		// Yes, this is ugly. I just really did not want to introduce a ton of
		// separate function for each font category.
		if (var >= font_idxs && var < font_idxs + NR_FONT_CATS)
		{
			font_cat cat = var - font_idxs;
			change_windows_font(cat);
			if (cat == UI_FONT)
			{
				// The mapmark font uses the same font as the user interface, but has its own
				/// scale factor
				font_idxs[MAPMARK_FONT] = value;
				change_windows_font(MAPMARK_FONT);
			}
		}
		else if (var == &local_encyclopedia_font)
		{
			font_idxs[ENCYCLOPEDIA_FONT] = get_fixed_width_font_number(value);
			change_windows_font(ENCYCLOPEDIA_FONT);
		}
	}
}

static void change_text_zoom(float *var, const float *value)
{
	float val = *value;
	if (val > 0.0)
	{
		if (val < 0.1)
			val = 0.1;

		*var = (disable_auto_highdpi_scale) ? val : get_highdpi_scale() * val;
		// Yes, this is ugly. I just really did not want to introduce a ton of
		// separate function for each font category.
		if (var >= font_scales && var < font_scales + NR_FONT_CATS)
		{
			font_cat cat = var - font_scales;
			change_windows_font(cat);
		}
	}
}

static void change_chat_zoom(float *var, float *value)
{
	if (*value < 0.0f)
		return;

	*var = (disable_auto_highdpi_scale) ? *value : get_highdpi_scale() * *value;
	change_windows_font(CHAT_FONT);
	// FIXME?
	if (input_widget != NULL)
	{
		text_field *tf= input_widget->widget_info;
		if (use_windowed_chat != 2)
		{
			int text_height = get_text_height(tf->nr_lines, CHAT_FONT, input_widget->size);
			widget_resize(input_widget->window_id, input_widget->id,
				input_widget->len_x, tf->y_space*2 + text_height);
		}
	}
}

#ifdef TTF
static void change_use_ttf(int *var)
{
	*var = !*var;
	if (*var)
		enable_ttf();
	else
		disable_ttf();
}
#endif

void update_highdpi_auto_scaling(void)
{
	change_text_zoom(&font_scales[UI_FONT], &font_scales[UI_FONT]);
	change_text_zoom(&font_scales[NAME_FONT], &font_scales[NAME_FONT]);
	change_chat_zoom(&font_scales[CHAT_FONT], &font_scales[CHAT_FONT]);
	change_text_zoom(&font_scales[NOTE_FONT], &font_scales[NOTE_FONT]);
	change_text_zoom(&font_scales[BOOK_FONT], &font_scales[BOOK_FONT]);
	change_text_zoom(&font_scales[RULES_FONT], &font_scales[RULES_FONT]);
	change_text_zoom(&font_scales[ENCYCLOPEDIA_FONT], &font_scales[ENCYCLOPEDIA_FONT]);
	change_ui_scale(&local_ui_scale, &local_ui_scale);
	change_minimap_scale(&local_minimap_size_coefficient, &local_minimap_size_coefficient);
}

static void change_disable_auto_highdpi_scale(int * var)
{
	*var= !*var;
	if (!delay_update_highdpi_auto_scaling)
		update_highdpi_auto_scaling();
}

#endif // MAP_EDITOR2
#endif // def ELC

static void change_dir_name (char *var, const char *str, int len)
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
static void change_aa(int *pointer) {
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
static void change_projection_float_init(float * var, float * value) {
	change_float(var, value);
}

static void change_projection_bool_init(int *pointer) {
	change_var(pointer);
}
#endif //OSX
static void change_projection_float(float * var, float * value) {
	change_float(var, value);
	if (video_mode_set)
	{
		resize_root_window ();
		set_all_intersect_update_needed (main_bbox_tree);
	}
}

static void change_projection_bool(int *pointer) {
	change_var(pointer);
	if (video_mode_set)
	{
		resize_root_window ();
		if (pointer == &isometric && isometric) ext_cam = 0;
		set_all_intersect_update_needed (main_bbox_tree);
	}
}

static void change_gamma(float *pointer, float *value)
{
	*pointer= *value;
	if(video_mode_set && !disable_gamma_adjust) {
		SDL_SetWindowBrightness(el_gl_window, *value);
	}
}

#ifndef MAP_EDITOR2
void change_windows_on_top(int *var)
{
	enum managed_window_enum i;
	*var=!*var;

	if (*var)
	{
		for (i = 0; i < MW_MAX; i++)
		{
			int win_id = get_id_MW(i);
			if (on_top_responsive_MW(i) && (win_id >= 0) && (win_id < windows_list.num_windows))
			{
				window_info *win = &windows_list.window[win_id];
				/* Change the root windows */
				move_window(win_id, -1, 0, win->pos_x, win->pos_y );
				/* Display any open windows */
				if (win->displayed != 0 || win->reinstate != 0)
					show_window(win_id);
			}
		}
	}
	else
	{
		// Change the root windows
		for (i = 0; i < MW_MAX; i++)
		{
			int win_id = get_id_MW(i);
			if (on_top_responsive_MW(i) && (win_id >= 0) && (win_id < windows_list.num_windows))
			{
				window_info *win = &windows_list.window[win_id];
				move_window(win_id, game_root_win, 0, win->pos_x, win->pos_y );
			}
		}
		// Hide all the windows if needed
		if (windows_list.window[game_root_win].displayed == 0)
			hide_window(game_root_win);
	}
}
#endif

#ifndef MAP_EDITOR2
static void change_separate_flag(int * pointer) {
	change_var(pointer);

	if (get_id_MW(MW_CHAT) >= 0) {
		update_chat_win_buffers();
	}
}
#endif

static void change_shadow_mapping (int *sm)
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
static void change_global_filters (int *use)
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

#ifndef MAP_EDITOR
static void change_reflection(int *rf)
{
	*rf= !*rf;
	update_fbos();
}

static void change_frame_buffer(int *fb)
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

#ifndef MAP_EDITOR
static void change_shadows(int *sh)
{
	*sh= !*sh;
	update_fbos();
}

static int int_max_water_shader_quality(void)
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

static void change_water_shader_quality(int *wsq, int value)
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

static void set_auto_save_interval (int *save_time, int time)
{
	if(time>0) {
		*save_time= time*60000;
	} else {
		*save_time= 0;
	}
}

static void switch_vidmode(int *pointer, int mode)
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
				return;
			case 7:
				window_width=1280;
				window_height=800;
			default:
				return;
		}
}

#endif

static int find_var(const char *str, var_name_type type)
{
	size_t i, len_to_check;

	len_to_check = strcspn(str, " =");
	for (i = 0; i < our_vars.no; i++)
	{
		const var_struct *var = our_vars.var[i];
		const char *var_name = (type != COMMAND_LINE_SHORT_VAR) ? var->name : var->shortname;

		if (strncmp(str, var_name, len_to_check) == 0 && var_name[len_to_check] == '\0')
			return i;
	}

	return -1;
}

const char *get_option_description(const char *str, var_name_type type)
{
	int var_index = find_var(str, type);
	if (var_index == -1)
	{
		LOG_ERROR("Can't find var '%s', type %d", str, type);
		return NULL;
	}
	return (const char *)our_vars.var[var_index]->display.desc;
}

int set_var_unsaved(const char *str, var_name_type type)
{
	int var_index = find_var(str, type);
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
	int var_index = find_var(str, INI_FILE_VAR);
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
// find an OPT_INT ot OPT_INT_F widget and set its's value
int set_var_OPT_INT(const char *str, int new_value)
{
	int var_index = find_var(str, INI_FILE_VAR);

	if ((var_index != -1) && ((our_vars.var[var_index]->type == OPT_INT) || (our_vars.var[var_index]->type == OPT_INT_F)))
	{
		int tab_win_id = elconfig_tabs[our_vars.var[var_index]->widgets.tab_id].tab;
		int widget_id = our_vars.var[var_index]->widgets.widget_id;
		// This bit belongs in the widgets module
		widget_list *widget = widget_find(tab_win_id, widget_id);
		our_vars.var[var_index]->func(our_vars.var[var_index]->var, new_value);
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

// find an OPT_FLOAT widget and set its's value
static int set_var_OPT_FLOAT(const char *str, float new_value)
{
	int var_index = find_var(str, INI_FILE_VAR);

	if ((var_index != -1) && (our_vars.var[var_index]->type == OPT_FLOAT))
	{
		int tab_win_id = elconfig_tabs[our_vars.var[var_index]->widgets.tab_id].tab;
		int widget_id = our_vars.var[var_index]->widgets.widget_id;
		// This bit belongs in the widgets module
		widget_list *widget = widget_find(tab_win_id, widget_id);
		our_vars.var[var_index]->func(our_vars.var[var_index]->var, &new_value);
		our_vars.var[var_index]->saved = 0;
		if(widget != NULL && widget->widget_info != NULL)
		{
			spinbutton *button = widget->widget_info;
			*(float *)button->data = new_value;
			safe_snprintf(button->input_buffer, sizeof(button->input_buffer), "%.2f", *(float *)button->data);
			return 1;
		}

		return 0;
	}

	LOG_ERROR("Can't find var '%s', type 'OPT_FLOAT'", str);
	return 0;
}

static int set_var_OPT_BOOL(const char *str, int new_value)
{
	int var_index = find_var(str, INI_FILE_VAR);

	if ((var_index != -1) && ((our_vars.var[var_index]->type == OPT_BOOL) || (our_vars.var[var_index]->type == OPT_BOOL_INI)))
	{
		var_struct *option = our_vars.var[var_index];
		if (*(int *)option->var != new_value)
		{
			*(int *)option->var = !new_value;
			option->func(option->var);
			option->saved = 0;
		}
	}

	LOG_ERROR("Can't find var '%s', type 'OPT_BOOL'", str);
	return 0;
}


static int set_var_OPT_MULTI(const char *str, size_t new_value)
{
	int var_index = find_var(str, INI_FILE_VAR);

	if ((var_index != -1) && ((our_vars.var[var_index]->type == OPT_MULTI) || (our_vars.var[var_index]->type == OPT_MULTI_H)))
	{
		var_struct *option = our_vars.var[var_index];
		int window_id = elconfig_tabs[option->widgets.tab_id].tab;
		int widget_id = option->widgets.widget_id;
		size_t max_sel = option->args.multi.count;
		if (new_value >= max_sel)
		{
			LOG_ERROR("Invalid value '%lu' for var '%s', type 'OPT_MULTI*' max '%lu'", new_value, str, max_sel);
			return 0;
		}
		option->func(option->var, new_value);
		option->saved = 0;
		if ((window_id > 0) && (widget_id > 0) && (widget_find(window_id, widget_id) != NULL))
			multiselect_set_selected(window_id, option->widgets.widget_id, new_value);
		return 1;
	}

	LOG_ERROR("Can't find var '%s', type 'OPT_MULTI'", str);
	return 0;
}

static float get_option_initial_value(const char *longname)
{
	int var_index = find_var(longname, COMMAND_LINE_LONG_VAR);
	if (var_index == -1)
	{
		LOG_ERROR("Can't find longname var '%s'", longname);
		return -1;
	}
	return our_vars.var[var_index]->config_file_val;
}

static void action_poor_man(int *poor_man)
{
	unload_texture_cache();
	update_max_actor_texture_handles();
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
		skybox_show_clouds = 0;
		skybox_show_sun = 0;
		skybox_show_moons = 0;
		skybox_show_stars = 0;
		if (far_plane > 50)
		{
			far_plane = 50;
			change_projection_float(&far_plane, &far_plane);
		}
	}
	else
	{
		show_reflection = get_option_initial_value("show_reflection");
		shadows_on= get_option_initial_value("shadows_on");
		clouds_shadows= get_option_initial_value("clouds_shadows");
		use_shadow_mapping= get_option_initial_value("use_shadow_mapping");
#ifndef MAP_EDITOR2
		special_effects= get_option_initial_value("special_effects");
		use_eye_candy = get_option_initial_value("use_eye_candy");
		use_fog= get_option_initial_value("render_fog");
		show_weather = get_option_initial_value("show_weather");
#endif
#ifndef MAP_EDITOR
		use_frame_buffer= get_option_initial_value("use_frame_buffer");
#endif
		skybox_show_clouds = get_option_initial_value("skybox_show_clouds");
		skybox_show_sun = get_option_initial_value("skybox_show_sun");
		skybox_show_moons = get_option_initial_value("skybox_show_moons");
		skybox_show_stars = get_option_initial_value("skybox_show_stars");
		far_plane = get_option_initial_value("far_plane");
		change_projection_float(&far_plane, &far_plane);
	}
	update_fbos();
}

static void change_poor_man(int *poor_man)
{
	*poor_man= !*poor_man;
	if (!delay_poor_man)
		action_poor_man(poor_man);
}

static size_t cm_id = CM_INIT_VALUE;

// set the new value form the label option's context menu
static int context_option_handler(window_info *win, int widget_id, int mx, int my, int menu_option)
{
	float new_value = 0;
	var_struct *option = (var_struct *)cm_get_data(cm_id);
	if (menu_option == 1)
		new_value = option->default_val;
	else if (menu_option == 2)
		new_value = option->config_file_val;
	else
		return 1;

	switch (option->type)
	{
		case OPT_INT:
		case OPT_INT_F:
			set_var_OPT_INT(option->name, (int)new_value);
			break;
		case OPT_MULTI:
		case OPT_MULTI_H:
			set_var_OPT_MULTI(option->name, (size_t)new_value);
			break;
		case OPT_BOOL:
			set_var_OPT_BOOL(option->name, (int)new_value);
			break;
		case OPT_FLOAT:
			set_var_OPT_FLOAT(option->name, new_value);
			break;
		default:
			break;
	}
	return 1;
}

// add a named preset value to the option's label context menu
static int add_cm_option_line(const char *prefix, var_struct *option, float value)
{
	char menu_text[256];
	switch (option->type)
	{
		case OPT_INT:
		case OPT_INT_F:
		case OPT_MULTI:
		case OPT_MULTI_H:
			safe_snprintf(menu_text, sizeof(menu_text), "\n%s: %d\n", prefix, (int)value);
			break;
		case OPT_BOOL:
			safe_snprintf(menu_text, sizeof(menu_text), "\n%s: %s\n", prefix, ((int)value) ?"true": "false");
			break;
		case OPT_FLOAT:
			safe_snprintf(menu_text, sizeof(menu_text), "\n%s: %g\n", prefix, value);
			break;
		default:
			return 0;
	}
	cm_add(cm_id, menu_text, NULL);
	return 1;
}

// create the context menu when we right click an option label
static void call_option_menu(var_struct *option)
{
	if (cm_id == CM_INIT_VALUE)
	{
		cm_id = cm_create(NULL, NULL);
	}
	cm_set_colour(cm_id, CM_GREY, 201.0/256.0, 254.0/256.0, 203.0/256.0);
	cm_set(cm_id, option->name, context_option_handler);
	cm_grey_line(cm_id, 0, 1);
	if (add_cm_option_line(cm_options_default_str, option, option->default_val))
	{
		add_cm_option_line(cm_options_initial_str, option, option->config_file_val);
#ifdef JSON_FILES
		if (get_use_json_user_files() && ready_for_user_files)
		{
			cm_add(cm_id, cm_options_per_character_str, NULL);
			cm_bool_line(cm_id, 3, &option->character_override, NULL);
		}
#endif
		cm_set_data(cm_id, (void *)option);
	}
	cm_show_direct(cm_id, -1, -1);
}

void restore_starting_video_mode(void)
{
	int var_index =find_var("video_width", INI_FILE_VAR);
	if (var_index != -1)
		set_var_OPT_INT(our_vars.var[var_index]->name, (size_t)our_vars.var[var_index]->config_file_val);
	if ((var_index = find_var("video_height", INI_FILE_VAR)) != -1)
		set_var_OPT_INT(our_vars.var[var_index]->name, (size_t)our_vars.var[var_index]->config_file_val);
	if ((var_index = find_var("video_mode", INI_FILE_VAR)) != -1)
		set_var_OPT_MULTI(our_vars.var[var_index]->name, (size_t)our_vars.var[var_index]->config_file_val);
	if ((var_index = find_var("full_screen", INI_FILE_VAR)) != -1)
		set_var_OPT_BOOL(our_vars.var[var_index]->name,  (int)our_vars.var[var_index]->config_file_val);
}

void set_user_defined_video_mode(void)
{
	int var_index = find_var("video_width", INI_FILE_VAR);
	if (var_index != -1)
	{
		set_var_OPT_INT(our_vars.var[var_index]->name, window_width);
		if ((var_index = find_var("video_height", INI_FILE_VAR)) != -1)
		{
			set_var_OPT_INT(our_vars.var[var_index]->name, window_height);
			if ((var_index = find_var("video_mode", INI_FILE_VAR)) != -1)
				set_var_OPT_MULTI(our_vars.var[var_index]->name, 0);
		}
	}
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

static __inline__ void check_option_var(const char* name)
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

void check_options(void)
{
	check_option_var("use_compiled_vertex_array");
	check_option_var("use_vertex_buffers");
	check_option_var("clouds_shadows");
	check_option_var("small_actor_texture_cache");
	check_option_var("use_eye_candy");
	check_option_var("use_point_particles");
	check_option_var("use_frame_buffer");
	check_option_var("use_shadow_mapping");
	check_option_var("shadow_map_size");
	check_option_var("water_shader_quality");
	check_option_var("use_animation_program");
}

/*!
 * Find a multiselect option in variable \a var by value.
 *
 * \return The index of the value, if found, otherwise -1.
 */
static int find_multi_option_by_value(const var_struct *var, const char* value)
{
	int i;

	if (!value)
		return -1;

	for (i = 0; i < var->args.multi.count; ++i)
	{
		const char *opt_value = var->args.multi.elems[i].value;
		if (opt_value && !strcmp(opt_value, value))
			return i;
	}

	return -1;
}

void check_deferred_options()
{
	int i;

	for (i = 0; i < defers_count; ++i)
	{
		deferred_option *option = &defers[i];
		var_struct *var = our_vars.var[option->var_idx];
		const char* opt_val = option->value;
		int opt_idx = option->opt_idx;
		int opt_idx_ok = (opt_idx < var->args.multi.count);

		if (opt_val)
		{
			const char* value = opt_idx_ok ? var->args.multi.elems[opt_idx].value : NULL;
			if (!value || strcmp(value, opt_val))
			{
				int new_idx = find_multi_option_by_value(var, opt_val);
				if (new_idx >= 0)
				{
					opt_idx = new_idx;
					opt_idx_ok = 1;
				}
				else
				{
					opt_idx_ok = 0;
				}
			}
		}

		if (opt_idx_ok)
		{
			var->func(var->var, opt_idx);
			var->config_file_val = (float)opt_idx;
		}
		else
		{
			if (opt_val)
			{
				LOG_ERROR("Failed to find value '%s' in multiselect option '%s'",
					opt_val, var->name);
			}
			else
			{
				LOG_ERROR("Failed to find index %d in multiselect option '%s'",
					opt_idx, var->name);
			}
		}

		free(option->value);
	}

	free(defers);
	defers = NULL;
	defers_size = 0;
	defers_count = 0;

	// Stop further deferrals
	do_defer = 0;
}

/*!
 * Defer setting multi-select variable.
 *
 * Some multi-select variables cannot be reliably set because they are not fully
 * initialized before the ini is read. This function stores the desired option
 * for setting later.
 * \param var_idx The index of the multi-select variable in \see our_vars.
 * \param opt_idx The index of the desired option in the options list of the variable
 * \param value   The value associated with the desired option.
 * \return -1 if the option cannot be stored, 1 otherwise.
 * \sa check_deferred_options()
 */
static int add_deferred_option(int var_idx, int opt_idx, const char* value)
{
	deferred_option *option;

	if (defers_count >= defers_size)
	{
		int new_size = defers_size + 8;
		deferred_option *new_defers = realloc(defers, new_size * sizeof(deferred_option));
		if (!new_defers)
			return -1;

		defers = new_defers;
		defers_size = new_size;
	}

	option = &defers[defers_count];
	option->var_idx = var_idx;
	option->opt_idx = opt_idx;
	option->value = value ? strdup(value) : NULL;
	++defers_count;

	return 1;
}

/*!
 * Set a the value of a multi-select variable.
 *
 * Set the value of the multiselect valuable at index \a var_idx in \see our_vars,
 * to the option described by \a str (a line from the ini file). If the description
 * contains both an index and a value, the value is preferred over the index,
 * and the variable is set to the first option that has the same value. If the
 * value cannot be found, or no value is set and the index is out of range,
 * and \a do_defer is not zero, the desired option is stored for later
 * processing.
 *
 * \param str      The description of the option to set
 * \param var_idx  The index of the multi-select variable in which to select an option
 * \return -1 is setting the option fails, 1 on success
 *
 * \sa check_deferred_options().
 */
static int check_multi_select(const char* str, int var_idx)
{
	var_struct *var = our_vars.var[var_idx];
	int nr_conv, opt_idx, opt_idx_ok;
	char value_buf[256] = { 0 };

	nr_conv = sscanf(str, "%d (%255[^\r\n)])", &opt_idx, value_buf);
	if (nr_conv == 0)
	{
		// Unable to parse the value at all
		return -1;
	}

	if (opt_idx < 0)
	{
		LOG_ERROR("Invalid value %d for '%s'", opt_idx, var->name);
		return -1;
	}
	opt_idx_ok = opt_idx <= var->args.multi.count;

	if (nr_conv == 2)
	{
		// Got a value. If it doesn't match the value at the index, find the option
		// with the correct value and update the index.
		const char* opt_value = (opt_idx < var->args.multi.count)
			? var->args.multi.elems[opt_idx].value : NULL;
		opt_idx_ok = opt_value && !strcmp(opt_value, value_buf);
		if (!opt_idx_ok)
		{
			int new_idx = find_multi_option_by_value(var, value_buf);
			if (new_idx >= 0)
			{
				opt_idx = new_idx;
				opt_idx_ok = 1;
			}
		}
	}

	if (opt_idx_ok)
	{
		var->func(var->var, opt_idx);
		var->config_file_val = (float)opt_idx;
		return 1;
	}

	// The index stored does not fit in the current range of the multiselect,
	// or the value stored with the index does not match the value at this index.
	// If do_defer is true, store the index and value to check again at a later
	// point, otherwise give up.
	if (do_defer)
	{
		char* value = *value_buf ? value_buf : NULL;
		return add_deferred_option(var_idx, opt_idx, value);
	}

	if (*value_buf)
	{
		LOG_ERROR("Failed to find value '%s' in multiselect option '%s'",
			value_buf, var->name);
	}
	else
	{
		LOG_ERROR("Failed to find index %d in multiselect option '%s'",
			opt_idx, var->name);
	}
	return -1;
}

int check_var(char *str, var_name_type type)
{
	int i, *p;
	char *ptr= str;
	float foo;
	input_line our_string;

	i = find_var(str, type);
	if (i < 0)
	{
		LOG_WARNING("Can't find var '%s', type %d", str, type);
		return -1;
	}
	our_vars.var[i]->in_ini_file = 1;

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
		// make sure in-game changes are stored in the ini file
		our_vars.var[i]->saved= 0;
	switch (our_vars.var[i]->type)
	{
		case OPT_INT_INI:
			// Needed, because var is never changed through widget
			our_vars.var[i]->saved= 0;
			// fallthrough
		case OPT_INT:
		case OPT_INT_F:
		{
			int new_val = atoi (ptr);
			our_vars.var[i]->func(our_vars.var[i]->var, new_val);
			our_vars.var[i]->config_file_val = (float)new_val;
			return 1;
		}
		case OPT_MULTI:
		case OPT_MULTI_H:
			return check_multi_select(ptr, i);
		case OPT_BOOL_INI:
			// Needed, because var is never changed through widget
			our_vars.var[i]->saved= 0;
			// fallthrough
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
			our_vars.var[i]->config_file_val = (float)new_val;
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
			our_vars.var[i]->config_file_val = foo;
			return 1;
	}
	return -1;
}

void free_vars(void)
{
	int i;
	for(i= 0; i < our_vars.no; i++)
	{
		switch(our_vars.var[i]->type) {
			case OPT_MULTI:
			case OPT_MULTI_H:
				if (our_vars.var[i]->args.multi.count > 0)
				{
					free(our_vars.var[i]->args.multi.elems);
					our_vars.var[i]->args.multi.count = 0;
				}
				break;
			default:
				/* do nothing */ ;
		}
		free(our_vars.var[i]);
	}
	if (our_vars.var != NULL)
	{
		free(our_vars.var);
		our_vars.var = NULL;
	}
	our_vars.no=0;
}

static void add_multi_option_to_var(var_struct *var, const char* label, const char* value)
{
	// FIXME? reallocating on every addition
	multi_element *new_elems = realloc(var->args.multi.elems, sizeof(multi_element) * (var->args.multi.count + 1));
	if (!new_elems)
	{
		LOG_ERROR("Failed to reallocate elements for variable '%s'", var->name);
		return;
	}

	new_elems[var->args.multi.count].label = label;
	new_elems[var->args.multi.count].value = value;
	var->args.multi.elems = new_elems;
	var->args.multi.count++;
}

static void add_var(option_type type, char * name, char * shortname, void * var, void * func, float def, char * short_desc, char * long_desc, int tab_id, ...)
{
	int *integer=var;
	float *f=var;
	int no=our_vars.no;
	const char *pointer;
	va_list ap;

	our_vars.var = realloc(our_vars.var, ++our_vars.no * sizeof(var_struct *));

	our_vars.var[no]=(var_struct*)calloc(1,sizeof(var_struct));
	switch(our_vars.var[no]->type=type)
	{
		case OPT_MULTI:
		case OPT_MULTI_H:
			our_vars.var[no]->args.multi.elems = NULL;
			our_vars.var[no]->args.multi.count = 0;
			va_start(ap, tab_id);
			while ((pointer = va_arg(ap, const char *)) != NULL)
			{
				add_multi_option_to_var(our_vars.var[no], pointer, NULL);
			}
			va_end(ap);
			*integer= (int)def;
		break;
		case OPT_INT:
		case OPT_INT_INI:
			va_start(ap, tab_id);
			our_vars.var[no]->args.imm.min = va_arg(ap, uintptr_t);
			our_vars.var[no]->args.imm.max = va_arg(ap, uintptr_t);
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
			va_start(ap, tab_id);
			our_vars.var[no]->args.fmmi.min = va_arg(ap, double);
			our_vars.var[no]->args.fmmi.max = va_arg(ap, double);
			our_vars.var[no]->args.fmmi.interval = va_arg(ap, double);
			va_end(ap);
			*f=def;
			break;
		case OPT_FLOAT_F:
			va_start(ap, tab_id);
			our_vars.var[no]->args.fmmif.min = va_arg(ap, float (*)());
			our_vars.var[no]->args.fmmif.max = va_arg(ap, float (*)());
			our_vars.var[no]->args.fmmif.interval = va_arg(ap, double);
			va_end(ap);
			*f = def;
			break;
		case OPT_INT_F:
			va_start(ap, tab_id);
			our_vars.var[no]->args.immf.min = va_arg(ap, int (*)());
			our_vars.var[no]->args.immf.max = va_arg(ap, int (*)());
			va_end(ap);
			*integer = (int)def;
			break;
	}
	our_vars.var[no]->var=var;
	our_vars.var[no]->config_file_val = our_vars.var[no]->default_val = def;
	our_vars.var[no]->func=func;
	our_vars.var[no]->name=name;
	our_vars.var[no]->shortname=shortname;
	our_vars.var[no]->nlen=strlen(our_vars.var[no]->name);
	our_vars.var[no]->snlen=strlen(our_vars.var[no]->shortname);
	our_vars.var[no]->saved= 0;
	our_vars.var[no]->in_ini_file = 0;
#ifdef JSON_FILES
	our_vars.var[no]->character_override = 0;
#endif
#ifdef ELC
	add_options_distringid(name, &our_vars.var[no]->display, short_desc, long_desc);
#endif //ELC
	our_vars.var[no]->widgets.tab_id= tab_id;
}

#ifndef MAP_EDITOR
void add_multi_option_with_id(const char* name, const char* str, const char* id,
	int add_button)
{
	int var_index = find_var(name, INI_FILE_VAR);
	if (var_index == -1)
	{
		LOG_ERROR("Can't find var '%s', type 'INI_FILE_VAR'", name);
		return;
	}

	add_multi_option_to_var(our_vars.var[var_index], str, id);

	if (add_button)
	{
		int window_id = elconfig_tabs[our_vars.var[var_index]->widgets.tab_id].tab;
		int widget_id = our_vars.var[var_index]->widgets.widget_id;
		if (window_id > 0)
		{
			int n = our_vars.var[var_index]->args.multi.count - 1;
			multiselect_button_add_extended(window_id, widget_id,
				0, n * (ELCONFIG_SCALED_VALUE(22)+SPACING), 0, str,
				elconf_scale * DEFAULT_SMALL_RATIO, 0);
		}
	}
}

void clear_multiselect_var(const char* name)
{
	int var_index = find_var(name, INI_FILE_VAR);
	int window_id, widget_id;

	if (var_index == -1)
	{
		LOG_ERROR("Can't find var '%s', type 'INI_FILE_VAR'", name);
		return;
	}

	if (our_vars.var[var_index]->args.multi.count == 0)
		// Not yet initialized. Or already empty, at least.
		return;

	our_vars.var[var_index]->args.multi.count = 0;

	window_id = elconfig_tabs[our_vars.var[var_index]->widgets.tab_id].tab;
	widget_id = our_vars.var[var_index]->widgets.widget_id;
	multiselect_clear(window_id, widget_id);
}

void set_multiselect_var(const char* name, int idx, int change_button)
{
	int var_index = find_var(name, INI_FILE_VAR);
	if (var_index == -1)
	{
		LOG_ERROR("Can't find var '%s', type 'INI_FILE_VAR'", name);
		return;
	}

	if (our_vars.var[var_index]->args.multi.count <= idx)
		return;

	our_vars.var[var_index]->func(our_vars.var[var_index]->var, idx);
	our_vars.var[var_index]->saved = 0;

	if (change_button)
	{
		int window_id = elconfig_tabs[our_vars.var[var_index]->widgets.tab_id].tab;
		int widget_id = our_vars.var[var_index]->widgets.widget_id;
		if (window_id > 0)
			multiselect_set_selected(window_id, widget_id, idx);
	}
}
#endif // !MAP_EDITOR

//ELC specific variables
#ifdef ELC
static void init_ELC_vars(void)
{
	int i;
	char * win_scale_description = "Multiplied by the user interface scaling factor. With the mouse over the window: change ctrl+mousewheel up/down or ctrl+cursor up/down, set default ctrl+HOME, set initial ctrl+END.";

	// CONTROLS TAB
	add_var(OPT_BOOL,"sit_lock","sl",&sit_lock,change_var,0,"Sit Lock","Enable this to prevent your character from moving by accident when you are sitting.",CONTROLS);
	add_var(OPT_BOOL,"always_pathfinding", "alwayspathfinding", &always_pathfinding, change_var, 0, "Extend the range of the walk cursor", "Extends the range of the walk cursor to as far as you can see.  Using this option, movement may be slightly less responsive on larger maps.", CONTROLS);
	add_var(OPT_BOOL,"target_close_clicked_creature", "targetcloseclickedcreature", &target_close_clicked_creature, change_var, 1, "Target creature if you click close to it", "When enabled, if you click close to a creature that is in range, you will attack it or select it as the target for an active spell.", CONTROLS);
	add_var(OPT_BOOL,"open_close_clicked_bag", "openupcloseclickedbag", &open_close_clicked_bag, change_var, 1, "Open a bag if you click close to it", "When enabled, if you click close to a bag that is in range, you will open it.", CONTROLS);
	add_var(OPT_BOOL,"use_floating_messages", "floating", &floatingmessages_enabled, change_var, 1, "Floating Messages", "Toggles the use of floating experience messages and other graphical enhancements", CONTROLS);
	add_var(OPT_BOOL,"floating_session_counters", "floatingsessioncounters", &floating_session_counters, change_var, 0, "Floating Session Counters", "Toggles the display of floating session counters.  Configure each type using the context menu of the counter category.", CONTROLS);
	add_var(OPT_BOOL,"enable_used_item_counter", "enable_used_item_counter", &enable_used_item_counter, change_var, 0, "Enable Used Item Counter", "WARNING: If enabled, saved counters will not be compatible with previous versions of the client.  Previous versions of the client may crash when loading counters.  If disabled, Used Item counts will not be saved.", CONTROLS);
	add_var(OPT_BOOL,"use_keypress_dialog_boxes", "keypressdialogues", &use_keypress_dialogue_boxes, change_var, 0, "Keypresses in dialogue boxes", "Toggles the ability to press a key to select a menu option in dialogue boxes (eg The Wraith)", CONTROLS);
	add_var(OPT_BOOL,"use_full_dialogue_window", "keypressdialoguesfullwindow", &use_full_dialogue_window, change_var, 0, "Keypresses allowed anywhere in dialogue boxes", "If set, the above will work anywhere in the Dialogue Window, if unset only on the NPC's face", CONTROLS);
	add_var(OPT_BOOL,"use_cursor_on_animal", "useanimal", &include_use_cursor_on_animals, change_var, 0, "For animals, right click includes use cursor", "Toggles inclusion of the use cursor when right clicking on animals, useful for your summoned creatures.  Even when this option is off, you can still click the use icon.", CONTROLS);
	add_var(OPT_BOOL,"disable_double_click", "disabledoubleclick", &disable_double_click, change_var, 0, "Disable double-click button safety", "Some buttons are protected from mis-click by requiring you to double-click them.  This option disables that protection.", CONTROLS);
	add_var(OPT_BOOL,"auto_disable_ranging_lock", "adrl", &auto_disable_ranging_lock, change_var, 1, "Auto-disable Ranging Lock when under attack", "Automatically disable Ranging Lock when the char is under attack and Ranging Lock is enabled", CONTROLS);
	add_var(OPT_BOOL,"achievements_ctrl_click", "achievementsctrlclick", &achievements_ctrl_click, change_var, 0, "Control click required to view achievements", "To view a players achievements, you click on them with the eye cursor.  With this option enabled, you must use Ctrl+click.", CONTROLS);
	add_var(OPT_BOOL,"independant_quickbar_action_modes", "iqbam", &independant_quickbar_action_modes, change_var, 0, "Use independant action modes for quick-bar window", "When enabled, actions set from the icon window or action keys will not change the mode for the quick-bar window.", CONTROLS);
	add_var(OPT_BOOL,"independant_inventory_action_modes", "iiwam", &independant_inventory_action_modes, change_var, 0, "Use independant action modes for inventory window", "When enabled, actions set from the icon window or action keys will not change the mode for the invenory window.", CONTROLS);
	add_var(OPT_INT,"mouse_limit","lmouse",&mouse_limit,change_int,15,"Mouse Limit","You can increase the mouse sensitivity and cursor changing by adjusting this number to lower numbers, but usually the FPS will drop as well!",CONTROLS,1,INT_MAX);
#ifdef OSX
	add_var(OPT_BOOL,"osx_right_mouse_cam","osxrightmousecam", &osx_right_mouse_cam, change_var,0,"Rotate Camera with right mouse button", "Allows to rotate the camera by pressing the right mouse button and dragging the cursor", CONTROLS);
	add_var(OPT_BOOL,"emulate_3_button_mouse","emulate3buttonmouse", &emulate3buttonmouse, change_var,0,"Emulate a 3 Button Mouse", "If you have a 1 Button Mouse you can use <apple> click to emulate a rightclick. Needs client restart.", CONTROLS);
#endif // OSX
	add_var(OPT_MULTI,"trade_log_mode","tradelogmode",&trade_log_mode,change_int, TRADE_LOG_NONE,"Trade log","Set how successful trades are logged.",CONTROLS,"Do not log trades", "Log only to console", "Log only to file", "Log to console and file", NULL);
	// CONTROLS TAB


	// HUD TAB
	add_var(OPT_BOOL,"show_fps","fps",&show_fps,change_var,1,"Show FPS","Show the current frames per second in the corner of the window",HUD);
	add_var(OPT_BOOL,"view_analog_clock","analog",&view_analog_clock,change_var,1,"Analog Clock","Toggle the analog clock",HUD);
	add_var(OPT_BOOL,"view_digital_clock","digit",&view_digital_clock,change_var,1,"Digital Clock","Toggle the digital clock",HUD);
	add_var(OPT_BOOL,"view_knowledge_bar","knowledge_bar",&view_knowledge_bar,change_var,1,"Knowledge Bar","Toggle the knowledge bar",HUD);
	add_var(OPT_BOOL,"view_hud_timer","timer",&view_hud_timer,change_var,1,"Countdown/Stopwatch Timer","Timer controls: Right-click for menu. Shift-left-click to toggle mode. Left-click to start/stop. Mouse wheel to reset, up/down to change countdown start time (+ctrl/alt to change step).",HUD);
	add_var(OPT_BOOL,"show_game_seconds","show_game_seconds",&show_game_seconds,change_var,0,"Show Game Seconds","Show seconds on the digital clock. Note: the seconds displayed are computed on client side and synchronized with the server at each new minute.",HUD);
	add_var(OPT_BOOL,"show_stats_in_hud","sstats",&show_stats_in_hud,change_var,0,"Stats In HUD","Toggle showing stats in the HUD",HUD);
	add_var(OPT_BOOL,"show_statbars_in_hud","sstatbars",&show_statbars_in_hud,change_var,0,"StatBars In HUD","Toggle showing statbars in the HUD. Needs Stats in HUD",HUD);
	add_var(OPT_BOOL,"show_action_bar","ssactionbar",&show_action_bar,change_show_action_bar,0,"Action Points Bar in HUD","Show the current action points level in a stats bar on the bottom HUD.",HUD);
	add_var(OPT_BOOL,"show_last_health_change_always","slhca",&show_last_health_change_always,change_var,0,"Always Show The Last Health Change", "Enable to always show the last health change.  Otherwise, it is only shown when the mouse is over the health bar.",HUD);
	add_var(OPT_BOOL,"show_indicators","showindicators",&show_hud_indicators,toggle_hud_indicators_window,1,"Show Status Indicators in HUD","Show status indicators for special day (left click to show day), harvesting, poision and message count (left click to zero). Right-click the window for settings.",HUD);
	add_var(OPT_BOOL,"logo_click_to_url","logoclick",&logo_click_to_url,change_var,0,"Logo Click To URL","Toggle clicking the LOGO opening a browser window",HUD);
	add_var(OPT_STRING,"logo_link", "logolink", LOGO_URL_LINK, change_string, sizeof(LOGO_URL_LINK),
		"Logo Link", "URL when clicking the logo", HUD);
	add_var(OPT_BOOL,"show_help_text","shelp",&show_help_text,change_var,1,"Help Text","Enable tooltips.",HUD);
	add_var(OPT_BOOL,"always_enlarge_text","aetext",&always_enlarge_text,change_var,1,"Always Enlarge Text","Some text can be enlarged by pressing ALT or CTRL, often only while the mouse is over it.  Setting this option effectively locks the ALT/CTRL state to on.",HUD);
	add_var(OPT_BOOL,"show_item_desc_text","showitemdesctext",&show_item_desc_text,change_var,1,"Item Description Text","Enable item description tooltips. Needs item_info.txt file.",HUD);
	add_var(OPT_BOOL,"use_alpha_border", "aborder", &use_alpha_border, change_var, 1,"Alpha Border","Toggle the use of alpha borders",HUD);	//ADVVID);
	add_var(OPT_BOOL,"use_alpha_banner", "abanner", &use_alpha_banner, change_var, 0,"Alpha Behind Name/Health Text","Toggle the use of an alpha background to name/health banners",HUD);
	add_var(OPT_BOOL,"cm_banner_disabled", "cmbanner", &cm_banner_disabled, change_var, 0,"Disable Name/Health Text Context Menu","Disable the context menu on your players name/health banner.",HUD);
	add_var(OPT_BOOL, "windows_on_top", "wot", &windows_on_top, change_windows_on_top, 0, "Windows On Top","Allows the Manufacture, Storage and Inventory windows to appear above the map and console.", HUD);
	add_var(OPT_BOOL,"opaque_window_backgrounds", "opaquewin", &opaque_window_backgrounds, change_var, 0,"Use Opaque Window Backgrounds","Toggle the current state of all windows between transparent and opaque background. Use CTRL+D to toggle the current state of an individual window.",HUD);
	add_var(OPT_SPECINT, "buff_icon_size","bufficonsize", &buff_icon_size, set_buff_icon_size, 32, "Buff Icon Size","The size of the icons of the active buffs.  Icons are not displayed when size set to zero.",HUD,0,48);
	add_var(OPT_BOOL,"relocate_quickbar", "requick", &quickbar_relocatable, change_quickbar_relocatable, 0,"Relocate Quickbar","Set whether you can move the quickbar",HUD);
	add_var(OPT_BOOL,"relocate_quickspells", "requickspells", &quickspells_relocatable, change_quickspells_relocatable, 0,"Relocate Quick Spells","Set whether you can move the quick spells window",HUD);
	add_var(OPT_INT,"num_quickbar_slots","numqbslots",&num_quickbar_slots,change_int,6,"Number Of Quickbar Item Slots","Set the number of quick slots for inventory items. May be automatically reduced for low resolutions",HUD,1,MAX_QUICKBAR_SLOTS);
	add_var(OPT_INT,"num_quickspell_slots","numqsslots",&num_quickspell_slots,change_int,6,"Number Of Quickbar Spell Slots","Set the number of quickbar slots for spells. May be automatically reduced for low resolutions",HUD,1,MAX_QUICKSPELL_SLOTS);
	add_var(OPT_INT,"max_food_level","maxfoodlevel",&max_food_level,change_int,45,"Maximum Food Level", "Set the maximum value displayed by the food level bar.",HUD,10,200);
	add_var(OPT_INT,"wanted_num_recipe_entries","wantednumrecipeentries",&wanted_num_recipe_entries,change_num_recipe_entries,10,"Number of recipe entries", "Sets the number of entries available for the manufacturing window stored recipes.",HUD,4,max_num_recipe_entries);
	add_var(OPT_INT,"exp_log_threshold","explogthreshold",&exp_log_threshold,change_int,5000,"Log exp gain to console", "If you gain experience of this value or over, then a console message will be written.  Set the value to zero to disable completely.",HUD,0,INT_MAX);
	add_var(OPT_STRING, "npc_mark_template", "npcmarktemplate", npc_mark_str, change_string,
		sizeof(npc_mark_str), "NPC map mark template",
		"The template used when setting a map mark from the NPC dialogue (right click name). The %s is substituted for the NPC name.",
		HUD);
	add_var(OPT_BOOL,"3d_map_markers","3dmarks",&marks_3d,change_3d_marks,1,"Enable 3D Map Markers","Shows user map markers in the game window",HUD);
	add_var(OPT_BOOL,"item_window_on_drop","itemdrop",&item_window_on_drop,change_var,1,"Item Window On Drop","Toggle whether the item window shows when you drop items",HUD);
	add_var(OPT_FLOAT,"minimap_scale", "minimapscale", &local_minimap_size_coefficient, change_minimap_scale, 0.7, "Minimap Scale", "Adjust the overall size of the minimap", HUD, 0.5, 1.5, 0.1);
	add_var(OPT_BOOL,"rotate_minimap","rotateminimap",&rotate_minimap,change_var,1,"Rotate Minimap","Toggle whether the minimap should rotate.",HUD);
	add_var(OPT_BOOL,"pin_minimap","pinminimap",&pin_minimap,change_var,0,"Pin Minimap","Toggle whether the minimap ignores close-all-windows.",HUD);
	add_var(OPT_BOOL, "continent_map_boundaries", "cmb", &show_continent_map_boundaries, change_var, 1, "Map Boundaries On Continent Map", "Show map boundaries on the continent map", HUD);
	add_var(OPT_BOOL,"enable_user_menus", "user_menus", &enable_user_menus, toggle_user_menus, 0, "Enable User Menus","Create .menu files in your config directory.  First line is the menu name. After that, each line is a command using the format \"Menus Text || command || command\".  Prompt for input using \"command text <prompt text>\".",HUD);
	add_var(OPT_BOOL,"console_scrollbar_enabled", "console_scrollbar", &console_scrollbar_enabled, toggle_console_scrollbar, 1, "Show Console Scrollbar","If enabled, a scrollbar will be shown in the console window.",HUD);
#if !defined(WINDOWS) && !defined(OSX)
	add_var(OPT_BOOL,"use_clipboard","uclb",&use_clipboard, change_var, 1, "Use Clipboard For Pasting", "Use CLIPBOARD for pasting (as e.g. GNOME does) or use PRIMARY cutbuffer (as xterm does)",HUD);
#endif

	add_var(OPT_BOOL,"show_poison_count", "poison_count", &show_poison_count, change_var, 0, "Show Food Poison Count", "Displays on the poison drop icon, the number of times you have been food poisoned since last being free of poison.",HUD);

	add_var(OPT_BOOL,"your_dynamic_banner_colour", "ydbc", &dynamic_banner_colour.yourself, change_var, 1, "Dynamic Health and Mana Banner Colours", "Dynamically change the colour of your health and mana banner. For example, the health banner changes from green to red as you loose health.",HUD);
	add_var(OPT_BOOL,"player_dynamic_banner_colour", "pdbc", &dynamic_banner_colour.other_players, change_var, 1, "Dynamic Other Players Health Banner Colour", "Dynamically change the colour of the health banner for other players. It changes from green to red as they loose health.",HUD);
	add_var(OPT_BOOL,"creature_dynamic_banner_colour", "cdbc", &dynamic_banner_colour.creatures, change_var, 1, "Dynamic Creatures Health Banner Colour", "Dynamically change the colour of the health banner for creatures. It changes from green to red as they loose health.",HUD);

	// instance mode options
	add_var(OPT_BOOL,"use_view_mode_instance","instance_mode",&view_mode_instance, change_var, 0, "Use instance mode banners", "Shows only your and mobs banners, adds mana bar to your banner.",HUD);
	add_var(OPT_FLOAT,"instance_mode_banner_height","instance_mode_bheight",&view_mode_instance_banner_height,change_float,5.0f,"Your instance banner height","Sets how high the banner is located above your character",HUD,1.0,12.0,0.2);
	add_var(OPT_FLOAT,"instance_mode_damage_height","instance_mode_dheight",&view_mode_instance_damage_height,change_float,5.0f,"Your instance heal/damage height","Sets how high the heal/damage are located above your character",HUD,-12.0,12.0,0.2);
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
	add_var(OPT_BOOL, "enable_chat_show_hide", "ecsh", &enable_chat_show_hide, change_enable_chat_show_hide, 0, "Enable Show/Hide For Chat", "If enabled, you can show or hide chat either using the #K_CHAT key (usually ALT+c) or using the optional icon-bar icon.", CHAT);
	add_var(OPT_INT,"max_chat_lines","mcl",&max_chat_lines.value,change_max_chat_lines,10,"Maximum Number Of Chat Lines","For Tabbed and Old behaviour chat modes, this value sets the maximium number of lines of chat displayed.",CHAT, max_chat_lines.lower, max_chat_lines.upper);
	add_var(OPT_BOOL,"local_chat_separate", "locsep", &local_chat_separate, change_separate_flag, 0, "Separate Local Chat", "Should local chat be separate?", CHAT);
	// The forces that be want PMs always global, so that they're less likely to be ignored
	//add_var (OPT_BOOL, "personal_chat_separate", "pmsep", &personal_chat_separate, change_separate_flag, 0, "Separate Personal Chat", "Should personal chat be separate?", CHAT);
	add_var(OPT_BOOL,"guild_chat_separate", "gmsep", &guild_chat_separate, change_separate_flag, 1, "Separate Guild Chat", "Should guild chat be separate?", CHAT);
	add_var(OPT_BOOL,"server_chat_separate", "scsep", &server_chat_separate, change_separate_flag, 0, "Separate Server Messages", "Should the messages from the server be separate?", CHAT);
	add_var(OPT_BOOL,"mod_chat_separate", "modsep", &mod_chat_separate, change_separate_flag, 0, "Separate Moderator Chat", "Should moderator chat be separated from the rest?", CHAT);
	// No longer supported, the code is just missing!
	//add_var(OPT_BOOL,"highlight_tab_on_nick", "highlight", &highlight_tab_on_nick, change_var, 1, "Highlight Tabs On Name", "Should tabs be highlighted when someone mentions your name?", CHAT);
	add_var(OPT_BOOL,"emote_filter", "emote_filter", &emote_filter, change_var, 1, "Emotes filter", "Do not display lines of text in local chat containing emotes only", CHAT);
	add_var(OPT_BOOL,"summoning_filter", "summ_filter", &summoning_filter, change_var, 0, "Summoning filter", "Do not display lines of text in local chat containing summoning messages", CHAT);
	add_var(OPT_BOOL,"mixed_message_filter", "mixedmessagefilter", &mixed_message_filter, change_var, 0, "Mixed item filter", "Do not display console messages for mixed items when other windows are closed", CHAT);
	add_var(OPT_INT,"time_warning_hour","warn_h",&time_warn_h,change_int,-1,"Time warning for new hour","If set to -1, there will be no warning given. Otherwise, you will get a notification in console this many minutes before the new hour",CHAT, -1, 30);
	add_var(OPT_INT,"time_warning_sun","warn_s",&time_warn_s,change_int,-1,"Time warning for dawn/dusk","If set to -1, there will be no warning given. Otherwise, you will get a notification in console this many minutes before sunrise/sunset",CHAT, -1, 30);
	add_var(OPT_INT,"time_warning_day","warn_d",&time_warn_d,change_int,-1,"Time warning for new #day","If set to -1, there will be no warning given. Otherwise, you will get a notification in console this many minutes before the new day",CHAT, -1, 30);
	add_var(OPT_SPECINT,"auto_afk_time","afkt",&afk_time_conf,set_afk_time,5,"AFK Time","The idle time in minutes before the AFK auto message",CHAT,0,INT_MAX);
	add_var(OPT_STRING, "afk_message", "afkm", afk_message, change_string, sizeof(afk_message),
		"AFK Message", "Set the AFK message", CHAT);
	add_var(OPT_BOOL, "afk_local", "afkl", &afk_local, change_var, 0, "Save Local Chat Messages When AFK", "When you go AFK, local chat messages are counted and saved as well as PMs", CHAT);
#ifdef NEW_SOUND
	add_var(OPT_BOOL, "afk_snd_warning", "afks", &afk_snd_warning, change_var, 0, "Play AFK Message Sound", "When you go AFK, a sound is played when you receive a message or trade request", CHAT);
#endif	//NEW_SOUND
	add_var(OPT_BOOL,"use_global_ignores","gign",&use_global_ignores,change_var,1,"Global Ignores","Global ignores is a list with people that are well known for being nasty, so we put them into a list (global_ignores.txt). Enable this to load that list on startup.",CHAT);
	add_var(OPT_BOOL,"save_ignores","sign",&save_ignores,change_var,1,"Save Ignores","Toggle saving of the local ignores list on exit.",CHAT);
	add_var(OPT_BOOL, "use_global_filters", "gfil", &use_global_filters, change_global_filters, 1, "Global Filter", "Toggle the use of global text filters.", CHAT);
	add_var(OPT_BOOL,"caps_filter","caps",&caps_filter,change_var,1,"Caps Filter","Toggle the caps filter",CHAT);
	add_var(OPT_BOOL,"show_timestamp","timestamp",&show_timestamp,change_var,0,"Show Time Stamps","Toggle time stamps for chat messages",CHAT);
	add_var(OPT_MULTI_H,"dark_channeltext","dark_channeltext",&dark_channeltext,change_dark_channeltext,0,"Channel Text Color","Display the channel text in a darker color for better reading on bright maps ('Dark' may be unreadable in F1 screen)",CHAT, "Normal", "Medium", "Dark", NULL);
	// CHAT TAB


	// FONT TAB
	add_var(OPT_BOOL,"disable_auto_highdpi_scale", "disautohighdpi", &disable_auto_highdpi_scale, change_disable_auto_highdpi_scale, 0, "Disable High-DPI auto scaling", "For systems with high-dpi support (e.g. OS X): When enabled, name, chat and notepad font values, and the user interface scaling factor are all automatically scaled using the system's scale factor.", FONT);
#ifdef TTF
	add_var(OPT_BOOL, "use_ttf", "ttf", &use_ttf, change_use_ttf, 1, "Use TTF",
			"Toggle the use of True Type fonts for text rendering", FONT);
	add_var(OPT_STRING, "ttf_directory", "ttfdir", ttf_directory, change_string, sizeof(ttf_directory),
		"TTF directory", "Scan this directory and its direct subdirectories for True Type fonts. This is only used when 'Use TTF' is enabled. Changes to this option only take effect after a restart of the client.", FONT);
#endif
	add_var(OPT_FLOAT,"ui_text_size","uisize",&font_scales[UI_FONT],change_text_zoom,1,"UI Text Size","Set the size of the text in the user interface",FONT,0.8,1.2,0.01);
	add_var(OPT_MULTI,"ui_font","uifont",&font_idxs[UI_FONT],change_font,0,"UI Font","Change the type of font used in the user interface",FONT, NULL);
	add_var(OPT_FLOAT,"name_text_size","nsize",&font_scales[NAME_FONT],change_text_zoom,1,"Name Text Size","Set the size of the players name text",FONT,0.1,2.0,0.01);
	add_var(OPT_MULTI,"name_font","nfont",&font_idxs[NAME_FONT],change_font,0,"Name Font","Change the type of font used for the name",FONT, NULL);
	add_var(OPT_FLOAT,"chat_text_size","csize",&font_scales[CHAT_FONT],change_chat_zoom,1,"Chat Text Size","Sets the size of the normal text",FONT,0.1,2.0,0.01);
	add_var(OPT_MULTI,"chat_font","cfont",&font_idxs[CHAT_FONT],change_font,0,"Chat Font","Set the type of font used for normal text",FONT, NULL);
	add_var(OPT_FLOAT,"book_text_size","bsize",&font_scales[BOOK_FONT],change_text_zoom,1,"Book Text Size","Set the size of the text in in-game books",FONT,0.1,2.0,0.01);
	add_var(OPT_MULTI,"book_font","bfont",&font_idxs[BOOK_FONT],change_font,0,"Book Font","Set the type of font used for text in in-game books",FONT, NULL);
	add_var(OPT_FLOAT,"note_text_size", "notesize", &font_scales[NOTE_FONT], change_text_zoom, 0.8, "Notepad Text Size","Sets the size of the text in the notepad", FONT, 0.1, 2.0, 0.01);
	add_var(OPT_MULTI,"note_font","notefont",&font_idxs[NOTE_FONT],change_font,0,"Note Font","Set the type of font used for text in user notes",FONT, NULL);
	add_var(OPT_FLOAT,"rules_text_size","rsize",&font_scales[RULES_FONT],change_text_zoom,1,"Rules Text Size","Set the size of the rules text",FONT,0.1,2.0,0.01);
	add_var(OPT_MULTI,"rules_font","rfont",&font_idxs[RULES_FONT],change_font,0,"Rules Font","Set the type of font used for drawing the game rules",FONT, NULL);
	add_var(OPT_FLOAT, "encyclopedia_text_size", "esize", &font_scales[ENCYCLOPEDIA_FONT],
		change_text_zoom, 1, "Encyclopedia Text Size",
		"Set the size of the encyclopedia and help  text", FONT, 0.1, 2.0, 0.01);
	add_var(OPT_MULTI, "encyclopedia_font", "efont", &local_encyclopedia_font,
		change_font, 0, "Encyclopedia Font",
		 "Set the type of font used for drawing the encycloepdia and ingame help",
		 FONT, NULL);
	add_var(OPT_FLOAT,"mapmark_text_size", "marksize", &font_scales[MAPMARK_FONT], change_text_zoom, 1.0, "Mapmark Text Size","Sets the size of the mapmark text", FONT, 0.1, 2.0, 0.01);
	add_var(OPT_FLOAT,"ui_scale","ui_scale",&local_ui_scale,change_ui_scale,1,"User interface scaling factor","Scale user interface by this factor, useful for high DPI displays.  Note: the options window will be rescaled after reopening.",FONT,0.75,3.0,0.01);
	add_var(OPT_INT,"cursor_scale_factor","cursor_scale_factor",&cursor_scale_factor ,change_cursor_scale_factor,cursor_scale_factor,"Mouse pointer scaling factor","The size of the mouse pointer is scaled by this factor",FONT, 1, max_cursor_scale_factor);
	add_var(OPT_BOOL,"disable_window_scaling_controls","disablewindowscalingcontrols", get_scale_flag_MW(), change_var, 0, "Disable Window Scaling Controls", "If you do not want to use keys or mouse+scrollwheel to scale individual windows, set this option.", FONT);
	add_var(OPT_FLOAT,"trade_win_scale","tradewinscale",get_scale_WM(MW_TRADE),change_win_scale_factor,1.0f,"Trade window scaling factor",win_scale_description,FONT,win_scale_min,win_scale_max,win_scale_step);
	add_var(OPT_FLOAT,"item_win_scale","itemwinscale",get_scale_WM(MW_ITEMS),change_win_scale_factor,1.0f,"Inventory window scaling factor",win_scale_description,FONT,win_scale_min,win_scale_max,win_scale_step);
	add_var(OPT_FLOAT,"bags_win_scale","bagswinscale",get_scale_WM(MW_BAGS),change_win_scale_factor,1.0f,"Ground bag window scaling factor",win_scale_description,FONT,win_scale_min,win_scale_max,win_scale_step);
	add_var(OPT_FLOAT,"spells_win_scale","spellswinscale",get_scale_WM(MW_SPELLS),change_win_scale_factor,1.0f,"Spells window scaling factor",win_scale_description,FONT,win_scale_min,win_scale_max,win_scale_step);
	add_var(OPT_FLOAT,"storage_win_scale","storagewinscale",get_scale_WM(MW_STORAGE),change_win_scale_factor,1.0f,"Storage window scaling factor",win_scale_description,FONT,win_scale_min,win_scale_max,win_scale_step);
	add_var(OPT_FLOAT,"manu_win_scale","manuwinscale",get_scale_WM(MW_MANU),change_win_scale_factor,1.0f,"Manufacturing window scaling factor",win_scale_description,FONT,win_scale_min,win_scale_max,win_scale_step);
	add_var(OPT_FLOAT,"emote_win_scale","emotewinscale",get_scale_WM(MW_EMOTE),change_win_scale_factor,1.0f,"Emote window scaling factor",win_scale_description,FONT,win_scale_min,win_scale_max,win_scale_step);
	add_var(OPT_FLOAT,"questlog_win_scale","questlogwinscale",get_scale_WM(MW_QUESTLOG),change_win_scale_factor,1.0f,"Quest log window scaling factor",win_scale_description,FONT,win_scale_min,win_scale_max,win_scale_step);
	add_var(OPT_FLOAT,"note_url_win_scale","noteurlwinscale",get_scale_WM(MW_INFO),change_win_scale_factor,1.0f,"Notepad/URL window scaling factor",win_scale_description,FONT,win_scale_min,win_scale_max,win_scale_step);
	add_var(OPT_FLOAT,"buddy_win_scale","buddywinscale",get_scale_WM(MW_BUDDY),change_win_scale_factor,1.0f,"Buddy window scaling factor",win_scale_description,FONT,win_scale_min,win_scale_max,win_scale_step);
	add_var(OPT_FLOAT,"stats_win_scale","statswinscale",get_scale_WM(MW_STATS),change_win_scale_factor,1.0f,"Stats window scaling factor",win_scale_description,FONT,win_scale_min,win_scale_max,win_scale_step);
	add_var(OPT_FLOAT,"help_win_scale","helpwinscale",get_scale_WM(MW_HELP),change_win_scale_factor,1.0f,"Help window scaling factor",win_scale_description,FONT,win_scale_min,win_scale_max,win_scale_step);
	add_var(OPT_FLOAT,"ranging_win_scale","rangingwinscale",get_scale_WM(MW_RANGING),change_win_scale_factor,1.0f,"Ranging window scaling factor",win_scale_description,FONT,win_scale_min,win_scale_max,win_scale_step);
	add_var(OPT_FLOAT,"achievements_win_scale","achievementswinscale",get_scale_WM(MW_ACHIEVE),change_win_scale_factor,1.0f,"Achievements window scaling factor",win_scale_description,FONT,win_scale_min,win_scale_max,win_scale_step);
	add_var(OPT_FLOAT,"dialogue_win_scale","dialoguewinscale",get_scale_WM(MW_DIALOGUE),change_win_scale_factor,1.0f,"Dialogue window scaling factor",win_scale_description,FONT,win_scale_min,win_scale_max,win_scale_step);
	add_var(OPT_FLOAT,"quickbar_win_scale","quickbarwinscale",get_scale_WM(MW_QUICKBAR),change_win_scale_factor,1.0f,"Quickbar window scaling factor",win_scale_description,FONT,win_scale_min,win_scale_max,win_scale_step);
	add_var(OPT_FLOAT,"quickspells_win_scale","quickspellswinscale",get_scale_WM(MW_QUICKSPELLS),change_win_scale_factor,1.0f,"Quickspells window scaling factor",win_scale_description,FONT,win_scale_min,win_scale_max,win_scale_step);
	add_var(OPT_FLOAT,"chat_win_scale","chatwinscale",get_scale_WM(MW_CHAT),change_win_scale_factor,1.0f,"Chat window scaling factor",win_scale_description,FONT,win_scale_min,win_scale_max,win_scale_step);
	add_var(OPT_FLOAT,"options_win_scale","optionswinscale",&elconf_custom_scale,change_elconf_win_scale_factor,1.0f,"Options window scaling factor","Multiplied by the user interface scaling factor. Change will take effect after closing then reopening the window.",FONT,win_scale_min,win_scale_max,win_scale_step);
#ifdef NEW_CURSOR
	add_var(OPT_BOOL,"sdl_cursors","sdl_cursors", &sdl_cursors, change_sdl_cursor,1,"Use Standard Black/White Mouse Pointers", "When disabled, use the experimental coloured mouse pointers. Needs the texture from Git dev-data-files/cursor2.dss.", FONT);
	add_var(OPT_BOOL,"big_cursors","big_cursors", &big_cursors, change_var,0,"Use Large Pointers", "When using the experiment coloured mouse pointers, use the large pointer set.", FONT);
	add_var(OPT_FLOAT,"pointer_size","pointer_size", &pointer_size, change_float,1.0,"Coloured Pointer Size", "When using the experiment coloured mouse pointers, set the scale of the pointer. 1.0 is 1:1 scale.", FONT,0.25,4.0,0.05);
#endif // NEW_CURSOR
	// FONT TAB


	// SERVER TAB
	add_var(OPT_STRING,"username", "u", active_username_str, change_string, sizeof(active_username_str),
		"Username", "Your user name here", SERVER);
	add_var(OPT_PASSWORD, "password", "p", active_password_str, change_string, sizeof(active_password_str),
		"Password", "Put your password here", SERVER);
	add_var(OPT_BOOL,"passmngr_enabled","pme",&passmngr_enabled,change_var,0,"Enable Password Manager", "If enabled, user names and passwords are saved locally by the built-in password manager.  Multiple sets of details can be saved.  You can choose which details to use at the login screen.",SERVER);
	add_var(OPT_MULTI,"log_chat","log",&log_chat,change_int,LOG_SERVER,"Log Messages","Log messages from the server (chat, harvesting events, GMs, etc)",SERVER,"Do not log chat", "Log chat only", "Log server messages", "Log server to srv_log.txt", NULL);
	add_var(OPT_BOOL,"rotate_chat_log","rclog",&rotate_chat_log_config_var,change_rotate_chat_log,0,"Rotate Chat Log File","Tag the chat/server message log files with year and month. You will still need to manage deletion of the old files. Requires a client restart.",SERVER);
	add_var(OPT_BOOL,"buddy_log_notice", "buddy_log_notice", &buddy_log_notice, change_var, 1, "Log Buddy Sign On/Off", "Toggle whether to display notices when people on your buddy list log on or off", SERVER);
	add_var(OPT_STRING,"language", "lang", lang, change_string, sizeof(lang), "Language", "Wah?", SERVER);
	add_var(OPT_STRING, "browser", "b", browser_name, change_string, sizeof(browser_name), "Browser",
		"Location of your web browser (Windows users leave blank to use default browser)",
		SERVER);
	add_var(OPT_BOOL,"write_ini_on_exit", "wini", &write_ini_on_exit, change_var, 1,"Save INI","Save options when you quit",SERVER);
	add_var(OPT_STRING,"data_dir","dir",datadir,change_dir_name,90,"Data Directory","Place were we keep our data. Can only be changed with a Client restart.",SERVER);
	add_var(OPT_BOOL,"serverpopup","spu",&use_server_pop_win,change_var,1,"Use Special Text Window","Toggles whether server messages from channel 255 are displayed in a pop up window.",SERVER);
	/* Note: We don't take any action on the already-running thread, as that wouldn't necessarily be good. */
	add_var(OPT_BOOL,"autoupdate","aup",&auto_update,change_var,1,"Automatic Updates","Toggles whether updates are automatically downloaded.",SERVER);
#ifdef CUSTOM_UPDATE
	add_var(OPT_BOOL,"customupdate","cup",&custom_update,change_custom_update,1,"Custom Looks Updates","Toggles whether custom look updates are automatically downloaded.",SERVER);
	add_var(OPT_BOOL,"showcustomclothing","scc",&custom_clothing,change_custom_clothing,1,"Show Custom clothing","Toggles whether custom clothing is shown.",SERVER);
#endif	//CUSTOM_UPDATE
#ifdef JSON_FILES
	add_var(OPT_BOOL, "use_json_user_files_v1", "usejsonuserfiles_v1", &use_json_user_files, change_use_json_user_files, 0, "Use New Format To Save User Files (.json)",
		"NOTE: Use this option to enable the new format for saving user data.  If you change this option, data is automatically saved using the chosen format.  Disable this option before switching back to 1.9.5p8 or older clients.", SERVER);
#endif
	// SERVER TAB


	// AUDIO TAB
#ifdef NEW_SOUND
	add_var(OPT_BOOL,"disable_sound", "nosound", &no_sound, disable_sound, 0, "Disable Sound & Music System", "Disable all of the sound effects and music processing", AUDIO);
	add_var(OPT_STRING, "sound_device", "snddev", sound_device, change_string, sizeof(sound_device),
		"Sound Device", "Device used for playing sounds & music", AUDIO);
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
	add_var(OPT_INT,"video_width","width",&video_user_width,change_int, 640,"Userdefined width","Userdefined window width",VIDEO, 640,INT_MAX);
	add_var(OPT_INT,"video_height","height",&video_user_height,change_int, 480,"Userdefined height","Userdefined window height",VIDEO, 480,INT_MAX);
	add_var(OPT_INT,"limit_fps","lfps",&limit_fps,change_fps,0,"Limit FPS","Limit the frame rate to reduce load on the system",VIDEO,0,INT_MAX);
	add_var(OPT_FLOAT,"gamma","g",&gamma_var,change_gamma,1,"Gamma","How bright your display should be.",VIDEO,0.10,3.00,0.05);
	add_var(OPT_BOOL,"disable_gamma_adjust","dga",&disable_gamma_adjust,change_var,0,"Disable Gamma Adjustment","Stop the client from adjusting the display gamma.",VIDEO);
#ifdef ANTI_ALIAS
	add_var(OPT_BOOL,"anti_alias", "aa", &anti_alias, change_aa, 0, "Toggle Anti-Aliasing", "Anti-aliasing makes edges look smoother", VIDEO);
#endif //ANTI_ALIAS
#ifdef	FSAA
	add_var(OPT_MULTI_H, "anti_aliasing", "fsaa", &fsaa_index, change_fsaa, 0, "Anti-Aliasing", "Full Scene Anti-Aliasing", VIDEO, get_fsaa_mode_str(0), NULL);
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
	add_var(OPT_BOOL,"small_actor_texture_cache","small_actor_tc",&small_actor_texture_cache,change_small_actor_texture_cache,0,"Small actor texture cache","A small Actor texture cache uses less video memory, but actor loading can be slower.",VIDEO);
	add_var(OPT_BOOL,"use_vertex_buffers","vbo",&use_vertex_buffers,change_vertex_buffers,0,"Vertex Buffer Objects","Toggle the use of the vertex buffer objects, restart required to activate it",VIDEO);
	add_var(OPT_BOOL, "use_animation_program", "uap", &use_animation_program, change_use_animation_program, 1, "Use animation program", "Use GL_ARB_vertex_program for actor animation", VIDEO);
	add_var(OPT_BOOL_INI, "video_info_sent", "svi", &video_info_sent, change_var, 0, "Video info sent", "Video information are sent to the server (like OpenGL version and OpenGL extentions)", VIDEO);
	// VIDEO TAB


	// GFX TAB
	add_var(OPT_BOOL,"shadows_on","shad",&shadows_on,change_shadows,0,"Shadows","Toggles the shadows", GFX);
	add_var(OPT_BOOL,"use_shadow_mapping", "sm", &use_shadow_mapping, change_shadow_mapping, 0, "Shadow Mapping", "If you want to use some better quality shadows, enable this. It will use more resources, but look prettier.", GFX);
	add_var(OPT_MULTI,"shadow_map_size","smsize",&shadow_map_size_multi,change_shadow_map_size,3,"Shadow Map Size","This parameter determines the quality of the shadow maps. You should as minimum set it to 512.",GFX,"256","512","768","1024","1280","1536","1792","2048","3072","4096",NULL);
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
	add_var(OPT_BOOL,"use_eye_candy", "ec", &use_eye_candy, change_eye_candy, 1, "Enable Eye Candy", "Toggles most visual effects, like spells' and harvesting events'. Needs OpenGL 1.5", GFX);
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
	add_var(OPT_BOOL,"clear_mod_keys_on_focus", "clear_mod_keys_on_focus", &clear_mod_keys_on_focus, change_var, 0, "Clear modifier keys when window focused","If you have trouble with modifier keys (shift/ctrl/alt etc) when keyboard focus returns, enable this option to force all modifier keys up.", TROUBLESHOOT);
	add_var(OPT_BOOL,"use_compiled_vertex_array","cva",&use_compiled_vertex_array,change_compiled_vertex_array,1,"Compiled Vertex Array","Some systems will not support the new compiled vertex array in EL. Disable this if some 3D objects do not display correctly.",TROUBLESHOOT);
	add_var(OPT_BOOL,"use_draw_range_elements","dre",&use_draw_range_elements,change_var,1,"Draw Range Elements","Disable this if objects appear partially stretched.",TROUBLESHOOT);
	add_var(OPT_BOOL,"use_point_particles","upp",&use_point_particles,change_point_particles,1,"Point Particles","Some systems will not support the new point based particles in EL. Disable this if your client complains about not having the point based particles extension.",TROUBLESHOOT);
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



void init_vars(void)
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
	add_var(OPT_BOOL,"show_reflections","srefl",&show_mapeditor_reflections, change_var, 0, "Show reflections", "Show reflections, disabling improves editor performance",HUD);
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

static void write_var(FILE *fout, int ivar)
{
	var_struct *var;

	if (fout == NULL) return;

	var = our_vars.var[ivar];
	switch (var->type)
	{
		case OPT_INT:
		case OPT_BOOL:
		case OPT_INT_F:
		case OPT_BOOL_INI:
		case OPT_INT_INI:
		{
			int *p = var->var;
			fprintf(fout, "#%s= %d\n", var->name, *p);
			break;
		}
		case OPT_MULTI:
		case OPT_MULTI_H:
		{
			int idx = *(const int*)var->var;
			const char* value = var->args.multi.elems[idx].value;
			if (value)
				fprintf(fout, "#%s= %d(%s)\n", var->name, idx, value);
			else
				fprintf(fout, "#%s= %d\n", var->name, idx);
			break;
		}
		case OPT_STRING:
			if (strcmp(var->name, "password") == 0)
				// Do not write the password to the file. If the user really wants it
				// s/he should edit the file.
				fprintf(fout, "#%s= \"\"\n", var->name);
			else
				fprintf(fout, "#%s= \"%s\"\n", var->name, (const char *)var->var);
			break;
		case OPT_PASSWORD:
			// Do not write the password to the file. If the user really wants it
			// s/he should edit the file.
			fprintf(fout, "#%s= \"\"\n", var->name);
			break;
		case OPT_FLOAT:
		case OPT_FLOAT_F:
		{
			float *g = var->var;
			fprintf(fout, "#%s= %g\n", var->name, *g);
			break;
		}
	}
	var->saved= 1;	// keep only one copy of this setting
}


int read_el_ini (void)
{
	input_line line;
#ifdef MAP_EDITOR
	FILE *fin= open_file_config("mapedit.ini", "r");
#else
	FILE *fin= open_file_config(ini_filename, "r");
#endif //MAP_EDITOR

	if (fin == NULL){
		LOG_ERROR("%s: %s \"%s\": %s\n", reg_error_str, cant_open_file, ini_filename, strerror(errno));
		return 0;
	}

#ifdef	ELC
	delay_poor_man = delay_update_highdpi_auto_scaling = 1;
#endif
	while ( fgets (line, sizeof (input_line), fin) )
	{
		if (line[0] == '#')
			check_var(&(line[1]), INI_FILE_VAR);
	}
#ifdef	ELC
	delay_poor_man = delay_update_highdpi_auto_scaling = 0;
	action_poor_man(&poor_man);
#endif

	fclose (fin);
	return 1;
}

int write_el_ini (void)
{
#if !defined(WINDOWS)
	int fd;
	struct stat statbuff;
#endif // !WINDOWS
	int nlines= 0, maxlines= 0, iline, ivar;
	input_line *cont= NULL;
	const char *last_line;
	FILE *file;
	short *written;

	// first check if we need to change anything
	//
	// The advantage of skipping this check is that a new ini file would be
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
	file = open_file_config(ini_filename, "r");
	if(file == NULL){
		LOG_ERROR("%s: %s \"%s\": %s\n", reg_error_str, cant_open_file, ini_filename, strerror(errno));
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
		fclose(file);
	}

	// Now write the contents of the file, updating those variables that have been changed
	file = open_file_config(ini_filename, "w");
	if(file == NULL){
		LOG_ERROR("%s: %s \"%s\": %s\n", reg_error_str, cant_open_file, ini_filename, strerror(errno));
		free(cont);
		return 0;
	}

	// Prevent duplicate entries by remembering which we have written
	written = calloc(our_vars.no, sizeof(short));

	last_line = "";
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
		last_line = cont[iline];
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
static int display_elconfig_handler(window_info *win)
{
	// Draw the long description of an option
	draw_string_zoomed_width_font(TAB_MARGIN, elconfig_menu_y_len-LONG_DESC_SPACE,
		elconf_description_buffer, window_width, MAX_LONG_DESC_LINES, win->font_category,
		elconf_scale * DEFAULT_SMALL_RATIO);

	// Show the context menu help message
	if (is_mouse_over_option)
		show_help(cm_help_options_str, 0, win->len_y + 10, win->current_scale);
	is_mouse_over_option = 0;

	return 1;
}

static int spinbutton_onkey_handler(widget_list *widget, int mx, int my, SDL_Keycode key_code, Uint32 key_unicode, Uint16 key_mod)
{
	if(widget != NULL) {
		int i;
		spinbutton *button;

		if (!(key_mod & KMOD_ALT) && !(key_mod & KMOD_CTRL)) {
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

static int spinbutton_onclick_handler(widget_list *widget, int mx, int my, Uint32 flags)
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

static int multiselect_click_handler(widget_list *widget, int mx, int my, Uint32 flags)
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

static int mouseover_option_handler(widget_list *widget, int mx, int my)
{
	int i;

	//Find the label in our_vars
	for (i = 0; i < our_vars.no; i++)
	{
		if (widget->id == our_vars.var[i]->widgets.label_id
			|| widget->id == our_vars.var[i]->widgets.widget_id)
			break;
	}
	if (i == our_vars.no)
		//We didn't find anything, abort
		return 0;
	if (i == last_description_idx)
		// We're still on the same variable
		return 1;

	safe_strncpy((char*)elconf_description_buffer, (const char*)our_vars.var[i]->display.desc,
		sizeof(elconf_description_buffer));
	reset_soft_breaks(elconf_description_buffer, strlen((const char*)elconf_description_buffer),
		sizeof(elconf_description_buffer), CONFIG_FONT, elconf_scale * DEFAULT_SMALL_RATIO,
		elconfig_menu_x_len - 2*TAB_MARGIN, NULL, NULL);

	last_description_idx = i;

	return 1;
}

static int mouseover_option_label_handler(widget_list *widget, int mx, int my)
{
	is_mouse_over_option = 1;
	return mouseover_option_handler(widget, mx, my);
}

static int onclick_label_handler(widget_list *widget, int mx, int my, Uint32 flags)
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

	if (flags&ELW_RIGHT_MOUSE)
	{
		call_option_menu(option);
		return 1;
	}

	if (option->type == OPT_BOOL)
	{
		option->func(option->var);
		option->saved= 0;
		do_click_sound();
	}

	return 1;
}

static int onclick_checkbox_handler(widget_list *widget, int mx, int my, Uint32 flags)
{
	int i;
	var_struct *option= NULL;

	for(i= 0; i < our_vars.no; i++) {
		if(our_vars.var[i]->widgets.widget_id == widget->id) {
			option= our_vars.var[i];
			break;
		}
	}

	if (!option)
		// shouldn't happen
		return 0;

	if (option->type == OPT_BOOL)
	{
		int *var= option->var;
		*var= !*var;
		option->func(var);
		option->saved= 0;
	}

	return 1;
}

static int string_onkey_handler(widget_list *widget)
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
				return 0;
			}
		}
	}

	return 0;
}

static int get_elconfig_content_width(void)
{
	int i, iopt, line_width, max_line_width = 0;
	var_struct *var;
	int spin_button_width = max2i(ELCONFIG_SCALED_VALUE(100),
		4 * get_max_digit_width_zoom(CONFIG_FONT, elconf_scale) + 4 * (int)(0.5 + 5 * elconf_scale));

	for (i = 0; i < our_vars.no; i++)
	{
		var = our_vars.var[i];

		switch (var->type)
		{
			case OPT_BOOL_INI:
			case OPT_INT_INI:
			case OPT_PASSWORD:
				// not shown
				line_width = 0;
				break;
			case OPT_BOOL:
				line_width = CHECKBOX_SIZE + SPACING
					+ get_string_width_zoom(var->display.str, CONFIG_FONT, elconf_scale);
				break;
			case OPT_INT:
			case OPT_FLOAT:
			case OPT_INT_F:
			case OPT_FLOAT_F:
				line_width = get_string_width_zoom(var->display.str, CONFIG_FONT, elconf_scale)
					+ SPACING + spin_button_width;
				break;
			case OPT_STRING:
				// don't display the username, if it is changed after login, any name tagged files will be saved using the new name
				if (strcmp(our_vars.var[i]->name, "username") == 0)
				{
					line_width = 0;
				}
				else
				{
					line_width = get_string_width_zoom(var->display.str, CONFIG_FONT, elconf_scale)
						+ SPACING + ELCONFIG_SCALED_VALUE(332);
				}
				break;
			case OPT_MULTI:
				line_width = get_string_width_zoom(var->display.str, CONFIG_FONT, elconf_scale)
					+ SPACING + ELCONFIG_SCALED_VALUE(250);
				break;
			case OPT_MULTI_H:
				line_width = get_string_width_zoom(var->display.str, CONFIG_FONT, elconf_scale);
				for (iopt = 0; iopt < our_vars.var[i]->args.multi.count; ++iopt)
				{
					int radius = elconf_scale * BUTTONRADIUS;
					const char *label= var->args.multi.elems[iopt].label;
					if (!*label)
						label = "??";

					line_width += SPACING + 2 * radius
						+ get_string_width_zoom((const unsigned char*)label, CONFIG_FONT, elconf_scale);
				}
				break;
			default:
				line_width = 0;
		}

		max_line_width = max2i(max_line_width, line_width);
	}

	return max_line_width + 2 * TAB_MARGIN + ELCONFIG_SCALED_VALUE(ELW_BOX_SIZE);
}

static void elconfig_populate_tabs(void)
{
	int i;
	int label_id=-1; //temporary storage for the label id
	int widget_id=-1; //temporary storage for the widget id
	int widget_width, widget_height, label_height; //Used to calculate the y pos of the next option
	int x;
	int line_height = get_line_height(CONFIG_FONT, elconf_scale);
	int y_label, y_widget, dx, dy, iopt;
	int spin_button_width = max2i(ELCONFIG_SCALED_VALUE(100),
		4 * get_max_digit_width_zoom(CONFIG_FONT, elconf_scale) + 4 * (int)(0.5 + 5 * elconf_scale));

	for(i= 0; i < MAX_TABS; i++) {
		//Set default values
		elconfig_tabs[i].x= TAB_MARGIN;
		elconfig_tabs[i].y= TAB_MARGIN;
	}

	for(i= 0; i < our_vars.no; i++)
	{
		var_struct *var = our_vars.var[i];
		int tab_id = var->widgets.tab_id;
		int window_id = elconfig_tabs[tab_id].tab;
		int window_width = get_window_content_width(window_id);
		int current_x = elconfig_tabs[tab_id].x;
		int current_y = elconfig_tabs[tab_id].y;

		switch(var->type)
		{
			case OPT_BOOL_INI:
			case OPT_INT_INI:
				// This variable should not be settable
				// through the window, so don't try to add it,
				// and more importantly, don't try to compute
				// its height
				continue;
			case OPT_BOOL:
				//Add checkbox
				dy = line_height - CHECKBOX_SIZE;
				y_widget = current_y + max2i(dy / 2, 0);
				widget_id = checkbox_add_extended(window_id, elconfig_free_widget_id++, NULL,
					current_x, y_widget, CHECKBOX_SIZE, CHECKBOX_SIZE, 0, elconf_scale, var->var);
				//Add label for the checkbox
				y_label = current_y - min2i(dy / 2, 0);
				label_id = label_add_extended(window_id, elconfig_free_widget_id++, NULL,
					current_x+CHECKBOX_SIZE+SPACING, y_label, 0, elconf_scale, (char*)var->display.str);
				//Set handlers
				widget_set_OnClick(window_id, widget_id, onclick_checkbox_handler);
			break;
			case OPT_INT:
				/* interval is always 1 */
				label_id = label_add_extended(window_id, elconfig_free_widget_id++, NULL,
					current_x, current_y, 0, elconf_scale, (char*)var->display.str);
				widget_width = spin_button_width;
				widget_id = spinbutton_add_extended(window_id, elconfig_free_widget_id++, NULL,
					window_width - TAB_MARGIN - widget_width, current_y, widget_width, line_height,
					SPIN_INT, var->var, var->args.imm.min,
					var->args.imm.max, 1.0, elconf_scale);
				widget_set_OnKey(window_id, widget_id, (int (*)())spinbutton_onkey_handler);
				widget_set_OnClick(window_id, widget_id, spinbutton_onclick_handler);
			break;
			case OPT_FLOAT:
				label_id = label_add_extended(window_id, elconfig_free_widget_id++, NULL,
					current_x, current_y, 0, elconf_scale, (char*)var->display.str);
				widget_width = spin_button_width;
				widget_id = spinbutton_add_extended(window_id, elconfig_free_widget_id++, NULL,
					window_width - TAB_MARGIN - widget_width, current_y, widget_width, line_height,
					SPIN_FLOAT, var->var, var->args.fmmi.min, var->args.fmmi.max,
					var->args.fmmi.interval, elconf_scale);
				widget_set_OnKey(window_id, widget_id, (int (*)())spinbutton_onkey_handler);
				widget_set_OnClick(window_id, widget_id, spinbutton_onclick_handler);
			break;
			case OPT_STRING:
				// don't display the username, if it is changed after login, any name tagged files will be saved using the new name
				if (strcmp(var->name, "username") == 0)
					continue;
				widget_width = ELCONFIG_SCALED_VALUE(332);
				widget_id = pword_field_add_extended(window_id, elconfig_free_widget_id++, NULL,
					window_width - TAB_MARGIN - widget_width, current_y, widget_width, 0,
					P_TEXT, elconf_scale, var->var, var->len);
				dy = widget_get_height(window_id, widget_id) - line_height;
				label_id = label_add_extended(window_id, elconfig_free_widget_id++, NULL,
					current_x, current_y + dy/2, 0, elconf_scale, (char*)var->display.str);
				widget_set_OnKey (window_id, widget_id, (int (*)())string_onkey_handler);
			break;
			case OPT_PASSWORD:
				// Grum: the client shouldn't store the password, so let's not add it to the configuration window
				//label_id= label_add_extended(window_id, elconfig_free_widget_id++, NULL, current_x, current_y, 0, 0, 0, 1.0, var->display.str);
				//widget_id= pword_field_add_extended(window_id, elconfig_free_widget_id++, NULL, elconfig_menu_x_len/2, current_y, 200, 20, P_NORMAL, 1.0f, var->var, var->len);
				//widget_set_OnKey (window_id, widget_id, string_onkey_handler);
				continue;
			case OPT_MULTI:
				label_id = label_add_extended(window_id, elconfig_free_widget_id++, NULL,
					current_x, current_y, 0, elconf_scale, (char*)var->display.str);
				widget_width = ELCONFIG_SCALED_VALUE(250);
				widget_id = multiselect_add_extended(window_id, elconfig_free_widget_id++, NULL,
					window_width - TAB_MARGIN - widget_width, current_y, widget_width,
					ELCONFIG_SCALED_VALUE(80), elconf_scale, gui_color[0], gui_color[1], gui_color[2],
					gui_invert_color[0], gui_invert_color[1], gui_invert_color[2], 0);
				for (iopt = 0; iopt < var->args.multi.count; ++iopt)
				{
					const char *label = var->args.multi.elems[iopt].label;
					if (!*label)
						label = "??";
					multiselect_button_add_extended(window_id, widget_id,
						0, iopt*(ELCONFIG_SCALED_VALUE(22)+SPACING), 0, label,
						DEFAULT_SMALL_RATIO*elconf_scale, iopt == *(int *)var->var);
				}
				multiselect_set_selected(window_id, widget_id, *((const int*)var->var));
				widget_set_OnClick(window_id, widget_id, multiselect_click_handler);
			break;
			case OPT_FLOAT_F:
				label_id = label_add_extended(window_id, elconfig_free_widget_id++, NULL,
					current_x, current_y, 0, elconf_scale, (char*)var->display.str);
				widget_width = spin_button_width;
				widget_id = spinbutton_add_extended(window_id, elconfig_free_widget_id++, NULL,
					window_width - TAB_MARGIN + widget_width, current_y, widget_width, line_height,
					SPIN_FLOAT, var->var, var->args.fmmif.min(), var->args.fmmif.max(),
					var->args.fmmif.interval, elconf_scale);
				widget_set_OnKey(window_id, widget_id, (int (*)())spinbutton_onkey_handler);
				widget_set_OnClick(window_id, widget_id, spinbutton_onclick_handler);
			break;
			case OPT_INT_F:
				/* interval is always 1 */
				label_id = label_add_extended(window_id, elconfig_free_widget_id++, NULL,
					current_x, current_y, 0, elconf_scale, (char*)var->display.str);
				widget_width = spin_button_width;
				widget_id = spinbutton_add_extended(window_id, elconfig_free_widget_id++, NULL,
					window_width - TAB_MARGIN - widget_width, current_y, widget_width, line_height,
					SPIN_INT, var->var, var->args.immf.min(), var->args.immf.max(), 1.0, elconf_scale);
				widget_set_OnKey(window_id, widget_id, (int (*)())spinbutton_onkey_handler);
				widget_set_OnClick(window_id, widget_id, spinbutton_onclick_handler);
			break;
			case OPT_MULTI_H:
				label_id= label_add_extended(window_id, elconfig_free_widget_id++, NULL,
					current_x, current_y, 0, elconf_scale, (const char*)var->display.str);
				x = current_x + widget_get_width(window_id, label_id) + SPACING;
				widget_id = multiselect_add_extended(window_id, elconfig_free_widget_id++,
					NULL, x, current_y, ELCONFIG_SCALED_VALUE(350), ELCONFIG_SCALED_VALUE(80),
					elconf_scale, gui_color[0], gui_color[1], gui_color[2], gui_invert_color[0],
					gui_invert_color[1], gui_invert_color[2], 0);
				dx = 0;
				for (iopt = 0; iopt < var->args.multi.count; ++iopt)
				{
					int radius = elconf_scale*BUTTONRADIUS;
					int width=0;
					const char *label= var->args.multi.elems[iopt].label;
					if (!*label)
						label = "??";

					width = 2 * radius
						+ get_string_width_zoom((const unsigned char*)label, CONFIG_FONT, elconf_scale);

					multiselect_button_add_extended(window_id, widget_id, dx, 0, width, label,
						DEFAULT_SMALL_RATIO * elconf_scale, iopt == *(int *)var->var);

					dx += width + SPACING;
				}

				widget_width = dx - SPACING;
				widget_height = widget_get_height(window_id, widget_id);
				dy = line_height - widget_height;
				if (dy < 0)
				{
					widget_move(window_id, label_id, current_x, current_y - dy / 2);
					widget_move(window_id, widget_id, window_width - TAB_MARGIN - widget_width, current_y);
				}
				else
				{
					widget_move(window_id, widget_id,
						window_width - TAB_MARGIN - widget_width, current_y + dy / 2);
				}

				multiselect_set_selected(window_id, widget_id, *((const int*)var->var));
				widget_set_OnClick(window_id, widget_id, multiselect_click_handler);
			break;
		}

		//Calculate y position of the next option.
		label_height = widget_get_height(window_id, label_id);
		widget_height = widget_get_height(window_id, widget_id);
		elconfig_tabs[tab_id].y += max2i(widget_height, label_height) + SPACING;
		//Set IDs
		our_vars.var[i]->widgets.label_id= label_id;
		our_vars.var[i]->widgets.widget_id= widget_id;
		//Make the description print when the mouse is over a widget
		widget_set_OnMouseover(window_id, label_id, mouseover_option_label_handler);
		widget_set_OnMouseover(window_id, widget_id, mouseover_option_handler);
		//left click used only to tolle BOOL, right click to open context menu
		widget_set_OnClick(window_id, label_id, onclick_label_handler);
	}
}

// TODO: replace this hack by something clean.
static int show_elconfig_handler(window_info * win) {
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
		init_window(win->window_id, ((not_on_top_now(MW_CONFIG) &&  !force_elconfig_win_ontop) ?game_root_win : -1),
			0, win->pos_x - pwinx, win->pos_y - pwiny, win->len_x, win->len_y);
	}
#else
	init_window(win->window_id, game_root_win, 0, win->pos_x - pwinx, win->pos_y - pwiny, win->len_x, win->len_y);
#endif

	return 1;
}

// It would be messy to resize the window each time the scale option is changed so
// keep the scale at the original value until we can recreate.
static int ui_scale_elconfig_handler(window_info *win)
{
	update_window_scale(win, elconf_scale); // stop scale change impacting immediately
	if (get_id_MW(MW_CONFIG) >= 0)
		recheck_window_scale = 1;
	return 1;
}

// Similar to the UI scale handler, when changing the UI font we don't want to
// disrupt the options window. So defer the font change here as well until
// after the window is closed.
static int change_elconfig_font_handler(window_info *win, font_cat cat)
{
	if (cat != UI_FONT)
		return 0;
	if (get_id_MW(MW_CONFIG) >= 0)
		recheck_window_scale = 1;
	return 1;
}

//  Called from the low freqency timer as we can't initiate distorying a window in from one of its call backs.
//  If the scale has changed and the window is hidden, destroy it, it will be re-create with the new scale
void check_for_config_window_scale(void)
{
	int elconfig_win = get_id_MW(MW_CONFIG);
	if (recheck_window_scale && (elconfig_win >= 0) && !get_show_window(elconfig_win))
	{
		size_t i;
		set_pos_MW(MW_CONFIG, windows_list.window[elconfig_win].cur_x, windows_list.window[elconfig_win].cur_y);
		for (i=MAX_TABS; i>0; i--)
			tab_collection_close_tab(elconfig_win, elconfig_tab_collection_id, i-1);
		destroy_window(elconfig_win);
		set_id_MW(MW_CONFIG, -1);
		recheck_window_scale = 0;
	}
}

void display_elconfig_win(void)
{
	int elconfig_win = get_id_MW(MW_CONFIG);

	if(elconfig_win < 0) {
		int i;

		set_config_font();

		elconf_scale = ui_scale * elconf_custom_scale;
		CHECKBOX_SIZE = ELCONFIG_SCALED_VALUE(15);
		SPACING = ELCONFIG_SCALED_VALUE(5);
		LONG_DESC_SPACE = SPACING +
			MAX_LONG_DESC_LINES * get_line_height(CONFIG_FONT, elconf_scale * DEFAULT_SMALL_RATIO);
		TAB_TAG_HEIGHT = tab_collection_calc_tab_height(CONFIG_FONT, elconf_scale);
		elconfig_menu_x_len = 4 * TAB_MARGIN + 4 * SPACING + CHECKBOX_SIZE
			+ 50 * ELCONFIG_SCALED_VALUE(DEFAULT_FIXED_FONT_WIDTH)
			+ ELCONFIG_SCALED_VALUE(ELW_BOX_SIZE);
		elconfig_menu_x_len = get_elconfig_content_width() + 2 * TAB_MARGIN;
		elconfig_menu_y_len = ELCONFIG_SCALED_VALUE(440);

		/* Set up the window */
		elconfig_win = create_window(win_configuration, (not_on_top_now(MW_CONFIG) ?game_root_win : -1), 0, get_pos_x_MW(MW_CONFIG), get_pos_y_MW(MW_CONFIG),
			elconfig_menu_x_len, elconfig_menu_y_len, ELW_WIN_DEFAULT|ELW_USE_UISCALE);
		set_id_MW(MW_CONFIG, elconfig_win);
		if (elconfig_win >=0 && elconfig_win < windows_list.num_windows)
			update_window_scale(&windows_list.window[elconfig_win], elconf_scale);
		check_proportional_move(MW_CONFIG);
		set_window_color(elconfig_win, ELW_COLOR_BORDER, gui_color[0], gui_color[1], gui_color[2], 0.0f);
		set_window_font_category(elconfig_win, CONFIG_FONT);
		set_window_handler(elconfig_win, ELW_HANDLER_DISPLAY, &display_elconfig_handler );
		set_window_handler(elconfig_win, ELW_HANDLER_UI_SCALE, &ui_scale_elconfig_handler );
		set_window_handler(elconfig_win, ELW_HANDLER_FONT_CHANGE, &change_elconfig_font_handler);
		// TODO: replace this hack by something clean.
		set_window_handler(elconfig_win, ELW_HANDLER_SHOW, &show_elconfig_handler);
		/* Create tabs */
		elconfig_tab_collection_id= tab_collection_add_extended (elconfig_win, elconfig_tab_collection_id, NULL,
			TAB_MARGIN, TAB_MARGIN, elconfig_menu_x_len-TAB_MARGIN*2, elconfig_menu_y_len-TAB_MARGIN*2-LONG_DESC_SPACE,
			0, DEFAULT_SMALL_RATIO * elconf_scale, MAX_TABS);
		/* Pass ELW_SCROLLABLE as the final argument to tab_add() if you want
		 * to put more widgets in the tab than the size of the window allows.*/
		elconfig_tabs[CONTROLS].tab= tab_add(elconfig_win, elconfig_tab_collection_id, ttab_controls, 0, 0, ELW_SCROLLABLE|ELW_USE_UISCALE);
		elconfig_tabs[HUD].tab= tab_add(elconfig_win, elconfig_tab_collection_id, ttab_hud, 0, 0, ELW_SCROLLABLE|ELW_USE_UISCALE);
		elconfig_tabs[CHAT].tab= tab_add(elconfig_win, elconfig_tab_collection_id, ttab_chat, 0, 0, ELW_SCROLLABLE|ELW_USE_UISCALE);
		elconfig_tabs[FONT].tab= tab_add(elconfig_win, elconfig_tab_collection_id, ttab_font, 0, 0, ELW_SCROLLABLE|ELW_USE_UISCALE);
		elconfig_tabs[SERVER].tab= tab_add(elconfig_win, elconfig_tab_collection_id, ttab_server, 0, 0, ELW_SCROLLABLE|ELW_USE_UISCALE);
		elconfig_tabs[AUDIO].tab= tab_add(elconfig_win, elconfig_tab_collection_id, ttab_audio, 0, 0, ELW_SCROLLABLE|ELW_USE_UISCALE);
		elconfig_tabs[VIDEO].tab= tab_add(elconfig_win, elconfig_tab_collection_id, ttab_video, 0, 0, ELW_SCROLLABLE|ELW_USE_UISCALE);
		elconfig_tabs[GFX].tab= tab_add(elconfig_win, elconfig_tab_collection_id, ttab_gfx, 0, 0, ELW_SCROLLABLE|ELW_USE_UISCALE);
		elconfig_tabs[CAMERA].tab= tab_add(elconfig_win, elconfig_tab_collection_id, ttab_camera, 0, 0, ELW_SCROLLABLE|ELW_USE_UISCALE);
		elconfig_tabs[TROUBLESHOOT].tab= tab_add(elconfig_win, elconfig_tab_collection_id, ttab_troubleshoot, 0, 0, ELW_SCROLLABLE|ELW_USE_UISCALE);
#ifdef DEBUG
		elconfig_tabs[DEBUGTAB].tab= tab_add(elconfig_win, elconfig_tab_collection_id, "Debug", 0, 0, ELW_SCROLLABLE|ELW_USE_UISCALE);
#endif
		elconfig_populate_tabs();

		/* configure scrolling for tabs */
		for (i=0; i<MAX_TABS; i++)
		{
			/* configure scrolling for any tabs that exceed the window length */
			int window_height = widget_get_height(elconfig_win, elconfig_tab_collection_id) -TAB_TAG_HEIGHT;
			if (elconfig_tabs[i].y > window_height)
			{
				set_window_scroll_len(elconfig_tabs[i].tab, elconfig_tabs[i].y - window_height);
				set_window_scroll_inc(elconfig_tabs[i].tab, TAB_TAG_HEIGHT);
			}
			/* otherwise disable scrolling */
			else
			{
				set_window_scroll_inc(elconfig_tabs[i].tab, 0);
				widget_set_flags(elconfig_tabs[i].tab, windows_list.window[elconfig_tabs[i].tab].scroll_id, WIDGET_DISABLED);
			}
		}
	}
	show_window(elconfig_win);
	select_window(elconfig_win);
}

#ifdef JSON_FILES
// If we have logged in to a character, save any override options.
//
// For each ini file option, check if we have the override value set:
//   If we do, save the value if it is new or changed.
//   If we do not, remove any existing override value.
//
void save_character_options(void)
{
	size_t i;

	if (!get_use_json_user_files() || !ready_for_user_files)
		return;

	for(i = 0; i < our_vars.no; i++)
	{
		var_struct *option = our_vars.var[i];
		if (option->character_override)
		{
			int do_save = 1;
			switch (option->type)
			{
				case OPT_INT:
				case OPT_INT_F:
				case OPT_MULTI:
				case OPT_MULTI_H:
				{
					if (json_character_options_exists(option->name))
						if (*((int *)option->var) == json_character_options_get_int(option->name, *((int *)option->var)))
							do_save = 0;
					if (do_save)
						json_character_options_set_int(option->name, *((int *)option->var));
					break;
				}
				case OPT_BOOL:
				{
					if (json_character_options_exists(option->name))
						if (*((int *)option->var) == json_character_options_get_bool(option->name, *((int *)option->var)))
							do_save = 0;
					if (do_save)
						json_character_options_set_bool(option->name, *((int *)option->var));
					break;
				}
				case OPT_FLOAT:
				{
					if (json_character_options_exists(option->name))
						if (*((float *)option->var) == json_character_options_get_float(option->name, *((float *)option->var)))
							do_save = 0;
					if (do_save)
						json_character_options_set_float(option->name, *((float *)option->var));
					break;
				}
				default:
					break;
			}
		}
		else
		{
			if (json_character_options_exists(option->name))
				json_character_options_remove(option->name);
		}
	}

	json_character_options_save_file();

}


// If we have logged in to a character, check for any override options.
//
// The character_options_<name>.json file can contain character specific
// values for options.  For each ini file option, check if we have an
// override value to use.  Also set the character_override flag for the var
// so that we can show the option as checked when using the context menu
// for the option. We do not set the unsaved state for vars that have
// been overridden so that they are not saved in the ini file.
//
void load_character_options(void)
{
	size_t i;
	char json_fname[128];
	static int already_loaded = 0;
	int have_user_video_mode = 0;

	if (!get_use_json_user_files() || !ready_for_user_files  || already_loaded)
		return;

	safe_snprintf(json_fname, sizeof(json_fname), "%scharacter_options_%s.json", get_path_config(), get_lowercase_username());
	json_character_options_set_file_name(json_fname);
	json_character_options_load_file();

	already_loaded = 1;

	for(i = 0; i < our_vars.no; i++)
	{
		if (json_character_options_exists(our_vars.var[i]->name))
		{
			var_struct *option = our_vars.var[i];
			int last_save = option->saved;
			option->character_override = 1;
			switch (option->type)
			{
				case OPT_INT:
				case OPT_INT_F:
				{
					int new_value = json_character_options_get_int(option->name, *((int *)option->var));
					set_var_OPT_INT(option->name, new_value);
					break;
				}
				case OPT_MULTI:
				case OPT_MULTI_H:
				{
					int new_value = json_character_options_get_int(option->name, *((int *)option->var));
					option->func(option->var, new_value);
					// user defined video mode is a special case as we may set the mode before we have the width/height
					if ((strcmp(option->name, "video_mode") == 0) && (*((int *)option->var) == 0))
						have_user_video_mode = 1;
					break;
				}
				case OPT_BOOL:
				{
					int new_value = json_character_options_get_bool(option->name, *((int *)option->var));
					if (*(int *)option->var != new_value)
					{
						*(int *)option->var = !new_value;
						option->func(option->var);
					}
					break;
				}
				case OPT_FLOAT:
				{
					float new_value = json_character_options_get_float(option->name, *((float *)option->var));
					set_var_OPT_FLOAT(option->name, new_value);
					break;
				}
				default:
					break;
			}
			option->saved = last_save;
		}
	}

	// if we have user defined video mode, try setting the mode again now as we will now have the width and height
	if (have_user_video_mode)
		switch_video(video_mode, full_screen);
}
#endif

#endif //ELC
