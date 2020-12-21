/*!
 * \file
 * \ingroup elwindows
 * \brief EL window manager.
 */
#ifndef	__EL_WINDOWS_H
#define	__EL_WINDOWS_H

#include <SDL_keycode.h>
#include "keys.h"
#include "widgets.h"

#ifdef __cplusplus
extern "C" {
#endif

//! The default color for GUI elements
extern const GLfloat gui_color[3];
extern const GLfloat gui_invert_color[3];
extern const GLfloat gui_bright_color[3];
extern const GLfloat gui_dull_color[3];

/*!
 * \name Title bar & other constants
 */
/*! @{ */
#define	ELW_TITLE_HEIGHT	16
#define	ELW_BOX_SIZE		20
#define ELW_TITLE_SIZE 35
#define ELW_CM_MENU_LEN		4
/*! @} */

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
	int scroll_id;		/*!< id of the scroll widget, if window is scrollable */
	int scroll_yoffset;	/*!< scroll bar will be placed below any close box, this is any additional y offset */

	Uint32	flags; /*!< window flags */

	float	back_color[4];		/*!< r,g,b,a for the background */
	float	border_color[4];	/*!< r,g,b,a for the border */
	float	line_color[4];		/*!< r,g,b,a for any internal lines */

	char	window_name[ELW_TITLE_SIZE];	/*!< should be a unique name suitable for display */

	char	displayed;	/*!< is the window currently being displayed? */
	//char	collapsed;	// is it collapsed or expanded?
	char	dragged;	/*!< are we dragging the window? */
	char	resized;	/*!< are we resizing the window? */
	char	drag_in;	/*!< are we dragging inside the window? */
	char	reinstate;	/*!< reinstate this window if the parent is shown again */
	int		opaque;		/*!< if non-zero, window is drawn opaque */
	char	owner_drawn_title_bar; /*the title bar is drawn by the window itself*/
	size_t	cm_id; 				/*!< optional context menu activated by right-clicking title */

	/*!
	 * \name scalable elements
	 */
	/*! @{ */
	float current_scale;
	float current_scale_small;
	float *custom_scale;
	int box_size;
	int title_height;
	font_cat font_category;
	int small_font_max_len_x;
	int small_font_len_y;
	int default_font_max_len_x;
	int default_font_len_y;

	/*!
	 * \name the handlers
	 */
	/*! @{ */
	int (*init_handler)();		/*!< init, scaling, etc */
	int (*display_handler)();	/*!< display the window */
	int (*pre_display_handler)();	/*!< display the window, before body (e.g. scissor) */
	int (*post_display_handler)();	/*!< display the window, after body (e.g. widgets, scissor) */
	int (*click_handler)();		/*!< handle mouse clicks */
	int (*drag_handler)();		/*!< handle dragging inside windows */
	int (*mouseover_handler)();	/*!< handle mouseovers */
	int (*resize_handler)();	/*!< handle window resize events */
	int (*keypress_handler)();	/*!< handle key presses */
	int (*close_handler)();		/*!< executed after window is closed */
	int (*destroy_handler)();	/*!< executed upon window destruction */
	int (*show_handler)();		/*!< executed before the window is shown */
	int (*after_show_handler)();		/*!< executed after the window is shown */
	int (*hide_handler)();		/*!< executed after the window is hidden */
	int (*ui_scale_handler)();	/*!< executed if the glabal scale ui_scale is changed */
	int (*font_change_handler)(); /*!< executed when font settings are changed */
	/*! @} */

	/*
	// and optional list/data storage - future expansion??
	void	*list;
	int	list_size;	// width of list items
	int	num_list;	// number of items usable in list
	int	max_list;	// amount of space allocated in list
	int	data_value;	// a simple data value associated with this window
	*/
	void * data; /*!< data for this window */
	widget_list *widgetlist; /*!< list of widgets for this window */
} window_info;

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
#define	ELW_USE_UISCALE		0x0800

#define ELW_CLICK_TRANSPARENT	0x1000

#define ELW_ALPHA_BORDER      0x2000
#define ELW_SWITCHABLE_OPAQUE 0x4000
#define ELW_SCROLLABLE        0x8000
/*! @} */

/*!
 * \name predefined window flags
 */
/*! @{ */
#define	ELW_WIN_DEFAULT (ELW_TITLE_BAR|ELW_CLOSE_BOX|ELW_DRAGGABLE|ELW_USE_BACKGROUND|ELW_USE_BORDER|ELW_SHOW|ELW_TITLE_NAME|ELW_ALPHA_BORDER|ELW_SWITCHABLE_OPAQUE)
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
#define	ELW_HANDLER_CLOSE	7
#define	ELW_HANDLER_DESTROY	8
#define	ELW_HANDLER_SHOW	9
#define	ELW_HANDLER_HIDE	10
#define	ELW_HANDLER_AFTER_SHOW	11
#define	ELW_HANDLER_PRE_DISPLAY	12
#define	ELW_HANDLER_POST_DISPLAY	13
#define ELW_HANDLER_UI_SCALE 14
#define ELW_HANDLER_FONT_CHANGE 15
/*! @} */

/*!
 * \name mouse click flags - first ones from events
 */
/*! @{ */
#define ELW_RIGHT_MOUSE		(1<<28)
#define ELW_MID_MOUSE		(1<<27)	// future expansion
#define ELW_LEFT_MOUSE		(1<<26)
#define ELW_MOUSE_BUTTON	(ELW_RIGHT_MOUSE|ELW_MID_MOUSE|ELW_LEFT_MOUSE)
#define ELW_DBL_CLICK		(1<<25)	// future expansion
#define ELW_WHEEL_UP		(1<<24)
#define ELW_WHEEL_DOWN		(1<<23)
#define ELW_WHEEL               (ELW_WHEEL_UP|ELW_WHEEL_DOWN)
#define ELW_MOUSE_BUTTON_WHEEL  (ELW_MOUSE_BUTTON|ELW_WHEEL)
/*! @} */

/*!
 * structure containing data for all windows used.
 */
typedef	struct	{
	window_info	*window; /*!< an array of \ref window_info windows */
	int	num_windows;	/*!< highest item used */
	int max_windows;	/*!< number of windows allocated */
	int	display_level;
} windows_info;

extern	windows_info	windows_list; /*!< global variable defining the list of windows */
extern int windows_on_top; /*!< global variable for whether windows appear on top of the console */
extern int top_SWITCHABLE_OPAQUE_window_drawn; /*!< the id of the top opaque switchable window */
extern int opaque_window_backgrounds;

/*!
 * \name managed window definitions, used to specify which window to access
 */
/*! @{ */
enum managed_window_enum
{
	MW_TRADE = 0,
	MW_ITEMS,
	MW_BAGS,
	MW_SPELLS,
	MW_STORAGE,
	MW_MANU,
	MW_EMOTE,
	MW_QUESTLOG,
	MW_INFO,
	MW_BUDDY,
	MW_STATS,
	MW_HELP,
	MW_RANGING,
	MW_ACHIEVE,
	MW_DIALOGUE,
	MW_QUICKBAR,
	MW_QUICKSPELLS,
	MW_CONFIG,
	MW_MINIMAP,
	MW_ASTRO,
	MW_TABMAP,
	MW_CONSOLE,
	MW_CHAT,
#ifdef ECDEBUGWIN
	MW_ECDEBUG,
#endif
	MW_MAX
};
/*! @} */

/*!
 * \name window creation and open functions, defined here to limit includes
 */
/*! @{ */
void display_trade_menu(void);
void display_items_menu(void);
void display_sigils_menu(void);
void display_storage_menu(void);
void display_manufacture_menu(void);
void display_emotes_menu(void);
void display_questlog(void);
void display_tab_info(void);
void display_buddy(void);
void display_tab_stats(void);
void display_tab_help(void);
void display_range_win(void);
void display_elconfig_win(void);
void display_minimap(void);
#ifdef ECDEBUGWIN
void display_ecdebugwin(void);
#endif
/*! @} */


// windows manager function

/*!
 * \name managed window access function
 */
/*! @{ */
int get_id_MW(enum managed_window_enum managed_win);
void set_id_MW(enum managed_window_enum managed_win, int win_id);
void set_pos_MW(enum managed_window_enum managed_win, int pos_x, int pos_y);
int get_pos_x_MW(enum managed_window_enum managed_win);
int get_pos_y_MW(enum managed_window_enum managed_win);
int on_top_responsive_MW(enum managed_window_enum managed_win);
int not_on_top_now(enum managed_window_enum managed_win);
void clear_was_open_MW(enum managed_window_enum managed_win);
void set_was_open_MW(enum managed_window_enum managed_win);
int was_open_MW(enum managed_window_enum managed_win);
int is_hideable_MW(enum managed_window_enum managed_win);
void clear_hideable_MW(enum managed_window_enum managed_win);
void set_hideable_MW(enum managed_window_enum managed_win);
void call_display_MW(enum managed_window_enum managed_win);
int match_keydef_MW(enum managed_window_enum managed_win, SDL_Keycode key_code, Uint16 key_mod);
float * get_scale_WM(enum managed_window_enum managed_win);
void set_save_pos_MW(enum managed_window_enum managed_win, int *pos_x, int *pos_y);
int * get_scale_flag_MW(void);
void toggle_window_MW(enum managed_window_enum managed_win);
int get_window_showable_MW(enum managed_window_enum managed_win);
enum managed_window_enum get_by_name_MW(const char *name);
const char *get_dict_name_WM(enum managed_window_enum managed_win, char *buf, size_t buf_len);
/*! @} */

/*!
 * \ingroup windows
 * \brief Check if the specified window has pending adjustments and position proportionably
 *
 * \param managed_win		the index of the window in the managed window array
 *
 * \callgraph
 */
void check_proportional_move(enum managed_window_enum managed_win);

/*!
 * \ingroup windows
 * \brief Adjust window positions proportionally using the supplied ratios
 *
 * \param pos_ratio_x		the ratio of the new window width over the old width
 * \param pos_ratio_y		the ratio of the new window height over the old height
 *
 * \callgraph
 */
void move_windows_proportionally(float pos_ratio_x, float pos_ratio_y);

/*!
 * \ingroup windows
 * \brief Adjusts window positions proportionally using window sizes from config
 *
 * \callgraph
 */
void restore_window_proportionally(void);

/*!
 * \ingroup elwindows
 * \brief   Set the window custom scale factor
 *
 *      This value is multiplied by the global scale value to
 * determine the specific scale used for this window.
 *
 * \param win_id    the id of the window to select
 * \param managed_win in index of the manage window which stores the scale
 * \callgraph
 */
void set_window_custom_scale(int win_id, enum managed_window_enum managed_win);

/*!
 * \ingroup elwindows
 * \brief   For each window using the specifed scale, update scaling.
 *
 * changed_window_custom_scale	pointer to the variable from custom_scale_factors_def
 * \callgraph
 */
void update_windows_custom_scale(float *changed_window_custom_scale);

/*!
 * \ingroup elwindows
 * \brief   Update scale settings for all windows
 *
 *      Update scale settings for all windows
 *
 * \param scale_factor     the scaling factor
 * \callgraph
 */
void update_windows_scale(float scale_factor);

/*!
 * \ingroup elwindows
 * \brief   Update scale settings for specific windows
 *
 *      Update scale settings for all windows
 *
 * \param win              pointer to window structure
 * \param scale_factor     the scaling factor
 * \callgraph
 */
void update_window_scale(window_info *win, float scale_factor);

/*!
 * \ingroup elwindows
 * \brief Handle a change in fonts
 *
 * Handle a change in font or text size for font category \a cat, in all windows.
 *
 * \param cat The font category that was changed,
 */
void change_windows_font(font_cat cat);

/*!
 * \ingroup elwindows
 * \brief   Displays all active windows
 *
 *      Displays all active windows
 *
 * \param level     the display level to display
 * \callgraph
 */
void	display_windows(int level);

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
 * \brief   Handles dragging and resizing of a window
 *
 *      Checks all the windows for one that is currently seletec and moves or resizes this one resp.
 *
 * \param mx        x coordinate of the mouse position
 * \param my        y coordinate of the mouse position
 * \param dx        amount in x direction to drag or resize the window
 * \param dy        amount in y direction to drag or resize the window
 * \retval int      the id of the window being dragged or resized
 * \callgraph
 */
int		drag_windows(int mx, int my, int dx, int dy);

/*!
 * \ingroup elwindows
 * \brief   callback function used when a key is pressed in a window.
 *
 *      This event handler gets called when a keypress is reported in a window. The coordinates \a mx and \a my denotes the (x,y) position of the mouse within the window.
 *
 * \param x         x coordinate of the mouse position where the click occurred
 * \param y         y coordinate of the mouse position where the click occurred
 * \param	key_code the SDL key code
 * \param	key_unicode the unicode representation of the key pressed
 * \param	key_mod the status bitmask for mod keys
 * \retval int
 * \callgraph
 */
int		keypress_in_windows(int x, int y, SDL_Keycode key_code, Uint32 key_unicode, Uint16 key_mod);

/*!
 * \ingroup elwindows
 * \brief   Resets the states of all windows which are related to dragging
 *
 *      Resets the dragged, resized and drag_in states of all windows.
 *
 */
void	end_drag_windows();

/*!
 * \ingroup elwindows
 * \brief   Selects the window given by \a win_id.
 *
 *      Selects the window given in \a win_id. The selected window and each of its childs, if applicable, is brought to front.
 *
 * \param win_id    the id of the window to select
 * \retval int  -1, if \a win_id is either <0 or greater than \ref windows_info::num_windows, or if \a win_id is not equal the \ref window_info::window_id stored for this \ref windows_info::window.
 *               0, if the \ref window_info::order of the \ref windows_info::window stored at this index is less than 0,
 *               else 1.
 *
 * \pre The parameter \a win_id must be both, greater or equal to 0 and less than \ref windows_info::num_windows
 * \pre The parameter \a win_id must be equal the \ref window_info::window_id which is currently stored in \ref windows_list.
 * \pre The \ref window_info::order of the \ref windows_info::window stored at the index \a win_id, must be greater than 0.
 */
int		select_window(int win_id);

// individual functions

/*!
 * \ingroup elwindows
 * \brief   Creates a new window and calls \ref init_window to place it.
 *
 *      Creates a new window with the given \a name and \a property_flags and then calls \ref init_window to place it accordingly.
 *
 * \param name              name of the window
 * \param pos_id            used to determine the visibility of this window, gets passed to \ref init_window
 * \param pos_loc           passed to \ref init_window
 * \param pos_x             passed to \ref init_window
 * \param pos_y             passed to \ref init_window
 * \param size_x            passed to \ref init_window
 * \param size_y            passed to \ref init_window
 * \param property_flags    the windows properties
 * \retval int              returns the window id as an index into the \ref windows_info::window array.
 * \callgraph
 *
 * \post    If this functions returns -1, it indicates an unhandled exception has occured.
 */
int		create_window(const char *name, int pos_id, Uint32 pos_loc, int pos_x, int pos_y, int size_x, int size_y, Uint32 property_flags);

/*!
 * \ingroup elwindows
 * \brief   Destroys the window with the given \a win_id.
 *
 *      Destroys the window with the given \a win_id and frees up memory.
 *
 * \param win_id    the index into the \ref windows_info::window array
 *
 * \pre If \a win_id is \<0 or \>\ref windows_info::num_windows this function returns without performing any actions.
 * \pre If the \ref window_info::window_id of the window stored at the index \a win_id into the \ref windows_list variable is not equal to \a win_id, this function returns without performing any actions.
 */
void	destroy_window(int win_id);

/*!
 * \ingroup elwindows
 * \brief   Initializes a new window.
 *
 *      Initializes a new window with the given \a win_id, position and size. Calls \ref move_window to place the window accordingly.
 *
 * \param win_id        the \ref window_info::window_id for the window
 * \param pos_id        the id of the position
 * \param pos_loc
 * \param pos_x         x coordinate of the upper left corner of the window
 * \param pos_y         y coordinate of the upper left corner of the window
 * \param size_x        width of the window
 * \param size_y        height of the window
 * \retval int
 * \sa move_window
 */
int		init_window(int win_id, int pos_id, Uint32 pos_loc, int pos_x, int pos_y, int size_x, int size_y);

/*!
 * \ingroup elwindows
 * \brief   Moves the window given by \a win_id
 *
 *      Moves the given window to the new position given.
 *
 * \param win_id        id for the window to move
 * \param pos_id        id of the new posisiotn
 * \param pos_loc
 * \param pos_x         x coordinate of the new position
 * \param pos_y         y coordinate of the new position
 * \retval int          -1, if \a win_id < 0 or \a win_id > \ref windows_info::num_windows,
 *                      or if the \ref window_info::window_id of the window at index \a win_id into \ref windows_list is not equal \a win_id,
 *                      else 1 is returned.
 *
 * \pre If \a win_id is less than 0, this functions returns -1 without performing any actions
 * \pre If \a win_id is greater than \ref windows_info::num_windows this function returns -1 without performing any actions
 * \pre If \a win_id is not equal \ref window_info::window_id of the window at the index \a win_id into \ref windows_list this functions returns -1, without performing any action.
 */
int		move_window(int win_id, int pos_id, Uint32 pos_loc, int pos_x, int pos_y);


/*!
 * \ingroup elwindows
 * \brief   Draws the window given by \a win_id
 *
 *      Draws the given window on the screen. Don't call this function directly,
 *	use the window manager instead. This declaration is only here for the
 *	loading window which bypasses the window manager since it cannot be
 *	drawn in the normal event loop.
 *
 * \param win_id        id for the window to move
 */
int display_window (int win_id);

//int	set_window_property(int win_id, Uint32 property_flag, int new_property);

/*!
 * \ingroup elwindows
 * \brief   Sets the window (background) color to the given values.
 *
 *      Sets the background color of the window given by \a win_id to the values given in \a r, \a g, \a b and \a a.
 *      \note This function is currently only used when displaying the options menu.
 *
 * \param win_id        the id of the window
 * \param color_id      an id for the color value
 * \param r             red value of the color (0<=r<=1)
 * \param g             green value of the color (0<=g<=1)
 * \param b             blue value of the color (0<=b<=1)
 * \param a             transparency of the color (0<=a<=1)
 * \retval int          0, if \a win_id is less than 0, or if \a win_id is greater than \ref windows_info::num_windows,
 *                      or if the \ref window_info::window_id of the window at index \a win_id into the \ref windows_list array is not equal to \a win_id,
 *                      or if no action was performed, else 1 is returned.
 *
 * \callgraph
 *
 * \pre If \a win_id < 0 this function returns 0 without performing any actions.
 * \pre If \a win_id is greater than \ref windows_info::num_windows, this functions returns 0 without performing any actions.
 * \pre If none of the above preconditions is true, but no action is performed else, this function returns 0.
 * \pre This functions only handles the \a color_id of \ref ELW_COLOR_BACK, \ref ELW_COLOR_BORDER and \ref ELW_COLOR_LINE.
 */
int	set_window_color(int win_id, Uint32 color_id, float r, float g, float b, float a);

/*!
 * \ingroup elwindows
 * \brief   Uses the color of the window \a win_id that is referred to by \a color_id.
 *
 *      The part of the window \a win_id given by \a color_id will be used further.
 *      \note This function is only used by the quickbar (\ref display_quickbar_handler)
 *
 * \param win_id        the id of the window to set the color
 * \param color_id      the color id to use
 * \retval int          0, if \a win_id is less than 0, or if \a win_id is greater than \ref windows_info::num_windows,
 *                      or if the \ref window_info::window_id of the window at index \a win_id into the \ref windows_list array is not equal to \a win_id,
 *                      or if no action was performed, else 1 is returned.
 *
 * \sa display_quickbar_handler
 *
 * \pre If \a win_id < 0 this function returns 0 without performing any actions.
 * \pre If \a win_id is greater than the \ref window_info::window_id of the window at index \a win_id into the \ref windows_list array, this function returns 0 without performing any actions.
 * \pre If none of the above preconditions is true, but no action is performed else, this function returns 0.
 * \pre This function only handles the \a color_id of \ref ELW_COLOR_BACK, \ref ELW_COLOR_BORDER and \ref ELW_COLOR_LINE.
 */
int		use_window_color(int win_id, Uint32 color_id);

/*!
 * \ingroup elwindows
 * \brief Sets the window's minimum size
 *
 *      Sets the minimum \a width and \a height for the resizeable window given by \a win_id
 *
 * \param win_id the number of the window
 * \param width the new minimum width
 * \param height the new minimum height
 * \retval int 0 on failure, 1 on success
 *
 * \pre If \a win_id < 0 this function returns 0, without performing any actions.
 * \pre If \a win_id is greater than \ref windows_info::num_windows this function returns 0 without performing any actions.
 * \pre If either \a width or \a height is less than 0 this function returns 0 without performing any actions.
 */
int set_window_min_size (int win_id, int width, int height);

/*!
 * \ingroup elwindows
 * \brief Sets one or more window flags
 *
 *     Sets the window flags in \a flag
 *
 * \param win_id the number of the window
 * \param flag the flag(s) to set
 */
int set_window_flag (int win_id, Uint32 flag);

/*!
 * \ingroup elwindows
 * \brief   Sets a new window handler callback function to be used for the given window.
 *
 *      The given \a handler will be set to be the callback function to handle \a handler_id callbacks for the \a win_id window.
 *
 * \param win_id        the id of the window for which to set a callback function.
 * \param handler_id    the type of handler that will be set.
 * \param handler       a pointer to the callback function to use.
 * \retval void*        a pointer to the old handler, or NULL.
 *
 * \pre If \a win_id < 0 this function returns NULL without performing any actions.
 * \pre If \a win_id  is greater than \ref windows_info::num_windows, this function returns NULL without performing any actions.
 * \pre If \a win_id is not equal the \ref window_info::window_id of the window at index \a win_id into the \ref windows_list array, this function returns NULL without performing any actions.
 * \pre If \a handler_id is not one of \ref ELW_HANDLER_INIT, \ref ELW_HANDLER_DISPLAY, \ref ELW_HANDLER_CLICK, \ref ELW_HANDLER_DRAG, \ref ELW_HANDLER_MOUSEOVER, \ref ELW_HANDLER_RESIZE, \ref ELW_HANDLER_KEYPRESS or \ref ELW_HANDLER_DESTROY, this function returns NULL.
 *
 */
void	*set_window_handler(int win_id, int handler_id, int (*handler)() );

/*!
 * \ingroup elwindows
 * \brief   Selects and shows the given window
 *
 *      Selects and shows the window given by \a win_id and all it's child windows where applicable.
 *
 * \param win_id    the id of the window to show
 *
 * \sa select_window
 *
 * \pre If \a win_id < 0 this function returns without performing any actions.
 * \pre If \a win_id is greater than \ref windows_info::num_windows, this function returns without performing any actions.
 * \pre If \a win_id is not equal the \ref window_info::window_id of the window at index \a win_id into the \ref windows_list array, this function returns without performing any actions.
 */
void	show_window(int win_id);

/*!
 * \ingroup elwindows
 * \brief   Hides the given window.
 *
 *      Hides the window given by \a win_id and all its child windows where applicable.
 *
 * \param win_id    the id of the window to hide.
 *
 * \pre If \a win_id < 0 this function returns without performing any actions.
 * \pre If \a win_id is greater than \ref windows_info::num_windows, this function returns without performing any actions.
 * \pre If \a win_id is not equal the \ref window_info::window_id of the window at index \a win_id into the \ref windows_list array, this function returns without performing any actions.
 */
void	hide_window(int win_id);

/*!
 * \ingroup elwindows
 * \brief   Toggles the visibility of the given window.
 *
 *      Toggles the visibility of the window given by \a win_id, by either calling \ref hide_window or \ref show_window.
 *
 * \param win_id    the id of the window to toggle.
 *
 * \callgraph
 *
 * \pre If \a win_id < 0 this function returns without performing any actions.
 * \pre If \a win_id is greater than \ref windows_info::num_windows, this function returns without performing any actions.
 * \pre If \a win_id is not equal the \ref window_info::window_id of the window at index \a win_id into the \ref windows_list array, this function returns without performing any actions.
 */
void	toggle_window(int win_id);

/*!
 * \ingroup elwindows
 * \brief   Sets the new size of a resizable window.
 *
 *      Resizes the window given by \a win_id to given \a new_width and \a new_height, if the window is resizable. Whether a window is resizable or not is determined by looking if the \ref window_info::resize_handler equals NULL or not.
 *
 * \param win_id        the id of the window to resize
 * \param new_width     the new width of the window
 * \param new_height    the new height of the window
 *
 * \callgraph
 *
 * \pre If \a win_id < 0 this function returns without performing any actions.
 * \pre If \a win_id is greater than \ref windows_info::num_windows, this function returns without performing any actions.
 * \pre If \a win_id is not equal the \ref window_info::window_id of the window at index \a win_id into the \ref windows_list array, this function returns without performing any actions.
 * \pre If \a new_width is less than the minimum width (\ref window_info::min_len_x) it will be adjusted accordingly before applying.
 * \pre If \a new_height is less than the minimum height (\ref window_info::min_len_y) it will be adjusted acoordingly before applying.
 */
void resize_window (int win_id, int new_width, int new_height);

/*!
 * \ingroup elwindows
 * \brief   Checks whether a window is currently displayed and returns a proper boolean value.
 *
 *      Returns the visibility status of the window given by \a win_id in a boolean value.
 *
 * \param win_id    the id of the window to check
 * \retval int      0 (false) if the window is hidden, else 1 (true).
 *
 * \pre If \a win_id < 0, this function returns false (0), without performing any actions.
 * \pre If \a win_id is greater than \ref windows_info::num_windows, this function returns false (0), without performing any actions.
 * \pre If \a win_id is not equal the \ref window_info::window_id of the window at index \a win_id into the \ref windows_list array, this function returns false (0), without performing any actions.
 */
int		get_show_window(int win_id);

/*!
 * \ingroup elwindows
 * \brief   Checks whether a window is currently displayable and returns a proper boolean value.
 *
 *      Checks if a window is either displayed or would be if its parent were visible
 *
 * \param win_id    the id of the window to check
 * \retval int      0 (false) if the window is hidden, else 1 (true).
 *
 * \pre If \a win_id < 0, this function returns false (0), without performing any actions.
 * \pre If \a win_id is greater than \ref windows_info::num_windows, this function returns false (0), without performing any actions.
 * \pre If \a win_id is not equal the \ref window_info::window_id of the window at index \a win_id into the \ref windows_list array, this function returns false (0), without performing any actions.
 */
int		get_window_showable(int win_id);

//void	collapse_window(int win_id);	// future expansion
//void	expand_window(int win_id);		// future expansion

/*!
 * \ingroup elwindows
 * \brief   Checks if the mouse coordinates are inside a window.
 *
 *      Checks whether the given mouse coordinates \a x and \a y are within the window given by \a win_id and returns either a boolean value or an error.
 *
 * \param win_id    the id of the window to check for the coordinates \a x and \a y
 * \param x         the x location of the mouse cursor to check
 * \param y         the y location of the mouse cursor to check
 * \retval int      -1, if either \a win_id < 0, or \a win_id is greater than \ref windows_info::num_windows,
 *                  or if \a win_id is not equal the \ref window_info::window_id of the window at index \a win_id into the \ref windows_list array.
 *                  0 (false), if the coordinates \a x and \a y are outside the window given by \a win_id,
 *                  else 1 (true).
 *
 * \pre If \a win_id < 0, this function returns -1, without performing any actions, indicating an error.
 * \pre If \a win_id is greater than \ref windows_info::num_windows, this function returns -1, without performing any actions, indicating an error.
 * \pre If \a win_id is not equal the \ref window_info::window_id of the window at index \a win_id into the \ref windows_list array, this function returns -1 without performing any actions, indicating an error.
 */
int		mouse_in_window(int win_id, int x, int y);	// is a coord in the window?

/*!
 * \ingroup elwindows
 * \brief   Checks if there is a click in a window at the given coordinates.
 *
 *      Checks whether a click occured inside the window given by \a win_id, at the coordinates \a x and \a y and returns a boolean value or an error.
 *
 * \param win_id    the id of the window to check
 * \param x         the x coordinate of the cursor position where the click occured
 * \param y         the y coordinate of the cursor position where the click occured
 * \param flags     the window flags of the window. They will be given to the \ref window_info::click_handler that handles the actual click event.
 * \retval int      -1, if either \a win_id < 0, or \a win_id is greater than \ref windows_info::num_windows,
 *                  of if \a win_id is not equal the \ref window_info::window_id of the given at index \a win_id into the \ref windows_list array.
 *                  1 (true), if the cursor is actualy inside the window,
 *                  else 0 (false).
 * \callgraph
 *
 * \pre If \a win_id < 0, this function returns -1, without performing any actions, indicating an error.
 * \pre If \a win_id is greater than \ref windows_info::num_windows, this function returns -1, without performing any actions, indicating an error.
 * \pre If \a win_id is not equal the \ref window_info::window_id of the window at index \a win_id into the \ref windows_list array, this function returns -1, without performing any actions, indicating an error.
 */
int		click_in_window(int win_id, int x, int y, Uint32 flags);	// click in  a coord in the window

/*!
 * \ingroup elwindows
 * \brief   Sets the length of the window's scrollbar
 *
 *      Sets the length of the window's scrollbar, ie. how many pixels you want it to scroll down.
 *
 * \param win_id    The id of the scrollable window
 * \param bar_len   The amount of pixels you want the bar to scroll.
 */
void set_window_scroll_len(int win_id, int bar_len);

/*!
 * \ingroup elwindows
 * \brief   Sets the scrollbar additional y offset
 *
 *      Normally the scrollbar is drawn from the top of the window or
 * 	just under the close bar (if one is in use).  Use this function
 * 	to move down further and have the size adjusted as required.
 *
 * \param win_id    The id of the scrollable window
 * \param yoffset   The number of pixels you want the bar moved down.
 */
void set_window_scroll_yoffset(int win_id, int yoffset);

/*!
 * \ingroup elwindows
 * \brief   Sets the scrollbar increment size
 *
 *      Set the number of pixels the window is moved up/down when the
 * 	scrollbar is moved one position.
 *
 * \param win_id    The id of the scrollable window
 * \param inc   The number of pixels moved when scrolling.
 */
void set_window_scroll_inc(int win_id, int inc);

/*!
 * \ingroup elwindows
 * \brief   Sets the scrollbar position
 *
 *      Sets the scrollbar position, i.e. the number of pixels
 * 	the window is scrolled down.
 *
 * \param win_id    The id of the scrollable window
 * \param pos   The new position for the scrollbar.
 */
void set_window_scroll_pos(int win_id, int pos);

/*!
 * \ingroup elwindows
 * \brief   Get the scrollbar position
 *
 *      Get the scrollbar position, i.e. the number of pixels
 * 	the window is currently scrolled down.
 *
 * \param win_id    The id of the scrollable window
 * \retval int   The number of the pixels the window is currently offset.
 */
int get_window_scroll_pos(int win_id);

/*!
 * \ingroup elwindows
 * \brief   Set the font category
 *
 * Set the font category for the text within the window with ID \a id window
 * to \a cat.
 *
 * \param win_id The ID of the  window
 * \param cat    The new font category
 * \return 1 if the font category was sucessfully set, 0 on failure
 */
int set_window_font_category(int win_id, font_cat cat);

/*!
 * \ingroup elwindows
 * \brief Get the content width of a window
 *
 * Get the content width of the window with identifier \a window_id. For non-scrollable windows,
 * this is the normal window width. For scrollable windows, the width of the scrollbar is subtracted.
 *
 * \param window_id The ID of the window
 * \return The content width of the window in pixels
 */
int get_window_content_width(int window_id);

/*!
 * \ingroup elwindows
 * \brief   The callback for context menu clicks
 *
 *      Called when an option is selected from the title context menu.  If
 *	the user window wants to use their own callback, they should still
 *  call this function to implement the title menu options.
 *
 * \param win    	Pointer to the windows structure
 * \param widget_id	The id of the widget clicked to open the menu or -1.
 * \param mx		The x coordinate in window of where the user clicked to open the window
 * \param my		The y coordinate as above
 * \param option	The menu line clicked, first line is 0
 * \retval int      1 if action was taken otherwise 0
*/
int cm_title_handler(window_info *win, int widget_id, int mx, int my, int option);

// low level functions
//window_info	*get_window_info(int win_id);
//window_info	*get_window_by_name(const Uint8 *name);

// default handlers - VERY basic
//int	init_handler(window_info *win);
//int	display_handler(window_info *win);
//int	click_handler(window_info *win);
//int	mouseover_handler(window_info *win);


/*!
 * \name managed window wrapper functions
 */
/*! @{ */
static inline void hide_window_MW(enum managed_window_enum managed_win) { hide_window(get_id_MW(managed_win)); }
static inline void show_window_MW(enum managed_window_enum managed_win) { show_window(get_id_MW(managed_win)); }
static inline int get_show_window_MW(enum managed_window_enum managed_win) { return get_show_window(get_id_MW(managed_win)); }
/*! @} */

#ifdef __cplusplus
} // extern "C"
#endif

#endif	//__EL_WINDOWS_H
