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

extern int qb_action_mode; /*!< flag indicating whether we are in quickbar action mode or not */

extern int show_stats_in_hud;

/*!
 * \name windows handlers
 */
/*! @{ */
extern int	quickbar_win; /*!< quickbar windows handler */
/*! @} */

extern int 	quickbar_relocatable; /*!< flag that indicates whether the quickbar is relocatable. */

/*!
 * \ingroup display_2d
 * \brief
 *
 *      Detail
 *
 * \callgraph
 */
void init_quickbar();

extern int hud_x;
extern int hud_y;

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

/*!
 * \ingroup other
 * \brief
 *
 *      Detail
 *
 * \sa init_stuff
 */
void build_levels_table();

#endif	//__HUD_H
