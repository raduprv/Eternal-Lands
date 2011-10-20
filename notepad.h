/*!
 * \file
 * \ingroup notepad_window
 * \brief   Handling of the in-game notepad.
 */
#ifndef __NOTEPAD_H__
#define __NOTEPAD_H__

#include "widgets.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int notepad_win;    /*!< ID of the notepad window */
extern int popup_win;      /*!< ID of the popup window */
extern int notepad_loaded; /*!< boolean flag, indicating whether the notepad was loaded before. */
extern int notepad_win_x;  /*!< x-coordinate of the notepad position */
extern int notepad_win_y;  /*!< y-coordinate of the notepad position */
extern float note_zoom;    /*!< Size of the text in the note pad */

/* state structure for an input popup window */
typedef struct
{
	int popup_win, popup_field, popup_label, popup_ok, popup_no;
	int popup_x_len, popup_y_len, parent, x, y;
	int maxlen, rows, accept_do_not_close, allow_nonprint_chars;
	void (*popup_cancel)(void *);
	void (*popup_input)(const char *, void *);
	Uint32 text_flags;
	text_message popup_text;
	void *data;
} INPUT_POPUP;

/*!
 * \ingroup notepad_window
 * \brief   Initialise an input popup state structure
 *
 *      Initialise an input popup state structure using the given values.
 *
 * \param ipu    	pointer to the input popup window state structure
 * \param parent    id of the parent window
 * \param x_len     width of the window in pixels
 * \param y_len     base height of the window if rows is 1
 * \param maxlen    the maximum length of the popup window text.
 * \param rows      number of rows for the text widget
 * \param cancel	callback function if the window is cancelled (or NULL)
 * \param input		callback function to pass entered text (or (unusefully) NULL)
 * \callgraph
 */
void init_ipu (INPUT_POPUP *ipu, int parent, int x_len, int y_len, int maxlen, int rows, void cancel(void *), void input(const char *, void *));

/*!
 * \ingroup notepad_window
 * \brief   Hides and clears an open pop up window.
 *
 *      Hides and clears an open pop up window.
 *
 * \param ipu    	pointer to the input popup window state structure
 * \callgraph
 */
void clear_popup_window (INPUT_POPUP *ipu);

/*!
 * \ingroup notepad_window
 * \brief   Closes any open window.
 *
 *      Closes any open input popup window and frees the text buffer.
 *
 * \param ipu    	pointer to the input popup window state structure
 * \callgraph
 */
void close_ipu (INPUT_POPUP *ipu);

/*!
 * \ingroup notepad_window
 * \brief   Displays a popup window
 *
 *      Displays a popup window using the given \a label.
 *
 * \param ipu    	pointer to the input popup window state structure
 * \param label     the label of the popup window
 * \callgraph
 */
void display_popup_win (INPUT_POPUP *ipu, const char* label);

/*!
 * \ingroup notepad_window
 * \brief   Displays the in-game notepad.
 *
 *      Displays the in-game notepad within the tabbed window.
 *
 * \callgraph
 */
void fill_notepad_window(void);

/*!
 * \ingroup notepad_window
 * \brief   Saves the current content of the notepad window into a default file.
 *
 *      Saves the current notepad content into the file notes.xml.
 *
 * \retval int  always 1
 */
int notepad_save_file(void);

/*!
 * \ingroup notepad_window
 * \brief   Update the size of the text in the notepad text fields
 *
 * Update the size of the text in the notepad text fields
 */
void notepad_win_update_zoom ();

#ifdef __cplusplus
} // extern "C"
#endif

#endif
