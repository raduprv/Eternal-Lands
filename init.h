#ifndef __INIT_H__
#define __INIT_H__

#define	CFG_VERSION	4

#ifndef DATA_DIR
#define DATA_DIR "./"
#endif

typedef struct
{
	int cfg_version_num;

	int items_menu_x;
	int items_menu_y;

	int ground_items_menu_x;
	int ground_items_menu_y;

	int manufacture_menu_x;
	int manufacture_menu_y;

	int trade_menu_x;
	int trade_menu_y;

	int options_menu_x;
	int options_menu_y;

	int attrib_menu_x;
	int attrib_menu_y;

	int sigil_menu_x;
	int sigil_menu_y;

	int dialogue_menu_x;
	int dialogue_menu_y;

	int knowledge_menu_x;
	int knowledge_menu_y;

	int encyclopedia_menu_x;
	int encyclopedia_menu_y;

	int questlog_menu_x;
	int questlog_menu_y;

	int quickbar_x;
	int quickbar_y;
	int quickbar_flags;
	
	int watch_this_stat;

	//!!!!!!!If you add any new INT option, decrement the reserved thingy accordingly!!!!!!
	int reserved[16];

	float camera_x;
	float camera_y;
	float camera_z;
	float zoom_level;
	float camera_angle;

	//!!!!!!!If you add any new FLOAT option, decrement the reserved thingy accordingly!!!!!!
	float freserved[20];

}bin_cfg;

extern int ini_file_size;
extern int have_stencil;
extern int poor_man;
extern int show_reflection;
extern int mouse_limit;
extern int show_fps;
extern int limit_fps;
extern int item_window_on_drop;
extern int no_adjust_shadows;
extern int clouds_shadows;
extern int compass_direction;
extern char configdir[256];
extern char datadir[256];

extern int show_stats_in_hud;
extern int show_help_text;

extern int disconnected;
extern int exit_now;
extern int have_url;
extern char current_url[160];
extern char broswer_name[120];

extern char lang[10];

void load_harvestable_list();
void load_entrable_list();
void read_config();
void read_bin_cfg();
void save_bin_cfg();
void init_md2_cache();
void init_texture_cache();
void init_e3d_cache();
void init_2d_obj_cache();
void init_stuff();
void resize_window();

void load_e3d_list();
void unload_e3d_list();
typedef struct{
	int id;
	char *fn;
}e3d_list;
extern e3d_list *e3dlist;
extern int e3dlistsize;

extern int video_mode_set;

#endif	//__INIT_H__

