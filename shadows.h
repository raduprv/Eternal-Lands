/*!
 * \file
 * \ingroup shadows
 * \brief Handles the handling of shadows.
 */
#ifndef __SHADOWS_H__
#define __SHADOWS_H__

/*
 * OBSOLETE: Queued for removal
 * Unused variables
 */
//extern float fDestMat[16];
//extern float fPlane[4];

extern float sun_position[4];

extern int shadows_on; /*!< flag indicating whether shadows are enabled or disabled */
extern int is_day; /*!< this flag shows whether it's day or night */

/*
 * OBSOLETE: Queued for removal
 * Only used in shadows.c, no need to declare them here
extern int shadows_texture;
extern GLenum z_texture_id;
 */

extern int use_shadow_mapping; /*!< flag whether to use shadow mapping or not */
extern GLuint depth_map_id;
extern GLenum depth_texture_target;
extern int max_shadow_map_size; /*!< max. size of the shadow maps in byte */

/*!
 * \ingroup shadows
 * \brief computes the shadow transformation matrix
 *
 *      Computes the shadow transformation matrix
 *
 * \callgraph
 */
void calc_shadow_matrix();

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup shadows
// * \brief draws 3d shadows for the given object
// *
// *      Draws 3D shadows for the given object3d object.
// *
// * \param object_id Pointer to the object for which shadows will be drawn.
// *
// * \callgraph
// */
//void draw_3d_object_shadow(object3d * object_id);

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup shadows
// * \brief draws the shadows for the body parts of an actor
// *
// *      Draws shadows caused by the body parts of an actor
// *
// * \param model_data    The md2 data
// * \param cur_frame     the current frame for which to draw the shadows
// * \param ghost         ghost
// */
//void draw_body_part_shadow(md2 *model_data,char *cur_frame, int ghost);

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup shadows
// * \brief draws the shadows for an enhanced_actor
// *
// *      Draws shadows for an enhanced_actor
// *
// * \param actor_id  Pointer to the actor
// *
// * \callgraph
// */
//void draw_enhanced_actor_shadow(actor * actor_id);

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup shadows
// * \brief draws shados for an actor
// *
// *      Draws shados for an actor
// *
// * \param actor_id  Pointer to the actor data
// *
// * \callgraph
// */
//void draw_actor_shadow(actor * actor_id);

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup shadows
// * \brief displays the shadows for an actor
// *
// *      Displays the shadows for an actor
// *
// * \callgraph
// */
//void display_actors_shadow();

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup shadows
// * \brief displays all relevant shadows
// *
// *      Displays shadows
// *
// * \callgraph
// */
//void display_shadows();

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup shadows
// * \brief displays 3D objects that lie on the ground
// *
// *      Displays 3D ground objects. display_3d_non_ground_objects
// *
// * \callgraph
// */
//void display_3d_ground_objects();

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup shadows
// * \brief displays 3D objects that don't lie on the ground
// *
// *      Displays 3D objects that are not ground objects. display_3d_ground_objects
// *
// * \callgraph
// */
//void display_3d_non_ground_objects();

/*!
 * \ingroup shadows
 * \brief redraws the scene with shadows casted by the sun.
 *
 *      Draws the scene with shadows enabled that are cast by the sun.
 *
 * \param any_reflection    Any reflection
 *
 * \callgraph
 */
void draw_sun_shadowed_scene(int any_reflection);

/*!
 * \ingroup shadows
 * \brief render_light_view to be documented
 *
 *      render_light_view to be documented
 *
 * \callgraph
 */
void render_light_view();

/*!
 * \ingroup shadows
 * \brief disables generation of textures
 *
 *      Disables generation of textures
 *
 */
void disable_texgen();

/*!
 * \ingroup shadows
 * \brief computes and sets the size of the shadow maps
 *
 *      Computes and sets the size of the shadow maps
 *
 * \sa floor_pow2
 */
void set_shadow_map_size();

#endif
