#ifndef __PATHFINDER_H__
#define __PATHFINDER_H__

enum {
	PF_STATE_NONE=0,
	PF_STATE_OPEN,
	PF_STATE_CLOSED
};

typedef struct
{
	Uint32 open_pos;
	Uint16 x;
	Uint16 y;
	Uint16 f;
	Uint16 g;

	Uint8 state;
	Uint8 z;
	
	void *parent;
} PF_TILE;

typedef struct
{
	PF_TILE **tiles;
	int count;
} PF_OPEN_LIST;

extern PF_TILE *pf_tile_map;
extern PF_TILE *pf_dst_tile;
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
