#ifndef	__HUD_STATSBAR_WINDOW_H
#define	__HUD_STATSBAR_WINDOW_H

#include <SDL_types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_WATCH_STATS	5	/*!< max number of stats watchable in hud */

extern int stats_bar_win; /*!< the window id for the stats bar of the bottom HUD */
extern int show_action_bar; /*!< saved in the ini file, the action points stats bar is display when true */
extern int show_last_health_change_always; /*!< saved in the ini file, the the last health change is always display when true */
extern int max_food_level; /*!< normally 45 but can be set from options for people with diffent values (big belly) */
extern int lock_skills_selection; /*!< if true, disables selection from the skills bar and the stats window */

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

/*!
 * \ingroup statsbar
 * \brief Set the watched stats bars.
 *
 *      Set the watched stats bars, read from the config file.
 *
 * \callgraph
 */

void set_statsbar_watched_stats(int *cfg_watch_this_stats);
/*!
 * \ingroup statsbar
 * \brief Get the watched stats bar infomation.
 *
 *      Get the watched stats bar infomation, so it can be saved to the config file.
 *
 * \callgraph
 */
void get_statsbar_watched_stats(int *cfg_watch_this_stats);

#ifdef __cplusplus
} // extern "C"
#endif

#endif	//__HUD_STATSBAR_WINDOW_H
