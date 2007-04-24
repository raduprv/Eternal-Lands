#ifdef EYE_CANDY

#include "../elc/eye_candy/eye_candy.h"
#include <vector>
#include "eye_candy_window.h"

extern "C"
{
#include "global.h"
#include "shadows.h"

int view_eye_candy_window=0;
int last_ec_index = -2;	// None selected.
int eye_candy_window = -1;
}

static int eye_candy_window_x=15;
static int eye_candy_window_y=50;
static int eye_candy_window_x_len=600;
static int eye_candy_window_y_len=470;
std::vector<effect_definition> effects;
effect_definition current_effect;

extern "C" void create_eye_candy_window ()
{
  if (eye_candy_window < 0)
  {
    eye_candy_window = create_window ((Uint8*)"eye_candy", 0, 0, eye_candy_window_x, eye_candy_window_y, eye_candy_window_x_len, eye_candy_window_y_len, ELW_WIN_DEFAULT & ~ELW_SHOW);

    set_window_handler (eye_candy_window, ELW_HANDLER_DISPLAY, &display_eye_candy_window_handler);
    set_window_handler (eye_candy_window, ELW_HANDLER_CLICK, &check_eye_candy_window_interface);
  }
}

extern "C" void change_eye_candy_effect()
{
  current_effect.effect = gtk_combo_box_get_active(GTK_COMBO_BOX(gtk_effect_list));
  switch (current_effect.effect)
  {
    case 0:    // Fire
      gtk_widget_show(gtk_effect_hue_box);
      gtk_widget_show(gtk_effect_saturation_box);
      gtk_widget_show(gtk_effect_scale_box);
      gtk_widget_hide(gtk_effect_density_box);
      gtk_widget_hide(gtk_effect_base_height_box);
      break;
    case 1:    // Cloud/Fog
      gtk_widget_show(gtk_effect_hue_box);
      gtk_widget_show(gtk_effect_saturation_box);
      gtk_widget_hide(gtk_effect_scale_box);
      gtk_widget_show(gtk_effect_density_box);
      gtk_widget_show(gtk_effect_base_height_box);
      break;
    case 2:    // Fireflies
      gtk_widget_show(gtk_effect_hue_box);
      gtk_widget_show(gtk_effect_saturation_box);
      gtk_widget_show(gtk_effect_scale_box);
      gtk_widget_show(gtk_effect_density_box);
      gtk_widget_show(gtk_effect_base_height_box);
      break;
    case 3:    // Fountain
      gtk_widget_show(gtk_effect_hue_box);
      gtk_widget_show(gtk_effect_saturation_box);
      gtk_widget_show(gtk_effect_scale_box);
      gtk_widget_hide(gtk_effect_density_box);
      gtk_widget_show(gtk_effect_base_height_box);
      break;
    case 4:    // Lamp/Candle/Torch
      gtk_widget_show(gtk_effect_hue_box);
      gtk_widget_show(gtk_effect_saturation_box);
      gtk_widget_show(gtk_effect_scale_box);
      gtk_widget_hide(gtk_effect_density_box);
      gtk_widget_hide(gtk_effect_base_height_box);
      break;
    case 5:    // Magic Protection
      gtk_widget_show(gtk_effect_hue_box);
      gtk_widget_show(gtk_effect_saturation_box);
      gtk_widget_show(gtk_effect_scale_box);
      gtk_widget_hide(gtk_effect_density_box);
      gtk_widget_hide(gtk_effect_base_height_box);
      break;
    case 6:    // Shield
      gtk_widget_show(gtk_effect_hue_box);
      gtk_widget_show(gtk_effect_saturation_box);
      gtk_widget_show(gtk_effect_scale_box);
      gtk_widget_hide(gtk_effect_density_box);
      gtk_widget_hide(gtk_effect_base_height_box);
      break;
    case 7:    // Magic Immunity
      gtk_widget_show(gtk_effect_hue_box);
      gtk_widget_show(gtk_effect_saturation_box);
      gtk_widget_show(gtk_effect_scale_box);
      gtk_widget_hide(gtk_effect_density_box);
      gtk_widget_hide(gtk_effect_base_height_box);
      break;
    case 8:    // Poison
      gtk_widget_show(gtk_effect_hue_box);
      gtk_widget_show(gtk_effect_saturation_box);
      gtk_widget_show(gtk_effect_scale_box);
      gtk_widget_hide(gtk_effect_density_box);
      gtk_widget_hide(gtk_effect_base_height_box);
      break;
    case 9:    // Smoke
      gtk_widget_show(gtk_effect_hue_box);
      gtk_widget_show(gtk_effect_saturation_box);
      gtk_widget_show(gtk_effect_scale_box);
      gtk_widget_show(gtk_effect_density_box);
      gtk_widget_hide(gtk_effect_base_height_box);
      break;
    case 10:  // Teleporter
      gtk_widget_show(gtk_effect_hue_box);
      gtk_widget_show(gtk_effect_saturation_box);
      gtk_widget_show(gtk_effect_scale_box);
      gtk_widget_hide(gtk_effect_density_box);
      gtk_widget_hide(gtk_effect_base_height_box);
      break;
    case 11:  // Leaves
      gtk_widget_show(gtk_effect_hue_box);
      gtk_widget_show(gtk_effect_saturation_box);
      gtk_widget_show(gtk_effect_scale_box);
      gtk_widget_show(gtk_effect_density_box);
      gtk_widget_hide(gtk_effect_base_height_box);
      break;
    case 12:  // Flower Petals
      gtk_widget_show(gtk_effect_hue_box);
      gtk_widget_show(gtk_effect_saturation_box);
      gtk_widget_show(gtk_effect_scale_box);
      gtk_widget_show(gtk_effect_density_box);
      gtk_widget_hide(gtk_effect_base_height_box);
      break;
    case 13:  // Waterfall
      gtk_widget_show(gtk_effect_hue_box);
      gtk_widget_show(gtk_effect_saturation_box);
      gtk_widget_show(gtk_effect_scale_box);
      gtk_widget_hide(gtk_effect_density_box);
      gtk_widget_show(gtk_effect_base_height_box);
      break;
    case 14:  // Bees
      gtk_widget_show(gtk_effect_hue_box);
      gtk_widget_show(gtk_effect_saturation_box);
      gtk_widget_show(gtk_effect_scale_box);
      gtk_widget_show(gtk_effect_density_box);
      gtk_widget_hide(gtk_effect_base_height_box);
      break;
    case 15:  // Portal
      gtk_widget_show(gtk_effect_hue_box);
      gtk_widget_show(gtk_effect_saturation_box);
      gtk_widget_show(gtk_effect_scale_box);
      gtk_widget_hide(gtk_effect_density_box);
      gtk_widget_hide(gtk_effect_base_height_box);
      break;
  }
}

void confirm_eye_candy_effect()
{
  current_effect.position = ec::Vec3(-1.0, -1.0, -1.0);
  current_effect.effect = gtk_combo_box_get_active(GTK_COMBO_BOX(gtk_effect_list));
  current_effect.hue = GTK_ADJUSTMENT(gtk_effect_hue_obj)->value;
  current_effect.saturation = GTK_ADJUSTMENT(gtk_effect_saturation_obj)->value;
  current_effect.scale = GTK_ADJUSTMENT(gtk_effect_scale_obj)->value;
  current_effect.density = GTK_ADJUSTMENT(gtk_effect_density_obj)->value;
  current_effect.base_height = atoi(gtk_entry_get_text(GTK_ENTRY(gtk_effect_base_height)));
  gtk_widget_hide(gtk_effect_win);
  minimap_on = 1;
}

extern "C" int display_eye_candy_window_handler()
{
  return 1;
}

extern "C" int check_eye_candy_window_interface()
{
  return 1;
}

extern "C" void add_eye_candy_point()
{
  int minimap_x_start=window_width/2-128;
  int minimap_y_start;
  float x_map_pos;
  float y_map_pos;
  int scale;
  
  if(window_width<window_height) scale=window_width/256;
  else scale=window_height/256;

  minimap_x_start/=scale;
  minimap_y_start=10*scale;
  
  if(mouse_x<minimap_x_start || mouse_y<minimap_y_start
  || mouse_x>minimap_x_start+256*scale || mouse_y>minimap_y_start+256*scale) return;

  x_map_pos=((float)(mouse_x-minimap_x_start)/(float)scale)*tile_map_size_x/256;
  y_map_pos=tile_map_size_y-((mouse_y-minimap_y_start)/(float)scale)*tile_map_size_y/256;
  const float z = tile_map[(int)(y_map_pos)*tile_map_size_x+(int)x_map_pos];

  if (left_click <= 1)
  {
    const bool ret = find_bounds_index(x_map_pos, y_map_pos);
    if (!ret)  // Didn't click on anything; create new.
    {
      if ((current_effect.bounds.size() == 0) && (current_effect.position == ec::Vec3(-1.0, -1.0, -1.0)))
        current_effect.position = ec::Vec3(x_map_pos, y_map_pos, z);
      else if (current_effect.bounds.size() < 13)
        current_effect.bounds.insert(current_effect.bounds.begin() + last_ec_index, angle_to(current_effect.position.x, current_effect.position.y, x_map_pos, y_map_pos));
      else
        ; // Can't add any more; too many already.
    }
  }
  else
  {
    if (last_ec_index == -1)  // Clicked on the center; drag it.
      current_effect.position = ec::Vec3(x_map_pos, y_map_pos, z);
    else if (last_ec_index >= 0)      // Clicked on another point; drag it.
    {
      const bound_angle_type new_angle = angle_to(current_effect.position.x, current_effect.position.y, x_map_pos, y_map_pos);
      current_effect.bounds.erase(current_effect.bounds.begin() + last_ec_index);
      int i;
      for (i = 0; i < (int)current_effect.bounds.size(); i++)
      {
        if (new_angle.first < current_effect.bounds[i].first)
          break;
      }
      current_effect.bounds.insert(current_effect.bounds.begin() + i, new_angle);
      last_ec_index = i;
    }
  }
}

extern "C" void delete_eye_candy_point()
{
  int minimap_x_start=window_width/2-128;
  int minimap_y_start;
  float x_map_pos;
  float y_map_pos;
  int scale;

  if(window_width<window_height) scale=window_width/256;
  else scale=window_height/256;

  minimap_x_start/=scale;
  minimap_y_start=10*scale;
  
  if(mouse_x<minimap_x_start || mouse_y<minimap_y_start
  || mouse_x>minimap_x_start+256*scale || mouse_y>minimap_y_start+256*scale) return;

  x_map_pos=((float)(mouse_x-minimap_x_start)/(float)scale)*tile_map_size_x/256;
  y_map_pos=tile_map_size_y-(((mouse_y-minimap_y_start))/(float)scale)*tile_map_size_y/256;

  if (find_bounds_index(x_map_pos, y_map_pos))
  {
    if (last_ec_index >= 0)  // Clicked on a bounds point; delete it
    {
      current_effect.bounds.erase(current_effect.bounds.begin() + last_ec_index);
    }
    else if (last_ec_index == -1)  // Clicked on the center; cancel the effect
    {
      current_effect.position = ec::Vec3(-1.0, -1.0, -1.0);
      current_effect.bounds.clear();
      cur_mode = mode_tile;
      minimap_on = 0;
    }
  }
}

extern "C" void eye_candy_add_effect()
{
  effects.push_back(current_effect);
  current_effect.bounds.clear();
  current_effect.position = ec::Vec3(-1.0, -1.0, -1.0);
}

extern "C" int eye_candy_get_effect()
{
  return current_effect.effect;
}

extern "C" void draw_bounds_on_minimap()
{
  glDisable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  for (std::vector<effect_definition>::iterator iter = effects.begin(); iter != effects.end(); iter++)
    draw_bound(*iter, false);
  draw_bound(current_effect, true);
  glDisable(GL_BLEND);
  glEnable(GL_TEXTURE_2D);
}

void draw_bound(effect_definition& eff, bool selected)
{
  int minimap_x_start=window_width/2-128;
  int minimap_y_end;
  int scale;

  if(window_width<window_height) scale=window_width/256;
  else scale=window_height/256;

  minimap_x_start/=scale;
  minimap_y_end=(10 + 256)*scale;
  
  glLineWidth(1.3 * scale);
  
  if (selected)
    glColor4f(0.65, 0.55, 0.45, 0.7);
  else
    glColor4f(0.65, 0.55, 0.45, 0.25);

  glBegin(GL_LINE_LOOP);
  if (eff.bounds.size() >= 2)
  {
    std::vector<bound_angle_type>::const_iterator prev_iter = eff.bounds.begin() + (eff.bounds.size() - 1);
    std::vector<bound_angle_type>::const_iterator next_iter = eff.bounds.begin();
    bool wrapped = false;
    for (float f = -ec::PI; f < ec::PI; f += (2 * ec::PI) / 350.0)
    {
      if ((f > next_iter->first) && (!wrapped))
      {
        prev_iter = next_iter;
        next_iter++;
        if (next_iter == eff.bounds.end())
        {
          wrapped = true;
          next_iter = eff.bounds.begin();
        }
      }
      float percent;
      if (wrapped)
        percent = (f - prev_iter->first) / (next_iter->first + (2 * ec::PI) - prev_iter->first);
      else if (f > prev_iter->first)
        percent = (f - prev_iter->first) / (next_iter->first - prev_iter->first);
      else
        percent = (f + 2 * ec::PI - prev_iter->first) / (next_iter->first + (2 * ec::PI) - prev_iter->first);
      const float dist = prev_iter->second * (1.0 - percent) + next_iter->second * percent;
      const float temp_x = minimap_x_start + (eff.position.x + dist * sin(f)) * scale;
      const float temp_y = minimap_y_end - (eff.position.y + dist * cos(f)) * scale;
      glVertex2f(temp_x, temp_y);
    }
  }
  glEnd();

  if (selected)
  {
    glColor4f(1.0, 1.0, 1.0, 0.7);
    glBegin(GL_QUADS);
    for (std::vector<bound_angle_type>::const_iterator iter = eff.bounds.begin(); iter != eff.bounds.end(); iter++)
    {
      const float temp_x = minimap_x_start + (eff.position.x + iter->second * sin(iter->first)) * scale;
      const float temp_y = minimap_y_end - (eff.position.y + iter->second * cos(iter->first)) * scale;
      glVertex2f(temp_x - 2.0 * scale, temp_y - 2.0 * scale);
      glVertex2f(temp_x - 2.0 * scale, temp_y + 2.0 * scale);
      glVertex2f(temp_x + 2.0 * scale, temp_y + 2.0 * scale);
      glVertex2f(temp_x + 2.0 * scale, temp_y - 2.0 * scale);
    }
    
    glColor4f(1.0, 0.8, 0.6, 1.0);
    const float temp_x = minimap_x_start + eff.position.x * scale;
    const float temp_y = minimap_y_end - eff.position.y * scale;
    glVertex2f(temp_x - 2.0 * scale, temp_y - 2.0 * scale);
    glVertex2f(temp_x - 2.0 * scale, temp_y + 2.0 * scale);
    glVertex2f(temp_x + 2.0 * scale, temp_y + 2.0 * scale);
    glVertex2f(temp_x + 2.0 * scale, temp_y - 2.0 * scale);
    glEnd();
  }
}

bool find_bounds_index(float x, float y)
{
  if ((current_effect.position.x == -1.0) && (current_effect.position.y == -1.0))
  {
    last_ec_index = -1;
    return false;
  }

  if ((fabs(x - current_effect.position.x) < 3.0) && (fabs(y - current_effect.position.y) < 3.0))
  {
    last_ec_index = -1;
    return true;
  }

  for (int i = 0; i < (int)current_effect.bounds.size(); i++)
  {
    std::vector<bound_angle_type>::const_iterator iter = current_effect.bounds.begin() + i;
    const float temp_x = current_effect.position.x + iter->second * sin(iter->first);
    const float temp_y = current_effect.position.y + iter->second * cos(iter->first);
    if ((fabs(x - temp_x) < 3.0) && (fabs(y - temp_y) < 3.0))
    {
      last_ec_index = i;
      return true;
    }
  }

  float cur_angle = angle_to(current_effect.position.x, current_effect.position.y, x, y).first;
  last_ec_index = current_effect.bounds.size();
  for (int i = 0; i < (int)current_effect.bounds.size(); i++)
  {
    const float angle = current_effect.bounds[i].first;
    if (cur_angle < angle)
    {
      last_ec_index = i;
      break;
    }
  }

  return false;
}

bound_angle_type angle_to(float start_x, float start_y, float end_x, float end_y)
{
  const float diff_x = end_x - start_x;
  const float diff_y = end_y - start_y;
  const float angle = atan2(diff_x, diff_y);
  const float dist = sqrt(diff_x * diff_x + diff_y * diff_y);
  return bound_angle_type(angle, dist);
}

#endif // EYE_CANDY
