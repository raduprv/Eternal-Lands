/*
	langselwin.c - Shows a language selection window if no language specified in el.ini

	Each available language is displayed from languages/langsel.xml.  The user can click on
	their preferred language and then press save which will save the value in el.ini.
	Beside the save button a note can be displayed explaining a bit about languages in EL. 
	If the language "en" is chosen the client will continue to open the login/new
	character/rules screen.   Otherwise, the client will be restarted.  All the text and
	colours are configurable in the langsel.xml file.

	Most user will only see this window once, existing users probably never :(
	
	23/09/07 bluap/pjbroad
*/


#include <string.h>
#include <libxml/parser.h>

#include "asc.h"
#include "elconfig.h"
#include "elwindows.h"
#include "errors.h"
#include "gamewin.h"
#include "gl_init.h"
#include "hud.h"
#include "init.h"
#include "interface.h"
#include "list.h"
#include "loginwin.h"
#include "multiplayer.h"
#include "openingwin.h"
#include "rules.h"
#include "sound.h"

typedef struct { char *code; char *text; char *save; char *note; } LANGSEL_LIST_NODE;

int langsel_rootwin = -1;
int have_saved_langsel = 0;
static int langsel_win = -1;
static int langsel_scroll_id = -1;
static int langsel_first_lang_line = 0;
static int langsel_num_note_lines = 4;
static char *langsel_save_note_boxed = NULL;
static char *langsel_list_error = NULL;
static list_node_t *langsel_list = NULL;
static LANGSEL_LIST_NODE *langsel_default_node = NULL;
static LANGSEL_LIST_NODE *langsel_chosen_node = NULL;
static LANGSEL_LIST_NODE *langsel_selected_node = NULL;
static float langsel_winRGB[4][3] = {{0.0f,0.25f,1.0f},{0.2f,0.7f,1.2f},{0.2f,1.0f,1.2f},{0.77f, 0.57f, 0.39f}};	


static int langsel_load_list(void)
{
	xmlDocPtr doc;
	xmlNodePtr cur;
	char *error_prefix = "Reading langsel.xml: ";
	
	if ((doc = xmlReadFile("languages/langsel.xml", NULL, 0)) == NULL)
	{
		langsel_list_error = "Can't open file.";
		LOG_ERROR("%s%s\n", error_prefix, langsel_list_error );
		return 0;
	}
	
	if ((cur = xmlDocGetRootElement (doc)) == NULL)
	{
		langsel_list_error = "Empty xml document.";
		LOG_ERROR("%s%s\n", error_prefix, langsel_list_error );
		xmlFreeDoc(doc);
		return 0;
	}
	
	if (xmlStrcasecmp (cur->name, (const xmlChar *) "LANGUAGE_LIST"))
	{
		langsel_list_error = "Not language list.";
		LOG_ERROR("%s%s\n", error_prefix, langsel_list_error );
		xmlFreeDoc(doc);
		return 0;
	}
	
	for (cur = cur->xmlChildrenNode; cur; cur = cur->next)
	{
		if (!xmlStrcasecmp(cur->name, (const xmlChar *)"LANG"))
		{
			LANGSEL_LIST_NODE *new_lang_node = NULL;
			char *note = (char*)(cur->children ? cur->children->content : NULL);
			char *code = (char*)xmlGetProp(cur, (xmlChar *)"CODE");
			char *text = (char*)xmlGetProp(cur, (xmlChar *)"TEXT");
			char *save = (char*)xmlGetProp(cur, (xmlChar *)"SAVE");
			char *def = (char*)xmlGetProp(cur, (xmlChar *)"DEFAULT");
			
			if ((code == NULL) || (text == NULL))
			{
				LOG_WARNING("%sInvalid language node\n", error_prefix );
				continue;
			}
			
			new_lang_node = (LANGSEL_LIST_NODE *)malloc(sizeof(LANGSEL_LIST_NODE));
			new_lang_node->note = new_lang_node->code = new_lang_node->text = new_lang_node->save = NULL;
			
			if (note)
				MY_XMLSTRCPY(&new_lang_node->note, note);
			if (code)
				MY_XMLSTRCPY(&new_lang_node->code, code);
			if (text)
				MY_XMLSTRCPY(&new_lang_node->text, text);
			if (save)
				MY_XMLSTRCPY(&new_lang_node->save, save);
			if (def)
				langsel_default_node = new_lang_node;
				
			xmlFree(code);
			xmlFree(text);
			xmlFree(save);
			xmlFree(def);
			
			list_push(&langsel_list, new_lang_node);
		}
		else if (!xmlStrcasecmp(cur->name, (const xmlChar *)"SETTINGS"))
		{
			char *propstr[4] = {"TEXT", "HIGHLIGHT", "CHOSEN", "BUTTONS" };
			char *noteslines = (char*)xmlGetProp(cur, (xmlChar *)"NOTELINES");			
			int i = atoi(noteslines);
			if (i >= 0)
				langsel_num_note_lines = i;
			for (i=0; i<4; i++)
			{
				char *text = (char*)xmlGetProp(cur, (xmlChar *)propstr[i]);
				float r = 0, g = 0, b = 0;
				if (sscanf(text, "%f %f %f", &r, &g, &b) == 3)
				{
					langsel_winRGB[i][0] = r;
					langsel_winRGB[i][1] = g;
					langsel_winRGB[i][2] = b;
				}
				else
					LOG_WARNING("%sColour error\n", error_prefix );
				xmlFree(text);
			}
		}
	}
	xmlFreeDoc(doc);
	
	if (!langsel_default_node || (langsel_default_node->save == NULL))
	{
		langsel_list_error = "Invalid default language.";
		LOG_ERROR("%s%s\n", error_prefix, langsel_list_error );
		if (langsel_default_node)
			langsel_default_node = NULL;
		return 0;
	}

	if (langsel_list == NULL)
	{
		langsel_list_error = "No languages found.";
		LOG_ERROR("%s%s\n", error_prefix, langsel_list_error );
		return 0;
	}
	
	return 1;
}


static void langsel_free_list(void)
{	
	while (langsel_list != NULL)
	{
		LANGSEL_LIST_NODE *new_lang_node = (LANGSEL_LIST_NODE *)list_pop(&langsel_list);
		if (new_lang_node->note)
			free(new_lang_node->note);
		if (new_lang_node->code)
			free(new_lang_node->code);
		if (new_lang_node->text)
			free(new_lang_node->text);
		if (new_lang_node->save)
			free(new_lang_node->save);
		free(new_lang_node);
	}
}


static void langsel_destroy_wins(void)
{
	if (langsel_save_note_boxed)
		free(langsel_save_note_boxed);

	destroy_window(langsel_win);
	destroy_window(langsel_rootwin);
	langsel_win = langsel_rootwin = -1;
}


static int langsel_save_handler(widget_list *widget, int mx, int my, Uint32 flags)
{
	char *selected_lang = lang;
	
	/* don't use scroll wheel - leave for scroll bar */
	if ((flags & ELW_MOUSE_BUTTON) == 0)
		return 0;

	langsel_destroy_wins();
	
	if (langsel_chosen_node)
		selected_lang = langsel_chosen_node->code;
	
	/* if the chosen language the that used during initialisation, no client restart is required */
	if (strcmp(selected_lang, lang) == 0)
	{
		/* go to the console->login screen */
		if (has_accepted)
		{
			show_window (opening_root_win);
			show_hud_windows();
			if (disconnected)
				connect_to_server();
		}
		/* unless we need to read the rules first */
		else
		{
			create_rules_root_window (window_width, window_height, opening_root_win, 15);
			show_window (rules_root_win);
		}
	}
	else
	{
		/* a different language has been chosen so restart the client */
		restart_required = 1;
		exit_now = 1;
	}
	
	/* set the chosen language, it will be saved on exit, then complete the clean up */
	change_language(selected_lang);
	langsel_free_list();
	have_saved_langsel = 1;
	
  	return 1;
}


static int langsel_quit_handler(widget_list *widget, int mx, int my, Uint32 flags)
{
	/* don't use scroll wheel - leave for scroll bar */
	if ((flags & ELW_MOUSE_BUTTON) == 0)
		return 0;
	langsel_destroy_wins();
	langsel_free_list();
	exit_now = 1;
  	return 1;
}


static int langsel_scroll_click_handler(widget_list *widget, int mx, int my, Uint32 flags)
{
	langsel_first_lang_line = vscrollbar_get_pos(widget->window_id, widget->id);
	return 1;
}


static int langsel_scroll_drag_handler(widget_list *widget, int mx, int my, Uint32 flags, int dx, int dy)
{
	return langsel_scroll_click_handler(widget, mx, my, flags);
}


static int click_langsel_handler(window_info *win, int mx, int my, Uint32 flags)
{
	if ((langsel_scroll_id > 0) && flags & ELW_WHEEL_UP)
		vscrollbar_scroll_up(langsel_win, langsel_scroll_id);
	else if ((langsel_scroll_id > 0) && flags & ELW_WHEEL_DOWN)
		vscrollbar_scroll_down(langsel_win, langsel_scroll_id);
	else if ((flags & ELW_MOUSE_BUTTON) && langsel_selected_node)
	{
		langsel_chosen_node = langsel_selected_node;
		do_click_sound();
	}
	if (langsel_scroll_id > 0)
		langsel_first_lang_line = vscrollbar_get_pos(langsel_win, langsel_scroll_id);
	return 1;
}


static int langsel_keypress_handler(window_info *win, int mx, int my, Uint32 key, Uint32 unikey)
{
	if (check_quit_or_fullscreen(key))
	{
		return 1;
	}
	else if (key == K_OPAQUEWIN)
	{
		win->opaque ^= 1;
	}	
	return 1;
}


/* display the loading/login image as a background */
static int langsel_display_root_handler(window_info *win)
{
	if (login_text > 0)
		draw_console_pic(login_text);
	return 1;
}


/* displayed if the config file is missing or invalid */
static int langsel_display_error_handler(window_info *win)
{
	static int first_time = 1;
	static int save_button = -1;
	static int quit_button = -1;
	static char *message = "The language selection file langsel.xml could\n"
						   "not be read. Either click Save to accept the\n"
						   "default language (English), or click Quit if\n"
						   "you wish to manually correct this error.\n\n"
						   "The error message was:";
	
	if (first_time)
	{
		widget_list *save_widget = NULL;
		
		save_button = button_add_extended(langsel_win, 100, NULL, 10, win->len_y-40,
			0, 0, 0, 1.0f, langsel_winRGB[3][0], langsel_winRGB[3][1], langsel_winRGB[3][2], "Save");
		widget_set_OnClick(langsel_win, save_button, langsel_save_handler);
		save_widget = widget_find(langsel_win, save_button);
		
		quit_button = button_add_extended(langsel_win, 101, NULL, 20 + save_widget->len_x, win->len_y-40,
			0, 0, 0, 1.0f, langsel_winRGB[3][0], langsel_winRGB[3][1], langsel_winRGB[3][2], "Quit");
		widget_set_OnClick(langsel_win, quit_button, langsel_quit_handler);
		
		first_time = 0;
		return 1;
	}

	draw_string_small(10, 10, (unsigned char *)message, 6);
	draw_string_small(10, 10 + SMALL_FONT_Y_LEN * 6,
		(unsigned char *)((langsel_list_error) ?langsel_list_error : "Unknown error"), 1);

	return 1;
}


/* the main business is done here */
static int display_langsel_handler(window_info *win)
{
	static float font_zoom = 1.5;
	static int num_lang_lines = 0;
	static float max_str_width = 0;
	static float line_step = DEFAULT_FONT_Y_LEN;
	static list_node_t *first_node = NULL;
	static LANGSEL_LIST_NODE *last_langsel_chosen_node = NULL;
	static int save_button = -1;
	static int first_time = 1;
	static int second_time = 0;
	static int add_scroll_bar = 0;
	static int max_lang_lines = 0;
	static int winwidth = 0;
	static int winheight = 0;
	static char *langwin_save_note = NULL;
	
	const float winsep = 15;
	const int scroll_width = 20;
	list_node_t *local_head = NULL;
	int current_y = 0;
	int lang_line_num = 0;
	float note_height = SMALL_FONT_Y_LEN * langsel_num_note_lines;
	
	set_font(0);
	
	/* first time through, create additional widgets and resize everything */
	if (first_time)
	{
		widget_list *save_widget = NULL;
		char *longest_string = NULL;
		int non_line_height = 0;
		float sizefrac = 0.80;
		
		/* count the number of language lines and find the widest line and the first line
		   - font_zoom unknow as yet */
		for (local_head = langsel_list; local_head; local_head = local_head->next)
		{
			LANGSEL_LIST_NODE *new_lang_node = (LANGSEL_LIST_NODE *)local_head->data;
			float str_width = get_string_width((unsigned char*)new_lang_node->text);
			if (str_width > max_str_width)
			{
				max_str_width = str_width;
				longest_string = new_lang_node->text;
			}
			num_lang_lines++;
			first_node = local_head;
		}
		max_str_width *= DEFAULT_FONT_X_LEN / 12.0;

		/* if required, change the zoom based on a reasonable limit of using all the main window */
		if ((font_zoom * max_str_width) > (sizefrac * window_width))
			font_zoom = sizefrac * window_width / max_str_width;

		/* number of pixels used per language line drawn */		
		line_step = 3 + line_step * font_zoom;
		
		/* the above calc will potentiall be wrong due to rounding
		   errors each time a char is printed - so we have to recalculate again! */
		max_str_width = 0;
		while (*longest_string != '\0')
			max_str_width += (0.5 + get_char_width(*longest_string++) * font_zoom * DEFAULT_FONT_X_LEN / 12.0);
		
		/* set the window width now things about the width are known */
		winwidth = max_str_width + 2 * winsep;
		
		/* to set the height, we need to know button sizes so create a temporary button */
		save_button = button_add_extended(langsel_win, 100, NULL, 0, 0, 0, 0, WIDGET_INVISIBLE, 1.0f, 0, 0, 0, "Temp");
		save_widget = widget_find(langsel_win, save_button);
		
		/* calculate the window height assuming we can display all languages without a scroll bar */
		max_lang_lines = num_lang_lines;
		if (save_widget->len_y > note_height)
			non_line_height = 3 * winsep + save_widget->len_y;
		else
			non_line_height = 3 * winsep + note_height;
		winheight = non_line_height + line_step * max_lang_lines;
		
		/* if the height is too big, reduce it to something reasonable and add a scroll bar */
		if (winheight > (sizefrac * window_height))
		{
			add_scroll_bar = 1;
			max_lang_lines = ((sizefrac * window_height) - non_line_height) / line_step;
			winheight = non_line_height + line_step * max_lang_lines;
			winwidth += scroll_width;
		}
		
		/* move the window to the centre of the screen and resize it as calculate */
		move_window(langsel_win, langsel_rootwin, 0, (window_width-winwidth)/2, (window_height-winheight)/2);
		resize_window(langsel_win, winwidth, winheight);
		
		/* clean up then exit, leave the rest to next time so the buttons don't flash on the screen */
		widget_destroy(langsel_win, save_button);
		save_button = -1;
		first_time = 0;
		second_time = 1;
		return 1;
	}

	/* if no button etc, or language choice has changed, redisplay the button and note */
	if (second_time || (langsel_chosen_node && (last_langsel_chosen_node != langsel_chosen_node)))
	{
		widget_list *save_widget = NULL;
		char *save = "Save";
		
		/* get the save button text, using the default if required */
		if (langsel_chosen_node->save)
			save = langsel_chosen_node->save;
		else if (langsel_default_node->save)
			save = langsel_default_node->save;
		
		/* create the save button, resized and moved to keep things tidy */
		if (save_button > 0)
			widget_destroy(langsel_win, save_button);
		save_button = button_add_extended(langsel_win, 100, NULL, 0, 0,
			0, 0, 0, 1.0f, langsel_winRGB[3][0], langsel_winRGB[3][1], langsel_winRGB[3][2], save);
		save_widget = widget_find(langsel_win, save_button);
		widget_move(langsel_win, save_button, winwidth - (winsep + save_widget->len_x), winheight - winsep - save_widget->len_y);
		widget_set_OnClick(langsel_win, save_button, langsel_save_handler);

		/* get the note text, using the default if required */
		if (langsel_chosen_node->note)
			langwin_save_note = langsel_chosen_node->note;
		else if (langsel_default_node->note)
			langwin_save_note = langsel_default_node->note;
		else
			langwin_save_note = NULL;

		/* wrap the text so that it fits into the window space available */
		if (langwin_save_note)
		{
			langsel_save_note_boxed = (char *)realloc(langsel_save_note_boxed, strlen(langwin_save_note)*2);
			put_small_colored_text_in_box(c_grey1, (const Uint8 *)langwin_save_note,
				strlen(langwin_save_note), winwidth - (2 * winsep + save_widget->len_x), langsel_save_note_boxed);
		}
		
		if (add_scroll_bar)
		{
			langsel_scroll_id = vscrollbar_add_extended(langsel_win, 102, NULL,  winwidth - scroll_width, winsep,
				scroll_width, line_step * max_lang_lines, 0, 1.0,
				langsel_winRGB[3][0], langsel_winRGB[3][1], langsel_winRGB[3][2], 0, 1, num_lang_lines - max_lang_lines);
			widget_set_OnDrag(langsel_win, langsel_scroll_id, langsel_scroll_drag_handler);
			widget_set_OnClick(langsel_win, langsel_scroll_id, langsel_scroll_click_handler);
			add_scroll_bar = 0;
		}
		
		last_langsel_chosen_node = langsel_chosen_node;
		second_time = 0;
	}
	
	/* if we have note text, display it */
	if (langwin_save_note)
		draw_string_small(winsep, winheight - winsep - note_height, (unsigned char *)langsel_save_note_boxed, langsel_num_note_lines);

	/* draw a line under the language list */
	glColor3f(langsel_winRGB[3][0], langsel_winRGB[3][1], langsel_winRGB[3][2]);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_LINES);
	glVertex2i(winsep, winsep + line_step * max_lang_lines);
	glVertex2i(winsep + max_str_width, winsep + line_step * max_lang_lines);
	glEnd();
	glEnable(GL_TEXTURE_2D);
	
	/* display the language list */
	current_y = winsep;
	langsel_selected_node = NULL;
	lang_line_num = 0;
	for (local_head = first_node; local_head; local_head = local_head->prev, lang_line_num++)
	{
		LANGSEL_LIST_NODE *new_lang_node = (LANGSEL_LIST_NODE *)local_head->data;
		
		/* if off the top if the window, don't display */
		if (lang_line_num < langsel_first_lang_line)
			continue;
		
		/* if off the bottom of the window, don't display this or any more lines */
		if ((lang_line_num - langsel_first_lang_line) >= max_lang_lines)
			break;
		
		/* colour the list lines as required*/
		if (new_lang_node == langsel_chosen_node)
			glColor3f(langsel_winRGB[2][0],langsel_winRGB[2][1],langsel_winRGB[2][2]);
		/* if the mouse is over the current line.... */
		else if ((mouse_y > win->cur_y + current_y) &&
			(mouse_y < win->cur_y + current_y + line_step) &&
			(mouse_x >= win->cur_x + winsep) &&
			(mouse_x - winsep <= win->cur_x + max_str_width))
		{
			glColor3f(langsel_winRGB[1][0],langsel_winRGB[1][1],langsel_winRGB[1][2]);
			/* save the highlighted line in case it is clicked - and so chosen */
			langsel_selected_node = new_lang_node;
		}
		else
			glColor3f(langsel_winRGB[0][0],langsel_winRGB[0][1],langsel_winRGB[0][2]);
		
		/* draw the line of text and step down for the next */
		draw_string_zoomed(winsep, current_y, (unsigned char *)new_lang_node->text, 1, font_zoom);		
		current_y += line_step;
	}
			
	return 1;
	
} /* end display_langsel_handler() */


/* load the language list and create the windows */
int display_langsel_win(void)
{
	int loaded_lang_list = langsel_load_list();
	langsel_chosen_node = langsel_default_node;
	
	/* create and show the root window */
	langsel_rootwin = create_window("", -1, -1, 0, 0, window_width, window_height, ELW_TITLE_NONE|ELW_SHOW_LAST);
	set_window_handler(langsel_rootwin, ELW_HANDLER_DISPLAY, &langsel_display_root_handler );
	show_window(langsel_rootwin);
	
	/* create and show the language selection window */
	langsel_win = create_window("", langsel_rootwin, -1, (window_width-400)/2, (window_height-400/1.62)/2,
		400, 400/1.62, ELW_WIN_DEFAULT^(ELW_CLOSE_BOX|ELW_TITLE_BAR));
	set_window_handler(langsel_win, ELW_HANDLER_CLICK, &click_langsel_handler );
	set_window_handler(langsel_win, ELW_HANDLER_KEYPRESS, &langsel_keypress_handler);
	
	/* use the error window if the list could not be read */
	if (loaded_lang_list)
		set_window_handler(langsel_win, ELW_HANDLER_DISPLAY, &display_langsel_handler );
	else
		set_window_handler(langsel_win, ELW_HANDLER_DISPLAY, &langsel_display_error_handler );
	
	show_window(langsel_win);
	
	return 1;
}
