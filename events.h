/*!
 * \file
 * \ingroup event_handle
 * \brief global event handling.
 */
#ifndef __EVENTS_H__
#define __EVENTS_H__

/*!
 * \name    Event types
 * @{
 */
enum { 
    EVENT_MOVEMENT_TIMER, /*!< event caused by the timer thread */
    EVENT_UPDATE_CAMERA, /*!< camera update event */
    EVENT_ANIMATE_ACTORS  /*!< animate actor event */
};
/*! @} */

extern int shift_on; /*!< flag indicating whether the Shift key is pressed. */
extern int alt_on; /*!< flag indicating whether the Alt key is pressed. */
extern int ctrl_on; /*!< flag indicating whether the Ctrl key is pressd. */
extern SDL_TimerID event_timer_clock;

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
 * \ingroup timers
 * \brief   event_timer
 *
 *      event_timer(Uint32,void*) to be documented
 *
 * \param interval
 * \param data
 * \retval Uint32
 */
Uint32 event_timer(Uint32 interval, void * data);

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
 * \ingroup interface_map
 * stores the position and the text for markings on the map.
 * \todo this struct should be moved to some place where it belongs, like map.h or sector.h
 */
typedef struct
{
    /*!
     * \name marking position
     */
    /*! @{ */
    int x ;
    int y;
    /*! @} */
    char text[512]; /*!< text of the marking */
}marking;

extern int adding_mark;
extern int mark_x, mark_y;
extern int max_mark;
extern marking marks[200];

#endif	// __EVENTS_H__
