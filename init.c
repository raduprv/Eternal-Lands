#include <stdlib.h>
#include <sys/stat.h>
#ifndef WINDOWS
#include <dirent.h>
#include <unistd.h>
#endif
#include <string.h>
#include "init.h"
#include <time.h>
#include "global.h"

#ifdef	CACHE_SYSTEM
cache_struct	*cache_md2=NULL;
cache_struct	*cache_e3d=NULL;
#endif	//CACHE_SYSTEM

void load_harvestable_list()
{
	FILE *f = NULL;
	int i=0;
	char strLine[255];

	memset(harvestable_objects, 0, sizeof(harvestable_objects)); 
	i=0;
	f=fopen("harvestable.lst", "rb");
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
	f=fopen("entrable.lst", "rb");
	if(!f)return;
	while(1)
		{
			fscanf(f,"%s",entrable_objects[i].name);
			i++;
			if(!fgets(strLine, 100, f))break;
		}
	fclose(f);
}

void read_config()
{
	FILE *f = NULL;
	Uint8 * file_mem;
	Uint8 * file_mem_start;
	int k,server_address_offset;
	struct stat ini_file;
#ifndef WINDOWS
	char el_ini[256];
	DIR *d = NULL;
	strcpy(configdir, getenv("HOME"));
	strcat(configdir, "/.elc/");
	my_strcp(datadir,".");
	d=opendir(configdir);
	if(!d)
			mkdir(configdir,0755);
	else
		{
			strcpy(el_ini, configdir);
			strcat(el_ini, "el.ini");
			closedir(d);
			f=fopen(el_ini,"rb"); //try to load local settings
		}
	if(!f) //use global settings
		{
			f=fopen("el.ini","rb");
			stat("el.ini",&ini_file);
		}
	else
		stat(el_ini,&ini_file);
#else
	f=fopen("el.ini","rb");
	stat("el.ini",&ini_file);
#endif
	if(!f)//oops, the file doesn't exist, use the defaults
		{
			char str[120];
			sprintf(str, "Fatal: couldn't read configuration file el.ini\n");
			log_error(str);
			SDL_Quit();
			exit(1);
		}
	ini_file_size = ini_file.st_size;
	file_mem = (Uint8 *) calloc(ini_file_size+2, sizeof(Uint8));
	file_mem_start=file_mem;
	fread (file_mem, 1, ini_file_size+1, f);
	//ok, now start to parse the file...
	video_mode=get_integer_after_string("#video_mode",file_mem,ini_file_size);
	shadows_on=get_integer_after_string("#shadows_on",file_mem,ini_file_size);
	poor_man=get_integer_after_string("#poor_man",file_mem,ini_file_size);
	show_reflection=get_integer_after_string("#show_reflection",file_mem,ini_file_size);
	if(show_reflection==-1)show_reflection=1;
	show_fps=get_integer_after_string("#show_fps",file_mem,ini_file_size);
	if(show_fps==-1)show_fps=1;
	limit_fps=get_integer_after_string("#limit_fps",file_mem,ini_file_size);
	if(limit_fps==-1)limit_fps=0;
	mouse_limit=get_integer_after_string("#mouse_limit",file_mem,ini_file_size);
	if(mouse_limit==-1)mouse_limit=15;
	full_screen=get_integer_after_string("#full_screen",file_mem,ini_file_size);
	clouds_shadows=get_integer_after_string("#clouds_shadows",file_mem,ini_file_size);
#ifdef	USE_VERTEXARRAYS
	use_vertex_array=get_integer_after_string("#use_vertex_array",file_mem,ini_file_size);
	if(use_vertex_array < 0) use_vertex_array=0;
	else if(use_vertex_array > 0) log_to_console(c_green2,"Vertex Arrays enabled (memory hog on!)...");
#endif	//USE_VERTEXARRAYS
	sit_lock=get_integer_after_string("#sit_lock",file_mem,ini_file_size);
	if(sit_lock==-1)sit_lock=0;
	use_global_ignores=get_integer_after_string("#use_global_ignores",file_mem,ini_file_size);
	use_global_filters=get_integer_after_string("#use_global_filters",file_mem,ini_file_size);
	caps_filter=get_integer_after_string("#caps_filter",file_mem,ini_file_size);
	if(caps_filter < 0) caps_filter=1;	//default to on
	save_ignores=get_integer_after_string("#save_ignores",file_mem,ini_file_size);
	log_server=get_integer_after_string("#log_server",file_mem,ini_file_size);
	no_sound=get_integer_after_string("#no_sound",file_mem,ini_file_size);
	normal_camera_rotation_speed=get_float_after_string("#normal_camera_rotation_speed",file_mem,ini_file_size);
	fine_camera_rotation_speed=get_float_after_string("#fine_camera_rotation_speed",file_mem,ini_file_size);
	name_zoom=get_float_after_string("#name_text_size",file_mem,ini_file_size);
	if(name_zoom<0.25f)name_zoom=1.0f;
	chat_zoom=get_float_after_string("#chat_text_size",file_mem,ini_file_size);
	if(chat_zoom<0.25f)chat_zoom=1.0f;
	name_font=get_integer_after_string("#name_font",file_mem,ini_file_size);
	chat_font=get_integer_after_string("#chat_font",file_mem,ini_file_size);

	no_adjust_shadows=get_integer_after_string("#no_adjust_shadows",file_mem,ini_file_size);
	port=get_integer_after_string("#server_port",file_mem,ini_file_size);

	//handle multiple setting changes if poor_man is on
	if(poor_man)
		{
			show_reflection=0;
			shadows_on=0;
			clouds_shadows=1;
		}

	//ok, now get the server address
	server_address_offset=get_string_after_string("#server_address",file_mem,ini_file_size,server_address, 70);

	//ok, now get the current browser
	server_address_offset=get_string_after_string("#browser",file_mem,ini_file_size,broswer_name,70);

	//check for a different default text filter phrase
	get_string_after_string("#text_filter_replace",file_mem,ini_file_size,text_filter_replace,127);

	// now the default user and password
	get_string_after_string("#username",file_mem,ini_file_size,username_str,16);
	get_string_after_string("#password",file_mem,ini_file_size,password_str,16);
	for(k=0;k<(int)strlen(password_str);k++) display_password_str[k]='*';
	display_password_str[k]=0;

#ifndef WINDOWS
	if(get_string_after_string("#data_dir",file_mem,ini_file_size,datadir,90)>0)
		chdir(datadir);
#endif

	if(video_mode>10 || video_mode<=0)
		{
			Uint8 str[80];
			video_mode=2;
			//warn about this error
			str[0]=c_red2+128;
			sprintf(&str[1],"Stop playing with the configuration file and select valid modes!");
			put_text_in_buffer(str,strlen(str),0);
		}
	setup_video_mode(full_screen,video_mode);

	fclose(f);
	free(file_mem_start);
}

void read_bin_cfg()
{
	FILE *f = NULL;
	bin_cfg cfg_mem;
	char el_cfg[256];

	strcpy(el_cfg, configdir);
	strcat(el_cfg, "el.cfg");
	f=fopen(el_cfg,"rb");
	if(!f)return;//no config file, use defaults
	memset(&cfg_mem, 0, sizeof(cfg_mem));	// make sure its clean

	fread(&cfg_mem,1,sizeof(cfg_mem),f);
	fclose(f);

	//verify the version number
	if(cfg_mem.cfg_version_num != CFG_VERSION) return; //oops! ignore the file

	//good, retrive the data
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

	options_menu_x=cfg_mem.options_menu_x;
	options_menu_y=cfg_mem.options_menu_y;

	knowledge_menu_x=cfg_mem.knowledge_menu_x;
	knowledge_menu_y=cfg_mem.knowledge_menu_y;

	cx=cfg_mem.camera_x;
	cy=cfg_mem.camera_y;
	cz=cfg_mem.camera_z;
	zoom_level=cfg_mem.zoom_level;
	rz=cfg_mem.camera_angle;

	if(zoom_level != 0.0f) resize_window();

}

void save_bin_cfg()
{
	FILE *f = NULL;
	bin_cfg cfg_mem;
	char el_cfg[256];

	strcpy(el_cfg, configdir);
	strcat(el_cfg, "el.cfg");
	f=fopen(el_cfg,"wb");
	if(!f)return;//blah, whatever
	memset(&cfg_mem, 0, sizeof(cfg_mem));	// make sure its clean

	cfg_mem.cfg_version_num=CFG_VERSION;	// set the version number
	//good, retrive the data
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

	cfg_mem.options_menu_x=options_menu_x;
	cfg_mem.options_menu_y=options_menu_y;

	cfg_mem.knowledge_menu_x=knowledge_menu_x;
	cfg_mem.knowledge_menu_y=knowledge_menu_y;

	cfg_mem.camera_x=cx;
	cfg_mem.camera_y=cy;
	cfg_mem.camera_z=cz;
	cfg_mem.zoom_level=zoom_level;
	cfg_mem.camera_angle=rz;

	fwrite(&cfg_mem,sizeof(cfg_mem),1,f);
	fclose(f);

}

void init_md2_cache()
{
#ifdef	CACHE_SYSTEM
	cache_md2=cache_init(1000, &destroy_md2);	// auto-free permitted
	//cache_md2=cache_init(1000, NULL);	// no auto-free permitted
	cache_set_name(cache_system, "MD2 cache", cache_md2);
	cache_set_compact(cache_md2, &free_md2_va);	// to compact, free VA arrays
	cache_set_time_limit(cache_md2, 5*60*1000);	// check every 5 minutes
	cache_set_size_limit(cache_md2, 64*1024*1024);
#else	//CACHE_SYSTEM
	memset(md2_cache, 0, sizeof(md2_cache)); 
#endif	//CACHE_SYSTEM
}

void init_texture_cache()
{
	memset(texture_cache, 0, sizeof(texture_cache)); 
}

void init_e3d_cache()
{
#ifdef	CACHE_SYSTEM
	//cache_e3d=cache_init(1000, &destroy_e3d);	//TODO: autofree the name as well
	cache_e3d=cache_init(1000, NULL);	//no aut- free permitted
	cache_set_name(cache_system, "E3D cache", cache_e3d);
	cache_set_compact(cache_e3d, &free_e3d_va);	// to compact, free VA arrays
	cache_set_time_limit(cache_e3d, 5*60*1000);
	cache_set_size_limit(cache_e3d, 8*1024*1024);
#else	//CACHE_SYSTEM
	memset(e3d_cache, 0, sizeof(e3d_cache)); 
#endif	//CACHE_SYSTEM
}

void init_2d_obj_cache()
{
	memset(obj_2d_def_cache, 0, sizeof(obj_2d_def_cache)); 
}

void init_stuff()
{
	int seed;
	Uint8 * extensions;
	int ext_str_len;

	Uint32 (*my_timer_pointer) (unsigned int) = my_timer;

	//clear dir pointers to default to current dir
	memset(configdir, 0, 256);
	memset(datadir, 0, 256);
	//TODO: process command line options
	//read the config file
	read_config();

	init_video();
	resize_window();

	seed = time (NULL);
	srand (seed);

#ifdef	CACHE_SYSTEM
	cache_system_init(MAX_CACHE_SYSTEM);
#endif
	init_texture_cache();
	init_md2_cache();
	init_e3d_cache();
	init_2d_obj_cache();
	load_ignores();
	load_filters();
	build_help();
	load_harvestable_list();
	load_entrable_list();
	load_cursors();
	build_cursors();
	change_cursor(CURSOR_ARROW);


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


	if(!no_sound)init_sound();

	//now load the multitexturing extension
	ELglActiveTextureARB = SDL_GL_GetProcAddress("glActiveTextureARB");
	ELglMultiTexCoord2fARB = SDL_GL_GetProcAddress("glMultiTexCoord2fARB");
	ELglMultiTexCoord2fvARB	= SDL_GL_GetProcAddress("glMultiTexCoord2fvARB");
	ELglClientActiveTextureARB = SDL_GL_GetProcAddress("glClientActiveTextureARB");
	ELglLockArraysEXT = SDL_GL_GetProcAddress("glLockArraysEXT");
	ELglUnlockArraysEXT = SDL_GL_GetProcAddress("glUnlockArraysEXT");

	//see if we really have multitexturing
	extensions=(GLubyte *)glGetString(GL_EXTENSIONS);
	ext_str_len=strlen(extensions);
	if(ELglActiveTextureARB && ELglMultiTexCoord2fARB && ELglMultiTexCoord2fvARB && ELglClientActiveTextureARB)
		{
			have_multitexture=get_string_occurance("GL_ARB_multitexture",extensions,ext_str_len,0);
			if(have_multitexture==-1)
				{
					have_multitexture=0;
					log_to_console(c_red1,"Couldn't find the GL_ARB_multitexture extension, giving up clouds shadows, and texture detail...");
				}
			else
				{
					have_multitexture=1;
					log_to_console(c_green2,"GL_ARB_multitexture extension found, using it");
				}
		}
	else
		{
			have_multitexture=0;
			log_to_console(c_red1,"Couldn't find one of the GL_ARB_multitexture functions, giving up clouds shadows, and texture detail...");
		}
	if(ELglLockArraysEXT && ELglUnlockArraysEXT)
		{
			have_compiled_vertex_array=get_string_occurance("GL_EXT_compiled_vertex_array",extensions,ext_str_len,0);
			if(have_compiled_vertex_array < 0)
				{
					have_compiled_vertex_array=0;
					log_to_console(c_red1,"Couldn't find the GL_EXT_compiled_vertex_array extension, not using it...");
				}
			else log_to_console(c_green2,"GL_EXT_compiled_vertex_array extension found, using it...");
		}
	else
		{
			have_compiled_vertex_array=0;
			log_to_console(c_red1,"Couldn't find one of the GL_EXT_compiled_vertex_array functions, not using it...");

		}
	check_gl_errors();


	//initialize the fonts
	init_fonts();
	check_gl_errors();

	//load the necesary textures
	//font_text=load_texture_cache("./textures/font.bmp",0);
	icons_text=load_texture_cache("./textures/gamebuttons.bmp",0);
	cons_text=load_texture_cache("./textures/console.bmp",255);
	sky_text_1=load_texture_cache("./textures/sky.bmp",70);
	particles_text=load_texture_cache("./textures/particles.bmp",0);
	items_text_1=load_texture_cache("./textures/items1.bmp",0);
	items_text_2=load_texture_cache("./textures/items2.bmp",0);
	items_text_3=load_texture_cache("./textures/items3.bmp",0);
	items_text_4=load_texture_cache("./textures/items4.bmp",0);
	items_text_5=load_texture_cache("./textures/items5.bmp",0);
	items_text_6=load_texture_cache("./textures/items6.bmp",0);
	items_text_7=load_texture_cache("./textures/items7.bmp",0);

	portraits1_tex=load_texture_cache("./textures/portraits1.bmp",0);
	portraits2_tex=load_texture_cache("./textures/portraits2.bmp",0);
	portraits3_tex=load_texture_cache("./textures/portraits3.bmp",0);
	portraits4_tex=load_texture_cache("./textures/portraits4.bmp",0);
	portraits5_tex=load_texture_cache("./textures/portraits5.bmp",0);

	sigils_text=load_texture_cache("./textures/sigils.bmp",0);

	if(have_multitexture)ground_detail_text=load_texture_cache("./textures/ground_detail.bmp",255);
	check_gl_errors();
	create_char_error_str[0]=0;
	init_opening_interface();
	init_hud_interface();
	make_sigils_list();

	if(SDLNet_Init()<0)
 		{
			char str[120];
			sprintf(str,"Couldn't initialize net: %s\n",SDLNet_GetError());
			log_error(str);
			SDLNet_Quit();
			SDL_Quit();
			exit(2);
		}

	if(SDL_InitSubSystem(SDL_INIT_TIMER)<0)
		{
 			char str[120];
			sprintf(str, "Couldn't initialize the timer: %s\n", SDL_GetError());
			log_error(str);
			SDL_Quit();
		 	exit(1);
		}
	SDL_SetTimer (1000/(18*4), my_timer_pointer);

	//we might want to do this later.
	connect_to_server();
}


