#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include "global.h"
#include "list.h"
#include "queue.h"

/* NOTE: This file contains implementations of the following, currently unused, and commented functions:
 *          Look at the end of the file.
 *
 * void print_log();
 * void cls();
 */

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

void add_line_to_history(const char *line, int len)
{
	char *copy;

	copy = malloc(len+1);
	snprintf(copy, len+1, "%.*s", len, line);
	list_push(&command_buffer, copy);
	command_buffer_offset = NULL;
}

char *history_get_line_up(void)
{
	if(command_buffer_offset == NULL) {
		/* This is the first up keypress.
		 * Return the first line we have, if there's one and remember what we had. */
		command_buffer_offset = command_buffer;
		snprintf(first_input, sizeof(first_input), "%.*s", input_text_line.len, input_text_line.data);
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
	snprintf(commands[command_count].command, sizeof(commands[command_count].command), "%s", command);
	commands[command_count].callback = callback;
	command_count++;
}

void add_name_to_tablist(const unsigned char *name)
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
	snprintf(name_list[name_count], sizeof(name_list[name_count]), "%s", name);
	name_count++;
}

#define COMMAND 1
#define NAME 2
#define CHANNEL 3

const char *tab_complete(const text_message *input)
{
	static char last_complete[48] = {0};
	static int have_last_complete = 0;
	static char last_count = -1;
	const char *return_value = NULL;

	if(input != NULL && input->len > 0 &&
		(*input->data == '#' || *input->data == *char_cmd_str ||
		*input->data == '/' || *input->data == *char_slash_str ||
		(input->len > 1 && (input->data[0] == '@' || input->data[0] == *char_at_str) &&
		(input->data[1] == '@' || input->data[1] == *char_at_str))))
	{
		const char *input_string = input->data;
		Uint8 type;
		int count;
		int i;
		short retries;
		node_t *step;

		if(*input_string == '/' || *input_string == char_slash_str[0]) {
			input_string++;
			type = NAME;
		} else if (input_string[0] == '@' || input_string[0] == char_at_str[0]) {
			input_string+=2;
			type = CHANNEL;
		} else {
			type = COMMAND;
			if(*input_string == '#' || *input_string == char_cmd_str[0]) {
				input_string++;
			}
		}
		if((*input_string && strncasecmp(input_string, last_complete, strlen(last_complete)) != 0) || !have_last_complete) {
			/* New input string, start over */
			last_count = -1;
			snprintf(last_complete, sizeof(last_complete), "%s", input_string);
			have_last_complete = 1;
		}
		/* Look through the list */
		for(retries = 0; retries < 2 && !return_value; retries++) {
			switch(type) {
				case NAME:
					for(i = 0, count = 0; i < name_count; i++) {
						if(strncasecmp(name_list[i], last_complete, strlen(last_complete)) == 0) {
							/* We have a match! */
							if(count > last_count) {
								/* This hasn't been returned yet, let's return it */
								last_count = count++;
								return_value = name_list[i];
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
							if(count > last_count) {
								/* We found something we haven't returned earlier, let's return it. */
								last_count = count++;
								return_value = ((chan_name*)(step->data))->name;
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
							if(count > last_count) {
								/* We found something we haven't returned earlier, let's return it. */
								last_count = count++;
								return_value = commands[i].command;
								break;
							}
							count++;
						}
					}
				break;
			}
			if(!return_value && count) {
				/* We checked the whole list and found something, but not
				 * anything we haven't returned earlier. Let's start from the beginning again. */
				last_count = -1;
			}
		}
	} else {
		have_last_complete = 0;
		*last_complete = '\0';
		last_count = -1;
	}
	return return_value;
}

void do_tab_complete(text_message *input)
{
	const char *completed_str = tab_complete(input);
	if(completed_str != NULL)
	{
		char suffix = '\0';

		/* Append a space if there isn't one already. */
		if(completed_str[strlen(completed_str)-1] != ' ')
		{
			suffix = ' ';
		}
		if (input->data[0] == '@' || input->data[0] == char_at_str[0]) {
			snprintf(input->data, input->size, "%c%c%s%c", input->data[0], input->data[1], completed_str, suffix);
		} else {
			snprintf(input->data, input->size, "%c%s%c", *input->data, completed_str, suffix);
		}
		input->len = strlen(input->data);
		if(input_widget && input_widget->widget_info) {
			text_field *tf = input_widget->widget_info;
			tf->cursor = tf->buffer->len;
		}
	}
}

void reset_tab_completer(void)
{
	tab_complete(NULL);
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
		snprintf (msg, sizeof(msg), location_info_str, map_x, map_y, ptr);
		LOG_TO_CONSOLE(c_orange1,msg);
	} else {
		snprintf (msg,sizeof(msg), invalid_location_str, map_x, map_y);
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
			snprintf (str, sizeof(str), marked_str, text);
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
			if (my_strcompare(marks[i].text, text))
			{
				char str[512];
				marks[i].x = marks[i].y = -1;
				save_markings();
				snprintf(str, sizeof(str), unmarked_str, marks[i].text);
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
	Uint8 name[16];
	int i;
	Uint8 ch='\0';
	int result;

	while (isspace(*text))
		text++;

	for (i = 0; i < 15; i++)
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

	if (i > 15 && ch != '\0')
	{
		Uint8 str[100];
		snprintf (str, sizeof(str), "%s %s", name_too_long, not_added_to_ignores);
		LOG_TO_CONSOLE (c_red1, str);
		return 1;
	}
	if (i < 3)
	{
		Uint8 str[100];
		snprintf (str, sizeof(str), "%s %s", name_too_short, not_added_to_ignores);
		LOG_TO_CONSOLE (c_red1, name_too_short);
		return 1;
	}

	result = add_to_ignore_list (name, save_ignores);
	if (result == -1)
	{
		Uint8 str[100];
		snprintf (str, sizeof(str), already_ignoring, name);
		LOG_TO_CONSOLE (c_red1, str);
		return 1;
	}
	if(result == -2)
	{
		LOG_TO_CONSOLE (c_red1, ignore_list_full);
	}
	else
	{
		Uint8 str[100];
		snprintf (str, sizeof(str), added_to_ignores, name);
		LOG_TO_CONSOLE (c_green1, str);
	}
	return 1;
}

int command_filter(char *text, int len)
{
	Uint8 name[256];
	Uint8 str[100];
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
		snprintf (str, sizeof (str), "%s %s", word_too_long, not_added_to_filter);
		LOG_TO_CONSOLE (c_red1, str);
		return 1;
	}
	else if (i < 3)
	{
		snprintf (str, sizeof (str), "%s %s", word_too_short, not_added_to_filter);
		LOG_TO_CONSOLE (c_red1, word_too_short);
		return 1;
	}

	result = add_to_filter_list (name, 1, save_ignores);
	if (result == -1)
	{
		snprintf (str, sizeof (str), already_filtering, name);
		LOG_TO_CONSOLE (c_red1, str);
	}
	else if (result == -2)
	{
		LOG_TO_CONSOLE (c_red1, filter_list_full);
	}
	else
	{
		snprintf (str, sizeof (str), added_to_filters, name);
		LOG_TO_CONSOLE (c_green1, str);
	}
	return 1;
}

int command_unignore(char *text, int len)
{
	Uint8 name[16];
	Uint8 str[200];
	int i;
	Uint8 ch = '\0';
	int result;

	while (isspace(*text))
		text++;

	for (i = 0; i < 15; i++)
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

	if (i>15 && ch != '\0')
	{
		snprintf (str, sizeof (str), "%s %s", name_too_long, not_removed_from_ignores);
		LOG_TO_CONSOLE (c_red1, str);
		return 1;
	}
	if (i < 3)
	{
		snprintf (str, sizeof (str), "%s %s", name_too_short, not_removed_from_filter);
		LOG_TO_CONSOLE (c_red1, str);
		return 1;
	}
	result = remove_from_ignore_list (name);
	if (result == -1)
	{
		snprintf (str, sizeof (str), not_ignoring, name);
		LOG_TO_CONSOLE (c_red1, str);
	}
	else
	{
		snprintf (str, sizeof (str), removed_from_ignores, name);
		LOG_TO_CONSOLE (c_green1, str);
	}
	return 1;
}

int command_unfilter(char *text, int len)
{
	Uint8 name[64];
	Uint8 str[200];
	int i;
	Uint8 ch = '\0';
	int result;

	while (isspace(*text))
		text++;

	for (i = 0; i < sizeof (name) - 1; i++)
	{
		ch = text[i];
		if (ch==' ' || ch=='\0')
		{
			ch = '\0';
			break;
		}
		name[i]=ch;
	}
	name[i] = '\0';

	if (i >= sizeof (name) - 1 && ch != '\0')
	{
		snprintf (str, sizeof (str), "%s %s", word_too_long, not_removed_from_filter);
		LOG_TO_CONSOLE (c_red1, str);
		return 1;
	}
	if (i < 3)
	{
		snprintf (str, sizeof (str), "%s %s", word_too_short, not_removed_from_filter);
		LOG_TO_CONSOLE (c_red1, str);
		return 1;
	}
	result = remove_from_filter_list (name);
	if(result == -1)
	{
		snprintf (str, sizeof (str), not_filtering, name);
		LOG_TO_CONSOLE (c_red1, str);
	}
	else
	{
		snprintf (str, sizeof (str), removed_from_filter, name);
		LOG_TO_CONSOLE (c_green1, str);
	}
	return 1;
}

int command_glinfo(char *text, int len)
{
	GLubyte *my_string;
	Uint8 this_string[8192];

	my_string = (GLubyte *)glGetString(GL_RENDERER);
	snprintf(this_string,sizeof(this_string),"%s: %s",video_card_str,my_string);
	LOG_TO_CONSOLE(c_red2,this_string);

	my_string = (GLubyte *)glGetString(GL_VENDOR);
	snprintf(this_string,sizeof(this_string),"%s: %s",video_vendor_str,my_string);
	LOG_TO_CONSOLE(c_yellow3,this_string);

	my_string = (GLubyte *)glGetString(GL_VERSION);
	snprintf(this_string,sizeof(this_string),"%s: %s",opengl_version_str,my_string);
	LOG_TO_CONSOLE(c_yellow2,this_string);

	my_string = (GLubyte *)glGetString(GL_EXTENSIONS);
	snprintf(this_string,sizeof(this_string),"%s: %s",supported_extensions_str,my_string);
	LOG_TO_CONSOLE(c_grey1,this_string);

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
			snprintf(afk_message, sizeof(afk_message), "%.*s", len, text);
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
	if (notepad_loaded) notepadSaveFile (NULL, 0, 0, 0);
#endif
#ifdef COUNTERS
	flush_counters();
#endif
	return 0;
}

void init_commands(const char *filename)
{
	FILE *fp = my_fopen(filename, "r");
	/* Read keywords from commands.lst */
	if(fp) {
		char buffer[255];
		char *ptr;

		while(fgets(buffer, sizeof(buffer), fp)) {
			/* Get rid of comments */
			if((ptr = strchr(buffer, '#')) != NULL) {
				*ptr = '\0';
			}
			/* Skip empty lines */
			if(strlen(buffer) > 1) {
				/* Get rid of the \n. */
				if(buffer[strlen(buffer)-1] == '\n') {
					buffer[strlen(buffer)-1] = '\0';
				}
				add_command(buffer, NULL);
			}
		}
		fclose(fp);
	}
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
	command_buffer_offset = NULL;
}

void print_version_string (char *buf, size_t len)
{
	char extra[100];
	
	if (client_version_patch > 0)
	{
		snprintf (extra, sizeof(extra), "p%d Beta %s", client_version_patch, DEF_INFO);
	}
	else
	{
		snprintf (extra, sizeof(extra), " Beta %s", DEF_INFO);
	}
	snprintf (buf, len, game_version_str, client_version_major, client_version_minor, client_version_release, extra);
}

/* Currently UNUSED
//cls - clears the text buffer
void cls()
{
	int i;

	display_console_text_buffer_first=0;
	display_text_buffer_first=0;
	display_text_buffer_last=0;

	//clear the buffer
	for(i=0;i<MAX_DISPLAY_TEXT_BUFFER_LENGTH;i++)display_text_buffer[i]=0;
	not_from_the_end_console=0;

	//also update the lines to show, and the last server message thing
	//without it, the text would dissapear very slowly...
	lines_to_show=0;
	last_server_message_time=cur_time;
}

void print_log()
{
	FILE *f = NULL;

  	f = my_fopen ("text_log.txt", "ab");
	if (!f) return;
	
  	fwrite (display_text_buffer, display_text_buffer_last, 1, f);
  	fclose (f);
}
*/
