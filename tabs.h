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

/*!
 * \name Sizes for tabbed windows
 * @{
 */
#define TAB_MARGIN       5   /*!< margin used for the tabbed window */
/*! @} */

/*!
 * \name tab positions for tabbed windows
 * @{
 */
enum { HELP_TAB_HELP = 0, HELP_TAB_SKILLS, HELP_TAB_ENCYCLOPEDIA, HELP_TAB_RULES };
enum { STATS_TAB_STATS = 0, STATS_TAB_KNOWLEDGE, STATS_TAB_COUNTERS, STATS_TAB_SESSION };
enum { INFO_TAB_NOTEPAD, INFO_TAB_URLWIN };
/*! @} */

extern int tab_stats_collection_id; /*!< pointer to the \ref tab_collection for statistics windows */

extern int tab_help_collection_id; /*!< pointer to the \ref tab_collection for help windows */

extern int tab_info_collection_id; /*!< pointer to the \ref tab_collection for info windows */

/*!
 * \ingroup tabs
 * \brief Displays the tabbed info window.
 *
 *      Each tab collection has 4 bits of the word, get the values to save to the cfg file.
 *
 * \callgraph
 */
unsigned get_tab_selected(void);

/*!
 * \ingroup tabs
 * \brief Displays the tabbed info window.
 *
 *      Each tab collection has 4 bits of the word, set the values to read from the cfg file.
 *
 * \callgraph
 */
void set_tab_selected(unsigned int new_tab_selected);

#ifdef JSON_FILES
/*!
 * \ingroup tabs
 *
 *      Write the selected tab info to the client state file.
 *
 * \callgraph
 */
void write_tab_selected(void);

/*!
 * \ingroup tabs
 *
 *      Read the selected tab info from the client state file.
 *
 * \callgraph
 */
void read_tab_selected(void);
#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif // def __TABS_H__
