#include <string.h>
#include "global.h"
#include <math.h>


//int hud_x=64;
//int hud_y=49;
int hud_x=0;
int hud_y=0;


// initialize anything related to the hud
void init_hud_interface()
{
	init_peace_icons();
	init_misc_display();
	init_stats_display();
	init_quickbar();
}

// draw everything related to the hud
void draw_hud_interface()
{
	get_and_set_texture_id(icons_text);
	glColor3f(1.0f, 1.0f, 1.0f);

    draw_peace_icons();
    draw_misc_display();
    draw_stats_display();
	draw_quickbar();
}

// check to see if a mouse click was on the hud
int check_hud_interface()
{
	if(check_peace_icons())return 1;
	if(check_misc_display())return 1;
	if(check_stats_display())return 1;
	if(check_quickbar())return 1;

	// nothing done here
	return 0;
}


// the icons section
float walk_icon_u_start=(float)0/255;
float walk_icon_v_start=1.0f-(float)0/255;
float walk_icon_u_end=(float)31/255;
float walk_icon_v_end=1.0f-(float)31/255;

float colored_walk_icon_u_start=(float)64/255;
float colored_walk_icon_v_start=1.0f-(float)64/255;
float colored_walk_icon_u_end=(float)95/255;
float colored_walk_icon_v_end=1.0f-(float)95/255;

float run_icon_u_start=(float)32/255;
float run_icon_v_start=1.0f-(float)0/255;
float run_icon_u_end=(float)63/255;
float run_icon_v_end=1.0f-(float)31/255;

float eye_icon_u_start=(float)64/255;
float eye_icon_v_start=1.0f-(float)0/255;
float eye_icon_u_end=(float)95/255;
float eye_icon_v_end=1.0f-(float)31/255;

float colored_eye_icon_u_start=(float)128/255;
float colored_eye_icon_v_start=1.0f-(float)64/255;
float colored_eye_icon_u_end=(float)159/255;
float colored_eye_icon_v_end=1.0f-(float)95/255;

float trade_icon_u_start=(float)128/255;
float trade_icon_v_start=1.0f-(float)0/255;
float trade_icon_u_end=(float)159/255;
float trade_icon_v_end=1.0f-(float)31/255;

float colored_trade_icon_u_start=(float)192/255;
float colored_trade_icon_v_start=1.0f-(float)64/255;
float colored_trade_icon_u_end=(float)223/255;
float colored_trade_icon_v_end=1.0f-(float)95/255;

float follow_icon_u_start=(float)192/255;
float follow_icon_v_start=1.0f-(float)0/255;
float follow_icon_u_end=(float)223/255;
float follow_icon_v_end=1.0f-(float)31/255;

float sit_icon_u_start=(float)224/255;
float sit_icon_v_start=1.0f-(float)0/255;
float sit_icon_u_end=(float)255/255;
float sit_icon_v_end=1.0f-(float)31/255;

float colored_sit_icon_u_start=(float)32/255;
float colored_sit_icon_v_start=1.0f-(float)96/255;
float colored_sit_icon_u_end=(float)63/255;
float colored_sit_icon_v_end=1.0f-(float)127/255;

float stand_icon_u_start=(float)0/255;
float stand_icon_v_start=1.0f-(float)32/255;
float stand_icon_u_end=(float)31/255;
float stand_icon_v_end=1.0f-(float)63/255;

float colored_stand_icon_u_start=(float)64/255;
float colored_stand_icon_v_start=1.0f-(float)96/255;
float colored_stand_icon_u_end=(float)95/255;
float colored_stand_icon_v_end=1.0f-(float)127/255;

float spell_icon_u_start=(float)32/255;
float spell_icon_v_start=1.0f-(float)32/255;
float spell_icon_u_end=(float)63/255;
float spell_icon_v_end=1.0f-(float)63/255;

float colored_spell_icon_u_start=(float)96/255;
float colored_spell_icon_v_start=1.0f-(float)96/255;
float colored_spell_icon_u_end=(float)127/255;
float colored_spell_icon_v_end=1.0f-(float)127/255;

float inventory_icon_u_start=(float)96/255;
float inventory_icon_v_start=1.0f-(float)32/255;
float inventory_icon_u_end=(float)127/255;
float inventory_icon_v_end=1.0f-(float)63/255;

float colored_inventory_icon_u_start=(float)160/255;
float colored_inventory_icon_v_start=1.0f-(float)96/255;
float colored_inventory_icon_u_end=(float)192/255;
float colored_inventory_icon_v_end=1.0f-(float)127/255;

float manufacture_icon_u_start=(float)128/255;
float manufacture_icon_v_start=1.0f-(float)32/255;
float manufacture_icon_u_end=(float)159/255;
float manufacture_icon_v_end=1.0f-(float)63/255;

float colored_manufacture_icon_u_start=(float)0/255;
float colored_manufacture_icon_v_start=1.0f-(float)128/255;
float colored_manufacture_icon_u_end=(float)31/255;
float colored_manufacture_icon_v_end=1.0f-(float)159/255;

float stats_icon_u_start=(float)160/255;
float stats_icon_v_start=1.0f-(float)32/255;
float stats_icon_u_end=(float)191/255;
float stats_icon_v_end=1.0f-(float)63/255;

float colored_stats_icon_u_start=(float)32/255;
float colored_stats_icon_v_start=1.0f-(float)128/255;
float colored_stats_icon_u_end=(float)63/255;
float colored_stats_icon_v_end=1.0f-(float)159/255;

float options_icon_u_start=(float)192/255;
float options_icon_v_start=1.0f-(float)32/255;
float options_icon_u_end=(float)223/255;
float options_icon_v_end=1.0f-(float)63/255;

float colored_options_icon_u_start=(float)64/255;
float colored_options_icon_v_start=1.0f-(float)128/255;
float colored_options_icon_u_end=(float)95/255;
float colored_options_icon_v_end=1.0f-(float)159/255;

float use_icon_u_start=(float)224/255;
float use_icon_v_start=1.0f-(float)32/255;
float use_icon_u_end=(float)255/255;
float use_icon_v_end=1.0f-(float)63/255;

float colored_use_icon_u_start=(float)96/255;
float colored_use_icon_v_start=1.0f-(float)128/255;
float colored_use_icon_u_end=(float)127/255;
float colored_use_icon_v_end=1.0f-(float)159/255;

float attack_icon_u_start=(float)160/255;
float attack_icon_v_start=1.0f-(float)0/255;
float attack_icon_u_end=(float)191/255;
float attack_icon_v_end=1.0f-(float)31/255;

float colored_attack_icon_u_start=(float)224/255;
float colored_attack_icon_v_start=1.0f-(float)64/255;
float colored_attack_icon_u_end=(float)255/255;
float colored_attack_icon_v_end=1.0f-(float)91/255;

float knowledge_icon_u_start=(float)0/255;
float knowledge_icon_v_start=1.0f-(float)64/255;
float knowledge_icon_u_end=(float)31/255;
float knowledge_icon_v_end=1.0f-(float)95/255;

float colored_knowledge_icon_u_start=(float)32/255;
float colored_knowledge_icon_v_start=1.0f-(float)64/255;
float colored_knowledge_icon_u_end=(float)63/255;
float colored_knowledge_icon_v_end=1.0f-(float)95/255;

// until we have an incon using the knowledge one
float encyclopedia_icon_u_start=(float)0/255;
float encyclopedia_icon_v_start=1.0f-(float)64/255;
float encyclopedia_icon_u_end=(float)31/255;
float encyclopedia_icon_v_end=1.0f-(float)95/255;

float colored_encyclopedia_icon_u_start=(float)32/255;
float colored_encyclopedia_icon_v_start=1.0f-(float)64/255;
float colored_encyclopedia_icon_u_end=(float)63/255;
float colored_encyclopedia_icon_v_end=1.0f-(float)95/255;

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

//stat bars
int health_bar_start_x;
int health_bar_start_y;
int health_bar_x_len;
int health_bar_y_len;

int mana_bar_start_x;
int mana_bar_start_y;
int mana_bar_x_len;
int mana_bar_y_len;

int food_bar_start_x;
int food_bar_start_y;
int food_bar_x_len;
int food_bar_y_len;

void init_peace_icons()
{
	//TODO: positions should be based on icon size
	//TODO: icon position and layout options (I.E. verticall, on top, etc)

	walk_icon_x_start=0;
	walk_icon_x_end=walk_icon_x_start+32;
	walk_icon_y_start=window_height-32;
	walk_icon_y_end=walk_icon_y_start+32;

	sit_icon_x_start=walk_icon_x_end+1;
	sit_icon_x_end=sit_icon_x_start+32;
	sit_icon_y_start=window_height-32;
	sit_icon_y_end=sit_icon_y_start+32;

	stand_icon_x_start=walk_icon_x_end+1;
	stand_icon_x_end=stand_icon_x_start+32;
	stand_icon_y_start=window_height-32;
	stand_icon_y_end=stand_icon_y_start+32;

	eye_icon_x_start=stand_icon_x_end+1;
	eye_icon_x_end=eye_icon_x_start+32;
	eye_icon_y_start=window_height-32;
	eye_icon_y_end=eye_icon_y_start+32;

	use_icon_x_start=eye_icon_x_end+1;
	use_icon_x_end=use_icon_x_start+32;
	use_icon_y_start=window_height-32;
	use_icon_y_end=use_icon_y_start+32;

	trade_icon_x_start=use_icon_x_end+1;
	trade_icon_x_end=trade_icon_x_start+32;
	trade_icon_y_start=window_height-32;
	trade_icon_y_end=trade_icon_y_start+32;

	inventory_icon_x_start=trade_icon_x_end+1;
	inventory_icon_x_end=inventory_icon_x_start+32;
	inventory_icon_y_start=window_height-32;
	inventory_icon_y_end=inventory_icon_y_start+32;

	spell_icon_x_start=inventory_icon_x_end+1;
	spell_icon_x_end=spell_icon_x_start+32;
	spell_icon_y_start=window_height-32;
	spell_icon_y_end=spell_icon_y_start+32;

	attack_icon_x_start=spell_icon_x_end+1;
	attack_icon_x_end=attack_icon_x_start+32;
	attack_icon_y_start=window_height-32;
	attack_icon_y_end=attack_icon_y_start+32;

	manufacture_icon_x_start=attack_icon_x_end+1;
	manufacture_icon_x_end=manufacture_icon_x_start+32;
	manufacture_icon_y_start=window_height-32;
	manufacture_icon_y_end=manufacture_icon_y_start+32;

	stats_icon_x_start=manufacture_icon_x_end+1;
	stats_icon_x_end=stats_icon_x_start+32;
	stats_icon_y_start=window_height-32;
	stats_icon_y_end=stats_icon_y_start+32;

	knowledge_icon_x_start=stats_icon_x_end+1;
	knowledge_icon_x_end=knowledge_icon_x_start+32;
	knowledge_icon_y_start=window_height-32;
	knowledge_icon_y_end=knowledge_icon_y_start+32;

	encyclopedia_icon_x_start=knowledge_icon_x_end+1;
	encyclopedia_icon_x_end=encyclopedia_icon_x_start+32;
	encyclopedia_icon_y_start=window_height-32;
	encyclopedia_icon_y_end=encyclopedia_icon_y_start+32;

	options_icon_x_start=encyclopedia_icon_x_end+1;
	options_icon_x_end=options_icon_x_start+32;
	options_icon_y_start=window_height-32;
	options_icon_y_end=options_icon_y_start+32;
}

void draw_peace_icons()
{
	get_and_set_texture_id(icons_text);
	glColor3f(1.0f,1.0f,1.0f);

	glEnable(GL_ALPHA_TEST);//enable alpha filtering, so we have some alpha key
	glAlphaFunc(GL_GREATER,0.03f);
	glBegin(GL_QUADS);

	if(action_mode==action_walk || (mouse_x>walk_icon_x_start && mouse_y>walk_icon_y_start &&
									mouse_x<walk_icon_x_end && mouse_y<walk_icon_y_end))
		draw_2d_thing(colored_walk_icon_u_start, colored_walk_icon_v_start, colored_walk_icon_u_end, colored_walk_icon_v_end,
					  walk_icon_x_start, walk_icon_y_start, walk_icon_x_end, walk_icon_y_end);
	else
		draw_2d_thing(walk_icon_u_start, walk_icon_v_start, walk_icon_u_end, walk_icon_v_end,
					  walk_icon_x_start, walk_icon_y_start, walk_icon_x_end, walk_icon_y_end);

	if(action_mode==action_look || (mouse_x>eye_icon_x_start && mouse_y>eye_icon_y_start &&
									mouse_x<eye_icon_x_end && mouse_y<eye_icon_y_end))
		draw_2d_thing(colored_eye_icon_u_start, colored_eye_icon_v_start, colored_eye_icon_u_end, colored_eye_icon_v_end,
					  eye_icon_x_start, eye_icon_y_start, eye_icon_x_end, eye_icon_y_end);
	else
		draw_2d_thing(eye_icon_u_start, eye_icon_v_start, eye_icon_u_end, eye_icon_v_end,
					  eye_icon_x_start, eye_icon_y_start, eye_icon_x_end, eye_icon_y_end);

	if(action_mode==action_use || (mouse_x>use_icon_x_start && mouse_y>use_icon_y_start &&
								   mouse_x<use_icon_x_end && mouse_y<use_icon_y_end))
		draw_2d_thing(colored_use_icon_u_start, colored_use_icon_v_start, colored_use_icon_u_end, colored_use_icon_v_end,
					  use_icon_x_start, use_icon_y_start, use_icon_x_end, use_icon_y_end);
	else
		draw_2d_thing(use_icon_u_start, use_icon_v_start, use_icon_u_end, use_icon_v_end,
					  use_icon_x_start, use_icon_y_start, use_icon_x_end, use_icon_y_end);


	if(action_mode==action_trade || (mouse_x>trade_icon_x_start && mouse_y>trade_icon_y_start &&
									 mouse_x<trade_icon_x_end && mouse_y<trade_icon_y_end))
		draw_2d_thing(colored_trade_icon_u_start, colored_trade_icon_v_start, colored_trade_icon_u_end, colored_trade_icon_v_end,
					  trade_icon_x_start, trade_icon_y_start, trade_icon_x_end, trade_icon_y_end);
	else
		draw_2d_thing(trade_icon_u_start, trade_icon_v_start, trade_icon_u_end, trade_icon_v_end,
					  trade_icon_x_start, trade_icon_y_start, trade_icon_x_end, trade_icon_y_end);



	if(!you_sit)
		{
			if(mouse_x>sit_icon_x_start && mouse_y>sit_icon_y_start &&
			   mouse_x<sit_icon_x_end && mouse_y<sit_icon_y_end)
				draw_2d_thing(colored_sit_icon_u_start, colored_sit_icon_v_start, colored_sit_icon_u_end, colored_sit_icon_v_end,
							  sit_icon_x_start, sit_icon_y_start, sit_icon_x_end, sit_icon_y_end);
			else
				draw_2d_thing(sit_icon_u_start, sit_icon_v_start, sit_icon_u_end, sit_icon_v_end,
							  sit_icon_x_start, sit_icon_y_start, sit_icon_x_end, sit_icon_y_end);
		}
	else
		{
			if(mouse_x>sit_icon_x_start && mouse_y>sit_icon_y_start &&
			   mouse_x<sit_icon_x_end && mouse_y<sit_icon_y_end)
				draw_2d_thing(colored_stand_icon_u_start, colored_stand_icon_v_start, colored_stand_icon_u_end, colored_stand_icon_v_end,
							  stand_icon_x_start, stand_icon_y_start, stand_icon_x_end, stand_icon_y_end);
			else
				draw_2d_thing(stand_icon_u_start, stand_icon_v_start, stand_icon_u_end, stand_icon_v_end,
							  stand_icon_x_start, stand_icon_y_start, stand_icon_x_end, stand_icon_y_end);
		}

	if(mouse_x>spell_icon_x_start && mouse_y>spell_icon_y_start &&
	   mouse_x<spell_icon_x_end && mouse_y<spell_icon_y_end)
		draw_2d_thing(colored_spell_icon_u_start, colored_spell_icon_v_start, colored_spell_icon_u_end, colored_spell_icon_v_end,
					  spell_icon_x_start, spell_icon_y_start, spell_icon_x_end, spell_icon_y_end);
	else
		draw_2d_thing(spell_icon_u_start, spell_icon_v_start, spell_icon_u_end, spell_icon_v_end,
					  spell_icon_x_start, spell_icon_y_start, spell_icon_x_end, spell_icon_y_end);

	if(action_mode==action_attack || (mouse_x>attack_icon_x_start && mouse_y>attack_icon_y_start &&
									  mouse_x<attack_icon_x_end && mouse_y<attack_icon_y_end))
		draw_2d_thing(colored_attack_icon_u_start, colored_attack_icon_v_start, colored_attack_icon_u_end, colored_attack_icon_v_end,
					  attack_icon_x_start, attack_icon_y_start, attack_icon_x_end, attack_icon_y_end);
	else
		draw_2d_thing(attack_icon_u_start, attack_icon_v_start, attack_icon_u_end, attack_icon_v_end,
					  attack_icon_x_start, attack_icon_y_start, attack_icon_x_end, attack_icon_y_end);


	if(mouse_x>inventory_icon_x_start && mouse_y>inventory_icon_y_start &&
	   mouse_x<inventory_icon_x_end && mouse_y<inventory_icon_y_end)
		draw_2d_thing(colored_inventory_icon_u_start, colored_inventory_icon_v_start, colored_inventory_icon_u_end, colored_inventory_icon_v_end,
					  inventory_icon_x_start, inventory_icon_y_start, inventory_icon_x_end, inventory_icon_y_end);
	else
		draw_2d_thing(inventory_icon_u_start, inventory_icon_v_start, inventory_icon_u_end, inventory_icon_v_end,
					  inventory_icon_x_start, inventory_icon_y_start, inventory_icon_x_end, inventory_icon_y_end);

	if(mouse_x>manufacture_icon_x_start && mouse_y>manufacture_icon_y_start &&
	   mouse_x<manufacture_icon_x_end && mouse_y<manufacture_icon_y_end)
		draw_2d_thing(colored_manufacture_icon_u_start, colored_manufacture_icon_v_start, colored_manufacture_icon_u_end, colored_manufacture_icon_v_end,
					  manufacture_icon_x_start, manufacture_icon_y_start, manufacture_icon_x_end, manufacture_icon_y_end);
	else
		draw_2d_thing(manufacture_icon_u_start, manufacture_icon_v_start, manufacture_icon_u_end, manufacture_icon_v_end,
					  manufacture_icon_x_start, manufacture_icon_y_start, manufacture_icon_x_end, manufacture_icon_y_end);

	if(mouse_x>stats_icon_x_start && mouse_y>stats_icon_y_start &&
	   mouse_x<stats_icon_x_end && mouse_y<stats_icon_y_end)
		draw_2d_thing(colored_stats_icon_u_start, colored_stats_icon_v_start, colored_stats_icon_u_end, colored_stats_icon_v_end,
					  stats_icon_x_start, stats_icon_y_start, stats_icon_x_end, stats_icon_y_end);
	else
		draw_2d_thing(stats_icon_u_start, stats_icon_v_start, stats_icon_u_end, stats_icon_v_end,
					  stats_icon_x_start, stats_icon_y_start, stats_icon_x_end, stats_icon_y_end);

	if(mouse_x>knowledge_icon_x_start && mouse_y>knowledge_icon_y_start &&
	   mouse_x<knowledge_icon_x_end && mouse_y<knowledge_icon_y_end)
		draw_2d_thing(colored_knowledge_icon_u_start, colored_knowledge_icon_v_start, colored_knowledge_icon_u_end, colored_knowledge_icon_v_end,
					  knowledge_icon_x_start, knowledge_icon_y_start, knowledge_icon_x_end, knowledge_icon_y_end);
	else
		draw_2d_thing(knowledge_icon_u_start, knowledge_icon_v_start, knowledge_icon_u_end, knowledge_icon_v_end,
					  knowledge_icon_x_start, knowledge_icon_y_start, knowledge_icon_x_end, knowledge_icon_y_end);

	if(mouse_x>encyclopedia_icon_x_start && mouse_y>encyclopedia_icon_y_start &&
	   mouse_x<encyclopedia_icon_x_end && mouse_y<encyclopedia_icon_y_end)
		draw_2d_thing(colored_encyclopedia_icon_u_start, colored_encyclopedia_icon_v_start, colored_encyclopedia_icon_u_end, colored_encyclopedia_icon_v_end,
					  encyclopedia_icon_x_start, encyclopedia_icon_y_start, encyclopedia_icon_x_end, encyclopedia_icon_y_end);
	else
		draw_2d_thing(encyclopedia_icon_u_start, encyclopedia_icon_v_start, encyclopedia_icon_u_end, encyclopedia_icon_v_end,
					  encyclopedia_icon_x_start, encyclopedia_icon_y_start, encyclopedia_icon_x_end, encyclopedia_icon_y_end);

	if(mouse_x>options_icon_x_start && mouse_y>options_icon_y_start &&
	   mouse_x<options_icon_x_end && mouse_y<options_icon_y_end)
		draw_2d_thing(colored_options_icon_u_start, colored_options_icon_v_start, colored_options_icon_u_end, colored_options_icon_v_end,
					  options_icon_x_start, options_icon_y_start, options_icon_x_end, options_icon_y_end);
	else
		draw_2d_thing(options_icon_u_start, options_icon_v_start, options_icon_u_end, options_icon_v_end,
					  options_icon_x_start, options_icon_y_start, options_icon_x_end, options_icon_y_end);

	glEnd();
	glDisable(GL_ALPHA_TEST);
}

int check_peace_icons()
{
	if(combat_mode)return 0;
	if(mouse_x<walk_icon_x_start || mouse_y<walk_icon_y_start ||
	   mouse_x>options_icon_x_end || mouse_y>options_icon_y_end)return 0;

	if(mouse_x>options_icon_x_start && mouse_y>options_icon_y_start &&
	   mouse_x<options_icon_x_end && mouse_y<options_icon_y_end)
		options_menu=!options_menu;
	else if(mouse_x>knowledge_icon_x_start && mouse_y>knowledge_icon_y_start &&
			mouse_x<knowledge_icon_x_end && mouse_y<knowledge_icon_y_end)
		{
			view_knowledge=!view_knowledge;
		}
	else if(mouse_x>eye_icon_x_start && mouse_y>eye_icon_y_start &&
			mouse_x<eye_icon_x_end && mouse_y<eye_icon_y_end)
		action_mode=action_look;
	else if(mouse_x>walk_icon_x_start && mouse_y>walk_icon_y_start &&
			mouse_x<walk_icon_x_end && mouse_y<walk_icon_y_end)
		action_mode=action_walk;
	else if(mouse_x>trade_icon_x_start && mouse_y>trade_icon_y_start &&
			mouse_x<trade_icon_x_end && mouse_y<trade_icon_y_end)
		action_mode=action_trade;
	else if(mouse_x>use_icon_x_start && mouse_y>use_icon_y_start &&
			mouse_x<use_icon_x_end && mouse_y<use_icon_y_end)
		action_mode=action_use;
	else if(mouse_x>attack_icon_x_start && mouse_y>attack_icon_y_start &&
			mouse_x<attack_icon_x_end && mouse_y<attack_icon_y_end)
		action_mode=action_attack;
	else if(mouse_x>manufacture_icon_x_start && mouse_y>manufacture_icon_y_start &&
			mouse_x<manufacture_icon_x_end && mouse_y<manufacture_icon_y_end)
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
	else if(mouse_x>spell_icon_x_start && mouse_y>spell_icon_y_start &&
			mouse_x<spell_icon_x_end && mouse_y<spell_icon_y_end)
		{
			if(view_trade_menu)
				{
					log_to_console(c_red2,"You can't cast spells while on trade.");
					return 0;
				}
			view_sigils_menu=!view_sigils_menu;
		}
	else if(mouse_x>stats_icon_x_start && mouse_y>stats_icon_y_start &&
			mouse_x<stats_icon_x_end && mouse_y<stats_icon_y_end)
		{
			view_self_stats=!view_self_stats;
		}
	else if(mouse_x>inventory_icon_x_start && mouse_y>inventory_icon_y_start &&
			mouse_x<inventory_icon_x_end && mouse_y<inventory_icon_y_end)
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
	else if(mouse_x>sit_icon_x_start && mouse_y>sit_icon_y_start &&
			mouse_x<sit_icon_x_end && mouse_y<sit_icon_y_end) {
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
	else if(mouse_x>encyclopedia_icon_x_start && mouse_y>encyclopedia_icon_y_start &&
			mouse_x<encyclopedia_icon_x_end && mouse_y<encyclopedia_icon_y_end)
		{
			view_encyclopedia=!view_encyclopedia;
		}
	return 1;
}


// the stats display
void init_stats_display()
{
	mana_bar_start_x=24;
	mana_bar_start_y=window_height-44;
	mana_bar_x_len=100;
	mana_bar_y_len=8;

	food_bar_start_x=mana_bar_start_x+mana_bar_x_len+40;
	food_bar_start_y=mana_bar_start_y;
	food_bar_x_len=100;
	food_bar_y_len=8;

	health_bar_start_x=food_bar_start_x+food_bar_x_len+40;
	health_bar_start_y=mana_bar_start_y;
	health_bar_x_len=100;
	health_bar_y_len=8;
}

void draw_stats_display()
{
	float health_adjusted_x_len;
	float food_adjusted_x_len;
	float mana_adjusted_x_len;
	char health_str[32];
	char food_str[32];
	char mana_str[32];

	sprintf(health_str, "%3i",your_info.material_points.cur);
	sprintf(food_str, "%3i",your_info.food_level);
	sprintf(mana_str, "%3i",your_info.ethereal_points.cur);

	//get the adjusted lenght

	if(!your_info.material_points.cur || !your_info.material_points.base)
	health_adjusted_x_len=0;//we don't want a div by 0
	else
	health_adjusted_x_len=health_bar_x_len/((float)your_info.material_points.base/(float)your_info.material_points.cur);

	if(your_info.food_level<=0)
	food_adjusted_x_len=0;//we don't want a div by 0
	else
	food_adjusted_x_len=health_bar_x_len/(45.0f/(float)your_info.food_level);

	if(!your_info.ethereal_points.cur || !your_info.ethereal_points.base)
	mana_adjusted_x_len=0;//we don't want a div by 0
	else
	mana_adjusted_x_len=health_bar_x_len/((float)your_info.ethereal_points.base/(float)your_info.ethereal_points.cur);

	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	//draw the healthbar
	glColor3f(0.5f, 0.2f, 0.2f);
	glVertex3i(health_bar_start_x,health_bar_start_y+health_bar_y_len,0);
	glColor3f(1.0f, 0.2f, 0.2f);
	glVertex3i(health_bar_start_x,health_bar_start_y,0);
	glColor3f(1.0f, 0.2f, 0.2f);
	glVertex3i(health_bar_start_x+health_adjusted_x_len,health_bar_start_y,0);
	glColor3f(0.5f, 0.2f, 0.2f);
	glVertex3i(health_bar_start_x+health_adjusted_x_len,health_bar_start_y+health_bar_y_len,0);

	//draw the food bar
	glColor3f(0.5f, 0.5f, 0.2f);
	glVertex3i(food_bar_start_x,food_bar_start_y+food_bar_y_len,0);
	glColor3f(1.0f, 1.0f, 0.2f);
	glVertex3i(food_bar_start_x,food_bar_start_y,0);
	glColor3f(1.0f, 1.0f, 0.2f);
	glVertex3i(food_bar_start_x+food_adjusted_x_len,food_bar_start_y,0);
	glColor3f(0.5f, 0.5f, 0.2f);
	glVertex3i(food_bar_start_x+food_adjusted_x_len,food_bar_start_y+food_bar_y_len,0);

	//draw the food bar
	glColor3f(0.5f, 0.5f, 0.2f);
	glVertex3i(food_bar_start_x,food_bar_start_y+food_bar_y_len,0);
	glColor3f(1.0f, 1.0f, 0.2f);
	glVertex3i(food_bar_start_x,food_bar_start_y,0);
	glColor3f(1.0f, 1.0f, 0.2f);
	glVertex3i(food_bar_start_x+food_adjusted_x_len,food_bar_start_y,0);
	glColor3f(0.5f, 0.5f, 0.2f);
	glVertex3i(food_bar_start_x+food_adjusted_x_len,food_bar_start_y+food_bar_y_len,0);

	//draw the mana bar
	glColor3f(0.2f, 0.2f, 0.5f);
	glVertex3i(mana_bar_start_x,mana_bar_start_y+mana_bar_y_len,0);
	glColor3f(0.2f, 0.2f, 1.0f);
	glVertex3i(mana_bar_start_x,mana_bar_start_y,0);
	glColor3f(0.2f, 0.2f, 1.0f);
	glVertex3i(mana_bar_start_x+mana_adjusted_x_len,mana_bar_start_y,0);
	glColor3f(0.2f, 0.2f, 0.5f);
	glVertex3i(mana_bar_start_x+mana_adjusted_x_len,mana_bar_start_y+mana_bar_y_len,0);

	glEnd();


	glColor3f(0.4f, 0.4f, 0.4f);
	glBegin(GL_LINES);
	//draw the frame for the health bar
	glVertex3i(health_bar_start_x,health_bar_start_y,0);
	glVertex3i(health_bar_start_x+health_bar_x_len,health_bar_start_y,0);
	glVertex3i(health_bar_start_x+health_bar_x_len,health_bar_start_y,0);
	glVertex3i(health_bar_start_x+health_bar_x_len,health_bar_start_y+health_bar_y_len,0);
	glVertex3i(health_bar_start_x+health_bar_x_len,health_bar_start_y+health_bar_y_len,0);
	glVertex3i(health_bar_start_x,health_bar_start_y+health_bar_y_len,0);
	glVertex3i(health_bar_start_x,health_bar_start_y+health_bar_y_len,0);
	glVertex3i(health_bar_start_x,health_bar_start_y,0);

	//draw the frame for the food bar
	glVertex3i(food_bar_start_x,food_bar_start_y,0);
	glVertex3i(food_bar_start_x+food_bar_x_len,food_bar_start_y,0);
	glVertex3i(food_bar_start_x+food_bar_x_len,food_bar_start_y,0);
	glVertex3i(food_bar_start_x+food_bar_x_len,food_bar_start_y+food_bar_y_len,0);
	glVertex3i(food_bar_start_x+food_bar_x_len,food_bar_start_y+food_bar_y_len,0);
	glVertex3i(food_bar_start_x,food_bar_start_y+food_bar_y_len,0);
	glVertex3i(food_bar_start_x,food_bar_start_y+food_bar_y_len,0);
	glVertex3i(food_bar_start_x,food_bar_start_y,0);

	//draw the frame for the mana bar
	glVertex3i(mana_bar_start_x,mana_bar_start_y,0);
	glVertex3i(mana_bar_start_x+mana_bar_x_len,mana_bar_start_y,0);
	glVertex3i(mana_bar_start_x+mana_bar_x_len,mana_bar_start_y,0);
	glVertex3i(mana_bar_start_x+mana_bar_x_len,mana_bar_start_y+mana_bar_y_len,0);
	glVertex3i(mana_bar_start_x+mana_bar_x_len,mana_bar_start_y+mana_bar_y_len,0);
	glVertex3i(mana_bar_start_x,mana_bar_start_y+mana_bar_y_len,0);
	glVertex3i(mana_bar_start_x,mana_bar_start_y+mana_bar_y_len,0);
	glVertex3i(mana_bar_start_x,mana_bar_start_y,0);

	glEnd();
	glEnable(GL_TEXTURE_2D);

	glColor3f(0.8f, 0.8f, 0.8f);
	draw_string_small(health_bar_start_x-24,health_bar_start_y-3,health_str,1);
	draw_string_small(food_bar_start_x-24,food_bar_start_y-3,food_str,1);
	draw_string_small(mana_bar_start_x-24,mana_bar_start_y-3,mana_str,1);
}

int check_stats_display()
{
	return 0;
}


// the misc section (compass, clock, ?)
float compass_u_start=(float)34/255;
float compass_v_start=1.0f-(float)194/255;

float compass_u_end=(float)96/255;
float compass_v_end=1.0f-(float)255/255;

float clock_u_start=(float)98/255;
float clock_v_start=1.0f-(float)191/255;

float clock_u_end=(float)157/255;
float clock_v_end=1.0f-(float)255/255;

float needle_u_start=(float)4/255;
float needle_v_start=1.0f-(float)200/255;

float needle_u_end=(float)14/255;
float needle_v_end=1.0f-(float)246/255;

float clock_needle_u_start=(float)21/255;
float clock_needle_v_start=1.0f-(float)193/255;

float clock_needle_u_end=(float)31/255;
float clock_needle_v_end=1.0f-(float)224/255;


void init_misc_display()
{

}

void draw_misc_display()
{
    glEnable(GL_ALPHA_TEST);//enable alpha filtering, so we have some alpha key
    glAlphaFunc(GL_GREATER, 0.001f);

	if(view_compass)
		{
			glTranslatef(window_width-32, window_height-30, 0);
			glRotatef(-rz, 0.0f, 0.0f, 1.0f);

			glBegin(GL_QUADS);
			draw_2d_thing(compass_u_start, compass_v_start, compass_u_end, compass_v_end, -32, -30,32, 30);
			glEnd();

			//draw the compass needle
			glLoadIdentity();
			glBegin(GL_QUADS);
			draw_2d_thing(needle_u_start, needle_v_start, needle_u_end, needle_v_end,
						  window_width-36, window_height-56, window_width-28, window_height-8);
			glEnd();
		}

	if(view_clock)
		{
			//draw the clock
			glBegin(GL_QUADS);
			draw_2d_thing(clock_u_start, clock_v_start, clock_u_end, clock_v_end,
						  window_width-132, window_height-64, window_width-68, window_height);
			glEnd();

			//draw the clock needle
			glTranslatef(window_width-(132-32), window_height-32, 0);
			glRotatef(game_minute, 0.0f, 0.0f, 1.0f);
			glBegin(GL_QUADS);
			draw_2d_thing(clock_needle_u_start, clock_needle_v_start, clock_needle_u_end, clock_needle_v_end, -5, -24,5, 6);
			glEnd();
			glLoadIdentity();
		}
	glDisable(GL_ALPHA_TEST);
}

int check_misc_display()
{
	if(view_clock && mouse_x>window_width-132 && mouse_x<window_width-68
	   && mouse_y>window_height-64 && mouse_y<window_height)
		{
			unsigned char protocol_name;
			protocol_name=GET_TIME;
			my_tcp_send(my_socket,&protocol_name,1);
			return 1;
		}
	//check to see if we clicked on the compass
	if(view_compass && mouse_x>window_width-64 && mouse_x<window_width
	   && mouse_y>window_height-64 && mouse_y<window_height)
		{
			unsigned char protocol_name;

			protocol_name=LOCATE_ME;
			my_tcp_send(my_socket,&protocol_name,1);
			return 1;
		}

	return 0;
}

int quickbar_x_len=51;
int quickbar_y_len=6*51;
int quickbar_x=0;
int quickbar_y=0;

//quickbar section
void init_quickbar() {
	quickbar_x_len=51;
	quickbar_y_len=6*51+1;
}

void draw_quickbar() {
	Uint8 str[80];
	int y,i;
	quickbar_x=window_width-quickbar_x_len-4;
	quickbar_y=window_height-100-6*52;


	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE,GL_SRC_ALPHA);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glColor4f(0.0f,0.0f,0.0f,0.5f);
	glVertex3i(quickbar_x,quickbar_y+quickbar_y_len,0);
	glVertex3i(quickbar_x,quickbar_y,0);
	glVertex3i(quickbar_x+quickbar_x_len,quickbar_y,0);
	glVertex3i(quickbar_x+quickbar_x_len,quickbar_y+quickbar_y_len,0);
	glEnd();

	glDisable(GL_BLEND);
	glColor3f(0.77f,0.57f,0.39f);
	glBegin(GL_LINES);
	glVertex3i(quickbar_x,quickbar_y,0);
	glVertex3i(quickbar_x+quickbar_x_len,quickbar_y,0);

	glVertex3i(quickbar_x+quickbar_x_len,quickbar_y,0);
	glVertex3i(quickbar_x+quickbar_x_len,quickbar_y+quickbar_y_len,0);

	glVertex3i(quickbar_x+quickbar_x_len,quickbar_y+quickbar_y_len,0);
	glVertex3i(quickbar_x,quickbar_y+quickbar_y_len,0);

	glVertex3i(quickbar_x,quickbar_y+quickbar_y_len,0);
	glVertex3i(quickbar_x,quickbar_y,0);

	//draw the grid
	for(y=1;y<6;y++)
		{
			glVertex3i(quickbar_x,quickbar_y+y*51+1,0);
			glVertex3i(quickbar_x+quickbar_x_len,quickbar_y+y*51+1,0);
		}
	glEnd();
	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0f,1.0f,1.0f);
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
					u_end=u_start+0.2f;
					v_start=(1.0f+2.0f/256.0f)-(0.2f*(cur_item/5));
					v_end=v_start-0.2f;

					//get the x and y
					cur_pos=item_list[i].pos;
					if(cur_pos<6)//don't even check worn items
						{
							x_start=quickbar_x+1;
							x_end=x_start+50;
							y_start=quickbar_y+51*(cur_pos%6)+1;
							y_end=y_start+50;

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
}

int check_quickbar() {
	int i,y;
	int x_screen,y_screen;
	Uint8 str[100];

	if(mouse_x>quickbar_x+quickbar_x_len || mouse_x<quickbar_x
	   || mouse_y<quickbar_y || mouse_y>quickbar_y+quickbar_y_len)return 0;


	//see if we clicked on any item in the main category
	for(y=0;y<6;y++)
		{
			x_screen=quickbar_x;
			y_screen=quickbar_y+y*51;
			if(mouse_x>x_screen && mouse_x<x_screen+51 && mouse_y>y_screen && mouse_y<y_screen+51)
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

									if(action_mode==action_look || right_click)
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
