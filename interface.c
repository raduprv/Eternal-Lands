#include <string.h>
#include "global.h"
#include "elwindows.h"
#include <math.h>

int mouse_x;
int mouse_y;
int mouse_delta_x;
int mouse_delta_y;
int right_click;
int middle_click;
int left_click;
int open_text;
int login_screen_menus;
char username_box_selected=1;
char password_box_selected=0;
char username_str[16]={0};
char password_str[16]={0};
char display_password_str[16]={0};
int username_text_lenght=0;
int password_text_lenght=0;

int have_a_map=0;
char interface_mode=interface_opening;
char create_char_error_str[520];
char log_in_error_str[520];
int combat_mode=0;
int auto_camera=0;
int view_health_bar=1;
int view_names=1;
int view_hp=0;
int view_chat_text_as_overtext=0;
int limit_fps=0;

int action_mode=action_walk;

Uint32 click_time=0;
int click_speed=300;

void get_world_x_y()
{
	float window_ratio;
	float x,y,x1,y1,a,t;
	window_ratio=(GLfloat)window_width/(GLfloat)window_height;

	x=(float)((mouse_x)*2.8f*zoom_level/(window_width-hud_x))-(2.8*zoom_level/2.0f);
	y=(float)((window_height-hud_y-mouse_y)*2.0f*zoom_level/(window_height-hud_y))-(2.0*zoom_level/2.0f);

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
	if(drag_windows(mouse_x, mouse_y, mouse_delta_x, mouse_delta_y) > 0)	return 1;

	return 0;
}

int check_scroll_bars()
{
	if(drag_in_windows(mouse_x, mouse_y, 0, mouse_delta_x, mouse_delta_y) > 0)	return 1;

	return 0;
}

void check_mouse_click()
{
	// check for a click on the HUD (between scene & windows)
	if(click_in_windows(mouse_x, mouse_y, 0) > 0)	return;

	//after we test for interface clicks
	// alternative drop method...
	if (item_dragged != -1){
		Uint8 str[10];
		int quantity = item_list[item_dragged].quantity;

		if(right_click){
			str[0]= USE_MAP_OBJECT;
			*((int *)(str+1))= object_under_mouse;
			*((int *)(str+5))= item_list[item_dragged].pos;
			my_tcp_send(my_socket, str, 9);
			item_dragged = -1;
			return;
		}
		if (quantity - item_quantity > 0)
			quantity = item_quantity;
		str[0] = DROP_ITEM;
		str[1] = item_list[item_dragged].pos;
		*((Uint16 *) (str + 2)) = quantity;
		my_tcp_send(my_socket, str, 4);
		if (item_list[item_dragged].quantity - quantity <= 0)
			item_dragged = -1;
		return;
	}
	//LOOK AT
	if((current_cursor==CURSOR_EYE && left_click) || (action_mode==action_look && right_click))
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

	//if we're following a path, stop now
	if (pf_follow_path) {
		pf_destroy_path();
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
	if((current_cursor==CURSOR_ATTACK && left_click) || (action_mode==action_attack && right_click))
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
	if(((current_cursor==CURSOR_TALK || current_cursor==CURSOR_ENTER || current_cursor==CURSOR_USE) && left_click) || (action_mode==action_use && right_click))
		{
			Uint8 str[10];

			if(object_under_mouse==-1)return;
			if(thing_under_the_mouse==UNDER_MOUSE_PLAYER || thing_under_the_mouse==UNDER_MOUSE_NPC || thing_under_the_mouse==UNDER_MOUSE_ANIMAL)
				{
					int i;
					str[0]=TOUCH_PLAYER;
					*((int *)(str+1))=object_under_mouse;
					my_tcp_send(my_socket,str,5);

					//clear the previous dialogue entries, so we won't have a left over from some other NPC
					for(i=0;i<20;i++)dialogue_responces[i].in_use=0;
					return;
				}
			action_mode=action_walk;
			str[0]=USE_MAP_OBJECT;
			*((int *)(str+1))=object_under_mouse;
			*((int *)(str+5))=-1;
			my_tcp_send(my_socket,str,9);
			return;
		}

	//OPEN BAG
	if(current_cursor==CURSOR_PICK && left_click)
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

	//WALK
	if((action_mode==action_walk && right_click) || (current_cursor==CURSOR_WALK && left_click && (!you_sit || !sit_lock)))
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
			return;
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
	glViewport(0, hud_y, window_width-hud_x, window_height-hud_y);
	//glViewport(0, 0, window_width-hud_x, window_height-hud_y);	// Reset The Current Viewport
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
	get_and_set_texture_id(which_texture);
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

void draw_2d_thing_r(float u_start,float v_start,float u_end,float v_end,int x_start,
				   int y_start,int x_end,int y_end)
{
	glTexCoord2f(u_start,v_start);
	glVertex3i(x_start,y_end,0);

	glTexCoord2f(u_end,v_start);
	glVertex3i(x_start,y_start,0);

	glTexCoord2f(u_end,v_end);
	glVertex3i(x_end,y_start,0);

	glTexCoord2f(u_start,v_end);
	glVertex3i(x_end,y_end,0);
}


void init_opening_interface()
{
	check_gl_errors();
	login_screen_menus=load_texture_cache("./textures/login_menu.bmp",0);
	check_gl_errors();
	login_text=load_texture_cache("./textures/login_back.bmp",255);
	check_gl_errors();
}

void draw_login_screen()
{
	char str[20];
	float selected_bar_u_start=(float)0/256;
	float selected_bar_v_start=1.0f-(float)0/256;

	float selected_bar_u_end=(float)174/256;
	float selected_bar_v_end=1.0f-(float)28/256;

	float unselected_bar_u_start=(float)0/256;
	float unselected_bar_v_start=1.0f-(float)40/256;

	float unselected_bar_u_end=(float)170/256;
	float unselected_bar_v_end=1.0f-(float)63/256;
	/////////////////////////
	float log_in_unselected_start_u=(float)0/256;
	float log_in_unselected_start_v=1.0f-(float)80/256;

	float log_in_unselected_end_u=(float)87/256;
	float log_in_unselected_end_v=1.0f-(float)115/256;

	float log_in_selected_start_u=(float)0/256;
	float log_in_selected_start_v=1.0f-(float)120/256;

	float log_in_selected_end_u=(float)87/256;
	float log_in_selected_end_v=1.0f-(float)155/256;
	/////////////////////////
	float new_char_unselected_start_u=(float)100/256;
	float new_char_unselected_start_v=1.0f-(float)80/256;

	float new_char_unselected_end_u=(float)238/256;
	float new_char_unselected_end_v=1.0f-(float)115/256;

	float new_char_selected_start_u=(float)100/256;
	float new_char_selected_start_v=1.0f-(float)120/256;

	float new_char_selected_end_u=(float)238/256;
	float new_char_selected_end_v=1.0f-(float)155/256;

	/////////////////////////////////////////////
	/////////////////////////////////////////////
	int half_screen_x=window_width/2;
	int half_screen_y=window_height/2;
	int len1=strlen(login_username_str);
	int len2=strlen(login_password_str);
	int offset=20+(len1>len2?(len1+1)*16:(len2+1)*16);
	int username_text_x=half_screen_x-offset;
	int username_text_y=half_screen_y-130;
	int password_text_x=half_screen_x-offset;
	int password_text_y=half_screen_y-100;
	int username_bar_x=half_screen_x-50;
	int username_bar_y=username_text_y-7;
	int username_bar_x_len=174;
	int username_bar_y_len=28;
	int password_bar_x=half_screen_x-50;
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
	sprintf(str,"%s: ",login_username_str);
	draw_string(username_text_x,username_text_y,str,1);
	sprintf(str,"%s: ",login_password_str);
	draw_string(password_text_x,password_text_y,str,1);
	//start drawing the actual interface pieces
	get_and_set_texture_id(login_screen_menus);
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

void draw_ingame_interface()
{
	//check_menus_out_of_screen();

	// watch for closing a bag
	if(ground_items_win > 0)
		{
			int	old_view= view_ground_items;

			view_ground_items= get_show_window(ground_items_win);
			// watch for telling the server we need to close the bag
			if(old_view && !view_ground_items)
				{
					unsigned char protocol_name;

					protocol_name= S_CLOSE_BAG;
					my_tcp_send(my_socket,&protocol_name,1);
				}
		}

	glColor3f(1.0f, 1.0f, 1.0f);
	draw_hud_frame();
	display_windows(1);	// Display all the windows handled by the window manager
	//draw_hud_interface();
    display_spells_we_have();
    if(item_dragged!=-1)drag_item();

}

int map_text;

int switch_to_game_map()
{
	int len;
	char map_map_file_name[256];

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
			return 0;
		}
	interface_mode=interface_map;
	if(current_cursor!=CURSOR_ARROW)change_cursor(CURSOR_ARROW);
	return 1;
}

void switch_from_game_map()
{
	glDeleteTextures(1,&map_text);
	interface_mode=interface_game;
}

void draw_game_map()
{
	int screen_x=0;
	int screen_y=0;
	int i;

   	glDisable(GL_DEPTH_TEST);
   	glDisable(GL_LIGHTING);

   	glViewport(0, 0 + hud_y, window_width-hud_x, window_height-hud_y);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(300, (GLdouble)0, (GLdouble)0, 200, -250.0, 250.0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	bind_texture_id(map_text);
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

	//if we're following a path, draw the destination on the map
	if (pf_follow_path) {
		int x = pf_dst_tile->x;
		int y = pf_dst_tile->y;

		screen_x=300-(50+200*x/(tile_map_size_x*6));
		screen_y=0+200*y/(tile_map_size_y*6);

		glColor3f(1.0f,0.0f,0.0f);
		glDisable(GL_TEXTURE_2D);
		glBegin(GL_LINES);
		glVertex2i(screen_x-3,screen_y-3);
		glVertex2i(screen_x+2,screen_y+2);

		glVertex2i(screen_x+2,screen_y-3);
		glVertex2i(screen_x-3,screen_y+2);
		glEnd();
	}

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
	float u_first_start=(float)31/256;
	float u_first_end=0;
	float v_first_start=1.0f-(float)160/256;
	float v_first_end=1.0f-(float)175/256;

	float u_middle_start=(float)32/256;
	float u_middle_end=(float)63/256;
	float v_middle_start=1.0f-(float)160/256;
	float v_middle_end=1.0f-(float)175/256;

	float u_last_start=0;
	float u_last_end=(float)31/256;
	float v_last_start=1.0f-(float)160/256;
	float v_last_end=1.0f-(float)175/256;

	int segments_no;
	int i;

	glColor3f(1.0f,1.0f,1.0f);
	//ok, now draw that shit...
	segments_no=x_len/32;

	get_and_set_texture_id(icons_text);
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

