#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include "console.h"
#include "asc.h"
#include "buddy.h"
#include "cache.h"
#include "chat.h"
#include "consolewin.h"
#include "elconfig.h"
#include "filter.h"
#include "global.h"
#include "hud.h"
#include "ignore.h"
#include "init.h"
#include "interface.h"
#include "knowledge.h"
#include "lights.h"
#include "list.h"
#include "mapwin.h"
#include "misc.h"
#include "multiplayer.h"
#include "notepad.h"
#include "pm_log.h"
#include "platform.h"
#include "sound.h"
#include "spells.h"
#include "tabs.h"
#include "translate.h"
#include "url.h"
#ifdef COUNTERS
#include "counters.h"
#endif
#ifdef MINIMAP
#include "minimap.h"
#endif
#ifdef NEW_FILE_IO
#include "errors.h"
#include "io/elpathwrapper.h"
#endif
#ifdef CALCULATOR
#include "calc.h"
#endif

typedef char name_t[32];

char auto_open_encyclopedia = 1;
/* Pointer to the array holding the commands */
command_t *commands;
/* Array holding names we have seen (for completion) */
name_t *name_list = NULL;
/* Counts how many commands we have */
Uint16 command_count = 0;
Uint16 name_count = 0;
/* The command buffer head pointer */
list_node_t *command_buffer = NULL;
/* Pointer to our position in the buffer */
list_node_t *command_buffer_offset = NULL;
/* The input line before we started moving around in the buffer. */
char first_input[256] = {0};

int time_warn_h, time_warn_s, time_warn_d;

void add_line_to_history(const char *line, int len)
{
	char *copy;

	copy = malloc(len+1);
	safe_snprintf(copy, len+1, "%.*s", len, line);
	list_push(&command_buffer, copy);
	command_buffer_offset = NULL;
}

char *history_get_line_up(void)
{
	if(command_buffer_offset == NULL) {
		/* This is the first up keypress.
		 * Return the first line we have, if there's one and remember what we had. */
		command_buffer_offset = command_buffer;
		safe_snprintf(first_input, sizeof(first_input), "%.*s", input_text_line.len, input_text_line.data);
	} else if(command_buffer_offset->next != NULL) {
		command_buffer_offset = command_buffer_offset->next;
	}
	if(command_buffer_offset != NULL) {
		return command_buffer_offset->data;
	} else {
		return NULL;
	}
}

char *history_get_line_down(void)
{
	if(command_buffer_offset != NULL) {
		command_buffer_offset = command_buffer_offset->prev;
		if(command_buffer_offset != NULL) {
			return command_buffer_offset->data;
		} else {
			/* We're at the bottom of the list, return what we initially had. */
			return first_input;
		}
	}
	return NULL;
}

void history_reset(void)
{
	command_buffer_offset = NULL;
}

void history_destroy(void)
{
	list_destroy(command_buffer);
}

void command_cleanup(void)
{
	if(command_count > 0) {
		free(commands);
	}
	if(name_count > 0) {
		free(name_list);
	}
}

void add_command(const char *command, int (*callback)())
{
	static int commands_size = 0;
	int i;
	/* Check if the command already is in the list. */
	for(i = 0; i < command_count; i++) {
		if(strcasecmp(commands[i].command, command) == 0) {
			/* It exists, just update the callback and return */
			commands[i].callback = callback;
			return;
		}
	}
	if(command_count >= commands_size) {
		if(commands_size == 0) {
			commands_size += 10;
			commands = malloc(commands_size*sizeof(*commands));
		} else {
			commands_size *= 2;
			commands = realloc(commands, commands_size*sizeof(*commands));
		}
	}
	safe_snprintf(commands[command_count].command, sizeof(commands[command_count].command), "%s", command);
	commands[command_count].callback = callback;
	command_count++;
}

void add_name_to_tablist(const char *name)
{
	static int list_size = 0;
	int i;

	for(i = 0; i < name_count; i++) {
		if(strcasecmp(name_list[i], name) == 0) {
			/* Name is already in the list, do nothing */
			return;
		}
	}
	if(name_count >= list_size) {
		if(list_size == 0) {
			list_size += 10;
		}
		list_size *= 2;
		name_list = realloc(name_list, list_size * sizeof(*name_list));
	}
	safe_snprintf(name_list[name_count], sizeof(name_list[name_count]), "%s", name);
	name_count++;
}

/* strmrchr: returns a pointer to the last occurence of c in s,
 * beginning the (reversed) search at begin */
const char *strmrchr(const char *s, const char *begin, int c)
{
	char *copy = strdup(s);
	char *cbegin = copy+(begin-s);
	char *result;

	*cbegin = '\0';
	result = strrchr(copy, c);
	free(copy);

	if(result == NULL) {
		return NULL;
	} else {
		return s+(result-copy);
	}
}

enum compl_type {
	COMMAND = 1,
	NAME,
	NAME_PM,
	CHANNEL
};

struct compl_str {
	const char *str;
	enum compl_type type;
};

struct compl_str tab_complete(const text_message *input, unsigned int cursor_pos)
{
	static char last_complete[48] = {0};
	static int have_last_complete = 0;
	static int last_str_count = -1;
	static enum compl_type last_type = NAME;
	struct compl_str return_value = {NULL, 0};

	if(input != NULL && input->len > 0 )
	{
		const char *input_string = (char*)input->data;
		int count;
		int i;
		short retries;
		node_t *step;

		if(!have_last_complete) {
			if(strchr(input_string, ' ') != NULL) {
				/* If we have a space in the input string, we're pretty certain 
				 * it's not a name, command or channel name. */
				return_value.type = NAME;
			} else if(*input_string == '/' || *input_string == char_slash_str[0]) {
				return_value.type = NAME_PM;
			} else if (input_string[0] == '@' || input_string[0] == char_at_str[0]) {
				return_value.type = CHANNEL;
			} else if(*input_string == '#' || *input_string == char_cmd_str[0]) {
				return_value.type = COMMAND;
			} else {
				return_value.type = NAME;
			}
		} else {
			return_value.type = last_type;
		}
		switch(return_value.type) {
			case CHANNEL:
				input_string++;
				/* No break, increment twice for channel */
			case NAME_PM:
			case COMMAND:
				input_string++;
			break;
			case NAME:
			{
				const char *last_space = strmrchr(input_string, input_string+cursor_pos, ' ');
				if(last_space != NULL) {
					/* Update the cursor position to be relative to last_complete */
					cursor_pos -= last_space+1-input_string;
					input_string = last_space+1;
				}
			}
			break;
		}

		if((*input_string && strncasecmp(input_string, last_complete, strlen(last_complete)) != 0) || !have_last_complete) {
			/* New input string, start over */
			last_str_count = -1;
			if(return_value.type == NAME) {
				/* If it's a name (completed anywhere), isolate the word 
				 * we're currently typing */
				size_t i;
				for(i = 0; i < sizeof(last_complete) && input_string[i] && i < cursor_pos
					&& !isspace((unsigned char)input_string[i]); i++) {
					last_complete[i] = input_string[i];
				}
				last_complete[i] = '\0';
			} else {
				safe_snprintf(last_complete, sizeof(last_complete), "%s", input_string);
			}
			have_last_complete = 1;
		}
		/* Look through the list */
		for(retries = 0; retries < 2 && !return_value.str; retries++) {
			switch(return_value.type) {
				case NAME:
				case NAME_PM:
					for(i = 0, count = 0; i < name_count; i++) {
						if(strncasecmp(name_list[i], last_complete, strlen(last_complete)) == 0) {
							/* We have a match! */
							if(count > last_str_count) {
								/* This hasn't been returned yet, let's return it */
								last_str_count = count++;
								return_value.str = name_list[i];
								break;
							}
							count++;
						}
					}
				break;
				case CHANNEL:
					for(step = queue_front_node(chan_name_queue), count = 0; step->next != NULL; step = step->next) {
						if(strncasecmp(((chan_name*)(step->data))->name, last_complete, strlen(last_complete)) == 0) {
							/* Yay! The chan-name begins with the string we're searching for. */
							if(count > last_str_count) {
								/* We found something we haven't returned earlier, let's return it. */
								last_str_count = count++;
								return_value.str = ((chan_name*)(step->data))->name;
								break;
							}
							count++;
						}
					}
				break;
				case COMMAND:
				default:
					for(i = 0, count = 0; i < command_count; i++) {
						if(strncasecmp(commands[i].command, last_complete, strlen(last_complete)) == 0) {
							/* Yay! The command begins with the string we're searching for. */
							if(count > last_str_count) {
								/* We found something we haven't returned earlier, let's return it. */
								last_str_count = count++;
								return_value.str = commands[i].command;
								break;
							}
							count++;
						}
					}
				break;
			}
			if(!return_value.str && count) {
				/* We checked the whole list and found something, but not
				 * anything we haven't returned earlier. Let's start from the beginning again. */
				last_str_count = -1;
			}
		}
		last_type = return_value.type;
	} else {
		have_last_complete = 0;
		*last_complete = '\0';
		last_str_count = -1;
	}
	return return_value;
}

void do_tab_complete(text_message *input)
{
	text_field *tf = input_widget->widget_info;
	struct compl_str completed = tab_complete(input, tf->cursor);
	if(completed.str != NULL)
	{
		char suffix = '\0';

		/* Append a space if there isn't one already. */
		if(completed.str[strlen(completed.str)-1] != ' ') {
			suffix = ' ';
		}
		switch(completed.type) {
			case CHANNEL:
				safe_snprintf(input->data, input->size, "%c%c%s%c", input->data[0], input->data[1], completed.str, suffix);
				input->len = strlen(input->data);
				tf->cursor = tf->buffer->len+1;
			break;
			case COMMAND:
			case NAME_PM:
				safe_snprintf(input->data, input->size, "%c%s%c", *input->data, completed.str, suffix);
				input->len = strlen(input->data);
				tf->cursor = tf->buffer->len;
			break;
			case NAME:
			{
				const char *last_space = strmrchr(input->data, input->data+tf->cursor, ' ');
				/* Find the length of the data we're removing */
				int len = input->data+tf->cursor-1-(last_space ? last_space : (input->data-1));
				int i;

				/* Erase the current input word */
				for (i = tf->cursor; i <= input->len; i++) {
					input->data[i-len] = input->data[i];
				}
				input->len -= len;
				tf->cursor -= len;
				paste_in_input_field((unsigned char*)completed.str);
				input->len = strlen(input->data);
			}
		}
	}
}

void reset_tab_completer(void)
{
	tab_complete(NULL, 0);
}

int test_for_console_command(char *text, int length)
{
	int i;
	int ptr_length = length;
	char *text_ptr = text;

	// Skip leading #s
	if(*text == '#' || *text == char_cmd_str[0]) {
		*text = '#';
		text++;
		length--;
	}
	//Skip leading spaces
	for(;isspace(*text); text++,length--);

	//Check if there's anything left of the command
	if (length <= 0 || (*text == '@' && length <= 1)) {
		return 0;
	} else {
		int cmd_len;
		/* Look for a matching command */
		for(i = 0; i < command_count; i++) {
			cmd_len = strlen(commands[i].command);
			if(strlen(text) >= cmd_len && my_strncompare(text, commands[i].command, cmd_len) && (isspace(text[cmd_len]) || text[cmd_len] == '\0')) {
				/* Command matched */
				if(commands[i].callback && commands[i].callback(text+cmd_len, length-cmd_len)) {
					/* The command was handled and we don't want to send it to the server */
					return 1;
				} else {
					/* the command wants to be sent to the server */
					break;
				}
			}
		}
	}
	send_input_text_line (text_ptr, ptr_length);
	return 0;
}

// -------- COMMAND CALLBACKS -------- //
// Return 0 if you want the string to be sent to the server.
// the first argument passed is the input string without the command itself
// if ie. '#filter foo' is passed to test_for_console_command(), only ' foo' is passed to the callback.

int command_cls(char *text, int len)
{
	clear_display_text_buffer ();
	return 1;
}

#ifdef CALCULATOR
int command_calc(char *text, int len)
{
	double res;
	char str[100];
	int calcerr;
	
	res = calc_exp(text);
	calcerr = calc_geterror();
	switch (calcerr){
		case CALCERR_OK:
			if (trunc(res)==res) safe_snprintf (str,sizeof(str), "Result: %.0f",res);
			else safe_snprintf (str,sizeof(str), "Result: %.2f",res);
			LOG_TO_CONSOLE (c_orange1, str);
			break;
		case CALCERR_SYNTAX:
			safe_snprintf (str,sizeof(str), "Syntax error");
			LOG_TO_CONSOLE (c_orange1, str);
			break;
		case CALCERR_DIVIDE:
			safe_snprintf (str, sizeof(str),"Divide by zero");
			LOG_TO_CONSOLE (c_orange1, str);
			break;
		case CALCERR_MEM:
			safe_snprintf (str,sizeof(str), "Memory error");
			LOG_TO_CONSOLE (c_orange1, str);
			break;
		case CALCERR_XPSYNTAX:
			safe_snprintf (str,sizeof(str), "Error after L");
			LOG_TO_CONSOLE (c_orange1, str);
			break;
			
	}
	return 1;
}
#endif

int command_markpos(char *text, int len)
{
	int map_x, map_y;
	char *ptr = text;
	char msg[512];
	const char *usage = help_cmd_markpos_str;
	
	while (isspace(*ptr))
		ptr++;
	if (sscanf(ptr, "%d,%d ", &map_x, &map_y) != 2) {
		LOG_TO_CONSOLE(c_red2, usage);
		return 1;
	}
	while (*ptr != ' ' && *ptr)
		ptr++;
	while (*ptr == ' ')
		ptr++;
	if (!*ptr) {
		LOG_TO_CONSOLE(c_red2, usage);
		return 1;
	}
	if (put_mark_on_position(map_x, map_y, ptr)) {
		safe_snprintf (msg, sizeof(msg), location_info_str, map_x, map_y, ptr);
		LOG_TO_CONSOLE(c_orange1,msg);
	} else {
		safe_snprintf (msg,sizeof(msg), invalid_location_str, map_x, map_y);
		LOG_TO_CONSOLE(c_red2,msg);
	}
	return 1;
}

int command_mark(char *text, int len)
{
	if (strlen(text) > 1) //check for empty marks
	{
		char str[512];

		for(;isspace(*text); text++);
		if(strlen(text) > 0) {
			put_mark_on_current_position(text);
			safe_snprintf (str, sizeof(str), marked_str, text);
			LOG_TO_CONSOLE(c_orange1,str);
		}
	}
	return 1;
}

int command_unmark(char *text, int len)
{
	int i;

	while (isspace(*text))
		text++;

	if(*text) {
		for (i = 0; i < max_mark; i ++)
		{
			if (my_strcompare(marks[i].text, text) && (marks[i].x != -1))
			{
				char str[512];
				marks[i].x = marks[i].y = -1;
				save_markings();
				safe_snprintf(str, sizeof(str), unmarked_str, marks[i].text);
				LOG_TO_CONSOLE(c_orange1, str);
				break;
			}
		}
	}
	return 1;
}

int command_stats(char *text, int len)
{
	unsigned char protocol_name;

	protocol_name = SERVER_STATS;
	my_tcp_send(my_socket, &protocol_name, 1);
	return 1;
}

int command_time(char *text, int len)
{
	unsigned char protocol_name;

	protocol_name = GET_TIME;
	my_tcp_send(my_socket,&protocol_name,1);
	return 1;
}

int command_ping(char *text, int len)
{
	Uint8 str[8];

	str[0] = PING;
	*((Uint32 *)(str+1)) = SDL_SwapLE32(SDL_GetTicks());
	my_tcp_send(my_socket, str, 5);
	return 1;
}

int command_date(char *text, int len)
{
	unsigned char protocol_name;

	protocol_name = GET_DATE;
	my_tcp_send(my_socket, &protocol_name, 1);
	return 1;
}

int command_quit(char *text, int len)
{
	exit_now = 1;
	return 1;
}

int command_mem(char *text, int len)
{
	cache_dump_sizes(cache_system);
#ifdef	DEBUG
	cache_dump_sizes(cache_e3d);
#endif	//DEBUG
	return 1;
}
int command_ver(char *text, int len)
{
	char str[250];

	print_version_string(str, sizeof(str));
	LOG_TO_CONSOLE(c_green1, str);
	return 1;
}

int command_ignore(char *text, int len)
{
	char name[16];
	int i;
	Uint8 ch='\0';
	int result;

	while (isspace(*text))
		text++;

	for (i = 0; i <= MAX_USERNAME_LENGTH; i++)
	{
		ch = text[i];
		if (ch == ' ' || ch == '\0')
		{
			ch = '\0';
			break;
		}
		name[i] = ch;
	}
	name[i] = '\0';

	//ttlanhil: Is this even used? The previous for() loop should never make i larger than MAX_USERNAME_LENGTH
	if (i > MAX_USERNAME_LENGTH + 1 && ch != '\0')
	{
		char str[100];
		safe_snprintf (str, sizeof(str), "%s %s", name_too_long, not_added_to_ignores);
		LOG_TO_CONSOLE (c_red1, str);
		return 1;
	}
	if (i < 3)
	{
		char str[100];
		safe_snprintf (str, sizeof(str), "%s %s", name_too_short, not_added_to_ignores);
		LOG_TO_CONSOLE (c_red1, name_too_short);
		return 1;
	}

	result = add_to_ignore_list (name, save_ignores);
	if (result == -1)
	{
		char str[100];
		safe_snprintf (str, sizeof(str), already_ignoring, name);
		LOG_TO_CONSOLE (c_red1, str);
		return 1;
	}
	if(result == -2)
	{
		LOG_TO_CONSOLE (c_red1, ignore_list_full);
	}
	else
	{
		char str[100];
		safe_snprintf (str, sizeof(str), added_to_ignores, name);
		LOG_TO_CONSOLE (c_green1, str);
	}
	return 1;
}

int command_filter(char *text, int len)
{
	char name[256];
	char str[100];
	int i;
	Uint8 ch = '\0';
	int result;

	while (isspace(*text))
		text++;

	for (i = 0; i < sizeof (name) - 1; i++)
	{
		ch = text[i];
		if (ch == '\0')
			break;
		name[i] = ch;
	}
	name[i] = '\0';

	if (i >= sizeof (name) - 1 && ch != '\0')
	{
		safe_snprintf (str, sizeof (str), "%s %s", word_too_long, not_added_to_filter);
		LOG_TO_CONSOLE (c_red1, str);
		return 1;
	}
	else if (i < 3)
	{
		safe_snprintf (str, sizeof (str), "%s %s", word_too_short, not_added_to_filter);
		LOG_TO_CONSOLE (c_red1, word_too_short);
		return 1;
	}

	result = add_to_filter_list (name, 1, save_ignores);
	if (result == -1)
	{
		safe_snprintf (str, sizeof (str), already_filtering, name);
		LOG_TO_CONSOLE (c_red1, str);
	}
	else if (result == -2)
	{
		LOG_TO_CONSOLE (c_red1, filter_list_full);
	}
	else
	{
		safe_snprintf (str, sizeof (str), added_to_filters, name);
		LOG_TO_CONSOLE (c_green1, str);
	}
	return 1;
}

int command_unignore(char *text, int len)
{
	char name[16];
	char str[200];
	int i;
	Uint8 ch = '\0';
	int result;

	while (isspace(*text))
		text++;

	for (i = 0; i <= MAX_USERNAME_LENGTH; i++)
	{
		ch = text[i];
		if (ch == ' ' || ch == '\0')
		{
			ch = '\0';
			break;
		}
		name[i] = ch;
	}
	name[i] = '\0';

	//ttlanhil: Is this even used? The previous for() loop should never make i larger than MAX_USERNAME_LENGTH
	if (i > MAX_USERNAME_LENGTH + 1 && ch != '\0')
	{
		safe_snprintf (str, sizeof (str), "%s %s", name_too_long, not_removed_from_ignores);
		LOG_TO_CONSOLE (c_red1, str);
		return 1;
	}
	if (i < 3)
	{
		safe_snprintf (str, sizeof (str), "%s %s", name_too_short, not_removed_from_filter);
		LOG_TO_CONSOLE (c_red1, str);
		return 1;
	}
	result = remove_from_ignore_list (name);
	if (result == -1)
	{
		safe_snprintf (str, sizeof (str), not_ignoring, name);
		LOG_TO_CONSOLE (c_red1, str);
	}
	else
	{
		safe_snprintf (str, sizeof (str), removed_from_ignores, name);
		LOG_TO_CONSOLE (c_green1, str);
	}
	return 1;
}

int command_unfilter(char *text, int len)
{
	char name[64];
	char str[200];
	int i;
	Uint8 ch = '\0';
	int result;

	while (isspace(*text))
		text++;

	for (i = 0; i < sizeof (name) - 1; i++)
	{
		ch = text[i];
		if (ch == '\0')
			break;
		name[i] = ch;
	}
	name[i] = '\0';

	if (i >= sizeof (name) - 1 && ch != '\0')
	{
		safe_snprintf (str, sizeof (str), "%s %s", word_too_long, not_removed_from_filter);
		LOG_TO_CONSOLE (c_red1, str);
		return 1;
	}
	if (i < 3)
	{
		safe_snprintf (str, sizeof (str), "%s %s", word_too_short, not_removed_from_filter);
		LOG_TO_CONSOLE (c_red1, str);
		return 1;
	}
	result = remove_from_filter_list (name);
	if(result == -1)
	{
		safe_snprintf (str, sizeof (str), not_filtering, name);
		LOG_TO_CONSOLE (c_red1, str);
	}
	else
	{
		safe_snprintf (str, sizeof (str), removed_from_filter, name);
		LOG_TO_CONSOLE (c_green1, str);
	}
	return 1;
}

int command_glinfo (const char *text, int len)
{
	const char *my_string;
	size_t size = 8192, minlen;
	char* this_string = calloc (size, 1);

	my_string = (const char*) glGetString (GL_RENDERER);
	minlen = strlen (video_card_str) + strlen (my_string) + 3;
	if (size < minlen)
	{
		while (size < minlen) size += size;
		this_string = realloc (this_string, size);
	}
	safe_snprintf (this_string, size,"%s: %s",video_card_str, my_string);
	LOG_TO_CONSOLE (c_red2, this_string);

	my_string = (const char*) glGetString (GL_VENDOR);
	minlen = strlen (video_vendor_str) + strlen (my_string) + 3;
	if (size < minlen)
	{
		while (size < minlen) size += size;
		this_string = realloc (this_string, size);
	}
	safe_snprintf (this_string, size,"%s: %s", video_vendor_str, my_string);
	LOG_TO_CONSOLE (c_yellow3, this_string);

	my_string = (const char*) glGetString (GL_VERSION);
	minlen = strlen (opengl_version_str) + strlen (my_string) + 3;
	if (size < minlen)
	{
		while (size < minlen) size += size;
		this_string = realloc (this_string, size);
	}
	safe_snprintf (this_string, size, "%s: %s", opengl_version_str, my_string);
	LOG_TO_CONSOLE (c_yellow2, this_string);

	my_string = (const char*) glGetString (GL_EXTENSIONS);
	minlen = strlen (supported_extensions_str) + strlen (my_string) + 3;
	if (size < minlen)
	{
		while (size < minlen) size += size;
		this_string = realloc (this_string, size);
	}
	safe_snprintf (this_string, size, "%s: %s", supported_extensions_str, my_string);
	LOG_TO_CONSOLE (c_grey1, this_string);

	free (this_string);
	
	return 1;
}


/*  Display book names that match the specified string, or all if
 *  no string specified.  Highlighing the books that have been read.
 */
int knowledge_command(char *text, int len)
{
	char this_string[80], count_str[60];
	char *cr;
	int i, num_read = 0, num_total = 0;

	// this bit of code is repeat enough to justify doing it once somewhere...
	// find first space, then skip any spaces
	while(*text && !isspace(*text))
		text++;
	while(*text && isspace(*text))
		text++;

	LOG_TO_CONSOLE(c_green2,knowledge_cmd_str);
	for (i=0; i<KNOWLEDGE_LIST_SIZE; i++)
	{
		// only display books that contain the specified parameter string
		// shows all books if no string specified
		if ((strlen(knowledge_list[i].name) > 0) &&
			(get_string_occurance(text, knowledge_list[i].name, strlen(knowledge_list[i].name), 1) != -1))
		{
			// remove any trailing carrage return
			safe_strncpy(this_string, knowledge_list[i].name, sizeof(this_string));
			if ( (cr = strchr(this_string, '\n')) != NULL)
				*cr = '\0';
			// highlight books that have been read
			if (knowledge_list[i].present){
				LOG_TO_CONSOLE(c_grey1,this_string);
				++num_read;
			} else {
				LOG_TO_CONSOLE(c_grey2,this_string);
			}
			++num_total;
		}
	}
	safe_snprintf(count_str, sizeof(count_str), book_count_str, num_read, num_total);
	LOG_TO_CONSOLE(c_grey1, count_str);
	return 1;
}


int command_log_conn_data(char *text, int len)
{
	if(!log_conn_data){
		LOG_TO_CONSOLE(c_grey1,logconn_str);
		log_conn_data = 1;
	} else {
		log_conn_data = 0;
	}
	return 1;
}

// TODO: make this automatic or a better command, m is too short
int command_msg(char *text, int len)
{
	int no;//, m=-1;

	// find first space, then skip any spaces
	while(*text && !isspace(*text))
		text++;
	while(*text && isspace(*text))
		text++;
	if(my_strncompare(text, "all", 3)) {
		for(no = 0; no < pm_log.ppl; no++) {
			print_message(no);
		}
	} else {
		no = atoi(text) - 1;
		if(no < pm_log.ppl && no >= 0) {
			print_message(no);
		}
	}
	return 1;
}

int command_afk(char *text, int len)
{
	// find first space, then skip any spaces
	while(*text && !isspace(*text)) {
		text++;
		len--;
	}
	while(*text && isspace(*text)) {
		text++;
		len--;
	}
	if(!afk)
	{
		if (len > 0) 
		{
			safe_snprintf(afk_message, sizeof(afk_message), "%.*s", len, text);
		}
		go_afk();
		last_action_time = cur_time-afk_time-1;
	} else {
		go_ifk();
	}
	return 1;
}
	
int command_help(char *text, int len)
{
	// help can open the Enc!
	if(auto_open_encyclopedia)
	{
		view_tab (&tab_help_win, &tab_help_collection_id, HELP_TAB_HELP);
	}
	// but fall thru and send it to the server
	return 0;
}

int command_storage(char *text, int len)
{
	int i;

	storage_filter[0] = '\0';

	for (i = 0; i < len; i++) {
		if (text[i] == ' ') {
			break;
		}
	}

	if (i < len)
	{
		int nb = len - i - 1;
		if (nb > sizeof (storage_filter) - 1)
			nb = sizeof (storage_filter) - 1;
		my_strncp (storage_filter, text+i+1, nb+1);
	}

	if (have_storage_list)
	{
		int size = strlen((char*)cached_storage_list)+1;
		unsigned char cached_storage_copy[sizeof(cached_storage_list)];
		unsigned char * endl;
		memcpy(cached_storage_copy, cached_storage_list, size);
		endl = (unsigned char*)strchr((char*)cached_storage_copy, '\n');
		if (endl == NULL)
		{
			// No newline? Our cached list isn't correct.
			return 0;
		}
		if (storage_filter[0] != '\0')
		{
			size = filter_storage_text((char*)endl+1, size, size);  //Note: filter from the first newline, which is where the item list starts
			size += (endl - cached_storage_copy +1);
		}
		put_text_in_buffer(CHAT_SERVER, cached_storage_copy, size);
		return 1;
	}
	return 0;
}

int command_accept_buddy(char *text, int len)
{
	/* This command is here to make sure the requests queue is up to date */
	while(*text && !isspace(*text))
		text++;
	while(*text && isspace(*text))
		text++;
	/* Make sure a name is given */
	if(*text && !queue_isempty(buddy_request_queue)) {
		node_t *node = queue_front_node(buddy_request_queue);

		/* Search for the node in the queue */
		while(node != NULL) {
			if(strcasecmp(text, node->data) == 0) {
				/* This is the node we're looking for, delete it */
				queue_delete_node(buddy_request_queue, node);
				break;
			}
			node = node->next;
		}
	}
	return 0;
}

int save_local_data(char * text, int len){
	save_bin_cfg();
	//Save the quickbar spells
	save_quickspells();
	// save el.ini if asked
	if (write_ini_on_exit) write_el_ini ();
#ifdef NOTEPAD
	// save notepad contents if the file was loaded
	if (notepad_loaded) notepad_save_file (NULL, 0, 0, 0);
#endif //NOTEPAD
#ifdef MINIMAP
	save_exploration_map();
#endif //MINIMAP
#ifdef COUNTERS
	flush_counters();
#endif //COUNTERS
	return 0;
}

void init_commands(const char *filename)
{
#ifndef NEW_FILE_IO
	FILE *fp = my_fopen(filename, "r");
	if(fp != NULL) {
#else /* NEW_FILE_IO */
	FILE *fp = open_file_data(filename, "r");
	if(fp == NULL) {
		LOG_ERROR("%s: %s \"%s\"\n", reg_error_str, cant_open_file, filename);
	} else {
#endif /* NEW_FILE_IO */
	/* Read keywords from commands.lst */
		char buffer[255];
		size_t buffer_len;
		char *ptr;

		while(fgets(buffer, sizeof(buffer), fp)) {
			/* Get rid of comments */
			if((ptr = strchr(buffer, '#')) != NULL) {
				*ptr = '\0';
			}
			buffer_len = strlen(buffer);
			/* Skip empty lines */
			if(strcmp(buffer, "\r\n") != 0 && strcmp(buffer, "\n") != 0
				&& strlen(buffer) > 0) {
				/* Get rid of the newline. */
				if(buffer[buffer_len-2] == '\r' && buffer[buffer_len-1] == '\n') {
					buffer[buffer_len-2] = '\0';
				} else if(buffer[buffer_len-1] == '\n') {
					buffer[buffer_len-1] = '\0';
				}
				add_command(buffer, NULL);
			}
		}
		fclose(fp);
	}
#ifdef CALCULATOR
	add_command("calc", &command_calc);
#endif
	add_command("cls", &command_cls);
	add_command(cmd_markpos, &command_markpos);
	add_command(cmd_mark, &command_mark);
	add_command(cmd_unmark, &command_unmark);
	add_command(cmd_stats, &command_stats);
	add_command("ping", &command_ping);
	add_command(cmd_time, &command_time);
	add_command(cmd_date, &command_date);
	add_command("quit", &command_quit);
	add_command("exit", &command_quit);
	add_command(cmd_exit, &command_quit);
	add_command("mem", &command_mem);
	add_command("cache", &command_mem);
	add_command("ver", &command_ver);
	add_command("vers", &command_ver);
	add_command(cmd_ignores, &list_ignores);
	add_command(cmd_ignore, &command_ignore);
	add_command(cmd_filters, &list_filters);
	add_command(cmd_filter, &command_filter);
	add_command(cmd_unignore, &command_unignore);
	add_command(cmd_unfilter, &command_unfilter);
	add_command(cmd_glinfo, &command_glinfo);
	add_command(cmd_knowledge_short, &knowledge_command);
	add_command(cmd_knowledge, &knowledge_command);
	add_command("log conn data", &command_log_conn_data);
	add_command(cmd_msg, &command_msg);
	add_command(cmd_afk, &command_afk);
	add_command("jc", &command_jlc);//since we only mess with the part after the
	add_command("lc", &command_jlc);//command, one function can do both
	add_command(help_cmd_str, &command_help);
	add_command("sto", &command_storage);
	add_command("storage", &command_storage);
	add_command("accept_buddy", &command_accept_buddy);
	add_command("current_song", &display_song_name);
	add_command("find", &history_grep);
	add_command("save", &save_local_data);
	add_command("url", &url_command);
#ifdef COUNTERS	
	add_command("chat_to_counters", &chat_to_counters_command);
#endif
	command_buffer_offset = NULL;
}


// NOTE: Len = length of the buffer, not the string (Verified)

void print_version_string (char *buf, size_t len)
{
	char extra[100];
	
	if (client_version_patch > 0)
	{
		safe_snprintf (extra, sizeof(extra), "p%d Beta %s", client_version_patch, DEF_INFO);
	}
	else
	{
		safe_snprintf (extra, sizeof(extra), " Beta %s", DEF_INFO);
	}
	safe_snprintf (buf, len, game_version_str, client_version_major, client_version_minor, client_version_release, extra);
}


void new_minute_console(void){
	if(!(game_minute%60)){
		timestamp_chat_log();
	}
	if(time_warn_h >= 0 && (time_warn_h+game_minute)%60 == 0){
		char str[75];
		safe_snprintf(str, sizeof(str), time_warn_hour_str, time_warn_h);
		LOG_TO_CONSOLE(c_purple1, str);
	}
	if(time_warn_s >= 0 && (time_warn_s+game_minute)%180 == 30){
		char str[100];
		if (time_warn_s+game_minute == 30) { // sunrise
			safe_snprintf(str, sizeof(str), time_warn_sunrise_str, time_warn_s);
		}
		else { // sunset
			safe_snprintf(str, sizeof(str), time_warn_sunset_str, time_warn_s);
		}
		LOG_TO_CONSOLE(c_purple1, str);
	}
	if(time_warn_d >= 0 && (time_warn_d+game_minute)%360 == 0){
		char str[75];
		safe_snprintf(str, sizeof(str), time_warn_day_str, time_warn_d);
		LOG_TO_CONSOLE(c_purple1, str);
	}
}
