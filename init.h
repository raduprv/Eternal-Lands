/*!
 * \file
 * \ingroup init
 * \brief Initialization related functions.
 */
#ifndef __INIT_H__
#define __INIT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "list.h"

/*!
 * Binary configuration data
 */
typedef struct
{
	int cfg_version_num; /*!< version number of the configuration */

	/*!
	 * \name Inventory window position
	 */
	/*! @{ */
	int items_menu_x;
	int items_menu_y;
	/*! @} */

	/*!
	 * \name Ground items menu position
	 */
	/*! @{ */
	int ground_items_menu_x;
	int ground_items_menu_y;
	/*! @} */

	/*!
	 * \name Manufacture window position
	 */
	/*! @{ */
	int manufacture_menu_x;
	int manufacture_menu_y;
	/*! @} */

	/*!
	 * \name Trade window position
	 */
	/*! @{ */
	int trade_menu_x;
	int trade_menu_y;
	/*! @} */

	/*!
	 * \name Options window position
	 */
	/*! @{ */
	int elconfig_menu_x;
	int elconfig_menu_y;
	/*! @} */

	/*!
	 * \name Stats window position
	 */
	/*! @{ */
	int tab_stats_x;
	int tab_stats_y;
	/*! @} */

	/*!
	 * \name Sigils window position
	 */
	/*! @{ */
	int sigil_menu_x;
	int sigil_menu_y;
	/*! @} */

	/*!
	 * \name Dialogues window position
	 */
	/*! @{ */
	int dialogue_menu_x;
	int dialogue_menu_y;
	/*! @} */

	/*!
	 * \name Help window position
	 */
	/*! @{ */
	int tab_help_x;
	int tab_help_y;
	/*! @} */

	/*!
	 * \name Quickbar location and flags
	 */
	/*! @{ */
	int quickbar_x;
	int quickbar_y;
	int quickbar_flags;
	/*! @} */

	int watch_this_stat;	// not used anymore, left here to avoid breaking the file-structure

	int has_accepted_rules;

	int quantity[6];

	int storage_win_x;
	int storage_win_y;

	int buddy_menu_x;
	int buddy_menu_y;

	int quantity_selected;

	/*!
	 * \name Banner settings configured via banner popup menu
	 */
	/*! @{ */
	int banner_settings;
	/*! @} */

	int unused_01;
	int unused_02;

	/*!
	 * \name quest log window position
	 */
	/*! @{ */
	int questlog_win_x;
	int questlog_win_y;
	/*! @} */

	/*!
	 * \name minimap window position and settings
	 */
	/*! @{ */
	int minimap_win_x;
	int minimap_win_y;
	int minimap_zoom;
	/*! @} */

	/*!
	 * \name current selected tabs 4 bits each
	 */
	/*! @{ */
	unsigned tab_selected;
	/*! @} */

	/*!
	 * \name Camera position and attributes
	 */
	/*! @{ */
	float camera_x;
	float camera_y;
	float camera_z;
	float zoom_level;
	/*! @} */

	/*!
	 * \name Astrology window position
	 */
	/*! @{ */
	int astrology_win_x;
	int astrology_win_y;
	/*! @} */

	/*!
	 * \name info tab window position
	 */
	/*! @{ */
	int tab_info_x;
	int tab_info_y;
	/*! @} */

	/*!
	 * \name Language selection window - saved flag
	 */
	/*! @{ */
	int have_saved_langsel;
	/*! @} */

	/*!
	 * \name Misc bool options
	 */
	/*! @{ */
	int misc_bool_options;
	/*! @} */

	/*!
	 * \name User menu options
	 */
	/*! @{ */
	int user_menu_options;
	int user_menu_win_x;
	int user_menu_win_y;
	/*! @} */

	/*!
	 * \name do spells start minimized?
	 */
	/*! @{ */
	int start_mini_spells;
	/*! @} */

	int emotes_menu_x;
	int emotes_menu_y;
	int watch_this_stats[5];
	unsigned int floating_counter_flags;
	unsigned int questlog_flags;

	/*!
	 * \name Ranging window loc
	 */
	/*! @{ */
	int ranging_win_x;
	int ranging_win_y;
	/*! @} */
}bin_cfg;

extern int auto_update; /*!<this flags signals whether or not autoupdates are performed at startup, or not. It requires a restart to have an effect. */
#ifdef  CUSTOM_UPDATE
extern int custom_update; /*!<this flags signals whether or not autoupdates of custom looks is permitted. */
extern int custom_clothing; /*!<this flags signals whether or not custom is displayed. */
#endif  //CUSTOM_UPDATE

extern int poor_man; /*!< this flag, if set to true, indicates we are running on a really poor machine */
extern int show_reflection; /*!< flag that indicates whether to display reflections or not */
#ifdef ANTI_ALIAS
extern int anti_alias; /*!< flag indicating whether anti-aliasing should be enabled */
#endif
extern int special_effects; /*!< flag indicating whether pretty spell effects should be enabled */
extern int isometric; /*!< use isometric instead of perspective view */
extern int mouse_limit;
extern int show_fps; /*!< flag that indicates whether to display FPS or not */
#ifdef OSX
extern int square_buttons; /* flag to overcome intel opengl issues on early MacBooks*/
extern int emulate3buttonmouse;
#endif
#ifdef DEBUG
extern int render_skeleton;
extern int render_mesh;
extern int render_bones_id;
extern int render_bones_orientation;
#endif
extern int limit_fps; /*!< contains the max FPS number we should use. If this is 0, the highest possible number will be used. */
extern int item_window_on_drop;
extern int no_adjust_shadows;
extern int clouds_shadows; /*!< flag that indicates whether the shadows of clouds should be displayed or not */
extern int buddy_log_notice; /*!< whether to log buddy logged on/off notices to screen */
extern char configdir[256]; /*!< the default directory where we look for configuration files */
extern char datadir[256]; /*!< the default directory where we look for data files (aka installation dir) */
extern int show_help_text;
#if !defined(WINDOWS) && !defined(OSX)
extern int use_clipboard; /*!< whether to use CLIPBOARD or PRIMARY for pasting */
#endif

extern int disconnected; /*!< indicates whether we are currently connected or not */
extern int exit_now; /*!< a flag that indicates we should exit the programm immediately */
extern int restart_required; /*!< a flag that the exit should attempt to restart, not just exit */
extern int allow_restart; /*!< a flag that indicates if restarting after download is permitted */

extern char lang[10]; /*!< contains the identifier for the current language. \todo Shouldn't this go into translate.h? */

extern int video_mode_set;

/*!
 * \ingroup loadsave
 * \brief   Stores the window layout in the binary el.cfg file.
 *
 *		Stores the window layout and related information in the binary el.cfg file.
 *
 * \pre If el.cfg could be opened for writing, the function returns without performing any actions.
 */
void save_bin_cfg();

/*!
 * \ingroup init
 * \brief   Does all the necessary initialization at program startup time.
 *
 *		The function will be called from \ref main and does all necessary initialization at program startup time.
 *
 * \callgraph
 *
 * \pre If the rules.xml file is missing, the function will terminate the program with an exit code of 3.
 *
 * \note If SDLNet_Init fails the function will terminate the program with an exit code of 2. If the SDL timer subsystem initialization fails, it will terminate with an exit code of 1.
 */
void init_stuff();

#ifdef __cplusplus
} // extern "C"
#endif

#endif //__INIT_H__
