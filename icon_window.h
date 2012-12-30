#ifndef __ICON_WINDOW_H
#define __ICON_WINDOW_H

#ifdef __cplusplus
extern "C" {
#endif

#include "platform.h"

int get_icons_win_active_len(void);

void init_newchar_icons(void);
void init_peace_icons(void);

extern int	icons_win;


/*!
 * \ingroup windows
 * \brief Frees the data used by the icons.
 *
 *      Frees the data used by \ref icon_list.
 */
void free_icons();

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
