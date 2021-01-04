#include <stdlib.h>
#include <string.h>
#include <SDL_timer.h>

#include "asc.h"
#include "chat.h"
#include "context_menu.h"
#include "font.h"
#include "elconfig.h"
#include "gl_init.h"
#include "gamewin.h"
#include "hud.h"
#include "hud_statsbar_window.h"
#include "icon_window.h"
#include "sound.h"

int stats_bar_win= -1;
int show_action_bar = 0;
int show_last_health_change_always = 0;
int max_food_level = 45;
int lock_skills_selection = 0;

static int actual_num_disp_stats=1;
static int exp_bar_text_len = 0;
static int stats_bar_text_len = 0;
static int statbar_cursor_x;
static int statbar_cursor_x;
static size_t cm_id = CM_INIT_VALUE;
static int exp_bar_start_x;
static int exp_bar_start_y;
static int health_bar_start_y;
static int mana_bar_start_x;
static int mana_bar_start_y;
static int food_bar_start_x;
static int food_bar_start_y;
static int load_bar_start_x;
static int load_bar_start_y;
static int action_bar_start_x;
static int action_bar_start_y;
static const int player_statsbar_y_offset = 4;
static int player_statsbar_bar_height = 0;
static int stats_bar_len;
static int health_bar_start_x;
static struct { int d; int h; Uint32 dt; Uint32 ht; } my_last_health = { 0, 0, 0, 0 };
static int watch_this_stats[MAX_WATCH_STATS]={NUM_WATCH_STAT -1, 0, 0, 0, 0};  // default to only watching overall


// return the number of watched stat bars, the number displayed in the botton HUD
static int get_num_statsbar_exp(void)
{
	size_t i;
	int num_skills_bar = 0;
	for (i=0; i<MAX_WATCH_STATS; i++)
		if ((watch_this_stats[i] > 0) && statsinfo[watch_this_stats[i]-1].is_selected)
			num_skills_bar++;
	return num_skills_bar;
}


// return the optimal length for stat bars for the botton HUD
static int calc_stats_bar_len(window_info *win, int num_exp)
{
	// calculate the maximum length for stats bars given the current number of both bar types
	int max_len = (int)(0.5 + win->current_scale * 100);
	int num_stat = (show_action_bar) ?5: 4;
	int prosed_len = (window_width-HUD_MARGIN_X-1) - (num_stat * stats_bar_text_len) - (num_exp * exp_bar_text_len);
	prosed_len /= num_stat + num_exp;

	// constrain the maximum and minimum length of the skills bars to reasonable size
	if (prosed_len < 50)
		prosed_len = 50;
	else if (prosed_len > max_len)
		prosed_len = max_len;

	return prosed_len;
}


// calculate the maximum number of exp bars
static int calc_max_disp_stats(int suggested_stats_bar_len)
{
	int exp_offset = ((show_action_bar)?5:4) * (suggested_stats_bar_len + stats_bar_text_len);
	int preposed_max_disp_stats = (window_width - HUD_MARGIN_X - exp_offset) / (suggested_stats_bar_len + exp_bar_text_len);
	if (preposed_max_disp_stats > MAX_WATCH_STATS)
		preposed_max_disp_stats = MAX_WATCH_STATS;
	return preposed_max_disp_stats;
}


/* draws damage and heal above the health bar */
static void draw_last_health_change(window_info *win)
{
	unsigned char str[20];
	static const Uint32 timeoutms = 2*60*1000;
	const int yoff = -(HUD_MARGIN_Y + win->default_font_len_y + 1 - (window_height - win->cur_y));
	/* damage in red */
	if (my_last_health.d != 0)
	{
		if ((SDL_GetTicks() - my_last_health.dt) > timeoutms)
			my_last_health.d = 0;
		else
		{
			safe_snprintf((char*)str, sizeof(str), " %d ", my_last_health.d);
			draw_text(health_bar_start_x+stats_bar_len/2-2, yoff, str, strlen((const char*)str),
				win->font_category, TDO_MAX_WIDTH, window_width - 80, TDO_HELP, 1, TDO_FOREGROUND,
				1.0f, 0.0f, 0.0f, TDO_ZOOM, win->current_scale, TDO_ALIGNMENT, RIGHT, TDO_END);
		}
	}
	/* heal in green */
	if (my_last_health.h != 0)
	{
		if ((SDL_GetTicks() - my_last_health.ht) > timeoutms)
			my_last_health.h = 0;
		else
		{
			safe_snprintf((char*)str, sizeof(str), " %d ", my_last_health.h);
			draw_text(health_bar_start_x+stats_bar_len/2+2, yoff, str, strlen((const char*)str),
				win->font_category, TDO_HELP, 1, TDO_FOREGROUND, 0.0, 1.0, 0.0,
				TDO_ZOOM, win->current_scale, TDO_END);
		}
	}
}


static int get_player_statsbar_active_height(void)
{
	return player_statsbar_y_offset + player_statsbar_bar_height;
}


// clear the context menu regions for all stats bars and set up again
static void reset_statsbar_exp_cm_regions(void)
{
	size_t i;
	cm_remove_regions(stats_bar_win);
	for (i=0; i<actual_num_disp_stats; i++)
		if (watch_this_stats[i] > 0)
			cm_add_region(cm_id, stats_bar_win, exp_bar_start_x+i*(stats_bar_len+exp_bar_text_len), exp_bar_start_y, stats_bar_len, get_player_statsbar_active_height());
}


// remove the specific stat bar
static void remove_watched_stat(size_t watched_stat_index)
{
	if (watched_stat_index >= MAX_WATCH_STATS)
		return;
	statsinfo[watch_this_stats[watched_stat_index]-1].is_selected = 0;
	if (watched_stat_index < MAX_WATCH_STATS - 1)
		memmove(&(watch_this_stats[watched_stat_index]), &(watch_this_stats[watched_stat_index + 1]), (MAX_WATCH_STATS-watched_stat_index - 1) * sizeof(int));
	watch_this_stats[MAX_WATCH_STATS-1] = 0;
}


static int cm_statsbar_handler(window_info *win, int widget_id, int mx, int my, int option)
{
	int i;
	int add_bar = 0;

	// selecting the same stat more than once, removing the last bar
	// or adding too many is not possible as options are greyed out.

	for (i=0; i<actual_num_disp_stats;i++)
	{
		if ((mx >= exp_bar_start_x+i*(stats_bar_len+exp_bar_text_len)) && (mx <= exp_bar_start_x+i*(stats_bar_len+exp_bar_text_len)+stats_bar_len))
		{
			// if deleting the bar, close any gap
			if (option == NUM_WATCH_STAT+1)
			{
				remove_watched_stat(i);
				init_stats_display();
			}
			else if (option == NUM_WATCH_STAT)
				add_bar = 1;
			else if (option < NUM_WATCH_STAT)
			{
				statsinfo[watch_this_stats[i]-1].is_selected=0;
				watch_this_stats[i] = option+1;
				statsinfo[option].is_selected=1;
			}
			break;
		}
	}

	// if we want another bar, assigning it to the first unwatched stat
	if (add_bar)
	{
		int proposed_max_disp_stats = calc_max_disp_stats(calc_stats_bar_len(win, get_num_statsbar_exp()+1));
		int next_free = -1;
		for (i=0; i<NUM_WATCH_STAT-1; i++)
			if (statsinfo[i].is_selected == 0)
			{
				next_free = i;
				break;
			}
		for (i=0; next_free>=0 && i<proposed_max_disp_stats; i++)
			if (watch_this_stats[i] == 0)
			{
				watch_this_stats[i] = next_free + 1;
				statsinfo[next_free].is_selected =1 ;
				break;
			}
		init_stats_display();
	}

	reset_statsbar_exp_cm_regions();

	return 1;
}


// calculate the number of digits for the specified Uint32
static Uint32 Uint32_digits(Uint32 number)
{
	Uint32 digits = 1;
	long step = 10;
	while ((step <= number) && (digits<11))
	{
		digits++;
		step *= 10;
	}
	return digits;
}


static void cm_statsbar_pre_show_handler(window_info *win, int widget_id, int mx, int my, window_info *cm_win)
{
	size_t i;
	int proposed_max_disp_stats = calc_max_disp_stats(calc_stats_bar_len(win, get_num_statsbar_exp()+1));
	for (i=0; i<NUM_WATCH_STAT-1; i++)
		cm_grey_line(cm_id, i, 0);
	for (i=0; i<MAX_WATCH_STATS; i++)
		if (watch_this_stats[i] > 0)
			cm_grey_line(cm_id, watch_this_stats[i]-1, 1);
	cm_grey_line(cm_id, NUM_WATCH_STAT, ((get_num_statsbar_exp() < proposed_max_disp_stats) ?0 :1));
	cm_grey_line(cm_id, NUM_WATCH_STAT+1, ((watch_this_stats[1]==0)?1:0));
}


static void draw_stats_bar(window_info *win, int x, int y, int val, int len, float r, float g, float b, float r2, float g2, float b2)
{
	char buf[32];
	int i; // i deals with massive bars by trimming at 110%
	int bar_height = player_statsbar_bar_height;
	int text_offset = (int)(0.5 + win->current_scale * 2);

	if(len>stats_bar_len*1.1)
		i=stats_bar_len*1.1;
	else
		i=len;
	glDisable(GL_TEXTURE_2D);

	if(i >= 0){
		glBegin(GL_QUADS);
		//draw the colored section
 		glColor3f(r2, g2, b2);
		glVertex3i(x, y+bar_height, 0);
		glColor3f(r, g, b);
		glVertex3i(x, y, 0);
		glColor3f(r, g, b);
		glVertex3i(x+i, y, 0);
		glColor3f(r2, g2, b2);
		glVertex3i(x+i, y+bar_height, 0);
		glEnd();
	}
	// draw the bar frame
	glColor3fv(gui_color);
	glBegin(GL_LINE_LOOP);
	glVertex3i(x, y, 0);
	glVertex3i(x+stats_bar_len, y, 0);
	glVertex3i(x+stats_bar_len, y+bar_height, 0);
	glVertex3i(x, y+bar_height, 0);
	glEnd();
	glEnable(GL_TEXTURE_2D);

	// handle the text
	safe_snprintf(buf, sizeof(buf), "%d", val);
	//glColor3f(0.8f, 0.8f, 0.8f); moved to next line
	draw_string_small_shadowed_zoomed_right(x - text_offset, y - text_offset, (const unsigned char*)buf,
		1, 0.8f, 0.8f, 0.8f, 0.0f, 0.0f, 0.0f, win->current_scale);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}


// check if we need to adjust exp_bar_text_len due to an exp change
static int recalc_exp_bar_text_len(window_info *win, int force)
{
	static int init_flag = 1;
	static Uint32 last_exp[NUM_WATCH_STAT-1];
	static Uint32 last_to_go_len[NUM_WATCH_STAT-1];
	static Uint32 last_selected[NUM_WATCH_STAT-1];
	int recalc = init_flag || force;
	int i;

	if (init_flag)
	{
		for (i=0; i<NUM_WATCH_STAT-1; i++)
		{
			last_exp[i] = *statsinfo[i].exp;
			last_to_go_len[i] = Uint32_digits(*statsinfo[i].next_lev - *statsinfo[i].exp);
			last_selected[i] = 0;
		}
		init_flag = 0;
	}

	for (i=0; i<NUM_WATCH_STAT-1; i++)
	{
		/* if any exp changes, recalculate the number of digits for next level value */
		if (last_exp[i] != *statsinfo[i].exp)
		{
			unsigned int curr = Uint32_digits(*statsinfo[i].next_lev - *statsinfo[i].exp);
			/* if the number of digit changes, we need to recalulate exp_bar_text_len */
			if (last_to_go_len[i] != curr)
			{
				last_to_go_len[i] = curr;
				recalc = 1;
			}
			last_exp[i] = *statsinfo[i].exp;
		}
		/* if the selected status of any skill changed, we need to recalulate exp_bar_text_len */
		if (last_selected[i] != statsinfo[i].is_selected)
		{
			last_selected[i] = statsinfo[i].is_selected;
			recalc = 1;
		}
	}

	/* recalc based only the skills being watched */
	if (recalc)
	{
		int max_len = 0;
		int max_digit_width = get_max_digit_width_zoom(win->font_category,
			win->current_scale_small);
		for (i=0; i<MAX_WATCH_STATS; i++)
			if ((watch_this_stats[i] > 0) && statsinfo[watch_this_stats[i]-1].is_selected &&
					(last_to_go_len[watch_this_stats[i]-1] > max_len))
				max_len = last_to_go_len[watch_this_stats[i]-1];
		return max_digit_width*(max_len+1.5);
	}
	else
		return exp_bar_text_len;
}


static void draw_exp_display(window_info *win)
{
	size_t i;
	int my_exp_bar_start_x = exp_bar_start_x;

	// default to overall if no valid first skill is set
	if(watch_this_stats[0]<1 || watch_this_stats[0]>=NUM_WATCH_STAT)
	{
		watch_this_stats[0]=NUM_WATCH_STAT-1;
		statsinfo[watch_this_stats[0]-1].is_selected=1;
		reset_statsbar_exp_cm_regions();
	}

	for (i=0; i<actual_num_disp_stats; i++)
	{
		if (watch_this_stats[i] > 0)
		{
			int name_y = exp_bar_start_y + 2 + player_statsbar_bar_height;
			int icon_x = get_icons_win_active_len();
			int cur_exp = *statsinfo[watch_this_stats[i]-1].exp;
			int nl_exp = *statsinfo[watch_this_stats[i]-1].next_lev;
			int baselev = statsinfo[watch_this_stats[i]-1].skillattr->base;
			const unsigned char* name = statsinfo[watch_this_stats[i]-1].skillnames->name;
			int exp_adjusted_x_len;
			int delta_exp;
			float prev_exp;
			int name_width;

			if(!baselev)
				prev_exp= 0;
			else
				prev_exp= exp_lev[baselev];

			delta_exp= nl_exp-prev_exp;

			if(!cur_exp || !nl_exp || delta_exp <=0)
				exp_adjusted_x_len= 0;
			else
				exp_adjusted_x_len= stats_bar_len-(float)stats_bar_len/(float)((float)delta_exp/(float)(nl_exp-cur_exp));

			name_width = get_string_width_zoom(name, win->font_category, win->current_scale_small);
			// the the name would overlap with the icons...
			if (my_exp_bar_start_x + stats_bar_len - name_width < icon_x)
			{
				name = statsinfo[watch_this_stats[i]-1].skillnames->shortname;
				name_y = (int)(0.5 + (player_statsbar_y_offset + player_statsbar_bar_height - win->small_font_len_y) / 2) - 1;
			}

			draw_stats_bar(win, my_exp_bar_start_x, exp_bar_start_y, nl_exp - cur_exp, exp_adjusted_x_len, 0.1f, 0.8f, 0.1f, 0.1f, 0.4f, 0.1f);
			draw_string_small_shadowed_zoomed_right(my_exp_bar_start_x + stats_bar_len,
				name_y, name, 1,1.0f,1.0f,1.0f,0.0f,0.0f,0.0f, win->current_scale);

			my_exp_bar_start_x += stats_bar_len+exp_bar_text_len;
		}
		else
			break;
	}

}

static void check_text_widths(window_info *win, int force)
{
	int proposed_len = 0;
	stats_bar_text_len = 4.5 * win->small_font_max_len_x;
	if ((proposed_len = recalc_exp_bar_text_len(win, force)) != exp_bar_text_len) // it will very rarely change
	{
		exp_bar_text_len = proposed_len;
		init_stats_display();
	}
}

static int	display_stats_bar_handler(window_info *win)
{
	static Uint32 last_time = 0;
	float health_adjusted_x_len;
	float food_adjusted_x_len;
	float mana_adjusted_x_len;
	float load_adjusted_x_len;
	float action_adjusted_x_len;
	int over_health_bar;

	// the space taken up by the exp bar text is minimised, but may change
	// don't have to check often but this is an easy place to do it and its quick anyway
	if ((SDL_GetTicks()-last_time) > 250)
	{
		check_text_widths(win, 0);
		last_time = SDL_GetTicks();
	}

	over_health_bar = statbar_cursor_x>health_bar_start_x && statbar_cursor_x < health_bar_start_x+stats_bar_len;

	//get the adjusted length

	if(!your_info.material_points.cur || !your_info.material_points.base)
		health_adjusted_x_len=0;//we don't want a div by 0
	else
		health_adjusted_x_len=stats_bar_len/((float)your_info.material_points.base/(float)your_info.material_points.cur);

	if(your_info.food_level<=0)
		food_adjusted_x_len=0;//we don't want a div by 0
	else
		food_adjusted_x_len=stats_bar_len/((float)max_food_level/(float)your_info.food_level);
	if(food_adjusted_x_len>stats_bar_len) food_adjusted_x_len=stats_bar_len;

	if(!your_info.ethereal_points.cur || !your_info.ethereal_points.base)
		mana_adjusted_x_len=0;//we don't want a div by 0
	else
		mana_adjusted_x_len=stats_bar_len/((float)your_info.ethereal_points.base/(float)your_info.ethereal_points.cur);

	if(!your_info.carry_capacity.cur || !your_info.carry_capacity.base)
		load_adjusted_x_len=0;//we don't want a div by 0
	else
		load_adjusted_x_len=stats_bar_len/((float)your_info.carry_capacity.base/(float)your_info.carry_capacity.cur);

	if(!your_info.action_points.cur || !your_info.action_points.base)
		action_adjusted_x_len=0;//we don't want a div by 0
	else
		action_adjusted_x_len=stats_bar_len/((float)your_info.action_points.base/(float)your_info.action_points.cur);

	draw_stats_bar(win, health_bar_start_x, health_bar_start_y, your_info.material_points.cur, health_adjusted_x_len, 1.0f, 0.2f, 0.2f, 0.5f, 0.2f, 0.2f);

	if (your_info.food_level<=max_food_level) //yellow
		draw_stats_bar(win, food_bar_start_x, food_bar_start_y, your_info.food_level, food_adjusted_x_len, 1.0f, 1.0f, 0.2f, 0.5f, 0.5f, 0.2f);
	else draw_stats_bar(win, food_bar_start_x, food_bar_start_y, your_info.food_level, food_adjusted_x_len, 1.0f, 0.5f, 0.0f, 0.7f, 0.3f, 0.0f); //orange

	draw_stats_bar(win, mana_bar_start_x, mana_bar_start_y, your_info.ethereal_points.cur, mana_adjusted_x_len, 0.2f, 0.2f, 1.0f, 0.2f, 0.2f, 0.5f);
	draw_stats_bar(win, load_bar_start_x, load_bar_start_y, your_info.carry_capacity.base-your_info.carry_capacity.cur, load_adjusted_x_len, 0.6f, 0.4f, 0.4f, 0.4f, 0.2f, 0.2f);
	if (show_action_bar)
		draw_stats_bar(win, action_bar_start_x, action_bar_start_y, your_info.action_points.cur, action_adjusted_x_len, 0.8f, 0.3f, 0.8f, 0.5f, 0.1f, 0.5f);

	draw_exp_display(win);

	if(show_help_text && statbar_cursor_x>=0)
	{
		if(over_health_bar) show_help((char*)attributes.material_points.name,health_bar_start_x+stats_bar_len+10,-3, win->current_scale);
		else if(statbar_cursor_x>food_bar_start_x && statbar_cursor_x < food_bar_start_x+stats_bar_len) show_help((char*)attributes.food.name,food_bar_start_x+stats_bar_len+10,-3, win->current_scale);
		else if(statbar_cursor_x>mana_bar_start_x && statbar_cursor_x < mana_bar_start_x+stats_bar_len) show_help((char*)attributes.ethereal_points.name,mana_bar_start_x+stats_bar_len+10,-3, win->current_scale);
		else if(statbar_cursor_x>load_bar_start_x && statbar_cursor_x < load_bar_start_x+stats_bar_len) show_help((char*)attributes.carry_capacity.name,load_bar_start_x+stats_bar_len+10,-3, win->current_scale);
		else if(show_action_bar && statbar_cursor_x>action_bar_start_x && statbar_cursor_x < action_bar_start_x+stats_bar_len) show_help((char*)attributes.action_points.name,action_bar_start_x+stats_bar_len+10,-3, win->current_scale);
	}

	if ((over_health_bar) || (show_last_health_change_always && get_show_window (game_root_win) && ((use_windowed_chat == 2) || !input_text_line.len)))
		draw_last_health_change(win);

	statbar_cursor_x = -1;

	return 1;
}


static int mouseover_stats_bar_handler(window_info *win, int mx, int my)
{
	statbar_cursor_x=mx;
	return 0;
}


// the stats display
static int ui_scale_stats_bar_handler(window_info *win)
{
	int i;
	int num_exp = get_num_statsbar_exp();
	int proposed_max_disp_stats = 0;
	int stats_height = 0;
	int stats_width = window_width - HUD_MARGIN_X;
	int stats_y_pos = window_height - (HUD_MARGIN_Y - player_statsbar_y_offset);

	player_statsbar_bar_height = (int)(0.5 + win->current_scale * 8);
	stats_height = get_player_statsbar_active_height();

	init_window(stats_bar_win, -1, 0, 0, stats_y_pos, stats_width, stats_height);

	/* use a fixed width for user attrib stat bar text */
	stats_bar_text_len = 4.5 * win->small_font_max_len_x;

	// calculate the statsbar len given curent config
	stats_bar_len = calc_stats_bar_len(win, num_exp);

	// calculate the maximum number of exp bars we can have
	proposed_max_disp_stats = calc_max_disp_stats(stats_bar_len);

	// if we need to reduce the number of bars, recalculate the optimum stats bar len
	if (num_exp > proposed_max_disp_stats)
	{
		stats_bar_len = calc_stats_bar_len(win, proposed_max_disp_stats);
		actual_num_disp_stats = proposed_max_disp_stats;
	}
	else
		actual_num_disp_stats = num_exp;

	// all the bars are at the top of the window
	mana_bar_start_y = food_bar_start_y = health_bar_start_y = load_bar_start_y = action_bar_start_y = exp_bar_start_y = 0;

	// calculate the stats bar x position
	mana_bar_start_x = stats_bar_text_len;
	food_bar_start_x = stats_bar_len + 2 * stats_bar_text_len;
	health_bar_start_x = 2 * stats_bar_len + 3 * stats_bar_text_len;
	load_bar_start_x = 3 * stats_bar_len + 4 * stats_bar_text_len;
	if (show_action_bar)
		action_bar_start_x = 4 * stats_bar_len + 5 * stats_bar_text_len;

	// the x position of the first exp bar, keep right aligned
	exp_bar_start_x = window_width + exp_bar_text_len - HUD_MARGIN_X - 2
		- actual_num_disp_stats * (exp_bar_text_len + stats_bar_len);

	// create the exp bars context menu, used by all active exp bars
	if (!cm_valid(cm_id))
	{
		int thestat;
		cm_id = cm_create(NULL, cm_statsbar_handler);
		for (thestat=0; thestat<NUM_WATCH_STAT-1; thestat++)
			cm_add(cm_id, (char *)statsinfo[thestat].skillnames->name, NULL);
		cm_add(cm_id, cm_stats_bar_base_str, NULL);
		cm_bool_line(cm_id, NUM_WATCH_STAT+2, &lock_skills_selection, NULL);
		cm_set_pre_show_handler(cm_id,cm_statsbar_pre_show_handler);
	}
	reset_statsbar_exp_cm_regions();

	for (i=0; i<MAX_WATCH_STATS; i++)
	{
		if (watch_this_stats[i] > 0)
			statsinfo[watch_this_stats[i]-1].is_selected = 1;
	}

	check_text_widths(win, 1);

	return 1;
}

static int change_stats_bar_font_handler(window_info* win, font_cat cat)
{
	if (cat != UI_FONT)
		return 0;
	check_text_widths(win, 1);
	return 1;
}

//create the stats bar window
void init_stats_display(void)
{
	if(stats_bar_win < 0)
	{
		static size_t cm_id_ap = CM_INIT_VALUE;
		stats_bar_win= create_window("Stats Bar", -1, 0, 0, 0, 0, 0, ELW_USE_UISCALE|ELW_TITLE_NONE|ELW_SHOW_LAST);
		set_window_handler(stats_bar_win, ELW_HANDLER_DISPLAY, &display_stats_bar_handler);
		set_window_handler(stats_bar_win, ELW_HANDLER_MOUSEOVER, &mouseover_stats_bar_handler);
		set_window_handler(stats_bar_win, ELW_HANDLER_UI_SCALE, &ui_scale_stats_bar_handler);
		set_window_handler(stats_bar_win, ELW_HANDLER_FONT_CHANGE, &change_stats_bar_font_handler);

		// context menu to enable/disable the action points bar
		cm_id_ap = cm_create(cm_statsbar_str, NULL);
		cm_add_window(cm_id_ap, stats_bar_win);
		cm_bool_line(cm_id_ap, 0, &show_action_bar, "show_action_bar");
		cm_bool_line(cm_id_ap, 1, &show_last_health_change_always, "show_last_health_change_always");
	}
	if (stats_bar_win >= 0 && stats_bar_win < windows_list.num_windows)
		ui_scale_stats_bar_handler(&windows_list.window[stats_bar_win]);
}


void handle_stats_selection(int stat, Uint32 flags)
{
	int i;

	if (lock_skills_selection || stats_bar_win < 0 || stats_bar_win >= windows_list.num_windows)
	{
		do_alert1_sound();
		return;
	}

	if ((flags & KMOD_ALT) || (flags & KMOD_SHIFT))
	{
		for (i=0;i<MAX_WATCH_STATS;i++)
		{
			// if already selected, unselect and remove bar, closing any gap
			if (watch_this_stats[i]==stat)
			{
				remove_watched_stat(i);
				break;
			}
			// if the bar is not in use, set it to the new stat
			if (watch_this_stats[i]==0)
			{
				watch_this_stats[i]=stat;
				statsinfo[stat-1].is_selected=1;
				break;
			}
		}
	}
	else
	{
		// if not already selected, select the stat and replace the first bar
		if (statsinfo[stat-1].is_selected==0)
		{
			statsinfo[watch_this_stats[0]-1].is_selected=0;
			watch_this_stats[0] = stat;
			statsinfo[stat-1].is_selected=1;
		}
		// else unselect the stat and remove the bar, closing any gap
		else
		{
			for (i=0;i<MAX_WATCH_STATS;i++)
				if (watch_this_stats[i] == stat)
				{
					remove_watched_stat(i);
					break;
				}
		}
	}

	// default to overall if no valid first skill is set
	if(watch_this_stats[0]<1 || watch_this_stats[0]>=NUM_WATCH_STAT)
	{
		watch_this_stats[0]=NUM_WATCH_STAT-1;
		statsinfo[watch_this_stats[0]-1].is_selected=1;
	}

	init_stats_display();
	do_click_sound();
}


/* called when our actor receives damage, displayed as hover over health bar */
void set_last_damage(int quantity)
{
	my_last_health.d = quantity;
	my_last_health.dt = SDL_GetTicks();
}


/* called when our actor heals, displayed as hover over health bar */
void set_last_heal(int quantity)
{
	my_last_health.h = quantity;
	my_last_health.ht = SDL_GetTicks();
}


void set_statsbar_watched_stats(int *cfg_watch_this_stats)
{
	int i;
#if MAX_WATCH_STATS != 5
#error You cannot just go around changing MAX_WATCH_STATS as its used by the cfg file so change init.h too.
#endif
	for(i=0;i<MAX_WATCH_STATS;i++)
	{
		watch_this_stats[i] = cfg_watch_this_stats[i];
		if (watch_this_stats[i]<0 || watch_this_stats[i]>=NUM_WATCH_STAT)
			watch_this_stats[i]=0;
	}
	if(watch_this_stats[0]<1 || watch_this_stats[0]>=NUM_WATCH_STAT)
		watch_this_stats[0]=NUM_WATCH_STAT-1;
}

void get_statsbar_watched_stats(int *cfg_watch_this_stats)
{
	int i;
	for(i=0;i<MAX_WATCH_STATS;i++)
		cfg_watch_this_stats[i]=watch_this_stats[i];
}
