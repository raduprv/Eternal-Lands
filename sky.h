#ifndef __SKY_H__
#define __SKY_H__

#ifdef SKY_FPV

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
extern float skybox_fog_color[4];
extern float skybox_fog_density;
extern float skybox_sunny_sky_bias;
extern float skybox_sunny_clouds_bias;
extern float skybox_sunny_fog_bias;

typedef enum {
    SKYBOX_NONE = 0,
    SKYBOX_CLOUDY = 1,
    SKYBOX_UNDERWORLD = 2
} skybox_type;

void blend_colors(float result[], float orig[], float dest[], float t, int size);
void blend_color_tables(float result[], float orig_table[360][4], float dest_table[360][4], float t, int size);

void skybox_compute_height();
float skybox_get_height();

void skybox_set_type(skybox_type sky);
void skybox_display();

void skybox_init_gl();
void skybox_init_defs(const char *map_name);

void skybox_update_positions();
void skybox_update_colors();

#endif // SKY_FPV

#endif // __SKY_H__
