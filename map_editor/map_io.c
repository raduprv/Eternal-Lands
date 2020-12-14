#include "global.h"

#ifdef EYE_CANDY
 #include "eye_candy_window.h"
#endif
#ifdef CLUSTER_INSIDES
#include "../cluster.h"
#endif
#include "../asc.h"

#define LATEST_MAP_VERSION 1

void destroy_map()
{
	int i;

	//kill the tile and height map
	if (tile_map)
	{
		free (tile_map);
		tile_map = NULL;
	}

	if (height_map)
	{
		free (height_map);
		height_map = NULL;
	}


	//kill the 3d objects links
	for (i = 0; i < MAX_OBJ_3D; i++)
	{
		if (objects_list[i])
		{
			free (objects_list[i]);
			objects_list[i] = NULL; // kill any reference to it
		}
	}

	//kill the 2d objects links
	for (i = 0; i < MAX_OBJ_2D; i++)
	{
		if (obj_2d_list[i])
		{
			free (obj_2d_list[i]);
			obj_2d_list[i] = NULL; // kill any reference to it
		}
	}

	//kill the lights links
	for (i = 0; i < MAX_LIGHTS; i++)
	{
		if (lights_list[i])
		{
			free (lights_list[i]);
			lights_list[i] = NULL; // kill any reference to it
		}
	}

	destroy_all_particles();
#ifdef EYE_CANDY
	destroy_all_eye_candy();
#endif
	
	selected_3d_object=selected_2d_object=selected_light=selected_particles_object=-1;
}

int save_map (const char* file_name)
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

#ifdef	ZLIBW
	gzFile f = NULL;
#else	//ZLIBW
	FILE *f = NULL;
#endif	//ZLIBW

#ifdef CLUSTER_INSIDES
	char* occupied = NULL;
	char* cluster_data = NULL;
	int cluster_data_len = 0;
#endif

	//get the sizes of structures (they might change in the future)
	obj_3d_io_size=sizeof(object3d_io);
	obj_2d_io_size=sizeof(obj_2d_io);
	lights_io_size=sizeof(light_io);
	particles_io_size=sizeof(particles_io);

	//get the number of objects and lights
	for (i = 0; i < MAX_OBJ_3D; i++)
		if (objects_list[i]) obj_3d_no++;
	for (i = 0; i < MAX_OBJ_2D; i++)
		if (obj_2d_list[i]) obj_2d_no++;
	for (i = 0; i < MAX_LIGHTS; i++)
		if (lights_list[i] && !lights_list[i]->locked) lights_no++;
	// We ignore temporary particle systems (i.e. ones with a ttl)
	for (i = 0; i < MAX_PARTICLE_SYSTEMS; i++)
		if (particles_list[i] && particles_list[i]->def && particles_list[i]->def != &def) particles_no++;

	//ok, now build the header...
	//clear the header
	memset (mem_map_header, 0, sizeof (map_header));

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
#if defined CLUSTER_INSIDES || defined NEW_LIGHT_FORMAT
	cur_map_header.version = LATEST_MAP_VERSION;
#endif
	cur_map_header.ambient_r=ambient_r;
	cur_map_header.ambient_g=ambient_g;
	cur_map_header.ambient_b=ambient_b;
	cur_map_header.particles_struct_len=particles_io_size;
#ifdef EYE_CANDY
        eye_candy_done_adding_effect();
	cur_map_header.particles_no=particles_no + get_eye_candy_count();
#else
	cur_map_header.particles_no=particles_no;
#endif
	cur_map_header.particles_offset=cur_map_header.lights_offset+lights_no*lights_io_size;
#ifdef CLUSTER_INSIDES
	cur_map_header.clusters_offset = cur_map_header.particles_offset + cur_map_header.particles_no * particles_io_size;
#endif

	// ok, now let's open/create the file, and start writing the header...
#ifdef	ZLIBW
	{
		char	gzfile_name[1024];
		strcpy(gzfile_name, file_name);
		//strcat(gzfile_name, ".gz");
		f= gzopen(gzfile_name, "wb");
	}
#else	//ZLIBW
	f= fopen(file_name, "wb");
#endif	//ZLIBW

	if (!f) {
		char msg[500];
		snprintf (msg, sizeof(msg), "Could not open file for writing: %s", file_name);
		LOG_ERROR(msg);
	} else {

#ifdef	ZLIBW
		//write the header
		gzwrite(f, mem_map_header, sizeof(map_header));

		//write the tiles map
		gzwrite(f, tile_map, tile_map_size_x*tile_map_size_y);

		//write the heights map
		gzwrite(f, height_map, tile_map_size_x*tile_map_size_y*6*6);
#else	//ZLIBW
		//write the header
		fwrite(mem_map_header, sizeof(map_header), 1, f);

		//write the tiles map
		fwrite(tile_map, tile_map_size_x*tile_map_size_y, 1, f);

		//write the heights map
		fwrite(height_map, tile_map_size_x*tile_map_size_y*6*6, 1, f);
#endif	//ZLIBW

#ifdef CLUSTER_INSIDES
		// Allocate memory for the occupation map, and initialize
		// it with the tiles and height maps
		occupied = calloc (tile_map_size_x*tile_map_size_y*6*6, 1);
		update_occupied_with_tile_map (occupied, tile_map);
		update_occupied_with_height_map (occupied, height_map);
#endif		

		//write the 3d objects
		j=0;
		for (i = 0; i < MAX_OBJ_3D; i++)
		{
			if (j > obj_3d_no)
				break;

			if (objects_list[i])
			{
				char* cur_3do_pointer = (char *) &cur_3d_obj_io;

				// clear the object
				memset (cur_3do_pointer, 0, sizeof (object3d_io));

				snprintf (cur_3d_obj_io.file_name, sizeof (cur_3d_obj_io.file_name), "%s", objects_list[i]->file_name);
				cur_3d_obj_io.x_pos = objects_list[i]->x_pos;
				cur_3d_obj_io.y_pos = objects_list[i]->y_pos;
				cur_3d_obj_io.z_pos = objects_list[i]->z_pos;

				cur_3d_obj_io.x_rot = objects_list[i]->x_rot;
				cur_3d_obj_io.y_rot = objects_list[i]->y_rot;
				cur_3d_obj_io.z_rot = objects_list[i]->z_rot;

				cur_3d_obj_io.self_lit = objects_list[i]->self_lit;
				cur_3d_obj_io.blended  = objects_list[i]->blended;

				cur_3d_obj_io.r = objects_list[i]->color[0];
				cur_3d_obj_io.g = objects_list[i]->color[1];
				cur_3d_obj_io.b = objects_list[i]->color[2];

#ifdef	ZLIBW
				gzwrite (f, cur_3do_pointer, sizeof(object3d_io));
#else	//ZLIBW
				fwrite (cur_3do_pointer, sizeof(object3d_io), 1, f);
#endif	//ZLIBW

#ifdef CLUSTER_INSIDES
				if (cur_3d_obj_io.blended != 20)
					update_occupied_with_3d (occupied, i);
#endif

				j++;
			}
		}

		//write the 2d objects
		j = 0;
		for (i = 0; i < MAX_OBJ_2D; i++)
		{
			if (j > obj_2d_no) break;
			if (obj_2d_list[i])
			{
				char* cur_2do_pointer = (char *) &cur_2d_obj_io;

				// clear the object
				memset (cur_2do_pointer, 0, sizeof (obj_2d_io));

				snprintf (cur_2d_obj_io.file_name, sizeof (cur_2d_obj_io.file_name), "%s", obj_2d_list[i]->file_name);
				cur_2d_obj_io.x_pos = obj_2d_list[i]->x_pos;
				cur_2d_obj_io.y_pos = obj_2d_list[i]->y_pos;
				cur_2d_obj_io.z_pos = obj_2d_list[i]->z_pos;

				cur_2d_obj_io.x_rot = obj_2d_list[i]->x_rot;
				cur_2d_obj_io.y_rot = obj_2d_list[i]->y_rot;
				cur_2d_obj_io.z_rot = obj_2d_list[i]->z_rot;

#ifdef	ZLIBW
				gzwrite (f, cur_2do_pointer, sizeof (obj_2d_io));
#else	//ZLIBW
				fwrite (cur_2do_pointer, sizeof (obj_2d_io), 1, f);
#endif	//ZLIBW

#ifdef CLUSTER_INSIDES
				update_occupied_with_2d (occupied, i);
#endif

				j++;
			}
		}

		//write the lights
		j=0;
		for (i = 0; i < MAX_LIGHTS; i++)
		{
			if (j > lights_no) break;
			if (lights_list[i] && !lights_list[i]->locked)
			{
				char* cur_light_pointer = (char *) &cur_light_io;

				// clear the object
				memset (cur_light_pointer, 0, sizeof (light_io));

				cur_light_io.pos_x = lights_list[i]->pos_x;
				cur_light_io.pos_y = lights_list[i]->pos_y;
				cur_light_io.pos_z = lights_list[i]->pos_z;

				cur_light_io.r = lights_list[i]->r;
				cur_light_io.g = lights_list[i]->g;
				cur_light_io.b = lights_list[i]->b;

#ifdef NEW_LIGHT_FORMAT
				// Insert default values for now
				cur_light_io.spec_r = 255;
				cur_light_io.spec_g = 255;
				cur_light_io.spec_b = 255;

				cur_light_io.light_dir_z_sign = 0;

				cur_light_io.quadric_attenuation = 0;
				cur_light_io.range = 0;
				cur_light_io.cutoff = -32768;
				cur_light_io.exponent = 0;

				cur_light_io.light_dir_x = 0;
				cur_light_io.light_dir_y = 0;
#endif

#ifdef	ZLIBW
				gzwrite (f, cur_light_pointer, sizeof (light_io));
#else	//ZLIBW
				fwrite (cur_light_pointer, sizeof (light_io), 1, f);
#endif	//ZLIBW

				j++;
			}
		}

		// Write the particle systems
		j = 0;
		for (i = 0; i < MAX_PARTICLE_SYSTEMS; i++)
		{
			if (j > particles_no) break;
			if (particles_list[i] && particles_list[i]->def && particles_list[i]->def != &def)
			{
				char *cur_particles_pointer = (char *) &cur_particles_io;

				memset (cur_particles_pointer, 0, sizeof (particles_io));

				snprintf (cur_particles_io.file_name, sizeof (cur_particles_io.file_name), "%s", particles_list[i]->def->file_name);
				cur_particles_io.x_pos = particles_list[i]->x_pos;
				cur_particles_io.y_pos = particles_list[i]->y_pos;
				cur_particles_io.z_pos = particles_list[i]->z_pos;
#ifdef	ZLIBW
				gzwrite (f, cur_particles_pointer, sizeof (particles_io));
#else	//ZLIBW
				fwrite (cur_particles_pointer, sizeof (particles_io), 1, f);
#endif	//ZLIBW
				j++;
			}
		}

#ifdef EYE_CANDY
		// Write the eye candy effects
		for (i = 0; i < get_eye_candy_count (); i++)
		{
			char *cur_particles_pointer = (char *) &cur_particles_io;
			serialize_eye_candy_effect (i, &cur_particles_io);

#ifdef	ZLIBW
			gzwrite (f, cur_particles_pointer, sizeof (particles_io));
#else	//ZLIBW
			fwrite (cur_particles_pointer, sizeof (particles_io), 1, f);
#endif	//ZLIBW
		}
#endif

#ifdef CLUSTER_INSIDES
		// Compute the clusters and save them
		compute_clusters (occupied);
		free (occupied);

		get_clusters (&cluster_data, &cluster_data_len);
#ifdef ZLIBW
		gzwrite (f, cluster_data, cluster_data_len);
#else
		fwrite (cluster_data, cluster_data_len, 1, f);
#endif // ZLIBW
		free (cluster_data);
#endif // CLUSTER_INSIDES

#ifdef	ZLIBW
		gzclose(f);
#else	//ZLIBW
		fclose(f);
#endif	//ZLIBW
	}

	return 1;

}

int load_map (const char* file_name)
{
	int i;
	map_header cur_map_header;
	char* mem_map_header=(char *)&cur_map_header;
#if defined CLUSTER_INSIDES || defined NEW_LIGHT_FORMAT
	unsigned char version;
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

#ifdef	ZLIB
	gzFile f = NULL;
	f= my_gzopen(file_name, "rb");
#else	//ZLIB
	FILE *f = NULL;
	f= fopen(file_name, "rb");
#endif	//ZLIB
	if(!f)return 0;

#ifdef	ZLIB
	gzread(f, mem_map_header, sizeof(cur_map_header));//header only
#else	//ZLIB
	fread(mem_map_header, 1, sizeof(cur_map_header), f);//header only
#endif	//ZLIB
	
	//verify if we have a valid file
	if(cur_map_header.file_sig[0]!='e'
		|| cur_map_header.file_sig[1]!='l'
		|| cur_map_header.file_sig[2]!='m'
		|| cur_map_header.file_sig[3]!='f')
	{
#ifdef ZLIB
		gzclose(f);
#else
		fclose(f);
#endif
		return 0;
	}

	destroy_map();//Only destroy the map now....

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

	//get the type of map, and the ambient light
	dungeon=cur_map_header.dungeon;
#if defined CLUSTER_INSIDES || defined NEW_LIGHT_FORMAT
	version = cur_map_header.version;
#endif
	ambient_r=cur_map_header.ambient_r;
	ambient_g=cur_map_header.ambient_g;
	ambient_b=cur_map_header.ambient_b;

	//this is useful if we go in/out a dungeon
	new_minute();

	//read the tiles map
#ifdef	ZLIB
	gzread(f, tile_map, tile_map_size_x*tile_map_size_y);
#else	//ZLIB
	fread(tile_map, 1, tile_map_size_x*tile_map_size_y, f);
#endif	//ZLIB

	//load the tiles in this map, if not already loaded
	load_map_tiles();

	//read the heights map
#ifdef	ZLIB
	gzread(f, height_map, tile_map_size_x*tile_map_size_y*6*6);
#else	//ZLIB
	fread(height_map, 1, tile_map_size_x*tile_map_size_y*6*6, f);
#endif	//ZLIB

	//read the 3d objects
	for(i=0; i < obj_3d_no; i++)
	{
		char *cur_3do_pointer = (char *)&cur_3d_obj_io;
		
#ifdef	ZLIB
		gzread (f, cur_3do_pointer, obj_3d_io_size);
#else	//ZLIB
		fread (cur_3do_pointer, 1, obj_3d_io_size, f);
#endif	//ZLIB

		add_e3d_keep_deleted (cur_3d_obj_io.file_name, cur_3d_obj_io.x_pos, cur_3d_obj_io.y_pos, cur_3d_obj_io.z_pos, cur_3d_obj_io.x_rot, cur_3d_obj_io.y_rot, cur_3d_obj_io.z_rot, cur_3d_obj_io.self_lit, cur_3d_obj_io.blended, cur_3d_obj_io.r, cur_3d_obj_io.g, cur_3d_obj_io.b);
	}

	//read the 2d objects
	for(i=0;i<obj_2d_no;i++)
		{
			char * cur_2do_pointer=(char *)&cur_2d_obj_io;
#ifdef	ZLIB
			gzread(f, cur_2do_pointer, obj_2d_io_size);
#else	//ZLIB
			fread(cur_2do_pointer, 1, obj_2d_io_size, f);
#endif	//ZLIB

			add_2d_obj(cur_2d_obj_io.file_name,cur_2d_obj_io.x_pos,cur_2d_obj_io.y_pos,
			cur_2d_obj_io.z_pos,cur_2d_obj_io.x_rot,cur_2d_obj_io.y_rot,cur_2d_obj_io.z_rot);
		}


	//read the lights
	for (i = 0; i < lights_no; i++)
	{
		char* cur_light_pointer = (char *)&cur_light_io;
#ifdef	ZLIB
		gzread(f, cur_light_pointer, lights_io_size);
#else	//ZLIB
		fread(cur_light_pointer, 1, lights_io_size, f);
#endif	//ZLIB

#ifdef NEW_LIGHT_FORMAT
		if (version == 0)
		{
			// Old map format, insert default values for 
			// extended light parameters
			cur_light_io.spec_r = 255;
			cur_light_io.spec_g = 255;
			cur_light_io.spec_b = 255;

			cur_light_io.light_dir_z_sign = 0;

			cur_light_io.quadric_attenuation = 0;
			cur_light_io.range = 0;
			cur_light_io.cutoff = -32768;
			cur_light_io.exponent = 0;

			cur_light_io.light_dir_x = 0;
			cur_light_io.light_dir_y = 0;
		}
#endif // NEW_LIGHT_FORMAT

		add_light (cur_light_io.pos_x, cur_light_io.pos_y, cur_light_io.pos_z, cur_light_io.r, cur_light_io.g, cur_light_io.b, 1.0f, 0);
	}

	//read particle systems
	for(i=0;i<particles_no;i++)
		{
			char *cur_particles_pointer=(char *)&cur_particles_io;
#ifdef	ZLIB
			gzread(f, cur_particles_pointer,particles_io_size);
#else	//ZLIB
			fread(cur_particles_pointer,1,particles_io_size,f);
#endif	//ZLIB

			if (!strncmp(cur_particles_io.file_name, "ec://", 5))
			{
#ifdef EYE_CANDY
				printf("Deserializing eye candy.\n");
				deserialize_eye_candy_effect(&cur_particles_io);
#else
				LOG_ERROR("Map contains eye candy effect, but map editor is compiled without eye candy support");
#endif
			}
			else
			{
				add_particle_sys(cur_particles_io.file_name,cur_particles_io.x_pos,cur_particles_io.y_pos,cur_particles_io.z_pos);
				if(particles_list[i]) particles_list[i]->ttl=-1;//Fail-safe if things mess up...
			}
		}

#ifdef	ZLIB
	gzclose(f);
#else	//ZLIB
	fclose(f);
#endif	//ZLIB

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
	mx=0;
	my=0;
	
	dungeon=0;
}


