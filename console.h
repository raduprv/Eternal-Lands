/*!
 * \file
 * \ingroup commands
 * \brief console related commands
 */
#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include "text.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DEF_INFO
 #define DEF_INFO ""
#endif

typedef struct {
	char command[64];
	int (*callback)();
} command_t;

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
void print_version_string (char *buf, size_t len);

/*!
 * \brief checks whether a console command is waiting and executes it if necessary.
 *
 *      Checks whether a console command is waiting in the que and executes it if necessary.
 *
 * \param text The input line
 * \param len the length of the input line
 * \callgraph
 */

int test_for_console_command (char *text, int len);

void command_cleanup(void);

void add_command(const char *command, int (*callback)());
void add_name_to_tablist(const unsigned char *name);

void init_commands(const char *filename);

void add_line_to_history(const char *line, int len);
char *history_get_line_up(void);
char *history_get_line_down(void);
void history_reset(void);
void history_destroy(void);
void do_tab_complete(text_message *input);
void reset_tab_completer(void);

int save_local_data(char * text, int len);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
