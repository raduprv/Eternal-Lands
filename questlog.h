#ifndef __QUESTLOG_H__
#define __QUESTLOG_H__

typedef struct ld
{
	char *msg;
	struct ld *Next;
}_logdata;

extern int view_questlog;
extern int questlog_menu_x;
extern int questlog_menu_y;
extern int questlog_menu_x_len;
extern int questlog_menu_y_len;
extern int questlog_menu_dragged;

void display_questlog();
int check_questlog_interface();
void load_questlog();
void unload_questlog();
void add_questlog(char *t);
void string_fix(char *t);
#endif
