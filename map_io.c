#include "global.h"

void destroy_map()
{
	int i;

	//kill the tile and height map
	if(tile_map)
		{
			free(tile_map);
			tile_map=0;
		}

	if(height_map)
		{
			free(height_map);
			height_map=0;
		}


	//kill the 3d objects links
	for(i=0;i<max_obj_3d;i++)
		{
			if(objects_list[i])
				{
					free(objects_list[i]);
					objects_list[i]=0;//kill any refference to it
				}
		}

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

	destroy_all_particles();
}

int save_map(char * file_name)
{
	int i,j;
	map_header cur_map_header;
	char * mem_map_header=(char *)&cur_map_header;

	object3d_io cur_3d_obj_io;
	object3d cur_3d_obj;
	int obj_3d_no=0;
	int obj_3d_io_size;

	obj_2d_io cur_2d_obj_io;
	obj_2d cur_2d_obj;
	int obj_2d_no=0;
	int obj_2d_io_size;

	light_io cur_light_io;
	light cur_light;
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
	for(i=0;i<max_obj_3d;i++)if(objects_list[i])obj_3d_no++;
	for(i=0;i<max_obj_2d;i++)if(obj_2d_list[i])obj_2d_no++;
	for(i=0;i<max_lights;i++)if(lights_list[i])lights_no++;
	// We ignore temporary particle systems (i.e. ones with a ttl)
	for(i=0;i<max_particle_systems;i++)if(particles_list[i] && particles_list[i]->def && particles_list[i]->def != &def)particles_no++;

	//ok, now build the header...
	//clear the header
	for(i=0;i<sizeof(map_header);i++)mem_map_header[i]=0;

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
	for(i=0;i<max_obj_3d;i++)
		{

			if(j>obj_3d_no)break;
			if(objects_list[i])
				{
					char * cur_3do_pointer=(char *)&cur_3d_obj_io;
					int k=0;

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
					int k=0;

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
					int k=0;

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
			if(particles_list[i] && particles_list[i]->def && particles_list[i]->def != &def)
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

int load_map(char * file_name)
{
	int i,j;
	map_header cur_map_header;
	char * mem_map_header=(char *)&cur_map_header;


	object3d_io cur_3d_obj_io;
	object3d cur_3d_obj;
	int obj_3d_no=0;
	int obj_3d_io_size;

	obj_2d_io cur_2d_obj_io;
	obj_2d cur_2d_obj;
	int obj_2d_no=0;
	int obj_2d_io_size;

	light_io cur_light_io;
	light cur_light;
	int lights_no=0;
	int lights_io_size;

	particles_io cur_particles_io;
	int particles_no=0;
	int particles_io_size;

	FILE *f = NULL;
	f=fopen(file_name, "rb");
	if(!f)return 0;

	destroy_map();

	fread(mem_map_header, 1, sizeof(cur_map_header), f);//header only

	//get the map size
	tile_map_size_x=cur_map_header.tile_map_x_len;
	tile_map_size_y=cur_map_header.tile_map_y_len;

	//allocate memory for the tile map (it was destroyed)
	tile_map=(unsigned char *)calloc(tile_map_size_x*tile_map_size_y, 1);
	//allocates the memory for the heights now
	height_map=(unsigned char *)calloc(tile_map_size_x*tile_map_size_y*6*6, 1);

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


	//see if we have to change the water texture (when we get from dungeon to surface or
	//the other way around
	if(dungeon && !cur_map_header.dungeon)
		{
			//ok, change the water to normal
			glDeleteTextures(1,&texture_cache[sky_text_1].texture_id);
			//also destroy the name of that texture, since it is not in cache anymore
			texture_cache[sky_text_1].file_name[0]=0;
			sky_text_1=load_texture_cache("./textures/sky.bmp",70);

		}
	if(!dungeon && cur_map_header.dungeon)
		{
			//ok, change the water to dungeon water
			glDeleteTextures(1,&texture_cache[sky_text_1].texture_id);
			//also destroy the name of that texture, since it is not in cache anymore
			texture_cache[sky_text_1].file_name[0]=0;
			sky_text_1=load_texture_cache("./textures/water2.bmp",70);

		}


	//get the type of map, and the ambient light
	dungeon=cur_map_header.dungeon;
	ambient_r=cur_map_header.ambient_r;
	ambient_g=cur_map_header.ambient_g;
	ambient_b=cur_map_header.ambient_b;

	//this is useful if we go in/out a dungeon
	new_minute();



	//verify if we have a valid file
	if(cur_map_header.file_sig[0]!='e')return 0;
	if(cur_map_header.file_sig[1]!='l')return 0;
	if(cur_map_header.file_sig[2]!='m')return 0;
	if(cur_map_header.file_sig[3]!='f')return 0;

	//read the tiles map
	fread(tile_map, 1, tile_map_size_x*tile_map_size_y, f);

	//load the tiles in this map, if not already loaded
	load_map_tiles();

	//read the heights map
	fread(height_map, 1, tile_map_size_x*tile_map_size_y*6*6, f);

	//read the 3d objects
	for(i=0;i<obj_3d_no;i++)
		{
			char * cur_3do_pointer=(char *)&cur_3d_obj_io;
			fread(cur_3do_pointer, 1, obj_3d_io_size, f);

			add_e3d(cur_3d_obj_io.file_name,cur_3d_obj_io.x_pos,cur_3d_obj_io.y_pos,
			cur_3d_obj_io.z_pos,cur_3d_obj_io.x_rot,cur_3d_obj_io.y_rot,cur_3d_obj_io.z_rot,
			cur_3d_obj_io.self_lit,cur_3d_obj_io.blended,cur_3d_obj_io.r,cur_3d_obj_io.g,cur_3d_obj_io.b);

		}

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
			particles_list[i]->ttl=-1;
		}

	fclose(f);

	return 1;

}

void new_map(int m_x_size,int m_y_size)
{
	int i;
	//destroy the previous map, if any
	destroy_map();

	//allocate memory for the tile map (it was destroyed)
	tile_map=(unsigned char *)calloc(m_x_size*m_y_size, 1);
	//now, fill the map
	for(i=0;i<m_x_size*m_y_size;i++)tile_map[i]=1;
	tile_map_size_x=m_x_size;
	tile_map_size_y=m_y_size;

	//allocates the memory for the heights now
	height_map=(unsigned char *)calloc(m_x_size*m_y_size*6*6, 1);
	//now, fill the map
	for(i=0;i<m_x_size*m_y_size*6*6;i++)height_map[i]=11;

	load_map_tiles();
	//reset the camera coordinates
	cx=0;
	cy=0;


}


