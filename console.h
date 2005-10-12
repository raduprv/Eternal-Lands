/*!
 * \file
 * \ingroup commands
 * \brief console related commands
 */
#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#ifndef DEF_INFO
#define DEF_INFO ""
#endif

extern char	auto_open_encyclopedia; /*!< flag, that indicates whether the encyclopedia window should be opened automatically upon startup of the client */

/*!
 * \brief Print the Eternal Lands version number
 *
 *	Print the Eternal Lands version in string \a str.
 *
 * \param str	the character buffer in which the string is placed
 * \param len	the size of the buffer
 * \callgraph
 */
void print_version_string (char *buf, int len);

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
