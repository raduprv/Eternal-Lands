/*!
 * \file
 * \ingroup	misc
 * \brief 	Structures and functions for the EL weather
 */
#ifndef __WEATHER_H__
#define __WEATHER_H__

#include <SDL_types.h>
#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int use_fog;			/*!< Whether we are using fog or not */



/* N E W   W E A T H E R *****************************************************/

#define MAX_WEATHER_TYPES 10 // including NONE
#define MAX_WEATHER_AREAS 10

extern float weather_color[];
extern float lightning_color[];
extern float lightning_ambient_color[];
extern float lightning_position[];
extern int lightning_falling;
extern int show_weather;

void weather_init();
void weather_clear();
void weather_set_area(int area, float x, float y, float radius, int type, float intensity, int change_duration);
void weather_get_from_server(const Uint8* data);

void weather_compute_ratios(float ratios[MAX_WEATHER_TYPES], float x, float y);
void weather_update();
void weather_render_fog();
void weather_render();

int weather_get_drops_count(int type);
float weather_get_intensity();
float weather_get_density();
float weather_get_density_from_ratios(float ratios[MAX_WEATHER_TYPES]);
void weather_get_color_from_ratios(float color[4], float ratios[MAX_WEATHER_TYPES]);

void weather_add_lightning(int type, float x, float y);
void weather_init_lightning_light();
void weather_render_lightning();
float weather_get_lightning_intensity(float x, float y);

void weather_sound_control();

#ifdef NEW_SOUND
float weather_adjust_gain(float in_gain, int in_cookie);
#endif // NEW_SOUND


#ifdef __cplusplus
} // extern "C"
#endif

#endif
