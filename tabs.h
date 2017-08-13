/*!
 * \file
 * \ingroup tabs
 * \brief declarations used by the tabbed windows.
 */
#ifndef __TABS_H__
#define __TABS_H__

#ifdef __cplusplus
extern "C" {
#endif

/*! \name Sizes for tabbed windows
 * @{ */
#define TAB_MARGIN       5   /*!< margin used for the tabbed window */
/* @} */

/*! \name tab positions for tabbed windows
 * @{ */
enum { HELP_TAB_HELP = 0, HELP_TAB_SKILLS, HELP_TAB_ENCYCLOPEDIA, HELP_TAB_RULES };
enum { STATS_TAB_STATS = 0, STATS_TAB_KNOWLEDGE, STATS_TAB_COUNTERS, STATS_TAB_SESSION };
enum { INFO_TAB_NOTEPAD, INFO_TAB_URLWIN };
/* @} */

/*! \name windows handlers 
 * @{ */
extern int tab_stats_win; /*!< handler for the stats tabbed window */
/* @} */

extern int tab_stats_x;
extern int tab_stats_y;
extern unsigned int tab_selected;

extern int tab_stats_collection_id; /*!< pointer to the \see tab_collection for statistics windows */

/*! \name windows handlers 
 * @{ */
extern int tab_help_win; /*!< handler for the help tabbed window */
/* @} */

extern int tab_help_x;
extern int tab_help_y;

extern int tab_help_collection_id; /*!< pointer to the \see tab_collection for help windows */

/*! \name windows handlers 
 * @{ */
extern int tab_info_win; /*!< handler for the info tabbed window */
/* @} */

extern int tab_info_x;
extern int tab_info_y;

extern int tab_info_collection_id; /*!< pointer to the \see tab_collection for info windows */

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


/*!
 * \ingroup tabs
 * \brief Displays the tabbed info window.
 *
 *      Displays the tabbed info window.
 *
 * \callgraph
 */
void display_tab_info ();


/*!
 * \ingroup tabs
 * \brief Displays the tabbed info window.
 *
 *      Each tab collection has 4 bits of the word, this function controls which is which.
 *
 * \callgraph
 */
unsigned get_tab_selected(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // def __TABS_H__
