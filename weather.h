/*!
 * \file
 * \internal Check groups!
 * \brief Structures and functions for the EL weather
 */
#ifndef __WEATHER_H__
#define __WEATHER_H__

/*!
 * \name    ???max number of thunders???
 */
/*! \{ */
#define MAX_THUNDERS 5
/*! \} */

/*!
 * \name    ???rain related constants???
 */
/*! \{ */
#define MAX_RAIN_DROPS 5000 /*!< MAX_RAIN_DROPS */
#define RAIN_SPEED 2        /*!< RAIN_SPEED */
#define rain_drop_len 5     /*!< rain_drop_len */
/*! \} */


extern int seconds_till_rain_starts;    /*!< seconds_till_rain_starts */
extern int seconds_till_rain_stops;     /*!< seconds_till_rain_stops */
extern int is_raining;                  /*!< is_raining */
extern int rain_sound;                  /*!< rain_sound */
extern int weather_light_offset;        /*!< weather_light_offset */
extern int rain_light_offset;           /*!< rain_light_offfset */
extern int thunder_light_offset;        /*!< thunder_light_offset */
extern int lightning_text;              /*!< lightning_text */


/*!
 * TODO: struct thunder
 */
typedef struct
{
	int in_use;
	int light_offset;
	int seconds_till_sound;
	int thunder_type;
	int time_since_started;
	int x;
	int y;
	int rot;
}thunder;

thunder thunders[MAX_THUNDERS];         /*!< thunders[] */

/*!
 *  TODO: struct rain_drop
 */
typedef struct
{
	short x;
	short y;
	short x2;
	short y2;
}rain_drop;

rain_drop rain_drops[MAX_RAIN_DROPS];   /*!< rain_drops[] */

/*!
 * \internal check groups!
 * \brief build_rain_table
 *
 *      TODO: build_rain_table
 *
 * \param   None
 * \return  None
 */
void build_rain_table();

/*!
 * \internal check groups!
 * \brief update_rain
 *
 *      TODO: update_rain
 *
 * \param   None
 * \return  None
 */
void update_rain();

/*!
 * \internal check groups!
 * \ingroup render
 * \brief render_rain
 *
 *      TODO: render_rain
 *
 * \param   None
 * \return  None
 */
void render_rain();

/*!
 * \internal check groups!
 * \brief rain_control
 *
 *      TODO: rain_control
 *
 * \param   None
 * \return  None
 */
void rain_control();

/*!
 * \internal check groups!
 * \brief thunder_control
 *
 *      TODO: thunder_control
 *
 * \param   None
 * \return  None
 */
void thunder_control();

/*!
 * \internal check groups!
 * \brief add_thunder
 *
 *      TODO: add_thunder
 *
 * \param   type
 * \param   sound_delay
 * \return  None
 */
void add_thunder(int type,int sound_delay);

/*!
 * \internal check groups!
 * \brief get_weather_light_level
 *
 *      TODO: get_weather_light_level
 *
 * \param   None
 * \return  None
 */
void get_weather_light_level();

/*!
 * \internal check groups!
 * \brief clear_thunders
 *
 *      TODO: clear_thunders
 *
 * \param   None
 * \return  None
 */
void clear_thunders();
#endif

