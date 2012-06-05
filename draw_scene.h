/*!
 * \file
 * \ingroup display
 * \brief handles the (re-)drawing of the scene
 */
#ifndef __DRAW_SCENE_H__
#define __DRAW_SCENE_H__

#include "platform.h"
#include "elwindows.h"

#ifdef __cplusplus
extern "C" {
#endif

extern float camera_x, camera_y, camera_z;
extern float rx,ry,rz;
extern float camera_rotation_speed; /*!< current speed for rotations of the camera */
extern int camera_rotation_duration;
extern float camera_tilt_speed;
extern int camera_tilt_duration;
extern int normal_animation_timer;

extern float normal_camera_deceleration;
extern float camera_rotation_deceleration;
extern float camera_tilt_deceleration;


//New camera features. See draw_scene.c for details
//Move comments here if desired.
extern float camera_kludge; /*!< Holds character's y rotation to allow camera to follow character  */
extern float last_kludge;
extern int fol_cam;
extern int fol_cam_behind;
extern float fol_con;
extern float fol_lin;
extern float fol_quad;
extern float fol_strn;
extern int ext_cam;
extern int ext_cam_auto_zoom;
extern float min_tilt_angle;
extern float max_tilt_angle;
extern float hold_camera;
extern int first_person;
extern int adjust_view;

extern float fine_camera_rotation_speed; /*!< configurable fine grained speed for rotating the camera */
extern float normal_camera_rotation_speed; /*!< configurable normal speed for rotating the camera */
extern float zoom_level; /*!< current displayed zoom level */
extern int camera_zoom_dir; /*!< direction of where the zoomed camera points to */
extern int camera_zoom_duration;
extern int camera_zoom_speed; /*!< A multipler for the camera zoom speed, increase if holding ctrl down */
extern float camera_distance; /*!< The camera is camera_distance*zoom_level (world coordinates) away from your actor. */
extern float new_zoom_level;
extern float max_zoom_level;

extern int reset_camera_at_next_update;

extern int last_texture; /*!< id of the last used texture */

/*! \name texture offset for movement of clouds. 
 * @{ */
extern float clouds_movement_u; /*!< offset for the u coordinate */
extern float clouds_movement_v; /*!< offset for the v coordinate */
/*! @} */

extern Uint32 last_clear_clouds; /*!< timestamp when the clouds cache gets cleared last time */

extern int read_mouse_now; /*!< flag to indicate to reread the status of the mouse */

extern Uint32 draw_delay; /*< the number of milliseconds to wait after drawing a frame */

extern GLenum base_unit,detail_unit,shadow_unit,extra_unit,normal_map_unit;

/*!
 * \ingroup display
 * \brief       draws the current scene and updates the display.
 *
 *      Draws the current scene and updates the display.
 *
 * \callgraph
 */
void draw_scene();

/*!
 * \ingroup move_actors
 * \brief       moves the camera
 *
 *      Moves the camera to the new character position
 *
 * \callgraph
 */
void move_camera ();

/*!
 * \ingroup display
 * \brief   Adjusts the camera rotation/angle/zoom if outside set limits
 *
 *      Adjusts the camera rotation/angle/zoom if outside set limits
 *
 * \callgraph
 */
void clamp_camera(void);

/*!
 * \ingroup display
 * \brief   updates the camera and redraws the scene
 *
 *      Updates the camera and redraws the scene
 *
 * \callgraph
 */
void update_camera();

/*!
 * \ingroup misc_utils
 * \brief   CalculateFrustum
 *
 *      CalculateFrustum() to be documented
 *
 * \callgraph
 */
void CalculateFrustum();

/*!
 * \ingroup	display
 * \brief	Window handler that updates the \see have_display flag.
 *
 *		Window handler that updates the \see have_display flag.
 */
int update_have_display(window_info *win);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
