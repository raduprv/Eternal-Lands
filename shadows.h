/*!
 * \file
 * \ingroup shadows
 * \brief Handles the handling of shadows.
 */
#ifndef __SHADOWS_H__
#define __SHADOWS_H__

extern float sun_position[4];

extern int shadows_on; /*!< flag indicating whether shadows are enabled or disabled */
extern int is_day; /*!< this flag shows whether it's day or night */

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
