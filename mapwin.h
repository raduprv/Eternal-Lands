/*!
 * \file
 * \ingroup interface_map
 * \brief Declarations for creation and handling of the map window
 */
#ifndef __MAPWIN_H__
#define __MAPWIN_H__

/*! \name windows handlers 
 * @{ */
extern int map_win; /*!< handler for the map window */
/*! @} */


#ifndef OLD_EVENT_HANDLER

/*!
 * \ingroup interface_map
 * \brief creates the map window
 *
 *      Creates the map window
 *
 * \callgraph
 */
void create_map_window ();

#endif

#endif // def __MAPWIN_H__
