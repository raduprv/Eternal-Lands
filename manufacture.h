/*!
 * \file
 * \ingroup manufacture_window
 * \brief display of the manufacture window
 */
#ifndef __MANUFACTURE_H__
#define __MANUFACTURE_H__

/*!
 * \name windows handlers
 */
/*! @{ */
extern int manufacture_win; /*!< manufacture windows handler */
/*! @} */

/*!
 * \ingroup manufacture_window
 * \brief Sets up the \ref manufacture_list.
 *
 *      Initializes the \ref manufacture_list used when an actor is manufacturing items with the \ref manufacture_win window.
 *
 */
void build_manufacture_list();

/*!
 * \ingroup manufacture_window
 * \brief Displays the manufacture window.
 *
 *      Displays the \ref manufacture_win window. If the window was not shown before it will first initialized.
 *
 * \callgraph
 */
void display_manufacture_menu();

//int check_manufacture_interface();

#endif
