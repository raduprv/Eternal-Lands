#include <string.h>
#include <math.h>
#include "global.h"
#include "elwindows.h"
#include "keys.h" //Avoid problems with SHIFT, ALT, CTRL

#define STATE(I) (icons.icon[I]->state&0x0F)
#define PRESSED (1|(1<<31))
#define NOT_ACTIVE 0

struct icons_struct icons =
	{
		0,
		0,
		0,
		{NULL},
	};

//Windows not handled by the window manager:
int map_win=0;
int console_win=0;

int	display_icons_handler(window_info *win);
int	click_icons_handler(window_info *win, int mx, int my, Uint32 flags);
int	mouseover_icons_handler(window_info *win, int mx, int my);
int	display_stats_bar_handler(window_info *win);
int	display_misc_handler(window_info *win);
int	click_misc_handler(window_info *win, int mx, int my, Uint32 flags);
int	display_quickbar_handler(window_info *win);
int	click_quickbar_handler(window_info *win, int mx, int my, Uint32 flags);
int	mouseover_stats_bar_handler(window_info *win, int mx, int my);

int hud_x= 64;
int hud_y= 48;
int hud_text;
int view_digital_clock= 0;
int	icons_win= -1;
int	stats_bar_win= -1;
int	misc_win= -1;
int	quickbar_win= -1;
int show_help_text=1;

int show_stats_in_hud=0;

// initialize anything related to the hud
void init_hud_interface()
{
	init_hud_frame();
	init_peace_icons();
	init_misc_display();
	init_stats_display();
	init_quickbar();
}

// draw everything related to the hud
void draw_hud_interface()
{
	glColor3f(1.0f, 1.0f, 1.0f);
	draw_hud_frame();
	display_windows(0);	// draw only the non-stacked windows
}

// check to see if a mouse click was on the hud
// used in non-standard modes
int check_hud_interface()
{
	return click_in_windows(mouse_x, mouse_y, 0);	// temporarily here for testing
}

// hud frame section
float vertical_bar_u_start=(float)192/256;
float vertical_bar_u_end=1.0f;
float vertical_bar_v_end=0.0f;
float vertical_bar_v_start;

float horizontal_bar_u_start=(float)144/256;
float horizontal_bar_u_end=(float)191/256;
float horizontal_bar_v_start=0;
float horizontal_bar_v_end;

void init_hud_frame()
{
	vertical_bar_v_start= (float)window_height/256;
	horizontal_bar_v_end= (float)(window_width-hud_x)/256;
}

float logo_u_start=(float)64/256;
float logo_v_start=1.0f-(float)128/256;

float logo_u_end=(float)127/256;
float logo_v_end=1.0f-(float)191/256;

void draw_hud_frame()
{
	get_and_set_texture_id(hud_text);
	glBegin(GL_QUADS);
	draw_2d_thing(vertical_bar_u_start, vertical_bar_v_start, vertical_bar_u_end, vertical_bar_v_end,window_width-hud_x, 0, window_width, window_height);
	draw_2d_thing_r(horizontal_bar_u_start, horizontal_bar_v_start, horizontal_bar_u_end, horizontal_bar_v_end,0,window_height,window_width-hud_x , window_height-hud_y);
	//draw the logo
	draw_2d_thing(logo_u_start, logo_v_start, logo_u_end, logo_v_end,window_width-hud_x, 0, window_width, 64);
	glEnd();
}


// the icons section
float walk_icon_u_start=(float)0/256;
float walk_icon_v_start=1.0f-(float)0/256;

float colored_walk_icon_u_start=(float)64/256;
float colored_walk_icon_v_start=1.0f-(float)64/256;

float eye_icon_u_start=(float)64/256;
float eye_icon_v_start=1.0f-(float)0/256;

float colored_eye_icon_u_start=(float)128/256;
float colored_eye_icon_v_start=1.0f-(float)64/256;

float trade_icon_u_start=(float)128/256;
float trade_icon_v_start=1.0f-(float)0/256;

float colored_trade_icon_u_start=(float)192/256;
float colored_trade_icon_v_start=1.0f-(float)64/256;

float sit_icon_u_start=(float)224/256;
float sit_icon_v_start=1.0f-(float)0/256;

float colored_sit_icon_u_start=(float)32/256;
float colored_sit_icon_v_start=1.0f-(float)96/256;

float stand_icon_u_start=(float)0/256;
float stand_icon_v_start=1.0f-(float)32/256;

float colored_stand_icon_u_start=(float)64/256;
float colored_stand_icon_v_start=1.0f-(float)96/256;

float spell_icon_u_start=(float)32/256;
float spell_icon_v_start=1.0f-(float)32/256;

float colored_spell_icon_u_start=(float)96/256;
float colored_spell_icon_v_start=1.0f-(float)96/256;

float inventory_icon_u_start=(float)96/256;
float inventory_icon_v_start=1.0f-(float)32/256;

float colored_inventory_icon_u_start=(float)160/256;
float colored_inventory_icon_v_start=1.0f-(float)96/256;

float manufacture_icon_u_start=(float)128/256;
float manufacture_icon_v_start=1.0f-(float)32/256;

float colored_manufacture_icon_u_start=(float)0/256;
float colored_manufacture_icon_v_start=1.0f-(float)128/256;

float stats_icon_u_start=(float)160/256;
float stats_icon_v_start=1.0f-(float)32/256;

float colored_stats_icon_u_start=(float)32/256;
float colored_stats_icon_v_start=1.0f-(float)128/256;

float options_icon_u_start=(float)192/256;
float options_icon_v_start=1.0f-(float)32/256;

float colored_options_icon_u_start=(float)64/256;
float colored_options_icon_v_start=1.0f-(float)128/256;

float use_icon_u_start=(float)224/256;
float use_icon_v_start=1.0f-(float)32/256;

float colored_use_icon_u_start=(float)96/256;
float colored_use_icon_v_start=1.0f-(float)128/256;

float attack_icon_u_start=(float)160/256;
float attack_icon_v_start=1.0f-(float)0/256;

float colored_attack_icon_u_start=(float)224/256;
float colored_attack_icon_v_start=1.0f-(float)64/256;

float knowledge_icon_u_start=(float)96/256;
float knowledge_icon_v_start=1.0f-(float)64/256;

float colored_knowledge_icon_u_start=(float)160/256;
float colored_knowledge_icon_v_start=1.0f-(float)64/256;

float encyclopedia_icon_u_start=(float)0/256;
float encyclopedia_icon_v_start=1.0f-(float)64/256;

float colored_encyclopedia_icon_u_start=(float)32/256;
float colored_encyclopedia_icon_v_start=1.0f-(float)64/256;

float questlog_icon_u_start=(float)96/256;
float questlog_icon_v_start=1.0f-(float)0/256;

float colored_questlog_icon_u_start=(float)192/256;
float colored_questlog_icon_v_start=1.0f-(float)0/256;

float map_icon_u_start=(float)128/256;
float map_icon_v_start=1.0f-(float)128/256;

float colored_map_icon_u_start=(float)160/256;
float colored_map_icon_v_start=1.0f-(float)128/256;

float console_icon_u_start=(float)32/256;
float console_icon_v_start=1.0f-(float)0/256;

float colored_console_icon_u_start=(float)128/256;
float colored_console_icon_v_start=1.0f-(float)96/256;

float buddy_icon_u_start=(float)64/256;
float buddy_icon_v_start=1.0f-(float)32/256;

float colored_buddy_icon_u_start=(float)0/256;
float colored_buddy_icon_v_start=1.0f-(float)96/256;

// to help highlight the proper icon
int	icon_cursor_x;

int	statbar_cursor_x;

//stat bars
int health_bar_start_x;
int health_bar_start_y;

int mana_bar_start_x;
int mana_bar_start_y;

int food_bar_start_x;
int food_bar_start_y;

int load_bar_start_x;
int load_bar_start_y;

int exp_bar_start_x;
int exp_bar_start_y;

void init_peace_icons()
{
	int type=action_walk;
	//create the icon window
	if(icons_win <= 0)
		{
			icons_win= create_window("Icons", 0, 0, 0, window_height-32, window_width-64, 32, ELW_TITLE_NONE|ELW_SHOW|ELW_SHOW_LAST);
			set_window_handler(icons_win, ELW_HANDLER_DISPLAY, &display_icons_handler);
			set_window_handler(icons_win, ELW_HANDLER_CLICK, &click_icons_handler);
			set_window_handler(icons_win, ELW_HANDLER_MOUSEOVER, &mouseover_icons_handler);
		}
	else
		{
			move_window(icons_win, 0, 0, 0, window_height-32);
		}

	if(icons.no) return;

	icons.y=0;
	icons.x=0;
	
	add_icon(walk_icon_u_start, walk_icon_v_start, colored_walk_icon_u_start, colored_walk_icon_v_start, tt_walk, switch_action_mode, &type, DATA_ACTIONMODE);
	
	if(you_sit)
		add_icon(stand_icon_u_start, stand_icon_v_start, colored_stand_icon_u_start, colored_stand_icon_v_start, tt_stand, sit_button_pressed, NULL, DATA_NONE);
	else
		add_icon(sit_icon_u_start, sit_icon_v_start, colored_sit_icon_u_start, colored_sit_icon_v_start, tt_sit, sit_button_pressed, NULL, DATA_NONE);
	
	type=action_look;
	add_icon(eye_icon_u_start, eye_icon_v_start, colored_eye_icon_u_start, colored_eye_icon_v_start, tt_look, switch_action_mode, &type, DATA_ACTIONMODE);

	type=action_use;
	add_icon(use_icon_u_start, use_icon_v_start, colored_use_icon_u_start, colored_use_icon_v_start, tt_use, switch_action_mode, &type, DATA_ACTIONMODE);

	type=action_trade;
	add_icon(trade_icon_u_start, trade_icon_v_start, colored_trade_icon_u_start, colored_trade_icon_v_start, tt_trade, switch_action_mode, &type, DATA_ACTIONMODE);

	type=action_attack;
	add_icon(attack_icon_u_start, attack_icon_v_start, colored_attack_icon_u_start, colored_attack_icon_v_start, tt_attack, switch_action_mode, &type, DATA_ACTIONMODE);

	//done with the integer variables - now for the windows
	
	add_icon(inventory_icon_u_start, inventory_icon_v_start, colored_inventory_icon_u_start, colored_inventory_icon_v_start, tt_inventory, view_window, &items_win, DATA_WINDOW);
	
	add_icon(spell_icon_u_start, spell_icon_v_start, colored_spell_icon_u_start, colored_spell_icon_v_start, tt_spell, view_window, &sigil_win, DATA_WINDOW);
	
	add_icon(manufacture_icon_u_start, manufacture_icon_v_start, colored_manufacture_icon_u_start, colored_manufacture_icon_v_start, tt_manufacture, view_window, &manufacture_win, DATA_WINDOW);
	
	add_icon(stats_icon_u_start, stats_icon_v_start, colored_stats_icon_u_start, colored_stats_icon_v_start, tt_stats, view_window, &stats_win, DATA_WINDOW);
	
	add_icon(knowledge_icon_u_start, knowledge_icon_v_start, colored_knowledge_icon_u_start, colored_knowledge_icon_v_start, tt_knowledge, view_window, &knowledge_win, DATA_WINDOW);
	
	add_icon(encyclopedia_icon_u_start, encyclopedia_icon_v_start, colored_encyclopedia_icon_u_start, colored_encyclopedia_icon_v_start, tt_encyclopedia, view_window, &encyclopedia_win, DATA_WINDOW);
	
	add_icon(questlog_icon_u_start, questlog_icon_v_start, colored_questlog_icon_u_start, colored_questlog_icon_v_start, tt_questlog, view_window, &questlog_win, DATA_WINDOW);
	
	add_icon(map_icon_u_start, map_icon_v_start, colored_map_icon_u_start, colored_map_icon_v_start, tt_mapwin, view_map_win, &map_win, DATA_WINDOW);
	
	add_icon(console_icon_u_start, console_icon_v_start, colored_console_icon_u_start, colored_console_icon_v_start, tt_console, view_console_win, &console_win, DATA_WINDOW);
	
	add_icon(buddy_icon_u_start, buddy_icon_v_start, colored_buddy_icon_u_start, colored_buddy_icon_v_start, tt_buddy, view_window, &buddy_win, DATA_WINDOW);
	
	add_icon(options_icon_u_start, options_icon_v_start, colored_options_icon_u_start, colored_options_icon_v_start, tt_options, view_window, &options_win, DATA_WINDOW);
}

void	add_icon(float u_start, float v_start, float colored_u_start, float colored_v_start, char * help_message, void * func, void * data, char data_type)
{
	int no=icons.no++;
	icons.icon[no]=(icon_struct*)calloc(1,sizeof(icon_struct));
	if(!no)
		icons.icon[no]->state=PRESSED;
	else
		icons.icon[no]->state=0;
	icons.icon[no]->u[0]=u_start;
	icons.icon[no]->u[1]=colored_u_start;
	icons.icon[no]->v[0]=v_start;
	icons.icon[no]->v[1]=colored_v_start;
	icons.icon[no]->func=func;
	icons.icon[no]->help_message=help_message;
	icons.icon[no]->free_data=0;
	switch(data_type)
		{
			case DATA_ACTIONMODE:
				icons.icon[no]->data=(int*)calloc(1,sizeof(int));
				memcpy(icons.icon[no]->data,data,sizeof(int));
				icons.icon[no]->free_data=1;
				break;
			case DATA_WINDOW:
				icons.icon[no]->data=(int*)data;
				break;
			case DATA_NONE:
				icons.icon[no]->data=NULL;
				break;
		}
	icons.icon[no]->data_type=data_type;
}

void free_icons()
{
	int i;
	for(i=0;i<icons.no;i++) {
		if(icons.icon[i]->free_data)
			free(icons.icon[i]->data);
		free(icons.icon[i]);
	}
}

int	mouseover_icons_handler(window_info *win, int mx, int my)
{
	icon_cursor_x= mx;	// just memorize for later

	return 0;
}

void draw_peace_icons()
{
    display_window(icons_win);
}

int	display_icons_handler(window_info *win)
{

	int i, state=-1, *z;

	//glEnable(GL_ALPHA_TEST);//enable alpha filtering, so we have some alpha key
	//glAlphaFunc(GL_GREATER,0.03f);

	for(i=0;i<icons.no;i++)
		{
			if(icons.icon[i]->data_type==DATA_WINDOW)
				{
					z = (int*)icons.icon[i]->data;
					if(*z && *z<windows_list.num_windows) icons.icon[i]->state=windows_list.window[*z].displayed;
				}
			else if(icons.icon[i]->data_type==DATA_ACTIONMODE)
				{
					z = (int*)icons.icon[i]->data;
					if(action_mode==*z)icons.icon[i]->state=1;
					else icons.icon[i]->state=0;
				}
		}
	
	if(mouse_in_window(win->window_id, mouse_x, mouse_y))
		{
			state=icon_cursor_x/32;//Icons must be 32 pixels wide...
			if(state<icons.no)icons.icon[state]->state|=0x01;//Set the state to be mouseover
			else state=-1;
		}
	
	get_and_set_texture_id(icons_text);
	glColor3f(1.0f,1.0f,1.0f);
	glBegin(GL_QUADS);
	
	for(i=0;i<icons.no;i++)
		{
			draw_2d_thing(
				icons.icon[i]->u[STATE(i)],
				icons.icon[i]->v[STATE(i)], 
				icons.icon[i]->u[STATE(i)]+(float)31/256,
				icons.icon[i]->v[STATE(i)]-(float)31/256,
				icons.x+i*32, 
				icons.y, 
				icons.x+i*32+31,
				icons.y+32
				);
			if(!(icons.icon[i]->state>>31))icons.icon[i]->state=0;//Else we pressed the button and it should still be pressed
		}
	glEnd();
	//glDisable(GL_ALPHA_TEST);
	if(state>=0 && show_help_text) show_help(icons.icon[state]->help_message, 32*(state+1)+2, 10);//Show the help message

	return 1;
}

void sit_button_pressed(void * none, int id)
{
	if(you_sit)
		{
			Uint8 str[4];
			you_stand_up();
			//Send message to server...	
			str[0]=SIT_DOWN;
			str[1]=0;
			my_tcp_send(my_socket,str,2);
		}
	else
		{
			Uint8 str[4];
			you_sit_down();
			//Send message to server...
			str[0]=SIT_DOWN;
			str[1]=1;
			my_tcp_send(my_socket,str,2);
		}
}

void you_sit_down()
{
	you_sit=1;
	icons.icon[1]->u[0]=stand_icon_u_start;//Change the icon to stand
	icons.icon[1]->u[1]=colored_stand_icon_u_start;
	icons.icon[1]->v[0]=stand_icon_v_start;
	icons.icon[1]->v[1]=colored_stand_icon_v_start;
	icons.icon[1]->help_message=tt_stand;
}

void you_stand_up()
{
	you_sit=0;
	icons.icon[1]->u[0]=sit_icon_u_start;
	icons.icon[1]->u[1]=colored_sit_icon_u_start;
	icons.icon[1]->v[0]=sit_icon_v_start;
	icons.icon[1]->v[1]=colored_sit_icon_v_start;
	icons.icon[1]->help_message=tt_sit;
}

void switch_action_mode(int * mode, int id)
{
	action_mode=*mode;
}

void view_console_win(int * win, int id)
{
	if(id<0)id=translate_win_id(&console_win);
	if(interface_mode==interface_console)
		{
			interface_mode=interface_game;
			icons.icon[id]->state=0;
		}
	else
		{
			interface_mode=interface_console;
			if(current_cursor!=CURSOR_ARROW)change_cursor(CURSOR_ARROW);
			icons.icon[id]->state=PRESSED;
			icons.icon[id-1]->state=0;
		}
}

void view_map_win(int * win, int id)
{
	if(id<0)id=translate_win_id(&map_win);
	if(interface_mode==interface_game || interface_mode==interface_console)
		{
			if(switch_to_game_map()) {
				icons.icon[id]->state=PRESSED;
				icons.icon[id+1]->state=0;
			}
		}
	else if(interface_mode==interface_map)
		{
			switch_from_game_map();
			icons.icon[id]->state=0;
		}
}

void view_window(int * window, int id)
{
	if(window==&items_win||window==&sigil_win||window==&manufacture_win)
		{
			if(get_show_window(trade_win))
				{
					log_to_console(c_red2,no_open_on_trade);
					return;
				}
		}
	if(!*window)
		{
			//OK, the window has not been created yet - use the standard functions
			if(window==&items_win)display_items_menu();
			else if(window==&sigil_win) display_sigils_menu();
			else if(window==&manufacture_win) display_manufacture_menu();
			else if(window==&options_win) display_options_menu();
			else if(window==&stats_win) display_stats(your_info);
			else if(window==&knowledge_win) display_knowledge();
			else if(window==&questlog_win) display_questlog();
			else if(window==&encyclopedia_win) display_encyclopedia();
			else if(window==&buddy_win) display_buddy();
			else if(window==&trade_win) display_trade_menu();
		}
	else toggle_window(*window);
}

int check_peace_icons()
{
    return(click_in_window(icons_win, mouse_x, mouse_y, 0));
}

int	click_icons_handler(window_info *win, int mx, int my, Uint32 flags)
{
	int id=mx/32;//Icons are always 32 bit wide
	if(combat_mode)return 0;

	if(id<icons.no)
		{
			switch(icons.icon[id]->data_type)
				{
					case DATA_ACTIONMODE:
					case DATA_WINDOW:
						{
							int * data=(int *)icons.icon[id]->data;
							icons.icon[id]->func((int *)data, id);
							break;
						}
					default:
						{
							icons.icon[id]->func(0, id);
							break;
						}
				}
		}
	return 1;
}

void show_help(char *help_message, int x, int y)
{
	Uint8 str[125];
	int len=strlen(help_message)*8+1;
	int width=window_width-80;
	
	if(x+len>width) x-=(x+len)-width;
	
	glColor4f(0.0f,0.0f,0.0f,0.5f);
	glDisable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glBegin(GL_QUADS);
	glVertex3i(x-1,y+15,0);
	glVertex3i(x-1,y,0);
	glVertex3i(x+len,y,0);
	glVertex3i(x+len,y+15,0);
	glEnd();
	
	glDisable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	
	glColor3f(1.0f,1.0f,1.0f);
	strcpy(str,help_message);
	draw_string_small(x, y,help_message,1);
}

int translate_win_id(int * win_id)
{
	int i=0;
	for(;i<icons.no;i++)
		{
			if(icons.icon[i]->data_type==DATA_WINDOW)
				{
					if(icons.icon[i]->data==win_id) return i;
				}
		}
	return -1;
}

// the stats display
void init_stats_display()
{
	//create the stats bar window
	if(stats_bar_win <= 0)
		{
			stats_bar_win= create_window("Stats Bar", 0, 0, 24, window_height-44, window_width-24-64, 12, ELW_TITLE_NONE|ELW_SHOW|ELW_SHOW_LAST);
			set_window_handler(stats_bar_win, ELW_HANDLER_DISPLAY, &display_stats_bar_handler);
			set_window_handler(stats_bar_win, ELW_HANDLER_MOUSEOVER, &mouseover_stats_bar_handler);
			//set_window_handler(stats_bar_win, ELW_HANDLER_CLICK, &click_stats_bar_handler);
		}
	else
		{
			init_window(stats_bar_win, 0, 0, 24, window_height-44, window_width-24-64, 12);
		}

	mana_bar_start_x=0;
	mana_bar_start_y=0;

	food_bar_start_x=mana_bar_start_x+100+40;
	food_bar_start_y=mana_bar_start_y;

	health_bar_start_x=food_bar_start_x+100+40;
	health_bar_start_y=mana_bar_start_y;

	load_bar_start_x=health_bar_start_x+100+40;
	load_bar_start_y=mana_bar_start_y;

	exp_bar_start_x=load_bar_start_x+100+70;
	exp_bar_start_y=mana_bar_start_y;
}

void draw_stats_bar(int x, int y, int val, int len, float r, float g, float b, float r2, float g2, float b2)
{
	unsigned char	buf[32];

	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	//draw the colored section
	glColor3f(r2, g2, b2);
	glVertex3i(x, y+8, 0);
	glColor3f(r, g, b);
	glVertex3i(x, y, 0);
	glColor3f(r, g, b);
	glVertex3i(x+len, y, 0);
	glColor3f(r2, g2, b2);
	glVertex3i(x+len, y+8, 0);
	glEnd();

	// draw the bar frame
	glColor3f(0.77f, 0.57f, 0.39f);
	glBegin(GL_LINES);
	glVertex3i(x, y, 0);
	glVertex3i(x+100, y, 0);
	glVertex3i(x+100, y, 0);
	glVertex3i(x+100, y+8, 0);
	glVertex3i(x+100, y+8, 0);
	glVertex3i(x, y+8, 0);
	glVertex3i(x, y+8, 0);
	glVertex3i(x, y, 0);
	glEnd();
	glEnable(GL_TEXTURE_2D);

	// handle the text
	snprintf(buf, 32, "%d", val);
	glColor3f(0.8f, 0.8f, 0.8f);
	draw_string_small(x-(1+8*strlen(buf)), y-3, buf, 1);
}

void draw_stats_display()
{
    display_window(stats_bar_win);
}

int	display_stats_bar_handler(window_info *win)
{
	float health_adjusted_x_len;
	float food_adjusted_x_len;
	float mana_adjusted_x_len;
	float load_adjusted_x_len;

	//get the adjusted lenght

	if(!your_info.material_points.cur || !your_info.material_points.base)
		health_adjusted_x_len=0;//we don't want a div by 0
	else
		health_adjusted_x_len=100/((float)your_info.material_points.base/(float)your_info.material_points.cur);

	if(your_info.food_level<=0)
		food_adjusted_x_len=0;//we don't want a div by 0
	else
		food_adjusted_x_len=100/(45.0f/(float)your_info.food_level);

	if(!your_info.ethereal_points.cur || !your_info.ethereal_points.base)
		mana_adjusted_x_len=0;//we don't want a div by 0
	else
		mana_adjusted_x_len=100/((float)your_info.ethereal_points.base/(float)your_info.ethereal_points.cur);

	if(!your_info.carry_capacity.cur || !your_info.carry_capacity.base)
		load_adjusted_x_len=0;//we don't want a div by 0
	else
		load_adjusted_x_len=100/((float)your_info.carry_capacity.base/(float)your_info.carry_capacity.cur);

	draw_stats_bar(health_bar_start_x, health_bar_start_y, your_info.material_points.cur, health_adjusted_x_len, 1.0f, 0.2f, 0.2f, 0.5f, 0.2f, 0.2f);
	draw_stats_bar(food_bar_start_x, food_bar_start_y, your_info.food_level, food_adjusted_x_len, 1.0f, 1.0f, 0.2f, 0.5f, 0.5f, 0.2f);
	draw_stats_bar(mana_bar_start_x, mana_bar_start_y, your_info.ethereal_points.cur, mana_adjusted_x_len, 0.2f, 0.2f, 1.0f, 0.2f, 0.2f, 0.5f);
	draw_stats_bar(load_bar_start_x, load_bar_start_y, your_info.carry_capacity.base-your_info.carry_capacity.cur, load_adjusted_x_len, 0.6f, 0.4f, 0.4f, 0.4f, 0.2f, 0.2f);
	if(win->len_x>640-64) draw_exp_display();
	
	if(show_help_text)
		{
        		if(mouse_in_window(win->window_id, mouse_x, mouse_y))
				{
					if(statbar_cursor_x>health_bar_start_x && statbar_cursor_x < health_bar_start_x+100) show_help(attributes.material_points.name,health_bar_start_x+110,-3);
					if(statbar_cursor_x>food_bar_start_x && statbar_cursor_x < food_bar_start_x+100) show_help(attributes.food.name,food_bar_start_x+110,-3);
					if(statbar_cursor_x>mana_bar_start_x && statbar_cursor_x < mana_bar_start_x+100) show_help(attributes.ethereal_points.name,mana_bar_start_x+110,-3);
					if(statbar_cursor_x>load_bar_start_x && statbar_cursor_x < load_bar_start_x+100) show_help(attributes.carry_capacity.name,load_bar_start_x+110,-3);
				}
		}

	return 1;
}

int mouseover_stats_bar_handler(window_info *win, int mx, int my)
{
	statbar_cursor_x=mx;
	return 0;
}

int check_stats_display()
{
	return 0;
}


// the misc section (compass, clock, ?)
float compass_u_start=(float)32/256;
float compass_v_start=1.0f-(float)192/256;

float compass_u_end=(float)95/256;
float compass_v_end=0;

float clock_u_start=0;
float clock_v_start=1.0f-(float)128/256;

float clock_u_end=(float)63/256;
float clock_v_end=1.0f-(float)191/256;

float needle_u_start=(float)4/256;
float needle_v_start=1.0f-(float)200/256;

float needle_u_end=(float)14/256;
float needle_v_end=1.0f-(float)246/256;

float clock_needle_u_start=(float)21/256;
float clock_needle_v_start=1.0f-(float)192/256;

float clock_needle_u_end=(float)31/256;
float clock_needle_v_end=1.0f-(float)223/256;


void init_misc_display()
{
	//create the misc window
	if(misc_win <= 0)
		{
			misc_win= create_window("Misc", 0, 0, window_width-64, window_height-145, 64, 145, ELW_TITLE_NONE|ELW_SHOW|ELW_SHOW_LAST);
			set_window_handler(misc_win, ELW_HANDLER_DISPLAY, &display_misc_handler);
			set_window_handler(misc_win, ELW_HANDLER_CLICK, &click_misc_handler);
		}
	else
		{
			move_window(misc_win, 0, 0, window_width-64, window_height-145);
		}
}

void draw_misc_display()
{
    display_window(misc_win);
}

int	display_misc_handler(window_info *win)
{
	get_and_set_texture_id(hud_text);
    //draw the compass
	glBegin(GL_QUADS);
	draw_2d_thing(compass_u_start, compass_v_start, compass_u_end, compass_v_end, 0,win->len_y-64,64,win->len_y);
	glEnd();

	//draw the compass needle
    glEnable(GL_ALPHA_TEST);//enable alpha filtering, so we have some alpha key
    glAlphaFunc(GL_GREATER, 0.09f);
	glPushMatrix();
	glTranslatef(32, win->len_y-32, 0);
	glRotatef(compass_direction*rz, 0.0f, 0.0f, 1.0f);

	glBegin(GL_QUADS);
	draw_2d_thing(needle_u_start, needle_v_start, needle_u_end, needle_v_end,-5, -28, 5, 28);
	glEnd();
	glPopMatrix();
	glDisable(GL_ALPHA_TEST);

	//draw the clock
	glBegin(GL_QUADS);
	draw_2d_thing(clock_u_start, clock_v_start, clock_u_end, clock_v_end,
				  0, win->len_y-128, 64, win->len_y-64);
	glEnd();

	//draw the clock needle
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.05f);
	glPushMatrix();
	glTranslatef(32, win->len_y-96, 0);
	glRotatef(game_minute, 0.0f, 0.0f, 1.0f);
	glBegin(GL_QUADS);
	draw_2d_thing(clock_needle_u_start, clock_needle_v_start, clock_needle_u_end, clock_needle_v_end, -5, -24,5, 6);
	glEnd();
	glPopMatrix();
	glDisable(GL_ALPHA_TEST);

	//Digital Clock
	if(view_digital_clock > 0){
		char str[6];	// one extra incase the length of the day ever changes
		int	x;

		snprintf(str, 6, "%1d:%02d", game_minute/60, game_minute%60);
		x= 3+(win->len_x - (get_string_width(str)*11)/12)/2;
		glColor3f(0.77f, 0.57f, 0.39f);
		draw_string(x, 2, str, 1);
	}
	if(show_stats_in_hud>0)
		{
			if(video_mode>2)draw_stats();
		}
	return	1;
}

int check_misc_display()
{
	return(click_in_window(misc_win, mouse_x, mouse_y, 0));
}

int	click_misc_handler(window_info *win, int mx, int my, Uint32 flags)
{
    //check to see if we clicked on the clock
	if(my>win->len_y-128 && my<win->len_y-64)
		{
			unsigned char protocol_name;

			protocol_name= GET_TIME;
			my_tcp_send(my_socket,&protocol_name,1);
			return 1;
		}
	//check to see if we clicked on the compass
	if(my>win->len_y-64 && my<win->len_y)
		{
			unsigned char protocol_name;

			protocol_name= LOCATE_ME;
			my_tcp_send(my_socket,&protocol_name,1);
			return 1;
		}

	return 0;
}

int quickbar_x_len= 30;
int quickbar_y_len= 6*30;
int quickbar_x=34;
int quickbar_y=64;
int quickbar_draggable=0;
int quickbar_dir=VERTICAL;
int quickbar_relocatable=0;


//quickbar section
void init_quickbar() {
	quickbar_x_len= 30;
	quickbar_y_len= 6*30+1;
	if(quickbar_win <= 0)
		{
			if(quickbar_dir==VERTICAL)
				quickbar_win= create_window("Quickbar", 0, 0, window_width-quickbar_x, quickbar_y, quickbar_x_len, quickbar_y_len, quickbar_draggable?ELW_TITLE_BAR|ELW_SHOW|ELW_USE_BACKGROUND|ELW_USE_BORDER|ELW_SHOW_LAST|ELW_DRAGGABLE:ELW_TITLE_NONE|ELW_SHOW|ELW_USE_BACKGROUND|ELW_USE_BORDER|ELW_SHOW_LAST);
			else
				quickbar_win= create_window("Quickbar", 0, 0, window_width-quickbar_x, quickbar_y, quickbar_y_len, quickbar_x_len, quickbar_draggable?ELW_TITLE_BAR|ELW_SHOW|ELW_USE_BACKGROUND|ELW_USE_BORDER|ELW_SHOW_LAST|ELW_DRAGGABLE:ELW_TITLE_NONE|ELW_SHOW|ELW_USE_BACKGROUND|ELW_USE_BORDER|ELW_SHOW_LAST);
			set_window_handler(quickbar_win, ELW_HANDLER_DISPLAY, &display_quickbar_handler);
			set_window_handler(quickbar_win, ELW_HANDLER_CLICK, &click_quickbar_handler);
		}
	else
		{
			if(quickbar_draggable) show_window(quickbar_win);
			else if(quickbar_y>window_height||quickbar_x>window_width) move_window(quickbar_win, 0, 0, 200, 64);//The player has done something stupid... let him/her correct it
			else move_window(quickbar_win, 0, 0, window_width-quickbar_x, quickbar_y);
		}
}

void draw_quickbar() {
	// failsafe until better integrated
	if(quickbar_dir==VERTICAL) {
		init_window(quickbar_win, 0, 0, window_width-quickbar_x, quickbar_y, quickbar_x_len, quickbar_y_len);
		if(quickbar_draggable) change_flags(quickbar_win, (ELW_TITLE_BAR|ELW_SHOW|ELW_USE_BACKGROUND|ELW_USE_BORDER|ELW_SHOW_LAST|ELW_DRAGGABLE));
	}
	else if(quickbar_dir==HORIZONTAL) {
		init_window(quickbar_win, 0, 0, window_width-quickbar_x, quickbar_y, quickbar_y_len, quickbar_x_len);
		if(quickbar_draggable) change_flags(quickbar_win, (ELW_TITLE_BAR|ELW_SHOW|ELW_USE_BACKGROUND|ELW_USE_BORDER|ELW_SHOW_LAST|ELW_DRAGGABLE));

	} 
	display_window(quickbar_win);
}

int	display_quickbar_handler(window_info *win)
{
	Uint8 str[80];
	int y, i;

	glDisable(GL_TEXTURE_2D);
	glBegin(GL_LINES);
	use_window_color(quickbar_win, ELW_COLOR_LINE);
	//draw the grid
	if(quickbar_dir==VERTICAL)
		{
			for(y=1;y<6;y++)
				{
					glVertex3i(0, y*30+1, 0);
					glVertex3i(quickbar_x_len, y*30+1, 0);
				}
		}
	else
		{
			for(y=1;y<6;y++)
				{
					glVertex3i(y*30+1, 0, 0);
					glVertex3i(y*30+1, quickbar_x_len, 0);
				}
		}
	glEnd();
	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0f,1.0f,1.0f);
	//ok, now let's draw the objects...
	for(i=0;i<ITEM_NUM_ITEMS;i++)
		{
			if(item_list[i].quantity > 0)
				{
					float u_start,v_start,u_end,v_end;
					int this_texture,cur_item,cur_pos;
					int x_start,x_end,y_start,y_end;

					//get the UV coordinates.
					cur_item=item_list[i].image_id%25;
					u_start=0.2f*(cur_item%5);
					u_end=u_start+(float)50/256;
					v_start=(1.0f+((float)50/256)/256.0f)-((float)50/256*(cur_item/5));
					v_end=v_start-(float)50/256;

					//get the x and y
					cur_pos=item_list[i].pos;
					if(cur_pos<6)//don't even check worn items
						{
							x_start= 1;
							x_end= x_start+29;
							y_start= 30*(cur_pos%6)+1;
							y_end= y_start+29;

							//get the texture this item belongs to
							this_texture=item_list[i].image_id/25;
							switch(this_texture) {
							case 0:
								this_texture=items_text_1;break;
							case 1:
								this_texture=items_text_2;break;
							case 2:
								this_texture=items_text_3;break;
							case 3:
								this_texture=items_text_4;break;
							case 4:
								this_texture=items_text_5;break;
							case 5:
								this_texture=items_text_6;break;
							case 6:
								this_texture=items_text_7;break;
							case 7:
								this_texture=items_text_8;break;
							case 8:
								this_texture=items_text_9;break;
							}

							get_and_set_texture_id(this_texture);
							glBegin(GL_QUADS);
							if(quickbar_dir==VERTICAL)
								{
									draw_2d_thing(u_start,v_start,u_end,v_end,x_start,y_start,x_end,y_end);
								}
							else
								{
									draw_2d_thing(u_start,v_start,u_end,v_end,y_start+1,x_start+1,y_end-1,x_end-1);
								}
							glEnd();
							sprintf(str,"%i",item_list[i].quantity);
							if(quickbar_dir==VERTICAL) draw_string_small(x_start,y_end-15,str,1);
							else draw_string_small(y_start,x_end-15,str,1);
						}
				}
		}
	return 1;
}

int check_quickbar() {
	return(click_in_window(quickbar_win, mouse_x, mouse_y, 0));
}

int	click_quickbar_handler(window_info *win, int mx, int my, Uint32 flags)
{
	int i,y;
	int x_screen,y_screen;
	Uint8 str[100];
	int trigger=ELW_LEFT_MOUSE|ELW_CTRL|ELW_SHIFT;//flags we'll use for the quickbar relocation handling

	// no in window check needed, already done
	//see if we clicked on any item in the main category
	for(y=0;y<6;y++)
		{
			if(quickbar_dir==VERTICAL)
				{
					x_screen=0;
					y_screen=y*30;
				}
			else
				{
					x_screen=y*30;
					y_screen=0;
				}
			if(mx>x_screen && mx<x_screen+30 && my>y_screen && my<y_screen+30)
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
					if(quickbar_relocatable>0)
						{
							if((flags&trigger)==(ELW_LEFT_MOUSE|ELW_CTRL))
								{
									//toggle draggable
									if((get_flags(quickbar_win)!=(ELW_TITLE_BAR|ELW_SHOW|ELW_USE_BACKGROUND|ELW_USE_BORDER|ELW_SHOW_LAST|ELW_DRAGGABLE)))
										{
											change_flags(quickbar_win, (ELW_TITLE_BAR|ELW_SHOW|ELW_USE_BACKGROUND|ELW_USE_BORDER|ELW_SHOW_LAST|ELW_DRAGGABLE));
											quickbar_draggable=1;
										}
									else 
										{
											change_flags(quickbar_win, (ELW_TITLE_NONE|ELW_SHOW|ELW_USE_BACKGROUND|ELW_USE_BORDER|ELW_SHOW_LAST));
											quickbar_draggable=0;
											quickbar_x=window_width-windows_list.window[quickbar_win].cur_x;
											quickbar_y=windows_list.window[quickbar_win].cur_y;
										}
								}
							else if (((flags&trigger)==(ELW_LEFT_MOUSE|ELW_SHIFT)) && (get_flags(quickbar_win)==(ELW_TITLE_BAR|ELW_SHOW|ELW_USE_BACKGROUND|ELW_USE_BORDER|ELW_SHOW_LAST|ELW_DRAGGABLE)))
								{
									//toggle vertical/horisontal
									flip_quickbar();
								}
							else if (((flags&trigger)==trigger))
								{
									//reset
									reset_quickbar();
								}
						}
					//see if there is any item there
					for(i=0;i<ITEM_NUM_ITEMS;i++)
						{
							//should we get the info for it?
							if(item_list[i].quantity && item_list[i].pos==y)
								{

									if(action_mode==action_look || (flags&ELW_RIGHT_MOUSE))
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

/*Change the quickbar from vertical to horizontal, or vice versa*/
void flip_quickbar() 
{
	if (quickbar_dir==VERTICAL) 
		{
			quickbar_dir=HORIZONTAL;
			init_window(quickbar_win, 0, 0, windows_list.window[quickbar_win].cur_x, windows_list.window[quickbar_win].cur_y, quickbar_y_len, quickbar_x_len);
		}      
	else if (quickbar_dir==HORIZONTAL) 
		{
			quickbar_dir=VERTICAL;
			init_window(quickbar_win, 0, 0, windows_list.window[quickbar_win].cur_x, windows_list.window[quickbar_win].cur_y, quickbar_x_len, quickbar_y_len);
		}
}

/*Return the quickbar to it's Built-in position*/
void reset_quickbar() 
{
	//Necessary Variables
	quickbar_x_len= 30;
	quickbar_y_len= 6*30+1;
	quickbar_x= quickbar_x_len+4;
	quickbar_y= 64;
	//Re-set to default orientation
	quickbar_dir=VERTICAL;
	quickbar_draggable=0;
	init_window(quickbar_win, 0, 0, quickbar_x, quickbar_y, quickbar_x_len, quickbar_y_len);
	//Re-set  Flags
	change_flags(quickbar_win, ELW_TITLE_NONE|ELW_SHOW|ELW_USE_BACKGROUND|ELW_USE_BORDER|ELW_SHOW_LAST);   
	//NEED x_offset
	move_window(quickbar_win, 0, 0, window_width-quickbar_x, 64);
}


Uint32 exp_lev[140];

void build_levels_table()
{
  int i;
  int exp=100;

  exp_lev[0]=0;
  for(i=1;i<120;i++)
    {
        if(i<=10)exp+=exp*40/100;
        else
        if(i<=20)exp+=exp*30/100;
        else
        if(i<=30)exp+=exp*20/100;
        else
        if(i<=40)exp+=exp*14/100;
        else
        if(i<=90)exp+=exp*7/100;
        else exp+=exp*4/100;

        exp_lev[i]=exp;
    }
}

void draw_stats()
{
	char str[20];
	int y=-20;
	int x=6;
	glColor3f(1.0f,1.0f,1.0f);
	sprintf(str,"%-4s %2i",attributes.attack_skill.shortname,your_info.attack_skill.base);
	draw_string_small(x, y, str, 1);
	y-=15;
	sprintf(str,"%-4s %2i",attributes.defense_skill.shortname,your_info.defense_skill.base);
	draw_string_small(x, y, str, 1);
	y-=15;
	sprintf(str,"%-4s %2i",attributes.harvesting_skill.shortname,your_info.harvesting_skill.base);
	draw_string_small(x, y, str, 1);
	y-=15;
	sprintf(str,"%-4s %2i",attributes.alchemy_skill.shortname,your_info.alchemy_skill.base);
	draw_string_small(x, y, str, 1);
	y-=15;
	sprintf(str,"%-4s %2i",attributes.magic_skill.shortname,your_info.magic_skill.base);
	draw_string_small(x, y, str, 1);
	y-=15;
	sprintf(str,"%-4s %2i",attributes.potion_skill.shortname,your_info.potion_skill.base);
	draw_string_small(x, y, str, 1);
	y-=15;
	sprintf(str,"%-4s %2i",attributes.summoning_skill.shortname,your_info.summoning_skill.base);
	draw_string_small(x, y, str, 1);
	y-=15;
	sprintf(str,"%-4s %2i",attributes.manufacturing_skill.shortname,your_info.manufacturing_skill.base);
	draw_string_small(x, y, str, 1);
	y-=15;
	sprintf(str,"%-4s %2i",attributes.crafting_skill.shortname,your_info.crafting_skill.base);
	draw_string_small(x, y, str, 1);
	y-=15;
	sprintf(str,"%-4s %2i",attributes.overall_skill.shortname,your_info.overall_skill.base);
	draw_string_small(x, y, str, 1);
}

void draw_exp_display()
{
	int exp_adjusted_x_len;
	int nl_exp, baselev, cur_exp;
	int delta_exp;
	char * name;
	float prev_exp;

	switch(watch_this_stat){
	case 1: // attack
		cur_exp = your_info.attack_exp;
		baselev = your_info.attack_skill.base;
		name = attributes.attack_skill.name;
		break;
	case 2: // defense
		cur_exp = your_info.defense_exp;
		baselev = your_info.defense_skill.base;
		name = attributes.defense_skill.name;
		break;
	case 3: // harvest
		cur_exp = your_info.harvesting_exp;
		baselev = your_info.harvesting_skill.base;
		name = attributes.harvesting_skill.name;
		break;
	case 4: // alchemy
		cur_exp = your_info.alchemy_exp;
		baselev = your_info.alchemy_skill.base;
		name = attributes.alchemy_skill.name;
		break;
	case 5: // magic
		cur_exp = your_info.magic_exp;
		baselev = your_info.magic_skill.base;
		name = attributes.magic_skill.name;
		break;
	case 6: // potion
		cur_exp = your_info.potion_exp;
		baselev = your_info.potion_skill.base;
		name = attributes.potion_skill.name;
		break;
	case 7: // summoning
		cur_exp = your_info.summoning_exp;
		baselev = your_info.summoning_skill.base;
		name = attributes.summoning_skill.name;
		break;
	case 8: // manufacture
		cur_exp = your_info.manufacturing_exp;
		baselev = your_info.manufacturing_skill.base;
		name = attributes.manufacturing_skill.name;
		break;
	case 9: // crafting
		cur_exp = your_info.crafting_exp;
		baselev = your_info.crafting_skill.base;
		name = attributes.crafting_skill.name;
		break;
	case 10: // overall
	default:
		cur_exp = your_info.overall_exp;
		baselev = your_info.overall_skill.base;
		name = attributes.overall_skill.name;
	}

	if(!baselev)
		prev_exp=0;
	else
		prev_exp=exp_lev[baselev];

	nl_exp=exp_lev[baselev+1];
	delta_exp=nl_exp-prev_exp;

	if(!cur_exp || !nl_exp)
		exp_adjusted_x_len = 0;
	else
		//exp_bar_length = (int)( (((float)cur_exp - prev_exp) / ((float)nl_exp - prev_exp)) * 100.0);
		exp_adjusted_x_len = 100-100.0f/(float)((float)delta_exp/(float)(nl_exp-cur_exp));

	draw_stats_bar(exp_bar_start_x, exp_bar_start_y, nl_exp - cur_exp, exp_adjusted_x_len, 0.1f, 0.8f, 0.1f, 0.1f, 0.4f, 0.1f);
	draw_string_small(exp_bar_start_x, exp_bar_start_y+10, name, 1);
}

/*Flag manipulation-I hate doing this by hand*/
/*Change flags*/
void change_flags(int win_id, Uint32 flags) {
	windows_list.window[win_id].flags=flags;     
}

/*Return flags*/
Uint32 get_flags(int win_id) {
	return windows_list.window[win_id].flags;
}
