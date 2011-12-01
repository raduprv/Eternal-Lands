#ifdef OSX
	#include <sys/malloc.h>
	#include <CoreFoundation/CoreFoundation.h>
#elif BSD
	#include <stdlib.h>
#else
	#include <malloc.h>
#endif /* OSX */
#if !defined(OSX) && !defined(WINDOWS)
#include <signal.h>
#endif
#include <sys/types.h>
#include <string.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif //MSVC
#include <ctype.h>
#include <SDL.h>
#include <SDL_thread.h>
#include "asc.h"
#include "client_serv.h"
#include "consolewin.h"
#include "context_menu.h"
#ifdef WINDOWS
#include "elloggingwrapper.h"
#endif
#include "elwindows.h"
#include "font.h"
#include "gamewin.h"
#include "hud.h"
#include "interface.h"
#include "list.h"
#include "load_gl_extensions.h"
#include "tabs.h"
#include "text.h"
#include "translate.h"
#include "url.h"
#ifdef OPENGL_TRACE
#include "gl_init.h"
#endif
#include "sound.h"

char browser_name[120];
int url_win_x = 100;
int url_win_y = 50;
int url_win = -1;

static const Uint32 url_win_sep = 5;
static const float url_win_text_zoom = 1.0;
static const int max_url_count = 100;

static int url_scroll_id = 0;
static int url_win_top_line = 0;
static int url_win_x_len = 0;
static int url_win_y_len = 0;
static Uint32 url_win_max_string_width = 0;
static int url_win_help_x = 0;
static int url_win_text_len_y = 0;
static int url_win_text_start_y = 0;
static int url_win_url_y_start = 0;
static int url_win_full_url_y_len = 0;
static int url_win_line_step = 0;
static Uint32 url_win_clicktime = 0;
static enum { URLW_EMPTY=1, URLW_CLEAR, URLW_OVER } url_win_status = URLW_EMPTY;
static list_node_t *url_win_clicked_url = NULL;
static list_node_t *url_win_hover_url = NULL;

static int have_url_count = 0;
static int saved_url_count = 0;

static list_node_t *newest_url = NULL;
static list_node_t *active_url = NULL;
static list_node_t *cm_url = NULL;


/* define the structure stored for each URL */
typedef struct
{
	int seen_count;		/* incremented each time a link is seen in chat */
	int visited;		/* true when link has been opened in the browser */
	char *text;			/* the actual url text */
} URLDATA;


/* store the current url count, used in num_new_url() */
void save_url_count(void)
{
	saved_url_count = have_url_count;
}


/* return the number of url since last save_url_count() call */
int num_new_url(void)
{
	return have_url_count - saved_url_count;
}


/* open last seen URL */
void open_last_seen_url(void)
{
	if (!have_url_count)
		return;
	open_web_link(((URLDATA *)active_url->data)->text);
	((URLDATA *)active_url->data)->visited = 1;
}


/* free all url list memory */
void destroy_url_list(void)
{
	if (have_url_count)
	{
		/* free all the text */
		list_node_t *local_head = newest_url;
		while (local_head->next != NULL)
		{
			if (local_head->data != NULL)
				free(((URLDATA *)local_head->data)->text);
			local_head = local_head->next;
		}

		/* free the list */
		list_destroy(newest_url);
		saved_url_count = have_url_count = 0;
		active_url = newest_url = NULL;
	}
}


/* #url command - List, clear list or open a specific URL */
int url_command(const char *text, int len)
{
	/* no URLs so far so display a message then exit */
	if (!have_url_count)
	{
		LOG_TO_CONSOLE(c_red2, urlcmd_none_str);
		return 1;
	}

	/* get any parameter text */
	while(*text && !isspace(*text))
		text++;
	while(*text && isspace(*text))
		text++;
        
	/* no parameter specified - list the URL(s) we have, oldest first */
	if (!strlen(text))
	{
		char *out_str = NULL;
		size_t out_len = 0;
		int irl_num = 0;
		int line_colour = c_grey1;
		list_node_t *local_head = newest_url;

		LOG_TO_CONSOLE(c_green2, urlcmd_list_str);

		/* go to the oldest in the list */
		while (local_head->next != NULL)
			local_head = local_head->next;
            
		/* display the list ending with the newest, alternating colours */
		while (local_head != NULL)
		{
			size_t new_len = sizeof(char) * (strlen(((URLDATA *)local_head->data)->text) + 60);
			if (new_len > out_len)
			{
				if (out_str != NULL)
					free(out_str);
				out_str = (char *)malloc(new_len);
				out_len = new_len;
			}
			safe_snprintf(out_str, new_len, "%c %d) %s (seen %d time%s) (%s)", ((local_head==active_url) ?'>':' '),
				 ++irl_num, ((URLDATA *)local_head->data)->text, ((URLDATA *)local_head->data)->seen_count,
				 ((URLDATA *)local_head->data)->seen_count == 1?"":"s",  ((((URLDATA *)local_head->data)->visited) ?"visited":"unvisited"));
			LOG_TO_CONSOLE(line_colour, out_str);
			local_head = local_head->prev;
			line_colour = (line_colour==c_grey1) ?c_grey2 :c_grey1;
		}

		if (out_str != NULL)
			free(out_str);
	}
    
	/* if parameter is "clear" delete all entries */
	else if (strcmp(text, urlcmd_clear_str) == 0)
	{
		destroy_url_list();
	}
    
	/* else assume parameter is an index, if its a valid index, open the URL */
	else
	{
		int open_index = atoi(text) - 1;
		int valid_node = 0;
		if (open_index >= 0)
		{
			list_node_t *local_head = newest_url;
			int url_num = 0;
			/* go to the oldest int the list */
			while (local_head->next != NULL)
				local_head = local_head->next;
			/* go to the specified entry */
			while ((local_head->prev != NULL) && (url_num < open_index))
			{
				local_head = local_head->prev;
				url_num++;
			}
			/* if we end up at a valid node, go for it */
			if ((local_head != NULL) && (url_num == open_index) && strlen(((URLDATA *)local_head->data)->text))
			{
				open_web_link(((URLDATA *)local_head->data)->text);
				((URLDATA *)local_head->data)->visited = 1;
				valid_node = 1;
			}
		}
		if (!valid_node)
			LOG_TO_CONSOLE(c_red2, urlcmd_invalid_str);
	}
            
	return 1;
}


/* find and store all urls in the provided string */
void find_all_url(const char *source_string, const int len)
{
	char search_for[][10] = {"http://", "https://", "ftp://", "www."};
	int next_start = 0;
    
	while (next_start < len)
	{
		int first_found = len-next_start; /* set to max */
		int i;
        
		/* find the first of the url start strings */
		for(i = 0; i < sizeof(search_for)/10; i++)
		{
			int found_at = get_string_occurance(search_for[i], source_string+next_start, len-next_start, 1);
			if ((found_at >= 0) && (found_at < first_found))
				first_found = found_at;
		}
        
		/* if url found, store (if new) it then continue the search straight after the end */
		if (first_found < len-next_start)
		{
			char *new_url = NULL;
			char *add_start = "";
			size_t url_len;
			int url_start = next_start + first_found;
			int have_already = 0;
			
			/* find the url end */
			for (next_start = url_start; next_start < len; next_start++)
			{
				char cur_char = source_string[next_start];
				if(!cur_char || cur_char == ' ' || cur_char == '\n' || cur_char == '<'
					|| cur_char == '>' || cur_char == '|' || cur_char == '"' || cur_char == '\'' || cur_char == '`'
					|| cur_char == ']' || cur_char == ';' || cur_char == '\\' || (cur_char&0x80) != 0)
					break;
			}
            
			/* prefix www. with http:// */
			if (strncmp(&source_string[url_start], "www.", 4) == 0)
				add_start = "http://";
			
			/* extract the string */
			url_len = strlen(add_start) + (next_start-url_start) + 1;
			new_url = (char *)malloc(sizeof(char)*url_len);
			/* could use safe_xxx() functions but I think its simpler not to here */
			strcpy(new_url, add_start);
			strncat(new_url, &source_string[url_start], next_start-url_start );
			new_url[url_len-1] = 0;
			
			/* check the new URL is not already in the list */
			if (have_url_count)
			{
				list_node_t *local_head = newest_url;
				while (local_head != NULL)
				{
					/* if its already stored, just make existing version active */
					if (strcmp(((URLDATA *)local_head->data)->text, new_url) == 0)
					{
						active_url = local_head;
						((URLDATA *)local_head->data)->seen_count++;
						have_already = 1;
						free(new_url);
						break;
					}
					local_head = local_head->next;
				}
			}
			
			/* if its a new url, create a new node in the url list */
			if (!have_already)
			{
				URLDATA *new_node = (URLDATA *)malloc(sizeof(URLDATA));
				
				/* if there's a max number of url and we've reached it, remove the oldest */
				/* we don't need to worry if its the active_url as thats going to change */
				if (max_url_count && (max_url_count==have_url_count))
				{
					list_node_t *local_head = newest_url;
					/* go to the oldest in the list */
					while (local_head->next != NULL)
						local_head = local_head->next;
					free(((URLDATA *)local_head->data)->text);
					free(local_head->data);
					if (local_head==newest_url)
					{
						/* the special case is when max_url_count=1... */
						free(local_head);
						newest_url = NULL;
					}
					else
					{
						local_head = local_head->prev;
						free(local_head->next);
						local_head->next = NULL;
					}
					have_url_count--;
				}
			
				new_node->seen_count = 1;
				new_node->visited = 0;
				new_node->text = new_url;
				list_push(&newest_url, new_node);
				active_url = newest_url;
				have_url_count++;
			}
			
		} /* end if url found */
        
		/* no more urls found so stop looking */
		else
			break;        
	}
    
} /* end find_all_url() */


#ifdef  WINDOWS
static int only_call_from_open_web_link__go_to_url(void * url)
{
	char browser_command[400];

	init_thread_log("web_link");

	// build the command line and execute it
	safe_snprintf (browser_command, sizeof (browser_command), "%s \"%s\"", browser_name, url),
	system(browser_command);	// Do not use this command on UNIX.

	// free the memory allocated in open_web_link()
	free(url);

	return 0;
}
#endif

void open_web_link(const char * url)
{
#ifdef OSX
	CFURLRef newurl = CFURLCreateWithString(kCFAllocatorDefault,CFStringCreateWithCStringNoCopy(NULL,url,kCFStringEncodingMacRoman, NULL),NULL);
	LSOpenCFURLRef(newurl,NULL);
	CFRelease(newurl);
#else
	// browser name can override the windows default, and if not defined in Linux, don't error
	if(*browser_name){
#ifndef WINDOWS
		static int have_set_signal = 0;
		
		/* we're not interested in the exit status of the child so
		   set SA_NOCLDWAIT to stop it becoming a zombie if we don't wait() */
		if (!have_set_signal)
		{
			struct sigaction act;
			memset(&act, 0, sizeof(act));
			act.sa_handler = SIG_DFL;
			act.sa_flags = SA_NOCLDWAIT;
			sigaction(SIGCHLD, &act, NULL);
			have_set_signal = 1;
		}
		
		if (fork() == 0){
			execlp(browser_name, browser_name, url, NULL);
			// in case the exec errors
			_exit(1);
		}
#else
		// make a copy of the url string as it may be freed by the caller
		// will be freed as the only_call_from_open_web_link__go_to_url() exits
		char *cp_url = malloc(strlen(url)+1);
		safe_strncpy(cp_url, url, strlen(url)+1);

		// windows needs to spawn it in its own thread
		SDL_CreateThread(only_call_from_open_web_link__go_to_url, cp_url);
	} else {
		ShellExecute(NULL, "open", url, NULL, NULL, SW_SHOWNOACTIVATE); //this returns an int we could check for errors, but that's mainly when you use shellexecute for local files
#endif  //_WIN32
	}
#endif // OSX
}


/*  Access the caught url list and display in a scrollable window. */
static int display_url_handler(window_info *win)
{
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE

	url_win_hover_url = NULL;
	
	/* save current position so K_WINDOWS_ON_TOP restores to where its been moved */
	url_win_x = win->cur_x;
	url_win_y = win->cur_y;
 
	glEnable(GL_TEXTURE_2D);
	set_font(0);
	
	/* check for external state change */
	if (!have_url_count && !url_win_status)
		url_win_status = URLW_EMPTY;
	/* if we have a status message, display it */
	if (url_win_status)
	{
		char *message[] = { urlcmd_none_str, urlwin_clear_str, urlwin_open_str };
		int y_start = (url_win_text_start_y - 0.75 * url_win_text_zoom * DEFAULT_FONT_Y_LEN)/2;
		glColor3f(1.0f,1.0f,1.0f);
		draw_string_zoomed(url_win_help_x, y_start, (unsigned char *)message[url_win_status-1], 1, 0.75 * url_win_text_zoom);
		url_win_status = (have_url_count) ?0 :URLW_EMPTY;
	}
	
	/* display a page of url */
 	if (have_url_count)
	{
		list_node_t *local_head = newest_url;
		int currenty = url_win_text_start_y;
		int start_url = 0;
		int num_url_displayed = 0;
		
		/* don't scroll if everything will fit in the window, also catch if the list has been cleared via #url */
		if (((url_win_line_step * have_url_count) <= url_win_text_len_y) || (url_win_top_line > have_url_count))
			url_win_top_line = 0;
		
		/* move to the first url to be displayed - set from the scroll bar */
		while (start_url < url_win_top_line && local_head->next != NULL)
		{
			local_head = local_head->next;
			start_url++;
		}
		
		/* loop over the remaining URLs while there is room in the window */
		while (local_head != NULL)
		{
			char *thetext = ((URLDATA *)local_head->data)->text;
			int dsp_string_len = 0;
			float string_width = 0;
			int highlight_url = 0;
			
			/* stop now if the url line will not fit into the window */
			if (((currenty - url_win_text_start_y) + url_win_line_step) > url_win_text_len_y)
				break;
				
			/* highlight the active (F2) url */
			if (local_head == active_url)
				glColor3f(0.0f,1.0f,0.0f);
			else
				glColor3f(1.0f,1.0f,1.0f);
			
			/* calculate the length of string we can display */
			while((*thetext != '\0') && (string_width < url_win_max_string_width))
			{
				float char_width = get_char_width(*thetext++) * url_win_text_zoom * DEFAULT_FONT_X_LEN / 12.0;
				if ((string_width+char_width) < url_win_max_string_width)
				{
					dsp_string_len++;
					string_width += char_width;
				}
			}
						
			/* if the string length will fit in the window, just draw it */
			if (dsp_string_len == strlen(((URLDATA *)local_head->data)->text))
				draw_string_zoomed(url_win_sep, currenty, (unsigned char *)((URLDATA *)local_head->data)->text, 1, url_win_text_zoom);
			/* otherwise, draw a truncated version with "..." at the end */
			else
			{
				//float toobig_width = (get_char_width('-') + get_char_width('>'))
				//	* url_win_text_zoom * DEFAULT_FONT_X_LEN / 12.0;
				float toobig_width = (3*get_char_width('.'))
					* url_win_text_zoom * DEFAULT_FONT_X_LEN / 12.0;
				draw_string_zoomed_width(url_win_sep, currenty, (unsigned char *)((URLDATA *)local_head->data)->text,
					url_win_sep + url_win_max_string_width - toobig_width, 1, url_win_text_zoom);
				draw_string_zoomed(url_win_sep + url_win_max_string_width - toobig_width, currenty,
					(unsigned char *)"..." , 1, url_win_text_zoom);
			}
			
			/* step down a line, do it now as the maths for mouse over below is easier */
			currenty += url_win_line_step;
			
			/* if the mouse is over the current line, hightlight it */
			if ((mouse_y >= win->cur_y + currenty - url_win_line_step) &&
				(mouse_y < win->cur_y + currenty) &&
				(mouse_x >= win->cur_x + (int)url_win_sep) &&
				(mouse_x - (int)url_win_sep <= win->cur_x + url_win_max_string_width))
			{
				/* remember which url we're over in case it's clicked */
				url_win_hover_url = local_head;				
				highlight_url = 1;
			}
				
			/* if a context menu is open, only hightlight the last URL hovered over before the context opened */
			if (cm_window_shown() != CM_INIT_VALUE)
			{
				if (cm_url == local_head)
					highlight_url = 1;
				else
					highlight_url = 0;
			}
			else
				cm_url = NULL;
			
			/* if mouse over or context activated, highlight the current URL */
			if (highlight_url)
			{
				char *help_substring = NULL;
				size_t help_substring_len = 0;
				int dsp_start = 0;
				int helpline = 0;
				Uint32 currenttime = SDL_GetTicks();
				size_t full_help_len = strlen(((URLDATA *)local_head->data)->text) + 30;
				char *full_help_text = (char *)malloc(sizeof(char) * full_help_len);
	
				/* display the mouse over help next time round */
				url_win_status = URLW_OVER;
			
				/* underline the text, just clicked links are red, otherwise blue - paler when visited */
				if ((currenttime - url_win_clicktime < 500) && (url_win_clicked_url == url_win_hover_url))
					glColor3f(1.0f,0.0f,0.3f);
				else if (((URLDATA *)local_head->data)->visited)
					glColor3f(0.3f,0.5f,1.0f);
				else
					glColor3f(0.1f,0.2f,1.0f);
				glDisable(GL_TEXTURE_2D);
				glBegin(GL_LINES);
				glVertex2i(url_win_sep, currenty-2);
				glVertex2i(url_win_sep+string_width, currenty-2);
				glEnd();
				glEnable(GL_TEXTURE_2D);
				
				/* write the full url as help text at the bottom of the window */
				safe_snprintf(full_help_text, full_help_len, "%s (seen %d time%s) (%s)",
					((URLDATA *)local_head->data)->text, ((URLDATA *)local_head->data)->seen_count,
					((URLDATA *)local_head->data)->seen_count == 1?"":"s",  ((((URLDATA *)local_head->data)->visited)?"visited":"unvisited"));
				thetext = full_help_text;
				dsp_string_len = 0;
				string_width = 0;
				while(*thetext != '\0')
				{
					float char_width = get_char_width(*thetext++) * SMALL_FONT_X_LEN / 12.0;
					if (((string_width+char_width) > (win->len_x - 2*url_win_sep)) || (*thetext == '\0'))
					{
						if (*thetext == '\0') /* catch the last line */
							dsp_string_len++;
						if (help_substring_len < dsp_string_len)
						{
							if (help_substring != NULL)
								free(help_substring);
							help_substring = (char *)malloc(sizeof(char)*(dsp_string_len+1));
							help_substring_len = dsp_string_len;
						}
						strncpy(help_substring, &full_help_text[dsp_start], dsp_string_len);
						help_substring[dsp_string_len] = '\0';
						show_help(help_substring, url_win_sep, (url_win_y_len - url_win_full_url_y_len) + helpline++ * SMALL_FONT_Y_LEN);
						dsp_start += dsp_string_len;
						dsp_string_len = 0;
						string_width = 0;
					}
					dsp_string_len++;
					string_width += char_width;
				}
				free(help_substring);
				free(full_help_text);
				
			} /* end if mouse over url */
			
			/* count how many displayed so we can set the scroll bar properly */
			num_url_displayed++;
			
			local_head = local_head->next;
		}
				
		/* set the number of steps for the scroll bar */
		vscrollbar_set_bar_len(url_win, url_scroll_id, have_url_count - num_url_displayed);

	} /* end if have url */
	
	/* draw a line below the list of url, above the current url full text */
	glColor3f(0.77f,0.59f,0.39f);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_LINES);
	glVertex2i(0, url_win_url_y_start);
	glVertex2i(url_win_x_len+1, url_win_url_y_start);
	glEnd();
	glEnable(GL_TEXTURE_2D);

#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
 
	return 1;
	
} /* end display_url_handler() */


/* common function to delete specified url record */
static void delete_current_url(list_node_t *chosen_url)
{
	if (have_url_count && chosen_url != NULL)
	{
		list_node_t *prev = chosen_url->prev;
		list_node_t *next = chosen_url->next;
		if (prev != NULL)
			prev->next = next;
		if (next != NULL)
			next->prev = prev;
		if (chosen_url == newest_url)
			newest_url = next;
		if (chosen_url == active_url)
			active_url = prev;
		if (active_url == NULL)
			active_url = next;
		free(((URLDATA *)chosen_url->data)->text);
		free(chosen_url->data);
		free(chosen_url);
		url_win_hover_url = NULL;
		have_url_count--;
	}
}

/* common function to open link of the specified url record */
static void open_current_url(list_node_t *chosen_url)
{
	if (have_url_count && chosen_url != NULL)
	{
		url_win_clicktime = SDL_GetTicks();
		url_win_clicked_url = chosen_url;
		open_web_link(((URLDATA *)chosen_url->data)->text);
		((URLDATA *)chosen_url->data)->visited = 1;
	}
}

/* called just before a context menu is displayed */
static void context_url_pre_show_handler(window_info *win, int widget_id, int mx, int my, window_info *cm_win)
{
	// propagate opacity from parent tab window
	if (cm_win!= NULL && tab_info_win >-1 && tab_info_win<windows_list.num_windows)
		cm_win->opaque = windows_list.window[tab_info_win].opaque;
}

/* called when a context menu option is selected */
static int context_url_handler(window_info *win, int widget_id, int mx, int my, int option)
{
	if (cm_url)
	{
		switch (option)
		{
			case 0: open_current_url(cm_url); break;
			case 1: 
				{
					char *theurl = ((URLDATA *)cm_url->data)->text;
					char *skiptext = "http://";
					if (theurl && strlen(theurl) > strlen(skiptext))
						history_grep(theurl+strlen(skiptext), strlen(theurl)-strlen(skiptext));
				}
				break;
			case 2: ((URLDATA *)cm_url->data)->visited = 1; break;
			case 3: ((URLDATA *)cm_url->data)->visited = 0; break;
			case 5: delete_current_url(cm_url); break;
			case 7: destroy_url_list(); break;
		}
		cm_url = NULL;
	}
	url_win_clicktime = SDL_GetTicks();
	return 1;
}

/* act on scroll wheel in the main window or clicking a URL */
static int click_url_handler(window_info *win, int mx, int my, Uint32 flags)
{
	static size_t cm_id = CM_INIT_VALUE;

	if (flags & ELW_WHEEL_UP)
		vscrollbar_scroll_up(url_win, url_scroll_id);
	else if (flags & ELW_WHEEL_DOWN)
		vscrollbar_scroll_down(url_win, url_scroll_id);
	else if (have_url_count && url_win_hover_url != NULL)
	{
		if (flags & ELW_CTRL)
		{
			delete_current_url(url_win_hover_url);
			do_window_close_sound();
		}
		else if (flags & ELW_RIGHT_MOUSE)
		{
			cm_url = url_win_hover_url;
			/* create first time needed */
			if (!cm_valid(cm_id))
			{
				cm_id = cm_create(cm_url_menu_str, context_url_handler);
				cm_set_pre_show_handler(cm_id, context_url_pre_show_handler);
			}
			cm_show_direct(cm_id, -1, -1);
		}
		else
		{
			/* open the URL but block double clicks */
			Uint32 currentclicktime = SDL_GetTicks();
			if (currentclicktime < url_win_clicktime)
				url_win_clicktime = 0; /* just in case we're running for 49 days :) */
			if ((currentclicktime - url_win_clicktime > 1000) || (url_win_clicked_url != url_win_hover_url))
			{
				do_click_sound();
				open_current_url(url_win_hover_url);
			}
		}
	}
	url_win_top_line = vscrollbar_get_pos(url_win, url_scroll_id);
	return 0;
}

static int url_win_click_clear_all(widget_list *widget, int mx, int my, Uint32 flags)
{
	if ((flags & ELW_WHEEL_UP) || (flags & ELW_WHEEL_DOWN))
		return 1;
	destroy_url_list();
	return 1;
}

static int url_win_mouseover_clear_all(widget_list *widget, int mx, int my)
{
	url_win_status = URLW_CLEAR;
	return 1;
}


static int url_win_scroll_click(widget_list *widget, int mx, int my, Uint32 flags)
{
	url_win_top_line = vscrollbar_get_pos(url_win, url_scroll_id);
	return 1;
}


static int url_win_scroll_drag(widget_list *widget, int mx, int my, Uint32 flags, int dx, int dy)
{
	return url_win_scroll_click(widget, mx, my, flags);
}


/* fill the URL window created as a tab. */
void fill_url_window(void)
{
	const Uint32 scroll_width = 20;
	const Uint32 cross_height = 20;
	int clear_all_button = 101;
	widget_list *widget;

	/* create the main window */
	url_win_x_len = INFO_TAB_WIDTH;
	url_win_y_len = INFO_TAB_HEIGHT;
	url_win_max_string_width = url_win_x_len - (2*url_win_sep + scroll_width);
	set_window_handler(url_win, ELW_HANDLER_DISPLAY, &display_url_handler );
	set_window_handler(url_win, ELW_HANDLER_CLICK, &click_url_handler );

	/* create the clear all button */
	clear_all_button = button_add_extended (url_win, clear_all_button, NULL,
		url_win_sep, url_win_sep, 0, 0, 0, 0.75, 0.77f, 0.57f, 0.39f, "CLEAR ALL ");
	widget_set_OnClick(url_win, clear_all_button, url_win_click_clear_all);
	widget = widget_find(url_win, clear_all_button);
	widget_set_OnMouseover(url_win, clear_all_button, url_win_mouseover_clear_all);
	
	/* calc text and help postions from size of other stuff */
	url_win_text_start_y = 2*url_win_sep + ((widget->len_y > cross_height) ?widget->len_y :cross_height);
	url_win_line_step = (int)(3 + DEFAULT_FONT_Y_LEN * url_win_text_zoom);
	url_win_text_len_y = 12 * url_win_line_step;
	url_win_full_url_y_len = url_win_sep + 3 * SMALL_FONT_Y_LEN;
	url_win_y_len = url_win_text_start_y + url_win_text_len_y + url_win_sep + url_win_full_url_y_len;
	url_win_help_x = widget->len_x + 2 * url_win_sep;
	url_win_url_y_start = url_win_y_len - url_win_full_url_y_len - url_win_sep/2;
	resize_window (url_win, url_win_x_len, url_win_y_len);
	
	/* create the scroll bar */
	url_scroll_id = vscrollbar_add_extended(url_win, url_scroll_id, NULL, 
		url_win_x_len - scroll_width, url_win_text_start_y, scroll_width,
		url_win_text_len_y, 0, 1.0, 0.77f, 0.57f, 0.39f, 0, 1, have_url_count);
	widget_set_OnDrag(url_win, url_scroll_id, url_win_scroll_drag);
	widget_set_OnClick(url_win, url_scroll_id, url_win_scroll_click);
}

