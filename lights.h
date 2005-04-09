/*!
 * \file
 * \ingroup lights
 * \brief light and sun handling
 */
#ifndef __LIGHTS_H__
#define __LIGHTS_H__

/*!
 * a light structure stores the position and color of a light
 */
typedef struct
{
    /*!
     * \name light position
     */
    /*! @{ */
  	float pos_x;
	float pos_y;
	float pos_z;
    /*! @} */
    
    /*!
     * \name light color
     */
    /*! @{ */
	float r;
	float g;
	float b;
    /*! @} */
}light;

//for the lights

/*!
 * \name Lights limits
 */
/*! @{ */
#define GLOBAL_LIGHTS_NO 60
/*! @} */

/*! \name Sky lights arrays 
/ * @{ */
extern GLfloat sky_lights_c1[GLOBAL_LIGHTS_NO*2][4];
extern GLfloat sky_lights_c2[GLOBAL_LIGHTS_NO*2][4];
extern GLfloat sky_lights_c3[GLOBAL_LIGHTS_NO*2][4];
extern GLfloat sky_lights_c4[GLOBAL_LIGHTS_NO*2][4];
/*! @} */

extern GLfloat sun_ambient_light[];

/*!
 * \name Lights limits
 */
/*! @{ */
#define MAX_LIGHTS 1000
/*! @} */

extern light *lights_list[MAX_LIGHTS]; /*!< global lights list */

extern unsigned char light_level; /*!< the light level */
extern short game_minute; /*!< the current game minute */

/*!
 * \ingroup lights
 * \brief
 *
 *      Detail
 *
 */
void disable_local_lights();

/*!
 * \ingroup lights
 * \brief
 *
 *      Detail
 *
 * \sa draw_scene
 */
void draw_lights();

/*!
 * \ingroup lights
 * \brief
 *
 *      Detail
 *
 * \param x
 * \param y
 * \param z
 * \param r
 * \param g
 * \param b
 * \param intensity
 * \retval int
 */
int add_light(GLfloat x, GLfloat y, GLfloat z, GLfloat r, GLfloat g, GLfloat b, GLfloat intensity);

/*!
 * \ingroup lights
 * \brief
 *
 *      Detail
 *
 * \sa draw_scene
 */
void update_scene_lights();

/*!
 * \ingroup other
 * \brief
 *
 *      Detail
 *
 * \sa init_stuff
 * \sa set_new_video_mode
 */
void init_lights();

/*!
 * \ingroup lights
 * \brief
 *
 *      Detail
 *
 */
void reset_material();

/*!
 * \ingroup lights
 * \brief
 *
 *      Detail
 *
 * \param r
 * \param g
 * \param b
 */
void set_material(float r, float g, float b);

/*!
 * \ingroup lights
 * \brief
 *
 *      Detail
 *
 * \sa draw_scene
 */
void draw_global_light();

/*!
 * \ingroup lights
 * \brief
 *
 *      Detail
 *
 * \sa draw_scene
 */
void draw_dungeon_light();

/*!
 * \ingroup other
 * \brief
 *
 *      Detail
 *
 * \callgraph
 */
void build_global_light_table();

/*!
 * \ingroup other
 * \brief
 *
 *      Detail
 *
 * \sa init_stuff
 */
void build_sun_pos_table();

/*!
 * \ingroup event_handle
 * \brief
 *
 *      Detail
 *
 * \callgraph
 */
void new_minute();

#endif
