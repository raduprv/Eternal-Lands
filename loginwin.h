#ifndef __LOGINWIN_H__
#define __LOGINWIN_H__

extern int login_root_win;

void set_login_error (const char *msg, int len);

void create_login_root_window (int width, int height);

#endif // def __LOGINWIN_H__
