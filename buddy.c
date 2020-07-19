#include <stdlib.h>
#include <math.h>
#include <string.h>
#ifndef _MSC_VER
#include <strings.h>
#endif
#include <time.h>
#include "buddy.h"
#include "asc.h"
#include "chat.h"
#include "console.h"
#include "elconfig.h"
#include "elwindows.h"
#include "gamewin.h"
#include "hud.h"
#include "icon_window.h"
#include "init.h"
#include "loginwin.h"
#include "multiplayer.h"
#include "queue.h"
#include "translate.h"
#ifdef OPENGL_TRACE
#include "gl_init.h"
#endif

#define	MAX_BUDDY 100
#define MAX_ACCEPT_BUDDY_WINDOWS MAX_BUDDY

typedef struct
{
   char name[32]; // name of your buddy
   unsigned char type;
}_buddy;

static int buddy_scroll_id = 0;
static int buddy_button_id = 1;
static int buddy_menu_x_len=0;
static int buddy_menu_y_len=0;
static int buddy_name_step_y = 0;
static int request_box_start_x = 0;
static int buddy_border_space = 0;
static const int num_displayed_buddies = 16;
static int buddy_add_win = -1;

static int buddy_change_win = -1;
static int buddy_type_input_id = -1;
static int buddy_delete = 0; //For the checkbox
static char *buddy_to_change = NULL;

static struct accept_window {
	int window_id; //Window ID
	char name[32]; //Name of the buddy to accept
	char *text; //Buffer for the text to display
	int checkbox; //Checkbox widget id
} accept_windows[MAX_ACCEPT_BUDDY_WINDOWS];
static queue_t *buddy_request_queue = NULL;

static unsigned char buddy_name_buffer[MAX_USERNAME_LENGTH] = {0};
static char description_buffer[255] = {0};
static _buddy buddy_list[MAX_BUDDY];

static int display_buddy_change(_buddy *buddy);
static int display_accept_buddy(char *name);

void destroy_buddy_queue(void)
{
	queue_destroy(buddy_request_queue);
}

int accept_buddy_console_command(const char *name)
{
	if (!queue_isempty(buddy_request_queue))
	{
		node_t *node = queue_front_node(buddy_request_queue);

		/* Search for the node in the queue */
		while(node != NULL) {
			if(strcasecmp(name, node->data) == 0) {
				/* This is the node we're looking for, delete it */
				queue_delete_node(buddy_request_queue, node);
				return 1;
			}
			node = node->next;
		}
	}
	return 0;
}

static int buddy_list_name_cmp( const void *arg1, const void *arg2)
{
	const _buddy *b1=arg1, *b2=arg2;
	if(b1->type==b2->type)
		return strcasecmp(b1->name,b2->name);
	else
		return b1->type-b2->type;
}

static int display_buddy_handler(window_info *win)
{
	int i = 0;
	int y = buddy_border_space;
	int offset = 0;

	glEnable(GL_TEXTURE_2D);
	// Draw buddies
	qsort(buddy_list,MAX_BUDDY,sizeof(_buddy),buddy_list_name_cmp);

	offset = vscrollbar_get_pos (win->window_id, buddy_scroll_id);
	if (offset >= 0)
	{
		for (i = offset; i < offset + num_displayed_buddies; i++)
		{
			switch(buddy_list[i].type){
				case 0:glColor3f(1.0,1.0,1.0);break;
				case 1:glColor3f(1.0,0,0);break;
				case 2:glColor3f(0,1.0,0);break;
				case 3:glColor3f(0.25,0.5,1.0);break;
				case 4:glColor3f(1.0,1.0,0);break;
				case 0xFE:glColor3f(0.5,0.55,0.60);break;
				default:glColor3f(1.0,1.0,1.0);//invalid number? make it white
			}
			draw_string_small_zoomed(buddy_border_space, y, (unsigned char*)buddy_list[i].name, 1, win->current_scale);
			y += buddy_name_step_y;
		}
	}
	//Draw a button for the requests
	if(!queue_isempty(buddy_request_queue)) {
		glDisable(GL_TEXTURE_2D);
		//glColor3f(0.77f, 0.59f, 0.39f);
		glColor3f(0.3, 1, 0.3);
		glBegin(GL_LINE_LOOP);
			glVertex2i(request_box_start_x - win->small_font_max_len_x, 0);
			glVertex2i(request_box_start_x, win->small_font_len_y + 1);
			glVertex2i(win->len_x - win->box_size - 1, win->small_font_len_y + 1);
			glVertex2i(win->len_x - win->box_size - 1, 0);
		glEnd();
		glEnable(GL_TEXTURE_2D);
		draw_string_small_zoomed(request_box_start_x + win->small_font_max_len_x, 0, (unsigned char*)buddy_request_str, 1, win->current_scale);
	}
	glColor3f(0.77f, 0.57f, 0.39f);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	return 1;
}

static int click_buddy_handler (window_info *win, int mx, int my, Uint32 flags)
{
	int x = mx;
	int y = my - buddy_border_space;
	char str[50];

	// scroll the winow with the mouse wheel
	if(flags & ELW_WHEEL_UP) {
		vscrollbar_scroll_up(win->window_id, buddy_scroll_id);
		return 1;
	} else if(flags & ELW_WHEEL_DOWN) {
		vscrollbar_scroll_down(win->window_id, buddy_scroll_id);
		return 1;
	}

	// only handle mouse button clicks, not scroll wheels moves
	if ( (flags & ELW_MOUSE_BUTTON) == 0)
		return 0;

	if(x > (win->len_x - win->box_size)) {
		//Clicked on the scrollbar. Let it fall through.
		return 0;
	} else if(!queue_isempty(buddy_request_queue) && mx > (request_box_start_x - win->small_font_max_len_x) && y < (win->small_font_len_y + 1)) {
		//Clicked on the requests button
		while(!queue_isempty(buddy_request_queue)) {
			char *name = queue_pop(buddy_request_queue);
			select_window(display_accept_buddy(name));
			free(name);
		}
		return 1;
	}

	if (y < 0)
        // Clicked above the names
        return 0;

	// clicked on a buddy's name
	y /= buddy_name_step_y;
	if (y >= num_displayed_buddies)
		return 0;
	y += vscrollbar_get_pos(win->window_id, buddy_scroll_id);
	if((strlen(buddy_list[y].name) == 0)||(buddy_list[y].type > 0xFE)) {
		//There's no name. Fall through.
		return 0;
	}
	if(flags&ELW_RIGHT_MOUSE) {
		if(flags & KMOD_CTRL) {
			//CTRL + right click, delete buddy.
			safe_snprintf(str, sizeof(str), "%c#del_buddy %s", RAW_TEXT, buddy_list[y].name);
			my_tcp_send(my_socket, (Uint8*)str, strlen(str+1)+1);
		} else {
			//Right click, open edit window
			display_buddy_change(&buddy_list[y]);
		}
	} else if (buddy_list[y].type < 0xFE) {
		//start a pm to them
		// clear the buffer
		clear_input_line();

		// insert the person's name
		safe_snprintf (str, sizeof(str),"/%s ", buddy_list[y].name);
		//put_string_in_buffer (&input_text_line, str, 0);
		//We'll just reuse the paste function here
		paste_in_input_field((unsigned char*)str);
	}
	return 1;
}

void init_buddy(void)
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

static int click_add_buddy_handler(widget_list *w, int mx, int my, Uint32 flags)
{
	if(strlen((char*)buddy_name_buffer) == 0) {
		return 1;
	} else {
		char string[255];

		string[0] = RAW_TEXT;
		safe_snprintf(string+1, sizeof(string)-1, "#add_buddy %s", buddy_name_buffer);
		my_tcp_send(my_socket, (Uint8*)string, strlen(string+1)+1);
		hide_window(buddy_add_win);
		buddy_name_buffer[0] = '\0';
		description_buffer[0] = '\0';
		return 1;
	}
}

static int click_change_buddy_handler(widget_list *w, int mx, int my, Uint32 flags)
{
	char string[255];
	int send_message = 1;

	if(buddy_delete) {
		safe_snprintf(string, sizeof(string), "%c#del_buddy %s", RAW_TEXT, buddy_to_change);
		buddy_delete = 0;
	} else if (buddy_type_input_id != -1) {
		safe_snprintf(string, sizeof(string), "%c#change_buddy %s %i", RAW_TEXT, buddy_to_change, multiselect_get_selected(buddy_change_win, buddy_type_input_id));
	} else {
		send_message = 0;
	}
	if (send_message) {
		my_tcp_send(my_socket, (Uint8*)string, strlen(string+1)+1);
	}
	destroy_window(buddy_change_win);
	buddy_change_win = -1;
	buddy_to_change = NULL;
	return 1;
}

static void split_long_show_help(window_info *win, const char *str, int x, int y)
{
	size_t str_len = strlen(str);
	unsigned char *tmp = malloc(str_len + 5);

	put_small_text_in_box_zoomed((const unsigned char*)str, str_len, win->len_x,
		tmp, win->current_scale);
	show_help((const char*)tmp, x, y, win->current_scale);

	free(tmp);
}

static int name_onmouseover_handler(widget_list *widget, int mx, int my)
{
	window_info *win = &windows_list.window[widget->window_id];
	split_long_show_help(win, buddy_long_name_str, 0, win->len_y + 10);
	return 1;
}

static int type_onmouseover_handler(widget_list *widget, int mx, int my)
{
	window_info *win = &windows_list.window[widget->window_id];
	split_long_show_help(win, buddy_long_type_str, 0, win->len_y + 10);
	return 1;
}

static int delete_onmouseover_handler(widget_list *widget, int mx, int my)
{
	window_info *win = &windows_list.window[widget->window_id];
	split_long_show_help(win, buddy_long_delete_str, 0, win->len_y + 10);
	return 1;
}

static int display_accept_buddy_handler(window_info *win)
{
	if(win != NULL) {
		int i;

		glColor3f(0.77f, 0.57f, 0.39f);
		for(i = 0; i < MAX_ACCEPT_BUDDY_WINDOWS; i++) {
			if(accept_windows[i].window_id == win->window_id) {
				draw_string_small_zoomed(buddy_border_space, buddy_border_space, (unsigned char*)accept_windows[i].text, 2, win->current_scale);
				break;
			}
		}
		return 1;
	} else {
		return 0;
	}
}

static int name_input_keypress_handler(widget_list *widget, int mx, int my, SDL_Keycode key_code, Uint32 key_unicode, Uint16 key_mod)
{
	if((key_code == SDLK_RETURN || key_code == SDLK_KP_ENTER) && strlen((char*)buddy_name_buffer) > 0) {
		return click_add_buddy_handler(widget, mx, my, ELW_LEFT_MOUSE);
	} else {
		return 0;
	}
}

static int click_accept_yes(widget_list *w, int mx, int my, Uint32 flags)
{
	char string[255];
	int window_id = w->window_id;
	int i;

	//TODO: get rid of this when the widget handlers can take custom arguments
	/* Find the position of the window clicked in the accept_windows array */
	for(i = 0; i < MAX_ACCEPT_BUDDY_WINDOWS; i++) {
		if(accept_windows[i].window_id == window_id) {
			break;
		}
	}
	if(i == MAX_ACCEPT_BUDDY_WINDOWS) {
		/* We didn't find it */
		return 0;
	}
	safe_snprintf(string, sizeof(string), "%c#accept_buddy %s", RAW_TEXT, accept_windows[i].name);

	my_tcp_send(my_socket, (Uint8*)string, strlen(string+1)+1);
	if(accept_windows[i].checkbox >= 0 && checkbox_get_checked(accept_windows[i].window_id, accept_windows[i].checkbox) > 0) {
		safe_snprintf(string, sizeof(string), "%c#add_buddy %s", RAW_TEXT, accept_windows[i].name);

		my_tcp_send(my_socket, (Uint8*)string, strlen(string+1)+1);
	}
	destroy_window(accept_windows[i].window_id);
	accept_windows[i].window_id = -1;
	accept_windows[i].checkbox = -1;
	free(accept_windows[i].text);
	accept_windows[i].text = NULL;
	accept_windows[i].name[0] = '\0';

	return 1;
}

static int click_accept_no(widget_list *w, int mx, int my, Uint32 flags)
{
	int i;
	int window_id = w->window_id;

	//TODO: get rid of this too
	/* Find the position of the window clicked in the accept_windows array */
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

static int click_delete_checkbox_label(widget_list *w, int mx, int my, Uint32 flags)
{
	buddy_delete = !buddy_delete;
	return 1;
}

int display_buddy_add(void)
{
	if(buddy_add_win < 0)
	{
		int label_id = 100, input_id = 101, button_id = 102;
		window_info *win = NULL;
		int label_width = 0;
		int win_x_len = 0;
		int win_y_len = 0;
		int cw;

		/* Create the window */
		buddy_add_win = create_window(buddy_add_str, get_id_MW(MW_BUDDY), 0, buddy_menu_x_len/2, buddy_menu_y_len/4, 0, 0, ELW_USE_UISCALE|ELW_WIN_DEFAULT);
		if (buddy_add_win <=0 || buddy_add_win >= windows_list.num_windows)
			return -1;
		win = &windows_list.window[buddy_add_win];
		set_window_custom_scale(buddy_add_win, MW_BUDDY);

		/* Add name input and label */
		cw = get_avg_char_width_zoom(UI_FONT, win->current_scale);
		label_id = label_add_extended(buddy_add_win, label_id, NULL,
			buddy_border_space, buddy_border_space, 0, win->current_scale, buddy_name_str);
		label_width = widget_get_width(buddy_add_win, label_id);
		input_id = pword_field_add_extended(buddy_add_win, input_id, NULL,
			2 * buddy_border_space + label_width, buddy_border_space,
			MAX_USERNAME_LENGTH * cw, win->default_font_len_y + buddy_border_space,
			P_TEXT, win->current_scale, buddy_name_buffer, MAX_USERNAME_LENGTH);
		widget_set_OnMouseover(buddy_add_win, label_id, name_onmouseover_handler);
		widget_set_OnMouseover(buddy_add_win, input_id, name_onmouseover_handler);
		widget_set_OnKey(buddy_add_win, input_id, (int (*)())name_input_keypress_handler);

		/* Add "Add buddy" button */
		button_id = button_add_extended(buddy_add_win, button_id, NULL, 0, 0, 0, 0, 0, win->current_scale, buddy_add_str);
		widget_set_OnClick(buddy_add_win, button_id, click_add_buddy_handler);

		/* Resize window and centre button */
		win_x_len = 3 * buddy_border_space + label_width + widget_get_width(buddy_add_win, input_id) + win->box_size;
		win_y_len = 3 * buddy_border_space + widget_get_height(buddy_add_win, input_id) + widget_get_height(buddy_add_win, button_id);
		resize_window(buddy_add_win, win_x_len, win_y_len);
		widget_move(buddy_add_win, button_id, (win_x_len - widget_get_width(buddy_add_win, button_id)) / 2, 3 * buddy_border_space + win->default_font_len_y);

		return buddy_add_win;
	}
	else
	{
		toggle_window(buddy_add_win);
		return buddy_add_win;
	}
}

static int display_buddy_change(_buddy *buddy)
{
	int label_id = 100, name_id = 101, type_label_id = 102, checkbox_id = 103, checkbox_label_id = 104, change_button_id = 105;
	int buddy_change_x_len = 0;
	int buddy_change_y_len = 0;
	window_info *win = NULL;
	int tmp_width = 0;

	buddy_to_change = buddy->name;
	if(buddy_change_win >= 0) {
		/* Destroy the window to make sure everything's set up. */
		destroy_window(buddy_change_win);
		buddy_change_win = -1;
		buddy_delete = 0;
	}

	/* Create the window */
	buddy_change_win = create_window(buddy_change_str, get_id_MW(MW_BUDDY), 0, buddy_menu_x_len/2, buddy_menu_y_len/4, 0, 0, ELW_USE_UISCALE|ELW_WIN_DEFAULT);
	if (buddy_change_win <=0 || buddy_change_win >= windows_list.num_windows)
		return -1;
	win = &windows_list.window[buddy_change_win];
	set_window_custom_scale(buddy_change_win, MW_BUDDY);

	/* Add name label and name */
	label_id = label_add_extended(buddy_change_win, label_id, NULL,
		buddy_border_space, buddy_border_space, 0, win->current_scale, buddy_name_str);
	tmp_width = widget_get_width(buddy_change_win, label_id);
	name_id = label_add_extended(buddy_change_win, name_id, NULL,
		2 * buddy_border_space + tmp_width, buddy_border_space, 0, win->current_scale, buddy_to_change);

	buddy_change_x_len = 3 * buddy_border_space + tmp_width + MAX_USERNAME_LENGTH * win->default_font_max_len_x + win->box_size;
	buddy_change_y_len = 2 * buddy_border_space + widget_get_height(buddy_change_win, name_id);

	buddy_type_input_id = -1;
	if (buddy->type < 0xFE)
	{
		size_t i;
		const size_t num_type_labels = 5;
		char * type_labels_text[] = {buddy_white_str, buddy_red_str, buddy_green_str, buddy_blue_str, buddy_yellow_str};

		/* Add type label and input widget */
		type_label_id = label_add_extended(buddy_change_win, type_label_id, NULL,
			buddy_border_space, buddy_change_y_len, 0, win->current_scale, buddy_type_str);
		tmp_width = widget_get_width(buddy_change_win, type_label_id);

		buddy_type_input_id = multiselect_add(buddy_change_win, NULL,
			2 * buddy_border_space + tmp_width, buddy_change_y_len, buddy_change_x_len - tmp_width - 3 * buddy_border_space);
		for (i=0; i<num_type_labels; i++)
		{
			multiselect_button_add_extended(buddy_change_win, buddy_type_input_id,
				0, i * 25 * win->current_scale, 0, type_labels_text[i],
				win->current_scale_small, i==0);
		}
		multiselect_set_selected(buddy_change_win, buddy_type_input_id, buddy->type);

		widget_set_OnMouseover(buddy_change_win, type_label_id, type_onmouseover_handler);
		widget_set_OnMouseover(buddy_change_win, buddy_type_input_id, type_onmouseover_handler);

		if ((3 * buddy_border_space + tmp_width + widget_get_width(buddy_change_win, buddy_type_input_id)) > buddy_change_x_len)
			buddy_change_x_len = 3 * buddy_border_space + tmp_width + widget_get_width(buddy_change_win, buddy_type_input_id);
		buddy_change_y_len += buddy_border_space + multiselect_get_height(buddy_change_win, buddy_type_input_id);
	}

	/* Delete buddy checkbox and label */
	checkbox_id = checkbox_add(buddy_change_win, NULL, 0, 0, win->default_font_len_y, win->default_font_len_y, &buddy_delete);
	checkbox_label_id = label_add_extended(buddy_change_win, checkbox_label_id, NULL,
		0, 0, 0, win->current_scale, buddy_delete_str);
	widget_set_OnClick(buddy_change_win, checkbox_label_id, click_delete_checkbox_label);
	widget_set_OnMouseover(buddy_change_win, checkbox_id, delete_onmouseover_handler);
	widget_set_OnMouseover(buddy_change_win, checkbox_label_id, delete_onmouseover_handler);
	tmp_width = widget_get_width(buddy_change_win, checkbox_id) + buddy_border_space + widget_get_width(buddy_change_win, checkbox_label_id);
	widget_move(buddy_change_win, checkbox_id, (buddy_change_x_len - tmp_width) / 2, buddy_change_y_len);
	widget_move(buddy_change_win, checkbox_label_id,
		buddy_change_x_len - ((buddy_change_x_len - tmp_width) / 2) - widget_get_width(buddy_change_win, checkbox_label_id), buddy_change_y_len);
	buddy_change_y_len += buddy_border_space + widget_get_height(buddy_change_win, checkbox_id);

	/* Change button */
	change_button_id = button_add_extended(buddy_change_win, change_button_id, NULL,
		0, 0, 0, 0, 0, win->current_scale, buddy_change_str);
	widget_set_OnClick(buddy_change_win, change_button_id, click_change_buddy_handler);
	widget_move(buddy_change_win, change_button_id,
		(buddy_change_x_len - widget_get_width(buddy_change_win, change_button_id)) / 2, buddy_change_y_len);
	buddy_change_y_len += buddy_border_space + widget_get_height(buddy_change_win, change_button_id);

	resize_window(buddy_change_win, buddy_change_x_len, buddy_change_y_len);

	return buddy_change_win;
}

static int click_accept_checkbox_label(widget_list *w, int mx, int my, Uint32 flags)
{
	int current_window;
	for(current_window = 0; current_window < MAX_ACCEPT_BUDDY_WINDOWS; current_window++)
		if(accept_windows[current_window].window_id == w->window_id)
			break;
	if(current_window == MAX_ACCEPT_BUDDY_WINDOWS)
		return 0;
	checkbox_set_checked(w->window_id, accept_windows[current_window].checkbox, !checkbox_get_checked(w->window_id, accept_windows[current_window].checkbox));
	return 1;
}


static int ui_scale_accept_handler(window_info *win)
{
	int label_id = 101, yes_button = 102, no_button = 103;
	int win_width = (int)(0.5 + win->current_scale * 400);
	int win_height = 3 * buddy_border_space + 2 * win->small_font_len_y;
	char string[250] = {0};
	int current_window;

	for(current_window = 0; current_window < MAX_ACCEPT_BUDDY_WINDOWS; current_window++)
		if(accept_windows[current_window].window_id == win->window_id)
			break;
	if(current_window == MAX_ACCEPT_BUDDY_WINDOWS)
		return 0;

	safe_snprintf(string, sizeof(string), buddy_wants_to_add_str, accept_windows[current_window].name);
	put_small_colored_text_in_box_zoomed(c_blue1, (const unsigned char*)string, strlen(string),
		win_width - 2 * buddy_border_space, (unsigned char*) accept_windows[current_window].text,
		win->current_scale);

	widget_resize(win->window_id, accept_windows[current_window].checkbox, win->small_font_len_y, win->small_font_len_y);
	widget_move(win->window_id, accept_windows[current_window].checkbox, buddy_border_space, win_height);
	widget_set_size(win->window_id, label_id, win->current_scale_small);
	widget_move(win->window_id, label_id, 2 * buddy_border_space + win->small_font_len_y, win_height);
	win_height += widget_get_height(win->window_id, accept_windows[current_window].checkbox) + 2 * buddy_border_space;

	button_resize(win->window_id, yes_button, 0, 0, win->current_scale);
	button_resize(win->window_id, no_button, 0, 0, win->current_scale);
	widget_move(win->window_id, yes_button, (win_width/2 - widget_get_width(win->window_id, yes_button)) / 2, win_height);
	widget_move(win->window_id, no_button, win_width/2 + (win_width/2 - widget_get_width(win->window_id, no_button)) / 2, win_height);

	win_height += widget_get_height(win->window_id, yes_button) + buddy_border_space;
	resize_window(win->window_id, win_width, win_height);

	return 1;
}

static int display_accept_buddy(char *name)
{
	window_info *win = NULL;
	int label_id = 101, yes_button = 102, no_button = 103;
	char string[250] = {0};
	int current_window;

	for(current_window = 0; current_window < MAX_ACCEPT_BUDDY_WINDOWS; current_window++) {
		/* Find a free slot in the array */
		if(accept_windows[current_window].window_id == -1) {
			break;
		}
	}

	if (current_window >= MAX_ACCEPT_BUDDY_WINDOWS)
	{
		// uh oh, no free window
		return -1;
	}

	safe_snprintf(accept_windows[current_window].name, sizeof (accept_windows[current_window].name), "%s", name);

	accept_windows[current_window].window_id = create_window(buddy_accept_str, get_id_MW(MW_BUDDY), 0,
		buddy_menu_x_len/2, buddy_menu_y_len/4, 0, 0, (ELW_USE_UISCALE|ELW_WIN_DEFAULT) ^ ELW_CLOSE_BOX);
	set_window_handler(accept_windows[current_window].window_id, ELW_HANDLER_DISPLAY, &display_accept_buddy_handler);
	set_window_handler(accept_windows[current_window].window_id, ELW_HANDLER_UI_SCALE, &ui_scale_accept_handler );
	if (accept_windows[current_window].window_id <=0 || accept_windows[current_window].window_id >= windows_list.num_windows)
		return -1;
	win = &windows_list.window[accept_windows[current_window].window_id];

	/* Add text */
	safe_snprintf(string, sizeof(string), buddy_wants_to_add_str, accept_windows[current_window].name);
	accept_windows[current_window].text = malloc((strlen(string)+5)*sizeof(*accept_windows[current_window].text));

	/* Add checkbox if we don't have him/her in our list. */
	if(!is_in_buddylist(accept_windows[current_window].name))
	{
		accept_windows[current_window].checkbox = checkbox_add(win->window_id, NULL,
			0, 0, win->small_font_len_y, win->small_font_len_y, NULL);
		widget_unset_color(win->window_id, accept_windows[current_window].checkbox);
		label_id = label_add_extended(win->window_id, label_id, NULL,
			0, 0, 0, win->current_scale_small, buddy_add_to_list_str);
		widget_unset_color(win->window_id, label_id);
		widget_set_OnClick(win->window_id, label_id, click_accept_checkbox_label);
	}

	/* Add buttons */
	yes_button = button_add_extended(win->window_id, yes_button, NULL, 0, 0, 0, 0, 0, 1.0f, yes_str);
	no_button = button_add_extended(win->window_id, no_button, NULL, 0, 0, 0, 0, 0, 1.0f, no_str);
	widget_set_OnClick(win->window_id, yes_button, click_accept_yes);
	widget_set_OnClick(win->window_id, no_button, click_accept_no);

	ui_scale_accept_handler(win);

	return win->window_id;
}

static int click_buddy_button_handler(widget_list *w, int mx, int my, Uint32 flags)
{
	if(buddy_add_win < 0) {
		buddy_add_win = display_buddy_add();
	} else {
		toggle_window(buddy_add_win);
	}
	return 1;
}

static void set_scrollbar_len(void)
{
	int i;
	int num_buddies = 0;
	for (i = 0; i < MAX_BUDDY; i++)
		if (buddy_list[i].type != 0xff)
			num_buddies++;
	vscrollbar_set_bar_len(get_id_MW(MW_BUDDY), buddy_scroll_id, ((num_buddies - num_displayed_buddies < 0) ?0: num_buddies - num_displayed_buddies));
}

static int ui_scale_buddy_handler(window_info *win)
{
	int button_len_y = win->default_font_len_y + 4*win->current_scale;
	buddy_border_space = (int)(0.5 + win->current_scale * 5);
	buddy_name_step_y = get_line_height(win->font_category, win->current_scale_small);

	buddy_menu_x_len = win->box_size + MAX_USERNAME_LENGTH * win->small_font_max_len_x + 2 * buddy_border_space;
	buddy_menu_y_len = button_len_y + 2* buddy_border_space + num_displayed_buddies * buddy_name_step_y;

	request_box_start_x = buddy_menu_x_len - win->box_size
		- get_string_width_zoom((const unsigned char*)buddy_request_str,
			win->font_category, win->current_scale_small)
		- 2 * win->small_font_max_len_x;

	resize_window(win->window_id, buddy_menu_x_len, buddy_menu_y_len);

	button_resize(win->window_id, buddy_button_id, buddy_menu_x_len, button_len_y, win->current_scale);
	widget_move(win->window_id, buddy_button_id, 0, buddy_menu_y_len - button_len_y);

	widget_resize(win->window_id, buddy_scroll_id, win->box_size, buddy_menu_y_len - win->box_size - button_len_y);
	widget_move(win->window_id, buddy_scroll_id, buddy_menu_x_len - win->box_size, win->box_size);

	if (buddy_add_win >= 0)
		destroy_window(buddy_add_win);
	if (buddy_change_win >= 0)
		destroy_window(buddy_change_win);
	buddy_add_win = buddy_change_win = -1;

	return 1;
}

static int change_buddy_font_handler(window_info *win, font_cat cat)
{
	if (cat != UI_FONT)
		return 0;
	return ui_scale_buddy_handler(win);
}

void display_buddy(void)
{
	int buddy_win = get_id_MW(MW_BUDDY);

	if(buddy_win < 0)
		{
			buddy_win = create_window(win_buddy, (not_on_top_now(MW_BUDDY) ?game_root_win : -1), 0,
				get_pos_x_MW(MW_BUDDY), get_pos_y_MW(MW_BUDDY), 0, 0, ELW_USE_UISCALE|ELW_WIN_DEFAULT);
			set_id_MW(MW_BUDDY, buddy_win);

			set_window_custom_scale(buddy_win, MW_BUDDY);
			set_window_handler(buddy_win, ELW_HANDLER_DISPLAY, &display_buddy_handler );
			set_window_handler(buddy_win, ELW_HANDLER_CLICK, &click_buddy_handler );
			set_window_handler(buddy_win, ELW_HANDLER_UI_SCALE, &ui_scale_buddy_handler );
			set_window_handler(buddy_win, ELW_HANDLER_FONT_CHANGE, &change_buddy_font_handler);

			buddy_scroll_id = vscrollbar_add_extended (buddy_win, buddy_scroll_id, NULL, 0, 0, 0, 0, 0, 1.0, 0, 1, 0);
			buddy_button_id = button_add_extended(buddy_win, buddy_button_id, NULL, 0, 0, 0, 0,
				BUTTON_SQUARE|BUTTON_VCENTER_CONTENT, 1.0, buddy_add_str);
			widget_set_OnClick(buddy_win, buddy_button_id, click_buddy_button_handler);

			if (buddy_win >=0 && buddy_win < windows_list.num_windows)
				ui_scale_buddy_handler(&windows_list.window[buddy_win]);
			check_proportional_move(MW_BUDDY);
		}
	else
		{
			toggle_window(buddy_win);
		}
}

void add_buddy (const char *name, int type, int len)
{
	int i, found = 0;
	char message[35];

	add_name_to_tablist(name);
	// Check if the buddy already exists
	for (i = 0; i < MAX_BUDDY; i++)
	{
		if(strncasecmp(buddy_list[i].name, name, len) == 0){
			//this name is already in our list
			if(buddy_list[i].type != type){
				//colour change, not a new entry
				if(buddy_log_notice == 1){
					if(buddy_list[i].type == 0xFE){//logging on
						safe_snprintf (message, sizeof(message), buddy_logon_str, len, name);
						LOG_TO_CONSOLE (c_green1, message);
						flash_icon(tt_buddy, 5);
					}else if(type == 0xFE){//logging off
						safe_snprintf (message, sizeof(message), buddy_logoff_str, len, name);
						LOG_TO_CONSOLE (c_green1, message);
						flash_icon(tt_buddy, 5);
					}//else it's just a normal colour change
				}
				buddy_list[i].type=type;
			}
			found = 1;
			break;
		}
	}
	if (found != 1) {
		// find empty space
		for (i = 0; i < MAX_BUDDY; i++)
		{
			if (buddy_list[i].type == 0xff)
			{
				// found then add buddy
				buddy_list[i].type = type;
				safe_snprintf (buddy_list[i].name, sizeof(buddy_list[i].name), "%.*s", len, name);
				// write optional online message
				if ((buddy_log_notice == 1) && (type != 0xFE))
				{
					safe_snprintf (message, sizeof(message), buddy_online_str, len, name);
					LOG_TO_CONSOLE (c_green1, message);
					flash_icon(tt_buddy, 5);
				}
				break;
			}
		}
	}
	set_scrollbar_len();
}

void del_buddy (const char *name, int len)
{
	int i;

	// find buddy
	for (i = 0; i < MAX_BUDDY; i++)
	{
		if (strncasecmp (name, buddy_list[i].name, len) == 0)
		{
			buddy_list[i].type = 0xff;
			memset (buddy_list[i].name, 0, sizeof (buddy_list[i].name));
			break;
		}
	}
	set_scrollbar_len();
}

void clear_buddy(void)
{
	int i;
	for(i=0; i<MAX_BUDDY; i++){
		buddy_list[i].type= 0xff;
		buddy_list[i].name[0]=0;
	}
	set_scrollbar_len();
}

int is_in_buddylist(const char *name)
{
	int i;
	char onlyname[32];
	if(!name || !*name) {
		return 0;
	}
	while(name[0] != '\0' && is_color ((unsigned char)name[0])){
		++name;
	}
	// strip GuildTag if existing
	for(i = 0; name[i]>32; i++){
		onlyname[i] = name[i];
	}
	onlyname[i] = '\0';

	for(i = 0; i < MAX_BUDDY; i++) {
		if(buddy_list[i].type < 0xff && strcasecmp(buddy_list[i].name, onlyname) == 0) {
			return 1;
		}
	}
	return 0;
}

void add_buddy_confirmation(char *name) {
	char *name_copy = malloc(strlen(name)+1);
	safe_snprintf(name_copy, strlen(name)+1, "%s", name);
	queue_push(buddy_request_queue, name_copy);
}
