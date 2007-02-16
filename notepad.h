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
 * \brief Computes and returns the current edit position in the notepad window.
 *
 *      Gets the current edit position in the notepad window.
 *
 * \param x                 current x coordinate
 * \param y                 current y coordinate
 * \param str               the string to search for the current edit position
 * \param maxchar           the highest character index into \a str that will be considered
 * \param text_zoom         zoom factor for the text
 * \retval unsigned int     the edit position within \a str
 */
unsigned int get_edit_pos(int x, int y, char *str, unsigned int maxchar, float text_zoom);

/*!
 * \ingroup notepad_window
 * \brief   Displays a popup window
 *
 *      Displays a popup window using the given \a label.
 *
 * \param parent    id of the parent window
 * \param x         x coordinate of the position where the popup window will be shown
 * \param y         y coordinate of the position where the popup window will be shown
 * \param label     the label of the popup window
 * \param maxlen    the maximum length of the popup window text.
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
