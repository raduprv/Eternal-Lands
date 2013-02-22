#ifndef __SKY_H__
#define __SKY_H__


#include "lights.h"

extern int skybox_show_sky;
extern int skybox_show_clouds;
extern int skybox_show_sun;
extern int skybox_show_moons;
extern int skybox_show_stars;
extern int skybox_show_horizon_fog;

// position of the sun (different from the light position!)
extern float skybox_sun_position[4];

extern double skybox_view[16];
extern double skybox_time_d;

extern float skybox_clouds[360][4];
extern float skybox_clouds_detail[360][4];
extern float skybox_clouds_sunny[360][4];
extern float skybox_clouds_detail_sunny[360][4];
extern float skybox_clouds_rainy[360][4];
extern float skybox_clouds_detail_rainy[360][4];
extern float skybox_sky1[360][4];
extern float skybox_sky2[360][4];
extern float skybox_sky3[360][4];
extern float skybox_sky4[360][4];
extern float skybox_sky5[360][4];
extern float skybox_sky1_sunny[360][4];
extern float skybox_sky2_sunny[360][4];
extern float skybox_sky3_sunny[360][4];
extern float skybox_sky4_sunny[360][4];
extern float skybox_sky5_sunny[360][4];
extern float skybox_sun[360][4];
extern float skybox_fog[360][4];
extern float skybox_fog_sunny[360][4];
extern float skybox_fog_rainy[360][4];
extern float skybox_light_ambient[360][4];
extern float skybox_light_diffuse[360][4];
extern float skybox_light_ambient_rainy[360][4];
extern float skybox_light_diffuse_rainy[360][4];
extern float skybox_sky_color[4];
extern float skybox_fog_color[4];
extern float skybox_fog_density;
extern float skybox_light_ambient_color[4];
extern float skybox_light_diffuse_color[4];
extern float skybox_sunny_sky_bias;
extern float skybox_sunny_clouds_bias;
extern float skybox_sunny_fog_bias;

typedef enum {
    SKYBOX_NONE = 0,
    SKYBOX_CLOUDY = 1,
    SKYBOX_UNDERWORLD = 2
} skybox_type;

void skybox_compute_z_position();
float skybox_get_z_position();

void skybox_direction_to_ground_coords(float dir[3], float *gx, float *gy);
void skybox_coords_from_ground_coords(float sky_coords[3], float gx, float gy);

void skybox_compute_element_projection(float proj[3], float pos[3]);
float skybox_get_height(float x, float y);

void skybox_set_type(skybox_type sky);
void skybox_display();

void skybox_init_gl();
void skybox_init_defs(const char *map_name);
void free_skybox();

void skybox_update_positions();
void skybox_update_colors();

static __inline__ void blend_colors(float result[], float orig[], float dest[], float t, int size)
{
    while (size--) result[size] = (1.0-t)*orig[size] + t*dest[size];
}

static __inline__ void skybox_get_current_color(float result[4], float table[360][4])
{
	blend_colors(result, table[game_minute], table[(game_minute+1)%360], (float)game_second/60.0, 4);
}

static __inline__ void skybox_blend_current_colors(float result[4], float orig_table[360][4], float dest_table[360][4], float t)
{
	float color1[4], color2[4];
	skybox_get_current_color(color1, orig_table);
	skybox_get_current_color(color2, dest_table);
	blend_colors(result, color1, color2, t, 4);
}


#endif // __SKY_H__
