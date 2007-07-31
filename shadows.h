/*!
 * \file
 * \ingroup shadows
 * \brief Handles the handling of shadows.
 */
#ifndef __SHADOWS_H__
#define __SHADOWS_H__

#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

extern float sun_position[4];

extern int shadows_on; /*!< flag indicating whether shadows are enabled or disabled */
extern int is_day; /*!< this flag shows whether it's day or night */

extern int use_shadow_mapping; /*!< flag whether to use shadow mapping or not */
extern GLuint depth_map_id;
extern GLenum depth_texture_target;
extern int shadow_map_size; /*!< max. size of the shadow maps in byte */

/*!
 * \ingroup shadows
 * \brief Computes the shadow transformation matrix
 *
 *      Computes the shadow transformation matrix
 *
 * \callgraph
 */
void calc_shadow_matrix();

/*!
 * \ingroup shadows
 * \brief Redraws the scene with shadows casted by the sun.
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
 * \brief Render_light_view to be documented
 *
 *      render_light_view to be documented
 *
 * \callgraph
 */
void render_light_view();

/*!
 * \ingroup shadows
 * \brief Disables generation of textures
 *
 *      Disables generation of textures
 *
 */
void disable_texgen();

/*!
 * \ingroup shadows
 * \brief Frees the shadow framebuffer.
 *
 *      Frees the shadow framebuffer.
 *
 * \callgraph
 */
void free_shadow_framebuffer();

/*!
 * \ingroup shadows
 * \brief Makes the shadow framebuffer.
 *
 *      Makes the shadow framebuffer.
 *
 * \callgraph
 */
void make_shadow_framebuffer();

/*!
 * \ingroup shadows
 * \brief Changes the shadow framebuffer.
 *
 *      Changes the shadow framebuffer.
 *
 * \callgraph
 */
void change_shadow_framebuffer_size();

#ifdef __cplusplus
} // extern "C"
#endif

#endif
