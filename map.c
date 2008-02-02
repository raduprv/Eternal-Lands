#include <stdlib.h>
#include <string.h>
#include "map.h"
#include "2d_objects.h"
#include "3d_objects.h"
#include "asc.h"
#include "bbox_tree.h"
#include "consolewin.h"
#include "cursors.h"
#include "dialogues.h"
#include "elwindows.h"
#include "errors.h"
#include "gamewin.h"
#include "gl_init.h"
#include "init.h"
#include "interface.h"
#include "lights.h"
#include "loading_win.h"
#include "mapwin.h"
#include "missiles.h"
#include "multiplayer.h"
#include "particles.h"
#include "pathfinder.h"
#include "reflection.h"
#include "sound.h"
#include "storage.h"
#include "tiles.h"
#include "translate.h"
#include "weather.h"
#ifdef CLUSTER_INSIDES
#include "cluster.h"
#endif
#ifdef COUNTERS
#include "counters.h"
#endif
#if defined SFX && defined EYE_CANDY
#include "eye_candy_wrapper.h"
#endif
#ifdef MINIMAP
#include "minimap.h"
#endif
#ifdef NEW_FILE_IO
#include "io/elpathwrapper.h"
#endif
#ifdef PAWN
#include "pawn/elpawn.h"
#endif
#ifdef SKY_FPV_CURSOR
#include "sky.h"
#endif
#ifdef MINES
#include "mines.h"
#endif // MINES
#ifdef NEW_LIGHTING
 #include "textures.h"
#endif

int map_type=1;
Uint32 map_flags=0;

void destroy_map()
{
	int i;
#ifdef EXTRA_DEBUG
	ERR();
#endif

	have_a_map = 0;

	clear_bbox_tree(main_bbox_tree);
	//kill the tile and height map
	if(tile_map)
	{
		free (tile_map);
		tile_map = NULL;
	}
	memset(tile_list,0,sizeof(tile_list));
	tile_map_size_x = tile_map_size_y = 0;

	if(height_map)
	{
		free (height_map);
		height_map = NULL;
	}

#ifndef MAP_EDITOR2
	///kill the pathfinding tile map
	if(pf_tile_map)
	{
		free(pf_tile_map);
		pf_tile_map = NULL;

		if (pf_follow_path)
			pf_destroy_path();
	}
#endif

	//kill the 3d objects links
	for(i=0;i<MAX_OBJ_3D;i++)
		{
			if(objects_list[i])
				{
#ifdef  EYE_CANDY
					ec_remove_obstruction_by_object3d(objects_list[i]);
#endif

					free(objects_list[i]);
					objects_list[i]=NULL;//kill any refference to it
				}
		}
	// reset the top pointer
	highest_obj_3d= 0;

	//kill the 2d objects links
	for(i=0;i<MAX_OBJ_2D;i++)
		{
			if(obj_2d_list[i])
				{
					free(obj_2d_list[i]);
					obj_2d_list[i]=0;//kill any refference to it
				}
		}

	//kill the lights links
	for(i=0;i<MAX_LIGHTS;i++)
		{
			if(lights_list[i])
				{
					free(lights_list[i]);
					lights_list[i]= NULL;	//kill any refference to it
				}
		}
	num_lights= 0;

#ifdef CLUSTER_INSIDES
	destroy_clusters_array ();
#endif
}

#ifndef MAP_EDITOR2
int get_cur_map (const char * file_name)
{
	int i;

	for (i=0; continent_maps[i].name != NULL; i++)
	{
		if (strcmp (continent_maps[i].name, file_name) == 0)
		{
			return i;
		}
	}

	return -1;	
}
#endif

static void init_map_loading(const char *file_name)
{
	destroy_map();

	/*
	 * Grum: the below is wrong: it never sets cur_map for the new map
	 * when you're leaving an inside map. Perhaps it would be useful not
	 * to set cur_map if you're *entering* an indide map, but we don't
	 * know that at this point. 
	 *
	 * I wonder why we souldn't want to set it anyway...
	 */
	/*
	//Otherwise we pretend that we don't know where we are - if anyone wants to do the work and input all coordinates it's fine by me however :o)
	if (!dungeon) cur_map = get_cur_map (file_name);
	else cur_map=-1;
	*/
	cur_map = get_cur_map (file_name);
	
	create_loading_win(window_width, window_height, 1);
	show_window(loading_win);
}

static __inline__ void build_path_map()
{
	int i, x, y;

	//create the tile map that will be used for pathfinding
	pf_tile_map = (PF_TILE *)calloc(tile_map_size_x*tile_map_size_y*6*6, sizeof(PF_TILE));

	i = 0;
	for (y = 0; y < tile_map_size_y*6; y++)
	{
		for (x = 0; x < tile_map_size_x*6; x++, i++)
		{
			pf_tile_map[i].x = x;
			pf_tile_map[i].y = y;
			pf_tile_map[i].z = height_map[i];
		}
	}
}

void updat_func(char *str, float percent)
{
	update_loading_win(str, percent);
}

static int el_load_map(const char * file_name)
{
	int ret;

	init_map_loading(file_name);
	ret = load_map(file_name, &updat_func);
	if (!ret)
		// don't try to build pathfinder maps etc. when loading 
		// the map failed...
		return ret;

#if NEW_LIGHTING
	set_scene_metadata(file_name);
#endif

#ifdef SKY_FPV_CURSOR
	if (strstr("underworld",file_name) != NULL){
		sky_type(UNDERWORLD_SKY);
	} else if (dungeon) {
		sky_type(INTERIORS_SKY);
	} else {
		sky_type(CLOUDY_SKY);
	}
#endif /* SKY_FPV_CURSOR */
	build_path_map();
	init_buffers();
	
	// reset light levels in case we enter or leave an inside map
	new_minute();

	destroy_loading_win();
	return ret;
}

void change_map (const char *mapname)
{
#ifndef	MAP_EDITOR
	remove_all_bags();
#ifdef MINES
	remove_all_mines();
#endif // MINES
#endif	//MAP_EDITOR

	set_all_intersect_update_needed(main_bbox_tree);
	object_under_mouse=-1;//to prevent a nasty crash, while looking for bags, when we change the map
#ifndef MAP_EDITOR2
#ifdef EXTRA_DEBUG
	ERR();
#endif
	close_dialogue();	// close the dialogue window if open
	close_storagewin(); //if storage is open, close it
	destroy_all_particles();
#ifdef	EYE_CANDY
	ec_delete_all_effects();
#endif	//EYE_CANDY
#ifdef NEW_SOUND
	stop_all_sounds();
#else
	kill_local_sounds();
#endif	//NEW_SOUND
#ifdef MISSILES
	missiles_clear();
#endif // MISSILES
	if (!el_load_map(mapname)) {
		char error[255];
		safe_snprintf(error, sizeof(error), cant_change_map, mapname);
		LOG_TO_CONSOLE(c_red4, error);
		LOG_TO_CONSOLE(c_red4, empty_map_str);
		LOG_ERROR(cant_change_map, mapname);
		load_empty_map();
	} else {
		locked_to_console = 0;
	}
#ifndef NEW_WEATHER
	rain_sound=0;//kill local sounds also kills the rain sound
#endif
#ifndef NEW_SOUND
	kill_local_sounds();
#ifdef	OGG_VORBIS
	playing_music=0;
#endif // OGG_VORBIS
#endif // !NEW_SOUND
	get_map_playlist();
#ifdef NEW_SOUND
	setup_map_sounds(get_cur_map(mapname));
#endif // NEW_SOUND
	have_a_map=1;
	//also, stop the rain
	clear_weather();

	if ( get_show_window (map_root_win) )
	{
		hide_window(map_root_win);
		switch_from_game_map ();
		show_window(game_root_win);
	}
	load_map_marks();//Load the map marks
#else // !MAP_EDITOR2
	destroy_all_particles();
#ifdef NEW_SOUND
	stop_all_sounds();
#else
	kill_local_sounds();
#endif	//NEW_SOUND
	if (!load_map(mapname)) {
		char error[255];
		safe_snprintf(error, sizeof(error), cant_change_map, mapname);
		LOG_TO_CONSOLE(c_red4, error);
		LOG_TO_CONSOLE(c_red4, empty_map_str);
		LOG_ERROR(cant_change_map, mapname);
		load_empty_map();
	}
#ifndef NEW_SOUND
	kill_local_sounds();
#ifdef	OGG_VORBIS
	playing_music=0;
#endif // OGG_VORBIS
#endif // !NEW_SOUND
	get_map_playlist();
#ifdef NEW_SOUND
	setup_map_sounds(get_cur_map(mapname));
#endif // NEW_SOUND
	have_a_map=1;
#endif  //MAP_EDITOR2
#ifdef  MINIMAP
	change_minimap();
#endif  //MINIMAP

#ifdef PAWN
	run_pawn_map_function ("change_map", "s", mapname);
#endif
}

int load_empty_map()
{
	if (!el_load_map("./maps/nomap.elm"))
	{
#ifndef MAP_EDITOR2
		locked_to_console = 1;
		hide_window (game_root_win);
		show_window (console_root_win);
		LOG_TO_CONSOLE(c_red4, no_nomap_str);
		LOG_ERROR(cant_change_map, "./maps/nomap.elm");
		SDLNet_TCP_Close(my_socket);
		disconnected = 1;
#ifdef NEW_SOUND
		stop_all_sounds();
#endif // NEW_SOUND
#ifdef COUNTERS
		disconnect_time = SDL_GetTicks();
#endif
		SDLNet_Quit();
		LOG_TO_CONSOLE(c_red3, disconnected_from_server);
		//Fake a map to make sure we don't get any crashes.
#endif
		safe_snprintf(map_file_name, sizeof(map_file_name), "./maps/nomap.elm");
		tile_map_size_y = 256;
		tile_map_size_x = 256;
		dungeon = 0;
		ambient_r = 0;
		ambient_g = 0;
		ambient_b = 0;
		tile_map = calloc(tile_map_size_x*tile_map_size_y, sizeof(char));
		height_map = calloc(tile_map_size_x*tile_map_size_y*6*6, sizeof(char));
#ifndef MAP_EDITOR2
		pf_tile_map = calloc(tile_map_size_x*tile_map_size_y*6*6, sizeof(char));
#endif
		return 0;
	}
	return 1;
}

void load_map_marks()
{ 
	FILE * fp = NULL;
	char marks_file[256] = {0}, text[600] = {0};
	char *mapname = strrchr (map_file_name,'/');

	if(mapname == NULL) {
		//Oops
		return;
	}
#ifndef NEW_FILE_IO
#ifndef WINDOWS
	safe_snprintf (marks_file, sizeof (marks_file), "%s%s.txt", configdir, mapname + 1);
#else
	safe_snprintf (marks_file, sizeof (marks_file), "%s.txt", mapname + 1);
#endif
	// don't use my_fopen here, not everyone uses map markers
	fp = fopen(marks_file, "r");
#else /* NEW_FILE_IO */
	safe_snprintf (marks_file, sizeof (marks_file), "maps/%s.txt", mapname + 1);
	fp = open_file_config(marks_file, "r");
	if(fp == NULL){
		//TODO: remove this after the next update. Until then, people may still have files in the old location.
		safe_snprintf (marks_file, sizeof (marks_file), "%s.txt", mapname + 1);
		fp = open_file_config(marks_file, "r");
	}
#endif /* NEW_FILE_IO */
	max_mark = 0;
	
	if (fp == NULL) return;
	
	while ( fgets(text, 600,fp) ) {
		if (strlen (text) > 1) {
			sscanf (text, "%d %d", &marks[max_mark].x, &marks[max_mark].y);
			text[strlen(text)-1] = '\0'; //remove the newline
			if ((strstr(text, " ") == NULL) || (strstr(strstr(text, " ")+1, " ") == NULL)) {
 				LOG_ERROR("Bad map mark file=[%s] text=[%s]", marks_file, text);
			}
			else {
				safe_strncpy(marks[max_mark].text, strstr(strstr(text, " ")+1, " ") + 1, sizeof(marks[max_mark].text));
				max_mark++;
				if ( max_mark > 200 ) break;
			}
		}
	}
	
	fclose(fp);
}

void init_buffers()
{
	int terrain_buffer_size;
	int water_buffer_size;
	int i, j, cur_tile;

	terrain_buffer_size = 0;
	water_buffer_size = 0;

	for(i = 0; i < tile_map_size_y; i++)
	{
		for(j = 0; j < tile_map_size_x; j++)
		{
			cur_tile = tile_map[i*tile_map_size_x+j];
			if (cur_tile != 255)
			{
				if (IS_WATER_TILE(cur_tile)) 
				{
					water_buffer_size++;
				}
				else 
				{
					terrain_buffer_size++;
				}
			}
		}
	}
	init_water_buffers(water_buffer_size);
	init_terrain_buffers(terrain_buffer_size);
	init_reflection_portals(water_buffer_size);
}

int get_3d_objects_from_server (int nr_objs, const Uint8 *data, int len)
{
	int iobj;
	int obj_x, obj_y;
	int offset, nb_left;
	float x = 0.0f, y = 0.0f, z = 0.0f, rx = 0.0f, ry = 0.0f, rz = 0.0f;
	char obj_name[128];
	int name_len, max_name_len;
	int id = -1;
	int all_ok = 1;
	
	offset = 0;
	nb_left = len;
	for (iobj = 0; iobj < nr_objs; iobj++)
	{
		int obj_err = 0;
		
		if (nb_left < 14)
		{
			// Warn about this error!
                        log_error ("Incomplete 3D objects list!");
			all_ok = 0;
                        break;
		}
		
		obj_x = SDL_SwapLE16 (*((Uint16 *)(&data[offset])));
		offset += 2;
		obj_y = SDL_SwapLE16 (*((Uint16 *)(&data[offset])));
		offset += 2;
		if (obj_x > tile_map_size_x * 6 || obj_y > tile_map_size_y * 6)
		{
			// Warn about this error!
			log_error("A 3D object was located OUTSIDE the map!");
			offset += 8;
			obj_err = 1;
                }
		else
		{
			rx = SwapLEFloat (*((float *)(&data[offset])));
			offset += 2;
			ry = SwapLEFloat (*((float *)(&data[offset])));
			offset += 2;
			rz = SwapLEFloat (*((float *)(&data[offset])));
			offset += 2;
			id = SDL_SwapLE16 (*((Uint16 *)(&data[offset])));
			offset += 2;

			x = 0.5f * obj_x + 0.25f;
			y = 0.5f * obj_y + 0.25f;
			z = -2.2f + height_map[obj_y*tile_map_size_x*6+obj_x] * 0.2f;
		}
		
		nb_left -= 12;
		max_name_len = nb_left > sizeof (obj_name) ? sizeof (obj_name) : nb_left;
		name_len = safe_snprintf (obj_name, max_name_len, "%s", &data[offset]);
		if (name_len < 0 || name_len >= sizeof (obj_name))
		{
			// Warn about this error!
                        log_error("3D object has invalid or too long file name!");
			all_ok = 0;
                        break;
		}
		
		offset += name_len + 1;
		nb_left -= name_len + 1;
		
		if (!obj_err)
			add_e3d_at_id (id, obj_name, x, y, z, rx, ry, rz, 0, 0, 1.0f, 1.0f, 1.0f, 1);
		else
			all_ok = 0;
	}
	
	return all_ok;
}
	
void remove_3d_object_from_server (int id)
{	
	if (id < 0 || id > MAX_OBJ_3D)
	{
		LOG_ERROR ("Trying to remove object with invalid id %d", id);
		return;
	}
	if (objects_list[id] == NULL)
	{
		LOG_ERROR ("Trying to remove non-existant object");
		return;
	}

	destroy_3d_object (id);
}
