#include <stdlib.h>
#include <string.h>
#include "init.h"
#include <time.h>
#include "global.h"

void load_harvestable_list()
{
	FILE *f = NULL;
	int i=0;
	char strLine[255];

	for(i=0;i<100;i++)
		{
			harvestable_objects[i].name[0]=0;
		}

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

	for(i=0;i<100;i++)
		{
			entrable_objects[i].name[0]=0;
		}

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

  	f=fopen("el.ini","rb");
  	if(!f)//oops, the file doesn't exist, use the defaults
  		{
			char str[120];
			sprintf(str, "Fatal: couldn't read configuration file el.ini\n");
			log_error(str);
			SDL_Quit();
			exit(1);
		}

  	file_mem = (Uint8 *) calloc(MAX_INI_FILE+2, sizeof(Uint8));
  	file_mem_start=file_mem;
  	fread (file_mem, 1, MAX_INI_FILE+1, f);
  	//ok, now start to parse the file...
  	video_mode=get_integer_after_string("#video_mode",file_mem,MAX_INI_FILE);
  	shadows_on=get_integer_after_string("#shadows_on",file_mem,MAX_INI_FILE);
  	poor_man=get_integer_after_string("#poor_man",file_mem,MAX_INI_FILE);
  	show_reflection=get_integer_after_string("#show_reflection",file_mem,MAX_INI_FILE);
	if(show_reflection==-1)show_reflection=1;
	show_fps=get_integer_after_string("#show_fps",file_mem,MAX_INI_FILE);
	if(show_fps==-1)show_fps=1;
  	mouse_limit=get_integer_after_string("#mouse_limit",file_mem,MAX_INI_FILE);
	if(mouse_limit==-1)mouse_limit=15;
  	full_screen=get_integer_after_string("#full_screen",file_mem,MAX_INI_FILE);
  	clouds_shadows=get_integer_after_string("#clouds_shadows",file_mem,MAX_INI_FILE);
	sit_lock=get_integer_after_string("#sit_lock",file_mem,MAX_INI_FILE);
	if(sit_lock==-1)sit_lock=0;
  	use_global_ignores=get_integer_after_string("#use_global_ignores",file_mem,MAX_INI_FILE);
  	use_global_filters=get_integer_after_string("#use_global_filters",file_mem,MAX_INI_FILE);
  	save_ignores=get_integer_after_string("#save_ignores",file_mem,MAX_INI_FILE);
  	log_server=get_integer_after_string("#log_server",file_mem,MAX_INI_FILE);
  	no_sound=get_integer_after_string("#no_sound",file_mem,MAX_INI_FILE);
  	normal_camera_rotation_speed=get_float_after_string("#normal_camera_rotation_speed",file_mem,MAX_INI_FILE);
  	fine_camera_rotation_speed=get_float_after_string("#fine_camera_rotation_speed",file_mem,MAX_INI_FILE);
  	name_zoom=get_float_after_string("#name_text_size",file_mem,MAX_INI_FILE);
	if(name_zoom<0.25f)name_zoom=1.0f;
  	chat_zoom=get_float_after_string("#chat_text_size",file_mem,MAX_INI_FILE);
	if(chat_zoom<0.25f)chat_zoom=1.0f;
  	name_font=get_integer_after_string("#name_font",file_mem,MAX_INI_FILE);
  	chat_font=get_integer_after_string("#chat_font",file_mem,MAX_INI_FILE);

  	no_adjust_shadows=get_integer_after_string("#no_adjust_shadows",file_mem,MAX_INI_FILE);
  	port=get_integer_after_string("#server_port",file_mem,MAX_INI_FILE);

	//handle multiple setting changes if poor_man is on
	if(poor_man)
		{
			show_reflection=0;
			shadows_on=0;
			clouds_shadows=1;
		}

  	//ok, now get the server address
  	server_address_offset=get_string_after_string("#server_address",file_mem,MAX_INI_FILE,server_address, 70);

  	//ok, now get the current browser
  	server_address_offset=get_string_after_string("#browser",file_mem,MAX_INI_FILE,broswer_name,70);

	//check for a different default text filter phrase
  	get_string_after_string("#text_filter_replace",file_mem,MAX_INI_FILE,text_filter_replace,127);

	// now the default user and password
  	get_string_after_string("#username",file_mem,MAX_INI_FILE,username_str,16);
  	get_string_after_string("#password",file_mem,MAX_INI_FILE,password_str,16);
	for(k=0;k<strlen(password_str);k++) display_password_str[k]='*';
	display_password_str[k]=0;

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

  	f=fopen("el.cfg","rb");
  	if(!f)return;//no config file, use defaults

  	fread(&cfg_mem,1,sizeof(cfg_mem),f);
  	fclose(f);

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

}

void save_bin_cfg()
{
	FILE *f = NULL;
	bin_cfg cfg_mem;

  	f=fopen("el.cfg","wb");
  	if(!f)return;//blah, whatever

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

	fwrite(&cfg_mem,sizeof(cfg_mem),1,f);
	fclose(f);

}

void init_md2_cache()
{
	int i;
	for(i=0;i<1000;i++)
		{
			md2_cache[i].file_name[0]=0;
			md2_cache[i].md2_id=0;
		}
}

void init_texture_cache()
{
	int i;
	for(i=0;i<1000;i++)
		{
			texture_cache[i].file_name[0]=0;
			texture_cache[i].texture_id=0;
		}
}

void init_e3d_cache()
{
	int i;
	for(i=0;i<1000;i++)
		{
			e3d_cache[i].file_name[0]=0;
			e3d_cache[i].e3d_id=0;
		}
}

void init_2d_obj_cache()
{
	int i;
	for(i=0;i<max_obj_2d_def;i++)
		{
			obj_2d_def_cache[i].file_name[0]=0;
			obj_2d_def_cache[i].obj_2d_def_id=0;
		}
}

void init_stuff()
{
	int i;
	int seed;
	Uint8 * extensions;
	int ext_str_len;

	Uint32 (*my_timer_pointer) (unsigned int) = my_timer;

	//read the config file
	read_config();

	init_video();
	resize_window();

    seed = time (NULL);
  	srand (seed);

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
	for(i=0;i<256;i++)tile_list[i]=0;
	for(i=0;i<max_lights;i++)lights_list[i]=0;
	init_particles_list();
	for(i=0;i<30*sizeof(actor_types);i++)
		{
			char *pointer_actors_defs=(char *)actors_defs;
			pointer_actors_defs[i]=0;
		}
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
#ifdef WINDOWS
	glActiveTextureARB		= (PFNGLACTIVETEXTUREARBPROC)		SDL_GL_GetProcAddress("glActiveTextureARB");
	glMultiTexCoord2fARB	= (PFNGLMULTITEXCOORD2FARBPROC)		SDL_GL_GetProcAddress("glMultiTexCoord2fARB");
	glMultiTexCoord2fvARB	= (PFNGLMULTITEXCOORD2FVARBPROC)	SDL_GL_GetProcAddress("glMultiTexCoord2fvARB");
	glClientActiveTextureARB= (PFNGLCLIENTACTIVETEXTUREARBPROC)	SDL_GL_GetProcAddress("glClientActiveTextureARB");
	glLockArraysEXT			= (PFNGLLOCKARRAYSEXTPROC)			SDL_GL_GetProcAddress("glLockArraysEXT");
	glUnlockArraysEXT		= (PFNGLUNLOCKARRAYSEXTPROC)		SDL_GL_GetProcAddress("glUnlockArraysEXT");
#endif

	//see if we really have multitexturing
	extensions=(GLubyte *)glGetString(GL_EXTENSIONS);
	ext_str_len=strlen(extensions);
	have_multitexture=get_string_occurance("GL_ARB_multitexture",extensions,ext_str_len,0);
	if(have_multitexture==-1)
		{
			have_multitexture=0;
			log_to_console(c_red1,"Couldn't find the GL_ARB_multitexture extension, giving up clouds shadows, and texture detail...");
		}
	else have_multitexture=1;
#ifdef	USE_VERTEXARRAYS
	have_vertex_array=get_string_occurance("GL_EXT_vertex_array",extensions,ext_str_len,0);
	if(have_vertex_array < 0)have_vertex_array=0;
	if(have_vertex_array < 0)
		{
			have_compiled_vertex_array=get_string_occurance("GL_EXT_compiled_vertex_array",extensions,ext_str_len,0);
			if(have_compiled_vertex_array < 0)have_compiled_vertex_array=0;
		}
#endif
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
	init_peace_icons_position();
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

    //VERY test
    //play_music();

}


