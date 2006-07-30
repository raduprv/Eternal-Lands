#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "global.h"
#include "chat.h"

text_message display_text_buffer[DISPLAY_TEXT_BUFFER_SIZE];
int last_message = -1;
int buffer_full = 0;
int total_nr_lines = 0;
Uint8 current_filter = FILTER_ALL;

int console_msg_nr = 0;
int console_msg_offset = 0;

text_message input_text_line;

char last_pm_from[32];

Uint32 last_server_message_time;
int lines_to_show=0;
int max_lines_no=10;

char not_from_the_end_console=0;

int log_chat = 2;

float	chat_zoom=1.0;
FILE	*chat_log=NULL;
FILE	*srv_log=NULL;

/* forward declaration */
void put_small_colored_text_in_box (Uint8 color, const Uint8 *text_to_add, int len, int pixels_limit, char *buffer);

void init_text_buffers ()
{
	memset ( display_text_buffer, 0, sizeof (display_text_buffer) );
	input_text_line.chan_idx = CHAT_ALL;
	input_text_line.len = 0;
	input_text_line.size = MAX_TEXT_MESSAGE_LENGTH + 1;
	input_text_line.data = malloc (input_text_line.size);
	input_text_line.data[0] = '\0';
}

void cleanup_text_buffers(void)
{
	int i;

	free(input_text_line.data);
	for(i = 0; i < DISPLAY_TEXT_BUFFER_SIZE; i++) {
		if(display_text_buffer[i].data != NULL) {
			free(display_text_buffer[i].data);
		}
	}
}

void update_text_windows (text_message * pmsg)
{
	if (console_root_win >= 0) update_console_win (pmsg);
	switch (use_windowed_chat) {
		case 0:
			rewrap_message(pmsg, chat_zoom, console_text_width, NULL);
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

void write_to_log (Uint8 *data, int len)
{
	int i, j;
	Uint8 ch;
	char str[4096];
	Uint8 starttime[200];
	struct tm *l_time; time_t c_time;
	char logmsg[4096];

	int server_message = 0;

	if(log_chat == 0) {
		return; //we're not logging anything
	}

	if (chat_log == NULL)
	{
		char chat_log_file[100];
		char srv_log_file[100];
#ifndef WINDOWS
		snprintf (chat_log_file, sizeof (chat_log_file),  "%s/chat_log.txt", configdir);
		snprintf (srv_log_file, sizeof (srv_log_file), "%s/srv_log.txt", configdir);
#else
		strcpy (chat_log_file, "chat_log.txt");
		strcpy (srv_log_file, "srv_log.txt");
#endif
  		chat_log = my_fopen (chat_log_file, "a");
  		srv_log = my_fopen (srv_log_file, "a");

		if (chat_log == NULL || srv_log == NULL)
		{
			// quit to prevent error log filling up with messages
			// caused by unability to open chat log
			//SDL_Quit ();	//We don't need to do this anymore for chat_log
			//exit (1);
			LOG_TO_CONSOLE(c_red3, "Unable to open log file to write. We will NOT be recording anything.");
			log_chat=0;
			return;
		}
		time(&c_time);
		l_time = localtime(&c_time);
		strftime(starttime, sizeof(starttime), "\n\nLog started at %Y-%m-%d %H:%M:%S localtime", l_time);
		snprintf(starttime, sizeof(starttime), "%s (%s)\n\n", starttime, tzname[l_time->tm_isdst>0]);
		if(log_chat>=3){
			fwrite (starttime, strlen(starttime), 1, chat_log);
		}
		fwrite (starttime, strlen(starttime), 1, chat_log);
	}

	j=0;
	for (i = 0; i < len && j < sizeof (str) - 1; i++)
	{
		ch = data[i];
		// remove colorization when writting to the chat log
		// Grum: don't log '\r'
		if ( (ch < 127 || ch > 127 + c_grey4) && ch != '\r')
		{
			str[j]=ch;
			j++;
		}
		else if (ch != 133 && ch != 129 && ch != 128)
		{
			server_message = 1;
		}
	}
	str[j++]='\n';
	str[j++]='\0';

	time(&c_time);
	l_time = localtime(&c_time);
	strftime(logmsg, sizeof(logmsg), "[%H:%M:%S] ", l_time);
	strcat(logmsg, str);

	if (server_message && log_chat>=3)
	{
		fwrite(logmsg, strlen(logmsg), 1, srv_log);
	}
	else if (!server_message || log_chat==2
		|| (log_chat == 1 && ((!server_message)||(!strncmp(str, "#GM ", 4))||(!strncmp(str, "#Mod ", 5))))
		)
	{
		fwrite(logmsg, strlen(logmsg), 1, chat_log);
	}
	fflush(chat_log);
  	fflush(srv_log);
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
	if (my_tcp_send (my_socket, str, len+1) < len+1)
	{
		//we got a nasty error, log it
	}

	return;
}

int filter_or_ignore_text (Uint8 *text_to_add, int len, int size, Uint8 channel)
{
	int l, idx;

	if (len <= 0) return 0;	// no point

	//check for auto receiving #help
	for (idx = 0; idx < len; idx++)
	{
		if (!IS_COLOR (text_to_add[idx])) break;
	}
	l = len - idx;
	if (l >= strlen(help_request_str) && text_to_add[idx] == '#' && (strncasecmp (&text_to_add[idx], help_request_str, strlen(help_request_str)) == 0 || strncasecmp (&text_to_add[idx], "#mod chat", 9) == 0))
	{
		auto_open_encyclopedia = 0;
	}
	if(my_strncompare(text_to_add+1,"Game Date", 9))
	{
		//we assume that the server will still send little-endian dd/mm/yyyy... we could make it safer by parsing the format too, but it's simpler to assume
		const char * const month_names[] = { "Aluwia", "Seedar", "Akbar", "Zartia", "Elandra", "Viasia", "Fruitfall", "Mortia", "Carnelar", "Nimlos", "Chimar", "Vespia" };
		const char * const day_names[] = { "1st", "2nd", "3rd", "4th", "5th", "6th", "7th", "8th", "9th", "10th", "11th", "12th", "13th", "14th", "15th", "16th", "17th", "18th", "19th", "20th", "21st", "22nd", "23rd", "24th", "25th", "26th", "27th", "28th", "29th", "30th" };
		char new_str[100];
		const Uint8 *ptr=text_to_add;
		short unsigned int day=1, month=1, year=0;

		while(!isdigit(*ptr)){ptr++;}

		if( ( sscanf(ptr,"%hu%*[-/]%hu%*[-/]%hu",&day,&month,&year) < 3 )
				|| ( day > 30 || month > 12 || year > 9999 ) ){
			LOG_ERROR("error parsing date string: %s",text_to_add);
			//something evil this way comes...
		}else{
			snprintf(new_str, sizeof(new_str), date_format, day_names[day-1], month_names[month-1], year);
			LOG_TO_CONSOLE(c_green1, new_str);
			return 0;
		}
	}

#ifdef COUNTERS
	if (channel == CHAT_SERVER) {
		if (my_strncompare(text_to_add+1, "You started to harvest ", 23)) {
			strncpy(harvest_name, text_to_add+1+23, len-1-23-1);
			harvest_name[len-1-23-1] = '\0';
			harvesting = 1;
		} else if (my_strncompare(text_to_add+1, "You stopped harvesting.", 23)) {
			harvesting = 0;
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
		}
	}
#endif

	//check if ignored
	//Make sure we don't check our own messages.
	if( !(channel == CHAT_PERSONAL && len >= strlen(pm_from_str) && strncasecmp (text_to_add+1, pm_from_str, strlen(pm_from_str)) != 0) &&
		!(channel == CHAT_MODPM && len >= strlen(mod_pm_from_str) && strncasecmp (text_to_add+1, mod_pm_from_str, strlen(mod_pm_from_str)) != 0)
	) {
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
#ifndef AFK_FIX
				add_message_to_pm_log (text_to_add, len);
#else
				add_message_to_pm_log (text_to_add, len, channel);
#endif //AFK_FIX
			}
			else if (channel == CHAT_LOCAL && text_to_add[0] == 127 + c_grey1 && is_talking_about_me (&text_to_add[1], len-1, 0))
			{
				// player mentions our name in local chat
#ifndef AFK_FIX
				send_afk_message (&text_to_add[1], len - 1, channel);
#else
				if (afk_local) {
					add_message_to_pm_log (&text_to_add[1], len - 1, channel);
				} else {
					send_afk_message (&text_to_add[1], len - 1, channel);
				}
#endif //AFK_FIX
			}
			else if (channel == CHAT_SERVER)
			{
				// check if this was a trade attempt
				int i;
				for (i = 1; i < len; i++) {
					if (text_to_add[i] == ' ' || text_to_add[i] == ':' || IS_COLOR (text_to_add[i])) {
						break;
					}
				}
				if (i < len-15 && strncasecmp (&text_to_add[i], " wants to trade", 15) == 0) {
					send_afk_message (&text_to_add[1], len - 1, channel);
				}
			}
		}
	}

	// parse for URLs
	find_last_url (text_to_add, len);

	// look for buddy-wants-to-add-you messages
	if(channel == CHAT_SERVER && text_to_add[0] == c_green1+127)
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
			/*entropy says: I really fail to understand the logic of that snprintf. And gcc can't understand it either
			  because the name is corrupted. so we implement it the old fashioned way.
			  Grum responds: actually it's the MingW compiler on windows that doesn't understand it, because it doesn't
			  terminate the string when the buffer threatens to overflow. It works fine with gcc on Unix, and using
			  sane_snprintf should also fix it on windows.
			snprintf (name, l, "%s", &text_to_add[1]);
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

	if (text_to_add[0] == 127 + c_grey1)
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
		if (IS_PRINT (ch))
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
		if (IS_PRINT (ch))
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
	int minlen;
	Uint32 cnr = 0, ibreak = -1;

	check_chat_text_to_overtext (text_to_add, len, channel);

	// check for auto-length
	if (len < 0)
		len = strlen (text_to_add);

	// set the time when we got this message
	last_server_message_time = cur_time;

	if (++last_message >= DISPLAY_TEXT_BUFFER_SIZE)
	{
		buffer_full = 1;
		last_message = 0;
	}

	msg = &(display_text_buffer[last_message]);

	if (buffer_full && msg->data) {
		total_nr_lines -= msg->wrap_lines;
		msg->deleted = 1;
		update_text_windows(msg);
	}

	// Allow for a null byte and up to 8 extra newlines and colour codes.
	minlen = len + 18;
	cnr = get_active_channel (channel);
	if (cnr != 0)
		// allow some space for the channel number
		minlen += 20;
	if (msg->data == NULL || msg->size < minlen)
	{
		if (msg->data != NULL) free (msg->data);
		msg->data = malloc (minlen);
		msg->size = minlen;
	}

	if (cnr != 0)
	{
		for (ibreak = 0; ibreak < len; ibreak++)
		{
			if (text_to_add[ibreak] == ']') break;
		}
	}

	if (ibreak < 0 || ibreak >= len)
	{
		// not a channel, or something's messed up
		if(!IS_COLOR(text_to_add[0]))
		{
			// force the color
			snprintf(msg->data, minlen, "%c%.*s", color + 127, len, text_to_add);
		}
		else
		{
			// color set by server
			snprintf(msg->data, minlen, "%.*s", len, text_to_add);
		}
	}
	else
	{
		char nr_str[16];
		if (cnr >= 1000000000)
			snprintf (nr_str, sizeof (nr_str), "guild");
		else
			snprintf (nr_str, sizeof (nr_str), "%u", cnr);

		if(!IS_COLOR(text_to_add[0]))
		{
			// force the color
			snprintf(msg->data, minlen, "%c%.*s @ %s%.*s", color + 127, ibreak, text_to_add, nr_str, len-ibreak, &text_to_add[ibreak]);
		}
		else
		{
			// color set by server
			snprintf(msg->data, minlen, "%.*s @ %s%.*s", ibreak, text_to_add, nr_str, len-ibreak, &text_to_add[ibreak]);
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
	write_to_log (msg->data, msg->len);

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
	if (!IS_COLOR (text_to_add[0]))
		buffer[last_text++] = 127 + color;
	
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
		Uint8 current_color = 127 + color;

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

			if (IS_COLOR (cur_char)) // we have a color, save it
			{
				current_color = cur_char;
				if (last_text > 0 && IS_COLOR ((Uint8)buffer[last_text-1]))
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

	return find_line_nr (total_nr_lines, total_nr_lines - lines_to_show, filter, msg, offset, chat_zoom, width);
}

int find_last_console_lines (int lines_no)
{
	return find_line_nr (total_nr_lines, total_nr_lines - lines_no, FILTER_ALL, &console_msg_nr, &console_msg_offset, chat_zoom, console_text_width);
}


int find_line_nr (int nr_lines, int line, Uint8 filter, int *msg, int *offset, float zoom, int width)
{
	int line_count = 0, lines_no = nr_lines - line;
	int imsg, ichar;
	char *data;
	
	imsg = last_message;
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

		if (--imsg < 0 && buffer_full)
			imsg = DISPLAY_TEXT_BUFFER_SIZE - 1;
	} while (imsg >= 0 && imsg != last_message);

	*msg = 0;
	*offset = 0;
	return 1;
}
void console_move_up ()
{
	int nl_found, ichar;
	int max_lines;

	// get the number of lines we have - the last one, which is the
	// command line
	max_lines = (window_height - hud_y) / 18 - 1;
	if (not_from_the_end_console) max_lines--;

	// if we have less lines of text than the max lines onscreen, don't
	// scroll up
	if (total_nr_lines > max_lines)
	{
		not_from_the_end_console = 1;

		nl_found = 0;
		while (1)
		{
			char *data = display_text_buffer[console_msg_nr].data;
			for (ichar = console_msg_offset; ichar >= 0; ichar--)
			{
				if (data[ichar] == '\n' || data[ichar] == '\r')
				{
					if (nl_found) break;
					nl_found = 1;
				}
			}
			if (nl_found)
			{
				console_msg_offset = ichar + 1;
				return;
			}
			if (--console_msg_nr < 0 && buffer_full)
				console_msg_nr = DISPLAY_TEXT_BUFFER_SIZE - 1;
			nl_found = 1;
		}
	}
}


void console_move_down ()
{
	int ichar;
	int max_lines;
	char *data = display_text_buffer[console_msg_nr].data;

	if (!not_from_the_end_console)
		// we can't scroll down anymore
		return;

	// get the number of lines we have on screen
	max_lines = (window_height-hud_y)/18 - 1;
	max_lines--;

	for (ichar = console_msg_offset; data[ichar] != '\0'; ichar++)
		if (data[ichar] == '\n' || data[ichar] == '\r')
			break;

	if (data[ichar] == '\n' || data[ichar] == '\r')
	{
		console_msg_offset = ichar + 1;
	}
	else
	{
		if (++console_msg_nr >= DISPLAY_TEXT_BUFFER_SIZE)
			console_msg_nr = 0;
		console_msg_offset = 0;
	}

	if (console_msg_nr == last_message)
	{
		data = display_text_buffer[console_msg_nr].data;
		for (ichar = console_msg_offset; data[ichar] != '\0'; ichar++)
			if (data[ichar] == '\n' || data[ichar] == '\r')
				break;
		if (data[ichar] == '\0')
			not_from_the_end_console = 0;
	}
}

void console_move_page_down()
{
	int max_lines;
	int i;

	max_lines=(window_height-hud_y)/18-3;

	for(i=0;i<max_lines;i++)
		{
			console_move_down();
		}
}

void console_move_page_up()
{
	int max_lines;
	int i;

	max_lines=(window_height-hud_y)/18-3;

	for(i=0;i<max_lines;i++)
		{
			console_move_up();
		}
}

// XXX FIXME (Grum): obsolete
/*
void display_console_text ()
{
	int max_lines;
	int command_line_y;
	int nr_command_lines = 1;
	int i;

	for (i = 0; input_text_line.data[i] != '\0'; i++)
		if (input_text_line.data[i] == '\n' || input_text_line.data[i] == '\r')
			nr_command_lines++;

	//get the number of lines we have - the last one, which is the command line
	max_lines = ( window_height - 17 * (2 + nr_command_lines) ) / 18 - 2;
	if (not_from_the_end_console) max_lines--;
	command_line_y = window_height - 17 * (4 + nr_command_lines);

	if(!not_from_the_end_console)
		find_last_console_lines (max_lines);

	draw_messages (0, 0, display_text_buffer, DISPLAY_TEXT_BUFFER_SIZE, console_msg_nr, console_msg_offset, -1, window_width, 17*max_lines, 1.0);

	glColor3f (1.0f, 1.0f, 1.0f);
	if (not_from_the_end_console)
		draw_string (0, command_line_y - 18, "^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^", 2);
	draw_string (0, command_line_y, input_text_line.data, nr_command_lines);
}
*/

void clear_display_text_buffer ()
{
	int i;
	for (i = 0; i < DISPLAY_TEXT_BUFFER_SIZE; ++i){
		if (display_text_buffer[i].data && display_text_buffer[i].data[0] &&
				!display_text_buffer[i].deleted){
			display_text_buffer[i].data[0]= '\0';
			free(display_text_buffer[i].data);
		}
		display_text_buffer[i].deleted= 1;
		display_text_buffer[i].len= 0;
		display_text_buffer[i].size= 0;
		display_text_buffer[i].data= NULL;
	}

	buffer_full= 0;
	console_msg_nr= 0;
	console_msg_offset= 0;
	last_message= -1;
	last_server_message_time= cur_time;
	total_nr_lines= 0;

	clear_console();
	if(use_windowed_chat == 2){
		clear_chat_wins();
	}
}

int rewrap_message(text_message * msg, float zoom, int width, int * cursor)
{
	int nlines;

	if (msg == NULL || msg->data == NULL)
		return 0;
	
	if (msg->wrap_width != width || msg->wrap_zoom != zoom)
	{
		if (msg->chan_idx != CHAT_NONE)
			total_nr_lines -= msg->wrap_lines;
 		nlines = reset_soft_breaks(msg->data, msg->len, msg->size, zoom, width, cursor);
		if (msg->chan_idx != CHAT_NONE)
			total_nr_lines += nlines;
		msg->len = strlen(msg->data);
		msg->wrap_lines = nlines;
		msg->wrap_width = width;
		msg->wrap_zoom = zoom;
	} else {
		nlines = msg->wrap_lines;
	}

	return nlines;
}
