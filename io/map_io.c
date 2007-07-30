#include "map_io.h"
#include "../global.h"
#ifdef	NEW_FILE_IO
#include "elfilewrapper.h"
#endif	//NEW_FILE_IO

#ifdef EYE_CANDY
 #include "../eye_candy_wrapper.h"
#endif
#ifdef CLUSTER_INSIDES
#include "../cluster.h"
#endif

#ifndef SHOW_FLICKERING
const float offset_2d_increment = (1.0f / 32768.0f);	// (1.0f / 8388608.0f) is the minimum for 32-bit floating point.
float offset_2d = (1.0f / 32768.0f);
const float offset_2d_max = 0.01f;
#endif

int load_map(const char *file_name, update_func *update_function)
{
	int i;
	int cur_tile, j;
	AABBOX bbox;
	map_header cur_map_header;
	char * mem_map_header=(char *)&cur_map_header;
#ifdef CLUSTER_INSIDES
	char* occupied;
#endif

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

#ifdef	NEW_FILE_IO
	el_file_ptr f = NULL;
	f = el_open(file_name);
#else	//NEW_FILE_IO
	FILE *f = NULL;
	f = my_fopen(file_name, "rb");
#endif	//NEW_FILE_IO
	if (!f)
	{
		return 0;
	}
#ifdef EXTRA_DEBUG
	ERR();
#endif
	my_strcp(map_file_name,file_name);

	main_bbox_tree_items = create_bbox_items(1024);
	// XXX (Grum): non-portable
#ifdef	NEW_FILE_IO
	el_read(f, sizeof(cur_map_header), mem_map_header);//header only
#else	//NEW_FILE_IO
	fread(mem_map_header, 1, sizeof(cur_map_header), f);//header only
#endif	//NEW_FILE_IO

	cur_map_header.tile_map_x_len = SDL_SwapLE32(cur_map_header.tile_map_x_len);
	cur_map_header.tile_map_y_len = SDL_SwapLE32(cur_map_header.tile_map_y_len);
	cur_map_header.tile_map_offset = SDL_SwapLE32(cur_map_header.tile_map_offset);
	cur_map_header.height_map_offset = SDL_SwapLE32(cur_map_header.height_map_offset);
	cur_map_header.obj_3d_struct_len = SDL_SwapLE32(cur_map_header.obj_3d_struct_len);
	cur_map_header.obj_3d_no = SDL_SwapLE32(cur_map_header.obj_3d_no);
	cur_map_header.obj_3d_offset = SDL_SwapLE32(cur_map_header.obj_3d_offset);
	cur_map_header.obj_2d_struct_len = SDL_SwapLE32(cur_map_header.obj_2d_struct_len);
	cur_map_header.obj_2d_no = SDL_SwapLE32(cur_map_header.obj_2d_no);
	cur_map_header.obj_2d_offset = SDL_SwapLE32(cur_map_header.obj_2d_offset);
	cur_map_header.lights_struct_len = SDL_SwapLE32(cur_map_header.lights_struct_len);
	cur_map_header.lights_no = SDL_SwapLE32(cur_map_header.lights_no);
	cur_map_header.lights_offset = SDL_SwapLE32(cur_map_header.lights_offset);
    
	cur_map_header.ambient_r = SwapLEFloat(cur_map_header.ambient_r);
	cur_map_header.ambient_g = SwapLEFloat(cur_map_header.ambient_g);
	cur_map_header.ambient_b = SwapLEFloat(cur_map_header.ambient_b);
	
	cur_map_header.particles_struct_len = SDL_SwapLE32(cur_map_header.particles_struct_len);
	cur_map_header.particles_no = SDL_SwapLE32(cur_map_header.particles_no);
	cur_map_header.particles_offset = SDL_SwapLE32(cur_map_header.particles_offset);
	
	//verify if we have a valid file
	if(cur_map_header.file_sig[0]!='e'||
	   cur_map_header.file_sig[1]!='l'||
	   cur_map_header.file_sig[2]!='m'||
	   cur_map_header.file_sig[3]!='f')
		{
			log_error(invalid_map, map_file_name);
			exit_now=1;//We might as well quit...
			return 0;
		}
	update_function(load_map_str, 0);
	//get the map size
	tile_map_size_x=cur_map_header.tile_map_x_len;
	tile_map_size_y=cur_map_header.tile_map_y_len;

	//allocate memory for the tile map (it was destroyed)
	tile_map=(unsigned char *)calloc(tile_map_size_x*tile_map_size_y, 1);
	//allocates the memory for the heights now
	height_map=(unsigned char *)calloc(tile_map_size_x*tile_map_size_y*6*6, 1);
#ifdef CLUSTER_INSIDES
	// allocate memory for occupation array
	occupied = calloc (tile_map_size_x*tile_map_size_y*6*6, 1);
#endif

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

	//read the tiles map
#ifdef	NEW_FILE_IO
	el_read(f, tile_map_size_x * tile_map_size_y, tile_map);
#else	//NEW_FILE_IO
	fread(tile_map, 1, tile_map_size_x*tile_map_size_y, f);
#endif	//NEW_FILE_IO
#ifdef CLUSTER_INSIDES
	update_occupied_with_tile_map (occupied, tile_map);
#endif

	//load the tiles in this map, if not already loaded
	load_map_tiles();
	init_buffers();
	for(i = 0; i < tile_map_size_y; i++)
	{
		bbox.bbmin[Y] = i*3.0f;
		bbox.bbmax[Y] = (i+1)*3.0f;
		for(j = 0; j < tile_map_size_x; j++)
		{
			cur_tile = tile_map[i*tile_map_size_x+j];
			if (cur_tile != 255)
			{
				bbox.bbmin[X] = j*3.0f;
				bbox.bbmax[X] = (j+1)*3.0f;
				if (IS_WATER_TILE(cur_tile)) 
				{
					bbox.bbmin[Z] = -0.25f;
					bbox.bbmax[Z] = -0.25f;
					if (IS_REFLECTING(cur_tile)) add_water_to_list(main_bbox_tree_items, get_terrain_id(j, i), bbox, cur_tile, 1);
					else add_water_to_list(main_bbox_tree_items, get_terrain_id(j, i), bbox, cur_tile, 0);
				}
				else 
				{
					bbox.bbmin[Z] = 0.0f;
					bbox.bbmax[Z] = 0.0f;
					add_terrain_to_list(main_bbox_tree_items, get_terrain_id(j, i), bbox, cur_tile);
				}
			}
		}
	}

	//read the heights map
#ifdef	NEW_FILE_IO
	el_read(f, tile_map_size_x * tile_map_size_y * 6 * 6, height_map);
#else	//NEW_FILE_IO
	fread(height_map, 1, tile_map_size_x*tile_map_size_y*6*6, f);
#endif	//NEW_FILE_IO
#ifdef CLUSTER_INSIDES
	update_occupied_with_height_map (occupied, height_map);
#endif

	update_function(load_3d_object_str, 0);
	//see which objects in our cache are not used in this map
	//read the 3d objects
	clear_objects_list_placeholders();
	for (i = 0; i < obj_3d_no; i++)
	{
		char * cur_3do_pointer=(char *)&cur_3d_obj_io;
#ifdef	NEW_FILE_IO
		el_read(f, obj_3d_io_size, cur_3do_pointer);
#else	//NEW_FILE_IO
		fread(cur_3do_pointer, 1, obj_3d_io_size, f);
#endif	//NEW_FILE_IO
			
		cur_3d_obj_io.x_pos = SwapLEFloat(cur_3d_obj_io.x_pos);
		cur_3d_obj_io.y_pos = SwapLEFloat(cur_3d_obj_io.y_pos);
		cur_3d_obj_io.z_pos = SwapLEFloat(cur_3d_obj_io.z_pos);
		cur_3d_obj_io.x_rot = SwapLEFloat(cur_3d_obj_io.x_rot);
		cur_3d_obj_io.y_rot = SwapLEFloat(cur_3d_obj_io.y_rot);
		cur_3d_obj_io.z_rot = SwapLEFloat(cur_3d_obj_io.z_rot);
		cur_3d_obj_io.r = SwapLEFloat(cur_3d_obj_io.r);
		cur_3d_obj_io.g = SwapLEFloat(cur_3d_obj_io.g);
		cur_3d_obj_io.b = SwapLEFloat(cur_3d_obj_io.b);

		if (cur_3d_obj_io.blended != 20)
		{
#ifdef CLUSTER_INSIDES
			int id;

			if (cur_3d_obj_io.blended != 1)
				cur_3d_obj_io.blended = 0;
			id = add_e3d (cur_3d_obj_io.file_name, cur_3d_obj_io.x_pos, cur_3d_obj_io.y_pos,
				cur_3d_obj_io.z_pos, cur_3d_obj_io.x_rot, cur_3d_obj_io.y_rot,
				cur_3d_obj_io.z_rot, cur_3d_obj_io.self_lit, cur_3d_obj_io.blended,
				cur_3d_obj_io.r, cur_3d_obj_io.g, cur_3d_obj_io.b, 0);

			update_occupied_with_3d (occupied, id);
#else
			if (cur_3d_obj_io.blended != 1) cur_3d_obj_io.blended = 0;
			add_e3d (cur_3d_obj_io.file_name, cur_3d_obj_io.x_pos, cur_3d_obj_io.y_pos,
				cur_3d_obj_io.z_pos, cur_3d_obj_io.x_rot, cur_3d_obj_io.y_rot,
				cur_3d_obj_io.z_rot, cur_3d_obj_io.self_lit, cur_3d_obj_io.blended,
				cur_3d_obj_io.r, cur_3d_obj_io.g, cur_3d_obj_io.b, 0);			
#endif
		}
		else inc_objects_list_placeholders();

		if (i % 100 == 0)
		{
			update_function(NULL, 0);
		}
	}

	//delete the unused objects from the cache

	update_function(load_2d_object_str, 20);
	//read the 2d objects
	for (i = 0; i < obj_2d_no; i++)
	{
#ifdef CLUSTER_INSIDES
		int id;
#endif
		char * cur_2do_pointer=(char *)&cur_2d_obj_io;

#ifdef	NEW_FILE_IO
		el_read(f, obj_2d_io_size, cur_2do_pointer);
#else	//NEW_FILE_IO
		fread(cur_2do_pointer, 1, obj_2d_io_size, f);
#endif	//NEW_FILE_IO
			
		cur_2d_obj_io.x_pos = SwapLEFloat(cur_2d_obj_io.x_pos);
		cur_2d_obj_io.y_pos = SwapLEFloat(cur_2d_obj_io.y_pos);
		cur_2d_obj_io.z_pos = SwapLEFloat(cur_2d_obj_io.z_pos);
		cur_2d_obj_io.x_rot = SwapLEFloat(cur_2d_obj_io.x_rot);
		cur_2d_obj_io.y_rot = SwapLEFloat(cur_2d_obj_io.y_rot);
		cur_2d_obj_io.z_rot = SwapLEFloat(cur_2d_obj_io.z_rot);

#ifndef SHOW_FLICKERING
		// Add in low-order bits to prevent flicker.
		cur_2d_obj_io.z_pos += offset_2d;
		offset_2d += offset_2d_increment;
		if (offset_2d >= offset_2d_max)
			offset_2d = offset_2d_increment;
#endif
			
#ifdef CLUSTER_INSIDES
		id = add_2d_obj (cur_2d_obj_io.file_name, cur_2d_obj_io.x_pos, cur_2d_obj_io.y_pos,
			cur_2d_obj_io.z_pos, cur_2d_obj_io.x_rot, cur_2d_obj_io.y_rot,
			cur_2d_obj_io.z_rot, 0);
		update_occupied_with_2d (occupied, id);
#else
		add_2d_obj(cur_2d_obj_io.file_name, cur_2d_obj_io.x_pos, cur_2d_obj_io.y_pos,
			cur_2d_obj_io.z_pos, cur_2d_obj_io.x_rot, cur_2d_obj_io.y_rot,
			cur_2d_obj_io.z_rot, 0);
#endif

		if (i % 100 == 0)
		{
			update_function(NULL, 0);
		}
	}

	update_function(load_lights_str, 20);
	//read the lights
	for (i = 0; i < lights_no; i++)
	{
#ifdef CLUSTER_INSIDES
		int id;
#endif
		char * cur_light_pointer=(char *)&cur_light_io;
#ifdef	NEW_FILE_IO
		el_read(f, lights_io_size, cur_light_pointer);
#else	//NEW_FILE_IO
		fread(cur_light_pointer, 1, lights_io_size, f);
#endif	//NEW_FILE_IO
			
		cur_light_io.pos_x = SwapLEFloat(cur_light_io.pos_x);
		cur_light_io.pos_y = SwapLEFloat(cur_light_io.pos_y);
		cur_light_io.pos_z = SwapLEFloat(cur_light_io.pos_z);
		cur_light_io.r = SwapLEFloat(cur_light_io.r);
		cur_light_io.g = SwapLEFloat(cur_light_io.g);
		cur_light_io.b = SwapLEFloat(cur_light_io.b);

		if (cur_light_io.pos_x < 0.0f || cur_light_io.pos_x > tile_map_size_x * 60 ||
			cur_light_io.pos_y < 0.0f || cur_light_io.pos_y > tile_map_size_y * 60 ||
			cur_light_io.pos_z < -1000.0f || cur_light_io.pos_z > 1000.0f ||
			cur_light_io.r < -1.0f || cur_light_io.r > 1000.0f ||
			cur_light_io.g < -1.0f || cur_light_io.g > 1000.0f ||
			cur_light_io.b < -1.0f || cur_light_io.b > 1000.0f)
		{
			LOG_ERROR("Bad light (number %d) when loading '%s'; co-ords [%f %f %f] "
				"colour [%f %f %f]", i, file_name, cur_light_io.pos_x,
				cur_light_io.pos_y, cur_light_io.pos_z, cur_light_io.r,
				cur_light_io.g, cur_light_io.b);
			cur_light_io.pos_x = cur_light_io.pos_y = 1.0f;
			cur_light_io.pos_z = 2.0f;
			cur_light_io.r = cur_light_io.g = cur_light_io.b = 1.0f;
			continue;
		}

#ifdef CLUSTER_INSIDES
		id = add_light (cur_light_io.pos_x, cur_light_io.pos_y, cur_light_io.pos_z,
			cur_light_io.r, cur_light_io.g, cur_light_io.b, 1.0f, 0);
		update_occupied_with_light (occupied, id);
#else
		add_light(cur_light_io.pos_x, cur_light_io.pos_y, cur_light_io.pos_z,
			cur_light_io.r, cur_light_io.g, cur_light_io.b, 1.0f, 0);
#endif

		if (i % 100 == 0)
		{
			update_function(NULL, 0);
		}
	}

	update_function(load_particles_str, 20);
	//read particle systems
	for (i = 0; i < particles_no; i++)
	{
		char *cur_particles_pointer=(char *)&cur_particles_io;
#ifdef	NEW_FILE_IO
		el_read(f, particles_io_size, cur_particles_pointer);
#else	//NEW_FILE_IO
		fread(cur_particles_pointer,1,particles_io_size,f);
#endif	//NEW_FILE_IO
			
		cur_particles_io.x_pos = SwapLEFloat(cur_particles_io.x_pos);
		cur_particles_io.y_pos = SwapLEFloat(cur_particles_io.y_pos);
		cur_particles_io.z_pos = SwapLEFloat(cur_particles_io.z_pos);
			
#ifdef EYE_CANDY
		if (!strncmp(cur_particles_io.file_name, "ec://", 5))
		{
			ec_create_effect_from_map_code(cur_particles_io.file_name + 5, cur_particles_io.x_pos, cur_particles_io.y_pos, cur_particles_io.z_pos, (poor_man ? 6 : 10));
		}
		else
		{
#endif
#ifdef CLUSTER_INSIDES
			int id = add_particle_sys (cur_particles_io.file_name, cur_particles_io.x_pos, cur_particles_io.y_pos, cur_particles_io.z_pos, 0);
			/* It seems highly unlikely to me that a particle
			 * system will be placed on an otherwise empty
			 * tile, so updating the occupied array with them
			 * is disabled for now
			 */
			(void) id; // shut up compiler warnings
			//update_occupied_with_particle_system (occupied, id);
#else
			add_particle_sys (cur_particles_io.file_name, cur_particles_io.x_pos, cur_particles_io.y_pos, cur_particles_io.z_pos, 0);
#endif // CLUSTER_INSIDES
#ifdef EYE_CANDY
		}
#endif
		if (i % 100 == 0)
		{
			update_function(NULL, 0);
		}
	}
	
#ifdef	NEW_FILE_IO
	el_close(f);
#else	//NEW_FILE_IO
	fclose(f);
#endif	//NEW_FILE_IO

#ifdef CLUSTER_INSIDES
	compute_clusters (occupied);
	free (occupied);

	// Ok, we have the clusters, now assign new IDs to each object 
	// that we added.
	for (i = 0; i < MAX_OBJ_3D; i++)
	{
		if (objects_list[i])
		{
			int x = (int) (objects_list[i]->x_pos / 0.5f);
			int y = (int) (objects_list[i]->y_pos / 0.5f);
			objects_list[i]->cluster = get_cluster (x, y);
		}
	}

	for (i = 0; i < MAX_OBJ_2D; i++)
	{
		if (obj_2d_list[i])
		{
			int x = (int) (obj_2d_list[i]->x_pos / 0.5f);
			int y = (int) (obj_2d_list[i]->y_pos / 0.5f);
			obj_2d_list[i]->cluster = get_cluster (x, y);
		}
	}

	for (i = 0; i < MAX_LIGHTS; i++)
	{
		if (lights_list[i])
		{
			int x = (int) (lights_list[i]->pos_x / 0.5f);
			int y = (int) (lights_list[i]->pos_y / 0.5f);
			lights_list[i]->cluster = get_cluster (x, y);
		}
	}

	for (i = 0;  i< MAX_PARTICLE_SYSTEMS; i++)
	{
		if (particles_list[i])
		{
			int x = (int) (particles_list[i]->x_pos / 0.5f);
			int y = (int) (particles_list[i]->y_pos / 0.5f);
			particles_list[i]->cluster = get_cluster (x, y);
		}
	}
#endif

	update_function(bld_sectors_str, 20);
	init_bbox_tree(main_bbox_tree, main_bbox_tree_items);
	free_bbox_items(main_bbox_tree_items);
	main_bbox_tree_items = NULL;
	update_function(init_done_str, 20);
#ifdef EXTRA_DEBUG
	ERR();//We finished loading the new map apparently...
#endif
	return 1;
}

