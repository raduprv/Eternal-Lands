/*!
 * \file
 * \ingroup hotkey
 * \brief copy & paste handling
 */
#ifndef __PASTE_H__
#define __PASTE_H__

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
void do_paste(Uint8 * buffer);

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
void startpaste();

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

#else

/*!
 * \ingroup hotkey
 * \brief The callback used for pasting.
 *
 *      A callback function used for pasting. This function is specific to Windows.
 *
 * \callgraph
 */
void windows_paste();

#endif // not def WINDOWS
#endif // not def OSX
#endif // not def __PASTE_H__
