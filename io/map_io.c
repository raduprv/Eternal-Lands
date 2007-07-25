#include "map_io.h"
#ifdef	NEW_FILE_IO
#include "elfilewrapper.h"
#endif	//NEW_FILE_IO
#include "../global.h"

#ifdef EYE_CANDY
 #include "../eye_candy_wrapper.h"
#endif

#ifndef SHOW_FLICKERING
const float offset_2d_increment = (1.0f / 32768.0f);	// (1.0f / 8388608.0f) is the minimum for 32-bit floating point.
float offset_2d = (1.0f / 32768.0f);
const float offset_2d_max = 0.01f;
#endif

#ifdef CLUSTER_INSIDES

static short* clusters = NULL;

static __inline__ void update_occupied_with_height_map (char* occupied, const unsigned char* height_map)
{
	int i;

	for (i = 0; i < tile_map_size_x*tile_map_size_y*6*6; i++)
		if (height_map[i]) occupied[i] = 1;
}

static __inline__ void update_occupied_with_tile_map (char* occupied, const unsigned char* tile_map)
{
	int nx = tile_map_size_x * 6;
	int ny = tile_map_size_y * 6;
	int x, y, idx;

	idx = 0;
	for (y = 0; y < ny; y += 6)
	{
		for (x = 0; x < nx; x += 6, idx++)
		{
			if (tile_map[idx] != 255)
			{
				int offset = y*nx + x;
				int i, j;

				for (j = 0; j < 6; j++)
					for (i = 0; i < 6; i++)
						occupied[offset+j*nx+i] = 1;
			}
		}
	}
}

static __inline__ void update_occupied_with_bbox (char* occupied, const AABBOX* box )
{
	int xs = (int) (box->bbmin[X] / 0.5f);
	int ys = (int) (box->bbmin[Y] / 0.5f);
	int xe = (int) (box->bbmax[X] / 0.5f) + 1;
	int ye = (int) (box->bbmax[Y] / 0.5f) + 1;
	int x, y;

	if (xs < 0) xs = 0;
	if (ys < 0) ys = 0;
	if (xe > tile_map_size_x*6) xe = tile_map_size_x*6;
	if (ye > tile_map_size_y*6) ye = tile_map_size_y*6;

	for (y = ys; y < ye; y++)
	{
		for (x = xs; x < xe; x++)
			occupied[y*tile_map_size_x*6+x] = 1;
	}
}

static __inline__ void update_occupied_with_3d (char* occupied, int id)
{
	const e3d_object* obj;
	int i;
	AABBOX box;

	if (id < 0 || id >= MAX_OBJ_3D || !objects_list[id])
		return;

	obj = objects_list[id]->e3d_data;
	if (!obj)
		return;

	for (i = 0; i < obj->material_no; i++)
	{
		box.bbmin[X] = obj->materials[i].min_x;
		box.bbmin[Y] = obj->materials[i].min_y;
		box.bbmin[Z] = obj->materials[i].min_z;
		box.bbmax[X] = obj->materials[i].max_x;
		box.bbmax[Y] = obj->materials[i].max_y;
		box.bbmax[Z] = obj->materials[i].max_z;
		matrix_mul_aabb (&box, objects_list[id]->matrix);

		update_occupied_with_bbox (occupied, &box);
	}
}

static __inline__ void update_occupied_with_2d (char* occupied, int id)
{
	AABBOX box;

	if (get_2d_bbox (id, &box))
		update_occupied_with_bbox (occupied, &box);
}

static __inline__  void update_occupied_with_light (char* occupied, int id)
{
	int x, y;

	if (id < 0 || id >= MAX_LIGHTS)
		return;

	x = (int) (lights_list[id]->pos_x / 0.5f);
	y = (int) (lights_list[id]->pos_y / 0.5f);

	if (x >= 0 && x < tile_map_size_x*6 && y >= 0 && y < tile_map_size_y*6)
		occupied[y*tile_map_size_x*6+x] = 1;
}

static void compute_clusters (const char* occupied) 
{
	int nr_clusters;
	short cluster_idx[1024];
	short nb_idx[4];
	int ic, cnr;

	int nx = tile_map_size_x * 6;
	int ny = tile_map_size_y * 6;
	int x, y, idx;

	clusters = calloc (nx * ny, sizeof (short));
	
	nr_clusters = 0;
	cluster_idx[0] = 0;
	idx = 0;
	if (occupied[idx])
	{
		nr_clusters++;
		clusters[idx] = cluster_idx[nr_clusters] = nr_clusters;
	}

	for (++idx; idx < nx; idx++)
	{
		if (occupied[idx])
		{
			if (occupied[idx-1])
			{
				clusters[idx] = clusters[idx-1];
			}
			else
			{
				nr_clusters++;
				clusters[idx] = cluster_idx[nr_clusters] = nr_clusters;
			}
		}
	}

	for (y = 1; y < ny; y++)
	{
		for (x = 0; x < nx; x++, idx++)
		{
			if (occupied[idx])
			{
				int cidx, i;

				nb_idx[0] = x > 0 ? cluster_idx[clusters[idx-nx-1]] : 0;
				nb_idx[1] = cluster_idx[clusters[idx-nx]];
				nb_idx[2] = x < nx-1 ? cluster_idx[clusters[idx-nx+1]] : 0;
				nb_idx[3] = x > 0 ? cluster_idx[clusters[idx-1]] : 0;

				cidx = 0;
				for (i = 0; i < 4; i++)
					if (nb_idx[i] && (!cidx || nb_idx[i] < cidx))
						cidx = nb_idx[i];

				if (!cidx)
				{
					nr_clusters++;
					clusters[idx] = cluster_idx[nr_clusters] = nr_clusters;
				}
				else
				{
					clusters[idx] = cidx;
					for (i = 0; i < 4; i++)
						if (nb_idx[i])
							cluster_idx[nb_idx[i]] = cidx;
				}
			}
		}
	}

	cnr = 0;
	for (ic = 1; ic < nr_clusters; ic++)
	{
		if (cluster_idx[ic] == ic)
			cluster_idx[ic] = ++cnr;
		else
			cluster_idx[ic] = cluster_idx[cluster_idx[ic]];
	}

	for (idx = 0; idx < nx*ny; idx++)
		clusters[idx] = cluster_idx[clusters[idx]];
}

short get_cluster (int x, int y)
{
	if (clusters == NULL)
		return 0;
	if (x < 0 || x >= tile_map_size_x*6 || y < 0 || y >= tile_map_size_y*6)
		return 0;
	return clusters[y*tile_map_size_x*6+x];
}

short get_actor_cluster ()
{
	actor *me = pf_get_our_actor ();
	return me ? me->cluster : 0;
}

#endif // CLUSTER_INSIDES

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

#ifdef CLUSTER_INSIDES
	// set the clusters array to zero to ensure that all objects are
	// initially added with cluster ID zero.
	if (clusters)
		free (clusters);
	clusters = NULL;
#endif

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
			add_particle_sys (cur_particles_io.file_name, cur_particles_io.x_pos, cur_particles_io.y_pos, cur_particles_io.z_pos, 0);
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
			int x = (int) (objects_list[i]->x_pos / 0.5f);
			int y = (int) (objects_list[i]->y_pos / 0.5f);
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

