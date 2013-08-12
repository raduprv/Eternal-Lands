#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "text.h"
#include "achievements.h"
#include "actors.h"
#include "asc.h"
#include "buddy.h"
#include "chat.h"
#include "console.h"
#include "consolewin.h"
#include "elconfig.h"
#include "errors.h"
#include "filter.h"
#include "gl_init.h"
#include "global.h"
#include "hud.h"
#include "init.h"
#include "lights.h"
#include "misc.h"
#include "multiplayer.h"
#include "paste.h"
#include "pm_log.h"
#include "translate.h"
#include "astrology.h"
#include "url.h"
#include "counters.h"
#include "io/elpathwrapper.h"
#include "spells.h"
#include "serverpopup.h"
#include "sky.h"
#include "sound.h"
#include "trade_log.h"
#include "actor_scripts.h"
#include "emotes.h"

int emote_filter=0;
int summoning_filter=0;

text_message display_text_buffer[DISPLAY_TEXT_BUFFER_SIZE];
int last_message = -1;
Uint8 current_filter = FILTER_ALL;

text_message input_text_line;

char last_pm_from[32];

Uint32 last_server_message_time;
int lines_to_show=0;

int show_timestamp = 0;

int dark_channeltext = 0;

int log_chat = LOG_SERVER;

float	chat_zoom=1.0;
FILE	*chat_log=NULL;
FILE	*srv_log=NULL;

ec_reference harvesting_effect_reference = NULL;

#ifndef NEW_SOUND
int afk_snd_warning = 0;
#endif

/* forward declaration */
void put_small_colored_text_in_box (Uint8 color, const Uint8 *text_to_add, int len, int pixels_limit, char *buffer);

void alloc_text_message_data (text_message *msg, int size)
{
	msg->data = size > 0 ? calloc (size, 1) : NULL;
	msg->size = size;
	msg->len = 0;
}

void resize_text_message_data (text_message *msg, int len)
{
	if (msg->size <= len)
	{
		int nsize = msg->size ? 2 * msg->size : len+1;
		while (nsize < len)
			nsize += nsize;
		msg->data = realloc (msg->data, nsize);
		msg->size = nsize;
        }
}

void set_text_message_data (text_message *msg, const char* data)
{
	if (data == NULL || data[0] == '\0')
	{
		clear_text_message_data (msg);
	}
	else if (msg->size > 0)
	{
		safe_strncpy (msg->data, data, msg->size);
		msg->len = strlen (msg->data);
        }
}

void init_text_buffers ()
{
	int i;

	for (i = 0; i < DISPLAY_TEXT_BUFFER_SIZE; i++)
		init_text_message (display_text_buffer + i, 0);

	init_text_message (&input_text_line, MAX_TEXT_MESSAGE_LENGTH + 1);
	input_text_line.chan_idx = CHAT_ALL;
	set_text_message_color (&input_text_line, 1.0f, 1.0f, 1.0f);
}

void cleanup_text_buffers(void)
{
	int i;

	free_text_message_data (&input_text_line);
	for(i = 0; i < DISPLAY_TEXT_BUFFER_SIZE; i++)
		free_text_message_data (display_text_buffer + i);
}

void update_text_windows (text_message * pmsg)
{
	if (console_root_win >= 0) update_console_win (pmsg);
	switch (use_windowed_chat) {
		case 0:
			rewrap_message(pmsg, chat_zoom, get_console_text_width(), NULL);
			lines_to_show += pmsg->wrap_lines;
			if (lines_to_show > 10) lines_to_show = 10;
			break;
		case 1:
			update_tab_bar (pmsg);
			break;
		case 2:
			update_chat_window (pmsg, 1);
			break;
	}
}

void open_chat_log(){
	char starttime[200], sttime[200];
	struct tm *l_time; time_t c_time;

	char chat_log_file[100];
	char srv_log_file[100];

	time(&c_time);
	l_time = localtime(&c_time);

	if (get_rotate_chat_log())
	{
		char logsuffix[7];
		strftime(logsuffix, sizeof(logsuffix), "%Y%m", l_time);
		safe_snprintf (chat_log_file, sizeof (chat_log_file),  "chat_log_%s.txt", logsuffix);
		safe_snprintf (srv_log_file, sizeof (srv_log_file), "srv_log_%s.txt", logsuffix); 
	}
	else
	{
		safe_strncpy(chat_log_file, "chat_log.txt", sizeof(chat_log_file));
		safe_strncpy(srv_log_file, "srv_log.txt", sizeof(srv_log_file));
	}

	chat_log = open_file_config (chat_log_file, "a");
	if (log_chat == LOG_SERVER || log_chat == LOG_SERVER_SEPERATE)
		srv_log = open_file_config (srv_log_file, "a");
	if (chat_log == NULL)
	{
		LOG_TO_CONSOLE(c_red3, "Unable to open log file to write. We will NOT be recording anything.");
		log_chat = LOG_NONE;
		return;
	}
	else if ((log_chat == LOG_SERVER || log_chat == LOG_SERVER_SEPERATE) && srv_log == NULL)
	{
		LOG_TO_CONSOLE(c_red3, "Unable to open server log file to write. We will fall back to recording everything in chat_log.txt.");
		log_chat = LOG_CHAT;
		return;
	}
	strftime(sttime, sizeof(sttime), "\n\nLog started at %Y-%m-%d %H:%M:%S localtime", l_time);
	safe_snprintf(starttime, sizeof(starttime), "%s (%s)\n\n", sttime, tzname[l_time->tm_isdst>0]);
	fwrite (starttime, strlen(starttime), 1, chat_log);
}


void timestamp_chat_log(){
	char starttime[200], sttime[200];
	struct tm *l_time; time_t c_time;

	if(log_chat == LOG_NONE) {
		return; //we're not logging anything
	}

	if (chat_log == NULL){
		open_chat_log();
	} else {
		time(&c_time);
		l_time = localtime(&c_time);
		strftime(sttime, sizeof(sttime), "Hourly time-stamp: log continued at %Y-%m-%d %H:%M:%S localtime", l_time);
		safe_snprintf(starttime, sizeof(starttime), "%s (%s)\n", sttime, tzname[l_time->tm_isdst>0]);
		fwrite (starttime, strlen(starttime), 1, chat_log);
	}
}


void write_to_log (Uint8 channel, const Uint8* const data, int len)
{
	int i, j;
	Uint8 ch;
	char str[1024];
	struct tm *l_time; time_t c_time;
	FILE *fout;

#ifdef NEW_SOUND
	// Check if this string matches text we play a sound for
	check_sound_alerts(data, len, channel);
#endif // NEW_SOUND

	if(log_chat == LOG_NONE || (channel == CHAT_SERVER && log_chat == LOG_CHAT))
		// We're not logging at all, or this is a server message and
		// we're not logging those
		return;

	if (chat_log == NULL){
		open_chat_log();
		if(chat_log == NULL){
			return;
		}
	}

	// The file we'll write to
	fout = (channel == CHAT_SERVER && log_chat >= 3) ? srv_log : chat_log;

	if(!show_timestamp)
	{
		// Start filling the buffer with the time stamp
		time (&c_time);
		l_time = localtime (&c_time);
		j = strftime (str, sizeof(str), "[%H:%M:%S] ", l_time);
	}
	else
	{
		//we already have a time stamp
		j=0;
	}

	i = 0;
	while (i < len)
	{
		for ( ; i < len && j < sizeof (str) - 1; i++)
		{
			ch = data[i];

			// remove colorization and soft wrapping characters when
			// writing to the chat log
			if (!is_color (ch) && ch != '\r')
				str[j++] = ch;
		}
		if (i >= len) str[j++]='\n';

		fwrite (str, j, 1, fout);

		// start again at the beginning of the buffer
		j = 0;
	}

	// Flush the file, so the content is written even when EL crashes.
	fflush (fout);
}

void send_input_text_line (char *line, int line_len)
{
	char str[256];
	int i,j;
	int len;
	Uint8 ch;

	switch(use_windowed_chat)
	{
		case 1:
			if(tabs[current_tab].channel != CHAT_ALL) {
				change_to_current_tab(line);
			}
		break;
		case 2:
			if(channels[active_tab].chan_nr != CHAT_ALL) {
				change_to_current_chat_tab(line);
			}
		break;
	}

	if ( caps_filter && line_len > 4 && my_isupper (line, -1) )
		my_tolower (line);

	i=0;
	j=1;
	if (line[0] != '/' && line[0] != char_slash_str[0])	//we don't have a PM
	{
		str[0] = RAW_TEXT;
	}
	else
	{
		str[0] = SEND_PM;
		i++;	// skip the leading /
	}

	for ( ; i < line_len && j < sizeof (str) - 1; i++)	// copy it, but ignore the enter
	{
		ch = line[i];
		if (ch != '\n' && ch != '\r')
		{
			str[j] = ch;
			j++;
		}
	}
	str[j] = 0;	// always a NULL at the end

	len = strlen (&str[1]);
	if (my_tcp_send (my_socket, (Uint8*)str, len+1) < len+1)
	{
		//we got a nasty error, log it
	}

	return;
}

int match_emote(emote_dict *command, actor *act, int send)
{
	hash_entry *match;

	// Try to match the input against an emote command and actor type	
	match=hash_get(emote_cmds,(void*)command->command);
	
	if(match){
		//printf("Emote <%s> sent (%p)\n",((emote_dict*)match->item)->command,((emote_dict*)match->item)->emote);
		//SEND emote to server
		if (send) send_emote(((emote_dict*)match->item)->emote->id);

		return 1;
	}
	return 0;
}


int parse_text_for_emote_commands(const char *text, int len)
{
	int i=0, j = 0, wf=0,ef=0, itsme=0;
	char name[20];	// Yeah, this should be done correctly
	emote_dict emote_text;
	actor *act;

	
	//printf("parsing local for emotes\n");
	//extract name
	while(text[i]&&i<20){
		if (is_color(text[i])) {i++; continue;}
		name[j]=text[i];
		if(text[i]==' ' || text[i]==':') {
			name[j]=0;
			if(text[i]==':') i++;
			break;
		} 
		i++;j++;
	}

	if(j>=20||name[j]) return 0; 		//out of bound or not terminated

	//check if we are saying text
	LOCK_ACTORS_LISTS();

	act = get_actor_ptr_from_id(yourself);
	if (!act){
		UNLOCK_ACTORS_LISTS();
		LOG_ERROR("Unable to find actor who just said local text?? name: %s", name);
		return 1;		// Eek! We don't have an actor match... o.O
	}

	if (!(!strncasecmp(act->actor_name, name, strlen(name)) && 
			(act->actor_name[strlen(name)] == ' ' ||
			act->actor_name[strlen(name)] == '\0'))){
		//we are not saying this text, return
		//UNLOCK_ACTORS_LISTS();			
		//return 0;
			itsme=0;
	} else itsme=1;

	j=0;
	do {
		if (is_color(text[i])) continue;
		if ((text[i]==' ' || text[i]==0)) {
			if (j&&j<=MAX_EMOTE_LEN) {
				wf++;
				emote_text.command[j]=0;
				ef+=match_emote(&emote_text,act,itsme);
			} else wf+= (j) ? 1:0;
			j=0;
		} else {
			if (j<MAX_EMOTE_LEN)
				emote_text.command[j]=text[i];
			j++;
		}
	} while(text[i++]);	
	//printf("ef=%i, wf=%i, filter=>%i\n",ef,wf,emote_filter);
	UNLOCK_ACTORS_LISTS();			

	return  ((ef==wf) ? (emote_filter):(0));

}


/* stop or restart the harvesting eye candy effect depending on the harvesting state */
void check_harvesting_effect(void)
{
	/* if the harvesting effect is on but we're not harvesting, stop it */
	if ((!harvesting || !use_harvesting_eye_candy) && (harvesting_effect_reference != NULL))
	{
		ec_recall_effect(harvesting_effect_reference);
		harvesting_effect_reference = NULL;
	}
	/* but if we are harvesting but there is no effect, start it if wanted */
	else if (harvesting && use_eye_candy && use_harvesting_eye_candy && (harvesting_effect_reference == NULL))
	{
		actor *act;
		LOCK_ACTORS_LISTS();
		act = get_actor_ptr_from_id(yourself);
		if (act != NULL)
			harvesting_effect_reference = ec_create_ongoing_harvesting2(act, 1.0, 1.0, (poor_man ? 6 : 10), 1.0);
		UNLOCK_ACTORS_LISTS();
	}
}	


int filter_or_ignore_text (char *text_to_add, int len, int size, Uint8 channel)
{
	int l, idx;

	if (len <= 0) return 0;	// no point

	//check for auto receiving #help
	for (idx = 0; idx < len; idx++)
	{
		if (!is_color (text_to_add[idx])) break;
	}
	l = len - idx;
	if (l >= strlen(help_request_str) && text_to_add[idx] == '#' && (strncasecmp (&text_to_add[idx], help_request_str, strlen(help_request_str)) == 0 || strncasecmp (&text_to_add[idx], "#mod chat", 9) == 0))
	{
		auto_open_encyclopedia = 0;
	}

	/*
	DANGER, WILL ROBINSON!

	The below code should not exist in it's present form.  I'd change it,
	but I'd need access to the server.  Simply checking text output (which
	is used for all sorts of things) for the phrase "Game Date" is very
	dangerous.  Example: what if, in the future, we allow spaces in
	character names?  Someone chooses the name "Game Date" and walks around
	saying "hi".  Everyone's clients in the area interpret this as being a
	Game Date command.

	I've made the below code not *as* dangerous. Had a user been able to
	fake out the below code, previously, it would have caused a buffer overflow
	in their client if they didn't write in only numbers after it.  Now, they
	won't crash; it'll just be misparsed.

	General practice recommendation: don't mix server commands with user
	input.

	 - Karen
	*/
	/*
	ed (ttlanhil): made it check if it's a server colour. still not perfect
	(this should have been done server-side instead of parsing the date), but safer
	*/
	if (from_color_char (text_to_add[0]) == c_green1 && my_strncompare(text_to_add+1,"Game Date", 9))
	{
		//we assume that the server will still send little-endian dd/mm/yyyy... we could make it safer by parsing the format too, but it's simpler to assume
		const char * const month_names[] = { "Aluwia", "Seedar", "Akbar", "Zartia", "Elandra", "Viasia", "Fruitfall", "Mortia", "Carnelar", "Nimlos", "Chimar", "Vespia" };
		const char * const day_names[] = { "1st", "2nd", "3rd", "4th", "5th", "6th", "7th", "8th", "9th", "10th", "11th", "12th", "13th", "14th", "15th", "16th", "17th", "18th", "19th", "20th", "21st", "22nd", "23rd", "24th", "25th", "26th", "27th", "28th", "29th", "30th" };
		char new_str[100];
		const char *ptr=text_to_add;
		short unsigned int day=1, month=1, year=0;
		int offset = 0;

		while(!isdigit(ptr[offset]))
		{
			offset++;
			if (offset >= sizeof(new_str))
			{
				LOG_ERROR("error (1) parsing date string: %s",text_to_add);
				//something evil this way comes...
				return 0;
			}
		}
		ptr += offset;

		if (sscanf (ptr,"%hu%*[-/]%hu%*[-/]%hu",&day,&month,&year) < 3
		    || day <= 0 || month <= 0
		    || day > 30 || month > 12 || year > 9999)
		{
			LOG_ERROR("error (2) parsing date string: %s",text_to_add);
			//something evil this way comes...
		}
		else
		{
			// only display initial or "#date" user requested date
			if (!set_date(ptr))
			{
				safe_snprintf(new_str, sizeof(new_str), date_format, day_names[day-1], month_names[month-1], year);
				LOG_TO_CONSOLE(c_green1, new_str);
			}

			//Calculate fraction Big Lunar month (2 conjunction months) less game clock time
			//Represented in Degrees.
			skybox_time_d = (SDL_GetTicks()%( 1296000 * 1000 ));
			skybox_time_d *= 360.0/( 1296000.0 * 1000.0);
			skybox_time_d = -skybox_time_d;
			skybox_time_d += 360.0 * (((month%2)*30 + day-1)*360 + game_minute)/21600.0;
			skybox_update_positions();
			return 0;
		}
	}

	if (from_color_char (text_to_add[0]) == c_green1 && my_strncompare(text_to_add+1,"Game Time", 9))
	{
		real_game_second = atoi(&text_to_add[18]);
		next_second_time = cur_time + 1000;
        new_second();
	}

	// Check for local messages to be translated into actor movements (contains [somthing])
	if (channel == CHAT_LOCAL)
	{
		if(parse_text_for_emote_commands(text_to_add, len)) return 0;
	}

	if (channel == CHAT_SERVER) {
		if (my_strncompare(text_to_add+1, "You started to harvest ", 23)) {
			strncpy(harvest_name, text_to_add+1+23, len-1-23-1);
			harvest_name[len-1-23-1] = '\0';
			harvesting = 1;
		}
		else if ((my_strncompare(text_to_add+1, "You stopped harvesting.", 23)) ||
			(my_strncompare(text_to_add+1, "You can't harvest while fighting (duh)!", 39)) ||
			(my_strncompare(text_to_add+1, "You can't do that while trading!", 32)) ||
			(my_strncompare(text_to_add+1, "You are too far away! Get closer!", 33)) ||
			(my_strncompare(text_to_add+1, "You can't harvest here", 22)) ||
			(my_strncompare(text_to_add+1, "You lack the knowledge of ", 26)) ||
			((my_strncompare(text_to_add+1, "You need to wear ", 17) && strstr(text_to_add, "order to harvest") != NULL)) ||
			((my_strncompare(text_to_add+1, "You need to have a ", 19) && strstr(text_to_add, "order to harvest") != NULL)))
		{
			harvesting = 0;
		}
		else if (is_death_message(text_to_add+1)) {
			// nothing to be done here cause all is done in the test function
		}
		else if (my_strncompare(text_to_add+1, "You found ", 10) && strstr(text_to_add+1, " coins.")) {
			decrement_harvest_counter(atoi(text_to_add+11));
		} 
		else if (my_strncompare(text_to_add+1, "Send Item UIDs ", 15)) {
			if (text_to_add[1+15] == '0')
				item_uid_enabled = 0;
			else if (text_to_add[1+15] == '1')
				item_uid_enabled = 1;
			printf("item_uid_enabled=%d\n", item_uid_enabled);
		}
		else if ((copy_next_LOCATE_ME > 0) && my_strncompare(text_to_add+1, "You are in ", 11)) {
			char buffer[4096];
			switch (copy_next_LOCATE_ME)
			{
				case 1:
					copy_to_clipboard(text_to_add+1);
					break;
				case 2:
					snprintf(buffer, sizeof(buffer), "@My Position: %s", text_to_add + 12);
					send_input_text_line(buffer, strlen(buffer));
					break;
			}
			copy_next_LOCATE_ME = 0;
			return 0;
		}
		else if (my_strncompare(text_to_add+1, "You see: ", 9)) {
			achievements_player_name(text_to_add+10, len-10);
		}
		else if (my_strncompare(text_to_add+1, "You just got food poisoned!", 27)) {
			increment_poison_incidence();
		}
		else if (strstr(text_to_add+1, "aborted the trade.")) {
			trade_aborted(text_to_add+1);
		}
		else if (strstr(text_to_add+1, "Trade session failed")) {
			trade_aborted(text_to_add+1);
		}
		else if (strstr(text_to_add+1, "You have been saved!")) {
			last_save_time = time(NULL);
		}
		
	} else if (channel == CHAT_LOCAL) {
		if (harvesting && my_strncompare(text_to_add+1, username_str, strlen(username_str))) {
			char *ptr = text_to_add+1+strlen(username_str);
			if (my_strncompare(ptr, " found a ", 9)) {
				ptr += 9;
				if (my_strncompare(ptr, "bag of gold, getting ", 21)) {
					decrement_harvest_counter(atoi(ptr+21));
				} else if (!strstr(ptr, " could not carry ")) {
					decrement_harvest_counter(1);
				}
			}
		} else if (my_strncompare(text_to_add+1, "(*) ", 4)) {
			increment_summon_counter(text_to_add+1+4);
			if (summoning_filter) return 0;
		}
	}
	/* check for misc counter strings */
	catch_counters_text(text_to_add+1);

	/* put #mpm in a popup box, on top of all else */
	if ((channel == CHAT_MODPM) && (my_strncompare(text_to_add+1, "[Mod PM from", 12))) {
		display_server_popup_win(text_to_add);
	}

	//Make sure we don't check our own messages.
	if( !(channel == CHAT_PERSONAL && len >= strlen(pm_from_str) && strncasecmp (text_to_add+1, pm_from_str, strlen(pm_from_str)) != 0) &&
		!(channel == CHAT_MODPM && len >= strlen(mod_pm_from_str) && strncasecmp (text_to_add+1, mod_pm_from_str, strlen(mod_pm_from_str)) != 0)
	) {

		//check if ignored - pre_check_if_ignored() checks for Mod PM's etc to not ignore (or it  would be asking for trouble)
		if (pre_check_if_ignored (text_to_add, len, channel))
		{
			return 0;
		}
		//All right, we do not ignore the person
		if (afk)
		{
			if (channel == CHAT_PERSONAL || channel == CHAT_MODPM)
			{
				// player sent us a PM
				add_message_to_pm_log (text_to_add, len, channel);
				if (afk_snd_warning) {
					do_afk_sound();
				}
			}
			else if (channel == CHAT_LOCAL && from_color_char (text_to_add[0]) == c_grey1 && is_talking_about_me (&text_to_add[1], len-1, 0))
			{
				// player mentions our name in local chat
				if (afk_local) {
					add_message_to_pm_log (&text_to_add[1], len - 1, channel);
					if (afk_snd_warning) {
						do_afk_sound();
					}
				} else {
					send_afk_message (&text_to_add[1], len - 1, channel);
				}
			}
			else if (channel == CHAT_SERVER)
			{
				// check if this was a trade attempt
				int i;
				for (i = 1; i < len; i++) {
					if (text_to_add[i] == ' ' || text_to_add[i] == ':' || is_color (text_to_add[i])) {
						break;
					}
				}
				if (i < len-15 && strncasecmp (&text_to_add[i], " wants to trade", 15) == 0) {
					send_afk_message (&text_to_add[1], len - 1, channel);
					if (afk_snd_warning) {
						do_afk_sound();
					}
				}
			}
		}
	} else {	//We sent this PM or MODPM. Can we expect a reply?
		int len = 0;
		char name[MAX_USERNAME_LENGTH];
		for(;text_to_add[len+8] != ':' && len < MAX_USERNAME_LENGTH - 1; ++len);
		safe_strncpy(name, text_to_add+8, len+1);
		if(check_if_ignored(name)){
			char msg[65];
			safe_snprintf(msg, sizeof(msg), warn_currently_ignoring, name);
			LOG_TO_CONSOLE(c_red2, msg);
		}
	}

	// parse for URLs
	find_all_url (text_to_add, len);

	// look for buddy-wants-to-add-you messages
	if(channel == CHAT_SERVER && from_color_char (text_to_add[0]) == c_green1)
	{
		for (l = 1; l < len; l++)
		{
			if (text_to_add[l] == ' ') break;
		}
		if (len - l >= strlen(msg_accept_buddy_str) && strncmp (&text_to_add[l], msg_accept_buddy_str, strlen(msg_accept_buddy_str)) == 0 && l <=32)
		{
			char name[32];
			int i;
			int cur_char;
			/*entropy says: I really fail to understand the logic of that safe_snprintf. And gcc can't understand it either
			  because the name is corrupted. so we implement it the old fashioned way.
			  Grum responds: actually it's the MingW compiler on windows that doesn't understand it, because it doesn't
			  terminate the string when the buffer threatens to overflow. It works fine with gcc on Unix, and using
			  sane_safe_snprintf should also fix it on windows.
			safe_snprintf (name, l, "%s", &text_to_add[1]);
			*/
			for (i = 0; i < sizeof (name); i++)
			{
				cur_char = text_to_add[i+1];
				name[i] = cur_char;
				if (cur_char == ' ')
				{
					name[i] = '\0';
					break;
				}
			}
			add_buddy_confirmation (name);
		}
	}

	// look for astrology messages
	if((channel == CHAT_SERVER) && is_astrology_message (text_to_add))
	{
		return 0;
	}

	// filter any naughty words out
	return filter_text (text_to_add, len, size);
}

void put_text_in_buffer (Uint8 channel, const Uint8 *text_to_add, int len)
{
	put_colored_text_in_buffer (c_grey1, channel, text_to_add, len);
}

//-- Logan Dugenoux [5/26/2004]
// Checks chat string, if it begins with an actor name,
// and the actor is displayed, put said sentence into an overtext bubble
#define ALLOWED_CHAR_IN_NAME(_x_)		(isalnum(_x_)||(_x_=='_'))
void check_chat_text_to_overtext (const Uint8 *text_to_add, int len, Uint8 channel)
{
	if (!view_chat_text_as_overtext || channel != CHAT_LOCAL)
		return;		// disabled

	if (from_color_char (text_to_add[0]) == c_grey1)
	{
		char playerName[128];
		char textbuffer[1024];
		int i;
		int j;

		j = 0;
		i = 1;
		while (text_to_add[i] < 128 && i < len)
		{
			if (text_to_add[i] != '[')
			{
				playerName[j] = (char)text_to_add[i];
				j++;
				if (j >= sizeof (playerName))
					return;//over buffer
			}
			i++;
		}

		if (i < len)
		{
			playerName[j] = '\0';
			while ( j > 0 && !ALLOWED_CHAR_IN_NAME (playerName[j]) )
				playerName[j--] = '\0';
			j = 0;
			while (i < len)
			{
				if ( j >= sizeof (textbuffer) )
					return;//over buffer
				textbuffer[j] = (char) text_to_add[i];
				i++; j++;
			}
			textbuffer[j] = '\0';
			for (i = 0; i < max_actors; i++)
			{
				char actorName[128];
				j = 0;
				// Strip clan info
				while ( ALLOWED_CHAR_IN_NAME (actors_list[i]->actor_name[j]) )
				{
					actorName[j] = actors_list[i]->actor_name[j];
					j++;
					if ( j >= sizeof (actorName) )
						return;	// over buffer
				}
				actorName[j] = '\0';
				if (strcmp (actorName, playerName) == 0)
				{
					add_displayed_text_to_actor (actors_list[i], textbuffer);
					break;
				}
			}
		}
	}
}

int put_char_in_buffer (text_message *buf, Uint8 ch, int pos)
{
	int i, nlen;

	if (pos < 0 || pos > buf->len || pos >= buf->size) {
		return 0;
	}

	// First shift everything after pos to the right
	nlen = buf->len + 1;
	if (nlen >= buf->size)
		nlen = buf->size - 1;
	buf->data[nlen] = '\0';
	for (i = nlen - 1; i > pos; i--)
		buf->data[i] = buf->data[i-1];

	// insert the new character, and update the length
	buf->data[pos] = ch;
	buf->len = nlen;

	return 1;
}

int put_string_in_buffer (text_message *buf, const Uint8 *str, int pos)
{
	int nr_free, nr_paste, ib, jb, nb;
	Uint8 ch;

	if (pos < 0 || pos > buf->len) return 0;
	if (str == NULL) return 0;

	// find out how many characters to paste
	nr_free = buf->size - pos - 1;
	nr_paste = 0;
	for (ib = 0; str[ib] && nr_paste < nr_free; ib++)
	{
		ch = str[ib];
		if (is_printable (ch) || ch == '\n')
			nr_paste++;
	}

	if (nr_paste == 0) {
		return 0;
	}

	// now move the characters right of the cursor (if any)
	nb = buf->len - pos;
	if (nb > nr_free - nr_paste)
		nb = nr_free - nr_paste;
	if (nb > 0)
	{
		for (ib = nb-1; ib >= 0; ib--)
			buf->data[pos+ib+nr_paste] = buf->data[pos+ib];
	}
	buf->data[pos+nb+nr_paste] = '\0';
	buf->len = pos+nb+nr_paste;

	// insert the pasted text
	jb = 0;
	for (ib = 0; str[ib]; ib++)
	{
		ch = str[ib];
		if (ch == '\n')
			ch = ' ';
		if (is_printable (ch))
		{
			buf->data[pos+jb] = ch;
			if (++jb >= nr_paste) break;
		}
	}

	return nr_paste;
}

void put_colored_text_in_buffer (Uint8 color, Uint8 channel, const Uint8 *text_to_add, int len)
{
	text_message *msg;
	int minlen, text_color;
	Uint32 cnr = 0, ibreak = -1, jbreak = -1;
	char time_stamp[12];
	struct tm *l_time; time_t c_time;

	check_chat_text_to_overtext (text_to_add, len, channel);

	// check for auto-length
	if (len < 0)
		len = strlen ((char*)text_to_add);

	// set the time when we got this message
	last_server_message_time = cur_time;

	// if the buffer is full, delete some old lines and move the remainder to the front
	if (++last_message >= DISPLAY_TEXT_BUFFER_SIZE)
	{
		const size_t num_move = DISPLAY_TEXT_BUFFER_SIZE - DISPLAY_TEXT_BUFFER_DEL;
		size_t i;
		for (i=0; i<DISPLAY_TEXT_BUFFER_DEL; i++)
		{
			msg = &(display_text_buffer[i]);
			if (msg->data)
			{
				msg->deleted = 1;
				update_text_windows(msg);
				free_text_message_data (msg);
			}
		}
		memmove(display_text_buffer, &display_text_buffer[DISPLAY_TEXT_BUFFER_DEL], sizeof(text_message)*num_move);
		last_message -= DISPLAY_TEXT_BUFFER_DEL;
		for (i = num_move; i < DISPLAY_TEXT_BUFFER_SIZE; i++)
			init_text_message (display_text_buffer + i, 0);
	}

	msg = &(display_text_buffer[last_message]);

	// Try to make a guess at the number of wrapping newlines required,
	// but allow al least for a null byte and up to 8 extra newlines and
	// colour codes
	minlen = len + 18 + (len/60);
	if (show_timestamp)
	{
		minlen += 12;
		time (&c_time);
		l_time = localtime (&c_time);
		strftime (time_stamp, sizeof(time_stamp), "[%H:%M:%S] ", l_time);
	}
	cnr = get_active_channel (channel);
	if (cnr != 0)
		// allow some space for the channel number
		minlen += 20;
	if (msg->data == NULL)
		alloc_text_message_data (msg, minlen);
	else
		resize_text_message_data (msg, minlen);

	if (cnr != 0)
	{
		for (ibreak = 0; ibreak < len; ibreak++)
		{
			if (text_to_add[ibreak] == ']') break;
		}
	}

	if (channel == CHAT_LOCAL)
	{
		for (jbreak = 0; jbreak < len; jbreak++)
		{
			if (text_to_add[jbreak] == ':' && text_to_add[jbreak+1] == ' ') break;
		}
	}

	if (dark_channeltext==1)
		text_color = c_grey2;
	else if (dark_channeltext==2)
		text_color = c_grey4;

	if (ibreak >= len)
	{
		// not a channel, or something's messed up
		if(!is_color (text_to_add[0]))
		{
			// force the color
			if (show_timestamp)
			{
				safe_snprintf (msg->data, msg->size, "%c%s%.*s", to_color_char (color), time_stamp, len, text_to_add);
			}
			else
			{
				safe_snprintf (msg->data, msg->size, "%c%.*s", to_color_char (color), len, text_to_add);
			}
		}
		else
		{
			// color set by server
			if (show_timestamp)
			{
				if(dark_channeltext && channel==CHAT_LOCAL && from_color_char(text_to_add[0])==c_grey1 && jbreak < (len-3))
				{
					safe_snprintf (msg->data, msg->size, "%c%s%.*s%.*s", to_color_char (text_color), time_stamp, jbreak+1, &text_to_add[1], len-jbreak-3, &text_to_add[jbreak+3]);
				}
				else
				{
					safe_snprintf (msg->data, msg->size, "%c%s%.*s", text_to_add[0], time_stamp, len-1, &text_to_add[1]);
				}
			}
			else
			{
				if(dark_channeltext && channel==CHAT_LOCAL && from_color_char(text_to_add[0])==c_grey1 && jbreak < (len-3))
				{
					safe_snprintf (msg->data, msg->size, "%c%.*s%.*s", to_color_char (text_color), jbreak+1, &text_to_add[1], len-jbreak-3, &text_to_add[jbreak+3]);
				}
				else
				{
					safe_snprintf (msg->data, msg->size, "%.*s", len, text_to_add);
				}
			}
		}
	}
	else
	{
		char nr_str[16];
		int has_additional_color = is_color(text_to_add[ibreak+3]);
		if (cnr >= 1000000000)
			safe_snprintf (nr_str, sizeof (nr_str), "guild");
		else
			safe_snprintf (nr_str, sizeof (nr_str), "%u", cnr);

		if(!is_color (text_to_add[0]))
		{
			// force the color
			if (show_timestamp)
			{
				if (dark_channeltext)
				{
					safe_snprintf (msg->data, msg->size, "%c%s%.*s @ %s%.*s%c%.*s", to_color_char (color), time_stamp, ibreak, text_to_add, nr_str, 3, &text_to_add[ibreak], to_color_char (text_color), len-ibreak-3-has_additional_color, &text_to_add[ibreak+3+has_additional_color]);
				}
				else
				{
					safe_snprintf (msg->data, msg->size, "%c%s%.*s @ %s%.*s", to_color_char (color), time_stamp, ibreak, text_to_add, nr_str, len-ibreak, &text_to_add[ibreak]);
				}
			}
			else
			{
				if (dark_channeltext)
				{
					safe_snprintf (msg->data, msg->size, "%c%.*s @ %s%.*s%c%.*s", to_color_char (color), ibreak, text_to_add, nr_str, 3, &text_to_add[ibreak], to_color_char (text_color), len-ibreak-3-has_additional_color, &text_to_add[ibreak+3+has_additional_color]);
				}
				else
				{
					safe_snprintf (msg->data, msg->size, "%c%.*s @ %s%.*s", to_color_char (color), ibreak, text_to_add, nr_str, len-ibreak, &text_to_add[ibreak]);
				}
			}
		}
		else
		{
			// color set by server
			if (show_timestamp)
			{
				if (dark_channeltext)
				{
					safe_snprintf (msg->data, msg->size, "%c%s%.*s @ %s%.*s%c%.*s", text_to_add[0], time_stamp, ibreak-1, &text_to_add[1], nr_str, 3, &text_to_add[ibreak], to_color_char (text_color), len-ibreak-3-has_additional_color, &text_to_add[ibreak+3+has_additional_color]);
				}
				else
				{
					safe_snprintf (msg->data, msg->size, "%c%s%.*s @ %s%.*s", text_to_add[0], time_stamp, ibreak-1, &text_to_add[1], nr_str, len-ibreak, &text_to_add[ibreak]);
				}
			}
			else
			{
				if (dark_channeltext)
				{
					safe_snprintf (msg->data, msg->size, "%.*s @ %s%.*s%c%.*s", ibreak, text_to_add, nr_str, 3, &text_to_add[ibreak], to_color_char (text_color), len-ibreak-3-has_additional_color, &text_to_add[ibreak+3+has_additional_color]);
				}
				else
				{
					safe_snprintf (msg->data, msg->size, "%.*s @ %s%.*s", ibreak, text_to_add, nr_str, len-ibreak, &text_to_add[ibreak]);
				}
			}
		}
	}

	msg->len = strlen (msg->data);
	msg->chan_idx = channel;
	msg->channel = cnr;

	// set invalid wrap data to force rewrapping
	msg->wrap_lines = 0;
	msg->wrap_zoom = 0.0f;
	msg->wrap_width = 0;

	msg->deleted = 0;
	recolour_message(msg);
	update_text_windows(msg);

	// log the message
	write_to_log (channel, (unsigned char*)msg->data, msg->len);

	return;
}

void put_small_text_in_box (const Uint8 *text_to_add, int len, int pixels_limit, char *buffer)
{
	put_small_colored_text_in_box (c_grey1, text_to_add, len, pixels_limit, buffer);
}

void put_small_colored_text_in_box (Uint8 color, const Uint8 *text_to_add, int len, int pixels_limit, char *buffer)
{
	int i;
	Uint8 cur_char;
	int last_text = 0;
	int x_chars_limit;

	// force the color
	if (!is_color (text_to_add[0]))
		buffer[last_text++] = to_color_char (color);

	//see if the text fits on the screen
	x_chars_limit = pixels_limit / 8;
	if (len <= x_chars_limit)
	{
		for (i = 0; i < len; i++)
		{
			cur_char = text_to_add[i];

			if (cur_char == '\0')
				break;

			buffer[last_text++] = cur_char;
		}
		if (last_text > 0 && buffer[last_text-1] != '\n')
			buffer[last_text++] = '\n';
		buffer[last_text] = '\0';
	}
	else //we have to add new lines to our text...
	{
		int k;
		int new_line_pos = 0;
		char semaphore = 0;
		Uint8 current_color = to_color_char (color);

		// go trought all the text
		for (i = 0; i < len; i++)
		{
			if (!semaphore && new_line_pos + x_chars_limit < len) //don't go through the last line
			{
				//find the closest space from the end of this line
				//if we have one really big word, then parse the string from the
				//end of the line backwards, untill the beginning of the line +2
				//the +2 is so we avoid parsing the ": " thing...
				for (k = new_line_pos + x_chars_limit - 1; k > new_line_pos + 2; k--)
				{
					cur_char = text_to_add[k];
					if (k > len) continue;
					if (cur_char == ' ' || cur_char == '\n')
					{
						k++; // let the space on the previous line
						break;
					}
				}
				if (k == new_line_pos + 2)
					new_line_pos += x_chars_limit;
				else
					new_line_pos = k;
				semaphore = 1;
			}

			cur_char = text_to_add[i];
			if (cur_char == '\0') break;

			if (is_color (cur_char)) // we have a color, save it
			{
				current_color = cur_char;
				if (last_text > 0 && is_color (buffer[last_text-1]))
					last_text--;
			}
			else if (cur_char == '\n')
			{
				new_line_pos = i;
			}

			if (i == new_line_pos)
			{
				buffer[last_text++] = '\n';
				// don't add color codes after the last newline
				if (i < len-1)
					buffer[last_text++] = current_color;
				semaphore = 0;
			}
			//don't add another new line, if the current char is already a new line...
			if (cur_char != '\n')
				buffer[last_text++] = cur_char;

		}
		// don't add extra newlines if there already is one
		if (last_text > 0 && buffer[last_text-1] != '\n')
			buffer[last_text++] = '\n';
		buffer[last_text] = '\0';
	}
}


// find the last lines, according to the current time
int find_last_lines_time (int *msg, int *offset, Uint8 filter, int width)
{
	// adjust the lines_no according to the time elapsed since the last message
	if ( (cur_time - last_server_message_time) / 1000 > 3)
	{
		if (lines_to_show > 0)
			lines_to_show--;
		last_server_message_time = cur_time;
	}
	if (lines_to_show <= 0) return 0;

	return find_line_nr (get_total_nr_lines(), get_total_nr_lines() - lines_to_show, filter, msg, offset, chat_zoom, width);
}


int find_line_nr (int nr_lines, int line, Uint8 filter, int *msg, int *offset, float zoom, int width)
{
	int line_count = 0, lines_no = nr_lines - line;
	int imsg, ichar;
	char *data;

	imsg = last_message;
	if ( imsg<0 ) {
		/* No data in buffer */
		*msg = *offset = 0;
		return 1;
	}
	do
	{
		int msgchan = display_text_buffer[imsg].chan_idx;

		switch (msgchan) {
			case CHAT_LOCAL:    if (!local_chat_separate)    msgchan = CHAT_ALL; break;
			case CHAT_PERSONAL: if (!personal_chat_separate) msgchan = CHAT_ALL; break;
			case CHAT_GM:       if (!guild_chat_separate)    msgchan = CHAT_ALL; break;
			case CHAT_SERVER:   if (!server_chat_separate)   msgchan = CHAT_ALL; break;
			case CHAT_MOD:      if (!mod_chat_separate)      msgchan = CHAT_ALL; break;
			case CHAT_MODPM:                                 msgchan = CHAT_ALL; break;
		}

		if (msgchan == filter || msgchan == CHAT_ALL || filter == FILTER_ALL)
		{
			data = display_text_buffer[imsg].data;
			if (data == NULL)
				// Hmmm... we messed up. This should not be
				// happening.
				break;

			rewrap_message(&display_text_buffer[imsg], zoom, width, NULL);

			for (ichar = display_text_buffer[imsg].len - 1; ichar >= 0; ichar--)
			{
				if (data[ichar] == '\n' || data[ichar] == '\r')
				{
					line_count++;
					if (line_count >= lines_no)
					{
						*msg = imsg;
						*offset = ichar+1;
						return 1;
					}
				}
			}

			line_count++;
			if (line_count >= lines_no)
			{
				*msg = imsg;
				*offset = 0;
				return 1;
			}
		}

		--imsg;

	} while (imsg >= 0 && imsg != last_message);

	*msg = 0;
	*offset = 0;
	return 1;
}

void clear_display_text_buffer ()
{
	int i;
	for (i = 0; i < DISPLAY_TEXT_BUFFER_SIZE; ++i)
	{
		if (!display_text_buffer[i].deleted && display_text_buffer[i].data != NULL && display_text_buffer[i].data[0] != '\0'){
			free_text_message_data (display_text_buffer + i);
		}
		display_text_buffer[i].deleted= 1;
	}

	last_message = -1;
	last_server_message_time = cur_time;

	clear_console();
	if(use_windowed_chat == 2){
		clear_chat_wins();
	}
}

int rewrap_message(text_message * msg, float zoom, int width, int * cursor)
{
	int nlines;
	float max_line_width = 0;

	if (msg == NULL || msg->data == NULL || msg->deleted)
		return 0;

	if (msg->wrap_width != width || msg->wrap_zoom != zoom)
	{
 		nlines = reset_soft_breaks(msg->data, msg->len, msg->size, zoom, width, cursor, &max_line_width);
		msg->len = strlen(msg->data);
		msg->wrap_lines = nlines;
		msg->wrap_width = width;
		msg->wrap_zoom = zoom;
		msg->max_line_width = max_line_width;
	} else {
		nlines = msg->wrap_lines;
	}

	return nlines;
}
