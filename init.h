/*!
 * \file
 * \ingroup init
 * \brief initialization related functions.
 */
#ifndef __INIT_H__
#define __INIT_H__

/*!
 * binary configuration data
 */
typedef struct
{
	int cfg_version_num; /*!< version number of the configuration */

    /*!
     * \name inventory window position
     */
    /*! @{ */
	int items_menu_x;
	int items_menu_y;
    /*! @} */
    
    /*!
     * \name ground items menu position
     */
    /*! @{ */
	int ground_items_menu_x;
	int ground_items_menu_y;
    /*! @} */
    
    /*!
     * \name manufacture window position
     */
    /*! @{ */
	int manufacture_menu_x;
	int manufacture_menu_y;
    /*! @} */
    
    /*!
     * \name trade window position
     */
    /*! @{ */
	int trade_menu_x;
	int trade_menu_y;
    /*! @} */
    
    /*!
     * \name options window position
     */
    /*! @{ */
	int options_menu_x;
	int options_menu_y;
    /*! @} */
    
    /*!
     * \name stats window position
     */
    /*! @{ */
	int attrib_menu_x;
	int attrib_menu_y;
    /*! @} */
    
    /*!
     * \name sigils window position
     */
    /*! @{ */
	int sigil_menu_x;
	int sigil_menu_y;
    /*! @} */
    
    /*!
     * \name dialogues window position
     */
    /*! @{ */
	int dialogue_menu_x;
	int dialogue_menu_y;
    /*! @} */
    
    /*!
     * \name knowledge window position
     */
    /*! @{ */
	int knowledge_menu_x;
	int knowledge_menu_y;
    /*! @} */
    
    /*!
     * \name encyclopedia window position
     */
    /*! @{ */
	int encyclopedia_menu_x;
	int encyclopedia_menu_y;
    /*! @} */
    
    /*!
     * \name questlog window position
     */
    /*! @{ */
	int questlog_menu_x;
	int questlog_menu_y;
    /*! @} */
    
    /*!
     * \name quickbar location and flags
     */
    /*! @{ */
	int quickbar_x;
	int quickbar_y;
	int quickbar_flags;
	/*! @} */
    
	int watch_this_stat;

	int has_accepted_rules;

	//!!!!!!!If you add any new INT option, decrement the reserved thingy accordingly!!!!!!
	int reserved[16];

    /*!
     * \name camera position and attributes
     */
    /*! @{ */
	float camera_x;
	float camera_y;
	float camera_z;
	float zoom_level;
	float camera_angle;
    /*! @} */
    
	//!!!!!!!If you add any new FLOAT option, decrement the reserved thingy accordingly!!!!!!
	float freserved[20];

}bin_cfg;

extern int poor_man; /*!< this flag, if set to true, indicates we are running on a really poor machine */
extern int show_reflection; /*!< flag that indicates whether to display reflections or not */
extern int mouse_limit;
extern int show_fps; /*!< flag that indicates whether to display FPS or not */
extern int limit_fps; /*!< contains the max FPS number we should use. If this is 0, the highest possible number will be used. */
extern int item_window_on_drop;
extern int no_adjust_shadows;
extern int clouds_shadows; /*!< flag that indicates whether the shadows of clouds should be displayed or not */
extern int compass_direction;
extern char configdir[256]; /*!< the default directory where we look for configuration files */
extern char datadir[256]; /*!< the default directory where we look for data files (aka installation dir) */
extern int show_help_text;

extern int disconnected; /*!< indicates whether we are currently connected or not */
extern int exit_now; /*!< a flag that indicates we should exit the programm immediately */
extern int have_url;
extern char current_url[160]; /*!< contains the last URL that was opened by hitting F2 */
extern char browser_name[120]; /*!< a buffer that contains the name of the browser we should use */

extern char lang[10]; /*!< contains the identifier for the current language. \todo Shouldn't this go into translate.h? */

extern int video_mode_set;

/*!
 * \ingroup loadsave
 * \brief
 *
 *      Detail
 *
 */
void save_bin_cfg();

/*!
 * \ingroup init
 * \brief
 *
 *      Detail
 *
 * \callgraph
 */
void init_stuff();

/*!
 * \ingroup load
 * \brief
 *
 *      Detail
 *
 * \sa start_rendering
 */
void unload_e3d_list();

#endif	//__INIT_H__
