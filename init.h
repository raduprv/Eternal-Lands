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

	unsigned int hud_indicators_options;

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

	/*!
	 * \name Item lists - active list index.
	 */
	/*! @{ */
	unsigned int active_item_list;
	/*! @} */

	unsigned int hud_indicators_position;

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

	// the following additions are backwards compatible to previous files and have the same version number

	/*!
	 * \name quick spell window position and options
	 */
	/*! @{ */
	unsigned int quickspell_win_position;
	unsigned int quickspell_win_options;
	/*! @} */

	/*!
	 * \name chat window position
	 */
	/*! @{ */
	int chat_win_x;
	int chat_win_y;
	/*! @} */


}bin_cfg;

extern char configdir[256]; /*!< the default directory where we look for configuration files */
extern char datadir[256]; /*!< the default directory where we look for data files (aka installation dir) */

/*!
 * \ingroup loadsave
 * \brief   Stores the window layout in the binary el.cfg file.
 *
 *		Stores the window layout and related information in the binary el.cfg file.
 *
 * \pre If el.cfg could be opened for writing, the function returns without performing any actions.
 */
void save_bin_cfg(void);

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
void init_stuff(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif //__INIT_H__
