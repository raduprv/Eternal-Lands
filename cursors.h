#ifndef __CURSORS_H__
#define __CURSORS_H__

//cursors
#define CURSOR_EYE 0
#define CURSOR_TALK 1
#define CURSOR_ATTACK 2
#define CURSOR_ENTER 3
#define CURSOR_PICK 4
#define CURSOR_HARVEST 5
#define CURSOR_WALK 6
#define CURSOR_ARROW 7
#define CURSOR_TRADE 8
#define CURSOR_MAGIC 9
#define CURSOR_USE 10

#define UNDER_MOUSE_NPC 0
#define UNDER_MOUSE_PLAYER 1
#define UNDER_MOUSE_ANIMAL 2
#define UNDER_MOUSE_3D_OBJ 3
#define UNDER_MOUSE_NOTHING 4
#define UNDER_MOUSE_NO_CHANGE 5

extern actor *actor_under_mouse;
extern int object_under_mouse;
extern int thing_under_the_mouse;
extern int current_cursor;
extern int elwin_mouse;

struct cursors_struct
{
	int hot_x;
	int hot_y;
	Uint8 *cursor_pointer;
};
extern struct cursors_struct cursors_array[20];

struct harvest_names_struct
{
	char name[80];
};
extern struct harvest_names_struct harvestable_objects[100];

struct enter_names_struct
{
	char name[80];
};
extern struct enter_names_struct entrable_objects[100];

void load_cursors();
void assign_cursor(int cursor_id);
void change_cursor(int cursor_id);
void change_cursor_show(int cursor_id);
void build_cursors();
void check_cursor_change();

#endif
