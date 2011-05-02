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
int server_pop_chan = CHAT_POPUP;
int use_server_pop_win = 1;

/* these are visible only to this code module but are needed by handlers for example */
static const int sep = 5;
static const int scroll_width = 20;
/* initialised by initialise() */
static int server_popup_win = -1;
static int server_popup_win_x;
static int server_popup_win_y;
static int min_width;
static int min_height;
static text_message widget_text;
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
	for (i=0, curr_line=0, last_start=0, is_start=1; i<wt->len; ++i)
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
	free_text_message_data(&widget_text);
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
	if (text_message_is_empty (&widget_text)) {
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
	if (!text_message_is_empty (&widget_text)) {
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
		if (!text_message_is_empty (&widget_text))
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

		/* if we have a scroll bar, enable the title and resize properties */
		win->flags |= ELW_RESIZEABLE | ELW_TITLE_BAR;
	}
	/* if no scroll bar, make sure the text is at the start of the buffer */
	else
	{
		scroll_line = 0;
		text_field_set_buf_pos( server_popup_win, textId, 0, 0 );
		if (win->flags & ELW_RESIZEABLE)
			win->flags ^= ELW_RESIZEABLE;
		if (win->flags & ELW_TITLE_BAR)
			win->flags ^= ELW_TITLE_BAR;
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
	widget_list *button_widget = NULL;
	Uint32 win_property_flags;
	
	/* exit now if message empty */
	if (!strlen(message)){
		return;
	}
	
	/* write the message to the log file */
	write_to_log (CHAT_SERVER, (unsigned char*)message, strlen(message));
	
	/* if the window already exists, copy new message to end */
	if (server_popup_win >= 0){
		
		window_info *win = &windows_list.window[server_popup_win];
		char *sep_str = "\n\n";

		/* resize to hold new message text + separator */
		resize_text_message_data (&widget_text, widget_text.len + 2*(strlen(message)+strlen(sep_str)));

		/* copy the message text into the text buffer */
		safe_strcat (widget_text.data, sep_str, widget_text.size);
		safe_strcat (widget_text.data, message, widget_text.size);
		widget_text.len = strlen(widget_text.data);

		/* this will re-wrap the text and add a scrollbar, title and resize widget as required */
		resize_handler(win, win->len_x, win->len_y);

		/* always show the window */
		show_window(server_popup_win);

	} else {
		/* restart from scratch and initialise the window text widget text buffer */
		initialise();
		init_text_message (&widget_text, 2*strlen(message));
		set_text_message_data (&widget_text, message);
	}

	/* make sure the text font is set so width calculations work properly */
	set_font(chat_font);
	
	/* do a pre-wrap of the text to the maximum screen width we can use
		 this will avoid the later wrap (after the resize) changing the number of lines */
	if (!text_message_is_empty (&widget_text)) {
		num_text_lines = rewrap_message(&widget_text, chat_zoom, (window_width - unusable_width) - 4*sep, NULL);
	}

	if (server_popup_win < 0){
		/* create the window with initial size and location */
		win_property_flags = ELW_DRAGGABLE|ELW_USE_BACKGROUND|ELW_USE_BORDER|ELW_SHOW|ELW_ALPHA_BORDER|ELW_SWITCHABLE_OPAQUE;
		server_popup_win = create_window( "", -1, 0,
			server_popup_win_x, server_popup_win_y, winWidth, winHeight, win_property_flags);
		set_window_handler( server_popup_win, ELW_HANDLER_RESIZE, &resize_handler);
		set_window_handler( server_popup_win, ELW_HANDLER_CLICK, &click_handler);
	
		/* create the OK button, setup its click handler and get its structure */
		buttonId = button_add_extended (server_popup_win, buttonId, NULL, 0,
			 0, 0, 0, 0, chat_zoom, 0.77f, 0.57f, 0.39f, "OK");
		widget_set_OnClick(server_popup_win, buttonId, close_handler);
	}

	button_widget = widget_find(server_popup_win, buttonId);

	/* calc the text widget height from the number of lines */
	winHeight = 2*sep + button_widget->len_y;
	if (!text_message_is_empty (&widget_text))
	{
		text_widget_height = 1 + 2*sep + num_text_lines * DEFAULT_FONT_Y_LEN * chat_zoom;
		winHeight = text_widget_height + 3*sep + button_widget->len_y;
	}
	
	/* make sure we can always display at least one line (if any) and the OK button */
	if (text_message_is_empty (&widget_text)) {
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
	if (!text_message_is_empty (&widget_text) && (text_widget_height < (2*sep + num_text_lines * DEFAULT_FONT_Y_LEN * chat_zoom))){
		actual_scroll_width = scroll_width;
	}

	/* calc the require window width for the text size */
	/* the fudge is that the width cal does not work 100% exactly for some fonts :( */
	if (!text_message_is_empty (&widget_text)) {
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
	
	/* insure a minimum width */
	if (winWidth < min_width){
		winWidth = min_width;
	}

	/* but limit to a maximum */
	if (winWidth > window_width - unusable_width){
		winWidth = window_width - unusable_width;
	}
	
	/* create the text widget */
	if ((!text_message_is_empty (&widget_text)) && (widget_find(server_popup_win, textId) == NULL)) {
		textId = text_field_add_extended( server_popup_win, textId, NULL, sep, sep,
			text_widget_width, text_widget_height, TEXT_FIELD_NO_KEYPRESS,
			chat_zoom, 0.77f, 0.57f, 0.39f, &widget_text, 1, FILTER_NONE, sep, sep);
	}

	/* resize the window now we have the required size */
	/* new sizes and positions for the widgets will be calculated by the callback */
	resize_window(server_popup_win, winWidth, winHeight);
	
	/* calculate the best position then move the window */
	server_popup_win_x = (window_width - unusable_width - winWidth)/2;
	server_popup_win_y = (window_height - unusable_height - winHeight)/2;
	move_window(server_popup_win, -1, 0, server_popup_win_x, server_popup_win_y);

} /* end display_server_popup_win() */

