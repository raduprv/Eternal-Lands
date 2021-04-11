#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "asc.h"
#include "context_menu.h"
#include "cursors.h"
#include "elconfig.h"
#include "elwindows.h"
#include "errors.h"
#include "gamewin.h"
#include "gl_init.h"
#include "hud.h"
#include "hud_quickspells_window.h"
#include "hud_misc_window.h"
#ifdef JSON_FILES
#include "json_io.h"
#endif
#include "loginwin.h"
#include "io/elpathwrapper.h"
#include "spells.h"
#include "sound.h"

int num_quickspell_slots = 6;
int quickspells_relocatable = 0;
mqbdata * mqb_data[MAX_QUICKSPELL_SLOTS+1]={NULL};//mqb_data will hold the magic quickspells name, image, pos.

static int default_quickspells_x = -1;
static int default_quickspells_y = -1;
static int quickspell_y_space = -1;
static int quickspells_loaded = 0;
static int quickspell_size = -1;
static int quickspell_x_len = -1;
static size_t cm_quickspells_id = CM_INIT_VALUE;
static int quickspell_over=-1;
static int shown_quickspell_slots = -1;
static int quickspells_moveable = 1;
static int quickspells_dir = HORIZONTAL;

enum {	CMQS_UP=0, CMQS_DOWN, CMQS_REMOVE, CMSQ_S1, CMQS_RELOC, CMQS_DRAG, CMQS_FLIP, CMSQ_S2, CMQS_RESET };

// get the quickspell window length - it depends on the number of slots active
static int get_quickspell_y_len(void)
{
	return shown_quickspell_slots * quickspell_y_space + 1;
}


// change window flags
static void change_flags(int win_id, Uint32 flags)
{
	if (win_id >= 0 && win_id < windows_list.num_windows)
	{
		int order = windows_list.window[win_id].order;
		windows_list.window[win_id].flags = flags;
		if ( (order > 0 && (flags & ELW_SHOW_LAST)) || (order < 0 && !(flags & ELW_SHOW_LAST)) )
			windows_list.window[win_id].order = -order;
	}
}


// return window flags
static Uint32 get_flags(int win_id)
{
	return windows_list.window[win_id].flags;
}


// returns true if the window is not in the default place, false if it is, even if it can be relocated
static int is_relocated(void)
{
	int quickspell_win = get_id_MW(MW_QUICKSPELLS);
	window_info *win = NULL;
	if (quickspell_win < 0 || quickspell_win >= windows_list.num_windows)
		return 1;
	win = &windows_list.window[quickspell_win];
	if ((quickspells_moveable) || (quickspells_dir != VERTICAL) ||
		(win->cur_x != default_quickspells_x) || (win->cur_y != default_quickspells_y))
		return 1;
	else
		return 0;
}


// enable/disable window title bar and dragability
static void toggle_quickspells_moveable(void)
{
	int quickspell_win = get_id_MW(MW_QUICKSPELLS);
	Uint32 flags = get_flags(quickspell_win);
	if (!quickspells_moveable)
	{
		flags &= ~ELW_SHOW_LAST;
		flags |= ELW_DRAGGABLE | ELW_TITLE_BAR;
		change_flags (quickspell_win, flags);
		quickspells_moveable = 1;
	}
	else
	{
		flags |= ELW_SHOW_LAST;
		flags &= ~(ELW_DRAGGABLE | ELW_TITLE_BAR);
		change_flags (quickspell_win, flags);
		quickspells_moveable = 0;
	}
}


// return the window to it's default position
static void reset_quickspells()
{
	int quickspell_win = get_id_MW(MW_QUICKSPELLS);
	limit_win_scale_to_default(get_scale_WM(MW_QUICKSPELLS));
	quickspells_dir = VERTICAL;
	quickspells_moveable = 0;
	if (quickspells_relocatable)
	{
		quickspells_relocatable = 0;
		set_var_unsaved("relocate_quickspells", INI_FILE_VAR);
	}
	if (quickspell_win >= 0 && quickspell_win < windows_list.num_windows)
	{
		change_flags(quickspell_win, ELW_USE_UISCALE|ELW_CLICK_TRANSPARENT|ELW_TITLE_NONE|ELW_SHOW_LAST);
		init_window(quickspell_win, -1, 0, default_quickspells_x, default_quickspells_y, quickspell_x_len, get_quickspell_y_len());
	}
}


// common function to resize window depending on orientation
static void resize_quickspells_window(int window_id)
{
	if (quickspells_dir==VERTICAL)
		resize_window(window_id, quickspell_x_len, get_quickspell_y_len());
	else
		resize_window(window_id, get_quickspell_y_len(), quickspell_x_len);
}


// change the window from vertical to horizontal, or vice versa*/
static void flip_quickspells(int window_id)
{
	if (quickspells_dir == VERTICAL)
		quickspells_dir = HORIZONTAL;
	else
		quickspells_dir = VERTICAL;
	resize_quickspells_window(window_id);
}


static void update_shown_quickspell_slots(window_info *win)
{
	int last_shown_slots = shown_quickspell_slots;
	int max_slots = 0;
	int last_active = 0;
	size_t i;

	if (quickspells_relocatable && is_relocated())
		max_slots = num_quickspell_slots;
	else
		max_slots = (window_height - get_min_hud_misc_len_y() - win->cur_y - 1) / quickspell_y_space;

	for(i = 1; i < MAX_QUICKSPELL_SLOTS+1; i++)
		if (mqb_data[i] && mqb_data[i]->spell_name[0])
			last_active = i;

	if (last_active > num_quickspell_slots)
		last_active = num_quickspell_slots;

	if (max_slots > last_active)
		shown_quickspell_slots = last_active;
	else if (max_slots < 1)
		shown_quickspell_slots = 1;
	else
		shown_quickspell_slots = max_slots;

	if (last_shown_slots != shown_quickspell_slots)
		resize_quickspells_window(win->window_id);
}


static void remove_quickspell (int pos)
{
	int i;

	if (pos < 1 || pos > shown_quickspell_slots || mqb_data[pos] == NULL) {
		return;
	}

	// remove the spell
	free (mqb_data[pos]);

	// move the other spells one up
	for (i = pos; i < MAX_QUICKSPELL_SLOTS; i++) {
		mqb_data[i] = mqb_data[i+1];
	}
	mqb_data[MAX_QUICKSPELL_SLOTS] = NULL;
	save_quickspells();
}


static void move_quickspell (int pos, int direction)
{
	int i=pos;
	mqbdata * mqb_temp;
	if (pos < 1 || pos > shown_quickspell_slots || mqb_data[pos] == NULL) return;
	if ((pos ==1 && direction==0)||(pos==shown_quickspell_slots && direction==1)) return;
	if (direction==0){
		mqb_temp=mqb_data[i-1];
		mqb_data[i-1]=mqb_data[i]; //move it up
		mqb_data[i]=mqb_temp; //move it up
		save_quickspells();
	}
	else if(direction==1){
		if(mqb_data[pos+1] == NULL) return;
		mqb_temp=mqb_data[i+1];
		mqb_data[i+1]=mqb_data[i]; //move it down
		mqb_data[i]=mqb_temp; //move it down
		save_quickspells();
	}
}


static int display_quickspell_handler(window_info *win)
{
	int i;

	update_shown_quickspell_slots(win);

#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.20f);
	glEnable(GL_BLEND);	// Turn Blending On
	glBlendFunc(GL_SRC_ALPHA,GL_DST_ALPHA);

	for(i=1;i<shown_quickspell_slots+1;i++) {
		if(mqb_data[i] && mqb_data[i]->spell_name[0]){
			if(quickspell_over==i){	//highlight if we are hovering over
				glColor4f(1.0f,1.0f,1.0f,1.0f);
			} else {	//otherwise shade it a bit
				glColor4f(1.0f,1.0f,1.0f,0.6f);
			}
			if (quickspells_dir == VERTICAL)
				draw_spell_icon(mqb_data[i]->spell_image, 0, (i-1) * quickspell_y_space + (quickspell_y_space - quickspell_size) / 2, quickspell_size,0,0);
			else
				draw_spell_icon(mqb_data[i]->spell_image, (i-1) * quickspell_y_space + (quickspell_y_space - quickspell_size) / 2, 0, quickspell_size,0,0);
		}
	}

	glColor4f(1.0f,1.0f,1.0f,1.0f);
	glDisable(GL_BLEND);	// Turn Blending Off
	glDisable(GL_ALPHA_TEST);

	if(quickspell_over!=-1 && mqb_data[quickspell_over])
	{
		float zoom = win->current_scale_small;
		int x = 0, y = 0;
		int len_str = get_string_width_zoom((const unsigned char*)mqb_data[quickspell_over]->spell_name, win->font_category, zoom)
			+ get_char_width_zoom(' ', win->font_category, zoom);

		// vertical place left (or right) and aligned with slot
		if (quickspells_dir==VERTICAL)
		{
			x = -len_str;
			if (win->cur_x + x < 0)
				x = win->len_x;
			y = (quickspell_over - 1) * quickspell_y_space + (quickspell_y_space - win->small_font_len_y) / 2;
		}
		// horizontal place right at bottom (or top) of window
		else
		{
			x = 0;
			y = win->len_y + 5;
			if ((x + len_str + win->cur_x) > window_width)
				x = window_width - win->cur_x - len_str;
			if ((y + win->small_font_len_y + win->cur_y) > window_height)
				y = -(5 + win->small_font_len_y + (quickspells_moveable * win->title_height));
		}
		show_help(mqb_data[quickspell_over]->spell_name, x, y, win->current_scale);
	}
	quickspell_over=-1;
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE

	return 1;
}


static int mouseover_quickspell_handler(window_info *win, int mx, int my)
{
	int pos;
	if (my < 0)
		return 0;

	if (quickspells_dir == VERTICAL)
		pos=my/quickspell_y_space+1;
	else
		pos=mx/quickspell_y_space+1;
	if(pos<shown_quickspell_slots+1 && pos>=1 && mqb_data[pos] && mqb_data[pos]->spell_name[0]) {
		quickspell_over=pos;
		elwin_mouse=CURSOR_WAND;
		return 1;
	}
	return 0;
}


static int click_quickspell_handler(window_info *win, int mx, int my, Uint32 flags)
{
	int pos;

	if (my < 0)
		return 0;

	if (quickspells_dir == VERTICAL)
		pos=my/quickspell_y_space+1;
	else
		pos=mx/quickspell_y_space+1;

	if(pos<shown_quickspell_slots+1 && pos>=1 && mqb_data[pos])
	{
		if ((flags & ELW_LEFT_MOUSE)&&(flags & KMOD_SHIFT))
		{
			move_quickspell (pos,0);
			return 1;
		}
		else if ((flags & ELW_RIGHT_MOUSE)&&(flags & KMOD_SHIFT))
		{
			move_quickspell (pos,1);
			return 1;
		}
		else if (flags & ELW_LEFT_MOUSE && mqb_data[pos]->spell_str[0])
		{
			send_spell(mqb_data[pos]->spell_str, mqb_data[pos]->spell_str[1]+2);
			return 1;
		}
		else if ((flags & ELW_RIGHT_MOUSE)&&(flags & KMOD_CTRL))
		{
			remove_quickspell (pos);
			return 1;
		}
	}
	return 0;
}


static int context_quickspell_handler(window_info *win, int widget_id, int mx, int my, int option)
{
	int pos = 0;

	if (quickspells_dir == VERTICAL)
		pos=my/quickspell_y_space+1;
	else
		pos=mx/quickspell_y_space+1;

	if(pos<shown_quickspell_slots+1 && pos>=1 && mqb_data[pos])
	{
		switch (option)
		{
			case CMQS_UP: move_quickspell (pos,0); break;
			case CMQS_DOWN: move_quickspell (pos,1); break;
			case CMQS_REMOVE: remove_quickspell (pos); break;
			case CMQS_RELOC: if (quickspells_relocatable) toggle_quickspells_moveable(); break;
			case CMQS_DRAG: quickspells_moveable ^= 1; toggle_quickspells_moveable(); break;
			case CMQS_FLIP: flip_quickspells(win->window_id); break;
			case CMQS_RESET: reset_quickspells(); break;

		}
	}
	return 1;
}


static void context_quickspell_pre_show_handler(window_info *win, int widget_id, int mx, int my, window_info *cm_win)
{
	cm_grey_line(cm_quickspells_id, CMQS_DRAG, (quickspells_relocatable) ?0 :1);
	cm_grey_line(cm_quickspells_id, CMQS_FLIP, (quickspells_relocatable) ?0 :1);
}


static int ui_scale_quickspell_handler(window_info *win)
{
	quickspell_size = (int)(0.5 + win->current_scale * 20);
	quickspell_x_len = (int)(0.5 + win->current_scale * 26);
	default_quickspells_x = window_width - HUD_MARGIN_X + (int)(0.5 + win->current_scale * 4);
	default_quickspells_y = get_hud_logo_size();
	quickspell_y_space = (int)(0.5 + win->current_scale * 30);
	if (!quickspells_relocatable)
		reset_quickspells();
	else
	{
		resize_quickspells_window(win->window_id);
		if (win->cur_x > window_width || win->cur_y > window_height)
		{
			move_window(win->window_id, -1, 0, 100, 100);
			quickspells_moveable = 0;
			toggle_quickspells_moveable();
		}
	}
	update_shown_quickspell_slots(win);
	return 1;
}


void init_quickspell(void)
{
	int quickspell_win = get_id_MW(MW_QUICKSPELLS);
	Uint32 flags = ELW_USE_UISCALE | ELW_CLICK_TRANSPARENT;

	if (!quickspells_relocatable)
	{
		flags |= ELW_SHOW_LAST;
		quickspells_moveable = 0;
	}
	if (quickspells_moveable)
		flags |= ELW_TITLE_BAR | ELW_DRAGGABLE;

	if (quickspell_win < 0){
		quickspell_win = create_window ("Quickspell", -1, 0, get_pos_x_MW(MW_QUICKSPELLS), get_pos_y_MW(MW_QUICKSPELLS), 0, 0, flags);
		set_id_MW(MW_QUICKSPELLS, quickspell_win);
		set_window_custom_scale(quickspell_win, MW_QUICKSPELLS);
		set_window_handler(quickspell_win, ELW_HANDLER_DISPLAY, &display_quickspell_handler);
		set_window_handler(quickspell_win, ELW_HANDLER_CLICK, &click_quickspell_handler);
		set_window_handler(quickspell_win, ELW_HANDLER_MOUSEOVER, &mouseover_quickspell_handler );
		set_window_handler(quickspell_win, ELW_HANDLER_UI_SCALE, &ui_scale_quickspell_handler );

		if (quickspell_win >= 0 && quickspell_win < windows_list.num_windows)
			ui_scale_quickspell_handler(&windows_list.window[quickspell_win]);

		cm_quickspells_id = cm_create(cm_quickspell_menu_str, &context_quickspell_handler);
		cm_add_window(cm_quickspells_id, quickspell_win);
		cm_set_pre_show_handler(cm_quickspells_id, context_quickspell_pre_show_handler);
		cm_bool_line(cm_quickspells_id, CMQS_RELOC, &quickspells_relocatable, "relocate_quickspells");
		cm_bool_line(cm_quickspells_id, CMQS_DRAG, &quickspells_moveable, NULL);
	} else {
		change_flags (quickspell_win, flags);
		if (quickspell_win >= 0 && quickspell_win < windows_list.num_windows)
			ui_scale_quickspell_handler(&windows_list.window[quickspell_win]);
		show_window (quickspell_win);
	}
}


void load_quickspells (void)
{
	char fname[128];
	Uint8 num_spells;
	FILE *fp;
	Uint32 i, index;
#ifdef JSON_FILES
	int json_num_spells = 0;
	int quickspell_ids[MAX_QUICKSPELL_SLOTS];
#endif

	// Grum: move this over here instead of at the end of the function,
	// so that quickspells are always saved when the player logs in.
	// (We're only interested in if this function is called, not if it
	// succeeds)
	quickspells_loaded = 1;

#ifdef JSON_FILES
	if (get_use_json_user_files())
	{
		USE_JSON_DEBUG("Loading json file");
		safe_snprintf(fname, sizeof(fname), "%sspells_%s.json", get_path_config(), get_lowercase_username());
		if ((json_num_spells = json_load_quickspells(fname, quickspell_ids, MAX_QUICKSPELL_SLOTS)) >= 0)
		{
			size_t i;
			memset(mqb_data, 0, sizeof (mqb_data));
			for (i = 0, index = 1; i < json_num_spells; i++)
				if (quickspell_ids[i] >= 0)
					mqb_data[index++] = build_quickspell_data(quickspell_ids[i]);
			return;
		}
	}

	// if there is no json file, or json use disabled, try to load the old binary format
	USE_JSON_DEBUG("Loading binary file");
#endif

	//open the data file
	safe_snprintf(fname, sizeof(fname), "spells_%s.dat",get_lowercase_username());

	/* sliently ignore non existing file */
	if (file_exists_config(fname)!=1)
		return;

	fp = open_file_config(fname,"rb");

	if (fp == NULL)
	{
		LOG_ERROR("%s: %s \"%s\": %s\n", reg_error_str, cant_open_file,
			fname, strerror(errno));
		return;
	}

	if (fread(&num_spells, sizeof(num_spells), 1, fp) != 1)
	{
		LOG_ERROR("%s() read failed for [%s] \n", __FUNCTION__, fname);
		fclose (fp);
		return;
	}

	ENTER_DEBUG_MARK("load spells");

	if (num_spells > 0)
	{
		num_spells--;
	}

	if (num_spells > MAX_QUICKSPELL_SLOTS)
	{
		LOG_WARNING("Too many spells (%d), only %d spells allowed",
			num_spells, MAX_QUICKSPELL_SLOTS);

		num_spells = MAX_QUICKSPELL_SLOTS;
	}

	memset(mqb_data, 0, sizeof (mqb_data));

	LOG_DEBUG("Reading %d spells from file '%s'", num_spells, fname);

	index = 1;

	for (i = 0; i < num_spells; i++)
	{
		mqbdata tmp;

		if (fread(&tmp, sizeof(mqbdata), 1, fp) != 1)
		{
			LOG_ERROR("Failed reading spell %d from file '%s'", i,
				fname);
			continue;
		}

		mqb_data[index] = build_quickspell_data(tmp.spell_id);

		if (mqb_data[index] == 0)
		{
			continue;
		}

		LOG_DEBUG("Added quickspell %d '%s' at index %d", i,
			mqb_data[index]->spell_name, index);

		index++;
	}
	fclose (fp);

	LEAVE_DEBUG_MARK("load spells");
}


void save_quickspells(void)
{
	char fname[128];
	FILE *fp;
	Uint8 i;
	size_t num_spells_to_write = 0;
#ifdef JSON_FILES
	size_t index;
	Uint16 *quickspell_ids = NULL;
#endif

	if (!quickspells_loaded)
		return;

	// get the number of quickspells
	for (i = 1; i < MAX_QUICKSPELL_SLOTS+1; i++)
	{
		if (mqb_data[i] == NULL)
			break;
		num_spells_to_write++;
	}

#ifdef JSON_FILES
	if (get_use_json_user_files())
	{
		USE_JSON_DEBUG("Saving json file");
		// save the quickspell to the json file
		quickspell_ids = malloc(num_spells_to_write * sizeof(Uint16));
		for (index = 0; index < num_spells_to_write; index++)
			quickspell_ids[index] = mqb_data[index+1]->spell_id;
		safe_snprintf(fname, sizeof(fname), "%sspells_%s.json", get_path_config(), get_lowercase_username());
		if (json_save_quickspells(fname, quickspell_ids, num_spells_to_write) < 0)
			LOG_ERROR("%s: %s \"%s\"\n", reg_error_str, cant_open_file, fname);
		free(quickspell_ids);
		return;
	}
	USE_JSON_DEBUG("Saving binary file");
#endif

	//write to the data file, for historical reasons, we will write all the information
	safe_snprintf(fname, sizeof(fname), "spells_%s.dat",get_lowercase_username());
	fp=open_file_config(fname,"wb");
	if(fp == NULL){
		LOG_ERROR("%s: %s \"%s\": %s\n", reg_error_str, cant_open_file, fname, strerror(errno));
		return;
	}

	ENTER_DEBUG_MARK("save spells");

	// write the number of spells + 1
	num_spells_to_write++;
	fwrite(&num_spells_to_write, sizeof(i), 1, fp);

	LOG_DEBUG("Writing %d spells to file '%s'", i, fname);

	for (i = 1; i < (MAX_QUICKSPELL_SLOTS + 1); i++)
	{
		if (mqb_data[i] == 0)
		{
			break;
		}

		if (fwrite(mqb_data[i], sizeof(mqbdata), 1, fp) != 1)
		{
			LOG_ERROR("Failed writing spell '%s' to file '%s'",
				mqb_data[i]->spell_name, fname);
			break;
		}

		LOG_DEBUG("Wrote spell '%s' to file '%s'",
			mqb_data[i]->spell_name, fname);
	}

	fclose(fp);

	LEAVE_DEBUG_MARK("save spells");
}


int action_spell_keys(SDL_Keycode key_code, Uint16 key_mod)
{
	size_t i;
	el_key_def keys[] = {K_SPELL1, K_SPELL2, K_SPELL3, K_SPELL4, K_SPELL5, K_SPELL6,
					 K_SPELL7, K_SPELL8, K_SPELL9, K_SPELL10, K_SPELL11, K_SPELL12 };
	for (i=0; (i<sizeof(keys)/sizeof(el_key_def)) & (i < num_quickspell_slots); i++)
		if(KEY_DEF_CMP(keys[i], key_code, key_mod))
		{
			if(mqb_data[i+1] && mqb_data[i+1]->spell_str[0])
				send_spell(mqb_data[i+1]->spell_str, mqb_data[i+1]->spell_str[1]+2);
			return 1;
		}
	return 0;
}


/*	returns the y coord position of the active base
	of the quickspell window.  If spell slots are unused
	the base is higher */
int get_quickspell_y_base(void)
{
	if ((!quickspells_loaded) || is_relocated())
		return default_quickspells_y;
	else
	{
		int active_len = default_quickspells_y + get_quickspell_y_len();
		int i;

		for (i = shown_quickspell_slots; i > 0; i--)
		{
			if (mqb_data[i] == NULL)
				active_len -= quickspell_y_space;
			else
				break;
		}
		return active_len;
	}
}


void add_quickspell(void)
{
	int i;

	if(!mqb_data[0])
		return;

	for(i=1;i<num_quickspell_slots+1;i++) {
		if(mqb_data[i] && mqb_data[0]->spell_id==mqb_data[i]->spell_id) {
			do_alert1_sound();
			return;
		}
	}

	do_click_sound();

	for (i = 1; i < num_quickspell_slots+1; i++)
	{
		if (mqb_data[i] == NULL)
		{
			// Free slot
			mqb_data[i] = calloc(1, sizeof (mqbdata));
			break;
		}
	}

	if (i >= num_quickspell_slots+1)
		// No free slot, overwrite the last entry
		i = num_quickspell_slots;

	memcpy (mqb_data[i], mqb_data[0], sizeof (mqbdata));
	save_quickspells();
}


// if relocatable, save the position and options to the cfg file
void get_quickspell_options(unsigned int *options, unsigned int *position)
{
	int quickspell_win = get_id_MW(MW_QUICKSPELLS);
	if (quickspell_win >= 0 && quickspell_win < windows_list.num_windows)
		*position = windows_list.window[quickspell_win].cur_x | (windows_list.window[quickspell_win].cur_y << 16);
	else
		*position = get_pos_x_MW(MW_QUICKSPELLS) | (get_pos_y_MW(MW_QUICKSPELLS) << 16);
	*options = (quickspells_dir & 1) | ((quickspells_moveable & 1) << 1);
}


// if relocatable, set position and options from the cfg file
void set_quickspell_options(unsigned int options, unsigned int position)
{
	if (quickspells_relocatable)
	{
		set_pos_MW(MW_QUICKSPELLS, position & 0xFFFF, position >> 16);
		quickspells_dir = options & 1;
		quickspells_moveable = (options & 2) >> 1;
	}
	else
		reset_quickspells();
}


#ifdef JSON_FILES
void read_quickspell_options(const char *dict_name)
{
	if (quickspells_relocatable)
	{
		quickspells_dir = (json_cstate_get_bool(dict_name, "vertical", 1) == VERTICAL) ?VERTICAL : HORIZONTAL;
		quickspells_moveable = json_cstate_get_bool(dict_name, "movable", 0);
	}
	else
		reset_quickspells();
}


void write_quickspell_options(const char *dict_name)
{
	json_cstate_set_bool(dict_name, "vertical", (quickspells_dir == VERTICAL) ?1 :0);
	json_cstate_set_bool(dict_name, "movable", quickspells_moveable);
}
#endif
