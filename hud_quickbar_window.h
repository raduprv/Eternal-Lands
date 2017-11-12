#ifndef	__HUD_QUICKBAR_WINDOW_H
#define	__HUD_QUICKBAR_WINDOW_H

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \name orientation constants
 */
/*! @{ */
#define HORIZONTAL 2
#define VERTICAL 1
/*! @} */

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
extern int quickbar_win; /*!< quickbar windows handler */
extern int quickbar_x;
extern int quickbar_y;
extern int quickbar_dir;
extern int quickbar_draggable;
extern int num_quickbar_slots;
extern int quickbar_relocatable; /*!< flag that indicates whether the quickbar is relocatable. */
extern int qb_action_mode; /*!< flag indicating whether we are in quickbar action mode or not */
extern int cm_quickbar_enabled;
/*! @} */


/*!
 * \ingroup hud_quickbar
 * \brief Initializes the quickbar
 *
 *      Initializes the quickbar, it's event handlers and shows it. If the quickbar has been moved by the player it will be drawn in its new position.
 */
void init_quickbar(void);

/*!
 * \ingroup hud_quickbar
 * \brief Checks if the keypress is an item use
 *
 *	returns 1 if the key is a item keypress, otherwise 0.
 */
int action_item_keys(Uint32 key);

void switch_action_mode(int * mode, int id);
int get_quickbar_y_base(void);
int shorten_quickbar(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif	//__HUD_QUICKBAR_WINDOW_H
