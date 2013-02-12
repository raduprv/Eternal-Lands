#ifndef __ITEM_INFO_H
#define __ITEM_INFO_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @ingroup item_info
 * @brief get the item description from the lookup table
 *
 * @param item_id the item unique id
 * @param image_id the item image id
 * @callgraph
 */
const char *get_item_description(Uint16 item_id, int image_id);

/**
 * @ingroup item_info
 * @brief get the item emu from the lookup table
 *
 * @param item_id the item unique id
 * @param image_id the item image id
 * @callgraph
 */
int get_item_emu(Uint16 item_id, int image_id);

/**
 * @ingroup item_info
 * @brief get the number of items that match these ids
 *
 * @param item_id the item unique id
 * @param image_id the item image id
 * @return Returns the number of matching items
 * 
 * @callgraph
 */
int get_item_count(Uint16 item_id, int image_id);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
