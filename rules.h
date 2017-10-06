/*!
 * \file
 * \ingroup interface_rules
 * \brief handling and display of the rules
 */
#ifndef __RULES_H__
#define __RULES_H__

#include <SDL_types.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int have_rules; /*!< *** flag indicating whether the rules.xml file is available of not */
extern int last_display; /*! indicates when the rules were last displayed */
extern int countdown; /*!< number of seconds how long the rules should be displayed */
extern int has_accepted; /*!< flag indicating whether the rules are accepted or rejected */

extern int rules_root_win;

/*!
 * \ingroup other
 * \brief Reads in the rules
 *
 *      Reads in the rules from the rules.xml file
 *
 * \retval int  0 on success, else != 0 ***
 * \callgraph
 */
int read_rules(void);

/*!
 * \ingroup rules_window
 * \brief Fills the rules tab
 *
 * 	Fills the rules tab.
 * 
 * \param window_id  the id of the rules window.
*/
void fill_rules_window(int window_id);

/*!
 * \ingroup other
 * \brief Cleans up and frees the memory used by the rules.
 *
 *      Cleans up and frees the memory used by the rules.
 *
 * \callgraph
 */
void cleanup_rules(void);

/*!
 * \ingroup interface_rules
 * \brief Highlights the specified rule with the type specified.
 *
 *      Highlights a particular rule, specified by no with a given highlighing type.
 *
 * \param type  the highlighing type to use
 * \param rules
 * \param no    the rule number to highlight.
 *
 * \callgraph
 */
void highlight_rule (int type, const Uint8 *rules, int no);

/*!
 * \ingroup interface_rules
 * \brief Creates and initializes the rules root window
 *
 *      Creates and initializes the rules root window
 *
 * \param width the width of the window
 * \param height the height of the window
 * \param next the ID of the window to open when  the rules are accepted
 * \param time the timeout in seconds
 *
 * \callgraph
 */
void create_rules_root_window (int width, int height, int next, int time);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
