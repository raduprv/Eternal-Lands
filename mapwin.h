/*!
 * \file
 * \ingroup interface_map
 * \brief Declarations for creation and handling of the map window
 */
#ifndef __MAPWIN_H__
#define __MAPWIN_H__

/*! \name windows handlers 
 * @{ */
extern int map_root_win; /*!< handler for the map window */
/*! @} */

/*!
 * \ingroup interface_map
 * stores the position and the text for markings on the map.
 * \todo this struct should be moved to some place where it belongs, like map.h or sector.h
 */
typedef struct
{
    /*!
     * \name marking position
     */
    /*! @{ */
    int x ;
    int y;
    /*! @} */
    char text[512]; /*!< text of the marking */
}marking;

extern int adding_mark; /*!< flag that indicates we are currently adding a mark to a map */
extern int mark_x, mark_y; /*!< map coordinates of the position of the mark */
extern int max_mark; /*!< max. number of marks we can handle */
extern marking marks[200]; /*!< a global array of marks */

#ifndef OLD_EVENT_HANDLER

/*!
 * \ingroup interface_map
 * \brief creates the map window
 *
 *      Creates the map window
 *
 * \callgraph
 */
void create_map_root_window ();

#endif

#endif // def __MAPWIN_H__
