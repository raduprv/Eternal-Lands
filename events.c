#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "global.h"

#ifndef WINDOWS
#include <SDL_syswm.h>
#endif

int mod_key_status;
Uint32 last_turn_around=0;

int shift_on;
int alt_on;
int ctrl_on;

#define SHIFT (1<<31)
#define CTRL (1<<30)
#define ALT (1<<29)

Uint32 K_CAMERAUP=273;
Uint32 K_CAMERADOWN=274;
Uint32 K_ZOOMOUT=281;
Uint32 K_ZOOMIN=280;
Uint32 K_TURNLEFT=277;
Uint32 K_TURNRIGHT=127;
Uint32 K_ADVANCE=278;
Uint32 K_HEALTHBAR=ALT|'h';
Uint32 K_VIEWNAMES=ALT|'n';
Uint32 K_STATS=CTRL|'a';
Uint32 K_WALK=CTRL|'w';
Uint32 K_LOOK=CTRL|'l';
Uint32 K_USE=CTRL|'u';
Uint32 K_OPTIONS=CTRL|'o';
Uint32 K_REPEATSPELL=CTRL|'r';
Uint32 K_SIGILS=CTRL|'s';
Uint32 K_MANUFACTURE=CTRL|'m';
Uint32 K_ITEMS=CTRL|'i';
Uint32 K_MAP=9;
Uint32 K_ROTATELEFT=276;
Uint32 K_ROTATERIGHT=275;
Uint32 K_FROTATELEFT=SHIFT|276;
Uint32 K_FROTATERIGHT=SHIFT|275;
Uint32 K_BROWSER=283;
Uint32 K_ESCAPE=27;
Uint32 K_CONSOLE=282;
Uint32 K_SHADOWS=284;
Uint32 K_KNOWLEDGE=CTRL|'k';
Uint32 K_ENCYCLOPEDIA=CTRL|'e';
Uint32 K_ITEM1=CTRL|'1';
Uint32 K_ITEM2=CTRL|'2';
Uint32 K_ITEM3=CTRL|'3';
Uint32 K_ITEM4=CTRL|'4';
Uint32 K_ITEM5=CTRL|'5';
Uint32 K_ITEM6=CTRL|'6';
Uint32 K_VIEWTEXTASOVERTEXT=ALT|'o';

Uint8 quick_use_str[2];
int i=0;


int HandleEvent(SDL_Event *event)
{
	int done=0;
	Uint8 ch;
	Uint32 key=0;

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
				(Uint16)key=(Uint16)event->key.keysym.sym;
				if(shift_on)key|=(1<<31);
				if(ctrl_on)key|=(1<<30);
				if(alt_on)key|=(1<<29);
				

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

				if ( event->key.keysym.sym == SDLK_UP && interface_mode==interface_console)
					console_move_up();

				if(key==K_CAMERAUP && interface_mode==interface_game)
					if(rx>-60)rx-=1.0f;


				if ( event->key.keysym.sym == SDLK_DOWN && interface_mode==interface_console)
					console_move_down();
						
				if(key==K_CAMERADOWN && interface_mode==interface_game)
					if(rx<-45)rx +=1.0f;

				if ( event->key.keysym.sym == SDLK_PAGEDOWN && interface_mode==interface_console)
					console_move_page_down();

				if(key==K_ZOOMOUT && interface_mode==interface_game){
					if(zoom_level<3.75f){
						zoom_level+=0.25f;
						resize_window();
					}
				}

				if ( event->key.keysym.sym == SDLK_PAGEUP && interface_mode==interface_console)
					console_move_page_up();

				if(key==K_ITEM1){
				  for(i=0;i<ITEM_NUM_ITEMS;i++){
				    if(item_list[i].pos==0 &&
				       item_list[i].quantity &&
				       item_list[i].use_with_inventory){
				      quick_use_str[0]=USE_INVENTORY_ITEM;
				      quick_use_str[1]=0;
				      my_tcp_send(my_socket,quick_use_str,2);
				      break;
				    }
				  }
				  break;
				}
				if(key==K_ITEM2){
				  for(i=0;i<ITEM_NUM_ITEMS;i++){
				    if(item_list[i].pos==1 &&
				       item_list[i].quantity &&
				       item_list[i].use_with_inventory){
				      quick_use_str[0]=USE_INVENTORY_ITEM;
				      quick_use_str[1]=1;
				      my_tcp_send(my_socket,quick_use_str,2);
				      break;
				    }
				  }
				  break;
				}
				if(key==K_ITEM3){
				  for(i=0;i<ITEM_NUM_ITEMS;i++){
				    if(item_list[i].pos==2 &&
				       item_list[i].quantity &&
				       item_list[i].use_with_inventory){
				      quick_use_str[0]=USE_INVENTORY_ITEM;
				      quick_use_str[1]=2;
				      my_tcp_send(my_socket,quick_use_str,2);
				      break;
				    }
				  }
				  break;
				}
				if(key==K_ITEM4){
				  for(i=0;i<ITEM_NUM_ITEMS;i++){
				    if(item_list[i].pos==3 &&
				       item_list[i].quantity &&
				       item_list[i].use_with_inventory){
				      quick_use_str[0]=USE_INVENTORY_ITEM;
				      quick_use_str[1]=3;
				      my_tcp_send(my_socket,quick_use_str,2);
				      break;
				    }
				  }
				  break;
				}
				if(key==K_ITEM5){
				  for(i=0;i<ITEM_NUM_ITEMS;i++){
				    if(item_list[i].pos==4 &&
				       item_list[i].quantity &&
				       item_list[i].use_with_inventory){
				      quick_use_str[0]=USE_INVENTORY_ITEM;
				      quick_use_str[1]=4;
				      my_tcp_send(my_socket,quick_use_str,2);
				      break;
				    }
				  }
				  break;
				}
				if(key==K_ITEM6){
				  for(i=0;i<ITEM_NUM_ITEMS;i++){
				    if(item_list[i].pos==5 &&
				       item_list[i].quantity &&
				       item_list[i].use_with_inventory){
				      quick_use_str[0]=USE_INVENTORY_ITEM;
				      quick_use_str[1]=5;
				      my_tcp_send(my_socket,quick_use_str,2);
				      break;
				    }
				  }
				  break;
				}



				if(key==K_ZOOMIN && interface_mode==interface_game){
					if(zoom_level>2.0f){
						zoom_level-=0.25f;
						resize_window();
					}
				}

				if (key==K_TURNLEFT)
					{
						if(!last_turn_around || last_turn_around+500<cur_time)
							{
								Uint8 str[2];
								last_turn_around=cur_time;
								str[0]=TURN_LEFT;
								my_tcp_send(my_socket,str,1);
							}
					}

				if (key==K_TURNRIGHT)
					{
						if(!last_turn_around || last_turn_around+500<cur_time)
							{
								Uint8 str[2];
								last_turn_around=cur_time;
								str[0]=TURN_RIGHT;
								my_tcp_send(my_socket,str,1);
							}
					}

				if (key==K_ADVANCE)
					{
						move_self_forward();
					}

				if(key==K_HEALTHBAR)
					{
						view_health_bar=!view_health_bar;
						break;
					}


				if(key==K_VIEWTEXTASOVERTEXT)
				{
					view_chat_text_as_overtext=!view_chat_text_as_overtext;
					break;
				}

				if(key==K_VIEWNAMES)
					{
						view_names=!view_names;
						break;
					}

				if(key==K_STATS)
					{
						view_self_stats=!view_self_stats;
						break;
					}

				if(key==K_WALK)
					{
						action_mode=action_walk;
						break;
					}

				if(key==K_LOOK)
					{
						action_mode=action_look;
						break;
					}

				if(key==K_USE)
					{
						action_mode=action_use;
						break;
					}

				if(key==K_OPTIONS)
					{
						options_menu=!options_menu;
						break;
					}
				
				if(key==K_KNOWLEDGE)
					{
						view_knowledge=!view_knowledge;
						break;
					}

				if(key==K_ENCYCLOPEDIA)
					{
						view_encyclopedia=!view_encyclopedia;
						break;
					}

				if(key==K_REPEATSPELL)	// REPEAT spell command
					{
						if(view_sigils_menu)
							{
								repeat_spell();
							}
						break;
					}

				if(key==K_SIGILS)
					{
						if(view_trade_menu)
							{
								log_to_console(c_red2,"You can't cast spells while on trade.");
								return(done);
							}
						view_sigils_menu=!view_sigils_menu;
						break;
					}

				if(key==K_MANUFACTURE)
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

				if(key==K_ITEMS)
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

				if (key==K_MAP)
					{
						if(interface_mode==interface_game)switch_to_game_map();
						else if(interface_mode==interface_map)switch_from_game_map();
					}

				if (key==K_ROTATELEFT)
					{
						camera_rotation_speed=normal_camera_rotation_speed/40;
						camera_rotation_frames=40;
					}
				if(key==K_FROTATELEFT)
					{
						camera_rotation_speed=fine_camera_rotation_speed/10;
						camera_rotation_frames=10;
					}

				if (key==K_ROTATERIGHT)
					{
						camera_rotation_speed=-normal_camera_rotation_speed/40;
						camera_rotation_frames=40;
					}

				if(key==K_FROTATERIGHT)
					{
						camera_rotation_speed=-fine_camera_rotation_speed/10;
						camera_rotation_frames=10;
					}


				if(key==K_BROWSER)
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

//TEST REMOVE LATER!!!!!!!!!!!!!!!!!!!!!!
				if(event->key.keysym.sym==SDLK_F4)
					{
						add_circular_burst(102, 142,1900, 0.5f, 0.5f, 1.0f, 0.3f);
					}

				if(event->key.keysym.sym==SDLK_F5)
					{
						add_circular_burst(102, 142,1900, 0.9f, 0.2f, 0.3f, 0.3f);
					}

				if(event->key.keysym.sym==SDLK_F6)
					{
						if(!hud_x)
							{
								hud_x=64;
								hud_y=49;
							}
						else
							{
								hud_x=0;
								hud_y=0;
							}
						resize_window();
					}
//END OF TEST!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1

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

				if(ch=='`' || key==K_CONSOLE)
					{
						if(interface_mode==interface_console)interface_mode=interface_game;
						else interface_mode=interface_console;
						break;
					}

				if(key==K_SHADOWS)
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
								if(input_text_lenght==(window_width-hud_x)/11-1)
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

		case SDL_MOUSEMOTION:
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
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
					encyclopedia_menu_dragged=0;
					buddy_menu_dragged=0;
					questlog_menu_dragged=0;
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
			else if((left_click==1 || right_click==1) &&
			   interface_mode==interface_map)
			   	pf_move_to_mouse_position();
			else if((left_click==1 || right_click==1) &&
			   interface_mode==interface_console)
				check_hud_interface();
			else
				if((left_click==1 || right_click==1) &&
				   interface_mode==interface_opening && !disconnected)
					{
						interface_mode=interface_log_in;
						left_click=2;
					}
			if (event->type == SDL_MOUSEBUTTONDOWN) {
				if (event->button.button == SDL_BUTTON_WHEELUP) {
					if (interface_mode == interface_console)
						console_move_up();
					else if (zoom_level > 2.0f) {
						zoom_level -= 0.25f;
						resize_window();
					}
				}
				if (event->button.button == SDL_BUTTON_WHEELDOWN) {
					if (interface_mode == interface_console)
						console_move_down();
					else if (zoom_level < 3.75f) {
						zoom_level += 0.25f;
						resize_window();
					}
				}
			}
			break;
		case SDL_USEREVENT:
			if (event->user.code == SDL_PF_MOVEMENT_TIMER) {
				pf_move();
			}
		}

	return(done);
}

