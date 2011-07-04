#include "map_io.h"
#include "../2d_objects.h"
#include "../3d_objects.h"
#include "../asc.h"
#include "../bbox_tree.h"
#include "../elconfig.h"
#include "../errors.h"
#include "../init.h"
#include "../lights.h"
#include "../map.h"
#include "../particles.h"
#include "../reflection.h"
#include "../tiles.h"
#include "../translate.h"
#include "elfilewrapper.h"
 #include "../eye_candy_wrapper.h"
#ifdef CLUSTER_INSIDES
#include "../cluster.h"
#endif

#ifndef SHOW_FLICKERING
const float offset_2d_increment = (1.0f / 32768.0f);	// (1.0f / 8388608.0f) is the minimum for 32-bit floating point.
float offset_2d = (1.0f / 32768.0f);
const float offset_2d_max = 0.01f;
#endif

int get_tile_map_sizes(const char *file_name, int *x, int *y)
{
	map_header cur_map_header;
	char* file_mem;
	el_file_ptr file;

	file = el_open(file_name);

	if (!file)
	{
		return 0;
	}

	file_mem = el_get_pointer(file);

	memcpy(&cur_map_header, file_mem, sizeof(cur_map_header));
	*x = SDL_SwapLE32(cur_map_header.tile_map_x_len);
	*y = SDL_SwapLE32(cur_map_header.tile_map_y_len);
	el_close(file);
	return 1;
}

static int do_load_map(const char *file_name, update_func *update_function)
{
	int i;
	int cur_tile, j;
	AABBOX bbox;
	map_header cur_map_header;
	char* file_mem;
#ifdef CLUSTER_INSIDES
	char* occupied = 0;
	int have_clusters;
#endif
	object3d_io* objs_3d;
	obj_2d_io* objs_2d;
	light_io* lights;
	particles_io* particles;
#ifndef FASTER_MAP_LOAD
	float progress;
#endif
	el_file_ptr file;

	file = el_open(file_name);

	if (!file)
	{
		return 0;
	}

#ifdef EXTRA_DEBUG
	ERR();
#endif
	file_mem = el_get_pointer(file);

	my_strcp(map_file_name, file_name);

	LOG_DEBUG("Loading map '%s'.", file_name);

	main_bbox_tree_items = create_bbox_items(1024);

	memcpy(&cur_map_header, file_mem, sizeof(cur_map_header));
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

#ifdef CLUSTER_INSIDES
	cur_map_header.clusters_offset = SDL_SwapLE32(cur_map_header.clusters_offset);
#endif

	LOG_DEBUG("Checking map '%s' file signature.", file_name);

	//verify if we have a valid file
	if(cur_map_header.file_sig[0]!='e'||
	   cur_map_header.file_sig[1]!='l'||
	   cur_map_header.file_sig[2]!='m'||
	   cur_map_header.file_sig[3]!='f')
	{
		LOG_ERROR(invalid_map, map_file_name);
		exit_now = 1; // We might as well quit...
		el_close(file);

		return 0;
	}

	LOG_DEBUG("Checking map '%s' sizes.", file_name);

	// Check the sizes of structures. If they don't match, we have a
	// major problem, since these structures are supposed to be
	// written flat out to disk.
	if (cur_map_header.obj_3d_struct_len != sizeof (object3d_io)
	    || cur_map_header.obj_2d_struct_len != sizeof (obj_2d_io)
	    || cur_map_header.lights_struct_len != sizeof (light_io)
	    || (cur_map_header.particles_struct_len != sizeof (particles_io) && cur_map_header.particles_no > 0)
	   )
	{
		LOG_ERROR ("Invalid object size on map %s", map_file_name);
		exit_now = 1; // We might as well quit...
		el_close(file);

		return 0;
	}

	update_function(load_map_str, 0);

	//get the map size
	tile_map_size_x = cur_map_header.tile_map_x_len;
	tile_map_size_y = cur_map_header.tile_map_y_len;

	LOG_DEBUG("Map '%s' size <%d, %d>.", file_name, tile_map_size_x,
		tile_map_size_y);

	// allocate the tile map (it was destroyed), and fill it
	tile_map = calloc (tile_map_size_x*tile_map_size_y, 1);
	memcpy(tile_map, file_mem + cur_map_header.tile_map_offset, tile_map_size_x*tile_map_size_y);

	// allocate the height map, and fill it
	height_map = calloc (tile_map_size_x*tile_map_size_y*6*6, 1);
	memcpy(height_map, file_mem + cur_map_header.height_map_offset, tile_map_size_x*tile_map_size_y*6*6);

#ifdef CLUSTER_INSIDES
	// check if we need to compute the clusters, or if they're stored
	// in the map
	have_clusters = (cur_map_header.clusters_offset > 0)
	                && ((cur_map_header.clusters_offset +
			tile_map_size_x * tile_map_size_y * 6 * 6 *
			sizeof(short)) <= el_get_size(file));

	LOG_DEBUG("Map '%s' has clusters: %d.", file_name, have_clusters);

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

	LOG_DEBUG("Map '%s' is dungeon %d and ambient <%f, %f, %f>.",
		file_name, cur_map_header.dungeon, cur_map_header.ambient_r,
		cur_map_header.ambient_g, cur_map_header.ambient_b);
	//get the type of map, and the ambient light
	dungeon = cur_map_header.dungeon;
	ambient_r = cur_map_header.ambient_r;
	ambient_g = cur_map_header.ambient_g;
	ambient_b = cur_map_header.ambient_b;

/* 	water_tiles_extension = (tile_map_size_x > tile_map_size_y ? */
/* 							 tile_map_size_x : tile_map_size_y * 1.5); */
/* 	if (water_tiles_extension < 500.0) */
/* 		water_tiles_extension = 500.0 - water_tiles_extension; */
/* 	else */
/* 		water_tiles_extension = 0.0; */

	LOG_DEBUG("Loading tiles");
	//load the tiles in this map, if not already loaded
	load_map_tiles();

	LOG_DEBUG("Initializing buffers");
	init_buffers();
#ifndef CLUSTER_INSIDES
	for(i = 0; i < tile_map_size_y; i++)
	{
		bbox.bbmin[Y] = i*3.0f;
		bbox.bbmax[Y] = (i+1)*3.0f;
		if (i == 0)
			bbox.bbmin[Y] -= water_tiles_extension;
		else if (i == tile_map_size_y-1)
			bbox.bbmax[Y] += water_tiles_extension;
		for(j = 0; j < tile_map_size_x; j++)
		{
			cur_tile = tile_map[i*tile_map_size_x+j];
			if (cur_tile != 255)
			{
				bbox.bbmin[X] = j*3.0f;
				bbox.bbmax[X] = (j+1)*3.0f;
				if (j == 0)
					bbox.bbmin[X] -= water_tiles_extension;
				else if (j == tile_map_size_x-1)
					bbox.bbmax[X] += water_tiles_extension;
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
#endif // CLUSTER_INSIDES

#ifdef FASTER_MAP_LOAD
	update_function(load_3d_object_str, 20.0f);
#else  // FASTER_MAP_LOAD
	progress = (cur_map_header.obj_3d_no + 249) / 250;
	if (progress > 0.0f)
	{
		update_function(load_3d_object_str, 0.0f);
		progress = 20.0f / progress;
	}
	else
	{
		update_function(load_3d_object_str, 20.0f);
		progress = 0.0f;
	}
#endif // FASTER_MAP_LOAD

	LOG_DEBUG("Loading %d 3d objects.", cur_map_header.obj_3d_no);

	//read the 3d objects
#ifndef FASTER_MAP_LOAD
	clear_objects_list_placeholders();
#endif
	objs_3d = (object3d_io*) (file_mem + cur_map_header.obj_3d_offset);

	ENTER_DEBUG_MARK("load 3d objects");
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

		LOG_DEBUG("Adding 3d object (%d) '%s' at <%d, %f, %f> with "
			"rotation <%f, %f, %f>, left lit %d, blended %d and "
			"color <%f, %f, %f>.", i, cur_3d_obj_io.file_name,
			cur_3d_obj_io.x_pos, cur_3d_obj_io.y_pos,
			cur_3d_obj_io.z_pos, cur_3d_obj_io.x_rot,
			cur_3d_obj_io.y_rot, cur_3d_obj_io.z_rot,
			cur_3d_obj_io.self_lit, cur_3d_obj_io.blended,
			cur_3d_obj_io.r, cur_3d_obj_io.g, cur_3d_obj_io.b);

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

#ifndef FASTER_MAP_LOAD
		if (i % 250 == 0)
		{
			update_function(load_3d_object_str, progress);
		}
#endif
	}
	LEAVE_DEBUG_MARK("load 3d objects");

#ifdef FASTER_MAP_LOAD
	update_function(load_2d_object_str, 20.0f);
#else  // FASTER_MAP_LOAD
	progress = (cur_map_header.obj_2d_no + 249) / 250;
	if (progress > 0)
	{
		update_function(load_2d_object_str, 0.0f);
		progress = 20.0f / progress;
	}
	else
	{
		update_function(load_2d_object_str, 20.0f);
		progress = 0.0f;
	}
#endif // FASTER_MAP_LOAD

	LOG_DEBUG("Loading %d 2d objects.", cur_map_header.obj_2d_no);

	//read the 2d objects
	objs_2d = (obj_2d_io*) (file_mem + cur_map_header.obj_2d_offset);

	ENTER_DEBUG_MARK("load 2d objects");
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

		LOG_DEBUG("Adding 2d object (%d) '%s' at <%d, %f, %f> with "
			"rotation <%f, %f, %f>.", i, cur_2d_obj_io.file_name,
			cur_2d_obj_io.x_pos, cur_2d_obj_io.y_pos,
			cur_2d_obj_io.z_pos, cur_2d_obj_io.x_rot,
			cur_2d_obj_io.y_rot, cur_2d_obj_io.z_rot);

#ifndef SHOW_FLICKERING
		// Add in low-order bits to prevent flicker.
		cur_2d_obj_io.z_pos += offset_2d;
		offset_2d += offset_2d_increment;
		if (offset_2d >= offset_2d_max)
			offset_2d = offset_2d_increment;
#endif

#ifdef FASTER_MAP_LOAD
#ifdef CLUSTER_INSIDES
		id = add_2d_obj(i, cur_2d_obj_io.file_name, cur_2d_obj_io.x_pos, cur_2d_obj_io.y_pos,
			cur_2d_obj_io.z_pos, cur_2d_obj_io.x_rot, cur_2d_obj_io.y_rot,
			cur_2d_obj_io.z_rot, 0);
		if (!have_clusters)
			update_occupied_with_2d (occupied, id);
#else  // CLUSTER_INSIDES
		add_2d_obj(i, cur_2d_obj_io.file_name, cur_2d_obj_io.x_pos, cur_2d_obj_io.y_pos,
			cur_2d_obj_io.z_pos, cur_2d_obj_io.x_rot, cur_2d_obj_io.y_rot,
			cur_2d_obj_io.z_rot, 0);
#endif // CLUSTER_INSIDES
#else  // FASTER_MAP_LOAD
#ifdef CLUSTER_INSIDES
		id = add_2d_obj (cur_2d_obj_io.file_name, cur_2d_obj_io.x_pos, cur_2d_obj_io.y_pos,
			cur_2d_obj_io.z_pos, cur_2d_obj_io.x_rot, cur_2d_obj_io.y_rot,
			cur_2d_obj_io.z_rot, 0);
		if (!have_clusters)
			update_occupied_with_2d (occupied, id);
#else  // CLUSTER_INSIDES
		add_2d_obj(cur_2d_obj_io.file_name, cur_2d_obj_io.x_pos, cur_2d_obj_io.y_pos,
			cur_2d_obj_io.z_pos, cur_2d_obj_io.x_rot, cur_2d_obj_io.y_rot,
			cur_2d_obj_io.z_rot, 0);
#endif // CLUSTER_INSIDES

		if (i % 250 == 0)
		{
			update_function(load_2d_object_str, progress);
		}
#endif // FASTER_MAP_LOAD
	}
	LEAVE_DEBUG_MARK("load 2d objects");

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

	// we finally add the tiles to the abt
	for(i = 0; i < tile_map_size_y; i++)
	{
		bbox.bbmin[Y] = i*3.0f;
		bbox.bbmax[Y] = (i+1)*3.0f;
		if (i == 0)
			bbox.bbmin[Y] -= water_tiles_extension;
		else if (i == tile_map_size_y-1)
			bbox.bbmax[Y] += water_tiles_extension;
		for(j = 0; j < tile_map_size_x; j++)
		{
			current_cluster = get_cluster(j*6, i*6);
			cur_tile = tile_map[i*tile_map_size_x+j];
			if (cur_tile != 255)
			{
				bbox.bbmin[X] = j*3.0f;
				bbox.bbmax[X] = (j+1)*3.0f;
				if (j == 0)
					bbox.bbmin[X] -= water_tiles_extension;
				else if (j == tile_map_size_x-1)
					bbox.bbmax[X] += water_tiles_extension;
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
#endif

#ifdef FASTER_MAP_LOAD
	update_function(load_lights_str, 20.0f);
#else  // FASTER_MAP_LOAD
	progress = (cur_map_header.lights_no + 99) / 100;
	if (progress > 0)
	{
		update_function(load_lights_str, 0.0f);
		progress = 20.0f / progress;
	}
	else
	{
		update_function(load_lights_str, 20.0f);
		progress = 0.0f;
	}
#endif // FASTER_MAP_LOAD

	LOG_DEBUG("Loading %d lights.", cur_map_header.lights_no);

	//read the lights
	lights = (light_io *) (file_mem + cur_map_header.lights_offset);

	ENTER_DEBUG_MARK("load lights");
	for (i = 0; i < cur_map_header.lights_no; i++)
	{
		light_io cur_light_io = lights[i];

		cur_light_io.pos_x = SwapLEFloat (cur_light_io.pos_x);
		cur_light_io.pos_y = SwapLEFloat (cur_light_io.pos_y);
		cur_light_io.pos_z = SwapLEFloat (cur_light_io.pos_z);
		cur_light_io.r = SwapLEFloat (cur_light_io.r);
		cur_light_io.g = SwapLEFloat (cur_light_io.g);
		cur_light_io.b = SwapLEFloat (cur_light_io.b);

		LOG_DEBUG("Adding light(%d) at <%d, %f, %f> with color "
			"<%f, %f, %f>.", i, cur_light_io.pos_x,
			cur_light_io.pos_y, cur_light_io.pos_z, cur_light_io.r,
			cur_light_io.g, cur_light_io.b);

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

#ifndef FASTER_MAP_LOAD
		if (i % 100 == 0)
		{
			update_function(load_lights_str, progress);
		}
#endif
	}
	LEAVE_DEBUG_MARK("load lights");

#ifdef FASTER_MAP_LOAD
	update_function(load_particles_str, 20.0f);
#else  // FASTER_MAP_LOAD
	progress = (cur_map_header.particles_no + 99) / 100;
	if (progress > 0.0f)
	{
		update_function(load_particles_str, 0.0f);
		progress = 20.0f / progress;
	}
	else
	{
		update_function(load_particles_str, 20.0f);
		progress = 0.0f;
	}
#endif // FASTER_MAP_LOAD

	LOG_DEBUG("Loading %d particles.", cur_map_header.particles_no);

	//read particle systems
	particles = (particles_io *) (file_mem + cur_map_header.particles_offset);

	ENTER_DEBUG_MARK("load particles");
	for (i = 0; i < cur_map_header.particles_no; i++)
	{
		particles_io cur_particles_io = particles[i];

		cur_particles_io.x_pos = SwapLEFloat (cur_particles_io.x_pos);
		cur_particles_io.y_pos = SwapLEFloat (cur_particles_io.y_pos);
		cur_particles_io.z_pos = SwapLEFloat (cur_particles_io.z_pos);

		LOG_DEBUG("Adding particle(%d) '%s' at <%d, %f, %f>.", i,
			cur_particles_io.file_name, cur_particles_io.x_pos,
			cur_particles_io.y_pos, cur_particles_io.z_pos);

		if (!strncmp(cur_particles_io.file_name, "ec://", 5))
		{
			ec_create_effect_from_map_code(cur_particles_io.file_name + 5, cur_particles_io.x_pos, cur_particles_io.y_pos, cur_particles_io.z_pos, (poor_man ? 6 : 10));
		}
		else
		{
#ifdef NEW_SOUND
			add_map_particle_sys (cur_particles_io.file_name, cur_particles_io.x_pos, cur_particles_io.y_pos, cur_particles_io.z_pos, 0);
#else
			add_particle_sys (cur_particles_io.file_name, cur_particles_io.x_pos, cur_particles_io.y_pos, cur_particles_io.z_pos, 0);
#endif // NEW_SOUND
		}

#ifndef FASTER_MAP_LOAD
		if (i % 100 == 0)
		{
			update_function(load_particles_str, progress);
		}
#endif
	}
	LEAVE_DEBUG_MARK("load particles");

	// Everything copied, get rid of the file data
	el_close(file);

	update_function(bld_sectors_str, 0.0f);

	LOG_DEBUG("Building bbox tree for map '%s'.", file_name);

	init_bbox_tree(main_bbox_tree, main_bbox_tree_items);
	free_bbox_items(main_bbox_tree_items);
	main_bbox_tree_items = 0;
	update_function(init_done_str, 20.0f);
#ifdef EXTRA_DEBUG
	ERR();//We finished loading the new map apparently...
#endif
	return 1;
}

int load_map(const char *file_name, update_func *update_function)
{
	int result;

	ENTER_DEBUG_MARK("load map");

	result = do_load_map(file_name, update_function);

	LEAVE_DEBUG_MARK("load map");

	return result;
}

