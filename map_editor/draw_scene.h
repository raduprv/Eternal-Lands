#ifndef	__DRAW_SCENE_H
#define	__DRAW_SCENE_H

void draw_scene();
void Move();
void update_camera();

Uint32 my_timer (Uint32 interval);

extern float camera_x,camera_y,camera_z;
extern float rx,ry,rz;
extern float mx,my,mz;
extern float camera_rotation_speed; /*!< current speed for rotations of the camera */
extern int camera_rotation_frames;
extern float camera_tilt_speed;
extern int camera_tilt_frames;
extern int normal_animation_timer;

extern float fine_camera_rotation_speed; /*!< configurable fine grained speed for rotating the camera */
extern float normal_camera_rotation_speed; /*!< configurable normal speed for rotating the camera */
extern float zoom_level; /*!< current displayed zoom level */
extern int camera_zoom_dir; /*!< direction of where the zoomed camera points to */
extern int camera_zoom_frames;
extern float camera_distance; /*!< The camera is camera_distance*zoom_level (world coordinates) away from your actor. */
extern float new_zoom_level;



#endif	//__DRAW__SCENE_H

