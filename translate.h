/*!
 * \file
 * \ingroup 	translation
 * \brief 	Functions related to internationalization of the client.
 */
#ifndef __TRANSLATE_H__
#define __TRANSLATE_H__
#include <libxml/parser.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XML_ID_SIZE 64

/*!
 * This is used for setting a short name and description - used for i.e. options and sigils
 */
typedef struct
{
	unsigned char str[51];     /*!< str */
#ifdef WRITE_XML
	int saved_str;             /*!< saved_str */
#endif
	unsigned char desc[251];   /*!< desc */
#ifdef WRITE_XML
	int saved_desc;            /*!< saved_desc */
#endif
} dichar;

#ifdef ELC
#include "stats.h"
#endif

/*! 
 * Defines a normal xml-node and a pointer to the variable the content should be saved to.
 */
typedef struct
{
	char xml_id[XML_ID_SIZE];
	char * var;
	int max_len;
#ifdef WRITE_XML
	int saved;
#endif
} string_item;

/*!
 * Defines a distring xml-node (contains both \<name\> and \<desc\> as child-nodes to \<xml_id\>).
 */
typedef struct
{
	char xml_id[XML_ID_SIZE];
	dichar * var;
#ifdef WRITE_XML
	int saved;
#endif
} distring_item;

/*!
 * Defines a name xml-node (contains both \<name\> and \<shortname\>) used for i.e. stats.
 */
#ifdef ELC
typedef struct
{
	char xml_id[XML_ID_SIZE];
	names * var;
#ifdef WRITE_XML
	int saved;
#endif
} statstring_item;
#endif

/*!
 * Defines an xml-group using string_item ID's.
 */
typedef struct
{
	char xml_id[XML_ID_SIZE];
	int no;
	string_item ** strings;
#ifdef WRITE_XML
	int saved;
#endif
} group_id;

/*!
 * Defines an xml-group using distring_item ID's
 */
typedef struct
{
	char xml_id[XML_ID_SIZE];
	int no;
	distring_item ** distrings;
#ifdef WRITE_XML
	int saved;
#endif
} group_id_di;

#ifdef ELC
/*!
 * Defines an xml-group using distring_item ID's
 */
typedef struct
{
	char xml_id[XML_ID_SIZE];
	int no;
	statstring_item ** statstrings;
#ifdef WRITE_XML
	int saved;
#endif
} group_stat;
#endif

/*! 
 * Defines an xml-structure with the root element and document - used when loading the file.
 */
struct xml_struct
{
	xmlDoc * file;
	xmlNode * root;
};


#ifdef ELC
#ifndef DOXYGEN_SKIP_THIS
//Options
extern char	switch_video_mode[50];

extern dichar	opt_shadows,
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

//Sigils
extern char 	sig_too_few_sigs[50];
#endif //DOXYGEN_SKIP_THIS
#endif //ELF

#ifdef ELC
#ifndef DOXYGEN_SKIP_THIS
extern dichar	sig_change,
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
#endif  //DOXYGEN_SKIP_THIS
#endif  //ELC

#ifdef ELC
#ifndef DOXYGEN_SKIP_THIS
//Tooltips
extern char 	tt_walk[30],
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


#endif  //DOXYGEN_SKIP_THIS
#endif  //ELC

#ifdef ELC
#ifndef DOXYGEN_SKIP_THIS
//Help messages
extern char	
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
		/*console.c*/
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
		cmd_session_counters[20],
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
		/*filter.c*/
		no_filters_str[50],
		filters_str[50],
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
		/*ignore.c*/
		no_ignores_str[50],
		ignores_str[50],
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
		mixoneall_help_str[50],
		itmlst_help_str[50],
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
		about_human[30],
		about_elves[30], 
		about_dwarfs[30],
		about_gnomes[30],
		about_orchans[30],
		about_draegoni[30],
		zoom_in_out[200],
		rotate_camera[200],
		p2p_race[100],
		char_help[200],
		invalid_pass[30],
		show_password[30],
		hide_password[30],
		char_done[15],
		char_back[15],
		/*pm_log.c*/
		going_afk[30],
		not_afk[50],
		new_messages[100],
		afk_names[15],
		afk_messages[25],
		afk_print_help[150],
		/*text.c*/
		pm_from_str[10],
		gm_from_str[10],
		ig_from_str[10],
		mc_from_str[20],
		mod_pm_from_str[15],
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
		/* item lists */
		cm_item_list_selected_str[40],
		cm_item_list_names_str[100],
		item_list_use_help_str[40],
		item_list_pickup_help_str[40],
		item_list_edit_help_str[40],
		item_list_add_help_str[40],
		item_list_drag_help_str[40],
		item_list_create_help_str[40],
		item_list_magic_str[80],
		item_list_find_str[20],
		item_list_find_help_str [40],
		/* new_character.c */
		use_appropriate_name[500];
#endif  //DOXYGEN_SKIP_THIS
#endif  //ELC

#ifndef DOXYGEN_SKIP_THIS
//Errors
extern char	reg_error_str[15],
		file_write_error_str[20],
		/*3d_objects.c*/
		cant_load_2d_object[30],
		cant_open_file[30], //2d_objects.c
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
		duplicate_actors_str[50],
		bad_actor_name_length[50],
		/*actor_scripts.c*/
		resync_server[50],
		cant_add_command[50],
		/* books.c*/
		book_open_err_str[30],
		/*cache.c*/
		cache_items_str[20],
		cache_size_str[20],
		/* cal.c */
		no_animation_err_str[30],
		/*console.c*/
		invalid_location_str[30],
		name_too_long[75], 
		name_too_short[75],
		not_added_to_ignores[75],
		already_ignoring[50],
		ignore_list_full[100],
		added_to_ignores[50],
		word_too_long[75],
		word_too_short[75],
		not_added_to_filter[50],
		already_filtering[50],
		filter_list_full[100],
		added_to_filters[50],
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
		help_request_str[20],
		help_cmd_str[10],
		char_cmd_str[2],
		char_at_str[2],
		char_slash_str[2],
		gm_cmd_str[5],
		mod_cmd_str[5],
		bc_cmd_str[5],
		msg_accept_buddy_str[55],
		date_format[100],
		book_count_str[60],
		know_help_str[60],
		/*cursors.c*/
		cursors_file_str[30],
		/*dialogues.c*/
		close_str[20],
		dialogue_copy_str[20],
		dialogue_repeat_str[20],
		open_storage_str[20],
		reopen_storage_str[50],
#endif
		/*XML and channel list errors from chat.c*/
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
		/*gl_init.c*/
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
		/*init.c*/
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
		/* item lists */
		item_list_format_error[50],
		item_list_save_error_str[50],
		item_list_cat_format_error_str[50],
		item_list_version_error_str[70],
		item_list_empty_list_str[50],
#endif  // ELC
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
		snd_media_ogg_info_noartist[50],
		snd_media_ogg_info[50], //sound.c
		stat_no_invalid[50], //stats.c
		timer_lagging_behind[100], //timers.c
		/*spells.c*/
		cast_str[20],
		invalid_spell_str[20],
		/*rules.c*/
		you_can_proceed[50],
		accepted_rules[80],
		accept_label[20],
		read_rules_str[50],
		parse_rules_str[50],
		rules_not_found[100],
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
		cant_load_encycl[70],
		warn_currently_ignoring[50],
		dc_note_remove[50],
		note_saved[50],
		note_save_failed[50];
#else
		;
#endif  // ELC

#endif  //DOXYGEN_SKIP_THIS

#ifdef ELC
#ifndef DOXYGEN_SKIP_THIS
// window/widget titles
extern char	win_notepad[20],
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
		button_cancel[10],
		button_new_category[30],
		button_remove_category[30],
		button_save_notes[30],
		game_version_str[60],
		label_note_name[20],
		label_cursor_coords[17],
		label_mark_filter[13],
		button_send[10];
#endif  // DOXYGEN_SKIP_THIS
#endif  // ELC

/*!
 * \ingroup	translation
 * \brief 	Initiates the translatable strings
 *
 * 		Initiates the translatable strings - uses the "See Also" subfunctions.
 *
 * \sa	init_console
 * \sa	init_help
 * \sa	init_spell_translatables
 * \sa	init_stats
 * \sa	init_errors
 * \callgraph
 */
void init_translatables();

/*!
 * \ingroup 	translation
 * \brief 	Loads the translatable strings from their xml-files.
 *
 *      	Loads the translatable strings from their xml-files. Uses the "See also" subfunctions.
 *
 * \sa		parse_errors
 * \sa		parse_console
 * \sa		parse_spells
 * \sa		parse_options
 * \sa		parse_stats
 * \callgraph
 */
void load_translatables();


/*!
 * \ingroup	translation
 * \brief 	Adds a label/description pair for a config option
 *
 *          Adds a label/description pair for a config option.
 *
 * \sa      add_var
 * \sa      add_xml_distringid
 * \callgraph
 */
void add_options_distringid(char * xml_id, dichar * var, char * str, char * desc);


/*!
 * \ingroup	translation
 *
 *          Retrieve a translated string by its name.
 *
 * \callgraph
 */
const char* get_named_string(const char* group_name, const char* string_name);


/*!
 * \ingroup	translation
 *
 *          Free allocated memory.
 *
 * \callgraph
 */
 void free_translations(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
