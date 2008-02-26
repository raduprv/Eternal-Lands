#include <stdlib.h>
#include <string.h>
#include <SDL_endian.h>
#include "dialogues.h"
#include "asc.h"
#include "elwindows.h"
#include "gamewin.h"
#include "interface.h"
#include "multiplayer.h"
#include "textures.h"
#include "translate.h"
#ifdef OPENGL_TRACE
#include "gl_init.h"
#endif

unsigned char dialogue_string[2048];
unsigned char npc_name[20];
int cur_portrait=8;
int portraits_tex[MAX_PORTRAITS_TEXTURES];

response dialogue_responces[MAX_RESPONSES];
int dialogue_win= -1;

int dialogue_menu_x=1;
int dialogue_menu_y=1;
int dialogue_menu_x_len=638;
int dialogue_menu_y_len=190;
//int dialogue_menu_dragged=0;

int no_bounding_box=0;
int show_keypress_letters=0;

void build_response_entries (const Uint8 *data, int total_length)
{
	int i;
	int len;
	int orig_len;
	int last_index=0;
	int x_start,y_start;
	int orig_x_start,orig_y_start;

	x_start=0;
	y_start=0;
	orig_x_start=0;
	orig_y_start=0;

	//first, clear the previous dialogue entries
	for(i=0;i<MAX_RESPONSES;i++)dialogue_responces[i].in_use=0;

	i=0;
	for(i=0;i<MAX_RESPONSES;i++)
	{
		// break if we don't have a length field
		if (last_index + 3 > total_length)
			break;
		orig_len=len=SDL_SwapLE16(*((Uint16 *)(data+last_index)));
		
		// break if we don't have a complete response
		if (last_index + 3 + len + 2 + 2 > total_length)
			break;
		dialogue_responces[i].in_use=1;
		my_strncp(dialogue_responces[i].text,(char*)&data[last_index+2], len);
		dialogue_responces[i].response_id=SDL_SwapLE16(*((Uint16 *)(data+last_index+2+len)));
		dialogue_responces[i].to_actor=SDL_SwapLE16(*((Uint16 *)(data+last_index+2+2+len)));
		last_index+=len+2+2+2;//why not len+6?
		dialogue_responces[i].orig_x_len=orig_len*8;
		dialogue_responces[i].orig_y_len=14;
		if(i<36) // [1-0, a-z] [']'] [space] eg 1] Physique 2] Coordination 3] Will
		{
		    len+=3;
		}
		dialogue_responces[i].x_len=len*8;
		dialogue_responces[i].y_len=14;

		if(orig_x_start+orig_len*8>dialogue_menu_x_len)
		{
			orig_x_start=0;
			orig_y_start+=14;
		}
		dialogue_responces[i].orig_x_start=orig_x_start;
		dialogue_responces[i].orig_y_start=orig_y_start;

		if(x_start+len*8>dialogue_menu_x_len)
		{
			x_start=0;
			y_start+=14;
		}
		dialogue_responces[i].x_start=x_start;
		dialogue_responces[i].y_start=y_start;
		x_start+=(len+2)*8;
		orig_x_start+=(orig_len+2)*8;
	}
}

int	display_dialogue_handler(window_info *win)
{
	int i;
	float u_start,v_start,u_end,v_end;
	int this_texture; //,cur_item,cur_pos; unused?
	int x_start,x_end,y_start,y_end;
	int npc_name_x_start,len;
	unsigned char str[128]; 

	//calculate the npc_name_x_start (to have it centered on the screen)
	len= strlen((char*)npc_name);
	npc_name_x_start= win->len_x/2-(len*8)/2;

	glDisable(GL_TEXTURE_2D);
	//draw the character frame
	glColor3f(0.0f,1.0f,1.0f);
	glBegin(GL_LINE_LOOP);
	glVertex3i(0,0,0);
	glVertex3i(66,0,0);
	glVertex3i(66,66,0);
	glVertex3i(0,66,0);
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
		this_texture=portraits_tex[this_texture];

		get_and_set_texture_id(this_texture);
		glBegin(GL_QUADS);
		draw_2d_thing(u_start,v_start,u_end,v_end,x_start,y_start,x_end,y_end);
		glEnd();
	}
	y_start=0;
	//draw the main text
	draw_string_small(70,2,dialogue_string,8);

	//ok, now draw the responses
	for(i=0;i<MAX_RESPONSES;i++)
	{
		if(dialogue_responces[i].in_use)
		{
			if(dialogue_responces[i].mouse_over)
				glColor3f(1.0f,0.5f,0.0f);
			else
				glColor3f(1.0f,1.0f,0.0f);
			if(mouse_x<win->pos_x || mouse_x>win->pos_x+win->len_x || mouse_y<win->pos_y || mouse_y>win->pos_y+win->len_y)
				show_keypress_letters=0;
			if(use_keypress_dialogue_boxes && show_keypress_letters)
			{
				if(i>=0 && i<=8) // 1-9
					safe_snprintf((char*)str,sizeof(str),"%c] %s",49+i,(unsigned char*)dialogue_responces[i].text);
				else if(i==9) //0
					safe_snprintf((char*)str,sizeof(str),"0] %s",(unsigned char*)dialogue_responces[i].text);
				else if(i>=10 && i<=35) // A-Z
					safe_snprintf((char*)str,sizeof(str),"%c] %s",55+i, (unsigned char*)dialogue_responces[i].text);
				else // too many dialogue options, you have to click these
					 safe_snprintf((char*)str,sizeof(str),"%s",(unsigned char*)dialogue_responces[i].text);
				draw_string_small(dialogue_responces[i].x_start+5,dialogue_responces[i].y_start+7*14,str,1);
				y_start=(dialogue_responces[i].y_start+7*14)+32;
			}
			else
			{
				safe_snprintf((char*)str,sizeof(str),"%s",(unsigned char*)dialogue_responces[i].text);
				draw_string_small(dialogue_responces[i].orig_x_start+5,dialogue_responces[i].orig_y_start+7*14,str,1);
				y_start=(dialogue_responces[i].orig_y_start+7*14)+32;
			}
		}
	}

	glColor3f(1.0f,1.0f,1.0f);
	if(y_start>win->orig_len_y)//automatically Y-resizing window if there are a lot of options
	    win->len_y=y_start;
    else
        win->len_y=win->orig_len_y;
        
	//now, draw the character name
	glColor3f(1.0f,1.0f,1.0f);
	draw_string_small(npc_name_x_start,win->len_y-16,npc_name,1);
	draw_string_small(win->len_x-60,win->len_y-16,(unsigned char*)close_str,1);

#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	return 0;
}


void close_dialogue()
{
	if(dialogue_win >= 0)
	{
		hide_window(dialogue_win);
	}
}

int mouseover_dialogue_handler(window_info *win, int mx, int my)
{
	int i;
	
	show_keypress_letters=0;
	if(use_keypress_dialogue_boxes)
	{
	 	if(use_full_dialogue_window || ((mx>=0 && mx<=64) && (my>=0 && my<=64)))
	 	{
			show_keypress_letters=1;
	    }	    
    }

	//first, clear the mouse overs
	for(i=0;i<MAX_RESPONSES;i++)dialogue_responces[i].mouse_over=0;

	for(i=0;i<MAX_RESPONSES;i++)
	{
		if(dialogue_responces[i].in_use)
		{
			if(show_keypress_letters)
			{
				if(mx>=dialogue_responces[i].x_start+5 && mx<=dialogue_responces[i].x_start+5+dialogue_responces[i].x_len &&
				   my>=dialogue_responces[i].y_start+7*14 && my<=dialogue_responces[i].y_start+7*14+dialogue_responces[i].y_len)
				{
					dialogue_responces[i].mouse_over=1;
					return 0;
				}
			}
			else
			{
				if(mx>=dialogue_responces[i].orig_x_start+5 && mx<=dialogue_responces[i].orig_x_start+5+dialogue_responces[i].orig_x_len &&
				   my>=dialogue_responces[i].orig_y_start+7*14 && my<=dialogue_responces[i].orig_y_start+7*14+dialogue_responces[i].orig_y_len)
				{
					dialogue_responces[i].mouse_over=1;
					return 0;
				}
			}

		}
	}
	return 0;
}

int click_dialogue_handler(window_info *win, int mx, int my, Uint32 flags)
{
	int i;
	Uint8 str[16];

	// only handle mouse button clicks, not scroll wheels moves
	if ( (flags & ELW_MOUSE_BUTTON) == 0) return 0;

	for(i=0;i<MAX_RESPONSES;i++)
		{
			if(dialogue_responces[i].in_use && dialogue_responces[i].mouse_over)
				{
					str[0]=RESPOND_TO_NPC;
					*((Uint16 *)(str+1))=SDL_SwapLE16((short)dialogue_responces[i].to_actor);
					*((Uint16 *)(str+3))=SDL_SwapLE16((short)dialogue_responces[i].response_id);
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

int keypress_dialogue_handler (window_info *win, int mx, int my, Uint32 key, Uint32 unikey)
{
	Uint8 str[16], ch;
	if(!use_keypress_dialogue_boxes)
	{
		return 0;
	}

	if ((use_full_dialogue_window == 0) && (mx<0 || mx>64 || my<0 || my>64))
	{
	    return 0;
	}

	ch = key_to_char (unikey);
	if ((key & 0xffff) == SDLK_ESCAPE) // close window if Escape pressed
	{
		hide_window(win->window_id);
		return 1;
   }
	if(ch<'0' || ch>'z') // do not send special keys
	{
		return 0;
   }
	if(ch>='a' && ch<='z')
		ch-=87; //a-z->10-35
   else if(ch>='A' && ch<='Z')
		ch-=55; //A-Z->10-35
	else if(ch>='1' && ch<='9')
		ch-=49; //1-9->0-8
	else if(ch=='0') //0->9
		ch=9;
	else //out of range
	{
		return 0;
   }
	if(MAX_RESPONSES-1<ch)//pressed a key that the client is not expecting, ignore it
	{
		return 1;
	}
	if(dialogue_responces[ch].in_use)
	{
		str[0]=RESPOND_TO_NPC;
		*((Uint16 *)(str+1))=SDL_SwapLE16((short)dialogue_responces[ch].to_actor);
		*((Uint16 *)(str+3))=SDL_SwapLE16((short)dialogue_responces[ch].response_id);
		my_tcp_send(my_socket,str,5);
		return 1;
	}
	return 0;
}

void display_dialogue()
{
	if(dialogue_win < 0){
		dialogue_win= create_window("Dialogue", game_root_win, 0, dialogue_menu_x, dialogue_menu_y, dialogue_menu_x_len, dialogue_menu_y_len, ELW_WIN_DEFAULT);

		set_window_handler(dialogue_win, ELW_HANDLER_DISPLAY, &display_dialogue_handler );
		set_window_handler(dialogue_win, ELW_HANDLER_MOUSEOVER, &mouseover_dialogue_handler );
		set_window_handler(dialogue_win, ELW_HANDLER_KEYPRESS, &keypress_dialogue_handler );
		set_window_handler(dialogue_win, ELW_HANDLER_CLICK, &click_dialogue_handler );
		
	} else {
		show_window(dialogue_win);
		select_window(dialogue_win);
	}
}

