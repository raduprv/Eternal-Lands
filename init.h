/*!
 * \file
 * \ingroup init
 * \brief initialization related functions.
 */
#ifndef __INIT_H__
#define __INIT_H__

#define	CFG_VERSION	5 /*!< version of the \see bin_cfg we are using */

#ifndef DATA_DIR
#define DATA_DIR "./"
#endif

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

extern int ini_file_size;
extern int have_stencil;
extern int poor_man;
extern int show_reflection;
extern int mouse_limit;
extern int show_fps;
extern int limit_fps;
extern int item_window_on_drop;
extern int no_adjust_shadows;
extern int clouds_shadows;
extern int compass_direction;
extern char configdir[256];
extern char datadir[256];

extern int show_stats_in_hud;
extern int show_help_text;

extern int disconnected;
extern int exit_now;
extern int have_url;
extern char current_url[160];
extern char broswer_name[120];

extern char lang[10];

/*!
 * \ingroup init
 * \brief
 *
 *      Detail
 *
 */
void load_harvestable_list();

/*!
 * \ingroup init
 * \brief
 *
 *      Detail
 *
 */
void load_entrable_list();

/*!
 * \ingroup loadsave
 * \brief
 *
 *      Detail
 *
 * \callgraph
 */
void read_config();

/*!
 * \ingroup loadsave
 * \brief
 *
 *      Detail
 *
 * \callgraph
 */
void read_bin_cfg();

/*!
 * \ingroup loadsave
 * \brief
 *
 *      Detail
 *
 */
void save_bin_cfg();

/*!
 * \ingroup cache
 * \brief
 *
 *      Detail
 *
 * \callgraph
 */
void init_md2_cache();

/*!
 * \ingroup cache
 * \brief
 *
 *      Detail
 *
 * \sa init_stuff
 */
void init_texture_cache();

/*!
 * \ingroup cache
 * \brief
 *
 *      Detail
 *
 * \callgraph
 */
void init_e3d_cache();

/*!
 * \ingroup init
 * \brief
 *
 *      Detail
 *
 * \sa init_stuff
 */
void init_2d_obj_cache();

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
 * \ingroup windows
 * \brief
 *
 *      Detail
 *
 */
void resize_root_window();

/*!
 * \ingroup load
 * \brief
 *
 *      Detail
 *
 * \sa init_stuff
 */
void load_e3d_list();

/*!
 * \ingroup load
 * \brief
 *
 *      Detail
 *
 * \sa start_rendering
 */
void unload_e3d_list();

/*!
 * a list of ids and associated function names used for e3d models
 */
typedef struct{
	int id;
	char *fn;
}e3d_list;
extern e3d_list *e3dlist;
extern int e3dlistsize;

extern int video_mode_set;

/*!
 * \ingroup init
 * \brief
 *
 *      Detail
 *
 * \sa init_stuff
 * \callgraph
 */
void read_command_line(); //from main.c

#endif	//__INIT_H__
