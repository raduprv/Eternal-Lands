/*!
 * \file
 * \brief console related commands
 * \ingroup commands
 */
#ifndef __CONSOLE_H__
#define __CONSOLE_H__

extern char	auto_open_encyclopedia;

/*!
 * \brief Clears the screen.
 *
 *      Clears the screen. As a side effect the chat log get's cleared too.
 *
 * \return None
 */
void cls();

/*!
 * \brief prints the current log
 *
 *      Prints the current log
 *
 * \return None
 */
void print_log();

/*!
 * \brief checks whether a console command is waiting and executes it if necessary.
 *
 *      Checks whether a console command is waiting in the que and executes it if necessary.
 *
 * \return None
 */
void test_for_console_command();
#endif

