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
char interface_mode=interface_rules;
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

extern marking marks[200];
extern int adding_mark;
extern int mark_x , mark_y;
extern int max_mark;

void get_world_x_y()
{
  	float mouse_z,z;	
	glReadPixels(mouse_x, window_height-mouse_y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &mouse_z);
	unproject_ortho(mouse_x,window_height-hud_y-mouse_y,mouse_z,&scene_mouse_x,&scene_mouse_y,&z);
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

	if(right_click) {
		if(object_under_mouse==-1) {
			action_mode=action_walk;
			if(use_item!=-1)
				use_item=-1;
			if(item_dragged!=-1)
				item_dragged=-1;
			return;
		}
		switch(current_cursor) {
		case CURSOR_EYE:
			if(thing_under_the_mouse==UNDER_MOUSE_PLAYER)
				action_mode=action_trade;
			else if(thing_under_the_mouse==UNDER_MOUSE_3D_OBJ)
				action_mode=action_use;
			else
				action_mode=action_walk;
			break;
		case CURSOR_HARVEST:
			action_mode=action_look;
			break;
		case CURSOR_TRADE:
			action_mode=action_attack;
			break;
		case CURSOR_USE_WITEM:
			if(use_item!=-1)
				use_item=-1;
			else
				action_mode=action_walk;
			break;
		case CURSOR_ATTACK:
			if(thing_under_the_mouse==UNDER_MOUSE_ANIMAL)
				action_mode=action_look;
			else
				action_mode=action_walk;
			break;
		case CURSOR_ENTER:
		case CURSOR_PICK:
		case CURSOR_WALK:
			if(thing_under_the_mouse==UNDER_MOUSE_3D_OBJ)
				action_mode=action_look;
			else
				action_mode=action_walk;
			break;
		case CURSOR_USE:
		case CURSOR_TALK:
		case CURSOR_ARROW:
		default:
			action_mode=action_walk;
			break;
		}
		return;
	}

	//after we test for interface clicks
	// alternative drop method...
	if (item_dragged != -1){
		Uint8 str[10];
		int quantity = item_list[item_dragged].quantity;

		if(right_click) {
			item_dragged=-1;
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

	//if we're following a path, stop now
	if (pf_follow_path) {
		pf_destroy_path();
	}

	switch(current_cursor) {
	case CURSOR_EYE:
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
		break;

	case CURSOR_TRADE:
		{
			Uint8 str[10];

			if(object_under_mouse==-1)return;
			if(thing_under_the_mouse!=UNDER_MOUSE_PLAYER)return;
			str[0]=TRADE_WITH;
			*((int *)(str+1))=object_under_mouse;
			my_tcp_send(my_socket,str,5);
			return;
		}
		break;

	case CURSOR_ATTACK:
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
		break;

	case CURSOR_USE:
	case CURSOR_USE_WITEM:
	case CURSOR_TALK:
	case CURSOR_ENTER:
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
			str[0]=USE_MAP_OBJECT;
			*((int *)(str+1))=object_under_mouse;
			if(use_item!=-1 && current_cursor==CURSOR_USE_WITEM)
				*((int *)(str+5))=item_list[use_item].pos;
			else
				*((int *)(str+5))=-1;
			my_tcp_send(my_socket,str,9);
			return;
		}
		break;

	case CURSOR_PICK:
		{
			if(object_under_mouse==-1)return;
			open_bag(object_under_mouse);
			return;
		}
		break;

	case CURSOR_HARVEST:
		{
			Uint8 str[10];

			if(object_under_mouse==-1)return;
			str[0]=HARVEST;
			*((short *)(str+1))=object_under_mouse;
			my_tcp_send(my_socket,str,3);
			return;
		}
		break;

	case CURSOR_WALK:
	default:
		if(!you_sit || !sit_lock)
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
	if(new_char_button_selected && left_click==1) {
		init_rules_interface(interface_new_char, 1.0f, 30);
		left_click=2;
	}

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
	if(item_dragged!=-1)drag_item(item_dragged,0);
	if(use_item!=-1 && current_cursor==CURSOR_USE_WITEM)drag_item(use_item,1);
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

GLuint map_text;
GLuint cont_text;
GLuint legend_text=0;
int cur_map;  //Is there a better way to do this?

const struct draw_map seridia_maps[] = {
	{409,107,450,147},//0 - Isla Prima
	{184,162,395,359},//1 - Whitestone
	{84,352,180,448}, //2 - Desert Pines
	{336,118,387,165},//3 - Tirnym
	{245,405,281,451},//4 - VOTD
	{84,270,177,357}, //5 - Portland
	{87,169,175,270}, //6 - Morcraven
	{130,128,178,168},//7 - Naralik
	{180,75,275,165}, //8 - Grubani
	{0,0,0,0},	  //9 -
	{282,358,385,454},//10 - Tarsengaard
	{232,359,283,403},//11 - Nordcarn
	{181,363,231,408},//12 - Southern KF
	{178,406,227,443},//13 - KF
	{2,324,75,431}    //14 - Tahraji
};

int switch_to_game_map()
{
	int len;
	char map_map_file_name[256];

	my_strcp(map_map_file_name,map_file_name);
	len=strlen(map_map_file_name);
	map_map_file_name[len-3]='b';
	map_map_file_name[len-2]='m';
	map_map_file_name[len-1]='p';
	map_text=load_bmp8_fixed_alpha(map_map_file_name,128);
	if(!map_text){
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


void draw_game_map(int map)
{     
	int screen_x=0;
	int screen_y=0;
	int x=-1,y=-1;
	int i;
	float scale=(float)(window_width-hud_x)/300.0f;
	float x_size=0,y_size=0;
	GLuint map_small, map_large;
	
	if(map){
		map_small=get_texture_id(cont_text);
		map_large=map_text;
	} else {
		map_small=map_text;
		map_large=get_texture_id(cont_text);
		if(cur_map!=-1){
			x_size=(float)((float)(seridia_maps[cur_map].x_end - seridia_maps[cur_map].x_start))/(float)tile_map_size_x;
			y_size=(float)((float)(seridia_maps[cur_map].y_end - seridia_maps[cur_map].y_start))/(float)tile_map_size_y;
		} else {
			x_size=y_size=0;
		}
	}
	
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

	glColor3f(1.0f,1.0f,1.0f);
    	
    bind_texture_id(map_large);
    
	glBegin(GL_QUADS);
		glTexCoord2f(1.0f,0.0f); glVertex3i(50,0,0); 
		glTexCoord2f(1.0f,1.0f); glVertex3i(50,200,0);
		glTexCoord2f(0.0f,1.0f); glVertex3i(250,200,0);
		glTexCoord2f(0.0f,0.0f); glVertex3i(250,0,0);
	glEnd();

	if(mouse_x > 0 && mouse_x < 50*scale && mouse_y > 0 && mouse_y < 55*scale){
		if(left_click==1){
			if(map)interface_mode=interface_cont;
			else interface_mode=interface_map;
			left_click=2;
		}
		glColor4f(1.0f,1.0f,1.0f,1.0f);
	} else 
		glColor4f(0.7f,0.7f,0.7f,0.7f);
    	
	glEnable(GL_ALPHA_TEST);
	
    bind_texture_id(map_small);
    	
    	glBegin(GL_QUADS);
		glTexCoord2f(1.0f,0.0f); glVertex3i(250,150,0);
		glTexCoord2f(1.0f,1.0f); glVertex3i(250,200,0);
		glTexCoord2f(0.0f,1.0f); glVertex3i(300,200,0);
		glTexCoord2f(0.0f,0.0f); glVertex3i(300,150,0);
	glEnd();
	
	glDisable(GL_ALPHA_TEST);
	
	glColor3f(1.0f,1.0f,1.0f);
    	
    get_and_set_texture_id(legend_text);
    
    	glBegin(GL_QUADS);
		glTexCoord2f(1.0f,0.0f); glVertex3i(250,50,0);
		glTexCoord2f(1.0f,1.0f); glVertex3i(250,150,0);
		glTexCoord2f(0.0f,1.0f); glVertex3i(300,150,0);
		glTexCoord2f(0.0f,0.0f); glVertex3i(300,50,0);
	glEnd();

    //if we're following a path, draw the destination on the map
    if (pf_follow_path) {
       	   int px = pf_dst_tile->x;
           int py = pf_dst_tile->y;

	   if(!map){
		if(cur_map!=-1){
			screen_x=300-(50+200*((x_size/6 * px)+(seridia_maps[cur_map].x_start))/(512));
			screen_y=0+200*((y_size/6 * py)+(seridia_maps[cur_map].y_start))/(512);
		} else {
			screen_x=screen_y=0;
		}
	   } else {
		screen_x=300-(50+200*px/(tile_map_size_x*6));
		screen_y=0+200*py/(tile_map_size_y*6);
	   }

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
			if(actors_list[i]->actor_id==yourself && actors_list[i]->tmp.have_tmp)
				{
						x=actors_list[i]->tmp.x_tile_pos;
						y=actors_list[i]->tmp.y_tile_pos;
						break;
					}
		}

	if(!map){
		if(cur_map!=-1){
			screen_x=300-(50+200*((x_size/6 * x)+(seridia_maps[cur_map].x_start))/(512));
			screen_y=0+200*((y_size/6 * y)+(seridia_maps[cur_map].y_start))/(512);
		} else {
			screen_x=screen_y=0;
		}
	} else {
		screen_x=300-(50+200*x/(tile_map_size_x*6));
		screen_y=0+200*y/(tile_map_size_y*6);
	}
	if((map||!dungeon)&&x!=-1){
		glColor3f(0.0f,0.0f,1.0f);
		glDisable(GL_TEXTURE_2D);
		glBegin(GL_LINES);
			glVertex2i(screen_x-3,screen_y-3);
			glVertex2i(screen_x+2,screen_y+2);

			glVertex2i(screen_x+2,screen_y-3);
			glVertex2i(screen_x-3,screen_y+2);
		glEnd();
	}
	
	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0f,1.0f,1.0f);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

// this is necessary for the text over map
	if(map&&(adding_mark||max_mark>0)){
   		glViewport(0, 0 + hud_y, window_width-hud_x, window_height-hud_y);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho((GLdouble)0, (GLdouble)300, (GLdouble)200, (GLdouble)0, -250.0, 250.0);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
 
		// draw a temporary mark until the text is entered
		if (adding_mark) {
        	        int x = mark_x;
        	        int y = mark_y;

			screen_x=(50+200*x/(tile_map_size_x*6));
			screen_y=200-200*y/(tile_map_size_y*6);

			glColor3f(1.0f,1.0f,0.0f);
			glDisable(GL_TEXTURE_2D);
			glBegin(GL_LINES);
				glVertex2i(screen_x-3,screen_y-3);
				glVertex2i(screen_x+2,screen_y+2);

				glVertex2i(screen_x+2,screen_y-3);
				glVertex2i(screen_x-3,screen_y+2);
			glEnd();
		        glEnable(GL_TEXTURE_2D);
		        glColor3f(1.0f,1.0f,0.0f);
			draw_string_zoomed(screen_x,screen_y,input_text_line,1,0.3);
		}


		// crave the markings
		for(i=0;i<max_mark;i++)
		 {
			int x = marks[i].x;
			int y = marks[i].y;
			if ( x > 0 ) {
				screen_x=(50+200*x/(tile_map_size_x*6));
				screen_y=200-200*y/(tile_map_size_y*6);

				glColor3f(0.4f,1.0f,0.0f);
				glDisable(GL_TEXTURE_2D);
				glBegin(GL_LINES);
					glVertex2i(screen_x-3,screen_y-3);
					glVertex2i(screen_x+2,screen_y+2);
				
					glVertex2i(screen_x+2,screen_y-3);
					glVertex2i(screen_x-3,screen_y+2);
				glEnd();
	        		glEnable(GL_TEXTURE_2D);
	        		glColor3f(0.2f,1.0f,0.0f);
				draw_string_zoomed(screen_x,screen_y,marks[i].text,1,0.3);
			}
	 	}

	    	glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
}


void put_mark_on_map_on_mouse_position()
{
        int min_mouse_x = (window_width-hud_x)/6;
        int min_mouse_y = 0;

        int max_mouse_x = min_mouse_x+((window_width-hud_x)/1.5);
        int max_mouse_y = window_height - hud_y;

        int screen_map_width = max_mouse_x - min_mouse_x;
        int screen_map_height = max_mouse_y - min_mouse_y;

        if (mouse_x < min_mouse_x
        || mouse_x > max_mouse_x
        || mouse_y < min_mouse_y
        || mouse_y > max_mouse_y) {
                return;
        }

        mark_x = ((mouse_x - min_mouse_x) * tile_map_size_x * 6) / screen_map_width;
        mark_y = (tile_map_size_y * 6) - ((mouse_y * tile_map_size_y * 6) / screen_map_height);
        adding_mark = 1;
}

void delete_mark_on_map_on_mouse_position()
{
        int min_mouse_x = (window_width-hud_x)/6;
        int min_mouse_y = 0;
	int mx , my , i;
        int max_mouse_x = min_mouse_x+((window_width-hud_x)/1.5);
        int max_mouse_y = window_height - hud_y;

        int screen_map_width = max_mouse_x - min_mouse_x;
        int screen_map_height = max_mouse_y - min_mouse_y;

        if (mouse_x < min_mouse_x
        || mouse_x > max_mouse_x
        || mouse_y < min_mouse_y
        || mouse_y > max_mouse_y) {
                return;
        }

        mx = ((mouse_x - min_mouse_x) * tile_map_size_x * 6) / screen_map_width;
        my = (tile_map_size_y * 6) - ((mouse_y * tile_map_size_y * 6) / screen_map_height);

	for ( i = 0 ; i < max_mark ; i ++ ) 
	    if (( abs( mx - marks[i].x) < 20 ) && (abs( my - marks[i].y) < 20 ) )
              {
		marks[i].x =  -1 ;
		marks[i].y =  -1 ;
		break;
	      }
save_markings();
}

void save_markings()
{
      FILE * fp;
      char marks_file[256];
      int i;

#ifndef WINDOWS
      strcpy(marks_file, getenv("HOME"));
      strcat(marks_file, "/.elc/");
      strcat(marks_file,strrchr(map_file_name,'/')+1);
#else
      strcpy(marks_file,strrchr(map_file_name,'/')+1);
#endif
      strcat(marks_file,".txt");
      fp = fopen(marks_file,"w");
      if ( fp ) {
	  for ( i = 0 ; i < max_mark ; i ++)
    	     if ( marks[i].x > 0 )
                fprintf(fp,"%d %d %s\n",marks[i].x,marks[i].y,marks[i].text);
          fclose(fp);
        };
}
