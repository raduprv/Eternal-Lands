#include <stdlib.h>
#include <sys/stat.h>
#ifdef __GNUC__
#include <dirent.h>
#include <unistd.h>
#endif
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "global.h"
#include "elwindows.h"
#include "keys.h"

#define	CFG_VERSION	5

#ifndef DATA_DIR
#define DATA_DIR "./"
#endif

#ifdef ENCYCLOPEDIA
#include "books/symbols.h"
#include "books/parser.h"
#endif

int ini_file_size=0;

int disconnected=1;
int exit_now=0;
int have_url=0;
char current_url[160];
char browser_name[120];
int poor_man=0;
#ifdef ANTI_ALIAS
int anti_alias=0;
#endif
int isometric=1;
int mouse_limit=15;
int no_adjust_shadows=0;
int clouds_shadows=1;
int item_window_on_drop=1;
int compass_direction=1;
char configdir[256]="./";
char datadir[256]=DATA_DIR;

char lang[10]={"en"};

int video_mode_set=0;

extern windows_info	windows_list;

typedef struct{
	int id;
	char *fn;
}e3d_list;

e3d_list *e3dlist=NULL;
int e3dlistsize=0;

void read_command_line(); //from main.c

void unload_e3d_list()
{
	int i;
	for(i=0;i<e3dlistsize;i++)
		free(e3dlist[i].fn);
	free(e3dlist);
}

void load_e3d_list()
{
	FILE *fp;
	int i=0;

	fp=my_fopen("e3dlist.txt","r");
	if(!fp){
		SDL_Quit();
		exit(1);
	}

	fscanf(fp,"%d",&e3dlistsize);
	e3dlist=(e3d_list*)malloc(sizeof(e3d_list)*e3dlistsize);

	for(i=0;i<e3dlistsize;i++){
		char temp[256];
		int id, len;
		fscanf(fp,"%255s %d",temp,&id);
		len = strlen(temp) + 1;
		e3dlist[i].fn=(char*)malloc(len);
		snprintf(e3dlist[i].fn, len, "%s", temp);
		e3dlist[i].id=id;
	}
	fclose(fp);
	return;
}

void load_harvestable_list()
{
	FILE *f = NULL;
	int i=0;
	char strLine[255];

	memset(harvestable_objects, 0, sizeof(harvestable_objects));
	i=0;
	f=my_fopen("harvestable.lst", "rb");
	if(!f)return;
	while(1)
		{
			fscanf(f,"%s",harvestable_objects[i].name);
			i++;
			if(!fgets(strLine, 100, f))break;
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
	f=my_fopen("entrable.lst", "rb");
	if(!f)return;
	while(1)
		{
			fscanf(f,"%s",entrable_objects[i].name);
			i++;
			if(!fgets(strLine, 100, f))break;
		}
	fclose(f);
}

void load_knowledge_list()
{
	FILE *f = NULL;
	int i=0;
	char strLine[255];
	char filename[200];
	
	memset(knowledge_list, 0, sizeof(knowledge_list));
	i=0;
	snprintf(filename,sizeof(filename),"languages/%s/knowledge.lst",lang);
	if((f=my_fopen(filename,"rb"))==NULL)
		{
			f=my_fopen("languages/en/knowledge.lst","rb");
		}
	if(!f)return;
	while(1)
		{
			if(!fgets(strLine, sizeof(strLine), f))break;
			snprintf(knowledge_list[i].name, sizeof(knowledge_list[i].name), "%s", strLine);
			i++;
		}
	fclose(f);
}


void read_config()
{
#ifndef WINDOWS
	DIR *d = NULL;
	my_strncp ( configdir, getenv ("HOME") , sizeof(configdir));
	strncat (configdir, "/.elc/", sizeof(configdir)-1);
	d = opendir (configdir);
	if (!d)
	{
		mkdir (configdir, 0755);
	}
#endif
	if ( !read_el_ini () )
	{
		// oops, the file doesn't exist, give up
		SDL_Quit ();
		exit (1);
	}

#ifndef WINDOWS
	chdir(datadir);
#endif
	
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
}

void read_bin_cfg()
{
	FILE *f = NULL;
	bin_cfg cfg_mem;
	char el_cfg[256];
	int i;

	snprintf(el_cfg,  sizeof(el_cfg), "%sel.cfg", configdir);
	// don't use my_fopen, absence of binary config is not an error
	f=fopen(el_cfg,"rb");
	if(!f)return;//no config file, use defaults
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

	attrib_menu_x=cfg_mem.attrib_menu_x;
	attrib_menu_y=cfg_mem.attrib_menu_y;

	elconfig_menu_x=cfg_mem.elconfig_menu_x;
	elconfig_menu_y=cfg_mem.elconfig_menu_y;

	knowledge_menu_x=cfg_mem.knowledge_menu_x;
	knowledge_menu_y=cfg_mem.knowledge_menu_y;

	encyclopedia_menu_x=cfg_mem.encyclopedia_menu_x;
	encyclopedia_menu_y=cfg_mem.encyclopedia_menu_y;

	questlog_menu_x=cfg_mem.questlog_menu_x;
	questlog_menu_y=cfg_mem.questlog_menu_y;

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
	
	cx=cfg_mem.camera_x;
	cy=cfg_mem.camera_y;
	cz=cfg_mem.camera_z;
	new_zoom_level=zoom_level=cfg_mem.zoom_level;
	rz=cfg_mem.camera_angle;

	for(i=0;i<6;i++){
		if(cfg_mem.quantity[i]){
			quantities.quantity[i].val=cfg_mem.quantity[i];
			snprintf(quantities.quantity[i].str, sizeof(quantities.quantity[i].str),"%d", cfg_mem.quantity[i]);
			quantities.quantity[i].len=strlen(quantities.quantity[i].str);
		}
	}

	if(zoom_level != 0.0f) resize_root_window();
}

void save_bin_cfg()
{
	FILE *f = NULL;
	bin_cfg cfg_mem;
	char el_cfg[256];
	int i;

	snprintf(el_cfg, sizeof(el_cfg), "%sel.cfg", configdir);
	f=my_fopen(el_cfg,"wb");
	if(!f)return;//blah, whatever
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
	
	cfg_mem.camera_x=cx;
	cfg_mem.camera_y=cy;
	cfg_mem.camera_z=cz;
	cfg_mem.zoom_level=zoom_level;
	cfg_mem.camera_angle=rz;
	
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
	//cache_e3d=cache_init(1000, &destroy_e3d);	//TODO: autofree the name as well
	cache_e3d=cache_init(1000, NULL);	//no aut- free permitted
	cache_set_name(cache_system, "E3D cache", cache_e3d);
	cache_set_compact(cache_e3d, &free_e3d_va);	// to compact, free VA arrays
	cache_set_time_limit(cache_e3d, 5*60*1000);
	cache_set_size_limit(cache_e3d, 8*1024*1024);
}

void init_2d_obj_cache()
{
	memset(obj_2d_def_cache, 0, sizeof(obj_2d_def_cache));
}

void check_options ()
{

}

void init_stuff()
{
	int seed;

	//TODO: process command line options
	chdir(DATA_DIR);

	//Initialize all strings
	init_translatables();

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

	//Parse command line options
	read_command_line();

	// check for invalid combinations
	check_options ();

	//OK, we have the video mode settings...
	setup_video_mode(full_screen,video_mode);
	//now you may set the video mode using the %<foo> in-game
	video_mode_set=1;

	//Good, we should be in the right working directory - load all translatables from their files
	load_translatables();
	
	init_video();
	// now create the root window

	// XXX FIXME (Grum): Maybe we should do this at a later time, after
	// we're logged in?
	create_game_root_window (window_width, window_height);
	create_console_root_window (window_width, window_height);
	create_map_root_window (window_width, window_height);
	create_login_root_window (window_width, window_height);
	

	init_gl_extensions();

	seed = time (NULL);
	srand (seed);

	
	cache_system_init(MAX_CACHE_SYSTEM);
	init_texture_cache();
	init_e3d_cache();
	init_2d_obj_cache();
	load_ignores();
	load_filters();
	load_harvestable_list();
	load_e3d_list();
	load_entrable_list();
	load_knowledge_list();
	load_cursors();
	build_cursors();
	change_cursor(CURSOR_ARROW);
	build_glow_color_table();

	init_actors_lists();
	memset(tile_list, 0, sizeof(tile_list));
	memset(lights_list, 0, sizeof(lights_list));
	init_particles_list();
	memset(actors_defs, 0, sizeof(actors_defs));
	init_actor_defs();

	load_map_tiles();
	
	//lights setup
	build_global_light_table();
	build_sun_pos_table();
	reset_material();
	init_lights();
	disable_local_lights();
	init_colors();
	clear_error_log();
	clear_conn_log();
	clear_thunders();
	build_rain_table();
	read_bin_cfg();
	build_levels_table();//for some HUD stuff

	if(!no_sound)init_sound();

	// now load the font textures
	load_font_textures ();
	CHECK_GL_ERRORS();

	//load the necesary textures
	//font_text=load_texture_cache("./textures/font.bmp",0);
	icons_text=load_texture_cache("./textures/gamebuttons.bmp",0);
	hud_text=load_texture_cache("./textures/gamebuttons2.bmp",0);
	cons_text=load_texture_cache("./textures/console.bmp",255);
	particle_textures[0]=load_texture_cache("./textures/particle0.bmp",0);
	particle_textures[1]=load_texture_cache("./textures/particle1.bmp",0);
	particle_textures[2]=load_texture_cache("./textures/particle2.bmp",0);
	particle_textures[3]=load_texture_cache("./textures/particle3.bmp",0);
	particle_textures[4]=load_texture_cache("./textures/particle4.bmp",0);
	particle_textures[5]=load_texture_cache("./textures/particle5.bmp",0);
	particle_textures[6]=load_texture_cache("./textures/particle6.bmp",0);
	particle_textures[7]=load_texture_cache("./textures/particle7.bmp",0);

	items_text_1=load_texture_cache("./textures/items1.bmp",0);
	items_text_2=load_texture_cache("./textures/items2.bmp",0);
	items_text_3=load_texture_cache("./textures/items3.bmp",0);
	items_text_4=load_texture_cache("./textures/items4.bmp",0);
	items_text_5=load_texture_cache("./textures/items5.bmp",0);
	items_text_6=load_texture_cache("./textures/items6.bmp",0);
	items_text_7=load_texture_cache("./textures/items7.bmp",0);
	items_text_8=load_texture_cache("./textures/items8.bmp",0);
	items_text_9=load_texture_cache("./textures/items9.bmp",0);
	items_text_10=load_texture_cache("./textures/items10.bmp",0);

	portraits1_tex=load_texture_cache("./textures/portraits1.bmp",0);
	portraits2_tex=load_texture_cache("./textures/portraits2.bmp",0);
	portraits3_tex=load_texture_cache("./textures/portraits3.bmp",0);
	portraits4_tex=load_texture_cache("./textures/portraits4.bmp",0);
	portraits5_tex=load_texture_cache("./textures/portraits5.bmp",0);
	portraits6_tex=load_texture_cache("./textures/portraits6.bmp",0);

	sigils_text=load_texture_cache("./textures/sigils.bmp",0);
	//Load the map legend and continent map
	legend_text=load_texture_cache("./maps/legend.bmp",0);
	cont_text = load_texture_cache (cont_map_file_names[0], 128);
	
	//Paper & book
	paper1_text=load_texture_cache("./textures/paper1.bmp",0);
	book1_text=load_texture_cache("./textures/book1.bmp",0);

	if(have_multitexture)ground_detail_text=load_texture_cache("./textures/ground_detail.bmp",255);
	CHECK_GL_ERRORS();
	create_char_error_str[0]=0;
	init_opening_interface();
	make_sigils_list();

	if(SDLNet_Init()<0)
 		{
			log_error("%s: %s\n", failed_sdl_net_init, SDLNet_GetError());
			SDLNet_Quit();
			SDL_Quit();
			exit(2);
		}

	if(SDL_InitSubSystem(SDL_INIT_TIMER)<0)
		{
			log_error("%s: %s\n", failed_sdl_timer_init, SDL_GetError());
			SDL_Quit();
		 	exit(1);
		}
	ReadXML("languages/en/Encyclopedia/index.xml");
	read_key_config();
	load_questlog();
	init_buddy();

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
	
	SDL_SetGamma(gamma_var, gamma_var, gamma_var);
	
	draw_scene_timer = SDL_AddTimer (1000/(18*4), my_timer, NULL);
	misc_timer = SDL_AddTimer (500, check_misc, NULL);
	
	//we might want to do this later.
//	connect_to_server();

	create_opening_root_window (window_width, window_height);
	// initialize the chat window
	if (use_windowed_chat == 1)
		display_tab_bar ();
	else if (use_windowed_chat == 2)
		display_chat ();

	// display something
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

#ifdef ENCYCLOPEDIA
	bp_init_symbols();
	bp_parseFile(malloc(sizeof(bp_Context)), "languages/en/Encyclopedia/encyclopedia.xml");
#endif

}

void add_key(Uint32 *key,Uint32 n)
{
	if(n==303 || n==304)//shift
		*key|=(1<<31);
	else{
		if(n==305 || n==306)//control
			*key|=(1<<30);
		else{
			if(n==307 || n==308)//alt
				*key|=(1<<29);
			else
				*(Uint16*)key=(Uint16)n;
		}
	}
}
