#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "global.h"

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
	//cls?
	if(my_strcompare(text_loc,"cls_"))
		{
			cls();
			return;
		}
	//stats ?
	if(my_strcompare(text_loc,"stats_"))
		{
			unsigned char protocol_name;
			protocol_name=SERVER_STATS;
			my_tcp_send(my_socket,&protocol_name,1);
			return;
		}
	//time?
	if(my_strcompare(text_loc,"time_"))
		{
			unsigned char protocol_name;
			protocol_name=GET_TIME;
			my_tcp_send(my_socket,&protocol_name,1);
			return;
		}
	//ping?
	if(my_strcompare(text_loc,"ping_"))
		{
			Uint8 str[8];
			str[0]=PING;
			*((Uint32 *)(str+1))=SDL_GetTicks();
			my_tcp_send(my_socket,str,5);
			return;
		}

	//date?
	if(my_strcompare(text_loc,"date_"))
		{
			unsigned char protocol_name;
			protocol_name=GET_DATE;
			my_tcp_send(my_socket,&protocol_name,1);
			return;
		}
	if(my_strcompare(text_loc,"quit_"))
		{
			exit_now=1;
			return;
		}
	if(my_strcompare(text_loc,"exit_"))
		{
			exit_now=1;
			return;
		}
	if(my_strcompare(text_loc,"ver_"))
		{
			char str[128];
			sprintf(str,"Eternal Lands Version 0.%d.%d Beta",version_first_digit,version_second_digit);
			log_to_console(c_green1,str);
			return;
		}

	if(my_strcompare(text_loc,"ignores_"))
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
					if(ch==' ' || ch=='_')
						{
							ch=0;
							break;
						}
					name[i]=ch;
				}

			name[i]=0;

			if(i==15 && !ch)
				{
					log_to_console(c_red1,"Name too long, the max limit is 15 characters. Name not added to the ignore list!");
					return;
				}
			if(i<3)
				{
					log_to_console(c_red1,"Name too short, only names>=3 characters can be used! Name not added to the ignore list!");
					return;
				}

			result=add_to_ignore_list(name,save_ignores);
			if(result==-1)
				{
					Uint8 str[100];
					sprintf(str,"You are already ignoring %s!",name);
					log_to_console(c_red1,str);
					return;
				}
			if(result==-2)
				{
					log_to_console(c_red1,"Wow, your ignore list is full, this is impossible!");
					return;
				}
			else
				{
					Uint8 str[100];
					sprintf(str,"Ok, %s was added to your ignore list!",name);
					log_to_console(c_green1,str);
					return;
				}
		}

	if(my_strcompare(text_loc,"filters_"))
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
					ch=text_loc[i+7];//7 because there is a space after "ignore"
					if(ch==' ' || ch=='_')
						{
							ch=0;
							break;
						}
					name[i]=ch;
				}

			name[i]=0;

			if(i==15 && !ch)
				{
					log_to_console(c_red1,"Word too long, the max limit is 15 characters. Word not added to the filter list!");
					return;
				}
			if(i<3)
				{
					log_to_console(c_red1,"Word too short, only names>=3 characters can be used! Word not added to the filter list!");
					return;
				}

			result=add_to_filter_list(name,save_ignores);
			if(result==-1)
				{
					Uint8 str[100];
					sprintf(str,"You are already filter %s!",name);
					log_to_console(c_red1,str);
					return;
				}
			if(result==-2)
				{
					log_to_console(c_red1,"Wow, your filter list is full, this is impossible!");
					return;
				}
			else
				{
					Uint8 str[100];
					sprintf(str,"Ok, %s was added to your filter list!",name);
					log_to_console(c_green1,str);
					return;
				}
		}

////////////////////////

////////////////////////
	if(my_strncompare(text_loc, "modes_", 5))
		{
		  //char str[1000]; unused?
			//see what modes are supported.
#ifndef WINDOWS
			log_to_console(c_grey1,"Since you are under Xwindows, you will always get the desktop color depth, wether or not you are in the full screen mode.");
#endif
			log_to_console(c_grey1,"This is a list with the supported modes, both in full screen and windowed. Please note that this list does not necesarly reflect the hardware accelerated modes, but the modes that CAN be used (even tho they might fall back in software)");
			log_to_console(c_orange2,"\nSupported Full Screen modes:");

			if(SDL_VideoModeOK(640, 480, 16, SDL_OPENGL|SDL_FULLSCREEN))
			log_to_console(c_green2,"Mode 1: 640x480x16: [Suported]");
			else log_to_console(c_red2,"Mode 1: 640x480x16: [Not Suported]");

			if(SDL_VideoModeOK(640, 480, 32, SDL_OPENGL|SDL_FULLSCREEN))
			log_to_console(c_green2,"Mode 2: 640x480x32: [Suported]");
			else log_to_console(c_red2,"Mode 2: 640x480x32: [Not Suported]");

			if(SDL_VideoModeOK(800, 600, 16, SDL_OPENGL|SDL_FULLSCREEN))
			log_to_console(c_green2,"Mode 3: 800x600x16: [Suported]");
			else log_to_console(c_red2,"Mode 3: 800x600x16: [Not Suported]");

			if(SDL_VideoModeOK(800, 600, 32, SDL_OPENGL|SDL_FULLSCREEN))
			log_to_console(c_green2,"Mode 4: 800x600x32: [Suported]");
			else log_to_console(c_red2,"Mode 4: 800x600x32: [Not Suported]");

			if(SDL_VideoModeOK(1024, 768, 16, SDL_OPENGL|SDL_FULLSCREEN))
			log_to_console(c_green2,"Mode 5: 1024x768x16: [Suported]");
			else log_to_console(c_red2,"Mode 5: 1024x768x16: [Not Suported]");

			if(SDL_VideoModeOK(1024, 768, 32, SDL_OPENGL|SDL_FULLSCREEN))
			log_to_console(c_green2,"Mode 6: 1024x768x32: [Suported]");
			else log_to_console(c_red2,"Mode 6: 1024x768x32: [Not Suported]");

			if(bpp==16)log_to_console(c_orange2,"\nSupported Windowed modes: [16 bpp]");
			else
			log_to_console(c_orange2,"\nSupported Windowed modes: [32 bpp]");
			if(SDL_VideoModeOK(640, 480, 16, SDL_OPENGL))
			log_to_console(c_green2,"Mode 1-2: 640x480: [Suported]");
			else log_to_console(c_red2,"Mode 1: 640x480: [Not Suported]");

			if(SDL_VideoModeOK(800, 600, 16, SDL_OPENGL))
			log_to_console(c_green2,"Mode 3-4: 800x600: [Suported]");
			else log_to_console(c_red2,"Mode 3: 800x600: [Not Suported]");

			if(SDL_VideoModeOK(1024, 768, 16, SDL_OPENGL))
			log_to_console(c_green2,"Mode 5-6: 1024x768: [Suported]");
			else log_to_console(c_red2,"Mode 5: 1024x768: [Not Suported]");

			return;
		}

////////////////////////
	if(my_strncompare(text_loc, "mode ", 5) && text_loc[5]>='1' && text_loc[5]<='6')
		{

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
					if(ch==' ' || ch=='_')
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
					my_strcp(str,"Name too long, the max limit is 15 characters. Name not removed from the ignore list!");
					log_to_console(c_red1,str);
					return;
				}
			if(i<3)
				{
					Uint8 str[200];
					my_strcp(str,"Name too short, only names>=3 characters can be used! Name not removed from the ignore list!");
					log_to_console(c_red1,str);
					return;
				}
			result=remove_from_ignore_list(name);
			if(result==-1)
				{
					Uint8 str[200];
					sprintf(str,"You are NOT ignoring %s in the first place!",name);
					log_to_console(c_red1,str);
					return;
				}
			else
				{
					Uint8 str[100];
					sprintf(str,"Ok, %s was removed from your ignore list!",name);
					log_to_console(c_green1,str);
					return;
				}
		}
////////////////////////
	if(my_strcompare(text_loc,"help_"))
		{
			display_help_topic("main");
			return;
		}
	if(my_strncompare(text_loc,"help ",5))
		{
			Uint8 topic[30];
			int i;
			Uint8 ch;
			//int result; unused?

			for(i=0;i<30 || i<text_lenght;i++)
				{
					ch=text_loc[i+5];//5 because there is a space after "help"
					if(ch==' ' || ch=='_')
						{
							ch=0;
							break;
						}
					topic[i]=ch;
				}
			topic[i]=0;
			display_help_topic(topic);
			return;
		}
////////////////////////


	if(my_strcompare(text_loc,"glinfo_"))
		{
			GLubyte *my_string;
			Uint8 this_string[8192];

			my_string=(GLubyte *)glGetString(GL_RENDERER);
			this_string[0]=0;
			my_strcp(&this_string[1],"Video Card: ");
			my_strcp(&this_string[strlen(this_string)],my_string);
			log_to_console(c_red2,this_string);

			my_string=(GLubyte *)glGetString(GL_VENDOR);
			this_string[0]=0;
			my_strcp(&this_string[1],"Vendor: ");
			my_strcp(&this_string[strlen(this_string)],my_string);
			log_to_console(c_yellow3,this_string);

			my_string=(GLubyte *)glGetString(GL_VERSION);
			this_string[0]=0;
			my_strcp(&this_string[1],"OpenGL Version: ");
			my_strcp(&this_string[strlen(this_string)],my_string);
			log_to_console(c_yellow2,this_string);

			my_string=(GLubyte *)glGetString(GL_EXTENSIONS);
			this_string[0]=0;
			my_strcp(&this_string[1],"Supported Extensions: ");
			my_strcp(&this_string[strlen(this_string)],my_string);
			log_to_console(c_grey1,this_string);

			return;
		}
	send_input_text_line();//no command, send it to the server, as plain text

}

