#include <string.h>
#include <math.h>
#include "global.h"
#include "elwindows.h"

int	display_icons_handler(window_info *win);
int	click_icons_handler(window_info *win, int mx, int my, Uint32 flags);
int	display_stats_bar_handler(window_info *win);
//int	click_stats_bar_handler(window_info *win, int mx, int my, Uint32 flags);
int	display_misc_handler(window_info *win);
int	click_misc_handler(window_info *win, int mx, int my, Uint32 flags);
int	display_quickbar_handler(window_info *win);
int	click_quickbar_handler(window_info *win, int mx, int my, Uint32 flags);

int hud_x= 64;
int hud_y= 48;
int hud_text;
int view_digital_clock= 0;
int	icons_win= -1;
int	stats_bar_win= -1;
int	misc_win= -1;
int	quickbar_win= -1;

// initialize anything related to the hud
void init_hud_interface()
{
	init_hud_frame();
	init_peace_icons();
	init_misc_display();
	init_stats_display();
	init_quickbar();
}

// draw everything related to the hud
void draw_hud_interface()
{
    // TODO: get the window manager to handle this
	get_and_set_texture_id(hud_text);
	glColor3f(1.0f, 1.0f, 1.0f);
	draw_hud_frame();
    draw_misc_display();

	get_and_set_texture_id(icons_text);
    draw_peace_icons();
    draw_stats_display();
	draw_quickbar();
}

// check to see if a mouse click was on the hud
//TODO: add the window manager to handle these
int check_hud_interface()
{
    // TODO: get the window manager to handle this
	if(check_peace_icons() > 0)return 1;
	if(check_misc_display() > 0)return 1;
	if(check_stats_display() > 0)return 1;
	if(check_quickbar() > 0)return 1;

	// nothing done here
	return 0;
}

// hud frame section
float vertical_bar_u_start=(float)192/256;
float vertical_bar_u_end=1.0f;
float vertical_bar_v_end=0.0f;
float vertical_bar_v_start;

float horizontal_bar_u_start=(float)144/256;
float horizontal_bar_u_end=(float)191/256;
float horizontal_bar_v_start=0;
float horizontal_bar_v_end;

void init_hud_frame()
{
	vertical_bar_v_start= (float)window_height/256;
	horizontal_bar_v_end= (float)(window_width-hud_x)/256;
}

float logo_u_start=(float)64/256;
float logo_v_start=1.0f-(float)128/256;

float logo_u_end=(float)127/256;
float logo_v_end=1.0f-(float)191/256;

void draw_hud_frame()
{
	glBegin(GL_QUADS);
	draw_2d_thing(vertical_bar_u_start, vertical_bar_v_start, vertical_bar_u_end, vertical_bar_v_end,window_width-hud_x, 0, window_width, window_height);
	draw_2d_thing_r(horizontal_bar_u_start, horizontal_bar_v_start, horizontal_bar_u_end, horizontal_bar_v_end,0,window_height,window_width-hud_x , window_height-hud_y);
	//draw the logo
	draw_2d_thing(logo_u_start, logo_v_start, logo_u_end, logo_v_end,window_width-hud_x, 0, window_width, 64);
	glEnd();
}


// the icons section
float walk_icon_u_start=(float)0/256;
float walk_icon_v_start=1.0f-(float)0/256;
float walk_icon_u_end=(float)31/256;
float walk_icon_v_end=1.0f-(float)31/256;

float colored_walk_icon_u_start=(float)64/256;
float colored_walk_icon_v_start=1.0f-(float)64/256;
float colored_walk_icon_u_end=(float)95/256;
float colored_walk_icon_v_end=1.0f-(float)95/256;

float eye_icon_u_start=(float)64/256;
float eye_icon_v_start=1.0f-(float)0/256;
float eye_icon_u_end=(float)95/256;
float eye_icon_v_end=1.0f-(float)31/256;

float colored_eye_icon_u_start=(float)128/256;
float colored_eye_icon_v_start=1.0f-(float)64/256;
float colored_eye_icon_u_end=(float)159/256;
float colored_eye_icon_v_end=1.0f-(float)95/256;

float trade_icon_u_start=(float)128/256;
float trade_icon_v_start=1.0f-(float)0/256;
float trade_icon_u_end=(float)159/256;
float trade_icon_v_end=1.0f-(float)31/256;

float colored_trade_icon_u_start=(float)192/256;
float colored_trade_icon_v_start=1.0f-(float)64/256;
float colored_trade_icon_u_end=(float)223/256;
float colored_trade_icon_v_end=1.0f-(float)95/256;

float sit_icon_u_start=(float)224/256;
float sit_icon_v_start=1.0f-(float)0/256;
float sit_icon_u_end=(float)255/256;
float sit_icon_v_end=1.0f-(float)31/256;

float colored_sit_icon_u_start=(float)32/256;
float colored_sit_icon_v_start=1.0f-(float)96/256;
float colored_sit_icon_u_end=(float)63/256;
float colored_sit_icon_v_end=1.0f-(float)127/256;

float stand_icon_u_start=(float)0/256;
float stand_icon_v_start=1.0f-(float)32/256;
float stand_icon_u_end=(float)31/256;
float stand_icon_v_end=1.0f-(float)63/256;

float colored_stand_icon_u_start=(float)64/256;
float colored_stand_icon_v_start=1.0f-(float)96/256;
float colored_stand_icon_u_end=(float)95/256;
float colored_stand_icon_v_end=1.0f-(float)127/256;

float spell_icon_u_start=(float)32/256;
float spell_icon_v_start=1.0f-(float)32/256;
float spell_icon_u_end=(float)63/256;
float spell_icon_v_end=1.0f-(float)63/256;

float colored_spell_icon_u_start=(float)96/256;
float colored_spell_icon_v_start=1.0f-(float)96/256;
float colored_spell_icon_u_end=(float)127/256;
float colored_spell_icon_v_end=1.0f-(float)127/256;

float inventory_icon_u_start=(float)96/256;
float inventory_icon_v_start=1.0f-(float)32/256;
float inventory_icon_u_end=(float)127/256;
float inventory_icon_v_end=1.0f-(float)63/256;

float colored_inventory_icon_u_start=(float)160/256;
float colored_inventory_icon_v_start=1.0f-(float)96/256;
float colored_inventory_icon_u_end=(float)192/256;
float colored_inventory_icon_v_end=1.0f-(float)127/256;

float manufacture_icon_u_start=(float)128/256;
float manufacture_icon_v_start=1.0f-(float)32/256;
float manufacture_icon_u_end=(float)159/256;
float manufacture_icon_v_end=1.0f-(float)63/256;

float colored_manufacture_icon_u_start=(float)0/256;
float colored_manufacture_icon_v_start=1.0f-(float)128/256;
float colored_manufacture_icon_u_end=(float)31/256;
float colored_manufacture_icon_v_end=1.0f-(float)159/256;

float stats_icon_u_start=(float)160/256;
float stats_icon_v_start=1.0f-(float)32/256;
float stats_icon_u_end=(float)191/256;
float stats_icon_v_end=1.0f-(float)63/256;

float colored_stats_icon_u_start=(float)32/256;
float colored_stats_icon_v_start=1.0f-(float)128/256;
float colored_stats_icon_u_end=(float)63/256;
float colored_stats_icon_v_end=1.0f-(float)159/256;

float options_icon_u_start=(float)192/256;
float options_icon_v_start=1.0f-(float)32/256;
float options_icon_u_end=(float)223/256;
float options_icon_v_end=1.0f-(float)63/256;

float colored_options_icon_u_start=(float)64/256;
float colored_options_icon_v_start=1.0f-(float)128/256;
float colored_options_icon_u_end=(float)95/256;
float colored_options_icon_v_end=1.0f-(float)159/256;

float use_icon_u_start=(float)224/256;
float use_icon_v_start=1.0f-(float)32/256;
float use_icon_u_end=(float)255/256;
float use_icon_v_end=1.0f-(float)63/256;

float colored_use_icon_u_start=(float)96/256;
float colored_use_icon_v_start=1.0f-(float)128/256;
float colored_use_icon_u_end=(float)127/256;
float colored_use_icon_v_end=1.0f-(float)159/256;

float attack_icon_u_start=(float)160/256;
float attack_icon_v_start=1.0f-(float)0/256;
float attack_icon_u_end=(float)191/256;
float attack_icon_v_end=1.0f-(float)31/256;

float colored_attack_icon_u_start=(float)224/256;
float colored_attack_icon_v_start=1.0f-(float)64/256;
float colored_attack_icon_u_end=(float)255/256;
float colored_attack_icon_v_end=1.0f-(float)95/256;

float knowledge_icon_u_start=(float)96/256;
float knowledge_icon_v_start=1.0f-(float)64/256;
float knowledge_icon_u_end=(float)127/256;
float knowledge_icon_v_end=1.0f-(float)95/256;

float colored_knowledge_icon_u_start=(float)160/256;
float colored_knowledge_icon_v_start=1.0f-(float)64/256;
float colored_knowledge_icon_u_end=(float)191/256;
float colored_knowledge_icon_v_end=1.0f-(float)95/256;

float encyclopedia_icon_u_start=(float)0/256;
float encyclopedia_icon_v_start=1.0f-(float)64/256;
float encyclopedia_icon_u_end=(float)31/256;
float encyclopedia_icon_v_end=1.0f-(float)95/256;

float colored_encyclopedia_icon_u_start=(float)32/256;
float colored_encyclopedia_icon_v_start=1.0f-(float)64/256;
float colored_encyclopedia_icon_u_end=(float)63/256;
float colored_encyclopedia_icon_v_end=1.0f-(float)95/256;

float questlog_icon_u_start=(float)96/256;
float questlog_icon_v_start=1.0f-(float)0/256;
float questlog_icon_u_end=(float)127/256;
float questlog_icon_v_end=1.0f-(float)31/256;

float colored_questlog_icon_u_start=(float)192/256;
float colored_questlog_icon_v_start=1.0f-(float)0/256;
float colored_questlog_icon_u_end=(float)223/256;
float colored_questlog_icon_v_end=1.0f-(float)31/256;

float map_icon_u_start=(float)128/256;
float map_icon_v_start=1.0f-(float)128/256;
float map_icon_u_end=(float)159/256;
float map_icon_v_end=1.0f-(float)159/256;

float colored_map_icon_u_start=(float)160/256;
float colored_map_icon_v_start=1.0f-(float)128/256;
float colored_map_icon_u_end=(float)191/256;
float colored_map_icon_v_end=1.0f-(float)159/256;

float console_icon_u_start=(float)32/256;
float console_icon_v_start=1.0f-(float)0/256;
float console_icon_u_end=(float)63/256;
float console_icon_v_end=1.0f-(float)31/256;

float colored_console_icon_u_start=(float)128/256;
float colored_console_icon_v_start=1.0f-(float)96/256;
float colored_console_icon_u_end=(float)159/256;
float colored_console_icon_v_end=1.0f-(float)127/256;

float buddy_icon_u_start=(float)64/256;
float buddy_icon_v_start=1.0f-(float)32/256;
float buddy_icon_u_end=(float)95/256;
float buddy_icon_v_end=1.0f-(float)63/256;

float colored_buddy_icon_u_start=(float)0/256;
float colored_buddy_icon_v_start=1.0f-(float)96/256;
float colored_buddy_icon_u_end=(float)31/256;
float colored_buddy_icon_v_end=1.0f-(float)127/256;

int walk_icon_x_start;
int walk_icon_x_end;
int walk_icon_y_start;
int walk_icon_y_end;

int run_icon_x_start;
int run_icon_x_end;
int run_icon_y_start;
int run_icon_y_end;

int eye_icon_x_start;
int eye_icon_x_end;
int eye_icon_y_start;
int eye_icon_y_end;

int trade_icon_x_start;
int trade_icon_x_end;
int trade_icon_y_start;
int trade_icon_y_end;

int attack_icon_x_start;
int attack_icon_x_end;
int attack_icon_y_start;
int attack_icon_y_end;

int follow_icon_x_start;
int follow_icon_x_end;
int follow_icon_y_start;
int follow_icon_y_end;

int sit_icon_x_start;
int sit_icon_x_end;
int sit_icon_y_start;
int sit_icon_y_end;

int stand_icon_x_start;
int stand_icon_x_end;
int stand_icon_y_start;
int stand_icon_y_end;

int spell_icon_x_start;
int spell_icon_x_end;
int spell_icon_y_start;
int spell_icon_y_end;

int inventory_icon_x_start;
int inventory_icon_x_end;
int inventory_icon_y_start;
int inventory_icon_y_end;

int manufacture_icon_x_start;
int manufacture_icon_x_end;
int manufacture_icon_y_start;
int manufacture_icon_y_end;

int stats_icon_x_start;
int stats_icon_x_end;
int stats_icon_y_start;
int stats_icon_y_end;

int options_icon_x_start;
int options_icon_x_end;
int options_icon_y_start;
int options_icon_y_end;

int use_icon_x_start;
int use_icon_x_end;
int use_icon_y_start;
int use_icon_y_end;

int attack_icon_x_start;
int attack_icon_x_end;
int attack_icon_y_start;
int attack_icon_y_end;

int knowledge_icon_x_start;
int knowledge_icon_x_end;
int knowledge_icon_y_start;
int knowledge_icon_y_end;

int encyclopedia_icon_x_start;
int encyclopedia_icon_x_end;
int encyclopedia_icon_y_start;
int encyclopedia_icon_y_end;

int questlog_icon_x_start;
int questlog_icon_x_end;
int questlog_icon_y_start;
int questlog_icon_y_end;

int map_icon_x_start;
int map_icon_x_end;
int map_icon_y_start;
int map_icon_y_end;

int console_icon_x_start;
int console_icon_x_end;
int console_icon_y_start;
int console_icon_y_end;

int buddy_icon_x_start;
int buddy_icon_x_end;
int buddy_icon_y_start;
int buddy_icon_y_end;

//stat bars
int health_bar_start_x;
int health_bar_start_y;

int mana_bar_start_x;
int mana_bar_start_y;

int food_bar_start_x;
int food_bar_start_y;

int load_bar_start_x;
int load_bar_start_y;

int exp_bar_start_x;
int exp_bar_start_y;

void init_peace_icons()
{
	//create the icon window
	icons_win= create_window("Icons", 0, 0, 0, window_height-32, window_width-64, 32, ELW_TITLE_NONE|ELW_SHOW|ELW_SHOW_LAST);
	set_window_handler(icons_win, ELW_HANDLER_DISPLAY, &display_icons_handler);
	set_window_handler(icons_win, ELW_HANDLER_CLICK, &click_icons_handler);
	
	//TODO: positions should be based on icon size

	walk_icon_x_start=0;
	walk_icon_x_end=walk_icon_x_start+31;
	walk_icon_y_start=0;
	walk_icon_y_end=walk_icon_y_start+31;

	sit_icon_x_start=walk_icon_x_end+1;
	sit_icon_x_end=sit_icon_x_start+31;
	sit_icon_y_start=0;
	sit_icon_y_end=sit_icon_y_start+31;

	stand_icon_x_start=walk_icon_x_end+1;
	stand_icon_x_end=stand_icon_x_start+31;
	stand_icon_y_start=0;
	stand_icon_y_end=stand_icon_y_start+31;

	eye_icon_x_start=stand_icon_x_end+1;
	eye_icon_x_end=eye_icon_x_start+31;
	eye_icon_y_start=0;
	eye_icon_y_end=eye_icon_y_start+31;

	use_icon_x_start=eye_icon_x_end+1;
	use_icon_x_end=use_icon_x_start+31;
	use_icon_y_start=0;
	use_icon_y_end=use_icon_y_start+31;

	trade_icon_x_start=use_icon_x_end+1;
	trade_icon_x_end=trade_icon_x_start+31;
	trade_icon_y_start=0;
	trade_icon_y_end=trade_icon_y_start+31;

	attack_icon_x_start=trade_icon_x_end+1;
	attack_icon_x_end=attack_icon_x_start+31;
	attack_icon_y_start=0;
	attack_icon_y_end=attack_icon_y_start+31;

	inventory_icon_x_start=attack_icon_x_end+1;
	inventory_icon_x_end=inventory_icon_x_start+31;
	inventory_icon_y_start=0;
	inventory_icon_y_end=inventory_icon_y_start+31;

	spell_icon_x_start=inventory_icon_x_end+1;
	spell_icon_x_end=spell_icon_x_start+31;
	spell_icon_y_start=0;
	spell_icon_y_end=spell_icon_y_start+31;

	manufacture_icon_x_start=spell_icon_x_end+1;
	manufacture_icon_x_end=manufacture_icon_x_start+31;
	manufacture_icon_y_start=0;
	manufacture_icon_y_end=manufacture_icon_y_start+31;

	stats_icon_x_start=manufacture_icon_x_end+1;
	stats_icon_x_end=stats_icon_x_start+31;
	stats_icon_y_start=0;
	stats_icon_y_end=stats_icon_y_start+31;

	knowledge_icon_x_start=stats_icon_x_end+1;
	knowledge_icon_x_end=knowledge_icon_x_start+31;
	knowledge_icon_y_start=0;
	knowledge_icon_y_end=knowledge_icon_y_start+31;

	encyclopedia_icon_x_start=knowledge_icon_x_end+1;
	encyclopedia_icon_x_end=encyclopedia_icon_x_start+31;
	encyclopedia_icon_y_start=0;
	encyclopedia_icon_y_end=encyclopedia_icon_y_start+31;

	questlog_icon_x_start=encyclopedia_icon_x_end+1;
	questlog_icon_x_end=questlog_icon_x_start+31;
	questlog_icon_y_start=0;
	questlog_icon_y_end=questlog_icon_y_start+31;

	map_icon_x_start=questlog_icon_x_end+1;
	map_icon_x_end=map_icon_x_start+31;
	map_icon_y_start=0;
	map_icon_y_end=map_icon_y_start+31;

	console_icon_x_start=map_icon_x_end+1;
	console_icon_x_end=console_icon_x_start+31;
	console_icon_y_start=0;
	console_icon_y_end=console_icon_y_start+31;

	buddy_icon_x_start=console_icon_x_end+1;
	buddy_icon_x_end=buddy_icon_x_start+31;
	buddy_icon_y_start=0;
	buddy_icon_y_end=buddy_icon_y_start+31;

	options_icon_x_start=buddy_icon_x_end+1;
	options_icon_x_end=options_icon_x_start+31;
	options_icon_y_start=0;
	options_icon_y_end=options_icon_y_start+31;
}

void draw_peace_icons()
{
    display_window(icons_win);
}

int	display_icons_handler(window_info *win)
{
    int	in_window= (mouse_y >= win->pos_y && mouse_y < win->pos_y+win->len_y);
	get_and_set_texture_id(icons_text);
	glColor3f(1.0f,1.0f,1.0f);

	//glEnable(GL_ALPHA_TEST);//enable alpha filtering, so we have some alpha key
	//glAlphaFunc(GL_GREATER,0.03f);
	glBegin(GL_QUADS);

	if(action_mode==action_walk || (in_window && mouse_x>walk_icon_x_start && mouse_x<walk_icon_x_end ))
		draw_2d_thing(colored_walk_icon_u_start, colored_walk_icon_v_start, colored_walk_icon_u_end, colored_walk_icon_v_end,
					  walk_icon_x_start, walk_icon_y_start, walk_icon_x_end, walk_icon_y_end);
	else
		draw_2d_thing(walk_icon_u_start, walk_icon_v_start, walk_icon_u_end, walk_icon_v_end,
					  walk_icon_x_start, walk_icon_y_start, walk_icon_x_end, walk_icon_y_end);

	if(action_mode==action_look || (in_window && mouse_x>eye_icon_x_start && mouse_x<eye_icon_x_end))
		draw_2d_thing(colored_eye_icon_u_start, colored_eye_icon_v_start, colored_eye_icon_u_end, colored_eye_icon_v_end,
					  eye_icon_x_start, eye_icon_y_start, eye_icon_x_end, eye_icon_y_end);
	else
		draw_2d_thing(eye_icon_u_start, eye_icon_v_start, eye_icon_u_end, eye_icon_v_end,
					  eye_icon_x_start, eye_icon_y_start, eye_icon_x_end, eye_icon_y_end);

	if(action_mode==action_use || (in_window && mouse_x>use_icon_x_start && mouse_x<use_icon_x_end ))
		draw_2d_thing(colored_use_icon_u_start, colored_use_icon_v_start, colored_use_icon_u_end, colored_use_icon_v_end,
					  use_icon_x_start, use_icon_y_start, use_icon_x_end, use_icon_y_end);
	else
		draw_2d_thing(use_icon_u_start, use_icon_v_start, use_icon_u_end, use_icon_v_end,
					  use_icon_x_start, use_icon_y_start, use_icon_x_end, use_icon_y_end);


	if(action_mode==action_trade || (in_window && mouse_x>trade_icon_x_start && mouse_x<trade_icon_x_end))
		draw_2d_thing(colored_trade_icon_u_start, colored_trade_icon_v_start, colored_trade_icon_u_end, colored_trade_icon_v_end,
					  trade_icon_x_start, trade_icon_y_start, trade_icon_x_end, trade_icon_y_end);
	else
		draw_2d_thing(trade_icon_u_start, trade_icon_v_start, trade_icon_u_end, trade_icon_v_end,
					  trade_icon_x_start, trade_icon_y_start, trade_icon_x_end, trade_icon_y_end);


	if(!you_sit)
		{
			if(in_window && mouse_x>sit_icon_x_start && mouse_x<sit_icon_x_end)
				draw_2d_thing(colored_sit_icon_u_start, colored_sit_icon_v_start, colored_sit_icon_u_end, colored_sit_icon_v_end,
							  sit_icon_x_start, sit_icon_y_start, sit_icon_x_end, sit_icon_y_end);
			else
				draw_2d_thing(sit_icon_u_start, sit_icon_v_start, sit_icon_u_end, sit_icon_v_end,
							  sit_icon_x_start, sit_icon_y_start, sit_icon_x_end, sit_icon_y_end);
		}
	else
		{
			if(in_window && mouse_x>sit_icon_x_start &&mouse_x<sit_icon_x_end)
				draw_2d_thing(colored_stand_icon_u_start, colored_stand_icon_v_start, colored_stand_icon_u_end, colored_stand_icon_v_end,
							  stand_icon_x_start, stand_icon_y_start, stand_icon_x_end, stand_icon_y_end);
			else
				draw_2d_thing(stand_icon_u_start, stand_icon_v_start, stand_icon_u_end, stand_icon_v_end,
							  stand_icon_x_start, stand_icon_y_start, stand_icon_x_end, stand_icon_y_end);
		}

	if(view_sigils_menu || (in_window && mouse_x>spell_icon_x_start && mouse_x<spell_icon_x_end))
		draw_2d_thing(colored_spell_icon_u_start, colored_spell_icon_v_start, colored_spell_icon_u_end, colored_spell_icon_v_end,
					  spell_icon_x_start, spell_icon_y_start, spell_icon_x_end, spell_icon_y_end);
	else
		draw_2d_thing(spell_icon_u_start, spell_icon_v_start, spell_icon_u_end, spell_icon_v_end,
					  spell_icon_x_start, spell_icon_y_start, spell_icon_x_end, spell_icon_y_end);

	if(action_mode==action_attack || (in_window && mouse_x>attack_icon_x_start && mouse_x<attack_icon_x_end))
		draw_2d_thing(colored_attack_icon_u_start, colored_attack_icon_v_start, colored_attack_icon_u_end, colored_attack_icon_v_end,
					  attack_icon_x_start, attack_icon_y_start, attack_icon_x_end, attack_icon_y_end);
	else
		draw_2d_thing(attack_icon_u_start, attack_icon_v_start, attack_icon_u_end, attack_icon_v_end,
					  attack_icon_x_start, attack_icon_y_start, attack_icon_x_end, attack_icon_y_end);


	if(view_my_items || (in_window && mouse_x>inventory_icon_x_start && mouse_x<inventory_icon_x_end))
		draw_2d_thing(colored_inventory_icon_u_start, colored_inventory_icon_v_start, colored_inventory_icon_u_end, colored_inventory_icon_v_end,
					  inventory_icon_x_start, inventory_icon_y_start, inventory_icon_x_end, inventory_icon_y_end);
	else
		draw_2d_thing(inventory_icon_u_start, inventory_icon_v_start, inventory_icon_u_end, inventory_icon_v_end,
					  inventory_icon_x_start, inventory_icon_y_start, inventory_icon_x_end, inventory_icon_y_end);

	if(view_manufacture_menu || (in_window && mouse_x>manufacture_icon_x_start && mouse_x<manufacture_icon_x_end))
		draw_2d_thing(colored_manufacture_icon_u_start, colored_manufacture_icon_v_start, colored_manufacture_icon_u_end, colored_manufacture_icon_v_end,
					  manufacture_icon_x_start, manufacture_icon_y_start, manufacture_icon_x_end, manufacture_icon_y_end);
	else
		draw_2d_thing(manufacture_icon_u_start, manufacture_icon_v_start, manufacture_icon_u_end, manufacture_icon_v_end,
					  manufacture_icon_x_start, manufacture_icon_y_start, manufacture_icon_x_end, manufacture_icon_y_end);

	if(view_self_stats || (in_window && mouse_x>stats_icon_x_start && mouse_x<stats_icon_x_end ))
		draw_2d_thing(colored_stats_icon_u_start, colored_stats_icon_v_start, colored_stats_icon_u_end, colored_stats_icon_v_end,
					  stats_icon_x_start, stats_icon_y_start, stats_icon_x_end, stats_icon_y_end);
	else
		draw_2d_thing(stats_icon_u_start, stats_icon_v_start, stats_icon_u_end, stats_icon_v_end,
					  stats_icon_x_start, stats_icon_y_start, stats_icon_x_end, stats_icon_y_end);

	if(view_knowledge || (in_window && mouse_x>knowledge_icon_x_start && mouse_x<knowledge_icon_x_end))
		draw_2d_thing(colored_knowledge_icon_u_start, colored_knowledge_icon_v_start, colored_knowledge_icon_u_end, colored_knowledge_icon_v_end,
					  knowledge_icon_x_start, knowledge_icon_y_start, knowledge_icon_x_end, knowledge_icon_y_end);
	else
		draw_2d_thing(knowledge_icon_u_start, knowledge_icon_v_start, knowledge_icon_u_end, knowledge_icon_v_end,
					  knowledge_icon_x_start, knowledge_icon_y_start, knowledge_icon_x_end, knowledge_icon_y_end);

	if(view_encyclopedia || (in_window && mouse_x>encyclopedia_icon_x_start && mouse_x<encyclopedia_icon_x_end))
		draw_2d_thing(colored_encyclopedia_icon_u_start, colored_encyclopedia_icon_v_start, colored_encyclopedia_icon_u_end, colored_encyclopedia_icon_v_end,
					  encyclopedia_icon_x_start, encyclopedia_icon_y_start, encyclopedia_icon_x_end, encyclopedia_icon_y_end);
	else
		draw_2d_thing(encyclopedia_icon_u_start, encyclopedia_icon_v_start, encyclopedia_icon_u_end, encyclopedia_icon_v_end,
					  encyclopedia_icon_x_start, encyclopedia_icon_y_start, encyclopedia_icon_x_end, encyclopedia_icon_y_end);

	if(view_questlog || (in_window && mouse_x>questlog_icon_x_start && mouse_x<questlog_icon_x_end))
		draw_2d_thing(colored_questlog_icon_u_start, colored_questlog_icon_v_start, colored_questlog_icon_u_end, colored_questlog_icon_v_end,
					  questlog_icon_x_start, questlog_icon_y_start, questlog_icon_x_end, questlog_icon_y_end);
	else
		draw_2d_thing(questlog_icon_u_start, questlog_icon_v_start, questlog_icon_u_end, questlog_icon_v_end,
					  questlog_icon_x_start, questlog_icon_y_start, questlog_icon_x_end, questlog_icon_y_end);

	if(interface_mode==interface_console || (in_window && mouse_x>console_icon_x_start && mouse_x<console_icon_x_end))
		draw_2d_thing(colored_console_icon_u_start, colored_console_icon_v_start, colored_console_icon_u_end, colored_console_icon_v_end,
					  console_icon_x_start, console_icon_y_start, console_icon_x_end, console_icon_y_end);
	else
		draw_2d_thing(console_icon_u_start, console_icon_v_start, console_icon_u_end, console_icon_v_end,
					  console_icon_x_start, console_icon_y_start, console_icon_x_end, console_icon_y_end);

	if(options_menu || (in_window && mouse_x>options_icon_x_start && mouse_x<options_icon_x_end))
		draw_2d_thing(colored_options_icon_u_start, colored_options_icon_v_start, colored_options_icon_u_end, colored_options_icon_v_end,
					  options_icon_x_start, options_icon_y_start, options_icon_x_end, options_icon_y_end);
	else
		draw_2d_thing(options_icon_u_start, options_icon_v_start, options_icon_u_end, options_icon_v_end,
					  options_icon_x_start, options_icon_y_start, options_icon_x_end, options_icon_y_end);

	if(interface_mode==interface_map || (in_window && mouse_x>map_icon_x_start && mouse_x<map_icon_x_end))
		draw_2d_thing(colored_map_icon_u_start, colored_map_icon_v_start, colored_map_icon_u_end, colored_map_icon_v_end,
					  map_icon_x_start, map_icon_y_start, map_icon_x_end, map_icon_y_end);
	else
		draw_2d_thing(map_icon_u_start, map_icon_v_start, map_icon_u_end, map_icon_v_end,
					  map_icon_x_start, map_icon_y_start, map_icon_x_end, map_icon_y_end);

	if(view_buddy || (in_window && mouse_x>buddy_icon_x_start && mouse_x<buddy_icon_x_end))
		draw_2d_thing(colored_buddy_icon_u_start, colored_buddy_icon_v_start, colored_buddy_icon_u_end, colored_buddy_icon_v_end,
					  buddy_icon_x_start, buddy_icon_y_start, buddy_icon_x_end, buddy_icon_y_end);
	else
		draw_2d_thing(buddy_icon_u_start, buddy_icon_v_start, buddy_icon_u_end, buddy_icon_v_end,
					  buddy_icon_x_start, buddy_icon_y_start, buddy_icon_x_end, buddy_icon_y_end);

	glEnd();
	//glDisable(GL_ALPHA_TEST);
}

int check_peace_icons()
{
    return(click_in_window(icons_win, mouse_x, mouse_y, 0));
}

int	click_icons_handler(window_info *win, int mx, int my, Uint32 flags)
{
	if(combat_mode)return 0;

	if(mx>options_icon_x_start && mx<options_icon_x_end)
		options_menu=!options_menu;
	else if(mx>knowledge_icon_x_start && mx<knowledge_icon_x_end)
		{
			view_knowledge=!view_knowledge;
		}
	else if(mx>eye_icon_x_start && mx<eye_icon_x_end)
		action_mode=action_look;
	else if(mx>walk_icon_x_start && mx<walk_icon_x_end)
		action_mode=action_walk;
	else if(mx>trade_icon_x_start && mx<trade_icon_x_end)
		action_mode=action_trade;
	else if(mx>use_icon_x_start && mx<use_icon_x_end)
		action_mode=action_use;
	else if(mx>attack_icon_x_start && mx<attack_icon_x_end)
		action_mode=action_attack;
	else if(mx>manufacture_icon_x_start && mx<manufacture_icon_x_end)
		{
			if(!view_manufacture_menu)
				{
					if(view_trade_menu)
						{
							log_to_console(c_red2,"You can't manufacture while on trade.");
							return 0;
						}
				}
			view_manufacture_menu=!view_manufacture_menu;
		}
	else if(mx>spell_icon_x_start && mx<spell_icon_x_end)
		{
			if(view_trade_menu)
				{
					log_to_console(c_red2,"You can't cast spells while on trade.");
					return 0;
				}
			view_sigils_menu=!view_sigils_menu;
		}
	else if(mx>stats_icon_x_start && mx<stats_icon_x_end)
		{
			view_self_stats=!view_self_stats;
		}
	else if(mx>inventory_icon_x_start && mx<inventory_icon_x_end)
		{
			if(!view_my_items)
				{
					if(view_trade_menu)
						{
							log_to_console(c_red2,"You can't view your inventory items while on trade.");
							return 0;
						}
					view_my_items=1;
				}
			else view_my_items=0;
		}
	else if(mx>sit_icon_x_start && mx<sit_icon_x_end) {
		if(!you_sit)
			{
				Uint8 str[4];
				str[0]=SIT_DOWN;
				str[1]=1;
				my_tcp_send(my_socket,str,2);
			}
		else
			{
				Uint8 str[4];
				str[0]=SIT_DOWN;
				str[1]=0;
				my_tcp_send(my_socket,str,2);
			}
	}
	else if(mx>encyclopedia_icon_x_start && mx<encyclopedia_icon_x_end)
		{
			view_encyclopedia=!view_encyclopedia;
		}
	else if(mx>questlog_icon_x_start && mx<questlog_icon_x_end)
		{
			view_questlog=!view_questlog;
		}
	else if(mx>console_icon_x_start && mx<console_icon_x_end)
		{
			if(interface_mode==interface_console)interface_mode=interface_game;
						else interface_mode=interface_console;
		}
	else if(mx>map_icon_x_start && mx<map_icon_x_end)
		{
			if(interface_mode==interface_game)switch_to_game_map();
						else if(interface_mode==interface_map)switch_from_game_map();
	}else if(mx>buddy_icon_x_start && mx<buddy_icon_x_end)
		{
			view_buddy=!view_buddy;
	}
	return 1;
}


// the stats display
void init_stats_display()
{
	//create the stats bar window
	stats_bar_win= create_window("Stats Bar", 0, 0, 24, window_height-44, window_width-24-64, 12, ELW_TITLE_NONE|ELW_SHOW|ELW_SHOW_LAST);
	set_window_handler(stats_bar_win, ELW_HANDLER_DISPLAY, &display_stats_bar_handler);
	//set_window_handler(stats_bar_win, ELW_HANDLER_CLICK, &click_stats_bar_handler);
	
	mana_bar_start_x=0;
	mana_bar_start_y=0;

	food_bar_start_x=mana_bar_start_x+100+40;
	food_bar_start_y=mana_bar_start_y;

	health_bar_start_x=food_bar_start_x+100+40;
	health_bar_start_y=mana_bar_start_y;

	load_bar_start_x=health_bar_start_x+100+40;
	load_bar_start_y=mana_bar_start_y;

	exp_bar_start_x=load_bar_start_x+100+70;
	exp_bar_start_y=mana_bar_start_y;
}

void draw_stats_bar(int x, int y, int val, int len, float r, float g, float b, float r2, float g2, float b2)
{
	unsigned char	buf[32];

	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	//draw the colored section
	glColor3f(r2, g2, b2);
	glVertex3i(x, y+8, 0);
	glColor3f(r, g, b);
	glVertex3i(x, y, 0);
	glColor3f(r, g, b);
	glVertex3i(x+len, y, 0);
	glColor3f(r2, g2, b2);
	glVertex3i(x+len, y+8, 0);
	glEnd();

	// draw the bar frame
	glColor3f(0.77f, 0.57f, 0.39f);
	glBegin(GL_LINES);
	glVertex3i(x, y, 0);
	glVertex3i(x+100, y, 0);
	glVertex3i(x+100, y, 0);
	glVertex3i(x+100, y+8, 0);
	glVertex3i(x+100, y+8, 0);
	glVertex3i(x, y+8, 0);
	glVertex3i(x, y+8, 0);
	glVertex3i(x, y, 0);
	glEnd();
	glEnable(GL_TEXTURE_2D);

	// handle the text
	snprintf(buf, 32, "%d", val);
	glColor3f(0.8f, 0.8f, 0.8f);
	draw_string_small(x-(1+8*strlen(buf)), y-3, buf, 1);
}

void draw_stats_display()
{
    display_window(stats_bar_win);
}

int	display_stats_bar_handler(window_info *win)
{
	float health_adjusted_x_len;
	float food_adjusted_x_len;
	float mana_adjusted_x_len;
	float load_adjusted_x_len;

	//get the adjusted lenght

	if(!your_info.material_points.cur || !your_info.material_points.base)
		health_adjusted_x_len=0;//we don't want a div by 0
	else
		health_adjusted_x_len=100/((float)your_info.material_points.base/(float)your_info.material_points.cur);

	if(your_info.food_level<=0)
		food_adjusted_x_len=0;//we don't want a div by 0
	else
		food_adjusted_x_len=100/(45.0f/(float)your_info.food_level);

	if(!your_info.ethereal_points.cur || !your_info.ethereal_points.base)
		mana_adjusted_x_len=0;//we don't want a div by 0
	else
		mana_adjusted_x_len=100/((float)your_info.ethereal_points.base/(float)your_info.ethereal_points.cur);

	if(!your_info.carry_capacity.cur || !your_info.carry_capacity.base)
		load_adjusted_x_len=0;//we don't want a div by 0
	else
		load_adjusted_x_len=100/((float)your_info.carry_capacity.base/(float)your_info.carry_capacity.cur);

	draw_stats_bar(health_bar_start_x, health_bar_start_y, your_info.material_points.cur, health_adjusted_x_len, 1.0f, 0.2f, 0.2f, 0.5f, 0.2f, 0.2f);
	draw_stats_bar(food_bar_start_x, food_bar_start_y, your_info.food_level, food_adjusted_x_len, 1.0f, 1.0f, 0.2f, 0.5f, 0.5f, 0.2f);
	draw_stats_bar(mana_bar_start_x, mana_bar_start_y, your_info.ethereal_points.cur, mana_adjusted_x_len, 0.2f, 0.2f, 1.0f, 0.2f, 0.2f, 0.5f);
	draw_stats_bar(load_bar_start_x, load_bar_start_y, your_info.carry_capacity.base-your_info.carry_capacity.cur, load_adjusted_x_len, 0.6f, 0.4f, 0.4f, 0.4f, 0.2f, 0.2f);
	if(win->len_x>640-64) draw_exp_display();
}

int check_stats_display()
{
	return 0;
}


// the misc section (compass, clock, ?)
float compass_u_start=(float)32/256;
float compass_v_start=1.0f-(float)192/256;

float compass_u_end=(float)95/256;
float compass_v_end=0;

float clock_u_start=0;
float clock_v_start=1.0f-(float)128/256;

float clock_u_end=(float)63/256;
float clock_v_end=1.0f-(float)191/256;

float needle_u_start=(float)4/256;
float needle_v_start=1.0f-(float)200/256;

float needle_u_end=(float)14/256;
float needle_v_end=1.0f-(float)246/256;

float clock_needle_u_start=(float)21/256;
float clock_needle_v_start=1.0f-(float)192/256;

float clock_needle_u_end=(float)31/256;
float clock_needle_v_end=1.0f-(float)223/256;


void init_misc_display()
{
	//create the icon window
	misc_win= create_window("Misc", 0, 0, window_width-64, window_height-145, 64, 145, ELW_TITLE_NONE|ELW_SHOW|ELW_SHOW_LAST);
	set_window_handler(misc_win, ELW_HANDLER_DISPLAY, &display_misc_handler);
	set_window_handler(misc_win, ELW_HANDLER_CLICK, &click_misc_handler);
}

void draw_misc_display()
{
    display_window(misc_win);
}

int	display_misc_handler(window_info *win)
{
    //draw the compass
	glBegin(GL_QUADS);
	draw_2d_thing(compass_u_start, compass_v_start, compass_u_end, compass_v_end, 0,win->len_y-64,64,win->len_y);
	glEnd();

	//draw the compass needle
    glEnable(GL_ALPHA_TEST);//enable alpha filtering, so we have some alpha key
    glAlphaFunc(GL_GREATER, 0.09f);
	glPushMatrix();
	glTranslatef(32, win->len_y-32, 0);
	glRotatef(compass_direction*rz, 0.0f, 0.0f, 1.0f);

	glBegin(GL_QUADS);
	draw_2d_thing(needle_u_start, needle_v_start, needle_u_end, needle_v_end,-5, -28, 5, 28);
	glEnd();
	glPopMatrix();
	glDisable(GL_ALPHA_TEST);

	//draw the clock
	glBegin(GL_QUADS);
	draw_2d_thing(clock_u_start, clock_v_start, clock_u_end, clock_v_end,
				  0, win->len_y-128, 64, win->len_y-64);
	glEnd();

	//draw the clock needle
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.05f);
	glPushMatrix();
	glTranslatef(32, win->len_y-98, 0);
	glRotatef(game_minute, 0.0f, 0.0f, 1.0f);
	glBegin(GL_QUADS);
	draw_2d_thing(clock_needle_u_start, clock_needle_v_start, clock_needle_u_end, clock_needle_v_end, -5, -24,5, 6);
	glEnd();
	glPopMatrix();
	glDisable(GL_ALPHA_TEST);

	//Digital Clock
	if(view_digital_clock==1){
		char str[5];
		snprintf(str,5,"%1d:%02d", game_minute/60, game_minute%60);
		glColor3f(0.77f,0.57f,0.39f);
		draw_string(3,12,str,1);
	}
}

int check_misc_display()
{
	return(click_in_window(misc_win, mouse_x, mouse_y, 0));
}

int	click_misc_handler(window_info *win, int mx, int my, Uint32 flags)
{
    //check to see if we clicked on the clock
	if(my>win->len_y-128 && my<win->len_y-64)
		{
			unsigned char protocol_name;

			protocol_name= GET_TIME;
			my_tcp_send(my_socket,&protocol_name,1);
			return 1;
		}
	//check to see if we clicked on the compass
	if(my>win->len_y-64 && my<win->len_y)
		{
			unsigned char protocol_name;

			protocol_name= LOCATE_ME;
			my_tcp_send(my_socket,&protocol_name,1);
			return 1;
		}

	return 0;
}

int quickbar_x_len= 30;
int quickbar_y_len= 6*30;
int quickbar_x= 0;
int quickbar_y= 0;

//quickbar section
void init_quickbar() {
	quickbar_x_len= 30;
	quickbar_y_len= 6*30+1;
	quickbar_win= create_window("Quickbar", 0, 0, window_width-quickbar_x_len-4, 64, quickbar_x_len, quickbar_y_len, ELW_TITLE_NONE|ELW_SHOW|ELW_USE_BACKGROUND|ELW_USE_BORDER|ELW_SHOW_LAST);
	set_window_handler(quickbar_win, ELW_HANDLER_DISPLAY, &display_quickbar_handler);
	set_window_handler(quickbar_win, ELW_HANDLER_CLICK, &click_quickbar_handler);
}

void draw_quickbar() {
	quickbar_x= window_width-quickbar_x_len-4;
	quickbar_y= 64;
	// failsafe until better integrated
	init_window(quickbar_win, 0, 0, quickbar_x, quickbar_y, quickbar_x_len, quickbar_y_len);
	display_window(quickbar_win);
}

int	display_quickbar_handler(window_info *win)
{
	Uint8 str[80];
	int y, i;

	glBegin(GL_LINES);
	use_window_color(quickbar_win, ELW_COLOR_LINE);
	//draw the grid
	for(y=1;y<6;y++)
		{
			glVertex3i(0, y*30+1, 0);
			glVertex3i(quickbar_x_len, y*30+1, 0);
		}
	glEnd();
	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0f,1.0f,1.0f);
	//ok, now let's draw the objects...
	for(i=0;i<ITEM_NUM_ITEMS;i++)
		{
			if(item_list[i].quantity)
				{
					float u_start,v_start,u_end,v_end;
					int this_texture,cur_item,cur_pos;
					int x_start,x_end,y_start,y_end;

					//get the UV coordinates.
					cur_item=item_list[i].image_id%25;
					u_start=0.2f*(cur_item%5);
					u_end=u_start+(float)50/256;
					v_start=(1.0f+((float)50/256)/256.0f)-((float)50/256*(cur_item/5));
					v_end=v_start-(float)50/256;

					//get the x and y
					cur_pos=item_list[i].pos;
					if(cur_pos<6)//don't even check worn items
						{
							x_start= 1;
							x_end= x_start+29;
							y_start= 30*(cur_pos%6)+1;
							y_end= y_start+29;

							//get the texture this item belongs to
							this_texture=item_list[i].image_id/25;
							if(this_texture==0)this_texture=items_text_1;
							else if(this_texture==1)this_texture=items_text_2;
							else if(this_texture==2)this_texture=items_text_3;
							else if(this_texture==3)this_texture=items_text_4;
							else if(this_texture==4)this_texture=items_text_5;
							else if(this_texture==5)this_texture=items_text_6;
							else if(this_texture==6)this_texture=items_text_7;

							get_and_set_texture_id(this_texture);
							glBegin(GL_QUADS);
							draw_2d_thing(u_start,v_start,u_end,v_end,x_start,y_start,x_end,y_end);
							glEnd();
							sprintf(str,"%i",item_list[i].quantity);
							draw_string_small(x_start,y_end-15,str,1);
						}
				}
		}
	return 1;
}

int check_quickbar() {
	return(click_in_window(quickbar_win, mouse_x, mouse_y, 0));
}

int	click_quickbar_handler(window_info *win, int mx, int my, Uint32 flags)
{
	int i,y;
	int x_screen,y_screen;
	Uint8 str[100];

	// no in window check needed, already done
	//see if we clicked on any item in the main category
	for(y=0;y<6;y++)
		{
			x_screen=0;
			y_screen=y*30;
			if(mx>x_screen && mx<x_screen+30 && my>y_screen && my<y_screen+30)
				{
					//see if there is an empty space to drop this item over.
					if(item_dragged!=-1)//we have to drop this item
						{
							int any_item=0;
							for(i=0;i<ITEM_NUM_ITEMS;i++)
								{
									if(item_list[i].quantity && item_list[i].pos==y)
										{
											any_item=1;
											if(item_dragged==i)//drop the item only over itself
												item_dragged=-1;
											return 1;
										}
								}
							if(!any_item)
								{
									//send the drop info to the server
									str[0]=MOVE_INVENTORY_ITEM;
									str[1]=item_list[item_dragged].pos;
									str[2]=y;
									my_tcp_send(my_socket,str,3);
									item_dragged=-1;
									return 1;
								}
						}

					//see if there is any item there

					for(i=0;i<ITEM_NUM_ITEMS;i++)
						{
							//should we get the info for it?
							if(item_list[i].quantity && item_list[i].pos==y)
								{

									if(action_mode==action_look || (flags&ELW_RIGHT_MOUSE))
										{
											if(cur_time<(click_time+click_speed))
												if(item_list[i].use_with_inventory)
													{
														str[0]=USE_INVENTORY_ITEM;
														str[1]=item_list[i].pos;
														my_tcp_send(my_socket,str,2);
														return 1;
													}
											click_time=cur_time;
											str[0]=LOOK_AT_INVENTORY_ITEM;
											str[1]=item_list[i].pos;
											my_tcp_send(my_socket,str,2);
										}
									else if(action_mode==action_use)
										{
											if(item_list[i].use_with_inventory)
												{
													str[0]=USE_INVENTORY_ITEM;
													str[1]=item_list[i].pos;
													my_tcp_send(my_socket,str,2);
													return 1;
												}
											return 1;
										}
									else//we might test for other things first, like use or drop
										{
											if(item_dragged==-1)//we have to drag this item
												{
													item_dragged=i;
												}
										}

									return 1;
								}
						}
				}
		}
	return 1;
}

Uint32 exp_lev[140];

void build_levels_table()
{
  int i;
  int exp=100;

  exp_lev[0]=0;
  for(i=1;i<120;i++)
    {
        if(i<=10)exp+=exp*40/100;
        else
        if(i<=20)exp+=exp*30/100;
        else
        if(i<=30)exp+=exp*20/100;
        else
        if(i<=40)exp+=exp*14/100;
        else
        if(i<=90)exp+=exp*7/100;
        else exp+=exp*4/100;

        exp_lev[i]=exp;
    }
}



void draw_exp_display()
{
	int exp_adjusted_x_len;
	int nl_exp, baselev, cur_exp;
	int delta_exp;
	float prev_exp;

	switch(watch_this_stat){
	case 1: // attack
		cur_exp = your_info.attack_exp;
		baselev = your_info.attack_skill.base;
		break;
	case 2: // defense
		cur_exp = your_info.defense_exp;
		baselev = your_info.defense_skill.base;
		break;
	case 3: // harvest
		cur_exp = your_info.harvesting_exp;
		baselev = your_info.harvesting_skill.base;
		break;
	case 4: // alchemy
		cur_exp = your_info.alchemy_exp;
		baselev = your_info.alchemy_skill.base;
		break;
	case 5: // magic
		cur_exp = your_info.magic_exp;
		baselev = your_info.magic_skill.base;
		break;
	case 6: // potion
		cur_exp = your_info.potion_exp;
		baselev = your_info.potion_skill.base;
		break;
	case 7: // summoning
		cur_exp = your_info.summoning_exp;
		baselev = your_info.summoning_skill.base;
		break;
	case 8: // manufacture
		cur_exp = your_info.manufacturing_exp;
		baselev = your_info.manufacturing_skill.base;
		break;
	case 9: // crafting
		cur_exp = your_info.crafting_exp;
		baselev = your_info.crafting_skill.base;
		break;
	case 10: // overall
	default:
		cur_exp = your_info.overall_exp;
		baselev = your_info.overall_skill.base;
	}

	if(!baselev)
		prev_exp=0;
	else
		prev_exp=exp_lev[baselev];

	nl_exp=exp_lev[baselev+1];
	delta_exp=nl_exp-prev_exp;

	if(!cur_exp || !nl_exp)
		exp_adjusted_x_len = 0;
	else
		//exp_bar_length = (int)( (((float)cur_exp - prev_exp) / ((float)nl_exp - prev_exp)) * 100.0);
		exp_adjusted_x_len = 100-100.0f/(float)((float)delta_exp/(float)(nl_exp-cur_exp));

	draw_stats_bar(exp_bar_start_x, exp_bar_start_y, nl_exp - cur_exp, exp_adjusted_x_len, 0.1f, 0.8f, 0.1f, 0.1f, 0.4f, 0.1f);
}
