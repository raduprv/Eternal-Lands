/*!
 * \file
 * \ingroup lights
 * \brief Light and sun handling.
 *
 *      This file contains datatypes and functions to use lights and the sun with the client.
 */
#ifndef __LIGHTS_H__
#define __LIGHTS_H__

#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif


/*!
 * A light structure stores the position and color of a light
 */
typedef struct
{
    /*!
     * \name Light position
     */
    /*! @{ */
  	float pos_x;
	float pos_y;
	float pos_z;
    /*! @} */
    
    /*!
     * \name Light color
     */
    /*! @{ */
	float r;
	float g;
	float b;
    /*! @} */
#ifdef MAP_EDITOR2
	int locked;
#endif
#ifdef CLUSTER_INSIDES
	short cluster;
#endif
}light;

/*! \name Lights limits */
/*! @{ */
#define GLOBAL_LIGHTS_NO 60 /*!< The maximum number of global lights to use */
#define MAX_LIGHTS 1000     /*!< The maximum amount of lights (global and local) */
/*! @} */

/*! \name Sky lights arrays */
/*! @{ */
extern GLfloat sky_lights_c1[GLOBAL_LIGHTS_NO*2][4];
extern GLfloat sky_lights_c2[GLOBAL_LIGHTS_NO*2][4];
extern GLfloat sky_lights_c3[GLOBAL_LIGHTS_NO*2][4];
extern GLfloat sky_lights_c4[GLOBAL_LIGHTS_NO*2][4];
/*! @} */

extern GLfloat ambient_light[]; /*!< An array for the ambient lights radiating from the sun */
extern GLfloat diffuse_light[]; /*!< An array for the diffuse light portion */

extern int	show_lights;	/*! the highest numbered light in the current GL display (0-6) */
extern int	num_lights; /*! the number of lights currently loaded */
extern light *lights_list[MAX_LIGHTS]; /*!< global lights list */

extern unsigned char light_level; /*!< the light level */
extern short game_minute; /*!< the current game minute */
extern short game_second; /*!< the current game second */
extern unsigned char freeze_time; /*!< when this value is equal to 1, the game minute is freezed */

/*!
 * \ingroup lights
 * \brief   Disables all local lightning.
 *
 *      Disables all local lightning.
 *
 */
void disable_local_lights();

/*!
 * \ingroup lights
 * \brief   Draws the default local lights.
 *
 *      Draws all default local lights. The lights must be enabled first.
 *
 * \sa draw_scene
 * \sa enable_local_lights
 */
void draw_lights();

/*
 * \ingroup	lights
 * \brief	Destroys the light at position i in the lights_list
 * 
 * 		Destroyes the light on position i in the lights_list - frees the memory and sets the lights_list[i]=NULL.
 *
 * \param	i The position in the lights_list
 *
 * \callgraph
 */
void destroy_light(int i);

/*!
 * \ingroup lights
 * \brief   Adds a new light using the given position and color.
 *
 *      Adds a new light.using the given position (\a x, \a y, \a z) and the color (\a r, \a g, \a b, \a intensity) to the global \ref lights_list.
 *
 * \param x             x coordinate of the lighs position
 * \param y             y coordinate of the lighs position
 * \param z             z coordinate of the lighs position
 * \param r             r (red) value of the lights color
 * \param g             g (green) value of the lights color
 * \param b             b (blue) value of the lights color
 * \param intensity     a (intensity) value of the lights color
 * \retval int          the index into the \ref lights_list array, where the light was added.
 */
#if defined (MAP_EDITOR2) || defined (MAP_EDITOR)
int add_light(GLfloat x, GLfloat y, GLfloat z, GLfloat r, GLfloat g, GLfloat b, GLfloat intensity, int locked, unsigned int dynamic);
#else
int add_light(GLfloat x, GLfloat y, GLfloat z, GLfloat r, GLfloat g, GLfloat b, GLfloat intensity, unsigned int dynamic);
#endif

/*!
 * \ingroup lights
 * \brief   Gets the lights visible in the scene.
 *
 *      Gets the lights visible in the scene.
 *
 * \sa draw_scene
 */
void update_scene_lights();

/*!
 * \ingroup other
 * \brief   Initializes the default lights and enables lighting.
 *
 *      Initializes the default lights and enables lighting.
 *
 * \sa init_stuff
 * \sa set_new_video_mode
 */
void init_lights();

/*!
 * \ingroup lights
 * \brief   Resets the material attributes to its default values.
 *
 *      Resets the material attributes to the default values.
 *
 */
void reset_material();

/*!
 * \ingroup lights
 * \brief   Sets the material attributes to the given color.
 *
 *      Sets the material attributes to the given color (\a r, \a g, \a b).
 *
 * \param r     red component of the material color
 * \param g     green component of the material color
 * \param b     blue component of the material color
 */
void set_material(float r, float g, float b);

/*!
 * \ingroup lights
 * \brief   Draws the global light.
 *
 *      Draws the global light, i.e. the sun and the diffuse light.
 *
 * \sa draw_scene
 */
void draw_global_light();

/*!
 * \ingroup lights
 * \brief   Draws the ambient light of dungeons.
 *
 *      Draws the ambient light of dungeons.
 *
 * \sa draw_scene
 */
void draw_dungeon_light();

/*!
 * \ingroup other
 * \brief   Initializies the global lights for the sun and lakes.
 *
 *      Initializes the global lights for the sun and lakes using \ref make_gradient_light.
 *
 */
void build_global_light_table();

/*!
 * \ingroup other
 * \brief   Computes the table for the sun positions.
 *
 *      Computes the global table of sun positions.
 *
 * \sa init_stuff
 */
void build_sun_pos_table();

/*!
 * \ingroup event_handle
 * \brief   Sets the \ref light_level depending on the current \ref game_minute
 *
 *      Sets the \ref light_level depending on the current \ref game_minute and adjusts the light.
 *
 * \callgraph
 */
void new_minute();

void new_second();

void cleanup_lights(void);


#ifdef __cplusplus
} // extern "C"
#endif

#endif
