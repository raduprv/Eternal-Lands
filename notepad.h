/*!
 * \file
 * \ingroup notepad_window
 * \brief   Handling of the in-game notepad.
 */
#ifndef __NOTEPAD_H__
#define __NOTEPAD_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef NOTEPAD
extern int notepad_win;    /*!< ID of the notepad window */
extern int notepad_loaded; /*!< boolean flag, indicating whether the notepad was loaded before. */
extern int notepad_win_x;  /*!< x-coordinate of the notepad position */
extern int notepad_win_y;  /*!< y-coordinate of the notepad position */
extern float note_zoom;    /*!< Size of the text in the note pad */

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
int notepad_save_file();

/*!
 * \ingroup notepad_window
 * \brief   Update the size of the text in the notepad text fields
 *
 * Update the size of the text in the notepad text fields
 */
void notepad_win_update_zoom ();
#endif // NOTEPAD

#ifdef __cplusplus
} // extern "C"
#endif

#endif
