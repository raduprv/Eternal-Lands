#ifdef EYE_CANDY

#ifndef __EYE_CANDY_WINDOW_H__
#define __EYE_CANDY_WINDOW_H__

#ifdef __cplusplus
 #include <vector>

 #include "../elc/eye_candy/eye_candy.h"
 
typedef std::pair<float, float> bound_angle_type;

typedef struct {
  int effect;
  float hue;
  float saturation;
  float scale;
  float density;
  float base_height;
  ec::Vec3 position;
  float angle;
  std::vector<bound_angle_type> bounds;
} effect_definition;

extern effect_definition current_effect;
extern std::vector<effect_definition> effects;
extern effect_definition current_effect;

extern "C"
{
#endif //__cplusplus

extern int view_eye_candy_window;
extern int last_ec_index;
extern int eye_candy_window;

void create_eye_candy_window();
void change_eye_candy_effect();
void confirm_eye_candy_effect();
int display_eye_candy_window_handler();
int check_eye_candy_window_interface();
void add_eye_candy_point();
void delete_eye_candy_point();
void eye_candy_add_effect();
int eye_candy_get_effect();
void draw_bounds_on_minimap();

#ifdef __cplusplus
}

void draw_bound(effect_definition& eff, bool selected);
bool find_bounds_index(float x, float y);
bound_angle_type angle_to(float start_x, float start_y, float end_x, float end_y);
#endif //__cplusplus

#endif //__EYE_CANDY_WINDOW__

#endif // EYE_CANDY
