/*!
 * \file
 * \ingroup event_handle
 * \brief global event handling.
 *
 */
#ifndef __EVENTS_H__
#define __EVENTS_H__

#include <SDL_events.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \name    Event types
 * @{
 */
enum {
	EVENT_MOVEMENT_TIMER,	 /*!< event caused by the timer thread */
	EVENT_UPDATE_PARTICLES,	 /*!< update the particles */
	EVENT_UPDATES_DOWNLOADED,/*!< the event to send when the main updates.lst has been downloaded */
	EVENT_DOWNLOAD_COMPLETE, /*!< the normal event to send when a download finishes */
#ifdef ANDROID
	EVENT_CURSOR_CALCULATION_COMPLETE, /*!< event to send after the what is under mouse was calculated in the display loop */
	EVENT_LONG_TOUCH,		 /*!< for non-moving touch event, this will trigger a right-click equlivient event after a deley */
#endif
#ifdef PAWN
	EVENT_PAWN_TIMER,        /*!< event for running Pawn timer callbacks */
#endif
#ifdef	CUSTOM_UPDATE
	EVENT_CUSTOM_UPDATE_COMPLETE,  /*!< the event to send when custom updates are downloaded. */
#endif	/* CUSTOM_UPDATE */
};
/*! @} */

#ifdef OSX
extern int osx_right_mouse_cam; /*!< flag indication whether the right mouse button should enable camera rotation */
#endif

#ifdef ANDROID
extern int back_on; /*!< flag indicating whether the Android back key is pressed. */
extern float long_touch_delay_s; /*!< Long Touch Delay in seconds. */
extern float motion_touch_delay_s;  /*!< the time in seconds before motion is chosen over touch */
extern int enable_keyboard_debug; /*!< enabled by #kbd, shos key codes in console */
#endif

/*!
 * \ingroup event_handle
 * \brief       main event handling routine
 *
 *      Main event handling routine.
 *
 * \param event     a pointer to last event occurred.
 * \retval int
 * \callgraph
 */
int HandleEvent(SDL_Event *event);

/*!
 * \ingroup event_handle
 * \brief       Make sure minimised and restored window state is noticed
 *
 * \callgraph
 */
void check_minimised_or_restore_window(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif	// __EVENTS_H__
