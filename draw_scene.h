#ifndef __DRAW_SCENE_H__
#define __DRAW_SCENE_H__

extern float cx,cy,cz;
extern float rx,ry,rz;
extern float camera_rotation_speed;
extern int camera_rotation_frames;
extern float camera_tilt_speed;
extern int camera_tilt_frames;
extern int normal_animation_timer;
extern double camera_x_speed;
extern int camera_x_frames;
extern double camera_y_speed;
extern int camera_y_frames;
extern double camera_z_speed;
extern int camera_z_frames;
extern float fine_camera_rotation_speed;
extern float normal_camera_rotation_speed;
extern float zoom_level;
extern int camera_zoom_dir;
extern int camera_zoom_frames;
extern float new_zoom_level;
extern float scene_mouse_x;
extern float scene_mouse_y;

extern float terrain_scale;
extern int last_texture;

extern float clouds_movement_u;
extern float clouds_movement_v;
extern Uint32 last_clear_clouds;

extern int read_mouse_now;

extern int	my_timer_adjust;
extern int	my_timer_clock;

extern GLenum base_unit,detail_unit,shadow_unit;

void draw_scene();
void Move();
void update_camera();
Uint32 my_timer(unsigned int some_int);

void CalculateFrustum();
#endif
