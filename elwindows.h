/*!
 * \file
 * \ingroup elwindows
 * \brief EL window manager.
 */
#ifndef	__EL_WINDOWS_H
#define	__EL_WINDOWS_H

#include "keys.h"
#include "widgets.h"

/*!
 * A simple window handler setup to reduce the code needed to do windows
 *
 */

typedef	struct	{
	int	window_id;	/*!< the unique window id */
	int	order;		/*!< the order the windows are to be displayed (layering) */
	int	pos_id;		/*!< id of parent window, pos_id < 0 for normal windows */
	int	pos_loc;	/*!< where is it compared to the pos id?	NOT SUPPORTED YET */
	int	pos_x, pos_y;	/*!< logical location on screen */
	int	len_x, len_y;	/*!< the size of the window in pixels */
	int	min_len_x, min_len_y;	/*!< for resizable windows, the minimum width and height */
	int	cur_x, cur_y;	/*!< current location on screen */

	Uint32	flags; /*!< window flags */

	float	back_color[4];		/*!< r,g,b,a for the background */
	float	border_color[4];	/*!< r,g,b,a for the border */
	float	line_color[4];		/*!< r,g,b,a for any internal lines */

	char	window_name[35];	/*!< should be a unique name suitable for display */

	char	displayed;	/*!< is the window currently being displayed? */
	//char	collapsed;	// is it collapsed or expanded?
	char	dragged;	/*!< are we dragging the window? */
	char	resized;	/*!< are we resizing the window? */
	char	drag_in;	/*!< are we dragging inside the window? */

    /*!
	 * \name the handlers
     */
    /*! @{ */
	int (*init_handler)();		/*!< init, scaling, etc */
	int (*display_handler)();	/*!< display the window */
	int (*click_handler)();		/*!< handle mouse clicks */
	int (*drag_handler)();		/*!< handle dragging inside windows */
	int (*mouseover_handler)();		/*!< handle mouseovers */
	int (*resize_handler)();	/*!< handle window resize events */
	int (*keypress_handler)();	/*!< handle key presses */
    /*! @} */

	/*
	// and optional list/data storage - future explansion??
	void	*list;
	int	list_size;	// width of list items
	int	num_list;	// number of items usable in list
	int	max_list;	// amount of space allocated in list
	int	data_value;	// a simple data value associated with this window
	*/
	void * data; /*!< data for this window */
	widget_list widgetlist; /*!< list of widgets for this window */
} window_info;

/*!
 * \name Title bar & other constants
 */
/*! @{ */
#define	ELW_TITLE_HEIGHT	16
#define	ELW_BOX_SIZE		20
/*! @} */

/*!
 * \name property flags in create
 */
/*! @{ */
#define	ELW_TITLE_NONE	0x0000
#define	ELW_TITLE_BAR	0x0001
#define	ELW_TITLE_NAME	0x0002
#define	ELW_CLOSE_BOX	0x0004

#define	ELW_SHOW		0x0010
#define	ELW_DRAGGABLE	0x0020
//#define	ELW_COLLAPSABLE	0x0040
#define	ELW_SHOW_LAST	0x0080
#define ELW_RESIZEABLE	0x0100

#define	ELW_USE_BACKGROUND	0x0200
#define	ELW_USE_BORDER		0x0400
//#define	ELW_USE_LINES		0x0800
/*! @} */

/*!
 * \name predefined window flags
 */
/*! @{ */
#define	ELW_WIN_DEFAULT	(ELW_TITLE_BAR|ELW_CLOSE_BOX|ELW_DRAGGABLE|ELW_USE_BACKGROUND|ELW_USE_BORDER|ELW_SHOW)
#define	ELW_WIN_INVISIBLE	(ELW_TITLE_NONE|ELW_SHOW)
/*! @} */

/*!
 * \name window position flags
 */
/*! @{ */
#define	ELW_VUPPER	0x00
#define	ELW_VCENTER	0x04
#define	ELW_VLOWER	0x08
#define	ELW_VAUTO	0x0C	//reserved
#define	ELW_HLEFT	0x00
#define	ELW_HCENTER	0x01
#define	ELW_HRIGHT	0x02
#define	ELW_HAUTO	0x03	//reserved

#define	ELW_POS_UL	(ELW_VUPPER|ELW_HLEFT)
#define	ELW_POS_UC	(ELW_VUPPER|ELW_HCENTER)
#define	ELW_POS_UR	(ELW_VUPPER|ELW_HRIGHT)

#define	ELW_POS_CL	(ELW_VCENTER|ELW_HLEFT)
#define	ELW_POS_CC	(ELW_VCENTER|ELW_HCENTER)
#define	ELW_POS_CR	(ELW_VCENTER|ELW_HRIGHT)

#define	ELW_POS_LL	(ELW_VLOWER|ELW_HLEFT)
#define	ELW_POS_LC	(ELW_VLOWER|ELW_HCENTER)
#define	ELW_POS_LR	(ELW_VLOWER|ELW_HRIGHT)
/*! @} */

/*!
 * \name window alignment flags
 */
/*! @{ */
#define	ELW_ALIGN_UL	((ELW_VUPPER|ELW_HLEFT)<<8)
#define	ELW_ALIGN_UC	((ELW_VUPPER|ELW_HCENTER)<<8)
#define	ELW_ALIGN_UR	((ELW_VUPPER|ELW_RIGHT)<<8)

#define	ELW_ALIGN_CL	((ELW_VCENTER|ELW_HLEFT)<<8)
#define	ELW_ALIGN_CC	((ELW_VCENTER|ELW_HCENTER)<<8)
#define	ELW_ALIGN_CR	((ELW_VCENTER|ELW_HRIGHT)<<8)

#define	ELW_ALIGN_LL	((ELW_VLOWER|ELW_HLEFT)<<8)
#define	ELW_ALIGN_LC	((ELW_VLOWER|ELW_HCENTER)<<8)
#define	ELW_ALIGN_LR	((ELW_VLOWER|ELW_HRIGHT)<<8)
/*! @} */

/*!
 * \name predefined flags
 */
/*! @{ */
#define	ELW_POS		ELW_POS_UL
#define	ELW_ALIGN	ELW_ALIGN_UL
#define	ELW_RELATIVE	(ELW_POS|ELW_ALIGN)
/*! @} */

/*!
 * \name window color id's
 */
/*! @{ */
#define	ELW_COLOR_BACK	0
#define	ELW_COLOR_BORDER	1
#define	ELW_COLOR_LINE	2
/*! @} */

/*!
 * \name window handler id's
 */
/*! @{ */
#define	ELW_HANDLER_INIT	0
#define	ELW_HANDLER_DISPLAY	1
#define	ELW_HANDLER_CLICK	2
#define	ELW_HANDLER_DRAG	3
#define	ELW_HANDLER_MOUSEOVER	4
#define	ELW_HANDLER_RESIZE	5
#define	ELW_HANDLER_KEYPRESS	6
/*! @} */

/*!
 * \name mouse click flags - first ones from events
 */
/*! @{ */
#define	ELW_SHIFT	SHIFT
#define	ELW_CTRL	CTRL
#define	ELW_ALT		ALT
#define ELW_RIGHT_MOUSE	(1<<28)
#define ELW_MID_MOUSE	(1<<27)	// future expansion
#define ELW_LEFT_MOUSE	(1<<26)
#define ELW_DBL_CLICK	(1<<25)	// future expansion
/*! @} */

/*!
 * structure containing data for all windows used.
 */
typedef	struct	{
	window_info	*window; /*!< an array of \see window_info window */
	int	num_windows;	/*!< highest item used */
	int max_windows;	/*! number of windows allocated */
	int	display_level;
} windows_info;

extern	windows_info	windows_list; /*!< global variable defining the list of windows */

#define	SCREEN	0 /*!< defines the SCREEN */

// windows manager function

/*!
 * \ingroup elwindows
 * \brief   displays all active windows
 *
 *      Displays all active windows
 *
 */
void	display_windows();

/*!
 * \ingroup elwindows
 * \brief   callback function used when a mouse click happens in a window.
 *
 *      This event handler gets called when a mouse click is reported in a window. The coordinates \a mx and \a my denotes the (x,y) position of the mouse within the window.
 *
 * \param mx        x coordinate of the mouse position where the click occurred
 * \param my        y coordinate of the mouse position where the click occurred
 * \param flags     mouseflags
 * \retval int
 * \callgraph
 */
int		click_in_windows(int mx, int my, Uint32 flags);

/*!
 * \ingroup elwindows
 * \brief   callback function used when a drag event happens in a window.
 *
 *      This drag event handler get called when a mouse drag event happens in a window. (\a mx, \a my) denotes the coordinates of the mouse, while \a dx and \a dy are the dragging distances in x- and y-direction.
 *
 * \param mx        x coordinate of the mouse position where the drag started
 * \param my        x coordinate of the mouse position where the drag started
 * \param flags     mouseflags
 * \param dx        dragging distance in x-direction
 * \param dy        dragging distance in y-direction
 * \retval int
 * \callgraph
 */
int		drag_in_windows(int mx, int my, Uint32 flags, int dx, int dy);

/*!
 * \ingroup elwindows
 * \brief
 *
 *      Detail
 *
 * \param mx
 * \param my
 * \param dx
 * \param dy
 * \retval int
 * \callgraph
 */
int		drag_windows(int mx, int my, int dx, int dy);

/*!
 * \ingroup elwindows
 * \brief
 *
 *      Detail
 *
 */
void	end_drag_windows();

/*!
 * \ingroup elwindows
 * \brief
 *
 *      Detail
 *
 * \param win_id
 * \retval int
 */
int		select_window(int win_id);
//void	close_windows();

// individual functions

/*!
 * \ingroup elwindows
 * \brief
 *
 *      Detail
 *
 * \param name
 * \param pos_id
 * \param pos_loc
 * \param pos_x
 * \param pos_y
 * \param size_x
 * \param size_y
 * \param property_flags
 * \retval int
 * \callgraph
 */
int		create_window(const Uint8 *name, int pos_id, Uint32 pos_loc, int pos_x, int pos_y, int size_x, int size_y, Uint32 property_flags);

/*!
 * \ingroup elwindows
 * \brief
 *
 *      Detail
 *
 * \param win_id
 */
void	destroy_window(int win_id);

/*!
 * \ingroup elwindows
 * \brief
 *
 *      Detail
 *
 * \param name
 * \retval int
 */
int		find_window(const char *name);

/*!
 * \ingroup elwindows
 * \brief
 *
 *      Detail
 *
 * \param win_id
 * \param pos_id
 * \param pos_loc
 * \param pos_x
 * \param pos_y
 * \param size_x
 * \param size_y
 * \retval int
 * \callgraph
 */
int		init_window(int win_id, int pos_id, Uint32 pos_loc, int pos_x, int pos_y, int size_x, int size_y);

/*!
 * \ingroup elwindows
 * \brief
 *
 *      Detail
 *
 * \param win_id
 * \param pos_id
 * \param pos_loc
 * \param pos_x
 * \param pos_y
 * \retval int
 */
int		move_window(int win_id, int pos_id, Uint32 pos_loc, int pos_x, int pos_y);
//int	set_window_property(int win_id, Uint32 property_flag, int new_property);

/*!
 * \ingroup elwindows
 * \brief
 *
 *      Detail
 *
 * \param win_id
 * \param color_id
 * \param r
 * \param g
 * \param b
 * \param a
 * \retval int
 *
 * \sa display_options_menu
 */
int		set_window_color(int win_id, Uint32 color_id, float r, float g, float b, float a);

/*!
 * \ingroup elwindows
 * \brief
 *
 *      Detail
 *
 * \param win_id
 * \param color_id
 * \retval int
 *
 * \sa display_quickbar_handler
 */
int		use_window_color(int win_id, Uint32 color_id);

/*!
 * \ingroup elwindows
 * \brief
 *
 *      Detail
 *
 * \param win_id
 * \param handler_id
 * \param handler
 * \retval void*
 */
void	*set_window_handler(int win_id, int handler_id, int (*handler)() );

/*!
 * \ingroup elwindows
 * \brief
 *
 *      Detail
 *
 * \param win_id
 * \param handler_id
 * \retval void*
 */
void	*get_window_handler(int win_id, int handler_id);

/*!
 * \ingroup elwindows
 * \brief
 *
 *      Detail
 *
 * \param win_id
 *
 * \callgraph
 */
void	show_window(int win_id);

/*!
 * \ingroup elwindows
 * \brief
 *
 *      Detail
 *
 * \param win_id
 */
void	hide_window(int win_id);

/*!
 * \ingroup elwindows
 * \brief
 *
 *      Detail
 *
 * \param win_id
 *
 * \callgraph
 */
void	toggle_window(int win_id);

/*!
 * \ingroup elwindows
 * \brief
 *
 *      Detail
 *
 * \param win_id
 * \param new_width
 * \param new_height
 *
 * \callgraph
 */
void resize_window (int win_id, int new_width, int new_height);

/*!
 * \ingroup elwindows
 * \brief
 *
 *      Detail
 *
 * \param win_id
 * \retval int
 */
int		get_show_window(int win_id);
//void	collapse_window(int win_id);	// future expansion
//void	expand_window(int win_id);		// future expansion

/*!
 * \ingroup elwindows
 * \brief
 *
 *      Detail
 *
 * \param win_id
 * \retval int
 * \callgraph
 */
int		display_window(int win_id);

/*!
 * \ingroup elwindows
 * \brief
 *
 *      Detail
 *
 * \param win_id
 * \param x
 * \param y
 * \retval int
 */
int		mouse_in_window(int win_id, int x, int y);	// is a coord in the window?

/*!
 * \ingroup elwindows
 * \brief
 *
 *      Detail
 *
 * \param win_id
 * \param x
 * \param y
 * \param flags
 * \retval int
 * \callgraph
 */
int		click_in_window(int win_id, int x, int y, Uint32 flags);	// click in  a coord in the window

/*!
 * \ingroup elwindows
 * \brief
 *
 *      Detail
 *
 * \param win_id
 * \param x
 * \param y
 * \param flags
 * \param dx
 * \param dy
 * \retval int
 * \callgraph
 */
int		drag_in_window(int win_id, int x, int y, Uint32 flags, int dx, int dy);

/*!
 * \ingroup elwindows
 * \brief
 *
 *      Detail
 *
 * \param win_id
 * \param x
 * \param y
 * \retval int
 * \callgraph
 */
int		mouseover_window(int win_id, int x, int y);	// do mouseover processing for a window

// low level functions
//window_info	*get_window_info(int win_id);
//window_info	*get_window_by_name(const Uint8 *name);

/*!
 * \ingroup elwindows
 * \brief
 *
 *      Detail
 *
 * \param win
 * \retval int
 * \callgraph
 */
int		draw_window(window_info *win);		// the complete window, including display_handler

/*!
 * \ingroup elwindows
 * \brief
 *
 *      Detail
 *
 * \param win
 * \retval int
 * \callgraph
 */
int		draw_window_title(window_info *win);// just the title bar if enabled

/*!
 * \ingroup elwindows
 * \brief
 *
 *      Detail
 *
 * \param win
 * \retval int
 */
int		draw_window_base(window_info *win);	// border & background

// default handlers - VERY basic
//int	init_handler(window_info *win);
//int	display_handler(window_info *win);
//int	click_handler(window_info *win);
//int	mouseover_handler(window_info *win);

#endif	//__EL_WINDOWS_H
