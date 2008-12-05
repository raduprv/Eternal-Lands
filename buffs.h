/*
 * buffs.h
 *
 *  Created on: 14.11.2008
 *      Author: superfloh
 */

#ifndef BUFFS_H_
#define BUFFS_H_

#include "platform.h"

// keep in sync with client_serv.h !!!
#define NUM_BUFFS 11

extern int view_buffs;
extern int buff_icon_size;

/*!
 * \ingroup	network_actors
 * \brief	Updates the buffs with the actor_id.
 *
 * 		Finds the player with the given actor_id and update's their buffs.
 *
 * \param	actor_id The server-side actor ID
 * \param	quantity The buffs.
 */
void update_actor_buffs(int actor_id, Uint32 in_buffs);

void draw_buffs(int, float x, float y,float z);

void update_buff_eye_candy(int actor_id);

#endif /* BUFFS_H_ */
