#include "global.h"

#ifdef WINDOW_CHAT

int root_win = -1;

// This is the main part of the old check_cursor_change ()
int mouseover_root_handler (window_info *win, int mx, int my)
{
	if(object_under_mouse == -1)
	{
		elwin_mouse = CURSOR_WALK;
	}

	else if (thing_under_the_mouse==UNDER_MOUSE_3D_OBJ && objects_list[object_under_mouse])
	{
		int i;
		
		if(action_mode==action_look)
		{
			elwin_mouse = CURSOR_EYE;
		}

		//see if it is a bag. (bag1.e3d)
		else if(get_string_occurance("bag1.e3d",objects_list[object_under_mouse]->file_name, 80,0)!=-1)
		{
			elwin_mouse = CURSOR_PICK;
		}

		else if(action_mode==action_use)
		{
			elwin_mouse = CURSOR_USE;
		}
		
		else
		{
			//see if the object is a harvestable resource.
			for(i=0;i<100;i++)
			{
				if(!harvestable_objects[i].name[0])break;//end of the objects
				if(get_string_occurance(harvestable_objects[i].name,objects_list[object_under_mouse]->file_name, 80,0)!=-1)
				{
					elwin_mouse = CURSOR_HARVEST;
					return 1;
				}
			}
			
			//see if the object is an entrable resource.
			for(i=0;i<100;i++)
			{
				if(!entrable_objects[i].name[0])break;//end of the objects
				if(get_string_occurance(entrable_objects[i].name,objects_list[object_under_mouse]->file_name, 80,0)!=-1)
				{
					elwin_mouse = CURSOR_ENTER;
					return 1;
				}
			}
		
			if(action_mode==action_use_witem)
			{
				elwin_mouse = CURSOR_USE_WITEM;
			}

			//hmm, no usefull object, so select walk....
			else 
			{
				elwin_mouse = CURSOR_WALK;
			}
		}
	}

	else if(thing_under_the_mouse==UNDER_MOUSE_NPC)
	{
		if(action_mode==action_look)
		{
			elwin_mouse = CURSOR_EYE;
		}
		
		else 
		{
			elwin_mouse = CURSOR_TALK;
		}
	}

	else if(thing_under_the_mouse==UNDER_MOUSE_PLAYER)
	{
		if(action_mode==action_use)
		{
			elwin_mouse = CURSOR_USE;
		}      

		else if(action_mode==action_look)
		{
			elwin_mouse = CURSOR_EYE;
		}

		else if(action_mode==action_trade)
		{
			elwin_mouse = CURSOR_TRADE;
		}

		else if(alt_on || action_mode==action_attack)
		{
			elwin_mouse = CURSOR_ATTACK;
		}

		else
		{
			elwin_mouse = CURSOR_EYE;
		}
	}

	else if(thing_under_the_mouse==UNDER_MOUSE_ANIMAL)
	{
		if(action_mode==action_use)
		{
			elwin_mouse = CURSOR_USE;
		}      
		
		else if(action_mode==action_look)
		{
			elwin_mouse = CURSOR_EYE;
		}

		else if(shift_on)
		{
			elwin_mouse = CURSOR_EYE;
		}

		else if(alt_on || action_mode==action_attack || (actor_under_mouse && !actor_under_mouse->dead))
		{
			elwin_mouse = CURSOR_ATTACK;
		}
	}

	// when all fails - walk
	else
	{
		elwin_mouse = CURSOR_WALK;
	}

	return 1;
}

// this is the main part of the old check_mouse_click ()
int click_root_handler (window_info *win, int mx, int my, Uint32 flags)
{
	int flag_ctrl = flags & ELW_CTRL;
	int flag_right = flags & ELW_RIGHT_MOUSE;
	int force_walk = (flag_ctrl && flag_right);
	
	if (!force_walk)
	{
		if (flag_right) 
		{
			if (item_dragged != -1 || use_item != -1 || object_under_mouse == -1)
			{
				use_item = -1;
				item_dragged = -1;
				action_mode = action_walk;
				return 1;
			}
			switch (current_cursor) 
			{
				case CURSOR_EYE:
					if (thing_under_the_mouse == UNDER_MOUSE_PLAYER)
						action_mode = action_trade;
					else if (thing_under_the_mouse == UNDER_MOUSE_3D_OBJ)
						action_mode = action_use;
					else
						action_mode = action_walk;
					break;
				case CURSOR_HARVEST:
					action_mode = action_look;
					break;
				case CURSOR_TRADE:
					action_mode = action_attack;
					break;
				case CURSOR_USE_WITEM:
					if(use_item != -1)
						use_item = -1;
					else
						action_mode = action_walk;
					break;
				case CURSOR_ATTACK:
					if(thing_under_the_mouse == UNDER_MOUSE_ANIMAL)
						action_mode = action_look;
					else
						action_mode = action_walk;
					break;
				case CURSOR_ENTER:
				case CURSOR_PICK:
				case CURSOR_WALK:
					if(thing_under_the_mouse == UNDER_MOUSE_3D_OBJ)
						action_mode = action_look;
					else
						action_mode = action_walk;
					break;
				case CURSOR_USE:
				case CURSOR_TALK:
				case CURSOR_ARROW:
				default:
					action_mode = action_walk;
					break;
			}
			return 1;
		}
	}

	// after we test for interface clicks
	// alternative drop method...
	if (item_dragged != -1)
	{
		Uint8 str[10];
		int quantity = item_list[item_dragged].quantity;

		if (flag_right)
		{
			item_dragged = -1;
			return 1;
		}

		if (quantity - item_quantity > 0)
			quantity = item_quantity;
		str[0] = DROP_ITEM;
		str[1] = item_list[item_dragged].pos;
		*((Uint16 *) (str + 2)) = quantity;
		my_tcp_send(my_socket, str, 4);
		if (item_list[item_dragged].quantity - quantity <= 0)
			item_dragged = -1;
		return 1;
	}

	// if we're following a path, stop now
	if (pf_follow_path) 
	{
		pf_destroy_path();
	}

	if (force_walk)
	{
		Uint8 str[10];
		short x,y;
		
		//get_old_world_x_y ();
		get_world_x_y ();
		x = scene_mouse_x / 0.5f;
		y = scene_mouse_y / 0.5f;
		// check to see if the coordinates are OUTSIDE the map
		if (y < 0 || x < 0 || x >= tile_map_size_x*6 || y >= tile_map_size_y*6)
			return 1;
		
		str[0] = MOVE_TO;
		*((short *)(str+1)) = x;
		*((short *)(str+3)) = y;

		my_tcp_send(my_socket,str,5);
		return 1;
	}
	
	switch(current_cursor) 
	{
		case CURSOR_EYE:
		{
			Uint8 str[10];

			if (object_under_mouse == -1)
				return 1;

			if (thing_under_the_mouse == UNDER_MOUSE_PLAYER || thing_under_the_mouse == UNDER_MOUSE_NPC || thing_under_the_mouse == UNDER_MOUSE_ANIMAL)
			{
				str[0] = GET_PLAYER_INFO;
				*((int *)(str+1)) = object_under_mouse;
				my_tcp_send (my_socket, str, 5);
				return 1;
			}
			else if (thing_under_the_mouse == UNDER_MOUSE_3D_OBJ)
			{
				str[0] = LOOK_AT_MAP_OBJECT;
				*((int *)(str+1)) = object_under_mouse;
				my_tcp_send (my_socket, str, 5);
				return 1;
			}

			break;
		}

		case CURSOR_TRADE:
		{
			Uint8 str[10];

			if (object_under_mouse == -1)
				return 1;
			if (thing_under_the_mouse != UNDER_MOUSE_PLAYER)
				return 1;
			str[0] = TRADE_WITH;
			*((int *)(str+1)) = object_under_mouse;
			my_tcp_send (my_socket, str, 5);
			return 1;
			
			break;
		}

		case CURSOR_ATTACK:
		{
			Uint8 str[10];

			if (object_under_mouse == -1)
				return 1;
			if (you_sit && sit_lock && !flag_ctrl)
				return 1;
			if (thing_under_the_mouse == UNDER_MOUSE_PLAYER || thing_under_the_mouse == UNDER_MOUSE_NPC || thing_under_the_mouse == UNDER_MOUSE_ANIMAL)
			{
				str[0] = ATTACK_SOMEONE;
				*((int *)(str+1)) = object_under_mouse;
				my_tcp_send (my_socket, str, 5);
				return 1;
			}

			break;
		}

		case CURSOR_ENTER:
			if(you_sit && sit_lock && !flag_ctrl)
				return 1;
		case CURSOR_USE:
		case CURSOR_USE_WITEM:
		case CURSOR_TALK:
		{
			Uint8 str[10];

			if (object_under_mouse == -1)
				return 1;
			if (thing_under_the_mouse == UNDER_MOUSE_PLAYER || thing_under_the_mouse == UNDER_MOUSE_NPC || thing_under_the_mouse == UNDER_MOUSE_ANIMAL)
			{
				int i;
				str[0] = TOUCH_PLAYER;
				*((int *)(str+1)) = object_under_mouse;
				my_tcp_send (my_socket, str, 5);

				// clear the previous dialogue entries, so we won't have a left over from some other NPC
				for (i=0; i<20; i++)
					dialogue_responces[i].in_use = 0;
				return 1;
			}
			
			str[0] = USE_MAP_OBJECT;
			*((int *)(str+1)) = object_under_mouse;
			if (use_item != -1 && current_cursor == CURSOR_USE_WITEM)
			{
				*((int *)(str+5)) = item_list[use_item].pos;
				use_item = -1;
				action_mode = action_walk;
			} 
			else
			{
				*((int *)(str+5)) = -1;
			}

			my_tcp_send (my_socket, str, 9);
			return 1;

			break;
		}

		case CURSOR_PICK:
			if (object_under_mouse == -1)
				return 1;
			if (you_sit && sit_lock && !flag_ctrl)
				return 1;
			open_bag (object_under_mouse);
			return 1;
			break;

		case CURSOR_HARVEST:
		{
			Uint8 str[10];

			if (object_under_mouse == -1)
				return 1;
			str[0] = HARVEST;
			*((short *)(str+1)) = object_under_mouse;
			my_tcp_send (my_socket, str, 3);
			return 1;
			break;
		}

		case CURSOR_WALK:
		default:
		{
			Uint8 str[10];
			short x, y;
		
			if (you_sit && sit_lock && !flag_ctrl)
				return 1;
			get_world_x_y ();
			x = scene_mouse_x / 0.5f;
			y = scene_mouse_y / 0.5f;
			// check to see if the coordinates are OUTSIDE the map
			if (y < 0 || x < 0 || x >= tile_map_size_x*6 || y >= tile_map_size_y*6)
				return 1;
			
			str[0] = MOVE_TO;
			*((short *)(str+1)) = x;
			*((short *)(str+3)) = y;
	
			my_tcp_send (my_socket, str, 5);
			return 1;
		}
	}

	left_click = 2;
	right_click = 2;
	return 1;
}

void display_root ()
{
	if (root_win < 0)
	{
		root_win = create_window ("root", -1, -1, 0, 0, window_width, window_height, ELW_WIN_INVISIBLE|ELW_SHOW_LAST);
		
        	set_window_handler (root_win, ELW_HANDLER_CLICK, &click_root_handler);
        	set_window_handler (root_win, ELW_HANDLER_MOUSEOVER, &mouseover_root_handler);
		
		resize_root_window();
	}
}

#endif
