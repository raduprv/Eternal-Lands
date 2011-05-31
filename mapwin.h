/*!
 * \file
 * \ingroup interface_map
 * \brief Declarations for creation and handling of the map window
 */
#ifndef __MAPWIN_H__
#define __MAPWIN_H__

#ifdef __cplusplus
extern "C" {
#endif

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
    char server_side;
    int r,g,b; //color of the marker
}marking;

#define MAX_MARKINGS 300
#define MAX_USER_MARKS 250
extern int adding_mark; /*!< flag that indicates we are currently adding a mark to a map */
extern int mark_x, mark_y; /*!< map coordinates of the position of the mark */
extern int max_mark; /*!< max. number of marks we can handle */
extern int max_temp_mark; /*!< max. number of temporary marks we can handle */
extern int temp_tile_map_size_x; /*!< The tile map size in the x direction */
extern int temp_tile_map_size_y; /*!< The tile map size in the y direction */
extern marking marks[MAX_MARKINGS]; /*!< a global array of marks */
extern marking temp_marks[MAX_USER_MARKS]; /*!< a global array of temporary marks */
extern int curmark_r,curmark_g,curmark_b; //current mark color

extern int reload_tab_map; /*!< flag that indicates the tabmap needs to be reloaded */

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

/* controls TAB map mark filtering */
extern int mark_filter_active;    /* true when filter active */
extern char mark_filter_text[];   /* the text of the current filter */


#ifdef __cplusplus
} // extern "C"
#endif

#endif // def __MAPWIN_H__
