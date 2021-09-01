/*!
 * \file
 * \ingroup 	display
 * \brief	Covers various of scripts used to animate and display actors as well as network data
 */
#ifndef __ACTOR_SCRIPTS_H__
#define __ACTOR_SCRIPTS_H__

#include <SDL_types.h>

#include "actors.h"			// Should we just move the function that needs this include away?

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \ingroup	move_actors
 * \brief	Gives the motion vector of an actor for a given move.
 * \param move_cmd the move command to use for computing the motion vector
 * \param dx the output delta on the x axis
 * \param dy the output delta on the y axis
 * \return 1 if the command was between move_n and move_nw, and 0 for other commands
 *
 * 		The function just computes the delta on X and Y axes corresponding
 * to the move that should be comprised between move_n and move_nw commands.
 *
 */
int get_motion_vector(int move_cmd, int *dx, int *dy);

/*!
 * \ingroup	move_actors
 * \brief	Finds the next frame in the md2-file for the current command.
 *
 * 		The move_to_next_frame function goes through the actors list and sets the cur_frame that's used when rendering.
 *
 */
void move_to_next_frame();

/*!
 * \ingroup	move_actors
 * \brief	This function is called from the display/network loop to animate actors.
 *
 * 		This function is called from the display/network loop, when the timer thread calls SDL_PushEvent EVENT_ANIMATE_ACTORS.
 * 		It's purpose is to animate the actors - that is, to change their x,y,z positions according to their current movement frames
 *
 */
void animate_actors();

/*!
 * \ingroup	move_actors
 * \brief	This function sets the correct idle animation for actor `actor` according to its
 * 	current state.
 * \param actor    Pointer to the actor for which to set the animation.
 * \param attached Pointer to the actor attached to \a actor (horse or rider), if any. NULL otherwise.
 * \callgraph
 */
void set_on_idle(actor *act, actor *attached);

/*!
 * \ingroup	move_actors
 * \brief	This function parses through the command queue (actors->que) and sets the cur_frame accordingly
 *
 * 		The function is called from the timer thread and parses through the command queue. If no new commands have been found or the actor is idle, it will copy that frame to the actor. Furthermore it changes the x, y, z movement speed and rotation speeds.
 *
 * 		This function needs to be passed the actors list (and its size), which should be obtained
 * 		under lock.
 *
 * \param actors_list The list of current actors
 * \param max_actors  The number of actors in \a actors_list
 * \callgraph
 */
void next_command(actor **actors_list, size_t max_actors);

/*!
 * \brief Free all the data that is contained in an actor
 * \param actor Pointer to the actor
 */
void free_actor_data(actor *act);

/*!
 * \ingroup	network_actors
 * \brief	The function destroys an actor and releases the memory it used.
 *
 * 		The function destroys the actor pointed to bya act, and releases the memory it used.
 *
 * \param	act The actor to destroy
 *
 * \sa		destroy_all_actors
 */
void destroy_actor(actor *act);

/*!
 * \ingroup	network_actors
 * \brief	The function destroys all actors.
 *
 * 		The function removes all actors from the actors list, destroys them, and free()s the
 * 		memory/textures.
 *
 * \sa		destroy_actor
 */
void destroy_all_actors();

/*!
 * \ingroup	network_actors
 * \brief	The function destroys all actors then requests an update.
 *
 * 		The calls destroy_all_actors() then requests the server sends all actor informaiton.
 *
 * \param	log_the_update If true, write message to the console, otherwise do it silently.
 *
 * \sa		destroy_all_actors
 */
void update_all_actors(int log_the_update);

/*!
 * \ingroup	network_actors
 * \brief	The function adds a command to the actor.
 *
 * 		This function is called whenever the client gets an ADD_ACTOR_COMMAND. It will add the command to the command queue (actor->que)
 *
 * \param	actor_id The server-side actor ID
 * \param	command The command that should be added to the actor
 *
 * \callgraph
 */
void add_command_to_actor(int actor_id, unsigned char command);

void add_emote_command_to_actor(actor * act, emote_data *emote);
void add_emote_to_actor(int actor_id, int emote_id);

/*!
 * \ingroup	network_actors
 * \brief	Sets the actor max health.
 *
 * 		Finds the actor with the given actor_id, sets max health points
 *
 * \param	actor_id The server-side actor ID
 * \param	damage The damage given by the actor
 */
void get_actor_damage(int actor_id, int damage);

/*!
 * \ingroup	network_actors
 * \brief	Sets the actor damage and removes the health from the actor.
 *
 * 		Finds the actor with the given actor_id, sets the damage time to 2000 ms and removes the health points lost (damage) from the actor.
 *
 * \param	actor_id The server-side actor ID
 * \param	damage The damage given by the actor
 */
void get_actor_health(int actor_id, int damage);

/*!
 * \ingroup	network_actors
 * \brief	Heals the player with the actor_id.
 *
 * 		Finds the player with the given actor_id and adds quantity health points to his current health points.
 *
 * \param	actor_id The server-side actor ID
 * \param	quantity The amount of healthpoints healed.
 */
void get_actor_heal(int actor_id, int quantity);

/*!
 * \ingroup	events_actors
 * \brief	Moves the actor 1 step forward.
 *
 * 		First it finds yourself in the actors_list, then moves you a step forward
 *
 * \callgraph
 */
void move_self_forward();

/*!
 * \ingroup other
 * \brief initializes the actor_def list
 *
 *      Initializes the actor_def list
 *
 * \callgraph
 */
void init_actor_defs();

/*!
 * \ingroup other
 * \brief frees the actor_def list
 *
 *      Frees the actor_def list
 *
 * \callgraph
 */
void free_actor_defs();

int checkvisitedlist(int x, int y);

/*!
 * \ingroup other
 * \brief loads the emotes def list
 *
 *      Loads the emotes def list
 *
 * \callgraph
 */
int read_emotes_defs();

void free_emotes();

#define HORSE_FIGHT_ROTATION 60
#define HORSE_RANGE_ROTATION 45
#define HORSE_FIGHT_TIME 180

void rotate_actor_and_horse_locked(actor* act, actor *horse, int mul);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
