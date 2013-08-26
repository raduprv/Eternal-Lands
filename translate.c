#include <stdio.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <string.h>
#include <stdarg.h>
#include "translate.h"
#include "asc.h"
#include "errors.h"
#include "init.h"

#define GROUP 0
#define DIGROUP 1
#ifdef ELC
#define STAT_GROUP 2
#endif


#ifdef ELC
typedef struct
{
	char *name;
	const char *string;
} named_string;

typedef struct
{
	char *name;
	size_t num_strings;
	named_string *strings;
} string_group;

static string_group* named_strings = NULL;
static size_t num_named_strings = 0;
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
	tt_customize[60],
	newchar_warning[50],
	newchar_cust_help[100],
	newchar_cred_help[100],
	newchar_done_help[100],
	tt_name[60],
	tt_info[30],
	tt_emotewin[30],
	tt_rangewin[30],
	tt_minimap[30];

#endif // ELC

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
	/*buddy.c*/
	buddy_name_str[20],
	buddy_long_name_str[100],
	buddy_type_str[20],
	buddy_long_type_str[100],
	buddy_add_str[30],
	buddy_change_str[30],
	buddy_accept_str[30],
	yes_str[10],
	no_str[10],
	buddy_delete_str[20],
	buddy_long_delete_str[100],
	buddy_wants_to_add_str[150],
	buddy_add_to_list_str[180],
	buddy_logon_str[30],
	buddy_online_str[30],
	buddy_logoff_str[30],
	buddy_white_str[10],
	buddy_red_str[10],
	buddy_green_str[10],
	buddy_blue_str[10],
	buddy_yellow_str[10],
	buddy_request_str[10],
	/* chat.c */
	channel_help_str[200],
	channel_color_title_str[30],
	channel_color_str[40],
	channel_color_add_str[20],
	channel_color_delete_str[20],
	/* console.c */
	help_cmd_markpos_str[50],
	location_info_str[40],
	knowledge_cmd_str[40],
	marked_str[30],
	unmarked_str[30],
	urlcmd_none_str[30],
	urlcmd_list_str[30],
	win_url_str[30],
	urlcmd_invalid_str[30],
	urlcmd_afk_str[30],
	urlcmd_clear_str[30],
	urlwin_open_str[50],
	urlwin_clear_str[30],
	/*draw_scene.c*/
	low_framerate_str[100],
	/*gamewin.c*/
	ranginglock_enabled_str[100],
	ranginglock_disabled_str[50],
	/*gl_init.c*/
	window_size_adjusted_str[50],
	/*hud.c*/
	no_open_on_trade[100],
	stats_scroll_help_str[100],
	remove_bar_message_str[50],
	cm_action_points_str[30],
	/*loginwin.c*/
	login_username_str[20],
	login_password_str[20],
	login_rules_str[120],
	/*items.c*/
	sto_all_str[8],
	get_all_str[8],
	drp_all_str[8],
	itm_lst_str[8],
	mix_one_str[8],
	mix_all_str[8],
	auto_get_all_str[30],
	item_list_but_str[35],
	inv_keeprow_str[30],
	quantity_edit_str[100],
	equip_here_str[100],
	equip_str[20],
	pick_item_help_str[50],
	multiuse_item_help_str[50],
	stoall_help_str[50],
	getall_help_str[50],
	dcdrpall_help_str[50],
	drpall_help_str[50],
	itmlst_help_str[50],
	mixoneall_help_str[50],
	items_stack_str[100],
	mixbut_empty_str[80],
	mix_empty_str[50],
	click_clear_str[50],
	double_click_clear_str[50],
	recipe_select_str[50],
	recipe_load_str[50],
	recipe_find_str[50],
	recipe_during_find_str[50],
	recipe_show_hide_str[70],
	recipe_save_str[70],
	/*knowledge.c*/
	completed_research[12],
	lessthanaminute_str[30],
	researching_str[30],
	not_researching_anything[25],
	not_researching_str[25],
	countdown_str[20],
	stopwatch_str[20],
	minutes_str[15],
	minute_str[15],
	idle_str[15],
	knowledge_read_book[15],
	knowledge_param_read[15],
	knowledge_param_unread[15],
	knowledge_param_total[15],
	unknown_book_short_str[50],
	unknown_book_long_str[150],
	know_highlight_prompt_str[20],
	know_highlight_cm_str[70],
	/*manufacture.c*/
	mix_str[5],
	mixall_str[10],
	clear_str[6],
	reset_str[6],
	manu_add_str[60],
	manu_remove_str[60],
	/*multiplayer.c*/
	connect_to_server_str[50],
	reconnect_str[50],
	alt_x_quit[50],
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
	error_bad_pass[30],
	error_confirm_create_char[100],
	error_max_digits[100],
	error_length[100],
	error_illegal_character[100],
	passwords_match[30],
	remember_change_appearance[200],
	appearance_str[15],
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
	/* ranging window */
	ranging_win_title_str[20],
	ranging_total_shots_str[40],
	ranging_sucessful_shots_str[40],
	ranging_missed_shots_str[40],
	ranging_success_rate_str[40],
	ranging_critical_rate_str[40],
	ranging_exp_per_arrow_str[40],
	/* session.c */
	session_reset_help[60],
	/*trade.c*/
	quantity_str[30],
	abort_str[10],
	you_str[10],
	accept_str[12],
	/*update.c*/
	update_complete_str[40],
	video_restart_str[80],
	rotate_chat_log_restart_str[80],
	client_restart_countdown_str[40],
	client_restarting_str[20],
	restart_now_label[20],
	/* context menu strings */
	cm_quickspell_menu_str[50],
	cm_textedit_menu_str[100],
	cm_quickbar_menu_str[150],
	cm_hud_menu_str[250],
	cm_banner_menu_str[240],
	cm_title_menu_str[150],
	cm_title_help_str[50],
	cm_items_menu_str[150],
	cm_storage_menu_str[75],
	cm_astro_menu_str[80],
	cm_ranging_menu_str[50],
	cm_dialog_options_str[80],
	cm_dialog_menu_str[60],
	cm_url_menu_str[150],
	cm_counters_menu_str[160],
	cm_help_options_str[50],
	cm_npcname_menu_str[60],
	cm_dialog_copy_menu_str[50],
	cm_minimap_menu_str[60],
	cm_user_menu_str[150],
	cm_item_list_selected_str[40],
	cm_item_list_names_str[100],
	cm_stats_bar_base_str[30],
	cm_recipe_menu_str[100],
	cm_manuwin_menu_str[50],
	/* user_menus.cpp */
	um_invalid_command_str[50],
	um_invalid_line_str[50],
	um_no_menus_str[50],
	um_window_title_str[50],
	/* quest_log.cpp */
	cm_questlog_menu_str[400],
	cm_questlist_menu_str[150],
	questlog_find_prompt_str[30],
	questlog_add_npc_prompt_str[20],
	questlog_add_text_prompt_str[20],
	questlog_npc_filter_title_str[20],
	questlist_filter_title_str[20],
	questlist_showall_str[20],
	questlog_cm_help_str[50],
	questlog_deldupe_start_str[50],
	questlog_deldupe_end_str[75],
	questlog_deleted_str[20],
	/* new_character.c */
	use_appropriate_name[500],
	item_list_use_help_str[40],
	item_list_pickup_help_str[40],
	item_list_edit_help_str[40],
	item_list_add_help_str[40],
	item_list_drag_help_str[40],
	item_list_create_help_str[40],
	item_list_magic_str[80],
	item_list_find_str[20],
	item_list_find_help_str[40];
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
	pm_from_str[10],
	gm_from_str[10],
	ig_from_str[10],
	mc_from_str[20],
	mod_pm_from_str[15],
	help_request_str[20],
	help_cmd_str[10],
	date_format[100],
	book_count_str[60],
	know_help_str[60],
	char_cmd_str[2],
	char_at_str[2],
	char_slash_str[2],
	gm_cmd_str[5],
	mod_cmd_str[5],
	bc_cmd_str[5],
	msg_accept_buddy_str[55],
	logconn_str[50],
	time_warn_hour_str[75],
	time_warn_sunrise_str[100],
	time_warn_sunset_str[100],
	time_warn_day_str[75],
	config_location_str[75],
	datadir_location_str[75],
	no_spell_to_show_str[40],
	invalid_spell_string_str[40],
	command_too_long_str[40],
	item_list_learn_cat_str[90],
	cmd_ignores[20],
	cmd_ignore[20],
	cmd_unignore[20],
	cmd_filter[20],
	cmd_filters[20],
	cmd_unfilter[20],
	cmd_glinfo[10],
	cmd_knowledge[20],
	cmd_knowledge_short[10],
	cmd_markpos[20],
	cmd_mark[20],
	cmd_unmark[20],
	cmd_stats[10],
	cmd_time[10],
	cmd_date[10],
	cmd_exit[10],
	cmd_msg[10],
	cmd_afk[5],
	cmd_keypress[20],
	cmd_user_menu_wait_time_ms[30],
	cmd_open_url[20],
	cmd_show_spell[20],
	cmd_cast_spell[20],
	cmd_reload_icons[20],
	cmd_session_counters[20];
#endif

/*! \name Errors */
/*! \{ */
char	reg_error_str[15],
	file_write_error_str[20],
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
	/* books.c */
	book_open_err_str[30],
	/*cache.c*/
	cache_items_str[20],
	cache_size_str[20],
	/* cal.c */
	no_animation_err_str[30],
	/* console.c */
	invalid_location_str[30],
	/*cursors.c*/
	cursors_file_str[30],
	/*dialogues.c*/
	close_str[20],
	dialogue_copy_str[20],
	dialogue_repeat_str[20],
	open_storage_str[20],
	reopen_storage_str[50],
#endif
	xml_bad_node[80],
	xml_bad_root_node[50],
	xml_undefined_node[80],
	using_builtin_chanlist[120],
	using_eng_chanlist[120],
	/*font.c*/
	cant_load_font[30],
#ifdef ELC
	/*gamewin.c*/
	no_walk_with_sitlock[100],
	/*init.c*/
	no_stencil_str[150],
	safemode_str[150],
	no_sdl_str[30],
	no_hardware_stencil_str[150],
	suggest_24_or_32_bit[150],
	fail_opengl_mode[30],
	stencil_falls_back_on_software_accel[150],
	last_chance_str[150],
	software_mode_str[200],
	gl_ext_found[100],
	gl_ext_found_not_used[100],
	gl_ext_not_found[100],
	gl_ext_no_multitexture[150],
	disabled_shadow_mapping[50],
	shadow_map_size_not_supported_str[100],
	disabled_framebuffer[50],
	/* framebuffer.c */
	fbo_attachment_error[100],
	fbo_missing_attachment_error[100],
	fbo_dimensions_error[100],
	fbo_formats_error[100],
	fbo_draw_buffer_error[100],
	fbo_read_buffer_error[100],
	fbo_unsupported_fromat_error[100],
	fbo_unknown_error[100],
	fbo_supported_format[100],
	gl_ext_not_found_emul_it[100],
	fatal_error_str[10],
	no_e3d_list[50],
	enabled_vertex_arrays[50],
	disabled_compiled_vertex_arrays[50],
	disabled_point_particles[50],
	disabled_particles_str[50],
	invalid_video_mode[75],
	failed_sdl_net_init[30],
	failed_sdl_timer_init[30], 
	cant_read_elini[50],
	must_use_tabs[80],
	init_opengl_str[35],
	init_random_str[35],
	load_ignores_str[35],
	load_filters_str[35],
	load_lists_str[35],
	load_cursors_str[35],
	bld_glow_str[35],
	init_lists_str[35],
	init_actor_defs_str[35],
	load_map_tiles_str[35],
	init_lights_str[35],
	init_logs_str[35],
	read_config_str[35],
	init_weather_str[35],
	init_audio_str[35],
	load_icons_str[35],
	load_textures_str[35],
#ifdef PAWN
	init_pawn_str[35],
#endif // PAWN
	init_network_str[35],
	init_timers_str[35],
	load_encyc_str[35],
	init_display_str[35],
	prep_op_win_str[35],
	/* interface;c */
	err_mapmarks_str[60],
	err_nomap_str[60],
	/* map_io.c */
	load_map_str[35],
	load_3d_object_str[35],
	load_2d_object_str[35],
	load_lights_str[35],
	load_particles_str[35],
	bld_sectors_str[35],
	init_done_str[35],
	/* mines.c */
	mines_config_open_err_str[50],
	mines_config_error[50],
	/* misc.c */
#ifdef PNG_SCREENSHOT
	max_screenshots_warning_str[200],
#endif //PNG_SCREENSHOT
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
	empty_map_str[100],
	no_nomap_str[150],
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
	/* item lists */
	item_list_format_error[50],
	item_list_save_error_str[50],
	item_list_cat_format_error_str[50],
	item_list_version_error_str[70],
	item_list_empty_list_str[50],
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
	accepted_rules[80],
	accept_label[20],
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
	snd_config_open_err_str[50],
	snd_config_error[50],
	snd_sound_overflow[50],
	snd_media_read[50],
	snd_media_notvorbis[50],
	snd_media_ver_mismatch[50],
	snd_media_invalid_header[50],
	snd_media_internal_error[50],
	snd_media_false[50],
	snd_media_eof[50],
	snd_media_hole[50],
	snd_media_einval[50],
	snd_media_ebadlink[50],
	snd_media_enoseek[50],	
	snd_media_ogg_error[50],
	snd_no_music[50],
	snd_media_music_stopped[50],
	snd_media_ogg_info[50],
	snd_media_ogg_info_noartist[50],
	/*stats.c*/
	stat_no_invalid[50],
	/*timers.c*/
	timer_lagging_behind[100],
	/*spells.c*/
	cast_str[20],
	invalid_spell_str[20],
	/* notepad.c */
	cant_parse_notes[100],
	notes_wrong[100],
	too_many_notes[100],
	wrong_note_node[100],
	cant_save_notes[100],
	exceed_note_buffer[100],
	user_no_more_notes[100],
	user_no_more_note_tabs[100],
	fatal_data_error[120],
	dc_note_remove[50],
	note_saved[50],
	note_save_failed[50],
	/* encyclopedia */
	cant_load_encycl[70],
	/* text.c */
	warn_currently_ignoring[50];
#else
	;
#endif  // ELC
/*! \} */

#ifdef ELC
/*! \name Window/Tab titles */
/*! \{ */
char	win_notepad[20],
	win_prompt[10],
	win_statistics[20],
	win_sigils[10],
	win_help[10],
	win_buddy[10],
	win_configuration[20],
	win_manufacture[20],
	win_astrology[20],
	win_principal[20],
	win_storage[10],
	win_storage_vo[15],
	win_trade[10],
	win_rules[10],
	win_bag[5],
	win_inventory[15],
	win_newchar[20],
	win_minimap[20],
	win_name_pass[30],
	win_design[25],
	ttab_controls[10],
	ttab_audio[10],
	ttab_hud[5],
	ttab_server[10],
	ttab_chat[10],
	ttab_video[10],
	ttab_gfx[10],
	ttab_camera[15],
	ttab_troubleshoot[15],
	ttab_font[10],
	tab_help[10],
	tab_encyclopedia[20],
	tab_skills[20],
	tab_rules[20],
	tab_statistics[20],
	tab_knowledge[20],
	tab_questlog[20],
	tab_counters[20],
	tab_session[20],
	tab_main[20],
	item_list_name_str[30],
	item_list_rename_str[30],
	item_list_preview_title[30],
	item_list_quantity_str[20],
	button_okay[10],
	button_send[10],
	button_cancel[10],
	button_new_category[30],
	button_remove_category[30],
	button_save_notes[30],
	label_note_name[20],
	game_version_str[60],
	label_cursor_coords[17],
	label_mark_filter[13];
#endif  // ELC
/*! \} */

#ifdef ELC
#define CONSOLE_STR 5
#define ERRORS 7
#define HELP_STR 5
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
group_id_di * options_str;
#ifdef ELC
group_id * console_str;
group_id * help_str;
group_id_di * sigils_str;
group_stat * stats_str;
group_id * stats_extra;
group_id * titles_str;
#endif

void init_console(void);
void init_help(void);
void init_spell_translatables ();
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
	console_str=add_xml_group(GROUP,CONSOLE_STR,"filter","ignore","misc","loading_msg","cmd");
	errors=add_xml_group(GROUP,ERRORS,"actors","load","misc","particles","snd","video","rules");
	help_str=add_xml_group(GROUP,HELP_STR,"afk","misc","new","tooltips","buddy");
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
			for(;i<no;i++)
				safe_snprintf (grp[i].xml_id, sizeof (grp[i].xml_id), "%s", va_arg (ap, char*));
			return grp;
		}
		case DIGROUP: {
			group_id_di * grp;
			grp=(group_id_di*)calloc(no,sizeof(group_id_di));
			for(;i<no;i++)
				safe_snprintf (grp[i].xml_id, sizeof (grp[i].xml_id), "%s", va_arg (ap, char*));
			return grp;
		}
#ifdef ELC
		case STAT_GROUP: {
			group_stat * grp;
			grp=(group_stat*)calloc(no,sizeof(group_stat));
			for(;i<no;i++)
				safe_snprintf (grp[i].xml_id, sizeof (grp[i].xml_id), "%s", va_arg (ap, char*));
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
	safe_snprintf (group->distrings[group->no]->xml_id, sizeof (group->distrings[group->no]->xml_id), "%s", xml_id);
	group->distrings[group->no]->var=var;
	safe_snprintf((char*)var->str, sizeof(var->str), "%s", str);
	safe_snprintf((char*)var->desc, sizeof(var->desc), "%s", desc);
	group->no++;
}

#ifdef ELC
void add_xml_statid(group_stat * group, char * xml_id, names * var, char * name, char * shortname)
{
	group->statstrings=(statstring_item**)realloc(group->statstrings,(group->no+1)*sizeof(statstring_item*));
	group->statstrings[group->no]=(statstring_item*)calloc(1,sizeof(statstring_item));
	safe_snprintf (group->statstrings[group->no]->xml_id, sizeof (group->statstrings[group->no]->xml_id), "%s", xml_id);
	group->statstrings[group->no]->var=var;
	safe_strncpy((char*)var->name, name, sizeof(var->name));
	safe_strncpy((char*)var->shortname, shortname, sizeof(var->shortname));
	group->no++;
}
#endif

void add_xml_identifier(group_id * group, char * xml_id, char * var, char * def, int max_len)
{
	group->strings=(string_item**)realloc(group->strings,(group->no+1)*sizeof(string_item*));
	group->strings[group->no]=(string_item*)calloc(1,sizeof(string_item));
	safe_snprintf (group->strings[group->no]->xml_id, sizeof (group->strings[group->no]->xml_id), "%s", xml_id);
	group->strings[group->no]->var=var;
	safe_strncpy(var, def, max_len);
	group->strings[group->no]->max_len=max_len-1;
	group->no++;
}
#ifdef ELC
void add_options_distringid(char * xml_id, dichar * var, char * str, char * desc)
{
	add_xml_distringid(options_str, xml_id, var, str, desc);
}
#endif //ELC
void init_translatables()
{
	init_groups();
	init_errors();
#ifdef ELC
	init_console();
	init_help();
	init_spell_translatables ();
	init_stats();
	init_titles();
#endif
}

#ifdef ELC
/* Save translated strings with their names for later lookup.
*/
void save_named_strings(const group_id *groups, size_t num_groups, const char *group_name)
{
	size_t i,j;

	for (j=0; j<num_groups; j++)
	{
		if (strcmp(groups[j].xml_id, group_name) == 0)
		{
			named_strings = (string_group*)realloc(named_strings, (num_named_strings+1) * sizeof(string_group));
			named_strings[num_named_strings].name = (char *)malloc(sizeof(char *) * (strlen(group_name) + 1));
			strcpy(named_strings[num_named_strings].name, group_name);

			named_strings[num_named_strings].num_strings = groups[j].no;
			named_strings[num_named_strings].strings = (named_string*)malloc(sizeof(named_string) * groups[j].no);

			for (i=0; i<groups[j].no; i++)
			{
				named_strings[num_named_strings].strings[i].name = (char *)malloc(sizeof(char *) * (strlen(groups[j].strings[i]->xml_id) + 1));
				strcpy(named_strings[num_named_strings].strings[i].name, groups[j].strings[i]->xml_id);
				named_strings[num_named_strings].strings[i].string = groups[j].strings[i]->var;
			}

			num_named_strings++;
			return;
		}
	}
}
#endif

#ifdef ELC
/* Retrieve a translated string by its name.
*/
const char* get_named_string(const char* group_name, const char* string_name)
{
	size_t i,j;
	if ((group_name!=NULL) && (string_name!=NULL))
		for (j=0; j<num_named_strings; j++)
			if (strcmp(named_strings[j].name, group_name) == 0)
				for (i=0; i<named_strings[j].num_strings; i++)
					if (strcmp(named_strings[j].strings[i].name, string_name) == 0)
						return named_strings[j].strings[i].string;
	return "Unknown string";
}
#endif

/* Free the memory allocated by translation module
*/
void free_translations(void)
{
#ifdef ELC
	/* the named strings */
	{
	size_t i,j;
	for (j=0; j<num_named_strings; j++)
	{
		for (i=0; i<named_strings[j].num_strings; i++)
			free(named_strings[j].strings[i].name);
		free(named_strings[j].name);
		free(named_strings[j].strings);
	}
	free(named_strings);
	num_named_strings = 0;
	named_strings = NULL;
	}
#endif
}

#ifdef ELC
void init_console()
{
	group_id * filter=&(console_str[0]);
	group_id * ignore=&(console_str[1]);
	group_id * misc=&(console_str[2]);
	group_id * loading_msg=&(console_str[3]);
	group_id * cmd_grp=&(console_str[4]);
	
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
	add_xml_identifier (filter,"flistfull", filter_list_full, "Your filter list is full, you can't add another filter", sizeof (filter_list_full));
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
	add_xml_identifier(misc,"pm_from",pm_from_str,"[PM from",sizeof(pm_from_str));
	add_xml_identifier(misc,"mod_pm_from",mod_pm_from_str,"[Mod PM from",sizeof(mod_pm_from_str));
	add_xml_identifier(misc,"gm_from",gm_from_str,"#GM from",sizeof(gm_from_str));
	add_xml_identifier(misc,"ig_from",ig_from_str,"#Ig [",sizeof(ig_from_str));
	add_xml_identifier(misc,"mc_from",mc_from_str,"#Mod Chat from",sizeof(mc_from_str));
	add_xml_identifier(misc,"date_format",date_format,"Today is the %s day in the month of %s, the year %04d, Age of the Eternals",sizeof(date_format));
	add_xml_identifier(misc,"book_count",book_count_str,"You have read %d of %d matching books",sizeof(book_count_str));
	add_xml_identifier(misc,"know_help",know_help_str,"Use -(t)otal -(r)ead or -(u)nread to select output",sizeof(know_help_str));
	add_xml_identifier(misc,"time_warn_hour",time_warn_hour_str,"This is your %d minute warning for the coming hour.",sizeof(time_warn_hour_str));
	add_xml_identifier(misc,"time_warn_sunrise",time_warn_sunrise_str,"This is your %d minute warning for the coming sunrise.",sizeof(time_warn_sunrise_str));
	add_xml_identifier(misc,"time_warn_sunset",time_warn_sunset_str,"This is your %d minute warning for the coming sunset.",sizeof(time_warn_sunset_str));
	add_xml_identifier(misc,"time_warn_day",time_warn_day_str,"This is your %d minute warning for the coming day.",sizeof(time_warn_day_str));
	add_xml_identifier(misc,"no_spell_to_show",no_spell_to_show_str,"No spell to show",sizeof(no_spell_to_show_str));
	add_xml_identifier(misc,"invalid_spell_string",invalid_spell_string_str,"Invalid spell string",sizeof(invalid_spell_string_str));
	add_xml_identifier(misc,"command_string_too_long",command_too_long_str,"Command string too long",sizeof(command_too_long_str));
	add_xml_identifier(misc,"item_list_learn_cat",item_list_learn_cat_str,"Note: storage categories need to be learnt by selecting each category.",sizeof(item_list_learn_cat_str));

	add_xml_identifier(loading_msg,"init_opengl",init_opengl_str,"Initializing OpenGL extensions",sizeof(init_opengl_str));
	add_xml_identifier(loading_msg,"init_random",init_random_str,"Generating random seed",sizeof(init_random_str));
	add_xml_identifier(loading_msg,"load_ignores",load_ignores_str,"Loading ignores",sizeof(load_ignores_str));
	add_xml_identifier(loading_msg,"load_filters",load_filters_str,"Loading filters",sizeof(load_filters_str));
	add_xml_identifier(loading_msg,"load_lists",load_lists_str,"Loading lists",sizeof(load_lists_str));
	add_xml_identifier(loading_msg,"load_cursors",load_cursors_str,"Loading cursors",sizeof(load_cursors_str));
	add_xml_identifier(loading_msg,"bld_glow",bld_glow_str,"Building glow table",sizeof(bld_glow_str));
	add_xml_identifier(loading_msg,"init_lists",init_lists_str,"Initializing lists",sizeof(init_lists_str));
	add_xml_identifier(loading_msg,"init_actor_defs",init_actor_defs_str,"Initializing actor definitions",sizeof(init_actor_defs_str));
	add_xml_identifier(loading_msg,"load_map_tiles",load_map_tiles_str,"Loading map tiles",sizeof(load_map_tiles_str));
	add_xml_identifier(loading_msg,"init_lights",init_lights_str,"Initializing lights",sizeof(init_lights_str));
	add_xml_identifier(loading_msg,"init_logs",init_logs_str,"Initializing logs",sizeof(init_logs_str));
	add_xml_identifier(loading_msg,"read_config",read_config_str,"Reading configuration",sizeof(read_config_str));
	add_xml_identifier(loading_msg,"init_weather",init_weather_str,"Initializing weather",sizeof(init_weather_str));
	add_xml_identifier(loading_msg,"init_audio",init_audio_str,"Initializing audio",sizeof(init_audio_str));
	add_xml_identifier(loading_msg,"load_icons",load_icons_str,"Loading icons",sizeof(load_icons_str));
	add_xml_identifier(loading_msg,"load_textures",load_textures_str,"Loading textures",sizeof(load_textures_str));
#ifdef PAWN
	add_xml_identifier (loading_msg, "init_pawn", init_pawn_str, "Initializing Pawn", sizeof(init_pawn_str));
#endif // PAWN
	add_xml_identifier(loading_msg,"init_network",init_network_str,"Initializing network",sizeof(init_network_str));
	add_xml_identifier(loading_msg,"init_timers",init_timers_str,"Initializing timers",sizeof(init_timers_str));
	add_xml_identifier(loading_msg,"load_encyc",load_encyc_str,"Loading Encyclopedia files",sizeof(load_encyc_str));
	add_xml_identifier(loading_msg,"init_display",init_display_str,"Initializing display stuff",sizeof(init_display_str));
	add_xml_identifier(loading_msg,"prep_op_win",prep_op_win_str,"Preparing opening window",sizeof(prep_op_win_str));
	add_xml_identifier(loading_msg,"load_map",load_map_str,"Loading map",sizeof(load_map_str));
	add_xml_identifier(loading_msg,"load_3d_object",load_3d_object_str,"Loading 3D objects",sizeof(load_3d_object_str));
	add_xml_identifier(loading_msg,"load_2d_object",load_2d_object_str,"Loading 2D objects",sizeof(load_2d_object_str));
	add_xml_identifier(loading_msg,"load_lights",load_lights_str,"Loading lights",sizeof(load_lights_str));
	add_xml_identifier(loading_msg,"load_particles",load_particles_str,"Loading particles",sizeof(load_particles_str));
	add_xml_identifier(loading_msg,"bld_sectors",bld_sectors_str,"Building sectors",sizeof(bld_sectors_str));
	add_xml_identifier(loading_msg,"init_done",init_done_str,"Done",sizeof(init_done_str));
	add_xml_identifier(loading_msg,"config_location",config_location_str,"Your personal settings and logs will be saved in %s",sizeof(config_location_str));
	add_xml_identifier(loading_msg,"datadir_location",datadir_location_str,"The location of the data files in use is %s",sizeof(datadir_location_str));

	add_xml_identifier(cmd_grp,"help_rq",help_request_str,"#help request",sizeof(help_request_str));
	add_xml_identifier(cmd_grp,"help_cmd",help_cmd_str,"help",sizeof(help_cmd_str));
	add_xml_identifier(cmd_grp,"char_cmd",char_cmd_str,"#",sizeof(char_cmd_str));
	add_xml_identifier(cmd_grp,"char_at",char_at_str,"@",sizeof(char_at_str));
	add_xml_identifier(cmd_grp,"char_slash",char_slash_str,"/",sizeof(char_slash_str));
	add_xml_identifier(cmd_grp,"gm_cmd",gm_cmd_str,"#gm",sizeof(gm_cmd_str));
	add_xml_identifier(cmd_grp,"mod_cmd",mod_cmd_str,"#mod",sizeof(mod_cmd_str));
	add_xml_identifier(cmd_grp,"bc_cmd",bc_cmd_str,"#bc",sizeof(bc_cmd_str));
	add_xml_identifier(cmd_grp,"msg_accept_buddy",msg_accept_buddy_str," wants to add you on his/her buddy list",sizeof(msg_accept_buddy_str));
	add_xml_identifier(cmd_grp,"filter",cmd_filter,"filter",sizeof(cmd_filter));
	add_xml_identifier(cmd_grp,"filters",cmd_filters,"filters",sizeof(cmd_filters));
	add_xml_identifier(cmd_grp,"unfilter",cmd_unfilter,"unfilter",sizeof(cmd_unfilter));
	add_xml_identifier(cmd_grp,"ignore",cmd_ignore,"ignore",sizeof(cmd_ignore));
	add_xml_identifier(cmd_grp,"ignores",cmd_ignores,"ignores",sizeof(cmd_ignores));
	add_xml_identifier(cmd_grp,"unignore",cmd_unignore,"unignore",sizeof(cmd_unignore));
	add_xml_identifier(cmd_grp,"markpos",cmd_markpos,"markpos",sizeof(cmd_markpos));
	add_xml_identifier(cmd_grp,"mark",cmd_mark,"mark",sizeof(cmd_mark));
	add_xml_identifier(cmd_grp,"unmark",cmd_unmark,"unmark",sizeof(cmd_unmark));
	add_xml_identifier(cmd_grp,"stats",cmd_stats,"stats",sizeof(cmd_stats));
	add_xml_identifier(cmd_grp,"time",cmd_time,"time",sizeof(cmd_time));
	add_xml_identifier(cmd_grp,"date",cmd_date,"date",sizeof(cmd_date));
	add_xml_identifier(cmd_grp,"exit",cmd_exit,"exit",sizeof(cmd_exit));
	add_xml_identifier(cmd_grp,"msg",cmd_msg,"msg",sizeof(cmd_msg));
	add_xml_identifier(cmd_grp,"afk",cmd_afk,"afk",sizeof(cmd_afk));
	add_xml_identifier(cmd_grp,"glinfo",cmd_glinfo,"glinfo",sizeof(cmd_glinfo));
	add_xml_identifier(cmd_grp,"knowledge",cmd_knowledge,"knowledge",sizeof(cmd_knowledge));
	add_xml_identifier(cmd_grp,"knowledge_short",cmd_knowledge_short,"know",sizeof(cmd_knowledge_short));
	add_xml_identifier(cmd_grp,"open_url",cmd_open_url,"open_url",sizeof(cmd_open_url));
	add_xml_identifier(cmd_grp,"keypress",cmd_keypress,"keypress",sizeof(cmd_keypress));
	add_xml_identifier(cmd_grp,"user_menu_wait_time_ms",cmd_user_menu_wait_time_ms,"user_menu_wait_time_ms",sizeof(cmd_user_menu_wait_time_ms));
	add_xml_identifier(cmd_grp,"show_spell",cmd_show_spell,"show_spell",sizeof(cmd_show_spell));
	add_xml_identifier(cmd_grp,"cast_spell",cmd_cast_spell,"cast_spell",sizeof(cmd_cast_spell));
	add_xml_identifier(cmd_grp,"session_counters",cmd_session_counters,"session_counters",sizeof(cmd_session_counters));
	add_xml_identifier(cmd_grp,"reload_icons",cmd_reload_icons,"reload_icons",sizeof(cmd_reload_icons));
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
	add_xml_identifier(load,"noteswrong",notes_wrong,"Document of the wrong type. It will be overwritten.",sizeof(notes_wrong));
	add_xml_identifier(load,"manynotes",too_many_notes,"Too many notes - Last nodes were ignored.",sizeof(too_many_notes));
	add_xml_identifier(load,"notenode",wrong_note_node,"Incorrect node type - could not copy.",sizeof(wrong_note_node));
	add_xml_identifier(load,"savenotes",cant_save_notes,"Unable to write notes to file %s",sizeof(cant_save_notes));
	add_xml_identifier(load,"exceednotes",exceed_note_buffer,"Tried to exceed notepad buffer! Ignored.",sizeof(exceed_note_buffer));
	add_xml_identifier(load,"nomorenotes",user_no_more_notes,"No room for more notes.",sizeof(user_no_more_notes));
	add_xml_identifier(load,"nomorenotetabs",user_no_more_note_tabs,"No room for more note tabs.",sizeof(user_no_more_note_tabs));
	add_xml_identifier(load,"fataldataerror",fatal_data_error,"Fatal error while loading data files. Either set the data_dir correctly or run from the data directory.",sizeof(fatal_data_error));
	add_xml_identifier(load,"encyclerror",cant_load_encycl,"Failed to load encyclopedia, check your installation.",sizeof(cant_load_encycl));


	//Miscellaneous errors
	add_xml_identifier(misc,"no_walk_sitlock",no_walk_with_sitlock,"Sitlock is enabled. Disable it or stand before walking.",sizeof(no_walk_with_sitlock));
	add_xml_identifier(misc,"error",reg_error_str,"Error",sizeof(reg_error_str));
	add_xml_identifier(load,"file_write_error",file_write_error_str,"Can't write to file",sizeof(file_write_error_str));
	add_xml_identifier(misc,"objerr",object_error_str,"Object error",sizeof(object_error_str));
	add_xml_identifier(misc,"nasty",nasty_error_str,"Something nasty happened while trying to process: %s",sizeof(nasty_error_str));
	add_xml_identifier(misc,"corrupt",corrupted_object,"Object seems to be corrupted. Skipping the object. Warning: This might cause further problems.",sizeof(corrupted_object));
	add_xml_identifier(misc,"badobj",bad_object,"Bad object",sizeof(bad_object));
	add_xml_identifier(misc,"multimat",multiple_material_same_texture,"Two or more materials with the same texture name!",sizeof(multiple_material_same_texture));
	add_xml_identifier(misc,"resync",resync_server,"Resync with the server...",sizeof(resync_server));
	add_xml_identifier(misc,"vertex",enabled_vertex_arrays,"Vertex Arrays enabled (memory hog on!)...",sizeof(enabled_vertex_arrays));
	add_xml_identifier(misc,"compiled",disabled_compiled_vertex_arrays,"Compiled Vertex Arrays disabled.",sizeof(disabled_compiled_vertex_arrays));
	add_xml_identifier(misc,"point",disabled_point_particles,"Point Particles disabled.",sizeof(disabled_point_particles));
	add_xml_identifier(misc,"particles",disabled_particles_str,"Particles completely disabled!",sizeof(disabled_particles_str));
	add_xml_identifier(misc,"net",failed_sdl_net_init,"Couldn't initialize net",sizeof(failed_sdl_net_init));
	add_xml_identifier(misc,"timer_fail",failed_sdl_timer_init,"Couldn't initialize the timer",sizeof(failed_sdl_timer_init));
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
	add_xml_identifier(misc,"timer_lag",timer_lagging_behind,"The %s timer was lagging severely behind or had stopped, restarted it", sizeof(timer_lagging_behind));
	add_xml_identifier(misc,"nameinuse",char_name_in_use,"Character name is already taken",sizeof(char_name_in_use));
	add_xml_identifier(misc,"notabs",must_use_tabs,"You cannot disable tabbed windows with video mode %d, forcing them",sizeof(must_use_tabs));
	add_xml_identifier (misc, "nomap", cant_change_map, "Unable to switch to map %s!", sizeof(cant_change_map));
	add_xml_identifier (misc, "emptymap", empty_map_str, "Using an empty map instead.", sizeof(empty_map_str));
	add_xml_identifier (misc, "nonomap", no_nomap_str, "Fatal error: Couldn't load map ./maps/nomap.elm.\nFix your maps.", sizeof(no_nomap_str));
	add_xml_identifier (misc, "nobmpmap", err_nomap_str, "There is no map for this place.", sizeof(err_nomap_str));
	add_xml_identifier (misc, "mapmarks", err_mapmarks_str, "Maximum number of mapmarks reached.", sizeof(err_mapmarks_str));
	add_xml_identifier (misc, "book_open", book_open_err_str, "Couldn't open the book: %s!", sizeof(book_open_err_str));
	add_xml_identifier (misc, "noanimation", no_animation_err_str, "No animation: %s!\n", sizeof(no_animation_err_str));
	add_xml_identifier (misc, "invalid_location", invalid_location_str, "Invalid location %d,%d", sizeof(invalid_location_str));
	add_xml_identifier (misc, "warn_currently_ignoring", warn_currently_ignoring, "Warning: %s is on your #ignore list", sizeof(warn_currently_ignoring));

	//XML errors. should these have their own group?
	add_xml_identifier (misc, "badnode", xml_bad_node, "There is something wrong with one of a node's fields.", sizeof(xml_bad_node));
	add_xml_identifier (misc, "badroot", xml_bad_root_node, "The root node in %s was incorrect.", sizeof(xml_bad_root_node));
	add_xml_identifier (misc, "undefnode", xml_undefined_node, "Found an unexpected node type while parsing %s (%s).", sizeof(xml_undefined_node));
	add_xml_identifier (misc, "use_builtin_chans", using_builtin_chanlist, "Could not load a channel list from file. Using a limited built-in set instead.", sizeof(using_builtin_chanlist));
	add_xml_identifier (misc, "use_eng_chans", using_eng_chanlist, "Could not load a channel list for language code %s. Using the english set instead.", sizeof(using_eng_chanlist));

	// Mines errors
	add_xml_identifier (misc, "mines_config_open", mines_config_open_err_str, "Error opening mines configuration file", sizeof(mines_config_open_err_str));
	add_xml_identifier (misc, "mines_config", mines_config_error, "Error loading mines configuration", sizeof(mines_config_error));
	
	// Misc
#ifdef PNG_SCREENSHOT
	add_xml_identifier (misc, "max_screenshots_warning", max_screenshots_warning_str, "You have reached the maximum capacity for screenshots. Please move them all to another folder, otherwise this image will be overwritten next time.", sizeof(max_screenshots_warning_str));
#endif //PNG_SCREENSHOT

	// item lists
	add_xml_identifier (misc, "item_list_format_error", item_list_format_error, "Format error while reading item list.", sizeof(item_list_format_error));
	add_xml_identifier (misc, "item_list_save_error", item_list_save_error_str, "Failed to save the item category file.", sizeof(item_list_save_error_str));
	add_xml_identifier (misc, "item_list_cat_format_error", item_list_cat_format_error_str, "Format error reading item categories.", sizeof(item_list_cat_format_error_str));
	add_xml_identifier (misc, "item_list_version_error", item_list_version_error_str, "Item lists file is not compatible with client version.", sizeof(item_list_version_error_str));
	add_xml_identifier (misc, "item_list_empty_list", item_list_empty_list_str, "No point saving an empty list.", sizeof(item_list_empty_list_str));

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
	add_xml_identifier(snd,"source",snd_source_error,"Error creating sources. Sound is disabled",sizeof(snd_source_error));
	add_xml_identifier(snd,"skip",snd_skip_speedup,"Skip! Speeding up...",sizeof(snd_skip_speedup));
	add_xml_identifier(snd,"tooslow",snd_too_slow,"Sorry, too slow to play music or backgrounds...",sizeof(snd_too_slow));
	add_xml_identifier(snd,"fail",snd_stop_fail,"Failed to stop all sounds.",sizeof(snd_stop_fail));
	add_xml_identifier(snd,"snd_init",snd_init_error,"Error initializing sound",sizeof(snd_init_error));
	add_xml_identifier(snd,"sndconfigopen",snd_config_open_err_str,"Error opening sound configuration file",sizeof(snd_config_open_err_str));
	add_xml_identifier(snd,"sndconfig",snd_config_error,"Error loading sound configuration",sizeof(snd_config_error));
	add_xml_identifier(snd,"toomany",snd_sound_overflow,"Too many sounds.",sizeof(snd_sound_overflow));
	add_xml_identifier(snd,"read",snd_media_read,"Read from media.",sizeof(snd_media_read));
	add_xml_identifier(snd,"notvorbis",snd_media_notvorbis,"Not Vorbis data.",sizeof(snd_media_notvorbis));
	add_xml_identifier(snd,"version",snd_media_ver_mismatch,"Vorbis version mismatch.",sizeof(snd_media_ver_mismatch));
	add_xml_identifier(snd,"header",snd_media_invalid_header,"Invalid Vorbis header.",sizeof(snd_media_invalid_header));
	add_xml_identifier(snd,"intern",snd_media_internal_error,"Internal logic fault (bug or heap/stack corruption.",sizeof(snd_media_internal_error));
	add_xml_identifier(snd,"unknown",snd_media_ogg_error,"Unknown Ogg error.",sizeof(snd_media_ogg_error));
	add_xml_identifier(snd,"false",snd_media_false,"Ogg error media false.",sizeof(snd_media_false));
	add_xml_identifier(snd,"hole",snd_media_hole,"Ogg error media hole.",sizeof(snd_media_hole));
	add_xml_identifier(snd,"einval",snd_media_einval,"Ogg error media EINVAL.",sizeof(snd_media_einval));
	add_xml_identifier(snd,"eof",snd_media_eof,"Ogg error media EOF.",sizeof(snd_media_eof));
	add_xml_identifier(snd,"ebadlink",snd_media_ebadlink,"Ogg error media EBADLINK.",sizeof(snd_media_ebadlink));
	add_xml_identifier(snd,"enoseek",snd_media_enoseek,"Ogg error media ENOSEEK.",sizeof(snd_media_enoseek));
	add_xml_identifier(snd,"enomusic",snd_no_music,"This client was built without music support",sizeof(snd_no_music));
	add_xml_identifier(snd,"musicstopped",snd_media_music_stopped,"No song is currently playing",sizeof(snd_media_music_stopped));
	add_xml_identifier(snd,"musicinfo",snd_media_ogg_info,"Currently playing: \"%s\" by %s (%d:%02d/%d:%02d)",sizeof(snd_media_ogg_info));
	add_xml_identifier(snd,"musicinfonoartist",snd_media_ogg_info_noartist,"Currently playing: \"%s\" (%d:%02d/%d:%02d)",sizeof(snd_media_ogg_info_noartist));

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
	add_xml_identifier(video,"toobigshadowmap",shadow_map_size_not_supported_str,"Shadow map size not supported! Shadow map size reduced to %d!",sizeof(shadow_map_size_not_supported_str));
	add_xml_identifier(video,"noframebuffer",disabled_framebuffer,"Framebuffer disabled (need newer driver)",sizeof(disabled_framebuffer));
	//Framebuffer errors
	add_xml_identifier(video,"fboattachmenterror",fbo_attachment_error,"Framebuffer: attachment error",sizeof(fbo_attachment_error));
	add_xml_identifier(video,"fbomissingattachmenterror",fbo_missing_attachment_error,"Framebuffer: missing attachment",sizeof(fbo_missing_attachment_error));
	add_xml_identifier(video,"fboformatserror",fbo_formats_error,"Framebuffer: formats error",sizeof(fbo_formats_error));
	add_xml_identifier(video,"fbodrawbuffererror",fbo_draw_buffer_error,"Framebuffer: draw buffer error",sizeof(fbo_draw_buffer_error));
	add_xml_identifier(video,"fboreadbuffererror",fbo_read_buffer_error,"Framebuffer: read buffer error",sizeof(fbo_read_buffer_error));
	add_xml_identifier(video,"fbounsupportedfromaterror",fbo_unsupported_fromat_error,"Framebuffer: unsupported format error",sizeof(fbo_unsupported_fromat_error));
	add_xml_identifier(video,"fbounknownerror",fbo_unknown_error,"Framebuffer: unkown error %d",sizeof(fbo_unknown_error));
	add_xml_identifier(video,"fbosupportedfromat",fbo_supported_format,"Frame buffer format: %s, depth bits: %d, stencil bits: %d is supported",sizeof(fbo_supported_format));
	add_xml_identifier(video,"extnotfoundemulit",gl_ext_not_found_emul_it,"Couldn't find the %s extension, emulating it...",sizeof(gl_ext_not_found_emul_it));
	add_xml_identifier(video,"invalid",invalid_video_mode,"Stop playing with the configuration file and select valid modes!",sizeof(invalid_video_mode));

	//Rule errors
	add_xml_identifier(rules,"proceed",you_can_proceed,"Read the rules and you can play in %dsec",sizeof(you_can_proceed));
	add_xml_identifier(rules,"ready",accepted_rules,"Read the rules and click on \"I Accept\" to play!",sizeof(accepted_rules));
	add_xml_identifier(rules,"accept",accept_label,"I Accept",sizeof(accept_label));
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
	group_id * buddy = &(help_str[4]);

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
	add_xml_identifier(misc,"dialog_copy",dialogue_copy_str,"[copy]",sizeof(dialogue_copy_str));
	add_xml_identifier(misc,"dialogue_repeat",dialogue_repeat_str,"[repeat]",sizeof(dialogue_repeat_str));
	add_xml_identifier(misc,"open_storage",open_storage_str,"Open storage",sizeof(open_storage_str));
	add_xml_identifier(misc,"reopen_storage",reopen_storage_str,"Reopen for setting to take effect",sizeof(reopen_storage_str));
	add_xml_identifier(misc,"low",low_framerate_str,"Low framerate detected, shadows and eye candy disabled!",sizeof(low_framerate_str));
	add_xml_identifier(misc,"size",window_size_adjusted_str,"Window size adjusted to %s",sizeof(window_size_adjusted_str));
	add_xml_identifier(misc,"trade",no_open_on_trade,"You can't open this window while on trade.",sizeof(no_open_on_trade));
	add_xml_identifier(misc,"user",login_username_str,"Username:",sizeof(login_username_str));
	add_xml_identifier(misc,"pass",login_password_str,"Password:",sizeof(login_password_str));
	add_xml_identifier(misc,"login_rules",login_rules_str,"If you log into this game, you accept the rules of Eternal Lands. Press F5 to read them in game.",sizeof(login_rules_str));
	add_xml_identifier(misc,"stoall",sto_all_str,"Sto All",sizeof(sto_all_str));
	add_xml_identifier(misc,"getall",get_all_str,"Get All",sizeof(get_all_str));
	add_xml_identifier(misc,"drpall",drp_all_str,"Drp All",sizeof(drp_all_str));
	add_xml_identifier(misc,"itmlst",itm_lst_str,"Itm Lst",sizeof(itm_lst_str));
	add_xml_identifier(misc,"mixone",mix_one_str,"Mix One",sizeof(mix_one_str));
	add_xml_identifier(misc,"mixall",mix_all_str,"Mix All",sizeof(mix_all_str));
	add_xml_identifier(misc,"autogetall",auto_get_all_str,"Empty Bag Automatically",sizeof(auto_get_all_str));
	add_xml_identifier(misc,"itemlistbut",item_list_but_str,"Open Left Of Inventory",sizeof(item_list_but_str));
	add_xml_identifier(misc,"inv_keeprow",inv_keeprow_str,"Keep First Row\nKeep Last Row",sizeof(inv_keeprow_str));
	add_xml_identifier(misc,"completed",completed_research,"COMPLETED",sizeof(completed_research));
	add_xml_identifier(misc,"lessthanaminute",lessthanaminute_str,"Less than a minute",sizeof(lessthanaminute_str));
	add_xml_identifier(misc,"research",researching_str,"Researching",sizeof(researching_str));
	add_xml_identifier(misc,"nothing",not_researching_anything,"Nothing",sizeof(not_researching_anything));
	add_xml_identifier(misc,"not_researching",not_researching_str,"Researching nothing",sizeof(not_researching_anything));
	add_xml_identifier(misc,"countdown",countdown_str,"Countdown",sizeof(countdown_str));
	add_xml_identifier(misc,"stopwatch",stopwatch_str,"Stopwatch",sizeof(stopwatch_str));
	add_xml_identifier(misc,"minutes",minutes_str,"minutes",sizeof(minutes_str));
	add_xml_identifier(misc,"minute",minute_str,"minute",sizeof(minute_str));
	add_xml_identifier(misc,"idle",idle_str,"Idle",sizeof(idle_str));
	add_xml_identifier(misc,"read_book",knowledge_read_book,"Read Book",sizeof(knowledge_read_book));
	add_xml_identifier(misc,"kp_read",knowledge_param_read,"-read",sizeof(knowledge_param_read));
	add_xml_identifier(misc,"kp_unread",knowledge_param_unread,"-unread",sizeof(knowledge_param_unread));
	add_xml_identifier(misc,"kp_total",knowledge_param_total,"-total",sizeof(knowledge_param_total));
	add_xml_identifier(misc,"unknown_book_s",unknown_book_short_str,"(Not yet known to client)",sizeof(unknown_book_short_str));
	add_xml_identifier(misc,"unknown_book_l",unknown_book_long_str,"Researching book not yet known to the client.  Don't worry, reading it will count!",sizeof(unknown_book_long_str));
	add_xml_identifier(misc,"know_highlight_prompt",know_highlight_prompt_str,"Highlight Text",sizeof(know_highlight_prompt_str));
	add_xml_identifier(misc,"know_highlight_cm",know_highlight_cm_str,"Set Highlight Text\nClear Highlight\nCopy Name",sizeof(know_highlight_cm_str));
	add_xml_identifier(misc,"mix",mix_str,"Mix",sizeof(mix_str));
	add_xml_identifier(misc,"mix_all",mixall_str,"Mix all",sizeof(mixall_str));
	add_xml_identifier(misc,"clear",clear_str,"Clear",sizeof(clear_str));
	add_xml_identifier(misc,"manu_add",manu_add_str,"Left-click or scrollwheel to add 1; or 10 with CTRL",sizeof(manu_add_str));
	add_xml_identifier(misc,"manu_remove",manu_remove_str,"Left-click or scrollwheel to remove 1; or 10 with CTRL",sizeof(manu_remove_str));
	add_xml_identifier(misc,"cast",cast_str,"Cast",sizeof(cast_str));
	add_xml_identifier (misc, "invalid_spell", invalid_spell_str, "Invalid spell", sizeof (invalid_spell_str));
	add_xml_identifier(misc,"connect",connect_to_server_str,"Connecting to Server...",sizeof(connect_to_server_str));
	add_xml_identifier(misc,"reconnect",reconnect_str,"Press any key to try again.",sizeof(reconnect_str));
	add_xml_identifier (misc, "x_quit", alt_x_quit, "Press Alt-x to close the game", sizeof (alt_x_quit));
	add_xml_identifier(misc,"license",license_check,"Entropy says: U R 2 g00d 2 r34d +h3 license.txt?\nBTW, that license.txt file is actually there for a reason.",sizeof(license_check));
	add_xml_identifier(misc,"session_reset_help",session_reset_help,"Double-click to reset session information",sizeof(session_reset_help));
	add_xml_identifier(misc,"quantity",quantity_str,"Quantity",sizeof(quantity_str));
	add_xml_identifier(misc,"abort",abort_str,"Abort",sizeof(abort_str));
	add_xml_identifier(misc,"sigils",sig_too_few_sigs,"This spell requires at least 2 sigils",sizeof(sig_too_few_sigs));
	add_xml_identifier(misc,"switch",switch_video_mode,"Switches to video mode %s",sizeof(switch_video_mode));
	add_xml_identifier(misc,"cachei",cache_items_str,"items",sizeof(cache_items_str));
	add_xml_identifier(misc,"caches",cache_size_str,"Cache size",sizeof(cache_size_str));
	add_xml_identifier (misc, "appropr_name", use_appropriate_name, "Use an appropriate name:\nPlease do not create a name that is obscene or offensive, contains more than 2 digits, is senseless or stupid (i.e. djrtq47fa), or is made with the intent of impersonating another player.\nTake into consideration that the name you choose does affect the atmosphere of the game. Inappropriate names can and will be locked.", sizeof (use_appropriate_name) );
	add_xml_identifier(misc,"edit_quantity",quantity_edit_str,"Rightclick on the quantity you wish to edit",sizeof(quantity_edit_str));
	add_xml_identifier(misc,"equip_here",equip_here_str,"Place an item in these boxes to equip it",sizeof(equip_here_str));
	add_xml_identifier(misc,"pick_item_help",pick_item_help_str,"Pickup item. +ctrl/+alt to drop/store all",sizeof(pick_item_help_str));
	add_xml_identifier(misc,"multiuse_item_help",multiuse_item_help_str,"Leftclick to use (+shift to use again)",sizeof(multiuse_item_help_str));
	add_xml_identifier(misc,"equipment",equip_str,"Equipment",sizeof(equip_str));
	add_xml_identifier(misc,"stoall_help",stoall_help_str,"Move all items into opened storage.",sizeof(stoall_help_str));
	add_xml_identifier(misc,"getall_help",getall_help_str,"Get all items from ground bag.",sizeof(getall_help_str));
	add_xml_identifier(misc,"dcdrpall_help",dcdrpall_help_str,"Double-click to drop all items.",sizeof(dcdrpall_help_str));
	add_xml_identifier(misc,"drpall_help",drpall_help_str,"Drop all items.",sizeof(drpall_help_str));
	add_xml_identifier(misc,"mixoneall_help",mixoneall_help_str,"Mix current manufacture recipe.",sizeof(mixoneall_help_str));
	add_xml_identifier(misc,"itmlst_help",itmlst_help_str,"Show/hide item lists window.",sizeof(itmlst_help_str));
	add_xml_identifier(misc,"items_stack",items_stack_str,"Client can't choose between multiple stacks, make a free slot and let the server do it!",sizeof(items_stack_str));
	add_xml_identifier(misc,"mixbut_empty",mixbut_empty_str,"Nothing to mix, add some items using the manufacture window.",sizeof(mixbut_empty_str));
	add_xml_identifier(misc,"mix_empty_str",mix_empty_str,"Nothing to mix, add some items.",sizeof(mix_empty_str));
	add_xml_identifier(misc,"click_clear",click_clear_str,"Click to clear message.",sizeof(click_clear_str));
	add_xml_identifier(misc,"double_click_clear",double_click_clear_str,"Double-click to clear message.",sizeof(double_click_clear_str));
	add_xml_identifier(misc,"recipe_select",recipe_select_str,"Left-click or scroll to select recipe slot.",sizeof(recipe_select_str));
	add_xml_identifier(misc,"recipe_load",recipe_load_str,"Double-click to load recipe.",sizeof(recipe_load_str));
	add_xml_identifier(misc,"recipe_find",recipe_find_str,"Type text - find recipe.",sizeof(recipe_find_str));
	add_xml_identifier(misc,"recipe_during_find",recipe_during_find_str,"Next match - return, +ctrl to load.",sizeof(recipe_during_find_str));
	add_xml_identifier(misc,"recipe_show_hide",recipe_show_hide_str,"Click to show/hide saved recipes. Wheel to scroll.",sizeof(recipe_show_hide_str));
	add_xml_identifier(misc,"recipe_save",recipe_save_str,"Click to save current recipe to selected slot.",sizeof(recipe_save_str));
	add_xml_identifier(misc,"you",you_str,"You",sizeof(you_str));
	add_xml_identifier(misc,"accept",accept_str,"Accept",sizeof(accept_str));
	add_xml_identifier(misc,"cmd_markpos",help_cmd_markpos_str,"Usage: #markpos <x-coord>,<y-coord> <name>",sizeof(help_cmd_markpos_str));
	add_xml_identifier(misc,"location_info",location_info_str,"Location %d,%d marked with %s",sizeof(location_info_str));
	add_xml_identifier(misc,"knowledge_command",knowledge_cmd_str,"List of matching knowledge:",sizeof(knowledge_cmd_str));
	add_xml_identifier(misc,"marked",marked_str,"%s marked",sizeof(marked_str));
	add_xml_identifier(misc,"unmarked",unmarked_str,"%s unmarked",sizeof(unmarked_str));
	add_xml_identifier(misc,"no_urls",urlcmd_none_str,"No URL seen",sizeof(urlcmd_none_str));
	add_xml_identifier(misc,"url_list",urlcmd_list_str,"URL list:",sizeof(urlcmd_list_str));
	add_xml_identifier(misc,"win_url",win_url_str,"URL list",sizeof(win_url_str));
	add_xml_identifier(misc,"invalid_url",urlcmd_invalid_str,"Invalid URL number",sizeof(urlcmd_invalid_str));
	add_xml_identifier(misc,"afk_url",urlcmd_afk_str,"URL seen while AFK:",sizeof(urlcmd_afk_str));
	add_xml_identifier(misc,"clear_url",urlcmd_clear_str,"clear",sizeof(urlcmd_clear_str));
	add_xml_identifier(misc,"open_urlwin",urlwin_open_str,"Click to open; right+click for options",sizeof(urlwin_open_str));
	add_xml_identifier(misc,"clear_urlwin",urlwin_clear_str,"Clear the URL list",sizeof(urlwin_clear_str));
	add_xml_identifier(misc,"reset",reset_str,"Reset",sizeof(reset_str));
	add_xml_identifier(misc,"channel_help",channel_help_str,"Click a Channel to join. You can be in up to 3 channels at a time.\n\nTo talk in a channel, type @ before your message. You do not have to type @ to talk in Local.",sizeof(channel_help_str));
	add_xml_identifier(misc,"channel_color_title",channel_color_title_str,"Channel Colors",sizeof(channel_color_title_str));
	add_xml_identifier(misc,"channel_color",channel_color_str,"Set/delete the color for channel",sizeof(channel_color_str));
	add_xml_identifier(misc,"channel_color_add",channel_color_add_str,"Set",sizeof(channel_color_add_str));
	add_xml_identifier(misc,"channel_color_delete",channel_color_delete_str,"Delete",sizeof(channel_color_delete_str));
	add_xml_identifier(misc,"stats_scroll_help",stats_scroll_help_str,"Scroll Up/Down using CTRL+left/CTRL+right click or scrollwheel.",sizeof(stats_scroll_help_str));
	add_xml_identifier(misc,"remove_bar_message",remove_bar_message_str,"Removed exp bar as space is limited.",sizeof(remove_bar_message_str));
	add_xml_identifier(misc,"cm_action_points",cm_action_points_str,"Show Action Points Bar",sizeof(cm_action_points_str));
	add_xml_identifier(misc,"dc_note_rm",dc_note_remove,"Double-click to remove this category",sizeof(dc_note_remove));
	add_xml_identifier(misc,"note_saved",note_saved,"Your notes have been saved",sizeof(note_saved));
	add_xml_identifier(misc,"note_save_failed",note_save_failed,"Failed to save your notes!",sizeof(note_save_failed));
	add_xml_identifier(misc,"ranginglock_enabled",ranginglock_enabled_str,"Ranging-Lock is now enabled. Disable it or unequip ranging weapon before walking.",sizeof(ranginglock_enabled_str));
	add_xml_identifier(misc,"ranginglock_disabled",ranginglock_disabled_str,"Ranging-Lock is now disabled.",sizeof(ranginglock_disabled_str));
	add_xml_identifier(misc,"video_restart", video_restart_str, "Video change will take effect at next restart.", sizeof(video_restart_str));
	add_xml_identifier(misc,"rotate_chat_log_restart", rotate_chat_log_restart_str, "Rotate chat log change will take effect at next restart.", sizeof(rotate_chat_log_restart_str));
	add_xml_identifier(misc,"ranging_win_title", ranging_win_title_str, "Ranging", sizeof(ranging_win_title_str));
	add_xml_identifier(misc,"ranging_total_shots", ranging_total_shots_str, "Total shots      %d", sizeof(ranging_total_shots_str));
	add_xml_identifier(misc,"ranging_sucessful_shots", ranging_sucessful_shots_str, "Successful hits  %d", sizeof(ranging_sucessful_shots_str));
	add_xml_identifier(misc,"ranging_missed_shots", ranging_missed_shots_str, "Missed hits      %d", sizeof(ranging_missed_shots_str));
	add_xml_identifier(misc,"ranging_success_rate", ranging_success_rate_str, "Success rate     %.2f %%", sizeof(ranging_success_rate_str));
	add_xml_identifier(misc,"ranging_critical_rate", ranging_critical_rate_str, "Critical rate    %.2f %%", sizeof(ranging_critical_rate_str));
	add_xml_identifier(misc,"ranging_exp_per_arrow", ranging_exp_per_arrow_str, "Exp/arrows       %.2f exp", sizeof(ranging_exp_per_arrow_str));

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
	add_xml_identifier(new,"passwordbad",error_bad_pass,"Bad password!",sizeof(error_bad_pass));
	add_xml_identifier(new,"passmatch",passwords_match,"Passwords are matching!",sizeof(passwords_match));
	add_xml_identifier(new,"appearance",remember_change_appearance,"Remember to change your characters appearance before pressing \"Done\"",sizeof(remember_change_appearance));
	add_xml_identifier(new,"appearance_box",appearance_str,"Appearance",sizeof(appearance_str));
	add_xml_identifier(new,"max_digits",error_max_digits,"You can only have 2 digits in your name!",sizeof(error_max_digits));
	add_xml_identifier(new,"max_length",error_length,"Names and passwords can max be 15 characters long",sizeof(error_length));
	add_xml_identifier(new,"illegal_char",error_illegal_character,"You have typed an illegal character!",sizeof(error_illegal_character));
	add_xml_identifier(new,"p2p_race",p2p_race,"You have to pay to create a char with this race",sizeof(p2p_race));
	add_xml_identifier(new,"char_help",char_help,"To customize your character and select name/password, press the buttons at the bottom.",sizeof(char_help));
	add_xml_identifier(new,"confirmcreate",error_confirm_create_char,"Click done again to create a character with that name and appearance.",sizeof(error_confirm_create_char));
	add_xml_identifier(new,"newcharwarning",newchar_warning,"Character creation screen",sizeof(newchar_warning));
	add_xml_identifier(new,"newcharcusthelp",newchar_cust_help,"Click the eye icon below to customize your character.",sizeof(newchar_cust_help)); // it pains me to spell customize with a z:(
#ifndef NEW_NEW_CHAR_WINDOW
	add_xml_identifier(new,"newcharcredhelp",newchar_cred_help,"Click the person icon below to choose your character name and password.",sizeof(newchar_cred_help));
#else
	add_xml_identifier(new,"newcharcredhelp",newchar_cred_help,"When ready, click \"Done\" to choose your character name and password.",sizeof(newchar_cred_help));
#endif
	add_xml_identifier(new,"newchardonehelp",newchar_done_help,"When ready, click \"Done\" to create your character and enter the game.",sizeof(newchar_done_help));
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
	add_xml_identifier(tooltips,"customize",tt_customize,"Customize your character",sizeof(tt_customize));
	add_xml_identifier(tooltips,"name_pass",tt_name,"Choose name and password",sizeof(tt_name));
	add_xml_identifier (tooltips, "info", tt_info, "View notepad/URL window", sizeof (tt_info));
	add_xml_identifier (tooltips, "emotewin", tt_emotewin, "View Emote window", sizeof (tt_emotewin));
	add_xml_identifier (tooltips, "range", tt_rangewin, "View Ranging window", sizeof (tt_rangewin));
	add_xml_identifier (tooltips, "minimap", tt_minimap, "View Minimap window", sizeof (tt_minimap));

	//Buddy list
	add_xml_identifier(buddy, "name", buddy_name_str, "Name:", sizeof(buddy_name_str));
	add_xml_identifier(buddy, "name_desc", buddy_long_name_str, "The name of your buddy", sizeof(buddy_long_name_str));
	add_xml_identifier(buddy, "color", buddy_type_str, "Color:", sizeof(buddy_type_str));
	add_xml_identifier(buddy, "color_desc", buddy_long_type_str, "The color you want your buddy to appear in in the list", sizeof(buddy_long_type_str));
	add_xml_identifier(buddy, "add", buddy_add_str, "Add buddy", sizeof(buddy_add_str));
	add_xml_identifier(buddy, "change", buddy_change_str, "Change buddy", sizeof(buddy_change_str));
	add_xml_identifier(buddy, "accept", buddy_accept_str, "Accept buddy", sizeof(buddy_accept_str));
	add_xml_identifier(buddy, "yes", yes_str, "Yes", sizeof(yes_str));
	add_xml_identifier(buddy, "no", no_str, "No", sizeof(yes_str));
	add_xml_identifier(buddy, "delete", buddy_delete_str, "Delete buddy", sizeof(buddy_delete_str));
	add_xml_identifier(buddy, "delete_desc", buddy_long_delete_str, "Check this to delete the buddy from the list", sizeof(buddy_long_delete_str));
	add_xml_identifier(buddy, "request_dialog", buddy_wants_to_add_str, "%s wants to add you to his/her buddy list. Do you wish to allow it?", sizeof(buddy_wants_to_add_str));
	add_xml_identifier(buddy, "add_to_list", buddy_add_to_list_str, "Add to my buddy list", sizeof(buddy_add_to_list_str));
	add_xml_identifier(buddy, "logon", buddy_logon_str, "%.*s has logged on.", sizeof(buddy_logon_str));
	add_xml_identifier(buddy, "online", buddy_online_str, "%.*s is online.", sizeof(buddy_online_str));
	add_xml_identifier(buddy, "logoff", buddy_logoff_str, "%.*s has logged off.", sizeof(buddy_logoff_str));
	add_xml_identifier(buddy, "white", buddy_white_str, "White", sizeof(buddy_white_str));
	add_xml_identifier(buddy, "red", buddy_red_str, "Red", sizeof(buddy_red_str));
	add_xml_identifier(buddy, "green", buddy_green_str, "Green", sizeof(buddy_green_str));
	add_xml_identifier(buddy, "blue", buddy_blue_str, "Blue", sizeof(buddy_blue_str));
	add_xml_identifier(buddy, "yellow", buddy_yellow_str, "Yellow", sizeof(buddy_yellow_str));
	add_xml_identifier(buddy, "request", buddy_request_str, "Requests", sizeof(buddy_request_str));

	// Update window
	add_xml_identifier(misc, "update_complete", update_complete_str, "The client has been updated", sizeof(update_complete_str));
	add_xml_identifier(misc, "restart_countdown", client_restart_countdown_str, "Client will restart in %d seconds", sizeof(client_restart_countdown_str));
	add_xml_identifier(misc, "restarting", client_restarting_str, "Restarting...", sizeof(client_restarting_str));
	add_xml_identifier(misc, "restart", restart_now_label, "Restart now", sizeof(restart_now_label));
	
	/* strings for context menus */
	add_xml_identifier(misc, "cm_quickspell_menu", cm_quickspell_menu_str, "Move Up\nMove Down\nRemove\n", sizeof(cm_quickspell_menu_str));
	add_xml_identifier(misc, "cm_textedit_menu", cm_textedit_menu_str, "Cut\nCopy\nPaste\n--\nDate\nTime\nCoords", sizeof(cm_textedit_menu_str));
	add_xml_identifier(misc, "cm_quickbar_menu", cm_quickbar_menu_str, "Quickbar Relocatable\nQuickbar Draggable\nReset Quickbar Position\nFlip Quickbar\nEnable Quickbar Menu\n", sizeof(cm_quickbar_menu_str));
	add_xml_identifier(misc, "cm_hud_menu", cm_hud_menu_str, "Show Stats\nShow Stats Bars\nShow Knowledge Bar\nShow Timer\nShow Digital Clock\nShow Analogue Clock\nShow Seconds\nShow FPS\nEnable Quickbar Menu\n--\nShow Minimap\nShow Ranging Stats\n--\nEnable Sound Effects\nEnable Music\n--\nCopy Location", sizeof(cm_hud_menu_str));
	add_xml_identifier(misc, "cm_banner_menu", cm_banner_menu_str, "Show Names\nShow Health Bars\nShow Health Numbers\nShow Ether Bar\nShow Ether Numbers\nEnable Instance Mode\nShow Speech Bubbles\nEnable Banner Background\nSit Lock\nRanging Lock\n--\nDisable This Menu\n", sizeof(cm_banner_menu_str));
	add_xml_identifier(misc, "cm_title_menu", cm_title_menu_str, "Hide Windows\nOpaque Background\nWindows On Top\n", sizeof(cm_title_menu_str));
	add_xml_identifier(misc, "cm_title_help", cm_title_help_str, "Right-click for window menu", sizeof(cm_title_help_str));
	add_xml_identifier(misc, "cm_items_menu", cm_items_menu_str, "--\nUse Small Window\nManual Window Size\nItem Window On Drop\nAllow Equipment Swap\n--\nOpen Storage (View Only)", sizeof(cm_items_menu_str));
	add_xml_identifier(misc, "cm_storage_menu", cm_storage_menu_str, "--\nPrint Items To Console\nSort Categories Alphabetically\n", sizeof(cm_storage_menu_str));
	add_xml_identifier(misc, "cm_astro_menu", cm_astro_menu_str, "--\nPrint Details To Console\nAlways Print Details To Console", sizeof(cm_astro_menu_str));
	add_xml_identifier(misc, "cm_ranging_menu", cm_ranging_menu_str, "--\nPrint To Console", sizeof(cm_ranging_menu_str));
	add_xml_identifier(misc, "cm_dialog_options", cm_dialog_options_str, "Auto close storage dialogue\nAuto select storage option in dialogue", sizeof(cm_dialog_options_str));
	add_xml_identifier(misc, "cm_dialog_menu", cm_dialog_menu_str, "--\nEnable Keypresses\nKeypresses Anywhere", sizeof(cm_dialog_menu_str));
	add_xml_identifier(misc, "cm_url_menu", cm_url_menu_str, "Open\nFind In Console\nMark Visited\nMark Unvisited\n--\nDelete\n--\nDelete All", sizeof(cm_url_menu_str));	
	add_xml_identifier(misc, "cm_counters_menu", cm_counters_menu_str, "Delete Entry\n--\nReset Session Total\n--\nEnable Floating Messages For Category\n--\nPrint Category\nPrint All Categories\nPrint Just Session Information", sizeof(cm_counters_menu_str));
	add_xml_identifier(misc, "cm_help_options", cm_help_options_str, "Right-click for options.", sizeof(cm_help_options_str));
	add_xml_identifier(misc, "cm_npcname_menu", cm_npcname_menu_str, "Copy NPC Name\nSet Map Mark", sizeof(cm_npcname_menu_str));
	add_xml_identifier(misc, "cm_dialog_copy_menu", cm_dialog_copy_menu_str, "Exclude Responses\nRemove Newlines", sizeof(cm_dialog_copy_menu_str));
	add_xml_identifier(misc, "cm_minimap_menu", cm_minimap_menu_str, "--\nRotate Minimap\nPin Minimap\nOpen On Start", sizeof(cm_minimap_menu_str));
	add_xml_identifier(misc, "cm_user_menu", cm_user_menu_str, "--\nShow Title\nDraw Border\nSmall Font\nStandard Menus\n--\nShow Commands\n--\nReload Menus\nDisable Menus", sizeof(cm_user_menu_str));
	add_xml_identifier(misc, "cm_item_list_selected", cm_item_list_selected_str, "Edit quantity\n--\nDelete", sizeof(cm_item_list_selected_str));
	add_xml_identifier(misc, "cm_item_list_names", cm_item_list_names_str, "Create new list\nRename active list\n--\nDelete active list\n--\nReload from file", sizeof(cm_item_list_names_str));
	add_xml_identifier(misc, "cm_stats_bar_base", cm_stats_bar_base_str, "--\nAdd Bar\nRemove Bar", sizeof(cm_stats_bar_base_str));
	add_xml_identifier(misc, "cm_recipe_menu", cm_recipe_menu_str, "Add additional recipe row\nClear selected recipe\nSort recipes by name", sizeof(cm_recipe_menu_str));
	add_xml_identifier(misc, "cm_manuwin_menu", cm_manuwin_menu_str, "\n--\nDisable key presses for window", sizeof(cm_manuwin_menu_str));
	
	/* user_menus.cpp */
	add_xml_identifier(misc, "um_invalid_command", um_invalid_command_str, "Invalid command text", sizeof(um_invalid_command_str));
	add_xml_identifier(misc, "um_invalid_line", um_invalid_line_str, "<Error: invalid line>", sizeof(um_invalid_line_str));
	add_xml_identifier(misc, "um_no_menus", um_no_menus_str, "No User Menus", sizeof(um_no_menus_str));
	add_xml_identifier(misc, "um_window_title", um_window_title_str, "User Menus", sizeof(um_window_title_str));

	/* quest_log.cpp */
	add_xml_identifier(misc, "cm_questlog_menu", cm_questlog_menu_str,
		"Show all quests & entries\nOpen quest list window\nOpen NPC list window\n"
		"Select NPCs, starting with none...\nShow just this NPC\nShow just this quest\n--\n"
		"Copy entry\nCopy all entries\nFind text...\nAdd entry...\n--\n"
		"Select entry\nUnselect entry\nSelect all entries\nUnselect all entires\nShow only selected entries\n--\n"
		"Delete entry\nUndelete entry\n--\n"
		"Delete duplicates entries\n--\n"
		"Save changes", sizeof(cm_questlog_menu_str));
	add_xml_identifier(misc, "cm_questlist_menu", cm_questlist_menu_str,
		"Quest completed\nAdd selected entries to quest\n--\n"
		"Hide completed quests\nDo not always open window\nStart window left of entires\n", sizeof(cm_questlist_menu_str));
	add_xml_identifier(misc, "questlog_find_prompt", questlog_find_prompt_str, "Text to Find", sizeof(questlog_find_prompt_str));
	add_xml_identifier(misc, "questlog_add_npc_prompt", questlog_add_npc_prompt_str, "NPC name", sizeof(questlog_add_npc_prompt_str));	
	add_xml_identifier(misc, "questlog_add_text_prompt", questlog_add_text_prompt_str, "Entry text", sizeof(questlog_add_text_prompt_str));	
	add_xml_identifier(misc, "questlog_npc_filter_title", questlog_npc_filter_title_str, "NPC list", sizeof(questlog_npc_filter_title_str));	
	add_xml_identifier(misc, "questlist_filter_title", questlist_filter_title_str, "Quest List", sizeof(questlist_filter_title_str));
	add_xml_identifier(misc, "questlist_showall", questlist_showall_str, "Show all quests", sizeof(questlist_showall_str));
	add_xml_identifier(misc, "questlog_cm_help", questlog_cm_help_str, "Right-click for command menu", sizeof(questlog_cm_help_str));
	add_xml_identifier(misc, "questlog_deldupe_start", questlog_deldupe_start_str, "Deleting duplicate quest log entries...", sizeof(questlog_deldupe_start_str));
	add_xml_identifier(misc, "questlog_deldupe_end", questlog_deldupe_end_str, "...unique entries: %d, deleted duplicates: %d.", sizeof(questlog_deldupe_end_str));
	add_xml_identifier(misc, "questlog_deleted", questlog_deleted_str, "(Deleted)", sizeof(questlog_deleted_str));	
	add_xml_identifier(misc, "item_list_use_help", item_list_use_help_str, "Use quantity - right-click", sizeof(item_list_use_help_str));	
	add_xml_identifier(misc, "item_list_pickup_help", item_list_pickup_help_str, "Pick up - left-click", sizeof(item_list_pickup_help_str));	
	add_xml_identifier(misc, "item_list_edit_help", item_list_edit_help_str, "Edit menu - ctrl+right-click", sizeof(item_list_edit_help_str));	
	add_xml_identifier(misc, "item_list_add_help", item_list_add_help_str, "Add to list - ctrl+left-click", sizeof(item_list_add_help_str));	
	add_xml_identifier(misc, "item_list_drag_help", item_list_drag_help_str, "Add to list - drag from inv/sto", sizeof(item_list_drag_help_str));
	add_xml_identifier(misc, "item_list_create_help", item_list_create_help_str, "Create new list", sizeof(item_list_create_help_str));
	add_xml_identifier(misc, "item_list_magic", item_list_magic_str, "Magical interference caused the list window to close O.O", sizeof(item_list_magic_str));	
	add_xml_identifier(misc, "item_list_find", item_list_find_str, "Find: ", sizeof(item_list_find_str));
	add_xml_identifier(misc, "item_list_find_help", item_list_find_help_str, "Find list - type text", sizeof(item_list_find_help_str));
}
#endif

#ifdef ELC
void init_spell_translatables ()
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
	add_xml_identifier(stats_extra,"base",(char*)attributes.base,"Basic Attributes",sizeof(attributes.base));
	add_xml_identifier(stats_extra,"cross",(char*)attributes.cross,"Cross Attributes",sizeof(attributes.cross));
	add_xml_identifier(stats_extra,"nexus",(char*)attributes.nexus,"Nexus",sizeof(attributes.nexus));
	add_xml_identifier(stats_extra,"skills",(char*)attributes.skills,"Skills",sizeof(attributes.skills));
	add_xml_identifier(stats_extra,"pickpoints",(char*)attributes.pickpoints,"Pickpoints",sizeof(attributes.pickpoints));

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
	add_xml_statid(misc,"action",&(attributes.action_points),"Action Points","actp");

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
	add_xml_statid(skills,"engineering",&(attributes.engineering_skill),"Engineering","eng");
	add_xml_statid(skills,"tailoring",&(attributes.tailoring_skill),"Tailoring","tai");
	add_xml_statid(skills,"ranging",&(attributes.ranging_skill),"Ranging","ran");
	add_xml_statid(skills,"overall",&(attributes.overall_skill),"Overall","oa");
}
#endif

#ifdef ELC
void init_titles ()
{
	add_xml_identifier (titles_str, "w_note", win_notepad, "Notepad", sizeof(win_notepad));
	add_xml_identifier (titles_str, "w_prompt", win_prompt, "Prompt", sizeof(win_prompt));
	add_xml_identifier (titles_str, "w_stats", win_statistics, "Statistics", sizeof(win_statistics));
	add_xml_identifier (titles_str, "w_sigils", win_sigils, "Sigils", sizeof(win_sigils));
	add_xml_identifier (titles_str, "w_inv", win_inventory, "Inventory", sizeof(win_inventory));
	add_xml_identifier (titles_str, "w_help", win_help, "Help", sizeof(win_help));
	add_xml_identifier (titles_str, "w_buddy", win_buddy, "Buddy", sizeof(win_buddy));
	add_xml_identifier (titles_str, "w_config", win_configuration, "Options", sizeof(win_configuration));
	add_xml_identifier (titles_str, "w_manu", win_manufacture, "Manufacture", sizeof(win_manufacture));
	add_xml_identifier (titles_str, "w_astro", win_astrology, "Astrology", sizeof(win_astrology));
	add_xml_identifier (titles_str, "w_principal", win_principal, "Eternal Lands", sizeof(win_principal));
	add_xml_identifier (titles_str, "w_storage", win_storage, "Storage", sizeof(win_storage));
	add_xml_identifier (titles_str, "w_storage_vo", win_storage_vo, " (view only)", sizeof(win_storage_vo));
	add_xml_identifier (titles_str, "w_trade", win_trade, "Trade", sizeof(win_trade));
	add_xml_identifier (titles_str, "w_rules", win_rules, "Rules", sizeof(win_rules));
	add_xml_identifier (titles_str, "w_bag", win_bag, "Bag", sizeof(win_bag));
	add_xml_identifier (titles_str, "w_design", win_design, "Design your character", sizeof(win_design));
	add_xml_identifier (titles_str, "w_name_pass", win_name_pass, "Choose name and password", sizeof(win_name_pass));
	add_xml_identifier (titles_str, "w_newchar", win_newchar, "New Character", sizeof(win_newchar));
	add_xml_identifier (titles_str, "w_minimap", win_minimap, "Minimap", sizeof(win_minimap));
	add_xml_identifier (titles_str, "tab_control", ttab_controls, "Controls", sizeof(ttab_controls));
	add_xml_identifier (titles_str, "tab_audio", ttab_audio, "Audio", sizeof(ttab_audio));
	add_xml_identifier (titles_str, "tab_hud", ttab_hud, "HUD", sizeof(ttab_hud));
	add_xml_identifier (titles_str, "tab_server", ttab_server, "Server", sizeof(ttab_server));
	add_xml_identifier (titles_str, "tab_chat", ttab_chat, "Chat", sizeof(ttab_chat));
	add_xml_identifier (titles_str, "tab_video", ttab_video, "Video", sizeof(ttab_video));
	add_xml_identifier (titles_str, "tab_gfx", ttab_gfx, "GFX", sizeof(ttab_gfx));
	add_xml_identifier (titles_str, "tab_camera", ttab_camera, "Camera", sizeof(ttab_camera));
	add_xml_identifier (titles_str, "tab_troubleshoot", ttab_troubleshoot, "Troubleshoot", sizeof(ttab_troubleshoot));
	add_xml_identifier (titles_str, "tab_font", ttab_font, "Font", sizeof(ttab_font));
	add_xml_identifier (titles_str, "t_help", tab_help, "Help", sizeof(tab_help));
	add_xml_identifier (titles_str, "t_ency", tab_encyclopedia, "Encyclopedia", sizeof(tab_encyclopedia));
	add_xml_identifier (titles_str, "t_skills", tab_skills, "Skills", sizeof(tab_skills));
	add_xml_identifier (titles_str, "t_rules", tab_rules, "Rules", sizeof(tab_rules));
	add_xml_identifier (titles_str, "t_stats", tab_statistics, "Statistics", sizeof(tab_statistics));
	add_xml_identifier (titles_str, "t_know", tab_knowledge, "Knowledge", sizeof(tab_knowledge));
	add_xml_identifier (titles_str, "t_qlog", tab_questlog, "Quest log", sizeof(tab_questlog));
	add_xml_identifier (titles_str, "t_kills", tab_counters, "Counters", sizeof(tab_counters));
	add_xml_identifier (titles_str, "t_session", tab_session, "Session", sizeof(tab_session));
	add_xml_identifier (titles_str, "t_main", tab_main, "Main", sizeof(tab_main));
	add_xml_identifier (titles_str, "b_okay", button_okay, "Okay", sizeof(button_okay));
	add_xml_identifier (titles_str, "b_cancel", button_cancel, "Cancel", sizeof(button_cancel));
	add_xml_identifier (titles_str, "b_new_cat", button_new_category, "New Category", sizeof(button_new_category));
	add_xml_identifier (titles_str, "b_rm_cat", button_remove_category, "Remove Category", sizeof(button_remove_category));
	add_xml_identifier (titles_str, "b_save", button_save_notes, "Save Notes", sizeof(button_save_notes));
	add_xml_identifier (titles_str, "l_nname", label_note_name, "Note name", sizeof(label_note_name));
	add_xml_identifier (titles_str, "l_cursor_coords", label_cursor_coords, "Cursor position", sizeof(label_cursor_coords));
	add_xml_identifier (titles_str, "l_mark_filter", label_mark_filter, "Mark filter", sizeof(label_mark_filter));
	add_xml_identifier (titles_str, "game_version", game_version_str, "Eternal Lands Version %d.%d.%d%s", sizeof(game_version_str));
	add_xml_identifier (titles_str, "b_send", button_send, "Send", sizeof(button_send));
	add_xml_identifier (titles_str, "item_list_name", item_list_name_str, "Enter list name", sizeof(item_list_name_str));
	add_xml_identifier (titles_str, "item_list_rename", item_list_rename_str, "Enter new name", sizeof(item_list_rename_str));
	add_xml_identifier (titles_str, "item_list_preview", item_list_preview_title, "Item lists", sizeof(item_list_preview_title));
	add_xml_identifier (titles_str, "item_list_quantity", item_list_quantity_str, "Quantity", sizeof(item_list_quantity_str));
}
#endif // ELC

#ifdef WRITE_XML
void save_strings(xmlDoc * doc, char * name)
{
	char str[50];
	
	//default language is en - change this if you want to save the strings to another folder...
	safe_snprintf (str, sizeof (str), "languages/en/strings/%s", name); 
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
#ifdef ELC
	save_named_strings(help_str,HELP_STR, "tooltips");
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
	safe_snprintf(file_name, sizeof(file_name), "languages/%s/strings/%s",lang,file);
	tmp=load_strings_file(file_name);
	if(tmp.file==NULL||tmp.root==NULL){
		safe_snprintf(file_name, sizeof(file_name), "languages/en/strings/%s",file);
		tmp=load_strings_file(file_name);
		if(tmp.file==NULL){
			//Notify about this error - english only
			LOG_ERROR("Could not read %s\n", file);
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
				LOG_ERROR("Fatal: couldn't find root element in %s\n",filename);
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
				if(!xmlStrcasecmp(cur->name, (xmlChar*)"name")) {
					char *p=(char*)string->var->str;
					my_xmlStrncopy(&p,  (char*)cur->children->content, sizeof(string->var->str) -1);
#ifdef WRITE_XML
					string->var->saved_str=1;
#endif
				} else if (!xmlStrcasecmp(cur->name, (xmlChar*)"desc")) {
					char *p=(char*)string->var->desc;
					my_xmlStrncopy(&p, (char*)cur->children->content, sizeof(string->var->desc) -1);
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
				if(!xmlStrcasecmp(cur->name, (xmlChar*)"name")) {
					char *p=(char*)string->var->name;
					my_xmlStrncopy(&p, (char*)cur->children->content, 20);
#ifdef WRITE_XML
					string->var->saved_name=1;
#endif
				} else if (!xmlStrcasecmp(cur->name, (xmlChar*)"shortname")){
					char *p=(char*)string->var->shortname;
					my_xmlStrncopy(&p, (char*)cur->children->content, 6);
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
					if(!xmlStrcasecmp(cur->name, (xmlChar*)group->statstrings[i]->xml_id)) {
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
					if(!xmlStrcasecmp(cur->name, (xmlChar*)group->distrings[i]->xml_id)){
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
					if(!xmlStrcasecmp(cur->name, (xmlChar*)group->strings[i]->xml_id))	{
						my_xmlStrncopy(&group->strings[i]->var, (char*)cur->children->content, group->strings[i]->max_len);
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
						if(!xmlStrcasecmp(cur->name, (xmlChar*)group[i].xml_id)) {
							parse_strings(cur,&(group[i]));
#ifdef WRITE_XML
							group[i].saved=1;
#endif
							i=size;
						}
						break;
					case DIGROUP:
						if(!xmlStrcasecmp(cur->name, (xmlChar*)Group[i].xml_id)) {
							parse_distrings(cur,&(Group[i]));
#ifdef WRITE_XML
							Group[i].saved=1;
#endif
							i=size;
						}
						break;
#ifdef ELC
					case STAT_GROUP:
						if(!xmlStrcasecmp(cur->name, (xmlChar*)stat[i].xml_id)) {
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
