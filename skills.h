/*!
 * \file
 * \ingroup skills_window
 * \brief handles the display of the skills window
 */
#ifndef __SKILLS_H__
#define __SKILLS_H__

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \names windows handlers
 */
/*! @{ */
extern int skills_win; /*!< skills window handler */
/*! @} */

/*
int skills_menu_x=150;
int skills_menu_y=70;
int skills_menu_x_len=150;
int skills_menu_y_len=200;
*/

/*!
 * \ingroup skills_window
 * \brief Sets the window handler functions for the skills window
 *
 *      Sets the window handler functions for the skills window
 *
 * \return None
 */
void fill_skills_win ();

#ifdef __cplusplus
} // extern "C"
#endif

#endif
