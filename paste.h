/*!
 * \file
 * \ingroup hotkey
 * \brief copy & paste handling
 */
#ifndef __PASTE_H__
#define __PASTE_H__

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

#ifndef OSX
#ifndef WINDOWS

#include <X11/Xlib.h>
/*!
 * \ingroup hotkey
 * \brief Callback used when pasting is started
 *
 *      A callback function used when pasting is started. This function is specific to the X Window system.
 *
 */
void startpaste(void);

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
 * \brief this function is used to start paste to certain text_field.
 *
 * \param[in] tf text_field to paste text to.
 */
void start_paste_to_text_field(text_field* tf);

/*!
 * \brief this function is called when we copy selected text to clipboard.
 *
 * For X system it only copies selected text into buffer, which will be used by process_copy().
 */
void copy_to_clipboard(const char* text);

/*!
 * \brief called when SelectionRequest received, it sends selected text to requester.
 *
 * \param[in] e contains information about SelectionRequest event.
 */
void process_copy(XSelectionRequestEvent* e);

#else

/*!
 * \ingroup hotkey
 * \brief The callback used for pasting.
 *
 *      A callback function used for pasting. This function is specific to Windows.
 *
 * \callgraph
 */
void windows_paste(void);

#endif // not def WINDOWS
#endif // not def OSX

#ifdef __cplusplus
} // extern "C"
#endif

#endif // not def __PASTE_H__
