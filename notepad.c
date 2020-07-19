/******************************************
 * notepad.c - Player notes in xml format *
 *  includes generic keypress handlers    *
 *  and popup window functionality        *
 ******************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libxml/parser.h>
#include <ctype.h>
#include <SDL.h>
#include "asc.h"
#include "context_menu.h"
#include "elwindows.h"
#include "errors.h"
#include "io/elfilewrapper.h"
#include "gamewin.h"
#include "gl_init.h"
#include "init.h"
#include "hud.h"
#include "loginwin.h"
#include "notepad.h"
#include "tabs.h"
#include "text.h"
#include "translate.h"


/******************************************
 *             Popup Section              *
 ******************************************/

void init_ipu (INPUT_POPUP *ipu, int parent, int maxlen, int rows, int cols, void cancel(void *), void input(const char *, void *))
{
	ipu->text_flags = TEXT_FIELD_BORDER|TEXT_FIELD_EDITABLE|TEXT_FIELD_NO_KEYPRESS|TEXT_FIELD_MOUSE_EDITABLE;
	ipu->text_flags |= (rows>1) ?TEXT_FIELD_SCROLLBAR :0;

	ipu->popup_win = ipu->popup_field = ipu->popup_line = ipu->popup_label
		= ipu->popup_ok = ipu->popup_no = -1;
	ipu->x = ipu->y = 0;

	ipu->popup_cancel = cancel;
	ipu->popup_input = input;
	ipu->popup_line_text = maxlen ? calloc(maxlen+1, 1) : NULL;
	ipu->data = NULL;

	ipu->parent = parent;
	ipu->maxlen = maxlen;
	ipu->rows = rows;
	ipu->cols = cols;
	ipu->accept_do_not_close = ipu->allow_nonprint_chars = 0;
}

void clear_popup_window(INPUT_POPUP *ipu)
{
	if (ipu->rows == 1)
		pword_clear(ipu->popup_win, ipu->popup_line);
	else
		text_field_clear (ipu->popup_win, ipu->popup_field);
	hide_window (ipu->popup_win);
}

void close_ipu (INPUT_POPUP *ipu)
{
	if (ipu->popup_line_text != NULL)
	{
		free(ipu->popup_line_text);
		ipu->popup_line_text = NULL;
	}
	if (ipu->popup_win > 0)
	{
		destroy_window(ipu->popup_win);
		clear_text_message_data (&ipu->popup_text);
		free_text_message_data (&ipu->popup_text);
		init_ipu(ipu, -1, 0, 0, 0, NULL, NULL);
	}
}

static INPUT_POPUP *ipu_from_widget (widget_list *w)
{
	if ((w == NULL) || (w->window_id < 0) || (windows_list.num_windows < w->window_id)
		|| (windows_list.window[w->window_id].data == NULL))
	{
		fprintf(stderr, "%s: NULL pointer error\n", __FUNCTION__);
		return NULL;
	}
	return (INPUT_POPUP *)windows_list.window[w->window_id].data;
}

static INPUT_POPUP *ipu_from_window (window_info *win)
{
	if ((win == NULL) || (win->data == NULL))
	{
		fprintf(stderr, "%s: NULL pointer error\n", __FUNCTION__);
		return NULL;
	}
	return (INPUT_POPUP *)win->data;
}

static void accept_popup_window (INPUT_POPUP *ipu)
{
	int istart, iend, itmp, len;
	unsigned char *data;

	if (ipu->rows == 1)
	{
		data = ipu->popup_line_text;
		len = strlen((const char*)data);
	}
	else
	{
		data = (unsigned char*)ipu->popup_text.data;
		len = ipu->popup_text.len;
	}

	// skip leading spaces
	istart = 0;
	while ( istart < len && isspace (data[istart]) )
		istart++;
	if (istart >= len)
		// empty string
		return;

	// remove soft breaks
	iend = itmp = istart;
	for (iend = istart, itmp = istart; iend < len; ++iend)
	{
		if (data[iend] != '\r')
			data[itmp++] = data[iend];
	}
	len = itmp;

	// stop at first non-printable character if allow_nonprint_chars not set
	if (!ipu->allow_nonprint_chars)
	{
		iend = istart;
		while (iend < len && is_printable(data[iend]))
			++iend;
		if (iend == istart)
			// empty string
			return;
	}

	// send the entered text to the window owner then clear up
	data[iend] = '\0';
	if (ipu->popup_input != NULL)
		(*ipu->popup_input) ((char*)&data[istart], ipu->data);
	if (!ipu->accept_do_not_close)
		clear_popup_window (ipu);
}

static int popup_cancel_button_handler(widget_list *w,
	int UNUSED(mx), int UNUSED(my), Uint32 flags)
{
	INPUT_POPUP *ipu = ipu_from_widget(w);
	if (ipu == NULL) return 0;

	// only handle mouse button clicks, not scroll wheels moves
	if ( (flags & ELW_MOUSE_BUTTON) == 0) return 0;

	// call the cancel function of the window owner then clear up
	if (ipu->popup_cancel != NULL)
		(*ipu->popup_cancel) (ipu->data);
	clear_popup_window (ipu);

	return 1;
}

static int popup_ok_button_handler(widget_list *w,
	int UNUSED(mx), int UNUSED(my), Uint32 flags)
{
	INPUT_POPUP *ipu = ipu_from_widget(w);
	if (ipu == NULL) return 0;

	// only handle mouse button clicks, not scroll wheels moves
	if ( (flags & ELW_MOUSE_BUTTON) == 0) return 0;

	accept_popup_window (ipu);

	return 1;
}

static int popup_keypress_handler(window_info *win,
	int UNUSED(mx), int UNUSED(my), SDL_Keycode key_code, Uint32 key_unicode, Uint16 key_mod)
{
	INPUT_POPUP *ipu = ipu_from_window(win);
	if (ipu == NULL) return 0;

	if (key_code == SDLK_RETURN || key_code == SDLK_KP_ENTER)
	{
		accept_popup_window (ipu);
		return 1;
	}
	else if (key_code == SDLK_ESCAPE)
	{
		if (ipu->popup_cancel != NULL)
			(*ipu->popup_cancel) (ipu->data);
		clear_popup_window (ipu);
		return 1;
	}
	else
	{
		// send other key presses to the text field
		int widget_id = ipu->rows == 1 ? ipu->popup_line : ipu->popup_field;
		widget_list *tfw = widget_find (win->window_id, widget_id);
		if (tfw != NULL)
		{
			// FIXME? This is a bit hackish, we don't allow the
			// widget to process keypresses, so that we end up
			// in this handler. But now we need the default
			// widget handler to take care of this keypress, so
			// we clear the flag, let the widget handle it, then
			// set the flag again.
			int res;
			tfw->Flags &= ~TEXT_FIELD_NO_KEYPRESS;
			res = widget_handle_keypress (tfw, mx - tfw->pos_x, my - tfw->pos_y, key_code, key_unicode, key_mod);
			tfw->Flags |= TEXT_FIELD_NO_KEYPRESS;
			return res;
		}
	}

	// shouldn't get here
	return 0;
}

static int popup_ui_scale_handler(window_info *win)
{
	int seperator = (int)(0.5 + win->current_scale * 10);
	int y_len = seperator;
	int char_width = get_avg_char_width_zoom(win->font_category, win->current_scale);
	int max_x = 0;
	int ok_w = 0;
	int no_w = 0;
	int tmp = 0;
	INPUT_POPUP *ipu = ipu_from_window(win);
	if (ipu == NULL) return 0;

	widget_set_size(win->window_id, ipu->popup_label, win->current_scale);
	widget_resize(win->window_id, ipu->popup_label, 0, 0);
	max_x = 2 * seperator + widget_get_width(win->window_id, ipu->popup_label);

	if (ipu->rows == 1)
	{
		widget_set_flags(win->window_id, ipu->popup_field, WIDGET_DISABLED);
		widget_unset_flags(win->window_id, ipu->popup_line, WIDGET_DISABLED);

		widget_set_size(win->window_id, ipu->popup_line, win->current_scale);
		widget_resize(win->window_id, ipu->popup_line,
			ipu->cols * char_width + 2 * seperator + 2 * 5,
			1 + ipu->rows * win->default_font_len_y + 2 * 5);
		// FIXME: hack - the text feld scrollbar does not function correct until the next resize, so do it twice for now
		widget_resize(win->window_id, ipu->popup_line,
			ipu->cols * char_width + 2 * seperator + 2 * 5,
			1 + ipu->rows * win->default_font_len_y + 2 * 5);
		max_x = max2i(max_x, 2 * seperator + widget_get_width(win->window_id, ipu->popup_line));

		button_resize(win->window_id, ipu->popup_ok, 0, 0, win->current_scale);
		button_resize(win->window_id, ipu->popup_no, 0, 0, win->current_scale);

		ok_w = widget_get_width(win->window_id, ipu->popup_ok);
		no_w = widget_get_width(win->window_id, ipu->popup_no);
		max_x = max2i(max_x, ok_w + no_w + 3 * seperator);
		tmp = (max_x - ok_w - no_w) / 3;

		widget_move(win->window_id, ipu->popup_label, (max_x - widget_get_width(win->window_id, ipu->popup_label)) / 2, y_len);
		y_len += widget_get_height(win->window_id, ipu->popup_label) + seperator;

		widget_move(win->window_id, ipu->popup_line, (max_x - widget_get_width(win->window_id, ipu->popup_line)) / 2, y_len);
		y_len += widget_get_height(win->window_id, ipu->popup_line) + seperator;
	}
	else
	{
		widget_set_flags(win->window_id, ipu->popup_field, ipu->text_flags);
		widget_set_flags(win->window_id, ipu->popup_line, WIDGET_DISABLED);

		widget_set_size(win->window_id, ipu->popup_field, win->current_scale);
		widget_resize(win->window_id, ipu->popup_field,
			ipu->cols * char_width + 2 * seperator + 2 * 5,
			1 + ipu->rows * win->default_font_len_y + 2 * 5);
		// FIXME: hack - the text feld scrollbar does not function correct until the next resize, so do it twice for now
		widget_resize(win->window_id, ipu->popup_field,
			ipu->cols * char_width + 2 * seperator + 2 * 5,
			1 + ipu->rows * win->default_font_len_y + 2 * 5);
		max_x = max2i(max_x, 2 * seperator + widget_get_width(win->window_id, ipu->popup_field));

		button_resize(win->window_id, ipu->popup_ok, 0, 0, win->current_scale);
		button_resize(win->window_id, ipu->popup_no, 0, 0, win->current_scale);

		ok_w = widget_get_width(win->window_id, ipu->popup_ok);
		no_w = widget_get_width(win->window_id, ipu->popup_no);
		max_x = max2i(max_x, ok_w + no_w + 3 * seperator);
		tmp = (max_x - ok_w - no_w) / 3;

		widget_move(win->window_id, ipu->popup_label, (max_x - widget_get_width(win->window_id, ipu->popup_label)) / 2, y_len);
		y_len += widget_get_height(win->window_id, ipu->popup_label) + seperator;

		widget_move(win->window_id, ipu->popup_field, (max_x - widget_get_width(win->window_id, ipu->popup_field)) / 2, y_len);
		y_len += widget_get_height(win->window_id, ipu->popup_field) + seperator;
	}

	widget_move(win->window_id, ipu->popup_ok, tmp, y_len);
	widget_move(win->window_id, ipu->popup_no, 2 * tmp + ok_w, y_len);
	y_len += widget_get_height(win->window_id, ipu->popup_ok) + seperator;

	resize_window(win->window_id, max_x, y_len);

	return 1;
}

void centre_popup_window(INPUT_POPUP *ipu)
{
	window_info *win = NULL;
	if (ipu == NULL) return;
	if (ipu->popup_win < 0 || ipu->popup_win >= windows_list.num_windows) return;

	win = &windows_list.window[ipu->popup_win];

	if ((ipu->parent > -1) && (ipu->parent < windows_list.num_windows))
	{
		window_info *pwin = &windows_list.window[ipu->parent];
		move_window(win->window_id, win->pos_id,win->pos_loc, win->pos_x + (pwin->len_x - win->len_x) / 2, win->pos_y + (pwin->len_y - win->len_y) / 2);
	}
	else
	{
		move_window(win->window_id, win->pos_id,win->pos_loc, (window_width - win->len_x) / 2, (window_height - win->len_y) / 2);
	}
}


void display_popup_win (INPUT_POPUP *ipu, const char* label)
{
	if(ipu->popup_win < 0)
	{
		window_info *win = NULL;
		int widget_id = 100;

		Uint32 flags = (ELW_USE_UISCALE|ELW_WIN_DEFAULT) & ~ELW_CLOSE_BOX;
		ipu->popup_win = create_window (win_prompt, ipu->parent, 0, ipu->x, ipu->y, 0, 0, flags);

		if (ipu->popup_win >= 0 && ipu->popup_win < windows_list.num_windows)
			win = &windows_list.window[ipu->popup_win];
		else
		{
			ipu->popup_win = -1;
			return;
		}

		// clear the buffers
		*ipu->popup_line_text = '\0';
		init_text_message (&ipu->popup_text, ipu->maxlen);
		set_text_message_color (&ipu->popup_text, 0.77f, 0.57f, 0.39f);

		// Label
		ipu->popup_label = label_add_extended(ipu->popup_win, widget_id++, NULL, 0, 0, 0, win->current_scale, label);

		// Input
		ipu->popup_field = text_field_add_extended(ipu->popup_win, widget_id++, NULL, 0, 0, 0, 0,
			ipu->text_flags, win->font_category, 1.0, &ipu->popup_text, 1, FILTER_ALL, 5, 5);
		widget_set_flags(win->window_id, ipu->popup_field, WIDGET_DISABLED);
		ipu->popup_line = pword_field_add_extended(ipu->popup_win, widget_id++,
			NULL, 0, 0, 0, 0, P_TEXT, 1.0, ipu->popup_line_text, ipu->maxlen);
		widget_set_flags(win->window_id, ipu->popup_line, WIDGET_DISABLED);

		// Accept
		ipu->popup_ok = button_add_extended (ipu->popup_win, widget_id++, NULL, 0, 0, 0, 0, 0, 1.0, button_okay);
		widget_set_OnClick (ipu->popup_win, ipu->popup_ok, popup_ok_button_handler);

		// Reject
		ipu->popup_no = button_add_extended (ipu->popup_win, widget_id++, NULL, 0, 0, 0, 0, 0, 1.0, button_cancel);
		widget_set_OnClick (ipu->popup_win, ipu->popup_no, popup_cancel_button_handler);

		set_window_handler (ipu->popup_win, ELW_HANDLER_KEYPRESS, (int (*)())&popup_keypress_handler);
		set_window_handler (ipu->popup_win, ELW_HANDLER_UI_SCALE, popup_ui_scale_handler);

		win->data = ipu;
		popup_ui_scale_handler(win);
	}
	else
	{
		if ((ipu->parent > -1) && (ipu->parent < windows_list.num_windows))
		{
			window_info *win = &windows_list.window[ipu->parent];
			move_window(ipu->popup_win, ipu->parent, 0, win->pos_x+ipu->x, win->pos_y+ipu->y);
		}
		text_field_clear(ipu->popup_win, ipu->popup_field);
		label_set_text (ipu->popup_win, ipu->popup_label, label);
		show_window (ipu->popup_win);
		select_window (ipu->popup_win);
		popup_ui_scale_handler(&windows_list.window[ipu->popup_win]);
	}
}


/******************************************
 *             Notepad Section            *
 ******************************************/

//Macro Definitions
#define NOTE_LIST_INIT_SIZE 5
#define NOTE_NAME_LEN       25

#define MIN_NOTE_SIZE	128

// Private to this module
typedef struct
{
	char name[NOTE_NAME_LEN];	// Name to display on tab title.
	int window;			// Track which window it owns.
	int input;			// Track it's text buffer
	int button;			// Track it's close button
	text_message text;		// Data in the window.
	int button_id;			// Button for opening the note
} note;

static note *note_list = 0;
static int note_list_size = 0;
static const char* default_file_name = "notes.xml";
static const char* character_file_name = "notes_%s.xml";
static char notes_file_name[128] = { 0 };
static int using_named_notes = 0;
static size_t cm_save_id = CM_INIT_VALUE;

// Widgets and Windows
int notepad_win = -1;
static int note_tabcollection_id = -1;
static int main_note_tab_id = -1;
static int new_note_button_id = -1;
static int save_notes_button_id = -1;
static int note_button_scroll_id = -1;

int notepad_loaded = 0;
static const float note_tab_zoom = DEFAULT_SMALL_RATIO * 0.9;
static float note_button_zoom = 0;
static int note_widget_id = 0;
static INPUT_POPUP popup_str;

// Misc.
static unsigned short nr_notes = 0;
static int widget_space = 0;

// Note selection button parameters
static const int note_button_max_rows = 10;
static int note_button_y_offset = 0;
static int note_button_width = 0;
static int note_button_height = 0;
static int note_button_space = 0;

// Help message
static const char* note_message;
static const char* timed_note_message;
static Uint32 note_message_timer = 0;

void note_button_set_pos (int id)
{
	int scroll_pos = vscrollbar_get_pos (main_note_tab_id, note_button_scroll_id);
	int row = id / 2 - scroll_pos;

	if (row < 0 || row >= note_button_max_rows)
	{
		widget_set_flags (main_note_tab_id, note_list[id].button_id, WIDGET_INVISIBLE);
	}
	else
	{
		int x = widget_space + (id % 2) * (note_button_width + widget_space);
		int y = note_button_y_offset + (note_button_height + note_button_space) * row;

		widget_unset_flags (main_note_tab_id, note_list[id].button_id, WIDGET_INVISIBLE);
		widget_move (main_note_tab_id, note_list[id].button_id, x, y);
	}
}

static int note_button_scroll_handler()
{
	int i;

	for (i = 0; i < nr_notes; i++)
		note_button_set_pos (i);

	return 1;
}

static void scroll_to_note_button(int nr)
{
	int pos = vscrollbar_get_pos (main_note_tab_id, note_button_scroll_id);
	int row = nr / 2;

	if (row <= pos)
	{
		vscrollbar_set_pos (main_note_tab_id, note_button_scroll_id, row);
		note_button_scroll_handler ();
	}
	else if (row >= pos + note_button_max_rows)
	{
		vscrollbar_set_pos (main_note_tab_id, note_button_scroll_id, row - note_button_max_rows + 1);
		note_button_scroll_handler ();
	}
}

static void update_note_button_scrollbar(int nr)
{
	int nr_rows = (nr_notes+1) / 2;

	if (nr_rows <= note_button_max_rows)
	{
		vscrollbar_set_bar_len (main_note_tab_id, note_button_scroll_id, 0);
		scroll_to_note_button (0);
	}
	else
	{
		vscrollbar_set_bar_len (main_note_tab_id, note_button_scroll_id, nr_rows - note_button_max_rows);
		scroll_to_note_button (nr);
	}
}

static void init_note (int id, const char* name, const char* content)
{
	int nsize = MIN_NOTE_SIZE;

	if (content)
	{
		int len = strlen (content);
		while (nsize <= len)
			nsize += nsize;
	}

	init_text_message (&(note_list[id].text), nsize);
	set_text_message_data (&(note_list[id].text), content);
	set_text_message_color (&(note_list[id].text), 0.77f, 0.57f, 0.39f);

	note_list[id].button_id = -1;
	note_list[id].window = -1;
	safe_strncpy (note_list[id].name, name, sizeof (note_list[id].name));
}

static void increase_note_storage(void)
{
	int new_size = note_list_size * 2;
	note_list = realloc (note_list, new_size * sizeof (note));
	memset(&note_list[note_list_size], 0, (new_size - note_list_size) * sizeof (note) );
	note_list_size = new_size;
}

static int notepad_load_file()
{
	xmlDocPtr doc;
	xmlNodePtr cur;

	if (note_list == 0)
	{
		note_list = calloc (NOTE_LIST_INIT_SIZE, sizeof (note));
		note_list_size = NOTE_LIST_INIT_SIZE;
	}

	notepad_loaded = 1;

	safe_snprintf(notes_file_name, sizeof(notes_file_name), character_file_name, get_lowercase_username());
	if (el_file_exists_config(notes_file_name))
		using_named_notes = 1;
	else
	{
		using_named_notes = 0;
		safe_snprintf(notes_file_name, sizeof(notes_file_name), default_file_name);
	}

	doc = xmlParseFile (notes_file_name);
	if (doc == NULL)
	{
		LOG_ERROR (cant_parse_notes);
		return 0;
	}

	cur = xmlDocGetRootElement (doc);
	if (cur == NULL)
	{
		// Not an error, just an empty notepad
		//LOG_ERROR ("Empty xml notepad. It will be overwritten.");
		xmlFreeDoc(doc);
		return 0;
	}

	if (xmlStrcasecmp (cur->name, (const xmlChar *) "PAD"))
	{
		LOG_ERROR (notes_wrong);
		xmlFreeDoc(doc);
		return 0;
	}

	// Load child node
	cur = cur->xmlChildrenNode;
	// Loop while we have a node, copying ATTRIBS, etc
	while (cur != NULL)
	{
		if ((!xmlStrcasecmp (cur->name, (const xmlChar *)"NOTE")))
		{
			xmlChar* xmlName = xmlGetProp (cur, BAD_CAST "NAME");
			char* name = fromUTF8 (xmlName, strlen ((const char*) xmlName));
			char* data = NULL;
			if (cur->children)
				 data = fromUTF8 (cur->children->content, strlen ((const char*)cur->children->content));

			if (nr_notes >= note_list_size)
				increase_note_storage();

			init_note (nr_notes, name, data);

			if (data) free (data);
			free (name);
			xmlFree (xmlName);

			nr_notes++;
		}
		else if(cur->type == XML_ELEMENT_NODE)
		{
			LOG_ERROR ("%s: [%s]", wrong_note_node, cur->name);
		}
		cur = cur->next;         // Advance to the next node.
	}
	return 1;
}

static int click_save_handler(widget_list *w, int UNUSED(mx), int UNUSED(my),
	Uint32 flags)
{
	// only handle mouse button clicks, not scroll wheels moves
	// Update: don't check when saving on exit (w == NULL)
	if ( (flags & ELW_MOUSE_BUTTON) == 0 && w != NULL) return 0;

	if (notepad_save_file())
	{
		if (using_named_notes)
			timed_note_message = character_notes_saved_str;
		else
			timed_note_message = note_saved;
	}
	else
		timed_note_message = note_save_failed;
	note_message_timer  = SDL_GetTicks();

	return 1;
}

static int mouseover_save_handler(widget_list *widget, int mx, int my)
{
	note_message = notes_save_tooltip_str;
	return 1;
}

static int cm_set_file_name_handler(window_info *win, int widget_id, int mx, int my, int option)
{
	if (option == 0)
	{
		cm_grey_line(cm_save_id, 0, 1);
		safe_snprintf(notes_file_name, sizeof(notes_file_name), character_file_name, get_lowercase_username());
		timed_note_message = using_character_notes_str;
		note_message_timer  = SDL_GetTicks();
		using_named_notes = 1;
		cm_grey_line(cm_save_id, 0, 1);
		return 1;
	}
	else
		return 0;
}

int notepad_save_file()
{
	int i;
	char file[256];
	xmlDocPtr doc = NULL;                      // document pointer
	xmlNodePtr root_node = NULL, node = NULL;  // node pointers

	safe_snprintf (file, sizeof (file), "%s%s", configdir, notes_file_name);

	doc = xmlNewDoc (BAD_CAST "1.0");
	root_node = xmlNewNode (NULL, BAD_CAST "PAD");
	xmlDocSetRootElement (doc, root_node);
	for (i = 0; i < nr_notes; i++)
	{
		xmlChar* data;
		char* subst_string = NULL;

		// libxml2 expects all data in UTF-8 encoding.
		xmlChar* name = toUTF8 (note_list[i].name, strlen (note_list[i].name));
		substitute_char_with_string (note_list[i].text.data, &subst_string, '&', "&amp;");
		data = toUTF8 (subst_string, strlen(subst_string));

		node = xmlNewChild (root_node, NULL, BAD_CAST "NOTE", data);
		xmlNewProp (node, BAD_CAST "NAME", name);

		free (subst_string);
		free (data);
		free (name);
	}

	if (xmlSaveFormatFileEnc (file, doc, "UTF-8", 1) < 0)
	{
#ifndef WINDOWS
		// error writing. try the data directory
		safe_snprintf (file, sizeof (file), "%s/%s", datadir, notes_file_name);
		if (xmlSaveFormatFileEnc(file, doc, "UTF-8", 1) < 0)
		{
			LOG_ERROR(cant_save_notes, file);
		}
#else
		LOG_ERROR(cant_save_notes, file);
#endif
		return 0;
	}

	// Success!
	return 1;
}


static int notepad_remove_category(widget_list* UNUSED(w),
	int UNUSED(mx), int UNUSED(my), Uint32 flags)
{
	static Uint32 last_click = 0;
	int i, id = -1, cur_tab, t;

	// only handle mouse button clicks, not scroll wheels moves
	if ( (flags & ELW_MOUSE_BUTTON) == 0) return 0;

	if (!safe_button_click(&last_click))
		return 1;

	t = tab_collection_get_tab_id (notepad_win, note_tabcollection_id);
	cur_tab = tab_collection_get_tab (notepad_win, note_tabcollection_id);

	for (i = 0; i < nr_notes; i++)
	{
		if (t == note_list[i].window)
		{
			id = i;
			break;
		}
	}
	if (id >= nr_notes || id == -1)
	{
		return 0;
	}

	tab_collection_close_tab (notepad_win, note_tabcollection_id, cur_tab);
	widget_destroy (main_note_tab_id, note_list[id].button_id);
	free_text_message_data (&(note_list[id].text));

	// shift all notes after the deleted note one up
	if (id < nr_notes-1)
	{
		memmove (&(note_list[id]), &(note_list[id+1]), (nr_notes-id-1) * sizeof (note));
		for ( ; id < nr_notes-1; id++)
			note_button_set_pos (id);
	}
	nr_notes--;

	update_note_button_scrollbar(0);

	return 1;
}

static int note_tab_destroy(window_info *w)
{
	int i;

	for(i = 0; i < nr_notes; i++)
	{
		if (note_list[i].window == w->window_id)
		{
			note_list[i].window = -1;
			return 1;
		}
	}

	return 0;
}

static int mouseover_remove_handler()
{
	if (!disable_double_click && show_help_text)
		note_message = dc_note_remove;
	return 1;
}

static void open_note_tab_continued(int id)
{
	widget_list *remove_but = NULL;
	window_info *tab_win = NULL;
	int tf_x = 0;
	int tf_y = 0;
	int tf_width = 0;
	int tf_height = 0;
	int tab;

	note_list[id].window = tab_add (notepad_win, note_tabcollection_id, note_list[id].name, 0, 1, ELW_USE_UISCALE);
	set_window_custom_scale(note_list[id].window, MW_INFO);
	if (note_list[id].window < 0 || note_list[id].window > windows_list.num_windows)
		return;
	tab_win = &windows_list.window[note_list[id].window];
	widget_set_color (notepad_win, note_list[id].window, 0.77f, 0.57f, 0.39f);
	set_window_handler (note_list[id].window, ELW_HANDLER_DESTROY, note_tab_destroy);

	// remove button
	note_list[id].button = button_add (note_list[id].window, NULL, button_remove_category, widget_space, widget_space);
	remove_but = widget_find(note_list[id].window, note_list[id].button);
	button_resize(note_list[id].window, note_list[id].button, 0, 0, tab_win->current_scale);
	widget_set_OnClick(note_list[id].window, note_list[id].button, notepad_remove_category);
	widget_set_OnMouseover(note_list[id].window, note_list[id].button, mouseover_remove_handler);

	// input text field
	tf_x = widget_space;
	tf_y = widget_space * 2 + remove_but->len_y;
	tf_width = tab_win->len_x - 2 * widget_space;
	tf_height = tab_win->len_y - widget_space * 3 - remove_but->len_y;
	note_list[id].input = text_field_add_extended(note_list[id].window, note_widget_id++,
		NULL, tf_x, tf_y, tf_width, tf_height,
		TEXT_FIELD_BORDER|TEXT_FIELD_EDITABLE|TEXT_FIELD_CAN_GROW|TEXT_FIELD_SCROLLBAR,
		NOTE_FONT, tab_win->current_scale, &note_list[id].text,
		1, FILTER_ALL, widget_space, widget_space);

	tab = tab_collection_get_tab_nr (notepad_win, note_tabcollection_id, note_list[id].window);
	tab_collection_select_tab (notepad_win, note_tabcollection_id, tab);
}

static int open_note_tab(widget_list* UNUSED(w),
	int UNUSED(mx), int UNUSED(my), Uint32 flags)
{
	int i = 0;

	// only handle mouse button clicks, not scroll wheels moves
	if ( (flags & ELW_MOUSE_BUTTON) == 0) return -1;

	for(i = 0; i < nr_notes; i++)
	{
		if (w->id == note_list[i].button_id)
		{
			if (note_list[i].window < 0)
			{
				open_note_tab_continued(i);
			}
			else
			{
				int tab = tab_collection_get_tab_nr(notepad_win, note_tabcollection_id, note_list[i].window);
				tab_collection_select_tab(notepad_win, note_tabcollection_id, tab);
			}
			return 1;
		}
	}

	// Button not found, shouldn't get here
	return 0;
}

static void note_button_add(int nr, int next_id)
{
	note_list[nr].button_id = button_add_extended (main_note_tab_id, next_id, NULL, 0, 0, note_button_width, note_button_height, 0, note_button_zoom, note_list[nr].name);
	widget_set_OnClick (main_note_tab_id, note_list[nr].button_id, open_note_tab);
	note_button_set_pos (nr);
	update_note_button_scrollbar (nr);
}

static void notepad_add_continued(const char *name, void* UNUSED(data))
{
	int i = nr_notes++;
	int potential_id, next_id = 0;

	if (i >= note_list_size)
		increase_note_storage();

	for (potential_id = 0; potential_id < nr_notes; potential_id++)
	{
		int test_id;
		int found_id = 0;
		for (test_id=0; test_id<nr_notes; test_id++)
		{
			widget_list *w = widget_find(main_note_tab_id,
				note_list[test_id].button_id);
			if (w && w->id == potential_id)
			{
				found_id = 1;
				break;
			}
		}
		if (!found_id)
		{
			next_id = potential_id;
			break;
		}
	}

	init_note (i, name, NULL);
	note_button_add (i, next_id);

	open_note_tab_continued (i);
}


static int notepad_add_category(widget_list* UNUSED(w),
	int UNUSED(mx), int UNUSED(my), Uint32 flags)
{
	// only handle mouse button clicks, not scroll wheels moves
	if ( (flags & ELW_MOUSE_BUTTON) == 0) return 0;

	if (nr_notes >= note_list_size)
		increase_note_storage();

	display_popup_win (&popup_str, label_note_name);
	centre_popup_window (&popup_str);
	return 1;
}

static int display_notepad_handler(window_info *win)
{
	if (timed_note_message && *timed_note_message)
	{
		if ((tab_collection_get_tab(notepad_win, note_tabcollection_id) != 0) || (SDL_GetTicks() - note_message_timer > 5000))
		{
			timed_note_message = NULL;
			note_message_timer = 0;
		}
		else
			show_help(timed_note_message, 0, win->len_y+10, win->current_scale);
	}
	else if (note_message && *note_message)
	{
		show_help(note_message, 0, win->len_y+10, win->current_scale);
		note_message = NULL;
	}

	return 1;
}

static int click_buttonwin_handler(window_info* UNUSED(win),
	int UNUSED(mx), int UNUSED(my), Uint32 flags)
{
	widget_list *w = widget_find(main_note_tab_id, note_button_scroll_id);
	if ((w == NULL) || (w->Flags & WIDGET_INVISIBLE))
		return 0;
	if (flags&ELW_WHEEL_UP)
	{
		vscrollbar_scroll_up(main_note_tab_id, note_button_scroll_id);
		note_button_scroll_handler();
	}
	else if(flags&ELW_WHEEL_DOWN)
	{
		vscrollbar_scroll_down(main_note_tab_id, note_button_scroll_id);
		note_button_scroll_handler();
	}
	return 1;
}

static int resize_buttonwin_handler(window_info *win, int new_width, int new_height)
{
	widget_list *scroll_w = widget_find(win->window_id, note_button_scroll_id);
	widget_list *wnew = widget_find(main_note_tab_id, new_note_button_id);
	widget_list *wsave = widget_find(main_note_tab_id, save_notes_button_id);
	int tab_tag_height = tab_collection_calc_tab_height(win->font_category,
		win->current_scale * note_tab_zoom);
	int nr;
	int but_space;

	button_resize(main_note_tab_id, new_note_button_id, 0, 0, win->current_scale);
	button_resize(main_note_tab_id, save_notes_button_id, 0, 0, win->current_scale);

	but_space = (win->len_x - win->box_size - widget_space * 4) / 2;
	if ((scroll_w == NULL) || (scroll_w->Flags & WIDGET_INVISIBLE))
		but_space += (win->box_size + widget_space) / 2;

	widget_move(main_note_tab_id, new_note_button_id,
		widget_space + (but_space - wnew->len_x)/2, 2*widget_space);
	widget_move(main_note_tab_id, save_notes_button_id,
		2 * widget_space + but_space + (but_space - wsave->len_x)/2, 2*widget_space);

	note_button_zoom = win->current_scale * 0.8;
	note_button_width = but_space;
	note_button_y_offset = (int)(0.5 + 3 * widget_space + wnew->len_y);

	for(nr = 0; nr < nr_notes; nr++)
		button_resize(main_note_tab_id, note_list[nr].button_id, note_button_width, 0, note_button_zoom);
	if (nr_notes > 0)
	{
		widget_list *tbut = widget_find(main_note_tab_id, note_list[0].button_id);
		note_button_height = tbut->len_y;
	}
	else
		note_button_height = (int)(0.5 + win->current_scale * 22);

	note_button_space = (int)((float)(win->len_y - note_button_y_offset - tab_tag_height) / (float)note_button_max_rows) - note_button_height;

	widget_resize(win->window_id, note_button_scroll_id, win->box_size, note_button_max_rows * (note_button_height + note_button_space) - widget_space);
	widget_move(win->window_id, note_button_scroll_id, win->len_x - win->box_size - widget_space, note_button_y_offset);

	for(nr = 0; nr < nr_notes; nr++)
		note_button_set_pos(nr);

	return 0;
}

static void notepad_win_close_tabs(void)
{
	widget_list *wid = widget_find (notepad_win, note_tabcollection_id);
	tab_collection *col = NULL;
	int closed_a_tab = -1;

	if ((wid == NULL) || ((col = (tab_collection *) wid->widget_info) == NULL))
		return;

	do
	{
		int i;
		closed_a_tab = -1;
		for(i = 0; i < col->nr_tabs; i++)
		{
			if (col->tabs[i].content_id != main_note_tab_id)
			{
				closed_a_tab = tab_collection_close_tab(notepad_win, note_tabcollection_id, i);
				break;
			}
		}
	}
	while (closed_a_tab >= 0);
}

static int resize_notepad_handler(window_info *win, int new_width, int new_height)
{
	widget_list *w = widget_find (win->window_id, note_tabcollection_id);
	int tab_tag_height = 0;

	notepad_win_close_tabs();

	widget_space = (int)(0.5 + win->current_scale * 5);
	widget_set_size(win->window_id, note_tabcollection_id, win->current_scale * note_tab_zoom);
	tab_tag_height = tab_collection_calc_tab_height(win->font_category,
		win->current_scale * note_tab_zoom);

	widget_resize(win->window_id, note_tabcollection_id, new_width, new_height - widget_space);
	widget_move(win->window_id, note_tabcollection_id, 0, widget_space);

	tab_collection_resize(w, new_width, new_height - widget_space);
	tab_collection_move(w, win->pos_x, win->pos_y + tab_tag_height);

	close_ipu(&popup_str);
	init_ipu (&popup_str, main_note_tab_id, NOTE_NAME_LEN+1, 1, NOTE_NAME_LEN+2, NULL, notepad_add_continued);

	update_note_button_scrollbar (0);

	return 0;
}

static int ui_scale_notepad_handler(window_info *win)
{
	int new_width = calc_button_width((const unsigned char*)button_new_category, win->font_category,
		win->current_scale);
	int save_width = calc_button_width((const unsigned char*)button_save_notes, win->font_category,
		win->current_scale);
	int min_width = 2*max2i(new_width, save_width) + 4 * widget_space + ELW_BOX_SIZE;
	win->min_len_x = min_width;
	return 1;
}

static int change_notepad_font_handler(window_info *win, font_cat cat)
{
	if (cat != win->font_category)
		return 0;
	ui_scale_notepad_handler(win);
	return 1;
}

void fill_notepad_window(int window_id)
{
	int i;
	notepad_win = window_id;
	set_window_custom_scale(window_id, MW_INFO);
	set_window_handler(window_id, ELW_HANDLER_DISPLAY, &display_notepad_handler);
	set_window_handler(window_id, ELW_HANDLER_CLICK, &click_buttonwin_handler);
	set_window_handler(window_id, ELW_HANDLER_RESIZE, &resize_notepad_handler );
	set_window_handler(window_id, ELW_HANDLER_UI_SCALE, &ui_scale_notepad_handler);
	set_window_handler(window_id, ELW_HANDLER_FONT_CHANGE, &change_notepad_font_handler);
	if (window_id >= 0 && window_id < windows_list.num_windows)
		ui_scale_notepad_handler(&windows_list.window[window_id]);

	note_tabcollection_id = tab_collection_add (window_id, NULL, 0, 0, 0, 0);
	main_note_tab_id = tab_add (window_id, note_tabcollection_id, tab_main, 0, 0, ELW_USE_UISCALE);
	set_window_custom_scale(main_note_tab_id, MW_INFO);
	widget_set_color (window_id, main_note_tab_id, 0.77f, 0.57f, 0.39f);
	set_window_handler(main_note_tab_id, ELW_HANDLER_CLICK, &click_buttonwin_handler);
	set_window_handler(main_note_tab_id, ELW_HANDLER_RESIZE, &resize_buttonwin_handler);

	// Add Category
	new_note_button_id = button_add(main_note_tab_id, NULL, button_new_category, 0, 0);
	widget_set_OnClick(main_note_tab_id, new_note_button_id, notepad_add_category);

	// Save Notes
	save_notes_button_id = button_add(main_note_tab_id, NULL, button_save_notes, 0, 0);
	widget_set_OnClick(main_note_tab_id, save_notes_button_id, click_save_handler);

	cm_save_id = cm_create(cm_use_character_notepad_str, cm_set_file_name_handler);
	cm_add_widget(cm_save_id, main_note_tab_id, save_notes_button_id);
	widget_set_OnMouseover(main_note_tab_id, save_notes_button_id, mouseover_save_handler);

	note_button_scroll_id = vscrollbar_add (main_note_tab_id, NULL, 0, 0, 0, 0);
	widget_set_OnClick (main_note_tab_id, note_button_scroll_id, note_button_scroll_handler);
	widget_set_OnDrag (main_note_tab_id, note_button_scroll_id, note_button_scroll_handler);

	notepad_load_file ();
	// Add the note selection buttons
	for(i = 0; i < nr_notes; i++)
		note_button_add (i, i);

	if (using_named_notes)
		cm_grey_line(cm_save_id, 0, 1);
}
