#include <stdlib.h>
#include "global.h"

PF_OPEN_LIST pf_open;
PF_TILE *pf_tile_map=NULL;
PF_TILE *pf_src_tile, *pf_dst_tile, *pf_cur_tile;
int pf_follow_path = 0;
int pf_actor_id;
SDL_TimerID pf_movement_timer = NULL;

#define PF_DIFF(a, b) ((a > b) ? a - b : b - a)
#define PF_HEUR(a, b) ((PF_DIFF(a->x, b->x) + PF_DIFF(a->y, b->y)) * 10)

PF_TILE *pf_get_tile(int x, int y)
{
	if (x >= tile_map_size_x*6 || y >= tile_map_size_y*6 || x < 0 || y < 0) {
		return NULL;
	}
	return &pf_tile_map[y*tile_map_size_x*6+x];
}

PF_TILE *pf_get_next_open_tile()
{
	PF_TILE *tmp, *ret = NULL;
	int i, j, done = 0;
	
	if (pf_open.count == 0) {
		return NULL;
	}
	
	ret = pf_open.tiles[1];
	
	ret->state = PF_STATE_CLOSED;
	
	pf_open.tiles[1] = pf_open.tiles[pf_open.count--];
	
	j = 1;
	
	while (!done) {
		i = j;
		
		if (2*i+1 <= pf_open.count) {
			if (pf_open.tiles[i]->f >= pf_open.tiles[2*i]->f) {
				j = 2*i;
			}
			if (pf_open.tiles[j]->f >= pf_open.tiles[2*i+1]->f) {
				j = 2*i+1;
			}
		} else if (2*i <= pf_open.count) {
			if (pf_open.tiles[i]->f >= pf_open.tiles[2*i]->f) {
				j = 2*i;
			}
		}
		
		if (i != j) {
			tmp = pf_open.tiles[i];
			pf_open.tiles[i] = pf_open.tiles[j];
			pf_open.tiles[j] = tmp;
		} else {
			done = 1;
		}
	}

	return ret;
}

void pf_add_tile_to_open_list(PF_TILE *current, PF_TILE *neighbour)
{
	PF_TILE *tmp;
	
	if (!neighbour || neighbour->z == 0 || (current && PF_DIFF(current->z, neighbour->z) > 2)) {
		return;
	}
	
	if (current) {
		int f, g, h;
		int diagonal = (neighbour->x != current->x && neighbour->y != current->y);
		
		g = current->g + (diagonal ? 14 : 10);
		h = PF_HEUR(neighbour, pf_dst_tile);
		f = g + h;
		
		if (neighbour->state != PF_STATE_NONE && f >= neighbour->f) {
			return;
		}
		
		neighbour->f = f;
		neighbour->g = g;
		neighbour->parent = current;
	} else {
		neighbour->f = PF_HEUR(pf_src_tile, pf_dst_tile);
		neighbour->g = 0;
		neighbour->parent = NULL;
	}
		
	if (neighbour->state != PF_STATE_OPEN) {
		neighbour->open_pos = ++pf_open.count;
		pf_open.tiles[neighbour->open_pos] = neighbour;
	}
	
	while (neighbour->open_pos > 1) {
		if (pf_open.tiles[neighbour->open_pos]->f <= pf_open.tiles[neighbour->open_pos/2]->f) {
			tmp = pf_open.tiles[neighbour->open_pos/2];
			pf_open.tiles[neighbour->open_pos/2] = pf_open.tiles[neighbour->open_pos];
			pf_open.tiles[neighbour->open_pos] = tmp;
			neighbour->open_pos /= 2;
		} else {
			break;
		}
	}
	
	neighbour->state = PF_STATE_OPEN;
}

int pf_find_path(int x, int y)
{
	actor *me;
	int i;
	
	pf_destroy_path();
	
	if (!(me = pf_get_our_actor())) {
		return -1;
	}
	
	pf_src_tile = pf_get_tile(me->x_tile_pos, me->y_tile_pos);
	pf_dst_tile = pf_get_tile(x, y);
	
	if (!pf_dst_tile || pf_dst_tile->z == 0) {
		return 0;
	}
	
	for (i = 0; i < tile_map_size_x*tile_map_size_y*6*6; i++) {
		pf_tile_map[i].state = PF_STATE_NONE;
		pf_tile_map[i].parent = NULL;
	}

	pf_open.tiles = (PF_TILE **)calloc(tile_map_size_x*tile_map_size_y*6*6, sizeof(PF_TILE*));
	pf_open.count = 0;
	
	pf_add_tile_to_open_list(NULL, pf_src_tile);
	
	while ((pf_cur_tile = pf_get_next_open_tile())) {
		if (pf_cur_tile == pf_dst_tile) {
			pf_follow_path = 1;
			
			pf_movement_timer_callback(0, NULL);
			pf_movement_timer = SDL_AddTimer(2500, pf_movement_timer_callback, NULL);
			break;
		}
		
		pf_add_tile_to_open_list(pf_cur_tile, pf_get_tile(pf_cur_tile->x, pf_cur_tile->y+1));
		pf_add_tile_to_open_list(pf_cur_tile, pf_get_tile(pf_cur_tile->x+1, pf_cur_tile->y+1));
		pf_add_tile_to_open_list(pf_cur_tile, pf_get_tile(pf_cur_tile->x+1, pf_cur_tile->y));
		pf_add_tile_to_open_list(pf_cur_tile, pf_get_tile(pf_cur_tile->x+1, pf_cur_tile->y-1));
		pf_add_tile_to_open_list(pf_cur_tile, pf_get_tile(pf_cur_tile->x, pf_cur_tile->y-1));
		pf_add_tile_to_open_list(pf_cur_tile, pf_get_tile(pf_cur_tile->x-1, pf_cur_tile->y-1));
		pf_add_tile_to_open_list(pf_cur_tile, pf_get_tile(pf_cur_tile->x-1, pf_cur_tile->y));
		pf_add_tile_to_open_list(pf_cur_tile, pf_get_tile(pf_cur_tile->x-1, pf_cur_tile->y+1));
	}
	
	free(pf_open.tiles);
	
	return pf_follow_path;
}

void pf_destroy_path()
{
	if (pf_movement_timer) {
		SDL_RemoveTimer(pf_movement_timer);
		pf_movement_timer = NULL;
	}
	pf_follow_path = 0;
}

actor *pf_get_our_actor()
{
	int i;
	
	if (yourself == -1) {
		return NULL;
	}

	for (i = 0; i < max_actors; i++) {
		if (actors_list[i]->actor_id == yourself) {
			return actors_list[i];
		}
	}
	
	return NULL;
}

void pf_move()
{
	int x, y;
	actor *me;
	
	if (!pf_follow_path || !(me = pf_get_our_actor())) {
		return;
	}
	
	x = me->x_tile_pos;
	y = me->y_tile_pos;
	
	if ((PF_DIFF(x, pf_dst_tile->x) < 2 && PF_DIFF(y, pf_dst_tile->y) < 2)) {
		pf_destroy_path();
	} else {
		PF_TILE *t = pf_get_tile(x, y);
		int i = 0, j = 0;
		
		for (pf_cur_tile = pf_dst_tile; pf_cur_tile; pf_cur_tile = pf_cur_tile->parent) {
			if (pf_cur_tile == t) {
				break;
			}
			i++;
		}

		if (pf_cur_tile == t) {
			for (pf_cur_tile = pf_dst_tile; pf_cur_tile; pf_cur_tile = pf_cur_tile->parent) {
				if (j++ == i-12) {
					break;
				}
			}
			if (pf_cur_tile) {
				Uint8 str[5];

				str[0] = MOVE_TO;
				*((short *)(str+1)) = pf_cur_tile->x;
				*((short *)(str+3)) = pf_cur_tile->y;
				my_tcp_send(my_socket, str, 5);

				return;
			}
		}

		for (pf_cur_tile = pf_dst_tile; pf_cur_tile; pf_cur_tile = pf_cur_tile->parent) {
			if (PF_DIFF(x, pf_cur_tile->x) <= 12 && PF_DIFF(y, pf_cur_tile->y) <= 12
			&& !pf_is_tile_occupied(pf_cur_tile->x, pf_cur_tile->y)) {
				Uint8 str[5];
				
				str[0] = MOVE_TO;
				*((short *)(str+1)) = pf_cur_tile->x;
				*((short *)(str+3)) = pf_cur_tile->y;
				my_tcp_send(my_socket, str, 5);
				break;
			}
		}
	}
}

int pf_is_tile_occupied(int x, int y)
{
	int i;
	
	for (i = 0; i < max_actors; i++) {
		if (actors_list[i]->x_tile_pos == x && actors_list[i]->y_tile_pos == y) {
			return 1;
		}
	}
	
	return 0;
}

Uint32 pf_movement_timer_callback(Uint32 interval, void *param)
{
	SDL_Event e;
	
	e.type = SDL_USEREVENT;
	e.user.code = EVENT_MOVEMENT_TIMER;
	SDL_PushEvent(&e);
	
	return interval;
}

void pf_move_to_mouse_position()
{
	int min_mouse_x = (window_width-hud_x)/6;
	int min_mouse_y = 0;
	
	int max_mouse_x = min_mouse_x+((window_width-hud_x)/1.5);
	int max_mouse_y = window_height - hud_y;
	
	int screen_map_width = max_mouse_x - min_mouse_x;
	int screen_map_height = max_mouse_y - min_mouse_y;
	
	int x, y, clicked_x, clicked_y;
	
	if(check_hud_interface() > 0);
	//if(mouse_x>map_icon_x_start && mouse_y>map_icon_y_start &&
	//		mouse_x<map_icon_x_end && mouse_y<map_icon_y_end)
	//	{
	//		if(interface_mode==interface_game)switch_to_game_map();
	//					else if(interface_mode==interface_map)switch_from_game_map();
	//		return;
	//}

	if (mouse_x < min_mouse_x
	|| mouse_x > max_mouse_x
	|| mouse_y < min_mouse_y
	|| mouse_y > max_mouse_y) {
		return;
	}
	
	x = clicked_x = ((mouse_x - min_mouse_x) * tile_map_size_x * 6) / screen_map_width;
	y = clicked_y = (tile_map_size_y * 6) - ((mouse_y * tile_map_size_y * 6) / screen_map_height);
	
	if (pf_find_path(x, y)) {
		return;
	}
	
	for (x = clicked_x-3; x <= clicked_x+3 ; x++) {
		for (y = clicked_y-3; y <= clicked_y+3; y++) {
			if (x == clicked_x && y == clicked_y) {
				continue;
			}
			if (pf_find_path(x, y)) {
				return;
			}
		}
	}
}

