#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "elwindows.h"

#define	MAX_BUDDY	100

int buddy_win=-1;
int buddy_scroll_id = 0;
int buddy_button_id = 1;
int buddy_menu_x=150;
int buddy_menu_y=70;
int buddy_menu_x_len=150;
int buddy_menu_y_len=220;

int buddy_add_win = -1;
int buddy_name_input_id = 0;
int buddy_type_input_id = -1;
int buddy_add_button_id = -1;
int buddy_add_x_len = 290;
int buddy_add_y_len = 225;
int buddy_add_y_pos = 0;
unsigned char buddy_name_buffer[16] = {0};
char description_buffer[255] = {0};
_buddy buddy_list[MAX_BUDDY];

int compare2( const void *arg1, const void *arg2)
{
	_buddy *b1=(_buddy*)arg1, *b2=(_buddy*)arg2;
	if(b1->type==b2->type)
		return strcmp(b1->name,b2->name);
	else
		return b1->type>b2->type ? 1: -1;
}

int display_buddy_handler(window_info *win)
{
	int i=0,x=2,y=2;
	glEnable(GL_TEXTURE_2D);
	// Draw budies
	qsort(buddy_list,MAX_BUDDY,sizeof(_buddy),compare2);
	for (i = vscrollbar_get_pos (buddy_win,buddy_scroll_id); i < vscrollbar_get_pos(buddy_win,buddy_scroll_id) + 19; i++)
	{
		switch(buddy_list[i].type){
			case 0:glColor3f(1.0,1.0,1.0);break;
			case 1:glColor3f(1.0,0,0);break;
			case 2:glColor3f(0,1.0,0);break;
			case 3:glColor3f(0,0,1.0);break;
			case 4:glColor3f(1.0,1.0,0);break;
		}
		draw_string_zoomed(x,y,buddy_list[i].name,1,0.7);
		y+=10;
	}
	return 1;
}

int click_buddy_handler (window_info *win, int mx, int my, Uint32 flags)
{
	int x=mx,y=my;
	char str[50];

	// only handle mouse button clicks, not scroll wheels moves
	if ( (flags & ELW_MOUSE_BUTTON) == 0)
		return 0;

	if(x>win->len_x-20) {
		//Clicked on the scrollbar. Let it fall through.
		return 0;
	}
	
	// clicked on a buddy's name
	y /= 10;
	y += vscrollbar_get_pos(buddy_win,buddy_scroll_id);
	if(strlen(buddy_list[y].name) == 0) {
		//There's no name here. Fall through.
		return 0;
	}
	if(flags&ELW_RIGHT_MOUSE && flags&ELW_CTRL) {
		//CTRL + right click, delete buddy.
		sprintf(str, "%c#del_buddy %s", RAW_TEXT, buddy_list[y].name);
		my_tcp_send(my_socket, str, strlen(str+1)+1);
	} else {
		//start a pm to them
		// clear the buffer
		input_text_line.data[0] = '\0';
		input_text_line.len = 0;
		
		// insert the person's name
		sprintf (str, "/%s ", buddy_list[y].name);
		put_string_in_buffer (&input_text_line, str, 0);
	}
	return 1;
}

void init_buddy()
{
	int i;

	for (i = 0; i < MAX_BUDDY; i++)
	{
		buddy_list[i].type = 0xff;
		memset (buddy_list[i].name, 0, sizeof (buddy_list[i].name));
	}
}

/*
int clika(widget_list *w){
	w->pos_x+=10;
	return 0;
}
int clikaa(widget_list *w){
	progressbar *b = (progressbar *)w->widget_info;
	b->progress++;
	return 0;
}
*/

int click_add_buddy_handler(widget_list *w, int mx, int my, Uint32 flags)
{
	if(strlen(buddy_name_buffer) == 0) {
		return 1;
	} else {
		char string[255];
		/*int i;

		for(i = 0; buddy_type_buffer[i] != '\0'; i++) {
			if(!isdigit(buddy_type_buffer[i])) {
				LOG_TO_CONSOLE(c_red2, "Invalid buddy type");
				return 1;
			}
		}*/
		sprintf(string, "%c#add_buddy %s %i", RAW_TEXT, buddy_name_buffer, multiselect_get_selected(buddy_add_win, buddy_type_input_id));
		my_tcp_send(my_socket, string, strlen(string+1)+1);
		toggle_window(buddy_add_win);
		buddy_name_buffer[0] = '\0';
		description_buffer[0] = '\0';
		return 1;
	}
}
int display_add_buddy_handler(window_info *win)
{
	//draw_smooth_button("White", int x, int y, int w, int lines, int highlight, float r, float g, float b, float a)

	/* Draw description_buffer and the separator */
	glColor3f(0.77f, 0.57f, 0.39f);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_LINES);
		glVertex3i(0, win->len_y-40,0);
		glVertex3i(win->len_x, win->len_y-40,0);
	glEnd();
	glEnable(GL_TEXTURE_2D);
	draw_string_small(5, win->len_y-5-30, description_buffer, 2);
	return 1;
}
int name_onmouseover_handler(widget_list *widget, int mx, int my)
{
	char str[] = "The name of your buddy";
	put_small_text_in_box(str, strlen(str),
                               buddy_add_x_len-10, description_buffer);
	return 1;
}
int type_onmouseover_handler(widget_list *widget, int mx, int my)
{
	char str[] = "The color you want your buddy to appear in in the list";
	put_small_text_in_box(str, strlen(str),
                               buddy_add_x_len-10, description_buffer);
	return 1;
}

char name_str[] = "Name:";
char type_str[] = "Color:";
char add_buddy_str[] = "Add buddy";

int click_buddy_button_handler(widget_list *w, int mx, int my, Uint32 flags)
{
	if(buddy_add_win < 0) {
		int string_width = get_string_width(type_str)*0.9f;
		int label_id = 10; //temporary variable
		int extra_space, x, y;

		buddy_add_win = create_window(add_buddy_str, buddy_win, 0, buddy_menu_x_len/2, buddy_menu_y_len/4, buddy_add_x_len, buddy_add_y_len, ELW_WIN_DEFAULT);
		set_window_handler(buddy_add_win, ELW_HANDLER_DISPLAY, &display_add_buddy_handler);
		/* Add name label and input widget */
		label_id = label_add_extended(buddy_add_win, label_id, NULL, 5, 5, 0, 0, 0, 0.9f, 1, 1, 1, name_str);
		x = get_string_width(type_str)*0.9f+5;
		y = 5;
		buddy_name_input_id = pword_field_add_extended(buddy_add_win, buddy_name_input_id, NULL, x, y, buddy_add_x_len-x*2+10, 20, P_TEXT, 0.9f, 0.77f, 0.57f, 0.39f, buddy_name_buffer, 15);
		widget_set_OnMouseover(buddy_add_win, label_id, name_onmouseover_handler);
		widget_set_OnMouseover(buddy_add_win, buddy_name_input_id, name_onmouseover_handler);
		/* Add type label and input widget */
		label_id++;
		y += 25;
		string_width = get_string_width(type_str)*0.9f;
		label_id = label_add_extended(buddy_add_win, label_id, NULL, 5, y, 0, 0, 0, 0.9f, 1, 1, 1, type_str);
		buddy_type_input_id = multiselect_add(buddy_add_win, NULL, x, y, buddy_add_x_len-string_width*2);
		multiselect_button_add(buddy_add_win, buddy_type_input_id, 0, 0, "White", 1);
		multiselect_button_add(buddy_add_win, buddy_type_input_id, 0, 25, "Red", 0);
		multiselect_button_add(buddy_add_win, buddy_type_input_id, 0, 50, "Green", 0);
		multiselect_button_add(buddy_add_win, buddy_type_input_id, 0, 75, "Blue", 0);
		multiselect_button_add(buddy_add_win, buddy_type_input_id, 0, 100, "Yellow", 0);
		widget_set_OnMouseover(buddy_add_win, label_id, type_onmouseover_handler);
		widget_set_OnMouseover(buddy_add_win, buddy_type_input_id, type_onmouseover_handler);
		/* Add "Add buddy" button */
		y += 5+multiselect_get_height(buddy_add_win, buddy_type_input_id);
		extra_space = 2+((buddy_add_x_len-string_width*2) - get_string_width(add_buddy_str))/2;
		buddy_add_button_id = button_add(buddy_add_win, NULL, add_buddy_str, x+extra_space, y);
		widget_set_OnClick(buddy_add_win, buddy_add_button_id, click_add_buddy_handler);
	} else {
		toggle_window(buddy_add_win);
	}
	return 1;
}

void display_buddy()
{
	if(buddy_win < 0)
		{
			//buddy_win = AddXMLWindow("buddy.xml");
			buddy_win = create_window("Buddy", game_root_win, 0, buddy_menu_x, buddy_menu_y, buddy_menu_x_len, buddy_menu_y_len, ELW_WIN_DEFAULT);

			set_window_handler(buddy_win, ELW_HANDLER_DISPLAY, &display_buddy_handler );
			set_window_handler(buddy_win, ELW_HANDLER_CLICK, &click_buddy_handler );

			buddy_scroll_id = vscrollbar_add_extended (buddy_win, buddy_scroll_id, NULL, 130, 20, 20, 180, 0, 1.0, 0.77f, 0.57f, 0.39f, 0, 1, MAX_BUDDY-19);
			buddy_button_id = button_add_extended(buddy_win, buddy_button_id, NULL, 0, buddy_menu_y_len-20, buddy_menu_x_len, 20, 0, 1.0, -1.0, -1.0, -1.0, add_buddy_str);
			widget_set_OnClick(buddy_win, buddy_button_id, click_buddy_button_handler);
		}
	else
		{
			toggle_window(buddy_win);/*
			show_window(buddy_win);
			select_window(buddy_win);*/
		}
}

void add_buddy(char *n, int t, int len)
{
	int i;

	//find empty space
	for(i=0; i<MAX_BUDDY; i++){
		if(buddy_list[i].type == 0xff){//found then add buddy
			buddy_list[i].type= t;
			strncpy(buddy_list[i].name,n,len);
			buddy_list[i].name[len]= '\0';
			break;
		}
	}
}

void del_buddy(char *n, int len)
{
	int i;

	//find buddy
	for (i = 0; i < MAX_BUDDY; i++)
	{
		if (!strncmp(n,buddy_list[i].name, len))
		{
			buddy_list[i].type = 0xff;
			memset (buddy_list[i].name, 0, sizeof (buddy_list[i].name));
			break;
		}
		
	}
}

void clear_buddy()
{
	int i;
	for(i=0; i<MAX_BUDDY; i++){
		buddy_list[i].type= 0xff;
		buddy_list[i].name[0]=0;
	}
}
