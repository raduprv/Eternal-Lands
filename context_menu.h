#if !defined(CONTEXMENU_H)
#define CONTEXMENU_H

#include "elwindows.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* define an initialisation value for context menu id */
#define CM_INIT_VALUE ((size_t) -1)


/*!
 * \ingroup context_menu
 * \brief Creates a new context menu.
 *
 *      Creates a new context menu with the returned unique id.  This id identifies the menu
 *	when control functions are called.  The menu_list and handler can be modified using the
 *	\ref cm_set function.  If the menu was opened from an associated window, the window
 *	window_info pointer is supplied to the handler; then any associated widget id, followed by the 
 * 	x and y mouse coordinates in the window where the right-click was done.  The last 
 * 	parameter is the selected menu line.
 *	No error codes are returned by this function.
 *
 * \param menu_list		\n separated list of menu entries.  Use "--" to specify a separator line.
 * \param handler		optional function to call on menu line selection
 * \retval size_t		returns the unique context menu id
 */
size_t cm_create(const char *menu_list, int (*handler)(window_info *, int, int, int, int));


/*!
 * \ingroup context_menu
 * \brief Deletes a context menu and all its activation entries; i.e. full_window, region or widgets.
 *
 * \param cm_id			id of context menu
 * \retval int 			1 for success, 0 for failure (invalid id)
 */
int cm_destroy(size_t cm_id);


/*!
 * \ingroup context_menu
 * \brief Test if a context menu id is valid.
 *
 * \param cm_id			id of context menu
 * \retval int 			1 for success, 0 for failure (invalid id)
 */
int cm_valid(size_t cm_id);


/*!
 * \ingroup context_menu
 * \brief Open a context menu if there is a valid activation for the specified window.
 *
 *		Activations are created using the \ref cm_add_window \ref cm_add_region and \ref cm_add_widget
 * functions.  If any exist for the specified window and the mouse is in a valid position, the context
 * menu is display.
 *
 * \param window_id		id of window to check for activations
 * \retval int 			1 for success a cm was opened , 0 for failure (invalid id) or no window opened
 */
int cm_show_if_active(int window_id);


/*!
 * \ingroup context_menu
 * \brief Opens a context menu without checking for window activation entries.
 *
 * \param  cm_id		id of context menu
 * \param  window_id	id of window to treat as parent used for callback or -1 for no id
 * \param  widget_id	id of widget used for callback or - -1 for no id
 * \retval int 			1 for success, 0 for failure (invalid id)
 */
int cm_show_direct(size_t cm_id, int window_id, int widget_id);


/*!
 * \ingroup context_menu
 * \brief Called by \ref click_in_windows to prepare for activation checks.
 * \retval int 			1 if the activation mouse state is true (i.e. right click), otherwise 0
 */
int cm_pre_show_check(Uint32 flags);


/*!
 * \ingroup context_menu
 * \brief Called by \ref click_in_windows to closes/hides any open context menu. 
 * \param  force	if true always close regardless of internal state
 */
void cm_post_show_check(int force);


/*!
 * \ingroup context_menu
 * \brief Replace all context menu lines and the callback function.
 * \param  cm_id		id of context menu
 * \param menu_list		\n separated list of menu entries.  Use "--" to specify a separator line.
 * \param handler		optional function to call on menu line selection
 * \retval int 			1 for success, 0 for failure (invalid id)
 */
int cm_set(size_t cm_id, const char *menu_list, int (*handler)(window_info *, int, int, int, int));


/*!
 * \ingroup context_menu
 * \brief Add additional menu lines and optionally replace the callback function.
 * \param  cm_id		id of context menu
 * \param menu_list		\n separated list of menu entries.  Use "--" to specify a separator line.
 * \param handler		optional function to call on menu line selection
 * \retval int 			1 for success, 0 for failure (invalid id)
 */
int cm_add(size_t cm_id, const char *menu_list, int (*handler)(window_info *, int, int, int, int));


/*!
 * \ingroup context_menu
 * \brief Add/replacee the pre-show callback function.
 * \param  cm_id		id of context menu
 * \param handler		function to call on just before the menu is shown
 * \retval int 			1 for success, 0 for failure (invalid id)
 */
int cm_set_pre_show_handler(size_t cm_id, void (*handler)(window_info *, int, int, int, window_info *));


/*!
 * \ingroup context_menu
 * \brief Set the border and zoom properties of the specified context menu.
 * \param  cm_id		id of context menu
 * \param  border		window border size, no hightlight
 * \param  text_border	border around text, hightlight extended to cover
 * \param  line_sep		line spacing
 * \param  zoom			text font zoom
 * \retval int 			1 for success, 0 for failure (invalid id)
 */
int cm_set_sizes(size_t cm_id, int border, int text_border, int line_sep, float zoom);


/*!
 * \ingroup context_menu
 * \brief Enumerated type for context enu colour properties.
 */
enum CM_COLOUR_NAME { CM_HIGHLIGHT_TOP, CM_HIGHLIGHT_BOTTOM, CM_TEXT, CM_GREY };


/*!
 * \ingroup context_menu
 * \brief Set a context menu colour value
 * \param  cm_id		id of context menu
 * \param  colour_name	\ref CM_COLOUR_NAME property to modify
 * \param  r			red value
 * \param  g			green value
 * \param  b			blue value
 * \retval int 			1 for success, 0 for failure (invalid id)
 */
int cm_set_colour(size_t cm_id, enum CM_COLOUR_NAME colour_name, float r, float g, float b);


/*!
 * \ingroup context_menu
 * \brief  Make a context menu line a tick box option.
 * 
 *		When the menu line is selected, the control variable value is toggled.  Also, if the
 * control variable is modified elsewhere, the context menu dynamically/automatically reflects
 * the state.  This can be used for \ref elconfig options.
 *
 * \param  cm_id		id of context menu
 * \param  line			the menu line number to change, range 0 - (number lines - 1)
 * \param  control_var	the address of the control variable
 * \paraam config_name	if not NULL, the elconfig option name - used to set unsaved
 * \retval int 			1 for success, 0 for failure (invalid id)
 */
int cm_bool_line(size_t cm_id, size_t line, int *control_var, const char *config_name);


/*!
 * \ingroup context_menu
 * \brief Enable/disable a context menu line (disable - grey it out).
 * \param  cm_id		id of context menu
 * \param  is_grey		if true, the line is disabled, otherwise enabled
 * \retval int 			1 for success, 0 for failure (invalid id)
 */
int cm_grey_line(size_t cm_id, size_t line, int is_grey);


/*!
 * \ingroup context_menu
 * \brief Add an activation entry for an entire window.
 *
 *		If the activation mouse click is done anywhere in the window the
 * context menu is shown.  This activation point can be removed using
 * the \ref cm_remove_window function.
 *
 * \param  cm_id		id of context menu
 * \param  window_id	id of window to add activation
 * \retval int 			1 for success, 0 for failure (invalid id or window)
 */
int cm_add_window(size_t cm_id, int window_id);


/*!
 * \ingroup context_menu
 * \brief Add an activation entry for a window region
 *
 *		If the activation mouse click is done anywhere in the region
 * within the window the context menu is shown.  This activation point
 * can be removed using the \ref cm_remove_regions function.  A region
 * activation point overides a window activation point.
 *
 * \param  cm_id		id of context menu
 * \param  window_id	id of window containing the region
 * \param  posx			top left x coordinate in window
 * \param  posy			top left y coordinate in window
 * \param  lenx			width of region
 * \param  leny			height of region
 * \retval int 			1 for success, 0 for failure (invalid id or window)
 */
int cm_add_region(size_t cm_id, int window_id, int posx, int posy, int lenx, int leny);


/*!
 * \ingroup context_menu
 * \brief Add an activation entry over a window widget
 *
 *		If the activation mouse click is done anywhere on the widget
 * within the window the context menu is shown.  This activation point
 * can be removed using the \ref cm_remove_widget function.  A widget
 * activation point overides both region and window activation points.
 *
 * \param  cm_id		id of context menu
 * \param  window_id	id of window containing the widget
 * \param  widget_id	id of widget
 * \retval int 			1 for success, 0 for failure (invalid id, window or widget)
 */
int cm_add_widget(size_t cm_id, int window_id, int widget_id);


/*!
 * \ingroup context_menu
 * \brief Removes the activation for the specified window
 * \param  window_id	id of window to remove
 * \retval int 			1 for success, 0 for failure (invalid window)
 */
int cm_remove_window(int window_id);


/*!
 * \ingroup context_menu
 * \brief Remove all activation regions for the specified window
 * \param  window_id	id of window containing regions
 * \retval int 			1 for success, 0 for failure (invalid window)
 */
int cm_remove_regions(int window_id);


/*!
 * \ingroup context_menu
 * \brief Remove activation for the specified window/widget
 * \param  window_id	id of window containing the widget
 * \param  widget_id	id of widget
 * \retval int 			1 for success, 0 for failure (invalid window)
 */
int cm_remove_widget(int window_id, int widget_id);


/*!
 * \ingroup context_menu
 * \brief Show information about the context menu system state
 */
void cm_showinfo(void);


/*!
 * \ingroup context_menu
 * \brief Return the id the currently open context menu or CM_INIT_VALUE if none open.
 */
size_t cm_window_shown(void);


/*!
 * \ingroup context_menu
 * \brief Set the data pointer assiociated with a menu
 * \param  cm_id	id of the assiociated context menu
 * \param  data 	the pointer to save
 */
void cm_set_data(size_t cm_id, void *data);


/*!
 * \ingroup context_menu
 * \brief Get the previously saved data pointer assiociated with a menu
 * \param  cm_id	id of the assiociated context menu
 * \retval void * 	NULL or the pointer previously set
 */
void *cm_get_data(size_t cm_id);


#ifdef __cplusplus
}
#endif

#endif
