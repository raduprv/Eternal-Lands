/*!
 * \file
 * \ingroup interface_console
 * \brief Declares functions to create and handle the console window
 */
#ifndef __CONSOLE_WIN__
#define __CONSOLE_WIN__

#define CONSOLE_SEP_HEIGHT	18

/*! \name windows handlers
 * @{ */
extern int console_root_win; /*!< handler for the console window */
/*! @} */

extern int console_in_id; /*!< ID of the console input widget */
extern int console_out_id; /*!< ID of the console output widget */

extern int locked_to_console; /*!< indicates whether the console win is locked. Used when we don't have any maps. */
extern int nr_console_lines;
extern int console_text_width;

/*!
 * \ingroup interface_console
 * \brief Signals the console window that the text buffer has changed
 *
 *      Signals the console window that the text buffer has changed
 *
 * \callgraph
 */
void update_console_win ();

/*!
 * \ingroup interface_console
 * \brief Creates and initializes the console window
 *
 *      Creates and initializes the console window
 *
 * \param width the width of the window
 * \param height the height of the window
 * \callgraph
 */
void create_console_root_window (int width, int height);

int input_field_resize(widget_list *w, Uint32 x, Uint32 y);

int history_grep(char * text, int len);

#endif // def __CONSOLE_WIN__
