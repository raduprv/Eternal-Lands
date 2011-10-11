#ifdef EYE_CANDY

#ifndef __EYE_CANDY_WINDOW_H__
#define __EYE_CANDY_WINDOW_H__

#ifdef __cplusplus
 #include <vector>

 #include "../eye_candy/eye_candy.h"
 #include "../eye_candy_wrapper.h"
 
class EffectDefinition
{
public:
  EffectDefinition()
  {
    position = ec::Vec3(-1.0, -1.0, 0.0);
    reference = NULL;
  };
  
  ~EffectDefinition()
  {
//    if (reference)
//      ec_recall_effect(reference);
  };

  int effect;
  float hue;
  float saturation;
  float scale;
  float density;
  float base_height;
  ec::Vec3 position;
  float angle;
  ec::SmoothPolygonBoundingRange bounds;
  ec_reference reference;
};

extern EffectDefinition current_effect;
extern std::vector<EffectDefinition> effects;
extern EffectDefinition current_effect;

extern "C"
{
#endif //__cplusplus

extern int view_eye_candy_window;
extern int last_ec_index;
extern int eye_candy_window;
extern int eye_candy_confirmed;
extern int eye_candy_initialized;
extern int eye_candy_ready_to_add;

void draw_eye_candy_obj_info();
void create_eye_candy_window();
void change_eye_candy_effect();
void confirm_eye_candy_effect();
void remove_current_eye_candy_effect();
int display_eye_candy_window_handler();
int check_eye_candy_window_interface();
void update_eye_candy_position(float x, float y);
void add_eye_candy_point();
void delete_eye_candy_point();
void eye_candy_add_effect();
void eye_candy_done_adding_effect();
void eye_candy_adjust_z(float offset);
int eye_candy_get_effect();
void draw_bounds_on_minimap();
void draw_eye_candy_selectors();
void select_eye_candy_effect(int i);
void kill_eye_candy_effect();
int get_eye_candy_count();
void deserialize_eye_candy_effect(particles_io* data);
void serialize_eye_candy_effect(int index, particles_io* data);
void destroy_all_eye_candy();

#ifdef __cplusplus
}
void draw_bound(EffectDefinition& eff, bool selected);
void draw_eye_candy_selector(const EffectDefinition*const effect, const int i);
bool find_bounds_index(float x, float y);
ec::SmoothPolygonElement angle_to(float start_x, float start_y, float end_x, float end_y);

#endif //__cplusplus

#endif //__EYE_CANDY_WINDOW_H__

#endif // EYE_CANDY
