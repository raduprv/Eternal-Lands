#ifndef __SECTOR_H__
#define __SECTOR_H__
#define sector_get(x,y) (((int)y/12)*(tile_map_size_x>>2)+(int)x/12)
typedef struct{
	Uint32 objects_checksum;
	Uint32 tiles_checksum;
	short e3d_local[100];
	short e2d_local[20];
	short lights_local[4];
	short particles_local[8];
}map_sector;
extern map_sector sectors[256*256];
extern int num_sectors;
void get_supersector(int sector, int *sx, int *sy, int *ex, int *ey);
int sector_add_3do(int objectid);
int sector_add_2do(int objectid);
int sector_add_light(int objectid);
int sector_add_particle(int objectid);
void sector_add_map();
#endif
