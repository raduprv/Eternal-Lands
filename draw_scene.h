/*!
 * \file
 * \ingroup display
 * \brief handles the (re-)drawing of the scene
 */
#ifndef __DRAW_SCENE_H__
#define __DRAW_SCENE_H__

extern GLuint paper1_text;

extern float cx,cy,cz;
extern float rx,ry,rz;
extern float camera_rotation_speed; /*!< current speed for rotations of the camera */
extern int camera_rotation_frames;
extern float camera_tilt_speed;
extern int camera_tilt_frames;
extern int normal_animation_timer;
/*
 * OBSOLETE: Queued for removal from this file.
 * They are only used in draw_scene.c, no need to declare them here.
 */
//extern double camera_x_speed; /*!< speed of the camera in x direction */
//extern int camera_x_frames;
//extern double camera_y_speed; /*!< speed of the camera in y direction */
//extern int camera_y_frames;
//extern double camera_z_speed; /*!< speed of the camera in z direction */
//extern int camera_z_frames;

extern float fine_camera_rotation_speed; /*!< configurable fine grained speed for rotating the camera */
extern float normal_camera_rotation_speed; /*!< configurable normal speed for rotating the camera */
extern float zoom_level; /*!< current displayed zoom level */
extern int camera_zoom_dir; /*!< direction of where the zoomed camera points to */
extern int camera_zoom_frames;
extern float new_zoom_level;
extern float scene_mouse_x; /*!< x coordinate of the mouse position */
extern float scene_mouse_y; /*!< y coordinate of the mouse position */

/*
 * OBSOLETE: Queued for removal from this file.
 * Only used in draw_scene.c, no need to declare it here.
 */
//extern float terrain_scale; /*!< scaling factor for terrain objects */

extern int last_texture; /*!< id of the last used texture */

/*! \name texture offset for movement of clouds. 
 * @{ */
extern float clouds_movement_u; /*!< offset for the u coordinate */
extern float clouds_movement_v; /*!< offset for the v coordinate */
/*! @} */

extern Uint32 last_clear_clouds; /*!< timestamp when the clouds cache gets cleared last time */

extern int read_mouse_now; /*!< flag to indicate to reread the status of the mouse */

#ifndef OLD_EVENT_HANDLER
extern Uint32 draw_delay; /*< the number of milliseconds to wait after drawing a frame */
#endif

extern GLenum base_unit,detail_unit,shadow_unit;

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
 * \brief       moves all actors in view range
 *
 *      Moves all actors in view range
 *
 * \callgraph
 */
void Move();

/*!
 * \ingroup display
 * \brief   updates the camera and redraws the scene
 *
 *      Updates the camera and redraws the scene
 *
 * \callgraph
 */
void update_camera();

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup network_actors
// * \brief      get_tmp_actor_data
// *
// *      get_tmp_actor_data() to be documented
// *
// * \sa draw_scene
// */
//void get_tmp_actor_data();

/*!
 * \ingroup misc_utils
 * \brief   CalculateFrustum
 *
 *      CalculateFrustum() to be documented
 *
 * \callgraph
 */
void CalculateFrustum();
#endif
