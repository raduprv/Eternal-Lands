#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include "global.h"

/* NOTE: This file contains implementations of the following, currently unused, and commented functions:
 *          Look at the end of the file.
 *
 * void print_log();
 * void cls();
 */

char	auto_open_encyclopedia= 1;

//do we have any console commands?
//if not, send the text to the server
void test_for_console_command (char *text, int len)
{
	char *text_loc = text;
	int text_lenght = len;
	//skip a leading #
	if(*text_loc=='#')
		{
			text_loc++;
			text_lenght--;
		}
	// skip leading spaces
	while(*text_loc==' ')
		{
			text_loc++;
			text_lenght--;
		}
	//remove trailing '_'
	if(text_loc[text_lenght] == '_')
		{
			text_loc[text_lenght]='\0';
			text_lenght--;
		}
	//cls?
	if(my_strcompare(text_loc,"cls"))
		{
			clear_display_text_buffer ();
			return;
		}
	else if(my_strncompare(text_loc,"goto",4))
	{
		Uint8 sx[3];
		Uint8 sy[3];
		int i,
			j,
			is_name = 0,
			sw = 0,
			x = 0,
			y = 0,
			found = 0;
		Uint8 ch='\0';
		char str[520];
		char name[512] = {0};
		char tmp_name[512] = {0};

		while (!isspace(*text_loc))
			text_loc++;
		while (isspace(*text_loc))
			text_loc++;
		for (i = 0; text_loc[i] != '\0'; i++)
		{
			if (!isdigit(text_loc[i]) && text_loc[i] != ' ' && text_loc[i] != ',')
			{
				is_name = 1;
				break;
			}
		}
		if (is_name)
		{
			for (i = 0; text_loc[i] != 0x0a && text_loc[i] != '\0'; i++)
			{
				name[i] = text_loc[i];
			}
			name[i] = '\0';
			
			{ 
				FILE * fp;
				char marks_file[256], text[600];
#ifndef WINDOWS
				my_strcp(marks_file, getenv("HOME"));
				my_strcat(marks_file, "/.elc/");
				my_strcat(marks_file, strrchr(map_file_name,'/')+1);
#else
				my_strcp(marks_file, strrchr(map_file_name,'/')+1);
#endif
				my_strcat(marks_file, ".txt");
				// If the player uses goto with a marker, the marks file 
				// should be there, so use my_fopen
				fp = my_fopen(marks_file, "r");
				if ( fp )
				{
					while ( fgets(text, 600,fp) )
					{
						if (strlen (text) > 1) //skip empty lines
						{
							my_strncp (tmp_name, strstr(strstr(text, " ")+1, " ")+1, 500);
							tmp_name[strlen(tmp_name)-1] = '\0'; //remove the newline
							if (my_strcompare(name, tmp_name))
							{
								sscanf (text, "%d %d", &x, &y);
								found = 1;
								break;
							}
						}
					}
					fclose(fp);
				}
			}
		}
		else
		{
			found = 1;
			for(i = 0, j = 0; i < 15; i++)
			{
				ch=text_loc[i];
				if((ch==' ' || ch=='\0' || ch == ',') && sw==0)
				{
					sw=1;
					j=0;
				}
				else if((ch==' ' || ch=='\0' || ch == ',') && sw==1)
					break;
				else if(sw==0)
				{
					sx[j]=ch;
					j++;
				}
				else if(sw==1)
				{
					sy[j]=ch;
					j++;
				}
			}
			x=atoi(sx);
			y=atoi(sy);
		}

		if (found)
		{
			int check;
			
			if (pf_follow_path)
			{
				pf_destroy_path();
			}
			check = pf_find_path(x, y);
			if (check)
				sprintf(str, "Goto: %d,%d", x, y);
			else
				sprintf(str, "Can't go to %d,%d", x, y);
			LOG_TO_CONSOLE((check?c_orange1:c_red2), str);
		}
		else
		{
			sprintf (str, "Mark %s not found", name);
			LOG_TO_CONSOLE(c_red2, str);
		}
		return;
	}
	else if(my_strncompare(text_loc,"markpos", 7))
	{
		if (strlen(text_loc) > 5) //check for empty marks
		{
			int map_x, map_y;
			char * ptr = text_loc + 8;
			char msg[540];
			
			while (*ptr == ' ') ptr++;
			if (sscanf(ptr, "%d,%d ", &map_x, &map_y) != 2) {
				LOG_TO_CONSOLE(c_red2, "Usage: #markpos <x-coord>,<y-coord> <name>");
				return;
			}
			while (*ptr != ' ' && *ptr) ptr++;
			while (*ptr == ' ') ptr++;
			if (!*ptr) {
				LOG_TO_CONSOLE(c_red2, "Usage: #markpos <x-coord>,<y-coord> <name>");
				return;
			}
			if (put_mark_on_position(map_x, map_y, ptr)) {
				sprintf (msg, "Location %d,%d marked with %s", map_x, map_y, ptr);
				LOG_TO_CONSOLE(c_orange1,msg);
			} else {
				sprintf (msg, "Invalid location %d,%d", map_x, map_y);
				LOG_TO_CONSOLE(c_red2,msg);
			}
		}
		return;		
	}
	else if(my_strncompare(text_loc,"mark", 4))
	{
		if (strlen(text_loc) > 5) //check for empty marks
		{
			char str[520];
			put_mark_on_current_position(text_loc+5);
			sprintf (str, "%s marked", text_loc+5);
			LOG_TO_CONSOLE(c_orange1,str);
		}
		return;		
	}
	else if (my_strncompare(text_loc,"unmark",6))
	{
		int i;
		while (!isspace(*text_loc))
			text_loc++;
		while (isspace(*text_loc))
			text_loc++;

		for (i = 0 ; i < max_mark ; i ++)
		{
			if (my_strcompare(marks[i].text, text_loc))
			{
				char str[520];
				marks[i].x = marks[i].y = -1;
				save_markings();
				sprintf(str,"%s removed", marks[i].text);
				LOG_TO_CONSOLE(c_orange1,str);
				break;
			}
		}
		return;
	}
	//stats ?
	else if(my_strcompare(text_loc,"stats"))
		{
			unsigned char protocol_name;
			protocol_name=SERVER_STATS;
			my_tcp_send(my_socket,&protocol_name,1);
			return;
		}
	//time?
	else if(my_strcompare(text_loc,"time"))
		{
			unsigned char protocol_name;
			protocol_name=GET_TIME;
			my_tcp_send(my_socket,&protocol_name,1);
			return;
		}
	//ping?
	else if(my_strcompare(text_loc,"ping"))
		{
			Uint8 str[8];
			str[0]=PING;
			*((Uint32 *)(str+1))=SDL_SwapLE32(SDL_GetTicks());
			my_tcp_send(my_socket,str,5);
			return;
		}

	//date?
	else if(my_strcompare(text_loc,"date"))
		{
			unsigned char protocol_name;
			protocol_name=GET_DATE;
			my_tcp_send(my_socket,&protocol_name,1);
			return;
		}
	else if(my_strcompare(text_loc,"quit") || my_strcompare(text_loc,"exit"))
		{
			exit_now=1;
			return;
		}
	else if(my_strcompare(text_loc,"mem") || my_strcompare(text_loc,"cache"))
		{
			cache_dump_sizes(cache_system);
#ifdef	DEBUG
			cache_dump_sizes(cache_e3d);
			cache_dump_sizes(cache_md2);
#endif	//DEBUG
			return;
		}
	else if(my_strcompare(text_loc,"ver") || my_strcompare(text_loc,"vers"))
		{
			char str[128];
			char extra[20];
			if(client_version_patch > 0)
				{
					sprintf(extra,"p%d Beta",client_version_patch);
				}
			else
				{
					sprintf(extra," Beta");
				}
			sprintf(str,"Eternal Lands Version %d.%d.%d%s",client_version_major,
					client_version_minor,client_version_release,extra);
			LOG_TO_CONSOLE(c_green1,str);
			return;
		}

	else if(my_strcompare(text_loc,"ignores"))
		{
			list_ignores();
			return;
		}
	else if(my_strncompare(text_loc,"ignore ", 7))
		{
			Uint8 name[16];
			int i;
			Uint8 ch='\0';
			int result;

			for(i=0;i<15;i++)
				{
					ch=text_loc[i+7];//7 because there is a space after "ignore"
					if(ch==' ' || ch=='\0')
						{
							ch=0;
							break;
						}
					name[i]=ch;
				}

			name[i]=0;

			if(i==15 && !ch)
				{
					Uint8 str[100];
					sprintf(str,"%s %s",name_too_long,not_added_to_ignores);
					LOG_TO_CONSOLE(c_red1,str);
					return;
				}
			if(i<3)
				{
					Uint8 str[100];
					sprintf(str,"%s %s",name_too_short,not_added_to_ignores);
					LOG_TO_CONSOLE(c_red1,name_too_short);
					return;
				}

			result=add_to_ignore_list(name,save_ignores);
			if(result==-1)
				{
					Uint8 str[100];
					sprintf(str,already_ignoring,name);
					LOG_TO_CONSOLE(c_red1,str);
					return;
				}
			if(result==-2)
				{
					LOG_TO_CONSOLE(c_red1,ignore_list_full);
					return;
				}
			else
				{
					Uint8 str[100];
					sprintf(str,added_to_ignores,name);
					LOG_TO_CONSOLE(c_green1,str);
					return;
				}
		}

	else if(my_strcompare(text_loc,"filters"))
		{
			list_filters();
			return;
		}
	else if(my_strncompare(text_loc,"filter ", 7))
		{
			Uint8 name[16];
			int i;
			Uint8 ch='\0';
			int result;

			for(i=0;i<15;i++)
				{
					ch=text_loc[i+7];//7 because there is a space after "filter"
					if(ch==' ' || ch=='\0')
						{
							ch=0;
							break;
						}
					name[i]=ch;
				}

			name[i]=0;

			if(i==15 && !ch)
				{
					Uint8 str[100];
					sprintf(str,"%s %s",word_too_long,not_added_to_filter);
					LOG_TO_CONSOLE(c_red1,str);
					return;
				}
			else if(i<3)
				{
					Uint8 str[100];
					sprintf(str,"%s %s",word_too_short,not_added_to_filter);
					LOG_TO_CONSOLE(c_red1,word_too_short);
					return;
				}

			result=add_to_filter_list(name,save_ignores);
			if(result==-1)
				{
					Uint8 str[100];
					sprintf(str,already_filtering,name);
					LOG_TO_CONSOLE(c_red1,str);
					return;
				}
			else if(result==-2)
				{
					LOG_TO_CONSOLE(c_red1,filter_list_full);
					return;
				}
			else
				{
					Uint8 str[100];
					sprintf(str,added_to_filters,name);
					LOG_TO_CONSOLE(c_green1,str);
					return;
				}
		}

	////////////////////////
	else if(my_strncompare(text_loc,"unignore ",9))
		{
			Uint8 name[16];
			int i;
			Uint8 ch='\0';
			int result;

			for(i=0;i<15;i++)
				{
					ch=text_loc[i+9];//9 because there is a space after "ignore"
					if(ch==' ' || ch=='\0')
						{
							ch=0;
							break;
						}
					name[i]=ch;
				}

			name[i]=0;

			if(i==15 && !ch)
				{
					Uint8 str[200];
					sprintf(str,"%s %s",name_too_long,not_removed_from_ignores);
					LOG_TO_CONSOLE(c_red1,str);
					return;
				}
			if(i<3)
				{
					Uint8 str[200];
					sprintf(str,"%s %s",name_too_short,not_removed_from_filter);
					LOG_TO_CONSOLE(c_red1,str);
					return;
				}
			result=remove_from_ignore_list(name);
			if(result==-1)
				{
					Uint8 str[200];
					sprintf(str,not_ignoring,name);
					LOG_TO_CONSOLE(c_red1,str);
					return;
				}
			else
				{
					Uint8 str[100];
					sprintf(str,removed_from_ignores,name);
					LOG_TO_CONSOLE(c_green1,str);
					return;
				}
		}
	else if(my_strncompare(text_loc,"unfilter ",9))
		{
			Uint8 name[16];
			int i;
			Uint8 ch='\0';
			int result;

			for(i=0;i<15;i++)
				{
					ch=text_loc[i+9];//9 because there is a space after "filter"
					if(ch==' ' || ch=='\0')
						{
							ch=0;
							break;
						}
					name[i]=ch;
				}

			name[i]=0;

			if(i==15 && !ch)
				{
					Uint8 str[200];
					sprintf(str,"%s %s",word_too_long,not_removed_from_filter);
					LOG_TO_CONSOLE(c_red1,str);
					return;
				}
			if(i<3)
				{
					Uint8 str[200];
					sprintf(str,"%s %s",word_too_short,not_removed_from_filter);
					LOG_TO_CONSOLE(c_red1,str);
					return;
				}
			result=remove_from_filter_list(name);
			if(result==-1)
				{
					Uint8 str[200];
					sprintf(str,not_filtering,name);
					LOG_TO_CONSOLE(c_red1,str);
					return;
				}
			else
				{
					Uint8 str[100];
					sprintf(str,removed_from_filter,name);
					LOG_TO_CONSOLE(c_green1,str);
					return;
				}
		}


	else if(my_strcompare(text_loc,"glinfo"))
		{
			GLubyte *my_string;
			Uint8 this_string[8192];

			my_string=(GLubyte *)glGetString(GL_RENDERER);
			sprintf(this_string,"%s: %s",video_card_str,my_string);
			LOG_TO_CONSOLE(c_red2,this_string);

			my_string=(GLubyte *)glGetString(GL_VENDOR);
			sprintf(this_string,"%s: %s",video_vendor_str,my_string);
			LOG_TO_CONSOLE(c_yellow3,this_string);

			my_string=(GLubyte *)glGetString(GL_VERSION);
			sprintf(this_string,"%s: %s",opengl_version_str,my_string);
			LOG_TO_CONSOLE(c_yellow2,this_string);

			my_string=(GLubyte *)glGetString(GL_EXTENSIONS);
			sprintf(this_string,"%s: %s",supported_extensions_str,my_string);
			LOG_TO_CONSOLE(c_grey1,this_string);

			return;
		}
	else if(my_strncompare(text_loc,"log conn data", 8))
		{
			LOG_TO_CONSOLE(c_grey1,logconn_str);
			log_conn_data=1;
			return;
		}

	// TODO: make this automatic or a better command, m is too short
	else if(my_strncompare(text_loc,"msg", 3))
		{
			int no;//, m=-1;

			// find first space, then skip any spaces
			while(*text_loc && !isspace(*text_loc))	text_loc++;
			while(*text_loc && isspace(*text_loc))	text_loc++;
			if(my_strncompare(text_loc, "all", 3)){
				for(no=0;no<pm_log.ppl;no++) print_message(no);
				return;
			}
			no=atoi(text_loc)-1;
			if(no<pm_log.ppl && no>=0)	print_message(no);
			return;
		}
	else if(my_strncompare(text_loc,"afk",3))
		{
			// find first space, then skip any spaces
			while(*text_loc && !isspace(*text_loc))	text_loc++;
			while(*text_loc && isspace(*text_loc))	text_loc++;
			if(!afk)
				{
					if ((text_lenght = len - (text_loc-text)) > 0) 
						{
							strncpy(afk_message, text_loc, text_lenght);
							afk_message[text_lenght]='\0';
						}
					go_afk();
					last_action_time=cur_time-afk_time-1;
				}
			else go_ifk();
			return;
		}
	
	else if(my_strncompare(text_loc,"help", 4))
		{
			// help can open the Enc!
			if(auto_open_encyclopedia)
			{
				view_tab (&tab_help_win, &tab_help_collection_id, 1);
			}
			// but fall thru and send it to the server
		}

	else if (my_strncompare (text_loc, "storage", 7))
		{
			if (text_loc[7] != ' ')
				{
					storage_filter[0] = '\0';
				}
			else
				{
					my_strncp (storage_filter, text_loc+8, 128-1);
					sprintf (text, "#storage");
					len = 8;
				}
		}
	send_input_text_line (text, len);	// no command, send it to the server, as plain text
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
