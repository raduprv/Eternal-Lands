#ifndef __TILE_H__
#define __TILE_H__
#include "../textures.h"

extern image_t map_tiles[256];

extern unsigned char *tile_map;
extern unsigned char *height_map;
extern int tile_map_size_x;
extern int tile_map_size_y;
extern int tile_list[256];

void draw_tile_map();
void load_map_tiles();
void destroy_map_tiles();

float get_tile_height(const float x, const float y);
int get_tile_walkable(const int x, const int y);
int get_tile_valid(const int x, const int y);

#endif
