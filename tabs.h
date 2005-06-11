/*!
 * \file
 * \ingroup tabs
 * \brief declarations used by the tabbed windows.
 */
#ifndef __TABS_H__
#define __TABS_H__

/*! \name Sizes for tabbed windows
 * @{ */
#define STATS_TAB_WIDTH  580 /*!< width of the statistics tabbed windows */
#define STATS_TAB_HEIGHT 350 /*!< height of the statistics tab windows */
#define HELP_TAB_WIDTH   500 /*!< width of the help tabbed windows */
#define HELP_TAB_HEIGHT  350 /*!< height of the help tabbed windows */
#define TAB_TAG_HEIGHT   20  /*!< the height of the tab at the top of the window */
#define TAB_MARGIN       5   /*!< margin used for the tabbed window */
#define TAB_SPACING      3   /*!< the spacing between two tabs */
/* @} */

/*! \name windows handlers 
 * @{ */
extern int tab_stats_win; /*!< handler for the stats tabbed window */
/* @} */

extern int tab_stats_collection_id; /*!< pointer to the \see tab_collection for statistics windows */

/*! \name windows handlers 
 * @{ */
extern int tab_help_win; /*!< handler for the help tabbed window */
/* @} */

extern int tab_help_collection_id; /*!< pointer to the \see tab_collection for help windows */

/*!
 * \ingroup tabs
 * \brief Displays the tabbed stats window.
 *
 *      Displays the tabbed stats window.
 *
 * \callgraph
 */
void display_tab_stats ();

/*!
 * \ingroup tabs
 * \brief Displays the tabbed help window.
 *
 *      Displays the tabbed help window.
 *
 * \callgraph
 */
void display_tab_help ();

#endif // def __TABS_H__
