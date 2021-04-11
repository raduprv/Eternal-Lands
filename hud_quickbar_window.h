#ifndef	__HUD_QUICKBAR_WINDOW_H
#define	__HUD_QUICKBAR_WINDOW_H

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \name Quickbar defines
 */
/*! @{ */
#define MAX_QUICKBAR_SLOTS 12
/*! @} */

/*!
 * \name windows handlers
 */
/*! @{ */
extern int quickbar_dir;
extern int quickbar_draggable;
extern int num_quickbar_slots;
extern int quickbar_relocatable; /*!< flag that indicates whether the quickbar is relocatable. */
extern int cm_quickbar_enabled;
extern int independant_quickbar_action_modes;
/*! @} */


/*!
 * \ingroup items_quickbar
 * \brief Initializes the quickbar
 *
 *      Initializes the quickbar, it's event handlers and shows it. If the quickbar has been moved by the player it will be drawn in its new position.
 */
void init_quickbar(void);

/*!
 * \ingroup items_quickbar
 * \brief Checks if the keypress is an item use
 *
 *	returns 1 if the key is a item keypress, otherwise 0.
 */
int action_item_keys(SDL_Keycode key_code, Uint16 key_mod);

/*!
 * \ingroup items_quickbar
 * \brief Get the window bottom y position if docked, or the default top.
 *
 *	returns y position.
 */
int get_quickbar_y_base(void);

/*!
 * \ingroup items_quickbar
 * \brief Set the action mode for the quickbar window.
 *
 * \param new_mode  the new action mode, ignored if not one that can be used
 *
 */
void set_quickbar_action_mode(int new_mode);

#ifdef __cplusplus
} // extern "C"
#endif

#endif	//__HUD_QUICKBAR_WINDOW_H
