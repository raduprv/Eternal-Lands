#ifndef __TILE_H__
#define __TILE_H__

typedef struct
{
	char * img;
	int x;
	int y;
} img_struct;

img_struct map_tiles[256];

extern unsigned char *tile_map;
extern unsigned char *height_map;
extern int tile_map_size_x;
extern int tile_map_size_y;
extern int tile_list[256];

void draw_tile_map();
void load_map_tiles();
void destroy_map_tiles();

#endif
