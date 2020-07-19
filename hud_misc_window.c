#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "asc.h"
#include "consolewin.h"
#include "context_menu.h"
#include "elconfig.h"
#include "elwindows.h"
#include "font.h"
#include "gamewin.h"
#include "gl_init.h"
#include "hud.h"
#include "hud_indicators.h"
#include "hud_misc_window.h"
#include "hud_quickbar_window.h"
#include "hud_quickspells_window.h"
#include "hud_statsbar_window.h"
#include "hud_timer.h"
#include "init.h"
#include "interface.h"
#include "knowledge.h"
#include "mapwin.h"
#include "minimap.h"
#include "missiles.h"
#include "multiplayer.h"
#include "new_character.h"
#include "sound.h"
#include "spells.h"
#include "stats.h"
#include "textures.h"


int misc_win = -1;
int view_analog_clock = 1;
int view_digital_clock = 0;
int view_knowledge_bar = 1;
int view_hud_timer = 1;
int show_stats_in_hud = 0;
int show_statbars_in_hud = 0;
int copy_next_LOCATE_ME = 0;


static size_t cm_hud_id = CM_INIT_VALUE;
static int cm_sound_enabled = 0;
static int cm_music_enabled = 0;
static int cm_minimap_shown = 0;
static int cm_rangstats_shown = 0;
static int first_disp_stat = 0;					/* first skill that will be display */
static int num_disp_stat = NUM_WATCH_STAT-1;		/* number of skills to be displayed */
static int stat_mouse_is_over = -1;				/* set to stat of the is mouse over that bar */
static int mouse_over_clock = 0;					/* 1 if mouse is over digital or analogue clock */
static int mouse_over_compass = 0;					/* 1 if mouse is over the compass */
static int mouse_over_knowledge_bar = 0;			/* 1 if mouse is over the knowledge bar */
static int knowledge_bar_height = 0;
static float side_stats_bar_text_zoom = 0.0f;
static int side_stats_bar_text_width = 0;
static int side_stats_bar_text_height = 0;
static int side_stats_bar_text_offset = 0;
static int side_stats_bar_height = 0;
static int quickbar_misc_sep = 5;
static float digital_clock_zoom = 0.0f;
static int digital_clock_height = 0;
static int analog_clock_size = 0;
static int compass_size = 0;
static const int min_side_stats_bar = 5;


enum {	CMH_STATS=0, CMH_STATBARS, CMH_KNOWBAR, CMH_TIMER, CMH_DIGCLOCK, CMH_ANACLOCK,
		CMH_SECONDS, CMH_FPS, CMH_INDICATORS, CMH_QUICKBM, CMH_SEP1, CMH_MINIMAP, CMH_RANGSTATS,
		CMH_SEP2, CMH_SOUND, CMH_MUSIC, CMH_SEP3, CMH_LOCATION };


static void draw_side_stats_bar(window_info *win, const int x, const int y, const int baselev, const int cur_exp, const int nl_exp, size_t colour)
{
	const int max_len = win->len_x - x - 1;
	const int bar_height = side_stats_bar_text_height + 2;
	int len = max_len - (float)max_len/(float)((float)(nl_exp-exp_lev[baselev])/(float)(nl_exp-cur_exp));

	GLfloat colours[2][2][3] = { { {0.11f, 0.11f, 0.11f}, {0.3f, 0.5f, 0.2f} },
								 { {0.10f,0.10f,0.80f}, {0.40f,0.40f,1.00f} } };

	if (colour > 1)
		colour = 0;

#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	glDisable(GL_TEXTURE_2D);
	if(len >= 0){
		glBegin(GL_QUADS);
		//draw the colored section
		glColor3fv(colours[colour][0]);
		glVertex3i(x, y+bar_height, 0);
		glColor3fv(colours[colour][1]);
		glVertex3i(x, y, 0);
		glColor3fv(colours[colour][1]);
		glVertex3i(x+len, y, 0);
		glColor3fv(colours[colour][0]);
		glVertex3i(x+len, y+bar_height, 0);
		glEnd();
	}

	// draw the bar frame
	glColor3f(0.77f, 0.57f, 0.39f);
	glBegin(GL_LINE_LOOP);
	glVertex3i(x, y, 0);
	glVertex3i(x+max_len, y, 0);
	glVertex3i(x+max_len, y+bar_height, 0);
	glVertex3i(x, y+bar_height, 0);
	glEnd();
	glEnable(GL_TEXTURE_2D);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}


static int context_hud_handler(window_info *win, int widget_id, int mx, int my, int option)
{
	unsigned char protocol_name;
	switch (option)
	{
		case CMH_MINIMAP: view_window(MW_MINIMAP); break;
		case CMH_RANGSTATS: view_window(MW_RANGING); break;
#ifdef NEW_SOUND
		case CMH_SOUND: toggle_sounds(&sound_on); set_var_unsaved("enable_sound", INI_FILE_VAR); break;
		case CMH_MUSIC: toggle_music(&music_on); set_var_unsaved("enable_music", INI_FILE_VAR); break;
#endif // NEW_SOUND
		case CMH_LOCATION:
			copy_next_LOCATE_ME = 1;
			protocol_name= LOCATE_ME;
			my_tcp_send(my_socket,&protocol_name,1);
			break;
		default:
			init_misc_display();
	}
	return 1;
}


static void context_hud_pre_show_handler(window_info *win, int widget_id, int mx, int my, window_info *cm_win)
{
#ifdef NEW_SOUND
	cm_sound_enabled = sound_on;
	cm_music_enabled = music_on;
#endif // NEW_SOUND
	cm_minimap_shown = get_show_window_MW(MW_MINIMAP);
	cm_rangstats_shown = get_show_window_MW(MW_RANGING);
}


/*	Calculate num_disp_stat and first_disp_stat
*/
static void calc_statbar_shown(int base_y_pos, float scale)
{
	int last_display = num_disp_stat;
	int quickspell_base = get_quickspell_y_base();
	int quickbar_base = get_quickbar_y_base() + (int)(0.5 + quickbar_misc_sep);
	int highest_win_pos_y = base_y_pos - side_stats_bar_height*(NUM_WATCH_STAT-1);

	/* calculate the overlap between the longest of the quickspell/bar and the statsbar */
	int winoverlap = ((quickspell_base > quickbar_base) ?quickspell_base :quickbar_base) - highest_win_pos_y;

	/* if they overlap, only display some skills and allow to scroll */
	if (winoverlap > 0)
	{
		num_disp_stat = (NUM_WATCH_STAT-1) - (winoverlap + side_stats_bar_height-1)/side_stats_bar_height;
		if ((first_disp_stat + num_disp_stat) > (NUM_WATCH_STAT-1))
			first_disp_stat = (NUM_WATCH_STAT-1) - num_disp_stat;
	}
	/* else display them all */
	else
	{
		first_disp_stat = 0;
		num_disp_stat = (NUM_WATCH_STAT-1);
	}

	/* if the number of stats displayed has changed, resize the window */
	if (last_display != num_disp_stat)
		init_misc_display();
}


static void reset_cm_regions(void)
{
	cm_remove_regions(game_root_win);
	cm_remove_regions(get_id_MW(MW_TABMAP));
	cm_remove_regions(get_id_MW(MW_CONSOLE));
	cm_add_region(cm_hud_id, game_root_win, window_width-hud_x, 0, hud_x, window_height);
	cm_add_region(cm_hud_id, get_id_MW(MW_TABMAP), window_width-hud_x, 0, hud_x, window_height);
	cm_add_region(cm_hud_id, get_id_MW(MW_CONSOLE), window_width-hud_x, 0, hud_x, window_height);
}


static int display_misc_handler(window_info *win)
{
	const int tooltip_sep = (int)(0.5 + win->current_scale * 5);
	const int scaled_5 = (int)(0.5 + win->current_scale * 5);
	const int scaled_28 = (int)(0.5 + win->current_scale * 28);
	int base_y_start = win->len_y;

	const float compass_u_start = (float)32/256;
	const float compass_v_start = (float)193/256;
	const float compass_u_end = (float)95/256;
	const float compass_v_end = 1.0f;
	const float needle_u_start = (float)4/256;
	const float needle_v_start = (float)201/256;
	const float needle_u_end = (float)14/256;
	const float needle_v_end = (float)247/256;

#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE

	bind_texture(hud_text);

	// allow for transparency
	glEnable(GL_ALPHA_TEST);//enable alpha filtering, so we have some alpha key
	glAlphaFunc(GL_GREATER, 0.09f);

	//draw the compass
	base_y_start -= compass_size;
	glBegin(GL_QUADS);
	draw_2d_thing(compass_u_start, compass_v_start, compass_u_end, compass_v_end, 0, base_y_start, compass_size, base_y_start + compass_size);
	glEnd();

	//draw the compass needle
	glPushMatrix();
	glTranslatef(compass_size/2, base_y_start + compass_size/2, 0);
	glRotatef(rz, 0.0f, 0.0f, 1.0f);
	glBegin(GL_QUADS);
	draw_2d_thing(needle_u_start, needle_v_start, needle_u_end, needle_v_end, -scaled_5, -scaled_28, scaled_5, scaled_28);
	glEnd();
	glPopMatrix();

	//draw the clock
	if(view_analog_clock > 0)
	{
		const int scaled_4 = (int)(0.5 + win->current_scale * 4);
		const int scaled_6 = (int)(0.5 + win->current_scale * 6);
		const float clock_u_start = 0.0f;
		const float clock_v_start = (float)128/256;
		const float clock_u_end = (float)63/256;
		const float clock_v_end = (float)191/256;
		const float clock_needle_u_start = (float)21/256;
		const float clock_needle_v_start = (float)193/256;
		const float clock_needle_u_end = (float)31/256;
		const float clock_needle_v_end = (float)223/256;
		const int scaled_24 = (int)(0.5 + win->current_scale * 24);
		base_y_start -= analog_clock_size;
		// draw the clock face
		glBegin(GL_QUADS);
		draw_2d_thing(clock_u_start, clock_v_start, clock_u_end, clock_v_end,0, base_y_start, analog_clock_size, base_y_start + analog_clock_size);
		glEnd();
		//draw the clock needle
		glAlphaFunc(GL_GREATER, 0.05f);
		glPushMatrix();
		glTranslatef(analog_clock_size/2, base_y_start + analog_clock_size/2, 0);
		glRotatef(real_game_minute, 0.0f, 0.0f, 1.0f);
		glBegin(GL_QUADS);
		draw_2d_thing(clock_needle_u_start, clock_needle_v_start, clock_needle_u_end, clock_needle_v_end, -scaled_4, -scaled_24, scaled_6, scaled_6);
		glEnd();
		glPopMatrix();
		glDisable(GL_ALPHA_TEST);
	}

#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE

	//Digital Clock
	if(view_digital_clock > 0)
	{
		char str[10];

		base_y_start -= digital_clock_height;
		// When seconds are shown, we keep the single minutes character fixed in the window,
		// and draw the rest of the string around it. If we would simply center the whole time
		// string, the entire string will move horizontally every second, which is visually
		// distracting. Now the horizontal alignment only changes once per minute.
		// If seconds are not drawn, we simply center the time.
		if (show_game_seconds)
		{
			int x;
			safe_snprintf(str, sizeof(str), "%1d:%02d:%02d", real_game_minute/60, real_game_minute%60, real_game_second);
			x = win->len_x / 2
				- get_buf_width_zoom((const unsigned char*)str, 4, win->font_category, digital_clock_zoom)
				+ get_char_width_zoom(str[3], win->font_category, digital_clock_zoom) / 2;
			draw_text(x, base_y_start, (const unsigned char*)str, strlen(str),
				win->font_category, TDO_SHADOW, 1, TDO_FOREGROUND, 0.77, 0.57, 0.39,
				TDO_BACKGROUND, 0.0, 0.0, 0.0, TDO_ZOOM, digital_clock_zoom, TDO_END);
		}
		else
		{
			safe_snprintf(str, sizeof(str), "%1d:%02d", real_game_minute/60, real_game_minute%60);
			draw_text(win->len_x/2, base_y_start, (const unsigned char*)str, strlen(str),
				win->font_category, TDO_SHADOW, 1, TDO_FOREGROUND, 0.77, 0.57, 0.39,
				TDO_BACKGROUND, 0.0, 0.0, 0.0, TDO_ALIGNMENT, CENTER, TDO_ZOOM, digital_clock_zoom,
				TDO_END);
		}
	}

	/* if mouse over the either of the clocks - display the time & date */
	if (mouse_over_clock)
	{
		char str[20];
		const char *the_date = get_date(NULL);
		int centre_y =  (view_analog_clock) ?win->len_y - compass_size - analog_clock_size/2 : base_y_start + digital_clock_height/2;

		safe_snprintf(str, sizeof(str), "%1d:%02d:%02d", real_game_minute/60, real_game_minute%60, real_game_second);
		draw_string_small_shadowed_zoomed_right(-tooltip_sep, centre_y-win->small_font_len_y,
			(const unsigned char*)str, 1, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
			win->current_scale);
		if (the_date != NULL)
		{
			safe_snprintf(str, sizeof(str), "%s", the_date);
			draw_string_small_shadowed_zoomed_right(-tooltip_sep, centre_y, (const unsigned char*)str,
				1, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, win->current_scale);
		}
		mouse_over_clock = 0;
	}

	/* if mouse over the compass - display the coords */
	if (mouse_over_compass)
	{
		char str[12];
		actor *me = get_our_actor ();
		if (me != NULL)
		{
			safe_snprintf(str, sizeof(str), "%d,%d", me->x_tile_pos, me->y_tile_pos);
			draw_string_small_shadowed_zoomed_right(-tooltip_sep, win->len_y-compass_size,
				(unsigned char*)str, 1, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
				win->current_scale);
		}
		mouse_over_compass = 0;
	}

	/* if the knowledge bar is enabled, show progress in bar and ETA as hover over */
	if (view_knowledge_bar)
	{
		char str[20];
		char *use_str = idle_str;
		int percentage_done = 0;
		int x = (int)(0.5 + win->current_scale * 3);
		int y = base_y_start - side_stats_bar_height - ((knowledge_bar_height - side_stats_bar_height) / 2);

		if (is_researching())
		{
			percentage_done = (int)(100 * get_research_fraction());
			safe_snprintf(str, sizeof(str), "%d%%", percentage_done);
			use_str = str;
		}
		draw_side_stats_bar(win, x, y, 0, percentage_done, 100, 1);
		draw_string_shadowed_zoomed_centered((x + win->len_x - 1) / 2, y,
			(unsigned char *)use_str, 1,1.0f,1.0f,1.0f,0.0f,0.0f,0.0f,
			side_stats_bar_text_zoom);

		if (mouse_over_knowledge_bar)
		{
			use_str = (is_researching()) ?get_research_eta_str(str, sizeof(str)) : not_researching_str;
			draw_text(-tooltip_sep, y + side_stats_bar_height / 2, (const unsigned char*)use_str,
				strlen(use_str), win->font_category, TDO_SHADOW, 1, TDO_FOREGROUND, 1.0, 1.0, 1.0,
				TDO_BACKGROUND, 0.0, 0.0, 0.0, TDO_ZOOM, win->current_scale_small, TDO_ALIGNMENT, RIGHT,
				TDO_VERTICAL_ALIGNMENT, CENTER_LINE, TDO_END);
			mouse_over_knowledge_bar = 0;
		}

		base_y_start -= knowledge_bar_height;
	}

	/* if the timer is visible, draw it */
	base_y_start -= display_timer(win, base_y_start, tooltip_sep);

	/*	Optionally display the stats bar.  If the current window size does not
		provide enough room, display only some skills and allow scrolling to view
		the rest */
	if(show_stats_in_hud && have_stats)
	{
		char str[20];
		int box_x = (int)(0.5 + win->current_scale * 3);
		int text_x_center = box_x + (win->len_x - box_x - 1) / 2;
		int text_x_left, text_x_right;
		int thestat;
		int y = 0;
		int skill_modifier;

		// trade the number of quickbar slots if there is not enough space for the minimum stats
		calc_statbar_shown(base_y_start + win->pos_y, win->current_scale);

		text_x_left = text_x_center - side_stats_bar_text_width / 2;
		text_x_right = text_x_left + side_stats_bar_text_width;

#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE

		for (thestat=0; thestat<NUM_WATCH_STAT-1; thestat++)
		{
			static const float lbl_color[6] = { 0.77f, 0.57f, 0.39f, 1.0f, 1.0f, 1.0f };

			int hover_offset = 0;
			const float *col;

			/* skill skills until we have the skill displayed first */
			if (thestat < first_disp_stat)
				continue;

			/* end now if we have display all we can */
			if (thestat > (first_disp_stat+num_disp_stat-1))
				break;

			if (show_statbars_in_hud)
				draw_side_stats_bar(win, box_x, y, statsinfo[thestat].skillattr->base,
					*statsinfo[thestat].exp, *statsinfo[thestat].next_lev, 0);

			col = (statsinfo[thestat].is_selected == 1) ? lbl_color : lbl_color + 3;
			draw_text(text_x_left, y + 1 + side_stats_bar_text_offset,
				statsinfo[thestat].skillnames->shortname,
				strlen((const char*)statsinfo[thestat].skillnames->shortname), win->font_category,
				TDO_SHADOW, 1, TDO_FOREGROUND, col[0], col[1], col[2], TDO_BACKGROUND, 0.0, 0.0, 0.0,
				TDO_ZOOM, side_stats_bar_text_zoom, TDO_END);
			safe_snprintf(str, sizeof(str), "%3d", statsinfo[thestat].skillattr->base);
			draw_text(text_x_right, y + 1 + side_stats_bar_text_offset, (const unsigned char*)str,
				strlen(str), win->font_category, TDO_SHADOW, 1, TDO_FOREGROUND, col[0], col[1], col[2],
				TDO_BACKGROUND, 0.0, 0.0, 0.0, TDO_ZOOM, side_stats_bar_text_zoom, TDO_ALIGNMENT, RIGHT,
				TDO_END);

			if((thestat!=NUM_WATCH_STAT-2) && floatingmessages_enabled &&
				(skill_modifier = statsinfo[thestat].skillattr->cur -
				 	statsinfo[thestat].skillattr->base) != 0)
			{
				int ytop = y + (side_stats_bar_height - win->small_font_len_y) / 2;
				safe_snprintf(str,sizeof(str),"%+i",skill_modifier);
				hover_offset = get_string_width_zoom((const unsigned char*)str,
					win->font_category, win->current_scale_small);
				if(skill_modifier > 0){
					draw_string_small_shadowed_zoomed_right(-tooltip_sep, ytop,
						(const unsigned char*)str, 1,0.3f, 1.0f, 0.3f,0.0f,0.0f,0.0f,
						win->current_scale);
				} else {
					draw_string_small_shadowed_zoomed_right(-tooltip_sep, ytop,
						(const unsigned char*)str, 1,1.0f, 0.1f, 0.2f,0.0f,0.0f,0.0f,
						win->current_scale);
				}
			}

			/* if the mouse is over the stat bar, draw the XP remaining */
			if (stat_mouse_is_over == thestat)
			{
				safe_snprintf(str,sizeof(str),"%li",(*statsinfo[thestat].next_lev - *statsinfo[thestat].exp));
				draw_text(-hover_offset-tooltip_sep, y + side_stats_bar_height / 2,
					(const unsigned char*)str, strlen(str), win->font_category, TDO_SHADOW, 1,
					TDO_FOREGROUND, 1.0, 1.0, 1.0, TDO_BACKGROUND, 0.0, 0.0, 0.0,
					TDO_ZOOM, win->current_scale_small, TDO_ALIGNMENT, RIGHT,
					TDO_VERTICAL_ALIGNMENT, CENTER_LINE, TDO_END);
				stat_mouse_is_over = -1;
			}

			y+=side_stats_bar_height;
		}

		base_y_start -= y;
	}

	// check if we need to resize the window
	if (base_y_start != 0)
		init_misc_display();

	{
		static int last_window_width = -1;
		static int last_window_height = -1;
		static int last_hud_x = -1;
		if (window_width != last_window_width || window_height != last_window_height || last_hud_x != hud_x)
		{
			reset_cm_regions();
			last_window_width = window_width;
			last_window_height = window_height;
			last_hud_x = hud_x;
		}
	}

#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	return	1;
}


static int mouse_is_over_knowedge_bar(window_info *win, int mx, int my)
{
	if (view_knowledge_bar)
	{
		int bar_y_pos = win->len_y - compass_size;
		if (view_analog_clock) bar_y_pos -= analog_clock_size;
		if (view_digital_clock) bar_y_pos -= digital_clock_height;
		if (my>bar_y_pos-knowledge_bar_height && my<bar_y_pos)
			return 1;
	}
	return 0;
}


static int click_misc_handler(window_info *win, int mx, int my, Uint32 flags)
{
	int clockheight = 0;
	int in_stats_bar = 0;

	// handle scrolling the stats bars if not all displayed
	if (show_stats_in_hud && (my >= 0) && (my < num_disp_stat*side_stats_bar_height))
	{
		in_stats_bar = 1;

		if ((first_disp_stat > 0) && ((flags & ELW_WHEEL_UP) ||
			((flags & ELW_LEFT_MOUSE) && (flags & KMOD_CTRL))))
		{
			first_disp_stat--;
			return 1;
		}
		else if ((first_disp_stat + num_disp_stat < NUM_WATCH_STAT-1) &&
				 ((flags & ELW_WHEEL_DOWN) || ((flags & ELW_RIGHT_MOUSE) && (flags & KMOD_CTRL))))
		{
			first_disp_stat++;
			return 1;
		}
	}

	if (mouse_is_over_timer(win, mx, my))
		return mouse_click_timer(flags);

	// only handle mouse button clicks, not scroll wheels moves
	if ( (flags & ELW_MOUSE_BUTTON) == 0) return 0;

	// reserve CTRL clicks for scrolling
	if (flags & KMOD_CTRL) return 0;

	//check to see if we clicked on the clock
	if(view_digital_clock>0){
		clockheight += digital_clock_height;
	}
	if(view_analog_clock>0){
		clockheight += analog_clock_size;
	}
	if(my > (win->len_y - compass_size - clockheight) && my < (win->len_y - compass_size))
	{
		unsigned char protocol_name;
		do_click_sound();
		protocol_name= GET_TIME;
		my_tcp_send(my_socket,&protocol_name,1);
		return 1;
	}

	/* show research if click on the knowledge bar */
	if (mouse_is_over_knowedge_bar(win, mx, my))
	{
		do_click_sound();
		send_input_text_line("#research", 9);
		return 1;
	}

	//check to see if we clicked on the compass
	if(my > (win->len_y - compass_size) && my < win->len_y)
	{
		unsigned char protocol_name;
		do_click_sound();
		protocol_name= LOCATE_ME;
		if (flags & KMOD_SHIFT)
		{
			copy_next_LOCATE_ME = 2;
		}
		my_tcp_send(my_socket,&protocol_name,1);
		return 1;
	}
	//check to see if we clicked on the stats
	if (in_stats_bar)
	{
		handle_stats_selection(first_disp_stat + (my / side_stats_bar_height) + 1, flags);
		return 1;
	}

	return 0;
}


static int mouseover_misc_handler(window_info *win, int mx, int my)
{
	/* Optionally display scrolling help if statsbar is active and restricted in size */
	if (show_help_text && show_stats_in_hud && (num_disp_stat < NUM_WATCH_STAT-1) &&
		(my >= 0) && (my < num_disp_stat*side_stats_bar_height))
	{
		int width = get_string_width_zoom((const unsigned char*)stats_scroll_help_str,
			win->font_category, win->current_scale_small);
		show_help(stats_scroll_help_str, - 10 - width,
			win->len_y - HUD_MARGIN_Y - win->small_font_len_y, win->current_scale);
	}

	/* stat hover experience left */
	if (show_stats_in_hud && have_stats && (my >= 0) && (my < num_disp_stat*side_stats_bar_height))
		stat_mouse_is_over = first_disp_stat + (my / side_stats_bar_height);

	/* if the mouse is over either clock - display the date and time */
	if (view_analog_clock)
	{
		if (my>win->len_y - compass_size - analog_clock_size && my < win->len_y - compass_size)
			mouse_over_clock = 1;
	}
	if (view_digital_clock && !mouse_over_clock)
	{
		int digital_clock_y_pos = win->len_y - compass_size;
		if (view_analog_clock) digital_clock_y_pos -= analog_clock_size;
		if (my > digital_clock_y_pos - digital_clock_height && my < digital_clock_y_pos)
			mouse_over_clock = 1;
	}

	/* check if over the knowledge bar */
	if (mouse_is_over_knowedge_bar(win, mx, my))
		mouse_over_knowledge_bar = 1;

	/* check if over the timer */
	if (mouse_is_over_timer(win, mx, my))
		set_mouse_over_timer();

	/* if mouse over the compass - display the coords */
	if(my>win->len_y - compass_size && my < win->len_y)
		mouse_over_compass = 1;

	return 0;
}


static int destroy_misc_handler(window_info *win)
{
	destroy_timer();
	return 0;
}


static int ui_scale_misc_handler(window_info *win)
{
	int box_x = (int)(0.5 + win->current_scale * 3);
	int y_len = 0;
	int thestat, max_width, width, text_width, text_top, text_bottom, top, bottom;
	unsigned char str[5];
	int digital_clock_width;

	// Set width, so we can use it in e.g. timer ui_scale handler
	resize_window(win->window_id, HUD_MARGIN_X, win->len_y);

	analog_clock_size = (int)(0.5 + win->current_scale * 64);
	compass_size = (int)(0.5 + win->current_scale * 64);
	knowledge_bar_height = win->small_font_len_y + 6;

	side_stats_bar_text_zoom = win->current_scale_small;
	text_width = get_string_width_zoom((const unsigned char*)idle_str, win->font_category,
		side_stats_bar_text_zoom);
	width = 3 * get_max_digit_width_zoom(win->font_category, side_stats_bar_text_zoom)
		+ get_char_width_zoom('%', win->font_category, side_stats_bar_text_zoom);
	text_width = max2i(text_width, width);
	for (thestat = 0; thestat < NUM_WATCH_STAT-1; ++thestat)
	{
		safe_snprintf((char*)str, sizeof(str), "%-3s ", statsinfo[thestat].skillnames->shortname);
		width = get_string_width_zoom(str, win->font_category, side_stats_bar_text_zoom)
			+ 3 * get_max_digit_width_zoom(win->font_category, side_stats_bar_text_zoom);
		text_width = max2i(text_width, width);
	}
	max_width = win->len_x - box_x - 1;
	if (text_width > max_width)
	{
		side_stats_bar_text_zoom *= (float)max_width / text_width;
		text_width = win->len_x - box_x - 1;
	}

	get_top_bottom((const unsigned char*)"0123456789%", 11, win->font_category, side_stats_bar_text_zoom,
		&text_top, &text_bottom);
	for (thestat = 0; thestat < NUM_WATCH_STAT-1; ++thestat)
	{
		get_top_bottom(statsinfo[thestat].skillnames->shortname,
			strlen((const char*)statsinfo[thestat].skillnames->shortname), win->font_category,
			side_stats_bar_text_zoom, &top, &bottom);
		text_top = min2i(text_top, top);
		text_bottom = max2i(text_bottom, bottom);
	}
	get_top_bottom((const unsigned char*)idle_str, strlen(idle_str), win->font_category,
		side_stats_bar_text_zoom, &top, &bottom);
	text_top = min2i(text_top, top);
	text_bottom = max2i(text_bottom, bottom);

	side_stats_bar_text_width = text_width;
	side_stats_bar_text_offset = -text_top + (int)(0.5 + win->current_scale);
	side_stats_bar_text_height = text_bottom - text_top // text
		+ 2 * (int)(0.5 + win->current_scale); // shadow, top and bottom
	side_stats_bar_height = side_stats_bar_text_height
		+ 2 // border around bar
		+ (int)(win->current_scale * 2); // separation between bars

	if (show_game_seconds)
		digital_clock_width = 5 * get_max_digit_width_zoom(win->font_category, 1.0)
			+ 2 * get_char_width_zoom(':', win->font_category, 1.0);
	else
		digital_clock_width = 3 * get_max_digit_width_zoom(win->font_category, 1.0)
			+ get_char_width_zoom(':', win->font_category, 1.0)
			+ 2 * get_char_width_zoom(' ', win->font_category, 1.0);
	digital_clock_zoom = (float)(win->len_x - (int)(0.5 + 6 * win->current_scale)) / digital_clock_width;
	digital_clock_height = get_line_height(win->font_category, digital_clock_zoom);

	ui_scale_timer(win);
	y_len = compass_size;
	if (view_hud_timer)
		y_len += get_height_of_timer();
	if (view_analog_clock)
		y_len += analog_clock_size;
	if (view_digital_clock)
		y_len += digital_clock_height;
	if (view_knowledge_bar)
		y_len += knowledge_bar_height;
	if (show_stats_in_hud && have_stats)
		y_len += num_disp_stat * side_stats_bar_height;
	resize_window(win->window_id, HUD_MARGIN_X, y_len);
	move_window(win->window_id, -1, 0, window_width-win->len_x, window_height-win->len_y);
	reset_cm_regions();

	quickbar_misc_sep = win->current_scale * 5;

	return 1;
}

int get_min_hud_misc_len_y(void)
{
	int min_len_y = 0;
	if (misc_win < 0 || misc_win > windows_list.num_windows)
		return min_len_y;
	min_len_y = windows_list.window[misc_win].len_y;
	if (show_stats_in_hud)
	{
		min_len_y -= num_disp_stat * side_stats_bar_height;
		min_len_y += min_side_stats_bar * side_stats_bar_height;
	}
	min_len_y += quickbar_misc_sep;
	return min_len_y;
}


void init_misc_display(void)
{
	//create the misc window
	if(misc_win < 0)
		{
			misc_win= create_window("Misc", -1, 0, 0, 0, 0, 0, ELW_USE_UISCALE|ELW_TITLE_NONE|ELW_SHOW_LAST);
			if (misc_win < 0 || misc_win >= windows_list.num_windows)
				return;
			set_window_handler(misc_win, ELW_HANDLER_DISPLAY, &display_misc_handler);
			set_window_handler(misc_win, ELW_HANDLER_CLICK, &click_misc_handler);
			set_window_handler(misc_win, ELW_HANDLER_MOUSEOVER, &mouseover_misc_handler );
			set_window_handler(misc_win, ELW_HANDLER_UI_SCALE, &ui_scale_misc_handler );
			set_window_handler(misc_win, ELW_HANDLER_FONT_CHANGE, &ui_scale_misc_handler);
			set_window_handler(misc_win, ELW_HANDLER_DESTROY, &destroy_misc_handler );
			cm_hud_id = cm_create(cm_hud_menu_str, context_hud_handler);
			cm_bool_line(cm_hud_id, CMH_STATS, &show_stats_in_hud, "show_stats_in_hud");
			cm_bool_line(cm_hud_id, CMH_STATBARS, &show_statbars_in_hud, "show_statbars_in_hud");
			cm_bool_line(cm_hud_id, CMH_KNOWBAR, &view_knowledge_bar, "view_knowledge_bar");
			cm_bool_line(cm_hud_id, CMH_TIMER, &view_hud_timer, "view_hud_timer");
			cm_bool_line(cm_hud_id, CMH_DIGCLOCK, &view_digital_clock, "view_digital_clock");
			cm_bool_line(cm_hud_id, CMH_ANACLOCK, &view_analog_clock, "view_analog_clock");
			cm_bool_line(cm_hud_id, CMH_SECONDS, &show_game_seconds, "show_game_seconds");
			cm_bool_line(cm_hud_id, CMH_FPS, &show_fps, "show_fps");
			cm_bool_line(cm_hud_id, CMH_INDICATORS, &show_hud_indicators, "show_indicators");
			cm_bool_line(cm_hud_id, CMH_MINIMAP, &cm_minimap_shown, NULL);
			cm_bool_line(cm_hud_id, CMH_RANGSTATS, &cm_rangstats_shown, NULL);
			cm_bool_line(cm_hud_id, CMH_QUICKBM, &cm_quickbar_enabled, NULL);
			cm_bool_line(cm_hud_id, CMH_SOUND, &cm_sound_enabled, NULL);
			cm_bool_line(cm_hud_id, CMH_MUSIC, &cm_music_enabled, NULL);
			cm_add_window(cm_hud_id, misc_win);
			cm_set_pre_show_handler(cm_hud_id, context_hud_pre_show_handler);
		}
	ui_scale_misc_handler(&windows_list.window[misc_win]);
}
