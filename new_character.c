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
int actor_creation_menu_y_end=260;

int skin_f=0;
int skin_b=0;
int hair_f=0;
int hair_b=0;
int shirt_f=0;
int shirt_b=0;
int pants_f=0;
int pants_b=0;
int boots_f=0;
int boots_b=0;
int head_f=0;
int head_b=0;


#define race_human 0
#define race_elf 2
#define race_dwarf 4

int male=1;
int race=race_human;

actor * our_model;
int any_model=0;

int username=1;
int password=0;
int confirm_pass=0;

int pass_text_lenght;
char pass_str[16];
char display_pass_str[16];

int conf_pass_text_lenght;
char conf_pass_str[16];
char display_conf_pass_str[16];

int user_text_lenght;
char user_str[16];

int back=0;
int done_button=0;

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


void change_actor()
{
	//if there is any loaded model, destroy it
	if(any_model)
		{
			glDeleteTextures(1,&our_model->texture_id);
			free(our_model->body_parts);
			free(our_model);
			our_model=0;
		}
	if(race==race_human)
		our_model=add_actor_interface(race+male,skin_color,hair_color,shirt_color,pants_color,boots_color,head);
	else if(race==race_elf)
		our_model=add_actor_interface(race+male,skin_color,hair_color,shirt_color,pants_color,boots_color,head);
	else if(race==race_dwarf)
		our_model=add_actor_interface(race+male,skin_color,hair_color,shirt_color,pants_color,boots_color,head);

	any_model=1;//we have an actor loaded
	last_texture=-1;//when we load a new char, we also bind the texture, so...
}


void check_for_input()
{

	if(mouse_x>110 && mouse_x<200 && mouse_y>197 && mouse_y<235)back=1;
	else back=0;

	if(mouse_x>10 && mouse_x<90 && mouse_y>197 && mouse_y<235)done_button=1;
	else done_button=0;

	if(mouse_x>=90 && mouse_y>270 && mouse_y<288 && mouse_x<=108)skin_b=1;else skin_b=0;
	if(mouse_x>=120 && mouse_y>270 && mouse_y<288 && mouse_x<=138)skin_f=1;else skin_f=0;
	if(mouse_x>=90 && mouse_y>290 && mouse_y<308 && mouse_x<=108)hair_b=1;else hair_b=0;
	if(mouse_x>=120 && mouse_y>290 && mouse_y<308 && mouse_x<=138)hair_f=1;else hair_f=0;
	if(mouse_x>=90 && mouse_y>310 && mouse_y<328 && mouse_x<=108)shirt_b=1;else shirt_b=0;
	if(mouse_x>=120 && mouse_y>310 && mouse_y<328 && mouse_x<=138)shirt_f=1;else shirt_f=0;
	if(mouse_x>=90 && mouse_y>330 && mouse_y<348 && mouse_x<=108)pants_b=1;else pants_b=0;
	if(mouse_x>=120 && mouse_y>330 && mouse_y<348 && mouse_x<=138)pants_f=1;else pants_f=0;
	if(mouse_x>=90 && mouse_y>350 && mouse_y<368 && mouse_x<=108)boots_b=1;else boots_b=0;
	if(mouse_x>=120 && mouse_y>350 && mouse_y<368 && mouse_x<=138)boots_f=1;else boots_f=0;
	if(mouse_x>=90 && mouse_y>370 && mouse_y<388 && mouse_x<=108)head_b=1;else head_b=0;
	if(mouse_x>=120 && mouse_y>370 && mouse_y<388 && mouse_x<=138)head_f=1;else head_f=0;



	if(left_click!=1)return;
	left_click=2;//to avoid further rechecks while the mouse button is down

	if(mouse_x>110 && mouse_x<200 && mouse_y>197 && mouse_y<235 && back)
		{
			interface_mode=interface_log_in;
			return;
		}
	if(mouse_x>10 && mouse_x<90 && mouse_y>197 && mouse_y<235 && done_button)
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
			if(hair_color==HAIR_BLACK)
				hair_color=HAIR_WHITE;
			else hair_color--;
			change_actor();
		}
	if(mouse_x>=120 && mouse_y>290 && mouse_y<308 && mouse_x<=138)
		{
			if(hair_color==HAIR_WHITE)
				hair_color=HAIR_BLACK;
			else hair_color++;
			change_actor();
		}

	//check shirt color change
	if(mouse_x>=90 && mouse_y>310 && mouse_y<328 && mouse_x<=108)
		{
			if(shirt_color==SHIRT_BLACK)
				shirt_color=SHIRT_YELLOW;
			else shirt_color--;
			change_actor();
		}
	if(mouse_x>=120 && mouse_y>310 && mouse_y<328 && mouse_x<=138)
		{
			if(shirt_color==SHIRT_YELLOW)
				shirt_color=SHIRT_BLACK;
			else shirt_color++;
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
					if(race==race_human)
						head=HEAD_5;
					else
						head=HEAD_4;
				}
			else head--;
			change_actor();
		}
	if(mouse_x>=120 && mouse_y>370 && mouse_y<388 && mouse_x<=138)
		{
			if(head==HEAD_4 &&!(race==race_human))
				{
					head=HEAD_1;
				}
			else if(head==HEAD_5)
				head=HEAD_1;
			else head++;
			change_actor();
		}
	//check to see if we changed the gender
	//530,300
	if(mouse_x>530 && mouse_x<620 && mouse_y>300 && mouse_y<320)
		{
			male=1;
			change_actor();
		}

	if(mouse_x>530 && mouse_x<620 && mouse_y>320 && mouse_y<340)
		{
			male=0;
			change_actor();
		}
	//check to see if we changed the race
	if(mouse_x>530 && mouse_x<620 && mouse_y>380 && mouse_y<400)
		{
			race=race_human;
			change_actor();
		}
	if(mouse_x>530 && mouse_x<620 && mouse_y>400 && mouse_y<420)
		{
			race=race_elf;
			change_actor();
		}
	if(mouse_x>530 && mouse_x<620 && mouse_y>420 && mouse_y<440)
		{
			race=race_dwarf;
			change_actor();
		}
	//check to see the selected dialogue boxes
	if(mouse_x>10 && mouse_x<200 && mouse_y>30 && mouse_y<58)
		{
			username=1;
			password=0;
			confirm_pass=0;
		}
	if(mouse_x>10 && mouse_x<200 && mouse_y>88 && mouse_y<116)
		{
			username=0;
			password=1;
			confirm_pass=0;
		}

	if(mouse_x>10 && mouse_x<200 && mouse_y>146 && mouse_y<174)
		{
			username=0;
			password=0;
			confirm_pass=1;
		}


}

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

	check_for_input();

	//see if we have to load a model (male or female)
	if(!any_model)
		{
			our_model=add_actor_interface(race+male,skin_color,hair_color,shirt_color,pants_color,boots_color,head);
			any_model=1;//we have an actor loaded
		}

	glColor3f(1.0f,1.0f,1.0f);
	draw_string(10,270,skin_str,1);
	if(skin_b)glColor3f(0.3f,1.0f,0.3f);
	else glColor3f(1.0f,0.7f,0.0f);
	draw_string(90,270,"<<",1);
	if(skin_f)glColor3f(0.3f,1.0f,0.3f);
	else glColor3f(1.0f,0.7f,0.0f);
	draw_string(120,270,">>",1);

	glColor3f(1.0f,1.0f,1.0f);
	draw_string(10,290,hair_str,1);
	if(hair_b)glColor3f(0.3f,1.0f,0.3f);
	else glColor3f(1.0f,0.7f,0.0f);
	draw_string(90,290,"<<",1);
	if(hair_f)glColor3f(0.3f,1.0f,0.3f);
	else glColor3f(1.0f,0.7f,0.0f);
	draw_string(120,290,">>",1);

	glColor3f(1.0f,1.0f,1.0f);
	draw_string(10,310,shirt_str,1);
	if(shirt_b)glColor3f(0.3f,1.0f,0.3f);
	else glColor3f(1.0f,0.7f,0.0f);
	draw_string(90,310,"<<",1);
	if(shirt_f)glColor3f(0.3f,1.0f,0.3f);
	else glColor3f(1.0f,0.7f,0.0f);
	draw_string(120,310,">>",1);

	glColor3f(1.0f,1.0f,1.0f);
	draw_string(10,330,pants_str,1);
	if(pants_b)glColor3f(0.3f,1.0f,0.3f);
	else glColor3f(1.0f,0.7f,0.0f);
	draw_string(90,330,"<<",1);
	if(pants_f)glColor3f(0.3f,1.0f,0.3f);
	else glColor3f(1.0f,0.7f,0.0f);
	draw_string(120,330,">>",1);

	glColor3f(1.0f,1.0f,1.0f);
	draw_string(10,350,boots_str,1);
	if(boots_b)glColor3f(0.3f,1.0f,0.3f);
	else glColor3f(1.0f,0.7f,0.0f);
	draw_string(90,350,"<<",1);
	if(boots_f)glColor3f(0.3f,1.0f,0.3f);
	else glColor3f(1.0f,0.7f,0.0f);
	draw_string(120,350,">>",1);

	glColor3f(1.0f,1.0f,1.0f);
	draw_string(10,370,head_str,1);
	if(head_b)glColor3f(0.3f,1.0f,0.3f);
	else glColor3f(1.0f,0.7f,0.0f);
	draw_string(90,370,"<<",1);
	if(head_f)glColor3f(0.3f,1.0f,0.3f);
	else glColor3f(1.0f,0.7f,0.0f);
	draw_string(120,370,">>",1);

	glColor3f(1.0f,0.2f,0.2f);
	draw_string(530,280,gender_str,1);

	if(male==1)glColor3f(0.0f,0.5f,1.0f);
	else glColor3f(1.0f,1.0f,1.0f);
	draw_string(530,300,male_str,1);

	if(male!=1)glColor3f(0.0f,0.5f,1.0f);
	else glColor3f(1.0f,1.0f,1.0f);
	draw_string(530,320,female_str,1);
	glColor3f(1.0f,1.0f,1.0f);

	glColor3f(1.0f,0.2f,0.2f);
	draw_string(530,360,"Race",1);

	if(race==race_human)glColor3f(0.0f,0.5f,1.0f);
	else glColor3f(1.0f,1.0f,1.0f);
	draw_string(530,380,human_str,1);
	glColor3f(1.0f,1.0f,1.0f);

	if(race==race_elf)glColor3f(0.0f,0.5f,1.0f);
	else glColor3f(1.0f,1.0f,1.0f);
	draw_string(530,400,elf_str,1);
	glColor3f(1.0f,1.0f,1.0f);

	if(race==race_dwarf)glColor3f(0.0f,0.5f,1.0f);
	else glColor3f(1.0f,1.0f,1.0f);
	draw_string(530,420,dwarf_str,1);
	glColor3f(1.0f,1.0f,1.0f);

	//draw the player frame
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glColor4f(0.4f,0.4f,0.4f,0.5f);
	glVertex3i(actor_creation_menu_x_start,actor_creation_menu_y_end,0);
	glVertex3i(actor_creation_menu_x_start,actor_creation_menu_y_start,0);
	glVertex3i(actor_creation_menu_x_end,actor_creation_menu_y_start,0);
	glVertex3i(actor_creation_menu_x_end,actor_creation_menu_y_end,0);

	glVertex3i(actor_creation_menu_x_start+200,actor_creation_menu_y_end,0);
	glVertex3i(actor_creation_menu_x_start+200,actor_creation_menu_y_start,0);
	glVertex3i(actor_creation_menu_x_end+200,actor_creation_menu_y_start,0);
	glVertex3i(actor_creation_menu_x_end+200,actor_creation_menu_y_end,0);
	glEnd();

	glColor3f(0.2f,0.2f,0.2f);
	glBegin(GL_LINES);
	glVertex3i(actor_creation_menu_x_start,actor_creation_menu_y_start,0);
	glVertex3i(actor_creation_menu_x_end,actor_creation_menu_y_start,0);
	glVertex3i(actor_creation_menu_x_end,actor_creation_menu_y_start,0);
	glVertex3i(actor_creation_menu_x_end,actor_creation_menu_y_end,0);
	glVertex3i(actor_creation_menu_x_end,actor_creation_menu_y_end,0);
	glVertex3i(actor_creation_menu_x_start,actor_creation_menu_y_end,0);
	glVertex3i(actor_creation_menu_x_start,actor_creation_menu_y_end,0);
	glVertex3i(actor_creation_menu_x_start,actor_creation_menu_y_start,0);

	glVertex3i(actor_creation_menu_x_start+200,actor_creation_menu_y_start,0);
	glVertex3i(actor_creation_menu_x_end+200,actor_creation_menu_y_start,0);
	glVertex3i(actor_creation_menu_x_end+200,actor_creation_menu_y_start,0);
	glVertex3i(actor_creation_menu_x_end+200,actor_creation_menu_y_end,0);
	glVertex3i(actor_creation_menu_x_end+200,actor_creation_menu_y_end,0);
	glVertex3i(actor_creation_menu_x_start+200,actor_creation_menu_y_end,0);
	glVertex3i(actor_creation_menu_x_start+200,actor_creation_menu_y_end,0);
	glVertex3i(actor_creation_menu_x_start+200,actor_creation_menu_y_start,0);

	glEnd();
	glEnable(GL_TEXTURE_2D);

	//draw the player
	glEnable(GL_DEPTH_TEST);
	draw_interface_actor(our_model,150.0f,300,250,0,90.0f,0.0f, 0.0f);

	draw_interface_actor(our_model,150.0f,500,250,0,270.0f,0.0f, -180.0f);
	glDisable(GL_DEPTH_TEST);

	//now start putting the dialogue boxes.
	get_and_set_texture_id(login_screen_menus);
	glColor3f(1.0f,1.0f,1.0f);
	glBegin(GL_QUADS);

	//username box
	if(username)
		draw_2d_thing(selected_bar_u_start,selected_bar_v_start,
					  selected_bar_u_end,selected_bar_v_end,10,30,200,58);
	else
		draw_2d_thing(unselected_bar_u_start,unselected_bar_v_start,
					  unselected_bar_u_end,unselected_bar_v_end,10,30,200,58);

	//password box
	if(password)
		draw_2d_thing(selected_bar_u_start,selected_bar_v_start,
					  selected_bar_u_end,selected_bar_v_end,10,88,200,116);
	else
		draw_2d_thing(unselected_bar_u_start,unselected_bar_v_start,
					  unselected_bar_u_end,unselected_bar_v_end,10,88,200,116);

	//confirm box
	if(confirm_pass)
		draw_2d_thing(selected_bar_u_start,selected_bar_v_start,
					  selected_bar_u_end,selected_bar_v_end,10,146,200,174);
	else
		draw_2d_thing(unselected_bar_u_start,unselected_bar_v_start,
					  unselected_bar_u_end,unselected_bar_v_end,10,146,200,174);

	//done_button button
	if(done_button)
		draw_2d_thing(done_button_selected_start_u,done_button_selected_start_v,
					  done_button_selected_end_u,done_button_selected_end_v,10,
					  200,97,235);
	else
		draw_2d_thing(done_button_unselected_start_u,done_button_unselected_start_v,
					  done_button_unselected_end_u,done_button_unselected_end_v,10,
					  200,97,235);

	//back button
	if(back)
		draw_2d_thing(back_selected_start_u,back_selected_start_v,
					  back_selected_end_u,back_selected_end_v,110,
					  200,197,235);
	else
		draw_2d_thing(back_unselected_start_u,back_unselected_start_v,
					  back_unselected_end_u,back_unselected_end_v,110,
					  200,197,235);

	glEnd();

	//now, draw the text 'labels'
	glColor3f(0.0f,1.0f,0.5f);
	draw_string(60,10,login_username_str,1);

	draw_string(60,68,login_password_str,1);

	draw_string(20,126,confirm_password,1);

	//put the username, pass, and conf pass user text
	glColor3f(0.0f,0.5f,1.0f);
	draw_string(15,97,display_pass_str,1);

	draw_string(15,154,display_conf_pass_str,1);

	draw_string(15,38,user_str,1);

	glColor3f(1.0f,0.0f,0.0f);
	//print the current error, if any
	draw_string(10,400,create_char_error_str,1);



}

void add_char_2_pass(unsigned char ch)
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

	if(ch==SDLK_TAB)
		{
			password=0;
			confirm_pass=1;
		}
}

void add_char_2_un(unsigned char ch)
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
	if(ch==SDLK_TAB)
		{
			username=0;
			password=1;
		}
}

void add_char_2_conf(unsigned char ch)
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

	if(ch==SDLK_TAB)
		{
			confirm_pass=0;
			username=1;
		}
}

void add_char_to_new_character(unsigned char ch)
{

	if(username)add_char_2_un(ch);
	else if(password)add_char_2_pass(ch);
	else if(confirm_pass)add_char_2_conf(ch);
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
