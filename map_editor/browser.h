#ifndef __BROWSER_H__
#define __BROWSER_H__

typedef struct{
	char DirName[32];
	char Files[256][256];
	char Names[256][32];
	int nf;
	float xrot[256],yrot[256],zrot[256],size[256];
}_Dir;

typedef struct{
	char Name[32];
	_Dir *Sub[44];
	int ns;
}_Cat;

extern int browser_menu_x;
extern int browser_menu_y;
extern int browser_menu_x_len;
extern int browser_menu_y_len;
//extern int browser_menu_dragged;
extern int view_browser;
extern int browser_win;
extern int close_browser_on_select;

void init_browser();
void display_browser();
int display_browser_handler();

#endif

