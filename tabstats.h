/*!
 * \file
 * \ingroup tabs
 * \brief declarations used by the tabbed windows.
 */
#ifndef __TABSTATS_H__
#define __TABSTATS_H__

/*! \name Sizes for tabbed windows
 * @{ */
#define TAB_WIDTH      580 /*!< default width of a tabbed window */
#define TAB_HEIGHT     350 /*!< default height of a tabbed window */
#define TAB_TAG_HEIGHT  20 /*!< the height of the tab at the top of the window */
#define TAB_MARGIN       5 /*!< margin used for the tabbed window */
/* @} */

extern int use_tabbed_windows; /*!< flag that indicates whether we use tabbed windows or the old separate windows. Can be set via commandline and in el.ini. */

/*! \name Windows handlers
 * @{ */
extern int tab_stats_win; /*!< handler for the stats tabbed window */
/* @} */

extern int tab_collection_id; /*!< pointer to the \see tab_collection used */

/*!
 * \ingroup tabs
 * \brief displays the tabbed stats window.
 *
 *      Displays the tabbed stats window.
 *
 * \callgraph
 */
void display_tab_stats ();

#endif // def __TABSTATS_H__
