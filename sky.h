/*!
 * \file
 * \ingroup 	display
 * \brief	Displays the 3d objects
 */
#ifndef __SKY_H__
#define __SKY_H__

#ifdef SKY_FPV_CURSOR

void (*display_sky)(int);
void init_sky();

extern int show_moons ,show_sun,show_stars,horizon_fog,clouds1,clouds2,reflect_sky;
extern int clouds_tex;
extern int cloud_detail_tex;
extern int smokey_cloud_tex;
extern int moon_tex;
extern int sun_tex;
extern float sun_appears[4];
extern double LongView[16];
extern double time_d;
extern int show_sky;

#define CLOUDS_NONE	0
#define CLOUDS_THICK	1
#define FOG_COLOR	0
#define SKY_COLOR	1
#define CLOUDY_SKY	0
#define UNDERWORLD_SKY	1
#define INTERIORS_SKY	2


void cloud_layer1(int clouds);
void cloud_layer2(int clouds);
void sky_color(int sky);
void sky_type(int sky);

#endif /* SKY_FPV_CURSOR */

#endif /* __SKY_H_ */
