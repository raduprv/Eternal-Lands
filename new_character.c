#include <stdlib.h>
#include <string.h>
#include "global.h"

int skin_color=SKIN_NORMAL;
int hair_color=HAIR_BLACK;
int shirt_color=SHIRT_GREEN;
int pants_color=PANTS_BLACK;
int boots_color=BOOTS_BROWN;
int head=HEAD_1;

int actor_creation_menu_x_start=250;
int actor_creation_menu_y_start=10;
int actor_creation_menu_x_end=350;
int actor_creation_menu_y_end=270;

int back_arrow_x_start = 90;
int back_arrow_x_end = 108;
int forward_arrow_x_start = 120;
int forward_arrow_x_end = 138;

int skin_text_y_start = 270;
int skin_text_y_end = 288;
int hair_text_y_start = 290;
int hair_text_y_end = 308;
int shirt_text_y_start = 310;
int shirt_text_y_end = 328;
int pants_text_y_start = 330;
int pants_text_y_end = 348;
int boots_text_y_start = 350;
int boots_text_y_end = 368;
int head_text_y_start = 370;
int head_text_y_end = 388;

int skin_forward = 0;
int skin_back = 0;
int hair_forward = 0;
int hair_back = 0;
int shirt_forward = 0;
int shirt_back = 0;
int pants_forward = 0;
int pants_back = 0;
int boots_forward = 0;
int boots_back = 0;
int head_forward = 0;
int head_back = 0;


#define RACE_HUMAN 0
#define RACE_ELF 2
#define RACE_DWARF 4
#define RACE_GNOME 37
#define RACE_ORCHAN 39
#define RACE_DRAEGONI 41

int male=1;
int race=RACE_HUMAN;

actor * our_model;
int any_model=0;

int username_label_x_start = 60;
int username_label_y_start = 10;
int password_label_x_start = 60;
int password_label_y_start = 68;
int confirm_label_x_start = 20;
int confirm_label_y_start = 126;

int error_x_start = 10;
int error_y_start = 400;

int box_x_start = 10;
int box_x_end = 200;
int username_box_y_start = 30;
int username_box_y_end = 58;
int password_box_y_start = 88;
int password_box_y_end = 116;
int confirm_box_y_start = 146;
int confirm_box_y_end = 174;

int username_selected = 1;
int password_selected = 0;
int confirm_pass_selected = 0;

int pass_text_lenght;
char pass_str[16];
char display_pass_str[16];

int conf_pass_text_lenght;
char conf_pass_str[16];
char display_conf_pass_str[16];

int user_text_lenght;
char user_str[16];

int back_button_x_start = 110;
int back_button_x_end = 200;
int back_button_y_start = 197;
int back_button_y_end = 235;

int done_button_x_start = 10;
int done_button_x_end = 90;
int done_button_y_start = 197;
int done_button_y_end = 235;

int back_selected = 0;
int done_selected = 0;

int gender_text_x_start = 460;
int gender_text_x_end = 590;
int gender_text_y_start = 280;
int male_text_y_start = 300;
int male_text_y_end = 320;
int female_text_y_start = 320;
int female_text_y_end = 340;

int race_text_x_start = 240;
int race_text_y_start = 280;
int p2p_text_x_start = 310;
int p2p_text_y_start = 280;

int human_text_x_start = 240;
int human_text_x_end = 300;
int human_text_y_start = 300;
int human_text_y_end = 320;

int elf_text_x_start = 240;
int elf_text_x_end = 300;
int elf_text_y_start = 320;
int elf_text_y_end = 340;

int dwarf_text_x_start = 240;
int dwarf_text_x_end = 300;
int dwarf_text_y_start = 340;
int dwarf_text_y_end = 360;

int gnome_text_x_start = 310;
int gnome_text_x_end = 400;
int gnome_text_y_start = 300;
int gnome_text_y_end = 320;

int orchan_text_x_start = 310;
int orchan_text_x_end = 400;
int orchan_text_y_start = 320;
int orchan_text_y_end = 340;

int draegoni_text_x_start = 310;
int draegoni_text_x_end = 400;
int draegoni_text_y_start = 340;
int draegoni_text_y_end = 360;

float done_button_unselected_start_u=(float)0/256;
float done_button_unselected_start_v=1.0f-(float)161/256;

float done_button_unselected_end_u=(float)87/256;
float done_button_unselected_end_v=1.0f-(float)196/256;

float done_button_selected_start_u=(float)0/256;
float done_button_selected_start_v=1.0f-(float)202/256;

float done_button_selected_end_u=(float)87/256;
float done_button_selected_end_v=1.0f-(float)237/256;
/////////////////////////
float back_unselected_start_u=(float)100/256;
float back_unselected_start_v=1.0f-(float)161/256;

float back_unselected_end_u=(float)188/256;
float back_unselected_end_v=1.0f-(float)196/256;

float back_selected_start_u=(float)100/256;
float back_selected_start_v=1.0f-(float)202/256;

float back_selected_end_u=(float)188/256;
float back_selected_end_v=1.0f-(float)237/256;

void change_actor ()
{
	// if there is any loaded model, destroy it
	if (any_model)
	{
		glDeleteTextures (1, &our_model->texture_id);
		free (our_model->body_parts);
		free (our_model);
		our_model = 0;
	}
	/*if(race==RACE_HUMAN)
		our_model=add_actor_interface(race+male,skin_color,hair_color,shirt_color,pants_color,boots_color,head);
	else if(race==RACE_ELF)
		our_model=add_actor_interface(race+male,skin_color,hair_color,shirt_color,pants_color,boots_color,head);
	else if(race==RACE_DWARF)
		our_model=add_actor_interface(race+male,skin_color,hair_color,shirt_color,pants_color,boots_color,head);
	else if(race==RACE_GNOME)*/
	if (hair_color == HAIR_BLOND) 
		if ((race == RACE_DRAEGONI && male) || race == RACE_ORCHAN) 
			hair_color++;
	if (hair_color > HAIR_WHITE && race != RACE_DRAEGONI) 
		hair_color = HAIR_BLACK;
	if (shirt_color == SHIRT_PINK && male) 
		shirt_color++;
	
	our_model = add_actor_interface (race+male, skin_color, hair_color, shirt_color, pants_color, boots_color, head);

	any_model = 1;	// we have an actor loaded
	last_texture = -1;	// when we load a new char, we also bind the texture, so...
}

// XXX FIXME (Grum): scheduled for removal
#ifdef OLD_EVENT_HANDLER
void check_for_input()
{

	if(mouse_x>110 && mouse_x<200 && mouse_y>197 && mouse_y<235)back_selected=1;
	else back_selected=0;

	if(mouse_x>10 && mouse_x<90 && mouse_y>197 && mouse_y<235)done_selected=1;
	else done_selected=0;

	if(mouse_x>=90 && mouse_y>270 && mouse_y<288 && mouse_x<=108)skin_back=1;else skin_back=0;
	if(mouse_x>=120 && mouse_y>270 && mouse_y<288 && mouse_x<=138)skin_forward=1;else skin_forward=0;
	if(mouse_x>=90 && mouse_y>290 && mouse_y<308 && mouse_x<=108)hair_back=1;else hair_back=0;
	if(mouse_x>=120 && mouse_y>290 && mouse_y<308 && mouse_x<=138)hair_forward=1;else hair_forward=0;
	if(mouse_x>=90 && mouse_y>310 && mouse_y<328 && mouse_x<=108)shirt_back=1;else shirt_back=0;
	if(mouse_x>=120 && mouse_y>310 && mouse_y<328 && mouse_x<=138)shirt_forward=1;else shirt_forward=0;
	if(mouse_x>=90 && mouse_y>330 && mouse_y<348 && mouse_x<=108)pants_back=1;else pants_back=0;
	if(mouse_x>=120 && mouse_y>330 && mouse_y<348 && mouse_x<=138)pants_forward=1;else pants_forward=0;
	if(mouse_x>=90 && mouse_y>350 && mouse_y<368 && mouse_x<=108)boots_back=1;else boots_back=0;
	if(mouse_x>=120 && mouse_y>350 && mouse_y<368 && mouse_x<=138)boots_forward=1;else boots_forward=0;
	if(mouse_x>=90 && mouse_y>370 && mouse_y<388 && mouse_x<=108)head_back=1;else head_back=0;
	if(mouse_x>=120 && mouse_y>370 && mouse_y<388 && mouse_x<=138)head_forward=1;else head_forward=0;



	if(left_click!=1)return;
	left_click=2;//to avoid further rechecks while the mouse button is down

	if(mouse_x>110 && mouse_x<200 && mouse_y>197 && mouse_y<235 && back_selected)
		{
			interface_mode=INTERFACE_LOG_IN;
			return;
		}
	if(mouse_x>10 && mouse_x<90 && mouse_y>197 && mouse_y<235 && done_selected)
		{
			send_new_char(user_str, pass_str,conf_pass_str,
						  skin_color, hair_color, shirt_color, pants_color, boots_color, head,race+male);
			return;
		}

	//check skin color change
	if(mouse_x>=90 && mouse_y>270 && mouse_y<288 && mouse_x<=108)
		{
			if(skin_color==SKIN_BROWN)
				skin_color=SKIN_TAN;
			else skin_color--;
			change_actor();
		}
	if(mouse_x>=120 && mouse_y>270 && mouse_y<288 && mouse_x<=138)
		{
			if(skin_color==SKIN_TAN)
				skin_color=SKIN_BROWN;
			else skin_color++;
			change_actor();
		}

	//check hair color change
	if(mouse_x>=90 && mouse_y>290 && mouse_y<308 && mouse_x<=108)
		{
			int wrap;
			if(race==RACE_DRAEGONI)wrap=HAIR_PURPLE;
			else wrap=HAIR_WHITE;
			if(hair_color==HAIR_BLACK)
				hair_color=wrap;
			else hair_color--;
			if(hair_color==HAIR_BLOND) if((race==RACE_DRAEGONI && male)||race==RACE_ORCHAN) hair_color--;
			change_actor();
		}
	if(mouse_x>=120 && mouse_y>290 && mouse_y<308 && mouse_x<=138)
		{
			int wrap;
			if(race==RACE_DRAEGONI)wrap=HAIR_PURPLE;
			else wrap=HAIR_WHITE;
			if(hair_color==wrap)
				hair_color=HAIR_BLACK;
			else hair_color++;
			if(hair_color==HAIR_BLOND) if((race==RACE_DRAEGONI && male)||race==RACE_ORCHAN) hair_color++;
			change_actor();
		}

	//check shirt color change
	if(mouse_x>=90 && mouse_y>310 && mouse_y<328 && mouse_x<=108)
		{
			if(shirt_color==SHIRT_BLACK)
				shirt_color=SHIRT_YELLOW;
			else shirt_color--;
			if(shirt_color==SHIRT_PINK) if(male)shirt_color--;
			change_actor();
		}
	if(mouse_x>=120 && mouse_y>310 && mouse_y<328 && mouse_x<=138)
		{
			if(shirt_color==SHIRT_YELLOW)
				shirt_color=SHIRT_BLACK;
			else shirt_color++;
			if(shirt_color==SHIRT_PINK) if(male)shirt_color++;
			change_actor();
		}

	//check pants color change
	if(mouse_x>=90 && mouse_y>330 && mouse_y<348 && mouse_x<=108)
		{
			if(pants_color==PANTS_BLACK)
				pants_color=PANTS_WHITE;
			else pants_color--;
			change_actor();
		}
	if(mouse_x>=120 && mouse_y>330 && mouse_y<348 && mouse_x<=138)
		{
			if(pants_color==PANTS_WHITE)
				pants_color=PANTS_BLACK;
			else pants_color++;
			change_actor();
		}
	//check boots color change
	if(mouse_x>=90 && mouse_y>350 && mouse_y<368 && mouse_x<=108)
		{
			if(boots_color==BOOTS_BLACK)
				boots_color=BOOTS_ORANGE;
			else boots_color--;
			change_actor();
		}
	if(mouse_x>=120 && mouse_y>350 && mouse_y<368 && mouse_x<=138)
		{
			if(boots_color==BOOTS_ORANGE)
				boots_color=BOOTS_BLACK;
			else boots_color++;
			change_actor();
		}
	//check head change
	if(mouse_x>=90 && mouse_y>370 && mouse_y<388 && mouse_x<=108)
		{
			if(head==HEAD_1)
				{
					if(race==RACE_HUMAN)
						head=HEAD_5;
					else
						head=HEAD_4;
				}
			else head--;
			change_actor();
		}
	if(mouse_x>=120 && mouse_y>370 && mouse_y<388 && mouse_x<=138)
		{
			if(head==HEAD_4 &&!(race==RACE_HUMAN))
				{
					head=HEAD_1;
				}
			else if(head==HEAD_5)
				head=HEAD_1;
			else head++;
			change_actor();
		}
	//check to see if we changed the gender
	//460,300
	if(mouse_x>460 && mouse_x<590 && mouse_y>300 && mouse_y<320)
		{
			male=1;
			change_actor();
		}

	if(mouse_x>460 && mouse_x<590 && mouse_y>320 && mouse_y<340)
		{
			male=0;
			change_actor();
		}
	//check to see if we changed the race
	if(mouse_x>240 && mouse_x<300 && mouse_y>300 && mouse_y<320)
		{
			race=RACE_HUMAN;
			change_actor();
		}
	if(mouse_x>240 && mouse_x<300 && mouse_y>320 && mouse_y<340)
		{
			race=RACE_ELF;
			change_actor();
		}
	if(mouse_x>240 && mouse_x<300 && mouse_y>340 && mouse_y<360)
		{
			race=RACE_DWARF;
			change_actor();
		}
	if(mouse_x>310 && mouse_x<400 && mouse_y>300 && mouse_y<320)
		{
			race=RACE_GNOME;
			change_actor();
		}
	if(mouse_x>310 && mouse_x<400 && mouse_y>320 && mouse_y<340)
		{
			race=RACE_ORCHAN;
			change_actor();
		}
	if(mouse_x>310 && mouse_x<400 && mouse_y>340 && mouse_y<360)
		{
			race=RACE_DRAEGONI;
			change_actor();
		}
	//check to see the selected dialogue boxes
	if(mouse_x>10 && mouse_x<200 && mouse_y>30 && mouse_y<58)
		{
			username_selected = 1;
			password_selected = 0;
			confirm_pass_selected = 0;
		}
	if(mouse_x>10 && mouse_x<200 && mouse_y>88 && mouse_y<116)
		{
			username_selected = 0;
			password_selected = 1;
			confirm_pass_selected = 0;
		}

	if(mouse_x>10 && mouse_x<200 && mouse_y>146 && mouse_y<174)
		{
			username_selected = 0;
			password_selected = 0;
			confirm_pass_selected = 1;
		}


}
#endif

void draw_new_char_screen()
{
	float selected_bar_u_start=(float)0/256;
	float selected_bar_v_start=1.0f-(float)0/256;

	float selected_bar_u_end=(float)174/256;
	float selected_bar_v_end=1.0f-(float)28/256;


	float unselected_bar_u_start=(float)0/256;
	float unselected_bar_v_start=1.0f-(float)40/256;

	float unselected_bar_u_end=(float)170/256;
	float unselected_bar_v_end=1.0f-(float)63/256;

#ifdef OLD_EVENT_HANDLER
	check_for_input();
#endif

	//see if we have to load a model (male or female)
	if (!any_model)
	{
		our_model = add_actor_interface (race+male, skin_color, hair_color, shirt_color, pants_color, boots_color, head);
		any_model = 1; // we have an actor loaded
	}

	glColor3f (1.0f, 1.0f, 1.0f);
	draw_string (10, skin_text_y_start, skin_str, 1);
	if (skin_back) 
		glColor3f (0.3f, 1.0f, 0.3f);
	else 
		glColor3f (1.0f, 0.7f, 0.0f);
	draw_string (back_arrow_x_start, skin_text_y_start, "<<", 1);
	if(skin_forward)
		glColor3f (0.3f, 1.0f, 0.3f);
	else
		glColor3f (1.0f, 0.7f, 0.0f);
	draw_string (forward_arrow_x_start, skin_text_y_start, ">>", 1);

	glColor3f (1.0f, 1.0f, 1.0f);
	draw_string (10, hair_text_y_start, hair_str, 1);
	if (hair_back)
		glColor3f (0.3f, 1.0f, 0.3f);
	else
		glColor3f (1.0f, 0.7f, 0.0f);
	draw_string (back_arrow_x_start, hair_text_y_start, "<<", 1);
	if (hair_forward)
		glColor3f (0.3f, 1.0f, 0.3f);
	else
		glColor3f (1.0f, 0.7f, 0.0f);
	draw_string (forward_arrow_x_start, hair_text_y_start, ">>", 1);

	glColor3f (1.0f, 1.0f, 1.0f);
	draw_string (10, shirt_text_y_start, shirt_str, 1);
	if (shirt_back)
		glColor3f (0.3f, 1.0f, 0.3f);
	else 
		glColor3f (1.0f, 0.7f, 0.0f);
	draw_string (back_arrow_x_start, shirt_text_y_start, "<<", 1);
	if (shirt_forward)
		glColor3f (0.3f, 1.0f, 0.3f);
	else 
		glColor3f (1.0f, 0.7f, 0.0f);
	draw_string (forward_arrow_x_start, shirt_text_y_start, ">>", 1);

	glColor3f (1.0f, 1.0f, 1.0f);
	draw_string (10, pants_text_y_start, pants_str, 1);
	if (pants_back)
		glColor3f (0.3f, 1.0f, 0.3f);
	else
		glColor3f (1.0f, 0.7f, 0.0f);
	draw_string (back_arrow_x_start, pants_text_y_start, "<<", 1);
	if (pants_forward)
		glColor3f (0.3f, 1.0f, 0.3f);
	else 
		glColor3f (1.0f, 0.7f, 0.0f);
	draw_string (forward_arrow_x_start, pants_text_y_start, ">>", 1);

	glColor3f (1.0f, 1.0f, 1.0f);
	draw_string (10, boots_text_y_start, boots_str, 1);
	if (boots_back) 
		glColor3f (0.3f, 1.0f, 0.3f);
	else 
		glColor3f (1.0f, 0.7f, 0.0f);
	draw_string (back_arrow_x_start, boots_text_y_start, "<<", 1);
	if (boots_forward)
		glColor3f (0.3f, 1.0f, 0.3f);
	else 
		glColor3f (1.0f, 0.7f, 0.0f);
	draw_string (forward_arrow_x_start, boots_text_y_start, ">>", 1);

	glColor3f (1.0f, 1.0f, 1.0f);
	draw_string (10, head_text_y_start, head_str, 1);
	if(head_back)
		glColor3f (0.3f, 1.0f, 0.3f);
	else 
		glColor3f (1.0f, 0.7f, 0.0f);
	draw_string (back_arrow_x_start, head_text_y_start, "<<", 1);
	if (head_forward)
		glColor3f (0.3f, 1.0f, 0.3f);
	else 
		glColor3f (1.0f, 0.7f, 0.0f);
	draw_string (forward_arrow_x_start, head_text_y_start, ">>", 1);

	glColor3f (1.0f, 0.2f, 0.2f);
	draw_string (gender_text_x_start, gender_text_y_start, gender_str, 1);

	if (male == 1) 
		glColor3f (0.0f, 0.5f, 1.0f);
	else 
		glColor3f (1.0f, 1.0f, 1.0f);
	draw_string (gender_text_x_start, male_text_y_start, male_str, 1);

	if (male != 1)
		glColor3f (0.0f, 0.5f, 1.0f);
	else 
		glColor3f (1.0f, 1.0f, 1.0f);
	draw_string (gender_text_x_start, female_text_y_start, female_str, 1);

	glColor3f (1.0f, 0.2f, 0.2f);
	draw_string (race_text_x_start, race_text_y_start, race_str, 1);

	if (race == RACE_HUMAN)
		glColor3f (0.0f, 0.5f, 1.0f);
	else
		glColor3f (1.0f, 1.0f, 1.0f);
	draw_string (human_text_x_start, human_text_y_start, human_str, 1);

	if (race == RACE_ELF)
		glColor3f (0.0f, 0.5f, 1.0f);
	else
		glColor3f (1.0f, 1.0f, 1.0f);
	draw_string (elf_text_x_start, elf_text_y_start, elf_str, 1);

	if (race == RACE_DWARF)
		glColor3f (0.0f, 0.5f, 1.0f);
	else
		glColor3f (1.0f, 1.0f, 1.0f);
	draw_string (dwarf_text_x_start, dwarf_text_y_start, dwarf_str, 1);

        glColor3f (1.0f, 0.2f, 0.2f);
	draw_string (p2p_text_x_start, p2p_text_y_start, "P2P Only!", 1);
	
	glDisable (GL_TEXTURE_2D);
	
	glBegin (GL_LINE_LOOP);
	glVertex2i (p2p_text_x_start-5, p2p_text_y_start-5);
	glVertex2i (p2p_text_x_start-5, draegoni_text_y_end);
	glVertex2i (draegoni_text_x_end + 10, draegoni_text_y_end);
	glVertex2i (draegoni_text_x_end + 10, p2p_text_y_start-5);
	glEnd ();

	glEnable (GL_TEXTURE_2D);
	
	if (race == RACE_GNOME)
		glColor3f (0.0f, 0.5f, 1.0f);
	else
		glColor3f (1.0f, 1.0f, 1.0f);
	draw_string (gnome_text_x_start, gnome_text_y_start, "Gnome", 1);

	if (race == RACE_ORCHAN)
		glColor3f (0.0f, 0.5f, 1.0f);
	else
		glColor3f (1.0f, 1.0f, 1.0f);
	draw_string (orchan_text_x_start, orchan_text_y_start, "Orchan", 1);
	
	if (race == RACE_DRAEGONI)
		glColor3f (0.0f, 0.5f, 1.0f);
	else
		glColor3f (1.0f, 1.0f, 1.0f);
	draw_string (draegoni_text_x_start, draegoni_text_y_start, "Draegoni", 1);
	
	//draw the player frame
	glDisable (GL_TEXTURE_2D);
	glBegin (GL_QUADS);
	glColor4f (0.4f, 0.4f, 0.4f, 0.5f);
	glVertex3i (actor_creation_menu_x_start, actor_creation_menu_y_end, 0);
	glVertex3i (actor_creation_menu_x_start, actor_creation_menu_y_start, 0);
	glVertex3i (actor_creation_menu_x_end, actor_creation_menu_y_start, 0);
	glVertex3i (actor_creation_menu_x_end, actor_creation_menu_y_end, 0);

	glVertex3i (actor_creation_menu_x_start + 200, actor_creation_menu_y_end, 0);
	glVertex3i (actor_creation_menu_x_start + 200, actor_creation_menu_y_start, 0);
	glVertex3i (actor_creation_menu_x_end + 200, actor_creation_menu_y_start, 0);
	glVertex3i (actor_creation_menu_x_end + 200, actor_creation_menu_y_end, 0);
	glEnd ();

	glColor3f (0.2f, 0.2f, 0.2f);
	glBegin (GL_LINES);
	glVertex3i (actor_creation_menu_x_start, actor_creation_menu_y_start, 0);
	glVertex3i (actor_creation_menu_x_end, actor_creation_menu_y_start, 0);
	glVertex3i (actor_creation_menu_x_end, actor_creation_menu_y_start, 0);
	glVertex3i (actor_creation_menu_x_end, actor_creation_menu_y_end, 0);
	glVertex3i (actor_creation_menu_x_end, actor_creation_menu_y_end, 0);
	glVertex3i (actor_creation_menu_x_start, actor_creation_menu_y_end, 0);
	glVertex3i (actor_creation_menu_x_start, actor_creation_menu_y_end, 0);
	glVertex3i (actor_creation_menu_x_start, actor_creation_menu_y_start, 0);

	glVertex3i (actor_creation_menu_x_start + 200, actor_creation_menu_y_start, 0);
	glVertex3i (actor_creation_menu_x_end + 200, actor_creation_menu_y_start, 0);
	glVertex3i (actor_creation_menu_x_end + 200, actor_creation_menu_y_start, 0);
	glVertex3i (actor_creation_menu_x_end + 200, actor_creation_menu_y_end, 0);
	glVertex3i (actor_creation_menu_x_end + 200, actor_creation_menu_y_end, 0);
	glVertex3i (actor_creation_menu_x_start + 200, actor_creation_menu_y_end, 0);
	glVertex3i (actor_creation_menu_x_start + 200, actor_creation_menu_y_end, 0);
	glVertex3i (actor_creation_menu_x_start + 200, actor_creation_menu_y_start, 0);

	glEnd ();
	glEnable (GL_TEXTURE_2D);

	//draw the player
	glEnable (GL_DEPTH_TEST);
	draw_interface_actor (our_model, 150.0f, actor_creation_menu_x_start + 50, actor_creation_menu_y_end - 10, 0, 90.0f, 0.0f, 0.0f);

	draw_interface_actor (our_model, 150.0f, actor_creation_menu_x_start + 250, actor_creation_menu_y_end - 10, 0, 270.0f, 0.0f, -180.0f);
	glDisable (GL_DEPTH_TEST);

	//now start putting the dialogue boxes.
	get_and_set_texture_id (login_screen_menus);
	glColor3f (1.0f, 1.0f, 1.0f);
	glBegin (GL_QUADS);

	//username box
	if (username_selected)
		draw_2d_thing (selected_bar_u_start, selected_bar_v_start, selected_bar_u_end, selected_bar_v_end, box_x_start, username_box_y_start, box_x_end, username_box_y_end);
	else
		draw_2d_thing (unselected_bar_u_start, unselected_bar_v_start, unselected_bar_u_end, unselected_bar_v_end, box_x_start, username_box_y_start, box_x_end, username_box_y_end);

	//password box
	if (password_selected)
		draw_2d_thing (selected_bar_u_start, selected_bar_v_start, selected_bar_u_end, selected_bar_v_end, box_x_start, password_box_y_start, box_x_end, password_box_y_end);
	else
		draw_2d_thing (unselected_bar_u_start, unselected_bar_v_start, unselected_bar_u_end, unselected_bar_v_end, box_x_start, password_box_y_start, box_x_end, password_box_y_end);

	//confirm box
	if (confirm_pass_selected)
		draw_2d_thing (selected_bar_u_start, selected_bar_v_start, selected_bar_u_end, selected_bar_v_end, box_x_start, confirm_box_y_start, box_x_end, confirm_box_y_end);
	else
		draw_2d_thing (unselected_bar_u_start, unselected_bar_v_start, unselected_bar_u_end, unselected_bar_v_end, box_x_start, confirm_box_y_start, box_x_end, confirm_box_y_end);

	//done_button button
	if(done_selected)
		draw_2d_thing (done_button_selected_start_u, done_button_selected_start_v, done_button_selected_end_u, done_button_selected_end_v, done_button_x_start, done_button_y_start, done_button_x_end, done_button_y_end);
	else
		draw_2d_thing (done_button_unselected_start_u, done_button_unselected_start_v, done_button_unselected_end_u, done_button_unselected_end_v, done_button_x_start, done_button_y_start, done_button_x_end, done_button_y_end);

	//back button
	if(back_selected)
		draw_2d_thing (back_selected_start_u, back_selected_start_v, back_selected_end_u, back_selected_end_v, back_button_x_start, back_button_y_start, back_button_x_end, back_button_y_end);
	else
		draw_2d_thing (back_unselected_start_u, back_unselected_start_v, back_unselected_end_u, back_unselected_end_v, back_button_x_start, back_button_y_start, back_button_x_end, back_button_y_end);

	glEnd();

	// now, draw the text 'labels'
	glColor3f (0.0f, 1.0f, 0.5f);
	draw_string (username_label_x_start, username_label_y_start, login_username_str, 1);
	draw_string (password_label_x_start, password_label_y_start, login_password_str, 1);
	draw_string (confirm_label_x_start, confirm_label_y_start, confirm_password, 1);

	//put the username, pass, and conf pass user text
	glColor3f (0.0f, 0.5f, 1.0f);
	draw_string (box_x_start + 5, password_box_y_start + 8, display_pass_str,1);
	draw_string (box_x_start + 5, confirm_box_y_start + 8, display_conf_pass_str,1);
	draw_string (box_x_start + 5, username_box_y_start + 8, user_str,1);

	glColor3f (1.0f, 0.0f, 0.0f);
	// print the current error, if any
	draw_string (10, 400, create_char_error_str,1);
}

void add_char_to_pass(unsigned char ch)
{
	if((ch>=32 && ch<=126) && pass_text_lenght<15)
		{
			pass_str[pass_text_lenght]=ch;
			display_pass_str[pass_text_lenght]='*';
			pass_str[pass_text_lenght+1]=0;
			pass_text_lenght++;
		}
	if(ch==SDLK_BACKSPACE && pass_text_lenght>0)
		{
			pass_text_lenght--;
			display_pass_str[pass_text_lenght]=0;
			pass_str[pass_text_lenght]=0;
		}
#ifdef OLD_EVENT_HANDLER
	if(ch==SDLK_TAB)
		{
			password_selected = 0;
			confirm_pass_selected = 1;
		}
#endif
}

void add_char_to_un(unsigned char ch)
{
	if(((ch>=48 && ch<=57) || (ch>=65 && ch<=90) || (ch>=97 && ch<=122) || (ch=='_')) && user_text_lenght<15)
		{
			user_str[user_text_lenght]=ch;
			user_str[user_text_lenght+1]=0;
			user_text_lenght++;
		}
	if(ch==SDLK_BACKSPACE && user_text_lenght>0)
		{
			user_text_lenght--;
			user_str[user_text_lenght]=0;
		}
#ifdef OLD_EVENT_HANDLER
	if(ch==SDLK_TAB)
		{
			username_selected = 0;
			password_selected = 1;
		}
#endif
}

void add_char_to_conf(unsigned char ch)
{
	if((ch>=32 && ch<=126) && conf_pass_text_lenght<15)
		{
			conf_pass_str[conf_pass_text_lenght]=ch;
			display_conf_pass_str[conf_pass_text_lenght]='*';
			conf_pass_str[conf_pass_text_lenght+1]=0;
			conf_pass_text_lenght++;
		}
	if(ch==SDLK_BACKSPACE && conf_pass_text_lenght>0)
		{
			conf_pass_text_lenght--;
			display_conf_pass_str[conf_pass_text_lenght]=0;
			conf_pass_str[conf_pass_text_lenght]=0;
		}

#ifdef OLD_EVENT_HANDLER
	if(ch==SDLK_TAB)
		{
			confirm_pass_selected = 0;
			username_selected = 1;
		}
#endif
}

void add_char_to_new_character(unsigned char ch)
{
	if (username_selected) add_char_to_un(ch);
	else if (password_selected) add_char_to_pass(ch);
	else if (confirm_pass_selected) add_char_to_conf(ch);
}

void login_from_new_char()
{
	Uint32 i;
	char ch;
	for(i=0;i<strlen(pass_str);i++)
		{
			ch=pass_str[i];
			if(ch)password_str[i]=ch;
			else break;
		}
	password_str[i]=0;

	for(i=0;i<strlen(user_str);i++)
		{
			ch=user_str[i];
			if(ch)username_str[i]=ch;
			else break;
		}
	username_str[i]=0;

	//now send the log in info
	send_login_info();

}

//////////////////////////////////////////////////////////////////////////

// New character window code below.

#ifndef OLD_EVENT_HANDLER

int newchar_root_win = -1;

int display_newchar_handler (window_info *win)
{
	draw_new_char_screen ();	
	CHECK_GL_ERRORS ();
	draw_delay = 20;
	return 1;
}

int mouseover_newchar_handler (window_info *win, int mx, int my)
{
	back_selected = 0;
	done_selected = 0;
	
	skin_back = 0;
	skin_forward = 0;
	hair_back = 0;
	hair_forward = 0;
	shirt_back = 0;
	shirt_forward = 0;
	pants_back = 0;
	pants_forward = 0;
	boots_back = 0;
	boots_forward = 0;
	head_back = 0;
	head_forward = 0;
	
	if (mx > back_button_x_start && mx < back_button_x_end && my > back_button_y_start && my < back_button_y_end)
	{
		back_selected = 1;
	}
	else if (mx > done_button_x_start && mx < done_button_x_end && my > done_button_y_start && my < done_button_y_end)
	{
		done_selected = 1;
	}
	else if (mx >= back_arrow_x_start && my > skin_text_y_start && my < skin_text_y_end && mouse_x <= back_arrow_x_end)
	{
		skin_back = 1;
	}
	else if (mx >= forward_arrow_x_start && my > skin_text_y_start && my < skin_text_y_end && mx <= forward_arrow_x_end)
	{
		skin_forward = 1;
	}
	else if (mx >= back_arrow_x_start && my > hair_text_y_start && my < hair_text_y_end && mx <= back_arrow_x_end)
	{
		hair_back = 1;
	}
	else if (mx >= forward_arrow_x_start && my > hair_text_y_start && my < hair_text_y_end && mx <= forward_arrow_x_end)
	{
		hair_forward = 1;
	}
	else if (mx >= back_arrow_x_start && my > shirt_text_y_start && my < shirt_text_y_end && mx <= back_arrow_x_end)
	{
		shirt_back = 1;
	}
	else if (mx >= forward_arrow_x_start && my > shirt_text_y_start && my < shirt_text_y_end && mx <= forward_arrow_x_end)
	{
		shirt_forward = 1;
	}
	else if (mx >= back_arrow_x_start && my > pants_text_y_start && my < pants_text_y_end && mx <= back_arrow_x_end)
	{
		pants_back = 1;
	}
	else if (mx >= forward_arrow_x_start && my > pants_text_y_start && my < pants_text_y_end && mx <= forward_arrow_x_end)
	{
		pants_forward = 1;
	}
	else if (mx >= back_arrow_x_start && my > boots_text_y_start && my < boots_text_y_end && mx <= back_arrow_x_end)
	{
		boots_back = 1;
	}
	else if (mx >= forward_arrow_x_start && my > boots_text_y_start && my < boots_text_y_end && mx <= forward_arrow_x_end)
	{
		boots_forward = 1;
	}
	else if (mx >= back_arrow_x_start && my > head_text_y_start && my < head_text_y_end && mx <= back_arrow_x_end)
	{
		head_back = 1;
	}
	else if (mx >= forward_arrow_x_start && my > head_text_y_start && my < head_text_y_end && mx <= forward_arrow_x_end)
	{
		head_forward = 1;
	}

	return 1;
}

int click_newchar_handler (window_info *win, int mx, int my, Uint32 flags)
{
	if (mx > back_button_x_start && mx < back_button_x_end && my > back_button_y_start && my < back_button_y_end && back_selected)
	{
		// don't destroy this window yet, maybe the use will come back
		hide_window (newchar_root_win);
		show_window (login_root_win);
		interface_mode=INTERFACE_LOG_IN;
	}
	else if (mx > done_button_x_start && mx < done_button_x_end && my > done_button_y_start && mouse_y < done_button_y_end && done_selected)
	{
		send_new_char (user_str, pass_str, conf_pass_str, skin_color, hair_color, shirt_color, pants_color, boots_color, head, race+male);
	}
	//check skin color change
	else if (mx >= back_arrow_x_start && my > skin_text_y_start && my < skin_text_y_end && mouse_x <= back_arrow_x_end)
	{
		if (skin_color == SKIN_BROWN)
			skin_color = SKIN_TAN;
		else 
			skin_color--;
		change_actor ();
	}
	else if (mx >= forward_arrow_x_start && my > skin_text_y_start && my < skin_text_y_end && mx <= forward_arrow_x_end)
	{
		if (skin_color == SKIN_TAN)
			skin_color = SKIN_BROWN;
		else
			skin_color++;
		change_actor();
	}
	//check hair color change
	else if (mx >= back_arrow_x_start && my > hair_text_y_start && my < hair_text_y_end && mx <= back_arrow_x_end)
	{
		int wrap = race == RACE_DRAEGONI ? HAIR_PURPLE : HAIR_WHITE;
		if(hair_color == HAIR_BLACK)
			hair_color=wrap;
		else
			hair_color--;
		if(hair_color == HAIR_BLOND)
			if ((race==RACE_DRAEGONI && male) || race==RACE_ORCHAN)
				hair_color--;
		change_actor();
	}
	else if(mx >= forward_arrow_x_start && my > hair_text_y_start && my < hair_text_y_end && mx <= forward_arrow_x_end)
	{
		int wrap = race == RACE_DRAEGONI ? HAIR_PURPLE : HAIR_WHITE;
		if(hair_color==wrap)
			hair_color = HAIR_BLACK;
		else
			hair_color++;
		if (hair_color == HAIR_BLOND)
			if ((race==RACE_DRAEGONI && male) || race==RACE_ORCHAN)
				hair_color++;
		change_actor();
	}
	//check shirt color change
	else if(mx >= back_arrow_x_start && my > shirt_text_y_start && my < shirt_text_y_end && mx <= back_arrow_x_end)
	{
		if (shirt_color == SHIRT_BLACK)
			shirt_color=SHIRT_YELLOW;
		else
			shirt_color--;
		if(shirt_color == SHIRT_PINK && male)
			shirt_color--;
		change_actor();
	}
	else if (mx >= forward_arrow_x_start && my > shirt_text_y_start && my < shirt_text_y_end && mx <= forward_arrow_x_end)
	{
		if (shirt_color == SHIRT_YELLOW)
			shirt_color = SHIRT_BLACK;
		else
			shirt_color++;
		if (shirt_color == SHIRT_PINK && male)
			shirt_color++;
		change_actor();
	}
	//check pants color change
	else if (mx >= back_arrow_x_start && my > pants_text_y_start && my < pants_text_y_end && mx <= back_arrow_x_end)
	{
		if (pants_color == PANTS_BLACK)
			pants_color = PANTS_WHITE;
		else
			pants_color--;
		change_actor();
	}
	else if(mx >= forward_arrow_x_start && my > pants_text_y_start && my < pants_text_y_end && mx <= forward_arrow_x_end)
	{
		if (pants_color == PANTS_WHITE)
			pants_color = PANTS_BLACK;
		else
			pants_color++;
		change_actor();
	}
	//check boots color change
	else if (mx >= back_arrow_x_start && my > boots_text_y_start && my < boots_text_y_end && mx <= back_arrow_x_end)
	{
		if (boots_color == BOOTS_BLACK)
			boots_color = BOOTS_ORANGE;
		else
			boots_color--;
		change_actor();
	}
	else if (mx >= forward_arrow_x_start && my > boots_text_y_start && my < boots_text_y_end && mx <= forward_arrow_x_end)
	{
		if (boots_color == BOOTS_ORANGE)
			boots_color = BOOTS_BLACK;
		else
			boots_color++;
		change_actor();
	}
	//check head change
	else if (mx >= back_arrow_x_start && my > head_text_y_start && my < head_text_y_end && mx <= back_arrow_x_end)
	{
		if(head == HEAD_1)
		{
			if (race == RACE_HUMAN)
				head = HEAD_5;
			else
				head = HEAD_4;
		}
		else 
		{
			head--;
		}
		change_actor();
	}
	else if (mx >= forward_arrow_x_start && my > head_text_y_start && my < head_text_y_end && mx <= forward_arrow_x_end)
	{
		if (head == HEAD_4 && race != RACE_HUMAN)
		{
			head = HEAD_1;
		}
		else if (head == HEAD_5)
		{
			head = HEAD_1;
		}
		else
		{
			head++;
		}
		change_actor();
	}
	//check to see if we changed the gender
	//460,300
	else if (mx > gender_text_x_start && mx < gender_text_x_end && my > male_text_y_start && my < male_text_y_end)
	{
		male = 1;
		change_actor ();
	}
	else if (mx > gender_text_x_start && mx < gender_text_x_end && my > female_text_y_start && my < female_text_y_end)
	{
		male = 0;
		change_actor ();
	}
	//check to see if we changed the race
	else if (mx > human_text_x_start && mx < human_text_x_end && my > human_text_y_start && my < human_text_y_end)
	{
		race = RACE_HUMAN;
		change_actor ();
	}
	else if (mx > elf_text_x_start && mx < elf_text_x_end && my > elf_text_y_start && my < elf_text_y_end)
	{
		race = RACE_ELF;
		change_actor ();
	}
	else if (mx > dwarf_text_x_start && mx < dwarf_text_x_end && my > dwarf_text_y_start && my < dwarf_text_y_end)
	{
		race = RACE_DWARF;
		change_actor ();
	}
	else if (mx > gnome_text_x_start && mx < gnome_text_x_end && my > gnome_text_y_start && my < gnome_text_y_end)
	{
		race = RACE_GNOME;
		change_actor();
	}
	else if (mx > orchan_text_x_start && mx < orchan_text_x_end && my > orchan_text_y_start && my < orchan_text_y_end)
	{
		race = RACE_ORCHAN;
		change_actor ();
	}
	else if (mx > draegoni_text_x_start && mx < draegoni_text_x_end && my > draegoni_text_y_start && my < draegoni_text_y_end)
	{
		race = RACE_DRAEGONI;
		change_actor();
	}
	//check to see the selected dialogue boxes
	else if (mx > box_x_start && mx < box_x_end && my > username_box_y_start && my < username_box_y_end)
	{
		username_selected = 1;
		password_selected = 0;
		confirm_pass_selected = 0;
	}
	else if(mx > box_x_start && mx < box_x_end && my > password_box_y_start && my < password_box_y_end)
	{
		username_selected = 0;
		password_selected = 1;
		confirm_pass_selected = 0;
	}
	else if(mx > box_x_start && mx < box_x_end && my > confirm_box_y_start && my < confirm_box_y_end)
	{
		username_selected = 0;
		password_selected = 0;
		confirm_pass_selected = 1;
	}
	// we couldn't handle it
	else
	{
		return 0;
	}

	return 1; // we captured this mouseclick
}

int keypress_newchar_handler (window_info *win, int mx, int my, Uint32 key, Uint32 unikey)
{
	Uint8 ch = key_to_char (unikey);
	
	// first, try to see if we pressed Alt+x, to quit.
	if ( check_quit_or_fullscreen (key) )
	{
		return 1;
	}
	else if (ch == SDLK_TAB)
	{
		if (username_selected)
		{
			username_selected = 0;
			password_selected = 1;
		}
		else if (password_selected)
		{
			password_selected = 0;
			confirm_pass_selected = 1;
		}
		else
		{
			confirm_pass_selected = 0;
			username_selected = 1;
		}
	}
	else 
	{
		add_char_to_new_character (ch);
	}
	
	return 1;
}

void create_newchar_root_window ()
{
	if (newchar_root_win < 0)
	{
		newchar_root_win = create_window ("New Character", -1, -1, 0, 0, window_width, window_height, ELW_TITLE_NONE|ELW_SHOW_LAST);

		set_window_handler (newchar_root_win, ELW_HANDLER_DISPLAY, &display_newchar_handler);
		set_window_handler (newchar_root_win, ELW_HANDLER_MOUSEOVER, &mouseover_newchar_handler);
		set_window_handler (newchar_root_win, ELW_HANDLER_CLICK, &click_newchar_handler);
		set_window_handler (newchar_root_win, ELW_HANDLER_KEYPRESS, &keypress_newchar_handler);
	}
}

#endif
