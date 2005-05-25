#include <string.h>
#include <ctype.h>
#include "global.h"

text_message display_text_buffer[DISPLAY_TEXT_BUFFER_SIZE];
int last_message = -1;
int buffer_full = 0;
int total_nr_lines = 0;

int console_msg_nr = 0;
int console_msg_offset = 0;

text_message input_text_line;

char last_pm_from[32];

Uint32 last_server_message_time;
int lines_to_show=0;
int max_lines_no=10;

char not_from_the_end_console=0;

int log_server = 1;

float	chat_zoom=1.0;
FILE	*chat_log=NULL;
FILE	*srv_log=NULL;

/* forward declaration */
void put_small_colored_text_in_box(Uint8 color,unsigned char *text_to_add, int len, int pixels_limit, char *buffer);

void init_text_buffers ()
{
	memset ( display_text_buffer, 0, sizeof (display_text_buffer) );
	input_text_line.chan_nr = CHANNEL_ALL;
	input_text_line.len = 0;
	input_text_line.size = MAX_TEXT_MESSAGE_LENGTH + 1;
	input_text_line.data = malloc (input_text_line.size);
	input_text_line.data[0] = '\0';
}

void update_text_windows (int nlines, int channel)
{
	update_console_win (nlines);
	if (use_windowed_chat)
		update_chat_window (nlines, channel);
}

void write_to_log (Uint8 *data, int len)
{
	int i, j;
	Uint8 ch;
	char str[4096];

	int server_message = 0;

	if (chat_log == NULL)
	{
		char chat_log_file[100];
		char srv_log_file[100];
#ifndef WINDOWS
		strcpy (chat_log_file, configdir);
		strcat (chat_log_file, "chat_log.txt");
		strcpy (srv_log_file, configdir);
		strcat (srv_log_file, "srv_log.txt");
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
			SDL_Quit ();
			exit (1);
		}
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

	if (server_message && log_server==2)
	{
		fwrite(str, j, 1, srv_log);
	}
	else if (!server_message || log_server==1)
	{
		fwrite(str, j, 1, chat_log);
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

	if ( caps_filter && line_len > 4 && my_isupper (line, -1) )
		my_tolower (line);
		
	i=0;
	j=1;
	if (line[0] != '/')	//we don't have a PM
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

int filter_or_ignore_text(unsigned char *text_to_add, int len)
{
	int	l, type;
	unsigned char *ptr;

	//check for auto receiving #help
	for(ptr=text_to_add, l=len; l >0; ptr++, l--){
		if(!(*ptr&0x80))	break;
	}
	if(len > 0 && *ptr == '#' && (!strncasecmp(ptr, "#help request", 9) || !strncasecmp(ptr, "#mod chat", 9))){
		auto_open_encyclopedia= 0;
	}

	//check if ignored
	type=strncasecmp(&text_to_add[1],"[PM from",8)?0:1;
	if(pre_check_if_ignored(text_to_add,type))return 0;
	//All right, we do not ignore the person
	if (afk)
	{
		if (type)
		{
			// player sent us a PM
			add_message_to_pm_log (text_to_add, len);
		}
		else if (text_to_add[0] == (c_grey1+127) && is_talking_about_me ( &text_to_add[1], len-1) )
		{
			// player mentions our name in local chat
			send_afk_message (&text_to_add[1], type);
		}
		else
		{
			// check if this was a trade attempt
			int i;
			for (i = 1; i < len; i++)
				if ( text_to_add[i] == ' ' || text_to_add[i] == ':' || (text_to_add[i] > 127+c_red1 && text_to_add[i] < 127+c_grey4) )
					break;
			if (i < len-15 && strncasecmp (&text_to_add[i], " wants to trade", 15) == 0)
				send_afk_message (&text_to_add[1], type);
		}
	}
	
	// parse for URLs
	find_last_url (text_to_add, len);
	
	// filter any naughty words out
	return filter_text (text_to_add, len);
}

void put_text_in_buffer(unsigned char *text_to_add, int len, int x_chars_limit)
{
	put_colored_text_in_buffer(c_grey1, text_to_add, len, x_chars_limit);
}


//-- Logan Dugenoux [5/26/2004]
// Checks chat string, if it begins with an actor name, 
// and the actor is displayed, put said sentence into an overtext bubble
#define ALLOWED_CHAR_IN_NAME(_x_)		(isalnum(_x_)||(_x_=='_'))
void check_chat_text_to_overtext (unsigned char *text_to_add, int len)
{	
	if (!view_chat_text_as_overtext)
		return;		// disabled

	if (text_to_add[0] == c_grey1)
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
	
	if (pos < 0 || pos > buf->len || pos >= buf->size) return 0;
	
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
		if ( (ch >= 32 && ch <= 126) || ch > 127 + c_grey4)
			nr_paste++;
	}
	
	if (nr_paste == 0) return 0;

	// now move the characters right of the cursor (if any)
	nb = buf->len - pos;
	if (nb > nr_free - nr_paste) nb = nr_free - nr_paste;
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
		if ( (ch >= 32 && ch <= 126) || ch > 127 + c_grey4)
		{
			buf->data[pos+jb] = str[ib];
			if (++jb > nr_paste) break;
		}
	}

	return nr_paste;	
}

void put_colored_text_in_buffer (Uint8 color, unsigned char *text_to_add, int len, int x_chars_limit)
{
	int i;
	int idx;
	Uint8 cur_char;
	text_message *msg;
	int nlines = 0, nltmp;
	int channel = CHANNEL_ALL;
	int minlen;

	check_chat_text_to_overtext (text_to_add, len);
	
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
	if (buffer_full && msg->data)
	{
		nlines--;
		for (i = 0; msg->data[i] != '\0'; i++)
			if (msg->data[i] == '\r' || msg->data[i] == '\n')
				nlines--;
	}
	
	// Allow for a null byte and up to 8 extra newlines and colour codes.
	minlen = len + 18; 
	if (msg->size < minlen)
	{
		if (msg->data != NULL) free (msg->data);
		msg->data = malloc (minlen);
		msg->size = minlen;
	}
	
	idx = 0;
	// force the color
	if(text_to_add[0] < 127 || text_to_add[0] > 127 + c_grey4)
		msg->data[idx++] = 127 + color;
	
	if (use_windowed_chat || x_chars_limit <= 0 || len <= x_chars_limit)
	{
		for (i = 0; i < len; i++)
		{
			if (text_to_add[i] == '\0')
				break;
			msg->data[idx++] = text_to_add[i];
		}
		msg->data[idx++] = '\0';
		
		if (use_windowed_chat)
			nltmp = reset_soft_breaks (msg->data, idx, msg->size, chat_zoom, chat_win_text_width, NULL);
		else if (x_chars_limit <= 0)
			nltmp = reset_soft_breaks (msg->data, idx, msg->size, chat_zoom, window_width - hud_x - 20, NULL);
		else
			nltmp = 1;
		
		msg->len = strlen (msg->data);
		
		nlines += nltmp;
		lines_to_show += nltmp;
		if (lines_to_show > max_lines_no)
			lines_to_show = max_lines_no;
		total_nr_lines += nlines;

		if (use_windowed_chat)
		{
			// determine the proper channel
			// XXX FIXME (Grum): hack
			if (msg->data[0] == 127+c_orange1 && strncmp (&(msg->data[1]), "[PM ", 4) == 0)
			{
				// Personal message
				channel = CHANNEL_ALL;
			} 
			else if (msg->data[0] == 127+c_blue2)
			{
				if (strncmp (&(msg->data[1]), "#GM ", 4) == 0)
				{
					// Guild message
					channel = CHANNEL_GM;
				}
				else if (strncmp (&(msg->data[1]), "#Message ", 9) == 0)
				{
					// Mod message
					channel = CHANNEL_ALL;
				}
				else
				{
					// Unknown, show it as local
					channel = CHANNEL_LOCAL;
				}
			}
			else if (msg->data[0] == 127+c_grey1 && msg->data[1] == '[')
			{
				// Channel chat
				channel = 0;
			}
			else
			{
				// all else, show it as local
				channel = CHANNEL_LOCAL;
			}
		}
		msg->chan_nr = channel;
		update_text_windows (nlines, channel);
		return;
	}
	else
	{
		// not using windowed chat, fixed width specified, and we need 
		// more than one line
		int line = 0;
		int k, j;
		int new_line_pos = 0;
		char semaphore = 0;
		unsigned char current_color = 127 + color;

		//go trought all the text
		j = 0;
		for (i = 0; i < len; i++)
		{
			// don't go trough the last line
			if (!semaphore && new_line_pos + x_chars_limit < len)
			{
				// find the closest space from the end of this line
				// if we have one really big word, then parse the string from the
				// end of the line backwards, until the beginning of the line +2
				// the +2 is so we avoid parsing the ": " thing...
				for (k = new_line_pos + x_chars_limit - 1; k > new_line_pos + 2; k--)
				{
					if (text_to_add[k] == ' ')
					{
						k++; //let the space on the previous line
						break;
					}
				}
				if (k == new_line_pos + 2)
					new_line_pos += x_chars_limit;
				else 
					new_line_pos = k;
				line++;
				semaphore = 1;
			}

			cur_char = text_to_add[i];

			if (cur_char == '\0')
			{
				j--;
				break;
			}

			if (cur_char >= 127 && cur_char <= 127 + c_grey4)	
				//we have a color, save it
				current_color = cur_char;
			else if (cur_char == '\n')
				new_line_pos = i;

			if (i == new_line_pos)
			{
				msg->data[idx++] = '\n';
				j++;
				msg->data[idx++] = current_color;
				j++;
				semaphore = 0;
				if (lines_to_show < max_lines_no)
					lines_to_show++;
				nlines++;
			}
			// don't add another new line, if the current char is already a new line...
			if (cur_char != '\n')
			{
				msg->data[idx++] = cur_char;
				j++;
			}
		}
		msg->data[idx++] = '\0';
		msg->len = idx;
	}

	total_nr_lines += nlines;
	update_text_windows (nlines, channel);
}

void put_small_text_in_box(unsigned char *text_to_add, int len, int pixels_limit, 
						   char *buffer)
{
	put_small_colored_text_in_box(c_grey1, text_to_add, len, pixels_limit, buffer);
}

void put_small_colored_text_in_box(Uint8 color,unsigned char *text_to_add, int len, 
								   int pixels_limit, char *buffer)
{
	int i;
	Uint8 cur_char;
	int last_text=0;
	int x_chars_limit;

	// force the color
	if(*text_to_add < 127 || *text_to_add > 127+c_grey4)
		{
			buffer[last_text]=127+color;
			last_text++;
		}
	//see if the text fits on the screen
	x_chars_limit=pixels_limit/8;
	if(len<=x_chars_limit)
		{
			for(i=0;i<len;i++)
				{
					cur_char=text_to_add[i];

					if(!cur_char)
						{
							i--;
							break;
						}

					buffer[i+last_text]=cur_char;
				}
			buffer[last_text+i]='\n';
			buffer[last_text+i+1]=0;

		}
	else//we have to add new lines to our text...
		{
			int line=0;
			int k,j;
			int new_line_pos=0;
			int text_lines;
			char semaphore=0;
			unsigned char current_color=127+color;

			//how many lines of text do we have?
			text_lines=len/x_chars_limit;
			//go trought all the text
			j=0;
			for(i=0;i<len;i++)
				{
					if(!semaphore && new_line_pos+x_chars_limit<len)//don't go through the last line
						{
							//find the closest space from the end of this line
							//if we have one really big word, then parse the string from the
							//end of the line backwards, untill the beginning of the line +2
							//the +2 is so we avoid parsing the ": " thing...
							for(k=new_line_pos+x_chars_limit-1;k>new_line_pos+2;k--)
								{
									cur_char=text_to_add[k];
									if(k>len)continue;
									if(cur_char==' ' || cur_char=='\n')
										{
											k++;//let the space on the previous line
											break;
										}
								}
							if(k==new_line_pos+2)
								new_line_pos=new_line_pos+x_chars_limit;
							else new_line_pos=k;
							line++;
							semaphore=1;
						}

					cur_char=text_to_add[i];

					if(!cur_char)
						{
							j--;
							break;
						}

					if(cur_char>=127 && cur_char <= 127+c_grey4)	//we have a color, save it
						current_color=cur_char;
					else if(cur_char=='\n')
						new_line_pos=i;

					if(i==new_line_pos)
						{
							buffer[j+last_text]='\n';
							j++;
							buffer[j+last_text]=current_color;
							j++;
							semaphore=0;
						}
					//don't add another new line, if the current char is already a new line...
					if(cur_char!='\n')
						{
							buffer[j+last_text]=cur_char;
							j++;
						}

				}
			buffer[last_text+j]='\n';
			buffer[last_text+j+1]=0;
			last_text+=j+1;
		}
}


// find the last lines, according to the current time
int find_last_lines_time (int *msg, int *offset)
{
	// adjust the lines_no according to the time elapsed since the last message
	if ( (cur_time - last_server_message_time) / 1000 > 3)
	{
		if (lines_to_show > 0)
			lines_to_show--;
		last_server_message_time = cur_time;
	}
	if (lines_to_show <= 0) return 0;
	
	return find_line_nr (total_nr_lines, total_nr_lines - lines_to_show, CHANNEL_ALL, msg, offset);
}

int find_last_console_lines (int lines_no)
{
	return find_line_nr (total_nr_lines, total_nr_lines - lines_no, CHANNEL_ALL, &console_msg_nr, &console_msg_offset);
}

int find_line_nr (int nr_lines, int line, int channel, int *msg, int *offset)
{
	int line_count = 0, lines_no = nr_lines - line;	
	int imsg, ichar;
	char *data;
	
	imsg = last_message;
	do 
	{
		int msgchan = display_text_buffer[imsg].chan_nr;

		if (msgchan == channel || msgchan == CHANNEL_ALL || channel == CHANNEL_ALL)
		{
			data = display_text_buffer[imsg].data;
			if (data == NULL)
				// Hmmm... we messed up. This should not be
				// happening.
				break;
			
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
	int imsg;
	
	for (imsg = 0; imsg < DISPLAY_TEXT_BUFFER_SIZE; imsg++)
	{
		if (display_text_buffer[imsg].data)
			display_text_buffer[imsg].data[0] = '\0';
		display_text_buffer[imsg].len = 0;
	}
	
	last_message = -1;
	buffer_full = 0;
	
	console_msg_nr = 0;
	console_msg_offset = 0;

	last_server_message_time = cur_time;
	lines_to_show = 0;
	
	not_from_the_end_console = 0;
}
