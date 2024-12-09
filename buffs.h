/*
 * buffs.h
 *
 *  Created on: 14.11.2008
 *      Author: superfloh
 */

#ifndef BUFFS_H_
#define BUFFS_H_

#include "actors.h"

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

extern int view_buffs;
extern int buff_icon_size;

/*!
 * \ingroup	network_actors
 * \brief	Updates the buffs with the actor_id.
 *
 * 		Find the player with ID \a actor_id and update their buffs.
 *
 * \param	actor_id The server-side actor ID
 * \param	in_buffs The buffs.
 */
void update_actor_buffs(int actor_id, Uint32 in_buffs);
void update_actor_buffs_locked(actor *act, actor *attached, Uint32 in_buffs);

void draw_buffs(actor *act, float x, float y,float z);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* BUFFS_H_ */
