#ifndef __INTERFACE_H__
#define __INTERFACE_H__

//modes
#define mode_tile 0
#define mode_2d 1
#define mode_3d 2
#define mode_light 3
#define mode_height 4
#define mode_map 5

//tools
#define tool_kill 0
#define tool_new 1
#define tool_select 2
#define tool_clone 3

extern int mouse_x;
extern int mouse_y;
extern int right_click;
extern int left_click;

extern float scene_mouse_x;
extern float scene_mouse_y;


extern int cur_mode;
extern int cur_tool;

extern int view_tile;
extern int view_2d;
extern int view_3d;
extern int view_light;
extern int view_height;

extern int selected_3d_object;
extern int selected_2d_object;
extern int selected_light;
extern int selected_tile;
extern int selected_height;
extern char move_tile_a_tile;
extern char move_tile_a_height;
extern int tiles_no;
extern int tile_offset;
extern char view_tiles_list;
extern char view_heights_list;
extern char view_new_map_menu;

extern float x_tile_menu_offset;
extern float y_tile_menu_offset;


extern SDLMod mod_key_status;
extern char shift_on;
extern char ctrl_on;
extern char alt_on;

extern int buttons_text;

#endif

