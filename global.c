#include "global.h"

int map_meters_size_x;
int map_meters_size_y;
float texture_scale=12.0;

int window_width=640;
int window_height=480;

int desktop_width;
int desktop_height;

int bpp=0;
int have_stencil=1;
int video_mode;
int full_screen;

Uint32 cur_time=0, last_time=0;//for FPS
//for the client/server sync
int server_time_stamp=0;
int client_time_stamp=0;
int client_server_delta_time=0;

//camera stuff
float cx=0;
float cy=0;
float cz=0;
float rx=-60;
float ry=0;
float rz=45;
float terrain_scale=2.0f;
float zoom_level=3.0f;
float name_zoom=1.0f;

float fine_camera_rotation_speed;
float normal_camera_rotation_speed;

float camera_rotation_speed;
int camera_rotation_frames;

double camera_x_speed;
int camera_x_frames;

double camera_y_speed;
int camera_y_frames;

double camera_z_speed;
int camera_z_frames;


int normal_animation_timer=0;

float scene_mouse_x;
float scene_mouse_y;


int last_texture=-2;
int font_text;
int cons_text;
int icons_text;
int open_text;
int login_text;
int ground_detail_text;

SDL_mutex *particles_list_mutex;	//used for locking between the timer and main threads
int particles_text;
particle_sys *particles_list[max_particle_systems];

texture_cache_struct texture_cache[1000];
md2_cache_struct md2_cache[1000];
e3d_cache_struct e3d_cache[1000];
obj_2d_cache_struct obj_2d_def_cache[max_obj_2d_def];

int yourself=-1;
int you_sit=0;
int sit_lock=0;
actor *actors_list[1000];
int max_actors=0;
SDL_mutex *actors_lists_mutex;	//used for locking between the timer and main threads
object3d *objects_list[max_obj_3d];
obj_2d *obj_2d_list[max_obj_2d];
actor_types actors_defs[40];

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

light *lights_list[max_lights];
unsigned char light_level=58;
sun sun_pos[60*3];
short game_minute=60;

//tile map things
unsigned char *tile_map;
unsigned char *height_map;
int tile_map_size_x;
int tile_map_size_y;
int tile_list[256];
char dungeon=0;//no sun
float ambient_r=0;
float ambient_g=0;
float ambient_b=0;
char map_file_name[60];

//multiplayer stuff
char our_name[20];
char our_password[20];
int log_conn_data=0;

//text stuff
char input_text_line[257];
int input_text_lenght=0;
char display_text_buffer[max_display_text_buffer_lenght];

int display_text_buffer_first=0;
int display_text_buffer_last=0;

int display_console_text_buffer_first=0;
int display_console_text_buffer_last=0;
char last_pm_from[32];

Uint32 last_server_message_time;
int lines_to_show=0;
int max_lines_no=10;
char console_mode=0;
char not_from_the_end_console=0;

int log_server = 1;

//interface
int have_a_map=0;
char interface_mode=interface_opening;
char create_char_error_str[520];
char log_in_error_str[520];
int view_clock=1;
int view_compas=1;
int options_menu=0;
int combat_mode=0;
int auto_camera=0;
int selected_3d_object;
int selected_inventory_object=-1;
int view_health_bar=1;
int view_names=1;
int show_fps=1;

float marble_menu_u_start=(float)160/255;
float marble_menu_v_start=1.0f-(float)128/255;
float marble_menu_u_end=(float)255/255;
float marble_menu_v_end=1.0f-(float)255/255;
float close_button_u_start=(float)192/255;
float close_button_v_start=1.0f-(float)96/255;
float close_button_u_end=(float)207/255;
float close_button_v_end=1.0f-(float)111/255;

int action_mode=action_walk;

//ignore things
ignore_slot ignore_list[max_ignores];
int ignored_so_far=0;
int save_ignores=1;
int use_global_ignores=1;

// text filtering
filter_slot filter_list[max_filters];
int filtered_so_far=0;
int use_global_filters=1;
char text_filter_replace[128]="smeg";

//sound
int have_sound=0;
int have_music=0;
int sound_on=1;
int music_on=1;
int no_sound=0;

//char interface_mode=interface_new_char;
int mouse_x;
int mouse_y;
int mouse_delta_x;
int mouse_delta_y;
int right_click;
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

//shadows
float fDestMat[16];
float fSunPos[4]={400.0, 400.0, 500.0, 0.0};
float fLightPos[4]={400.0, 400.0, 500.0, 0.0};
float fPlane[4]={0,0,1,0};
int shadows_on=0;
int day_shadows_on;
int night_shadows_on;
int shadows_texture;

//reflections
water_vertex noise_array[16*16];
int sky_text_1;
float water_deepth_offset=-0.25f;
int lake_waves_timer=0;
float water_movement_u=0;
float water_movement_v=0;
int show_reflection=1;

//client server stuff
int port=2000;
unsigned char server_address[60];
TCPsocket my_socket=0;
SDLNet_SocketSet set=0;
Uint8 in_data[8192];
int previously_logged_in=0;
int last_heart_beat;

//debuging info
int debug_info=0;
float debug_float;
int triangles_normal=0;
int triangles_shadow=0;

//spells
sigil_def sigils_list[SIGILS_NO];
int sigil_menu_x=10;
int sigil_menu_y=20;
int sigil_menu_x_len=12*33+20;
int sigil_menu_y_len=6*33;
int sigil_menu_dragged=0;

int sigils_text;
Uint8 spell_text[256];
int view_sigils_menu=0;
int sigils_we_have;
int have_error_message=0;
Sint8 active_spells[10];

//stats
int view_self_stats=0;
player_attribs your_info;
player_attribs someone_info;
int attrib_menu_x=100;
int attrib_menu_y=20;
int attrib_menu_x_len=516;
int attrib_menu_y_len=348;
int attrib_menu_dragged=0;

//items
item item_list[36+6];
item manufacture_list[36+6];
ground_item ground_item_list[50];
bag bag_list[200];

item inventory_trade_list[36];
item your_trade_list[24];
item others_trade_list[24];
int trade_you_accepted=0;
int trade_other_accepted=0;
char other_player_trade_name[20];

int view_my_items=0;
int view_ground_items=0;
int view_manufacture_menu=0;
int view_trade_menu=0;
int no_view_my_items=0;

int items_menu_x=10;
int items_menu_y=20;
int items_menu_x_len=6*51+110;
int items_menu_y_len=6*51+60;
int items_menu_dragged=0;

int ground_items_menu_x=6*51+100+20;
int ground_items_menu_y=20;
int ground_items_menu_x_len=6*33;
int ground_items_menu_y_len=10*33;
int ground_items_menu_dragged=0;

int manufacture_menu_x=10;
int manufacture_menu_y=20;
int manufacture_menu_x_len=12*33+20;
int manufacture_menu_y_len=6*33;
int manufacture_menu_dragged=0;

int trade_menu_x=10;
int trade_menu_y=20;
int trade_menu_x_len=13*33;
int trade_menu_y_len=11*33;
int trade_menu_dragged=0;

int options_menu_x=220;
int options_menu_y=50;
int options_menu_x_len=390;
int options_menu_y_len=260;
int options_menu_dragged=0;

int items_text_1;
int items_text_2;
int items_text_3;
int items_text_4;
int items_text_5;
int items_text_6;
int items_text_7;

char items_string[300];
int item_dragged=-1;
int item_quantity=1;


//weather
int seconds_till_rain_starts=-1;
int seconds_till_rain_stops=-1;
int is_raining=0;
int rain_sound=0;
int weather_light_offset=0;
int rain_light_offset=0;
int thunder_light_offset;
thunder thunders[MAX_THUNDERS];
int lightning_text;

//dialogues
char dialogue_string[1024];
char npc_name[20];
int cur_portrait=8;
int portraits1_tex;
int portraits2_tex;
int portraits3_tex;
int portraits4_tex;
int portraits5_tex;

response dialogue_responces[20];
int have_dialogue=0;

int dialogue_menu_x=1;
int dialogue_menu_y=1;
int dialogue_menu_x_len=638;
int dialogue_menu_y_len=160;
int dialogue_menu_dragged=0;

int no_bounding_box=0;

//others
int disconnected=1;
int exit_now = 0;
int have_url=0;
char current_url[160];
char broswer_name[120];
int poor_man=0;
int mouse_limit=15;
int no_adjust_shadows=0;
int clouds_shadows=1;
int no_alpha_sat=0;
help_entry help_list[MAX_HELP_ENTRIES];
char home[100];
char datadir[200];

//extensions
#ifdef WINDOWS//linux has those functins already...
PFNGLMULTITEXCOORD2FARBPROC		glMultiTexCoord2fARB	= NULL;
PFNGLMULTITEXCOORD2FVARBPROC	glMultiTexCoord2fvARB	= NULL;
PFNGLACTIVETEXTUREARBPROC		glActiveTextureARB		= NULL;
PFNGLCLIENTACTIVETEXTUREARBPROC	glClientActiveTextureARB= NULL;
PFNGLLOCKARRAYSEXTPROC			glLockArraysEXT			= NULL;
PFNGLUNLOCKARRAYSEXTPROC		glUnlockArraysEXT		= NULL;
#endif

int have_multitexture=0;
float clouds_movement_u=-8;
float clouds_movement_v=-3;
int last_clear_clouds=0;
int reflection_texture;
int have_compiled_vertex_array=0;

int shift_on;
int alt_on;
int ctrl_on;

//cursors
actor *actor_under_mouse;
int object_under_mouse;
int thing_under_the_mouse;
int current_cursor;
int read_mouse_now=0;

struct cursors_struct cursors_array[20];
struct harvest_names_struct harvestable_objects[100];
struct enter_names_struct entrable_objects[100];

char version_string[]=VER_STRING;
int	client_version_major=VER_MAJOR;
int client_version_minor=VER_MINOR;
int client_version_release=VER_RELEASE;
int	client_version_patch=VER_BUILD;
int version_first_digit=9;	//protocol/game version sent to server
int version_second_digit=6;



