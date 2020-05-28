#if !defined(USER_MENUS_H)
#define USER_MENUS_H

#ifdef __cplusplus
extern "C"
{
#endif

/* config option variable */
extern int enable_user_menus;
extern int ready_for_user_menus;

/*!
 * \ingroup user_menus
 * \brief Set the user menus position and options.
 *
 * \param win_x			x coord of window
 * \param win_y			y coord of window
 * \param options		window options (bit fields)
 */
void set_options_user_menus(int win_x, int win_y, int options);


/*!
 * \ingroup user_menus
 * \brief Return the user menus position and options.
 *
 * \param win_x			x coord of window
 * \param win_y			y coord of window
 * \param options		window options (bit fields)
 */
void get_options_user_menus(int *win_x, int *win_y, int *options);


/*!
 * \ingroup user_menus
 * \brief Open the user menus window.
 */
void display_user_menus(void);


/*!
 * \ingroup user_menus
 * \brief Called when the user menu config option is changed.
 *
 * \param enable		open window (1), close window (0)
 */
void toggle_user_menus(int *enable);


/*!
 * \ingroup user_menus
 * \brief free memory from user menus
 *
 */
void destroy_user_menus(void);

#ifdef __cplusplus
}
#endif

#endif

