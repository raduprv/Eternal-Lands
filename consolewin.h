/*!
 * \file
 * \ingroup interface_console
 * \brief Declares functions to create and handle the console window
 */
#ifndef __CONSOLE_WIN__
#define __CONSOLE_WIN__

/*! \name windows handlers
 * @{ */
extern int console_win; /*!< handler for the console window */
/*! @} */

#ifdef WINDOW_CHAT

/*!
 * \ingroup interface_console
 * \brief creates and initializes the console window
 *
 *      Creates and initializes the console window
 *
 * \callgraph
 */
void create_console_window ();

#endif

#endif // def __CONSOLE_WIN__
