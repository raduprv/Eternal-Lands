#ifndef __INIT_H__
#define __INIT_H__

#define	CFG_VERSION	2

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

	int reserved[20];

	float camera_x;
	float camera_y;
	float camera_z;
	float zoom_level;
	float camera_angle;
	float freserved[20];

}bin_cfg;

extern int ini_file_size;
extern int have_stencil;
extern int poor_man;
extern int show_reflection;
extern int no_alpha_sat;
extern int mouse_limit;
extern int show_fps;
extern int limit_fps;
extern int item_window_on_drop;
extern int no_adjust_shadows;
extern int clouds_shadows;
extern char configdir[256];
extern char datadir[256];

extern int disconnected;
extern int exit_now;
extern int have_url;
extern char current_url[160];
extern char broswer_name[120];

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
void read_key_config();
unsigned int CRC32(unsigned char *, int);
unsigned short get_key_code(char *);
unsigned int parse_key_string(char *s);
void add_key(unsigned int *key,unsigned int n);
#endif
