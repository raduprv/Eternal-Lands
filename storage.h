/*!
 * \file
 * \ingroup storage_window
 * \brief The new storage interface
 */
#ifndef __STORAGE_H__
#define __STORAGE_H__

#include "bags.h"

#ifdef __cplusplus
extern "C" {
#endif

#define STORAGE_ITEMS_SIZE 300

/*! \name window handler
 * @{ */
extern int storage_win; /*!< window handler for the new storage window */
/*! @} */
extern int storage_item_dragged;
extern ground_item storage_items[STORAGE_ITEMS_SIZE]; /*!< list of storage items. */

extern int storage_win_x;
extern int storage_win_y;
extern int view_only_storage;
extern int sort_storage_categories;
extern Uint32 drop_fail_time;

/*!
 * \ingroup storage_window
 * \brief Gets the storage categories from the server data \a in_data.
 *
 *      Gets the storage categories from the server data \a in_data and stores them for further usage.
 *
 * \param in_data   the data from the server containing all the storage categories
 * \param len       unused
 *
 * \callgraph
 */
void get_storage_categories (const char * in_data, int len);

/*!
 * \ingroup storage_window
 * \brief Gets or updates the storage items from the server data \a in_data
 *
 *      Gets or updates the storage items from the server data \a in_data and stores them for further usage. If the fist byte of \a in_data equals 255, the function will perform an update of the items, else a new list of items will be initialized.
 *
 * \param in_data   the data from the server containing the storage items
 * \param len       unused
 *
 * \callgraph
 */
void get_storage_items (const Uint8 * in_data, int len);

/*!
 * \ingroup storage_window
 * \brief Gets the storage text form the server data \a in_data up to the length \a len.
 *
 *      Gets the storage text from the server data \a in_data up to a length of \a len. If len is >200 it will be adjusted accordingly.
 *
 * \param in_data   the data from the server containing the storage text
 * \param len       the maximum length of \a in_data
 *
 * \pre If \a len is >200 it will be adjusted to be equal to 200.
 */
void get_storage_text (const Uint8 * in_data, int len);

/*!
 * \ingroup storage_window
 * \brief Closes the storage window
 *
 * 	Closes the storage window.
 */
void close_storagewin();

/*!
 * \ingroup storage_window
 * \brief Displays the new storage window
 *
 *      Displays the new storage window. If the window hasn't been created before it will first created.
 *
 * \callgraph
 */
void display_storage_menu();

/*!
 * \ingroup storage_window
 * \brief Pick-up the specified item.
 *
 *      Move the the specified category and pick-up the specified item if it is present.
 *
 * \param  image_id		the image id of the requested object
 * \param  item_id		the unique id of the object, or unset_item_uid is not available
 * \param  cat_id		the storage category id in which the find the object
 * 
 * \callgraph
 */
void pickup_storage_item(int image_id, Uint16 item_id, int cat_id);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
