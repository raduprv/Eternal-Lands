/*!
 * \file
 * \ingroup storage_window
 * \brief The new storage interface
 */
#ifndef __STORAGE_H__
#define __STORAGE_H__

/*! \name window handler
 * @{ */
extern int storage_win; /*!< window handler for the new storage window */
/*! @} */
extern int storage_item_dragged;
extern ground_item storage_items[200]; /*!< list of storage items. */

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
void get_storage_categories(char * in_data, int len);

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
void get_storage_items(Uint8 * in_data, int len);

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
void get_storage_text(Uint8 * in_data, int len);

/*!
 * \ingroup storage_window
 * \brief   Renders the storage grid
 *
 *      Renders a storage grid with up to \a columns columns and \a rows rows. The parameters \a left and \a top indicate the starting position and \a width and \a height indicate the size of the grid window.
 *
 * \param columns   number of columns to use for the grid
 * \param rows      number of rows to use for the grid
 * \param left      x coordinate of the grid window
 * \param top       y coordinate of the grid window
 * \param width     width of the grid window
 * \param height    height of the grid window
 */
void rendergrid(int columns, int rows, int left, int top, int width, int height);

/*!
 * \ingroup storage_window
 * \brief   Gets the mouse position within the storage grid window
 *
 *      Gets the mouse position within the storage grid window.
 *
 * \param mx        x coordinate of the mouse position
 * \param my        y coordinate of the mouse position
 * \param columns   number of columns of the grid window
 * \param rows      number of rows of the grid window
 * \param left      x coordinate of the grid window
 * \param top       y coordinate of the grid window
 * \param width     width of the grid window
 * \param height    height of the grid window
 * \retval int      the grid position of the mouse, i.e. the grid number where the mouse cursor currently is, or -1 if the mouse cursor is outside the grid window
 */
int get_mouse_pos_in_grid(int mx, int my, int columns, int rows, int left, int top, int width, int height);

/*!
 * \ingroup storage_window
 * \brief Displays the new storage window
 *
 *      Displays the new storage window. If the window hasn't been created before it will first created.
 *
 * \callgraph
 */
void display_storage_menu();

#endif
