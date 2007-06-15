#ifndef _MINIMAP_H_
#define _MINIMAP_H_

#ifdef MINIMAP

#ifdef __cplusplus
extern "C" {
#endif

extern int minimap_win;
extern int minimap_win_x;
extern int minimap_win_y;
extern int minimap_flags;
extern int minimap_zoom;

int minimap_get_pin();

void display_minimap();
int display_minimap_handler(window_info *win);

/*!
 * \ingroup minimap
 * \brief Frees the minimap frame buffer.
 *
 * Frees the minimap frame buffer.
 *
 * \callgraph
 */
void minimap_free_framebuffer();

 /*!
 * \ingroup minimap
 * \brief Makes the minimap frame buffer.
 *
 * Makes the minimap frame buffer.
 *
 * \callgraph
 */
void minimap_make_framebuffer();

 /*!
 * \ingroup minimap
 * \brief Causes the minimap to be redrawn.
 *
 * Causes the minimap to be redrawn. Has no effect if not using the framebuffer.
 *
 * \callgraph
 */
void minimap_touch(void);

//called when map changes or window size changes (we have to reload textures)
void change_minimap();

//called when player moves
void update_exploration_map();

void save_exploration_map();

#ifdef __cplusplus
} // extern "C"
#endif

#endif //MINIMAP

#endif // _MINIMAP_H_

