#ifndef __cal3d_wrap_h__
#define __cal3d_wrap_h__

#ifdef __cplusplus
extern "C" {
#endif
void create_cal3d_model();
void destroy_cal3d_model();
void init_cal3d_model();
void render_cal3d_model(actor * actor_id);
void update_cal3d_model();
#ifdef __cplusplus
}
#endif
#endif
