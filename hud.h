/*!
 * \file
 * \ingroup windows
 * \brief handling and displaying the HUD
 */
#ifndef	__HUD_H
#define	__HUD_H

#include "elwindows.h"

/*!
 * \name orientation constants
 */
/*! @{ */
#define HORIZONTAL 2
#define VERTICAL 1
/*! @} */

/*!
 * structure to store the data and event handlers for an icon
 */
typedef struct
{
	int state; /*!< the state of the icon, some icons are toggable */
	/*!
     * \name icon image position
     */
    /*! @{ */
	float u[2];
	float v[2];
    /*! @} */
    
	char * help_message; /*!< icon help message */
    
    /*!
     * \name Function pointer and data
     */
    /*! @{ */
	int (*func)(void*, int);
	void * data;
    /*! @} */
    
	char data_type; /*!< data type indicator for \a data */
	char free_data; /*!< inidicator whether to free the data after use or not */
} icon_struct;

/*
 * OBSOLETE: Queued for removal from this file.
 * Only used in hud.c, no need to declare it here.
 */
//extern struct icons_struct icons; /*!< global variable of used icons */

extern int qb_action_mode; /*!< flag indicating whether we are in quickbar action mode or not */

extern int show_stats_in_hud;

/*
 * OBSOLETE: Queued for removal from this file.
 * Only used in hud.c, no need to declare them here.
 */
// The map icon:
//extern float map_icon_u_start;
//extern float map_icon_v_start;

/*!
 * \name windows handlers
 */
/*! @{ */

/*
 * OBSOLETE: Queued for removal from this file.
 * Only used in hud.c, no need to declare them here.
 */
//extern int	icons_win; /*!< icons window handler */
//extern int	stats_bar_win; /*!< stats bar windows handler */
//extern int	misc_win; /*!< misc. windows handler */

extern int	quickbar_win; /*!< quickbar windows handler */
/*! @} */

extern int 	quickbar_relocatable; /*!< flag that indicates whether the quickbar is relocatable. */

extern int hud_x;
extern int hud_y;

/*
 * OBSOLETE: Queued for removal from this file.
 * Unused variable or redundant declaration
 */
//extern int hud_y;
//extern int map_icon_x_start;
//extern int map_icon_y_start;
//extern int map_icon_x_end;
//extern int map_icon_y_end;

extern int view_digital_clock;

extern int quickbar_x;
extern int quickbar_y;
extern int quickbar_dir;
extern int quickbar_draggable;

// the main hud handling

/*!
 * \ingroup other
 * \brief
 *
 *      Detail
 *
 * \callgraph
 */
void init_hud_interface();

/*!
 * \ingroup other
 * \brief
 *
 *      Detail
 *
 * \callgraph
 */
void show_hud_windows ();

/*!
 * \ingroup other
 * \brief
 *
 *      Detail
 *
 * \callgraph
 */
void hide_hud_windows ();

/*!
 * \ingroup display_2d
 * \brief
 *
 *      Detail
 *
 * \callgraph
 */
void draw_hud_interface();

/*!
 * \ingroup windows
 * \brief
 *
 *      Detail
 *
 * \retval int
 * \callgraph
 */
int check_hud_interface();

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup other
// * \brief
// *
// *      Detail
// *
// * \sa init_hud_interface
// */
//void init_hud_frame();

/*!
 * \ingroup display_2d
 * \brief
 *
 *      Detail
 *
 * \callgraph
 */
void draw_hud_frame();

// icons subsection

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup other
// * \brief
// *
// *      Detail
// *
// * \callgraph
// */
//void init_peace_icons();

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup windows
// * \brief
// *
// *      Detail
// *
// * \retval int
// * \callgraph
// */
//int check_peace_icons();

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup windows
// * \brief
// *
// *      Detail
// *
// * \param u_start
// * \param v_start
// * \param colored_u_start
// * \param colored_v_start
// * \param help_message
// * \param func
// * \param data
// * \param data_type
// *
// * \sa init_peace_icons
// */
//void add_icon(float u_start, float v_start, float colored_u_start, float colored_v_start, char * help_message, void * func, void * data, char data_type);

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup windows
// * \brief
// *
// *      Detail
// *
// * \param no
// * \param ...
// */
//void set_icon_order(int no, ...);

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup windows
// * \brief
// *
// *      Detail
// *
// * \param id
// * \param state
// */
//void reset_states(int id, int state);

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup windows
// * \brief
// *
// *      Detail
// *
// * \param win_id
// * \retval int
// */
//int translate_win_id(int * win_id);

/*!
 * \ingroup windows
 * \brief
 *
 *      Detail
 *
 * \sa start_rendering
 */
void free_icons();

//Functions for the function pointers

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup network_actors
// * \brief
// *
// *      Detail
// *
// * \param mode
// * \param id
// *
// * \sa init_peace_icons
// */
//void switch_action_mode(int * mode, int id);

/*!
 * \ingroup windows
 * \brief
 *
 *      Detail
 *
 * \param unused
 * \param id
 */
void sit_button_pressed(void *unused, int id);

/*!
 * \ingroup windows
 * \brief
 *
 *      Detail
 *
 * \param win
 * \param id
 *
 * \callgraph
 */
void view_window(int * win, int id);

/*!
 * \ingroup windows
 * \brief
 *
 *      Detail
 *
 * \param window
 * \param col_id
 * \param tab
 *
 * \callgraph
 */
void view_tab (int *window, int *col_id, int tab);

/*!
 * \ingroup windows
 * \brief   views the console window (i.e. switch to console mode)
 *
 *      This is not handled by the window manager, so we have to call this function
 *
 * \param win
 * \param id
 *
 * \callgraph
 */
void view_console_win(int * win, int id);

/*!
 * \ingroup windows
 * \brief
 *
 *      Detail
 *
 * \param win
 * \param id
 *
 * \callgraph
 */
void view_map_win(int *win, int id);

/*!
 * \ingroup windows
 * \brief
 *
 *      Detail
 *
 * \param message
 * \param x
 * \param y
 *
 * \callgraph
 */
void show_help(char *message, int x, int y);

//stats/health section

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup other
// * \brief
// *
// *      Detail
// *
// * \callgraph
// */
//void init_stats_display();

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup windows
// * \brief
// *
// *      Detail
// *
// * \retval int
// */
//int check_stats_display();

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup display_2d
// * \brief
// *
// *      Detail
// *
// * \callgraph
// */
//void draw_exp_display();

/*!
 * \ingroup other
 * \brief
 *
 *      Detail
 *
 * \sa init_stuff
 */
void build_levels_table();

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup display_2d
// * \brief
// *
// *      Detail
// *
// * \callgraph
// */
//void draw_stats();

//misc section (compass, clock, ?)

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup other
// * \brief
// *
// *      Detail
// *
// * \callgraph
// */
//void init_misc_display();

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup windows
// * \brief
// *
// *      Detail
// *
// * \retval int
// * \callgraph
// */
//int check_misc_display();

//quickbar section

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup other
// * \brief
// *
// *      Detail
// *
// * \callgraph
// */
//void init_quickbar();

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup display_2d
// * \brief
// *
// *      Detail
// *
// * \callgraph
// */
//void draw_quickbar();

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup windows
// * \brief
// *
// *      Detail
// *
// * \retval int
// * \callgraph
// */
//int check_quickbar();

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup windows
// * \brief
// *
// *      Detail
// *
// * \callgraph
// */
//void flip_quickbar();

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup windows
// * \brief
// *
// *      Detail
// *
// * \callgraph
// */
//void reset_quickbar();

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup windows
// * \brief
// *
// *      Detail
// *
// * \param win_id
// * \param flags
// */
//void change_flags(int win_id, Uint32 flags);

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup windows
// * \brief
// *
// *      Detail
// *
// * \param win_id
// * \retval Uint32
// *
// * \sa click_quickbar_handler
// */
//Uint32 get_flags(int win_id);

#endif	//__HUD_H
