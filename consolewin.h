/*!
 * \file
 * \ingroup interface_console
 * \brief Declares functions to create and handle the console window
 */
#ifndef __CONSOLE_WIN__
#define __CONSOLE_WIN__

/*! \name windows handlers
 * @{ */
extern int console_root_win; /*!< handler for the console window */
/*! @} */

extern int console_in_id; /*!< ID of the console input widget */

/*!
 * \ingroup interface_console
 * \brief signals the console window that the text buffer has changed
 *
 *      signals the console window that the text buffer has changed
 *
 * \callgraph
 */
void update_console_win ();

/*!
 * \ingroup interface_console
 * \brief creates and initializes the console window
 *
 *      Creates and initializes the console window
 *
 * \param width the width of the window
 * \param height the height of the window
 * \callgraph
 */
void create_console_root_window (int width, int height);

#endif // def __CONSOLE_WIN__
