#include "global.h"

void write_to_log(Uint8 * data,int len)
{
	int i,j;
	Uint8 ch;
	char str[1000];
	FILE *f = NULL;

	int server_message = 0;

  	f = fopen ("chat_log.txt", "a");

	j=0;
	for(i=0;i<len;i++)
		{
			ch=data[i];
			if(ch<127)
				{
					str[j]=ch;
					j++;
				}
			else
			  if (ch != 133 && ch != 129 && ch != 128)
			    server_message = 1;
		}
	str[j]='\n';

	if(!server_message || log_server)
	  fwrite(str, j+1, 1, f);
  	fclose(f);
}

void send_input_text_line()
{
	char str[170];
	int i,j;
	int len;
	Uint8 ch;

	j=1;
	if(input_text_line[0]!='/')//we don't have a PM
		{
			str[0]=RAW_TEXT;
			for(i=0;i<input_text_lenght;i++)
				{
					ch=input_text_line[i];
					if(ch!=0x0a)
						{
							str[j]=ch;
							j++;
						}

				}
			str[j]=0;

			len=strlen(&str[1]);
			if(my_tcp_send(my_socket,str,len+1)<len+1)
				{
					//we got a nasty error, log it
				}
			return;

		}
	j=1;
	if(input_text_line[0]=='/')//we have a PM
		{
			str[0]=SEND_PM;
			for(i=0;i<input_text_lenght;i++)
				{
					ch=input_text_line[i+1];
					if(ch!=0x0a)
						{
							str[j]=ch;
							j++;
						}

				}
			str[j]=0;

			len=strlen(&str[1]);
			if(my_tcp_send(my_socket,str,len)<len)
				{
					//we got a nasty error, log it
				}

		}


}

int filter_or_ignore_text(unsigned char *text_to_add, int len)
{
	//check if ignored
	if(pre_check_if_ignored(text_to_add))return 0;
	//parse for URLs
	find_last_url(text_to_add,len);
	//filter any naughty words out
	return(filter_text(text_to_add, len));
}

void put_text_in_buffer(unsigned char *text_to_add, int len, int x_chars_limit)
{
	put_colored_text_in_buffer(c_grey1, text_to_add, len, x_chars_limit);
}

void put_colored_text_in_buffer(Uint8 color, unsigned char *text_to_add, int len, int x_chars_limit)
{
	int i;
	Uint8 cur_char;

	// check for auto-length
	if(len<0)len=strlen(text_to_add);
	//get the time when we got this line
	last_server_message_time=cur_time;
	if(lines_to_show<max_lines_no)lines_to_show++;

	//see if the text fits on the screen
	if(!x_chars_limit)x_chars_limit=window_width/11;
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
			unsigned char current_color=127+color;	// switch to the color specified

			//how many lines of text do we have?
			text_lines=len/x_chars_limit;
			//go trought all the text
			j=0;
			for(i=0;i<len;i++)
				{
					//if(!semaphore && line<text_lines)//don't go through the last line
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

void put_small_text_in_box(unsigned char *text_to_add, int len, int pixels_limit, char *buffer)
{
	put_small_colored_text_in_box(c_grey1, text_to_add, len, pixels_limit, buffer);
}

void put_small_colored_text_in_box(Uint8 color,unsigned char *text_to_add, int len, int pixels_limit, char *buffer)
{
	int i;
	Uint8 cur_char;
	int last_text=0;
	int x_chars_limit;

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
					//if(!semaphore && line<text_lines)//don't go through the last line
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
	Uint8 cur_char;
	int line_count=0;

	//adjust the lines_no according to the time elapsed since the last message
	if(((cur_time-last_server_message_time)/1000)>3)
		{
			if(lines_to_show>0)lines_to_show--;
			last_server_message_time=cur_time;
		}
	//if(lines_to_show<max_lines_no)max_lines_no=lines_to_show;
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
	Uint8 cur_char;
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
	Uint8 cur_char;
	int line_count=0;

	//get the total number of lines
	for(i=0;i<display_text_buffer_last;i++)
		{
			if(display_text_buffer[i]=='\n')total_lines_no++;
		}

	//get the number of lines we have - the last one, which is the command line
	max_lines=window_height/18-1;
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
	int lines_to_the_end=0;
	int lines_we_have=0;
	int max_lines;
	Uint8 cur_char;
	int line_count=0;

	if(!not_from_the_end_console)return;//we can't scrool down anymore

	//get the number of lines we have on screen
	max_lines=window_height/18-1;
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

	max_lines=window_height/18-3;

	for(i=0;i<max_lines;i++)
	console_move_down();
}

void console_move_page_up()
{
	int max_lines;
	int i;

	max_lines=window_height/18-3;

	for(i=0;i<max_lines;i++)
	console_move_up();
}

void display_console_text()
{
	int max_lines;
	int command_line_y;

	//get the number of lines we have - the last one, which is the command line
	max_lines=window_height/18-2;
	if(not_from_the_end_console)max_lines--;
	command_line_y=window_height-36;

	if(!not_from_the_end_console)
	find_last_console_lines(max_lines);
	//else get_console_text();
	draw_string(0,0,&display_text_buffer[display_console_text_buffer_first],max_lines);
	glColor3f(1.0f,1.0f,1.0f);
	if(not_from_the_end_console)draw_string(0,command_line_y-18,
	"^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^");
	draw_string(0,command_line_y,input_text_line,2);

}

