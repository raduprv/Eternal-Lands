#ifndef	__EVENTS_H
#define	__EVENTS_H

#define SHIFT (1<<31)
#define CTRL (1<<30)
#define ALT (1<<29)

void zoomin();
void zoomout();
int HandleEvent(SDL_Event *event);

/*!
 * \name    Event types
 * @{
 */
enum {
        EVENT_UPDATE_CAMERA    /*!< camera update event */
};
/*! @} */

#endif	//__EVENTS_H
