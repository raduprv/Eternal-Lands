/*!
 * \file
 * \ingroup commands
 * \brief console related commands
 */
#ifndef __CONSOLE_H__
#define __CONSOLE_H__

extern char	auto_open_encyclopedia; /*!< flag, that indicates whether the encyclopedia window should be opened automatically upon startup of the client */

/*!
 * \brief Clears the screen.
 *
 *      Clears the screen. As a side effect the chat log get's cleared too.
 *
 */
void cls();

/*!
 * \brief prints the current log
 *
 *      Prints the current log
 *
 */
void print_log();

/*!
 * \brief checks whether a console command is waiting and executes it if necessary.
 *
 *      Checks whether a console command is waiting in the que and executes it if necessary.
 *
 * \param text The input line
 * \param len the length of the input line
 * \callgraph
 */
void test_for_console_command (char *text, int len);
#endif
