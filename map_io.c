#include <stdlib.h>
#include <string.h>
#include "global.h"

int map_type=1;

void destroy_map()
{
	int i;
#ifdef EXTRA_DEBUG
	ERR();
#endif

	//kill the tile and height map
	if(tile_map)
		{
			free(tile_map);
			tile_map=0;
		}
	memset(tile_list,0,sizeof(tile_list));

	if(height_map)
		{
			free(height_map);
			height_map=0;
		}

	///kill the pathfinding tile map
	if(pf_tile_map)
		{
			free(pf_tile_map);
			pf_tile_map=0;
			
			if (pf_follow_path) {
				pf_destroy_path();
			}
		}

	//kill the 3d objects links
	for(i=0;i<max_obj_3d;i++)
		{
			if(objects_list[i])
				{
					if(objects_list[i]->clouds_uv)free(objects_list[i]->clouds_uv);
					free(objects_list[i]);
					objects_list[i]=0;//kill any refference to it
				}
		}
	// reset the top pointer
	highest_obj_3d= 0;

	//kill the 2d objects links
	for(i=0;i<max_obj_2d;i++)
		{
			if(obj_2d_list[i])
				{
					free(obj_2d_list[i]);
					obj_2d_list[i]=0;//kill any refference to it
				}
		}

	//kill the lights links
	for(i=0;i<max_lights;i++)
		{
			if(lights_list[i])
				{
					free(lights_list[i]);
					lights_list[i]=0;//kill any refference to it
				}
		}

}

int save_map(char * file_name)
{
	int i,j;
	map_header cur_map_header;
	char * mem_map_header=(char *)&cur_map_header;

	object3d_io cur_3d_obj_io;
	int obj_3d_no=0;
	int obj_3d_io_size;

	obj_2d_io cur_2d_obj_io;
	int obj_2d_no=0;
	int obj_2d_io_size;

	light_io cur_light_io;
	int lights_no=0;
	int lights_io_size;

	particles_io cur_particles_io;
	int particles_no=0;
	int particles_io_size;

	FILE *f = NULL;


	//get the sizes of structures (they might change in the future)
	obj_3d_io_size=sizeof(object3d_io);
	obj_2d_io_size=sizeof(obj_2d_io);
	lights_io_size=sizeof(light_io);
	particles_io_size=sizeof(particles_io);

	//get the number of objects and lights
	for(i=0;i<highest_obj_3d;i++)if(objects_list[i])obj_3d_no++;
	for(i=0;i<max_obj_2d;i++)if(obj_2d_list[i])obj_2d_no++;
	for(i=0;i<max_lights;i++)if(lights_list[i])lights_no++;
	// We ignore temporary particle systems (i.e. ones with a ttl>=0)
	for(i=0;i<max_particle_systems;i++)if(particles_list[i] && particles_list[i]->def && particles_list[i]->def->ttl<0)particles_no++;

	//ok, now build the header...
	//clear the header
	memset(mem_map_header, 0, sizeof(mem_map_header));

	//build the file signature
	cur_map_header.file_sig[0]='e';
	cur_map_header.file_sig[1]='l';
	cur_map_header.file_sig[2]='m';
	cur_map_header.file_sig[3]='f';

	cur_map_header.tile_map_x_len=tile_map_size_x;
	cur_map_header.tile_map_y_len=tile_map_size_y;
	cur_map_header.tile_map_offset=sizeof(map_header);
	cur_map_header.height_map_offset=cur_map_header.tile_map_offset+tile_map_size_x*tile_map_size_y;
	cur_map_header.obj_3d_struct_len=obj_3d_io_size;
	cur_map_header.obj_3d_no=obj_3d_no;
	cur_map_header.obj_3d_offset=cur_map_header.height_map_offset+tile_map_size_x*tile_map_size_y*6*6;
	cur_map_header.obj_2d_struct_len=obj_2d_io_size;
	cur_map_header.obj_2d_no=obj_2d_no;
	cur_map_header.obj_2d_offset=cur_map_header.obj_3d_offset+obj_3d_no*obj_3d_io_size;
	cur_map_header.lights_struct_len=lights_io_size;
	cur_map_header.lights_no=lights_no;
	cur_map_header.lights_offset=cur_map_header.obj_2d_offset+obj_2d_no*obj_2d_io_size;
	cur_map_header.dungeon=dungeon;
	cur_map_header.ambient_r=ambient_r;
	cur_map_header.ambient_g=ambient_g;
	cur_map_header.ambient_b=ambient_b;
	cur_map_header.particles_struct_len=particles_io_size;
	cur_map_header.particles_no=particles_no;
	cur_map_header.particles_offset=cur_map_header.lights_offset+lights_no*lights_io_size;

	//ok, now let's open/create the file, and start writting the header...
	f=fopen(file_name, "wb");

	//write the header
	fwrite(mem_map_header, sizeof(map_header), 1, f);

	//write the tiles map
	fwrite(tile_map, tile_map_size_x*tile_map_size_y, 1, f);

	//write the heights map
	fwrite(height_map, tile_map_size_x*tile_map_size_y*6*6, 1, f);

	//write the 3d objects
	j=0;
	for(i=0;i<highest_obj_3d;i++)
		{

			if(j>obj_3d_no)break;
			if(objects_list[i])
				{
					char * cur_3do_pointer=(char *)&cur_3d_obj_io;
					Uint32 k=0;

					//clear the object
					for(k=0;k<sizeof(object3d_io);k++)cur_3do_pointer[k]=0;

					sprintf(cur_3d_obj_io.file_name,"%s",objects_list[i]->file_name);
					cur_3d_obj_io.x_pos=objects_list[i]->x_pos;
					cur_3d_obj_io.y_pos=objects_list[i]->y_pos;
					cur_3d_obj_io.z_pos=objects_list[i]->z_pos;

					cur_3d_obj_io.x_rot=objects_list[i]->x_rot;
					cur_3d_obj_io.y_rot=objects_list[i]->y_rot;
					cur_3d_obj_io.z_rot=objects_list[i]->z_rot;

					cur_3d_obj_io.self_lit=objects_list[i]->self_lit;
					cur_3d_obj_io.blended=objects_list[i]->blended;

					cur_3d_obj_io.r=objects_list[i]->r;
					cur_3d_obj_io.g=objects_list[i]->g;
					cur_3d_obj_io.b=objects_list[i]->b;

					fwrite(cur_3do_pointer, sizeof(object3d_io), 1, f);

					j++;
				}
		}

	//write the 2d objects
	j=0;
	for(i=0;i<max_obj_2d;i++)
		{

			if(j>obj_2d_no)break;
			if(obj_2d_list[i])
				{
					char * cur_2do_pointer=(char *)&cur_2d_obj_io;
					Uint32 k=0;

					//clear the object
					for(k=0;k<sizeof(obj_2d_io);k++)cur_2do_pointer[k]=0;

					sprintf(cur_2d_obj_io.file_name,"%s",obj_2d_list[i]->file_name);
					cur_2d_obj_io.x_pos=obj_2d_list[i]->x_pos;
					cur_2d_obj_io.y_pos=obj_2d_list[i]->y_pos;
					cur_2d_obj_io.z_pos=obj_2d_list[i]->z_pos;

					cur_2d_obj_io.x_rot=obj_2d_list[i]->x_rot;
					cur_2d_obj_io.y_rot=obj_2d_list[i]->y_rot;
					cur_2d_obj_io.z_rot=obj_2d_list[i]->z_rot;

					fwrite(cur_2do_pointer, sizeof(obj_2d_io), 1, f);

					j++;
				}
		}

	//write the lights
	j=0;
	for(i=0;i<max_lights;i++)
		{
			if(j>lights_no)break;
			if(lights_list[i])
				{
					char * cur_light_pointer=(char *)&cur_light_io;
					Uint32 k=0;

					//clear the object
					for(k=0;k<sizeof(light_io);k++)cur_light_pointer[k]=0;

					cur_light_io.pos_x=lights_list[i]->pos_x;
					cur_light_io.pos_y=lights_list[i]->pos_y;
					cur_light_io.pos_z=lights_list[i]->pos_z;

					cur_light_io.r=lights_list[i]->r;
					cur_light_io.g=lights_list[i]->g;
					cur_light_io.b=lights_list[i]->b;

					fwrite(cur_light_pointer, sizeof(light_io), 1, f);

					j++;
				}
		}

	// Write the particle systems
	j=0;
	for(i=0;i<max_particle_systems;i++)
		{
			if(j>particles_no)break;
			if(particles_list[i] && particles_list[i]->def && particles_list[i]->def->ttl<0)
				{
					char *cur_particles_pointer=(char *)&cur_particles_io;
					Uint32 k=0;
					for(k=0;k<sizeof(particles_io);k++)cur_particles_pointer[k]=0;
					sprintf(cur_particles_io.file_name,"%s",particles_list[i]->def->file_name);
					cur_particles_io.x_pos=particles_list[i]->x_pos;
					cur_particles_io.y_pos=particles_list[i]->y_pos;
					cur_particles_io.z_pos=particles_list[i]->z_pos;
					fwrite(cur_particles_pointer,sizeof(particles_io),1,f);
					j++;
				}
		}

	fclose(f);

	return 1;

}


int get_cur_map(char * file_name)
{
    //Well, where are we? Let's check.
    //Skip "./maps/"
    file_name += 7;
    
    if(!strcmp("startmap.elm", file_name)) return 0;
    else if(!strcmp("map2.elm", file_name)) return 1;
    else if(!strcmp("map3.elm", file_name)) return 2;
    else if(!strcmp("map4f.elm", file_name)) return 3;
    else if(!strcmp("map5nf.elm", file_name)) return 4;
    else if(!strcmp("map6nf.elm", file_name)) return 5;
    else if(!strcmp("map7.elm", file_name)) return 6;
    else if(!strcmp("map8.elm", file_name)) return 7;
    else if(!strcmp("map9f.elm", file_name)) return 8;
    else if(!strcmp("map10.elm", file_name)) return 9;
    else if(!strcmp("map11.elm", file_name)) return 10;
    else if(!strcmp("map12.elm", file_name)) return 11;
    else if(!strcmp("map13.elm", file_name)) return 12;
    else if(!strcmp("map14f.elm", file_name)) return 13;
    else if(!strcmp("map15f.elm", file_name)) return 14;
    return -1; //We don't know where we are...
}


int load_map(char * file_name)
{
	int i;
	map_header cur_map_header;
	char * mem_map_header=(char *)&cur_map_header;

	object3d_io cur_3d_obj_io;
	int obj_3d_no=0;
	int obj_3d_io_size;

	obj_2d_io cur_2d_obj_io;
	int obj_2d_no=0;
	int obj_2d_io_size;

	light_io cur_light_io;
	int lights_no=0;
	int lights_io_size;

	particles_io cur_particles_io;
	int particles_no=0;
	int particles_io_size;

	FILE *f = NULL;
	f=fopen(file_name, "rb");
	if(!f)return 0;

#ifdef EXTRA_DEBUG
	ERR();
#endif
	
	my_strcp(map_file_name,file_name);

	destroy_map();

	fread(mem_map_header, 1, sizeof(cur_map_header), f);//header only
	
	
	//verify if we have a valid file
	if(cur_map_header.file_sig[0]!='e'||
	   cur_map_header.file_sig[1]!='l'||
	   cur_map_header.file_sig[2]!='m'||
	   cur_map_header.file_sig[3]!='f')
		{
			char str[200];
			sprintf(str,invalid_map,map_file_name);
			log_error(str);
			exit_now=1;//We might as well quit...
			return 0;
		}
    
	//get the map size
	tile_map_size_x=cur_map_header.tile_map_x_len;
	tile_map_size_y=cur_map_header.tile_map_y_len;

	//allocate memory for the tile map (it was destroyed)
	tile_map=(char *)calloc(tile_map_size_x*tile_map_size_y, 1);
	//allocates the memory for the heights now
	height_map=(char *)calloc(tile_map_size_x*tile_map_size_y*6*6, 1);

	//get the sizes of structures (they might change in the future)
	obj_3d_io_size=cur_map_header.obj_3d_struct_len;
	obj_2d_io_size=cur_map_header.obj_2d_struct_len;
	lights_io_size=cur_map_header.lights_struct_len;
	particles_io_size=cur_map_header.particles_struct_len;

	//get the number of objects and lights
	obj_3d_no=cur_map_header.obj_3d_no;
	obj_2d_no=cur_map_header.obj_2d_no;
	lights_no=cur_map_header.lights_no;
	particles_no=cur_map_header.particles_no;

	//get the type of map, and the ambient light
	dungeon=cur_map_header.dungeon;
	ambient_r=cur_map_header.ambient_r;
	ambient_g=cur_map_header.ambient_g;
	ambient_b=cur_map_header.ambient_b;

	if(!dungeon) cur_map=get_cur_map(file_name);//Otherwise we pretend that we don't know where we are - if anyone wants to do the work and input all coordinates it's fine by me however :o)
	else cur_map=-1;

	if(!strcmp(file_name+7,"startmap.elm")) {
		map_type=2; //Remove this hack and set the map type from the server!
		colors_list[c_grey1].r1=0.6f*255;
		colors_list[c_grey1].g1=0.9f*255;
		colors_list[c_grey1].b1=0.9f*255;
		colors_list[c_yellow1].r1=217;
		colors_list[c_yellow1].g1=212;
		colors_list[c_yellow1].b1=126;
	} else {
		colors_list[c_grey1].r1=255;
		colors_list[c_grey1].g1=255;
		colors_list[c_grey1].b1=255;
		colors_list[c_yellow1].r1=251;
		colors_list[c_yellow1].g1=250;
		colors_list[c_yellow1].b1=190;
		map_type=1;
	}

	//this is useful if we go in/out a dungeon
	new_minute();

	//read the tiles map
	fread(tile_map, 1, tile_map_size_x*tile_map_size_y, f);

	//load the tiles in this map, if not already loaded
	load_map_tiles();

	//read the heights map
	fread(height_map, 1, tile_map_size_x*tile_map_size_y*6*6, f);

	//create the tile map that will be used for pathfinding
	pf_tile_map = (PF_TILE *)calloc(tile_map_size_x*tile_map_size_y*6*6, sizeof(PF_TILE));
	{
		int i, x, y;
		
		for (x = 0; x < tile_map_size_x*6; x++) {
			for (y = 0; y < tile_map_size_y*6; y++) {
				i = y*tile_map_size_x*6+x;
				pf_tile_map[i].x = x;
				pf_tile_map[i].y = y;
				pf_tile_map[i].z = height_map[i];
			}
		}
	}
	
	//see which objects in our cache are not used in this map
	//read the 3d objects
	for(i=0;i<obj_3d_no;i++)
		{
			char * cur_3do_pointer=(char *)&cur_3d_obj_io;
			fread(cur_3do_pointer, 1, obj_3d_io_size, f);

			add_e3d(cur_3d_obj_io.file_name,cur_3d_obj_io.x_pos,cur_3d_obj_io.y_pos,
					cur_3d_obj_io.z_pos,cur_3d_obj_io.x_rot,cur_3d_obj_io.y_rot,cur_3d_obj_io.z_rot,
					cur_3d_obj_io.self_lit,cur_3d_obj_io.blended,cur_3d_obj_io.r,cur_3d_obj_io.g,cur_3d_obj_io.b);
		}

	//delete the unused objects from the cache

	//read the 2d objects
	for(i=0;i<obj_2d_no;i++)
		{
			char * cur_2do_pointer=(char *)&cur_2d_obj_io;
			fread(cur_2do_pointer, 1, obj_2d_io_size, f);

			add_2d_obj(cur_2d_obj_io.file_name,cur_2d_obj_io.x_pos,cur_2d_obj_io.y_pos,
					   cur_2d_obj_io.z_pos,cur_2d_obj_io.x_rot,cur_2d_obj_io.y_rot,cur_2d_obj_io.z_rot);
		}


	//read the lights
	for(i=0;i<lights_no;i++)
		{
			char * cur_light_pointer=(char *)&cur_light_io;
			fread(cur_light_pointer, 1, lights_io_size, f);
			add_light(cur_light_io.pos_x,cur_light_io.pos_y,cur_light_io.pos_z,cur_light_io.r,cur_light_io.g,cur_light_io.b,1.0f);
		}

	//read particle systems
	for(i=0;i<particles_no;i++)
		{
			char *cur_particles_pointer=(char *)&cur_particles_io;
			fread(cur_particles_pointer,1,particles_io_size,f);
			add_particle_sys(cur_particles_io.file_name,cur_particles_io.x_pos,cur_particles_io.y_pos,cur_particles_io.z_pos);
		}

	fclose(f);

#ifdef EXTRA_DEBUG
	ERR();//We finished loading the new map apparently...
#endif
	sector_add_map();
	return 1;

}

void new_map(int m_x_size,int m_y_size)
{
	int i;

#ifdef EXTRA_DEBUG
	ERR();
#endif
	//destroy the previous map, if any
	destroy_map();

	//allocate memory for the tile map (it was destroyed)
	tile_map=(char *)calloc(m_x_size*m_y_size, 1);
	//now, fill the map
	for(i=0;i<m_x_size*m_y_size;i++)tile_map[i]=1;
	tile_map_size_x=m_x_size;
	tile_map_size_y=m_y_size;

	//allocates the memory for the heights now
	height_map=(char *)calloc(m_x_size*m_y_size*6*6, 1);
	//now, fill the map
	for(i=0;i<m_x_size*m_y_size*6*6;i++)height_map[i]=11;

	load_map_tiles();


}

