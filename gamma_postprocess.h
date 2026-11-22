/**
 * @file
 * @ingroup display_utils
 * @brief Software brightness fallback for platforms where
 *        SDL_SetWindowBrightness() is not supported (e.g. Wayland).
 *
 * SDL2's Wayland backend (and some X11/driver combinations) cannot set the
 * display gamma ramp - SDL_SetWindowBrightness() simply fails there. When
 * that happens, we approximate the gamma slider by blending a translucent
 * black (darken) or white (brighten) quad over the already-rendered frame,
 * right before it is presented. Platforms where SDL_SetWindowBrightness()
 * already works are completely unaffected.
 *
 * This deliberately avoids a GLSL shader: an ARB_shader_objects program
 * bound during this post-process step - even a minimal one, even with a
 * vertex shader attached - was found to corrupt fixed-function terrain
 * rendering on subsequent frames on at least one Mesa/radeonsi driver. A
 * plain glBlendFunc()/glColor4f() overlay only uses OpenGL 1.0 features and
 * doesn't reproduce a true gamma curve, but reliably brightens/darkens the
 * screen without that risk.
 */
#ifndef GAMMA_POSTPROCESS_H
#define GAMMA_POSTPROCESS_H

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief Non-zero if SDL_SetWindowBrightness() works on this system. */
extern int hardware_gamma_available;
/*! \brief Non-zero if the software brightness fallback is active. */
extern int software_gamma_active;

/*!
 * \brief Try to set up the software brightness fallback.
 *
 * Must be called after hardware_gamma_available has been determined. Only
 * has an effect when hardware gamma is unavailable and gamma adjustment is
 * not disabled by the user.
 */
void init_gamma_postprocess(void);

/*! \brief Currently a no-op; kept for symmetry with init_gamma_postprocess(). */
void free_gamma_postprocess(void);

/*!
 * \brief Call once per frame, after the frame is fully rendered into the
 *        default framebuffer, but before SDL_GL_SwapWindow(). A no-op
 *        unless the software fallback is active and gamma_var != 1.0.
 */
void gamma_postprocess_apply(void);

#ifdef __cplusplus
}
#endif

#endif
