#ifndef __BUDDY_H__
#define __BUDDY_H__

typedef struct
{
   char name[32];//name of your buddy
   unsigned char type;
}_buddy;
#define	MAX_BUDDY	100

extern int buddy_menu_x;
extern int buddy_menu_y;
extern int buddy_menu_x_len;
extern int buddy_menu_y_len;
extern int buddy_win;

void init_buddy();
void display_buddy();
void add_buddy(char *n, int t, int len);
void del_buddy(char *n, int len);
#endif

