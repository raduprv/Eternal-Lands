/******************************************
 * notepad.c - Player notes in xml format *
 *  includes generic keypress handlers    *
 *  and popup window functionality        *
 ******************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libxml/parser.h>
#include "global.h"

#ifdef NOTEPAD

/******************************************
 *             MISC Functions             *
 ******************************************/
 
//Return 1 so we don't go into the input buffer
int keypress_input_window_handler (window_info *win, int mx, int my, Uint32 key, Uint32 unikey)
{
	return 1;
}

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

// Misc
unsigned char POPUP = 0;
void (*PopupAccept)();

// Coordinates
int popup_x = 60;
int popup_y = 60;
int popup_x_len = 200;
int popup_y_len = 120;

// widget id
int note_widget_id = 0;

int clear_popup_window ()
{
	//int i = 0;
	
	POPUP = 0;
	// Grum
	memset (popup_text.data, 0, MAX_POPUP_BUFFER);
	//while (i < MAX_POPUP_BUFFER)
	//{
	//	popup_text.data[i] = 0;
	//	i++;
	//}
	popup_text.len = 0;
	popup_text.size = 0;

	hide_window(popup_win);

	return 1; 
}

int accept_popup_window ()
{
	PopupAccept (popup_text.data);
	return clear_popup_window ();
}

void display_popup_win (char* label, int maxlen, void (*OnReturn)())
{
	//int i = 0;

	if (POPUP == 1) return;

	popup_text.len = 0;
	popup_text.size = maxlen;
	popup_text.chan_nr = CHANNEL_ALL;
	PopupAccept = OnReturn;

	if(popup_win < 0)
	{		  
		popup_win = create_window ("Prompt", game_root_win, 0, popup_x, popup_y, popup_x_len, popup_y_len, ELW_WIN_DEFAULT);	 
          
		popup_text.data = malloc (MAX_POPUP_BUFFER);
          
	  	// Grum
		memset (popup_text.data, 0, MAX_POPUP_BUFFER);
		//while(i < MAX_POPUP_BUFFER)
		//{
		//	popup_text.data[i] = 0;
		//	i++;
          	//}
          
		popup_label = label_add (popup_win, NULL, label, 5, 15);
		widget_set_color (popup_win, popup_label, 0.77f, 0.57f, 0.39f);
		popup_field = text_field_add_extended (popup_win, note_widget_id++, NULL, 5, 35, popup_x_len - 20, popup_y_len - 55, TEXT_FIELD_BORDER|TEXT_FIELD_EDITABLE, 1.0f, 0.77f, 0.57f, 0.39f, &popup_text, 1, 0, 0, 0, 0.77f, 0.57f, 0.39f);
		widget_set_color (popup_win, popup_field, 0.77f, 0.57f, 0.39f);
      	  
		popup_ok = button_add(popup_win, NULL, "OKAY", 10, 100);
		widget_set_OnClick(popup_win, popup_ok, accept_popup_window);
		widget_set_color(popup_win, popup_ok, 0.77f, 0.57f, 0.39f);
		popup_no = button_add(popup_win, NULL, "Cancel", 60, 100);
		widget_set_OnClick(popup_win, popup_no, clear_popup_window);
		widget_set_color(popup_win, popup_no, 0.77f, 0.57f, 0.39f);
      	  
		set_window_handler(popup_win, ELW_HANDLER_KEYPRESS, keypress_input_window_handler);
	}
	else
	{
		label_set_text(popup_win, popup_label, label);
		show_window(popup_win);
		select_window(popup_win);
	}
	POPUP = 1;
}


/******************************************
 *             Notepad Section            *
 ******************************************/  
 
//Macro Definitions                             
#define MAX_NOTES         10
#define NOTE_NAME_LEN     16
#define NOTE_DATA_LEN     1536
#define MAX_TABS          3

// Private to this module
struct Note
{
	char name[NOTE_NAME_LEN];	// Name to display on tab title.
	int window;			// Track which window it owns.
	int scroll;			// Track it's scrollbar
	int input;			// Track it's text buffer
	int button;			// Track it's close button
} *note[MAX_NOTES];
text_message data[MAX_NOTES];                      // Data in the window.
char notepadbuffer[MAX_NOTES][NOTE_DATA_LEN];      // Data in the window.

// Widgets and Windows
int notepad_win = -1;
int note_tabcollection_id = -1;
int main_note_tab_id = -1;
int buttons[2];
int note_buttons[MAX_NOTES];

// Misc.
unsigned short no_notes = 0;

// Coordinates
int note_win_x = 30; // near left corner by default
int note_win_y = 10;
int note_win_x_len = 380;
int note_win_y_len = 400;

// Ungh. Malloc has failed me... 
// XXX FIXME (Grum): Fix that
void init_notepad_buffers ()
{
	int i = 0;
     
	for(i = 0; i < MAX_NOTES; i++)
	{
		data[i].data = &notepadbuffer[i][0];
		data[i].len = 0;
		data[i].size = 0;
		data[i].chan_nr = CHANNEL_ALL;
		note_buttons[i] = -1;
	}
}

int notepadLoadFile ()
{
	xmlDocPtr doc;
	xmlNodePtr cur;
	xmlChar *name;

	// XXX FIXME: read from $HOME/.elc on unix	
	doc = xmlParseFile ("notes.xml");
	if (doc == NULL )
	{
		log_error ("Unable to parse xml notepad. It will be overwritten.");
		return 0;
	}
	
	cur = xmlDocGetRootElement (doc);
	if (cur == NULL)
	{
		log_error ("Empty xml notepad. It will be overwritten.");
		xmlFreeDoc(doc);
		return 0;
	}
	
	if (xmlStrcasecmp (cur->name, (const xmlChar *) "PAD"))
	{
		fprintf (stderr, "Document of the wrong type. It will be overwritten.\n");
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
			if(no_notes >= MAX_NOTES)
			{
				log_error("Too many notes - Last nodes were ignored\n");
				return 2;
			}
			note[no_notes] = malloc(sizeof(struct Note));
			if (cur->children == NULL)
			{
				data[no_notes].len = 0;
				data[no_notes].data[0] = '\0';
			}
			else
			{
				my_strcp (data[no_notes].data, cur->children->content);
				data[no_notes].len = strlen (data[no_notes].data);
			}
			data[no_notes].size = NOTE_DATA_LEN;
			data[no_notes].chan_nr = CHANNEL_ALL;
				    
			reset_soft_breaks (data[no_notes].data, data[no_notes].len, 1, note_win_x_len - 70);
			
			name = xmlGetProp (cur, "NAME");
			strncpy (note[no_notes]->name, name, NOTE_NAME_LEN);
			note[no_notes]->window = -1;
			xmlFree (name);
			no_notes++;
		}
		else
		{
			log_error ("Incorrect node type - could not copy.\n");
		}
		cur = cur->next;         // Advance to the next node.
	}
	return 1;
}


int notepadSaveFile()
{
	int i = 0;
	xmlDocPtr doc = NULL;                      // document pointer
	xmlNodePtr root_node = NULL, node = NULL;  // node pointers
    
	doc = xmlNewDoc (BAD_CAST "1.0");
	root_node = xmlNewNode(NULL, BAD_CAST "PAD");
	xmlDocSetRootElement(doc, root_node);
	while (i < no_notes)
	{
		node = xmlNewChild(root_node, NULL, BAD_CAST "NOTE", BAD_CAST data[i].data);
		xmlNewProp (node, BAD_CAST "NAME", BAD_CAST note[i]->name);
		i++;
	}
	xmlSaveFormatFileEnc("notes.xml", doc, "UTF-8", 1);
	xmlFreeDoc(doc);

	return 1;
}

int notepadRemoveCategory (widget_list *w)
{
	int i, id = -1, t;
	widget_list *k;
	unsigned short x, y;

	t = tab_collection_get_tab_id(notepad_win, note_tabcollection_id);
     
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
	widget_destroy (note[id]->window, note[id]->scroll);
	widget_destroy (note[id]->window, note[id]->input);
	widget_destroy (note[id]->window, note[id]->button);
	note[i]->button = label_add (note[id]->window, NULL, "You should close this tab now.", 5, 5);
     
	free(data[id].data);
	free(note[id]);
	if(id == no_notes-1)
	{
		note[id] = NULL;
		data[id].data = NULL;
		data[id].len = 0;
		data[id].size = 0;
      
		widget_destroy (main_note_tab_id, note_buttons[id]);
		note_buttons[id] = -1;
	}
	else
	{
		k = widget_find(main_note_tab_id, note_buttons[id]);
		x = k->pos_x;
		y = k->pos_y;

		note[id] = note[no_notes-1];
		widget_destroy(main_note_tab_id, note_buttons[id]);
		note_buttons[id] = note_buttons[no_notes-1];
		widget_move(main_note_tab_id, note_buttons[id], x, y);
		note[no_notes-1] = NULL;
		data[id].data = data[no_notes-1].data;
		data[id].len = data[no_notes-1].len;
		data[id].size = data[no_notes-1].size;
		data[no_notes-1].data = NULL;
		data[no_notes-1].len = 0;
		data[no_notes-1].size = 0;
	}
	no_notes--;
	return 1;
}


int tabOnDestroy(window_info *w)
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
	if (note[id]->scroll) widget_destroy(note[id]->window, note[id]->scroll);
	if (note[id]->input) widget_destroy(note[id]->window, note[id]->input);
	if (note[id]->button) widget_destroy(note[id]->window, note[id]->button);
    
	note[i]->window = -1;
    
	return 1;
}


void openNoteTabContinued (int id)
{
	if (tab_collection_get_nr_tabs (notepad_win, note_tabcollection_id) >= MAX_TABS)
	{
		return;
	} 
	note[id]->window = tab_add (notepad_win, note_tabcollection_id, note[id]->name, 0, 1);
	widget_set_color (notepad_win, note[id]->window, 0.77f, 0.57f, 0.39f);
	note[id]->scroll = vscrollbar_add (note[id]->window, NULL, note_win_x_len - 30, 15, 20, note_win_y_len - 50);
	widget_set_color (note[id]->window, note[id]->scroll, 0.77f, 0.57f, 0.39f);
	note[id]->input = text_field_add_extended (note[id]->window, note_widget_id++, NULL, 20, 55, note_win_x_len - 70, note_win_y_len - 85, TEXT_FIELD_BORDER|TEXT_FIELD_EDITABLE, 1.0f, 0.77f, 0.57f, 0.39f, &data[id], 1, 0, 0, 0, 0.77f, 0.57f, 0.39f);
	widget_set_color (note[id]->window, note[id]->input, 0.77f, 0.57f, 0.39f);
	note[id]->button = button_add (note[id]->window, NULL, "Remove Category" , 55, 26);
	widget_set_OnClick (note[id]->window, note[id]->button, notepadRemoveCategory);
	widget_set_color (note[id]->window, note[id]->button, 0.77f, 0.57f, 0.39f);
    
	set_window_handler (note[id]->window, ELW_HANDLER_DESTROY, tabOnDestroy);
}
     

int openNoteTab (widget_list *w)
{
	int i = 0, id = -1;

	for(i = 0; i < no_notes; i++)
	{
		if (w->id == note_buttons[i])
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


void notepadAddContinued (char *name)
{
	int y_pos;
	
	if (no_notes == 0)
	{
		y_pos = 0;
	}
	else
	{
		widget_list *w = widget_find (main_note_tab_id, note_buttons[no_notes-1]);
		y_pos = w->pos_y + 21;
	}

	if (no_notes >= MAX_NOTES)
	{
		log_error ("Tried to exceed notepad buffer! Ignored.\n");
		return;
	}
	no_notes++;
	
	note[no_notes-1] = malloc ( sizeof (struct Note) );
	note[no_notes-1]->window = -1;
	my_strcp (note[no_notes-1]->name, name);
	note_buttons[no_notes-1] = label_add (main_note_tab_id, NULL, note[no_notes-1]->name, 5, y_pos);
	widget_set_OnClick (main_note_tab_id, note_buttons[no_notes-1], openNoteTab);
	widget_set_color(main_note_tab_id, note_buttons[no_notes-1], 0.77f, 0.57f, 0.39f);
     
	openNoteTabContinued(no_notes-1);
}


int notepadAddCategory()
{
	display_popup_win ("Note Name:", 16, notepadAddContinued);

	return 1;
}   

	
void display_notepad()
{
	int i;
	int j = 0;
     
	if (notepad_win < 0)
	{
		notepad_win = create_window ("NotePad", game_root_win, 0, note_win_x, note_win_y, note_win_x_len, note_win_y_len, ELW_WIN_DEFAULT|ELW_TITLE_NAME);
		
		note_tabcollection_id = tab_collection_add (notepad_win, NULL, 4, 5, note_win_x_len - 10, note_win_y_len - 8, 20, 3);
		widget_set_size (notepad_win, note_tabcollection_id, 0.7);
		widget_set_color (notepad_win, note_tabcollection_id, 0.77f, 0.57f, 0.39f);
		main_note_tab_id = tab_add(notepad_win, note_tabcollection_id, "Main", 0, 0);
		widget_set_color(notepad_win, main_note_tab_id, 0.77f, 0.57f, 0.39f);
		
		buttons[0] = button_add(main_note_tab_id, NULL, "New Category" , 55, 5+(21*j++));
		widget_set_OnClick (main_note_tab_id, buttons[0], notepadAddCategory);
		widget_set_color(main_note_tab_id, buttons[0], 0.77f, 0.57f, 0.39f);
		buttons[1] = button_add(main_note_tab_id, NULL, "Save Notes" , 55, 5+(21*j++));
		widget_set_OnClick(main_note_tab_id, buttons[1], notepadSaveFile);
		widget_set_color(main_note_tab_id, buttons[1], 0.77f, 0.57f, 0.39f);
		
		init_notepad_buffers ();	
		notepadLoadFile ();

		for(i = 0; i < no_notes; i++)
		{
			note_buttons[i] = label_add (main_note_tab_id, NULL, note[i]->name, 5, 5+(21*(i+j+1)));
			widget_set_OnClick (main_note_tab_id, note_buttons[i], openNoteTab);
			widget_set_color(main_note_tab_id, note_buttons[i], 0.77f, 0.57f, 0.39f);
		}
        
		set_window_handler (notepad_win, ELW_HANDLER_KEYPRESS, keypress_input_window_handler);	
	}
	else
	{
		show_window(notepad_win);
		select_window(notepad_win);
	}
}

#endif
