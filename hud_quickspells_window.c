#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "asc.h"
#include "context_menu.h"
#include "cursors.h"
#include "elconfig.h"
#include "elwindows.h"
#include "errors.h"
#include "gl_init.h"
#include "hud.h"
#include "hud_quickspells_window.h"
#include "interface.h"
#include "io/elpathwrapper.h"
#include "spells.h"

int quickspell_win = -1;
int num_quickspell_slots = 6;
mqbdata * mqb_data[MAX_QUICKSPELL_SLOTS+1]={NULL};//mqb_data will hold the magic quickspells name, image, pos.

static int quickspell_x = -1;
static int quickspell_y = -1;
static int quickspell_y_space = -1;
static int quickspells_loaded = 0;
static int quickspell_size = -1;
static int quickspell_x_len = -1;
static size_t cm_quickspells_id = CM_INIT_VALUE;
static int quickspell_over=-1;


static int get_shown_quickspell_slots(void)
{
	return num_quickspell_slots;
}


// get the quickspell window length - it depends on the number of slots active
static int get_quickspell_y_len(void)
{
	return get_shown_quickspell_slots() * quickspell_y_space + 1;
}


static void cm_update_quickspells(void)
{
	int active_y_len = 0, i;
	if (quickspell_win < 0)
		return;
	for (i = get_shown_quickspell_slots(); i > 0; i--)
	{
		if (mqb_data[i] != NULL)
			active_y_len += quickspell_y_space;
	}
	cm_remove_regions(quickspell_win);
	cm_add_region(cm_quickspells_id, quickspell_win, 0, 0, quickspell_x_len, active_y_len);
}


static void remove_quickspell (int pos)
{
	int i;

	if (pos < 1 || pos > get_shown_quickspell_slots() || mqb_data[pos] == NULL) {
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
	cm_update_quickspells();
}


static void move_quickspell (int pos, int direction)
{
	int i=pos;
	mqbdata * mqb_temp;
	if (pos < 1 || pos > get_shown_quickspell_slots() || mqb_data[pos] == NULL) return;
	if ((pos ==1 && direction==0)||(pos==get_shown_quickspell_slots() && direction==1)) return;
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
	static int last_shown_quickspell_slots = -1;

	// Check for a change of the number of quickspells slots
	if (last_shown_quickspell_slots == -1)
		last_shown_quickspell_slots = get_shown_quickspell_slots();
	else if (last_shown_quickspell_slots != get_shown_quickspell_slots())
	{
		last_shown_quickspell_slots = get_shown_quickspell_slots();
		init_window(win->window_id, -1, 0, win->cur_x, win->cur_y, win->len_x, get_quickspell_y_len());
		cm_update_quickspells();
	}

#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.20f);
	glEnable(GL_BLEND);	// Turn Blending On
	glBlendFunc(GL_SRC_ALPHA,GL_DST_ALPHA);

	for(i=1;i<get_shown_quickspell_slots()+1;i++) {
		if(mqb_data[i] && mqb_data[i]->spell_name[0]){
			if(quickspell_over==i){	//highlight if we are hovering over
				glColor4f(1.0f,1.0f,1.0f,1.0f);
			} else {	//otherwise shade it a bit
				glColor4f(1.0f,1.0f,1.0f,0.6f);
			}
			draw_spell_icon(mqb_data[i]->spell_image, 0, (i-1) * quickspell_y_space + (quickspell_y_space - quickspell_size) / 2, quickspell_size,0,0);
		}
	}

	glColor4f(1.0f,1.0f,1.0f,1.0f);
	glDisable(GL_BLEND);	// Turn Blending Off
	glDisable(GL_ALPHA_TEST);

	if(quickspell_over!=-1 && mqb_data[quickspell_over])
		show_help(mqb_data[quickspell_over]->spell_name,
			-((strlen(mqb_data[quickspell_over]->spell_name) + 1) * win->small_font_len_x),
			(quickspell_over - 1) * quickspell_y_space + (quickspell_y_space - win->small_font_len_y) / 2, win->current_scale);
	quickspell_over=-1;
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE

	return 1;
}


static int mouseover_quickspell_handler(window_info *win, int mx, int my)
{
	int pos;

	pos=my/quickspell_y_space+1;
	if(pos<get_shown_quickspell_slots()+1 && pos>=1 && mqb_data[pos] && mqb_data[pos]->spell_name[0]) {
		quickspell_over=pos;
		elwin_mouse=CURSOR_WAND;
		return 1;
	}
	return 0;
}


static int click_quickspell_handler(window_info *win, int mx, int my, Uint32 flags)
{
	int pos;

	pos=my/quickspell_y_space+1;

	if(pos<get_shown_quickspell_slots()+1 && pos>=1 && mqb_data[pos])
	{
		if ((flags & ELW_LEFT_MOUSE)&&(flags & ELW_SHIFT))
		{
			move_quickspell (pos,0);
			return 1;
		}
		else if ((flags & ELW_RIGHT_MOUSE)&&(flags & ELW_SHIFT))
		{
			move_quickspell (pos,1);
			return 1;
		}
		else if (flags & ELW_LEFT_MOUSE && mqb_data[pos]->spell_str[0])
		{
			send_spell(mqb_data[pos]->spell_str, mqb_data[pos]->spell_str[1]+2);
			return 1;
		}
		else if ((flags & ELW_RIGHT_MOUSE)&&(flags & ELW_CTRL))
		{
			remove_quickspell (pos);
			return 1;
		}
	}
	return 0;
}


static int context_quickspell_handler(window_info *win, int widget_id, int mx, int my, int option)
{
	int pos=my/quickspell_y_space+1;
	if(pos<get_shown_quickspell_slots()+1 && pos>=1 && mqb_data[pos])
	{
		switch (option)
		{
			case 0: move_quickspell (pos,0); break;
			case 1: move_quickspell (pos,1); break;
			case 2: remove_quickspell (pos); break;
		}
	}
	return 1;
}


static int ui_scale_quickspell_handler(window_info *win)
{
	quickspell_size = (int)(0.5 + win->current_scale * 20);
	quickspell_x_len = (int)(0.5 + win->current_scale * 26);
	quickspell_x = (int)(0.5 + win->current_scale * 60);
	quickspell_y = get_hud_logo_size();
	quickspell_y_space = (int)(0.5 + win->current_scale * 30);
	resize_window(win->window_id, quickspell_x_len, get_quickspell_y_len());
	move_window (win->window_id, -1, 0, window_width - quickspell_x, quickspell_y);
	return 1;
}


void init_quickspell(void)
{
	if (quickspell_win < 0){
		quickspell_win = create_window ("Quickspell", -1, 0, 0, 0, 0, 0, ELW_USE_UISCALE|ELW_CLICK_TRANSPARENT|ELW_TITLE_NONE|ELW_SHOW_LAST);
		set_window_handler(quickspell_win, ELW_HANDLER_DISPLAY, &display_quickspell_handler);
		set_window_handler(quickspell_win, ELW_HANDLER_CLICK, &click_quickspell_handler);
		set_window_handler(quickspell_win, ELW_HANDLER_MOUSEOVER, &mouseover_quickspell_handler );
		set_window_handler(quickspell_win, ELW_HANDLER_UI_SCALE, &ui_scale_quickspell_handler );

		if (quickspell_win >= 0 && quickspell_win < windows_list.num_windows)
			ui_scale_quickspell_handler(&windows_list.window[quickspell_win]);

		cm_quickspells_id = cm_create(cm_quickspell_menu_str, &context_quickspell_handler);
	} else {
		if (quickspell_win >= 0 && quickspell_win < windows_list.num_windows)
			ui_scale_quickspell_handler(&windows_list.window[quickspell_win]);
		show_window (quickspell_win);
	}
	cm_update_quickspells();
}


void load_quickspells (void)
{
	char fname[128];
	Uint8 num_spells;
	FILE *fp;
	Uint32 i, index;

	// Grum: move this over here instead of at the end of the function,
	// so that quickspells are always saved when the player logs in.
	// (We're only interested in if this function is called, not if it
	// succeeds)
	quickspells_loaded = 1;

	//open the data file
	safe_snprintf(fname, sizeof(fname), "spells_%s.dat",username_str);
	my_tolower(fname);

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

	cm_update_quickspells();

	LEAVE_DEBUG_MARK("load spells");
}


void save_quickspells(void)
{
	char fname[128];
	FILE *fp;
	Uint8 i;

	if (!quickspells_loaded)
		return;

	//write to the data file, to ensure data integrity, we will write all the information
	safe_snprintf(fname, sizeof(fname), "spells_%s.dat",username_str);
	my_tolower(fname);
	fp=open_file_config(fname,"wb");
	if(fp == NULL){
		LOG_ERROR("%s: %s \"%s\": %s\n", reg_error_str, cant_open_file, fname, strerror(errno));
		return;
	}

	for (i = 1; i < MAX_QUICKSPELL_SLOTS+1; i++)
	{
		if (mqb_data[i] == NULL)
			break;
	}

	ENTER_DEBUG_MARK("save spells");

	// write the number of spells + 1
	fwrite(&i, sizeof(i), 1, fp);

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


int shorten_quickspell(void)
{
	if (num_quickspell_slots > 1)
	{
		num_quickspell_slots--;
		set_var_OPT_INT("num_quickspell_slots", num_quickspell_slots);
		return 1;
	}
	else
		return 0;
}


int action_spell_keys(Uint32 key)
{
	size_t i;
	Uint32 keys[] = {K_SPELL1, K_SPELL2, K_SPELL3, K_SPELL4, K_SPELL5, K_SPELL6,
					 K_SPELL7, K_SPELL8, K_SPELL9, K_SPELL10, K_SPELL11, K_SPELL12 };
	for (i=0; (i<sizeof(keys)/sizeof(Uint32)) & (i < get_shown_quickspell_slots()); i++)
		if(key == keys[i])
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
	int active_len = quickspell_y + get_quickspell_y_len();
	int i;

	if (!quickspells_loaded)
		return quickspell_y;

	for (i = get_shown_quickspell_slots(); i > 0; i--)
	{
		if (mqb_data[i] == NULL)
			active_len -= quickspell_y_space;
		else
			break;
	}
	return active_len;
}


void add_quickspell(void)
{
	int i;

	if(!mqb_data[0])
		return;

	for(i=1;i<get_shown_quickspell_slots()+1;i++) {
		if(mqb_data[i] && mqb_data[0]->spell_id==mqb_data[i]->spell_id) {
			return;
		}
	}

	for (i = 1; i < get_shown_quickspell_slots()+1; i++)
	{
		if (mqb_data[i] == NULL)
		{
			// Free slot
			mqb_data[i] = calloc(1, sizeof (mqbdata));
			break;
		}
	}

	if (i >= get_shown_quickspell_slots()+1)
		// No free slot, overwrite the last entry
		i = get_shown_quickspell_slots();

	memcpy (mqb_data[i], mqb_data[0], sizeof (mqbdata));
	save_quickspells();
	cm_update_quickspells();
}

