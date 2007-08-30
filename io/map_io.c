#include "map_io.h"
#include "../2d_objects.h"
#include "../3d_objects.h"
#include "../asc.h"
#include "../bbox_tree.h"
#include "../errors.h"
#include "../init.h"
#include "../lights.h"
#include "../map.h"
#include "../particles.h"
#include "../reflection.h"
#include "../tiles.h"
#include "../translate.h"
#ifdef	NEW_FILE_IO
#include "elfilewrapper.h"
#else
#include <sys/stat.h>
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
	int file_size;
	char* file_mem;
#ifndef NEW_FILE_IO
	struct stat file_stat;
#endif
#ifdef CLUSTER_INSIDES
	char* occupied = NULL;
	int have_clusters;
#endif
	object3d_io* objs_3d;
	obj_2d_io* objs_2d;
	light_io* lights;
	particles_io* particles;

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
#ifdef NEW_FILE_IO
	file_size = el_get_size (f);
#else
	fstat (fileno (f), &file_stat);
	file_size = file_stat.st_size;
#endif
#ifdef EXTRA_DEBUG
	ERR();
#endif
	file_mem = calloc (file_size, 1);
	if (!file_mem)
		return 0;

	my_strcp (map_file_name, file_name);

	// read the entire file into memory
#ifdef	NEW_FILE_IO
	el_read (f, file_size, file_mem);
	el_close (f);
#else	//NEW_FILE_IO
	fread (file_mem, 1, file_size, f);
	fclose (f);
#endif	//NEW_FILE_IO

	main_bbox_tree_items = create_bbox_items(1024);

	memcpy (&cur_map_header, file_mem, sizeof (cur_map_header));
	cur_map_header.tile_map_x_len = SDL_SwapLE32(cur_map_header.tile_map_x_len);
	cur_map_header.tile_map_y_len = SDL_SwapLE32(cur_map_header.tile_map_y_len);
	cur_map_header.tile_map_offset = SDL_SwapLE32(cur_map_header.tile_map_offset);
	cur_map_header.height_map_offset = SDL_SwapLE32(cur_map_header.height_map_offset);
	cur_map_header.obj_3d_struct_len = SDL_SwapLE32(cur_map_header.obj_3d_struct_len);
	cur_map_header.obj_3d_no = SDL_SwapLE32 (cur_map_header.obj_3d_no);
	cur_map_header.obj_3d_offset = SDL_SwapLE32(cur_map_header.obj_3d_offset);
	cur_map_header.obj_2d_struct_len = SDL_SwapLE32(cur_map_header.obj_2d_struct_len);
	cur_map_header.obj_2d_no = SDL_SwapLE32 (cur_map_header.obj_2d_no);
	cur_map_header.obj_2d_offset = SDL_SwapLE32(cur_map_header.obj_2d_offset);
	cur_map_header.lights_struct_len = SDL_SwapLE32(cur_map_header.lights_struct_len);
	cur_map_header.lights_no = SDL_SwapLE32 (cur_map_header.lights_no);
	cur_map_header.lights_offset = SDL_SwapLE32(cur_map_header.lights_offset);
    
	cur_map_header.ambient_r = SwapLEFloat(cur_map_header.ambient_r);
	cur_map_header.ambient_g = SwapLEFloat(cur_map_header.ambient_g);
	cur_map_header.ambient_b = SwapLEFloat(cur_map_header.ambient_b);
	
	cur_map_header.particles_struct_len = SDL_SwapLE32(cur_map_header.particles_struct_len);
	cur_map_header.particles_no = SDL_SwapLE32 (cur_map_header.particles_no);
	cur_map_header.particles_offset = SDL_SwapLE32(cur_map_header.particles_offset);

#ifdef CLUSTER_INSIDES
	cur_map_header.clusters_offset = SDL_SwapLE32 (cur_map_header.clusters_offset);
#endif 
	
	//verify if we have a valid file
	if(cur_map_header.file_sig[0]!='e'||
	   cur_map_header.file_sig[1]!='l'||
	   cur_map_header.file_sig[2]!='m'||
	   cur_map_header.file_sig[3]!='f')
	{
		log_error (invalid_map, map_file_name);
		exit_now = 1; // We might as well quit...
		free (file_mem);
		return 0;
	}

	// Check the sizes of structures. If they don't match, we have a
	// major problem, since these structures are supposed to be 
	// written flat out to disk.
	if (cur_map_header.obj_3d_struct_len < sizeof (object3d_io)
	    || cur_map_header.obj_2d_struct_len != sizeof (obj_2d_io)
	    || cur_map_header.lights_struct_len != sizeof (light_io)
	    || cur_map_header.particles_struct_len != sizeof (particles_io))
	{
		LOG_ERROR ("Invalid object size on map %s", map_file_name);
		exit_now = 1; // We might as well quit...
		free (file_mem);
		return 0;
	}

	update_function (load_map_str, 0);

	//get the map size
	tile_map_size_x = cur_map_header.tile_map_x_len;
	tile_map_size_y = cur_map_header.tile_map_y_len;

	// allocate the tile map (it was destroyed), and fill it
	tile_map = calloc (tile_map_size_x*tile_map_size_y, 1);
	memcpy (tile_map, file_mem + cur_map_header.tile_map_offset, tile_map_size_x*tile_map_size_y);
	// allocate the height map, and fill it
	height_map = calloc (tile_map_size_x*tile_map_size_y*6*6, 1);
	memcpy (height_map, file_mem + cur_map_header.height_map_offset, tile_map_size_x*tile_map_size_y*6*6);

#ifdef CLUSTER_INSIDES
	// check if we need to compute the clusters, or if they're stored
	// in the map
	have_clusters = cur_map_header.clusters_offset > 0 
	                && cur_map_header.clusters_offset + tile_map_size_x*tile_map_size_y*6*6 * sizeof (short) <= file_size;
	if (have_clusters)
	{
		// clusters are stored in the map, set them
		set_clusters (file_mem + cur_map_header.clusters_offset);
	}
	else
	{
		// We need to compute the clusters, allocate memory for 
		// the occupation array, and initialize it with the
		// tile and height maps
		occupied = calloc (tile_map_size_x*tile_map_size_y*6*6, 1);
		update_occupied_with_tile_map (occupied, tile_map);
		update_occupied_with_height_map (occupied, height_map);
	}
#endif

	//get the type of map, and the ambient light
	dungeon = cur_map_header.dungeon;
	ambient_r = cur_map_header.ambient_r;
	ambient_g = cur_map_header.ambient_g;
	ambient_b = cur_map_header.ambient_b;

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

	update_function(load_3d_object_str, 0);
	//see which objects in our cache are not used in this map
	//read the 3d objects
	clear_objects_list_placeholders();
	objs_3d = (object3d_io*) (file_mem + cur_map_header.obj_3d_offset);
	for (i = 0; i < cur_map_header.obj_3d_no; i++)
	{
		object3d_io cur_3d_obj_io = objs_3d[i];
			
		cur_3d_obj_io.x_pos = SwapLEFloat (cur_3d_obj_io.x_pos);
		cur_3d_obj_io.y_pos = SwapLEFloat (cur_3d_obj_io.y_pos);
		cur_3d_obj_io.z_pos = SwapLEFloat (cur_3d_obj_io.z_pos);
		cur_3d_obj_io.x_rot = SwapLEFloat (cur_3d_obj_io.x_rot);
		cur_3d_obj_io.y_rot = SwapLEFloat (cur_3d_obj_io.y_rot);
		cur_3d_obj_io.z_rot = SwapLEFloat (cur_3d_obj_io.z_rot);
		cur_3d_obj_io.r = SwapLEFloat (cur_3d_obj_io.r);
		cur_3d_obj_io.g = SwapLEFloat (cur_3d_obj_io.g);
		cur_3d_obj_io.b = SwapLEFloat (cur_3d_obj_io.b);

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

			if (!have_clusters)
				update_occupied_with_3d (occupied, id);
#else
			if (cur_3d_obj_io.blended != 1) cur_3d_obj_io.blended = 0;
			add_e3d (cur_3d_obj_io.file_name, cur_3d_obj_io.x_pos, cur_3d_obj_io.y_pos,
				cur_3d_obj_io.z_pos, cur_3d_obj_io.x_rot, cur_3d_obj_io.y_rot,
				cur_3d_obj_io.z_rot, cur_3d_obj_io.self_lit, cur_3d_obj_io.blended,
				cur_3d_obj_io.r, cur_3d_obj_io.g, cur_3d_obj_io.b, 0);			
#endif
		}
		else
		{
			inc_objects_list_placeholders();
		}

		if (i % 100 == 0)
		{
			update_function(NULL, 0);
		}
	}

	update_function(load_2d_object_str, 20);
	//read the 2d objects
	objs_2d = (obj_2d_io*) (file_mem + cur_map_header.obj_2d_offset);
	for (i = 0; i < cur_map_header.obj_2d_no; i++)
	{
		obj_2d_io cur_2d_obj_io = objs_2d[i];
#ifdef CLUSTER_INSIDES
		int id;
#endif

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
		if (!have_clusters)
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

#ifdef CLUSTER_INSIDES
	// If we need to compute the clusters, do it here, so that the
	// newly added lights and particle systems get the right cluster
	// number automagically. We don't update the occupation map with
	// lights or particle systems since I don't expect them to bridge
	// any clusters, and if they do happen to hang in the void, 
	// they'll be shown anyway.
	if (!have_clusters)
	{
		compute_clusters (occupied);
		free (occupied);

		// Ok, we have the clusters, now assign new IDs to each 
		// object that we added.
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
	}
#endif

	update_function(load_lights_str, 20);
	//read the lights
	lights = (light_io *) (file_mem + cur_map_header.lights_offset);
	for (i = 0; i < cur_map_header.lights_no; i++)
	{
		light_io cur_light_io = lights[i];
			
		cur_light_io.pos_x = SwapLEFloat (cur_light_io.pos_x);
		cur_light_io.pos_y = SwapLEFloat (cur_light_io.pos_y);
		cur_light_io.pos_z = SwapLEFloat (cur_light_io.pos_z);
		cur_light_io.r = SwapLEFloat (cur_light_io.r);
		cur_light_io.g = SwapLEFloat (cur_light_io.g);
		cur_light_io.b = SwapLEFloat (cur_light_io.b);

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

		add_light (cur_light_io.pos_x, cur_light_io.pos_y, cur_light_io.pos_z,
		           cur_light_io.r, cur_light_io.g, cur_light_io.b, 1.0f, 0);

		if (i % 100 == 0)
		{
			update_function(NULL, 0);
		}
	}

	update_function(load_particles_str, 20);
	//read particle systems
	particles = (particles_io *) (file_mem + cur_map_header.particles_offset);
	for (i = 0; i < cur_map_header.particles_no; i++)
	{
		particles_io cur_particles_io = particles[i];
			
		cur_particles_io.x_pos = SwapLEFloat (cur_particles_io.x_pos);
		cur_particles_io.y_pos = SwapLEFloat (cur_particles_io.y_pos);
		cur_particles_io.z_pos = SwapLEFloat (cur_particles_io.z_pos);
			
#ifdef EYE_CANDY
		if (!strncmp(cur_particles_io.file_name, "ec://", 5))
		{
			ec_create_effect_from_map_code(cur_particles_io.file_name + 5, cur_particles_io.x_pos, cur_particles_io.y_pos, cur_particles_io.z_pos, (poor_man ? 6 : 10));
		}
		else
		{
#endif
			add_particle_sys (cur_particles_io.file_name, cur_particles_io.x_pos, cur_particles_io.y_pos, cur_particles_io.z_pos, 0);
#ifdef EYE_CANDY
		}
#endif
		if (i % 100 == 0)
		{
			update_function(NULL, 0);
		}
	}

	// Everything copied, get rid of the file data
	free (file_mem);
	
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

