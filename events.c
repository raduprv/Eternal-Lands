#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "global.h"

#ifndef WINDOWS
#include <SDL_syswm.h>
#endif

int mod_key_status;
Uint32 last_turn_around=0;

int HandleEvent(SDL_Event *event)
{
	int done=0;
	Uint8 ch;

	if (event->type == SDL_NOEVENT) return 0;

	mod_key_status=SDL_GetModState();
	if(mod_key_status&KMOD_SHIFT)shift_on=1;
	else shift_on=0;

	mod_key_status=SDL_GetModState();
	if(mod_key_status&KMOD_ALT)alt_on=1;
	else alt_on=0;

	mod_key_status=SDL_GetModState();
	if(mod_key_status&KMOD_CTRL)ctrl_on=1;
	else ctrl_on=0;

	switch( event->type )
		{

#ifndef WINDOWS
		case SDL_SYSWMEVENT:
			{
				if(event->syswm.msg->event.xevent.type == SelectionNotify)
					finishpaste(event->syswm.msg->event.xevent.xselection);

			}
			break;
#endif
	    case SDL_KEYDOWN:
			{
				//first, try to see if we pressed Alt+x, to quit.
				if ( (event->key.keysym.sym == SDLK_x && alt_on)
					 || (event->key.keysym.sym == SDLK_q && ctrl_on) )
					{
						done = 1;
						//break;
					}

#ifndef WINDOWS
				if((event->key.keysym.sym == SDLK_v && ctrl_on) ||
				   (event->key.keysym.sym == SDLK_INSERT && shift_on))
					startpaste();
#else
				if((event->key.keysym.sym == SDLK_v && ctrl_on) ||
				   (event->key.keysym.sym == SDLK_INSERT && shift_on))
					windows_paste();
#endif
				if(interface_mode==interface_opening && !disconnected)
					{
						interface_mode=interface_log_in;
						break;
					}

				if(disconnected && !alt_on && !ctrl_on)
					{
						connect_to_server();
						break;
					}

				if ( event->key.keysym.sym == SDLK_RETURN && alt_on)
					{
						toggle_full_screen();
						break;
					}

				if ( event->key.keysym.sym == SDLK_UP)
					{
						if(interface_mode==interface_console)console_move_up();
						else
							{
								if(rx>-60)rx-=1.0f;
							}

					}


				if ( event->key.keysym.sym == SDLK_DOWN )
					{
						if(interface_mode==interface_console)console_move_down();
						else
							{
								if(rx<-45)rx +=1.0f;
							}

					}

				if ( event->key.keysym.sym == SDLK_PAGEDOWN )
					{
						if(interface_mode==interface_console)console_move_page_down();
						else if(zoom_level<3.75f)
							{
								zoom_level+=0.25f;
								resize_window();
							}
					}

				if ( event->key.keysym.sym == SDLK_PAGEUP )
					{
						if(interface_mode==interface_console)console_move_page_up();
						else if(zoom_level>2.0f)
							{
								zoom_level-=0.25f;
								resize_window();
							}
					}

				if ( event->key.keysym.sym == SDLK_INSERT )
					{
						if(!last_turn_around || last_turn_around+500<cur_time)
							{
								Uint8 str[2];
								last_turn_around=cur_time;
								str[0]=TURN_LEFT;
								my_tcp_send(my_socket,str,1);
							}
					}

				if ( event->key.keysym.sym == SDLK_DELETE )
					{
						if(!last_turn_around || last_turn_around+500<cur_time)
							{
								Uint8 str[2];
								last_turn_around=cur_time;
								str[0]=TURN_RIGHT;
								my_tcp_send(my_socket,str,1);
							}
					}

				if ( event->key.keysym.sym == SDLK_HOME )
					{
						move_self_forward();
					}


				if( event->key.keysym.sym == SDLK_g && ctrl_on)
					{
						//get_updates();
						get_knowledge_list("this,that,other,this1,that1,other1,this2,that2,other2,this3,that3,other3,this4,that4,other4,this5,that5,other5,this,that,other,this1,that1,other1,this2,that2,other2,this3,that3,other3,this4,that4,other4,this5,that5,other5,this,that,other,this1,that1,other1,this2,that2,other2,this3,that3,other3,this4,that4,other4,this5,that5,other5,this,that,other,this1,that1,other1,this2,that2,other2,this3,that3,other3,this4,that4,other4,this5,that5,other5,this,that,other,this1,that1,other1,this2,that2,other2,this3,that3,other3,this4,that4,other4,this5,that5,other5,this,that,other,this1,that1,other1,this2,that2,other2,this3,that3,other3,this4,that4,other4,this5,that5,other5,this,that,other,this1,that1,other1,this2,that2,other2,this3,that3,other3,this4,that4,other4,this5,that5,other5,this,that,other,this1,that1,other1,this2,that2,other2,this3,that3,other3,this4,that4,other4,this5,that5,other5");
						break;
					}

				if( event->key.keysym.sym == SDLK_h && alt_on)
					{
						view_health_bar=!view_health_bar;
						break;
					}


				if( event->key.keysym.sym == SDLK_n && alt_on)
					{
						view_names=!view_names;
						break;
					}

				if( event->key.keysym.sym == SDLK_a && ctrl_on)
					{
						view_self_stats=!view_self_stats;
						break;
					}

				if( event->key.keysym.sym == SDLK_w && ctrl_on)
					{
						action_mode=action_walk;
						break;
					}

				if( event->key.keysym.sym == SDLK_l && ctrl_on)
					{
						action_mode=action_look;
						break;
					}

				if( event->key.keysym.sym == SDLK_u && ctrl_on)
					{
						action_mode=action_use;
						break;
					}

				if( event->key.keysym.sym == SDLK_o && ctrl_on)
					{
						options_menu=!options_menu;
						break;
					}

				if( event->key.keysym.sym == SDLK_r && ctrl_on)	// REPEAT spell command
					{
						if(view_sigils_menu)
							{
								repeat_spell();
							}
						break;
					}

				if( event->key.keysym.sym == SDLK_s && ctrl_on)
					{
						if(view_trade_menu)
							{
								log_to_console(c_red2,"You can't cast spells while on trade.");
								return(done);
							}
						view_sigils_menu=!view_sigils_menu;
						break;
					}

				if( event->key.keysym.sym == SDLK_m && ctrl_on)
					{
						if(!view_manufacture_menu)
							{
								if(view_trade_menu)
									{
										log_to_console(c_red2,"You can't manufacture while on trade.");
										break;
									}
							}
						view_manufacture_menu=!view_manufacture_menu;
						break;
					}

				if( event->key.keysym.sym == SDLK_i && ctrl_on)
					{
						if(!view_my_items)
							{
								if(view_trade_menu)
									{
										log_to_console(c_red2,"You can't view your inventory items while on trade.");
										break;
									}
								view_my_items=1;
							}
						else view_my_items=0;
						break;
					}

				if ( event->key.keysym.sym == SDLK_TAB && !alt_on)
					{
						if(interface_mode==interface_game)switch_to_game_map();
						else if(interface_mode==interface_map)switch_from_game_map();
					}

				if ( event->key.keysym.sym == SDLK_LEFT )
					{
						if(shift_on)
							{
								camera_rotation_speed=fine_camera_rotation_speed/10;
								camera_rotation_frames=10;
							}
						else
							{
								camera_rotation_speed=normal_camera_rotation_speed/40;
								camera_rotation_frames=40;
							}

					}
				if ( event->key.keysym.sym == SDLK_RIGHT )
					{
						if(shift_on)
							{
								camera_rotation_speed=-fine_camera_rotation_speed/10;
								camera_rotation_frames=10;
							}
						else
							{
								camera_rotation_speed=-normal_camera_rotation_speed/40;
								camera_rotation_frames=40;
							}
					}

				if(event->key.keysym.sym==SDLK_F2)
					{
#ifndef WINDOWS
						char browser_command[400];
						if(have_url)
							{
								my_strcp(browser_command,broswer_name);
								my_strcat(browser_command," \"");
								my_strcat(browser_command,current_url);
								my_strcat(browser_command,"\"&");
								system(browser_command);
							}
#else
						SDL_Thread *go_to_url_thread;
						go_to_url_thread=SDL_CreateThread(go_to_url, 0);
#endif
					}

				if ( event->key.keysym.sym == SDLK_ESCAPE)
					{
						input_text_lenght=0;
						input_text_line[0]=0;
					}

				//see if we get any text
				ch = event->key.keysym.unicode;// & 0x7F;

				if(interface_mode==interface_log_in)
					{
						if(ch==SDLK_RETURN && username_str[0] && password_str[0])
							{
								send_login_info();
								break;
							}
						else
							if(username_box_selected)add_char_to_username(ch);
							else
								add_char_to_password(ch);
						break;
					}

				if(interface_mode==interface_new_char)
					{
						add_char_to_new_character(ch);
						break;
					}

				if(ch=='`' || event->key.keysym.sym==SDLK_F1)
					{
						if(interface_mode==interface_console)interface_mode=interface_game;
						else interface_mode=interface_console;
						break;
					}

				if(event->key.keysym.sym==SDLK_F3)
					{
						clouds_shadows=!clouds_shadows;
					}



				if(((ch>=32 && ch<=126) || (ch>127+c_grey4)) &&
				   input_text_lenght<160)
					{
						//watch for the '//' shortcut
						if(input_text_lenght==1 && ch=='/' && input_text_line[0]=='/' && last_pm_from[0])
							{
								int i;
								int l=strlen(last_pm_from);
								for(i=0;i<l;i++)input_text_line[input_text_lenght++]=last_pm_from[i];
								input_text_line[input_text_lenght]=' ';	//and a space after it
								input_text_line[input_text_lenght+1]='_';
								input_text_line[input_text_lenght+2]=0;
								input_text_lenght++;
							}
						else
							{
								//not the shortcut, add the character to the buffer
								input_text_line[input_text_lenght]=ch;
								input_text_line[input_text_lenght+1]='_';
								input_text_line[input_text_lenght+2]=0;
								input_text_lenght++;
								if(input_text_lenght==window_width/11-1)
									{
										input_text_line[input_text_lenght]=0x0a;
										input_text_line[input_text_lenght+1]='_';
										input_text_line[input_text_lenght+2]=0;
										input_text_lenght++;
									}
							}
					}
				if(ch==SDLK_BACKSPACE && input_text_lenght>0)
					{
						input_text_lenght--;
						if(input_text_line[input_text_lenght]==0x0a)
							{
								input_text_lenght--;
								input_text_line[input_text_lenght]='_';
								input_text_line[input_text_lenght+1]=0;
							}
						input_text_line[input_text_lenght]='_';
						input_text_line[input_text_lenght+1]=0;
					}

				if(ch==SDLK_RETURN && input_text_lenght>0)
					{
						if(*input_text_line=='#' || interface_mode==interface_console) test_for_console_command();
						else send_input_text_line();
						//also clear the buffer
						input_text_lenght=0;
						input_text_line[0]=0;
					}
				break;
			}//key down

		case SDL_VIDEORESIZE:
	    	{
	    	 	window_width = event->resize.w;
	      		window_height = event->resize.h;
		  		resize_window();
	    	}
			break;

	    case SDL_QUIT:
			done = 1;
			break;


		}

	if(event->type==SDL_MOUSEMOTION || event->type==SDL_MOUSEBUTTONDOWN ||
	   event->type==SDL_MOUSEBUTTONUP)
		{
			mouse_x=event->motion.x;
			mouse_y=event->motion.y;

			mouse_delta_x = event->motion.xrel;
			mouse_delta_y = event->motion.yrel;

			if(event->type==SDL_MOUSEBUTTONDOWN) {
				if(event->button.button==SDL_BUTTON_LEFT)
					left_click++;
			}
			else if (event->type==SDL_MOUSEMOTION && (event->motion.state & SDL_BUTTON(SDL_BUTTON_LEFT)))
				left_click++;
			else
				{
					left_click = 0;
					attrib_menu_dragged=0;
					items_menu_dragged=0;
					ground_items_menu_dragged=0;
					manufacture_menu_dragged=0;
					trade_menu_dragged=0;
					sigil_menu_dragged=0;
					options_menu_dragged=0;
					dialogue_menu_dragged=0;
					knowledge_menu_dragged=0;
				}

			if(event->type==SDL_MOUSEBUTTONDOWN) {
				if(event->button.button==SDL_BUTTON_RIGHT)
					right_click++;
			}
			else if (event->type==SDL_MOUSEMOTION && (event->motion.state
													  & SDL_BUTTON(SDL_BUTTON_RIGHT)))
				right_click++;
			else
				right_click= 0;

			if(SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(2))
				{

					camera_rotation_speed=normal_camera_rotation_speed*mouse_delta_x/220;
					camera_rotation_frames=40;
				}

			if((left_click>=1) && interface_mode==interface_game)
				if(check_drag_menus())return(done);

			if((left_click==1 || right_click==1) &&
			   interface_mode==interface_game)
				check_mouse_click();
			else
				if((left_click==1 || right_click==1) &&
				   interface_mode==interface_opening && !disconnected)
					{
						interface_mode=interface_log_in;
						left_click=2;
					}

		}

	return(done);
}

