#ifndef __DIALOGUES_H__
#define __DIALOGUES_H__

#define MAX_RESPONSES 40

extern char dialogue_string[2048];
extern char npc_name[20];
extern int cur_portrait;

extern int portraits1_tex;
extern int portraits2_tex;
extern int portraits3_tex;
extern int portraits4_tex;
extern int portraits5_tex;

typedef struct{
	char text[200];
	int x_start;
	int y_start;
	int x_len;
	int y_len;
	int to_actor;
	int response_id;
	int in_use;
	int mouse_over;
}response;

extern response dialogue_responces[MAX_RESPONSES];
extern int dialogue_win;

extern int dialogue_menu_x;
extern int dialogue_menu_y;
extern int dialogue_menu_x_len;
extern int dialogue_menu_y_len;
//extern int dialogue_menu_dragged;

extern int no_bounding_box;

void build_response_entries(Uint8 *data,int total_lenght);
void display_dialogue();
void close_dialogue();

#endif

