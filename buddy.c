#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "elwindows.h"
#include "queue.h"

#define	MAX_BUDDY	100
#define MAX_ACCEPT_BUDDY_WINDOWS MAX_BUDDY

int buddy_win=-1;
int buddy_scroll_id = 0;
int buddy_button_id = 1;
int buddy_menu_x=150;
int buddy_menu_y=70;
int buddy_menu_x_len=150;
int buddy_menu_y_len=220;

int buddy_add_win = -1;
int buddy_name_input_id = 0;
int buddy_add_button_id = -1;
int buddy_add_x_len = 290;
int buddy_add_y_len = 97;

int buddy_change_win = -1;
int buddy_change_x_len = 290;
int buddy_change_y_len = 225;
int buddy_type_input_id = -1;
int buddy_change_button_id = -1;
char *buddy_to_change = NULL;

struct accept_window {
	Uint32 window_id; //Window ID
	char name[32]; //Name of the buddy to accept
	char *text; //Buffer for the text to display
	int checkbox; //Checkbox widget id
} accept_windows[MAX_ACCEPT_BUDDY_WINDOWS];
int buddy_accept_x_len = 400;
int buddy_accept_y_len = 130;
queue_t *buddy_request_queue;

char name_str[] = "Name:";
char type_str[] = "Color:";
char add_buddy_str[] = "Add buddy";
char change_buddy_str[] = "Change buddy";
char accept_buddy_str[] = "Accept buddy";
char yes_str[] = "Yes";
char no_str[] = "No";

unsigned char buddy_name_buffer[16] = {0};
char description_buffer[255] = {0};
_buddy buddy_list[MAX_BUDDY];

int is_in_buddylist(const char *name);
int create_buddy_interface_win(const char *title, void *argument);

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
	// Draw buddies
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
	//Draw a button for the requests
	if(!queue_isempty(buddy_request_queue)) {
		glDisable(GL_TEXTURE_2D);
		//glColor3f(0.77f, 0.59f, 0.39f);
		glColor3f(0.3, 1, 0.3);
		glBegin(GL_LINE_LOOP);
			glVertex2i(win->len_x/3, 0);
			glVertex2i(win->len_x/3+10, 16);
			glVertex2i(win->len_x-20, 16);
			glVertex2i(win->len_x-20, 0);
		glEnd();
		glEnable(GL_TEXTURE_2D);
		draw_string_zoomed(win->len_x/3+10,1,"Requests",1,0.7);
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
	} else if(!queue_isempty(buddy_request_queue) && mx > win->len_x/3 && y < 16) {
		//Clicked on the requests button
		while(!queue_isempty(buddy_request_queue)) {
			char *name = queue_pop(buddy_request_queue);
			create_buddy_interface_win(accept_buddy_str, name);
			free(name);
		}
		return 1;
	}
	
	// clicked on a buddy's name
	y /= 10;
	y += vscrollbar_get_pos(buddy_win,buddy_scroll_id);
	if(strlen(buddy_list[y].name) == 0) {
		//There's no name here. Fall through.
		return 0;
	}
	if(flags&ELW_RIGHT_MOUSE) {
		if(flags&ELW_CTRL) {
			//CTRL + right click, delete buddy.
			snprintf(str,sizeof(str), "%c#del_buddy %s", RAW_TEXT, buddy_list[y].name);
			my_tcp_send(my_socket, str, strlen(str+1)+1);
		} else {
			//Right click, open edit window
			create_buddy_interface_win(change_buddy_str, &buddy_list[y]);
		}
	} else {
		//start a pm to them
		// clear the buffer
		input_text_line.data[0] = '\0';
		input_text_line.len = 0;

		// insert the person's name
		snprintf (str, sizeof(str),"/%s ", buddy_list[y].name);
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
	for(i = 0; i < MAX_ACCEPT_BUDDY_WINDOWS; i++)
	{
		accept_windows[i].window_id = -1;
		memset(accept_windows[i].name, 0, sizeof(accept_windows[i].name));
	}
	queue_initialise(&buddy_request_queue);
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

		snprintf(string, sizeof(string), "%c#add_buddy %s", RAW_TEXT, buddy_name_buffer);
		my_tcp_send(my_socket, string, strlen(string+1)+1);
		hide_window(buddy_add_win);
		buddy_name_buffer[0] = '\0';
		description_buffer[0] = '\0';
		return 1;
	}
}

int click_change_buddy_handler(widget_list *w, int mx, int my, Uint32 flags)
{
	char string[255];
	
	snprintf(string, sizeof(string), "%c#change_buddy %s %i", RAW_TEXT, buddy_to_change, multiselect_get_selected(buddy_change_win, buddy_type_input_id));
	my_tcp_send(my_socket, string, strlen(string+1)+1);
	destroy_window(buddy_change_win);
	buddy_change_win = -1;
	buddy_to_change = NULL;
	return 1;
}

int display_add_buddy_handler(window_info *win)
{
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

int display_accept_buddy_handler(window_info *win)
{
	if(win != NULL) {
		int i;
		
		for(i = 0; i < MAX_ACCEPT_BUDDY_WINDOWS; i++) {
			if(accept_windows[i].window_id == win->window_id) {
				draw_string_small(5, 5, accept_windows[i].text, 2);
				break;
			}
		}
		return 1;
	} else {
		return 0;
	}
}

int click_accept_yes(widget_list *w, int mx, int my, Uint32 flags)
{
	unsigned char string[255];
	int window_id = -1;
	int i;
	widget_list *tmp_w;

	//TODO: get rid of this crap when the widget handlers can take custom arguments
	/* Find the window ID by searching through the windows' widget lists 
	 * until we find a pointer to this widget */
	for(i = 0; i < windows_list.num_windows; i++) {
		tmp_w = windows_list.window[i].widgetlist;
		while (tmp_w != NULL && window_id == -1) {
			if(tmp_w == w) {
				window_id = windows_list.window[i].window_id;
				break;
			} else {
				tmp_w = tmp_w->next;
			}
		}
	}
	if(window_id == -1) {
		/* We didn't find it */
		return 0;
	}
	for(i = 0; i < MAX_ACCEPT_BUDDY_WINDOWS; i++) {
		if(accept_windows[i].window_id == window_id) {
			break;
		}
	}
	if(i == MAX_ACCEPT_BUDDY_WINDOWS) {
		/* We didn't find it */
		return 0;
	}
	snprintf(string, 255, "%c#accept_buddy %s", RAW_TEXT, accept_windows[i].name);

	my_tcp_send(my_socket, string, strlen(string+1)+1);
	if(accept_windows[i].checkbox >= 0 && checkbox_get_checked(accept_windows[i].window_id, accept_windows[i].checkbox) > 0) {
		snprintf(string, sizeof(string), "%c#add_buddy %s", RAW_TEXT, accept_windows[i].name);

		my_tcp_send(my_socket, string, strlen(string+1)+1);
	}
	destroy_window(accept_windows[i].window_id);
	accept_windows[i].window_id = -1;
	accept_windows[i].checkbox = -1;
	free(accept_windows[i].text);
	accept_windows[i].text = NULL;
	accept_windows[i].name[0] = '\0';

	return 1;
}

int click_accept_no(widget_list *w, int mx, int my, Uint32 flags)
{
	int i;
	int window_id = -1;
	widget_list *tmp_w = NULL;

	//TODO: get rid of this too
	/* Find the window ID by searching through the windows' widget lists 
	 * until we find a pointer to this widget */
	for(i = 0; i < windows_list.num_windows; i++) {
		tmp_w = windows_list.window[i].widgetlist;
		while (tmp_w != NULL && window_id == -1) {
			if(tmp_w == w) {
				window_id = windows_list.window[i].window_id;
				break;
			} else {
				tmp_w = tmp_w->next;
			}
		}
	}
	if(window_id == -1) {
		/* We didn't find it */
		return 0;
	}
	for(i = 0; i < MAX_ACCEPT_BUDDY_WINDOWS; i++) {
		if(accept_windows[i].window_id == window_id) {
			break;
		}
	}
	if(i == MAX_ACCEPT_BUDDY_WINDOWS) {
		/* We didn't find it */
		return 0;
	}
	destroy_window(accept_windows[i].window_id);
	accept_windows[i].window_id = -1;
	accept_windows[i].checkbox = -1;
	free(accept_windows[i].text);
	accept_windows[i].text = NULL;
	accept_windows[i].name[0] = '\0';

	return 1;
}

int create_buddy_interface_win(const char *title, void *argument)
{
	int label_id = 10; //temporary variable
	int string_width;
	int extra_space;
	int x = 5;
	int y = 5;

	if(strcmp(title, add_buddy_str) == 0) {
		if(buddy_add_win < 0) {
			/* Create the window */
			buddy_add_win = create_window(title, buddy_win, 0, buddy_menu_x_len/2, buddy_menu_y_len/4, buddy_add_x_len, buddy_add_y_len, ELW_WIN_DEFAULT);
			set_window_handler(buddy_add_win, ELW_HANDLER_DISPLAY, &display_add_buddy_handler);
			/* Add name input and label */
			label_id = label_add_extended(buddy_add_win, label_id, NULL, x, y, 0, 0, 0, 0.9f, 1, 1, 1, name_str);
			string_width = get_string_width(name_str)*0.9f;
			x = string_width+5;
			y = 5;
			buddy_name_input_id = pword_field_add_extended(buddy_add_win, buddy_name_input_id, NULL, x, y, buddy_add_x_len-x*2+10, 20, P_TEXT, 0.9f, 0.77f, 0.57f, 0.39f, buddy_name_buffer, 15);
			widget_set_OnMouseover(buddy_add_win, buddy_name_input_id, name_onmouseover_handler);
			widget_set_OnMouseover(buddy_add_win, label_id, name_onmouseover_handler);
			/* Add "Add buddy" button */
			y += 5+20;
			extra_space = 2+((buddy_add_x_len-string_width*2) - get_string_width(add_buddy_str))/2;
			buddy_add_button_id = button_add(buddy_add_win, NULL, add_buddy_str, x+extra_space, y);
			widget_set_OnClick(buddy_add_win, buddy_add_button_id, click_add_buddy_handler);
			return buddy_add_win;
		} else {
			toggle_window(buddy_add_win);
			return buddy_add_win;
		}
	} else if (strcmp(title, change_buddy_str) == 0) {
		_buddy *buddy = argument;
		buddy_to_change = buddy->name;
		if(buddy_change_win < 0) {
			/* Create the window */
			buddy_change_win = create_window(title, buddy_win, 0, buddy_menu_x_len/2, buddy_menu_y_len/4, buddy_change_x_len, buddy_change_y_len, ELW_WIN_DEFAULT);
			set_window_handler(buddy_change_win, ELW_HANDLER_DISPLAY, &display_add_buddy_handler);
			/* Add name label and name */
			label_id = label_add_extended(buddy_change_win, label_id, NULL, x, y, 0, 0, 0, 0.9f, 1, 1, 1, name_str);
			widget_set_OnMouseover(buddy_change_win, label_id, name_onmouseover_handler);
			x += get_string_width(name_str)*0.9f+5;
			label_id = label_add_extended(buddy_change_win, ++label_id, NULL, x, y, 0, 0, 0, 0.9f, 1, 1, 1, buddy_to_change);
			widget_set_OnMouseover(buddy_change_win, label_id, name_onmouseover_handler);
			label_id++;
			/* Add type label and input widget */
			y += 25;
			string_width = get_string_width(type_str)*0.9f;
			label_id = label_add_extended(buddy_change_win, label_id, NULL, 5, y, 0, 0, 0, 0.9f, 1, 1, 1, type_str);

			buddy_type_input_id = multiselect_add(buddy_change_win, NULL, x, y, buddy_add_x_len-string_width*2);
			multiselect_button_add(buddy_change_win, buddy_type_input_id, 0, 0, "White", 1);
			multiselect_button_add(buddy_change_win, buddy_type_input_id, 0, 25, "Red", 0);
			multiselect_button_add(buddy_change_win, buddy_type_input_id, 0, 50, "Green", 0);
			multiselect_button_add(buddy_change_win, buddy_type_input_id, 0, 75, "Blue", 0);
			multiselect_button_add(buddy_change_win, buddy_type_input_id, 0, 100, "Yellow", 0);
			multiselect_set_selected(buddy_change_win, buddy_type_input_id, buddy->type);

			widget_set_OnMouseover(buddy_change_win, label_id, type_onmouseover_handler);
			widget_set_OnMouseover(buddy_change_win, buddy_type_input_id, type_onmouseover_handler);
			y += 5+multiselect_get_height(buddy_change_win, buddy_type_input_id);
			/* Add button */
			extra_space = 2+((buddy_change_x_len-string_width*2) - get_string_width(change_buddy_str))/2;
			buddy_change_button_id = button_add(buddy_change_win, NULL, change_buddy_str, x+extra_space, y);
			widget_set_OnClick(buddy_change_win, buddy_change_button_id, click_change_buddy_handler);
			return buddy_change_win;
		} else {
			toggle_window(buddy_change_win);
			return buddy_change_win;
		}
	} else if (strcmp(title, accept_buddy_str) == 0) {
		char string[250] = {0};
		int current_window;
		int yes_button;
		int no_button;
		int win_height = buddy_accept_y_len;

		/* Argument is the name of the buddy that want to add us */
		for(current_window = 0; current_window < MAX_ACCEPT_BUDDY_WINDOWS; current_window++) {
			/* Find a free slot in the array */
			if(accept_windows[current_window].window_id == -1) {
				break;
			}
		}
		snprintf(accept_windows[current_window].name, sizeof (accept_windows[current_window].name), "%s", (char *)argument);
		if(is_in_buddylist(accept_windows[current_window].name)) {
			/* We don't need to make room for the checkbox because the buddy is already in our list. */
			win_height -= 20;
		}
		accept_windows[current_window].window_id = create_window(title, game_root_win, 0, 200, 200, buddy_accept_x_len, win_height, ELW_TITLE_BAR|ELW_DRAGGABLE|ELW_USE_BACKGROUND|ELW_USE_BORDER|ELW_SHOW|ELW_TITLE_NAME|ELW_ALPHA_BORDER);
		set_window_handler(accept_windows[current_window].window_id, ELW_HANDLER_DISPLAY, &display_accept_buddy_handler);
		/* Add text */
		snprintf(string, sizeof(string), "%s wants to add you to his/her buddy list. Do you wish to allow it?", accept_windows[current_window].name);
		//label_add_extended(buddy_accept_win, label_id++, NULL, x, y, 0, 0, 0, 0.8, -1, -1, -1, string);
		accept_windows[current_window].text = malloc((strlen(string)+5)*sizeof(*accept_windows[current_window].text));
		put_small_text_in_box(string, strlen(string), buddy_accept_x_len-10, accept_windows[current_window].text);
		y += 40;
		/* Add checkbox if we don't have him/her in our list. */
		if(!is_in_buddylist(accept_windows[current_window].name)) {
			accept_windows[current_window].checkbox = checkbox_add(accept_windows[current_window].window_id, NULL, x, y, 15, 15, NULL);
			x += 20;
			snprintf(string, 350, "Add to my buddy list");
			label_add_extended(accept_windows[current_window].window_id, label_id++, NULL, x, y, 0, 0, 0, 0.8, -1, -1, -1, string);
			y += 20;
			x = 5;
		}
		/* Add buttons */
		x = (buddy_accept_x_len - (get_string_width(yes_str)*0.9 + 2 + 10 + 2 + get_string_width(no_str)))/2;
		yes_button = button_add(accept_windows[current_window].window_id, NULL, yes_str, x, y);
		x += get_string_width(yes_str) + 2 + 10 ;
		no_button = button_add(accept_windows[current_window].window_id, NULL, no_str, x, y);
		widget_set_OnClick(accept_windows[current_window].window_id, yes_button, click_accept_yes);
		widget_set_OnClick(accept_windows[current_window].window_id, no_button, click_accept_no);

		return accept_windows[current_window].window_id;
	} else {
		return -1;
	}
}

int click_buddy_button_handler(widget_list *w, int mx, int my, Uint32 flags)
{
	if(buddy_add_win < 0) {
		buddy_add_win = create_buddy_interface_win(add_buddy_str, NULL);
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
			toggle_window(buddy_win);
		}
}

void add_buddy(char *n, int t, int len)
{
	int i;

	//find empty space
	for(i=0; i<MAX_BUDDY; i++){
		if(buddy_list[i].type == 0xff){//found then add buddy
			buddy_list[i].type= t;
			snprintf(buddy_list[i].name, len+1, "%s", n);
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

int is_in_buddylist(const char *name)
{
	if(name && *name) {
		int i;
		for(i = 0; i < MAX_BUDDY; i++) {
			if(buddy_list[i].type < 0xff && strncmp(buddy_list[i].name, name, sizeof(buddy_list[i].name)) == 0) {
				return 1;
			}
		}
	}
	return 0;
}

void add_buddy_confirmation(char *name) {
	char *name_copy = malloc(strlen(name)+1);
	snprintf(name_copy, strlen(name)+1, "%s", name);
	queue_push(buddy_request_queue, name_copy);
}
