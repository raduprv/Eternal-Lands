#include <time.h>
#include "global.h"

void log_to_console(Uint8 color,Uint8 *buffer);


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

	//cls?
	if(strcmp(input_text_line,"cls_")==0)
		{
			cls();
			return;
		}
	//stats ?
	if(strcmp(input_text_line,"stats_")==0)
		{
			unsigned char protocol_name;
			protocol_name=SERVER_STATS;
			my_tcp_send(my_socket,&protocol_name,1);
			return;
		}
	//time?
	if(strcmp(input_text_line,"time_")==0)
		{
			unsigned char protocol_name;
			protocol_name=GET_TIME;
			my_tcp_send(my_socket,&protocol_name,1);
			return;
		}
	//ping?
	if(strcmp(input_text_line,"ping_")==0)
		{
			Uint8 str[8];
			str[0]=PING;
			*((Uint32 *)(str+1))=SDL_GetTicks();
			my_tcp_send(my_socket,str,5);
			return;
		}

	//date?
	if(strcmp(input_text_line,"date_")==0)
		{
			unsigned char protocol_name;
			protocol_name=GET_DATE;
			my_tcp_send(my_socket,&protocol_name,1);
			return;
		}
	if(strcmp(input_text_line,"quit_")==0)
		{
			exit_now=1;
			return;
		}
	if(strcmp(input_text_line,"exit_")==0)
		{
			exit_now=1;
			return;
		}
	if(strcmp(input_text_line,"ver_")==0)
		{
			char version[60];
			version[0]=127+c_green1;
			my_strcp(&version[1],"Eternal Lands Version 0.9.3 Beta");
			put_text_in_buffer(version, strlen(version), 0);
			return;
		}

	if(strcmp(input_text_line,"ignores_")==0)
		{
			list_ignores();
			return;
		}
	if(input_text_line[0]=='i' && input_text_line[1]=='g' && input_text_line[2]=='n'
	&& input_text_line[3]=='o' && input_text_line[4]=='r' && input_text_line[5]=='e')
		{
			Uint8 name[16];
			int i;
			Uint8 ch;
			int result;

			for(i=0;i<15;i++)
				{
					ch=input_text_line[i+7];//7 because there is a space after "ignore"
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
					Uint8 str[100];
					str[0]=127+c_red1;
					my_strcp(&str[1],"Name too long, the max limit is 15 characters. Name not added to the ignore list!");
					put_text_in_buffer(str,strlen(str),0);
					return;
				}
			if(i<3)
				{
					Uint8 str[100];
					str[0]=127+c_red1;
					my_strcp(&str[1],"Name too short, only names>=3 characters can be used! Name not added to the ignore list!");
					put_text_in_buffer(str,strlen(str),0);
					return;
				}

			result=add_to_ignore_list(name,save_ignores);
			if(result==-1)
				{
					Uint8 str[100];
					str[0]=127+c_red1;
					sprintf(&str[1],"You are already ignoring %s!",name);
					put_text_in_buffer(str,strlen(str),0);
					return;
				}
			if(result==-2)
				{
					Uint8 str[100];
					str[0]=127+c_red1;
					my_strcp(&str[1],"Wow, your ignore list is full, this is impossible!");
					put_text_in_buffer(str,strlen(str),0);
					return;
				}
			else
				{
					Uint8 str[100];
					str[0]=127+c_green1;
					sprintf(&str[1],"Ok, %s was added to your ignore list!",name);
					put_text_in_buffer(str,strlen(str),0);
					return;
				}
		}


////////////////////////
	if(input_text_line[0]=='m' && input_text_line[1]=='o' && input_text_line[2]=='d' &&
	input_text_line[3]=='e' && input_text_line[4]=='s' && input_text_line[5]=='_')
		{
			char str[1000];
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
	if(input_text_line[0]=='m' && input_text_line[1]=='o' && input_text_line[2]=='d' &&
	input_text_line[3]=='e' && input_text_line[4]==' ' && input_text_line[5]>='1' && input_text_line[5]<='6')
		{

		}

////////////////////////
	if(input_text_line[0]=='u' && input_text_line[1]=='n' && input_text_line[2]=='i' &&
	input_text_line[3]=='g' && input_text_line[4]=='n' && input_text_line[5]=='o' &&
	input_text_line[6]=='r' && input_text_line[7]=='e')
		{
			Uint8 name[16];
			int i;
			Uint8 ch;
			int result;

			for(i=0;i<15;i++)
				{
					ch=input_text_line[i+9];//9 because there is a space after "ignore"
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
					str[0]=127+c_red1;
					my_strcp(&str[1],"Name too long, the max limit is 15 characters. Name not removed from the ignore list!");
					put_text_in_buffer(str,strlen(str),0);
					return;
				}
			if(i<3)
				{
					Uint8 str[200];
					str[0]=127+c_red1;
					my_strcp(&str[1],"Name too short, only names>=3 characters can be used! Name not removed from the ignore list!");
					put_text_in_buffer(str,strlen(str),0);
					return;
				}
			result=remove_from_ignore_list(name);
			if(result==-1)
				{
					Uint8 str[200];
					str[0]=127+c_red1;
					sprintf(&str[1],"You are NOT ignoring %s in the first place!",name);
					put_text_in_buffer(str,strlen(str),0);
					return;
				}
			else
				{
					Uint8 str[100];
					str[0]=127+c_green1;
					sprintf(&str[1],"Ok, %s was removed from your ignore list!",name);
					put_text_in_buffer(str,strlen(str),0);
					return;
				}
		}
////////////////////////
	if((input_text_lenght>=5 && input_text_line[0]=='h' && input_text_line[1]=='e' && input_text_line[2]=='l' &&
	input_text_line[3]=='p' && input_text_line[4]==' ') ||
	(input_text_lenght==4 && input_text_line[0]=='h' && input_text_line[1]=='e' && input_text_line[2]=='l' &&
	input_text_line[3]=='p'))
		{
			Uint8 topic[30];
			int i;
			Uint8 ch;
			int result;

			if(input_text_lenght==4)
				{
					display_help_topic("main");
					return;
				}
			for(i=0;i<30 || i<input_text_lenght;i++)
				{
					ch=input_text_line[i+5];//5 because there is a space after "help"
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


	if(strcmp(input_text_line,"glinfo_")==0)
		{
			GLubyte *my_string;
			Uint8 this_string[8192];

			my_string=(GLubyte *)glGetString(GL_RENDERER);
			this_string[0]=127+c_red2;
			my_strcp(&this_string[1],"Video Card: ");
			my_strcp(&this_string[strlen(this_string)],my_string);
			put_text_in_buffer(this_string, strlen(this_string), 0);

			my_string=(GLubyte *)glGetString(GL_VENDOR);
			this_string[0]=127+c_yellow3;
			my_strcp(&this_string[1],"Vendor: ");
			my_strcp(&this_string[strlen(this_string)],my_string);
			put_text_in_buffer(this_string, strlen(this_string), 0);

			my_string=(GLubyte *)glGetString(GL_VERSION);
			this_string[0]=127+c_yellow2;
			my_strcp(&this_string[1],"OpenGL Version: ");
			my_strcp(&this_string[strlen(this_string)],my_string);
			put_text_in_buffer(this_string, strlen(this_string), 0);

			my_string=(GLubyte *)glGetString(GL_EXTENSIONS);
			this_string[0]=127+c_grey1;
			my_strcp(&this_string[1],"Supported Extensions: ");
			my_strcp(&this_string[strlen(this_string)],my_string);
			put_text_in_buffer(this_string, strlen(this_string), 0);

			return;
		}
	send_input_text_line();//no command, send it to the server, as plain text

}

void log_to_console(Uint8 color,Uint8 *buffer)
{
			Uint8 str[9000];
			str[0]=127+color;
			my_strcp(&str[1],buffer);
			put_text_in_buffer(str,strlen(str),0);
}


