#include <string.h>
#include "global.h"
#include <math.h>

// initialize anything related to the hud
void init_hud_interface()
{
	init_peace_icons();
	init_misc_display();
	init_stats_display();
}

// draw everything related to the hud
void draw_hud_interface()
{
	get_and_set_texture_id(icons_text);
	glColor3f(1.0f, 1.0f, 1.0f);

    draw_peace_icons();
    draw_misc_display();
    draw_stats_display();
}

// check to see if a mouse click was on the hud
int check_hud_interface()
{
	if(check_peace_icons())return 1;
	if(check_misc_display())return 1;
	if(check_stats_display())return 1;

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

float pick_icon_u_start=(float)96/255;
float pick_icon_v_start=1.0f-(float)0/255;
float pick_icon_u_end=(float)127/255;
float pick_icon_v_end=1.0f-(float)31/255;

float colored_pick_icon_u_start=(float)160/255;
float colored_pick_icon_v_start=1.0f-(float)64/255;
float colored_pick_icon_u_end=(float)191/255;
float colored_pick_icon_v_end=1.0f-(float)95/255;

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

float harvest_icon_u_start=(float)64/255;
float harvest_icon_v_start=1.0f-(float)32/255;
float harvest_icon_u_end=(float)95/255;
float harvest_icon_v_end=1.0f-(float)63/255;

float colored_harvest_icon_u_start=(float)128/255;
float colored_harvest_icon_v_start=1.0f-(float)96/255;
float colored_harvest_icon_u_end=(float)159/255;
float colored_harvest_icon_v_end=1.0f-(float)127/255;

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

float knowledge_icon_u_start=(float)192/255;
float knowledge_icon_v_start=1.0f-(float)32/255;
float knowledge_icon_u_end=(float)223/255;
float knowledge_icon_v_end=1.0f-(float)63/255;

float colored_knowledge_icon_u_start=(float)64/255;
float colored_knowledge_icon_v_start=1.0f-(float)128/255;
float colored_knowledge_icon_u_end=(float)95/255;
float colored_knowledge_icon_v_end=1.0f-(float)159/255;

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

int pick_icon_x_start;
int pick_icon_x_end;
int pick_icon_y_start;
int pick_icon_y_end;

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

int harvest_icon_x_start;
int harvest_icon_x_end;
int harvest_icon_y_start;
int harvest_icon_y_end;

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

	pick_icon_x_start=stand_icon_x_end+1;
	pick_icon_x_end=pick_icon_x_start+32;
	pick_icon_y_start=window_height-32;
	pick_icon_y_end=pick_icon_y_start+32;

	eye_icon_x_start=pick_icon_x_end+1;
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

	harvest_icon_x_start=attack_icon_x_end+1;
	harvest_icon_x_end=harvest_icon_x_start+32;
	harvest_icon_y_start=window_height-32;
	harvest_icon_y_end=harvest_icon_y_start+32;

	manufacture_icon_x_start=harvest_icon_x_end+1;
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
	
	options_icon_x_start=knowledge_icon_x_end+1;
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

	if(action_mode==action_pick || (mouse_x>pick_icon_x_start && mouse_y>pick_icon_y_start &&
									mouse_x<pick_icon_x_end && mouse_y<pick_icon_y_end))
		draw_2d_thing(colored_pick_icon_u_start, colored_pick_icon_v_start, colored_pick_icon_u_end, colored_pick_icon_v_end,
					  pick_icon_x_start, pick_icon_y_start, pick_icon_x_end, pick_icon_y_end);
	else
		draw_2d_thing(pick_icon_u_start, pick_icon_v_start, pick_icon_u_end, pick_icon_v_end,
					  pick_icon_x_start, pick_icon_y_start, pick_icon_x_end, pick_icon_y_end);

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

	if(action_mode==action_harvest || (mouse_x>harvest_icon_x_start && mouse_y>harvest_icon_y_start &&
									   mouse_x<harvest_icon_x_end && mouse_y<harvest_icon_y_end))
		draw_2d_thing(colored_harvest_icon_u_start, colored_harvest_icon_v_start, colored_harvest_icon_u_end, colored_harvest_icon_v_end,
					  harvest_icon_x_start, harvest_icon_y_start, harvest_icon_x_end, harvest_icon_y_end);
	else
		draw_2d_thing(harvest_icon_u_start, harvest_icon_v_start, harvest_icon_u_end, harvest_icon_v_end,
					  harvest_icon_x_start, harvest_icon_y_start, harvest_icon_x_end, harvest_icon_y_end);

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
	else if(mouse_x>harvest_icon_x_start && mouse_y>harvest_icon_y_start &&
			mouse_x<harvest_icon_x_end && mouse_y<harvest_icon_y_end)
		action_mode=action_harvest;
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
	else if(mouse_x>pick_icon_x_start && mouse_y>pick_icon_y_start &&
			mouse_x<pick_icon_x_end && mouse_y<pick_icon_y_end)
		action_mode=action_pick;
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
	return 1;
}


// the stats display
void init_stats_display()
{

}

void draw_stats_display()
{

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


