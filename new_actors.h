/*!
 * \file
 * \ingroup display_actors
 * \brief handles the adding of new actors.
 */
#ifndef __NEW_ACTORS_H__
#define __NEW_ACTORS_H__

extern float sitting; /*!< used to compute several actor related z values */

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup display_actors
// * \brief
// *
// *      Detail
// *
// * \param model_data
// * \param cur_frame
// * \param ghost
// */
//void draw_body_part(md2 *model_data,char *cur_frame, int ghost);

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup network_actors
// * \brief
// *
// *      Detail
// *
// * \param this_actor
// * \param frame_name
// * \param x_pos
// * \param y_pos
// * \param z_pos
// * \param z_rot
// * \param actor_id
// * \retval int
// * \callgraph
// */
//int add_enhanced_actor(enhanced_actor *this_actor,char * frame_name,float x_pos, float y_pos,
//					   float z_pos, float z_rot, int actor_id);

/*!
 * \ingroup display_actors
 * \brief   Draws the given enhanced actor
 *
 *      Draws the enhanced actor given by \a actor_id, by calling \ref draw_model for the individual parts of the actor.
 *
 * \param actor_id  the id for the actor to draw
 *
 * \callgraph
 */
void draw_enhanced_actor(actor * actor_id);

/*!
 * \ingroup display_actors
 * \brief   Unwears the given item \a which_part from the actor \a actor_id.
 *
 *      Unwears the given item \a which_part from the actor given by \a actor_id.
 *
 * \param actor_id      the id for the actor to unwear some item
 * \param which_part    the id for the item to unwear from the actor
 * \callgraph
 *
 * \pre If the given \a actor_id is not in the list of actors \ref actors_list, this function won't perform any action.
 * \pre The following items are currently supported as \a which_part: \ref KIND_OF_WEAPON, \ref KIND_OF_SHIELD, \ref KIND_OF_CAPE and \ref KIND_OF_HELMET
 *
 * \sa actor_wear_item
 */
void unwear_item_from_actor(int actor_id,Uint8 which_part);

/*!
 * \ingroup display_actors
 * \brief   The item given by \a which_id will be worn by the actor \a actor_id at the specified part \a which_part.
 *
 *      The actor \a actor_id will wear the item given by \a which_id at the specified part \a which_part.
 *
 * \param actor_id      the id of the actor to wear the item \a which_id
 * \param which_part    the part of the body at which the item \a which_id should be worn
 * \param which_id      the id of the item to wear
 * \callgraph
 *
 * \pre If the given \a actor_id is not in the list of actors \ref actors_list, this function won't perform any action.
 * \pre The following items are currently supported as \a which_part: \ref KIND_OF_WEAPON, \ref KIND_OF_SHIELD, \ref KIND_OF_CAPE, \ref KIND_OF_HELMET, \ref KIND_OF_BODY_ARMOR, \ref KIND_OF_LEG_ARMOR and \ref KIND_OF_BOOT_ARMOR.
 *
 * \sa unwear_item_from_actor
 */
void actor_wear_item(int actor_id,Uint8 which_part, Uint8 which_id);

/*!
 * \ingroup network_actors
 * \brief   Adds a new server character. The actor data is in \a in_data.
 *
 *      Adds a new server character. The data for the actor will be sent by the server an is given by \a in_data.
 *
 * \param in_data   the data for the new actor.
 *
 * \callgraph
 */
void add_enhanced_actor_from_server(char * in_data);

/*!
 * \ingroup other
 * \brief   Gets called by \ref init_stuff to create the glow color table used by the glow effects.
 *
 *      Creates the glow color table, which is used by the glow effects and initializes it with the default values. Currently the following glow effects are considered: \ref GLOW_NONE, \ref GLOW_FIRE, \ref GLOW_COLD, \ref GLOW_THERMAL and \ref GLOW_MAGIC.
 *
 * \sa init_stuff
 */
void build_glow_color_table();
#endif
