/**
 * @file
 * @ingroup reflections
 * @brief handles the reflection of reflective surfaces
 */
#ifndef __REFLECTION_H__
#define __REFLECTION_H__

#ifdef __cplusplus
extern "C" {
#endif

extern float water_movement_u; /**< movement of the water in u direction */
extern float water_movement_v; /**< movement of the water in v direction */
extern int water_shader_quality; /**< quality of the shader used for drawing water. Zero means no shader. */

/**
 * defines whether a tile is a water tile or not
 */
#define IS_WATER_TILE(i) (!i || (i>230 && i<255))

/**
 * The following macro tests if a _water tile_ is reflecting
 */
#define IS_REFLECTING(i) (i<240)

/**
 * @ingroup reflections
 * @brief Finds all reflections on the current map
 *
 *      Finds all reflections on the current map.
 *
 * @retval int
 * @callgraph
 */
int find_reflection();

/**
 * @ingroup reflections
 * @brief Displays all reflections caused by 3d objects.
 *
 *      Displays all reflections caused by 3d objects.
 *
 * @callgraph
 */
void display_3d_reflection();

/**
 * @ingroup reflections
 * @brief Adds noise to the water of lakes.
 *
 *      Adds noise to the water of lakes to make them look more realistic.
 *
 * @sa mrandom
 */
void make_lake_water_noise();

/**
 * @ingroup reflections
 * @brief Blends the fog into reflections
 *
 *      Blends the fog into reflections
 *
 * @callgraph
 */
void blend_reflection_fog();

/**
 * @ingroup reflections
 * @brief Draws the tiles of all lakes on the map
 *
 *      Draws all the tiles of all lakes on the current map
 *
 * @callgraph
 */
void draw_lake_tiles();

/**
 * @ingroup reflections
 * @brief Draws the sky background in open areas
 *
 *      Draws the sky background in open areas
 *
 * @callgraph
 */
void draw_sky_background();

/**
 * @ingroup reflections
 * @brief Draws the sky background in dungeons
 *
 *      Draws the sky background in dungeons
 *
 * @callgraph
 */
void draw_dungeon_sky_background();

/**
 * @ingroup reflections
 * @brief Draws the water background
 *
 *      Draws the water background
 *
 * @callgraph
 */
void draw_water_background();

/**
 * @ingroup reflections
 * @brief Draws a lake water tile
 *
 * 	Draws a lake water tile
 *
 * @param x_pos The x position
 * @param y_pos The y position
 */
void draw_lake_water_tile(float x_pos, float y_pos);

/**
 * @ingroup reflections
 * @brief Frees the reflection frame buffer.
 *
 * Frees the reflection frame buffer.
 *
 * @callgraph
 */
void free_reflection_framebuffer();

/**
 * @ingroup reflections
 * @brief Makes the reflection frame buffer.
 *
 * Makes the reflection frame buffer.
 * 
 * @param width The new width.
 * @param height The new height.
 * @callgraph
 */
void make_reflection_framebuffer(int width, int height);

/**
 * @ingroup reflections
 * @brief Changes the size of the reflection frame buffer.
 *
 * Changes the size of the reflection frame buffer.
 *
 * @param width The new width.
 * @param height The new height.
 * @callgraph
 */
void change_reflection_framebuffer_size(int width, int height);

/**
 * @ingroup reflections
 * @brief Inits the buffer used for water.
 *
 * Inits the buffer used for water (reflectiv and non reflectiv). Must be called every time map
 * size increase, but also should be called every time time map size decrease.
 *
 * @param water_buffer_size The new size of the buffer in number of elements.
 * @callgraph
 */
void init_water_buffers(int water_buffer_size);

/**
 * @ingroup reflections
 * @brief Gets the maximum quality for water rendering supported by the hardware.
 *
 * Gets the maximum quality for water rendering supported by the hardware.
 * @callgraph
 */
int get_max_supported_water_shader_quality();

#ifdef __cplusplus
} // extern "C"
#endif

#endif
