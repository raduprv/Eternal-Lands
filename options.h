#ifndef __OPTIONS_H__
#define __OPTIONS_H__


typedef struct
{
	char * name;
	char * desc;
	int type;
	int column;
	void (*func)(int*,int*);
	int * data_1;
	int * data_2;
} option_struct;

struct options_struct
{
	int no;
	option_struct * option[25];
};

extern int options_win;

extern int options_menu_x;
extern int options_menu_y;
extern int options_menu_x_len;
extern int options_menu_y_len;
//extern int options_menu_dragged;

void display_options_menu();
void init_display_options_menu();
void add_option(int type, char * name, char * desc, void * func, int * data_1, int * data_2, int column);
void change_option(int * data_1, int * data_2);
void move_to_full_screen(int  * unused, int * unused2);
void switch_video_modes(int * unused, int * mode);
void change_sound(int  * unused, int * unused2);
void change_music(int  * unused, int * unused2);

#define NONE 0		//0000b
#define OPTION 1	//0001b
#define VIDEO_MODE 2 	//0010b

#endif
