/*!
 * \file
 * \ingroup hotkey
 * \brief copy & paste handling
 */
#ifndef __PASTE_H__
#define __PASTE_H__

/*!
 * \ingroup hotkey
 * \brief pastes the contents of the given buffer to the chat window
 *
 *      Pastes the contents of the given buffer to the chat window
 *
 * \param buffer    the data to paste
 * \return None
 */
void do_paste(Uint8 * buffer);

#ifndef WINDOWS
#include <X11/Xlib.h>

/*!
 * \ingroup hotkey
 * \brief callback used when pasting is started
 *
 *      A callback function used when pasting is started. This function is specific to the X Window system.
 *
 * \return None
 */
void startpaste();

/*!
 * \ingroup hotkey
 * \brief callback used when pasting is finishing.
 *
 *      A callback function used when pasting is finishing. This function is specific to the X Window system.
 *
 * \param event     the X Window event used for pasting
 * \return None
 */
void finishpaste(XSelectionEvent event);

#else

/*!
 * \ingroup hotkey
 * \brief the callback used for pasting.
 *
 *      A callback function used for pasting. This function is specific to Windows.
 *
 * \return None
 */
void windows_paste();
#endif

#endif

