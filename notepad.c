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
#ifdef MAP_EDITOR2
#include "../map_editor2/global.h"
#else
#include "global.h"
#endif

#ifdef NOTEPAD

void notepad_add_continued (const char *name);

int notepad_loaded = 0;

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
		
	// stop at first character that's not a letter, digit, or space
	iend = istart;
	while ( iend < len && (isalnum (data[iend]) || data[iend] == ' ') )
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

int keypress_popup_handler (window_info *win, int mx, int my, Uint32 key, Uint32 unikey)
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
		// disable the default text field key handler, we trap
		// events in the window itself and call the text field 
		// handler explicitly if necessary
		widget_set_OnKey (popup_win, popup_field, NULL);
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

		set_window_handler (popup_win, ELW_HANDLER_KEYPRESS, &keypress_popup_handler);
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
#define MAX_TABS            3

#define MIN_NOTE_SIZE	128

// Private to this module
typedef struct
{
	char name[NOTE_NAME_LEN];	// Name to display on tab title.
	int window;			// Track which window it owns.
	int input;			// Track it's text buffer
	int button;			// Track it's close button
	text_message text;		// Data in the window.
	int note_buttons;
} note;

note *note_list = 0;
int note_list_size = 0;

// Widgets and Windows
int notepad_win = -1;
int note_tabcollection_id = -1;
int main_note_tab_id = -1;
int buttons[2];

// Misc.
unsigned short no_notes = 0;

// Coordinates
int note_win_x = 30; // near left corner by default
int note_win_y = 10;
int note_win_x_len = 380;
int note_win_y_len = 400;

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
	note_list[id].note_buttons = -1;
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
		
			if (no_notes >= note_list_size)
			{
				int new_size = note_list_size * 2;
				note_list = realloc (note_list, new_size * sizeof (note));
				note_list_size = new_size;
			}

			name = xmlGetProp (cur, (xmlChar *)"NAME");
			init_note(no_notes, (char *)name);
			xmlFree(name);

			if (cur->children == NULL)
				len = 0;
			else
				len = strlen ((char*)cur->children->content);
			while (nsize <= len)
				nsize += nsize;
			note_list[no_notes].text.data = calloc ( nsize, sizeof (char) );
			note_list[no_notes].text.size = nsize;
			if (len > 0)
				my_strcp (note_list[no_notes].text.data, (char*)cur->children->content);
			note_list[no_notes].text.len = len;
				    
			rewrap_message(&note_list[no_notes].text, 1.0f, note_win_x_len - 70, NULL);
			
			no_notes++;
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
	for (i = 0; i < no_notes; i++)
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
     
	for (i = 0; i < no_notes; i++)
	{
		if (t == note_list[i].window)
		{
			id = i;
			break;
		}
	}
	if (id >= no_notes || id == -1)
	{
		return 0;
	}
	
	tab_collection_close_tab (notepad_win, note_tabcollection_id, cur_tab);
	widget_destroy (main_note_tab_id, note_list[id].note_buttons);
	free (note_list[id].text.data);

	// shift all notes after the deleted note one up
	if (id < no_notes-1)
	{
		memmove (&(note_list[id]), &(note_list[id+1]), (no_notes-id-1) * sizeof (note));
		for ( ; id < no_notes-1; id++)
			widget_move_rel (main_note_tab_id, note_list[id].note_buttons, 0, -21);
	}
	no_notes--;
     
	return 1;
}


int note_tab_destroy (window_info *w)
{
	int i;

	for(i = 0; i < no_notes; i++)
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
	int tf_width = note_win_x_len - 50;
	int tf_height = note_win_y_len - 80;

	if (tab_collection_get_nr_tabs (notepad_win, note_tabcollection_id) >= MAX_TABS)
	{
		LOG_TO_CONSOLE(c_red2,user_no_more_note_tabs);
		return;
	}
	 
	note_list[id].window = tab_add (notepad_win, note_tabcollection_id, note_list[id].name, 0, 1);
	widget_set_color (notepad_win, note_list[id].window, 0.77f, 0.57f, 0.39f);

	// input text field
	note_list[id].input = text_field_add_extended (note_list[id].window, note_widget_id++, NULL, tf_x, tf_y, tf_width, tf_height, TEXT_FIELD_BORDER|TEXT_FIELD_EDITABLE|TEXT_FIELD_CAN_GROW|TEXT_FIELD_SCROLLBAR, 0.8f, 0.77f, 0.57f, 0.39f, &note_list[id].text, 1, FILTER_ALL, 5, 5, 0.77f, 0.57f, 0.39f);
	
	// remove button
	note_list[id].button = button_add (note_list[id].window, NULL, button_remove_category, 20, 8);
	widget_set_OnClick (note_list[id].window, note_list[id].button, notepad_remove_category);
	widget_set_color (note_list[id].window, note_list[id].button, 0.77f, 0.57f, 0.39f);
    
	set_window_handler (note_list[id].window, ELW_HANDLER_DESTROY, note_tab_destroy);
}
     

int open_note_tab (widget_list *w, int mx, int my, Uint32 flags)
{
	int i = 0, id = -1;

	// only handle mouse button clicks, not scroll wheels moves
	if ( (flags & ELW_MOUSE_BUTTON) == 0) return 0;

	for(i = 0; i < no_notes; i++)
	{
		if (w->id == note_list[i].note_buttons)
		{
			id = i;
		}
	}
	if (id == -1)
	{
		return 0;
	}
	if (note_list[id].window >= 0)
	{
		return 0;
	}
    
	open_note_tab_continued (id);
    
	return 1;
}


void notepad_add_continued (const char *name)
{
	int y_pos;
	
	if (no_notes == 0)
	{
		y_pos = 50;
	}
	else
	{
		widget_list *w = widget_find (main_note_tab_id, note_list[no_notes-1].note_buttons);
		y_pos = w->pos_y + 21;
	}

	if (no_notes >= note_list_size)
	{
		int new_size = note_list_size * 2;
		note_list = realloc (note_list, new_size * sizeof (note));
		note_list_size = new_size;
	}
	no_notes++;
	
	init_note(no_notes-1, name);
	note_list[no_notes-1].note_buttons = label_add (main_note_tab_id, NULL, note_list[no_notes-1].name, 5, y_pos);
	note_list[no_notes-1].text.size = MIN_NOTE_SIZE;
	note_list[no_notes-1].text.data = calloc ( MIN_NOTE_SIZE, sizeof (char) );
    
	widget_set_OnClick (main_note_tab_id, note_list[no_notes-1].note_buttons, open_note_tab);
	widget_set_color(main_note_tab_id, note_list[no_notes-1].note_buttons, 0.77f, 0.57f, 0.39f);
	
	open_note_tab_continued (no_notes-1);
}


int notepad_add_category (widget_list *w, int mx, int my, Uint32 flags)
{
	int x = (note_win_x_len - popup_x_len) / 2;
	int y = (note_win_y_len - popup_y_len) / 2;
	
	// only handle mouse button clicks, not scroll wheels moves
	if ( (flags & ELW_MOUSE_BUTTON) == 0) return 0;

	if (no_notes >= note_list_size)
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
		notepad_win = create_window (win_notepad, game_root_win, 0, note_win_x, note_win_y, note_win_x_len, note_win_y_len, ELW_WIN_DEFAULT|ELW_TITLE_NAME);
		
		note_tabcollection_id = tab_collection_add (notepad_win, NULL, 5, 5, note_win_x_len - 10, note_win_y_len - 10, 20);
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
		widget_move(main_note_tab_id, buttons[0], (note_win_x_len - 10 - wnew->len_x - wsave->len_x)/3, 10);
		widget_move(main_note_tab_id, buttons[1], wnew->len_x + 2*(note_win_x_len - 10 - wnew->len_x - wsave->len_x)/3, 10);
		
		notepad_load_file ();

		for(i = 0; i < no_notes; i++)
		{
			note_list[i].note_buttons = label_add (main_note_tab_id, NULL, note_list[i].name, 5, 50+21*i);
			widget_set_OnClick (main_note_tab_id, note_list[i].note_buttons, open_note_tab);
			widget_set_color(main_note_tab_id, note_list[i].note_buttons, 0.77f, 0.57f, 0.39f);
		}
	}
	else
	{
		show_window(notepad_win);
		select_window(notepad_win);
	}
}

#endif
