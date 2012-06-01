/*!
 * \file
 * \ingroup     display
 * \brief 	Displays the tiles
 */
#ifndef __TILE_H__
#define __TILE_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MAP_EDITOR2
typedef struct
{
	char * img;
	int x;
	int y;
} img_struct;

extern img_struct map_tiles[256];
#endif

extern unsigned char *tile_map;     /*!< The tile-map is an unsigned char array of map_tile_size_x*map_tile_size_y */
extern unsigned char *height_map;   /*!< The height-map is an unsigned char array of (map_tile_size_x*6)*(map_tile_size_y*6) - each tile has 36 heightmap blocks */
extern int tile_map_size_x;         /*!< The tile map size in the x direction */
extern int tile_map_size_y;         /*!< The tile map size in the y direction */
extern int tile_list[256];          /*!< A list containing the texture ID's of the different tiles */
extern int ground_detail_text;      /*!< The texture for ground details (clouds shadows) */

/*!
 * \ingroup	tile
 * \brief 	Displays the tile map.
 *
 *      draw_tile_map
 *
 * \callgraph
 */
void draw_tile_map();

#ifdef	NEW_TEXTURES
/*!
 * \ingroup	tile
 * \brief 	Draw quad tiles.
 *
 *      draw_quad_tiles
 *
 * \callgraph
 */
void draw_quad_tiles(const unsigned int start, const unsigned int stop,
	unsigned int idx, const unsigned int zero_id);
#endif	/* NEW_TEXTURES */

/*!
 * \ingroup 	maps
 * \brief 	Loads the map tiles
 *
 *      	Loads the map tiles - tile_list[tile]==0 && tile!=255 it will load the tile (from tiles/tile\<tile_id\>.bmp).
 *
 * \callgraph
 */
void load_map_tiles();

#ifdef NEW_SOUND
/*!
 * \ingroup 	tile
 * \brief 	Returns the tile for input coordinates
 *
 *      	Return the type of tile for \a x and \a y.
 *
 * \param x						X coordinate
 * \param y						Y coordinate
 * \callgraph
 */
int get_tile_type(int x, int y);
#endif // NEW_SOUND

/*!
 * \ingroup 	tile
 * \brief 	Returns the height at the given position.
 *
 *      	Returns the height at the given position \a x and \a y.
 *
 * \param x						X coordinate
 * \param y						Y coordinate
 * \callgraph
 */
float get_tile_height(const float x, const float y);

/*!
 * \ingroup 	tile
 * \brief 	Returns if the given position is walkable
 *
 *      	Returns if the given position is walkable at \a x and \a y.
 *
 * \param x						X coordinate
 * \param y						Y coordinate
 * \callgraph
 */
int get_tile_walkable(const int x, const int y);

/*!
 * \ingroup 	tile
 * \brief 	Returns if the given position is valid
 *
 *      	Returns if the given position is valid at \a x and \a y.
 *
 * \param x						X coordinate
 * \param y						Y coordinate
 * \callgraph
 */
int get_tile_valid(const int x, const int y);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
