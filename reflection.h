/*!
 * \file
 * \ingroup reflections
 * \brief handles the reflection of reflective surfaces
 */
#ifndef __REFLECTION_H__
#define __REFLECTION_H__

extern int lake_waves_timer;
extern float water_movement_u; /*!< movement of the water in u direction */
extern float water_movement_v; /*!< movement of the water in v direction */

/*!
 * defines whether a tile is a water tile or not
 */
#define IS_WATER_TILE(i) (!i || (i>230 && i<255))

/*!
 * The following macro tests if a _water tile_ is reflecting
 */
#define IS_REFLECTING(i) (i<240)

/*!
 * \ingroup reflections
 * \brief Finds all reflections on the current map
 *
 *      Finds all reflections on the current map.
 *
 * \retval int
 * \callgraph
 */
int find_reflection();

/*!
 * \ingroup reflections
 * \brief Displays all reflections caused by 3d objects.
 *
 *      Displays all reflections caused by 3d objects.
 *
 * \callgraph
 */
void display_3d_reflection();

/*!
 * \ingroup reflections
 * \brief Adds noise to the water of lakes.
 *
 *      Adds noise to the water of lakes to make them look more realistic.
 *
 * \sa mrandom
 */
void make_lake_water_noise();

/*!
 * \ingroup reflections
 * \brief Draws the tiles of all lakes on the map
 *
 *      Draws all the tiles of all lakes on the current map
 *
 * \callgraph
 */
void draw_lake_tiles();

/*!
 * \ingroup reflections
 * \brief Draws the sky background in open areas
 *
 *      Draws the sky background in open areas
 *
 * \callgraph
 */
void draw_sky_background();

/*!
 * \ingroup reflections
 * \brief Draws the sky background in dungeons
 *
 *      Draws the sky background in dungeons
 *
 * \callgraph
 */
void draw_dungeon_sky_background();

#endif
