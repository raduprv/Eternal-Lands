#ifndef __INTERFACE_H__
#define __INTERFACE_H__

extern int have_a_map;
extern int auto_camera;

#define action_walk 0
#define action_look 1
#define action_use 2
#define action_harvest 3
#define action_pick 4
#define action_trade 5
#define action_attack 6

extern int action_mode;

extern int mouse_x;
extern int mouse_y;
extern int mouse_delta_x;
extern int mouse_delta_y;

extern int right_click;
extern int left_click;

extern int view_clock;
extern int view_compas;
extern int options_menu;
extern int combat_grid;
extern int view_self_stats;
extern int view_health_bar;
extern int view_names;

extern int login_screen_menus;

#define interface_game 0
#define interface_log_in 1
#define interface_new_char 2
#define interface_console 3
#define interface_opening 4
#define interface_map 5

extern char interface_mode;
extern char username_box_selected;
extern char password_box_selected;


extern char username_str[16];
extern char password_str[16];
extern char display_password_str[16];
extern int username_text_lenght;
extern int password_text_lenght;

extern int font_text;
extern int cons_text;
extern int icons_text;
extern int open_text;
extern int login_text;

extern float marble_menu_u_start;
extern float marble_menu_v_start;
extern float marble_menu_u_end;
extern float marble_menu_v_end;

extern float close_button_u_start;
extern float close_button_v_start;
extern float close_button_u_end;
extern float close_button_v_end;

extern int options_menu_x;
extern int options_menu_y;
extern int options_menu_x_len;
extern int options_menu_y_len;
extern int options_menu_dragged;


typedef struct
{
	char supported;
	char selected;
}mode_flag;


void get_world_x_y();
int check_drag_menus();
void check_menus_out_of_screen();
void check_mouse_click();
void Enter2DMode();
void Leave2DMode();
void build_video_mode_array();
void draw_console_pic(int which_texture);
void init_opening_interface();
void draw_login_screen();
void add_char_to_username(unsigned char ch);
void add_char_to_password(unsigned char ch);
void init_peace_icons_position();
void draw_peace_icons();
int check_peace_menu();
void draw_2d_thing(float u_start,float v_start,float u_end,float v_end,int x_start,
				   int y_start,int x_end,int y_end);
void draw_options_menu();
int check_options_menu();
void draw_ingame_interface();
void switch_to_game_map();
void switch_from_game_map();
void draw_game_map();

void draw_menu_title_bar(int x, int y, int x_len);

#endif

