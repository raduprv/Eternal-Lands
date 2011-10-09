#include "e3d.h"
#include "global.h"

int have_arb_compression=0;
int have_s3_compression=0;
int elwin_mouse;

int map_meters_size_x;
int map_meters_size_y;

int window_width=800;
int window_height=570;

int bpp;

Uint32 cur_time=0, last_time=0;//for FPS
char exec_path[256];

int video_mode=2;
int limit_fps=60;
int auto_save_time=0;

float camera_x_end_point;
float camera_z_end_point;

int last_texture=-2;

float gcr=0.8f,gcg=0.8f,gcb=0.8f;

#ifndef	NEW_TEXTURES
texture_cache_struct texture_cache[TEXTURE_CACHE_MAX];
#endif	// NEW_TEXTURES
e3d_cache_struct e3d_cache[1000];
obj_2d_cache_struct obj_2d_def_cache[MAX_OBJ_2D_DEF];

object3d *objects_list[MAX_OBJ_3D];
obj_2d *obj_2d_list[MAX_OBJ_2D];

  Uint8 *e3d_file_mem;
  Uint8 *handle_e3d_file_mem;

//lights
GLfloat global_lights[global_lights_no][4];

GLfloat sky_lights_c1[global_lights_no*2][4];
GLfloat sky_lights_c2[global_lights_no*2][4];
GLfloat sky_lights_c3[global_lights_no*2][4];
GLfloat sky_lights_c4[global_lights_no*2][4];


GLfloat light_0_position[4];
GLfloat light_0_diffuse[4];
GLfloat light_0_dist;

GLfloat light_1_position[4];
GLfloat light_1_diffuse[4];
GLfloat light_1_dist;

GLfloat light_2_position[4];
GLfloat light_2_diffuse[4];
GLfloat light_2_dist;

GLfloat light_3_position[4];
GLfloat light_3_diffuse[4];
GLfloat light_3_dist;

GLfloat light_4_position[4];
GLfloat light_4_diffuse[4];
GLfloat light_4_dist;

GLfloat light_5_position[4];
GLfloat light_5_diffuse[4];
GLfloat light_5_dist;

GLfloat light_6_position[4];
GLfloat light_6_diffuse[4];
GLfloat light_6_dist;

light *lights_list[MAX_LIGHTS];
sun sun_pos[60*3];
char lights_on=1;
unsigned char light_level=0;
int game_minute=60;

//tile map things
unsigned char *tile_map;
int tile_map_size_x;
int tile_map_size_y;
int tile_list[256];

char dungeon=0;//no sun
float ambient_r=0;
float ambient_g=0;
float ambient_b=0;

//interface
int mouse_x;
int mouse_y;
int mouse_delta_x;
int mouse_delta_y;
int right_click;
int left_click;
int middle_click;

int icons_text;

float scene_mouse_x;
float scene_mouse_y;

int cur_mode=mode_3d;
int cur_tool=tool_select;
int view_tile=1;
int view_2d=1;
int view_3d=1;
int view_particles=1;
int view_particle_handles=1;
int view_eye_candy=1;
int view_light=1;
int view_height=0;
int selected_3d_object=-1;
int selected_2d_object=-1;
int selected_particles_object=-1;
int selected_light=-1;
int selected_tile=255;
int selected_height=-1;
char move_tile_a_tile=0;
char move_tile_a_height=0;
int tiles_no=0;
int tile_offset=0;
char view_tiles_list=0;
float x_tile_menu_offset=64;
float y_tile_menu_offset=128;
char view_new_map_menu=0;
char view_grid=0;

SDLMod mod_key_status;
char shift_on=0;
char ctrl_on=0;
char alt_on=0;

int buttons_text;


//editor things
unsigned char *height_map;
char heights_3d=0;
char minimap_on=0;
int new_map_menu = -1;

//shadows

float fDestMat[16];
float fSunPos[4]={400.0, 400.0, 500.0, 0.0};
float fLightPos[4]={400.0, 400.0, 500.0, 0.0};
float fPlane[4]={0,0,1,0.0};
int shadows_on=0;
int day_shadows_on;
int night_shadows_on;


//reflections
water_vertex noise_array[16*16];
float water_deepth_offset=-0.25f;
int lake_waves_timer=0;
float water_movement_u=0;
float water_movement_v=0;

#ifndef LINUX //extensions
PFNGLMULTITEXCOORD2FARBPROC		glMultiTexCoord2fARB	= NULL;
PFNGLMULTITEXCOORD2FVARBPROC	glMultiTexCoord2fvARB	= NULL;
PFNGLACTIVETEXTUREARBPROC		glActiveTextureARB		= NULL;
PFNGLCLIENTACTIVETEXTUREARBPROC	glClientActiveTextureARB= NULL;
#endif
int have_multitexture;
int poor_man=0;
int ground_detail_text;

float clouds_movement_u=-8;
float clouds_movement_v=-3;
Uint32 last_clear_clouds=0;
float texture_scale=12.0;
int clouds_shadows=1;
int use_mipmaps=0;

