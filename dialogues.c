#include "global.h"



void build_response_entries(Uint8 *data,int total_lenght)
{
	int i;
	int len;
	int last_index=0;
	int x_start,y_start;

	x_start=0;
	y_start=0;

	//first, clear the previous dialogue entries
	for(i=0;i<20;i++)dialogue_responces[i].in_use=0;

	i=0;
	for(i=0;i<20;i++)
		{
			len=*((Uint16 *)(data+last_index));
			dialogue_responces[i].in_use=1;
			my_strcp(dialogue_responces[i].text,&data[last_index+2]);
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

			//see if we exceeded the limit
			if(last_index+3>total_lenght)break;
		}
}

void display_dialogue()
{
	Uint8 str[80];
	int x,y,i;
	float u_start,v_start,u_end,v_end;
	int this_texture,cur_item,cur_pos;
	int x_start,x_end,y_start,y_end;
	int npc_name_x_start,len;

	draw_menu_title_bar(dialogue_menu_x,dialogue_menu_y-16,dialogue_menu_x_len);

	//calculate the npc_name_x_start (to have it centered on the screen)
	len=strlen(npc_name);
	npc_name_x_start=dialogue_menu_x+dialogue_menu_x_len/2-(len*8)/2;

	//first of all, draw the actual menu.
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE,GL_SRC_ALPHA);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glColor4f(0.0f,0.0f,0.0f,0.3f);
	glVertex3i(dialogue_menu_x,dialogue_menu_y+dialogue_menu_y_len,0);
	glVertex3i(dialogue_menu_x,dialogue_menu_y,0);
	glVertex3i(dialogue_menu_x+dialogue_menu_x_len,dialogue_menu_y,0);
	glVertex3i(dialogue_menu_x+dialogue_menu_x_len,dialogue_menu_y+dialogue_menu_y_len,0);
	glEnd();

	glDisable(GL_BLEND);

	glColor3f(0.0f,1.0f,0.3f);
	glBegin(GL_LINES);
	glVertex3i(dialogue_menu_x,dialogue_menu_y,0);
	glVertex3i(dialogue_menu_x+dialogue_menu_x_len,dialogue_menu_y,0);

	glVertex3i(dialogue_menu_x+dialogue_menu_x_len,dialogue_menu_y,0);
	glVertex3i(dialogue_menu_x+dialogue_menu_x_len,dialogue_menu_y+dialogue_menu_y_len,0);

	glVertex3i(dialogue_menu_x+dialogue_menu_x_len,dialogue_menu_y+dialogue_menu_y_len,0);
	glVertex3i(dialogue_menu_x,dialogue_menu_y+dialogue_menu_y_len,0);

	glVertex3i(dialogue_menu_x,dialogue_menu_y+dialogue_menu_y_len,0);
	glVertex3i(dialogue_menu_x,dialogue_menu_y,0);


	//draw the character frame
	glColor3f(0.0f,1.0f,1.0f);
	glVertex3i(dialogue_menu_x,dialogue_menu_y,0);
	glVertex3i(dialogue_menu_x+66,dialogue_menu_y,0);

	glVertex3i(dialogue_menu_x+66,dialogue_menu_y,0);
	glVertex3i(dialogue_menu_x+66,dialogue_menu_y+66,0);

	glVertex3i(dialogue_menu_x+66,dialogue_menu_y+66,0);
	glVertex3i(dialogue_menu_x,dialogue_menu_y+66,0);

	glVertex3i(dialogue_menu_x,dialogue_menu_y+66,0);
	glVertex3i(dialogue_menu_x,dialogue_menu_y,0);

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
					x_start=dialogue_menu_x+1;
					x_end=x_start+64;
					y_start=dialogue_menu_y+1;
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


					if(last_texture!=texture_cache[this_texture].texture_id)
						{
							glBindTexture(GL_TEXTURE_2D, texture_cache[this_texture].texture_id);
							last_texture=texture_cache[this_texture].texture_id;
						}

					glBegin(GL_QUADS);
					draw_2d_thing(u_start,v_start,u_end,v_end,x_start,y_start,x_end,y_end);
					glEnd();
		}
	//draw the main text
	draw_string_small(dialogue_menu_x+70,dialogue_menu_y+2,dialogue_string,8);
	//now, draw the character name
	glColor3f(1.0f,1.0f,1.0f);
	draw_string_small(npc_name_x_start,dialogue_menu_y+dialogue_menu_y_len-16,npc_name,1);
	draw_string_small(dialogue_menu_x+dialogue_menu_x_len-60,dialogue_menu_y+dialogue_menu_y_len-16,"[close]",1);

	//ok, now draw the responses
	for(i=0;i<20;i++)
		{
			if(dialogue_responces[i].in_use)
				{
					if(dialogue_responces[i].mouse_over)
					glColor3f(1.0f,0.5f,0.0f);
					else
					glColor3f(1.0f,1.0f,0.0f);
					draw_string_small(dialogue_responces[i].x_start+dialogue_menu_x+5,dialogue_responces[i].y_start+dialogue_menu_y+7*14,dialogue_responces[i].text,1);
				}

		}

	glColor3f(1.0f,1.0f,1.0f);
}

int highlight_dialogue_response()
{
		int i;

		//first, clear the mouse overs
		for(i=0;i<20;i++)dialogue_responces[i].mouse_over=0;

		for(i=0;i<20;i++)
			{
				if(dialogue_responces[i].in_use)
					{
						if(mouse_x>=dialogue_responces[i].x_start+dialogue_menu_x+5 && mouse_x<=dialogue_responces[i].x_start+dialogue_menu_x+5+dialogue_responces[i].x_len &&
						mouse_y>=dialogue_responces[i].y_start+dialogue_menu_y+7*14 && mouse_y<=dialogue_responces[i].y_start+dialogue_menu_y+7*14+dialogue_responces[i].y_len)
							{
								dialogue_responces[i].mouse_over=1;
								return;
							}
					}
			}
}

int check_dialogue_response()
{
		int i;
		Uint8 str[16];

		for(i=0;i<20;i++)
			{
				if(dialogue_responces[i].in_use)
					{
						if(mouse_x>=dialogue_responces[i].x_start+dialogue_menu_x+5 && mouse_x<=dialogue_responces[i].x_start+dialogue_menu_x+5+dialogue_responces[i].x_len &&
						mouse_y>=dialogue_responces[i].y_start+dialogue_menu_y+7*14 && mouse_y<=dialogue_responces[i].y_start+dialogue_menu_y+7*14+dialogue_responces[i].y_len)
							{
								str[0]=RESPOND_TO_NPC;
								*((Uint16 *)(str+1))=dialogue_responces[i].to_actor;
								*((Uint16 *)(str+3))=dialogue_responces[i].response_id;
								my_tcp_send(my_socket,str,5);
								return 1;
							}
					}
			}

			if(mouse_x>=dialogue_menu_x+dialogue_menu_x_len-60 && mouse_x<=dialogue_menu_x+dialogue_menu_x_len
			&& mouse_y>=dialogue_menu_y+dialogue_menu_y_len-16 && mouse_y<=dialogue_menu_y+dialogue_menu_y_len)
				{
					have_dialogue=0;
					return 1;
				}
	return 0;

}






