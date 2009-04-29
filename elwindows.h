#ifndef	__EL_WINDOWS_H
#define	__EL_WINDOWS_H

#include <SDL_types.h>

/*
 * A simple window handler setup to reduce the code needed to do windows
 *
 */

typedef	struct window_info
{
	int	window_id;	// the unique window id
	int	order;		// the order the windows are to be displayed (layering)
	int	pos_id;		// id of item position is compared to	//NOT SUPPORTED YET
	int	pos_loc;	// where is it compared to the pos id?	//NOT SUPPORTED YET
	int	pos_x, pos_y;	// logical location on screen
	int	len_x, len_y;	// the size of the window in pixels
	int	cur_x, cur_y;	// current location on screen

	Uint32	flags;

	float	back_color[4];		// r,g,b,a for the background
	float	border_color[4];	// r,g,b,a for the border
	float	line_color[4];		// r,g,b,a for any internal lines

	char	window_name[32];	// should be a unique name suitable for display

	char	displayed;	// is the window currently being displayed?
	//char	collapsed;	// is it collapsed or expanded?
	char	dragged;	// are we dragging the window?

	// the handlers
	int (*init_handler)(void*);		// init, scaling, etc
	int (*display_handler)(void*);	// display the window
	int (*click_handler)(void*, int, int, Uint32);		// handle mouse clicks
	int (*mouseover_handler)(void*, int, int);		// handle mouseovers

	/*
	// and optional list/data storage - future explansion??
	void	*list;
	int	list_size;	// width of list items
	int	num_list;	// number of items usable in list
	int	max_list;	// amount of space allocated in list
	int	data_value;	// a simple data value associated with this window
	*/
} window_info;

// Title bar & other constants
#define	ELW_TITLE_HEIGHT	16
#define	ELW_BOX_SIZE		20

// property flags in create
#define	ELW_TITLE_NONE	0x0000
#define	ELW_TITLE_BAR	0x0001
#define	ELW_TITLE_NAME	0x0002
#define	ELW_CLOSE_BOX	0x0004

#define	ELW_SHOW		0x0010
#define	ELW_DRAGGABLE	0x0020
//#define	ELW_COLLAPSABLE	0x0040
#define	ELW_SHOW_LAST	0x0080

#define	ELW_USE_BACKGROUND	0x0100
#define	ELW_USE_BORDER		0x0200
//#define	ELW_USE_LINES		0x0400

#define	ELW_WIN_DEFAULT	(ELW_TITLE_BAR|ELW_CLOSE_BOX|ELW_DRAGGABLE|ELW_USE_BACKGROUND|ELW_USE_BORDER|ELW_SHOW)
#define	ELW_WIN_INVISIBLE	(ELW_TITLE_NONE|ELW_SHOW)

// window position flags
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

// window alignment flags
#define	ELW_ALIGN_UL	((ELW_VUPPER|ELW_HLEFT)<<8)
#define	ELW_ALIGN_UC	((ELW_VUPPER|ELW_HCENTER)<<8)
#define	ELW_ALIGN_UR	((ELW_VUPPER|ELW_RIGHT)<<8)

#define	ELW_ALIGN_CL	((ELW_VCENTER|ELW_HLEFT)<<8)
#define	ELW_ALIGN_CC	((ELW_VCENTER|ELW_HCENTER)<<8)
#define	ELW_ALIGN_CR	((ELW_VCENTER|ELW_HRIGHT)<<8)

#define	ELW_ALIGN_LL	((ELW_VLOWER|ELW_HLEFT)<<8)
#define	ELW_ALIGN_LC	((ELW_VLOWER|ELW_HCENTER)<<8)
#define	ELW_ALIGN_LR	((ELW_VLOWER|ELW_HRIGHT)<<8)

#define	ELW_POS		ELW_POS_UL
#define	ELW_ALIGN	ELW_ALIGN_UL
#define	ELW_RELATIVE	(ELW_POS|ELW_ALIGN)

// window color id's
#define	ELW_COLOR_BACK	0
#define	ELW_COLOR_BORDER	1
#define	ELW_COLOR_LINE	2

// window handler id's
#define	ELW_HANDLER_INIT	0
#define	ELW_HANDLER_DISPLAY	1
#define	ELW_HANDLER_CLICK	2
#define	ELW_HANDLER_MOUSEOVER	3

// mouse click flags - first ones from events
#define	ELW_SHIFT	SHIFT
#define	ELW_CTRL	CTRL
#define	ELW_ALT		ALT
#define ELW_RIGHT_MOUSE	(1<<28)
#define ELW_MID_MOUSE	(1<<27)	// future expansion
#define ELW_LEFT_MOUSE	(1<<26)
#define ELW_DBL_CLICK	(1<<25)	// future expansion

typedef	struct	{
	window_info	*window;
	int	num_windows;	// highest item used
	int max_windows;	// number of windows allocated
	int	display_level;
} windows_info;

extern	windows_info	windows_list;
#define	SCREEN	0

// windows manager function
void	display_windows();
int		click_in_windows(int _x, int _y, Uint32 flags);
int		drag_windows(int _x, int _y, int dx, int dy);
void	end_drag_windows();
int		select_window(int win_id);
//void	close_windows();
//TODO: check windows on screen

// individual functions
int		create_window(const char *name, int pos_id, Uint32 pos_loc, int pos_x, int pos_y, int size_x, int size_y, Uint32 property_flags);
void	destroy_window(int win_id);
int		init_window(int win_id, int pos_id, Uint32 pos_loc, int pos_x, int pos_y, int size_x, int size_y);
int		move_window(int win_id, int pos_id, Uint32 pos_loc, int pos_x, int pos_y);
//int	set_window_property(int win_id, Uint32 property_flag, int new_property);
int		set_window_color(int win_id, Uint32 color_id, float r, float g, float b, float a);
int		use_window_color(int win_id, Uint32 color_id);
void	*set_window_handler(int win_id, int handler_id, int (*handler)() );
void	show_window(int win_id);
void	hide_window(int win_id);
void	toggle_window(int win_id);
int		get_show_window(int win_id);
//void	collapse_window(int win_id);	// future expansion
//void	expand_window(int win_id);		// future expansion

int	display_window(int win_id);
int	mouse_in_window(int win_id, int x, int y);	// is a coord in the window?
int	click_in_window(int win_id, int x, int y, Uint32 flags);	// click in  a coord in the window
int	mouseover_window(int win_id, int x, int y);	// do mouseover processing for a window

// low level functions
//window_info	*get_window_info(int win_id);
//window_info	*get_window_by_name(const Uint8 *name);
int	draw_window(window_info *win);		// the complete window, including display_handler
int	draw_window_title(window_info *win);// just the title bar if enabled
int	draw_window_base(window_info *win);	// border & background

// default handlers - VERY basic
//int	init_handler(window_info *win);
//int	display_handler(window_info *win);
//int	click_handler(window_info *win);
//int	mouseover_handler(window_info *win);

#endif	//__EL_WINDOWS_H

