/*!
 * \file
 * \ingroup notepad_win
 * \brief   Handling of the in-game notepad.
 */
#ifndef __NOTEPAD_H__
#define __NOTEPAD_H__

#ifdef NOTEPAD
extern int notepad_loaded; /*!< boolean flag, indicating whether the notepad was loaded before. */

/*!
 * \ingroup notepad_win
 * \brief
 *
 *      Detail
 *
 * \param x
 * \param y
 * \param str
 * \param maxchar
 * \param text_zoom
 * \retval unsigned int
 */
unsigned int get_edit_pos(unsigned short x, unsigned short y, char *str, unsigned int maxchar, float text_zoom);

/*!
 * \ingroup notepad_win
 * \brief   Displays a popup window
 *
 *      Displays a popup window using the given \a label.
 *
 * \param label
 * \param maxlen
 * \callgraph
 */
void display_popup_win(char* label, int maxlen);

/*!
 * \ingroup notepad_win
 * \brief   Displays the in-game notepad window.
 *
 *      Displays the in-game notepad window. The window will be created if it was not used before.
 *
 * \callgraph
 */
void display_notepad();

/*!
 * \ingroup notepad_win
 * \brief   Saves the current content of the notepad window into a default file.
 *
 *      Saves the current notepad content into the file notes.xml.
 *
 * \retval int  always 1
 */
int notepadSaveFile();
#endif

#endif
