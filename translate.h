#ifndef __TRANSLATE_H__
#define __TRANSLATE_H__

typedef struct
{
	unsigned char str[31];
#ifdef WRITE_XML
	int saved_str;
#endif
	unsigned char desc[101];
#ifdef WRITE_XML
	int saved_desc;
#endif
} dichar;

#ifdef ELC
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
#endif

#ifdef ELC
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
#endif

#ifdef ELC
//Tooltips
extern char 	tt_walk[30],
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
#endif

#ifdef ELC
//Help messages
extern char	
		/*3d_objects.c*/
		values_str[20], 
		/*console.c*/
		logconn_str[50],
		/*draw_scene.c*/
		low_framerate_str[100],
		/*filter.c*/
		no_filters_str[50],
		filters_str[50],
		/*gl_init.c*/
		window_size_adjusted_str[50],
		/*hud.c*/
		no_open_on_trade[100], 
		/*ignore.c*/
		no_ignores_str[50],
		ignores_str[50],
		/*interface.c*/
		login_username_str[20],
		login_password_str[20],
		/*items.c*/
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
		afk_no[5],
		afk_names[15],
		afk_messages[25],
		afk_print_help[150],
		/*trade.c*/
		quantity_str[30],
		abort_str[10];
#endif

//Errors
extern char	reg_error_str[15],
		/*3d_objects.c*/
		cant_load_2d_object[30],
		cant_open_file[30], //2d_objects.c
		object_error_str[30], 
		nasty_error_str[50], 
		corrupted_object[100], 
		bad_object[30],
		multiple_material_same_texture[100],
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
		/*cache.c*/
		cache_size_str[20],
		/*console.c*/
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
		/*cursors.c*/
		cursors_file_str[30],
		/*dialogues.c*/
		close_str[20],
#endif
		/*font.c*/
		cant_load_font[30],
#ifdef ELC
		/*gl_init.c*/
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
		/*init.c*/
		fatal_error_str[10],
		no_e3d_list[50],
		enabled_vertex_arrays[50],
		disabled_point_particles[50],
		disabled_particles_str[50],
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
		redefine_your_colours[250],
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
		part_def_str[20],
		part_sys_str[20],
		part_part_str[20]
#ifdef ELC
		/*paste.c*/
		,not_ascii[20],
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
		snd_media_ogg_error[50], //sound.c
		stat_no_invalid[50]; //stats.c
#else
		;
#endif

void init_translatables();
void load_translatables();

#endif
