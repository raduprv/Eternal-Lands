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
#include "notepad.h"
#include "text.h"
#include "translate.h"
#include "widgets.h"

#ifdef NOTEPAD

void notepad_add_continued (const char *name);

int notepad_loaded = 0;
float note_zoom = 0.8f;

/******************************************
 *             Popup Section              *
 ******************************************/
 
// Macro Definitions
#define MAX_POPUP_BUFFER 156
 
// Widgets and Windows
int popup_win = -1;
int popup_field = -1;
int popup_label = -1;
int popup_ok = -1;
int popup_no = -1;
text_message popup_text;

// Coordinates
int popup_x_len = 200;
int popup_y_len = 100;

// widget id
int note_widget_id = 0;

void clear_popup_window ()
{
	text_field_clear (popup_win, popup_field);

	hide_window (popup_win);
}

void accept_popup_window ()
{
	int istart, iend, len = popup_text.len;
	char *data = popup_text.data;

	// skip leading spaces
	istart = 0;
	while ( istart < len && isspace (data[istart]) )
		istart++;
	if (istart >= len)
		// empty string
		return;
		
	// stop at first non-printable character
	iend = istart;
	while ( iend < len && isprint (data[iend]) )
		iend++;
	if (iend == istart)
		// empty string
		return;
	
	data[iend] = '\0';
	notepad_add_continued (&data[istart]);
	clear_popup_window ();
}

int popup_cancel_button_handler (widget_list *w, int mx, int my, Uint32 flags)
{
	// only handle mouse button clicks, not scroll wheels moves
	if ( (flags & ELW_MOUSE_BUTTON) == 0) return 0;

	clear_popup_window ();

	return 1;
}

int popup_ok_button_handler (widget_list *w, int mx, int my, Uint32 flags)
{
	// only handle mouse button clicks, not scroll wheels moves
	if ( (flags & ELW_MOUSE_BUTTON) == 0) return 0;

	accept_popup_window ();
	
	return 1;
}

int popup_keypress_handler (window_info *win, int mx, int my, Uint32 key, Uint32 unikey)
{
	if (key == SDLK_RETURN)
	{
		accept_popup_window ();
		return 1;
	}
	else if (key == SDLK_ESCAPE)
	{
		clear_popup_window ();
		return 1;
	}
	else
	{
		// send other key presses to the text field
		widget_list *tfw = widget_find (win->window_id, popup_field);
		if (tfw != NULL)
		{
			// FIXME? This is a bit hackish, we don't allow the
			// widget to process keypresses, so that we end up
			// in this handler. But now we need the default
			// widget handler to take care of this keypress, so
			// we clear the flag, let the widget handle it, then
			// set the flag again.
			int res;
			tfw->Flags ^= TEXT_FIELD_NO_KEYPRESS;
			res = widget_handle_keypress (tfw, mx - tfw->pos_x, my - tfw->pos_y, key, unikey);
			tfw->Flags |= TEXT_FIELD_NO_KEYPRESS;
			return res;
		}
	}

	// shouldn't get here
	return 0;
}

void display_popup_win (int parent, int x, int y, char* label, int maxlen)
{
	widget_list *wok;
	widget_list *wno;

	popup_text.len = 0;
	popup_text.size = maxlen;
	popup_text.chan_idx = CHAT_NONE;

	if(popup_win < 0)
	{		  
		popup_win = create_window (win_prompt, parent, 0, x, y, popup_x_len, popup_y_len, ELW_WIN_DEFAULT);

		// clear the buffer
		popup_text.data = calloc ( popup_text.size, sizeof (char) );

		// Label
		popup_label = label_add (popup_win, NULL, label, 5, 5);
		widget_set_color (popup_win, popup_label, 0.77f, 0.57f, 0.39f);
		
		// Input
		popup_field = text_field_add_extended (popup_win, note_widget_id++, NULL, 5, 28, popup_x_len - 20, 28, TEXT_FIELD_BORDER|TEXT_FIELD_EDITABLE|TEXT_FIELD_NO_KEYPRESS, 1.0f, 0.77f, 0.57f, 0.39f, &popup_text, 1, FILTER_ALL, 5, 5, 0.77f, 0.57f, 0.39f);
		widget_set_color (popup_win, popup_field, 0.77f, 0.57f, 0.39f);

		// Accept
		popup_ok = button_add (popup_win, NULL, button_okay, 0, 0);
		widget_set_OnClick (popup_win, popup_ok, popup_ok_button_handler);
		widget_set_color (popup_win, popup_ok, 0.77f, 0.57f, 0.39f);

		// Reject
		popup_no = button_add (popup_win, NULL, button_cancel, 0, 0);
		widget_set_OnClick (popup_win, popup_no, popup_cancel_button_handler);
		widget_set_color (popup_win, popup_no, 0.77f, 0.57f, 0.39f);
		
		// align the buttons
		wok = widget_find(popup_win, popup_ok);
		wno = widget_find(popup_win, popup_no);
		widget_move(popup_win, popup_ok, (popup_x_len - wok->len_x - wno->len_x)/3, popup_y_len - (wok->len_y + 5));
		widget_move(popup_win, popup_no, wok->len_x + 2*(popup_x_len - wok->len_x - wno->len_x)/3, popup_y_len - (wno->len_y + 5));

		set_window_handler (popup_win, ELW_HANDLER_KEYPRESS, popup_keypress_handler);
	}
	else
	{
		label_set_text (popup_win, popup_label, label);
		show_window (popup_win);
		select_window (popup_win);
	}
}


/******************************************
 *             Notepad Section            *
 ******************************************/  
 
//Macro Definitions                             
#define NOTE_LIST_INIT_SIZE 5
#define NOTE_NAME_LEN       16

#define MIN_NOTE_SIZE	128

// Private to this module
typedef struct
{
	char name[NOTE_NAME_LEN];	// Name to display on tab title.
	int window;			// Track which window it owns.
	int input;			// Track it's text buffer
	int button;			// Track it's close button
	text_message text;		// Data in the window.
	int button_id;                  // Button for opening the note
} note;

note *note_list = 0;
int note_list_size = 0;

// Widgets and Windows
int notepad_win = -1;
int note_tabcollection_id = -1;
int main_note_tab_id = -1;
int buttons[2];
int note_button_scroll_id = -1;

// Misc.
unsigned short nr_notes = 0;

// Coordinates
int notepad_win_x = 30; // near left corner by default
int notepad_win_y = 10;
int notepad_win_x_len = 380;
int notepad_win_y_len = 420;

// note button scrollbar parameters
int note_button_scroll_width = 20;
int note_button_scroll_height;

// Note selection button parameters
int note_button_x_space = 5;
int note_button_y_space = 5;
int note_button_width;
int note_button_height = 22;

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

int note_button_scroll_handler ()
{
	int i;

	for (i = 0; i < nr_notes; i++)
		note_button_set_pos (i);

	return 1;
}

void scroll_to_note_button (int nr)
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

void update_note_button_scrollbar (int nr)
{
	int max_nr_rows = note_button_scroll_height / (note_button_height + note_button_y_space);
	int nr_rows = (nr_notes+1) / 2;

	if (nr_rows <= max_nr_rows)
	{
		widget_set_flags (main_note_tab_id, note_button_scroll_id, WIDGET_INVISIBLE);
		vscrollbar_set_bar_len (main_note_tab_id, note_button_scroll_id, 0);
	}
	else
	{
		widget_unset_flags (main_note_tab_id, note_button_scroll_id, WIDGET_INVISIBLE);
		vscrollbar_set_bar_len (main_note_tab_id, note_button_scroll_id, nr_rows - max_nr_rows);
		scroll_to_note_button (nr);
	}
}
	
static void init_note(int id, const char* name)
{
	note_list[id].text.chan_idx = CHAT_NONE;
	note_list[id].text.data = NULL;
	note_list[id].text.size = 0;
	note_list[id].text.len = 0;
	note_list[id].text.wrap_width = 0;
	note_list[id].text.wrap_zoom = 0;
	note_list[id].text.wrap_lines = 0;
	note_list[id].text.max_line_width = 0;
	note_list[id].text.deleted = 0;
	note_list[id].button_id = -1;
	note_list[id].window = -1;
	my_strncp (note_list[id].name, name, NOTE_NAME_LEN);
}

int notepad_load_file ()
{
	char file[256];
	xmlDocPtr doc;
	xmlNodePtr cur;
	xmlChar *name;
	
	if (note_list == 0)
	{
		note_list = calloc (NOTE_LIST_INIT_SIZE, sizeof (note));
		note_list_size = NOTE_LIST_INIT_SIZE;
	}

	notepad_loaded = 1;

	safe_snprintf (file, sizeof (file), "%snotes.xml", configdir);
	doc = xmlParseFile (file);
	if (doc == NULL )
	{
#ifndef WINDOWS
		// try the data directory then
		safe_snprintf (file, sizeof (file), "%s/%s", datadir, "notes.xml");
		doc = xmlParseFile (file);
		if (doc == NULL )
		{
			LOG_ERROR (cant_parse_notes);
			return 0;
		}
#else
		LOG_ERROR (cant_parse_notes);
		return 0;
#endif
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
			int nsize = MIN_NOTE_SIZE, len = 0;
		
			if (nr_notes >= note_list_size)
			{
				int new_size = note_list_size * 2;
				note_list = realloc (note_list, new_size * sizeof (note));
				note_list_size = new_size;
			}

			name = xmlGetProp (cur, (xmlChar *)"NAME");
			init_note(nr_notes, (char *)name);
			xmlFree(name);

			if (cur->children == NULL)
				len = 0;
			else
				len = strlen ((char*)cur->children->content);
			while (nsize <= len)
				nsize += nsize;
			note_list[nr_notes].text.data = calloc ( nsize, sizeof (char) );
			note_list[nr_notes].text.size = nsize;
			if (len > 0)
				my_strcp (note_list[nr_notes].text.data, (char*)cur->children->content);
			note_list[nr_notes].text.len = len;
				    
			rewrap_message(&note_list[nr_notes].text, 1.0f, notepad_win_x_len - 70, NULL);
			
			nr_notes++;
		}
		else
		{
			LOG_ERROR (wrong_note_node);
		}
		cur = cur->next;         // Advance to the next node.
	}
	return 1;
}


int notepad_save_file (widget_list *w, int mx, int my, Uint32 flags)
{
	int i;
	char file[256];
	xmlDocPtr doc = NULL;                      // document pointer
	xmlNodePtr root_node = NULL, node = NULL;  // node pointers

	// only handle mouse button clicks, not scroll wheels moves
	// Update: don't check when saving on exit (w == NULL)
	if ( (flags & ELW_MOUSE_BUTTON) == 0 && w != NULL) return 0;
    
	safe_snprintf (file, sizeof (file), "%snotes.xml", configdir);

	doc = xmlNewDoc (BAD_CAST "1.0");
	root_node = xmlNewNode (NULL, BAD_CAST "PAD");
	xmlDocSetRootElement (doc, root_node);
	for (i = 0; i < nr_notes; i++)
	{
		node = xmlNewChild (root_node, NULL, BAD_CAST "NOTE", BAD_CAST note_list[i].text.data);
		xmlNewProp (node, BAD_CAST "NAME", BAD_CAST note_list[i].name);
	}
	if (xmlSaveFormatFileEnc (file, doc, "ISO-8859-1", 1) < 0)
	{
#ifndef WINDOWS
		// error writing. try the data directory
		safe_snprintf (file, sizeof (file), "%s/%s", datadir, "notes.xml");
		if (xmlSaveFormatFileEnc(file, doc, "ISO-8859-1", 1) < 0)
		{
			LOG_ERROR(cant_save_notes, file);
		}
#else
		LOG_ERROR(cant_save_notes, file);
#endif
	}
	
	return 1;
}

int notepad_remove_category (widget_list *w, int mx, int my, Uint32 flags)
{
	int i, id = -1, cur_tab, t;

	// only handle mouse button clicks, not scroll wheels moves
	if ( (flags & ELW_MOUSE_BUTTON) == 0) return 0;

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
	free (note_list[id].text.data);

	// shift all notes after the deleted note one up
	if (id < nr_notes-1)
	{
		memmove (&(note_list[id]), &(note_list[id+1]), (nr_notes-id-1) * sizeof (note));
		for ( ; id < nr_notes-1; id++)
			note_button_set_pos (id);
	}
	nr_notes--;
     
	return 1;
}

int note_tab_destroy (window_info *w)
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

void open_note_tab_continued (int id)
{
	int tf_x = 20;
	int tf_y = 45;
	int tf_width = notepad_win_x_len - 50;
	int tf_height = notepad_win_y_len - 100;
	int tab;

	note_list[id].window = tab_add (notepad_win, note_tabcollection_id, note_list[id].name, 0, 1);
	widget_set_color (notepad_win, note_list[id].window, 0.77f, 0.57f, 0.39f);

	// input text field
	note_list[id].input = text_field_add_extended (note_list[id].window, note_widget_id++, NULL, tf_x, tf_y, tf_width, tf_height, TEXT_FIELD_BORDER|TEXT_FIELD_EDITABLE|TEXT_FIELD_CAN_GROW|TEXT_FIELD_SCROLLBAR, note_zoom, 0.77f, 0.57f, 0.39f, &note_list[id].text, 1, FILTER_ALL, 5, 5, 0.77f, 0.57f, 0.39f);
	
	// remove button
	note_list[id].button = button_add (note_list[id].window, NULL, button_remove_category, 20, 8);
	widget_set_OnClick (note_list[id].window, note_list[id].button, notepad_remove_category);
	widget_set_color (note_list[id].window, note_list[id].button, 0.77f, 0.57f, 0.39f);
    
	set_window_handler (note_list[id].window, ELW_HANDLER_DESTROY, note_tab_destroy);

	tab = tab_collection_get_tab_nr (notepad_win, note_tabcollection_id, note_list[id].window);
	tab_collection_select_tab (notepad_win, note_tabcollection_id, tab);
}
     
int open_note_tab (widget_list *w, int mx, int my, Uint32 flags)
{
	int i = 0;

	// only handle mouse button clicks, not scroll wheels moves
	if ( (flags & ELW_MOUSE_BUTTON) == 0) return 0;

	for(i = 0; i < nr_notes; i++)
	{
		if (w->id == note_list[i].button_id)
		{
			if (note_list[i].window < 0)
			{
				open_note_tab_continued (i);
			}
			else
			{
				int tab = tab_collection_get_tab_nr (notepad_win, note_tabcollection_id, note_list[i].window);
				tab_collection_select_tab (notepad_win, note_tabcollection_id, tab);
			}
			return 1;
		}
	}

	// Button not found, shouldn't get here
	return 0;
}

void note_button_add (int nr)
{
	note_list[nr].button_id = button_add_extended (main_note_tab_id, nr, NULL, 0, 0, note_button_width, note_button_height, 0, 0.8, 0.77f, 0.57f, 0.39f, note_list[nr].name);
	widget_set_OnClick (main_note_tab_id, note_list[nr].button_id, open_note_tab);
	note_button_set_pos (nr);
	update_note_button_scrollbar (nr);
}

void notepad_add_continued (const char *name)
{
	int i = nr_notes++;

	if (i >= note_list_size)
	{
		int new_size = note_list_size * 2;
		note_list = realloc (note_list, new_size * sizeof (note));
		note_list_size = new_size;
	}

	init_note(i, name);
	note_list[i].text.size = MIN_NOTE_SIZE;
	note_list[i].text.data = calloc ( MIN_NOTE_SIZE, sizeof (char) );
	note_button_add (i);
	
	open_note_tab_continued (i);
}


int notepad_add_category (widget_list *w, int mx, int my, Uint32 flags)
{
	int x = (notepad_win_x_len - popup_x_len) / 2;
	int y = (notepad_win_y_len - popup_y_len) / 2;
	
	// only handle mouse button clicks, not scroll wheels moves
	if ( (flags & ELW_MOUSE_BUTTON) == 0) return 0;

	if (nr_notes >= note_list_size)
	{
		int new_size = note_list_size * 2;
		note_list = realloc (note_list, new_size * sizeof (note));
		note_list_size = new_size;
	}
    
	display_popup_win (main_note_tab_id, x, y, label_note_name, 16);
	return 1;
}

void display_notepad()
{
	int i;
	widget_list *wnew;
	widget_list *wsave;
     
	if (notepad_win < 0)
	{
		int note_tabs_width = notepad_win_x_len - 10;
		int note_tabs_height = notepad_win_y_len - 30;
		
		note_button_scroll_height = note_tabs_height - 55 - 20; // -20 for the tab tags
		note_button_width = (note_tabs_width - note_button_scroll_width - note_button_x_space - 15) / 2;

		notepad_win = create_window (win_notepad, windows_on_top ? -1 : game_root_win, 0, notepad_win_x, notepad_win_y, notepad_win_x_len, notepad_win_y_len, ELW_WIN_DEFAULT|ELW_TITLE_NAME);
		
		note_tabcollection_id = tab_collection_add (notepad_win, NULL, 5, 25, note_tabs_width, note_tabs_height, 20);
		widget_set_size (notepad_win, note_tabcollection_id, 0.7);
		widget_set_color (notepad_win, note_tabcollection_id, 0.77f, 0.57f, 0.39f);
		main_note_tab_id = tab_add (notepad_win, note_tabcollection_id, tab_main, 0, 0);
		widget_set_color (notepad_win, main_note_tab_id, 0.77f, 0.57f, 0.39f);

		// Add Category
		buttons[0] = button_add (main_note_tab_id, NULL, button_new_category, 0, 0);
		widget_set_OnClick (main_note_tab_id, buttons[0], notepad_add_category);
		widget_set_color (main_note_tab_id, buttons[0], 0.77f, 0.57f, 0.39f);

		// Save Notes
		buttons[1] = button_add (main_note_tab_id, NULL, button_save_notes, 0, 0);
		widget_set_OnClick(main_note_tab_id, buttons[1], notepad_save_file);
		widget_set_color(main_note_tab_id, buttons[1], 0.77f, 0.57f, 0.39f);
		
		// align the buttons
		wnew = widget_find(main_note_tab_id, buttons[0]);
		wsave = widget_find(main_note_tab_id, buttons[1]);
		widget_move(main_note_tab_id, buttons[0], (notepad_win_x_len - 10 - wnew->len_x - wsave->len_x)/3, 10);
		widget_move(main_note_tab_id, buttons[1], wnew->len_x + 2*(notepad_win_x_len - 10 - wnew->len_x - wsave->len_x)/3, 10);
		
		notepad_load_file ();

		note_button_scroll_id = vscrollbar_add (main_note_tab_id, NULL, note_tabs_width - note_button_scroll_width - 5, 50, note_button_scroll_width, note_button_scroll_height);
		widget_set_OnClick (main_note_tab_id, note_button_scroll_id, note_button_scroll_handler);
		widget_set_OnDrag (main_note_tab_id, note_button_scroll_id, note_button_scroll_handler);

		// Add the note selection buttons and their scroll bar
		for(i = 0; i < nr_notes; i++)
			note_button_add (i);

		update_note_button_scrollbar (0);
	}
	else
	{
		show_window(notepad_win);
		select_window(notepad_win);
	}
}

void notepad_win_update_zoom ()
{
	int i;

	if (notepad_win < 0)
		return;

	for (i = 0; i < nr_notes; i++)
		widget_set_size (notepad_win, note_list[i].input, note_zoom);
}

#endif
