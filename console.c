#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include "global.h"

char	auto_open_encyclopedia= 1;

//cls - clears the text buffer
void cls()
{
	int i;

	display_console_text_buffer_first=0;
	display_text_buffer_first=0;
	display_text_buffer_last=0;

	//clear the buffer
	for(i=0;i<max_display_text_buffer_lenght;i++)display_text_buffer[i]=0;
	not_from_the_end_console=0;

	//also update the lines to show, and the last server message thing
	//without it, the text would dissapear very slowly...
	lines_to_show=0;
	last_server_message_time=cur_time;
}

void print_log()
{
	FILE *f = NULL;

  	f = fopen ("text_log.txt", "ab");
  	fwrite (display_text_buffer, display_text_buffer_last, 1, f);
  	fclose (f);
}

//do we have any console commands?
//if not, send the text to the server
void test_for_console_command()
{
	char *text_loc=input_text_line;
	int text_lenght=input_text_lenght;
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
			cls();
			return;
		}
	//stats ?
	if(my_strcompare(text_loc,"stats"))
		{
			unsigned char protocol_name;
			protocol_name=SERVER_STATS;
			my_tcp_send(my_socket,&protocol_name,1);
			return;
		}
	//time?
	if(my_strcompare(text_loc,"time"))
		{
			unsigned char protocol_name;
			protocol_name=GET_TIME;
			my_tcp_send(my_socket,&protocol_name,1);
			return;
		}
	//ping?
	if(my_strcompare(text_loc,"ping"))
		{
			Uint8 str[8];
			str[0]=PING;
			*((Uint32 *)(str+1))=SDL_GetTicks();
			my_tcp_send(my_socket,str,5);
			return;
		}

	//date?
	if(my_strcompare(text_loc,"date"))
		{
			unsigned char protocol_name;
			protocol_name=GET_DATE;
			my_tcp_send(my_socket,&protocol_name,1);
			return;
		}
	if(my_strcompare(text_loc,"quit") || my_strcompare(text_loc,"exit"))
		{
			exit_now=1;
			return;
		}
	if(my_strcompare(text_loc,"mem") || my_strcompare(text_loc,"cache"))
		{
			cache_dump_sizes(cache_system);
#ifdef	DEBUG
			cache_dump_sizes(cache_e3d);
			cache_dump_sizes(cache_md2);
#endif	//DEBUG
			return;
		}
	if(my_strcompare(text_loc,"ver") || my_strcompare(text_loc,"vers"))
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
			log_to_console(c_green1,str);
			return;
		}

	if(my_strcompare(text_loc,"ignores"))
		{
			list_ignores();
			return;
		}
	if(my_strncompare(text_loc,"ignore ", 7))
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
					log_to_console(c_red1,str);
					return;
				}
			if(i<3)
				{
					Uint8 str[100];
					sprintf(str,"%s %s",name_too_short,not_added_to_ignores);
					log_to_console(c_red1,name_too_short);
					return;
				}

			result=add_to_ignore_list(name,save_ignores);
			if(result==-1)
				{
					Uint8 str[100];
					sprintf(str,already_ignoring,name);
					log_to_console(c_red1,str);
					return;
				}
			if(result==-2)
				{
					log_to_console(c_red1,ignore_list_full);
					return;
				}
			else
				{
					Uint8 str[100];
					sprintf(str,added_to_ignores,name);
					log_to_console(c_green1,str);
					return;
				}
		}

	if(my_strcompare(text_loc,"filters"))
		{
			list_filters();
			return;
		}
	if(my_strncompare(text_loc,"filter ", 7))
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
					log_to_console(c_red1,str);
					return;
				}
			if(i<3)
				{
					Uint8 str[100];
					sprintf(str,"%s %s",word_too_short,not_added_to_filter);
					log_to_console(c_red1,word_too_short);
					return;
				}

			result=add_to_filter_list(name,save_ignores);
			if(result==-1)
				{
					Uint8 str[100];
					sprintf(str,already_filtering,name);
					log_to_console(c_red1,str);
					return;
				}
			if(result==-2)
				{
					log_to_console(c_red1,filter_list_full);
					return;
				}
			else
				{
					Uint8 str[100];
					sprintf(str,added_to_filters,name);
					log_to_console(c_green1,str);
					return;
				}
		}

	////////////////////////
	if(my_strncompare(text_loc,"unignore ",9))
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
					log_to_console(c_red1,str);
					return;
				}
			if(i<3)
				{
					Uint8 str[200];
					sprintf(str,"%s %s",name_too_short,not_removed_from_filter);
					log_to_console(c_red1,str);
					return;
				}
			result=remove_from_ignore_list(name);
			if(result==-1)
				{
					Uint8 str[200];
					sprintf(str,not_ignoring,name);
					log_to_console(c_red1,str);
					return;
				}
			else
				{
					Uint8 str[100];
					sprintf(str,removed_from_ignores,name);
					log_to_console(c_green1,str);
					return;
				}
		}
	if(my_strncompare(text_loc,"unfilter ",9))
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
					log_to_console(c_red1,str);
					return;
				}
			if(i<3)
				{
					Uint8 str[200];
					sprintf(str,"%s %s",word_too_short,not_removed_from_filter);
					log_to_console(c_red1,str);
					return;
				}
			result=remove_from_filter_list(name);
			if(result==-1)
				{
					Uint8 str[200];
					sprintf(str,not_filtering,name);
					log_to_console(c_red1,str);
					return;
				}
			else
				{
					Uint8 str[100];
					sprintf(str,removed_from_filter,name);
					log_to_console(c_green1,str);
					return;
				}
		}


	if(my_strcompare(text_loc,"glinfo"))
		{
			GLubyte *my_string;
			Uint8 this_string[8192];

			my_string=(GLubyte *)glGetString(GL_RENDERER);
			sprintf(this_string,"%s: %s",video_card_str,my_string);
			log_to_console(c_red2,this_string);

			my_string=(GLubyte *)glGetString(GL_VENDOR);
			sprintf(this_string,"%s: %s",video_vendor_str,my_string);
			log_to_console(c_yellow3,this_string);

			my_string=(GLubyte *)glGetString(GL_VERSION);
			sprintf(this_string,"%s: %s",opengl_version_str,my_string);
			log_to_console(c_yellow2,this_string);

			my_string=(GLubyte *)glGetString(GL_EXTENSIONS);
			sprintf(this_string,"%s: %s",supported_extensions_str,my_string);
			log_to_console(c_grey1,this_string);

			return;
		}
	if(my_strncompare(text_loc,"log conn data", 8))
		{
			log_to_console(c_grey1,logconn_str);
			log_conn_data=1;
			return;
		}

	// TODO: make this automatic or a better command, m is too short
	if(my_strncompare(text_loc,"msg", 3))
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
	if(my_strncompare(text_loc,"afk",3))
		{
			// find first space, then skip any spaces
			while(*text_loc && !isspace(*text_loc))	text_loc++;
			while(*text_loc && isspace(*text_loc))	text_loc++;
			if(!afk)
				{
					if((text_lenght=input_text_lenght-(text_loc-input_text_line))) strncpy(afk_message, text_loc, text_lenght);
					go_afk();
					last_action_time=cur_time-afk_time-1;
				}
			else go_ifk();
			return;
		}
	
	if(my_strncompare(text_loc,"help", 4))
		{
			// help can open the Enc!
			if(auto_open_encyclopedia)	view_window(&encyclopedia_win, -1);
			// but fall thru and send it to the server
		}

	if (my_strncompare (text_loc, "storage", 7))
		{
			if (text_loc[7] != ' ')
				{
					storage_filter[0] = '\0';
				}
			else
				{
					my_strncp (storage_filter, text_loc+8, 128-1);
					sprintf (input_text_line, "#storage");
					input_text_lenght = 8;
				}
		}

	send_input_text_line();//no command, send it to the server, as plain text

}

