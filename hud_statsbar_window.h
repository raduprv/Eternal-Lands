#ifndef	__HUD_STATSBAR_WINDOW_H
#define	__HUD_STATSBAR_WINDOW_H

#ifdef __cplusplus
extern "C" {
#endif

extern int stats_bar_win; /*!< the window id for the stats bar of the bottom HUD */
extern int show_action_bar; /*!< saved in the el.ini file, the action points stats bar is display when true */
extern int max_food_level; /*!< normally 45 but can be set from options for people with diffent values (big belly) */
extern int watch_this_stats[]; /*!< used for displaying more than 1 stat in the hud */

/*!
 * \ingroup statsbar
 * \brief Control which stats bars are shown.
 *
 *      Control which stats bars are shown when selected from the status window or the misc window stats bars,.
 *
 \callgraph
 */
void handle_stats_selection(int stat, Uint32 flags);

/*!
 * \ingroup statsbar
 * \brief Initialise the stat bars.
 *
 *      Initialise the stat bars, (size, position and number), for in the bottom HUD.
 *
 \callgraph
 */
void init_stats_display(void);

/*!
 * \ingroup statsbar
 * \brief Update displayed damage value.
 *
 *      The last damage is drawn as a hover over the health bar.
 *
 * \callgraph
 */
void set_last_damage(int quantity);

/*!
 * \ingroup statsbar
 * \brief Update displayed heal value.
 *
 *      The last heal is drawn as a hover over the health bar.
 *
 * \callgraph
 */
void set_last_heal(int quantity);


#ifdef __cplusplus
} // extern "C"
#endif

#endif	//__HUD_STATSBAR_WINDOW_H
