/*!
 * \file
 * \ingroup display_actors
 * \brief handles the adding of new actors.
 */
#ifndef __NEW_ACTORS_H__
#define __NEW_ACTORS_H__

extern float sitting;

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
 * \brief
 *
 *      Detail
 *
 * \param actor_id
 *
 * \callgraph
 */
void draw_enhanced_actor(actor * actor_id);

/*!
 * \ingroup display_actors
 * \brief
 *
 *      Detail
 *
 * \param actor_id
 * \param which_part
 *
 * \callgraph
 */
void unwear_item_from_actor(int actor_id,Uint8 which_part);

/*!
 * \ingroup display_actors
 * \brief
 *
 *      Detail
 *
 * \param actor_id
 * \param which_part
 * \param which_id
 *
 * \callgraph
 */
void actor_wear_item(int actor_id,Uint8 which_part, Uint8 which_id);

/*!
 * \ingroup network_actors
 * \brief
 *
 *      Detail
 *
 * \param in_data
 *
 * \callgraph
 */
void add_enhanced_actor_from_server(char * in_data);

/*!
 * \ingroup other
 * \brief
 *
 *      Detail
 *
 * \sa init_stuff
 */
void build_glow_color_table();
#endif
