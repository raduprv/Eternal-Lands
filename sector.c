#include "global.h"

map_sector sectors[256*256];
int num_sectors=256*256;
Uint16 active_sector;
int current_sector;


void get_supersector(int sector, int *sx, int *sy, int *ex, int *ey)
{
	int tile_map_size_y_4=tile_map_size_y>>2;
	int tile_map_size_x_4=tile_map_size_x>>2;
	int fy=sector/(tile_map_size_y_4);
	int fx=sector%(tile_map_size_x_4);

	*sx=fx-1;
	if(*sx<0)*sx=0;
	*sy=fy-1;
	if(*sy<0)*sy=0;
	*ex=fx+1;
	if(*ex==tile_map_size_x_4)*ex=tile_map_size_x_4-1;
	*ey=fy+1;
	if(*ey==tile_map_size_y_4)*ey=tile_map_size_y_4-1;
}


int sector_add_3do(int objectid)
{
	int i;
	int sector_no=sector_get(objects_list[objectid]->x_pos, objects_list[objectid]->y_pos);

	if(sector_no>=num_sectors) return -1;

	for(i=0;i<100;i++){
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
	int sector_no=sector_get(obj_2d_list[objectid]->x_pos, obj_2d_list[objectid]->y_pos);

	if(sector_no>=num_sectors) return -1;

	for(i=0;i<20;i++){
		if(sectors[sector_no].e2d_local[i]==-1){
			sectors[sector_no].e2d_local[i]=objectid;
	//		add_change();
			return i;
		}
	}
	return -1;
}


int sector_add_light(int objectid)
{
	int i;
	int sector_no=sector_get(lights_list[objectid]->pos_x, lights_list[objectid]->pos_y);

	if(sector_no>=num_sectors) return -1;

	for(i=0;i<4;i++){
		if(sectors[sector_no].lights_local[i]==-1){
			sectors[sector_no].lights_local[i]=objectid;
	//		add_change();
			return i;
		}
	}
	return -1;
}


int sector_add_particle(int objectid)
{
	int i;
	int sector_no=sector_get(particles_list[objectid]->x_pos, particles_list[objectid]->y_pos);

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



// adds everything from the maps to the sectors
void sector_add_map()
{
	int i,j=0;
	int obj_3d_no=0;
	int obj_2d_no=0;
	int lights_no=0;
	int particles_no=0;
	memset(sectors,-1,sizeof(map_sector)*256*256);

	for(i=0;i<max_obj_3d;i++)if(objects_list[i])obj_3d_no++;
	for(i=0;i<max_obj_2d;i++)if(obj_2d_list[i])obj_2d_no++;
	for(i=0;i<max_lights;i++)if(lights_list[i])lights_no++;
	for(i=0;i<max_particle_systems;i++){
		if(particles_list[i])
			particles_no++;
	}
	// 3d objects
	for(i=0;i<max_obj_3d;i++){
		if(j>obj_3d_no)
			break;

		if(objects_list[i]){
			sector_add_3do(i);
			j++;
		}
	}

	//2d objects
	j=0;
	for(i=0;i<max_obj_2d;i++){
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
	for(i=0;i<max_lights;i++){
		if(j>lights_no)break;
		if(lights_list[i]){
			sector_add_light(i);
			j++;
		}
	}

	//particle systems
	j=0;
	for(i=0;i<max_particle_systems;i++){
		if(j>particles_no)
			break;
		if(particles_list[i]){
			sector_add_particle(i);
			j++;
		}
	}
*/

}
