#include <stdlib.h>
#include <string.h>
#ifdef MAP_EDITOR2
#include "../map_editor2/global.h"
#else
#include "global.h"
#endif

/* NOTE: This file contains implementations of the following, currently unused, and commented functions:
 *          Look at the end of the file.
 *
 * int sector_add_light(int);
 */

#ifndef	NEW_FRUSTUM
map_sector sectors[256*256];
int num_sectors=256*256;
Uint16 active_sector;
int current_sector;
#else
BBOX_TREE* bbox_tree = NULL;
BBOX_ITEMS* items = NULL;

#define INVALID -1
#define GROUND 0
#define PLANT 1
#define FENCE 2
#endif


#ifndef	NEW_FRUSTUM
void get_supersector(int sector, int *sx, int *sy, int *ex, int *ey)
{
	int tile_map_size_y_4=tile_map_size_y>>2;
	int tile_map_size_x_4=tile_map_size_x>>2;
	int fy=sector/(tile_map_size_y_4);
	int fx=sector%(tile_map_size_x_4);

	*sx=fx-2;
	if(*sx<0)*sx=0;
	*sy=fy-2;
	if(*sy<0)*sy=0;
	*ex=fx+2;
	if(*ex>=tile_map_size_x_4)*ex=tile_map_size_x_4-1;
	*ey=fy+2;
	if(*ey>=tile_map_size_y_4)*ey=tile_map_size_y_4-1;
}


int sector_add_3do(int objectid)
{
	int i;
	int sector_no=SECTOR_GET(objects_list[objectid]->x_pos, objects_list[objectid]->y_pos);

	if(sector_no>=num_sectors) return -1;

	for(i=0;i<MAX_3D_OBJECTS;i++){
		if(sectors[sector_no].e3d_local[i]==-1){
			sectors[sector_no].e3d_local[i]=objectid;
//			add_change();
			return i;
		}
	}
	return -1;
}


int sector_add_2do(int objectid)
{
	int i;
	int sector_no=SECTOR_GET(obj_2d_list[objectid]->x_pos, obj_2d_list[objectid]->y_pos);

	if(sector_no>=num_sectors) return -1;

	for(i=0;i<100;i++){
		if(sectors[sector_no].e2d_local[i]==-1){
			sectors[sector_no].e2d_local[i]=objectid;
	//		add_change();
			return i;
		}
	}
	return -1;
}

int sector_add_particle(int objectid)
{
	int i;
	int sector_no=SECTOR_GET(particles_list[objectid]->x_pos, particles_list[objectid]->y_pos);

	if(sector_no>=num_sectors) return -1;

	for(i=0;i<8;i++){
		if(sectors[sector_no].particles_local[i]==-1){
			sectors[sector_no].particles_local[i]=objectid;
	//		add_change();
			return i;
		}
	}
	return -1;
}
#endif


// adds everything from the maps to the sectors
void sector_add_map()
{
#ifndef	NEW_FRUSTUM
	int i, j = 0;
	int obj_3d_no=0;
	int obj_2d_no=0;
	//int lights_no=0;
	int particles_no=0;
	memset(sectors,-1,sizeof(map_sector)*256*256);

	for(i=0;i<MAX_OBJ_3D;i++)if(objects_list[i])obj_3d_no++;
	for(i=0;i<MAX_OBJ_2D;i++)if(obj_2d_list[i])obj_2d_no++;
	for(i=0;i<MAX_LIGHTS;i++){
		if(lights_list[i]){
			//lights_no++;
			num_lights= i;
		}
	}
	for(i=0;i<MAX_PARTICLE_SYSTEMS;i++){
		if(particles_list[i]){
			particles_no++;
		}
	}
	// 3d objects
	for(i=0;i<MAX_OBJ_3D;i++){
		if(j>obj_3d_no){
			break;
		}
		if(objects_list[i]){
			sector_add_3do(i);
			j++;
		}
	}

	//2d objects
	j=0;
	for(i=0;i<MAX_OBJ_2D;i++){
		if(j>obj_2d_no)
			break;
		if(obj_2d_list[i]){
			sector_add_2do(i);
			j++;
		}
	}
/*
	//lights
	j=0;
	for(i=0;i<MAX_LIGHTS;i++){
		if(j>lights_no)break;
		if(lights_list[i]){
			sector_add_light(i);
			j++;
		}
	}

	//particle systems
	j=0;
	for(i=0;i<MAX_PARTICLE_SYSTEMS;i++){
		if(j>particles_no)
			break;
		if(particles_list[i]){
			sector_add_particle(i);
			j++;
		}
	}
*/

#endif
}

/* currently UNUSED
int sector_add_light(int objectid)
{
	int i;
	int sector_no=SECTOR_GET(lights_list[objectid]->pos_x, lights_list[objectid]->pos_y);

	if(sector_no>=num_sectors) return -1;
	if(objectid >= num_lights)	num_lights= objectid+1;

	for(i=0;i<4;i++){
		if(sectors[sector_no].lights_local[i]==-1){
			sectors[sector_no].lights_local[i]=objectid;
	//		add_change();
			return i;
		}
	}
	return -1;
}
*/
