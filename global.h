#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#ifdef WINDOWS
#include <SDL.h>
#include <SDL_net.h>
#include <SDL_thread.h>
#include <SDL_mixer.h>
#else
#include <SDL/SDL.h>
#include <SDL/SDL_net.h>
#include <SDL/SDL_thread.h>
#include <SDL/SDL_mixer.h>
#endif

#include "elc_private.h"
#include "SDL_opengl.h"
#include "asc.h"
#include "md2.h"
#include "actors.h"
#include "new_actors.h"
#include "actor_scripts.h"
#include "e3d.h"
#include "errors.h"
#include "2d_objects.h"
#include "3d_objects.h"
#include "tiles.h"
#include "lights.h"
#include "client_serv.h"
#include "multiplayer.h"
#include "text.h"
#include "interface.h"
#include "map_io.h"
#include "reflection.h"
#include "shadows.h"
#include "particles.h"
#include "spells.h"
#include "sound.h"
#include "ignore.h"
#include "filter.h"
#include "help.h"
#include "weather.h"
#include "stats.h"
#include "items.h"
#include "dialogues.h"
#include "draw_scene.h"
#include "colors.h"
#include "console.h"
#include "cursors.h"
#include "events.h"
#include "font.h"
#include "gl_init.h"
#include "manufacture.h"
#include "misc.h"
#include "paste.h"
#include "textures.h"
#include "trade.h"
#include "new_character.h"

//cursors
#define CURSOR_EYE 0
#define CURSOR_TALK 1
#define CURSOR_ATTACK 2
#define CURSOR_ENTER 3
#define CURSOR_PICK 4
#define CURSOR_HARVEST 5
#define CURSOR_WALK 6
#define CURSOR_ARROW 7
#define CURSOR_TRADE 8
#define CURSOR_MAGIC 9
#define CURSOR_USE 10

#define sector_size_x 15
#define sector_size_y 15

extern int map_meters_size_x;
extern int map_meters_size_y;
extern float texture_scale;

extern int window_width;
extern int window_height;

extern int desktop_width;
extern int desktop_height;

extern int bpp;
extern int video_mode;
extern int full_screen;
extern int have_stencil;
extern int poor_man;
extern int show_reflection;
extern int mouse_limit;

extern Uint32 cur_time, last_time;
extern int server_time_stamp;
extern int client_time_stamp;
extern int client_server_delta_time;

extern float cx,cy,cz;
extern float rx,ry,rz;
extern float camera_rotation_speed;
extern int camera_rotation_frames;
extern int normal_animation_timer;
extern double camera_x_speed;
extern int camera_x_frames;
extern double camera_y_speed;
extern int camera_y_frames;
extern double camera_z_speed;
extern int camera_z_frames;
extern float fine_camera_rotation_speed;
extern float normal_camera_rotation_speed;

extern float scene_mouse_x;
extern float scene_mouse_y;

extern float terrain_scale;
extern int last_texture;

extern char create_char_error_str[520];
extern char log_in_error_str[520];

extern int cur_lake_waves_time,last_lake_waves_time;

typedef struct
{
	int texture_id;
	int last_access_time;
    char file_name[100];
	unsigned char alpha;
}texture_cache_struct;

extern texture_cache_struct texture_cache[1000];

extern  Uint8 *e3d_file_mem;
extern  Uint8 *handle_e3d_file_mem;

//for the lights
#define global_lights_no 60
extern GLfloat global_lights[global_lights_no][4];

typedef struct
{
	Uint8 r1;
	Uint8 g1;
	Uint8 b1;
	Uint8 r2;
	Uint8 g2;
	Uint8 b2;
	Uint8 r3;
	Uint8 g3;
	Uint8 b3;
	Uint8 r4;
	Uint8 g4;
	Uint8 b4;

} color_rgb;

extern color_rgb colors_list[30];

extern int debug_info;
extern float debug_float;
extern int triangles_normal;
extern int triangles_shadow;

//others
extern int disconnected;
extern int exit_now;
extern int have_url;
extern char current_url[160];
extern char broswer_name[120];
extern int no_adjust_shadows;
extern int clouds_shadows;
extern int selected_3d_object;
extern int selected_inventory_object;

extern int no_bounding_box;

//some prototypes, that won't fit somewhere else
Uint32 my_timer(unsigned int some_int);
int SphereInFrustum(float x, float y, float z, float radius);
int check_tile_in_frustrum(float x,float y);
void draw_ingame_string(float x, float y,unsigned char * our_string,int max_lines,int big);
void init_colors();

#ifdef WINDOWS
extern PFNGLMULTITEXCOORD2FARBPROC		glMultiTexCoord2fARB;
extern PFNGLMULTITEXCOORD2FVARBPROC		glMultiTexCoord2fvARB;
extern PFNGLACTIVETEXTUREARBPROC		glActiveTextureARB;
extern PFNGLCLIENTACTIVETEXTUREARBPROC	glClientActiveTextureARB;
#endif

extern int shift_on;
extern int alt_on;
extern int ctrl_on;
extern int read_mouse_now;

extern int have_multitexture;
float clouds_movement_u;
float clouds_movement_v;
extern int last_clear_clouds;
extern int reflection_texture;

#define UNDER_MOUSE_NPC 0
#define UNDER_MOUSE_PLAYER 1
#define UNDER_MOUSE_ANIMAL 2
#define UNDER_MOUSE_3D_OBJ 3
#define UNDER_MOUSE_NOTHING 4
#define UNDER_MOUSE_MENU 5
#define UNDER_MOUSE_NO_CHANGE 6

extern int object_under_mouse;
extern int thing_under_the_mouse;
extern int current_cursor;

struct cursors_struct
{
	int hot_x;
	int hot_y;
	Uint8 *cursor_pointer;
};
extern struct cursors_struct cursors_array[20];

struct harvest_names_struct
{
	char name[80];
};
extern struct harvest_names_struct harvestable_objects[100];

struct enter_names_struct
{
	char name[80];
};
extern struct enter_names_struct entrable_objects[100];


#endif

