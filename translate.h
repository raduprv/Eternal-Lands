/*!
 * \file
 * \brief 	Functions related to internationalization of the client.
 * \ingroup 	translation
 */
#ifndef __TRANSLATE_H__
#define __TRANSLATE_H__
#include <libxml/parser.h>
#ifdef ELC
#include "stats.h"
#endif

/*! \name Group types*/
/*! \{ */
#define GROUP 0
#define DIGROUP 1
#ifdef ELC
#define STAT_GROUP 2
#endif
/*! \} */

/*!
 * This is used for setting a short name and description - used for i.e. options and sigils
 */
typedef struct
{
	unsigned char str[31];     /*!< str */
#ifdef WRITE_XML
	int saved_str;             /*!< saved_str */
#endif
	unsigned char desc[101];   /*!< desc */
#ifdef WRITE_XML
	int saved_desc;            /*!< saved_desc */
#endif
} dichar;

/*! 
 * Defines a normal xml-node and a pointer to the variable the content should be saved to.
 */
typedef struct
{
	char xml_id[15];
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
	char xml_id[15];
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
	char xml_id[15];
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
	char xml_id[15];
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
	char xml_id[15];
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
	char xml_id[15];
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
		tt_help[30];
#endif  //DOXYGEN_SKIP_THIS
#endif  //ELC

#ifdef ELC
#ifndef DOXYGEN_SKIP_THIS
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
#endif  //DOXYGEN_SKIP_THIS
#endif  //ELC

#ifndef DOXYGEN_SKIP_THIS
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
		stat_no_invalid[50], //stats.c
		timer_lagging_behind[100], //timers.c
		you_can_proceed[50],
		accepted_rules[50],
		read_rules_str[50],
		parse_rules_str[50],
		rules_not_found[100];
#else
		;
#endif

#endif  //DOXYGEN_SKIP_THIS

/*!
 * \ingroup	translation
 * \brief 	Initiates the translatable strings
 *
 * 		Initiates the translatable strings - uses the "See Also" subfunctions.
 *
 * \return  	None
 * \sa	init_console
 * \sa	init_help
 * \sa	init_options
 * \sa	init_spells
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
 * \return  	None
 * \sa		parse_errors
 * \sa		parse_console
 * \sa		parse_spells
 * \sa		parse_options
 * \sa		parse_stats
 * \callgraph
 */
void load_translatables();

#ifdef ELC
/*!
 * \ingroup 	translation
 * \brief	Initiates the console strings
 * 
 * 		Initiates the console strings.
 *
 * \return	None
 */
void init_console(void);

/*!
 * \ingroup	translation
 * \brief	Initiates the help strings
 * 		
 * 		Initiates the help strings.
 * 
 * \return 	None
 */
void init_help(void);

/*!
 * \ingroup	translation
 * \brief	Initiates the options strings
 * 
 * 		Initiates the options strings.
 *
 * \return 	None
 */
void init_options(void);

/*!
 * \ingroup	translation
 * \brief	Initiates the spells/sigils strings
 *
 * 		Initiates the spells/sigils strings.
 *
 * \return	None
 */
void init_spells(void);

/*!
 * \ingroup	translation
 * \brief	Initiates the stats strings
 * 
 * 		Initiates the stats strings
 *
 * \return	None
 */
void init_stats(void);
#endif

/*!
 * \ingroup	translation
 * \brief	Initiates the error strings
 *
 * 		Initiates the error strings
 *
 * \return	None
 */
void init_errors(void);

/*!
 * \ingroup	translation
 * \brief	Defines a new XML-group
 *
 * 		Defines a new XML-group of the given type with no objects (followed by their xml ID's)
 *
 * 		Example usage:
 * \code
 * 		test=add_xml_group(GROUP,6,"nothing","something","anything","misc","tuesday","friday","yesterday");
 * \endcode
 * 		Would look for a the elements:
 * \code
 * 		<root>
 * 			<nothing>...</nothing>
 * 			<someting>...</something>
 * 			<anything>...</anything>
 * 			<misc>...</misc>
 * 			<tuesday>...</tuesday>
 * 			<friday>...</friday>
 * 			<yesterday>...</yesterday>
 * 		</root>
 * \endcode
 * 		You have to add the elements it will look for inside these group ID's later.
 * 
 * \param	type The xml-group type. GROUP, DIGROUP or STAT_GROUP.
 * \param	no The number of objects in the group
 * \param	... The groups xml-objects
 * \return	A pointer to the xml-group array
 */
void * add_xml_group(int type, int no, ...);

/*!
 * \ingroup	translation
 * \brief	Adds a new distring_item to the given group.
 *
 * 		Adds a new distring_item to the given group, with the given xml-id. It also sets the variables default string/description.
 *
 * \param	group The group you wish to add the distring_item to
 * \param	xml_id The xml id
 * \param	var A pointer to the variable
 * \param	str The default string
 * \param	desc The default description
 * \return	None
 * \sa		distring_item
 * \sa		add_xml_group
 * \sa		init_translatables
 */
void add_xml_distringid(group_id_di * group, char * xml_id, dichar * var, char * str, char * desc);

#ifdef ELC
/*!
 * \ingroup	translation
 * \brief	Adds a new statstring_item to the group. 
 *
 * 		Adds a new statstring_item to the group, and associates it with teh xml_id. Also sets the variables default name and shortname.
 * 
 * \param	group The group you wish to add the statstring_item to.
 * \param	xml_id The xml identifier
 * \param	var The variable the new statstring_item points to
 * \param	name The default name
 * \param	shortname The default short name
 * \return	None
 * \sa		statstring_item
 * \sa		add_xml_group
 * \sa		init_translatables
 */
void add_xml_statid(group_stat * group, char * xml_id, names * var, char * name, char * shortname);
#endif

/*!
 * \ingroup	translation
 * \brief	Adds a new string_item to the group
 *
 * 		The function adds a new string_item to the given group and associates it with teh given xml_id. Furthermore it sets the variables default name and the maximum length of the variable.
 *
 * \param	group The group you wish to add the string_item to
 * \param	xml_id The xml-nodes ID
 * \param	var A pointer to the variable
 * \param	def The default string
 * \param	max_len The maximum length
 * \return	None
 * \sa		string_item
 * \sa		add_xml_group
 * \sa		init_translatables
 */
void add_xml_identifier(group_id * group, char * xml_id, char * var, char * def, int max_len);

/*!
 * \ingroup	translation
 * \brief	Free's the memory used by the xml-parser
 *
 * 		Free's the memory used by the xml-parser to store the groups and their children. Only call when you're done parsing!
 *
 * \param	type The type of group
 * \param	gPtr A pointer ot the group
 * \param	no The number of items in the group
 * \return	None
 */
void free_xml_parser(int type, void * gPtr, int no);

/*!
 * \ingroup	translation
 * \brief	Parses the errors
 * 
 * 		Checks the current document and parses the errors group.
 *
 * \param	in The root xml-node
 * \return	None
 */
void parse_errors(xmlNode * in);

#ifdef ELC
/*!
 * \ingroup	translation
 * \brief	Parses the console
 * 		
 * 		Checks the current document and parses the console group.
 *
 * \param	in The root xml-node
 * \return	None
 */
void parse_console(xmlNode * in);

/*!
 * \ingroup	translation
 * \brief	Parses the help
 * 		
 * 		Checks the current document and parses the help group.
 *
 * \param	in The root xml-node
 * \return	None
 */
void parse_help(xmlNode * in);

/*!
 * \ingroup	translation
 * \brief	Parses the options
 * 		
 * 		Checks the current document and parses the options group.
 *
 * \param	in The root xml-node
 * \return	None
 */
void parse_options(xmlNode * in);

/*!
 * \ingroup	translation
 * \brief	Parses the sigils/spells
 * 		
 * 		Checks the current document and parses the spells/sigils group.
 *
 * \param	in The root xml-node
 * \return	None
 */
void parse_spells(xmlNode * in);

/*!
 * \ingroup	translation
 * \brief	Parses the stats
 * 		
 * 		Checks the current document and parses the stats/sigils group.
 *
 * \param	in The root xml-node
 * \return	None
 */
void parse_stats(xmlNode * in);
#endif

#ifdef WRITE_XML
/*!
 * \ingroup	translation
 * \brief	Saves the group strings to ./languages/en/strings/\<filename\>
 *
 * 		If you have defined WRITE_XML it will write the strings to ./languages/en/strings (be sure to make the directories first).
 *
 * \param	doc A pointer to the current document
 * \param	name The filename of the document
 * \return	None
 */
void save_strings(xmlDoc * doc, char * name);
#endif

/*!
 * \ingroup	translation
 * \brief	Loads the strings from a file (calls load_strings_file)
 *
 * 		First tries loading the file from the native language directory, then it loads the strings from the ./languages/en dir.
 *
 * \param	file The filename you wish to load (not the directory, this function will call load_strings_file to open the file in the right dir)
 * \return	A structure containing the xmlDoc * and a xmlNode * to the root element.
 * \sa		load_strings_file
 */
struct xml_struct load_strings(char * file);

/*!
 * \ingroup	translation
 * \brief	Loads an xml file with the strings
 *
 * 		Loads the xml-file pointed to by filename. Returns the an xml_struct containing a pointer to the document and the root element, if the document and root element is found/loaded succesfully. Otherwise it returns NULL in both pointers.
 */
struct xml_struct load_strings_file(char * filename);

/*!
 * \ingroup	translation
 * \brief	Copies the strings from the xmlNode to the distring_item *.
 *
 * 		When the xml_id is found this function is called to copy the distring_item (the strings are in the \<name\> and \<desc\> tags)
 *
 * \param	in The current xmlNode
 * \param	string A pointer to the distring_item.
 * \return	None
 */
void copy_strings(xmlNode * in, distring_item * string);

#ifdef ELC
/*!
 * \ingroup	translation
 * \brief	Copies the stats from the xmlNode to the statstring_item *.
 *
 * 		When the xmlNode * is found in the current document, this function is called to copy the stats to the statstring_item *
 *
 * \param	in The current xmlNode
 * \param	string A pointer to the statstring_item
 * \return	None
 */
void copy_stats(xmlNode * in, statstring_item * string);

/*!
 * \ingroup	translation
 * \brief	Parses a statstring_item group
 * 
 * 		Parses a statstring group and calls copy_stats if it finds a match.
 *
 * \param	in The current xmlNode
 * \param	group The group you wish to parse
 * \return	None
 */
void parse_statstrings(xmlNode * in, group_stat * group);
#endif

/*!
 * \ingroup	translation
 * \brief	Parses a distring_item group
 *
 * 		Parses the distring_item group (group) and calls copy_strings if a match is found.
 *
 * \param	in The current xmlNode
 * \param	group The current group
 * \return	None
 */
void parse_distrings(xmlNode * in, group_id_di * group);

/*!
 * \ingroup	translation
 * \brief	Parses a string_item group
 *
 * 		Parses the string_item group (group) and copies the string if a match is found (using UTF8Toisolat1).
 *
 * \param	in The current xmlNode*
 * \param	group The current group
 * \return 	None
 */
void parse_strings(xmlNode * in, group_id * group);

/*!
 * \ingroup	translation
 * \brief	Called when parsing any group.
 *
 *		This is the main parser. It is called by all of the groups when parsing their different xml element names, comparing it with the current xmlNodes name - if it finds a match it calls either parse_strings, parse_distrings or parse_statstrings.
 *
 * \param	in The current xmlNode
 * \param	gPtr The current group
 * \param	size The size of the group
 * \param	type The type of the group
 * \return	None
 * \todo	An idea might be to improve the groups to also have function *, to tell them what function it should call depending on the kind of group. group->parse(xmlNode *, group *), would look better than the current switch statement.
 */
void parse_groups(xmlNode * in, void * gPtr, int size, int type);

#endif
