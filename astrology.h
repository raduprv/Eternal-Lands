#ifndef __ASTROLOGY_H__
#define __ASTROLOGY_H__

#include "items.h"
#include "elwindows.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int astrology_win_x;
extern int astrology_win_y;
extern int astrology_win_x_len;
extern int astrology_win_y_len;

extern int astrology_win;
extern int always_show_astro_details;

/*!
 * \ingroup astrology_window
 * \brief   Displays the astrology window.
 *
 *      Displays the astrology window. The window will be created if it was not used before.
 *
 * \callgraph
 */
void display_astrology_window();

/*!
 * \brief Check for astrology messages
 *
 *	Check if a text message from the server is an astrology message,
 *	and if so, update and show the astrology window.
 *
 * \param RawText The text to check.
 * \return 1 if the message is an astrology message, 0 otherwise.
 */
int is_astrology_message (const char* RawText);

/*!
 * \brief Free astro window memeory
 *
 */
void free_astro_buffer();

#ifdef __cplusplus
} // extern "C"
#endif

#endif //__ASTROLOGY_H__
