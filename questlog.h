#ifndef __QUESTLOG_H__
#define __QUESTLOG_H__

typedef struct ld
{
	char *msg;
	struct ld *Next;
}_logdata;

extern int questlog_win;
extern int questlog_menu_x;
extern int questlog_menu_y;
extern int questlog_menu_x_len;
extern int questlog_menu_y_len;

void display_questlog();
void load_questlog();
void unload_questlog();
void add_questlog(char *t, int len);
void add_questlog_line(char *t, int len);
void goto_questlog_entry(int ln);
void string_fix(char *t, int len);

#endif	//__QUESTLOG_H__

