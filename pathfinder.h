#ifndef __PATHFINDER_H__
#define __PATHFINDER_H__

enum { SDL_PF_MOVEMENT_TIMER };

typedef struct
{
	int open;
	int open_pos;
	int closed;
	
	int x;
	int y;
	int z;
	
	int f;
	int g;
	int h;
	
	void *parent;
} PF_TILE;

typedef struct
{
	PF_TILE **tiles;
	int count;
} PF_OPEN_LIST;

extern PF_TILE *pf_tile_map;
extern PF_TILE *pf_dst_tile;
extern int pf_path_length;
extern int pf_follow_path;

PF_TILE *pf_get_tile(int x, int y);
PF_TILE *pf_get_next_open_tile();
void pf_add_tile_to_open_list(PF_TILE *current, PF_TILE *neighbour);
int pf_find_path(int x, int y);
void pf_destroy_path();
actor *pf_get_our_actor();
void pf_move();
int pf_is_tile_occupied(int x, int y);
Uint32 pf_movement_timer_callback(Uint32 interval, void *param);
void pf_move_to_mouse_position();

#endif /* __PATHFINDER_H__ */
