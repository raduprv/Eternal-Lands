#ifndef	__MISC_H
#define	__MISC_H

void open_3d_obj();
void open_2d_obj();
void open_map_file();
void save_map_file();
void open_particles_obj();
void save_particle_def_file();

#ifdef LINUX
void open_3d_obj_continued();
void open_2d_obj_continued();
void open_map_file_continued();
void save_map_file_continued();
void open_particles_obj_continued();
void save_particle_def_file_continued();
#endif


void kill_height_map_at_texture_tile(int tex_pos);
int evaluate_colision();
void get_3d_object_under_mouse();
void kill_3d_object(int object_id);
void move_3d_object(int object_id);
void clone_3d_object(int object_id);
void open_3d_obj();

void open_2d_obj();
void get_2d_object_under_mouse();
void kill_2d_object(int object_id);
void move_2d_object(int object_id);
void clone_2d_object(int object_id);

void get_particles_object_under_mouse();
void kill_particles_object(int object_id);
void move_particles_object(int object_id);
void clone_particles_object(int object_id);

void load_all_tiles();
void get_tile_under_mouse_from_list();
void move_tile();
void draw_light_source(light * object_id);
void visualise_lights();
void get_light_under_mouse();
void move_light(int object_id);
void kill_light(int object_id);
void clone_light(int object_id);
void change_color_height(unsigned char cur_height);
void move_height_tile();
void get_height_under_mouse_from_list();
void draw_big_height_tile(int size);
void draw_heights_wireframe();
void draw_height_map();

#ifndef LINUX
void open_map_file();
void save_map_file();
#endif

#ifdef LINUX
void open_3d_obj_continued();
void open_2d_obj_continued();
void open_map_file();
void open_map_file_continued();
void save_map_file();
void save_map_file_continued();
#endif

#endif	//__MISC_H
