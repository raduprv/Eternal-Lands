#ifndef __SHADOWS_H__
#define __SHADOWS_H__

extern float fDestMat[16];
extern float sun_position[4];
extern float fPlane[4];

extern int shadows_on;
extern int is_day;
extern int shadows_texture;
extern int use_shadow_mapping;
extern GLuint depth_map_id,z_texture_id;
extern GLenum depth_texture_target;
extern int max_shadow_map_size;

void calc_shadow_matrix();
void draw_3d_object_shadow(object3d * object_id);
void draw_body_part_shadow(md2 *model_data,char *cur_frame, int ghost);
void draw_enhanced_actor_shadow(actor * actor_id);
void draw_actor_shadow(actor * actor_id);
void display_actors_shadow();
void display_shadows();
void display_3d_ground_objects();
void display_3d_non_ground_objects();
void draw_sun_shadowed_scene(int any_reflection);
void render_light_view();
void disable_texgen();
void set_shadow_map_size();

#endif
