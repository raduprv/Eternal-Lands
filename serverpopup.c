/*
	Implements the "Generic special text window" feature.

	Messages from server channel 255 are displayed in a text 
	window that automatically pops up.  Each message is treaded 
	as a new block of text to display.  Any existing pop up 
	window will be closed and the text discarded.

	The first line of the message is used as the widow title.
	If there are more lines, they are displayed in a text widget.
	If the text width will not fit on screen, the text is wrapped.
	If all the lines can not be displayed on screen, a scroll bar
	will be added.
	An "OK" button is provided at the base of the window.
	The use of this feature can be disabled via the Server tab 
	in the game options window.
	Great length have been taken to make sure this works with
	different fonts and font sizes!

	bluap aka pjbroad November 2006
*/

/* todo
bugs:
	font width fudge for non standard font
may be do:
	reused last position for new windows if suitable
	key to display last message window?
*/

#include <string.h>
#include <stdlib.h>

#include "serverpopup.h"
#include "asc.h"
#include "elwindows.h"
#include "gamewin.h"
#include "gl_init.h"
#include "text.h"
#include "widgets.h"

/* these are visible externally and exported in the header file */
int server_popup_win = -1;
int use_server_pop_win = 1;
int server_pop_chan = CHAT_POPUP;
/* initialised by initialise() */
int server_popup_win_x;
int server_popup_win_y;

/* these are visible only to this code module but are needed by handlers for example */
static const int sep = 5;
static const int scroll_width = 20;
/* initialised by initialise() */
static int min_width;
static int min_height;
static text_message widget_text;
static char *message_body;
static int textId;
static int buttonId;
static int scroll_id;
static int text_widget_width;
static int text_widget_height;
static int actual_scroll_width;
static int num_text_lines;
static int scroll_line;

/*
	return the offset into the text_message of the beginning of the specified line
	if the line number does not exist, set the passed line to the last line
	there is a similar function in text.c find_line_nr() but it is only for chat channel windows
*/

static int get_line_number_offset(int *line, const text_message * const wt)
{
	int i, curr_line, last_start, is_start;
	for (i=0, curr_line=0, last_start=0, is_start=1; i<strlen(wt->data); ++i)
	{
		/* if a line start */
		if (is_start){
			/* return the start offset if this is the line number we want */
			if (curr_line == *line){
				 return i;
			}
			last_start = i; /* remember in case this is the last line */
			is_start = 0;
		}
		/* if a line end */
		if (wt->data[i] == '\n' || wt->data[i] == '\r'){
			curr_line++;
			is_start = 1;
		}
	}
	/* the line number was not found so set the passed value to the actual last line number */
	*line = curr_line;
	return last_start;
}


/* set the displayed start of the text_message to the current scroll line */
static void set_text_line()
{
	int offset = get_line_number_offset(&scroll_line, &widget_text);  
	text_field_set_buf_pos( server_popup_win, textId, 0, offset );
}


/* scroll click handler, set the displayed text start to the new scroll position */
static int scroll_click(widget_list *widget, int mx, int my, Uint32 flags)
{
	scroll_line = vscrollbar_get_pos( server_popup_win, scroll_id );
	set_text_line();
	return 1;
}


/* scroll drag handler, set the displayed text start to the new scroll position */
static int scroll_drag(widget_list *widget, int mx, int my, Uint32 flags, int dx, int dy)
{
	return scroll_click(widget, mx, my, flags);
}


/* if the mouse scroll wheel is used, move the scroll bar if we have one */
int click_handler(window_info *win, int mx, int my, Uint32 flags)
{
	if (!actual_scroll_width){
		return 1;
	}
	if (flags & ELW_WHEEL_UP){
		vscrollbar_scroll_up(server_popup_win, scroll_id);
	}
	else if (flags & ELW_WHEEL_DOWN){
		vscrollbar_scroll_down(server_popup_win, scroll_id);
	}
	scroll_line = vscrollbar_get_pos(server_popup_win, scroll_id);
	set_text_line();
	return 1;
}


/* static vars are only required for window lifetime */
static void initialise()
{
	server_popup_win = -1;
	min_width = 0;
	min_height = 0;
	message_body = NULL;
	textId = 101;
	buttonId = 102;
	scroll_id = 103;
	text_widget_width = 0;
	text_widget_height = 0;
	actual_scroll_width = 0;
	num_text_lines = 0;
	scroll_line = 0;
	server_popup_win_x = 100;
	server_popup_win_y = 100;
}
	

/* destroy all the widgets, deallocate memory and destroy the main window */
static int close_handler(widget_list *widget, int mx, int my, Uint32 flags)
{
	widget_destroy(server_popup_win, scroll_id);
	widget_destroy(server_popup_win, buttonId);
	widget_destroy(server_popup_win, textId);
	if (message_body != NULL){
		free(message_body);
		message_body = NULL;
	}
	destroy_window(server_popup_win);
	initialise();
	return 1;
}


/* calculate window height give a number of lines */
static int get_height(int num_lines)
{
	widget_list *button_widget = widget_find(server_popup_win, buttonId);
	int retvalue;
	if (!button_widget){
		return 0;
	}
	retvalue = 2 * sep + button_widget->len_y;
	if (num_lines > 0){
		retvalue += 3*sep + num_lines * DEFAULT_FONT_Y_LEN * chat_zoom;
	}
	return retvalue;
}
	

/* the window resize handler, keep things neat and add scroll bar if required */
static int resize_handler(window_info *win, int width, int height)
{
	widget_list *button_widget;
 
	/* ensure the minimum size, this function will get called
		 again with the new size so exit after the call */
	if (width < min_width)
	{
		resize_window(server_popup_win, min_width, height);
		return 1;
	}
	if (height < min_height)
	{
		resize_window(server_popup_win, width, min_height);
		return 1;
	}
	
	/* make sure the text font is set so width calculations work properly */
	set_font(chat_font);

	/* get the ok button information and move it to the middle of the window bottom */
	button_widget = widget_find(server_popup_win, buttonId);
	widget_move(server_popup_win, buttonId, (width - button_widget->len_x)/2,
		height - sep - button_widget->len_y);

	/* if there is no text widget, we're done */
	if (!strlen(widget_text.data)){
		return 1;
	}
	
	/* set the text widget height */		
	text_widget_height = height - (3*sep + button_widget->len_y);

	/* remove any existing scroll bar */
	widget_destroy(server_popup_win, scroll_id);
	actual_scroll_width = 0;
	
	/* only add a scroll bar if needed, i.e. more lines than we can display */
	if (text_widget_height < (2*sep + num_text_lines * DEFAULT_FONT_Y_LEN * chat_zoom)){
		actual_scroll_width = scroll_width;
	}

	/* set the text widget width, allowing for a scroll bar if required */
	text_widget_width = width - (2*sep + actual_scroll_width);
	
	/* resize the text widget and rewrap the text as the size will have changed */
	widget_resize(server_popup_win, textId, text_widget_width, text_widget_height);
	if (strlen(widget_text.data)){
		num_text_lines = rewrap_message(&widget_text, chat_zoom, text_widget_width - 2*sep, NULL);
	}

	/* if we (only now) need a scroll bar, adjust again */
	if (!actual_scroll_width &&
		(text_widget_height < (2*sep + num_text_lines * DEFAULT_FONT_Y_LEN * chat_zoom)))
	{
		actual_scroll_width = scroll_width;
		text_widget_width = width - (2*sep + actual_scroll_width);
		widget_resize(server_popup_win, textId, text_widget_width, text_widget_height);
		/* rewrap the text again as the available width is now less */
		if (strlen(widget_text.data))
			num_text_lines = rewrap_message(&widget_text, chat_zoom, text_widget_width - 2*sep, NULL);
	}
	
	/* if the text widget is really short, the scroll bar can extent beyond the height */
	/* could be considered a bug in the scroll widget - prevent for normal font sizes anyway */
	if (actual_scroll_width && (height < get_height(3)))
	{
		resize_window(server_popup_win, width, get_height(3));
		return 1;
	}

	/* create the scroll bar reusing any existing scroll position */
	if (actual_scroll_width)
	{
		scroll_id = vscrollbar_add_extended( server_popup_win, scroll_id, NULL,
		width - (scroll_width + sep), sep, scroll_width, text_widget_height,
		0, 1, 0.77f, 0.57f, 0.39f, scroll_line, 1, num_text_lines);
		widget_set_OnDrag(server_popup_win, scroll_id, scroll_drag);
		widget_set_OnClick(server_popup_win, scroll_id, scroll_click);
		set_text_line();
	}
	/* if no scroll bar, make sure the text is at the start of the buffer */
	else
	{
		scroll_line = 0;
		text_field_set_buf_pos( server_popup_win, textId, 0, 0 );
	}
	
	return 1;
} /* end resize_handler */


/*
	Create the server popup window, destroying an existing window first.
*/
void display_server_popup_win(const char * const message)
{		 
	const int unusable_width = 75;
	const int unusable_height = 75;
	int winWidth = 0;
	int winHeight = 0;
	int max_line_len = 0;
	int last_line_len = 0;
	widget_list *button_widget = NULL;
	char win_title[ELW_TITLE_SIZE];
	int our_root_win = -1;
	int message_body_size = 0;
	int body_index;
	int i, j;
	Uint32 win_property_flags;
	
	/* if the window already exists, destroy it first, freeing all the memory
		 always initialising vars */
	if (server_popup_win > 0){
		close_handler(NULL, 0, 0, 0);
	} else {
		initialise();
	}

	/* exit now if message empty */
	if (!strlen(message)){
		return;
	}
	
	/* write the message to the log file */
	write_to_log((unsigned char*)message, strlen(message));
		
	/* copy the first line of text into the title buffer */
	for (i=0, j=0; (i<strlen(message)) && (j<ELW_TITLE_SIZE-1); ++i)
	{
		if (message[i] == '\n'){
			break;
		}
		/* could remove colour codes
			if ((message[i] > 31) && (message[i] < 127)) */
		win_title[j++] = message[i];
	}
	win_title[j] = '\0';
	
	/* find the start of the message body, after the first '\n', allow for no body */
	for (body_index=0; body_index<strlen(message); ++body_index)
	{
		if (message[body_index] == '\n'){
			body_index++;
			break;
		}
	}
	
	/* make a copy of the message body so we can modify it and so the caller can free the memory
		 make the copy bigger than we need so it can be text rewrapped */
	message_body_size = 2*strlen(message);
	message_body = (char *)calloc(message_body_size, sizeof(Uint8));
	safe_strncpy(message_body, &message[body_index], message_body_size * sizeof(Uint8));
		
	/* initialise the window text widget text buffer */
	widget_text.chan_idx = CHAT_NONE;
	widget_text.data = message_body;
	widget_text.len = strlen(message_body);
	widget_text.size = message_body_size;
	widget_text.wrap_width = 0;
	widget_text.wrap_zoom = 0;
	widget_text.wrap_lines = 0;
	widget_text.max_line_width = 0;
	
	/* make sure the text font is set so width calculations work properly */
		set_font(chat_font);
	
	/* do a pre-wrap of the text to the maximum screen width we can use
		 this will avoid the later wrap (after the resize) changing the number of lines */
	if (strlen(widget_text.data)){
		num_text_lines = rewrap_message(&widget_text, chat_zoom, (window_width - unusable_width) - 4*sep, NULL);
	}
	
	/* find the longest line */
	for (i=0; i<strlen(message_body); ++i)
	{
		if ((message_body[i] == '\n') || (message_body[i] == '\r'))
		{
			if (last_line_len > max_line_len){
				 max_line_len = last_line_len;
			}
			last_line_len = 0;
		} else {
			last_line_len++;
		}
	}
	if (last_line_len > max_line_len){
		max_line_len = last_line_len;
	}
		
	/* config and alt-w control if shown on top of console */
	if (!windows_on_top){
		 our_root_win = game_root_win;
	}
		
	/* create the window with initial size and location */
	win_property_flags = ELW_TITLE_BAR|ELW_DRAGGABLE|ELW_USE_BACKGROUND|ELW_USE_BORDER|ELW_SHOW|ELW_TITLE_NAME|ELW_ALPHA_BORDER;
	if (strlen(widget_text.data)){
		win_property_flags |= ELW_RESIZEABLE;
	}
	server_popup_win = create_window( win_title, our_root_win, 0,
		server_popup_win_x, server_popup_win_y, winWidth, winHeight, win_property_flags);
	set_window_handler( server_popup_win, ELW_HANDLER_RESIZE, &resize_handler);
	set_window_handler( server_popup_win, ELW_HANDLER_CLICK, &click_handler);

	/* create the OK button, setup its click handler and get its structure */
	buttonId = button_add_extended (server_popup_win, buttonId, NULL, 0,
		 0, 0, 0, 0, chat_zoom, 0.77f, 0.57f, 0.39f, "OK");
	widget_set_OnClick(server_popup_win, buttonId, close_handler);
	button_widget = widget_find(server_popup_win, buttonId);
 
	/* calc the text widget height from the number of lines */
	winHeight = 2*sep + button_widget->len_y;
	if (strlen(widget_text.data))
	{
		text_widget_height = 1 + 2*sep + num_text_lines * DEFAULT_FONT_Y_LEN * chat_zoom;
		winHeight = text_widget_height + 3*sep + button_widget->len_y;
	}
	
	/* make sure we can always display at least one line (if any) and the OK button */
	if (!strlen(widget_text.data)){
		min_height = get_height(0);
	} else if (min_height < get_height(1)){
		min_height = get_height(1);
	}
	
	/* ensure a minimum height */
	if (winHeight < min_height)
		winHeight = min_height;
	
	/* but limit to a maximum */
	if (winHeight > window_height - unusable_height)
	{
		winHeight = window_height - unusable_height;
		text_widget_height = winHeight - 3*sep + button_widget->len_y;
	}
	
	/* if we'll need a scroll bar allow for it in the width calulation */
	if (strlen(widget_text.data) && (text_widget_height < (2*sep + num_text_lines * DEFAULT_FONT_Y_LEN * chat_zoom))){
		actual_scroll_width = scroll_width;
	}

	/* calc the require window width for the text size */
	/* the fudge is that the width cal doe snot work 100% exactly for some fonts :( */
	if (strlen(widget_text.data)){
		text_widget_width = sep/*fudge*/ + 2*sep + widget_text.max_line_width;
	} else {
		text_widget_width = 0;
	}
	winWidth = text_widget_width + 2*sep + actual_scroll_width;
	
	/* make sure we can always display at least the OK button
		 Note, until Patch #1624 is applied, this will still draw the button off the window */
	if (min_width < 2*sep + button_widget->len_x){
		min_width = 2*sep + button_widget->len_x;
	}
	
	/* use the title width to set the minimum width, numbers from draw_window_title() would help */
	if (min_width < (50 + ((get_string_width((unsigned char*)win_title)*8)/12))){
		min_width = 50 + ((get_string_width((unsigned char*)win_title)*8)/12);
	}
		
	/* insure a minimum width */
	if (winWidth < min_width){
		winWidth = min_width;
	}

	/* but limit to a maximum */
	if (winWidth > window_width - unusable_width){
		winWidth = window_width - unusable_width;
	}
	
	/* create the text widget */
	if (strlen(widget_text.data)){
		textId = text_field_add_extended( server_popup_win, textId, NULL, sep, sep,
			text_widget_width, text_widget_height, TEXT_FIELD_BORDER|TEXT_FIELD_NO_KEYPRESS,
			chat_zoom, 0.77f, 0.57f, 0.39f, &widget_text, 1, FILTER_NONE, sep, sep, -1.0, -1.0, -1.0);
	}

	/* resize the window now we have the required size */
	/* new sizes and positions for the widgets will be calculated by the callback */
	resize_window(server_popup_win, winWidth, winHeight);
	
	/* calculate the best position then move the window */
	server_popup_win_x = (window_width - unusable_width - winWidth)/2;
	server_popup_win_y = (window_height - unusable_height - winHeight)/2;
	move_window(server_popup_win, our_root_win, 0, server_popup_win_x, server_popup_win_y);

} /* end display_server_popup_win() */

