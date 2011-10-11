#ifndef __INTERFACE_H__
#define __INTERFACE_H__

#include <SDL_keysym.h>
#include "../platform.h"

//modes
#define mode_tile 0
#define mode_2d 1
#define mode_3d 2
#define mode_light 3
#define mode_height 4
#define mode_map 5
#define mode_particles 6
#define mode_eye_candy 7

//tools
#define tool_kill 0
#define tool_new 1
#define tool_select 2
#define tool_clone 3

extern int mouse_x;
extern int mouse_y;
extern int mouse_delta_x;
extern int mouse_delta_y;
extern int right_click;
extern int left_click;
extern int middle_click;

extern float scene_mouse_x;
extern float scene_mouse_y;


extern int cur_mode;
extern int cur_tool;

extern int view_tile;
extern int view_2d;
extern int view_3d;
extern int view_light;
extern int view_height;
extern int view_particles;
extern int view_particle_handles;
extern int view_eye_candy;

extern int selected_3d_object;
extern int selected_2d_object;
extern int selected_light;
extern int selected_particles_object;
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

extern int map_has_changed;
extern GLuint minimap_tex;
extern int show_position_on_minimap;

int check_interface_buttons();
void get_world_x_y();
void Enter2DMode();
void Leave2DMode();
void draw_2d_thing(float u_start,float v_start,float u_end,float v_end,int x_start,int y_start,int x_end,int y_end);
void draw_toolbar();
void draw_3d_obj_info();
void draw_2d_obj_info();
void draw_light_info();
void draw_height_info();
void display_tiles_list();
void display_heights_list();
void check_mouse_minimap();
void draw_mouse_minimap();
void draw_minimap();
void display_new_map_menu();
void display_map_settings();

#endif

