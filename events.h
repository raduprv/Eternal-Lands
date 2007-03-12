/*!
 * \file
 * \ingroup event_handle
 * \brief global event handling.
 *
 * \note Struct marking, and the variables adding_mark, mark_x, mark_y, max_mark and marks have been
 *       moved to mapwin.h
 */
#ifndef __EVENTS_H__
#define __EVENTS_H__

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \name    Event types
 * @{
 */
enum {
	EVENT_MOVEMENT_TIMER,	/*!< event caused by the timer thread */
	EVENT_UPDATE_CAMERA,	/*!< camera update event */
	EVENT_ANIMATE_ACTORS,	/*!< animate actor event */
	EVENT_UPDATE_PARTICLES,	/*!< update the particles */
	EVENT_UPDATES_DOWNLOADED,	/*!< the event to send when the main updates.lst has been downloaded */
	EVENT_DOWNLOAD_COMPLETE, /*!< the normal event to send when a download finishes */
};
/*! @} */

extern int shift_on; /*!< flag indicating whether the Shift key is pressed. */
extern int alt_on; /*!< flag indicating whether the Alt key is pressed. */
extern int ctrl_on; /*!< flag indicating whether the Ctrl key is pressd. */
#ifdef OSX
extern int meta_on; /*!< flag indicating whether the Apple/Command  key is pressed. */
#endif

/*!
 * \ingroup event_handle
 * \brief   gets called when a use mouse click occurs in the quickbar.
 *
 *      Gets called when a use mouse click occurs in the quickbar.
 *
 * \param use_id    the id of the item to use
 */
void	quick_use(int use_id);

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

#ifdef __cplusplus
} // extern "C"
#endif

#endif	// __EVENTS_H__
