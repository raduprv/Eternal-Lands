#include <stdlib.h>
#include "pathfinder.h"
#include "actors_list.h"
#include "events.h"
#include "gl_init.h"
#include "hud.h"
#include "interface.h"
#include "multiplayer.h"
#include "tiles.h"

// Maximum deviation from the target tile, when clicking on an unwalkable tile. Used only when
// search_destination_area is non-zero.
#define MAX_DEVIATION 2

// Cost of a grid-aligned step
static const uint16_t aligned_cost = 10;
// Cost of a diagonal step
static const uint16_t diagonal_cost = 14;

PF_TILE *pf_tile_map = NULL;
PF_TILE *pf_dst_tile;
int pf_follow_path = 0;
int pf_search_destination_area = 1;

static PF_OPEN_LIST pf_open;
static PF_TILE *pf_src_tile, *pf_cur_tile;
static int pf_visited_squares[20];
static SDL_TimerID pf_movement_timer = 0;

#define PF_DIFF(a, b) ((a > b) ? a - b : b - a)
#define PF_HEUR(a, b) pf_heuristic(a->x-b->x, a->y-b->y);
#define PF_SWAP(i, j) {\
	PF_TILE *a = pf_open.tiles[i], *b = pf_open.tiles[j];\
	a->open_pos = j; b->open_pos = i;\
	pf_open.tiles[i] = b; pf_open.tiles[j] = a;\
}

// Return the octile distance (allowing diagonal moves) for a movement of dx steps in the
// x direction and dy steps in the y direction. This is _not_ a cost function, it returns the
// number of steps required for covering a distance (dx, dy), with diagonal steps treated the
// same as grid-aligned steps.
static inline int octile_distance(int dx, int dy)
{
	if (dx < 0) dx = -dx;
	if (dy < 0) dy = -dy;
	return max2i(dx, dy);
}

static __inline__ int pf_heuristic(int dx, int dy)
{
	if (dx < 0) dx = -dx;
	if (dy < 0) dy = -dy;
	// Grum: below heuristic overestimates the distance to the target since
	// it doesn't take diagonal moves into account. The paths it generates
	// may therefore be slightly too long, but it's much faster than the
	// more accurate heuristic below (mainly because it expands fewer nodes)
	return aligned_cost * (dx + dy);
#if 0
	// Grum: Is the cost of a diagonal move really sqrt(2) times that of
	// an aligned move? If not, the below should simply be max(dx, dy), and
	// the cost function in pf_add_tile_to_open_list() should also be
	// updated.
	return dx < dy ? diagonal_cost*dx + aligned_cost*(dy-dx) : diagonal_cost*dy + aligned_cost*(dx-dy);
#endif
}

#ifdef NO_PF_MACRO
static __inline__ PF_TILE *pf_get_tile(int x, int y)
{
	if (x >= tile_map_size_x*6 || y >= tile_map_size_y*6 || x < 0 || y < 0) {
		return NULL;
	}
	return &pf_tile_map[y*tile_map_size_x*6+x];
}
#else
#define pf_get_tile(x, y) \
	(((x) >= tile_map_size_x*6 || (y) >= tile_map_size_y*6 || ((Sint32)(x)) < 0 || ((Sint32)(y)) < 0) ? NULL : &pf_tile_map[(y)*tile_map_size_x*6+(x)])
#endif

static PF_TILE *pf_get_next_open_tile()
{
	PF_TILE *ret;

	if (pf_open.count == 0)
		return NULL;

	ret = pf_open.tiles[0];
	ret->state = PF_STATE_CLOSED;

	if (--pf_open.count)
	{
		int i, j;
		PF_TILE *tmp = pf_open.tiles[0] = pf_open.tiles[pf_open.count];

		tmp->open_pos = 0;
		i = 0;
		while ( (j = 2*i + 1) < pf_open.count )
		{
			if (j+1 < pf_open.count && pf_open.tiles[j+1]->f < pf_open.tiles[j]->f)
				j++;
			if (pf_open.tiles[j]->f >= tmp->f)
				break;
			PF_SWAP(i, j);
			i = j;
		}
	}

	return ret;
}

static void pf_add_tile_to_open_list(PF_TILE *current, PF_TILE *neighbour)
{
	if (!neighbour
		|| neighbour->state == PF_STATE_CLOSED
		|| neighbour->z == 0
		|| (current && PF_DIFF(current->z, neighbour->z) > 2))
		return;

	if (current)
	{
		int f, g, h;
		int diagonal = (neighbour->x != current->x && neighbour->y != current->y);

#ifdef	FUZZY_PATHS
		g = current->g + (diagonal ? diagonal_cost : aligned_cost) + rand()%3;
#else	//FUZZY_PATHS
		g = current->g + (diagonal ? diagonal_cost : aligned_cost);
#endif	//FUZZY_PATHS
		h = PF_HEUR(neighbour, pf_dst_tile);
		f = g + h;

		if (neighbour->state == PF_STATE_OPEN && f >= neighbour->f)
			return;

		neighbour->f = f;
		neighbour->g = g;
		neighbour->parent = current;
	}
	else
	{
		neighbour->f = PF_HEUR(pf_src_tile, pf_dst_tile);
		neighbour->g = 0;
		neighbour->parent = NULL;
	}

	if (neighbour->state != PF_STATE_OPEN)
	{
		neighbour->open_pos = pf_open.count++;
		pf_open.tiles[neighbour->open_pos] = neighbour;
	}

	while (neighbour->open_pos > 0)
	{
		int idx = neighbour->open_pos;
		int parent_idx = (idx-1) / 2;
		PF_TILE *parent = pf_open.tiles[parent_idx];

		if (neighbour->f >= parent->f)
			break;

		PF_SWAP(idx, parent_idx);
	}

	neighbour->state = PF_STATE_OPEN;
}

static Uint32 pf_movement_timer_callback_locked(const actor* me, Uint32 interval)
{
	SDL_Event e;

	e.type = SDL_USEREVENT;
	e.user.code = EVENT_MOVEMENT_TIMER;
	SDL_PushEvent(&e);

	return me ? me->step_duration * 10 : interval;
}

// Find the maximum deviation from the destination position (dst_x, dst_y), i.e. the minimum
// distance to the destination at which there is a walkable tile. If the destination itself is
// walkable, 0 is returned. If there is no walkable tile within MAX_DEVIATION tiles of the
// destination, -1 is returned.
static int find_max_deviation(int src_x, int src_y, int dst_x, int dst_y)
{
	PF_TILE* tile = pf_get_tile(dst_x, dst_y);
	if (tile && tile->z != 0)
		return 0;
	if (!pf_search_destination_area)
		return -1;

	for (int dist = 1; dist <= MAX_DEVIATION; ++dist)
	{
		int x, y;

		y = dst_y - dist;
		for (x = dst_x - dist; x < dst_x + dist; ++x)
		{
			tile = pf_get_tile(x, y);
			if (tile && tile->z != 0)
				return dist;
		}
		for ( ; y < dst_y + dist; ++y)
		{
			tile = pf_get_tile(x, y);
			if (tile && tile->z != 0)
				return dist;
		}
		for ( ; x > dst_x - dist; --x)
		{
			tile = pf_get_tile(x, y);
			if (tile && tile->z != 0)
				return dist;
		}
		for ( ; y > dst_y - dist; --y)
		{
			tile = pf_get_tile(x, y);
			if (tile && tile->z != 0)
				return dist;
		}
	}
	return -1;
}

static Uint32 pf_movement_timer_callback(Uint32 interval, void* UNUSED(param))
{
	actor *me;
	locked_list_ptr actors_list = lock_and_get_self(&me);
	Uint32 res = pf_movement_timer_callback_locked(me, interval);
	if(actors_list)
		release_locked_actors_list_and_invalidate(actors_list, &me);

	return res;
}

int pf_find_path(const actor* me, int x, int y)
{
	int i;
	int attempts= 0;
	int max_dev;
	uint16_t max_cost;

	pf_destroy_path();

	pf_dst_tile = pf_get_tile(x, y);
	if (!pf_dst_tile)
		// Destination is outside the map
		return 0;

	pf_src_tile = pf_get_tile(me->x_tile_pos, me->y_tile_pos);
	max_dev = find_max_deviation(pf_src_tile->x, pf_src_tile->y, x, y);
	if (max_dev < 0)
		// Unable to get even near the destination tile
		return 0;

	max_cost = max_dev > 0 ? 3 * pf_heuristic(x - pf_src_tile->x, y - pf_src_tile->y) : 0xffff;

	for (i = 0; i < tile_map_size_x*tile_map_size_y*6*6; i++)
	{
		pf_tile_map[i].state = PF_STATE_NONE;
		pf_tile_map[i].parent = NULL;
	}

	pf_open.tiles = calloc(tile_map_size_x*tile_map_size_y*6*6, sizeof(PF_TILE*));
	pf_open.count = 0;

	pf_add_tile_to_open_list(NULL, pf_src_tile);

	while ((pf_cur_tile = pf_get_next_open_tile()) && attempts++ < MAX_PATHFINDER_ATTEMPTS)
	{
		if (octile_distance(pf_cur_tile->x - x, pf_cur_tile->y - y) <= max_dev)
		{
			pf_dst_tile = pf_cur_tile;
			pf_follow_path = 1;

			pf_movement_timer_callback_locked(me, 0);
			pf_movement_timer = SDL_AddTimer(me->step_duration * 10,
				pf_movement_timer_callback, NULL);
			break;
		}

		if (max_cost - pf_cur_tile->f > diagonal_cost)
		{
			pf_add_tile_to_open_list(pf_cur_tile, pf_get_tile(pf_cur_tile->x,   pf_cur_tile->y+1));
			pf_add_tile_to_open_list(pf_cur_tile, pf_get_tile(pf_cur_tile->x+1, pf_cur_tile->y+1));
			pf_add_tile_to_open_list(pf_cur_tile, pf_get_tile(pf_cur_tile->x+1, pf_cur_tile->y));
			pf_add_tile_to_open_list(pf_cur_tile, pf_get_tile(pf_cur_tile->x+1, pf_cur_tile->y-1));
			pf_add_tile_to_open_list(pf_cur_tile, pf_get_tile(pf_cur_tile->x,   pf_cur_tile->y-1));
			pf_add_tile_to_open_list(pf_cur_tile, pf_get_tile(pf_cur_tile->x-1, pf_cur_tile->y-1));
			pf_add_tile_to_open_list(pf_cur_tile, pf_get_tile(pf_cur_tile->x-1, pf_cur_tile->y));
			pf_add_tile_to_open_list(pf_cur_tile, pf_get_tile(pf_cur_tile->x-1, pf_cur_tile->y+1));
		}
	}

	free(pf_open.tiles);

	return pf_follow_path;
}

void pf_destroy_path()
{
	int i;

	if (pf_movement_timer)
	{
		SDL_RemoveTimer(pf_movement_timer);
		pf_movement_timer = 0;
	}
	pf_follow_path = 0;
	for (i = 0; i < 20; i++)
		pf_visited_squares[i]=-1;
}

int checkvisitedlist(int x, int y)
{
	/*
	 * This (slightly) optimised version of the code stores the X and Y in the same word
	 * x is bits 0-15, y is bits 16-31
	 */
	int i,visited = 0,tx,ty;//visited: Is this square already in the list?
	int square;

	x &= 0xFFFF;
	y &= 0xFFFF;

	square = x | (y << 16);
	for (i = 20-1; i > 0; i--)
	{
		if (pf_visited_squares[i] == square)
			visited = 1; //yes
		//move everything in the list up one place
		//to make room for the new square
		pf_visited_squares[i] = pf_visited_squares[i-1];
	}
	if (pf_visited_squares[0] == square)
		visited = 1; //yes

	//put the new square at the start of the list
	pf_visited_squares[0] = square;

	//check if we have visited and the destination is close to us
	if (visited)
	{
		if (pf_dst_tile->x > x)
        		tx = pf_dst_tile->x-x;
		else
			tx = x-pf_dst_tile->x;

		if (pf_dst_tile->y>y)
			ty = pf_dst_tile->y-y;
		else
			ty = y-pf_dst_tile->y;

		if (tx<5 && ty<5)
			visited = 1;//yes
		else
			visited = 0;//yes
	}

	return visited;
}

static inline int pf_is_tile_occupied(int x, int y)
{
	return actor_occupies_tile(x, y);
}

void pf_move()
{
	int x, y;

	if (!pf_follow_path)
		return;

	if (!self_tile_position(&x, &y))
		return;

	if (PF_DIFF(x, pf_dst_tile->x) < 2 && PF_DIFF(y, pf_dst_tile->y) < 2) {
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
#ifdef	FUZZY_PATHS
			int	limit= i-(10+rand()%3);
#else	//FUZZY_PATHS
			int	limit= i-12;
#endif	//FUZZY_PATHS
			for (pf_cur_tile = pf_dst_tile; pf_cur_tile; pf_cur_tile = pf_cur_tile->parent) {
				if (j++ == limit) {
					break;
				}
			}
			if (pf_cur_tile) {
				Uint8 str[5];

				str[0] = MOVE_TO;
				*((short *)(str+1)) = SDL_SwapLE16((short)pf_cur_tile->x);
				*((short *)(str+3)) = SDL_SwapLE16((short)pf_cur_tile->y);
				my_tcp_send(str, 5);

				return;
			}
		}

		for (pf_cur_tile = pf_dst_tile; pf_cur_tile; pf_cur_tile = pf_cur_tile->parent) {
			if (PF_DIFF(x, pf_cur_tile->x) <= 12 && PF_DIFF(y, pf_cur_tile->y) <= 12
			&& !pf_is_tile_occupied(pf_cur_tile->x, pf_cur_tile->y)) {
				Uint8 str[5];

				str[0] = MOVE_TO;
				*((short *)(str+1)) = SDL_SwapLE16((short)pf_cur_tile->x);
				*((short *)(str+3)) = SDL_SwapLE16((short)pf_cur_tile->y);
				my_tcp_send(str, 5);
				break;
			}
		}
	}
}

int pf_get_mouse_position(int mouse_x, int mouse_y, int * px, int * py)
{
	return pf_get_mouse_position_extended(mouse_x, mouse_y, px, py, tile_map_size_x, tile_map_size_y);
}

int pf_get_mouse_position_extended(int mouse_x, int mouse_y, int * px, int * py, int tile_x, int tile_y)
{
	int screen_map_width = main_map_screen_x_right - main_map_screen_x_left;
	int screen_map_height = main_map_screen_y_bottom - main_map_screen_y_top;

	if (mouse_x < main_map_screen_x_left
		|| mouse_x >= main_map_screen_x_right
		|| mouse_y < main_map_screen_y_top
		|| mouse_y >= main_map_screen_y_bottom)
	{
		return 0;
	}

	*px = ((mouse_x - main_map_screen_x_left) * tile_x * 6) / screen_map_width;
	*py = (tile_y * 6 - 1) - ((mouse_y - main_map_screen_y_top) * tile_y * 6) / screen_map_height;
	return 1;
}

void pf_move_to_mouse_position()
{
	int x, y, clicked_x, clicked_y;
	int tries;
	locked_list_ptr actors_list;
	actor *me;

	if (!pf_get_mouse_position(mouse_x, mouse_y, &clicked_x, &clicked_y)) return;
	x = clicked_x; y = clicked_y;

	actors_list = lock_and_get_self(&me);
	if (!actors_list)
		return;

	if (!pf_find_path(me, x, y))
	{
		for (x= clicked_x-3, tries= 0; x <= clicked_x+3 && tries < 4 ; x++)
		{
			for (y= clicked_y-3; y <= clicked_y+3 && tries < 4; y++)
			{
				if (x == clicked_x && y == clicked_y)
					continue;

				pf_dst_tile = pf_get_tile(x, y);
				if (pf_dst_tile && pf_dst_tile->z > 0)
				{
					if (pf_find_path(me, x, y))
						goto path_found;
					tries++;
				}
			}
		}
	}

path_found:
	release_locked_actors_list_and_invalidate(actors_list, &me);
}
