#ifndef __BUDDY_H__
#define __BUDDY_H__

typedef struct
{
   char name[16];//name of your buddy
   unsigned char type;
}_buddy;

extern int buddy_menu_x;
extern int buddy_menu_y;
extern int buddy_menu_x_len;
extern int buddy_menu_y_len;
extern int buddy_menu_dragged;
extern int buddy_win;

void init_buddy();
void display_buddy();
void add_buddy(char *n, int t);
void del_buddy(char *n);
#endif

