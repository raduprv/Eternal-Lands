#ifndef __BROWSER_H__
#define __BROWSER_H__

typedef struct{
	char DirName[24];
	char Files[256][24];
	int nf;
}_Dir;

extern int browser_menu_x;
extern int browser_menu_y;
extern int browser_menu_x_len;
extern int browser_menu_y_len;
extern int browser_menu_dragged;
extern int view_browser;

void init_browser();
void display_browser();
int check_browser_interface();

#endif

