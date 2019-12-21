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
