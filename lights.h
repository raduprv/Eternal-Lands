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

/*
 * OBSOLETE: Queued for removal from this file.
 * Used only in lights.c, no need to declare it here.
 */
//*!
// * a position for the sun
// */
//typedef struct
//{
//	float x; /*<! x coordinate of the suns position */
//	float y; /*<! y coordinate of the suns position */
//	float z; /*<! z coordinate of the suns position */
//	float w; /*<! w coordinate of the suns position */
//}sun;

/*
 * OBSOLETE: Queued for removal from this file.
 * Used only in lights.c, no need to declare them here.
extern GLfloat light_0_position[4];
extern GLfloat light_0_diffuse[4];
extern GLfloat light_0_dist;

extern GLfloat light_1_position[4];
extern GLfloat light_1_diffuse[4];
extern GLfloat light_1_dist;

extern GLfloat light_2_position[4];
extern GLfloat light_2_diffuse[4];
extern GLfloat light_2_dist;

extern GLfloat light_3_position[4];
extern GLfloat light_3_diffuse[4];
extern GLfloat light_3_dist;

extern GLfloat light_4_position[4];
extern GLfloat light_4_diffuse[4];
extern GLfloat light_4_dist;

extern GLfloat light_5_position[4];
extern GLfloat light_5_diffuse[4];
extern GLfloat light_5_dist;

extern GLfloat light_6_position[4];
extern GLfloat light_6_diffuse[4];
extern GLfloat light_6_dist;
*/

//for the lights

/*!
 * \name Lights limits
 */
/*! @{ */
#define GLOBAL_LIGHTS_NO 60
/*! @} */

/*
 * OBSOLETE: Queued for removal from this file.
 * Used only in lights.c, no need to declare it here.
 */
//extern GLfloat global_lights[GLOBAL_LIGHTS_NO][4]; /*!< stores the lights that are global used */

/*! \name Sky lights arrays 
/ * @{ */
extern GLfloat sky_lights_c1[GLOBAL_LIGHTS_NO*2][4];
extern GLfloat sky_lights_c2[GLOBAL_LIGHTS_NO*2][4];
extern GLfloat sky_lights_c3[GLOBAL_LIGHTS_NO*2][4];
extern GLfloat sky_lights_c4[GLOBAL_LIGHTS_NO*2][4];
/*! @} */

/*
 * OBSOLETE: Queued for removal from this file.
 * Used only in lights.c, no need to declare it here.
 */
//extern sun sun_pos[60*3]; /*!< an array of sun positions */

extern GLfloat sun_ambient_light[];

/*
 * OBSOLETE: Queued for removal from this file.
 * Used only in lights.c, no need to declare it here.
 */
//extern int sun_use_static_position; /*!< flag that indicates whether the sun should stay on a static position or move over the sky */

/*!
 * \name Lights limits
 */
/*! @{ */
#define MAX_LIGHTS 1000
/*! @} */

extern light *lights_list[MAX_LIGHTS]; /*!< global lights list */

/*
 * OBSOLETE: Queued for removal from this file.
 * Unused variable
 */
//extern char lights_on; /*!< this indicates whether lights are on or not */

extern unsigned char light_level; /*!< the light level */
extern short game_minute; /*!< the current game minute */

/*!
 * \ingroup lights
 * \brief
 *
 *      Detail
 *
 * \todo Check this. It seems not get called from anywhere.
 */
void draw_test_light();

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
 * \sa new_minute
 */
void enable_local_lights();

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
 * \ingroup lights
 * \brief
 *
 *      Detail
 *
 * \param start
 * \param steps
 * \param light_table
 * \param r_start
 * \param g_start
 * \param b_start
 * \param r_end
 * \param g_end
 * \param b_end
 */
void make_gradient_light(int start,int steps,float *light_table, float r_start, 
						 float g_start, float b_start, float r_end, float g_end, float b_end);

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
