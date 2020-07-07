/*
	Implements the "Generic special text window" feature.

	Messages from server channel 255 are displayed in a text
	window that automatically pops up.  Each message is treated
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
#include "chat.h"
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
/* initialised by initialise() */
static int server_popup_win = -1;
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
static int click_handler(window_info *win, int mx, int my, Uint32 flags)
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
	textId = 101;
	buttonId = 102;
	scroll_id = 103;
	text_widget_width = 0;
	text_widget_height = 0;
	actual_scroll_width = 0;
	num_text_lines = 0;
	scroll_line = 0;
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


static int server_popup_get_text_height(int num_lines)
{
	if (num_lines > 0)
		return 1 + 2 * sep + get_text_height(num_lines, CHAT_FONT, 1.0);
	else
		return 0;
}


static int get_non_text_height(void)
{
	return 3 * sep + widget_get_height(server_popup_win, buttonId);
}


static int get_height(int num_lines)
{
	return server_popup_get_text_height(num_lines) + get_non_text_height();
}


static void set_min_window_size(window_info *win)
{
	int min_height = get_height((text_message_is_empty (&widget_text)) ?0: 1);
	int min_width = win->box_size + 2 * sep + widget_get_width(server_popup_win, buttonId);
	int min_text_width = win->box_size + 2 * sep + (int)(5 * DEFAULT_FIXED_FONT_WIDTH);
	if (min_width < min_text_width)
		min_width = min_text_width;
	set_window_min_size (win->window_id, min_width, min_height);
}


/* the window resize handler, keep things neat and add scroll bar if required */
static int resize_handler(window_info *win, int width, int height)
{
	/* if there is no text widget, we're done */
	if (text_message_is_empty (&widget_text)) {
		return 1;
	}

	/* set the text widget height */
	text_widget_height = height - get_non_text_height();

	/* remove any existing scroll bar */
	widget_destroy(server_popup_win, scroll_id);
	actual_scroll_width = 0;

	/* only add a scroll bar if needed, i.e. more lines than we can display */
	if (text_widget_height < server_popup_get_text_height(num_text_lines)){
		actual_scroll_width = win->box_size;
	}

	/* set the text widget width, allowing for a scroll bar if required */
	text_widget_width = width - (2*sep + actual_scroll_width);

	/* resize the text widget and rewrap the text as the size will have changed */
	widget_resize(server_popup_win, textId, text_widget_width, text_widget_height);
	if (!text_message_is_empty (&widget_text))
	{
		num_text_lines = rewrap_message(&widget_text, CHAT_FONT, 1.0,
			text_widget_width - 2*sep, NULL);
	}

	/* if we (only now) need a scroll bar, adjust again */
	if (!actual_scroll_width && (text_widget_height < server_popup_get_text_height(num_text_lines)))
	{
		actual_scroll_width = win->box_size;
		text_widget_width = width - (2*sep + actual_scroll_width);
		widget_resize(server_popup_win, textId, text_widget_width, text_widget_height);
		/* rewrap the text again as the available width is now less */
		if (!text_message_is_empty (&widget_text))
		{
			num_text_lines = rewrap_message(&widget_text, CHAT_FONT, 1.0,
				text_widget_width - 2*sep, NULL);
		}
	}

	/* if the text widget is really short, the scroll bar can extent beyond the height */
	/* could be considered a bug in the scroll widget - try to avoid anyway */
	if (actual_scroll_width)
	{
		int local_min = get_non_text_height() + max2i(server_popup_get_text_height(1), 3 * win->box_size);
		if (height < local_min)
		{
			resize_window(server_popup_win, width, local_min);
			return 1;
		}
	}

	/* create the scroll bar reusing any existing scroll position */
	if (actual_scroll_width)
	{
		scroll_id = vscrollbar_add_extended( win->window_id, scroll_id, NULL,
			width - win->box_size, sep, win->box_size, text_widget_height,
			0, 1, scroll_line, 1, num_text_lines);
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

	/* move the text widget and OK button to the middle of the window bottom */
	widget_move(win->window_id, buttonId, (win->len_x - widget_get_width(win->window_id, buttonId) - actual_scroll_width)/2,
		win->len_y - sep - widget_get_height(win->window_id, buttonId));

	return 1;
} /* end resize_handler */


static int ui_scale_handler(window_info *win)
{
	button_resize(win->window_id, buttonId, 0, 0, win->current_scale);
	set_min_window_size(win);
	resize_window(win->window_id, win->len_x, ((actual_scroll_width) ?win->len_y : get_height(num_text_lines) ));
	return 1;
}

static void set_actual_window_size(window_info *win)
{
	const int unusable_width = 2 * sep + HUD_MARGIN_X;
	const int unusable_height = 2 * sep + HUD_MARGIN_Y;
	int winWidth, winHeight;

	/* do a pre-wrap of the text to the maximum screen width we can use
		 this will avoid the later wrap (after the resize) changing the number of lines */
	if (!text_message_is_empty(&widget_text))
	{
		num_text_lines = rewrap_message(&widget_text, CHAT_FONT, 1.0,
			(window_width - unusable_width) - 4*sep, NULL);
	}

	/* calc the text widget height from the number of lines */
	winHeight = get_non_text_height();
	if (!text_message_is_empty(&widget_text))
	{
		text_widget_height = server_popup_get_text_height(num_text_lines);
		winHeight = text_widget_height + get_non_text_height();
	}

	/* but limit to a maximum */
	if (winHeight > window_height - unusable_height - ((actual_scroll_width) ? win->title_height : 0))
	{
		winHeight = window_height - unusable_height - ((actual_scroll_width) ? win->title_height : 0);
		text_widget_height = winHeight - get_non_text_height();

		/* if we'll need a scroll bar allow for it in the width calulation */
		if (!text_message_is_empty(&widget_text) && (text_widget_height < server_popup_get_text_height(num_text_lines)))
			actual_scroll_width = win->box_size;
	}

	/* calc the require window width for the text size */
	if (!text_message_is_empty (&widget_text)) {
		/* The fudge is because the line wrapping code allows for a cursor to fit on the last
		 * position in the line, but this is not reflected in the actual width of the line. Hence,
		 * if the last character in the line is less wide than the cursor, the calculated width will
		 * be too small according to rewrap_message(). Add the width of a cursor to the required
		 * width to be certain it is large enough.
		 */
		int fudge = get_char_width_zoom('_', CHAT_FONT, win->current_scale);
		text_widget_width = fudge + 2*sep + widget_text.max_line_width;
	} else {
		text_widget_width = 0;
	}
	winWidth = text_widget_width + 2*sep + actual_scroll_width;

	/* but limit to a maximum */
	winWidth = min2i(winWidth, window_width - unusable_width);

	/* resize the window now we have the required size */
	/* new sizes and positions for the widgets will be calculated by the callback */
	resize_window(server_popup_win, winWidth, winHeight);

	/* calculate the best position then move the window */
	move_window(server_popup_win, -1, 0, sep + (window_width - unusable_width - win->len_x)/2, sep + (window_height - unusable_height - win->len_y)/2);
}

static int font_change_handler(window_info *win, font_cat cat)
{
	if (cat != win->font_category && cat != CHAT_FONT)
		return 0;
	set_min_window_size(win);
	set_actual_window_size(win);
	return 1;
}

/*
	Create the server popup window, destroying an existing window first.
*/
void display_server_popup_win(const unsigned char* message)
{
	int winWidth = 0;
	int winHeight = 0;
	Uint32 win_property_flags;
	window_info *win = NULL;
	int msg_len = strlen((const char*)message);

	/* exit now if message empty */
	if (!*message)
		return;

	/* write the message to the log file */
	write_to_log(CHAT_SERVER, message, msg_len);

	/* if the window already exists, copy new message to end */
	if (server_popup_win >= 0)
	{
		const char *sep_str = "\n\n";
		win = &windows_list.window[server_popup_win];

		/* resize to hold new message text + separator */
		widget_set_size(server_popup_win, textId, 1.0);
		resize_text_message_data(&widget_text, widget_text.len + 3*(msg_len+strlen(sep_str)));

		/* copy the message text into the text buffer */
		safe_strcat(widget_text.data, sep_str, widget_text.size);
		safe_strcat(widget_text.data, (const char*)message, widget_text.size);
		widget_text.len = strlen(widget_text.data);

		/* this will re-wrap the text and add a scrollbar, title and resize widget as required */
		resize_handler(win, win->len_x, win->len_y);

		/* always show the window */
		show_window(server_popup_win);

	} else {
		/* restart from scratch and initialise the window text widget text buffer */
		initialise();
		init_text_message(&widget_text, 3*msg_len);
		set_text_message_data(&widget_text, (const char*)message);
	}

	if (server_popup_win < 0){
		/* create the window with initial size and location */
		win_property_flags = ELW_USE_UISCALE|ELW_DRAGGABLE|ELW_USE_BACKGROUND|ELW_USE_BORDER|ELW_SHOW|ELW_ALPHA_BORDER|ELW_SWITCHABLE_OPAQUE;
		server_popup_win = create_window( "", -1, 0,
			0, 0, winWidth, winHeight, win_property_flags);
		set_window_handler( server_popup_win, ELW_HANDLER_RESIZE, &resize_handler);
		set_window_handler( server_popup_win, ELW_HANDLER_CLICK, &click_handler);
		set_window_handler( server_popup_win, ELW_HANDLER_UI_SCALE, &ui_scale_handler);
		set_window_handler( server_popup_win, ELW_HANDLER_FONT_CHANGE, &font_change_handler);

		if (server_popup_win >= 0 && server_popup_win < windows_list.num_windows)
			win = &windows_list.window[server_popup_win];

		/* create the OK button, setup its click handler and get its structure */
		buttonId = button_add_extended (server_popup_win, buttonId, NULL, 0,
			 0, 0, 0, 0, win->current_scale, "OK");
		widget_set_OnClick(server_popup_win, buttonId, close_handler);
	}
	if (win == NULL)
		return;

	/* create the text widget */
	if ((!text_message_is_empty (&widget_text)) && (widget_find(server_popup_win, textId) == NULL))
	{
		textId = text_field_add_extended(server_popup_win, textId, NULL, sep, sep,
			window_width, window_height, TEXT_FIELD_NO_KEYPRESS,
			CHAT_FONT, 1.0, &widget_text, 1, FILTER_NONE, sep, sep);
	}

	set_min_window_size(win);
	set_actual_window_size(win);
} /* end display_server_popup_win() */
