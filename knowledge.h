#ifndef __KNOWLEDGE_H__
#define __KNOWLEDGE_H__

typedef struct
{
   char name[16];
   unsigned char mouse_over;
}_knowledge;

void display_knowledge();
int knowledge_mouse_over();
int check_knowledge_interface();
void get_knowledge_list(char *list);

extern int knowledge_menu_x;
extern int knowledge_menu_y;
extern int knowledge_menu_x_len;
extern int knowledge_menu_y_len;
extern int knowledge_menu_dragged;


#endif
