#include <string.h>
#include "global.h"
#include "elwindows.h"
#include <math.h>

int mouse_x;
int mouse_y;
int mouse_delta_x;
int mouse_delta_y;
int right_click;
int middle_click;
int left_click;
int open_text;
int login_screen_menus;
char username_box_selected=1;
char password_box_selected=0;
char username_str[16]={0};
char password_str[16]={0};
char display_password_str[16]={0};
int username_text_lenght=0;
int password_text_lenght=0;

int have_a_map=0;
char interface_mode=interface_opening;
char create_char_error_str[520];
char log_in_error_str[520];
int options_menu=0;
int combat_mode=0;
int auto_camera=0;
int view_health_bar=1;
int view_names=1;
int view_chat_text_as_overtext=0;
int show_fps=1;
int limit_fps=0;
int	options_win= 0;

int action_mode=action_walk;

int you_sit=0;
int sit_lock=0;

Uint32 click_time=0;
int click_speed=300;

void get_world_x_y()
{
	float window_ratio;
	float x,y,x1,y1,a,t;
	window_ratio=(GLfloat)window_width/(GLfloat)window_height;

	x=(float)((mouse_x)*2.8f*zoom_level/(window_width-hud_x))-(2.8*zoom_level/2.0f);
	y=(float)((window_height-hud_y-mouse_y)*2.0f*zoom_level/(window_height-hud_y))-(2.0*zoom_level/2.0f);

	a=(rz)*3.1415926/180;
	t=(rx)*3.1415926/180;

	y=y/cos(t);

	x1=x*cos(a)+y*sin(a);
	y1=y*cos(a)-x*sin(a);

	scene_mouse_x=-cx+x1;
	scene_mouse_y=-cy+y1;
}

int check_drag_menus()
{
	if(drag_windows(mouse_x, mouse_y, mouse_delta_x, mouse_delta_y))	return 1;

	if(sigil_menu_dragged || (view_sigils_menu && mouse_x>sigil_menu_x && mouse_x<=sigil_menu_x+sigil_menu_x_len && mouse_y>sigil_menu_y-16 && mouse_y<=sigil_menu_y))
		if(!attrib_menu_dragged && !items_menu_dragged && !ground_items_menu_dragged && !manufacture_menu_dragged &&
		   !trade_menu_dragged && !options_menu_dragged && !dialogue_menu_dragged && !knowledge_menu_dragged && !encyclopedia_menu_dragged && !questlog_menu_dragged && !questlog_menu_dragged && !buddy_menu_dragged)
			{
				sigil_menu_dragged=1;
				if(left_click>1)
					{
						sigil_menu_x+=mouse_delta_x;
						sigil_menu_y+=mouse_delta_y;
					}
				return 1;
			}

	/*
	if(options_menu_dragged || (options_menu && mouse_x>options_menu_x && mouse_x<=options_menu_x + options_menu_x_len && mouse_y>options_menu_y-16 && mouse_y<=options_menu_y))
		if(!attrib_menu_dragged && !items_menu_dragged && !ground_items_menu_dragged && !manufacture_menu_dragged &&
		   !trade_menu_dragged && !sigil_menu_dragged && !dialogue_menu_dragged && !knowledge_menu_dragged && !encyclopedia_menu_dragged && !questlog_menu_dragged && !questlog_menu_dragged && !buddy_menu_dragged)

			{
				options_menu_dragged=1;
				if(left_click>1)
					{
						options_menu_x+=mouse_delta_x;
						options_menu_y+=mouse_delta_y;
					}
				return 1;
			}
	*/

	if(trade_menu_dragged || (view_trade_menu && mouse_x>trade_menu_x && mouse_x<=trade_menu_x+trade_menu_x_len && mouse_y>trade_menu_y-16 && mouse_y<=trade_menu_y))
		if(!attrib_menu_dragged && !items_menu_dragged && !ground_items_menu_dragged && !manufacture_menu_dragged &&
		   !sigil_menu_dragged && !options_menu_dragged && !dialogue_menu_dragged && !knowledge_menu_dragged && !encyclopedia_menu_dragged && !questlog_menu_dragged && !questlog_menu_dragged && !buddy_menu_dragged)

			{
				trade_menu_dragged=1;
				if(left_click>1)
					{
						trade_menu_x+=mouse_delta_x;
						trade_menu_y+=mouse_delta_y;
					}
				return 1;
			}

	/*
	if(manufacture_menu_dragged || (view_manufacture_menu && mouse_x>manufacture_menu_x && mouse_x<=manufacture_menu_x+manufacture_menu_x_len && mouse_y>manufacture_menu_y-16 && mouse_y<=manufacture_menu_y))
		if(!attrib_menu_dragged && !items_menu_dragged && !ground_items_menu_dragged &&
		   !trade_menu_dragged && !sigil_menu_dragged && !options_menu_dragged && !dialogue_menu_dragged && !knowledge_menu_dragged && !encyclopedia_menu_dragged && !questlog_menu_dragged && !questlog_menu_dragged && !buddy_menu_dragged)

			{
				manufacture_menu_dragged=1;
				if(left_click>1)
					{
						manufacture_menu_x+=mouse_delta_x;
						manufacture_menu_y+=mouse_delta_y;
					}
				return 1;
			}
	*/

	if(ground_items_menu_dragged || (view_ground_items && mouse_x>ground_items_menu_x && mouse_x<=ground_items_menu_x+ground_items_menu_x_len && mouse_y>ground_items_menu_y-16 && mouse_y<=ground_items_menu_y))
		if(!attrib_menu_dragged && !items_menu_dragged && !manufacture_menu_dragged &&
		   !trade_menu_dragged && !sigil_menu_dragged && !options_menu_dragged && !dialogue_menu_dragged && !knowledge_menu_dragged && !encyclopedia_menu_dragged && !questlog_menu_dragged && !questlog_menu_dragged && !buddy_menu_dragged)

			{
				ground_items_menu_dragged=1;
				if(left_click>1)
					{
						ground_items_menu_x+=mouse_delta_x;
						ground_items_menu_y+=mouse_delta_y;
					}
				return 1;
			}

	if(items_menu_dragged || (view_my_items && mouse_x>items_menu_x && mouse_x<=items_menu_x+items_menu_x_len && mouse_y>items_menu_y-16 && mouse_y<=items_menu_y))
		if(!attrib_menu_dragged && !ground_items_menu_dragged && !manufacture_menu_dragged &&
		   !trade_menu_dragged && !sigil_menu_dragged && !options_menu_dragged && !dialogue_menu_dragged && !knowledge_menu_dragged && !encyclopedia_menu_dragged && !questlog_menu_dragged && !questlog_menu_dragged && !buddy_menu_dragged)
			{
				items_menu_dragged=1;
				if(left_click>1)
					{
						items_menu_x+=mouse_delta_x;
						items_menu_y+=mouse_delta_y;
					}
				return 1;
			}
	/*
	if(attrib_menu_dragged || (view_self_stats && mouse_x>attrib_menu_x && mouse_x<=attrib_menu_x+attrib_menu_x_len && mouse_y>attrib_menu_y-16 && mouse_y<=attrib_menu_y))
		if(!items_menu_dragged && !ground_items_menu_dragged && !manufacture_menu_dragged &&
		   !trade_menu_dragged && !sigil_menu_dragged && !options_menu_dragged && !dialogue_menu_dragged && !knowledge_menu_dragged && !encyclopedia_menu_dragged && !questlog_menu_dragged && !questlog_menu_dragged && !buddy_menu_dragged)

			{
				attrib_menu_dragged=1;
				if(left_click>1)
					{
						attrib_menu_x+=mouse_delta_x;
						attrib_menu_y+=mouse_delta_y;
						move_window(stats_win, 0, 0, attrib_menu_x, attrib_menu_y);
					}
				return 1;
			}
	
	if(dialogue_menu_dragged || (have_dialogue && mouse_x>dialogue_menu_x && mouse_x<=dialogue_menu_x+dialogue_menu_x_len && mouse_y>dialogue_menu_y-16 && mouse_y<=dialogue_menu_y))
		if(!attrib_menu_dragged && !items_menu_dragged && !ground_items_menu_dragged && !manufacture_menu_dragged &&
		   !trade_menu_dragged && !options_menu_dragged && !sigil_menu_dragged && !knowledge_menu_dragged && !encyclopedia_menu_dragged && !questlog_menu_dragged && !questlog_menu_dragged && !buddy_menu_dragged)
			{
				dialogue_menu_dragged=1;
				if(left_click>1)
					{
						dialogue_menu_x+=mouse_delta_x;
						dialogue_menu_y+=mouse_delta_y;
					}
				return 1;
			}

	if(knowledge_menu_dragged || (view_knowledge && mouse_x>knowledge_menu_x && mouse_x<=knowledge_menu_x+knowledge_menu_x_len && mouse_y>knowledge_menu_y-16 && mouse_y<=knowledge_menu_y))
		if(!attrib_menu_dragged && !items_menu_dragged && !ground_items_menu_dragged && !manufacture_menu_dragged &&
		   !trade_menu_dragged && !options_menu_dragged && !sigil_menu_dragged && !dialogue_menu_dragged && !encyclopedia_menu_dragged && !questlog_menu_dragged && !questlog_menu_dragged && !buddy_menu_dragged)
			{
				knowledge_menu_dragged=1;
				if(left_click>1)
					{
						knowledge_menu_x+=mouse_delta_x;
						knowledge_menu_y+=mouse_delta_y;
					}
				return 1;
			}

	if(encyclopedia_menu_dragged || (view_encyclopedia && mouse_x>encyclopedia_menu_x && mouse_x<=encyclopedia_menu_x+encyclopedia_menu_x_len && mouse_y>encyclopedia_menu_y-16 && mouse_y<=encyclopedia_menu_y))
		if(!attrib_menu_dragged && !items_menu_dragged && !ground_items_menu_dragged && !manufacture_menu_dragged &&
		   !trade_menu_dragged && !options_menu_dragged && !sigil_menu_dragged && !dialogue_menu_dragged && !knowledge_menu_dragged && !questlog_menu_dragged && !questlog_menu_dragged && !buddy_menu_dragged)
			{
				encyclopedia_menu_dragged=1;
				if(left_click>1)
					{
						encyclopedia_menu_x+=mouse_delta_x;
						encyclopedia_menu_y+=mouse_delta_y;
					}
				return 1;
			}

	if(questlog_menu_dragged || (view_questlog && mouse_x>questlog_menu_x && mouse_x<=questlog_menu_x+questlog_menu_x_len && mouse_y>questlog_menu_y-16 && mouse_y<=questlog_menu_y))
		if(!attrib_menu_dragged && !items_menu_dragged && !ground_items_menu_dragged && !manufacture_menu_dragged &&
        !trade_menu_dragged && !options_menu_dragged && !sigil_menu_dragged && !dialogue_menu_dragged && !knowledge_menu_dragged && !encyclopedia_menu_dragged && !buddy_menu_dragged)
		{
			questlog_menu_dragged=1;
            if(left_click>1)
               {
                  questlog_menu_x+=mouse_delta_x;
                  questlog_menu_y+=mouse_delta_y;
               }
			return 1;
         }

	if(buddy_menu_dragged || (view_buddy && mouse_x>buddy_menu_x && mouse_x<=buddy_menu_x+buddy_menu_x_len && mouse_y>buddy_menu_y-16 && mouse_y<=buddy_menu_y))
		if(!attrib_menu_dragged && !items_menu_dragged && !ground_items_menu_dragged && !manufacture_menu_dragged &&
        !trade_menu_dragged && !options_menu_dragged && !sigil_menu_dragged && !dialogue_menu_dragged && !knowledge_menu_dragged && !encyclopedia_menu_dragged && !questlog_menu_dragged)
		{
			buddy_menu_dragged=1;
            if(left_click>1)
               {
                  buddy_menu_x+=mouse_delta_x;
                  buddy_menu_y+=mouse_delta_y;
               }
			return 1;
         }
	*/

	return 0;
}

int check_scroll_bars()
{
	//for knowledge window only. don't modify other scroll bars yet.
	if(knowledge_scroll_dragged || (view_knowledge && mouse_x>knowledge_menu_x+knowledge_menu_x_len-20 && mouse_x<knowledge_menu_x+knowledge_menu_x_len && mouse_y>knowledge_menu_y+35+(120*knowledge_page_start)/(300-38) && mouse_y<knowledge_menu_y+55+(120*knowledge_page_start)/(300-38))) {
		knowledge_scroll_dragged=1;
		if(left_click>1)
			knowledge_page_start+=mouse_delta_y*2;
		if(knowledge_page_start<0)knowledge_page_start=0;
		if(knowledge_page_start>300-38)knowledge_page_start=300-38;
		return 1;
	}
	return 0;
}

void check_menus_out_of_screen()
{
	if(attrib_menu_y-16<0)attrib_menu_y=16;
	if(attrib_menu_y>window_height-32)attrib_menu_y=window_height-32;
	if(attrib_menu_x+attrib_menu_x_len<10)attrib_menu_x=0-attrib_menu_x_len+10;
	if(attrib_menu_x>window_width-10)attrib_menu_x=window_width-10;

	if(items_menu_y-16<0)items_menu_y=16;
	if(items_menu_y>window_height-32)items_menu_y=window_height-32;
	if(items_menu_x+items_menu_x_len<10)items_menu_x=0-items_menu_x_len+11;
	if(items_menu_x>window_width-10)items_menu_x=window_width-10;

	if(ground_items_menu_y-16<0)ground_items_menu_y=16;
	if(ground_items_menu_y>window_height-32)ground_items_menu_y=window_height-32;
	if(ground_items_menu_x+ground_items_menu_x_len<10)ground_items_menu_x=0-ground_items_menu_x_len+11;
	if(ground_items_menu_x>window_width-10)ground_items_menu_x=window_width-10;

	if(manufacture_menu_y-16<0)manufacture_menu_y=16;
	if(manufacture_menu_y>window_height-32)manufacture_menu_y=window_height-32;
	if(manufacture_menu_x+manufacture_menu_x_len<10)manufacture_menu_x=0-manufacture_menu_x_len+11;
	if(manufacture_menu_x>window_width-10)manufacture_menu_x=window_width-10;

	if(trade_menu_y-16<0)trade_menu_y=16;
	if(trade_menu_y>window_height-32)trade_menu_y=window_height-32;
	if(trade_menu_x+trade_menu_x_len<10)trade_menu_x=0-trade_menu_x_len+11;
	if(trade_menu_x>window_width-10)trade_menu_x=window_width-10;

	if(sigil_menu_y-16<0)sigil_menu_y=16;
	if(sigil_menu_y>window_height-32)sigil_menu_y=window_height-32;
	if(sigil_menu_x+sigil_menu_x_len<10)sigil_menu_x=0-sigil_menu_x_len+11;
	if(sigil_menu_x>window_width-10)sigil_menu_x=window_width-10;

	if(dialogue_menu_y-16<0)dialogue_menu_y=16;
	if(dialogue_menu_y>window_height-32)dialogue_menu_y=window_height-32;
	if(dialogue_menu_x+dialogue_menu_x_len<10)dialogue_menu_x=0-dialogue_menu_x_len+11;
	if(dialogue_menu_x>window_width-10)dialogue_menu_x=window_width-10;

	if(options_menu_y-16<0)options_menu_y=16;
	if(options_menu_y>window_height-32)options_menu_y=window_height-32;
	if(options_menu_x + options_menu_x_len<10)options_menu_x=0-(options_menu_x + options_menu_x_len-options_menu_x)+11;
	if(options_menu_x>window_width-10)options_menu_x=window_width-10;

	if(knowledge_menu_y-16<0)knowledge_menu_y=16;
	if(knowledge_menu_y>window_height-32)knowledge_menu_y=window_height-32;
	if(knowledge_menu_x+knowledge_menu_x_len<10)knowledge_menu_x=0-knowledge_menu_x_len+11;
	if(knowledge_menu_x>window_width-10)knowledge_menu_x=window_width-10;

	if(encyclopedia_menu_y-16<0)encyclopedia_menu_y=16;
	if(encyclopedia_menu_y>window_height-32)encyclopedia_menu_y=window_height-32;
	if(encyclopedia_menu_x+encyclopedia_menu_x_len<10)encyclopedia_menu_x=0-encyclopedia_menu_x_len+11;
	if(encyclopedia_menu_x>window_width-10)encyclopedia_menu_x=window_width-10;

	if(questlog_menu_y-16<0)questlog_menu_y=16;
	if(questlog_menu_y>window_height-32)questlog_menu_y=window_height-32;
	if(questlog_menu_x+questlog_menu_x_len<10)questlog_menu_x=0-questlog_menu_x_len+11;
	if(questlog_menu_x>window_width-10)questlog_menu_x=window_width-10;

	if(buddy_menu_y-16<0)buddy_menu_y=16;
	if(buddy_menu_y>window_height-32)buddy_menu_y=window_height-32;
	if(buddy_menu_x+buddy_menu_x_len<10)buddy_menu_x=0-buddy_menu_x_len+11;
	if(buddy_menu_x>window_width-10)buddy_menu_x=window_width-10;
}

void check_mouse_click()
{
	/*
    if(have_dialogue)
    	{
    		if(mouse_x>=dialogue_menu_x && mouse_x<=dialogue_menu_x+dialogue_menu_x_len
			   && mouse_y>=dialogue_menu_y && mouse_y<=dialogue_menu_y+dialogue_menu_y_len)
				{
					if (check_dialogue_response()) return;	// avoid click thrus
				}
		}
	*/

	if(view_sigils_menu && mouse_x>(sigil_menu_x+sigil_menu_x_len-20) && mouse_x<=(sigil_menu_x+sigil_menu_x_len)
	   && mouse_y>sigil_menu_y && mouse_y<=sigil_menu_y+20)
		{
			view_sigils_menu=0;
			return;
		}
	if(check_sigil_interface())return;
	//if(check_options_menu())return;
	if(check_trade_interface())return;

	/*
	if(view_manufacture_menu && mouse_x>(manufacture_menu_x+manufacture_menu_x_len-20) && mouse_x<=(manufacture_menu_x+manufacture_menu_x_len)
	   && mouse_y>manufacture_menu_y && mouse_y<=manufacture_menu_y+20)
		{
			view_manufacture_menu=0;
			return;
		}
	if(check_manufacture_interface())return;
	*/

	if(view_ground_items && mouse_x>(ground_items_menu_x+ground_items_menu_x_len-20) && mouse_x<=(ground_items_menu_x+ground_items_menu_x_len)
	   && mouse_y>ground_items_menu_y && mouse_y<=ground_items_menu_y+20)
		{
			unsigned char protocol_name;

			view_ground_items=0;
			protocol_name=S_CLOSE_BAG;
			my_tcp_send(my_socket,&protocol_name,1);
			return;
		}
	if(item_dragged == -1 && check_ground_items_interface())return;

	if(view_my_items && mouse_x>(items_menu_x+items_menu_x_len-20) && mouse_x<=(items_menu_x+items_menu_x_len)
	   && mouse_y>items_menu_y && mouse_y<=items_menu_y+20)
		{
			view_my_items=0;

			return;
		}
	if(check_items_interface())return;

	/*
	if(view_knowledge && mouse_x>(knowledge_menu_x+knowledge_menu_x_len-20) && mouse_x<=(knowledge_menu_x+knowledge_menu_x_len)
	   && mouse_y>knowledge_menu_y && mouse_y<=knowledge_menu_y+20)
		{
			view_knowledge=0;
			return;
		}
	if(check_knowledge_interface())return;

	if(view_encyclopedia && mouse_x>(encyclopedia_menu_x+encyclopedia_menu_x_len-20) && mouse_x<=(encyclopedia_menu_x+encyclopedia_menu_x_len)
	   && mouse_y>encyclopedia_menu_y && mouse_y<=encyclopedia_menu_y+20)
		{
			view_encyclopedia=0;
			return;
		}
	if(check_encyclopedia_interface())return;

	if(view_self_stats)
		{
			if(click_in_window(stats_win, mouse_x, mouse_y, 0)){
				// is it the close button?
				view_self_stats= get_show_window(stats_win);
				return;
			}
		}

	if(view_questlog && mouse_x>(questlog_menu_x+questlog_menu_x_len-20) && mouse_x<=(questlog_menu_x+questlog_menu_x_len)
	   && mouse_y>questlog_menu_y && mouse_y<=questlog_menu_y+20)
		{
			view_questlog=0;
			return;
		}
	if(check_questlog_interface())return;

	if(view_buddy && mouse_x>(buddy_menu_x+buddy_menu_x_len-20) && mouse_x<=(buddy_menu_x+buddy_menu_x_len)
	   && mouse_y>buddy_menu_y && mouse_y<=buddy_menu_y+20)
		{
			view_buddy=0;
			return;
		}
	if(check_buddy_interface())return;
	*/

	// check for a click on the HUD (between scene & windows)
	//if(check_hud_interface()) return;
	if(click_in_windows(mouse_x, mouse_y, 0) > 0)	return;	// temporarily here for testing

	//after we test for interface clicks
	// alternative drop method...
	if (item_dragged != -1){
		Uint8 str[10];
		int quantity = item_list[item_dragged].quantity;
		if(right_click){
			str[0]=USE_MAP_OBJECT;
			*((int *)(str+1))=object_under_mouse;
			*((int *)(str+5))=item_list[item_dragged].pos;
			my_tcp_send(my_socket, str, 9);
			item_dragged = -1;
			return;
		}
		if (quantity - item_quantity > 0)
			quantity = item_quantity;
		str[0] = DROP_ITEM;
		str[1] = item_list[item_dragged].pos;
		*((Uint16 *) (str + 2)) = quantity;
		my_tcp_send(my_socket, str, 4);
		if (item_list[item_dragged].quantity - quantity <= 0)
			item_dragged = -1;
		return;
	}
	//LOOK AT
	if((current_cursor==CURSOR_EYE && left_click) || (action_mode==action_look && right_click))
		{
			Uint8 str[10];

			if(object_under_mouse==-1)return;

			if(thing_under_the_mouse==UNDER_MOUSE_PLAYER || thing_under_the_mouse==UNDER_MOUSE_NPC || thing_under_the_mouse==UNDER_MOUSE_ANIMAL)
				{
					str[0]=GET_PLAYER_INFO;
					*((int *)(str+1))=object_under_mouse;
					my_tcp_send(my_socket,str,5);
					return;
				}
			if(thing_under_the_mouse==UNDER_MOUSE_3D_OBJ)
				{
					str[0]=LOOK_AT_MAP_OBJECT;
					*((int *)(str+1))=object_under_mouse;
					my_tcp_send(my_socket,str,5);
					return;
				}
		}

	//if we're following a path, stop now
	if (pf_follow_path) {
		pf_destroy_path();
	}

	//TRADE
	if((action_mode==action_trade && right_click) || (current_cursor==CURSOR_TRADE && left_click))
		{
			Uint8 str[10];

			if(object_under_mouse==-1)return;
			if(thing_under_the_mouse!=UNDER_MOUSE_PLAYER)return;
			str[0]=TRADE_WITH;
			*((int *)(str+1))=object_under_mouse;
			my_tcp_send(my_socket,str,5);
			return;
		}

	//ATTACK
	if((current_cursor==CURSOR_ATTACK && left_click) || (action_mode==action_attack && right_click))
		{
			Uint8 str[10];

			if(object_under_mouse==-1)return;
			if(thing_under_the_mouse==UNDER_MOUSE_PLAYER || thing_under_the_mouse==UNDER_MOUSE_NPC || thing_under_the_mouse==UNDER_MOUSE_ANIMAL)
				{
					str[0]=ATTACK_SOMEONE;
					*((int *)(str+1))=object_under_mouse;
					my_tcp_send(my_socket,str,5);
					return;
				}
		}

	//USE
	if(((current_cursor==CURSOR_TALK || current_cursor==CURSOR_ENTER || current_cursor==CURSOR_USE) && left_click) || (action_mode==action_use && right_click))
		{
			Uint8 str[10];

			if(object_under_mouse==-1)return;
			if(thing_under_the_mouse==UNDER_MOUSE_PLAYER || thing_under_the_mouse==UNDER_MOUSE_NPC || thing_under_the_mouse==UNDER_MOUSE_ANIMAL)
				{
					int i;
					str[0]=TOUCH_PLAYER;
					*((int *)(str+1))=object_under_mouse;
					my_tcp_send(my_socket,str,5);

					//clear the previous dialogue entries, so we won't have a left over from some other NPC
					for(i=0;i<20;i++)dialogue_responces[i].in_use=0;
					return;
				}
			action_mode=action_walk;
			str[0]=USE_MAP_OBJECT;
			*((int *)(str+1))=object_under_mouse;
			*((int *)(str+5))=-1;
			my_tcp_send(my_socket,str,9);
			return;
		}

	//OPEN BAG
	if(current_cursor==CURSOR_PICK && left_click)
		{
			if(object_under_mouse==-1)return;
			open_bag(object_under_mouse);
			return;
		}

	//HARVEST
	if(current_cursor==CURSOR_HARVEST && left_click)
		{
			Uint8 str[10];

			if(object_under_mouse==-1)return;
			str[0]=HARVEST;
			*((short *)(str+1))=object_under_mouse;
			my_tcp_send(my_socket,str,3);
			return;
		}

	//WALK
	if((action_mode==action_walk && right_click) || (current_cursor==CURSOR_WALK && left_click && (!you_sit || !sit_lock)))
		{
			Uint8 str[10];
			short x,y;

			get_world_x_y();
			x=scene_mouse_x/0.5f;
			y=scene_mouse_y/0.5f;
			//check to see if the coordinates are OUTSIDE the map
			if(y<0 || x<0 || x>=tile_map_size_x*6 || y>=tile_map_size_y*6)return;

			str[0]=MOVE_TO;
			*((short *)(str+1))=x;
			*((short *)(str+3))=y;

			my_tcp_send(my_socket,str,5);
			return;
		}

	left_click=2;
	right_click=2;
}

void Enter2DMode()
{
	glPushAttrib(GL_LIGHTING_BIT|GL_DEPTH_BUFFER_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	glViewport(0, 0, window_width, window_height);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0, (GLdouble)window_width, (GLdouble)window_height, 0.0, -250.0, 250.0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
}

void Leave2DMode()
{
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopAttrib();
	glViewport(0, hud_y, window_width-hud_x, window_height-hud_y);
	//glViewport(0, 0, window_width-hud_x, window_height-hud_y);	// Reset The Current Viewport
}

mode_flag video_modes[10];

void build_video_mode_array()
{
	int i;
	int flags;

	for(i=0;i<10;i++)
		{
			video_modes[i].selected=0;
			video_modes[i].supported=0;
		}
	video_modes[video_mode-1].selected=1;

	if(full_screen)flags=SDL_OPENGL|SDL_FULLSCREEN;
	else
		flags=SDL_OPENGL;

#ifdef WINDOWS
	if(bpp==16 || full_screen)
#else
		if(bpp==16)
#endif
			if(SDL_VideoModeOK(640, 480, 16, flags))video_modes[0].supported=1;

#ifdef WINDOWS
	if(bpp==32 || full_screen)
#else
		if(bpp==32)
#endif
			if(SDL_VideoModeOK(640, 480, 32, flags))video_modes[1].supported=1;

#ifdef WINDOWS
	if(bpp==16 || full_screen)
#else
		if(bpp==16)
#endif
			if(SDL_VideoModeOK(800, 600, 16, flags))video_modes[2].supported=1;

#ifdef WINDOWS
	if(bpp==32 || full_screen)
#else
		if(bpp==32)
#endif
			if(SDL_VideoModeOK(800, 600, 32, flags))video_modes[3].supported=1;

#ifdef WINDOWS
	if(bpp==16 || full_screen)
#else
		if(bpp==16)
#endif
			if(SDL_VideoModeOK(1024, 768, 16, flags))video_modes[4].supported=1;

#ifdef WINDOWS
	if(bpp==32 || full_screen)
#else
		if(bpp==32)
#endif
			if(SDL_VideoModeOK(1024, 768, 32, flags))video_modes[5].supported=1;

#ifdef WINDOWS
	if(bpp==16 || full_screen)
#else
		if(bpp==16)
#endif
			if(SDL_VideoModeOK(1152, 864, 16, flags))video_modes[6].supported=1;

#ifdef WINDOWS
	if(bpp==32 || full_screen)
#else
		if(bpp==32)
#endif
			if(SDL_VideoModeOK(1152, 864, 32, flags))video_modes[7].supported=1;

#ifdef WINDOWS
	if(bpp==16 || full_screen)
#else
		if(bpp==16)
#endif
			if(SDL_VideoModeOK(1280, 1024, 16, flags))video_modes[8].supported=1;

#ifdef WINDOWS
	if(bpp==32 || full_screen)
#else
		if(bpp==32)
#endif
			if(SDL_VideoModeOK(1280, 1024, 32, flags))video_modes[9].supported=1;
}

void draw_console_pic(int which_texture)
{
	get_and_set_texture_id(which_texture);
	glColor3f(1.0f,1.0f,1.0f);
	glBegin(GL_QUADS);
	//draw the texture

	glTexCoord2f(0.0f,1.0f);
	glVertex3i(0,0,0);

	glTexCoord2f(0.0f,0.0f);
	glVertex3i(0,window_height,0);

	glTexCoord2f(1.0f,0.0f);
	glVertex3i(window_width,window_height,0);

	glTexCoord2f(1.0f,1.0f);
	glVertex3i(window_width,0,0);

	glEnd();
}

void draw_2d_thing(float u_start,float v_start,float u_end,float v_end,int x_start,
				   int y_start,int x_end,int y_end)
{
	glTexCoord2f(u_start,v_end);
	glVertex3i(x_start,y_end,0);

	glTexCoord2f(u_start,v_start);
	glVertex3i(x_start,y_start,0);

	glTexCoord2f(u_end,v_start);
	glVertex3i(x_end,y_start,0);

	glTexCoord2f(u_end,v_end);
	glVertex3i(x_end,y_end,0);
}

void draw_2d_thing_r(float u_start,float v_start,float u_end,float v_end,int x_start,
				   int y_start,int x_end,int y_end)
{
	glTexCoord2f(u_start,v_start);
	glVertex3i(x_start,y_end,0);

	glTexCoord2f(u_end,v_start);
	glVertex3i(x_start,y_start,0);

	glTexCoord2f(u_end,v_end);
	glVertex3i(x_end,y_start,0);

	glTexCoord2f(u_start,v_end);
	glVertex3i(x_end,y_end,0);
}


void init_opening_interface()
{
	check_gl_errors();
	login_screen_menus=load_texture_cache("./textures/login_menu.bmp",0);
	check_gl_errors();
	login_text=load_texture_cache("./textures/login_back.bmp",255);
	check_gl_errors();
}

void draw_login_screen()
{
	float selected_bar_u_start=(float)0/256;
	float selected_bar_v_start=1.0f-(float)0/256;

	float selected_bar_u_end=(float)174/256;
	float selected_bar_v_end=1.0f-(float)28/256;

	float unselected_bar_u_start=(float)0/256;
	float unselected_bar_v_start=1.0f-(float)40/256;

	float unselected_bar_u_end=(float)170/256;
	float unselected_bar_v_end=1.0f-(float)63/256;
	/////////////////////////
	float log_in_unselected_start_u=(float)0/256;
	float log_in_unselected_start_v=1.0f-(float)80/256;

	float log_in_unselected_end_u=(float)87/256;
	float log_in_unselected_end_v=1.0f-(float)115/256;

	float log_in_selected_start_u=(float)0/256;
	float log_in_selected_start_v=1.0f-(float)120/256;

	float log_in_selected_end_u=(float)87/256;
	float log_in_selected_end_v=1.0f-(float)155/256;
	/////////////////////////
	float new_char_unselected_start_u=(float)100/256;
	float new_char_unselected_start_v=1.0f-(float)80/256;

	float new_char_unselected_end_u=(float)238/256;
	float new_char_unselected_end_v=1.0f-(float)115/256;

	float new_char_selected_start_u=(float)100/256;
	float new_char_selected_start_v=1.0f-(float)120/256;

	float new_char_selected_end_u=(float)238/256;
	float new_char_selected_end_v=1.0f-(float)155/256;

	/////////////////////////////////////////////
	/////////////////////////////////////////////
	int half_screen_x=window_width/2;
	int half_screen_y=window_height/2;
	int username_text_x=half_screen_x-125;
	int username_text_y=half_screen_y-130;
	int password_text_x=half_screen_x-125;
	int password_text_y=half_screen_y-100;
	int username_bar_x=username_text_x+100;
	int username_bar_y=username_text_y-7;
	int username_bar_x_len=174;
	int username_bar_y_len=28;
	int password_bar_x=password_text_x+100;
	int password_bar_y=password_text_y-7;
	int password_bar_x_len=174;
	int password_bar_y_len=28;
	int log_in_x=half_screen_x-125;
	int log_in_y=half_screen_y-50;
	int log_in_x_len=87;
	int log_in_y_len=35;
	int new_char_x=half_screen_x+50;
	int new_char_y=half_screen_y-50;
	int new_char_x_len=138;
	int new_char_y_len=35;

	char log_in_button_selected=0;
	char new_char_button_selected=0;

	draw_console_pic(login_text);

	//check to see if the log in button is active, or not
	if(mouse_x>=log_in_x && mouse_x<=log_in_x+log_in_x_len && mouse_y>=log_in_y &&
	   mouse_y<=log_in_y+log_in_y_len && username_str[0] && password_str[0])
		log_in_button_selected=1;
	else
		log_in_button_selected=0;


	//check to see if the new char button is active, or not
	if(mouse_x>=new_char_x && mouse_x<=new_char_x+new_char_x_len && mouse_y>=new_char_y &&
	   mouse_y<=new_char_y+new_char_y_len)
		new_char_button_selected=1;
	else
		new_char_button_selected=0;

	//check to see if we clicked on the username/password box
	if(mouse_x>=username_bar_x && mouse_x<=username_bar_x+username_bar_x_len
	   && mouse_y>=username_bar_y && mouse_y<=username_bar_y+username_bar_y_len && left_click==1)
		{
			username_box_selected=1;
			password_box_selected=0;
		}

	if(mouse_x>=password_bar_x && mouse_x<=password_bar_x+password_bar_x_len
	   && mouse_y>=password_bar_y && mouse_y<=password_bar_y+password_bar_y_len && left_click==1)
		{
			username_box_selected=0;
			password_box_selected=1;
		}


	//check to see if we clicked on the ACTIVE Log In button
	if(log_in_button_selected && left_click==1)
		{
			send_login_info();
			left_click=2;//don't relogin 100 times like a moron
		}

	//check to see if we clicked on the ACTIVE New Char button
	if(new_char_button_selected && left_click==1)interface_mode=interface_new_char;

	//ok, start drawing the interface...
	draw_string(username_text_x,username_text_y,"Username: ",1);
	draw_string(password_text_x,password_text_y,"Password: ",1);
	//start drawing the actual interface pieces
	get_and_set_texture_id(login_screen_menus);
	glColor3f(1.0f,1.0f,1.0f);
	glBegin(GL_QUADS);

	//username box
	if(username_box_selected)
		draw_2d_thing(selected_bar_u_start,selected_bar_v_start,
					  selected_bar_u_end,selected_bar_v_end,username_bar_x,
					  username_bar_y,username_bar_x+username_bar_x_len,username_bar_y+username_bar_y_len);
	else
		draw_2d_thing(unselected_bar_u_start,unselected_bar_v_start,
					  unselected_bar_u_end,unselected_bar_v_end,username_bar_x,
					  username_bar_y,username_bar_x+username_bar_x_len,username_bar_y+username_bar_y_len);

	//password box
	if(password_box_selected)
		draw_2d_thing(selected_bar_u_start,selected_bar_v_start,
					  selected_bar_u_end,selected_bar_v_end,password_bar_x,
					  password_bar_y,password_bar_x+password_bar_x_len,password_bar_y+password_bar_y_len);
	else
		draw_2d_thing(unselected_bar_u_start,unselected_bar_v_start,
					  unselected_bar_u_end,unselected_bar_v_end,password_bar_x,
					  password_bar_y,password_bar_x+password_bar_x_len,password_bar_y+password_bar_y_len);

	//log in button
	if(log_in_button_selected)
		draw_2d_thing(log_in_selected_start_u,log_in_selected_start_v,
					  log_in_selected_end_u,log_in_selected_end_v,log_in_x,
					  log_in_y,log_in_x+log_in_x_len,log_in_y+log_in_y_len);
	else
		draw_2d_thing(log_in_unselected_start_u,log_in_unselected_start_v,
					  log_in_unselected_end_u,log_in_unselected_end_v,log_in_x,
					  log_in_y,log_in_x+log_in_x_len,log_in_y+log_in_y_len);

	//new char button
	if(new_char_button_selected)
		draw_2d_thing(new_char_selected_start_u,new_char_selected_start_v,
					  new_char_selected_end_u,new_char_selected_end_v,new_char_x,
					  new_char_y,new_char_x+new_char_x_len,new_char_y+new_char_y_len);
	else
		draw_2d_thing(new_char_unselected_start_u,new_char_unselected_start_v,
					  new_char_unselected_end_u,new_char_unselected_end_v,new_char_x,
					  new_char_y,new_char_x+new_char_x_len,new_char_y+new_char_y_len);
	glEnd();

	glColor3f(0.0f,0.9f,1.0f);
	draw_string(username_bar_x+4,username_text_y,username_str,1);
	draw_string(password_bar_x+4,password_text_y,display_password_str,1);
	glColor3f(1.0f,1.0f,1.0f);

	glColor3f(1.0f,0.0f,0.0f);
	//print the current error, if any
	draw_string(0,log_in_y+40,log_in_error_str,5);
}

void add_char_to_username(unsigned char ch)
{
	if(((ch>=48 && ch<=57) || (ch>=65 && ch<=90) || (ch>=97 && ch<=122) || (ch=='_')) && username_text_lenght<15)
		{
			username_str[username_text_lenght]=ch;
			username_str[username_text_lenght+1]=0;
			username_text_lenght++;
		}
	if(ch==SDLK_BACKSPACE && username_text_lenght>0)
		{
			username_text_lenght--;
			username_str[username_text_lenght]=0;
		}
	if(ch==SDLK_TAB)
		{
			username_box_selected=0;
			password_box_selected=1;
		}
}

void add_char_to_password(unsigned char ch)
{
	if((ch>=32 && ch<=126) && password_text_lenght<15)
		{
			password_str[password_text_lenght]=ch;
			display_password_str[password_text_lenght]='*';
			password_str[password_text_lenght+1]=0;
			display_password_str[password_text_lenght+1]=0;
			password_text_lenght++;
		}
	if(ch==SDLK_BACKSPACE && password_text_lenght>0)
		{
			password_text_lenght--;
			display_password_str[password_text_lenght]=0;
			password_str[password_text_lenght]=0;
		}

	if(ch==SDLK_TAB)
		{
			username_box_selected=1;
			password_box_selected=0;
		}
}

float lit_gem_u_start=(float)224/256;
float lit_gem_v_start=1.0f-(float)112/256;
float lit_gem_u_end=(float)255/256;
float lit_gem_v_end=1.0f-(float)128/256;

float broken_gem_u_start=(float)192/256;
float broken_gem_v_start=1.0f-(float)112/256;
float broken_gem_u_end=(float)223/256;
float broken_gem_v_end=1.0f-(float)128/256;

float unlit_gem_u_start=(float)224/256;
float unlit_gem_v_start=1.0f-(float)96/256;
float unlit_gem_u_end=(float)255/256;
float unlit_gem_v_end=1.0f-(float)111/256;

int display_options_handler(windows_info *win);
int click_options_handler(window_info *win, int mx, int my, Uint32 flags);
void display_options_menu()
{
	if(options_win <= 0){
		options_win= create_window("Options", 0, 0, options_menu_x, options_menu_y, options_menu_x_len, options_menu_y_len, ELW_WIN_DEFAULT);

		set_window_color(options_win, ELW_COLOR_BORDER, 0.0f, 1.0f, 0.0f, 0.0f);
		set_window_handler(options_win, ELW_HANDLER_DISPLAY, &display_options_handler );
		set_window_handler(options_win, ELW_HANDLER_CLICK, &click_options_handler );
	} else {
		show_window(options_win);
		select_window(options_win);
	}
	display_window(options_win);
}

int display_options_handler(windows_info *win)
{
	get_and_set_texture_id(icons_text);
	glBegin(GL_QUADS);
	if(!have_stencil)
		draw_2d_thing(broken_gem_u_start, broken_gem_v_start, broken_gem_u_end, broken_gem_v_end,
					  8, 35, 38, 51);
	else if(shadows_on)
		draw_2d_thing(lit_gem_u_start, lit_gem_v_start, lit_gem_u_end, lit_gem_v_end,
					  8, 35, 38, 51);
	else
		draw_2d_thing(unlit_gem_u_start, unlit_gem_v_start, unlit_gem_u_end, unlit_gem_v_end,
					  8, 35, 38, 51);

	if(!have_multitexture)
		draw_2d_thing(broken_gem_u_start, broken_gem_v_start, broken_gem_u_end, broken_gem_v_end,
					  8, 55, 38, 71);
	else if(clouds_shadows)
		draw_2d_thing(lit_gem_u_start, lit_gem_v_start, lit_gem_u_end, lit_gem_v_end,
					  8, 55, 38, 71);
	else
		draw_2d_thing(unlit_gem_u_start, unlit_gem_v_start, unlit_gem_u_end, unlit_gem_v_end,
					  8, 55, 38, 71);
	if(show_reflection)
		draw_2d_thing(lit_gem_u_start, lit_gem_v_start, lit_gem_u_end, lit_gem_v_end,
					  8, 75, 38, 91);
	else
		draw_2d_thing(unlit_gem_u_start, unlit_gem_v_start, unlit_gem_u_end, unlit_gem_v_end,
					  8, 75, 38, 91);
	if(show_fps)
		draw_2d_thing(lit_gem_u_start, lit_gem_v_start, lit_gem_u_end, lit_gem_v_end,
					  8, 95, 38, 111);
	else
		draw_2d_thing(unlit_gem_u_start, unlit_gem_v_start, unlit_gem_u_end, unlit_gem_v_end,
					  8, 95, 38, 111);

	if(sit_lock)
		draw_2d_thing(lit_gem_u_start, lit_gem_v_start, lit_gem_u_end, lit_gem_v_end,
					  8, 115, 38, 131);
	else
		draw_2d_thing(unlit_gem_u_start, unlit_gem_v_start, unlit_gem_u_end, unlit_gem_v_end,
					  8, 115, 38, 131);

	if(caps_filter)
		draw_2d_thing(lit_gem_u_start, lit_gem_v_start, lit_gem_u_end, lit_gem_v_end,
					  8, 135, 38, 151);
	else
		draw_2d_thing(unlit_gem_u_start, unlit_gem_v_start, unlit_gem_u_end, unlit_gem_v_end,
					  8, 135, 38, 151);

	if(!have_sound)
		draw_2d_thing(broken_gem_u_start, broken_gem_v_start, broken_gem_u_end, broken_gem_v_end,
					  8, 155, 38, 171);
	else if(sound_on)
		draw_2d_thing(lit_gem_u_start, lit_gem_v_start, lit_gem_u_end, lit_gem_v_end,
					  8, 155, 38, 171);
	else
		draw_2d_thing(unlit_gem_u_start, unlit_gem_v_start, unlit_gem_u_end, unlit_gem_v_end,
					  8, 155, 38, 171);

	if(!have_music)
		draw_2d_thing(broken_gem_u_start, broken_gem_v_start, broken_gem_u_end, broken_gem_v_end,
					  8, 175, 38, 191);
	else if(music_on)
		draw_2d_thing(lit_gem_u_start, lit_gem_v_start, lit_gem_u_end, lit_gem_v_end,
					  8, 175, 38, 191);
	else
		draw_2d_thing(unlit_gem_u_start, unlit_gem_v_start, unlit_gem_u_end, unlit_gem_v_end,
					  8, 175, 38, 191);

	if(auto_camera)
		draw_2d_thing(lit_gem_u_start, lit_gem_v_start, lit_gem_u_end, lit_gem_v_end,
					  8, 195, 38, 211);
	else
		draw_2d_thing(unlit_gem_u_start, unlit_gem_v_start, unlit_gem_u_end, unlit_gem_v_end,
					  8, 195, 38, 211);

	//video modes
	if(full_screen)
		draw_2d_thing(lit_gem_u_start, lit_gem_v_start, lit_gem_u_end, lit_gem_v_end,
					  193, 35, 220, 51);
	else
		draw_2d_thing(unlit_gem_u_start, unlit_gem_v_start, unlit_gem_u_end, unlit_gem_v_end,
					  193, 35, 220, 51);

	if(video_modes[0].selected)
		draw_2d_thing(lit_gem_u_start, lit_gem_v_start, lit_gem_u_end, lit_gem_v_end,
					  193, 55, 220, 71);
	else if(video_modes[0].supported)
		draw_2d_thing(unlit_gem_u_start, unlit_gem_v_start, unlit_gem_u_end, unlit_gem_v_end,
					  193, 55, 220, 71);
	else
		draw_2d_thing(broken_gem_u_start, broken_gem_v_start, broken_gem_u_end, broken_gem_v_end,
					  193, 55, 220, 71);

	if(video_modes[1].selected)
		draw_2d_thing(lit_gem_u_start, lit_gem_v_start, lit_gem_u_end, lit_gem_v_end,
					  193, 75, 220, 91);
	else if(video_modes[1].supported)
		draw_2d_thing(unlit_gem_u_start, unlit_gem_v_start, unlit_gem_u_end, unlit_gem_v_end,
					  193, 75, 220, 91);
	else
		draw_2d_thing(broken_gem_u_start, broken_gem_v_start, broken_gem_u_end, broken_gem_v_end,
					  193, 75, 220, 91);

	if(video_modes[2].selected)
		draw_2d_thing(lit_gem_u_start, lit_gem_v_start, lit_gem_u_end, lit_gem_v_end,
					  193, 95, 220, 111);
	else if(video_modes[2].supported)
		draw_2d_thing(unlit_gem_u_start, unlit_gem_v_start, unlit_gem_u_end, unlit_gem_v_end,
					  193, 95, 220, 111);
	else
		draw_2d_thing(broken_gem_u_start, broken_gem_v_start, broken_gem_u_end, broken_gem_v_end,
					  193, 95, 220, 111);

	if(video_modes[3].selected)
		draw_2d_thing(lit_gem_u_start, lit_gem_v_start, lit_gem_u_end, lit_gem_v_end,
					  193, 115, 220, 131);
	else if(video_modes[3].supported)
		draw_2d_thing(unlit_gem_u_start, unlit_gem_v_start, unlit_gem_u_end, unlit_gem_v_end,
					  193, 115, 220, 131);
	else
		draw_2d_thing(broken_gem_u_start, broken_gem_v_start, broken_gem_u_end, broken_gem_v_end,
					  193, 115, 220, 131);

	if(video_modes[4].selected)
		draw_2d_thing(lit_gem_u_start, lit_gem_v_start, lit_gem_u_end, lit_gem_v_end,
					  193, 135, 220, 151);
	else if(video_modes[4].supported)
		draw_2d_thing(unlit_gem_u_start, unlit_gem_v_start, unlit_gem_u_end, unlit_gem_v_end,
					  193, 135, 220, 151);
	else
		draw_2d_thing(broken_gem_u_start, broken_gem_v_start, broken_gem_u_end, broken_gem_v_end,
					  193, 135, 220, 151);

	if(video_modes[5].selected)
		draw_2d_thing(lit_gem_u_start, lit_gem_v_start, lit_gem_u_end, lit_gem_v_end,
					  193, 155, 220, 171);
	else if(video_modes[5].supported)
		draw_2d_thing(unlit_gem_u_start, unlit_gem_v_start, unlit_gem_u_end, unlit_gem_v_end,
					  193, 155, 220, 171);
	else
		draw_2d_thing(broken_gem_u_start, broken_gem_v_start, broken_gem_u_end, broken_gem_v_end,
					  193, 155, 220, 171);

	if(video_modes[6].selected)
		draw_2d_thing(lit_gem_u_start, lit_gem_v_start, lit_gem_u_end, lit_gem_v_end,
					  193, 175, 220, 191);
	else if(video_modes[6].supported)
		draw_2d_thing(unlit_gem_u_start, unlit_gem_v_start, unlit_gem_u_end, unlit_gem_v_end,
					  193, 175, 220, 191);
	else
		draw_2d_thing(broken_gem_u_start, broken_gem_v_start, broken_gem_u_end, broken_gem_v_end,
					  193, 175, 220, 191);

	if(video_modes[7].selected)
		draw_2d_thing(lit_gem_u_start, lit_gem_v_start, lit_gem_u_end, lit_gem_v_end,
					  193, 195, 220, 211);
	else if(video_modes[7].supported)
		draw_2d_thing(unlit_gem_u_start, unlit_gem_v_start, unlit_gem_u_end, unlit_gem_v_end,
					  193, 195, 220, 211);
	else
		draw_2d_thing(broken_gem_u_start, broken_gem_v_start, broken_gem_u_end, broken_gem_v_end,
					  193, 195, 220, 211);

	if(video_modes[8].selected)
		draw_2d_thing(lit_gem_u_start, lit_gem_v_start, lit_gem_u_end, lit_gem_v_end,
					  193, 215, 220, 231);
	else if(video_modes[8].supported)
		draw_2d_thing(unlit_gem_u_start, unlit_gem_v_start, unlit_gem_u_end, unlit_gem_v_end,
					  193, 215, 220, 231);
	else
		draw_2d_thing(broken_gem_u_start, broken_gem_v_start, broken_gem_u_end, broken_gem_v_end,
					  193, 215, 220, 231);

	if(video_modes[9].selected)
		draw_2d_thing(lit_gem_u_start, lit_gem_v_start, lit_gem_u_end, lit_gem_v_end,
					  193, 235, 220, 251);
	else if(video_modes[9].supported)
		draw_2d_thing(unlit_gem_u_start, unlit_gem_v_start, unlit_gem_u_end, unlit_gem_v_end,
					  193, 235, 220, 251);
	else
		draw_2d_thing(broken_gem_u_start, broken_gem_v_start, broken_gem_u_end, broken_gem_v_end,
					  193, 235, 220, 251);

	glEnd();
	draw_string(55,10,"Options",1);
	draw_string(45,35,"Shadows",1);
	draw_string(45,55,"Clouds",1);
	draw_string(45,75,"Reflections",1);
	draw_string(45,95,"Show FPS",1);
	draw_string(45,115,"Sit Lock",1);
	draw_string(45,135,"Filter CAPS",1);
	draw_string(45,155,"Sound",1);
	draw_string(45,175,"Music",1);
	draw_string(45,195,"Auto Camera",1);

	draw_string(225,10,"Video Modes",1);
	draw_string(225,35,"Full Screen",1);
	draw_string(225,55,"640x480x16",1);
	draw_string(225,75,"640x480x32",1);
	draw_string(225,95,"800x600x16",1);
	draw_string(225,115,"800x600x32",1);
	draw_string(225,135,"1024x768x16",1);
	draw_string(225,155,"1024x768x32",1);
	draw_string(225,175,"1152x864x16",1);
	draw_string(225,195,"1152x864x32",1);
	draw_string(225,215,"1280x1024x16",1);
	draw_string(225,235,"1280x1024x32",1);

	return 1;
}

int click_options_handler(window_info *win, int mx, int my, Uint32 flags)
{
	// in the first column?
	if(mx>8 && mx<38)
		{
			if(my>35 && my<51)
				shadows_on=!shadows_on;
			else if(my>55 && my<71)
				clouds_shadows=!clouds_shadows;
			else if(my>75 && my<91)
				show_reflection=!show_reflection;
			else if(my>95 && my<111)
				show_fps=!show_fps;
			else if(my>115 && my<131)
				sit_lock=!sit_lock;
			else if(my>135 && my<151)
				caps_filter=!caps_filter;
			else if(my>155 && my<171)
				if(sound_on)turn_sound_off();
				else turn_sound_on();
			else if(my>175 && my<191)
				if(music_on)turn_music_off();
				else turn_music_on();
			else if(my>195 && my<211)
				auto_camera=!auto_camera;
		}
	else if(mx>193 && mx<220)
		{
			if(my>35 && my<51)
				toggle_full_screen();
			else if(my>55 && my<71)
				{
					if(video_modes[0].supported && !video_modes[0].selected)
						set_new_video_mode(full_screen,1);
				}
			else if(my>75 && my<91)
				{
					if(video_modes[1].supported && !video_modes[1].selected)
						set_new_video_mode(full_screen,2);
				}
			else if(my>95 && my<111)
				{
					if(video_modes[2].supported && !video_modes[2].selected)
						set_new_video_mode(full_screen,3);
				}
			else if(my>115 && my<131)
				{
					if(video_modes[3].supported && !video_modes[3].selected)
						set_new_video_mode(full_screen,4);
				}
			else if(my>135 && my<151)
				{
					if(video_modes[4].supported && !video_modes[4].selected)
						set_new_video_mode(full_screen,5);
				}
			else if(my>155 && my<171)
				{
					if(video_modes[5].supported && !video_modes[5].selected)
						set_new_video_mode(full_screen,6);
				}
			else if(my>175 && my<191)
				{
					if(video_modes[6].supported && !video_modes[6].selected)
						set_new_video_mode(full_screen,7);
				}
			else if(my>195 && my<211)
				{
					if(video_modes[7].supported && !video_modes[7].selected)
						set_new_video_mode(full_screen,8);
				}
			else if(my>215 && my<231)
				{
					if(video_modes[8].supported && !video_modes[8].selected)
						set_new_video_mode(full_screen,9);
				}
			else if(my>235 && my<251)
				{
					if(video_modes[9].supported && !video_modes[9].selected)
						set_new_video_mode(full_screen,10);
				}
		}
	return 1;
}

void draw_ingame_interface()
{
	check_menus_out_of_screen();

	// hack for X & dragging
	// TODO: finish the conversion process enough to remove these lines
	if(options_win > 0)	options_menu= get_show_window(options_win);
	if(stats_win > 0)	view_self_stats= get_show_window(stats_win);
	if(quest_win > 0)	view_questlog= get_show_window(quest_win);
	if(buddy_win > 0)	view_buddy= get_show_window(buddy_win);
	if(encyclopedia_win > 0)	view_encyclopedia= get_show_window(encyclopedia_win);
	if(knowledge_win > 0)	view_knowledge= get_show_window(knowledge_win);
	if(manufacture_win > 0)	view_manufacture_menu= get_show_window(manufacture_win);

	glColor3f(1.0f, 1.0f, 1.0f);
	draw_hud_frame();
	display_windows(1);	// Display all the windows handled by the window manager
	//draw_hud_interface();
    display_spells_we_have();

	/*
    if(have_dialogue)
    	{
    		display_dialogue();
    		if(mouse_x>=dialogue_menu_x && mouse_x<=dialogue_menu_x+dialogue_menu_x_len
			   && mouse_y>=dialogue_menu_y && mouse_y<=dialogue_menu_y+dialogue_menu_y_len)
				highlight_dialogue_response();
		}
	*/
    //if(view_self_stats)display_stats(your_info);
	//if(view_knowledge){knowledge_mouse_over();display_knowledge();}
	//if(view_encyclopedia){encyclopedia_mouse_over();display_encyclopedia();}
	//if(view_questlog)display_questlog();
	//if(view_buddy)display_buddy();
    if(view_my_items)display_items_menu();
    if(view_ground_items)draw_pick_up_menu();
    if(item_dragged!=-1)drag_item();
    //if(view_manufacture_menu)display_manufacture_menu();
    if(view_trade_menu)display_trade_menu();
    //if(options_menu)draw_options_menu();
    if(view_sigils_menu)
    	{
			check_sigil_mouseover();
    		display_sigils_menu();
		}
	
}

int map_text;

void switch_to_game_map()
{
	int len;
	char map_map_file_name[256];

	//try to see if we can find a valid map
	my_strcp(map_map_file_name,map_file_name);
	len=strlen(map_map_file_name);
	map_map_file_name[len-3]='b';
	map_map_file_name[len-2]='m';
	map_map_file_name[len-1]='p';
	map_text=load_bmp8_fixed_alpha(map_map_file_name,128);
	if(!map_text)//this map has no map (sounds so stupid)
		{
			log_to_console(c_yellow2,"There is no map for this place.");
			return;
		}
	interface_mode=interface_map;
}

void switch_from_game_map()
{
	glDeleteTextures(1,&map_text);
	interface_mode=interface_game;
}

void draw_game_map()
{
	int screen_x=0;
	int screen_y=0;
	int i;

   	glDisable(GL_DEPTH_TEST);
   	glDisable(GL_LIGHTING);

   	glViewport(0, 0 + hud_y, window_width-hud_x, window_height-hud_y);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(300, (GLdouble)0, (GLdouble)0, 200, -250.0, 250.0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	bind_texture_id(map_text);
	glColor3f(1.0f,1.0f,1.0f);
	glBegin(GL_QUADS);
	//draw the texture

	glTexCoord2f(1.0f,0.0f);
	glVertex3i(50,0,0);

	glTexCoord2f(1.0f,1.0f);
	glVertex3i(50,200,0);

	glTexCoord2f(0.0f,1.0f);
	glVertex3i(250,200,0);

	glTexCoord2f(0.0f,0.0f);
	glVertex3i(250,0,0);

	glEnd();

	//if we're following a path, draw the destination on the map
	if (pf_follow_path) {
		int x = pf_dst_tile->x;
		int y = pf_dst_tile->y;

		screen_x=300-(50+200*x/(tile_map_size_x*6));
		screen_y=0+200*y/(tile_map_size_y*6);

		glColor3f(1.0f,0.0f,0.0f);
		glDisable(GL_TEXTURE_2D);
		glBegin(GL_LINES);
		glVertex2i(screen_x-3,screen_y-3);
		glVertex2i(screen_x+2,screen_y+2);

		glVertex2i(screen_x+2,screen_y-3);
		glVertex2i(screen_x-3,screen_y+2);
		glEnd();
	}

	//ok, now let's draw our possition...
	for(i=0;i<max_actors;i++)
		{
			if(actors_list[i])
				if(actors_list[i]->actor_id==yourself)
					{
						int x=actors_list[i]->x_tile_pos;
						int y=actors_list[i]->y_tile_pos;
						screen_x=300-(50+200*x/(tile_map_size_x*6));
						screen_y=0+200*y/(tile_map_size_y*6);
						break;
					}
		}

	glColor3f(0.0f,0.0f,1.0f);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_LINES);
	glVertex2i(screen_x-3,screen_y-3);
	glVertex2i(screen_x+2,screen_y+2);

	glVertex2i(screen_x+2,screen_y-3);
	glVertex2i(screen_x-3,screen_y+2);
	glEnd();

	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0f,1.0f,1.0f);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
}

void draw_menu_title_bar(int x, int y, int x_len)
{
	float u_first_start=(float)31/256;
	float u_first_end=0;
	float v_first_start=1.0f-(float)160/256;
	float v_first_end=1.0f-(float)175/256;

	float u_middle_start=(float)32/256;
	float u_middle_end=(float)63/256;
	float v_middle_start=1.0f-(float)160/256;
	float v_middle_end=1.0f-(float)175/256;

	float u_last_start=0;
	float u_last_end=(float)31/256;
	float v_last_start=1.0f-(float)160/256;
	float v_last_end=1.0f-(float)175/256;

	int segments_no;
	int i;

	glColor3f(1.0f,1.0f,1.0f);
	//ok, now draw that shit...
	segments_no=x_len/32;

	get_and_set_texture_id(icons_text);
	glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER,0.03f);
	glBegin(GL_QUADS);

	glTexCoord2f(u_first_end,v_first_start);
	glVertex3i(x,y,0);
	glTexCoord2f(u_first_end,v_first_end);
	glVertex3i(x,y+16,0);
	glTexCoord2f(u_first_start,v_first_end);
	glVertex3i(x+32,y+16,0);
	glTexCoord2f(u_first_start,v_first_start);
	glVertex3i(x+32,y,0);

	for(i=1;i<segments_no-1;i++)
		{
			glTexCoord2f(u_middle_end,v_middle_start);
			glVertex3i(x+i*32,y,0);
			glTexCoord2f(u_middle_end,v_middle_end);
			glVertex3i(x+i*32,y+16,0);
			glTexCoord2f(u_middle_start,v_middle_end);
			glVertex3i(x+i*32+32,y+16,0);
			glTexCoord2f(u_middle_start,v_middle_start);
			glVertex3i(x+i*32+32,y,0);
		}

	glTexCoord2f(u_last_end,v_last_start);
	glVertex3i(x+i*32,y,0);
	glTexCoord2f(u_last_end,v_last_end);
	glVertex3i(x+i*32,y+16,0);
	glTexCoord2f(u_last_start,v_last_end);
	glVertex3i(x+i*32+32,y+16,0);
	glTexCoord2f(u_last_start,v_last_start);
	glVertex3i(x+i*32+32,y,0);

	glEnd();
	glDisable(GL_ALPHA_TEST);
}

