#include <string.h>
#include "global.h"
#include "elwindows.h"

char dialogue_string[2048];
char npc_name[20];
int cur_portrait=8;
int portraits1_tex;
int portraits2_tex;
int portraits3_tex;
int portraits4_tex;
int portraits5_tex;

response dialogue_responces[MAX_RESPONSES];
int dialogue_win= 0;

int dialogue_menu_x=1;
int dialogue_menu_y=1;
int dialogue_menu_x_len=638;
int dialogue_menu_y_len=190;
//int dialogue_menu_dragged=0;

int no_bounding_box=0;


void build_response_entries(Uint8 *data,int total_lenght)
{
	int i;
	int len;
	int last_index=0;
	int x_start,y_start;

	x_start=0;
	y_start=0;

	//first, clear the previous dialogue entries
	for(i=0;i<MAX_RESPONSES;i++)dialogue_responces[i].in_use=0;

	i=0;
	for(i=0;i<MAX_RESPONSES;i++)
		{
			// break if we don't have a length field
			if (last_index + 3 > total_lenght)
				break;
			len=*((Uint16 *)(data+last_index));
			// break if we don't have a complete response
			if (last_index + 3 + len + 2 + 2 > total_lenght)
				break;
			dialogue_responces[i].in_use=1;
			my_strncp(dialogue_responces[i].text,&data[last_index+2], len);
			dialogue_responces[i].response_id=*((Uint16 *)(data+last_index+2+len));
			dialogue_responces[i].to_actor=*((Uint16 *)(data+last_index+2+2+len));
			dialogue_responces[i].x_len=len*8;
			dialogue_responces[i].y_len=14;

			if(x_start+len*8>dialogue_menu_x_len)
				{
					x_start=0;
					y_start+=14;
				}
			dialogue_responces[i].x_start=x_start;
			dialogue_responces[i].y_start=y_start;
			last_index+=len+2+2+2;
			x_start+=(len+2)*8;
		}
}

int	display_dialogue_handler(window_info *win)
{
	int i;
	float u_start,v_start,u_end,v_end;
	int this_texture; //,cur_item,cur_pos; unused?
	int x_start,x_end,y_start,y_end;
	int npc_name_x_start,len;

	//calculate the npc_name_x_start (to have it centered on the screen)
	len= strlen(npc_name);
	npc_name_x_start= win->len_x/2-(len*8)/2;

	glDisable(GL_TEXTURE_2D);
	//draw the character frame
	glColor3f(0.0f,1.0f,1.0f);
	glBegin(GL_LINES);
	glVertex3i(0,0,0);
	glVertex3i(66,0,0);

	glVertex3i(66,0,0);
	glVertex3i(66,66,0);

	glVertex3i(66,66,0);
	glVertex3i(0,66,0);

	glVertex3i(0,66,0);
	glVertex3i(0,0,0);

	glEnd();
	glEnable(GL_TEXTURE_2D);

	glColor3f(1.0f,1.0f,1.0f);
	//ok, now let's draw the portrait
	if(cur_portrait!=-1)
		{
			//get the UV coordinates.
			u_start=0.25f*(cur_portrait%4);
			u_end=u_start+0.25f;
			v_start=1.0f-(0.25f*(cur_portrait/4));
			v_end=v_start-0.25f;

			//get the x and y
			x_start=1;
			x_end=x_start+64;
			y_start=1;
			y_end=y_start+64;

			//get the texture this item belongs to
			this_texture=cur_portrait/16;
			if(this_texture==0)this_texture=portraits1_tex;
			else
				if(this_texture==1)this_texture=portraits2_tex;
				else
					if(this_texture==2)this_texture=portraits3_tex;
					else
						if(this_texture==3)this_texture=portraits4_tex;
						else
							if(this_texture==4)this_texture=portraits5_tex;


			get_and_set_texture_id(this_texture);
			glBegin(GL_QUADS);
			draw_2d_thing(u_start,v_start,u_end,v_end,x_start,y_start,x_end,y_end);
			glEnd();
		}
	//draw the main text
	draw_string_small(70,2,dialogue_string,8);
	//now, draw the character name
	glColor3f(1.0f,1.0f,1.0f);
	draw_string_small(npc_name_x_start,win->len_y-16,npc_name,1);
	draw_string_small(win->len_x-60,win->len_y-16,close_str,1);

	//ok, now draw the responses
	for(i=0;i<MAX_RESPONSES;i++)
		{
			if(dialogue_responces[i].in_use)
				{
					if(dialogue_responces[i].mouse_over)
						glColor3f(1.0f,0.5f,0.0f);
					else
						glColor3f(1.0f,1.0f,0.0f);
					draw_string_small(dialogue_responces[i].x_start+5,dialogue_responces[i].y_start+7*14,dialogue_responces[i].text,1);
				}

		}

	glColor3f(1.0f,1.0f,1.0f);
	return 0;
}


void close_dialogue()
{
	if(dialogue_win > 0)
	{
		hide_window(dialogue_win);
	}
}

int mouseover_dialogue_handler(window_info *win, int mx, int my)
{
	int i;

	//first, clear the mouse overs
	for(i=0;i<MAX_RESPONSES;i++)dialogue_responces[i].mouse_over=0;

	for(i=0;i<MAX_RESPONSES;i++)
		{
			if(dialogue_responces[i].in_use)
				{
					if(mx>=dialogue_responces[i].x_start+5 && mx<=dialogue_responces[i].x_start+5+dialogue_responces[i].x_len &&
					   my>=dialogue_responces[i].y_start+7*14 && my<=dialogue_responces[i].y_start+7*14+dialogue_responces[i].y_len)
						{
							dialogue_responces[i].mouse_over=1;
							return 0;
						}
				}
		}
	return 0;
}

int click_dialogue_handler(window_info *win, int mx, int my, Uint32 flags)
{
	int i;
	Uint8 str[16];

	for(i=0;i<MAX_RESPONSES;i++)
		{
			if(dialogue_responces[i].in_use && dialogue_responces[i].mouse_over)
				{
					str[0]=RESPOND_TO_NPC;
					*((Uint16 *)(str+1))=dialogue_responces[i].to_actor;
					*((Uint16 *)(str+3))=dialogue_responces[i].response_id;
					my_tcp_send(my_socket,str,5);
					return 1;
				}
		}
	if(mx>=win->len_x-60 && my>=win->len_y-16)
		{
			hide_window(win->window_id);
			return 1;
		}

	return 0;
}

void display_dialogue()
{
	if(dialogue_win <= 0){
		dialogue_win= create_window("Dialogue", 0, 0, dialogue_menu_x, dialogue_menu_y, dialogue_menu_x_len, dialogue_menu_y_len, ELW_WIN_DEFAULT);

		set_window_handler(dialogue_win, ELW_HANDLER_DISPLAY, &display_dialogue_handler );
		set_window_handler(dialogue_win, ELW_HANDLER_MOUSEOVER, &mouseover_dialogue_handler );
		set_window_handler(dialogue_win, ELW_HANDLER_CLICK, &click_dialogue_handler );
	} else {
		show_window(dialogue_win);
		select_window(dialogue_win);
	}
}

