/*!
 * \file
 * \ingroup notepad_window
 * \brief   Handling of the in-game notepad.
 */
#ifndef __NOTEPAD_H__
#define __NOTEPAD_H__

#ifdef NOTEPAD
extern int notepad_win; /*!< ID of the notepad window */
extern int notepad_loaded; /*!< boolean flag, indicating whether the notepad was loaded before. */

/*!
 * \ingroup notepad_window
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
 * \ingroup notepad_window
 * \brief   Displays a popup window
 *
 *      Displays a popup window using the given \a label.
 *
 * \param parent
 * \param x
 * \param y
 * \param label
 * \param maxlen
 * \callgraph
 */
void display_popup_win(int parent, int x, int y, char* label, int maxlen);

/*!
 * \ingroup notepad_window
 * \brief   Displays the in-game notepad window.
 *
 *      Displays the in-game notepad window. The window will be created if it was not used before.
 *
 * \callgraph
 */
void display_notepad();

/*!
 * \ingroup notepad_window
 * \brief   Saves the current content of the notepad window into a default file.
 *
 *      Saves the current notepad content into the file notes.xml.
 *
 * \retval int  always 1
 */
int notepadSaveFile();
#endif

#endif
