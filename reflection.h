/*!
 * \file
 * \ingroup reflections
 * \brief handles the reflection of reflective surfaces
 */
#ifndef __REFLECTION_H__
#define __REFLECTION_H__

/*
 * OBSOLETE: Queued for removal from this file.
 * Only used in reflection.c, no need to declare it here.
 */
//*!
// * a water_vertex
// */
//typedef struct
//{
//	float u; /*!< u coordinate */
//	float v; /*!< v coordinate */
//	float z; /*!< z coordinage */
//
//}water_vertex;

/*
 * OBSOLETE: Queued for removal from this file.
 * Only used in reflection.c, no need to declare them here.
 */
//extern water_vertex noise_array[16*16]; /*!< an array of noise values for water reflections */
//extern int sky_text_1;
//extern float water_deepth_offset;

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
 * \ingroup misc_utils
 * \brief mrandom
 *
 *      mrandom(float)
 *
 * \param max
 * \retval float
 */
float mrandom(float max);

/*!
 * \ingroup reflections
 * \brief draws the reflections cause by the specified body parts.
 *
 *      Draws the reflections caused by the specified body parts.
 *
 * \param model_data    the md2 model data used to calculate the reflections
 * \param cur_frame     the current frame
 * \param ghost         specifies wheter the body part is a ghost or not
 */
void draw_body_part_reflection(md2 *model_data,char *cur_frame, int ghost);

/*!
 * \ingroup reflections
 * \brief draws the reflection caused by the specified actor
 *
 *      Draws the reflection caused by the specified actor
 *
 * \param actor_id  the actor for which the reflections are drawn
 *
 * \callgraph
 */
void draw_actor_reflection(actor * actor_id);

/*!
 * \ingroup reflections
 * \brief draws the reflection caused by the specified actor
 *
 *      Draws the reflection caused by the specified actor
 *
 * \param actor_id  the actor for which the reflections are drawn.
 *
 * \callgraph
 */
void draw_enhanced_actor_reflection(actor * actor_id);

/*!
 * \ingroup reflections
 * \brief draws reflections caused by the given object3d
 *
 *      Draws reflections caused by the given object3d
 *
 * \param object_id the 3d object for which reflections are drawn.
 *
 * \callgraph
 */
void draw_3d_reflection(object3d * object_id);

/*!
 * \ingroup reflections
 * \brief finds all reflections on the current map
 *
 *      Finds all reflections on the current map.
 *
 * \retval int
 * \callgraph
 */
int find_reflection();

/*!
 * \ingroup reflections
 * \brief finds reflections from the given position up to a specified range.
 *
 *      Finds reflections from the given position up to a specified range.
 *
 * \param x_pos the x coordinate of the position
 * \param y_pos the y coordinate of the position
 * \param range the range up to which reflections should be searched for.
 * \retval int
 */
int find_local_reflection(int x_pos,int y_pos,int range);

/*!
 * \ingroup reflections
 * \brief displays all reflections caused by 3d objects.
 *
 *      Displays all reflections caused by 3d objects.
 *
 * \callgraph
 */
void display_3d_reflection();

/*!
 * \ingroup reflections
 * \brief adds noise to the water of lakes.
 *
 *      Adds noise to the water of lakes to make them look more realistic.
 *
 * \sa mrandom
 */
void make_lake_water_noise();

/*!
 * \ingroup reflections
 * \brief draws a tile at the given position as a lake water tile.
 *
 *      Draws a tile at the given position as a lake water tile.
 *
 * \param x_pos the x coordinate of the position
 * \param y_pos the y coordinate of the position
 */
void draw_lake_water_tile(float x_pos, float y_pos);

/*!
 * \ingroup reflections
 * \brief draws the tiles of all lakes on the map
 *
 *      Draws all the tiles of all lakes on the current map
 *
 * \callgraph
 */
void draw_lake_tiles();

/*!
 * \ingroup reflections
 * \brief draws the sky background in open areas
 *
 *      Draws the sky background in open areas
 *
 * \callgraph
 */
void draw_sky_background();

/*!
 * \ingroup reflections
 * \brief draws the sky background in dungeons
 *
 *      Draws the sky background in dungeons
 *
 * \callgraph
 */
void draw_dungeon_sky_background();

#endif
