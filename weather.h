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

#ifdef NEW_WEATHER

#ifndef SKY_FPV
#error The NEW_WEATHER option requires the SKY_FPV option!
#endif

/* N E W   W E A T H E R *****************************************************/

#define MAX_WEATHER_TYPES 10 // including NONE
#define MAX_WEATHER_AREAS 10

extern float weather_color[];
extern float lightning_color[];
extern float lightning_ambient_color[];
extern float lightning_position[];
extern int lightning_falling;

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

#else // def NEW_WEATHER

/* O L D   W E A T H E R *****************************************************/

extern int seconds_till_rain_starts;    /*!< Seconds till the rain starts */
extern int seconds_till_rain_stops;     /*!< Seconts till the rain stops */
extern int is_raining;                  /*!< Specifies if it's raining - if it is, draw the raindrops */
#ifdef DEBUG
extern int num_rain_drops;
extern int last_rain_calls;
extern GLfloat rain_color[];
#endif // DEBUG
extern float rain_strength_bias;        /*!< Specifies the heaviness of the rain */
extern GLfloat fogColor[];              /*!< The current fog color. Calculated by \see render_fog */
extern float fogAlpha;                  /*!< Specifies how close the sky color shall be to the fog color */
extern int rain_sound;                  /*!< Specifies the rain sound */
extern int weather_light_offset;        /*!< Sets the current light offset */
extern int rain_light_offset;           /*!< Sets the current rain offset */
extern int thunder_light_offset;        /*!< Sets the current thunder light offset */
#ifdef SKY_FPV
extern float weather_rain_intensity; /*!< Rain intensity (between 0.0 and 1.0) */
#endif // SKY_FPV

void start_weather(int seconds_till_start, float severity);
void stop_weather(int seconds_till_stop, float severity);
void clear_weather();
void render_weather();

void get_weather_from_server(const Uint8* data);

#ifdef NEW_SOUND
float weather_adjust_gain(float in_gain, int in_cookie);
#endif // NEW_SOUND

/*!
 * \ingroup	other
 * \brief 	Builds the rain table.
 *
 * 		Builds the current rain table, called on init. Defines where the raindrops will go in the screen.
 *
 */
void build_rain_table();

/*!
 * \ingroup	weather_timer
 * \brief 	Updates the rain
 *
 * 		Updates the rain on the screen (moves the location a bit, it's just a 2d effect).
 *
 */
void update_rain();

/*!
 * \ingroup 	display_weather
 * \brief 	Renders the rain
 *
 *      	Draws the raindrops on the screen.
 *
 */
void render_rain();

/*!
 * \ingroup	weather_timer
 * \brief 	Check whether it needs to start raining
 *
 *      	The function checks every second if it should begin raining.
 *
 * \callgraph
 */
void rain_control();

/*!
 * \ingroup	weather_timer
 * \brief 	Controls the thunders
 *
 * 		Checks if it needs to start a thunder nearby.
 *
 * \callgraph
 */
void thunder_control();

/*!
 * \ingroup	display_weather
 * \brief	Gets the current light level for the weather
 *
 * 		Gets the current light level for the weather.
 *
 */
void get_weather_light_level();

/*!
 * \ingroup	other
 * \brief 	Reset the thunders
 *
 *      	Resets the thunders.
 *
 */
void clear_thunders();

void init_weather();

/*!
 * \ingroup	network_misc
 * \brief 	add_thunder
 *
 *		Adds a thunder of the specified type with the given sound_delay.
 * 
 * \param   	type
 * \param   	sound_delay
 *
 * \todo Fix documentation
 */
void add_thunder(int type,int sound_delay);

/*!
 * \ingroup display_weather
 * \brief Sets the fog according to weather & athmospherical effects
 *
 * 	Sets the fog according to weather & athmospherical effects
 */

void render_fog();

#endif	// NEW_WEATHER

#ifdef __cplusplus
} // extern "C"
#endif

#endif
