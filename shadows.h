#ifndef __SHADOWS_H__
#define __SHADOWS_H__

extern float fDestMat[16];
extern float fLightPos[4];
extern float fPlane[4];
extern float fSunPos[4];

extern int shadows_on;
extern int day_shadows_on;
extern int night_shadows_on;
extern int shadows_texture;

void SetShadowMatrix();
void draw_3d_object_shadow(object3d * object_id);
void draw_body_part_shadow(md2 *model_data,char *cur_frame, int ghost);
void draw_enhanced_actor_shadow(actor * actor_id);
void draw_actor_shadow(actor * actor_id);
void display_actors_shadow();
void display_shadows();
void display_3d_ground_objects();
void display_3d_non_ground_objects();
void draw_sun_shadowed_scene();

#endif
