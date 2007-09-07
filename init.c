#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef __GNUC__
 #include <dirent.h>
 #include <unistd.h>
#endif
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "init.h"
#include "2d_objects.h"
#include "actor_scripts.h"
#include "asc.h"
#include "books.h"
#include "buddy.h"
#include "chat.h"
#include "colors.h"
#include "console.h"
#include "consolewin.h"
#include "cursors.h"
#include "dialogues.h"
#include "draw_scene.h"
#include "elconfig.h"
#include "elwindows.h"
#include "encyclopedia.h"
#include "errors.h"
#include "filter.h"
#include "framebuffer.h"
#include "gamewin.h"
#include "gl_init.h"
#include "hud.h"
#include "items.h"
#include "keys.h"
#include "knowledge.h"
#include "lights.h"
#include "loading_win.h"
#include "loginwin.h"
#include "multiplayer.h"
#include "manufacture.h"
#include "mapwin.h"
#include "new_actors.h"
#include "openingwin.h"
#include "particles.h"
#include "questlog.h"
#include "reflection.h"
#include "rules.h"
#include "spells.h"
#include "storage.h"
#include "tabs.h"
#include "textures.h"
#include "tiles.h"
#include "timers.h"
#include "trade.h"
#include "translate.h"
#include "update.h"
#include "weather.h"
#include "url.h"
#ifdef	EYE_CANDY
#include "eye_candy_wrapper.h"
#endif	//EYE_CANDY
#ifdef MINIMAP
#include "minimap.h"
#endif
#ifdef NEW_FILE_IO
#include "io/elpathwrapper.h"
#include "io/elfilewrapper.h"
#else
#include "misc.h"
#endif
#ifdef PAWN
#include "pawn/elpawn.h"
#endif // PAWN
#ifdef SKY_FPV_CURSOR
#include "sky.h"
#endif

#define	CFG_VERSION 7	// change this when critical changes to el.cfg are made that will break it

int ini_file_size=0;

int disconnected= 1;
#ifdef AUTO_UPDATE
int auto_update= 1;
#ifdef  CUSTOM_UPDATE
int custom_update= 0;
#endif  //CUSTOM_UPDATE
#endif  //AUTO_UPDATE

int exit_now=0;
int restart_required=0;
int allow_restart=1;
int poor_man=0;

#ifdef ANTI_ALIAS
int anti_alias=0;
#endif  //ANTI_ALIAS

#ifdef SFX
int special_effects=0;
#endif  //SFX

int isometric=1;
int mouse_limit=15;
int no_adjust_shadows=0;
int clouds_shadows=1;
int item_window_on_drop=1;
int compass_direction=1;
int buddy_log_notice=1;
char configdir[256]="./";
#ifdef DATA_DIR
char datadir[256]=DATA_DIR;
#else
char datadir[256]="./";
#endif //DATA_DIR

char lang[10]={"en"};

int video_mode_set=0;

void read_command_line(); //from main.c

void load_harvestable_list()
{
	FILE *f = NULL;
	int i = 0;
	char strLine[255];

	memset(harvestable_objects, 0, sizeof(harvestable_objects));
#ifndef NEW_FILE_IO
	f = my_fopen("harvestable.lst", "rb");
	if(!f) {
#else /* NEW_FILE_IO */
	f = open_file_data("harvestable.lst", "rb");
	if(f == NULL) {
		LOG_ERROR("%s: %s \"harvestable.lst\"\n", reg_error_str, cant_open_file);
#endif /* NEW_FILE_IO */
		return;
	}
	while(1)
	{
		if (fscanf (f, "%254s", strLine) != 1)
			break;
		my_strncp (harvestable_objects[i], strLine, sizeof (harvestable_objects[i]));

		i++;
		if(!fgets(strLine, sizeof(strLine), f)) {
			break;
		}
	}
	fclose(f);
}

void load_entrable_list()
{
	FILE *f = NULL;
	int i=0;
	char strLine[255];

	memset(entrable_objects, 0, sizeof(entrable_objects));
	i=0;
#ifndef NEW_FILE_IO
	f=my_fopen("entrable.lst", "rb");
	if(f == NULL){
#else /* NEW_FILE_IO */
	f=open_file_data("entrable.lst", "rb");
	if(f == NULL){
		LOG_ERROR("%s: %s \"entrable.lst\"\n", reg_error_str, cant_open_file);
#endif /* NEW_FILE_IO */
		return;
	}
	while(1)
		{
			if (fscanf (f, "%254s", strLine) != 1)
				break;
			my_strncp (entrable_objects[i], strLine, sizeof (entrable_objects[i]));

			i++;
			if(!fgets(strLine, sizeof(strLine), f))break;
		}
	fclose(f);
}

void load_knowledge_list()
{
	FILE *f = NULL;
	int i=0;
	char strLine[255];
	char *out;
#ifndef NEW_FILE_IO
	char filename[200];
#endif /* NEW_FILE_IO */
	
	memset(knowledge_list, 0, sizeof(knowledge_list));
	i= 0;
	knowledge_count= 0;
	// try the language specific knowledge list
#ifndef NEW_FILE_IO
	safe_snprintf(filename,sizeof(filename),"languages/%s/knowledge.lst",lang);
	if((f=my_fopen(filename,"rb"))==NULL)
		{
			// Failed, try the default/english knowledge list
			f=my_fopen("languages/en/knowledge.lst","rb");
		}
	if(f == NULL){
#else /* NEW_FILE_IO */
	f=open_file_lang("knowledge.lst", "rb");
	if(f == NULL){
		LOG_ERROR("%s: %s \"knowledge.lst\"\n", reg_error_str, cant_open_file);
#endif /* NEW_FILE_IO */
		return;
	}
	while(1)
		{
			if(!fgets(strLine, sizeof(strLine), f)) {
				break;
			}
			out = knowledge_list[i].name;
			my_xmlStrncopy(&out, strLine, sizeof(knowledge_list[i].name)-1);
			i++;
		}
	// memorize the count
	knowledge_count= i;
	// close the file
	fclose(f);
}


void read_config()
{
#if !defined(WINDOWS) && !defined(NEW_FILE_IO)
	DIR *d = NULL;
#endif // !WINDOWS && !NEW_FILE_IO

#ifdef NEW_FILE_IO
	const char * tcfg = get_path_config();
#endif /* NEW_FILE_IO */

#ifndef WINDOWS
#ifndef NEW_FILE_IO
	my_strncp ( configdir, getenv ("HOME") , sizeof(configdir));
#ifndef OSX
	safe_strcat (configdir, "/.elc/", sizeof(configdir));
#else
	safe_strcat (configdir, "/Library/Application\ Support/Eternal\ Lands/", sizeof(configdir));
#endif // OSX
#else /* NEW_FILE_IO */
	my_strncp ( configdir, tcfg , sizeof(configdir));
#endif /* NEW_FILE_IO */
#ifndef NEW_FILE_IO
	d = opendir (configdir);
	if (d == NULL){
		mkdir (configdir, 0700);
	} else {
		struct stat statbuff;
		int fd = dirfd (d);
		fstat (fd, &statbuff);
		/* Set perms to 700 on configdir if they anything else */
		if (statbuff.st_mode != S_IRWXU)
			fchmod (fd, S_IRWXU);
	}
#endif // !NEW_FILE_IO
#endif // !WINDOWS
	if ( !read_el_ini () )
	{
		// oops, the file doesn't exist, give up
		LOG_ERROR("Failure reading el.ini");
		SDL_Quit ();
		exit (1);
	}

#ifndef WINDOWS
	chdir(datadir);
#endif //!WINDOWS

	if(password_str[0])//We have a password
	{
		size_t k;
		
		for (k=0; k < strlen (password_str); k++)
			display_password_str[k] = '*';
		display_password_str[k] = 0;
	}
	else if (username_str[0]) //We have a username but not a password...
	{
		username_box_selected = 0;
		password_box_selected = 1;
	}
#if !defined(WINDOWS) && !defined(NEW_FILE_IO)
	closedir(d);
#endif /* not NEW_FILE_IO or WINDOWS */
}

void read_bin_cfg()
{
	FILE *f = NULL;
	bin_cfg cfg_mem;
#ifndef NEW_FILE_IO
	char el_cfg[256];
#endif /* not NEW_FILE_IO */
	int i;

#ifndef NEW_FILE_IO
	safe_snprintf(el_cfg,  sizeof(el_cfg), "%sel.cfg", configdir);
	// don't use my_fopen, absence of binary config is not an error
	f=fopen(el_cfg,"rb");
	if(!f)return;//no config file, use defaults
#else /* NEW_FILE_IO */
	f=open_file_config("el.cfg","rb");
	if(f == NULL)return;//no config file, use defaults
#endif /* NEW_FILE_IO */
	memset(&cfg_mem, 0, sizeof(cfg_mem));	// make sure its clean

	fread(&cfg_mem,1,sizeof(cfg_mem),f);
	fclose(f);

	//verify the version number
	if(cfg_mem.cfg_version_num != CFG_VERSION) return; //oops! ignore the file

	//good, retrive the data
	// TODO: move window save/restore into the window handler
	items_menu_x=cfg_mem.items_menu_x;
	items_menu_y=cfg_mem.items_menu_y;

	ground_items_menu_x=cfg_mem.ground_items_menu_x;
	ground_items_menu_y=cfg_mem.ground_items_menu_y;

	trade_menu_x=cfg_mem.trade_menu_x;
	trade_menu_y=cfg_mem.trade_menu_y;

	sigil_menu_x=cfg_mem.sigil_menu_x;
	sigil_menu_y=cfg_mem.sigil_menu_y;

	dialogue_menu_x=cfg_mem.dialogue_menu_x;
	dialogue_menu_y=cfg_mem.dialogue_menu_y;

	manufacture_menu_x=cfg_mem.manufacture_menu_x;
	manufacture_menu_y=cfg_mem.manufacture_menu_y;

	tab_stats_x=cfg_mem.tab_stats_x;
	tab_stats_y=cfg_mem.tab_stats_y;

	elconfig_menu_x=cfg_mem.elconfig_menu_x;
	elconfig_menu_y=cfg_mem.elconfig_menu_y;

	tab_help_x=cfg_mem.tab_help_x;
	tab_help_y=cfg_mem.tab_help_y;

	storage_win_x=cfg_mem.storage_win_x;
	storage_win_y=cfg_mem.storage_win_y;

	buddy_menu_x=cfg_mem.buddy_menu_x;
	buddy_menu_y=cfg_mem.buddy_menu_y;
	
	url_win_x=cfg_mem.url_win_x;
	url_win_y=cfg_mem.url_win_y;

#ifdef MINIMAP
	minimap_win_x=cfg_mem.minimap_win_x;
	minimap_win_y=cfg_mem.minimap_win_y;
	minimap_flags=cfg_mem.minimap_flags;
	minimap_zoom=cfg_mem.minimap_zoom;
#endif //MINIMAP

	if(quickbar_relocatable>0)
		{
			if((quickbar_x=cfg_mem.quickbar_x)>window_width||quickbar_x<=0)quickbar_x=34;
			if((quickbar_y=cfg_mem.quickbar_y)>window_height||quickbar_y<=0)quickbar_y=64;
			if((quickbar_dir=cfg_mem.quickbar_flags&0xFF)!=HORIZONTAL)quickbar_dir=VERTICAL;
			if((quickbar_draggable=(cfg_mem.quickbar_flags&0xFF00)>>8)!=1)quickbar_draggable=0;
		}
	
	watch_this_stat=cfg_mem.watch_this_stat;
	if(watch_this_stat<0 || watch_this_stat>=NUM_WATCH_STAT)
		watch_this_stat=0;

	has_accepted=cfg_mem.has_accepted_rules;
	
	rx=cfg_mem.camera_x;
	ry=cfg_mem.camera_y;
	rz=cfg_mem.camera_z;
	new_zoom_level=zoom_level=cfg_mem.zoom_level;

	view_health_bar=cfg_mem.view_health_bar;
	view_names=cfg_mem.view_names;
	view_hp=cfg_mem.view_hp;
	quantities.selected=cfg_mem.quantity_selected;

	for(i=0;i<6;i++){
		if(cfg_mem.quantity[i]){
			quantities.quantity[i].val=cfg_mem.quantity[i];
			safe_snprintf(quantities.quantity[i].str, sizeof(quantities.quantity[i].str),"%d", cfg_mem.quantity[i]);
			quantities.quantity[i].len=strlen(quantities.quantity[i].str);
		}
	}

	if(zoom_level != 0.0f) resize_root_window();
}

void save_bin_cfg()
{
	FILE *f = NULL;
	bin_cfg cfg_mem;
#ifndef NEW_FILE_IO
	char el_cfg[256];
#endif /* not NEW_FILE_IO */
	int i;

#ifndef NEW_FILE_IO
	safe_snprintf(el_cfg, sizeof(el_cfg), "%sel.cfg", configdir);
	f=my_fopen(el_cfg,"wb");
	if(!f)return;//blah, whatever
#else /* NEW_FILE_IO */
	f=open_file_config("el.cfg","wb");
	if(f == NULL){
		LOG_ERROR("%s: %s \"el.cfg\"\n", reg_error_str, cant_open_file);
		return;//blah, whatever
	}
#endif /* NEW_FILE_IO */
	memset(&cfg_mem, 0, sizeof(cfg_mem));	// make sure its clean

	cfg_mem.cfg_version_num=CFG_VERSION;	// set the version number
	//good, retrive the data
	/*
	// TODO: move window save/restore into the window handler
	cfg_mem.items_menu_x=items_menu_x;
	cfg_mem.items_menu_y=items_menu_y;

	cfg_mem.ground_items_menu_x=ground_items_menu_x;
	cfg_mem.ground_items_menu_y=ground_items_menu_y;

	cfg_mem.trade_menu_x=trade_menu_x;
	cfg_mem.trade_menu_y=trade_menu_y;

	cfg_mem.sigil_menu_x=sigil_menu_x;
	cfg_mem.sigil_menu_y=sigil_menu_y;

	cfg_mem.dialogue_menu_x=dialogue_menu_x;
	cfg_mem.dialogue_menu_y=dialogue_menu_y;

	cfg_mem.manufacture_menu_x=manufacture_menu_x;
	cfg_mem.manufacture_menu_y=manufacture_menu_y;

	cfg_mem.attrib_menu_x=attrib_menu_x;
	cfg_mem.attrib_menu_y=attrib_menu_y;

	cfg_mem.elconfig_menu_x=elconfig_menu_x;
	cfg_mem.elconfig_menu_y=elconfig_menu_y;

	cfg_mem.knowledge_menu_x=knowledge_menu_x;
	cfg_mem.knowledge_menu_y=knowledge_menu_y;

	cfg_mem.encyclopedia_menu_x=encyclopedia_menu_x;
	cfg_mem.encyclopedia_menu_y=encyclopedia_menu_y;

	cfg_mem.questlog_menu_x=questlog_menu_x;
	cfg_mem.questlog_menu_y=questlog_menu_y;
*/

	if(tab_help_win >= 0) {
		cfg_mem.tab_help_x=windows_list.window[tab_help_win].cur_x;
		cfg_mem.tab_help_y=windows_list.window[tab_help_win].cur_y;
	} else {
		cfg_mem.tab_help_x=tab_help_x;
		cfg_mem.tab_help_y=tab_help_y;
	}

	if(items_win >= 0) {
		cfg_mem.items_menu_x=windows_list.window[items_win].cur_x;
		cfg_mem.items_menu_y=windows_list.window[items_win].cur_y;
	} else {
		cfg_mem.items_menu_x=items_menu_x;
		cfg_mem.items_menu_y=items_menu_y;
	}

	if(ground_items_win >= 0) {
		cfg_mem.ground_items_menu_x=windows_list.window[ground_items_win].cur_x;
		cfg_mem.ground_items_menu_y=windows_list.window[ground_items_win].cur_y;
	} else {
		cfg_mem.ground_items_menu_x=ground_items_menu_x;
		cfg_mem.ground_items_menu_y=ground_items_menu_y;
	}

	if(trade_win >= 0) {
		cfg_mem.trade_menu_x=windows_list.window[trade_win].cur_x;
		cfg_mem.trade_menu_y=windows_list.window[trade_win].cur_y;
	} else {
		cfg_mem.trade_menu_x=trade_menu_x;
		cfg_mem.trade_menu_y=trade_menu_y;
	}

	if(sigil_win >= 0) {
		cfg_mem.sigil_menu_x=windows_list.window[sigil_win].cur_x;
		cfg_mem.sigil_menu_y=windows_list.window[sigil_win].cur_y;
	} else {
		cfg_mem.sigil_menu_x=sigil_menu_x;
		cfg_mem.sigil_menu_y=sigil_menu_y;
	}

	if(dialogue_win >= 0) {
		cfg_mem.dialogue_menu_x=windows_list.window[dialogue_win].cur_x;
		cfg_mem.dialogue_menu_y=windows_list.window[dialogue_win].cur_y;
	} else {
		cfg_mem.dialogue_menu_x=dialogue_menu_x;
		cfg_mem.dialogue_menu_y=dialogue_menu_y;
	}

	if(manufacture_win >= 0) {
		cfg_mem.manufacture_menu_x=windows_list.window[manufacture_win].cur_x;
		cfg_mem.manufacture_menu_y=windows_list.window[manufacture_win].cur_y;
	} else {
		cfg_mem.manufacture_menu_x=manufacture_menu_x;
		cfg_mem.manufacture_menu_y=manufacture_menu_y;
	}

	if(elconfig_win >= 0) {
		cfg_mem.elconfig_menu_x=windows_list.window[elconfig_win].cur_x;
		cfg_mem.elconfig_menu_y=windows_list.window[elconfig_win].cur_y;
	} else {
		cfg_mem.elconfig_menu_x=elconfig_menu_x;
		cfg_mem.elconfig_menu_y=elconfig_menu_y;
	}

	if(storage_win >= 0) {
		cfg_mem.storage_win_x=windows_list.window[storage_win].cur_x;
		cfg_mem.storage_win_y=windows_list.window[storage_win].cur_y;
	} else {
		cfg_mem.storage_win_x=storage_win_x;
		cfg_mem.storage_win_y=storage_win_y;
	}

	if(tab_stats_win >= 0) {
		cfg_mem.tab_stats_x=windows_list.window[tab_stats_win].cur_x;
		cfg_mem.tab_stats_y=windows_list.window[tab_stats_win].cur_y;
	} else {
		cfg_mem.tab_stats_x=tab_stats_x;
		cfg_mem.tab_stats_y=tab_stats_y;
	}

	if(buddy_win >= 0) {
		cfg_mem.buddy_menu_x=windows_list.window[buddy_win].cur_x;
		cfg_mem.buddy_menu_y=windows_list.window[buddy_win].cur_y;
	} else {
		cfg_mem.buddy_menu_x=buddy_menu_x;
		cfg_mem.buddy_menu_y=buddy_menu_y;
	}
	
	if(url_win >= 0) {
		cfg_mem.url_win_x=windows_list.window[url_win].cur_x;
		cfg_mem.url_win_y=windows_list.window[url_win].cur_y;
	} else {
		cfg_mem.url_win_x=url_win_x;
		cfg_mem.url_win_y=url_win_y;
	}

#ifdef MINIMAP
	if(minimap_win >= 0) {
		cfg_mem.minimap_win_x=windows_list.window[minimap_win].cur_x;
		cfg_mem.minimap_win_y=windows_list.window[minimap_win].cur_y;
	} else {
		cfg_mem.minimap_win_x=minimap_win_x;
		cfg_mem.minimap_win_y=minimap_win_y;
	}
	cfg_mem.minimap_flags=minimap_flags;
	cfg_mem.minimap_zoom=minimap_zoom;
#endif //MINIMAP
	cfg_mem.view_health_bar=view_health_bar;
	cfg_mem.view_names=view_names;
	cfg_mem.view_hp=view_hp;
	cfg_mem.quantity_selected=quantities.selected;

	if(quickbar_relocatable>0)
		{
			if(quickbar_win >= 0){
				cfg_mem.quickbar_x=window_width-windows_list.window[quickbar_win].cur_x;
				cfg_mem.quickbar_y=windows_list.window[quickbar_win].cur_y;
				cfg_mem.quickbar_flags=quickbar_dir|(quickbar_draggable<<8);
			} else {
				cfg_mem.quickbar_x=quickbar_x;
				cfg_mem.quickbar_y=quickbar_y;
				cfg_mem.quickbar_flags=VERTICAL;
			}
		}

	cfg_mem.watch_this_stat=watch_this_stat;

	cfg_mem.has_accepted_rules=has_accepted;
	
	cfg_mem.camera_x=rx;
	cfg_mem.camera_y=ry;
	cfg_mem.camera_z=rz;
	cfg_mem.zoom_level=zoom_level;
	
	for(i=0;i<6;i++){
		cfg_mem.quantity[i]=quantities.quantity[i].val;
	}

	fwrite(&cfg_mem,sizeof(cfg_mem),1,f);
	fclose(f);

}

void init_texture_cache()
{
	memset(texture_cache, 0, sizeof(texture_cache));
}

void init_e3d_cache()
{
	//cache_e3d= cache_init(1000, &destroy_e3d);	//TODO: autofree the name as well
	cache_e3d= cache_init(1000, NULL);	//no aut- free permitted
	cache_set_name(cache_system, "E3D cache", cache_e3d);
	cache_set_compact(cache_e3d, &free_e3d_va);	// to compact, free VA arrays
	cache_set_time_limit(cache_e3d, 5*60*1000);
	cache_set_size_limit(cache_e3d, 8*1024*1024);
}

void init_2d_obj_cache()
{
	memset(obj_2d_def_cache, 0, sizeof(obj_2d_def_cache));
}

void init_stuff()
{
	int seed;
	char file_name[250];
	int i;
#ifdef NEW_FILE_IO
	char config_location[300];
	const char * cfgdir;
#endif //NEW_FILE_IO	

	//TODO: process command line options
	chdir(datadir);

#ifdef WRITE_XML
	load_translatables();//Write to the current working directory - hopefully we'll have write rights here...
#endif
	// initialize the text buffers
	init_text_buffers ();
	// XXX FIXME (Grum): actually this should only be done when windowed
	// chat is not used (which we don't know yet at this point), but let's
	// leave it here until we're certain that the chat channel buffers are
	// never used otherwise, then move it down till after the configuration
	// is read.
	init_chat_channels ();

	// initialize the fonts, but don't load the textures yet. Do that here
	// because the messages need the font widths.
	init_fonts();
	
	//read the config file
	read_config();

#ifdef	NEW_FILE_IO
	add_paths();
	// Here you can add zip files, like
	// add_zip_archive("./data.zip", datadir, 0);
#endif	// NEW_FILE_IO

	//Parse command line options
	read_command_line();
	options_set= 1;

	//OK, we have the video mode settings...
	setup_video_mode(full_screen,video_mode);
	//now you may set the video mode using the %<foo> in-game
	video_mode_set=1;

	//Good, we should be in the right working directory - load all translatables from their files
	load_translatables();

	init_video();

	//Init the caches here, as the loading window needs them
	cache_system_init(MAX_CACHE_SYSTEM);
	init_texture_cache();
	init_e3d_cache();
	init_2d_obj_cache();
	//now load the font textures
	load_font_textures ();
	CHECK_GL_ERRORS();
	init_colors();

	// read the continent map info
	read_mapinfo();

	// now create the root window

	// XXX FIXME (Grum): Maybe we should do this at a later time, after
	// we're logged in?
	create_game_root_window (window_width, window_height);
	create_console_root_window (window_width, window_height);
	create_map_root_window (window_width, window_height);
	create_login_root_window (window_width, window_height);

	//create the loading window
	create_loading_win (window_width, window_height, 0);
	show_window(loading_win);

	update_loading_win(init_opengl_str, 5);
	init_gl_extensions();
	
	// Setup the new eye candy system
#ifdef	EYE_CANDY
	ec_init();
#endif	//EYE_CANDY

	// check for invalid combinations
	check_options();

	update_loading_win(init_random_str, 4);
	seed= time (NULL);
	srand(seed);

	update_loading_win(load_ignores_str, 1);
	load_ignores();
	update_loading_win(load_filters_str, 2);
	load_filters();
	update_loading_win(load_lists_str, 2);
	load_harvestable_list();
	load_entrable_list();
	load_knowledge_list();
	update_loading_win(load_cursors_str, 5);
	load_cursors();
	build_cursors();
	change_cursor(CURSOR_ARROW);
	update_loading_win(bld_glow_str, 3);
	build_glow_color_table();

	update_loading_win(init_lists_str, 2);
	init_actors_lists();
	update_loading_win(NULL, 4);
	memset(tile_list, 0, sizeof(tile_list));
	memset(lights_list, 0, sizeof(lights_list));
	main_bbox_tree = build_bbox_tree();
	init_particles ();
#ifdef NEW_SOUND
	update_loading_win(init_audio_str, 1);
	load_sound_config_data(SOUND_CONFIG_PATH);
#endif // NEW_SOUND
	update_loading_win(init_actor_defs_str, 4);
	memset(actors_defs, 0, sizeof(actors_defs));
	init_actor_defs();
	update_loading_win(load_map_tiles_str, 4);
	load_map_tiles();

	update_loading_win(init_lights_str, 4);
	//lights setup
	build_global_light_table();
	build_sun_pos_table();
	reset_material();
	init_lights();
	disable_local_lights();
	update_loading_win(init_logs_str, 4);
	clear_error_log();
	clear_conn_log();
	update_loading_win(read_config_str, 2);
	read_bin_cfg();
#ifndef NEW_WEATHER
 	update_loading_win(init_weather_str, 3);
 	init_weather();	// initialize the weather system
#endif //NEW_WEATHER
	build_levels_table();//for some HUD stuff

	update_loading_win(load_icons_str, 4);
	//load the necesary textures
#ifdef	NEW_ALPHA
	icons_text= load_texture_cache("./textures/gamebuttons.bmp", -1);
	hud_text= load_texture_cache("./textures/gamebuttons2.bmp", -1);
#else	//NEW_ALPHA
	icons_text= load_texture_cache("./textures/gamebuttons.bmp",0);
	hud_text= load_texture_cache("./textures/gamebuttons2.bmp",0);
#endif	//NEW_ALPHA
	update_loading_win(load_textures_str, 4);
	cons_text= load_texture_cache("./textures/console.bmp",255);

	update_loading_win(NULL, 5);

	for(i=0; i<MAX_ITEMS_TEXTURES; i++){
		char	buffer[256];

		safe_snprintf(buffer, sizeof(buffer), "./textures/items%d.bmp", i+1);
		if(gzfile_exists(buffer)){
			items_text[i]= load_texture_cache(buffer, 0);
		}
	}
	update_loading_win(NULL, 5);

	for(i=0; i<MAX_PORTRAITS_TEXTURES; i++){
		char	buffer[256];

		safe_snprintf(buffer, sizeof(buffer), "./textures/portraits%d.bmp", i+1);
		if(gzfile_exists(buffer)){
			portraits_tex[i]= load_texture_cache_deferred(buffer, 0);
		}
	}
	update_loading_win(NULL, 5);

#ifdef SKY_FPV_CURSOR

	disable_compression();
	cursors_tex = load_texture_cache("./textures/cursors2.bmp",0);
	enable_compression();


//Emajekral's hi-color & big cursor code
	if (!sdl_cursors) SDL_ShowCursor(0);


#endif /* SKY_FPV_CURSOR */
	//Load the map legend and continent map
	legend_text= load_texture_cache("./maps/legend.bmp",0);

	ground_detail_text=load_texture_cache("./textures/ground_detail.bmp",255);
	CHECK_GL_ERRORS();
	init_login_screen ();
	init_spells ();

#ifdef PAWN
	update_loading_win (init_pawn_str, 0);
	initialize_pawn ();
#endif

	update_loading_win(init_network_str, 5);	
	if(SDLNet_Init()<0){
		log_error("%s: %s\n", failed_sdl_net_init, SDLNet_GetError());
		SDLNet_Quit();
		SDL_Quit();
		exit(2);
	}
	update_loading_win(init_timers_str, 5);

	if(SDL_InitSubSystem(SDL_INIT_TIMER)<0){
		log_error("%s: %s\n", failed_sdl_timer_init, SDL_GetError());
		SDL_Quit();
	 	exit(1);
	}
	update_loading_win(load_encyc_str, 5);
	safe_snprintf(file_name, sizeof(file_name), "languages/%s/Encyclopedia/index.xml", lang);
	ReadXML(file_name);
	read_key_config();
	load_questlog();
	init_buddy();
	init_channel_names();
#ifdef	OLC
	olc_finish_init();
#endif	//OLC

#ifdef  AUTO_UPDATE
	if(auto_update){
		init_update();
#ifdef  CUSTOM_UPDATE
	} else if(custom_update){
		init_custom_update();
#endif  //CUSTOM_UPDATE
	}
#endif

	have_rules=read_rules();
	if(!have_rules){
		log_error(rules_not_found);
		SDL_Quit();
		exit(3);
	}

	//initiate function pointers
	init_attribf();

	//Read the books for i.e. the new char window
	init_books();

	update_loading_win(init_display_str, 5);
	SDL_SetGamma(gamma_var, gamma_var, gamma_var);

	draw_scene_timer= SDL_AddTimer (1000/(18*4), my_timer, NULL);
	misc_timer= SDL_AddTimer (500, check_misc, NULL);

#ifdef NEW_FILE_IO
	cfgdir = get_path_config();
	if(cfgdir != NULL){
		//Realistically, if this failed, then there's not much point in continuing, but oh well...
		safe_snprintf(config_location, sizeof(config_location), config_location_str, cfgdir);
	}
	LOG_TO_CONSOLE(c_green4, config_location);
	file_check_datadir();
#endif //NEW_FILE_IO	

	update_loading_win(prep_op_win_str, 7);
	create_opening_root_window (window_width, window_height);
	// initialize the chat window
	if (use_windowed_chat == 2) {
		display_chat ();
	}

	init_commands("commands.lst");

#ifdef NEW_SOUND
	// Try to turn the sound on now so we have it for the login window
	turn_sound_on();		
#endif // NEW_SOUND
	
	// display something
	destroy_loading_win();
	if (has_accepted)
	{
		show_window (opening_root_win);
		connect_to_server();
	}
	else 
	{
		create_rules_root_window (window_width, window_height, opening_root_win, 15);
		show_window (rules_root_win);
	}

	if (use_frame_buffer) make_reflection_framebuffer(window_width, window_height);

#ifdef SKY_FPV_CURSOR
	init_sky();
#endif /* SKY_FPV_CURSOR */
}
