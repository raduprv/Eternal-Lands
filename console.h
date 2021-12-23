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

extern char	auto_open_encyclopedia; /*!< flag, that indicates whether the encyclopedia window should be opened automatically upon startup of the client */

extern int time_warn_h;	/*!< How many minutes before the new hour to give a warning */
extern int time_warn_s;	/*!< How many minutes before sunrise/sunset to give a warning */
extern int time_warn_d;	/*!< How many minutes before the new day to give a warning */

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

void add_name_to_tablist(const char *name);

void init_commands(const char *filename);

void add_line_to_history(const char *line, int len);
char *history_get_line_up(void);
char *history_get_line_down(void);
void history_reset(void);
void history_destroy(void);
void do_tab_complete(text_message *input);
void reset_tab_completer(void);

void auto_save_local_and_server(void);
void save_local_data(void);

int command_time(char *text, int len);
int command_date(char *text, int len);
int command_mark(char *text, int len);
int command_unmark_special(char *text, int len, int do_log);
int command_ping(char *text, int len);

void new_minute_console(void);

int summon_attack_is_active(void);
int summon_attack_is_unknown(void);
void check_summon_attack_mode(unsigned char *buffer, size_t len);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
