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

#ifdef __cplusplus
} // extern "C"
#endif

#endif
