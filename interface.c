#include <string.h>
#include "global.h"
#include <math.h>

void get_world_x_y()
{
	float window_ratio;
	float x,y,x1,y1,a,t;
	window_ratio=(GLfloat)window_width/(GLfloat)window_height;

	x=(float)(mouse_x*9.0f/window_width)-(9.0f/2.0f);
	y=(float)((window_height-mouse_y)*6.0f/window_height)-(6.0f/2.0f);

	a=(rz)*3.1415926/180;
	t=(rx)*3.1415926/180;

	y=y/cos(t);

	x1=x*cos(a)+y*sin(a);
	y1=y*cos(a)-x*sin(a);


	scene_mouse_x=-cx+x1;
	scene_mouse_y=-cy+y1;

}

int check_drag_menus()
{
	if(sigil_menu_dragged || (view_sigils_menu && mouse_x>sigil_menu_x && mouse_x<=sigil_menu_x+sigil_menu_x_len && mouse_y>sigil_menu_y-16 && mouse_y<=sigil_menu_y))
		if(!attrib_menu_dragged && !items_menu_dragged && !ground_items_menu_dragged && !manufacture_menu_dragged &&
		   !trade_menu_dragged && !options_menu_dragged && !dialogue_menu_dragged)
			{
				sigil_menu_dragged=1;
				if(left_click>1)
					{
						sigil_menu_x+=mouse_delta_x;
						sigil_menu_y+=mouse_delta_y;
					}
				return 1;
			}

	if(options_menu_dragged || (options_menu && mouse_x>options_menu_x && mouse_x<=options_menu_x + options_menu_x_len && mouse_y>options_menu_y-16 && mouse_y<=options_menu_y))
		if(!attrib_menu_dragged && !items_menu_dragged && !ground_items_menu_dragged && !manufacture_menu_dragged &&
		   !trade_menu_dragged && !sigil_menu_dragged && !dialogue_menu_dragged)

			{
				options_menu_dragged=1;
				if(left_click>1)
					{
						options_menu_x+=mouse_delta_x;
						options_menu_y+=mouse_delta_y;
					}
				return 1;
			}

	if(trade_menu_dragged || (view_trade_menu && mouse_x>trade_menu_x && mouse_x<=trade_menu_x+trade_menu_x_len && mouse_y>trade_menu_y-16 && mouse_y<=trade_menu_y))
		if(!attrib_menu_dragged && !items_menu_dragged && !ground_items_menu_dragged && !manufacture_menu_dragged &&
		   !sigil_menu_dragged && !options_menu_dragged && !dialogue_menu_dragged)

			{
				trade_menu_dragged=1;
				if(left_click>1)
					{
						trade_menu_x+=mouse_delta_x;
						trade_menu_y+=mouse_delta_y;
					}
				return 1;
			}

	if(manufacture_menu_dragged || (view_manufacture_menu && mouse_x>manufacture_menu_x && mouse_x<=manufacture_menu_x+manufacture_menu_x_len && mouse_y>manufacture_menu_y-16 && mouse_y<=manufacture_menu_y))
		if(!attrib_menu_dragged && !items_menu_dragged && !ground_items_menu_dragged &&
		   !trade_menu_dragged && !sigil_menu_dragged && !options_menu_dragged && !dialogue_menu_dragged)

			{
				manufacture_menu_dragged=1;
				if(left_click>1)
					{
						manufacture_menu_x+=mouse_delta_x;
						manufacture_menu_y+=mouse_delta_y;
					}
				return 1;
			}

	if(ground_items_menu_dragged || (view_ground_items && mouse_x>ground_items_menu_x && mouse_x<=ground_items_menu_x+ground_items_menu_x_len && mouse_y>ground_items_menu_y-16 && mouse_y<=ground_items_menu_y))
		if(!attrib_menu_dragged && !items_menu_dragged && !manufacture_menu_dragged &&
		   !trade_menu_dragged && !sigil_menu_dragged && !options_menu_dragged && !dialogue_menu_dragged)

			{
				ground_items_menu_dragged=1;
				if(left_click>1)
					{
						ground_items_menu_x+=mouse_delta_x;
						ground_items_menu_y+=mouse_delta_y;
					}
				return 1;
			}

	if(items_menu_dragged || (view_my_items && mouse_x>items_menu_x && mouse_x<=items_menu_x+items_menu_x_len && mouse_y>items_menu_y-16 && mouse_y<=items_menu_y))
		if(!attrib_menu_dragged && !ground_items_menu_dragged && !manufacture_menu_dragged &&
		   !trade_menu_dragged && !sigil_menu_dragged && !options_menu_dragged && !dialogue_menu_dragged)
			{
				items_menu_dragged=1;
				if(left_click>1)
					{
						items_menu_x+=mouse_delta_x;
						items_menu_y+=mouse_delta_y;
					}
				return 1;
			}

	if(attrib_menu_dragged || (view_self_stats && mouse_x>attrib_menu_x && mouse_x<=attrib_menu_x+attrib_menu_x_len && mouse_y>attrib_menu_y-16 && mouse_y<=attrib_menu_y))
		if(!items_menu_dragged && !ground_items_menu_dragged && !manufacture_menu_dragged &&
		   !trade_menu_dragged && !sigil_menu_dragged && !options_menu_dragged && !dialogue_menu_dragged)

			{
				attrib_menu_dragged=1;
				if(left_click>1)
					{
						attrib_menu_x+=mouse_delta_x;
						attrib_menu_y+=mouse_delta_y;
					}
				return 1;
			}

	if(dialogue_menu_dragged || (have_dialogue && mouse_x>dialogue_menu_x && mouse_x<=dialogue_menu_x+dialogue_menu_x_len && mouse_y>dialogue_menu_y-16 && mouse_y<=dialogue_menu_y))
		if(!attrib_menu_dragged && !items_menu_dragged && !ground_items_menu_dragged && !manufacture_menu_dragged &&
		   !trade_menu_dragged && !options_menu_dragged && !sigil_menu_dragged)
			{
				dialogue_menu_dragged=1;
				if(left_click>1)
					{
						dialogue_menu_x+=mouse_delta_x;
						dialogue_menu_y+=mouse_delta_y;
					}
				return 1;
			}

	return 0;
}

void check_menus_out_of_screen()
{
	if(attrib_menu_y-16<0)attrib_menu_y=16;
	if(attrib_menu_y>window_height-32)attrib_menu_y=window_height-32;
	if(attrib_menu_x+attrib_menu_x_len<10)attrib_menu_x=0-attrib_menu_x_len+10;
	if(attrib_menu_x>window_width-10)attrib_menu_x=window_width-10;

	if(items_menu_y-16<0)items_menu_y=16;
	if(items_menu_y>window_height-32)items_menu_y=window_height-32;
	if(items_menu_x+items_menu_x_len<10)items_menu_x=0-items_menu_x_len+11;
	if(items_menu_x>window_width-10)items_menu_x=window_width-10;

	if(ground_items_menu_y-16<0)ground_items_menu_y=16;
	if(ground_items_menu_y>window_height-32)ground_items_menu_y=window_height-32;
	if(ground_items_menu_x+ground_items_menu_x_len<10)ground_items_menu_x=0-ground_items_menu_x_len+11;
	if(ground_items_menu_x>window_width-10)ground_items_menu_x=window_width-10;

	if(manufacture_menu_y-16<0)manufacture_menu_y=16;
	if(manufacture_menu_y>window_height-32)manufacture_menu_y=window_height-32;
	if(manufacture_menu_x+manufacture_menu_x_len<10)manufacture_menu_x=0-manufacture_menu_x_len+11;
	if(manufacture_menu_x>window_width-10)manufacture_menu_x=window_width-10;

	if(trade_menu_y-16<0)trade_menu_y=16;
	if(trade_menu_y>window_height-32)trade_menu_y=window_height-32;
	if(trade_menu_x+trade_menu_x_len<10)trade_menu_x=0-trade_menu_x_len+11;
	if(trade_menu_x>window_width-10)trade_menu_x=window_width-10;

	if(sigil_menu_y-16<0)sigil_menu_y=16;
	if(sigil_menu_y>window_height-32)sigil_menu_y=window_height-32;
	if(sigil_menu_x+sigil_menu_x_len<10)sigil_menu_x=0-sigil_menu_x_len+11;
	if(sigil_menu_x>window_width-10)sigil_menu_x=window_width-10;

	if(dialogue_menu_y-16<0)dialogue_menu_y=16;
	if(dialogue_menu_y>window_height-32)dialogue_menu_y=window_height-32;
	if(dialogue_menu_x+dialogue_menu_x_len<10)dialogue_menu_x=0-dialogue_menu_x_len+11;
	if(dialogue_menu_x>window_width-10)dialogue_menu_x=window_width-10;

	if(options_menu_y-16<0)options_menu_y=16;
	if(options_menu_y>window_height-32)options_menu_y=window_height-32;
	if(options_menu_x + options_menu_x_len<10)options_menu_x=0-(options_menu_x + options_menu_x_len-options_menu_x)+11;
	if(options_menu_x>window_width-10)options_menu_x=window_width-10;
}

void check_mouse_click()
{
    if(have_dialogue)
    	{
    		if(mouse_x>=dialogue_menu_x && mouse_x<=dialogue_menu_x+dialogue_menu_x_len
			   && mouse_y>=dialogue_menu_y && mouse_y<=dialogue_menu_y+dialogue_menu_y_len)
				{
					if (check_dialogue_response()) return;	// avoid cloick thrus
				}
		}

	if(view_sigils_menu && mouse_x>(sigil_menu_x+sigil_menu_x_len-20) && mouse_x<=(sigil_menu_x+sigil_menu_x_len)
	   && mouse_y>sigil_menu_y && mouse_y<=sigil_menu_y+20)
		{
			view_sigils_menu=0;
			return;
		}
	if(check_sigil_interface())return;
	if(check_options_menu())return;
	if(check_trade_interface())return;

	if(view_manufacture_menu && mouse_x>(manufacture_menu_x+manufacture_menu_x_len-20) && mouse_x<=(manufacture_menu_x+manufacture_menu_x_len)
	   && mouse_y>manufacture_menu_y && mouse_y<=manufacture_menu_y+20)
		{
			view_manufacture_menu=0;
			return;
		}
	if(check_manufacture_interface())return;

	if(view_ground_items && mouse_x>(ground_items_menu_x+ground_items_menu_x_len-20) && mouse_x<=(ground_items_menu_x+ground_items_menu_x_len)
	   && mouse_y>ground_items_menu_y && mouse_y<=ground_items_menu_y+20)
		{
			unsigned char protocol_name;

			view_ground_items=0;
			protocol_name=S_CLOSE_BAG;
			my_tcp_send(my_socket,&protocol_name,1);
			return;
		}
	if(check_ground_items_interface())return;

	if(view_my_items && mouse_x>(items_menu_x+items_menu_x_len-20) && mouse_x<=(items_menu_x+items_menu_x_len)
	   && mouse_y>items_menu_y && mouse_y<=items_menu_y+20)
		{
			view_my_items=0;

			return;
		}
	if(check_items_interface())return;

	if(view_self_stats && mouse_x>(attrib_menu_x+attrib_menu_x_len-20) && mouse_x<=(attrib_menu_x+attrib_menu_x_len)
	   && mouse_y>attrib_menu_y && mouse_y<=attrib_menu_y+20)
		{
			view_self_stats=0;
			return;
		}

	if(check_peace_menu())return;

	if(view_clock && mouse_x>window_width-132 && mouse_x<window_width-68
	   && mouse_y>window_height-64 && mouse_y<window_height)
		{
			unsigned char protocol_name;
			protocol_name=GET_TIME;
			my_tcp_send(my_socket,&protocol_name,1);
			return;
		}
	//check to see if we clicked on the compas
	if(view_compas && mouse_x>window_width-64 && mouse_x<window_width
	   && mouse_y>window_height-64 && mouse_y<window_height)
		{
			unsigned char protocol_name;

			protocol_name=LOCATE_ME;
			my_tcp_send(my_socket,&protocol_name,1);
			return;
		}

	//after we test for interface clicks
	//LOOK AT
	if(current_cursor==CURSOR_EYE && left_click)
		{
			Uint8 str[10];

			if(object_under_mouse==-1)return;

			if(thing_under_the_mouse==UNDER_MOUSE_PLAYER || thing_under_the_mouse==UNDER_MOUSE_NPC || thing_under_the_mouse==UNDER_MOUSE_ANIMAL)
				{
					str[0]=GET_PLAYER_INFO;
					*((int *)(str+1))=object_under_mouse;
					my_tcp_send(my_socket,str,5);
					return;
				}
			if(thing_under_the_mouse==UNDER_MOUSE_3D_OBJ)
				{
					str[0]=LOOK_AT_MAP_OBJECT;
					*((int *)(str+1))=object_under_mouse;
					my_tcp_send(my_socket,str,5);
					return;
				}
		}
	if(action_mode==action_look && right_click)
		{
			Uint8 str[10];

			if(object_under_mouse==-1)return;

			if(thing_under_the_mouse==UNDER_MOUSE_PLAYER || thing_under_the_mouse==UNDER_MOUSE_NPC || thing_under_the_mouse==UNDER_MOUSE_ANIMAL)
				{
					str[0]=GET_PLAYER_INFO;
					*((int *)(str+1))=object_under_mouse;
					my_tcp_send(my_socket,str,5);
					return;
				}
			if(thing_under_the_mouse==UNDER_MOUSE_3D_OBJ)
				{
					str[0]=LOOK_AT_MAP_OBJECT;
					*((int *)(str+1))=object_under_mouse;
					my_tcp_send(my_socket,str,5);
					return;
				}
		}
	//TRADE
	if((action_mode==action_trade && right_click) || (current_cursor==CURSOR_TRADE && left_click))
		{
			Uint8 str[10];

			if(object_under_mouse==-1)return;
			if(thing_under_the_mouse!=UNDER_MOUSE_PLAYER)return;
			str[0]=TRADE_WITH;
			*((int *)(str+1))=object_under_mouse;
			my_tcp_send(my_socket,str,5);
			return;
		}

	//ATTACK
	if(current_cursor==CURSOR_ATTACK && left_click)
		{
			Uint8 str[10];

			if(object_under_mouse==-1)return;
			if(thing_under_the_mouse==UNDER_MOUSE_PLAYER || thing_under_the_mouse==UNDER_MOUSE_NPC || thing_under_the_mouse==UNDER_MOUSE_ANIMAL)
				{
					str[0]=ATTACK_SOMEONE;
					*((int *)(str+1))=object_under_mouse;
					my_tcp_send(my_socket,str,5);
					return;
				}
		}
	if(action_mode==action_attack && right_click)
		{
			Uint8 str[10];

			if(object_under_mouse==-1)return;
			if(thing_under_the_mouse==UNDER_MOUSE_PLAYER || thing_under_the_mouse==UNDER_MOUSE_NPC || thing_under_the_mouse==UNDER_MOUSE_ANIMAL)
				{
					str[0]=ATTACK_SOMEONE;
					*((int *)(str+1))=object_under_mouse;
					my_tcp_send(my_socket,str,5);
					return;
				}

		}

	//USE
	if((current_cursor==CURSOR_TALK || current_cursor==CURSOR_ENTER) && left_click)
		{
			Uint8 str[10];

			if(object_under_mouse==-1)return;
			if(thing_under_the_mouse==UNDER_MOUSE_PLAYER || thing_under_the_mouse==UNDER_MOUSE_NPC || thing_under_the_mouse==UNDER_MOUSE_ANIMAL)
				{
					int i;
					str[0]=TOUCH_PLAYER;
					*((int *)(str+1))=object_under_mouse;
					my_tcp_send(my_socket,str,5);

					//clear the previous dialogue entries, so we won't have a left over from some othr NPC
					for(i=0;i<20;i++)dialogue_responces[i].in_use=0;
					return;
				}
			action_mode=action_walk;
			str[0]=USE_MAP_OBJECT;
			*((int *)(str+1))=object_under_mouse;
			*((int *)(str+5))=selected_inventory_object;
			my_tcp_send(my_socket,str,9);
			return;
		}
	if(action_mode==action_use && right_click)
		{
			Uint8 str[10];

			if(object_under_mouse==-1)return;
			if(thing_under_the_mouse==UNDER_MOUSE_PLAYER || thing_under_the_mouse==UNDER_MOUSE_NPC || thing_under_the_mouse==UNDER_MOUSE_ANIMAL)
				{
					int i;
					str[0]=TOUCH_PLAYER;
					*((int *)(str+1))=object_under_mouse;
					my_tcp_send(my_socket,str,5);

					//clear the previous dialogue entries, so we won't have a left over from some othr NPC
					for(i=0;i<20;i++)dialogue_responces[i].in_use=0;
					return;
				}
			action_mode=action_walk;
			str[0]=USE_MAP_OBJECT;
			*((int *)(str+1))=object_under_mouse;
			*((int *)(str+5))=selected_inventory_object;
			my_tcp_send(my_socket,str,9);
			return;
		}

	//OPEN BAG
	if(current_cursor==CURSOR_PICK && left_click)
		{
			if(object_under_mouse==-1)return;
			open_bag(object_under_mouse);
			action_mode=action_pick;
			return;
		}
	if(action_mode==action_pick && right_click)
		{
			if(object_under_mouse==-1)return;
			open_bag(object_under_mouse);
			return;
		}

	//HARVEST
	if(current_cursor==CURSOR_HARVEST && left_click)
		{
			Uint8 str[10];

			if(object_under_mouse==-1)return;
			str[0]=HARVEST;
			*((short *)(str+1))=object_under_mouse;
			my_tcp_send(my_socket,str,3);
			return;
		}
	if(action_mode==action_harvest && right_click)
		{
			Uint8 str[10];

			if(object_under_mouse==-1)return;
			str[0]=HARVEST;
			*((short *)(str+1))=object_under_mouse;
			my_tcp_send(my_socket,str,3);
			return;
		}


	if((action_mode==action_walk && right_click) || (current_cursor==CURSOR_WALK && left_click))
		{
			Uint8 str[10];
			short x,y;

			get_world_x_y();
			x=scene_mouse_x/0.5f;
			y=scene_mouse_y/0.5f;
			//check to see if the coordinates are OUTSIDE the map
			if(y<0 || x<0 || x>=tile_map_size_x*6 || y>=tile_map_size_y*6)return;

			str[0]=MOVE_TO;
			*((short *)(str+1))=x;
			*((short *)(str+3))=y;

			my_tcp_send(my_socket,str,5);

		}
	left_click=2;
	right_click=2;
}

void Enter2DMode()
{
	glPushAttrib(GL_LIGHTING_BIT|GL_DEPTH_BUFFER_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	glViewport(0, 0, window_width, window_height);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0, (GLdouble)window_width, (GLdouble)window_height, 0.0, -250.0, 250.0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
}

void Leave2DMode()
{
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopAttrib();
}


mode_flag video_modes[10];

void build_video_mode_array()
{
	int i;
	int flags;

	for(i=0;i<10;i++)
		{
			video_modes[i].selected=0;
			video_modes[i].supported=0;
		}
	video_modes[video_mode-1].selected=1;

	if(full_screen)flags=SDL_OPENGL|SDL_FULLSCREEN;
	else
		flags=SDL_OPENGL;

#ifdef WINDOWS
	if(bpp==16 || full_screen)
#else
		if(bpp==16)
#endif
			if(SDL_VideoModeOK(640, 480, 16, flags))video_modes[0].supported=1;

#ifdef WINDOWS
	if(bpp==32 || full_screen)
#else
		if(bpp==32)
#endif
			if(SDL_VideoModeOK(640, 480, 32, flags))video_modes[1].supported=1;

#ifdef WINDOWS
	if(bpp==16 || full_screen)
#else
		if(bpp==16)
#endif
			if(SDL_VideoModeOK(800, 600, 16, flags))video_modes[2].supported=1;

#ifdef WINDOWS
	if(bpp==32 || full_screen)
#else
		if(bpp==32)
#endif
			if(SDL_VideoModeOK(800, 600, 32, flags))video_modes[3].supported=1;

#ifdef WINDOWS
	if(bpp==16 || full_screen)
#else
		if(bpp==16)
#endif
			if(SDL_VideoModeOK(1024, 768, 16, flags))video_modes[4].supported=1;

#ifdef WINDOWS
	if(bpp==32 || full_screen)
#else
		if(bpp==32)
#endif
			if(SDL_VideoModeOK(1024, 768, 32, flags))video_modes[5].supported=1;

#ifdef WINDOWS
	if(bpp==16 || full_screen)
#else
		if(bpp==16)
#endif
			if(SDL_VideoModeOK(1152, 864, 16, flags))video_modes[6].supported=1;

#ifdef WINDOWS
	if(bpp==32 || full_screen)
#else
		if(bpp==32)
#endif
			if(SDL_VideoModeOK(1152, 864, 32, flags))video_modes[7].supported=1;

#ifdef WINDOWS
	if(bpp==16 || full_screen)
#else
		if(bpp==16)
#endif
			if(SDL_VideoModeOK(1280, 1024, 16, flags))video_modes[8].supported=1;

#ifdef WINDOWS
	if(bpp==32 || full_screen)
#else
		if(bpp==32)
#endif
			if(SDL_VideoModeOK(1280, 1024, 32, flags))video_modes[9].supported=1;
}

void draw_console_pic(int which_texture)
{
	if(last_texture!=texture_cache[which_texture].texture_id)
		{
			glBindTexture(GL_TEXTURE_2D, texture_cache[which_texture].texture_id);
			last_texture=texture_cache[which_texture].texture_id;
		}
	glColor3f(1.0f,1.0f,1.0f);
	glBegin(GL_QUADS);
	//draw the texture

	glTexCoord2f(0.0f,1.0f);
	glVertex3i(0,0,0);

	glTexCoord2f(0.0f,0.0f);
	glVertex3i(0,window_height,0);

	glTexCoord2f(1.0f,0.0f);
	glVertex3i(window_width,window_height,0);

	glTexCoord2f(1.0f,1.0f);
	glVertex3i(window_width,0,0);

	glEnd();

}


void draw_2d_thing(float u_start,float v_start,float u_end,float v_end,int x_start,
				   int y_start,int x_end,int y_end)
{

	glTexCoord2f(u_start,v_end);
	glVertex3i(x_start,y_end,0);

	glTexCoord2f(u_start,v_start);
	glVertex3i(x_start,y_start,0);

	glTexCoord2f(u_end,v_start);
	glVertex3i(x_end,y_start,0);

	glTexCoord2f(u_end,v_end);
	glVertex3i(x_end,y_end,0);

}


void init_opening_interface()
{
	login_screen_menus=load_texture_cache("./textures/login_menu.bmp",0);
	login_text=load_texture_cache("./textures/login_back.bmp",255);

}


void draw_login_screen()
{

	float selected_bar_u_start=(float)0/255;
	float selected_bar_v_start=1.0f-(float)0/255;

	float selected_bar_u_end=(float)174/255;
	float selected_bar_v_end=1.0f-(float)28/255;


	float unselected_bar_u_start=(float)0/255;
	float unselected_bar_v_start=1.0f-(float)40/255;

	float unselected_bar_u_end=(float)170/255;
	float unselected_bar_v_end=1.0f-(float)63/255;
	/////////////////////////
	float log_in_unselected_start_u=(float)0/255;
	float log_in_unselected_start_v=1.0f-(float)80/255;

	float log_in_unselected_end_u=(float)87/255;
	float log_in_unselected_end_v=1.0f-(float)115/255;

	float log_in_selected_start_u=(float)0/255;
	float log_in_selected_start_v=1.0f-(float)120/255;

	float log_in_selected_end_u=(float)87/255;
	float log_in_selected_end_v=1.0f-(float)155/255;
	/////////////////////////
	float new_char_unselected_start_u=(float)100/255;
	float new_char_unselected_start_v=1.0f-(float)80/255;

	float new_char_unselected_end_u=(float)238/255;
	float new_char_unselected_end_v=1.0f-(float)115/255;

	float new_char_selected_start_u=(float)100/255;
	float new_char_selected_start_v=1.0f-(float)120/255;

	float new_char_selected_end_u=(float)238/255;
	float new_char_selected_end_v=1.0f-(float)155/255;

	/////////////////////////////////////////////
	/////////////////////////////////////////////
	int half_screen_x=window_width/2;
	int half_screen_y=window_height/2;
	int username_text_x=half_screen_x-125;
	int username_text_y=half_screen_y-130;
	int password_text_x=half_screen_x-125;
	int password_text_y=half_screen_y-100;
	int username_bar_x=username_text_x+100;
	int username_bar_y=username_text_y-7;
	int username_bar_x_len=174;
	int username_bar_y_len=28;
	int password_bar_x=password_text_x+100;
	int password_bar_y=password_text_y-7;
	int password_bar_x_len=174;
	int password_bar_y_len=28;
	int log_in_x=half_screen_x-125;
	int log_in_y=half_screen_y-50;
	int log_in_x_len=87;
	int log_in_y_len=35;
	int new_char_x=half_screen_x+50;
	int new_char_y=half_screen_y-50;
	int new_char_x_len=138;
	int new_char_y_len=35;

	char log_in_button_selected=0;
	char new_char_button_selected=0;

	draw_console_pic(login_text);

	//check to see if the log in button is active, or not
	if(mouse_x>=log_in_x && mouse_x<=log_in_x+log_in_x_len && mouse_y>=log_in_y &&
	   mouse_y<=log_in_y+log_in_y_len && username_str[0] && password_str[0])
		log_in_button_selected=1;
	else
		log_in_button_selected=0;


	//check to see if the new char button is active, or not
	if(mouse_x>=new_char_x && mouse_x<=new_char_x+new_char_x_len && mouse_y>=new_char_y &&
	   mouse_y<=new_char_y+new_char_y_len)
		new_char_button_selected=1;
	else
		new_char_button_selected=0;

	//check to see if we clicked on the username/password box
	if(mouse_x>=username_bar_x && mouse_x<=username_bar_x+username_bar_x_len
	   && mouse_y>=username_bar_y && mouse_y<=username_bar_y+username_bar_y_len && left_click==1)
		{
			username_box_selected=1;
			password_box_selected=0;
		}

	if(mouse_x>=password_bar_x && mouse_x<=password_bar_x+password_bar_x_len
	   && mouse_y>=password_bar_y && mouse_y<=password_bar_y+password_bar_y_len && left_click==1)
		{
			username_box_selected=0;
			password_box_selected=1;
		}


	//check to see if we clicked on the ACTIVE Log In button
	if(log_in_button_selected && left_click==1)
		{
			send_login_info();
			left_click=2;//don't relogin 100 times like a moron
		}

	//check to see if we clicked on the ACTIVE New Char button
	if(new_char_button_selected && left_click==1)interface_mode=interface_new_char;

	//ok, start drawing the interface...
	draw_string(username_text_x,username_text_y,"Username: ",1);
	draw_string(password_text_x,password_text_y,"Password: ",1);
	//start drawing the actual interface pieces
	if(last_texture!=texture_cache[login_screen_menus].texture_id)
		{
			glBindTexture(GL_TEXTURE_2D, texture_cache[login_screen_menus].texture_id);
			last_texture=texture_cache[login_screen_menus].texture_id;
		}

	glColor3f(1.0f,1.0f,1.0f);
	glBegin(GL_QUADS);

	//username box
	if(username_box_selected)
		draw_2d_thing(selected_bar_u_start,selected_bar_v_start,
					  selected_bar_u_end,selected_bar_v_end,username_bar_x,
					  username_bar_y,username_bar_x+username_bar_x_len,username_bar_y+username_bar_y_len);
	else
		draw_2d_thing(unselected_bar_u_start,unselected_bar_v_start,
					  unselected_bar_u_end,unselected_bar_v_end,username_bar_x,
					  username_bar_y,username_bar_x+username_bar_x_len,username_bar_y+username_bar_y_len);

	//password box
	if(password_box_selected)
		draw_2d_thing(selected_bar_u_start,selected_bar_v_start,
					  selected_bar_u_end,selected_bar_v_end,password_bar_x,
					  password_bar_y,password_bar_x+password_bar_x_len,password_bar_y+password_bar_y_len);
	else
		draw_2d_thing(unselected_bar_u_start,unselected_bar_v_start,
					  unselected_bar_u_end,unselected_bar_v_end,password_bar_x,
					  password_bar_y,password_bar_x+password_bar_x_len,password_bar_y+password_bar_y_len);

	//log in button
	if(log_in_button_selected)
		draw_2d_thing(log_in_selected_start_u,log_in_selected_start_v,
					  log_in_selected_end_u,log_in_selected_end_v,log_in_x,
					  log_in_y,log_in_x+log_in_x_len,log_in_y+log_in_y_len);
	else
		draw_2d_thing(log_in_unselected_start_u,log_in_unselected_start_v,
					  log_in_unselected_end_u,log_in_unselected_end_v,log_in_x,
					  log_in_y,log_in_x+log_in_x_len,log_in_y+log_in_y_len);

	//new char button
	if(new_char_button_selected)
		draw_2d_thing(new_char_selected_start_u,new_char_selected_start_v,
					  new_char_selected_end_u,new_char_selected_end_v,new_char_x,
					  new_char_y,new_char_x+new_char_x_len,new_char_y+new_char_y_len);
	else
		draw_2d_thing(new_char_unselected_start_u,new_char_unselected_start_v,
					  new_char_unselected_end_u,new_char_unselected_end_v,new_char_x,
					  new_char_y,new_char_x+new_char_x_len,new_char_y+new_char_y_len);

	glEnd();

	glColor3f(0.0f,0.9f,1.0f);
	draw_string(username_bar_x+4,username_text_y,username_str,1);
	draw_string(password_bar_x+4,password_text_y,display_password_str,1);
	glColor3f(1.0f,1.0f,1.0f);

	glColor3f(1.0f,0.0f,0.0f);
	//print the current error, if any
	draw_string(0,log_in_y+40,log_in_error_str,5);
}

void add_char_to_username(unsigned char ch)
{
	if(((ch>=48 && ch<=57) || (ch>=65 && ch<=90) || (ch>=97 && ch<=122) || (ch=='_')) && username_text_lenght<15)
		{
			username_str[username_text_lenght]=ch;
			username_str[username_text_lenght+1]=0;
			username_text_lenght++;
		}
	if(ch==SDLK_BACKSPACE && username_text_lenght>0)
		{
			username_text_lenght--;
			username_str[username_text_lenght]=0;
		}
	if(ch==SDLK_TAB)
		{
			username_box_selected=0;
			password_box_selected=1;
		}
}

void add_char_to_password(unsigned char ch)
{
	if((ch>=32 && ch<=126) && password_text_lenght<15)
		{
			password_str[password_text_lenght]=ch;
			display_password_str[password_text_lenght]='*';
			password_str[password_text_lenght+1]=0;
			display_password_str[password_text_lenght+1]=0;
			password_text_lenght++;
		}
	if(ch==SDLK_BACKSPACE && password_text_lenght>0)
		{
			password_text_lenght--;
			display_password_str[password_text_lenght]=0;
			password_str[password_text_lenght]=0;
		}

	if(ch==SDLK_TAB)
		{
			username_box_selected=1;
			password_box_selected=0;
		}
}


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

void init_peace_icons_position()
{
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

	options_icon_x_start=stats_icon_x_end+1;
	options_icon_x_end=options_icon_x_start+32;
	options_icon_y_start=window_height-32;
	options_icon_y_end=options_icon_y_start+32;
}

void draw_peace_icons()
{

	if(last_texture!=texture_cache[icons_text].texture_id)
		{
			glBindTexture(GL_TEXTURE_2D, texture_cache[icons_text].texture_id);
			last_texture=texture_cache[icons_text].texture_id;
		}

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

int check_peace_menu()
{
	if(combat_mode)return 0;
	if(mouse_x<walk_icon_x_start || mouse_y<walk_icon_y_start ||
	   mouse_x>options_icon_x_end || mouse_y>options_icon_y_end)return 0;

	if(mouse_x>options_icon_x_start && mouse_y>options_icon_y_start &&
	   mouse_x<options_icon_x_end && mouse_y<options_icon_y_end)
		options_menu=!options_menu;
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
					//Uint8 str[100];
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


float lit_gem_u_start=(float)224/255;
float lit_gem_v_start=1.0f-(float)112/255;
float lit_gem_u_end=(float)255/255;
float lit_gem_v_end=1.0f-(float)128/255;

float broken_gem_u_start=(float)192/255;
float broken_gem_v_start=1.0f-(float)112/255;
float broken_gem_u_end=(float)223/255;
float broken_gem_v_end=1.0f-(float)128/255;

float unlit_gem_u_start=(float)224/255;
float unlit_gem_v_start=1.0f-(float)96/255;
float unlit_gem_u_end=(float)255/255;
float unlit_gem_v_end=1.0f-(float)111/255;


void draw_options_menu()
{

	draw_menu_title_bar(options_menu_x,options_menu_y-16,options_menu_x + options_menu_x_len-options_menu_x);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE,GL_SRC_ALPHA);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glColor4f(0.0f,0.0f,0.0f,0.5f);
	glVertex3i(options_menu_x,options_menu_y + options_menu_y_len,0);
	glVertex3i(options_menu_x,options_menu_y,0);
	glVertex3i(options_menu_x + options_menu_x_len,options_menu_y,0);
	glVertex3i(options_menu_x + options_menu_x_len,options_menu_y + options_menu_y_len,0);
	glEnd();

	glDisable(GL_BLEND);

	glColor3f(0.0f,1.0f,0.0f);
	glBegin(GL_LINES);
	glVertex3i(options_menu_x,options_menu_y,0);
	glVertex3i(options_menu_x + options_menu_x_len,options_menu_y,0);
	glVertex3i(options_menu_x + options_menu_x_len,options_menu_y,0);
	glVertex3i(options_menu_x + options_menu_x_len,options_menu_y + options_menu_y_len,0);
	glVertex3i(options_menu_x + options_menu_x_len,options_menu_y + options_menu_y_len,0);
	glVertex3i(options_menu_x,options_menu_y + options_menu_y_len,0);
	glVertex3i(options_menu_x,options_menu_y + options_menu_y_len,0);
	glVertex3i(options_menu_x,options_menu_y,0);

	//draw the corner, with the X in
	glVertex3i(options_menu_x + options_menu_x_len,options_menu_y+20,0);
	glVertex3i(options_menu_x + options_menu_x_len-20,options_menu_y+20,0);

	glVertex3i(options_menu_x + options_menu_x_len-20,options_menu_y+20,0);
	glVertex3i(options_menu_x + options_menu_x_len-20,options_menu_y,0);

	glEnd();


	glEnable(GL_TEXTURE_2D);

	draw_string_small(options_menu_x + options_menu_x_len-16,options_menu_y+2,"X",1);

	glColor3f(1.0f,1.0f,1.0f);

	if(last_texture!=texture_cache[icons_text].texture_id)
		{
			glBindTexture(GL_TEXTURE_2D, texture_cache[icons_text].texture_id);
			last_texture=texture_cache[icons_text].texture_id;
		}


	glBegin(GL_QUADS);
	if(!have_stencil)
		draw_2d_thing(broken_gem_u_start, broken_gem_v_start, broken_gem_u_end, broken_gem_v_end,
					  options_menu_x+8, options_menu_y+35, options_menu_x+38, options_menu_y+51);
	else if(shadows_on)
		draw_2d_thing(lit_gem_u_start, lit_gem_v_start, lit_gem_u_end, lit_gem_v_end,
					  options_menu_x+8, options_menu_y+35, options_menu_x+38, options_menu_y+51);
	else
		draw_2d_thing(unlit_gem_u_start, unlit_gem_v_start, unlit_gem_u_end, unlit_gem_v_end,
					  options_menu_x+8, options_menu_y+35, options_menu_x+38, options_menu_y+51);

	if(!have_multitexture)
		draw_2d_thing(broken_gem_u_start, broken_gem_v_start, broken_gem_u_end, broken_gem_v_end,
					  options_menu_x+8, options_menu_y+55, options_menu_x+38, options_menu_y+71);
	else if(clouds_shadows)
		draw_2d_thing(lit_gem_u_start, lit_gem_v_start, lit_gem_u_end, lit_gem_v_end,
					  options_menu_x+8, options_menu_y+55, options_menu_x+38, options_menu_y+71);
	else
		draw_2d_thing(unlit_gem_u_start, unlit_gem_v_start, unlit_gem_u_end, unlit_gem_v_end,
					  options_menu_x+8, options_menu_y+55, options_menu_x+38, options_menu_y+71);
	if(show_reflection)
		draw_2d_thing(lit_gem_u_start, lit_gem_v_start, lit_gem_u_end, lit_gem_v_end,
					  options_menu_x+8, options_menu_y+75, options_menu_x+38, options_menu_y+91);
	else
		draw_2d_thing(unlit_gem_u_start, unlit_gem_v_start, unlit_gem_u_end, unlit_gem_v_end,
					  options_menu_x+8, options_menu_y+75, options_menu_x+38, options_menu_y+91);
	if(show_fps)
		draw_2d_thing(lit_gem_u_start, lit_gem_v_start, lit_gem_u_end, lit_gem_v_end,
					  options_menu_x+8, options_menu_y+95, options_menu_x+38, options_menu_y+111);
	else
		draw_2d_thing(unlit_gem_u_start, unlit_gem_v_start, unlit_gem_u_end, unlit_gem_v_end,
					  options_menu_x+8, options_menu_y+95, options_menu_x+38, options_menu_y+111);

	if(view_compas)
		draw_2d_thing(lit_gem_u_start, lit_gem_v_start, lit_gem_u_end, lit_gem_v_end,
					  options_menu_x+8, options_menu_y+115, options_menu_x+38, options_menu_y+131);
	else
		draw_2d_thing(unlit_gem_u_start, unlit_gem_v_start, unlit_gem_u_end, unlit_gem_v_end,
					  options_menu_x+8, options_menu_y+115, options_menu_x+38, options_menu_y+131);

	if(view_clock)
		draw_2d_thing(lit_gem_u_start, lit_gem_v_start, lit_gem_u_end, lit_gem_v_end,
					  options_menu_x+8, options_menu_y+135, options_menu_x+38, options_menu_y+151);
	else
		draw_2d_thing(unlit_gem_u_start, unlit_gem_v_start, unlit_gem_u_end, unlit_gem_v_end,
					  options_menu_x+8, options_menu_y+135, options_menu_x+38, options_menu_y+151);

	if(!have_sound)
		draw_2d_thing(broken_gem_u_start, broken_gem_v_start, broken_gem_u_end, broken_gem_v_end,
					  options_menu_x+8, options_menu_y+155, options_menu_x+38, options_menu_y+171);
	else if(sound_on)
		draw_2d_thing(lit_gem_u_start, lit_gem_v_start, lit_gem_u_end, lit_gem_v_end,
					  options_menu_x+8, options_menu_y+155, options_menu_x+38, options_menu_y+171);
	else
		draw_2d_thing(unlit_gem_u_start, unlit_gem_v_start, unlit_gem_u_end, unlit_gem_v_end,
					  options_menu_x+8, options_menu_y+155, options_menu_x+38, options_menu_y+171);

	if(!have_music)
		draw_2d_thing(broken_gem_u_start, broken_gem_v_start, broken_gem_u_end, broken_gem_v_end,
					  options_menu_x+8, options_menu_y+175, options_menu_x+38, options_menu_y+191);
	else if(music_on)
		draw_2d_thing(lit_gem_u_start, lit_gem_v_start, lit_gem_u_end, lit_gem_v_end,
					  options_menu_x+8, options_menu_y+175, options_menu_x+38, options_menu_y+191);
	else
		draw_2d_thing(unlit_gem_u_start, unlit_gem_v_start, unlit_gem_u_end, unlit_gem_v_end,
					  options_menu_x+8, options_menu_y+175, options_menu_x+38, options_menu_y+191);

	if(auto_camera)
		draw_2d_thing(lit_gem_u_start, lit_gem_v_start, lit_gem_u_end, lit_gem_v_end,
					  options_menu_x+8, options_menu_y+195, options_menu_x+38, options_menu_y+211);
	else
		draw_2d_thing(unlit_gem_u_start, unlit_gem_v_start, unlit_gem_u_end, unlit_gem_v_end,
					  options_menu_x+8, options_menu_y+195, options_menu_x+38, options_menu_y+211);


	//video modes

	if(full_screen)
		draw_2d_thing(lit_gem_u_start, lit_gem_v_start, lit_gem_u_end, lit_gem_v_end,
					  options_menu_x+193, options_menu_y+35, options_menu_x+220, options_menu_y+51);
	else
		draw_2d_thing(unlit_gem_u_start, unlit_gem_v_start, unlit_gem_u_end, unlit_gem_v_end,
					  options_menu_x+193, options_menu_y+35, options_menu_x+220, options_menu_y+51);

	if(video_modes[0].selected)
		draw_2d_thing(lit_gem_u_start, lit_gem_v_start, lit_gem_u_end, lit_gem_v_end,
					  options_menu_x+193, options_menu_y+55, options_menu_x+220, options_menu_y+71);
	else if(video_modes[0].supported)
		draw_2d_thing(unlit_gem_u_start, unlit_gem_v_start, unlit_gem_u_end, unlit_gem_v_end,
					  options_menu_x+193, options_menu_y+55, options_menu_x+220, options_menu_y+71);
	else
		draw_2d_thing(broken_gem_u_start, broken_gem_v_start, broken_gem_u_end, broken_gem_v_end,
					  options_menu_x+193, options_menu_y+55, options_menu_x+220, options_menu_y+71);

	if(video_modes[1].selected)
		draw_2d_thing(lit_gem_u_start, lit_gem_v_start, lit_gem_u_end, lit_gem_v_end,
					  options_menu_x+193, options_menu_y+75, options_menu_x+220, options_menu_y+91);
	else if(video_modes[1].supported)
		draw_2d_thing(unlit_gem_u_start, unlit_gem_v_start, unlit_gem_u_end, unlit_gem_v_end,
					  options_menu_x+193, options_menu_y+75, options_menu_x+220, options_menu_y+91);
	else
		draw_2d_thing(broken_gem_u_start, broken_gem_v_start, broken_gem_u_end, broken_gem_v_end,
					  options_menu_x+193, options_menu_y+75, options_menu_x+220, options_menu_y+91);

	if(video_modes[2].selected)
		draw_2d_thing(lit_gem_u_start, lit_gem_v_start, lit_gem_u_end, lit_gem_v_end,
					  options_menu_x+193, options_menu_y+95, options_menu_x+220, options_menu_y+111);
	else if(video_modes[2].supported)
		draw_2d_thing(unlit_gem_u_start, unlit_gem_v_start, unlit_gem_u_end, unlit_gem_v_end,
					  options_menu_x+193, options_menu_y+95, options_menu_x+220, options_menu_y+111);
	else
		draw_2d_thing(broken_gem_u_start, broken_gem_v_start, broken_gem_u_end, broken_gem_v_end,
					  options_menu_x+193, options_menu_y+95, options_menu_x+220, options_menu_y+111);

	if(video_modes[3].selected)
		draw_2d_thing(lit_gem_u_start, lit_gem_v_start, lit_gem_u_end, lit_gem_v_end,
					  options_menu_x+193, options_menu_y+115, options_menu_x+220, options_menu_y+131);
	else if(video_modes[3].supported)
		draw_2d_thing(unlit_gem_u_start, unlit_gem_v_start, unlit_gem_u_end, unlit_gem_v_end,
					  options_menu_x+193, options_menu_y+115, options_menu_x+220, options_menu_y+131);
	else
		draw_2d_thing(broken_gem_u_start, broken_gem_v_start, broken_gem_u_end, broken_gem_v_end,
					  options_menu_x+193, options_menu_y+115, options_menu_x+220, options_menu_y+131);

	if(video_modes[4].selected)
		draw_2d_thing(lit_gem_u_start, lit_gem_v_start, lit_gem_u_end, lit_gem_v_end,
					  options_menu_x+193, options_menu_y+135, options_menu_x+220, options_menu_y+151);
	else if(video_modes[4].supported)
		draw_2d_thing(unlit_gem_u_start, unlit_gem_v_start, unlit_gem_u_end, unlit_gem_v_end,
					  options_menu_x+193, options_menu_y+135, options_menu_x+220, options_menu_y+151);
	else
		draw_2d_thing(broken_gem_u_start, broken_gem_v_start, broken_gem_u_end, broken_gem_v_end,
					  options_menu_x+193, options_menu_y+135, options_menu_x+220, options_menu_y+151);

	if(video_modes[5].selected)
		draw_2d_thing(lit_gem_u_start, lit_gem_v_start, lit_gem_u_end, lit_gem_v_end,
					  options_menu_x+193, options_menu_y+155, options_menu_x+220, options_menu_y+171);
	else if(video_modes[5].supported)
		draw_2d_thing(unlit_gem_u_start, unlit_gem_v_start, unlit_gem_u_end, unlit_gem_v_end,
					  options_menu_x+193, options_menu_y+155, options_menu_x+220, options_menu_y+171);
	else
		draw_2d_thing(broken_gem_u_start, broken_gem_v_start, broken_gem_u_end, broken_gem_v_end,
					  options_menu_x+193, options_menu_y+155, options_menu_x+220, options_menu_y+171);

	if(video_modes[6].selected)
		draw_2d_thing(lit_gem_u_start, lit_gem_v_start, lit_gem_u_end, lit_gem_v_end,
					  options_menu_x+193, options_menu_y+175, options_menu_x+220, options_menu_y+191);
	else if(video_modes[6].supported)
		draw_2d_thing(unlit_gem_u_start, unlit_gem_v_start, unlit_gem_u_end, unlit_gem_v_end,
					  options_menu_x+193, options_menu_y+175, options_menu_x+220, options_menu_y+191);
	else
		draw_2d_thing(broken_gem_u_start, broken_gem_v_start, broken_gem_u_end, broken_gem_v_end,
					  options_menu_x+193, options_menu_y+175, options_menu_x+220, options_menu_y+191);

	if(video_modes[7].selected)
		draw_2d_thing(lit_gem_u_start, lit_gem_v_start, lit_gem_u_end, lit_gem_v_end,
					  options_menu_x+193, options_menu_y+195, options_menu_x+220, options_menu_y+211);
	else if(video_modes[7].supported)
		draw_2d_thing(unlit_gem_u_start, unlit_gem_v_start, unlit_gem_u_end, unlit_gem_v_end,
					  options_menu_x+193, options_menu_y+195, options_menu_x+220, options_menu_y+211);
	else
		draw_2d_thing(broken_gem_u_start, broken_gem_v_start, broken_gem_u_end, broken_gem_v_end,
					  options_menu_x+193, options_menu_y+195, options_menu_x+220, options_menu_y+211);

	if(video_modes[8].selected)
		draw_2d_thing(lit_gem_u_start, lit_gem_v_start, lit_gem_u_end, lit_gem_v_end,
					  options_menu_x+193, options_menu_y+215, options_menu_x+220, options_menu_y+231);
	else if(video_modes[8].supported)
		draw_2d_thing(unlit_gem_u_start, unlit_gem_v_start, unlit_gem_u_end, unlit_gem_v_end,
					  options_menu_x+193, options_menu_y+215, options_menu_x+220, options_menu_y+231);
	else
		draw_2d_thing(broken_gem_u_start, broken_gem_v_start, broken_gem_u_end, broken_gem_v_end,
					  options_menu_x+193, options_menu_y+215, options_menu_x+220, options_menu_y+231);

	if(video_modes[9].selected)
		draw_2d_thing(lit_gem_u_start, lit_gem_v_start, lit_gem_u_end, lit_gem_v_end,
					  options_menu_x+193, options_menu_y+235, options_menu_x+220, options_menu_y+251);
	else if(video_modes[9].supported)
		draw_2d_thing(unlit_gem_u_start, unlit_gem_v_start, unlit_gem_u_end, unlit_gem_v_end,
					  options_menu_x+193, options_menu_y+235, options_menu_x+220, options_menu_y+251);
	else
		draw_2d_thing(broken_gem_u_start, broken_gem_v_start, broken_gem_u_end, broken_gem_v_end,
					  options_menu_x+193, options_menu_y+235, options_menu_x+220, options_menu_y+251);


	glEnd();
	draw_string(options_menu_x+55,options_menu_y+10,"Options",1);
	draw_string(options_menu_x+45,options_menu_y+35,"Shadows",1);
	draw_string(options_menu_x+45,options_menu_y+55,"Clouds",1);
	draw_string(options_menu_x+45,options_menu_y+75,"Reflections",1);
	draw_string(options_menu_x+45,options_menu_y+95,"Show FPS",1);
	draw_string(options_menu_x+45,options_menu_y+115,"Compass",1);
	draw_string(options_menu_x+45,options_menu_y+135,"Clock",1);
	draw_string(options_menu_x+45,options_menu_y+155,"Sound",1);
	draw_string(options_menu_x+45,options_menu_y+175,"Music",1);
	draw_string(options_menu_x+45,options_menu_y+195,"Auto Camera",1);

	draw_string(options_menu_x+225,options_menu_y+10,"Video Modes",1);
	draw_string(options_menu_x+225,options_menu_y+35,"Full Screen",1);
	draw_string(options_menu_x+225,options_menu_y+55,"640x480x16",1);
	draw_string(options_menu_x+225,options_menu_y+75,"640x480x32",1);
	draw_string(options_menu_x+225,options_menu_y+95,"800x600x16",1);
	draw_string(options_menu_x+225,options_menu_y+115,"800x600x32",1);
	draw_string(options_menu_x+225,options_menu_y+135,"1024x768x16",1);
	draw_string(options_menu_x+225,options_menu_y+155,"1024x768x32",1);
	draw_string(options_menu_x+225,options_menu_y+175,"1152x864x16",1);
	draw_string(options_menu_x+225,options_menu_y+195,"1152x864x32",1);
	draw_string(options_menu_x+225,options_menu_y+215,"1280x1024x16",1);
	draw_string(options_menu_x+225,options_menu_y+235,"1280x1024x32",1);
}


int check_options_menu()
{
	if(!options_menu)return 0;
	if(mouse_x<options_menu_x || mouse_y<options_menu_y ||
	   mouse_x>options_menu_x + options_menu_x_len || mouse_y>options_menu_y + options_menu_y_len)return 0;

	if(mouse_x>options_menu_x + options_menu_x_len-16 && mouse_y>options_menu_y &&
	   mouse_x<options_menu_x + options_menu_x_len && mouse_y<options_menu_y+16)
		options_menu=0;
	else if(mouse_x>options_menu_x+8 && mouse_y>options_menu_y+35 &&
			mouse_x<options_menu_x+38 && mouse_y<options_menu_y+51)
		shadows_on=!shadows_on;
	else if(mouse_x>options_menu_x+8 && mouse_y>options_menu_y+55 &&
			mouse_x<options_menu_x+38 && mouse_y<options_menu_y+71)
		clouds_shadows=!clouds_shadows;
	else if(mouse_x>options_menu_x+8 && mouse_y>options_menu_y+75 &&
			mouse_x<options_menu_x+38 && mouse_y<options_menu_y+91)
		show_reflection=!show_reflection;
	else if(mouse_x>options_menu_x+8 && mouse_y>options_menu_y+95 &&
			mouse_x<options_menu_x+38 && mouse_y<options_menu_y+111)
		show_fps=!show_fps;
	else if(mouse_x>options_menu_x+8 && mouse_y>options_menu_y+115 &&
			mouse_x<options_menu_x+38 && mouse_y<options_menu_y+131)
		view_compas=!view_compas;
	else if(mouse_x>options_menu_x+8 && mouse_y>options_menu_y+135 &&
			mouse_x<options_menu_x+38 && mouse_y<options_menu_y+151)
		view_clock=!view_clock;
	else if(mouse_x>options_menu_x+8 && mouse_y>options_menu_y+155 &&
			mouse_x<options_menu_x+38 && mouse_y<options_menu_y+171)
		if(sound_on)turn_sound_off();
		else turn_sound_on();
	else if(mouse_x>options_menu_x+8 && mouse_y>options_menu_y+175 &&
			mouse_x<options_menu_x+38 && mouse_y<options_menu_y+191)
		music_on=!music_on;
	else if(mouse_x>options_menu_x+8 && mouse_y>options_menu_y+195 &&
			mouse_x<options_menu_x+38 && mouse_y<options_menu_y+211)
		auto_camera=!auto_camera;

	else if(mouse_x>options_menu_x+193 && mouse_y>options_menu_y+35 &&
			mouse_x<options_menu_x+220 && mouse_y<options_menu_y+51)
		toggle_full_screen();
	else if(mouse_x>options_menu_x+193 && mouse_y>options_menu_y+55 &&
			mouse_x<options_menu_x+220 && mouse_y<options_menu_y+71)
		{
			if(video_modes[0].supported && !video_modes[0].selected)
				set_new_video_mode(full_screen,1);
		}
	else if(mouse_x>options_menu_x+193 && mouse_y>options_menu_y+75 &&
			mouse_x<options_menu_x+220 && mouse_y<options_menu_y+91)
		{
			if(video_modes[1].supported && !video_modes[1].selected)
				set_new_video_mode(full_screen,2);
		}
	else if(mouse_x>options_menu_x+193 && mouse_y>options_menu_y+95 &&
			mouse_x<options_menu_x+220 && mouse_y<options_menu_y+111)
		{
			if(video_modes[2].supported && !video_modes[2].selected)
				set_new_video_mode(full_screen,3);
		}
	else if(mouse_x>options_menu_x+193 && mouse_y>options_menu_y+115 &&
			mouse_x<options_menu_x+220 && mouse_y<options_menu_y+131)
		{
			if(video_modes[3].supported && !video_modes[3].selected)
				set_new_video_mode(full_screen,4);
		}
	else if(mouse_x>options_menu_x+193 && mouse_y>options_menu_y+135 &&
			mouse_x<options_menu_x+220 && mouse_y<options_menu_y+151)
		{
			if(video_modes[4].supported && !video_modes[4].selected)
				set_new_video_mode(full_screen,5);
		}
	else if(mouse_x>options_menu_x+193 && mouse_y>options_menu_y+155 &&
			mouse_x<options_menu_x+220 && mouse_y<options_menu_y+171)
		{
			if(video_modes[5].supported && !video_modes[5].selected)
				set_new_video_mode(full_screen,6);
		}
	else if(mouse_x>options_menu_x+193 && mouse_y>options_menu_y+175 &&
			mouse_x<options_menu_x+220 && mouse_y<options_menu_y+191)
		{
			if(video_modes[6].supported && !video_modes[6].selected)
				set_new_video_mode(full_screen,7);
		}
	else if(mouse_x>options_menu_x+193 && mouse_y>options_menu_y+195 &&
			mouse_x<options_menu_x+220 && mouse_y<options_menu_y+211)
		{
			if(video_modes[7].supported && !video_modes[7].selected)
				set_new_video_mode(full_screen,8);
		}
	else if(mouse_x>options_menu_x+193 && mouse_y>options_menu_y+215 &&
			mouse_x<options_menu_x+220 && mouse_y<options_menu_y+231)
		{
			if(video_modes[8].supported && !video_modes[8].selected)
				set_new_video_mode(full_screen,9);
		}
	else if(mouse_x>options_menu_x+193 && mouse_y>options_menu_y+235 &&
			mouse_x<options_menu_x+220 && mouse_y<options_menu_y+251)
		{
			if(video_modes[9].supported && !video_modes[9].selected)
				set_new_video_mode(full_screen,10);
		}

	return 1;
}

void draw_ingame_interface()
{

	float compas_u_start=(float)34/255;
	float compas_v_start=1.0f-(float)194/255;

	float compas_u_end=(float)96/255;
	float compas_v_end=1.0f-(float)255/255;

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

	check_menus_out_of_screen();

    glEnable(GL_ALPHA_TEST);//enable alpha filtering, so we have some alpha key
    glAlphaFunc(GL_GREATER,0.001f);

	if(last_texture!=texture_cache[icons_text].texture_id)
		{
			glBindTexture(GL_TEXTURE_2D, texture_cache[icons_text].texture_id);
			last_texture=texture_cache[icons_text].texture_id;
		}

	glColor3f(1.0f,1.0f,1.0f);

	if(view_compas)
		{
			glTranslatef(window_width-32, window_height-30, 0);
			glRotatef(-rz, 0.0f, 0.0f, 1.0f);

			glBegin(GL_QUADS);
			draw_2d_thing(compas_u_start, compas_v_start, compas_u_end, compas_v_end, -32, -30,32, 30);
			glEnd();

			//draw the compas needle
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

    draw_peace_icons();
    display_spells_we_have();

    if(have_dialogue)
    	{
    		display_dialogue();
    		if(mouse_x>=dialogue_menu_x && mouse_x<=dialogue_menu_x+dialogue_menu_x_len
			   && mouse_y>=dialogue_menu_y && mouse_y<=dialogue_menu_y+dialogue_menu_y_len)
				highlight_dialogue_response();
		}
    if(view_self_stats)display_stats(your_info);
    if(view_my_items)display_items_menu();
    if(view_ground_items)draw_pick_up_menu();
    if(item_dragged!=-1)drag_item();
    if(view_manufacture_menu)display_manufacture_menu();
    if(view_trade_menu)display_trade_menu();
    if(options_menu)draw_options_menu();
    if(view_sigils_menu)
    	{
			check_sigil_mouseover();
    		display_sigils_menu();
		}
}

int map_text;

void switch_to_game_map()
{
	int len;
	char map_map_file_name[60];

	//try to see if we can find a valid map
	my_strcp(map_map_file_name,map_file_name);
	len=strlen(map_map_file_name);
	map_map_file_name[len-3]='b';
	map_map_file_name[len-2]='m';
	map_map_file_name[len-1]='p';
	map_text=load_bmp8_fixed_alpha(map_map_file_name,128);
	if(!map_text)//this map has no map (sounds so stupid)
		{
			log_to_console(c_yellow2,"There is no map for this place.");
			return;
		}
	interface_mode=interface_map;
}

void switch_from_game_map()
{
	glDeleteTextures(1,&map_text);
	interface_mode=interface_game;
}


void draw_game_map()
{
	int screen_x;
	int screen_y;
	int i;

   	glDisable(GL_DEPTH_TEST);
   	glDisable(GL_LIGHTING);

   	glViewport(0, 0, window_width, window_height);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(300, (GLdouble)0, (GLdouble)0, 200, -250.0, 250.0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();


	glBindTexture(GL_TEXTURE_2D, map_text);
	last_texture=-1;

	glColor3f(1.0f,1.0f,1.0f);
	glBegin(GL_QUADS);
	//draw the texture

	glTexCoord2f(1.0f,0.0f);
	glVertex3i(50,0,0);

	glTexCoord2f(1.0f,1.0f);
	glVertex3i(50,200,0);

	glTexCoord2f(0.0f,1.0f);
	glVertex3i(250,200,0);

	glTexCoord2f(0.0f,0.0f);
	glVertex3i(250,0,0);

	glEnd();

	//ok, now let's draw our possition...
	for(i=0;i<max_actors;i++)
		{
			if(actors_list[i])
				if(actors_list[i]->actor_id==yourself)
					{
						int x=actors_list[i]->x_tile_pos;
						int y=actors_list[i]->y_tile_pos;
						screen_x=300-(50+200*x/(tile_map_size_x*6));
						screen_y=0+200*y/(tile_map_size_y*6);
						break;
					}
		}

	glColor3f(0.0f,0.0f,1.0f);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_LINES);
	glVertex2i(screen_x-3,screen_y-3);
	glVertex2i(screen_x+2,screen_y+2);

	glVertex2i(screen_x+2,screen_y-3);
	glVertex2i(screen_x-3,screen_y+2);
	glEnd();


	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0f,1.0f,1.0f);


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
}

void draw_menu_title_bar(int x, int y, int x_len)
{

	float u_first_start=(float)31/255;
	float u_first_end=0;
	float v_first_start=1.0f-(float)160/255;
	float v_first_end=1.0f-(float)175/255;

	float u_middle_start=(float)32/255;
	float u_middle_end=(float)63/255;
	float v_middle_start=1.0f-(float)160/255;
	float v_middle_end=1.0f-(float)175/255;

	float u_last_start=0;
	float u_last_end=(float)31/255;
	float v_last_start=1.0f-(float)160/255;
	float v_last_end=1.0f-(float)175/255;


	int segments_no;
	int i;

	glColor3f(1.0f,1.0f,1.0f);
	//ok, now draw that shit...
	segments_no=x_len/32;

	if(last_texture!=texture_cache[icons_text].texture_id)
		{
			glBindTexture(GL_TEXTURE_2D, texture_cache[icons_text].texture_id);
			last_texture=texture_cache[icons_text].texture_id;
		}


	glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER,0.03f);
	glBegin(GL_QUADS);

	glTexCoord2f(u_first_end,v_first_start);
	glVertex3i(x,y,0);
	glTexCoord2f(u_first_end,v_first_end);
	glVertex3i(x,y+16,0);
	glTexCoord2f(u_first_start,v_first_end);
	glVertex3i(x+32,y+16,0);
	glTexCoord2f(u_first_start,v_first_start);
	glVertex3i(x+32,y,0);


	for(i=1;i<segments_no-1;i++)
		{
			glTexCoord2f(u_middle_end,v_middle_start);
			glVertex3i(x+i*32,y,0);
			glTexCoord2f(u_middle_end,v_middle_end);
			glVertex3i(x+i*32,y+16,0);
			glTexCoord2f(u_middle_start,v_middle_end);
			glVertex3i(x+i*32+32,y+16,0);
			glTexCoord2f(u_middle_start,v_middle_start);
			glVertex3i(x+i*32+32,y,0);
		}

	glTexCoord2f(u_last_end,v_last_start);
	glVertex3i(x+i*32,y,0);
	glTexCoord2f(u_last_end,v_last_end);
	glVertex3i(x+i*32,y+16,0);
	glTexCoord2f(u_last_start,v_last_end);
	glVertex3i(x+i*32+32,y+16,0);
	glTexCoord2f(u_last_start,v_last_start);
	glVertex3i(x+i*32+32,y,0);

	glEnd();
	glDisable(GL_ALPHA_TEST);
}





