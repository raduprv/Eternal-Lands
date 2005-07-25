#include <stdio.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <string.h>
#include <stdarg.h>
#include "translate.h"
#ifdef MAP_EDITOR
#include "global.h"
#endif

#define GROUP 0
#define DIGROUP 1
#ifdef ELC
#define STAT_GROUP 2
#endif


/*! \name Tooltips*/
/*! \{ */
#ifdef ELC
char	tt_walk[30],
	tt_sit[30],
	tt_stand[30],
	tt_look[30],
	tt_use[30],
	tt_use_witem[30],
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
	tt_options[30],
	tt_help[30],
	tt_costumize[60],
	tt_name[60];
#endif
/*! \} */

#ifdef ELC
/*! \name Options*/
/*! \{ */
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
#endif
/*! \} */

#ifdef ELC
/*! \name Sigils/spells */
/*! \{ */
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
#endif
/*! \} */

#ifdef ELC
/*! \name Help messages*/
/*! \{ */
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
	/*items.c*/
	get_all_str[8],
	quantity_edit_str[100],
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
	skin_str[15],
	hair_str[15],
	shirt_str[15],
	pants_str[15],
	boots_str[15],
	head_str[15],
	gender_str[15],
	male_str[15],
	female_str[15],
	race_str[15],
	human_str[15],
	elf_str[15],
	dwarf_str[15],
	gnome_str[15],
	orchan_str[15],
	draegoni_str[15],
	confirm_password[30],
	error_username_length[50], 
	error_password_length[50], 
	error_pass_no_match[30],
	error_confirm_create_char[100],
	error_max_digits[100],
	error_length[100],
	passwords_match[30],
	remember_change_appearance[200],
	p2p_race[100],
	char_help[200],
	invalid_pass[30],
	show_password[30],
	hide_password[30],
	char_done[15],
	char_back[15],
	about_human[30],
	about_elves[30], 
	about_dwarfs[30],
	about_gnomes[30],
	about_orchans[30],
	about_draegoni[30],
	zoom_in_out[200],
	rotate_camera[200],
	/*pm_log.c*/
	going_afk[30],
	not_afk[50],
	new_messages[100],
	afk_names[15],
	afk_messages[25],
	afk_print_help[150],
	/*trade.c*/
	quantity_str[30],
	abort_str[10],
	you_str[10],
	accept_str[12],
	/* new_character.c */
	use_appropriate_name[500];
#endif
/*! \} */

#ifdef ELC
/*! \name Console*/
/*! \{ */
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
#endif

/*! \name Errors */
/*! \{ */
char	reg_error_str[15],
	/*2d_objects.c*/
	cant_load_2d_object[30],
	cant_open_file[30],
	/*3d_objects.c*/
	object_error_str[30], 
	nasty_error_str[50],
	corrupted_object[100],
	bad_object[30],
	multiple_material_same_texture[100], 	
	invalid_map[40],
#ifdef ELC
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
#endif
	/*font.c*/
	cant_load_font[30],
	/*init.c*/
#ifdef ELC
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
	disabled_shadow_mapping[50],
	fatal_error_str[10],
	no_e3d_list[50],
	enabled_vertex_arrays[50],
	disabled_point_particles[50],
	disabled_particles_str[50],
	invalid_video_mode[75],
	failed_sdl_net_init[30],
	failed_sdl_timer_init[30], 
	cant_read_elini[50],
	must_use_tabs[80],
	/*multiplayer.c*/
	failed_resolve[150],
	failed_connect[100], 
	redefine_your_colours[250],
	char_dont_exist[30],
	char_name_in_use[50],
	server_latency[30],
	update_your_client[100],
	client_ver_not_supported[100],
	packet_overrun[50],
	disconnected_from_server[100],
	cant_change_map[100],
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
#endif
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
	part_part_str[20]
#ifdef ELC
	/*paste.c*/
	,not_ascii[20],
	/*rules.c*/
	you_can_proceed[50],
	accepted_rules[50],
	read_rules_str[50],
	parse_rules_str[50],
	rules_not_found[100],
	/*sound.c*/
	snd_wav_load_error[50],
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
	stat_no_invalid[50],
	/*timers.c*/
	timer_lagging_behind[100],
	/* notepad.c */
	cant_parse_notes[100],
	notes_wrong[100],
	too_many_notes[100],
	wrong_note_node[100],
	cant_save_notes[100],
	exceed_note_buffer[100];
#else
	;
#endif  // ELC
/*! \} */

#ifdef ELC
/*! \name Window/Tab titles */
/*! \{ */
char	win_notepad[20],
	win_prompt[10],
	tab_help[10],
	tab_encyclopedia[20],
	tab_skills[20],
	tab_rules[20],
	tab_statistics[20],
	tab_knowledge[20],
	tab_questlog[20],
	tab_all[20],
	tab_local[20],
	tab_newbie_channel[20],
	tab_help_channel[20],
	tab_market_channel[20],
	tab_general_channel[20],
	tab_offtopic_channel[20],
	tab_channel[20],
	tab_guild[10],
	tab_mod[10],
	tab_personal[10],
	tab_server[10],
	tab_main[20],
	button_okay[10],
	button_cancel[10],
	button_new_category[30],
	button_remove_category[30],
	button_save_notes[30],
	label_note_name[20],
	label_cursor_coords[17];
#endif  // ELC
/*! \} */

#ifdef ELC
#define CONSOLE_STR 3
#define ERRORS 7
#define HELP_STR 4
#define OPTIONS_STR 1
#define SIGILS_STR 1
#define STATS_STR 5
#define STATS_EXTRA 1
#define TITLES_STR 1
#endif

#ifdef MAP_EDITOR
#define ERRORS 1
#endif

group_id * errors;
#ifdef ELC
group_id * console_str;
group_id * help_str;
group_id_di * options_str;
group_id_di * sigils_str;
group_stat * stats_str;
group_id * stats_extra;
group_id * titles_str;
#endif

void init_console(void);
void init_help(void);
void init_options(void);
void init_spells(void);
void init_stats(void);
void init_titles(void);
void init_errors(void);
void * add_xml_group(int type, int no, ...);
void free_xml_parser(int type, void * gPtr, int no);
void parse_errors(xmlNode * in);
#ifdef ELC
void parse_console(xmlNode * in);
void parse_help(xmlNode * in);
void parse_options(xmlNode * in);
void parse_spells(xmlNode * in);
void parse_stats(xmlNode * in);
void parse_titles(xmlNode * in);
#endif
struct xml_struct load_strings(char * file);
struct xml_struct load_strings_file(char * filename);

void init_groups()
{
#ifdef ELC
	console_str=add_xml_group(GROUP,CONSOLE_STR,"filter","ignore","misc");
	errors=add_xml_group(GROUP,ERRORS,"actors","load","misc","particles","snd","video","rules");
	help_str=add_xml_group(GROUP,HELP_STR,"afk","misc","new","tooltips");
	options_str=add_xml_group(DIGROUP,OPTIONS_STR,"options");
	sigils_str=add_xml_group(DIGROUP,SIGILS_STR,"sigils");
	stats_str=add_xml_group(STAT_GROUP,STATS_STR,"base","cross","misc","nexus","skills");
	stats_extra=add_xml_group(GROUP,STATS_EXTRA,"extra");
	titles_str = add_xml_group (GROUP, TITLES_STR, "titles");
#endif
#ifdef MAP_EDITOR
	errors=add_xml_group(GROUP,ERRORS,"particles");
#endif
}

void * add_xml_group(int type, int no, ...)
{
	va_list ap;
	int i=0;
	va_start(ap, no);
	switch(type){
		case GROUP: {
			group_id * grp;
			grp=(group_id*)calloc(no,sizeof(group_id));
			for(;i<no;i++) strncpy(grp[i].xml_id,va_arg(ap, char*),15);
			return grp;
		}
		case DIGROUP: {
			group_id_di * grp;
			grp=(group_id_di*)calloc(no,sizeof(group_id_di));
			for(;i<no;i++) strncpy(grp[i].xml_id,va_arg(ap, char*),15);
			return grp;
		}
#ifdef ELC
		case STAT_GROUP: {
			group_stat * grp;
			grp=(group_stat*)calloc(no,sizeof(group_stat));
			for(;i<no;i++) strncpy(grp[i].xml_id,va_arg(ap, char*),15);
			return grp;
		}
#endif
		default: 
			return NULL;
	}
}

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

#ifdef ELC
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
#endif

void add_xml_identifier(group_id * group, char * xml_id, char * var, char * def, int max_len)
{
	group->strings=(string_item**)realloc(group->strings,(group->no+1)*sizeof(string_item*));
	group->strings[group->no]=(string_item*)calloc(1,sizeof(string_item));
	strcpy(group->strings[group->no]->xml_id,xml_id);
	group->strings[group->no]->var=var;
	strncpy(var, def, max_len-1);
	group->strings[group->no]->max_len=max_len-1;
	group->no++;
}

void init_translatables()
{
	init_groups();
	init_errors();
#ifdef ELC
	init_console();
	init_help();
	init_options();
	init_spells();
	init_stats();
	init_titles();
#endif
}

#ifdef ELC
void init_console()
{
	group_id * filter=&(console_str[0]);
	group_id * ignore=&(console_str[1]);
	group_id * misc=&(console_str[2]);
	
	add_xml_identifier(ignore,"toolong",name_too_long,"Name too long, the max limit is 15 characters.",sizeof(name_too_long));
	add_xml_identifier(ignore,"tooshort",name_too_short,"Name too short, only names>=3 characters can be used!",sizeof(name_too_short));
	add_xml_identifier(ignore,"noadd",not_added_to_ignores,"Name not added to the ignore list!",sizeof(not_added_to_ignores));
	add_xml_identifier(ignore,"already",already_ignoring,"You are already ignoring %s!",sizeof(already_ignoring));
	add_xml_identifier(ignore,"full",ignore_list_full,"Wow, your ignore list is full, this is impossible!",sizeof(ignore_list_full));
	add_xml_identifier(ignore,"add",added_to_ignores,"%s was added to your ignore list!",sizeof(added_to_ignores));
	add_xml_identifier(ignore,"norem",not_removed_from_ignores,"Name not removed from the ignore list!",sizeof(not_removed_from_ignores));
	add_xml_identifier(ignore,"not",not_ignoring,"You are NOT ignoring %s in the first place!",sizeof(not_ignoring));
	add_xml_identifier(ignore,"rem",removed_from_ignores,"OK, %s was removed from your ignore list!",sizeof(removed_from_ignores));
	add_xml_identifier(ignore,"none",no_ignores_str,"You are ignoring no one!",sizeof(no_ignores_str));
	add_xml_identifier(ignore,"cur",ignores_str,"You are currently ignoring",sizeof(ignores_str));

	add_xml_identifier(filter,"toolong",word_too_long,"Word too long, the max limit is 15 characters.",sizeof(word_too_long));
	add_xml_identifier(filter,"tooshort",word_too_short,"Word too short, only words>=3 characters can be used!",sizeof(word_too_short));
	add_xml_identifier(filter,"notadd",not_added_to_filter,"Word not added to the filter list!",sizeof(not_added_to_filter));
	add_xml_identifier(filter,"already",already_filtering,"You are already filtering %s",sizeof(already_filtering));
	add_xml_identifier(filter,"add",added_to_filters,"OK, %s was added to your filter list!",sizeof(added_to_filters));
	add_xml_identifier(filter,"norem",not_removed_from_filter,"Word not removed from the filter list!",sizeof(not_removed_from_filter));
	add_xml_identifier(filter,"not",not_filtering,"You are NOT filtering %s in the first place!",sizeof(not_filtering));
	add_xml_identifier(filter,"rem",removed_from_filter,"OK, %s was removed from your filter list!",sizeof(removed_from_filter));
	add_xml_identifier(filter,"none",no_filters_str,"You are filtering nothing!",sizeof(no_filters_str));
	add_xml_identifier(filter,"cur",filters_str,"You are currently filtering",sizeof(filters_str));
	
	add_xml_identifier(misc,"log",logconn_str,"Logging raw connection data",sizeof(logconn_str));
	add_xml_identifier(misc,"card",video_card_str,"Video card",sizeof(video_card_str));
	add_xml_identifier(misc,"vendor",video_vendor_str,"Vendor ID",sizeof(video_vendor_str));
	add_xml_identifier(misc,"ext",supported_extensions_str,"Supported extensions",sizeof(supported_extensions_str));
	add_xml_identifier(misc,"opengl",opengl_version_str,"OpenGL Version",sizeof(opengl_version_str));
}
#endif

void init_errors()
{
#ifdef ELC
	group_id * actors=&(errors[0]);
	group_id * load=&(errors[1]);
	group_id * misc=&(errors[2]);
	group_id * particles=&(errors[3]);
	group_id * snd=&(errors[4]);
	group_id * video=&(errors[5]);
	group_id * rules=&(errors[6]);
#endif
#ifdef MAP_EDITOR
	group_id * particles=&(errors[0]);
#endif

#ifdef ELC
	//Actor related errors
	add_xml_identifier(actors,"load",cant_load_actor,"Can't load actor",sizeof(cant_load_actor));
	add_xml_identifier(actors,"frame",cant_find_frame,"Couldn't find frame",sizeof(cant_find_frame));
	add_xml_identifier(actors,"unk_frame",unknown_frame,"Unknown frame",sizeof(unknown_frame));
	add_xml_identifier(actors,"dup_id",duplicate_actors_str,"Duplicate actor ID",sizeof(duplicate_actors_str));
	add_xml_identifier(actors,"namelen",bad_actor_name_length,"Bad actor name/length",sizeof(bad_actor_name_length));
	add_xml_identifier(actors,"addcommand",cant_add_command,"Unable to add command",sizeof(cant_add_command));
	add_xml_identifier(actors,"loadbody",error_body_part,"Can't load body part",sizeof(error_body_part));
	add_xml_identifier(actors,"head",error_head,"head",sizeof(error_head));
	add_xml_identifier(actors,"torso",error_torso,"torso",sizeof(error_torso));
	add_xml_identifier(actors,"weapon",error_weapon,"weapon",sizeof(error_weapon));
	add_xml_identifier(actors,"helmet",error_helmet,"helmet",sizeof(error_helmet));
	add_xml_identifier(actors,"cape",error_cape,"cape",sizeof(error_cape));
	add_xml_identifier(actors,"dupnpc",duplicate_npc_actor,"Duplicate actor name",sizeof(duplicate_npc_actor));
	
	//Loading errors
	add_xml_identifier(load,"obj",cant_load_2d_object,"Can't load 2d object",sizeof(cant_load_2d_object));
	add_xml_identifier(load,"file",cant_open_file,"Can't open file",sizeof(cant_open_file));
	add_xml_identifier(load,"cursors",cursors_file_str,"Can't open cursors file.",sizeof(cursors_file_str));
	add_xml_identifier(load,"font",cant_load_font,"Unable to load font",sizeof(cant_load_font));
	add_xml_identifier(load,"fatal",fatal_error_str,"Fatal",sizeof(fatal_error_str));
	add_xml_identifier(load,"noe3d",no_e3d_list,"Couldn't read e3dlist.txt",sizeof(no_e3d_list));
	add_xml_identifier(load,"elini",cant_read_elini,"Couldn't read configuration file el.ini",sizeof(cant_read_elini));
	add_xml_identifier(load,"invmap",invalid_map,"%s is an invalid map!",sizeof(invalid_map));
	add_xml_identifier(load,"parsenotes",cant_parse_notes,"Unable to parse xml notepad. It will be overwritten.",sizeof(cant_parse_notes));
	add_xml_identifier(load,"parsenotes",cant_parse_notes,"Unable to parse xml notepad. It will be overwritten.",sizeof(cant_parse_notes));
	add_xml_identifier(load,"noteswrong",notes_wrong,"Document of the wrong type. It will be overwritten.",sizeof(notes_wrong));
	add_xml_identifier(load,"manynotes",too_many_notes,"Too many notes - Last nodes were ignored.",sizeof(too_many_notes));
	add_xml_identifier(load,"notenode",wrong_note_node,"Incorrect node type - could not copy.",sizeof(wrong_note_node));
	add_xml_identifier(load,"savenotes",cant_save_notes,"Unable to write notes to file %s",sizeof(cant_save_notes));
	add_xml_identifier(load,"exceednotes",exceed_note_buffer,"Tried to exceed notepad buffer! Ignored.",sizeof(exceed_note_buffer));

	//Miscellaneous errors
	add_xml_identifier(misc,"error",reg_error_str,"Error",sizeof(reg_error_str));
	add_xml_identifier(misc,"objerr",object_error_str,"Object error",sizeof(object_error_str));
	add_xml_identifier(misc,"nasty",nasty_error_str,"Something nasty happened while trying to process: %s",sizeof(nasty_error_str));
	add_xml_identifier(misc,"corrupt",corrupted_object,"Object seems to be corrupted. Skipping the object. Warning: This might cause further problems.",sizeof(corrupted_object));
	add_xml_identifier(misc,"badobj",bad_object,"Bad object",sizeof(bad_object));
	add_xml_identifier(misc,"multimat",multiple_material_same_texture,"Two or more materials with the same texture name!",sizeof(multiple_material_same_texture));
	add_xml_identifier(misc,"resync",resync_server,"Resync with the server...",sizeof(resync_server));
	add_xml_identifier(misc,"vertex",enabled_vertex_arrays,"Vertex Arrays enabled (memory hog on!)...",sizeof(enabled_vertex_arrays));
	add_xml_identifier(misc,"point",disabled_point_particles,"Point Particles disabled.",sizeof(disabled_point_particles));
	add_xml_identifier(misc,"particles",disabled_particles_str,"Particles completely disabled!",sizeof(disabled_particles_str));
	add_xml_identifier(misc,"net",failed_sdl_net_init,"Couldn't initialize net",sizeof(failed_sdl_net_init));
	add_xml_identifier(misc,"timer",failed_sdl_timer_init,"Couldn't initialize the timer",sizeof(failed_sdl_timer_init));
	add_xml_identifier(misc,"resolve",failed_resolve,"Can't resolve server address.\nPerhaps you are not connected to the Internet or your DNS server is down!",sizeof(failed_resolve));
	add_xml_identifier(misc,"connect",failed_connect,"Can't connect to server :(",sizeof(failed_connect));
	add_xml_identifier(misc,"redefine",redefine_your_colours,"You need to update your character, due to the new models!\nGo on the New Character screen, type your existing\nusername and password, update your character, then press\nDone. *YOUR STATS AND ITEMS WILL NOT BE AFFECTED*",sizeof(redefine_your_colours));
	add_xml_identifier(misc,"noexist",char_dont_exist,"You don't exist!",sizeof(char_dont_exist));
	add_xml_identifier(misc,"latency",server_latency,"Server latency",sizeof(server_latency));
	add_xml_identifier(misc,"newver",update_your_client,"There is a new version of the client, please update it",sizeof(update_your_client));
	add_xml_identifier(misc,"notsup",client_ver_not_supported,"This version is no longer supported, please update!",sizeof(client_ver_not_supported));
	add_xml_identifier(misc,"packets",packet_overrun,"Packet overrun...data lost!",sizeof(packet_overrun));
	add_xml_identifier(misc,"disconnect",disconnected_from_server,"Disconnected from server!",sizeof(disconnected_from_server));
	add_xml_identifier(misc,"stat",stat_no_invalid,"Server sent invalid stat number",sizeof(stat_no_invalid));
	add_xml_identifier(misc,"ascii",not_ascii,"Not ASCII",sizeof(not_ascii));
	add_xml_identifier(misc,"timer",timer_lagging_behind,"The %s timer was lagging severely behind or had stopped, restarted it", sizeof(timer_lagging_behind));
	add_xml_identifier(misc,"nameinuse",char_name_in_use,"Character name is already taken",sizeof(char_name_in_use));
	add_xml_identifier(misc,"notabs",must_use_tabs,"You cannot disable tabbed windows with video mode %d, forcing them",sizeof(must_use_tabs));
	add_xml_identifier (misc, "nomap", cant_change_map, "Unable to switch to map %s!", sizeof(cant_change_map));
#endif

	//Particle errors
	add_xml_identifier(particles,"version",particles_filever_wrong,"Particle file %s version (%i) doesn't match file reader version (%i)!",sizeof(particles_filever_wrong));
	add_xml_identifier(particles,"overrun",particle_system_overrun,"Particle file %s tries to define %i particles, when %i is the maximum!",sizeof(particle_system_overrun));
	add_xml_identifier(particles,"pos",particle_strange_pos,"Particle file %s contained strange position/constraint values. Tried to fix.",sizeof(particle_strange_pos));
	add_xml_identifier(particles,"sysdump",particle_system_dump,"-- PARTICLE SYSTEM DUMP --",sizeof(particle_system_dump));
	add_xml_identifier(particles,"disabled",particles_disabled_str,"Particles disabled!",sizeof(particles_disabled_str));
	add_xml_identifier(particles,"point",point_sprites_enabled,"Using point sprites",sizeof(point_sprites_enabled));
	add_xml_identifier(particles,"quads",using_textured_quads,"Using textured quads",sizeof(using_textured_quads));
	add_xml_identifier(particles,"defs",definitions_str,"Definitions",sizeof(definitions_str));
	add_xml_identifier(particles,"system",part_sys_str,"systems",sizeof(part_sys_str));
	add_xml_identifier(particles,"particles",part_part_str,"particles",sizeof(part_part_str));

#ifdef ELC
	//Sound errors
	add_xml_identifier(snd,"loadwav",snd_wav_load_error,"Failed to load wav file %s",sizeof(snd_wav_load_error));
	add_xml_identifier(snd,"loadfile",snd_ogg_load_error,"Failed to load ogg file",sizeof(snd_ogg_load_error));
	add_xml_identifier(snd,"loadstream",snd_ogg_stream_error,"Failed to load ogg stream",sizeof(snd_ogg_stream_error));
	add_xml_identifier(snd,"buffer",snd_buff_error,"Error creating buffer",sizeof(snd_buff_error));
	add_xml_identifier(snd,"number",snd_invalid_number,"Got invalid sound number",sizeof(snd_invalid_number));
	add_xml_identifier(snd,"source",snd_source_error,"Error creating a source",sizeof(snd_source_error));
	add_xml_identifier(snd,"skip",snd_skip_speedup,"Skip! Speeding up...",sizeof(snd_skip_speedup));
	add_xml_identifier(snd,"tooslow",snd_too_slow,"Sorry, too slow to play music...",sizeof(snd_too_slow));
	add_xml_identifier(snd,"fail",snd_stop_fail,"Failed to stop all sounds.",sizeof(snd_stop_fail));
	add_xml_identifier(snd,"init",snd_init_error,"Error initializing sound",sizeof(snd_init_error));
	add_xml_identifier(snd,"toomany",snd_sound_overflow,"Too many sounds.",sizeof(snd_sound_overflow));
	add_xml_identifier(snd,"read",snd_media_read,"Read from media.",sizeof(snd_media_read));
	add_xml_identifier(snd,"notvorbis",snd_media_notvorbis,"Not Vorbis data.",sizeof(snd_media_notvorbis));
	add_xml_identifier(snd,"version",snd_media_ver_mismatch,"Vorbis version mismatch.",sizeof(snd_media_ver_mismatch));
	add_xml_identifier(snd,"header",snd_media_invalid_header,"Invalid Vorbis header.",sizeof(snd_media_invalid_header));
	add_xml_identifier(snd,"intern",snd_media_internal_error,"Internal logic fault (bug or heap/stack corruption.",sizeof(snd_media_internal_error));
	add_xml_identifier(snd,"unknown",snd_media_ogg_error,"Unknown Ogg error.",sizeof(snd_media_ogg_error));
	
	//Video errors
	add_xml_identifier(video,"nostencil",no_stencil_str,"Video mode %s with a stencil buffer is not available\nTrying this mode without a stencil buffer...",sizeof(no_stencil_str));
	add_xml_identifier(video,"safemode",safemode_str,"Video mode %s without a stencil buffer is not available\nTrying the safemode (640x480x32) Full Screen (no stencil)",sizeof(safemode_str));
	add_xml_identifier(video,"nosdl",no_sdl_str,"Couldn't initialize SDL",sizeof(no_sdl_str));
	add_xml_identifier(video,"nohwstencil",no_hardware_stencil_str,"Couldn't find a hardware accelerated stencil buffer.\nShadows are not available.",sizeof(no_hardware_stencil_str));
	add_xml_identifier(video,"depth",suggest_24_or_32_bit,"Hint: Try a 32 BPP resolution (if you are under XWindows, set your screen display to 24 or 32 bpp).",sizeof(suggest_24_or_32_bit));
	add_xml_identifier(video,"glmode",fail_opengl_mode,"Couldn't set GL mode",sizeof(fail_opengl_mode));
	add_xml_identifier(video,"swstencil",stencil_falls_back_on_software_accel,"Hmm... This mode seems to fall back in software 'acceleration'.\nTrying to disable the stencil buffer.",sizeof(stencil_falls_back_on_software_accel));
	add_xml_identifier(video,"last_try",last_chance_str,"Hmm... No luck without a stencil buffer either...\nLet's try one more thing...",sizeof(last_chance_str));
	add_xml_identifier(video,"swmode",software_mode_str,"Damn, it seems that you are out of luck, we are in the software mode now, so the game will be veeeeery slow. If you DO have a 3D accelerated card, try to update your OpenGl drivers...",sizeof(software_mode_str));
	add_xml_identifier(video,"extfound",gl_ext_found,"%s extension found, using it.",sizeof(gl_ext_found));
	add_xml_identifier(video,"extnouse",gl_ext_found_not_used,"%s extension found, NOT using it...",sizeof(gl_ext_found_not_used));
	add_xml_identifier(video,"extnotfound",gl_ext_not_found,"Couldn't find the %s extension, not using it...",sizeof(gl_ext_not_found));
	add_xml_identifier(video,"multitex",gl_ext_no_multitexture,"Couldn't find the GL_ARB_multitexture extension, giving up clouds shadows, and texture detail...",sizeof(gl_ext_no_multitexture));
	add_xml_identifier(video,"noshadowmapping",disabled_shadow_mapping,"Shadowmapping disabled (need newer hardware)",sizeof(disabled_shadow_mapping));
	add_xml_identifier(video,"invalid",invalid_video_mode,"Stop playing with the configuration file and select valid modes!",sizeof(invalid_video_mode));

	//Rule errors
	add_xml_identifier(rules,"proceed",you_can_proceed,"You can proceed in %d seconds",sizeof(you_can_proceed));
	add_xml_identifier(rules,"accept",accepted_rules,"Click on \"I Accept\" to play the game!",sizeof(accepted_rules));
	add_xml_identifier(rules,"read",read_rules_str,"An error occured while reading the rules",sizeof(read_rules_str));
	add_xml_identifier(rules,"parse",parse_rules_str,"An error occored while parsing the rules",sizeof(parse_rules_str));
	add_xml_identifier(rules,"notfound",rules_not_found,"The rules.xml file was not found. You will have to redownload your game.",sizeof(rules_not_found));
#endif
}

#ifdef ELC
void init_help()
{
	group_id * afk = &(help_str[0]);
	group_id * misc = &(help_str[1]);
	group_id * new = &(help_str[2]);
	group_id * tooltips = &(help_str[3]);

	//AFK Messages
	add_xml_identifier(afk,"going",going_afk,"Going AFK",sizeof(going_afk));
	add_xml_identifier(afk,"not",not_afk,"Not AFK any more",sizeof(not_afk));
	add_xml_identifier(afk,"back",new_messages,"You have %d new messages from the following people: ",sizeof(new_messages));
	add_xml_identifier(afk,"names",afk_names,"Names",sizeof(afk_names));
	add_xml_identifier(afk,"messages",afk_messages,"Messages",sizeof(afk_messages));
	add_xml_identifier(afk,"help",afk_print_help,"To print the messages from the different people type #msg <number> or #msg all to view them all",sizeof(afk_print_help));
	//Miscellaneous
	add_xml_identifier(misc,"values",values_str,"values",sizeof(values_str));
	add_xml_identifier(misc,"close",close_str,"[close]",sizeof(close_str));
	add_xml_identifier(misc,"low",low_framerate_str,"Low framerate detected, shadows disabled!",sizeof(low_framerate_str));
	add_xml_identifier(misc,"size",window_size_adjusted_str,"Window size adjusted to %s",sizeof(window_size_adjusted_str));
	add_xml_identifier(misc,"trade",no_open_on_trade,"You can't open this window while on trade.",sizeof(no_open_on_trade));
	add_xml_identifier(misc,"user",login_username_str,"Username:",sizeof(login_username_str));
	add_xml_identifier(misc,"pass",login_password_str,"Password:",sizeof(login_password_str));
	add_xml_identifier(misc,"getall",get_all_str,"Get All",sizeof(get_all_str));
	add_xml_identifier(misc,"completed",completed_research,"COMPLETED",sizeof(completed_research));
	add_xml_identifier(misc,"research",researching_str,"Researching",sizeof(researching_str));
	add_xml_identifier(misc,"nothing",not_researching_anything,"Nothing",sizeof(not_researching_anything));
	add_xml_identifier(misc,"mix",mix_str,"Mix",sizeof(mix_str));
	add_xml_identifier(misc,"clear",clear_str,"Clear",sizeof(clear_str));
	add_xml_identifier(misc,"connect",connect_to_server_str,"Connecting to Server...",sizeof(connect_to_server_str));
	add_xml_identifier(misc,"reconnect",reconnect_str,"Press any key to try again.",sizeof(reconnect_str));
	add_xml_identifier(misc,"license",license_check,"Entropy says: U R 2 g00d 2 r34d +h3 license.txt?\nBTW, that license.txt file is actually there for a reason.",sizeof(license_check));
	add_xml_identifier(misc,"quantity",quantity_str,"Quantity",sizeof(quantity_str));
	add_xml_identifier(misc,"abort",abort_str,"Abort",sizeof(abort_str));
	add_xml_identifier(misc,"sigils",sig_too_few_sigs,"This spell requires at least 2 sigils",sizeof(sig_too_few_sigs));
	add_xml_identifier(misc,"switch",switch_video_mode,"Switches to video mode %s",sizeof(switch_video_mode));
	add_xml_identifier(misc,"cache",cache_size_str,"Cache size",sizeof(cache_size_str));
	add_xml_identifier (misc, "appropriate_name", use_appropriate_name, "Use an appropriate name:\nPlease do not create a name that is obscene or offensive, contains more than 3 numbers, is senseless or stupid (i.e. djrtq47fa), or is made with the intent of impersonating another player.\nTake into consideration that the name you choose does affect the atmosphere of the game. Inappropriate names can and will be locked.", sizeof (use_appropriate_name) );
	add_xml_identifier(misc,"edit_quantity",quantity_edit_str,"Rightclick on the quantity you wish to edit",sizeof(quantity_edit_str));
	add_xml_identifier(misc,"you",you_str,"You",sizeof(you_str));
	add_xml_identifier(misc,"accept",accept_str,"Accept",sizeof(accept_str));

	//New characters
	add_xml_identifier(new,"skin",skin_str,"Skin",sizeof(skin_str));
	add_xml_identifier(new,"hair",hair_str,"Hair",sizeof(hair_str));
	add_xml_identifier(new,"shirt",shirt_str,"Shirt",sizeof(shirt_str));
	add_xml_identifier(new,"pants",pants_str,"Pants",sizeof(pants_str));
	add_xml_identifier(new,"boots",boots_str,"Boots",sizeof(boots_str));
	add_xml_identifier(new,"head",head_str,"Head",sizeof(head_str));
	add_xml_identifier(new,"gender",gender_str,"Gender",sizeof(gender_str));
	add_xml_identifier(new,"male",male_str,"Male",sizeof(male_str));
	add_xml_identifier(new,"female",female_str,"Female",sizeof(female_str));
	add_xml_identifier(new,"race",race_str,"Race",sizeof(race_str));
	add_xml_identifier(new,"human",human_str,"Human",sizeof(human_str));
	add_xml_identifier(new,"elf",elf_str,"Elf",sizeof(elf_str));
	add_xml_identifier(new,"dwarf",dwarf_str,"Dwarf",sizeof(dwarf_str));
	add_xml_identifier(new,"gnome",gnome_str,"Gnome",sizeof(gnome_str));
	add_xml_identifier(new,"orchan",orchan_str,"Orchan",sizeof(orchan_str));
	add_xml_identifier(new,"draegoni",draegoni_str,"Draegoni",sizeof(draegoni_str));
	add_xml_identifier(new,"confirm",confirm_password,"Confirm:",sizeof(confirm_password));
	add_xml_identifier(new,"userlen",error_username_length,"Username MUST be at least 3 characters long!",sizeof(error_username_length));
	add_xml_identifier(new,"passlen",error_password_length,"The password MUST be at least 4 characters long!",sizeof(error_password_length));
	add_xml_identifier(new,"passnomatch",error_pass_no_match,"Passwords don't match!",sizeof(error_pass_no_match));
	add_xml_identifier(new,"passmatch",passwords_match,"Passwords are matching!",sizeof(passwords_match));
	add_xml_identifier(new,"appearance",remember_change_appearance,"Remember to change your characters appearance before pressing \"Done\"",sizeof(remember_change_appearance));
	add_xml_identifier(new,"max_digits",error_max_digits,"You can only have 3 digits in your name!",sizeof(error_max_digits));
	add_xml_identifier(new,"max_length",error_length,"Names and passwords can max be 15 characters long",sizeof(error_length));
	add_xml_identifier(new,"p2p_race",p2p_race,"You have to pay to create a char with this race",sizeof(p2p_race));
	add_xml_identifier(new,"char_help",char_help,"To costumize your character and select name/password, press the buttons at the bottom.",sizeof(char_help));
	add_xml_identifier(new,"confirmcreate",error_confirm_create_char,"Click done again to create a character with that name and appearance.",sizeof(error_confirm_create_char));
	add_xml_identifier(new,"wrongpass",invalid_pass,"Invalid password!",sizeof(invalid_pass));
	add_xml_identifier(new,"showpass",show_password,"Show password",sizeof(show_password));
	add_xml_identifier(new,"hidepass",hide_password,"Hide password",sizeof(hide_password));
	add_xml_identifier(new,"done",char_done,"Done",sizeof(char_done));
	add_xml_identifier(new,"back",char_back,"Back",sizeof(char_back));
	add_xml_identifier(new,"a_human",about_human,"About Human",sizeof(about_human));
	add_xml_identifier(new,"a_elf",about_elves,"About Elves",sizeof(about_elves));
	add_xml_identifier(new,"a_dwarf",about_dwarfs,"About Dwarfs",sizeof(about_dwarfs));
	add_xml_identifier(new,"a_gnome",about_gnomes,"About Gnomes",sizeof(about_gnomes));
	add_xml_identifier(new,"a_orchan",about_orchans,"About Orchans",sizeof(about_orchans));
	add_xml_identifier(new,"a_draegoni",about_draegoni,"About Draegoni",sizeof(about_draegoni));
	add_xml_identifier(new,"zoom_in_out",zoom_in_out,"To zoom in/out: Middle mouse wheel or Page Up/Down",sizeof(zoom_in_out));
	add_xml_identifier(new,"rotate_camera",rotate_camera,"To rotate the camera: Middle mouse button or arrow keys",sizeof(rotate_camera));
	
	//Icons
	add_xml_identifier(tooltips,"walk",tt_walk,"Walk",sizeof(tt_walk));
	add_xml_identifier(tooltips,"sit",tt_sit,"Sit down",sizeof(tt_sit));
	add_xml_identifier(tooltips,"stand",tt_stand,"Stand up",sizeof(tt_stand));
	add_xml_identifier(tooltips,"look",tt_look,"Look at",sizeof(tt_look));
	add_xml_identifier(tooltips,"use",tt_use,"Use",sizeof(tt_use));
	add_xml_identifier(tooltips,"use_witem",tt_use_witem,"Use with",sizeof(tt_use_witem));
	add_xml_identifier(tooltips,"trade",tt_trade,"Trade",sizeof(tt_trade));
	add_xml_identifier(tooltips,"attack",tt_attack,"Attack",sizeof(tt_attack));
	add_xml_identifier(tooltips,"invent",tt_inventory,"View inventory",sizeof(tt_inventory));
	add_xml_identifier(tooltips,"spell",tt_spell,"View spell window",sizeof(tt_spell));
	add_xml_identifier(tooltips,"manu",tt_manufacture,"View manufacture window",sizeof(tt_manufacture));
	add_xml_identifier(tooltips,"stats",tt_stats,"View stats",sizeof(tt_stats));
	add_xml_identifier(tooltips,"know",tt_knowledge,"View knowledge window",sizeof(tt_knowledge));
	add_xml_identifier(tooltips,"ency",tt_encyclopedia,"View encyclopedia window",sizeof(tt_encyclopedia));
	add_xml_identifier(tooltips,"quest",tt_questlog,"View questlog",sizeof(tt_questlog));
	add_xml_identifier(tooltips,"map",tt_mapwin,"View map",sizeof(tt_mapwin));
	add_xml_identifier(tooltips,"console",tt_console,"View console",sizeof(tt_console));
	add_xml_identifier(tooltips,"buddy",tt_buddy,"View buddy",sizeof(tt_buddy));
	add_xml_identifier(tooltips,"opts",tt_options,"View options",sizeof(tt_options));
	add_xml_identifier(tooltips,"help",tt_help,"View help",sizeof(tt_help));
	add_xml_identifier(tooltips,"costumize",tt_costumize,"Costumize your character",sizeof(tt_costumize));
	add_xml_identifier(tooltips,"name_pass",tt_name,"Choose name and password",sizeof(tt_name));

}
#endif

#ifdef ELC
void init_options()
{
	//Options
	add_xml_distringid(options_str,"shadows",&opt_shadows,"Shadows","Enables shadows - disable if you experience performance problems");
	add_xml_distringid(options_str,"clouds",&opt_clouds,"Clouds","Enables clouds - disable if you experience performance problems");
	add_xml_distringid(options_str,"reflections",&opt_reflections,"Reflections","Enable reflections - disable if you experience performance problems");
	add_xml_distringid(options_str,"fps",&opt_show_fps,"Show FPS","Show the current framerate in upper left corner");
	add_xml_distringid(options_str,"sitlock",&opt_sit_lock,"Sit lock","Locks you in a sitting position until you press the \"Stand\" button or right click to move.");
	add_xml_distringid(options_str,"caps",&opt_caps_filter,"Filter CAPS","Turns on/off a filter for capitalized letters");
	add_xml_distringid(options_str,"sound",&opt_sound,"Sound","Turns on/off sound effects");
	add_xml_distringid(options_str,"music",&opt_music,"Music","Turns on/off in-game music");
	add_xml_distringid(options_str,"autocam",&opt_autocam,"Auto Camera","Automatically change the camera according to the actor position");
	add_xml_distringid(options_str,"exit",&opt_exit,"Exit","Exits the game");
	add_xml_distringid(options_str,"fullscreen",&opt_full_screen,"Full Screen","Switches between full screen and windowed mode");
	add_xml_distringid(options_str,"strings",&opt_strings,"Options","Video mode");
}
#endif

#ifdef ELC
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
#endif

#ifdef ELC
void init_stats()
{
	group_stat * base = &(stats_str[0]);
	group_stat * cross = &(stats_str[1]);
	group_stat * misc = &(stats_str[2]);
	group_stat * nexus = &(stats_str[3]);
	group_stat * skills = &(stats_str[4]);

	//Initial strings
	add_xml_identifier(stats_extra,"base",attributes.base,"Basic Attributes",sizeof(attributes.base));
	add_xml_identifier(stats_extra,"cross",attributes.cross,"Cross Attributes",sizeof(attributes.cross));
	add_xml_identifier(stats_extra,"nexus",attributes.nexus,"Nexus",sizeof(attributes.nexus));
	add_xml_identifier(stats_extra,"skills",attributes.skills,"Skills",sizeof(attributes.skills));
	add_xml_identifier(stats_extra,"pickpoints",attributes.pickpoints,"Pickpoints",sizeof(attributes.pickpoints));

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
#endif

#ifdef ELC
void init_titles ()
{
	add_xml_identifier (titles_str, "w_note", win_notepad, "Notepad", sizeof(win_notepad));
	add_xml_identifier (titles_str, "w_prompt", win_prompt, "Prompt", sizeof(win_prompt));
	add_xml_identifier (titles_str, "t_help", tab_help, "Help", sizeof(tab_help));
	add_xml_identifier (titles_str, "t_ency", tab_encyclopedia, "Encyclopedia", sizeof(tab_encyclopedia));
	add_xml_identifier (titles_str, "t_skills", tab_skills, "Skills", sizeof(tab_skills));
	add_xml_identifier (titles_str, "t_rules", tab_rules, "Rules", sizeof(tab_rules));
	add_xml_identifier (titles_str, "t_stats", tab_statistics, "Statistics", sizeof(tab_statistics));
	add_xml_identifier (titles_str, "t_know", tab_knowledge, "Knowledge", sizeof(tab_knowledge));
	add_xml_identifier (titles_str, "t_qlog", tab_questlog, "Quest log", sizeof(tab_questlog));
	add_xml_identifier (titles_str, "t_all", tab_all, "All", sizeof(tab_all));
	add_xml_identifier (titles_str, "t_local", tab_local, "Local", sizeof(tab_local));
	add_xml_identifier (titles_str, "c_newbie", tab_newbie_channel, "Newbie Channel", sizeof(tab_newbie_channel));
	add_xml_identifier (titles_str, "c_help", tab_help_channel, "Help Channel", sizeof(tab_help_channel));
	add_xml_identifier (titles_str, "c_market", tab_market_channel, "Market Channel", sizeof(tab_market_channel));
	add_xml_identifier (titles_str, "c_general", tab_general_channel, "General Channel", sizeof(tab_general_channel));
	add_xml_identifier (titles_str, "c_offtopic", tab_offtopic_channel, "Offtopic Channel", sizeof(tab_offtopic_channel));
	add_xml_identifier (titles_str, "t_channel", tab_channel, "Channel %d", sizeof(tab_channel));
	add_xml_identifier (titles_str, "c_guild", tab_guild, "Guild", sizeof(tab_guild));
	add_xml_identifier (titles_str, "c_mod", tab_mod, "Mod", sizeof(tab_mod));
	add_xml_identifier (titles_str, "c_personal", tab_personal, "PM", sizeof(tab_personal));
	add_xml_identifier (titles_str, "c_server", tab_server, "Server", sizeof(tab_server));
	add_xml_identifier (titles_str, "t_main", tab_main, "Main", sizeof(tab_main));
	add_xml_identifier (titles_str, "b_okay", button_okay, "Okay", sizeof(button_okay));
	add_xml_identifier (titles_str, "b_cancel", button_cancel, "Cancel", sizeof(button_cancel));
	add_xml_identifier (titles_str, "b_new_cat", button_new_category, "New Category", sizeof(button_new_category));
	add_xml_identifier (titles_str, "b_rm_cat", button_remove_category, "Remove Category", sizeof(button_remove_category));
	add_xml_identifier (titles_str, "b_save", button_save_notes, "Save Notes", sizeof(button_save_notes));
	add_xml_identifier (titles_str, "l_nname", label_note_name, "Note name", sizeof(label_note_name));
	add_xml_identifier (titles_str, "l_cursor_coords", label_cursor_coords, "Cursor position", sizeof(label_cursor_coords));
}
#endif // ELC

#ifdef WRITE_XML
void save_strings(xmlDoc * doc, char * name)
{
	char str[50];
	
	//default language is en - change this if you want to save the strings to another folder...
	snprintf (str, sizeof (str), "languages/en/strings/%s", name); 
	xmlSaveFormatFileEnc (str, doc, "UTF-8", 1);//We'll save the file in UTF-8
}
#endif

void load_translatables()
{
	struct xml_struct file=load_strings("console.xml");
#ifdef ELC
	if(file.file!=NULL) {
		//Parse file
		parse_console(file.root);
#ifdef WRITE_XML
		save_strings(file.file,"console.xml");
#endif
		xmlFreeDoc(file.file);
	}
#endif
	file = load_strings("errors.xml");
	if(file.file!=NULL) {
		parse_errors(file.root);
#ifdef WRITE_XML
		save_strings(file.file,"errors.xml");
#endif
		xmlFreeDoc(file.file);
	}
#ifdef ELC
	file = load_strings("help.xml");
	if(file.file!=NULL) {
		parse_help(file.root);
#ifdef WRITE_XML
		save_strings(file.file,"help.xml");
#endif
		xmlFreeDoc(file.file);
	}
#endif
#ifdef ELC
	file = load_strings("options.xml");
	if(file.file!=NULL) {
		parse_options(file.root);
#ifdef WRITE_XML
		save_strings(file.file,"options.xml");
#endif
		xmlFreeDoc(file.file);
	}
#endif
#ifdef ELC
	file = load_strings("spells.xml");
	if(file.file!=NULL) {
		parse_spells(file.root);
#ifdef WRITE_XML
		save_strings(file.file,"spells.xml");
#endif
		xmlFreeDoc(file.file);
	}
#endif 
#ifdef ELC
	file = load_strings("stats.xml");
	if(file.file!=NULL){
		parse_stats(file.root);
#ifdef WRITE_XML
		save_strings(file.file,"stats.xml");
#endif
		xmlFreeDoc(file.file);
	}
#endif
#ifdef ELC
	file = load_strings("titles.xml");
	if(file.file!=NULL){
		parse_titles(file.root);
#ifdef WRITE_XML
		save_strings(file.file,"titles.xml");
#endif
		xmlFreeDoc(file.file);
	}
#endif
#ifndef WRITE_XML
//There's no need for these variables to be hanging around any more...
	free_xml_parser(GROUP,errors,ERRORS);
#ifdef ELC
	free_xml_parser(GROUP,console_str,CONSOLE_STR);
	free_xml_parser(GROUP,help_str,HELP_STR);
	free_xml_parser(DIGROUP,options_str,OPTIONS_STR);
	free_xml_parser(DIGROUP,sigils_str,SIGILS_STR);
	free_xml_parser(STAT_GROUP,stats_str,STATS_STR);
	free_xml_parser(GROUP,stats_extra,STATS_EXTRA);
	free_xml_parser(GROUP,titles_str,TITLES_STR);
#endif
#endif
}

struct xml_struct load_strings(char * file)
{
	char file_name[120];
	struct xml_struct tmp={NULL,NULL};
	sprintf(file_name,"languages/%s/strings/%s",lang,file);
	tmp=load_strings_file(file_name);
	if(tmp.file==NULL||tmp.root==NULL){
		sprintf(file_name,"languages/en/strings/%s",file);
		tmp=load_strings_file(file_name);
		if(tmp.file==NULL){
			//Notify about this error - english only
			log_error("Could not read %s", file);
		}
	}
	return tmp;
}

struct xml_struct load_strings_file(char * filename)
{
	struct xml_struct file={NULL,NULL};
	if ((file.file = xmlReadFile(filename, NULL, 0)) == NULL){
#ifdef WRITE_XML
		if ((file.file = xmlNewDoc(BAD_CAST "1.0"))==NULL){
			xmlFreeDoc(file.file);
			file.file=NULL;
		}
#else
		xmlFreeDoc(file.file);
		file.file=NULL;
#endif
	}
	if(file.file){
		if((file.root=xmlDocGetRootElement(file.file))==NULL){
#ifdef WRITE_XML
			file.root=xmlNewNode(NULL,"root");
			xmlDocSetRootElement (file.file, file.root);
			if((file.root=xmlDocGetRootElement(file.file))==NULL) {
#endif
				log_error("Fatal: couldn't find root element in %s\n",filename);
				xmlFreeDoc(file.file);
				file.file=NULL;
#ifdef WRITE_XML
			}
#endif
		}
	}
	return file;
}

void copy_strings(xmlNode * in, distring_item * string)
{
	xmlNode *cur = in->children?in->children:in;
	for(;cur; cur = cur-> next) {
		if(cur->type == XML_ELEMENT_NODE) {
			if(cur->children) {
				if(!xmlStrcasecmp(cur->name,"name")) {
					char *p=string->var->str;
					my_xmlStrncopy(&p, cur->children->content, 30);
#ifdef WRITE_XML
					string->var->saved_str=1;
#endif
				} else if (!xmlStrcasecmp(cur->name,"desc")) {
					char *p=string->var->desc;
					my_xmlStrncopy(&p, cur->children->content, 100);
#ifdef WRITE_XML
					string->var->saved_desc=1;
#endif
				}
			}
		}
	}
#ifdef WRITE_XML
	if(!string->var->saved_str) xmlNewTextChild(in, NULL, "name", string->var->str);
	if(!string->var->saved_desc) {
		if(string->var->desc!=NULL) xmlNewTextChild(in, NULL, "desc", string->var->desc);
		else xmlNewTextChild(in, NULL, "desc", " ");
	}
#endif
}
#ifdef ELC
void copy_stats(xmlNode * in, statstring_item * string)
{
	xmlNode *cur = in->children?in->children:in;
	for(; cur; cur = cur-> next) {
		if(cur->type == XML_ELEMENT_NODE) {
			if(cur->children){
				if(!xmlStrcasecmp(cur->name,"name")) {
					char *p=string->var->name;
					my_xmlStrncopy(&p, cur->children->content, 20);
#ifdef WRITE_XML
					string->var->saved_name=1;
#endif
				} else if (!xmlStrcasecmp(cur->name,"shortname")){
					char *p=string->var->shortname;
					my_xmlStrncopy(&p, cur->children->content, 5);
#ifdef WRITE_XML
					string->var->saved_shortname=1;
#endif
				}
			}
		}
	}
#ifdef WRITE_XML
	if(!string->var->saved_name) xmlNewTextChild(in, NULL, "name", string->var->name);
	if(!string->var->saved_shortname) xmlNewTextChild(in, NULL, "shortname", string->var->shortname);
#endif
}
#endif

#ifdef ELC
void parse_statstrings(xmlNode * in, group_stat * group)
{
	xmlNode * cur = in->children?in->children:in;
	int i;
	for(;cur;cur=cur->next) {
		if(cur->type == XML_ELEMENT_NODE) {
			if(cur->children) {
				for(i=0;i<group->no;i++){
					if(!xmlStrcasecmp(cur->name,group->statstrings[i]->xml_id)) {
						copy_stats(cur->children,group->statstrings[i]);
#ifdef WRITE_XML
						group->statstrings[i]->saved=1;
#endif
						break;
					}
				}
			}
		}
	}
#ifdef WRITE_XML
	for(i=0;i<group->no;i++) {
		if(!group->statstrings[i]->saved) {
			cur=xmlNewTextChild(in, NULL, group->statstrings[i]->xml_id, NULL);
			copy_stats(cur, group->statstrings[i]);
		}
	}
#endif
}
#endif

void parse_distrings(xmlNode * in, group_id_di * group)
{
	xmlNode * cur = in->children?in->children:in;
	int i;
	for(;cur;cur=cur->next) {
		if(cur->type==XML_ELEMENT_NODE) {
			if(cur->children) {
				for(i=0;i<group->no;i++){
					if(!xmlStrcasecmp(cur->name,group->distrings[i]->xml_id)){
						copy_strings(cur->children,group->distrings[i]);
#ifdef WRITE_XML
						group->distrings[i]->saved=1;
#endif
						break;
					}
				}
			}
		}
	}
#ifdef WRITE_XML
	for(i=0;i<group->no;i++) {
		if(!group->distrings[i]->saved) {
			cur=xmlNewTextChild(in, NULL, group->distrings[i]->xml_id, NULL);
			copy_strings(cur, group->distrings[i]);
		}
	}
#endif
}

void parse_strings(xmlNode * in, group_id * group)
{
	int i;
	xmlNode * cur = in->children?in->children:in;
	for(;cur;cur=cur->next) {
		if(cur->type==XML_ELEMENT_NODE) {
			if(cur->children) {
				for(i=0;i<group->no;i++){
					if(!xmlStrcasecmp(cur->name,group->strings[i]->xml_id))	{
						my_xmlStrncopy(&group->strings[i]->var, cur->children->content, group->strings[i]->max_len);
#ifdef WRITE_XML
						group->strings[i]->saved=1;
#endif
						break;
					}
				}
			}
		}
	}
#ifdef WRITE_XML
	for(i=0;i<group->no;i++) {
		if(!group->strings[i]->saved) {
			xmlNewTextChild(in, NULL, group->strings[i]->xml_id, group->strings[i]->var);
		}
	}
#endif
}

void parse_groups(xmlNode * in, void * gPtr, int size, int type)
{
	group_id * group=gPtr;
	group_id_di * Group=gPtr;
#ifdef ELC
	group_stat * stat=gPtr;
#endif
	int i;
	xmlNode * cur = in->children?in->children:in;
	for(;cur;cur=cur->next) {
		if(cur->type==XML_ELEMENT_NODE) {
			for(i=0;i<size;i++) {
				switch(type) {
					case GROUP:
						if(!xmlStrcasecmp(cur->name,group[i].xml_id)) {
							parse_strings(cur,&(group[i]));
#ifdef WRITE_XML
							group[i].saved=1;
#endif
							i=size;
						}
						break;
					case DIGROUP:
						if(!xmlStrcasecmp(cur->name,Group[i].xml_id)) {
							parse_distrings(cur,&(Group[i]));
#ifdef WRITE_XML
							Group[i].saved=1;
#endif
							i=size;
						}
						break;
#ifdef ELC
					case STAT_GROUP:
						if(!xmlStrcasecmp(cur->name,stat[i].xml_id)) {
							parse_statstrings(cur,&(stat[i]));
#ifdef WRITE_XML
							stat[i].saved=1;
#endif
							i=size;
						}
						break;
#endif
					default: break;
				}
			}
		}
	}
#ifdef WRITE_XML
	for(i=0;i<size;i++) {
		switch(type) {
			case GROUP:
				if(!group[i].saved) {
					cur=xmlNewChild(in, NULL, group[i].xml_id, NULL);
					parse_strings(cur,&(group[i]));
				}
				break;
			case DIGROUP:
				if(!Group[i].saved) {
					cur=xmlNewChild(in, NULL, Group[i].xml_id, NULL);
					parse_distrings(cur,&(Group[i]));
				}
				break;
			case STAT_GROUP:
				if(!stat[i].saved) {
					cur=xmlNewChild(in, NULL, stat[i].xml_id, NULL);
					parse_statstrings(cur,&(stat[i]));
				}
			default: break;
		}
	}
#endif
}

#ifdef ELC
void parse_console(xmlNode * in)
{
	parse_groups(in,console_str,CONSOLE_STR, GROUP);
}
#endif

void parse_errors(xmlNode * in)
{
	parse_groups(in,errors,ERRORS,GROUP);
}

#ifdef ELC
void parse_help(xmlNode * in)
{
	parse_groups(in, help_str, HELP_STR, GROUP);
}
#endif

#ifdef ELC
void parse_options(xmlNode * in)
{
	parse_groups(in, options_str, OPTIONS_STR, DIGROUP);
}
#endif

#ifdef ELC
void parse_spells(xmlNode * in)
{
	parse_groups(in, sigils_str, SIGILS_STR, DIGROUP);
}
#endif

#ifdef ELC
void parse_stats(xmlNode * in)
{
	parse_groups(in, stats_extra, STATS_EXTRA, GROUP);
	parse_groups(in, stats_str, STATS_STR, STAT_GROUP);
}
#endif 

#ifdef ELC
void parse_titles(xmlNode * in)
{
	parse_groups(in, titles_str, TITLES_STR, GROUP);
}
#endif

void free_xml_parser(int type, void * gPtr, int no)
{
	group_id * grp=gPtr;
	group_id_di * Grp=gPtr;
#ifdef ELC
	group_stat * stat=gPtr;
#endif
	int i=0,j;
	switch(type) {
		case GROUP:
			for(;i<no;i++){
				for(j=0;j<grp[i].no;j++) {
					free(grp[i].strings[j]);
				}
				free(grp[i].strings);
			}
			free(grp);
			break;
		case DIGROUP:
			for(;i<no;i++) {
				for(j=0;j<Grp[i].no;j++) {
					free(Grp[i].distrings[j]);
				}
				free(Grp[i].distrings);
			}
			free(Grp);
			break;
#ifdef ELC
		case STAT_GROUP:
			for(;i<no;i++) {
				for(j=0;j<stat[i].no;j++) {
					free(stat[i].statstrings[j]);
				}
				free(stat[i].statstrings);
			}
			free(stat);
#endif
		default: break;
	}
}
