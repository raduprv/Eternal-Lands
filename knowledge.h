#ifndef __KNOWLEDGE_H__
#define __KNOWLEDGE_H__

typedef struct
{
	Uint8 present;
	Uint8 mouse_over;
	char name[40];
}knowledge;

extern int knowledge_win;
extern int knowledge_menu_x;
extern int knowledge_menu_y;
extern int knowledge_menu_x_len;
extern int knowledge_menu_y_len;
//extern int knowledge_menu_dragged;
//extern int knowledge_scroll_dragged;
extern int knowledge_page_start;

extern knowledge knowledge_list[300];
extern char knowledge_string[400];

void display_knowledge();
//int knowledge_mouse_over();
//int check_knowledge_interface();
void get_knowledge_list(Uint16 size, char *list);
void get_new_knowledge(Uint16 idx);
#endif
