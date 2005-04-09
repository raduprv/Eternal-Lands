/*!
 * \file
 * \ingroup	misc
 * \brief 	Structures and functions for the EL weather
 */
#ifndef __WEATHER_H__
#define __WEATHER_H__

extern int seconds_till_rain_starts;    /*!< Seconds till the rain starts */
extern int seconds_till_rain_stops;     /*!< Seconts till the rain stops */
extern int is_raining;                  /*!< Specifies if it's raining - if it is, draw the raindrops */
extern int rain_sound;                  /*!< Specifies the rain sound */
extern int weather_light_offset;        /*!< Sets the current light offset */
extern int rain_light_offset;           /*!< Sets the current rain offset */
extern int thunder_light_offset;        /*!< Sets the current thunder light offset */

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
 * \ingroup	network_misc
 * \brief 	add_thunder
 *
 *		Adds a thunder of the specified type with the given sound_delay.
 * 
 * \param   	type
 * \param   	sound_delay
 */
void add_thunder(int type,int sound_delay);

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
#endif
