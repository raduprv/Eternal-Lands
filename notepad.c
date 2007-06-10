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

void notepadAddContinued (const char *name);

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

int clear_popup_window (widget_list *w, int mx, int my, Uint32 flags)
{
	// only handle mouse button clicks, not scroll wheels moves
	if ( (flags & ELW_MOUSE_BUTTON) == 0) return 0;

	memset (popup_text.data, 0, popup_text.size);
	popup_text.len = 0;

	hide_window(popup_win);
	return 1;
}

int accept_popup_window (widget_list *w, int mx, int my, Uint32 flags)
{
	int istart, iend, len = popup_text.len;
	char *data = popup_text.data;
	
	// only handle mouse button clicks, not scroll wheels moves
	if ( (flags & ELW_MOUSE_BUTTON) == 0) return 0;

	// skip leading spaces
	istart = 0;
	while ( istart <len && isspace (data[istart]) )
		istart++;
	if (istart >= len)
		// empty string
		return 1;
		
	// stop at first character that's not a letter, digit, or space
	iend = istart;
	while ( iend < len && (isalnum (data[iend]) || data[iend] == ' ') )
		iend++;
	if (iend == istart)
		// empty string
		return 1;
	
	data[len] = '\0';
	notepadAddContinued (&data[istart]);
	clear_popup_window (w, mx, my, flags);
	return 1;
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
		popup_field = text_field_add_extended (popup_win, note_widget_id++, NULL, 5, 28, popup_x_len - 20, 28, TEXT_FIELD_BORDER|TEXT_FIELD_EDITABLE, 1.0f, 0.77f, 0.57f, 0.39f, &popup_text, 1, FILTER_ALL, 5, 5, 0.77f, 0.57f, 0.39f);
		widget_set_color (popup_win, popup_field, 0.77f, 0.57f, 0.39f);

		// Accept
		popup_ok = button_add (popup_win, NULL, button_okay, 0, 0);
		widget_set_OnClick (popup_win, popup_ok, accept_popup_window);
		widget_set_color (popup_win, popup_ok, 0.77f, 0.57f, 0.39f);

		// Reject
		popup_no = button_add (popup_win, NULL, button_cancel, 0, 0);
		widget_set_OnClick (popup_win, popup_no, clear_popup_window);
		widget_set_color (popup_win, popup_no, 0.77f, 0.57f, 0.39f);
		
		// align the buttons
		wok = widget_find(popup_win, popup_ok);
		wno = widget_find(popup_win, popup_no);
		widget_move(popup_win, popup_ok, (popup_x_len - wok->len_x - wno->len_x)/3, popup_y_len - (wok->len_y + 5));
		widget_move(popup_win, popup_no, wok->len_x + 2*(popup_x_len - wok->len_x - wno->len_x)/3, popup_y_len - (wno->len_y + 5));
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
#define MAX_NOTES         10
#define NOTE_NAME_LEN     16
#define MAX_TABS          3

#define MIN_NOTE_SIZE	128

// Private to this module
struct Note
{
	char name[NOTE_NAME_LEN];	// Name to display on tab title.
	int window;			// Track which window it owns.
	int scroll;			// Track it's scrollbar
	int input;			// Track it's text buffer
	int button;			// Track it's close button
	text_message text;	// Data in the window.
	int note_buttons;
} *note[MAX_NOTES];

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
	note[id]->text.chan_idx = CHAT_NONE;
	note[id]->text.data = NULL;
	note[id]->text.size = 0;
	note[id]->text.len = 0;
	note[id]->text.wrap_width = 0;
	note[id]->text.wrap_zoom = 0;
	note[id]->text.wrap_lines = 0;
	note[id]->text.max_line_width = 0;
	note[id]->text.deleted = 0;
	note[id]->note_buttons = -1;
	note[id]->window = -1;
	my_strncp (note[id]->name, name, NOTE_NAME_LEN);
}

int notepadLoadFile ()
{
	char file[256];
	xmlDocPtr doc;
	xmlNodePtr cur;
	xmlChar *name;
	
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
		
			if(no_notes >= MAX_NOTES)
			{
				LOG_ERROR (too_many_notes);
				return 2;
			}

			note[no_notes] = malloc ( sizeof (struct Note) );
			name = xmlGetProp (cur, (xmlChar *)"NAME");
			init_note(no_notes, (char *)name);
			xmlFree(name);

			if (cur->children == NULL)
				len = 0;
			else
				len = strlen ((char*)cur->children->content);
			while (nsize <= len)
				nsize += nsize;
			note[no_notes]->text.data = calloc ( nsize, sizeof (char) );
			note[no_notes]->text.size = nsize;
			if (len > 0)
				my_strcp (note[no_notes]->text.data, (char*)cur->children->content);
			note[no_notes]->text.len = len;
				    
			rewrap_message(&note[no_notes]->text, 1.0f, note_win_x_len - 70, NULL);
			
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


int notepadSaveFile (widget_list *w, int mx, int my, Uint32 flags)
{
	int i = 0;
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
	while (i < no_notes)
	{
		node = xmlNewChild (root_node, NULL, BAD_CAST "NOTE", BAD_CAST note[i]->text.data);
		xmlNewProp (node, BAD_CAST "NAME", BAD_CAST note[i]->name);
		i++;
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

int notepadRemoveCategory (widget_list *w, int mx, int my, Uint32 flags)
{
	int i, id = -1, cur_tab, t, inote;

	// only handle mouse button clicks, not scroll wheels moves
	if ( (flags & ELW_MOUSE_BUTTON) == 0) return 0;

	t = tab_collection_get_tab_id (notepad_win, note_tabcollection_id);
	cur_tab = tab_collection_get_tab (notepad_win, note_tabcollection_id);
     
	for (i = 0; i < no_notes; i++)
	{
		if (t == note[i]->window)
		{
			id = i;
			break;
		}
	}
	if (id >= MAX_NOTES || id == -1)
	{
		return 0;
	}
	
	tab_collection_close_tab (notepad_win, note_tabcollection_id, cur_tab);
	widget_destroy (main_note_tab_id, note[id]->note_buttons);
	free (note[id]->text.data);
	free (note[id]);
	// shift all notes one up
	for (inote = id+1; inote < no_notes; inote++)
	{
		widget_move_rel (main_note_tab_id, note[inote]->note_buttons, 0, -21);
		note[inote-1] = note[inote];
	}
	note[no_notes-1] = NULL;
	no_notes--;
     
	return 1;
}


int tabOnDestroy (window_info *w)
{
	int i, id = -1;

	for(i = 0; i < no_notes; i++)
	{
		if (w->window_id == note[i]->window)
		{
			id = i;
			break;
		}
	}
	if(id >= MAX_NOTES || id == -1)
	{
		return 0;
	}
	
	if (note[id]->scroll >= 0) widget_destroy (note[id]->window, note[id]->scroll);
	if (note[id]->input >= 0) widget_destroy (note[id]->window, note[id]->input);
	if (note[id]->button >= 0) widget_destroy (note[id]->window, note[id]->button);
    
	note[id]->window = -1;
    
	return 1;
}

void openNoteTabContinued (int id)
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
	 
	note[id]->window = tab_add (notepad_win, note_tabcollection_id, note[id]->name, 0, 1);
	widget_set_color (notepad_win, note[id]->window, 0.77f, 0.57f, 0.39f);

	// input text field
	note[id]->input = text_field_add_extended (note[id]->window, note_widget_id++, NULL, tf_x, tf_y, tf_width, tf_height, TEXT_FIELD_BORDER|TEXT_FIELD_EDITABLE|TEXT_FIELD_CAN_GROW|TEXT_FIELD_SCROLLBAR, 1.0f, 0.77f, 0.57f, 0.39f, &note[id]->text, 1, FILTER_ALL, 5, 5, 0.77f, 0.57f, 0.39f);
	
	// remove button
	note[id]->button = button_add (note[id]->window, NULL, button_remove_category, 20, 8);
	widget_set_OnClick (note[id]->window, note[id]->button, notepadRemoveCategory);
	widget_set_color (note[id]->window, note[id]->button, 0.77f, 0.57f, 0.39f);
    
	set_window_handler (note[id]->window, ELW_HANDLER_DESTROY, tabOnDestroy);
}
     

int openNoteTab (widget_list *w, int mx, int my, Uint32 flags)
{
	int i = 0, id = -1;

	// only handle mouse button clicks, not scroll wheels moves
	if ( (flags & ELW_MOUSE_BUTTON) == 0) return 0;

	for(i = 0; i < no_notes; i++)
	{
		if (w->id == note[i]->note_buttons)
		{
			id = i;
		}
	}
	if (id == -1)
	{
		return 0;
	}
	if (note[id]->window >= 0)
	{
		return 0;
	}
    
	openNoteTabContinued (id);
    
	return 1;
}


void notepadAddContinued (const char *name)
{
	int y_pos;
	
	if (no_notes == 0)
	{
		y_pos = 50;
	}
	else
	{
		widget_list *w = widget_find (main_note_tab_id, note[no_notes-1]->note_buttons);
		y_pos = w->pos_y + 21;
	}

	if (no_notes >= MAX_NOTES)
	{
		LOG_ERROR (exceed_note_buffer);
		return;
	}
	no_notes++;
	
	note[no_notes-1] = malloc ( sizeof (struct Note) );
	init_note(no_notes-1, name);
	note[no_notes-1]->note_buttons = label_add (main_note_tab_id, NULL, note[no_notes-1]->name, 5, y_pos);
	note[no_notes-1]->text.size = MIN_NOTE_SIZE;
	note[no_notes-1]->text.data = calloc ( MIN_NOTE_SIZE, sizeof (char) );
    
	widget_set_OnClick (main_note_tab_id, note[no_notes-1]->note_buttons, openNoteTab);
	widget_set_color(main_note_tab_id, note[no_notes-1]->note_buttons, 0.77f, 0.57f, 0.39f);
	
	openNoteTabContinued(no_notes-1);
}


int notepadAddCategory (widget_list *w, int mx, int my, Uint32 flags)
{
	int x = (note_win_x_len - popup_x_len) / 2;
	int y = (note_win_y_len - popup_y_len) / 2;
	
	// only handle mouse button clicks, not scroll wheels moves
	if ( (flags & ELW_MOUSE_BUTTON) == 0) return 0;

	if (no_notes >= MAX_NOTES)
	{
		LOG_TO_CONSOLE(c_red2,user_no_more_notes);
		return 1;
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
		widget_set_OnClick (main_note_tab_id, buttons[0], notepadAddCategory);
		widget_set_color (main_note_tab_id, buttons[0], 0.77f, 0.57f, 0.39f);

		// Save Notes
		buttons[1] = button_add (main_note_tab_id, NULL, button_save_notes, 0, 0);
		widget_set_OnClick(main_note_tab_id, buttons[1], notepadSaveFile);
		widget_set_color(main_note_tab_id, buttons[1], 0.77f, 0.57f, 0.39f);
		
		// align the buttons
		wnew = widget_find(main_note_tab_id, buttons[0]);
		wsave = widget_find(main_note_tab_id, buttons[1]);
		widget_move(main_note_tab_id, buttons[0], (note_win_x_len - 10 - wnew->len_x - wsave->len_x)/3, 10);
		widget_move(main_note_tab_id, buttons[1], wnew->len_x + 2*(note_win_x_len - 10 - wnew->len_x - wsave->len_x)/3, 10);
		
		for(i = 0; i < MAX_NOTES; i++)
		{
			note[i] = NULL;
		}
		notepadLoadFile ();

		for(i = 0; i < no_notes; i++)
		{
			note[i]->note_buttons = label_add (main_note_tab_id, NULL, note[i]->name, 5, 50+21*i);
			widget_set_OnClick (main_note_tab_id, note[i]->note_buttons, openNoteTab);
			widget_set_color(main_note_tab_id, note[i]->note_buttons, 0.77f, 0.57f, 0.39f);
		}
	}
	else
	{
		show_window(notepad_win);
		select_window(notepad_win);
	}
}

#endif
