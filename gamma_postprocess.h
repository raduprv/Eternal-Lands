/**
 * @file
 * @ingroup display_utils
 * @brief Software gamma correction fallback for platforms where
 *        SDL_SetWindowBrightness() is not supported (e.g. Wayland).
 *
 * SDL2's Wayland backend (and some X11/driver combinations) cannot set the
 * display gamma ramp - SDL_SetWindowBrightness() simply fails there. When
 * that happens, and the GPU supports shaders + framebuffer objects, we
 * render the frame into an offscreen buffer and apply the gamma curve
 * ourselves with a fragment shader before presenting it. Platforms where
 * SDL_SetWindowBrightness() already works are completely unaffected.
 */
#ifndef GAMMA_POSTPROCESS_H
#define GAMMA_POSTPROCESS_H

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief Non-zero if SDL_SetWindowBrightness() works on this system. */
extern int hardware_gamma_available;
/*! \brief Non-zero if the software gamma correction pass is active. */
extern int software_gamma_active;

/*!
 * \brief Try to set up the software gamma fallback.
 *
 * Must be called after the GL extensions have been loaded and
 * hardware_gamma_available has been determined. Only has an effect when
 * hardware gamma is unavailable, gamma adjustment is not disabled by the
 * user, and the required GL features (shaders, FBOs) are present. Safe to
 * call when those conditions are not met - it is then a no-op.
 */
void init_gamma_postprocess(void);

/*! \brief Release all GL resources owned by the software gamma fallback. */
void free_gamma_postprocess(void);

/*!
 * \brief Call at the very start of a frame.
 * \return Non-zero if the frame must be finished with
 *         gamma_postprocess_end_frame() before SDL_GL_SwapWindow().
 */
int gamma_postprocess_begin_frame(void);

/*!
 * \brief Call right before SDL_GL_SwapWindow(), only if
 *        gamma_postprocess_begin_frame() returned non-zero.
 */
void gamma_postprocess_end_frame(void);

#ifdef __cplusplus
}
#endif

#endif
