#ifndef	__MISC_H
#define	__MISC_H

#ifdef ZLIB
#include <zlib.h>
#endif // ZLIB
#include "lights.h"

extern float grid_height;

void draw_checkbox (int startx, int starty, int checked);

void open_3d_obj();
void open_2d_obj();
void open_map_file();
void save_map_file();
void open_particles_obj();
void save_particle_def_file();
void open_eye_candy_obj();

//#ifdef LINUX
void open_3d_obj_continued();
void open_2d_obj_continued();
void open_map_file_continued();
void save_map_file_continued();
void open_particles_obj_continued();
void open_eye_candy_obj_continued();
void save_particle_def_file_continued();
//#endif


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

void display_particle_handles ();
void get_particles_object_under_mouse();
void kill_particles_object(int object_id);
void move_particles_object(int object_id);
void clone_particles_object(int object_id);

void load_all_tiles();
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
void map_floodfill ();
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

extern char* selected_file;
FILE *my_fopen (const char *fname, const char *mode);
off_t get_file_size(const char *fname);
int file_exists(const char *fname);
int gzfile_exists(const char *fname);
#ifdef ZLIB
/*!
 * \ingroup misc
 * \brief Append '.gz' to a filename and try to open it using gzopen
 *
 * Appends the '.gz' to a filename and tries to open the file with that
 * name. If it fails, tries to open the file with the original filename.
 *
 * \param filename The file to open
 * \param mode The i/o mode (see open())
 * \return a zlib file handle
 */
gzFile * my_gzopen(const char * filename, const char * mode);
#endif // ZLIB

static __inline__ int min2i (int x, int y)
{
	return (x <= y)? x : y;
}

static __inline__ int max2i (int x, int y)
{
	return (x >= y)? x : y;
}

static __inline__ unsigned min2u (unsigned x, unsigned y)
{
	return (x <= y)? x : y;
}

static __inline__ unsigned max2u (unsigned x, unsigned y)
{
	return (x >= y)? x : y;
}

static __inline__ float min2f (float x, float y)
{
	return (x <= y)? x : y;
}

static __inline__ float max2f (float x, float y)
{
	return (x >= y)? x : y;
}

static __inline unsigned clampu(unsigned x, unsigned l, unsigned u)
{
	return min2u(max2u(x,l),u);
}

static __inline int clampi(int x, int l, int u)
{
	return min2i(max2i(x,l),u);
}

static __inline float clampf(float x, float l, float u)
{
	return min2f(max2f(x,l),u);
}

#endif	//__MISC_H
