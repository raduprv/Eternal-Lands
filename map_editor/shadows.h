#ifndef __SHADOWS_H__
#define __SHADOWS_H__

extern float fDestMat[16];
extern float fLightPos[4];
extern float fPlane[4];
extern float fSunPos[4];

extern int shadows_on;
extern int day_shadows_on;
extern int night_shadows_on;
extern GLuint depth_map_id;

void SetShadowMatrix();
void draw_3d_object_shadow(object3d * object_id);
void display_shadows();
void display_night_shadows(int phase);
void display_3d_ground_objects();
void display_3d_non_ground_objects();
void draw_sun_shadowed_scene();
void draw_night_shadowed_scene();

#endif
