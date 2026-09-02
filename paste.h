/*!
 * \file
 * \ingroup hotkey
 * \brief copy & paste handling
 */
#ifndef __PASTE_H__
#define __PASTE_H__

#include <SDL_types.h>
#include "widgets.h"
#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \ingroup hotkey
 * \brief Pastes the contents of the given buffer to the chat window
 *
 *      Pastes the contents of the given buffer to the chat window
 *
 * \param buffer    the data to paste
 *
 * \callgraph
 */
void do_paste(const Uint8 * buffer);

/*!
 * \brief this function is used to start paste to certain text_field.
 *
 * \param[in] widget text widget to paste text to.
 */
void start_paste(widget_list *widget);
#if !defined OSX && !defined WINDOWS
void start_paste_from_primary(widget_list *widget);
#endif

/*!
 * \brief this function is called when we copy selected text to clipboard.
 *
 * For X system it only copies selected text into buffer, which will be used by process_copy().
 */
void copy_to_clipboard(const char* text);
#if !defined OSX && !defined WINDOWS
void copy_to_primary(const char* text);
#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif // not def __PASTE_H__
