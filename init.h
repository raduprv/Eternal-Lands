#ifndef __INIT_H__
#define __INIT_H__

#define MAX_INI_FILE 6000
#define	CFG_VERSION	1

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


	int reserved[20];

	float camera_x;
	float camera_y;
	float camera_z;
	float zoom_level;
	float freserved[20];

}bin_cfg;

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
#endif
