#ifndef __ICON_WINDOW_H
#define __ICON_WINDOW_H

#ifdef __cplusplus
extern "C" {
#endif

#include "platform.h"

extern int	icons_win;	/*!< the icon window id  */

typedef enum { NEW_CHARACTER_ICONS=1, MAIN_WINDOW_ICONS=2 } icon_window_mode; /*!< possible icom window modes  */


/*!
 * \ingroup windows
 * \brief Reload the icon window from file.
 *
 * \param	all ignored
 * \return	always 1
 * \callgraph
 */
int reload_icon_window(char *text, int len);


/*!
 * \ingroup windows
 * \brief Return the current width of the icon window.
 *
 * \callgraph
 */
int get_icons_win_active_len(void);


/*!
 * \ingroup windows
 * \brief Return the current height of the icon window.
 *
 * \callgraph
 */
int get_icons_win_active_height(void);


/*!
 * \ingroup windows
 * \brief Initialise the icon window to the specified mode.
 *
 * \param	icon_mode	The new icom mode.
 *
 * \callgraph
 */
void init_icon_window(icon_window_mode icon_mode);


/*!
 * \ingroup windows
 * \brief Flash an icon.
 *
 *      Makes the specified icon flash between pressed/not press state.
 *
 * \param	title	the help text of the icon (to find it in the list).
 * \param	seconds	The number of seconds to flash.
 *
 * \callgraph
 */
void flash_icon(const char* name, Uint32 seconds);


/*!
 * \ingroup windows
 * \brief Set the icon size.
 *
 *      Set the icon size.  Can be called before creating the icon window.
 * If called after creation, the hud and root windows will need resizing
 * which must include recalling the init_icon_window() function. The
 * global_scaling factor will be applied.
 *
 * \param	icon_size	the size in pixels of one icon, both height & width.
 *
 * \callgraph
 */
void set_icon_size(int icon_size);


/*!
 * \ingroup windows
 * \brief Set the spacing between icons.
 *
 *      Set the spacing between icons. If other hud compoents use the
 * latest window width using get_icons_win_active_len(), no further
 * resizing should be necessary.  The global_scaling factor will be applied.
 *
 * \param	icon_spacing	The size in pixels of the space between icons.
 *
 * \callgraph
 */
void set_icon_spacing(int icon_spacing);


/*!
 * \ingroup windows
 * \brief Destroy the icon window.
 * \callgraph
 */
void destroy_icon_window(void);


/*!
 * \ingroup windows
 * \brief Enable or disable an icon in the icon window.
 *
 * \param	help_name	the help name of the window.
 * \param	enabled		true of we want to icon enabled.
 * \callgraph
 */
void set_icon_state(const char *help_name, int enabled);


#ifdef __cplusplus
} // extern "C"
#endif

#endif
