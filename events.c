#include	<string.h>
#include	"global.h"
#include	"elwindows.h"
#include	"keys.h"

#ifndef WINDOWS
#include <SDL_syswm.h>
#endif

int mod_key_status;
Uint32 last_turn_around=0;

int shift_on;
int alt_on;
int ctrl_on;

void	quick_use(int use_id)
{
	Uint8 quick_use_str[2];
	int	i;

	for(i=0; i<ITEM_NUM_ITEMS; i++){
		if(item_list[i].pos==use_id &&
			item_list[i].quantity &&
			item_list[i].use_with_inventory){
				quick_use_str[0]= USE_INVENTORY_ITEM;
				quick_use_str[1]= use_id;
				my_tcp_send(my_socket,quick_use_str,2);
				break;
		}
	}
}


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
				key=(Uint16)event->key.keysym.sym;
				
				if(shift_on)key|=(1<<31);
				if(ctrl_on)key|=(1<<30);
				if(alt_on)key|=(1<<29);
				
				//first, try to see if we pressed Alt+x, to quit.
				if ( (event->key.keysym.sym == SDLK_x && alt_on)
					 || (event->key.keysym.sym == SDLK_q && ctrl_on && !alt_on) )
					{
						done = 1;
						//break;
					}

				if(afk_time) 
					last_action_time=cur_time; //Set the latest event... Don't let the modifiers ALT, CTRL and SHIFT change the state

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

				if (event->key.keysym.sym == SDLK_RETURN && alt_on)
					{
						toggle_full_screen();
						break;
					}

				if (event->key.keysym.sym == SDLK_UP && interface_mode==interface_console)
					console_move_up();

				if(key==K_CAMERAUP && interface_mode==interface_game)
					if(rx>-60)rx-=1.0f;


				if (event->key.keysym.sym == SDLK_DOWN && interface_mode==interface_console)
					console_move_down();
						
				if(key==K_CAMERADOWN && interface_mode==interface_game)
					if(rx<-45)rx +=1.0f;

				if (event->key.keysym.sym == SDLK_PAGEDOWN && interface_mode==interface_console)
					console_move_page_down();

				if(key==K_ZOOMIN && interface_mode==interface_game){
					if(zoom_level>2.0f) {
						new_zoom_level=zoom_level-0.25;
					}
				}

				if(key==K_ZOOMOUT && interface_mode==interface_game){
					if(zoom_level<3.75f) {
						new_zoom_level=zoom_level+0.25;
					}
				}

				if (event->key.keysym.sym == SDLK_PAGEUP && interface_mode==interface_console)
					console_move_page_up();

				if(key==K_ITEM1){
					quick_use(0);
					break;
				} else if(key==K_ITEM2){
					quick_use(1);
					break;
				} else if(key==K_ITEM3){
					quick_use(2);
					break;
				} else if(key==K_ITEM4){
					quick_use(3);
					break;
				} else if(key==K_ITEM5){
					quick_use(4);
					break;
				} else if(key==K_ITEM6){
					quick_use(5);
					break;
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

				if(key==K_VIEWHP)
					{
						view_hp=!view_hp;
						break;
					}

				if(key==K_STATS)
					{
						view_window(&stats_win, 0);
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
						view_window(&options_win, 0);
						break;
					}
				
				if(key==K_KNOWLEDGE)
					{
						view_window(&knowledge_win,0);
						break;
					}

				if(key==K_ENCYCLOPEDIA)
					{
						view_window(&encyclopedia_win,0);
						break;
					}

				if(key==K_REPEATSPELL)	// REPEAT spell command
					{
						if(get_show_window(sigil_win))
							{
								if(!get_show_window(trade_win))
									{
										repeat_spell();
									}
							}
						break;
					}

				if(key==K_SIGILS)
					{
						view_window(&sigil_win,-1);
						break;
					}

				if(key==K_MANUFACTURE)
					{
						view_window(&manufacture_win,-1);
						break;
					}

				if(key==K_ITEMS)
					{
						view_window(&items_win,-1);
						break;
					}

				if (key==K_MAP)
					{
						view_map_win(&map_win,-1);
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


				if(key==K_AFK)
					{
						if(!afk) 
							{
								go_afk();
								last_action_time=cur_time-afk_time;
							}
						else go_ifk();
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
				if(event->key.keysym.sym==SDLK_F8)
				  have_point_sprite=!have_point_sprite;
				if(event->key.keysym.sym==SDLK_F9) {
				  actor *me=get_actor_ptr_from_id(yourself);
				  add_particle_sys("./particles/fire_small.part",me->x_pos+0.25f,me->y_pos+0.25f,-2.2f+height_map[me->y_tile_pos*tile_map_size_x*6+me->x_tile_pos]*0.2f+0.1f);
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
						input_text_lines=1;
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
						view_console_win(&console_win,-1);
						break;
					}

				if(key==K_SHADOWS)
					{
						clouds_shadows=!clouds_shadows;
						break;
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
								put_char_in_buffer(' ');
							}
						else
							{
								//not the shortcut, add the character to the buffer
								put_char_in_buffer(ch);
							}
					}
				if(ch==SDLK_BACKSPACE && input_text_lenght>0)
					{
						input_text_lenght--;
						if(input_text_line[input_text_lenght]==0x0a)
							{
								input_text_lenght--;
								if(input_text_lines>1)input_text_lines--;
								input_text_line[input_text_lenght]='_';
								input_text_line[input_text_lenght+1]=0;
							}
						input_text_line[input_text_lenght]='_';
						input_text_line[input_text_lenght+1]=0;
					}

				if(ch==SDLK_RETURN && input_text_lenght>0)
					{
						if(*input_text_line=='%' && input_text_lenght>1) 
							{
								input_text_line[input_text_lenght]=0;
								if((check_var(input_text_line+1,1))<0) send_input_text_line();
							}
						else if(*input_text_line=='#' || interface_mode==interface_console) test_for_console_command();
						else send_input_text_line();
						//also clear the buffer
						input_text_lenght=0;
						input_text_lines=1;
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

		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			if(afk_time)last_action_time=cur_time;//Set the latest events - don't make mousemotion set the afk_time... (if you prefer that mouse motion sets/resets the afk_time, then move this one step below...
		case SDL_MOUSEMOTION:
			if(event->type==SDL_MOUSEMOTION)
				{
					mouse_x= event->motion.x;
					mouse_y= event->motion.y;

					mouse_delta_x= event->motion.xrel;
					mouse_delta_y= event->motion.yrel;
				}
			else
				{
					mouse_x= event->button.x;
					mouse_y= event->button.y;
					mouse_delta_x= mouse_delta_y= 0;
				}

			if(event->type==SDL_MOUSEBUTTONDOWN) {
				if(event->button.button==SDL_BUTTON_LEFT)
					left_click++;
			}
			else if (event->type==SDL_MOUSEMOTION && (event->motion.state & SDL_BUTTON(SDL_BUTTON_LEFT)))
				left_click++;
			else
				{
					if(left_click) end_drag_windows();
					left_click = 0;
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

			if(event->type==SDL_MOUSEBUTTONDOWN) {
				if(event->button.button==SDL_BUTTON_MIDDLE)
					middle_click++;
			}
			else if (event->type==SDL_MOUSEMOTION && (event->motion.state
													  & SDL_BUTTON(SDL_BUTTON_MIDDLE)))
				middle_click++;
			else
				middle_click= 0;

			if(SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(2))
				{
					camera_rotation_speed=normal_camera_rotation_speed*mouse_delta_x/220;
					camera_rotation_frames=40;
					camera_tilt_speed=normal_camera_rotation_speed*mouse_delta_y/220;
					camera_tilt_frames=40;
				}

			if((left_click>=1) && interface_mode==interface_game) {
				if(check_drag_menus())return(done);
				if(check_scroll_bars())return(done);
			}

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
					else {
						if(camera_zoom_dir == -1)
							camera_zoom_frames+=5;
						else
							camera_zoom_frames=5;
						camera_zoom_dir=-1;
					}
				}
				if (event->button.button == SDL_BUTTON_WHEELDOWN) {
					if (interface_mode == interface_console)
						console_move_down();
					else {
						if(camera_zoom_dir == 1)
							camera_zoom_frames+=5;
						else
							camera_zoom_frames=5;
						camera_zoom_dir=1;
					}
				}
			}
			break;
		case SDL_USEREVENT:
			if (event->user.code == EVENT_MOVEMENT_TIMER) {
				pf_move();
			} else if(event->user.code == EVENT_UPDATE_CAMERA)update_camera();
			else if(event->user.code == EVENT_ANIMATE_ACTORS)animate_actors();
		}

	return(done);
}

