/*!
 * \file
 * \brief Displays the tiles
 * \ingroup     render
 * \internal    Check groups!
 */
#ifndef __TILE_H__
#define __TILE_H__


extern unsigned char *tile_map;     /*!< tile_map */
extern unsigned char *height_map;   /*!< height_map */
extern int tile_map_size_x;         /*!< tile_map_size_x */
extern int tile_map_size_y;         /*!< tile_map_size_y */
extern int tile_list[256];          /*!< tile_list[] */
extern int ground_detail_text;      /*!< ground_detail_text */

/*!
 * \brief draw_tile_map
 *
 *      TODO: draw_tile_map
 *
 * \param   None
 * \return  None
 */
void draw_tile_map();

/*!
 * \internal check group!
 * \ingroup load
 * \brief load_map_tiles
 *
 *      TODO: load_map_tiles
 *
 * \param   None
 * \return  None
 */
void load_map_tiles();
#endif

