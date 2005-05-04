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
 * Stores the position and the text for markings on the map.
 * \todo this struct should be moved to some place where it belongs, like map.h or sector.h
 */
typedef struct
{
    /*! \name marking position */
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

/*!
 * \ingroup interface_map
 * \brief Creates the map window
 *
 *      Creates the map window
 *
 * \param width the width of the window
 * \param height the height of the window
 * \callgraph
 */
void create_map_root_window (int width, int height);

#endif // def __MAPWIN_H__
