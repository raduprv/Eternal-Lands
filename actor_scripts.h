/*!
 * \file
 * \brief	Covers various of scripts used to animate and display actors as well as network data
 * \ingroup 	display
 */
#ifndef __ACTOR_SCRIPTS_H__
#define __ACTOR_SCRIPTS_H__

/*!
 * \ingroup 	actor_utils
 * \brief	Calculates the current rotation from a float to degree
 *
 * 		This function is used for calculating the rotation in degrees instead of a floating point number from 0<=fAngle<=1
 *
 * \param	fAngle Denotes the floating point rotation (0<=fAngle<=1)
 * \return	The angle in degrees
 */
float unwindAngle_Degrees( float fAngle );

/*!
 * \ingroup	actor_utils
 * \brief	Gets the rotation vector from fStartAngle to fEndAngle in degrees
 *
 * 		Used for getting the rotation vector between fStartAngle and fEndAngle in degrees. Calls unwindAngle_Degrees.
 *
 * \param	fStartAngle Sets the starting angle (0<=fAngle<=1)
 * \param	fEndAngle Sets the end angle (0<=fAngle<=1)
 * \return	The rotation vector in degrees
 */
float get_rotation_vector( float fStartAngle, float fEndAngle );

/*!
 * \ingroup	move_actors
 * \brief	Finds the next frame in the md2-file for the current command.
 *
 * 		The move_to_next_frame function goes through the actors list and sets the cur_frame that's used when rendering.
 *
 * \return	None
 */
void move_to_next_frame();

/*!
 * \ingroup	move_actors
 * \brief	This function is called from the display/network loop to animate actors.
 *
 * 		This function is called from the display/network loop, when the timer thread calls SDL_PushEvent EVENT_ANIMATE_ACTORS. 
 * 		It's purpose is to animate the actors - that is, to change their x,y,z positions according to their current movement frames
 *
 * \return	None
 */
void animate_actors();

/*!
 * \ingroup	move_actors
 * \brief	This function parses through the command queue (actors->que) and sets the cur_frame accordingly
 *
 * 		The function is called from the timer thread and parses through the command queue. If no new commands have been found or the actor is idle, it will copy that frame to the actor. Furthermore it changes the x, y, z movement speed and rotation speeds.
 *
 * \return	None
 */
void next_command();

/*!
 * \ingroup	network_actors
 * \brief	The function destroys the actor with the given actor_id (server-side actor ID).
 *
 * 		The function parses through the actors_list and destroys the actor with the matching actor_id.
 *
 * \param	actor_id The server-side actor ID
 * \return 	None
 * \sa		destroy_all_actors
 * \sa		actors_list
 */
void destroy_actor(int actor_id);

/*!
 * \ingroup	network_actors
 * \brief	The function destroys all actors.
 *
 * 		The function destroys all actors, free()s the memory/textures and sets the *actors_list=NULL
 *
 * \return	None
 * \sa		destroy_actor
 */
void destroy_all_actors();

/*!
 * \ingroup	network_actors
 * \brief	The function requests all actors from the server.
 *
 * 		The function is called whenever the client gets a message called "Resyncing with the server". It will call destroy_all_actors and send a SEND_ME_MY_ACTORS to the server hence getting the actors in range again.
 *
 * \return	None
 * \sa		destroy_all_actors
 */
void update_all_actors();

/*!
 * \ingroup	network_actors
 * \brief	The function adds a command to the actor.
 *
 * 		This function is called whenever the client gets an ADD_ACTOR_COMMAND. It will add the command to the command queue (actor->que)
 *
 * \param	actor_id The server-side actor ID
 * \param	command The command that should be added to the actor
 * \return	None
 */
void add_command_to_actor(int actor_id, char command);

/*!
 * \ingroup	network_actors
 * \brief	Sets the actor damage and removes the health from the actor.
 *
 * 		Finds the actor with the given actor_id, sets the damage time to 2000 ms and removes the health points lost (damage) from the actor.
 *
 * \param	actor_id The server-side actor ID
 * \param	damage The damage given by the actor
 * \return	None
 */
void get_actor_damage(int actor_id, int damage);

/*!
 * \ingroup	network_actors
 * \brief	Heals the player with the actor_id.
 *
 * 		Finds the player with the given actor_id and adds quantity health points to his current health points.
 *
 * \param	actor_id The server-side actor ID
 * \param	quantity The amount of healthpoints healed.
 * \return	None
 */
void get_actor_heal(int actor_id, int quantity);

/*!
 * \ingroup	events_actors
 * \brief	Moves the actor 1 step forward.
 *
 * 		First it finds yourself in the actors_list, then moves you a step forward (sends a MOVE_TO to the server).
 *
 * \return	None
 */
void move_self_forward();
void init_actor_defs();
void you_sit_down();
void you_stand_up();
#endif
