#include <stdlib.h>
#include <string.h>
#include <errno.h>
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
#include "global.h"
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
#include "counters.h"
#include "eye_candy_wrapper.h"
#include "minimap.h"
#include "io/elpathwrapper.h"
#ifdef PAWN
#include "pawn/elpawn.h"
#endif
#include "sky.h"
#include "mines.h"
#include "highlight.h"

int map_type=1;
Uint32 map_flags=0;

hash_table *server_marks=NULL;


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
	destroy_all_3d_objects();

	//kill the 2d objects links
	destroy_all_2d_objects();

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


	if (strstr(file_name, "underworld") != NULL)
	{
		skybox_set_type(SKYBOX_UNDERWORLD);
		skybox_update_colors();
	}
	else if (dungeon)
	{
		skybox_set_type(SKYBOX_NONE);
		skybox_update_colors();
	}
	else
	{
		skybox_set_type(SKYBOX_CLOUDY);
		skybox_init_defs(file_name);
	}
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
	remove_all_mines();
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
	ec_delete_all_effects();
#ifdef NEW_SOUND
	stop_all_sounds();
#endif	//NEW_SOUND
	missiles_clear();
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
	load_map_marks();
	
#ifdef NEW_SOUND
	get_map_playlist();
	setup_map_sounds(get_cur_map(mapname));
#endif // NEW_SOUND
	have_a_map=1;
	//also, stop the rain
	weather_clear();

	if ( get_show_window (map_root_win) )
	{
		hide_window(map_root_win);
		switch_from_game_map ();
		show_window(game_root_win);
	}
#else // !MAP_EDITOR2
	destroy_all_particles();
#ifdef NEW_SOUND
	stop_all_sounds();
#endif	//NEW_SOUND
	if (!load_map(mapname)) {
		char error[255];
		safe_snprintf(error, sizeof(error), cant_change_map, mapname);
		LOG_TO_CONSOLE(c_red4, error);
		LOG_TO_CONSOLE(c_red4, empty_map_str);
		LOG_ERROR(cant_change_map, mapname);
		load_empty_map();
	}

#ifdef NEW_SOUND
	get_map_playlist();
	setup_map_sounds(get_cur_map(mapname));
#endif // NEW_SOUND
	have_a_map=1;
#endif  //MAP_EDITOR2
	change_minimap();

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
		disconnect_time = SDL_GetTicks();
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


void init_server_markers(){
	//init hash table
	destroy_hash_table(server_marks);
	server_marks= create_hash_table(50,hash_fn_int,cmp_fn_int,free);
}

void add_server_markers(){

	hash_entry *he;
	server_mark *sm;
	int i,l;
	char *mapname = map_file_name;

	//find the slot to add server marks
	for(i=0;i<max_mark;i++)
		if(marks[i].server_side) break;
	l=i;
	
	if(!server_marks) init_server_markers();
	if(server_marks) {
		hash_start_iterator(server_marks);
		while((he=hash_get_next(server_marks))){
			sm = (server_mark *) he->item;
			//is it in this map?
			if(strcmp(mapname,sm->map_name)) continue;
			//find the next slot. If not there, add 1
			for(i=l;i<MAX_MARKINGS;i++) 
				if(marks[i].server_side||i>=max_mark) {l=i; if(l>=max_mark) max_mark=l+1; break;}
			//add the marker
			marks[l].x=sm->x;
			marks[l].y=sm->y;
			marks[l].server_side=1;
			safe_strncpy(marks[l].text, sm->text, sizeof(marks[l].text));
			l++;
		}
		//remove server side markings if necessary
		for(i=l+1;i<max_mark;i++)
			if(marks[i].server_side) {marks[i].server_side=0;marks[i].x=marks[i].y=-1;}
	}
}

void load_marks_to_buffer(char* mapname, marking* buffer, int* max)
{ 
	FILE * fp = NULL;
	char marks_file[256] = {0}, text[600] = {0};
	
	if(mapname == NULL) {
		//Oops
		return;
	}
	safe_snprintf (marks_file, sizeof (marks_file), "%s.txt", mapname + 1);
	fp = open_file_config(marks_file, "r");
	*max = 0;
	
	if (fp == NULL) return;

	//load user markers
	while ( fgets(text, 600,fp) ) {
		if (strlen (text) > 1) {
			int r,g,b;			
			sscanf (text, "%d %d", &buffer[*max].x, &buffer[*max].y);
			//scanning mark color. It can be optional -> default=green
			if(sscanf(text,"%*d %*d|%d,%d,%d|",&r,&g,&b)<3) { //NO SPACES in RGB format string!
				r=b=0;
				g=255;
			}
			buffer[*max].server_side=0;
			text[strlen(text)-1] = '\0'; //remove the newline
			if ((strstr(text, " ") == NULL) || (strstr(strstr(text, " ")+1, " ") == NULL)) {
 				LOG_ERROR("Bad map mark file=[%s] text=[%s]", marks_file, text);
			}
			else {
				safe_strncpy(buffer[*max].text, strstr(strstr(text, " ")+1, " ") + 1, sizeof(buffer[*max].text));
				buffer[*max].r=r;
				buffer[*max].g=g;
				buffer[*max].b=b;
				*max = *max + 1;
				if ( *max >= MAX_USER_MARKS ) break;
			}
		}
	}
	
	fclose(fp);

	LOG_DEBUG("Read map markings from file '%s'", marks_file);

}

void load_map_marks()
{ 
	//load user markers
	load_marks_to_buffer(map_file_name, marks, &max_mark);

	//load server markers on this map
	add_server_markers();

}

void save_markings()
{
      FILE * fp;
      char marks_file[256];
      int i;

	safe_snprintf (marks_file, sizeof (marks_file), "maps/%s.txt", strrchr (map_file_name,'/') + 1);

	fp = open_file_config(marks_file,"w");
	if ( fp == NULL ){
		LOG_ERROR("%s: %s \"%s\": %s\n", reg_error_str, cant_open_file, marks_file, strerror(errno));
	} else {
		for ( i = 0 ; i < max_mark ; i ++){
			if ( marks[i].x > 0 && !marks[i].server_side){
				fprintf(fp,"%d %d|%d,%d,%d| %s\n",marks[i].x,marks[i].y,marks[i].r,marks[i].g,marks[i].b,marks[i].text);
			}
		}
		fclose(fp);
	}

	LOG_DEBUG("Wrote map markings to file '%s'", marks_file);
}




void load_server_markings(){
	char fname[128];
	FILE *fp;
	server_mark sm;
	int rf;

	init_server_markers();
	
	//open server markings file
	safe_snprintf(fname, sizeof(fname), "servermarks_%s.dat",username_str);
	my_tolower(fname);

	/* sliently ignore non existing file */
	if (file_exists_config(fname)!=1)
		return;

	fp = open_file_config(fname,"r");
	if(fp == NULL){
		LOG_ERROR("%s: %s \"%s\": %s\n", reg_error_str, cant_open_file, fname, strerror(errno));
		return;
	}

	while((rf=fscanf(fp,"%d %d %d %s %[^\n]s\n",&sm.id,&sm.x,&sm.y,sm.map_name,sm.text))==5){		
		server_mark *nm = calloc(1,sizeof(server_mark));
		memcpy(nm,&sm,sizeof(server_mark));
		hash_add(server_marks,(NULL+sm.id),(void*) nm);
	}
	
	fclose (fp);

	LOG_DEBUG("Read server markings from file '%s'", fname);

	add_server_markers();
}


void save_server_markings(){
	char fname[128];
	FILE *fp;
	server_mark *sm;
	hash_entry *he;
	
	if(!server_marks) return;

	//open server markings file
	safe_snprintf(fname, sizeof(fname), "servermarks_%s.dat",username_str);
	my_tolower(fname);
	fp = open_file_config(fname,"w");
	if(fp == NULL){
		LOG_ERROR("%s: %s \"%s\": %s\n", reg_error_str, cant_open_file, fname, strerror(errno));
		return;
	}

	hash_start_iterator(server_marks);
	
	while((he=hash_get_next(server_marks))){		
		sm = (server_mark *) he->item;
		fprintf(fp,"%d %d %d %s %s\n",sm->id, sm->x, sm->y, sm->map_name, sm->text);
	}
	
	fclose (fp);	

	LOG_DEBUG("Wrote server markings to file '%s'", fname);
}

//called in elconfig.c when turning markers on/off
void change_3d_marks(int *rel){
	*rel= !*rel;
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

void free_buffers()
{
	if (water_tile_buffer)
		free(water_tile_buffer);
	if (terrain_tile_buffer)
		free(terrain_tile_buffer);
	if (reflection_portals)
		free(reflection_portals);
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
                        LOG_WARNING("Incomplete 3D objects list!");
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
			LOG_WARNING("A 3D object was located OUTSIDE the map!");
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
			z = get_tile_height(obj_x, obj_y);
		}
		
		nb_left -= 12;
		max_name_len = nb_left > sizeof (obj_name) ? sizeof (obj_name) : nb_left;
		name_len = safe_snprintf (obj_name, max_name_len, "%s", &data[offset]);
		if (name_len < 0 || name_len >= sizeof (obj_name))
		{
			// Warn about this error!
                        LOG_WARNING("3D object has invalid or too long file name!");
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


//3D MAP MARKERS
#define MAX(a,b) ( ((a)>(b)) ? (a):(b) )
#define ABS(a) ( ((a)<0)?(-(a)):(a)  )
#define DST(xa,ya,xb,yb) ( MAX(ABS(xa-xb),ABS(ya-yb))  )
int marks_3d=1;
float mark_z_rot=0;

void animate_map_markers(){

	int dt;
	static int last_rot=0;

	dt=cur_time-last_rot;
	last_rot+=dt;
	mark_z_rot+=0.1*dt;
	if(mark_z_rot>360) mark_z_rot-=360;

}

void display_map_marks(){
	actor *me;
	float x,y,z;
	int i,ax,ay;
	float dx = (TILESIZE_X / 6);
	float dy = (TILESIZE_Y / 6);
	float fr = mark_z_rot/360;
	float j,ff=0;

	me = get_our_actor();
	if(!me) return;
	ax = me->x_pos;
	ay = me->y_pos;
	
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glEnable(GL_ALPHA_TEST);	

	for(i=0;i<max_mark;i++){
		x=marks[i].x/2.0;
		y=marks[i].y/2.0;
		x += (TILESIZE_X / 2);
		y += (TILESIZE_Y / 2);
		if(DST(ax,ay,x,y)>MARK_DIST||marks[i].x<0||!marks_3d) continue;
		z = get_tile_height(marks[i].x, marks[i].y);
		for(j=z-fr/5,ff=1;j<z+2;j+=0.1,ff=(2-(j-z))/2) {
			if(marks[i].server_side) glColor4f(0.0f, 0.0f, 1.0f, 0.9f-(j-z)/3);
			else glColor4f((float)marks[i].r/255, (float)marks[i].g/255, (float)marks[i].b/255, 0.7f-(j-z)/3);
			glBegin(GL_QUADS);
				glVertex3f(x-dx*ff,y-dy*ff,j);
				glVertex3f(x-dx*ff,y+dy*ff,j);
				glVertex3f(x+dx*ff,y+dy*ff,j);
				glVertex3f(x+dx*ff,y-dy*ff,j);
			glEnd();
		}
		
	}
	
	glDisable(GL_ALPHA_TEST);
	//glEnable(GL_LIGHTING);
	glDisable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	
}

void display_map_markers() {
	int ax, ay;
	float z,x,y;
	int i;
	GLdouble model[16],proj[16];
	GLint view[4];
	GLdouble hx,hy,hz;
	float banner_width;
	float font_scale = 1.0f/ALT_INGAME_FONT_X_LEN;
	float font_size_x=font_scale*SMALL_INGAME_FONT_X_LEN;
	float font_size_y=font_scale*SMALL_INGAME_FONT_Y_LEN;
	char tmpb[4];
	actor *me;

	me = get_our_actor();
	if(!me) return;
	ax = me->x_pos;
	ay = me->y_pos;
	
	glGetDoublev(GL_MODELVIEW_MATRIX, model);
	glGetDoublev(GL_PROJECTION_MATRIX, proj);
	glGetIntegerv(GL_VIEWPORT, view);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(view[0],view[2]+view[0],view[1],view[3]+view[1],0.0f,-1.0f);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_TEXTURE_2D);
	glColor4f(1.0,1.0,1.0,1.0);
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	
	for(i=0;i<max_mark;i++){
		x=marks[i].x/2.0;
		y=marks[i].y/2.0;
		x += (TILESIZE_X / 2);
		y += (TILESIZE_Y / 2);
		if(DST(ax,ay,x,y)>MARK_DIST||marks[i].x<0||!marks_3d) continue;
		z = get_tile_height(marks[i].x, marks[i].y)+2.3;
		gluProject(x, y, z, model, proj, view, &hx, &hy, &hz);
		//shorten text
		memcpy(tmpb,marks[i].text+MARK_CLIP_POS,4);
		marks[i].text[MARK_CLIP_POS]=marks[i].text[MARK_CLIP_POS+1]=marks[i].text[MARK_CLIP_POS+2]='.';
		marks[i].text[MARK_CLIP_POS+3]=0;
		banner_width = ((float)get_string_width((unsigned char*)marks[i].text)*(font_size_x*name_zoom))/2.0;
		draw_ortho_ingame_string(hx-banner_width, hy, hz, (unsigned char*)marks[i].text, 4, font_size_x, font_size_y);
		//restore text
		memcpy(marks[i].text+MARK_CLIP_POS,tmpb,4);
			
	}
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	//glEnable(GL_LIGHTING);
	glDepthFunc(GL_LESS);
	
	
}



