#include <stdio.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <string.h>
#include "translate.h"
#include "stats.h"

#define GROUP 0
#define DIGROUP 1
#define STAT_GROUP 2

typedef struct
{
	char xml_id[15];
	char * var;
	int max_len;
} string_item;

typedef struct
{
	char xml_id[15];
	dichar * var;
} distring_item;

typedef struct
{
	char xml_id[15];
	names * var;
} statstring_item;

typedef struct
{
	char xml_id[15];
	int no;
	string_item ** strings;
} group_id;

typedef struct
{
	char xml_id[15];
	int no;
	distring_item ** distrings;
} group_id_di;

typedef struct
{
	char xml_id[15];
	int no;
	statstring_item ** statstrings;
} group_stat;

//Tooltips
char	tt_walk[30],
	tt_sit[30],
	tt_stand[30],
	tt_look[30],
	tt_use[30],
	tt_trade[30],
	tt_attack[30],
	tt_inventory[30],
	tt_spell[30],
	tt_manufacture[30],
	tt_stats[30],
	tt_knowledge[30],
	tt_encyclopedia[30],
	tt_questlog[30],
	tt_mapwin[30],
	tt_console[30],
	tt_buddy[30],
	tt_options[30];

//Options
dichar	opt_shadows,
	opt_clouds,
	opt_reflections,
	opt_show_fps,
	opt_sit_lock,
	opt_caps_filter,
	opt_sound,
	opt_music,
	opt_autocam,
	opt_exit,
	opt_full_screen,
	opt_strings;

char 	switch_video_mode[50],
	opt_options[20],
	opt_vidmode[20];

//Sigils
char 	sig_too_few_sigs[50];

dichar	sig_change,
	sig_restore,
	sig_space,
	sig_increase,
	sig_decrease,
	sig_temp,
	sig_perm,
	sig_move,
	sig_local,
	sig_global,
	sig_fire,
	sig_water,
	sig_air,
	sig_earth,
	sig_spirit,
	sig_matter,
	sig_energy,
	sig_magic,
	sig_destroy,
	sig_create,
	sig_knowledge,
	sig_protection,
	sig_remove,
	sig_health,
	sig_life,
	sig_death;

//Help messages
char	
	/*3d_objects.c*/
	values_str[20],
	/*draw_scene.c*/
	low_framerate_str[100],
	/*gl_init.c*/
	window_size_adjusted_str[50],
	/*hud.c*/
	no_open_on_trade[100],
	/*interface.c*/
	login_username_str[20],
	login_password_str[20],
	get_all_str[8],
	/*knowledge.c*/
	completed_research[12],
	researching_str[30],
	not_researching_anything[25],
	/*manufacture.c*/
	mix_str[4],
	clear_str[6],
	/*multiplayer.c*/
	connect_to_server_str[50],
	reconnect_str[50],
	alt_x_quit[30],
	license_check[150], 
	/*new_character.c*/
	skin_str[10],
	hair_str[10],
	shirt_str[10],
	pants_str[10],
	boots_str[10],
	head_str[10],
	gender_str[10],
	male_str[10],
	female_str[10],
	race_str[10],
	human_str[10],
	elf_str[10],
	dwarf_str[10],
	confirm_password[30],
	/*pm_log.c*/
	going_afk[30],
	not_afk[50],
	new_messages[100],
	afk_names[15],
	afk_messages[25],
	afk_print_help[150],
	/*trade.c*/
	quantity_str[30],
	abort_str[10];

//Console
char	name_too_long[75], 
	name_too_short[75],
	not_added_to_ignores[75],
	already_ignoring[50],
	ignore_list_full[100],
	added_to_ignores[50],
	no_ignores_str[50],
	ignores_str[50],
	word_too_long[75],
	word_too_short[75],
	not_added_to_filter[50],
	already_filtering[50],
	filter_list_full[100],
	added_to_filters[50],
	no_filters_str[50],
	filters_str[50],
	not_removed_from_ignores[50],
	not_ignoring[75],
	removed_from_ignores[50],
	not_removed_from_filter[50],
	not_filtering[75],
	removed_from_filter[50],
	video_card_str[20],
	video_vendor_str[20],
	opengl_version_str[20],
	supported_extensions_str[30],
	logconn_str[50];

//Errors
char	error_str[15],
	/*2d_objects.c*/
	cant_load_2d_object[30],
	cant_open_file[30],
	/*3d_objects.c*/
	object_error_str[30], 
	nasty_error_str[50],
	corrupted_object[100],
	bad_object[30],
	multiple_material_same_texture[100], 
	/*actors.c*/
	cant_load_actor[30],
	cant_find_frame[30],
	unknown_frame[20],
	/*actor_scripts.c*/
	resync_server[50],
	cant_add_command[50],
	/*cache.c*/
	cache_size_str[20],
	/*cursors.c*/
	cursors_file_str[30],
	/*dialogues.c*/
	close_str[20],
	/*font.c*/
	cant_load_font[30],
	/*init.c*/
	no_stencil_str[150],
	safemode_str[150],
	no_sdl_str[30],
	no_hardware_stencil_str[150],
	suggest_24_or_32_bit[150],
	fail_opengl_mode[30],
	stencil_falls_back_on_software_accel[150],
	last_chance_str[150],
	software_mode_str[150],
	gl_ext_found[100],
	gl_ext_found_not_used[100],
	gl_ext_not_found[100],
	gl_ext_no_multitexture[150],
	fatal_error_str[10],
	no_e3d_list[50],
	enabled_vertex_arrays[50],
	disabled_point_particles[30],
	disabled_particles_str[30],
	invalid_video_mode[75],
	failed_sdl_net_init[30],
	failed_sdl_timer_init[30], 
	cant_read_elini[50],
	/*multiplayer.c*/
	failed_resolve[150],
	failed_connect[100], 
	error_username_length[50], 
	error_password_length[50], 
	error_pass_no_match[30],
	invalid_pass[30],
	redefine_your_colours[200],
	char_dont_exist[30],
	char_name_in_use[50],
	server_latency[30],
	update_your_client[100],
	client_ver_not_supported[100],
	packet_overrun[50],
	disconnected_from_server[100],
	/*new_actors.c*/
	error_body_part[30],
	error_head[15],
	error_torso[15],
	error_weapon[15],
	error_helmet[15],
	error_cape[15],
	duplicate_npc_actor[50],
	duplicate_actors_str[50],
	bad_actor_name_length[50],
	/*particles.c*/
	particles_filever_wrong[100],
	particle_system_overrun[100],
	particle_strange_pos[100],
	particle_system_dump[50],
	particles_disabled_str[50],
	point_sprites_enabled[50],
	using_textured_quads[50],
	definitions_str[20],
	part_sys_str[20],
	part_part_str[20],
	/*paste.c*/
	not_ascii[20],
	/*sound.c*/
	snd_ogg_load_error[50],
	snd_ogg_stream_error[50],
	snd_buff_error[50],
	snd_invalid_number[50],
	snd_source_error[50],
	snd_skip_speedup[50],
	snd_too_slow[50],
	snd_stop_fail[50],
	snd_init_error[50],
	snd_sound_overflow[50],
	snd_media_read[50],
	snd_media_notvorbis[50],
	snd_media_ver_mismatch[50],
	snd_media_invalid_header[50],
	snd_media_internal_error[50],
	snd_media_ogg_error[50],
	/*stats.c*/
	stat_no_invalid[50]; 

#define ERRORS 6
group_id errors[ERRORS]=
	{
		{ "actors", 0, NULL },
		{ "load", 0, NULL },
		{ "misc", 0, NULL },
		{ "particles", 0, NULL },
		{ "snd", 0, NULL },
		{ "video", 0, NULL }
	};

#define HELP_STR 4
group_id help_str[HELP_STR]=
	{
		{ "afk", 0, NULL },
		{ "misc", 0, NULL },
		{ "new", 0, NULL },
		{ "tooltips", 0, NULL }
	};

#define CONSOLE_STR 3
group_id console_str[CONSOLE_STR]=
	{
		{ "filter", 0, NULL },
		{ "ignore", 0, NULL },
		{ "misc", 0, NULL }
	};

#define OPTIONS_STR 1
group_id_di options_str[OPTIONS_STR] = { {"options", 0, NULL} };

#define SIGILS_STR 1
group_id_di sigils_str[SIGILS_STR] = { {"sigils", 0, NULL} };

#define STATS_STR 5
group_stat stats_str[STATS_STR]=
	{
		{ "base", 0, NULL },
		{ "cross", 0, NULL },
		{ "misc", 0, NULL },
		{ "nexus", 0, NULL },
		{ "skills", 0, NULL },
	};

#define STATS_EXTRA 1
group_id stats_extra[STATS_EXTRA] = { {"extra", 0, NULL }};

void add_xml_distringid(group_id_di * group, char * xml_id, dichar * var, char * str, char * desc)
{
	group->distrings=(distring_item**)realloc(group->distrings,(group->no+1)*sizeof(distring_item*));
	group->distrings[group->no]=(distring_item*)calloc(1,sizeof(distring_item));
	strcpy(group->distrings[group->no]->xml_id,xml_id);
	group->distrings[group->no]->var=var;
	strncpy(var->str, str, 30);
	strncpy(var->desc, desc, 100);
	group->no++;
}

void add_xml_statid(group_stat * group, char * xml_id, names * var, char * name, char * shortname)
{
	group->statstrings=(statstring_item**)realloc(group->statstrings,(group->no+1)*sizeof(statstring_item*));
	group->statstrings[group->no]=(statstring_item*)calloc(1,sizeof(statstring_item));
	strcpy(group->statstrings[group->no]->xml_id,xml_id);
	group->statstrings[group->no]->var=var;
	strncpy(var->name, name, 20);
	strncpy(var->shortname, shortname, 5);
	group->no++;
}

void add_xml_identifier(group_id * group, char * xml_id, char * var, char * def, int max_len)
{
	group->strings=(string_item**)realloc(group->strings,(group->no+1)*sizeof(string_item*));
	group->strings[group->no]=(string_item*)calloc(1,sizeof(string_item));
	strcpy(group->strings[group->no]->xml_id,xml_id);
	group->strings[group->no]->var=var;
	strncpy(var, def, max_len);
	group->strings[group->no]->max_len=max_len;
	group->no++;
}

void init_console();
void init_errors();
void init_help();
void init_options();
void init_spells();
void init_stats();

void init_translatables()
{
	init_console();
	init_errors();
	init_help();
	init_options();
	init_spells();
	init_stats();
}

void init_console()
{
	group_id * filter=&(console_str[0]);
	group_id * ignore=&(console_str[1]);
	group_id * misc=&(console_str[2]);
	
	add_xml_identifier(ignore,"toolong",name_too_long,"Name too long, the max limit is 15 characters.",75);
	add_xml_identifier(ignore,"tooshort",name_too_short,"Name too short, only names>=3 characters can be used!",75);
	add_xml_identifier(ignore,"noadd",not_added_to_ignores,"Name not added to the ignore list!",75);
	add_xml_identifier(ignore,"already",already_ignoring,"You are already ignoring %s!",50);
	add_xml_identifier(ignore,"full",ignore_list_full,"Wow, your ignore list is full, this is impossible!",100);
	add_xml_identifier(ignore,"add",added_to_ignores,"%s was added to your ignore list!",50);
	add_xml_identifier(ignore,"norem",not_removed_from_ignores,"Name not removed from the ignore list!",50);
	add_xml_identifier(ignore,"not",not_ignoring,"You are NOT ignoring %s in the first place!",75);
	add_xml_identifier(ignore,"rem",removed_from_ignores,"OK, %s was removed from your ignore list!",50);
	add_xml_identifier(ignore,"none",no_ignores_str,"You are ignoring no one!",50);
	add_xml_identifier(ignore,"cur",ignores_str,"You are currently ignoring",50);
	
	add_xml_identifier(filter,"toolong",word_too_long,"Word too long, the max limit is 15 characters.",75);
	add_xml_identifier(filter,"tooshort",word_too_short,"Word too short, only words>=3 characters can be used!",75);
	add_xml_identifier(filter,"notadd",not_added_to_filter,"Word not added to the filter list!",50);
	add_xml_identifier(filter,"already",already_filtering,"You are already filtering %s",50);
	add_xml_identifier(filter,"add",added_to_filters,"OK, %s was added to your filter list!",50);
	add_xml_identifier(filter,"norem",not_removed_from_filter,"Word not removed from the filter list!",50);
	add_xml_identifier(filter,"not",not_filtering,"You are NOT filtering %s in the first place!",75);
	add_xml_identifier(filter,"rem",removed_from_filter,"OK, %s was removed from your filter list!",50);
	add_xml_identifier(filter,"none",no_filters_str,"You are filtering nothing!",50);
	add_xml_identifier(filter,"cur",filters_str,"You are currently filtering",50);
	
	add_xml_identifier(misc,"log",logconn_str,"Logging raw connection data",50);
	add_xml_identifier(misc,"card",video_card_str,"Video card",20);
	add_xml_identifier(misc,"vendor",video_vendor_str,"Vendor ID",20);
	add_xml_identifier(misc,"ext",supported_extensions_str,"Supported extensions",30);
	add_xml_identifier(misc,"opengl",opengl_version_str,"OpenGL Version",20);
}

void init_errors()
{
	group_id * actors=&(errors[0]);
	group_id * load=&(errors[1]);
	group_id * misc=&(errors[2]);
	group_id * particles=&(errors[3]);
	group_id * video=&(errors[4]);
	group_id * snd=&(errors[5]);
	
	//Actor related errors
	add_xml_identifier(actors,"load",cant_load_actor,"Can't load actor",30);
	add_xml_identifier(actors,"frame",cant_find_frame,"Couldn't find frame",30);
	add_xml_identifier(actors,"unk_frame",unknown_frame,"Unknown frame",20);
	add_xml_identifier(actors,"dup_id",duplicate_actors_str,"Duplicate actor ID",30);
	add_xml_identifier(actors,"namelen",bad_actor_name_length,"Bad actor name/length",30);
	add_xml_identifier(actors,"addcommand",cant_add_command,"Unable to add command",50);
	add_xml_identifier(actors,"loadbody",error_body_part,"Can't load body part",30);
	add_xml_identifier(actors,"head",error_head,"head",15);
	add_xml_identifier(actors,"torso",error_torso,"torso",15);
	add_xml_identifier(actors,"weapon",error_weapon,"weapon",15);
	add_xml_identifier(actors,"helmet",error_helmet,"helmet",15);
	add_xml_identifier(actors,"cape",error_cape,"cape",15);
	add_xml_identifier(actors,"dupnpc",duplicate_npc_actor,"Duplicate actor name",50);
	
	//Loading errors
	add_xml_identifier(load,"2dobj",cant_load_2d_object,"Can't load 2d object",30);
	add_xml_identifier(load,"file",cant_open_file,"Can't open file",30);
	add_xml_identifier(load,"cursors",cursors_file_str,"Can't open cursors file.",30);
	add_xml_identifier(load,"font",cant_load_font,"Unable to load font",30);
	add_xml_identifier(load,"fatal",fatal_error_str,"Fatal",10);
	add_xml_identifier(load,"noe3d",no_e3d_list,"Couldn't read e3dlist.txt",50);
	add_xml_identifier(load,"elini",cant_read_elini,"Couldn't read configuration file el.ini",50);
	
	//Miscellaneous errors
	add_xml_identifier(misc,"error",error_str,"Error",15);
	add_xml_identifier(misc,"objerr",object_error_str,"Object error",30);
	add_xml_identifier(misc,"nasty",nasty_error_str,"Something nasty happened while trying to process: %s",50);
	add_xml_identifier(misc,"corrupt",corrupted_object,"Object seems to be corrupted. Skipping the object. Warning: This might cause further problems.",100);
	add_xml_identifier(misc,"badobj",bad_object,"Bad object",30);
	add_xml_identifier(misc,"multimat",multiple_material_same_texture,"Two or more materials with the same texture name!",100);
	add_xml_identifier(misc,"resync",resync_server,"Resync with the server...",50);
	add_xml_identifier(misc,"vertex",enabled_vertex_arrays,"Vertex Arrays enabled (memory hog on!)...",50);
	add_xml_identifier(misc,"point",disabled_point_particles,"Point Particles disabled.",50);
	add_xml_identifier(misc,"particles",disabled_particles_str,"Particles completely disabled!",50);
	add_xml_identifier(misc,"net",failed_sdl_net_init,"Couldn't initialize net",30);
	add_xml_identifier(misc,"timer",failed_sdl_timer_init,"Couldn't initialize the timer",30);
	add_xml_identifier(misc,"resolve",failed_resolve,"Can't resolve server address.\nPerhaps you are not connected to the Internet or your DNS server is down!",150);
	add_xml_identifier(misc,"connect",failed_connect,"Can't connect to server :(",100);
	add_xml_identifier(misc,"userlen",error_username_length,"Username MUST be at least 3 characters long!",50);
	add_xml_identifier(misc,"passlen",error_password_length,"The password MUST be at least 4 characters long!",50);
	add_xml_identifier(misc,"passnomatch",error_pass_no_match,"Passwords don't match!",30);
	add_xml_identifier(misc,"wrongpass",invalid_pass,"Invalid password!",30);
	add_xml_identifier(misc,"redefine",redefine_your_colours,"You need to update your character, due to the new models!\nGo on the New Character screen, type your existing\nusername and password, update your character, then press\nDone. *YOUR STATS AND ITEMS WILL NOT BE AFFECTED*",200);
	add_xml_identifier(misc,"noexist",char_dont_exist,"You don't exist!",30);
	add_xml_identifier(misc,"latency",server_latency,"Server latency",30);
	add_xml_identifier(misc,"newver",update_your_client,"There is a new version of the client, please update it",100);
	add_xml_identifier(misc,"notsup",client_ver_not_supported,"This version is no longer supported, please update!",100);
	add_xml_identifier(misc,"packets",packet_overrun,"Packet overrun...data lost!",50);
	add_xml_identifier(misc,"disconnect",disconnected_from_server,"Disconnected from server!",50);
	add_xml_identifier(misc,"stat",stat_no_invalid,"Server sent invalid stat number",50);
	add_xml_identifier(misc,"ascii",not_ascii,"Not ASCII",20);
	
	//Particle errors
	add_xml_identifier(particles,"version",particles_filever_wrong,"Particle file %s version (%i) doesn't match file reader version (%i)!",100);
	add_xml_identifier(particles,"overrun",particle_system_overrun,"Particle file %s tries to define %i particles, when %i is the maximum!",100);
	add_xml_identifier(particles,"pos",particle_strange_pos,"Particle file %s contained strange position/constraint values. Tried to fix.",100);
	add_xml_identifier(particles,"sysdump",particle_system_dump,"-- PARTICLE SYSTEM DUMP --",50);
	add_xml_identifier(particles,"disabled",particles_disabled_str,"Particles disabled!",50);
	add_xml_identifier(particles,"point",point_sprites_enabled,"Using point sprites",50);
	add_xml_identifier(particles,"quads",using_textured_quads,"Using textured quads",50);
	add_xml_identifier(particles,"defs",definitions_str,"Definitions",20);
	add_xml_identifier(particles,"system",part_sys_str,"systems",20);
	add_xml_identifier(particles,"particles",part_part_str,"particles",20);
	
	//Sound errors
	add_xml_identifier(snd,"loadfile",snd_ogg_load_error,"Failed to load ogg file",50);
	add_xml_identifier(snd,"loadstream",snd_ogg_stream_error,"Failed to load ogg stream",50);
	add_xml_identifier(snd,"buffer",snd_buff_error,"Error creating buffer",50);
	add_xml_identifier(snd,"number",snd_invalid_number,"Got invalid sound number",50);
	add_xml_identifier(snd,"source",snd_source_error,"Error creating a source",50);
	add_xml_identifier(snd,"skip",snd_skip_speedup,"Skip! Speeding up...",50);
	add_xml_identifier(snd,"tooslow",snd_too_slow,"Sorry, too slow to play music...",50);
	add_xml_identifier(snd,"fail",snd_stop_fail,"Failed to stop all sounds.",50);
	add_xml_identifier(snd,"init",snd_init_error,"Error initializing sound",50);
	add_xml_identifier(snd,"toomany",snd_sound_overflow,"Too many sounds.",50);
	add_xml_identifier(snd,"read",snd_media_read,"Read from media.",50);
	add_xml_identifier(snd,"notvorbis",snd_media_notvorbis,"Not Vorbis data.",50);
	add_xml_identifier(snd,"version",snd_media_ver_mismatch,"Vorbis version mismatch.",50);
	add_xml_identifier(snd,"header",snd_media_invalid_header,"Invalid Vorbis header.",50);
	add_xml_identifier(snd,"intern",snd_media_internal_error,"Internal logic fault (bug or heap/stack corruption.",50);
	add_xml_identifier(snd,"unknown",snd_media_ogg_error,"Unknown Ogg error.",50);
	
	//Video errors
	add_xml_identifier(video,"nostencil",no_stencil_str,"Video mode %s with a stencil buffer is not available\nTrying this mode without a stencil buffer...",150);
	add_xml_identifier(video,"safemode",safemode_str,"Video mode %s without a stencil buffer is not available\nTrying the safemode (640x480x32) Full Screen (no stencil)",150);
	add_xml_identifier(video,"nosdl",no_sdl_str,"Couldn't initialize SDL",30);
	add_xml_identifier(video,"nohwstencil",no_hardware_stencil_str,"Couldn't find a hardware accelerated stencil buffer.\nShadows are not available.",150);
	add_xml_identifier(video,"depth",suggest_24_or_32_bit,"Hint: Try a 32 BPP resolution (if you are under XWindows, set your screen display to 24 or 32 bpp).",150);
	add_xml_identifier(video,"glmode",fail_opengl_mode,"Couldn't set GL mode",30);
	add_xml_identifier(video,"swstencil",stencil_falls_back_on_software_accel,"Hmm... This mode seems to fall back in software 'acceleration'.\nTrying to disable the stencil buffer.",150);
	add_xml_identifier(video,"last_try",last_chance_str,"Hmm... No luck without a stencil buffer either...\nLet's try one more thing...",150);
	add_xml_identifier(video,"swmode",software_mode_str,"Damn, it seems that you are out of luck, we are in the software mode now, so the game will be veeeeery slow. If you DO have a 3D accelerated card, try to update your OpenGl drivers...",200);
	add_xml_identifier(video,"extfound",gl_ext_found,"%s extension found, using it.",100);
	add_xml_identifier(video,"extnouse",gl_ext_found_not_used,"%s extension found, NOT using it...",100);
	add_xml_identifier(video,"extnotfound",gl_ext_not_found,"Couldn't find the %s extension, not using it...",100);
	add_xml_identifier(video,"multitex",gl_ext_no_multitexture,"Couldn't find the GL_ARB_multitexture extension, giving up clouds shadows, and texture detail...",150);
	add_xml_identifier(video,"invalid",invalid_video_mode,"Stop playing with the configuration file and select valid modes!",75);
}

void init_help()
{
	group_id * afk = &(help_str[0]);
	group_id * misc = &(help_str[1]);
	group_id * new = &(help_str[2]);
	group_id * tooltips = &(help_str[3]);

	//AFK Messages
	add_xml_identifier(afk,"going",going_afk,"Going AFK",30);
	add_xml_identifier(afk,"not",not_afk,"Not AFK any more",50);
	add_xml_identifier(afk,"back",new_messages,"You have %d new messages from the following people: ",100);
	add_xml_identifier(afk,"names",afk_names,"Names",15);
	add_xml_identifier(afk,"messages",afk_messages,"Messages",25);
	add_xml_identifier(afk,"help",afk_print_help,"To print the messages from the different people type #msg <number> or #msg all to view them all",150);
	//Miscellaneous
	add_xml_identifier(misc,"values",values_str,"values",20);
	add_xml_identifier(misc,"close",close_str,"[close]",20);
	add_xml_identifier(misc,"low",low_framerate_str,"Low framerate detected, shadows disabled!",100);
	add_xml_identifier(misc,"size",window_size_adjusted_str,"Window size adjusted to %s",50);
	add_xml_identifier(misc,"trade",no_open_on_trade,"You can't open this window while on trade.",100);
	add_xml_identifier(misc,"user",login_username_str,"Username",20);
	add_xml_identifier(misc,"pass",login_password_str,"Password",20);
	add_xml_identifier(misc,"getall",get_all_str,"Get All",7);
	add_xml_identifier(misc,"completed",completed_research,"COMPLETED",12);
	add_xml_identifier(misc,"research",researching_str,"Researching",30);
	add_xml_identifier(misc,"nothing",not_researching_anything,"Nothing",25);
	add_xml_identifier(misc,"mix",mix_str,"Mix",4);
	add_xml_identifier(misc,"clear",clear_str,"Clear",6);
	add_xml_identifier(misc,"connect",connect_to_server_str,"Connecting to Server...",50);
	add_xml_identifier(misc,"reconnect",reconnect_str,"Press any key to try again.",50);
	add_xml_identifier(misc,"license",license_check,"Entropy says: U R 2 g00d 2 r34d +h3 license.txt?\nBTW, that license.txt file is actually there for a reason.",20);
	add_xml_identifier(misc,"quantity",quantity_str,"Quantity",30);
	add_xml_identifier(misc,"abort",abort_str,"Abort",6);
	add_xml_identifier(misc,"sigils",sig_too_few_sigs,"This spell requires at least 2 sigils",50);
	add_xml_identifier(misc,"switch",switch_video_mode,"Switches to video mode %s",50);
	add_xml_identifier(misc,"cache",cache_size_str,"Cache size",20);

	//New characters
	add_xml_identifier(new,"skin",skin_str,"Skin",10);
	add_xml_identifier(new,"hair",hair_str,"Hair",10);
	add_xml_identifier(new,"shirt",shirt_str,"Shirt",10);
	add_xml_identifier(new,"pants",pants_str,"Pants",10);
	add_xml_identifier(new,"boots",boots_str,"Boots",10);
	add_xml_identifier(new,"head",head_str,"Head",10);
	add_xml_identifier(new,"gender",gender_str,"Gender",10);
	add_xml_identifier(new,"male",male_str,"Male",10);
	add_xml_identifier(new,"female",female_str,"Female",10);
	add_xml_identifier(new,"race",race_str,"Race",10);
	add_xml_identifier(new,"human",human_str,"Human",10);
	add_xml_identifier(new,"elf",elf_str,"Elf",10);
	add_xml_identifier(new,"dwarf",dwarf_str,"Dwarf",10);
	add_xml_identifier(new,"confirm",confirm_password,"Confirm Password",30);
	
	//Icons
	add_xml_identifier(tooltips,"walk",tt_walk,"Walk",30);
	add_xml_identifier(tooltips,"sit",tt_sit,"Sit down",30);
	add_xml_identifier(tooltips,"stand",tt_stand,"Stand up",30);
	add_xml_identifier(tooltips,"look",tt_look,"Look at",30);
	add_xml_identifier(tooltips,"use",tt_use,"Use",30);
	add_xml_identifier(tooltips,"trade",tt_trade,"Trade",30);
	add_xml_identifier(tooltips,"attack",tt_attack,"Attack",30);
	add_xml_identifier(tooltips,"invent",tt_inventory,"View inventory",30);
	add_xml_identifier(tooltips,"spell",tt_spell,"View spell window",30);
	add_xml_identifier(tooltips,"manu",tt_manufacture,"View manufacture window",30);
	add_xml_identifier(tooltips,"stats",tt_stats,"View stats",30);
	add_xml_identifier(tooltips,"know",tt_knowledge,"View knowledge window",30);
	add_xml_identifier(tooltips,"ency",tt_encyclopedia,"View encyclopedia window",30);
	add_xml_identifier(tooltips,"quest",tt_questlog,"View questlog",30);
	add_xml_identifier(tooltips,"map",tt_mapwin,"View map",30);
	add_xml_identifier(tooltips,"console",tt_console,"View console",30);
	add_xml_identifier(tooltips,"buddy",tt_buddy,"View buddy",30);
	add_xml_identifier(tooltips,"opts",tt_options,"View options",30);
}

void init_options()
{
	//Options
	add_xml_distringid(options_str,"shadows",&opt_shadows,"Shadows","Enables shadows - disable if you experience performance problems");
	add_xml_distringid(options_str,"clouds",&opt_clouds,"Clouds","Enables clouds - disable if you experience performance problems");
	add_xml_distringid(options_str,"reflections",&opt_reflections,"Reflections","Enable reflections - disable if you experience performance problems");
	add_xml_distringid(options_str,"fps",&opt_show_fps,"Show FPS","Show the current framerate in upper left corner");
	add_xml_distringid(options_str,"sitlock",&opt_sit_lock,"Sit lock","Locks you in a sitting position untill you press the \"Stand\" button or rightclicks to move.");
	add_xml_distringid(options_str,"caps",&opt_caps_filter,"Filter CAPS","Turns on/off a filter for capitalized letters");
	add_xml_distringid(options_str,"sound",&opt_sound,"Sound","Turns on/off sound effects");
	add_xml_distringid(options_str,"music",&opt_music,"Music","Turns on/off in-game music");
	add_xml_distringid(options_str,"autocam",&opt_autocam,"Auto Camera","Automatically change the camera according to the actor position");
	add_xml_distringid(options_str,"exit",&opt_exit,"Exit","Exits the game");
	add_xml_distringid(options_str,"fullscreen",&opt_full_screen,"Full Screen","Switches between full screen and windowed mode");
	add_xml_distringid(options_str,"strings",&opt_strings,"Options","Video mode");
}

void init_spells()
{
	//Sigils
	add_xml_distringid(sigils_str,"change",&sig_change,"Change","");
	add_xml_distringid(sigils_str,"restore",&sig_restore,"Restore","");
	add_xml_distringid(sigils_str,"space",&sig_space,"Space","");
	add_xml_distringid(sigils_str,"increase",&sig_increase,"Increase","");
	add_xml_distringid(sigils_str,"decrease",&sig_decrease,"Decrease","");
	add_xml_distringid(sigils_str,"temporary",&sig_temp,"Temporary","");
	add_xml_distringid(sigils_str,"permanent",&sig_perm,"Permanent","");
	add_xml_distringid(sigils_str,"move",&sig_move,"Move","");
	add_xml_distringid(sigils_str,"local",&sig_local,"Local","");
	add_xml_distringid(sigils_str,"global",&sig_global,"Global","");
	add_xml_distringid(sigils_str,"fire",&sig_fire,"Fire","");
	add_xml_distringid(sigils_str,"water",&sig_water,"Water","");
	add_xml_distringid(sigils_str,"air",&sig_air,"Air","");
	add_xml_distringid(sigils_str,"earth",&sig_earth,"Earth","");
	add_xml_distringid(sigils_str,"spirit",&sig_spirit,"Spirit","");
	add_xml_distringid(sigils_str,"matter",&sig_matter,"Matter","");
	add_xml_distringid(sigils_str,"energy",&sig_energy,"Energy","");
	add_xml_distringid(sigils_str,"magic",&sig_magic,"Magic","");
	add_xml_distringid(sigils_str,"destroy",&sig_destroy,"Destroy","");
	add_xml_distringid(sigils_str,"create",&sig_create,"Create","");
	add_xml_distringid(sigils_str,"knowledge",&sig_knowledge,"Knowledge","");
	add_xml_distringid(sigils_str,"protection",&sig_protection,"Protection","");
	add_xml_distringid(sigils_str,"remove",&sig_remove,"Remove","");
	add_xml_distringid(sigils_str,"health",&sig_health,"Health","");
	add_xml_distringid(sigils_str,"life",&sig_life,"Life","");
	add_xml_distringid(sigils_str,"death",&sig_death,"Death","");
}

void init_stats()
{
	group_stat * base = &(stats_str[0]);
	group_stat * cross = &(stats_str[1]);
	group_stat * misc = &(stats_str[2]);
	group_stat * nexus = &(stats_str[3]);
	group_stat * skills = &(stats_str[4]);

	memset(&attributes, 0, sizeof(attributes));
	
	//Initial strings
	add_xml_identifier(stats_extra,"base",attributes.base,"Basic Attributes",30);
	add_xml_identifier(stats_extra,"cross",attributes.cross,"Cross Attributes",30);
	add_xml_identifier(stats_extra,"nexus",attributes.nexus,"Nexus",30);
	add_xml_identifier(stats_extra,"skills",attributes.skills,"Skills",30);
	add_xml_identifier(stats_extra,"pickpoints",attributes.pickpoints,"Pickpoints",30);

	add_xml_statid(base,"phy",&(attributes.phy),"Physique","phy");
	add_xml_statid(base,"coo",&(attributes.coo),"Coordination","coo");
	add_xml_statid(base,"rea",&(attributes.rea),"Reasoning","rea");
	add_xml_statid(base,"will",&(attributes.wil),"Will","wil");
	add_xml_statid(base,"inst",&(attributes.ins),"Instinct","ins");
	add_xml_statid(base,"vit",&(attributes.vit),"Vitality","vit");

	add_xml_statid(cross,"might",&(attributes.might),"Might","mig");
	add_xml_statid(cross,"matter",&(attributes.matter),"Matter","mat");
	add_xml_statid(cross,"tough",&(attributes.tough),"Toughness","tou");
	add_xml_statid(cross,"charm",&(attributes.charm),"Charm","cha");
	add_xml_statid(cross,"react",&(attributes.react),"Reaction","reac");
	add_xml_statid(cross,"perc",&(attributes.perc),"Perception","per");
	add_xml_statid(cross,"rat",&(attributes.ration),"Rationality","rat");
	add_xml_statid(cross,"dext",&(attributes.dext),"Dexterity","dex");
	add_xml_statid(cross,"eth",&(attributes.eth),"Ethereality","eth");

	add_xml_statid(misc,"material",&(attributes.material_points),"Material Points","matp");
	add_xml_statid(misc,"ethereal",&(attributes.ethereal_points),"Ethereal Points","ethp");
	add_xml_statid(misc,"carry",&(attributes.carry_capacity),"Carry capacity","Load");
	add_xml_statid(misc,"food",&(attributes.food),"Food level","food");

	add_xml_statid(nexus,"human",&(attributes.human_nex),"Human","hum");
	add_xml_statid(nexus,"animal",&(attributes.animal_nex),"Animal","ani");
	add_xml_statid(nexus,"vegetal",&(attributes.vegetal_nex),"Vegetal","veg");
	add_xml_statid(nexus,"inorganic",&(attributes.inorganic_nex),"Inorganic","ino");
	add_xml_statid(nexus,"artificial",&(attributes.artificial_nex),"Artificial","art");
	add_xml_statid(nexus,"magic",&(attributes.magic_nex),"Magic","magn");

	add_xml_statid(skills,"attack",&(attributes.attack_skill),"Attack","att");
	add_xml_statid(skills,"defense",&(attributes.defense_skill),"Defense","def");
	add_xml_statid(skills,"harvest",&(attributes.harvesting_skill),"Harvest","har");
	add_xml_statid(skills,"alch",&(attributes.alchemy_skill),"Alchemy","alc");
	add_xml_statid(skills,"magic",&(attributes.magic_skill),"Magic","mag");
	add_xml_statid(skills,"potion",&(attributes.potion_skill),"Potion","pot");
	add_xml_statid(skills,"summon",&(attributes.summoning_skill),"Summoning","sum");
	add_xml_statid(skills,"manu",&(attributes.manufacturing_skill),"Manufacturing","man");
	add_xml_statid(skills,"crafting",&(attributes.crafting_skill),"Crafting","cra");
	add_xml_statid(skills,"overall",&(attributes.overall_skill),"Overall","oa");
}

void parse_console(xmlNode * in);
void parse_errors(xmlNode * in);
void parse_help(xmlNode * in);
void parse_options(xmlNode * in);
void parse_spells(xmlNode * in);
void parse_stats(xmlNode * in);

struct xml_struct
{
	xmlDoc * file;
	xmlNode * root;
};

struct xml_struct load_strings(char * file);
struct xml_struct load_strings_file(char * filename);

void load_translatables()
{
	struct xml_struct file=load_strings("console.xml");
	if(file.file!=NULL)
		{
			//Parse file
			fprintf(stdout,"parsing file console.xml\n");
			parse_console(file.root->children);
			xmlFreeDoc(file.file);
		}
	file = load_strings("errors.xml");
	if(file.file!=NULL)
		{
			//Parse file
			fprintf(stdout,"parsing file errors.xml\n");
			parse_errors(file.root->children);
			xmlFreeDoc(file.file);
		}
	file = load_strings("help.xml");
	if(file.file!=NULL)
		{
			fprintf(stdout,"parsing file help.xml\n");
			parse_help(file.root->children);
			xmlFreeDoc(file.file);
		}
	file = load_strings("options.xml");
	if(file.file!=NULL)
		{
			fprintf(stdout,"parsing file options.xml\n");
			parse_options(file.root->children);
			xmlFreeDoc(file.file);
		}
	file = load_strings("spells.xml");
	if(file.file!=NULL)
		{
			fprintf(stdout,"parsing file spells.xml\n");
			parse_spells(file.root->children);
			xmlFreeDoc(file.file);
		}
	file = load_strings("stats.xml");
	if(file.file!=NULL)
		{
			fprintf(stdout,"parsing file statse.xml\n");
			parse_stats(file.root->children);
			xmlFreeDoc(file.file);
		}
}

struct xml_struct load_strings(char * file)
{
	char file_name[120];
	struct xml_struct tmp={NULL,NULL};
	sprintf(file_name,"strings/%s/%s",lang,file);
	tmp=load_strings_file(file_name);
	if(tmp.file==NULL||tmp.root)
		{
			sprintf(file_name,"strings/en/%s",file);
			tmp=load_strings_file(file_name);
			if(tmp.file==NULL)
				{
					//Notify about this error - english only
					char str[120];
					sprintf(str,"Could not read %s",file);
					log_error(str);
				}
		}
	return tmp;
}

struct xml_struct load_strings_file(char * filename)
{
	struct xml_struct file={NULL,NULL};
	
	if ((file.file = xmlReadFile(filename, NULL, 0)) == NULL)
		{
			xmlFreeDoc(file.file);
			file.file=NULL;
		}
	
	if((file.root=xmlDocGetRootElement(file.file))==NULL)
		{
                        char str[120];
			sprintf(str, "Fatal: couldn't find root element in %s\n",filename);
			log_error(str);
			xmlFreeDoc(file.file);
			file.file=NULL;
		}
	
	return file;
}

void copy_strings(xmlNode * in, distring_item * string)
{
	xmlNode *cur = in;
	for(;cur; cur = cur-> next)
		{
			if(cur->type == XML_ELEMENT_NODE)
				{
					if(!xmlStrcasecmp(cur->name,"name")) strncpy(string->var->str,cur->children->content,30);
					else if (!xmlStrcasecmp(cur->name,"desc")) strncpy(string->var->str,cur->children->content,100);
				}
		}
}

void copy_stats(xmlNode * in, statstring_item * string)
{
	xmlNode *cur = in;
	for(; cur; cur = cur-> next)
		{
			if(cur->type == XML_ELEMENT_NODE)
				{
					if(!xmlStrcasecmp(cur->name,"name")) 
						strncpy(string->var->name,cur->children->content,30);
					else if (!xmlStrcasecmp(cur->name,"shortname"))	
						strncpy(string->var->shortname,cur->children->content,5);
				}
		}
}

void parse_statstrings(xmlNode * in, group_stat * group)
{
	xmlNode * cur = in;
	int i;
	for(;cur;cur=cur->next)
		{
			if(cur->type == XML_ELEMENT_NODE)
				{
					for(i=0;i<group->no;i++)
						if(!xmlStrcmp(cur->name,group->statstrings[i]->xml_id))
							{
								copy_stats(cur->children,group->statstrings[i]);
								break;
							}
				}
		}
}

void parse_distrings(xmlNode * in, group_id_di * group)
{
	xmlNode * cur = in;
	int i;
	for(;cur;cur=cur->next)
		{
			if(cur->type==XML_ELEMENT_NODE)
				{
					for(i=0;i<group->no;i++)
						if(!xmlStrcmp(cur->name,group->distrings[i]->xml_id))
							{
								copy_strings(cur->children,group->distrings[i]);
								break;
							}
				}
		}
}

void parse_strings(xmlNode * in, group_id * group)
{
	xmlNode * cur = in;
	int i;
	for(;cur;cur=cur->next)
		{
			if(cur->type==XML_ELEMENT_NODE)
				{
					for(i=0;i<group->no;i++)
						if(!xmlStrcmp(cur->name,group->strings[i]->xml_id))
							{
								strncpy(group->strings[i]->var,cur->children->content,group->strings[i]->max_len);
								break;
							}
				}
		}
}

void parse_groups(xmlNode * in, void * gPtr, int size, int type)
{
	group_id * group=gPtr;
	group_id_di * Group=gPtr;
	group_stat * stat=gPtr;
	int i=in->type;
	xmlNode * cur = NULL;
	for(cur=in;cur;cur=cur->next)
		{
			if(cur->type==XML_ELEMENT_NODE)
				{
					for(i=0;i<size;i++)
						{
							switch(type)
								{
									case GROUP:
										if(!xmlStrcmp(cur->name,group[i].xml_id))
											{
												parse_strings(cur->children,&(group[i]));
												break;
											}
										break;
									case DIGROUP:
										if(!xmlStrcasecmp(cur->name,Group[i].xml_id))
											{
												parse_distrings(cur->children,&(Group[i]));
												break;
											}
										break;
									case STAT_GROUP:
										if(!xmlStrcmp(cur->name,stat[i].xml_id))
											{
												parse_statstrings(cur->children,&(stat[i]));
												break;
											}
										break;
									default: break;
								}
						}
				}
		}
}

void parse_console(xmlNode * in)
{
	parse_groups(in,console_str,CONSOLE_STR, GROUP);
}

void parse_errors(xmlNode * in)
{
	parse_groups(in,errors,ERRORS, GROUP);
}

void parse_help(xmlNode * in)
{
	parse_groups(in, help_str, HELP_STR, GROUP);
}

void parse_options(xmlNode * in)
{
	parse_groups(in, options_str, OPTIONS_STR, DIGROUP);
}

void parse_spells(xmlNode * in)
{
	parse_groups(in, sigils_str, SIGILS_STR, DIGROUP);
}

void parse_stats(xmlNode * in)
{
	parse_groups(in, stats_str, STATS_STR, STAT_GROUP);
}
