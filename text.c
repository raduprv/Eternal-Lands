#include <string.h>
#include <ctype.h>
#include "global.h"

char input_text_line[257];
int input_text_lenght=0;
int input_text_lines=1;
char display_text_buffer[max_display_text_buffer_lenght];

int display_text_buffer_first=0;
int display_text_buffer_last=0;

int display_console_text_buffer_first=0;
int display_console_text_buffer_last=0;
char last_pm_from[32];

Uint32 last_server_message_time;
int lines_to_show=0;
int max_lines_no=10;
char console_mode=0;
char not_from_the_end_console=0;

int log_server = 1;

float	chat_zoom=1.0;
FILE	*chat_log=NULL;

void write_to_log(Uint8 * data,int len)
{
	int i,j;
	Uint8 ch;
	char str[4096];

	int server_message = 0;

	if (chat_log == NULL)
		{
			char chat_log_file[100];
#ifndef WINDOWS
			strcpy(chat_log_file, configdir);
			strcat(chat_log_file, "chat_log.txt");
#else
			strcpy(chat_log_file, "chat_log.txt");
#endif
  			chat_log = fopen (chat_log_file, "a");
		}

	j=0;
	for(i=0;i<len && j < 4090;i++)
		{
			ch= data[i];
			// remove colorization when writting to the chat log
			if(ch<127)
				{
					str[j]=ch;
					j++;
				}
			else if (ch != 133 && ch != 129 && ch != 128)
				{
					server_message = 1;
				}
		}
	str[j]='\n';

	if(!server_message || log_server)
		{
			fwrite(str, j+1, 1, chat_log);
		}
  	fflush(chat_log);
}

void send_input_text_line()
{
	char str[256];
	int i,j;
	int len;
	Uint8 ch;

	if(caps_filter && strlen(input_text_line) > 4 && my_isupper(input_text_line, -1)) my_tolower(input_text_line);
	i=0;
	j=1;
	if(input_text_line[0]!='/')//we don't have a PM
		{
			str[0]=RAW_TEXT;
		}
	else
		{
			str[0]=SEND_PM;
			i++;	// skip the leading /
		}
	for(;i<input_text_lenght;i++)	// copy it, but ignore the enter
		{
			ch=input_text_line[i];
			if(ch!=0x0a)
				{
					str[j]=ch;
					j++;
				}

		}
	str[j]=0;	// always a NULL at the end

	len=strlen(&str[1]);
	if(my_tcp_send(my_socket,str,len+1)<len+1)
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
	if(len > 0 && *ptr == '#' && !strncasecmp(ptr, "#help request", 9)){
		auto_open_encyclopedia= 0;
	}

	//check if ignored
	type=strncasecmp(&text_to_add[1],"[PM from",8)?0:1;
	if(pre_check_if_ignored(text_to_add,type))return 0;
	//All right, we do not ignore the person
	if(afk)
		{
			if(type)add_message_to_pm_log(text_to_add,len);
			else if(is_talking_about_me(&text_to_add[1],len-1))send_afk_message(&text_to_add[1], type);
		}
	//parse for URLs
	find_last_url(text_to_add,len);
	//filter any naughty words out
	return(filter_text(text_to_add, len));
}

void put_text_in_buffer(unsigned char *text_to_add, int len, int x_chars_limit)
{
	put_colored_text_in_buffer(c_grey1, text_to_add, len, x_chars_limit);
}


//-- Logan Dugenoux [5/26/2004]
// Checks chat string, if it begins with an actor name, 
// and the actor is displayed, put said sentence into an overtext bubble
#define allowedCharInName(_x_)		(isalnum(_x_)||(_x_=='_'))
void check_chat_text_to_overtext(unsigned char *text_to_add, int len)
{	
	if (!view_chat_text_as_overtext)
		return;		// disabled

	if (text_to_add[0] == 133)
	{
		char playerName[128];
		char textbuffer[1024];
		int i;
		int j;
		j = 0;
		i = 1;
		while ((text_to_add[i]<128)&&(i<len))
		{
			if (text_to_add[i] != '[')
			{
				playerName[j] = (char)text_to_add[i];
				j++;
				if (j>=128)
					return;//over buffer
			}
			i++;
		}
		if (i!=len)
		{
			playerName[j] = 0;
			while ((j>0)&&(!allowedCharInName(playerName[j])))
				playerName[j--] = 0;
			j = 0;
			while (i<len)
			{
				if (j>=1024)
					return;//over buffer
				textbuffer[j] = (char)text_to_add[i];
				i++;j++;
			}
			textbuffer[j]=0;
			for (i = 0; i < max_actors; i++)
			{
				char actorName[128];
				j = 0;
				// Strip clan info
				while (allowedCharInName(actors_list[i]->actor_name[j]))
				{
					actorName[j] = actors_list[i]->actor_name[j];
					j++;
					if (j>=128)
						return;//over buffer
				}
				actorName[j] = 0;
				if (strcmp(actorName, playerName)==0)
				{
					add_displayed_text_to_actor( actors_list[i], textbuffer );
					break;
				}
			}
			
		}
		
	}
}

void put_char_in_buffer(unsigned char ch)
{
	input_text_line[input_text_lenght]=ch;
	input_text_line[input_text_lenght+1]='_';
	input_text_line[input_text_lenght+2]=0;
	input_text_lenght++;
	if(input_text_lenght==(input_text_lines*(window_width-hud_x))/(get_char_width(ch)-1)-1)
		{
			input_text_line[input_text_lenght]=0x0a;
			input_text_line[input_text_lenght+1]='_';
			input_text_line[input_text_lenght+2]=0;
			input_text_lenght++;
			input_text_lines++;
		}
}


void put_colored_text_in_buffer(Uint8 color, unsigned char *text_to_add, int len, 
								int x_chars_limit)
{
	int i;
	Uint8 cur_char;

	check_chat_text_to_overtext( text_to_add, len );

	// check for auto-length
	if(len<0)len=strlen(text_to_add);
	//set the time when we got this line
	last_server_message_time=cur_time;
	if(lines_to_show<max_lines_no)lines_to_show++;
	//watch for the end of buffer!
	while(display_text_buffer_last+len+8 >= max_display_text_buffer_lenght)
		{
			memmove(display_text_buffer, display_text_buffer+1024, display_text_buffer_last-1024);
			display_text_buffer_last-=1024;
			display_text_buffer_first-=1024;
			if(display_console_text_buffer_first<0)
				{
					display_console_text_buffer_first=0;
				}
		}
	
	// force the color
	if(*text_to_add <= 127 || *text_to_add > 127+c_grey4)
		{
			display_text_buffer[display_text_buffer_last]=127+color;
			display_text_buffer_last++;
		}

	//see if the text fits on the screen
	if(!x_chars_limit)x_chars_limit=(window_width-hud_x)/11;
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

					display_text_buffer[i+display_text_buffer_last]=cur_char;
				}
			display_text_buffer[display_text_buffer_last+i]='\n';
			display_text_buffer[display_text_buffer_last+i+1]=0;
			display_text_buffer_last+=i+1;
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
					if(!semaphore && new_line_pos+x_chars_limit<len)//don't go trough the last line
						{
							//find the closest space from the end of this line
							//if we have one really big word, then parse the string from the
							//end of the line backwards, untill the beginning of the line +2
							//the +2 is so we avoid parsing the ": " thing...
							for(k=new_line_pos+x_chars_limit-1;k>new_line_pos+2;k--)
								{
									cur_char=text_to_add[k];
									if(k>len)continue;
									if(cur_char==' ')
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

					if(cur_char>=127)	//we have a color, save it
						current_color=cur_char;
					if(cur_char=='\n')
						{
							new_line_pos=i;
						}



					if(i==new_line_pos)
						{
							display_text_buffer[j+display_text_buffer_last]='\n';
							j++;
							display_text_buffer[j+display_text_buffer_last]=current_color;
							j++;
							semaphore=0;
							if(lines_to_show<max_lines_no)lines_to_show++;
						}
					//don't add another new line, if the current char is already a new line...
					if(cur_char!='\n')
						{
							display_text_buffer[j+display_text_buffer_last]=cur_char;
							j++;
						}

				}
			display_text_buffer[display_text_buffer_last+j]='\n';
			display_text_buffer[display_text_buffer_last+j+1]=0;
			display_text_buffer_last+=j+1;
		}
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
	if(*text_to_add <= 127)
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

					if(cur_char>=127)	//we have a color, save it
						current_color=cur_char;
					else if(cur_char=='\n')
						{
							new_line_pos=i;
						}



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


//find the last lines, according to the current time
int find_last_lines_time()
{
	int i;
	int line_count=0;

	//adjust the lines_no according to the time elapsed since the last message
	if(((cur_time-last_server_message_time)/1000)>3)
		{
			if(lines_to_show>0)lines_to_show--;
			last_server_message_time=cur_time;
		}
	if(lines_to_show<=0)return 0;

	for(i=display_text_buffer_last-2;i>=0;i--)
		{
			//parse the text backwards, until we meet the 10'th \n
			if(display_text_buffer[i]=='\n')
				{
					line_count++;
					if(line_count>=lines_to_show)break;
				}
		}
	display_text_buffer_first=i+1;//after the new line
	return 1;
}

int find_last_console_lines(int lines_no)
{
	int i;
	int line_count=0;

	for(i=display_text_buffer_last-2;i>=0;i--)
		{
			//parse the text backwards, until we meet the 10'th \n
			if(display_text_buffer[i]=='\n')
				{
					line_count++;
					if(line_count>=lines_no)break;
				}
		}
	display_console_text_buffer_first=i+1;//after the new line
	if(display_console_text_buffer_first<0)display_console_text_buffer_first=0;
	return 1;
}

void console_move_up()
{
	int i;
	int total_lines_no=0;
	int max_lines;

	//get the total number of lines
	for(i=0;i<display_text_buffer_last;i++)
		{
			if(display_text_buffer[i]=='\n')total_lines_no++;
		}

	//get the number of lines we have - the last one, which is the command line
	max_lines=(window_height-hud_y)/18-1;
	if(not_from_the_end_console)max_lines--;

	//if we have less lines of text than the max lines onscreen, don't scrool up
	if(total_lines_no>max_lines)
		{
			for(i=display_console_text_buffer_first-2;i>=0;i--)
				{
					//parse the text backwards, one line
					if(display_text_buffer[i]=='\n')break;
				}
			display_console_text_buffer_first=i+1;//after the new line
			if(display_console_text_buffer_first<0)display_console_text_buffer_first=0;
			not_from_the_end_console=1;
		}
}


void console_move_down()
{
	int i;
	int lines_we_have=0;
	int max_lines;

	if(!not_from_the_end_console)return;//we can't scrool down anymore

	//get the number of lines we have on screen
	max_lines=(window_height-hud_y)/18-1;
	max_lines--;


	for(i=display_console_text_buffer_first;i<display_text_buffer_last;i++)
		{
			if(display_text_buffer[i]=='\n')lines_we_have++;
		}

	if(lines_we_have>max_lines+1)
		{
			for(i=display_console_text_buffer_first+1;i<display_text_buffer_last;i++)
				{
					//parse the text upwards, one line
					if(display_text_buffer[i]=='\n')break;
				}
			display_console_text_buffer_first=i+1;//after the new line
		} else not_from_the_end_console=0;
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

void display_console_text()
{
	int max_lines;
	int command_line_y;

	//get the number of lines we have - the last one, which is the command line
	max_lines=(window_height-17*(2+input_text_lines))/18-2;
	if(not_from_the_end_console)max_lines--;
	command_line_y=window_height-17*(4+input_text_lines);

	if(!not_from_the_end_console)
		find_last_console_lines(max_lines);
	draw_string(0,0,&display_text_buffer[display_console_text_buffer_first],max_lines);
	glColor3f(1.0f,1.0f,1.0f);
	if(not_from_the_end_console)draw_string(0,command_line_y-18,
											"^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^",2);
	draw_string(0,command_line_y,input_text_line,input_text_lines);
}

