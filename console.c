#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "asc.h"
#include "buddy.h"
#include "cache.h"
#include "cal.h"
#include "chat.h"
#include "consolewin.h"
#include "elconfig.h"
#include "filter.h"
#include "gamewin.h"
#include "gl_init.h"
#include "hud.h"
#include "hud_quickspells_window.h"
#include "ignore.h"
#include "icon_window.h"
#include "init.h"
#include "item_lists.h"
#include "interface.h"
#include "knowledge.h"
#include "list.h"
#include "mapwin.h"
#include "manufacture.h"
#include "misc.h"
#include "multiplayer.h"
#include "new_actors.h"
#include "notepad.h"
#include "password_manager.h"
#include "pm_log.h"
#include "platform.h"
#include "questlog.h"
#include "sound.h"
#include "spells.h"
#include "stats.h"
#include "tabs.h"
#include "translate.h"
#include "url.h"
#include "command_queue.h"
#include "counters.h"
#include "map.h"
#include "minimap.h"
#include "errors.h"
#include "io/elpathwrapper.h"
#include "io/elfilewrapper.h"
#include "calc.h"
#include "text_aliases.h"
//only for debugging command #add_emote <actor name> <emote id>, can be removed later
#include "actor_scripts.h"
#include "emotes.h"
#ifdef	CUSTOM_UPDATE
#include "custom_update.h"
#endif	/* CUSTOM_UPDATE */

typedef char name_t[32];

#define MAX_COMMAND_NAME_LEN 64
typedef struct {
	char command[MAX_COMMAND_NAME_LEN];
	int (*callback)();
} command_t;

char auto_open_encyclopedia = 1;
int time_warn_h = -1, time_warn_s = -1, time_warn_d = -1;

/* Pointer to the array holding the commands */
static command_t *commands;
/* Array holding names we have seen (for completion) */
static name_t *name_list = NULL;
/* Counts how many commands we have */
static Uint16 command_count = 0;
static Uint16 name_count = 0;
/* The command buffer head pointer */
static list_node_t *command_buffer = NULL;
/* Pointer to our position in the buffer */
static list_node_t *command_buffer_offset = NULL;
/* The input line before we started moving around in the buffer. */
static char first_input[256] = {0};


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


static void cleanup_commands_help(void);

void command_cleanup(void)
{
	if(command_count > 0) {
		free(commands);
	}
	if(name_count > 0) {
		free(name_list);
	}
	cleanup_commands_help();
}


static void add_command(const char *command, int (*callback)())
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
static const char *strmrchr(const char *s, const char *begin, int c)
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

static struct compl_str tab_complete(const text_message *input, unsigned int cursor_pos)
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
		short retries;

		if(!have_last_complete) {
			if(cursor_pos > 0 &&
				strmrchr(input_string, input_string+cursor_pos-1, ' ') != NULL) {
				/* If we have a space in the input string, we're pretty certain
				 * it's not a PM-name, command or channel name. */
				return_value.type = NAME;
			} else if(*input_string == '/' || *input_string == *char_slash_str) {
				return_value.type = NAME_PM;
			} else if (*input_string == '@' || *input_string == *char_at_str) {
				return_value.type = CHANNEL;
			} else if(*input_string == '#' || *input_string == *char_cmd_str) {
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
				// Fallthrough
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

		if(!have_last_complete || (*input_string && strncasecmp(input_string, last_complete, strlen(last_complete)) != 0)) {
			/* New input string, start over */
			size_t i;

			last_str_count = -1;

			/* Isolate the word we're currently typing (and completing) */
			for(i = 0;
				i+1 < sizeof(last_complete) &&
				input_string[i] && i < cursor_pos &&
				!isspace((unsigned char)input_string[i]);
				i++) {
				last_complete[i] = input_string[i];
			}
			last_complete[i] = '\0';
			have_last_complete = 1;
		}
		/* Look through the list */
		for(retries = 0; retries < 2 && !return_value.str; retries++) {
			size_t i;

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
					for(set_first_tab_channel(), count = 0; get_tab_channel_name() != NULL; set_next_tab_channel()) {
						if(strncasecmp(get_tab_channel_name(), last_complete, strlen(last_complete)) == 0) {
							/* Yay! The chan-name begins with the string we're searching for. */
							if(count > last_str_count) {
								/* We found something we haven't returned earlier, let's return it. */
								last_str_count = count++;
								return_value.str = get_tab_channel_name();
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
	int input_cursor = get_console_input_cursor();
	struct compl_str completed = tab_complete(input, input_cursor);

	if(completed.str != NULL)
	{
		size_t len;
		size_t i;

		/* Find the length of the data we're removing */
		if(completed.type == NAME) {
			const char *last_space = strmrchr(input->data, input->data+input_cursor, ' ');
			/* Name is a bit special because it can be anywhere in a string,
			 * not just at the beginning like the other types. */
			len = input->data+input_cursor-1-(last_space ? last_space : (input->data-1));
		} else if (completed.type == CHANNEL) {
			len = input_cursor-2;
		} else {
			len = input_cursor-1;
		}

		/* Erase the current input word */
		for (i = input_cursor; i <= input->len; i++) {
			input->data[i-len] = input->data[i];
		}
		input->len -= len;
		set_console_input_cursor(input_cursor - len);
		paste_in_input_field((unsigned char*)completed.str);
		input->len = strlen(input->data);
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
		/* Handle numeric shortcuts */
		if ( isdigit(text[0]) ) {
			if ( process_text_alias(text,length) >= 0 ) {
				return 1;
			}
		}
		/* Look for a matching command */
		for(i = 0; i < command_count; i++) {
			cmd_len = strlen(commands[i].command);
			if(strlen(text) >= cmd_len && !strncasecmp(text, commands[i].command, cmd_len) && (isspace(text[cmd_len]) || text[cmd_len] == '\0')) {
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


/* given the command text, return the start of any parameter text */
/* e.g. find first space, then skip any spaces */
static char *getparams(char *text)
{
	while(*text && !isspace(*text))
		text++;
	while(*text && isspace(*text))
		text++;
	return text;
}


static int print_emotes(char *text, int len){

	hash_entry *he;;
	LOG_TO_CONSOLE(c_orange1,"EMOTES");
	LOG_TO_CONSOLE(c_orange1,"--------------------");
	hash_start_iterator(emote_cmds);
	while((he=hash_get_next(emote_cmds)))
		LOG_TO_CONSOLE(c_orange1,((emote_dict *)he->item)->command);
	return 1;
}


#ifdef EMOTES_DEBUG
int add_emote(char *text, int len){

	int j;
	char *id;
	actor *act=NULL;

	for(j=1;j<len;j++) if(text[j]==' ') {text[j]=0; break;}
	id=&text[j+1];
	text++;
	printf("Actor [%s] [%s]\n",text,id);
	LOCK_ACTORS_LISTS();
	for (j = 0; j < max_actors; j++){
		if (!strncasecmp(actors_list[j]->actor_name, text, strlen(text)) &&
	  	   (actors_list[j]->actor_name[strlen(text)] == ' ' ||
	    	   actors_list[j]->actor_name[strlen(text)] == '\0')){
			act = actors_list[j];
			LOG_TO_CONSOLE(c_orange1, "actor found, adding emote");
			printf("actor found\n");
			add_emote_to_actor(act->actor_id,atoi(id));
			printf("message added %s\n",id);
		}
	}
	if (!act){
		UNLOCK_ACTORS_LISTS();
		LOG_TO_CONSOLE(c_orange1, "actor not found");
		return 1;
	}
	UNLOCK_ACTORS_LISTS();
	*(id-1)=' ';
	return 1;

}


static int send_cmd(char *text, int len){

	int j,x;
	char *id;
	actor *act=NULL;

	for(j=1;j<len;j++) if(text[j]==' ') {text[j]=0; break;}
	id=&text[j+1];
	x=j;
	text++;
	printf("Actor [%s] [%s]\n",text,id);
	LOCK_ACTORS_LISTS();
	for (j = 0; j < max_actors; j++){
		if (!strncasecmp(actors_list[j]->actor_name, text, strlen(text)) &&
	  	   (actors_list[j]->actor_name[strlen(text)] == ' ' ||
	    	   actors_list[j]->actor_name[strlen(text)] == '\0')){
			act = actors_list[j];
			LOG_TO_CONSOLE(c_orange1, "actor found, adding command");
			printf("actor found\n");
			while(*id){
			add_command_to_actor(act->actor_id,atoi(id));
			id++;
				while(*id!=' '&&*id!=0) id++;
			}
			printf("command added %s\n",id);
		}
	}
	if (!act){
		UNLOCK_ACTORS_LISTS();
		LOG_TO_CONSOLE(c_orange1, "actor not found");
		return 1;
	}
	UNLOCK_ACTORS_LISTS();
	text[x-1]=' ';
	return 1;
}


static int set_idle(char *text, int len){

	int j,x;
	char *id;
	actor *act=NULL;

	for(j=1;j<len;j++) if(text[j]==' ') {text[j]=0; break;}
	id=&text[j+1];
	x=j;
	text++;
	printf("Actor [%s] [%s]\n",text,id);
	LOCK_ACTORS_LISTS();
	for (j = 0; j < max_actors; j++){
		if (!strncasecmp(actors_list[j]->actor_name, text, strlen(text)) &&
	  	   (actors_list[j]->actor_name[strlen(text)] == ' ' ||
	    	   actors_list[j]->actor_name[strlen(text)] == '\0')){
				struct CalMixer *mixer;
			act = actors_list[j];
				mixer=CalModel_GetMixer(act->calmodel);
			LOG_TO_CONSOLE(c_orange1, "actor found, adding anims");
			printf("actor found\n");
			CalMixer_ClearCycle(mixer,act->cur_anim.anim_index, 0.0f);
			while(*id){
				int anim_id;
				double anim_wg;

				anim_id=atoi(id);
			id++;
				while(*id!=' '&&*id!=0) id++;
				anim_wg=atof(id);
			id++;
				while(*id!=' '&&*id!=0) id++;
				printf("setting anim %i with weight %f\n",anim_id,anim_wg);
				if(anim_wg<0) CalMixer_ClearCycle(mixer,actors_defs[act->actor_type].cal_frames[anim_id].anim_index, 0.0f);
				else CalMixer_BlendCycle(mixer,actors_defs[act->actor_type].cal_frames[anim_id].anim_index,anim_wg, 0.1f);
			}
			printf("command added %s\n",id);
		}
	}
	if (!act){
		UNLOCK_ACTORS_LISTS();
		LOG_TO_CONSOLE(c_orange1, "actor not found");
		return 1;
	}
	UNLOCK_ACTORS_LISTS();
	text[x-1]=' ';
	return 1;
}


static int set_action(char *text, int len){

	int j,x;
	char *id;
	actor *act=NULL;

	for(j=1;j<len;j++) if(text[j]==' ') {text[j]=0; break;}
	id=&text[j+1];
	x=j;
	text++;
	printf("Actor [%s] [%s]\n",text,id);
	LOCK_ACTORS_LISTS();
	for (j = 0; j < max_actors; j++){
		if (!strncasecmp(actors_list[j]->actor_name, text, strlen(text)) &&
	  	   (actors_list[j]->actor_name[strlen(text)] == ' ' ||
	    	   actors_list[j]->actor_name[strlen(text)] == '\0')){
				struct CalMixer *mixer;
			act = actors_list[j];
				mixer=CalModel_GetMixer(act->calmodel);
			LOG_TO_CONSOLE(c_orange1, "actor found, adding anims");
			printf("actor found\n");
			while(*id){
				int anim_id;
				double anim_wg;

				anim_id=atoi(id);
			id++;
				while(*id!=' '&&*id!=0) id++;
				anim_wg=atof(id);
			id++;
				while(*id!=' '&&*id!=0) id++;
				printf("setting action %i with weight %f\n",anim_id,anim_wg);
				if(anim_wg<0) CalMixer_RemoveAction(mixer,actors_defs[act->actor_type].cal_frames[anim_id].anim_index);
				else CalMixer_ExecuteActionExt(mixer,actors_defs[act->actor_type].cal_frames[anim_id].anim_index,0.0f,0.0f,anim_wg, 1);
			}
			printf("command added %s\n",id);
		}
	}
	if (!act){
		UNLOCK_ACTORS_LISTS();
		LOG_TO_CONSOLE(c_orange1, "actor not found");
		return 1;
	}
	UNLOCK_ACTORS_LISTS();
	text[x-1]=' ';
	return 1;
}
#endif


#ifdef MORE_ATTACHED_ACTORS_DEBUG
static int horse_cmd(char* text, int len){

	int j,x;
	char *id;
	actor *act=NULL;

	for(j=1;j<len;j++) if(text[j]==' ') {text[j]=0; break;}
	id=&text[j+1];
	x=j;
	text++;
	printf("Actor [%s] [%s] [%i]\n",text,id,atoi(id));
	LOCK_ACTORS_LISTS();
	for (j = 0; j < max_actors; j++){
		if (!strncasecmp(actors_list[j]->actor_name, text, strlen(text)) &&
	  	   (actors_list[j]->actor_name[strlen(text)] == ' ' ||
	    	   actors_list[j]->actor_name[strlen(text)] == '\0')){
			act = actors_list[j];
			LOG_TO_CONSOLE(c_orange1, "actor found, adding horse");
		}
	}
	text[x-1]=' ';

	if (!act){
		UNLOCK_ACTORS_LISTS();
		LOG_TO_CONSOLE(c_orange1,"Actor doesn't exist");
		return 1;		// Eek! We don't have an actor match... o.O
	}

	act->sit_idle=act->stand_idle=0;

	if(act->attached_actor>=0){
		//remove horse
		remove_actor_attachment(act->actor_id);
		LOG_TO_CONSOLE(c_orange1,"De-horsified");

	} else {
		//add horse
		int hh=atoi(id);
		if (hh<=0) hh=200;
		add_actor_attachment(act->actor_id, hh);
		LOG_TO_CONSOLE(c_orange1,"Horsified");
	}
	UNLOCK_ACTORS_LISTS();

	return 1;

}
#endif


#ifdef NECK_ITEMS_DEBUG
static int set_neck(char *text, int len){

	int j;
	char *id;
	actor *act=NULL;

	for(j=1;j<len;j++) if(text[j]==' ') {text[j]=0; break;}
	id=&text[j+1];
	text++;
	printf("Actor [%s] [%s]\n",text,id);
	LOCK_ACTORS_LISTS();
	for (j = 0; j < max_actors; j++){
		if (!strncasecmp(actors_list[j]->actor_name, text, strlen(text)) &&
	  	   (actors_list[j]->actor_name[strlen(text)] == ' ' ||
	    	   actors_list[j]->actor_name[strlen(text)] == '\0')){
			act = actors_list[j];
			LOG_TO_CONSOLE(c_orange1, "actor found, adding neck item");
			printf("actor found\n");
			if(atoi(id)) {
				//wear
				unwear_item_from_actor(act->actor_id,KIND_OF_NECK);
				actor_wear_item(act->actor_id,KIND_OF_NECK, atoi(id));
			} else {
				//unwear
				unwear_item_from_actor(act->actor_id,KIND_OF_NECK);

			}
		}
	}
	if (!act){
		UNLOCK_ACTORS_LISTS();
		LOG_TO_CONSOLE(c_orange1, "actor not found");
		return 1;
	}
	UNLOCK_ACTORS_LISTS();
	*(id-1)=' ';
	return 1;

}
#endif


static int command_cls(char *text, int len)
{
	clear_display_text_buffer ();
	return 1;
}


static int command_calc(char *text, int len)
{
	double res;
	char str[100];
	int calcerr;

	res = calc_exp(text, &calcerr);
	switch (calcerr){
		case CALCERR_OK:
			if (trunc(res)==res) safe_snprintf (str,sizeof(str), "%s = %.0lf",text,res);
			else safe_snprintf (str,sizeof(str), "%s = %.2lf",text,res);
			LOG_TO_CONSOLE (c_orange1, str);
			break;
		case CALCERR_SYNTAX:
			safe_snprintf (str,sizeof(str), "%s = Syntax error",text);
			LOG_TO_CONSOLE (c_orange1, str);
			break;
		case CALCERR_DIVIDE:
			safe_snprintf (str, sizeof(str),"%s = Divide by zero",text);
			LOG_TO_CONSOLE (c_orange1, str);
			break;
		case CALCERR_MEM:
			safe_snprintf (str,sizeof(str), "%s = Memory error",text);
			LOG_TO_CONSOLE (c_orange1, str);
			break;
		case CALCERR_XOPSYNTAX:
			safe_snprintf (str,sizeof(str), "%s = Bad argument for X", text);
			LOG_TO_CONSOLE (c_orange1, str);
			break;
		case CALCERR_LOPSYNTAX:
			safe_snprintf (str,sizeof(str), "%s = Bad argument for L", text);
			LOG_TO_CONSOLE (c_orange1, str);
			break;
		case CALCERR_EOPSYNTAX:
			safe_snprintf (str,sizeof(str), "%s = Bad argument for E", text);
			LOG_TO_CONSOLE (c_orange1, str);
			break;
		case CALCERR_NOPSYNTAX:
			safe_snprintf (str,sizeof(str), "%s = Bad argument for N", text);
			LOG_TO_CONSOLE (c_orange1, str);
			break;
		case CALCERR_ZOPSYNTAX:
			safe_snprintf (str,sizeof(str), "%s = Bad argument for Z", text);
			LOG_TO_CONSOLE (c_orange1, str);
			break;
		case CALCERR_QOPSYNTAX:
			safe_snprintf (str,sizeof(str), "%s = Bad argument for Q", text);
			LOG_TO_CONSOLE (c_orange1, str);
			break;
	}
	return 1;
}


static int command_markpos(char *text, int len)
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
			if (put_mark_on_current_position(text))
			{
				safe_snprintf (str, sizeof(str), marked_str, text);
				LOG_TO_CONSOLE(c_orange1,str);
			}
		}
	}
	return 1;
}


int command_unmark_special(char *text, int len, int do_log)
{
	int i;

	while (isspace(*text))
		text++;

	if(*text) {
		for (i = 0; i < max_mark; i ++)
		{
			if (!strcasecmp(marks[i].text, text) && (marks[i].x != -1))
			{
				char str[512];
				marks[i].x = marks[i].y = -1;
				if (marks[i].server_side)
				{
					hash_delete(server_marks,(void *)(uintptr_t)(marks[i].server_side_id));
					save_server_markings();
				}
				if (do_log)
				{
					safe_snprintf(str, sizeof(str), unmarked_str, marks[i].text);
					LOG_TO_CONSOLE(c_orange1, str);
				}
				save_markings();
				load_map_marks(); // simply to compact the array and make room for new marks
				break;
			}
		}
	}
	return 1;
}


int command_unmark(char *text, int len)
{
	return command_unmark_special(text, len, 1);
}


static int command_mark_color(char *text, int len)
{
	char str[512];

	while (isspace(*text))
		text++;

	if(*text) {
		int r=-1,g,b;

		if(sscanf(text,"%d %d %d",&r,&g,&b)==3) {
			if(!(r>=0&&r<=255&&g>=0&&g<=255&&b>=0&&b<=255)) r=-1; //don't set color
		} else {
			if(strcasecmp(text,"red")==0) {r=255;g=0;b=0;}
			else if(strcasecmp(text,"blue")==0) {r=0;g=0;b=255;}
			else if(strcasecmp(text,"green")==0) {r=0;g=255;b=0;}
			else if(strcasecmp(text,"yellow")==0) {r=255;g=255;b=0;}
			else if(strcasecmp(text,"cyan")==0) {r=0;g=255;b=255;}
			else if(strcasecmp(text,"magenta")==0) {r=255;g=0;b=255;}
			else if(strcasecmp(text,"white")==0) {r=255;g=255;b=255;}
		}
		if(r>-1) {
			//set color
				curmark_r=r;
				curmark_g=g;
				curmark_b=b;
		}
	}
	safe_snprintf (str, sizeof(str), "Current marker color is (RGB): %d %d %d", curmark_r,curmark_g,curmark_b);
	LOG_TO_CONSOLE(c_orange1,str);
	return 1;
}


static int command_stats(char *text, int len)
{
	unsigned char protocol_name = SERVER_STATS;
	my_tcp_send(&protocol_name, 1);
	return 1;
}


int command_time(char *text, int len)
{
	unsigned char protocol_name = GET_TIME;
	my_tcp_send(&protocol_name, 1);
	return 1;
}


int command_ping(char *text, int len)
{
	Uint8 str[5] = { PING };

	*((Uint32 *)(str+1)) = SDL_SwapLE32(SDL_GetTicks());
	my_tcp_send(str, 5);
	return 1;
}


int command_date(char *text, int len)
{
	unsigned char protocol_name = GET_DATE;
	my_tcp_send(&protocol_name, 1);
	return 1;
}


static int command_quit(char *text, int len)
{
	exit_now = 1;
	return 1;
}


static int command_mem(char *text, int len)
{
	cache_dump_sizes(cache_system);
#ifdef	DEBUG
	cache_dump_sizes(cache_e3d);
#endif	//DEBUG
	return 1;
}


static int command_ver(char *text, int len)
{
	char str[250];

	get_version_string(str, sizeof(str));
	LOG_TO_CONSOLE(c_green1, str);
	return 1;
}


static int command_ignore(const char *text, int len)
{
	char name[MAX_USERNAME_LENGTH];
	int i;
	int result;

	while (isspace(*text))
		text++;

	for (i = 0; i < MAX_USERNAME_LENGTH - 1; i++)
	{
		Uint8 ch = text[i];
		if (ch == ' ' || ch == '\0')
			break;
		name[i] = ch;
	}
	name[i] = '\0';

	if (i >= MAX_USERNAME_LENGTH - 1 && text[i] != '\0') // This is the max chrs of name but isn't a null terminator
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


static int command_filter(char *text, int len)
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


static int command_unignore(char *text, int len)
{
	char name[MAX_USERNAME_LENGTH];
	char str[200];
	int i;
	Uint8 ch = '\0';
	int result;

	while (isspace(*text))
		text++;

	for (i = 0; i < MAX_USERNAME_LENGTH - 1; i++)
	{
		ch = text[i];
		if (ch == ' ' || ch == '\0')
		{
			break;
		}
		name[i] = ch;
	}
	name[i] = '\0';

	if (i >= MAX_USERNAME_LENGTH - 1 && text[i] != '\0') // This is the max chrs of name but isn't a null terminator
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


static int command_unfilter(char *text, int len)
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


static int command_glinfo (const char *text, int len)
{
	const char *my_string;
	size_t size = 8192, minlen;
	char* this_string = calloc (size, 1);

	my_string = (const char*) glGetString (GL_RENDERER);
	if (my_string == NULL)
	{
		const char *error_str = "glGetString() returned NULL for GL_RENDERER=0x%x";
		DO_CHECK_GL_ERRORS();
		LOG_ERROR(error_str, GL_RENDERER);
		LOG_TO_CONSOLE (c_red2, "Error getting glinfo");
		return 1;
	}

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
static int knowledge_command(char *text, int len)
{
	char this_string[90], count_str[60];
	char *cr;
	int num_read = 0, num_total = 0;
	int show_read = 1, show_unread = 1, show_help = 0;
	size_t i;
	char * pstr[3] = { knowledge_param_read, knowledge_param_unread, knowledge_param_total };
	size_t plen[3] = { strlen(knowledge_param_read), strlen(knowledge_param_unread), strlen(knowledge_param_total) };

	// find first space, then skip any spaces
	text = getparams(text);

	// use the short form of the params (-r -u -t) if valid and different
	if ((plen[0] > 1) && (plen[1] > 1) && (plen[2] > 1) &&
		(pstr[0][0] == '-') && (pstr[1][0] == '-') && (pstr[2][0] == '-') &&
		(pstr[0][1] != pstr[1][1]) && (pstr[0][1] != pstr[2][1]) && (pstr[1][1] != pstr[2][1]))
		plen[0] = plen[1] = plen[2] = 2;

	// show the help if no paramaters specified
	if (strlen(text) == 0)
		show_help = 1;

	// Look for -read, -unread or -total paramaters and vary the output appropriately
	else if (strncmp(text, knowledge_param_read, plen[0]) == 0)
	{
		show_unread = 0;
		text = getparams(text+plen[0]);
	}
	else if (strncmp(text, knowledge_param_unread, plen[1]) == 0)
	{
		show_read = 0;
		text = getparams(text+plen[1]);
	}
	else if (strncmp(text, knowledge_param_total, plen[2]) == 0)
	{
		show_read = show_unread = 0;
		text = getparams(text+plen[2]);
	}

	if (show_read || show_unread)
		LOG_TO_CONSOLE(c_green2,knowledge_cmd_str);

	for (i=0; i<KNOWLEDGE_LIST_SIZE; i++)
	{
		size_t len = strlen(knowledge_list[i].name);
		// only display books that contain the specified parameter string
		// shows all books if no string specified
		if (len > 0 && safe_strcasestr(knowledge_list[i].name, len, text, strlen(text)))
		{
			// remove any trailing carrage return
			safe_strncpy(this_string, knowledge_list[i].name, sizeof(this_string));
			if ( (cr = strchr(this_string, '\n')) != NULL)
				*cr = '\0';
			if ((!knowledge_list[i].present) && (your_info.researching == i))
				safe_strcat(this_string, knowledge_reading_book_tag, sizeof(this_string));
			// highlight books that have been read, unread or being read
			if (knowledge_list[i].present)
			{
				if (show_read)
					LOG_TO_CONSOLE((your_info.researching == i) ?c_green2 :c_grey1,this_string);
				++num_read;
			}
			else if (show_unread)
				LOG_TO_CONSOLE((your_info.researching == i) ?c_green1 :c_grey2,this_string);
			++num_total;
		}
	}
	safe_snprintf(count_str, sizeof(count_str), book_count_str, num_read, num_total);
	LOG_TO_CONSOLE(c_grey1, count_str);

	// give help only if no parameters specified
	if (show_help)
		LOG_TO_CONSOLE(c_grey1, know_help_str);

	return 1;
}


static int command_log_conn_data(char *text, int len)
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
static int command_msg(char *text, int len)
{
	int no;

	// find first space, then skip any spaces
	text = getparams(text);
	if(!strncasecmp(text, "all", 3))
	{
		print_all_messages();
	}
	else
	{
		no = atoi(text) - 1;
		print_message(no);
	}
	return 1;
}


static int command_afk(char *text, int len)
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


static int command_help(char *text, int len)
{
	// help can open the Enc!
	if(auto_open_encyclopedia)
		view_tab (MW_HELP, tab_help_collection_id, HELP_TAB_HELP);
	return 1;
}


static int command_storage(char *text, int len)
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
		safe_strncpy(storage_filter, text+i+1, nb+1);
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


static int command_accept_buddy(char *text, int len)
{
	/* This command is here to make sure the requests queue is up to date */
	text = getparams(text);
	/* Make sure a name is given */
	if(*text)
		accept_buddy_console_command(text);
	return 0; // also pass to server
}


/* open the specified url in the configured browser */
static int command_open_url(char *text, int len)
{
	text = getparams(text);
	if (*text)
		open_web_link(text);
	return 1;
}


/* set the command queues wait time between commands */
static int command_set_user_menu_wait_time_ms(char *text, int len)
{
	text = getparams(text);
	if (*text)
		set_command_queue_wait_time_ms(atol(text));
	else
		set_command_queue_wait_time_ms(0);
	return 1;
}


/* parse the command parameters for a spell message, then cast it */
static int command_cast_spell(char *text, int len)
{
	int index = 0;
	int valid_looking_message = 1;
	Uint8 str[30];

	/* valid messages start with the CAST_SPELL message of 39 or 0x27 */
	text = getparams(text);
	if (!*text || strstr(text, "27")==NULL)
		valid_looking_message = 0;
	/* skip past everything until the CAST_SPELL message type */
	else
		text = strstr(text, "27");

	/* while we have hex digit pairs to process */
	while (valid_looking_message && strlen(text)>0 && index<30)
	{
		int i;
		Uint8 d[2];
		while (*text==' ')
			text++;
		if (strlen(text)<2)
			break;
		for (i=0; i<2; i++)
		{
			d[i] = *text++;
			if (d[i] >= '0' && d[i] <= '9')
				d[i] -= '0';
			else if (d[i] >= 'a' && d[i] <= 'f')
				d[i] -= 'a'-10;
			else if (d[i] >= 'A' && d[i] <= 'F')
				d[i] -= 'A'-10;
			else
			{
				valid_looking_message = 0;
				break;
			}
		}
		/* store the spell message byte */
		if (valid_looking_message)
			str[index++] = d[1] + 16*d[0];
	}

	/* if we're now at the end of the text, we have some message bytes and it looks valid */
	if (!*text && index && valid_looking_message)
		send_spell(str, index);
	else
		LOG_TO_CONSOLE(c_red2, invalid_spell_string_str);

	return 1;
}


/* display or test the md5sum of the current map or the specified file */
static int command_ckdata(char *text, int len)
{
	const int DIGEST_LEN = 16;
	Uint8 digest[DIGEST_LEN];
	char digest_str[DIGEST_LEN*2+1];
	char expected_digest_str[DIGEST_LEN*2+1];
	char result_str[256];
	char filename[256];

	if ((cur_map < 0)  || (continent_maps[cur_map].name == NULL))
	{
		LOG_TO_CONSOLE(c_red1, "Invalid current map");
		return 1;
	}

	/* paramters are optional, first is expected checksum value, second is filename */
	/* if only a filename is specfied, we display checksum rather than do match */
	filename[0] = digest_str[0] = expected_digest_str[0] = '\0';
	text = getparams(text);
	if (*text)
	{
		/* if we have at least one space and the first string is of digest length, assume we matching */
		char *tempstr = safe_strcasestr(text, strlen(text), " ", 1);
		if ((tempstr != NULL) && (strlen(text) - strlen(tempstr) == DIGEST_LEN*2))
		{
			safe_strncpy2(expected_digest_str, text, DIGEST_LEN*2+1, DIGEST_LEN*2 );
			/* trim leading space from filename */
			while (*tempstr == ' ')
				tempstr++;
			if (*tempstr)
				safe_strncpy(filename, tempstr, 256);
		}
		/* else we only have a filename */
		else
			safe_strncpy(filename, text, 256 );
	}
	/* if no parameters default to current map elm file */
	else
		safe_strncpy(filename, continent_maps[cur_map].name, 256 );

	/* calculate, display checksum if we're not matching */
	if (*filename && el_file_exists(filename) && get_file_digest(filename, digest))
	{
		int i;
		for(i=0; i<DIGEST_LEN; i++)
			sprintf(&digest_str[2*i], "%02x", (int)digest[i]);
		digest_str[DIGEST_LEN*2] = 0;
		if (! *expected_digest_str)
		{
			safe_snprintf(result_str, sizeof(result_str), "#ckdata %s %s", digest_str, filename );
			LOG_TO_CONSOLE(c_grey1,result_str);
		}
	}
	/* show help if something fails */
	else
	{
		LOG_TO_CONSOLE(c_red2, "ckdata: invalid file or command syntax.");
		LOG_TO_CONSOLE(c_red1, "Show current map (elm): #ckdata");
		LOG_TO_CONSOLE(c_red1, "Show specified file:    #ckdata file_name");
		LOG_TO_CONSOLE(c_red1, "Check specified file:   #ckdata expected_checksum file_name");
		return 1;
	}

	/* if we have an expected value, compare then display an appropriate message */
	if (*expected_digest_str)
	{
		if (!strcasecmp(digest_str, expected_digest_str))
			LOG_TO_CONSOLE(c_green2,"ckdata: File matches expected checksum");
		else
			LOG_TO_CONSOLE(c_red2,"ckdata: File does not match expected checksum");
	}

	return 1;

} /* end command_ckdata() */


/* pretend the specified key has been pressed - allows user menu to trigger keypress events */
static int command_keypress(char *text, int len)
{
	text = getparams(text);
	if (*text)
	{
		el_key_def value = get_key_value(text);
		if (value.key_code != SDLK_UNKNOWN)
			do_keypress(value);
	}
	return 1;
}


//	Set the number of items to transfer
//
static int command_quantity(char *text, int len)
{
	char str[80];
	int calcerr;
	double res;
	text = getparams(text);
	res = calc_exp(text, &calcerr);
	if ((calcerr != CALCERR_OK) || (res < 1.0) || (res > (double)INT_MAX))
		LOG_TO_CONSOLE(c_red1, um_invalid_command_str);
	else
	{
		// set the quantity to the result of the calculation, truncated
		quantities.selected = ITEM_EDIT_QUANT;
		item_quantity = quantities.quantity[ITEM_EDIT_QUANT].val = (int)res;
		if (trunc(res)==res)
			safe_snprintf(str, sizeof(str), "%s: %s = %d", quantity_str, text, item_quantity);
		else
			safe_snprintf(str, sizeof(str), "%s: %s = %.2lf -> %d", quantity_str, text, res, item_quantity);
		LOG_TO_CONSOLE(c_green1, str);
	}
	return 1;
}


void save_local_data(void)
{
	save_bin_cfg();
	//Save the quickbar spells
	save_quickspells();
	//Save recipes
	save_recipes();
	// save ini file if asked
	if (write_ini_on_exit) write_el_ini ();
	// save notepad contents if the file was loaded
	if (notepad_loaded) notepad_save_file();
	//save_exploration_map();
	flush_counters();
	// for the new questlog, this actually just saves any pending changes
	// should be renamed when NEW_QUESTLOG #def is removed
	unload_questlog();
	save_item_lists();
	save_channel_colors();
#ifdef JSON_FILES
	save_character_options();
#endif
}


//	On a regular basis, send the "#save" command to the server and save local data.
//
void auto_save_local_and_server(void)
{
	time_t time_delta = 60 * 90;
	actor *me;

	me = get_our_actor();
	if(!is_disconnected() && me && !me->fighting && ((last_save_time + time_delta) <= time(NULL)))
	{
		last_save_time = time(NULL);
		save_local_data();
		LOG_TO_CONSOLE(c_green1, full_save_str);
		send_input_text_line("#save", 5);
	}
}


/* show counters for this session */
static int session_counters(char *text, int len)
{
	text = getparams(text);
	print_session_counters(text);
	return 1;
}


/* the #save command, save local file then pass to server to save there too */
static int command_save(char *text, int len)
{
	save_local_data();
	LOG_TO_CONSOLE(c_green1, full_save_str);
	return 0; // pass onto server
}


/* the #disco or #disconnect command forces a server logout */
static int command_disconnect(char *text, int len)
{
	save_local_data();
	LOG_TO_CONSOLE(c_green1, local_only_save_str);
	force_server_disconnect(user_disconnect_str);
	return 1;
}


/* initilates a test for server connection, the client will enter the disconnected state if needed */
static int command_relogin(char *text, int len)
{
	start_testing_server_connection();
	return 1;
}


static int command_change_pass(char *text, int len)
{
	if (!passmngr_pending_pw_change(getparams(text)))
	{
		LOG_TO_CONSOLE(c_red1, invalid_pass);
		LOG_TO_CONSOLE(c_red1, password_format_str);
		return 1;
	}
	else
		return 0;
}


static int command_reset_res(char *text, int len)
{
	restore_starting_video_mode();
	LOG_TO_CONSOLE(c_yellow1, reset_res_str);
	return 1;
}


static int command_set_res(char *text, int len)
{
	text = getparams(text);
	if (*text)
	{
		int new_width = 0, new_height = 0;
		new_width = atoi(text);
		text = getparams(text);
		if (*text)
			new_height = atoi(text);
		if ((new_width > 0) && (new_height > 0))
		{
			set_client_window_size(new_width, new_height);
			LOG_TO_CONSOLE(c_yellow1, set_res_str);
			return 1;
		}
	}
	LOG_TO_CONSOLE(c_red1, um_invalid_command_str);
	return 1;
}


static int command_save_res(char *text, int len)
{
	set_user_defined_video_mode();
	LOG_TO_CONSOLE(c_yellow1, save_res_str);
	return 1;
}


static int command_show_res(char *text, int len)
{
	char str[80];
	safe_snprintf(str, sizeof(str), "%s %dx%d", show_res_str, window_width, window_height);
	LOG_TO_CONSOLE(c_yellow1, str);
	return 1;
}


// Summoning attack state for the hud indicator, -1, 0 not "attack at will", 1 is "attack at will".
static int summon_attack_mode_state = -1;
int summon_attack_is_active(void) { return (summon_attack_mode_state == 1); }
int summon_attack_is_unknown(void) { return (summon_attack_mode_state < 0); }

typedef struct { const char opt; const char code; const char *desc; } summon_attack_modes;
static const summon_attack_modes modes[] =
{
	{ .opt = 'a', .code = '\1', .desc = no_attack_str },
	{ .opt = 'b', .code = '\0', .desc = attack_my_opponent_str },
	{ .opt = 'c', .code = '\2', .desc = do_not_attack_my_opponent_str },
	{ .opt = 'd', .code = '\3', .desc = attack_only_summoned_str },
	{ .opt = 'e', .code = '\4', .desc = do_not_attack_summoned_str },
	{ .opt = 'f', .code = '\5', .desc = attack_at_will_str }
};
static const size_t attack_at_will_index  = 5;

// Check the message just sent to the server from the popup menu
// Set the summon attack mode if it was a summoning attack mode message
void check_summon_attack_mode(unsigned char *buffer, size_t len)
{
	if ((len == 5) && (buffer[0] == POPUP_REPLY) && (buffer[1] == 0) && (buffer[2] == 0) && (buffer[3] == 1))
	{
		size_t i;
		for (i = 0; i < sizeof(modes)/sizeof(summon_attack_modes); i++)
			if (buffer[4] == modes[i].code)
			{
				if (i == attack_at_will_index)
					summon_attack_mode_state = 1;
				else
					summon_attack_mode_state = 0;
				break;
			}
	}
}

// Send summoning attack mode to server.
// Provides a direct version of summoning popup menu.
//
static int command_summon_attack(char *text, int len)
{
	size_t option_index = sizeof(modes)/sizeof(summon_attack_modes);
	int show_help = 1;
	char str[128];
	size_t i;

	text = getparams(text);

	// if no parameter specified, toggle between attack at will, and no attack
	if (!*text)
	{
		if (summon_attack_mode_state == 1)
			option_index = 0;
		else
			option_index = attack_at_will_index;
		show_help = 0;
	}

	// else if its a one character parameter, check if its an option
	else if (strlen(text) == 1)
	{
		for (i = 0; i < sizeof(modes)/sizeof(summon_attack_modes); i++)
			if (tolower(text[0]) == modes[i].opt)
			{
				option_index = i;
				show_help = 0;
				break;
			}
	}

	// if we have a valid option, send the message to the server and set the state
	if (option_index < sizeof(modes)/sizeof(summon_attack_modes))
	{
		unsigned char buffer[] = { POPUP_REPLY, 0, 0, 1, modes[option_index].code};
		my_tcp_send(buffer, sizeof(buffer));
		safe_snprintf(str, sizeof(str), "%s %s", summon_attack_set_mode_str, modes[option_index].desc);
		LOG_TO_CONSOLE(c_green1, str);
		show_help = 0;
		summon_attack_mode_state = (option_index == attack_at_will_index) ?1: 0;
	}

	// if option specified but not valid, show help
	if (show_help)
	{
		LOG_TO_CONSOLE(c_green1, summon_attack_help_str);
		for (i = 0; i < sizeof(modes)/sizeof(summon_attack_modes); i++)
		{
			safe_snprintf(str, sizeof(str), "  %c - %s", modes[i].opt, modes[i].desc);
			LOG_TO_CONSOLE(c_green1, str);
		}
	}

	return 1;
}


#ifdef CONTEXT_MENUS_TEST
int cm_test_window(char *text, int len);
#endif


// list the #commands available
static void commands_summary(void)
{
	char *str = NULL;
	const char *delim = "  .  ";
	const size_t str_len = 1 + (MAX_COMMAND_NAME_LEN + strlen(delim)) * command_count;
	size_t i;

	if ((str = malloc(str_len)) == NULL)
		return;

	LOG_TO_CONSOLE(c_green1, commands_help_description_help_str);
	LOG_TO_CONSOLE(c_green1, commands_help_search_help_str);
	str[0] = '\0';
	for(i = 0; i < command_count; i++) 
	{
		if (str[0] != '\0')
			safe_strcat(str, delim, str_len);
		safe_strcat(str, commands[i].command, str_len);
	}
	if (str[0] != '\0')
		LOG_TO_CONSOLE(c_grey1, str);

	free(str);
}


// a structure to store command name, paramaters string and description string for commands help
typedef struct
{
	char *c_str;
	char *p_str;
	char *d_str;
} commands_help_t;

static commands_help_t *commands_help = NULL;
static size_t commands_help_size = 0;
static int commands_help_loaded = 0;


// free memory used by the commands help
static void cleanup_commands_help(void)
{
	size_t i;
	if (!commands_help_size)
		return;
	commands_help_loaded = 0;
	for (i = 0; i < commands_help_size; i++)
	{
		if (commands_help[i].c_str != NULL)
			free(commands_help[i].c_str); 
		if (commands_help[i].p_str != NULL)
			free(commands_help[i].p_str); 
		if (commands_help[i].d_str != NULL)
			free(commands_help[i].d_str); 
	}
	free(commands_help);
	commands_help_size = 0;
	commands_help = NULL;
}


// Load the commands help file
static int load_commands_help(void)
{
	FILE *fp = NULL;
	char *line = NULL;
	const char *delim = " ## ";
	size_t delim_len = strlen(delim);
	const size_t line_buf_len = 2048;
	const char *filename = "commands_help.txt";
	if ((fp = open_file_lang(filename, "r")) == NULL)
	{
		LOG_ERROR("%s [%s]\n", cant_open_file, filename);
		return 0;
	}
	line = malloc(line_buf_len);
	if (line == NULL)
		return 0;
	while (!feof(fp))
	{
		if ((fgets(line, line_buf_len, fp) != NULL) && (strlen(line) > 3 * delim_len))
		{
			size_t line_len = strlen(line);
			size_t c_len, p_len, d_len;
			char *c_str, *p_str, *d_str;
			int is_valid = 0;
			if (line[line_len - 1] == '\n')
				line[--line_len] = '\0';
			c_str = line;
			p_str = strstr(c_str, delim);
			if ((p_str != NULL) && (strlen(p_str) > delim_len))
			{
				d_str = strstr(p_str + delim_len, delim);
				if ((d_str != NULL) && (strlen(d_str) > delim_len))
				{
					c_len = p_str - c_str;
					p_str += delim_len;
					p_len = d_str - p_str;
					d_str += delim_len;
					d_len = strlen(d_str);
					if ((c_len > 0) && (d_len > 0))
					{
						commands_help = realloc(commands_help, sizeof(commands_help_t) * ++commands_help_size);
						if (commands_help != NULL)
						{
							commands_help_t *ptr = &commands_help[commands_help_size - 1];
							ptr->c_str = malloc(c_len + 1);
							ptr->p_str = malloc(p_len + 1);
							ptr->d_str = malloc(d_len + 1);
							safe_strncpy2(ptr->c_str, c_str, c_len + 1, c_len);
							safe_strncpy2(ptr->p_str, p_str, p_len + 1, p_len);
							safe_strncpy2(ptr->d_str, d_str, d_len + 1, d_len);
							is_valid = 1;
						}
					}
				}
			}
			if (!is_valid)
				LOG_ERROR("Invalid command help line [%s]\n", line);
		}
	}
	free(line);
	fclose(fp);
	return 1;
}

// list #commands where the specified text can be found in there
// command name, parameters or description.
static int command_commands_search(char *text, int len)
{
	text = getparams(text);
	if (*text)
	{
		char str[256];
		size_t i;
		if (!commands_help_loaded)
			commands_help_loaded = load_commands_help();
		safe_snprintf(str, sizeof(str), "%s <%s> :-", commands_search_prefix_str, text);
		LOG_TO_CONSOLE(c_green1, str);
		for (i = 0; i < commands_help_size; i++)
		{
			if ((safe_strcasestr(commands_help[i].d_str, strlen(commands_help[i].d_str), text, strlen(text)) != NULL) ||
				(safe_strcasestr(commands_help[i].c_str, strlen(commands_help[i].c_str), text, strlen(text)) != NULL) ||
				(safe_strcasestr(commands_help[i].p_str, strlen(commands_help[i].p_str), text, strlen(text)) != NULL))
			{
				safe_snprintf(str, sizeof(str), "%s %s - %s",
					commands_help[i].c_str, commands_help[i].p_str, commands_help[i].d_str);
				LOG_TO_CONSOLE(c_grey1, str);
			}
		}
	}
	return 1;
}


// list all commands or show help for specified command
static int command_commands(char *text, int len)
{
	text = getparams(text);
	if (*text)
	{
		char str[128];
		size_t i;
		if (!commands_help_loaded)
			commands_help_loaded = load_commands_help();
		for (i = 0; i < commands_help_size; i++)
		{
			if (strcmp(text, commands_help[i].c_str) == 0)
			{
				safe_snprintf(str, sizeof(str), "%s: #%s %s",
					commands_help_prefix_str, commands_help[i].c_str, commands_help[i].p_str);
				LOG_TO_CONSOLE(c_green1, str);
				LOG_TO_CONSOLE(c_grey1, commands_help[i].d_str);
				return 1;
			}
		}
		if (commands_help_size)
		{
			safe_snprintf(str, sizeof(str), "%s [%s]", commands_help_not_recognsed_str, text);
			LOG_TO_CONSOLE(c_red1, str);
		}
		else
			LOG_TO_CONSOLE(c_red1, commands_help_not_loaded_str);
	}
	else
		commands_summary();
	return 1;
}


static int command_cmp(const void *a, const void *b)
{
	return strcmp(((const command_t *)a)->command,
				((command_t *)b)->command);
}


void init_commands(const char *filename)
{
	char tmp_name[MAX_COMMAND_NAME_LEN];
	FILE *fp = open_file_data(filename, "r");
	if(fp == NULL) {
		LOG_ERROR("%s: %s \"%s\": %s\n", reg_error_str, cant_open_file, filename, strerror(errno));
	} else {
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

#ifdef MORE_ATTACHED_ACTORS_DEBUG
	add_command("horse", &horse_cmd);
#endif
#ifdef NECK_ITEMS_DEBUG
	add_command("set_neck", &set_neck);
#endif
	add_command("emotes", &print_emotes);
#ifdef EMOTES_DEBUG
	add_command("add_emote", &add_emote);
	add_command("send_cmd", &send_cmd);
	add_command("set_idle", &set_idle);
	add_command("set_action", &set_action);
#endif
	add_command("marker_color", &command_mark_color);
	add_command("calc", &command_calc);
	add_command("cls", &command_cls);
	add_command(cmd_markpos, &command_markpos);
	add_command(cmd_mark, &command_mark);
	add_command(cmd_unmark, &command_unmark);
	add_command(cmd_stats, &command_stats);
	add_command("ping", &command_ping);
#ifdef	CUSTOM_UPDATE
	add_command("update", &command_update);
	add_command("update_status", &command_update_status);
#endif	/* CUSTOM_UPDATE */
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
	add_command("channel_colors", &command_channel_colors);
	add_command(help_cmd_str, &command_help);
	add_command("sto", &command_storage);
	add_command("storage", &command_storage);
	add_command("accept_buddy", &command_accept_buddy);
#ifdef NEW_SOUND
	add_command("current_song", &display_song_name);
#endif // NEW_SOUND
	add_command("find", &history_grep);
	add_command("save", &command_save);
	add_command("url", &url_command);
	add_command(cmd_session_counters, &session_counters);
	add_command("exp", &show_exp);
#ifdef CONTEXT_MENUS_TEST
	add_command("cmtest", &cm_test_window);
#endif
	add_command("alias", &alias_command);
	add_command("unalias", &unalias_command);
	add_command("aliases", &aliases_command);
	add_command("ckdata", &command_ckdata);
#if defined(BUFF_DURATION_DEBUG)
	add_command("buffd", &command_buff_duration);
#endif
	add_command(cmd_reload_icons, &reload_icon_window);
	add_command(cmd_open_url, &command_open_url);
	add_command(cmd_show_spell, &command_show_spell);
	add_command(cmd_cast_spell, &command_cast_spell);
	add_command(cmd_keypress, &command_keypress);
	add_command(cmd_user_menu_wait_time_ms, &command_set_user_menu_wait_time_ms);
	add_command(cmd_relogin, &command_relogin);
	add_command(cmd_disconnect, &command_disconnect);
	add_command(cmd_disco, &command_disconnect);
	add_command("q", &command_quantity);
	safe_strncpy(tmp_name, quantity_str, MAX_COMMAND_NAME_LEN);
	add_command(my_tolower(tmp_name), &command_quantity);
	add_command("change_pass", &command_change_pass);
	add_command("reset_res", &command_reset_res);
	add_command("set_res", &command_set_res);
	add_command("save_res", &command_save_res);
	add_command("show_res", &command_show_res);
	add_command("#", &command_commands);
	add_command("?", &command_commands_search);
	add_command("set_default_fonts", &command_set_default_fonts);
	add_command(cmd_summon_attack, &command_summon_attack);
	add_command(cmd_summon_attack_short, &command_summon_attack);

	// Sort the command list alphabetically so that the #command lists
	// them sorted and the ctrl+SPACE cycles them sorted.  Assumes no
	// other add_command() calls take place after this, otherwise the
	// sorted order will likely be wrong.
	qsort(commands, command_count, sizeof(command_t), command_cmp);

	command_buffer_offset = NULL;
}


void new_minute_console(void){
	if(!(real_game_minute%60)){
		timestamp_chat_log();
	}
	if(time_warn_h >= 0 && (time_warn_h+real_game_minute)%60 == 0){
		char str[75];
		safe_snprintf(str, sizeof(str), time_warn_hour_str, time_warn_h);
		LOG_TO_CONSOLE(c_purple1, str);
	}
	if(time_warn_s >= 0 && (time_warn_s+real_game_minute)%180 == 30){
		char str[100];
		if (time_warn_s+real_game_minute == 30) { // sunrise
			safe_snprintf(str, sizeof(str), time_warn_sunrise_str, time_warn_s);
		}
		else { // sunset
			safe_snprintf(str, sizeof(str), time_warn_sunset_str, time_warn_s);
		}
		LOG_TO_CONSOLE(c_purple1, str);
	}
	if(time_warn_d >= 0 && (time_warn_d+real_game_minute)%360 == 0){
		char str[75];
		safe_snprintf(str, sizeof(str), time_warn_day_str, time_warn_d);
		LOG_TO_CONSOLE(c_purple1, str);
	}
}
