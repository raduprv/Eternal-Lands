/*!
 * \file
 * \brief 	Displays the tiles
 * \ingroup     display
 */
#ifndef __TILE_H__
#define __TILE_H__


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
 *      TODO: draw_tile_map
 *
 * \return  None
 */
void draw_tile_map();

/*!
 * \ingroup 	maps
 * \brief 	Loads the map tiles
 *
 *      	Loads the map tiles - tile_list[tile]==0 && tile!=255 it will load the tile (from tiles/tile\<tile_id\>.bmp).
 *
 * \return  	None
 */
void load_map_tiles();
#endif

