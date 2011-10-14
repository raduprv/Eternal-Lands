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
 * \param[in] tf text_field to paste text to.
 */
void start_paste(text_field* tf);
#if !defined OSX && !defined WINDOWS
void start_paste_from_primary(text_field* tf);
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

#if !defined OSX && !defined WINDOWS

#include <X11/Xlib.h>

/*!
 * \ingroup hotkey
 * \brief Callback used when pasting is finishing.
 *
 *      A callback function used when pasting is finishing. This function is specific to the X Window system.
 *
 * \param event     the X Window event used for pasting
 *
 * \callgraph
 */
void finishpaste(XSelectionEvent event);

/*!
 * \brief called when SelectionRequest received, it sends selected text to requester.
 *
 * \param[in] e contains information about SelectionRequest event.
 */
void process_copy(XSelectionRequestEvent* e);

#endif // !def OSX && !def WINDOWS

/*!
 * \brief Paste the given string into the specified text widget
 *
 * \param tf text widget to receive string
 * \param text string to paste into widget
 */
void do_paste_to_text_field (text_field *tf, const char* text);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // not def __PASTE_H__
