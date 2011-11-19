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
#include "elwindows.h"
#include "errors.h"
#include "gamewin.h"
#include "init.h"
#include "hud.h"
#include "notepad.h"
#include "tabs.h"
#include "text.h"
#include "translate.h"


/******************************************
 *             Popup Section              *
 ******************************************/

void init_ipu (INPUT_POPUP *ipu, int parent, int x_len, int y_len, int maxlen, int rows, void cancel(void *), void input(const char *, void *))
{
	ipu->text_flags = TEXT_FIELD_BORDER|TEXT_FIELD_EDITABLE|TEXT_FIELD_NO_KEYPRESS|TEXT_FIELD_MOUSE_EDITABLE;
	ipu->text_flags |= (rows>1) ?TEXT_FIELD_SCROLLBAR :0;

	ipu->popup_win = ipu->popup_field = ipu->popup_label = ipu->popup_ok =
		ipu->popup_no = ipu->x = ipu->y = -1;

	if (x_len == -1)
		ipu->popup_x_len = DEFAULT_FONT_X_LEN * maxlen + 20;
	else
		ipu->popup_x_len = x_len;

	if (y_len == -1)
		ipu->popup_y_len = 100;
	else
		ipu->popup_y_len = y_len;
	ipu->popup_y_len += ((rows>1) ?(rows-1)*28 :0);

	ipu->popup_cancel = cancel;
	ipu->popup_input = input;
	ipu->data = NULL;

	ipu->parent = parent;
	ipu->maxlen = maxlen;
	ipu->rows = rows;
	ipu->accept_do_not_close = ipu->allow_nonprint_chars = 0;
}

void clear_popup_window (INPUT_POPUP *ipu)
{
	text_field_clear (ipu->popup_win, ipu->popup_field);
	hide_window (ipu->popup_win);
}

void close_ipu (INPUT_POPUP *ipu)
{
	if (ipu->popup_win > 0)
	{
		destroy_window(ipu->popup_win);
		clear_text_message_data (&ipu->popup_text);
		free_text_message_data (&ipu->popup_text);
		init_ipu(ipu, -1, 200, 100, 10, 1, NULL, NULL);
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
	int istart, iend, itmp, len = ipu->popup_text.len;
	char *data = ipu->popup_text.data;

	// skip leading spaces
	istart = 0;
	while ( istart < len && isspace (data[istart]) )
		istart++;
	if (istart >= len)
		// empty string
		return;

	// remove soft breaks
	iend = itmp = istart;
	while ( iend < len )
	{
		if (data[iend] != '\r')
			data[itmp++] = data[iend];
		iend++;
	}
	len = itmp;

	// stop at first non-printable character if allow_nonprint_chars not set
	iend = istart;
	while ( iend < len && (ipu->allow_nonprint_chars || is_printable (data[iend]) ))
		iend++;
	if (iend == istart)
		// empty string
		return;

	// send the entered text to the window owner then clear up
	data[iend] = '\0';
	if (ipu->popup_input != NULL)
		(*ipu->popup_input) (&data[istart], ipu->data);
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
	int UNUSED(mx), int UNUSED(my), Uint32 key, Uint32 unikey)
{
	INPUT_POPUP *ipu = ipu_from_window(win);
	if (ipu == NULL) return 0;

	if (key == SDLK_RETURN)
	{
		accept_popup_window (ipu);
		return 1;
	}
	else if (key == SDLK_ESCAPE)
	{
		if (ipu->popup_cancel != NULL)
			(*ipu->popup_cancel) (ipu->data);
		clear_popup_window (ipu);
		return 1;
	}
	else
	{
		// send other key presses to the text field
		widget_list *tfw = widget_find (win->window_id, ipu->popup_field);
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
			res = widget_handle_keypress (tfw, mx - tfw->pos_x, my - tfw->pos_y, key, unikey);
			tfw->Flags |= TEXT_FIELD_NO_KEYPRESS;
			return res;
		}
	}

	// shouldn't get here
	return 0;
}

void display_popup_win (INPUT_POPUP *ipu, const char* label)
{
	widget_list *wok;
	widget_list *wno;

	if(ipu->popup_win < 0)
	{
		Uint32 flags = ELW_WIN_DEFAULT & ~ELW_CLOSE_BOX;
		ipu->popup_win = create_window (win_prompt, ipu->parent, 0, ipu->x, ipu->y, ipu->popup_x_len, ipu->popup_y_len, flags);

		// clear the buffer
		init_text_message (&ipu->popup_text, ipu->maxlen);
		set_text_message_color (&ipu->popup_text, 0.77f, 0.57f, 0.39f);

		// Label
		ipu->popup_label = label_add (ipu->popup_win, NULL, label, 5, 5);
		widget_set_color (ipu->popup_win, ipu->popup_label, 0.77f, 0.57f, 0.39f);

		// Input
		ipu->popup_field = text_field_add_extended (ipu->popup_win, 101, NULL, 5, 28, ipu->popup_x_len - 10, 28*ipu->rows, ipu->text_flags, 1.0f, 0.77f, 0.57f, 0.39f, &ipu->popup_text, 1, FILTER_ALL, 5, 5);
		widget_set_color (ipu->popup_win, ipu->popup_field, 0.77f, 0.57f, 0.39f);

		// Accept
		ipu->popup_ok = button_add (ipu->popup_win, NULL, button_okay, 0, 0);
		widget_set_OnClick (ipu->popup_win, ipu->popup_ok, popup_ok_button_handler);
		widget_set_color (ipu->popup_win, ipu->popup_ok, 0.77f, 0.57f, 0.39f);

		// Reject
		ipu->popup_no = button_add (ipu->popup_win, NULL, button_cancel, 0, 0);
		widget_set_OnClick (ipu->popup_win, ipu->popup_no, popup_cancel_button_handler);
		widget_set_color (ipu->popup_win, ipu->popup_no, 0.77f, 0.57f, 0.39f);

		// align the buttons
		wok = widget_find(ipu->popup_win, ipu->popup_ok);
		wno = widget_find(ipu->popup_win, ipu->popup_no);
		widget_move(ipu->popup_win, ipu->popup_ok, (ipu->popup_x_len - wok->len_x - wno->len_x)/3, ipu->popup_y_len - (wok->len_y + 5));
		widget_move(ipu->popup_win, ipu->popup_no, wok->len_x + 2*(ipu->popup_x_len - wok->len_x - wno->len_x)/3, ipu->popup_y_len - (wno->len_y + 5));

		set_window_handler (ipu->popup_win, ELW_HANDLER_KEYPRESS, popup_keypress_handler);

		if ((ipu->popup_win > -1) && (ipu->popup_win < windows_list.num_windows))
			windows_list.window[ipu->popup_win].data = ipu;
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

// Widgets and Windows
int notepad_win = -1;
static int note_tabcollection_id = -1;
static int main_note_tab_id = -1;
static int new_note_button_id = -1;
static int save_notes_button_id = -1;
static int note_button_scroll_id = -1;

int notepad_loaded = 0;
float note_zoom = 0.8f;
static int note_widget_id = 0;
static INPUT_POPUP popup_str;

// Misc.
static unsigned short nr_notes = 0;

// Coordinates
int notepad_win_x = 30; // near left corner by default
int notepad_win_y = 10;
static int notepad_win_x_len = INFO_TAB_WIDTH;
static int notepad_win_y_len = INFO_TAB_HEIGHT;

// note button scrollbar parameters
static int note_button_scroll_width = 20;
static int note_button_scroll_height;

// Note selection button parameters
static int note_button_x_space = 5;
static int note_button_y_space = 5;
static int note_button_width;
static int note_button_height = 22;

// Help message
static const char* note_message;
static const char* note_static_message;

void note_button_set_pos (int id)
{
	int scroll_pos = vscrollbar_get_pos (main_note_tab_id, note_button_scroll_id);
	int max_nr_rows = note_button_scroll_height / (note_button_height + note_button_y_space);
	int row = id / 2 - scroll_pos;

	if (row < 0 || row >= max_nr_rows)
	{
		widget_set_flags (main_note_tab_id, note_list[id].button_id, WIDGET_INVISIBLE);
	}
	else
	{
		int x = 5 + (id % 2) * (note_button_width + note_button_x_space);
		int y = 50 + (note_button_height + note_button_y_space) * row;

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
	int max_nr_rows = note_button_scroll_height / (note_button_height + note_button_y_space);
	int row = nr / 2;

	if (row < pos)
	{
		vscrollbar_set_pos (main_note_tab_id, note_button_scroll_id, row);
		note_button_scroll_handler ();
	}
	else if (row >= pos + max_nr_rows)
	{
		vscrollbar_set_pos (main_note_tab_id, note_button_scroll_id, row - max_nr_rows + 1);
		note_button_scroll_handler ();
	}
}

static void update_note_button_scrollbar(int nr)
{
	int max_nr_rows = note_button_scroll_height / (note_button_height + note_button_y_space);
	int nr_rows = (nr_notes+1) / 2;

	if (nr_rows <= max_nr_rows)
	{
		widget_set_flags (main_note_tab_id, note_button_scroll_id, WIDGET_INVISIBLE|WIDGET_DISABLED);
		vscrollbar_set_bar_len (main_note_tab_id, note_button_scroll_id, 0);
		scroll_to_note_button (0);
	}
	else
	{
		widget_unset_flags (main_note_tab_id, note_button_scroll_id, WIDGET_INVISIBLE|WIDGET_DISABLED);
		vscrollbar_set_bar_len (main_note_tab_id, note_button_scroll_id, nr_rows - max_nr_rows);
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

	doc = xmlParseFile ("notes.xml");
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
			{
				int new_size = note_list_size * 2;
				note_list = realloc (note_list, new_size * sizeof (note));
				note_list_size = new_size;
			}

			init_note (nr_notes, name, data);

			if (data) free (data);
			free (name);
			xmlFree (xmlName);

			rewrap_message (&note_list[nr_notes].text, 1.0f, notepad_win_x_len - 70, NULL);

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
		note_static_message = note_saved;
	else
		note_static_message = note_save_failed;

	return 1;
}

int notepad_save_file()
{
	int i;
	char file[256];
	xmlDocPtr doc = NULL;                      // document pointer
	xmlNodePtr root_node = NULL, node = NULL;  // node pointers

	safe_snprintf (file, sizeof (file), "%snotes.xml", configdir);

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
		safe_snprintf (file, sizeof (file), "%s/%s", datadir, "notes.xml");
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
	int tf_x = 10;
	int tf_y = 45;
	int tf_width = notepad_win_x_len - 20;
	int tf_height = notepad_win_y_len - 80;
	int tab;

	note_list[id].window = tab_add (notepad_win, note_tabcollection_id, note_list[id].name, 0, 1, 0);
	widget_set_color (notepad_win, note_list[id].window, 0.77f, 0.57f, 0.39f);

	// input text field
	note_list[id].input = text_field_add_extended(note_list[id].window, note_widget_id++, NULL, tf_x, tf_y, tf_width, tf_height, TEXT_FIELD_BORDER|TEXT_FIELD_EDITABLE|TEXT_FIELD_CAN_GROW|TEXT_FIELD_SCROLLBAR, note_zoom, 0.77f, 0.57f, 0.39f, &note_list[id].text, 1, FILTER_ALL, 5, 5);

	// remove button
	note_list[id].button = button_add (note_list[id].window, NULL, button_remove_category, 20, 8);
	widget_set_OnClick(note_list[id].window, note_list[id].button, notepad_remove_category);
	widget_set_color(note_list[id].window, note_list[id].button, 0.77f, 0.57f, 0.39f);
	widget_set_OnMouseover(note_list[id].window, note_list[id].button, mouseover_remove_handler);

	set_window_handler (note_list[id].window, ELW_HANDLER_DESTROY, note_tab_destroy);

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
	note_list[nr].button_id = button_add_extended (main_note_tab_id, next_id, NULL, 0, 0, note_button_width, note_button_height, 0, 0.8, 0.77f, 0.57f, 0.39f, note_list[nr].name);
	widget_set_OnClick (main_note_tab_id, note_list[nr].button_id, open_note_tab);
	note_button_set_pos (nr);
	update_note_button_scrollbar (nr);
}

static void notepad_add_continued(const char *name, void* UNUSED(data))
{
	int i = nr_notes++;
	int potential_id, next_id = 0;

	if (i >= note_list_size)
	{
		int new_size = note_list_size * 2;
		note_list = realloc (note_list, new_size * sizeof (note));
		note_list_size = new_size;
	}

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
	{
		int new_size = note_list_size * 2;
		note_list = realloc (note_list, new_size * sizeof (note));
		note_list_size = new_size;
	}

	display_popup_win (&popup_str, label_note_name);
	return 1;
}

static int display_notepad_handler(window_info *win)
{
	if (note_message && *note_message)
	{
		show_help(note_message, 0, win->len_y+10);
		note_message = note_static_message = NULL;
	}
	else if (note_static_message && *note_static_message)
	{
		if (tab_collection_get_tab(notepad_win, note_tabcollection_id) != 0)
			note_static_message = NULL;
		else
			show_help(note_static_message, 0, win->len_y+10);
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

void fill_notepad_window()
{
	int i, tmp;
	widget_list *wnew;
	widget_list *wsave;

	int note_tabs_width = notepad_win_x_len;
	int note_tabs_height = notepad_win_y_len - 5;

	note_button_scroll_height = note_tabs_height - 55 - 20; // -20 for the tab tags
	note_button_width = (note_tabs_width - note_button_scroll_width - note_button_x_space - 15) / 2;

	set_window_handler(notepad_win, ELW_HANDLER_DISPLAY, &display_notepad_handler);
	set_window_handler(notepad_win, ELW_HANDLER_CLICK, &click_buttonwin_handler);

	note_tabcollection_id = tab_collection_add (notepad_win, NULL, 0, 5, note_tabs_width, note_tabs_height, 20);
	widget_set_size (notepad_win, note_tabcollection_id, 0.7);
	widget_set_color (notepad_win, note_tabcollection_id, 0.77f, 0.57f, 0.39f);
	main_note_tab_id = tab_add (notepad_win, note_tabcollection_id, tab_main, 0, 0, 0);
	widget_set_color (notepad_win, main_note_tab_id, 0.77f, 0.57f, 0.39f);
	set_window_handler(main_note_tab_id, ELW_HANDLER_CLICK, &click_buttonwin_handler);

	// Add Category
	new_note_button_id = button_add(main_note_tab_id, NULL, button_new_category, 0, 0);
	widget_set_OnClick(main_note_tab_id, new_note_button_id, notepad_add_category);
	widget_set_color(main_note_tab_id, new_note_button_id, 0.77f, 0.57f, 0.39f);

	// Save Notes
	save_notes_button_id = button_add(main_note_tab_id, NULL, button_save_notes, 0, 0);
	widget_set_OnClick(main_note_tab_id, save_notes_button_id, click_save_handler);
	widget_set_color(main_note_tab_id, save_notes_button_id, 0.77f, 0.57f, 0.39f);

	// align the buttons
	wnew = widget_find(main_note_tab_id, new_note_button_id);
	wsave = widget_find(main_note_tab_id, save_notes_button_id);
	tmp = (notepad_win_x_len - note_button_scroll_width)/2;
	widget_move(main_note_tab_id, new_note_button_id, (tmp - wnew->len_x)/2, 10);
	widget_move(main_note_tab_id, save_notes_button_id, tmp + (tmp - wsave->len_x)/2, 10);

	notepad_load_file ();

	init_ipu (&popup_str, main_note_tab_id, NOTE_NAME_LEN*DEFAULT_FONT_X_LEN, 100, NOTE_NAME_LEN, 1, NULL, notepad_add_continued);
	popup_str.x = (notepad_win_x_len - popup_str.popup_x_len) / 2;
	popup_str.y = (notepad_win_y_len - popup_str.popup_y_len) / 2;

	note_button_scroll_id = vscrollbar_add (main_note_tab_id, NULL, note_tabs_width - note_button_scroll_width - 5, 50, note_button_scroll_width, note_button_scroll_height);
	widget_set_OnClick (main_note_tab_id, note_button_scroll_id, note_button_scroll_handler);
	widget_set_OnDrag (main_note_tab_id, note_button_scroll_id, note_button_scroll_handler);

	// Add the note selection buttons and their scroll bar
	for(i = 0; i < nr_notes; i++)
		note_button_add (i, i);

	update_note_button_scrollbar (0);
}


void notepad_win_update_zoom()
{
	int i;

	if (notepad_win < 0)
		return;

	for (i = 0; i < nr_notes; i++)
		widget_set_size (notepad_win, note_list[i].input, note_zoom);
}

