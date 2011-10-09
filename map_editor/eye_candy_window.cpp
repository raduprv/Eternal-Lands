/*
TODO: 
 * Get Roja working.
 * New effect options.
 * New effects.
*/

#ifdef EYE_CANDY

extern "C"
{
#include "global.h"
#include "interface.h"
#include "elwindows.h"
#include "shadows.h"
#include "gui.h"
#include "editor.h"
}

#include "../eye_candy/eye_candy.h"
#include <vector>
#include "eye_candy_window.h"

extern "C"
{
int view_eye_candy_window=0;
int last_ec_index = -2;	// None selected.
int eye_candy_window = -1;
int eye_candy_confirmed = 0;
int eye_candy_initialized = 0;
int eye_candy_ready_to_add = 0;
}

static int eye_candy_window_x=15;
static int eye_candy_window_y=50;
static int eye_candy_window_x_len=600;
static int eye_candy_window_y_len=470;
std::vector<EffectDefinition> effects;
EffectDefinition current_effect;

extern "C" void create_eye_candy_window ()
{
  if (eye_candy_window < 0)
  {
    eye_candy_window = create_window ("eye_candy", 0, 0, eye_candy_window_x, eye_candy_window_y, eye_candy_window_x_len, eye_candy_window_y_len, ELW_WIN_DEFAULT & ~ELW_SHOW);

    set_window_handler (eye_candy_window, ELW_HANDLER_DISPLAY, &display_eye_candy_window_handler);
    set_window_handler (eye_candy_window, ELW_HANDLER_CLICK, &check_eye_candy_window_interface);
  }
}

extern "C" void change_eye_candy_effect()
{
  if (!eye_candy_initialized)
    return;
  current_effect.effect = gtk_combo_box_get_active(GTK_COMBO_BOX(gtk_effect_list));
  current_effect.hue = GTK_ADJUSTMENT(gtk_effect_hue_obj)->value;
  current_effect.saturation = GTK_ADJUSTMENT(gtk_effect_saturation_obj)->value;
  current_effect.scale = GTK_ADJUSTMENT(gtk_effect_scale_obj)->value;
  current_effect.density = GTK_ADJUSTMENT(gtk_effect_density_obj)->value;
  current_effect.base_height = atoi(gtk_entry_get_text(GTK_ENTRY(gtk_effect_base_height)));
  if (current_effect.reference)
  {
    ec_recall_effect(current_effect.reference);
    current_effect.reference = NULL;
  }
  switch (current_effect.effect)
  {
    case 0:    // Fire
      gtk_widget_show(gtk_effect_hue_box);
      gtk_widget_show(gtk_effect_saturation_box);
      gtk_widget_show(gtk_effect_scale_box);
      gtk_widget_hide(gtk_effect_density_box);
      gtk_widget_hide(gtk_effect_base_height_box);
      current_effect.reference = ec_create_campfire(current_effect.position.x, current_effect.position.y, current_effect.position.z, current_effect.hue, current_effect.saturation, 10, current_effect.scale);
      break;
    case 1:    // Cloud/Fog
      gtk_widget_show(gtk_effect_hue_box);
      gtk_widget_show(gtk_effect_saturation_box);
      gtk_widget_hide(gtk_effect_scale_box);
      gtk_widget_show(gtk_effect_density_box);
      gtk_widget_hide(gtk_effect_base_height_box);
      current_effect.reference = ec_create_cloud(current_effect.position.x, current_effect.position.y, current_effect.position.z, current_effect.hue, current_effect.saturation, current_effect.density, (current_effect.bounds.elements.size() > 1 ? &current_effect.bounds : &initial_bounds), 10);
      break;
    case 2:    // Fireflies
      gtk_widget_show(gtk_effect_hue_box);
      gtk_widget_show(gtk_effect_saturation_box);
      gtk_widget_show(gtk_effect_scale_box);
      gtk_widget_show(gtk_effect_density_box);
      gtk_widget_hide(gtk_effect_base_height_box);
      current_effect.reference = ec_create_fireflies(current_effect.position.x, current_effect.position.y, current_effect.position.z, current_effect.hue, current_effect.saturation, current_effect.density, current_effect.scale, (current_effect.bounds.elements.size() > 1 ? &current_effect.bounds : &initial_bounds));
      break;
    case 3:    // Fountain
      gtk_widget_show(gtk_effect_hue_box);
      gtk_widget_show(gtk_effect_saturation_box);
      gtk_widget_show(gtk_effect_scale_box);
      gtk_widget_hide(gtk_effect_density_box);
      gtk_widget_show(gtk_effect_base_height_box);
      current_effect.reference = ec_create_fountain(current_effect.position.x, current_effect.position.y, current_effect.position.z, current_effect.hue, current_effect.saturation, current_effect.base_height, false, current_effect.scale, 10);
      break;
    case 4:    // Lamp/Torch
      gtk_widget_show(gtk_effect_hue_box);
      gtk_widget_show(gtk_effect_saturation_box);
      gtk_widget_show(gtk_effect_scale_box);
      gtk_widget_hide(gtk_effect_density_box);
      gtk_widget_hide(gtk_effect_base_height_box);
      current_effect.reference = ec_create_lamp(current_effect.position.x, current_effect.position.y, current_effect.position.z, current_effect.hue, current_effect.saturation, current_effect.scale, 10);
      break;
    case 5:    // Magic Protection
      gtk_widget_show(gtk_effect_hue_box);
      gtk_widget_show(gtk_effect_saturation_box);
      gtk_widget_show(gtk_effect_scale_box);
      gtk_widget_hide(gtk_effect_density_box);
      gtk_widget_hide(gtk_effect_base_height_box);
      current_effect.reference = ec_create_ongoing_magic_protection(current_effect.position.x, current_effect.position.y, current_effect.position.z, current_effect.hue, current_effect.saturation, 10, current_effect.scale);
      break;
    case 6:    // Shield
      gtk_widget_show(gtk_effect_hue_box);
      gtk_widget_show(gtk_effect_saturation_box);
      gtk_widget_show(gtk_effect_scale_box);
      gtk_widget_hide(gtk_effect_density_box);
      gtk_widget_hide(gtk_effect_base_height_box);
      current_effect.reference = ec_create_ongoing_shield(current_effect.position.x, current_effect.position.y, current_effect.position.z, current_effect.hue, current_effect.saturation, 10, current_effect.scale);
      break;
    case 7:    // Magic Immunity
      gtk_widget_show(gtk_effect_hue_box);
      gtk_widget_show(gtk_effect_saturation_box);
      gtk_widget_show(gtk_effect_scale_box);
      gtk_widget_hide(gtk_effect_density_box);
      gtk_widget_hide(gtk_effect_base_height_box);
      current_effect.reference = ec_create_ongoing_magic_immunity(current_effect.position.x, current_effect.position.y, current_effect.position.z, current_effect.hue, current_effect.saturation, 10, current_effect.scale);
      break;
    case 8:    // Poison
      gtk_widget_show(gtk_effect_hue_box);
      gtk_widget_show(gtk_effect_saturation_box);
      gtk_widget_show(gtk_effect_scale_box);
      gtk_widget_hide(gtk_effect_density_box);
      gtk_widget_hide(gtk_effect_base_height_box);
      current_effect.reference = ec_create_ongoing_poison(current_effect.position.x, current_effect.position.y, current_effect.position.z, current_effect.hue, current_effect.saturation, 10, current_effect.scale);
      break;
    case 9:    // Smoke
      gtk_widget_show(gtk_effect_hue_box);
      gtk_widget_show(gtk_effect_saturation_box);
      gtk_widget_hide(gtk_effect_scale_box);
      gtk_widget_show(gtk_effect_density_box);
      gtk_widget_hide(gtk_effect_base_height_box);
      current_effect.reference = ec_create_smoke(current_effect.position.x, current_effect.position.y, current_effect.position.z, current_effect.hue, current_effect.saturation, current_effect.scale, 10);
      break;
    case 10:  // Teleporter
      gtk_widget_show(gtk_effect_hue_box);
      gtk_widget_show(gtk_effect_saturation_box);
      gtk_widget_show(gtk_effect_scale_box);
      gtk_widget_hide(gtk_effect_density_box);
      gtk_widget_hide(gtk_effect_base_height_box);
      current_effect.reference = ec_create_teleporter(current_effect.position.x, current_effect.position.y, current_effect.position.z, current_effect.hue, current_effect.saturation, current_effect.scale, 10);
      break;
    case 11:  // Leaves
      gtk_widget_show(gtk_effect_hue_box);
      gtk_widget_show(gtk_effect_saturation_box);
      gtk_widget_show(gtk_effect_scale_box);
      gtk_widget_show(gtk_effect_density_box);
      gtk_widget_hide(gtk_effect_base_height_box);
//      std::cout << "1: " << current_effect.hue << " / " << current_effect.saturation << " / " << current_effect.scale << " / " << current_effect.density << std::endl;
      current_effect.reference = ec_create_wind_leaves(current_effect.position.x, current_effect.position.y, current_effect.position.z, current_effect.hue, current_effect.saturation, current_effect.scale, current_effect.density, (current_effect.bounds.elements.size() > 1 ? &current_effect.bounds : &initial_bounds), 1.0, 0.0, 0.0);
      break;
    case 12:  // Flower Petals
      gtk_widget_show(gtk_effect_hue_box);
      gtk_widget_show(gtk_effect_saturation_box);
      gtk_widget_show(gtk_effect_scale_box);
      gtk_widget_show(gtk_effect_density_box);
      gtk_widget_hide(gtk_effect_base_height_box);
      current_effect.reference = ec_create_wind_petals(current_effect.position.x, current_effect.position.y, current_effect.position.z, current_effect.hue, current_effect.saturation, current_effect.scale, current_effect.density, (current_effect.bounds.elements.size() > 1 ? &current_effect.bounds : &initial_bounds), 1.0, 0.0, 0.0);
      break;
    case 13:  // Waterfall
      gtk_widget_show(gtk_effect_hue_box);
      gtk_widget_show(gtk_effect_saturation_box);
      gtk_widget_show(gtk_effect_scale_box);
      gtk_widget_hide(gtk_effect_density_box);
      gtk_widget_show(gtk_effect_base_height_box);
//      current_effect.reference = ec_create_waterfall(current_effect.position.x, current_effect.position.y, current_effect.position.z, current_effect.hue, current_effect.saturation);
      break;
    case 14:  // Bees
      gtk_widget_show(gtk_effect_hue_box);
      gtk_widget_show(gtk_effect_saturation_box);
      gtk_widget_show(gtk_effect_scale_box);
      gtk_widget_show(gtk_effect_density_box);
      gtk_widget_hide(gtk_effect_base_height_box);
      current_effect.reference = NULL;
//      current_effect.reference = ec_create_bees(current_effect.position.x, current_effect.position.y, current_effect.position.z, current_effect.hue, current_effect.saturation);
      break;
    case 15:  // Portal
      gtk_widget_show(gtk_effect_hue_box);
      gtk_widget_show(gtk_effect_saturation_box);
      gtk_widget_show(gtk_effect_scale_box);
      gtk_widget_hide(gtk_effect_density_box);
      gtk_widget_hide(gtk_effect_base_height_box);
      current_effect.reference = NULL;
//      current_effect.reference = ec_create_bees(current_effect.position.x, current_effect.position.y, current_effect.position.z, current_effect.hue, current_effect.saturation);
      break;
    case 16:    // Candle
      gtk_widget_show(gtk_effect_hue_box);
      gtk_widget_show(gtk_effect_saturation_box);
      gtk_widget_show(gtk_effect_scale_box);
      gtk_widget_hide(gtk_effect_density_box);
      gtk_widget_hide(gtk_effect_base_height_box);
      current_effect.reference = ec_create_candle(current_effect.position.x, current_effect.position.y, current_effect.position.z, current_effect.hue, current_effect.saturation, current_effect.scale, 10);
      break;
  }
}

extern "C" void remove_current_eye_candy_effect()
{
  if (current_effect.reference)
  {
    ec_recall_effect(current_effect.reference);
    current_effect.reference = NULL;
  }
}

void confirm_eye_candy_effect()
{
  eye_candy_confirmed = 1;
  change_eye_candy_effect();
  gtk_widget_hide(gtk_effect_win);
  switch (current_effect.effect)
  {
    case 1:    // Cloud/Fog
    case 2:    // Fireflies
    case 11:   // Leaves
    case 12:   // Flower Petals
    {
      current_effect.position = ec::Vec3(-1.0, -1.0, 0.0);
      minimap_on = 1;
      break;
    }
  }
  eye_candy_ready_to_add = 1;
}

extern "C" int display_eye_candy_window_handler()
{
  return 1;
}

extern "C" int check_eye_candy_window_interface()
{
  return 1;
}

extern "C" void update_eye_candy_position(float x, float y)
{
  if (!minimap_on)
  {
    switch (current_effect.effect)
    {
      case 1:    // Cloud/Fog
      case 2:    // Fireflies
      case 11:   // Leaves
      case 12:   // Flower Petals
      {
        if (current_effect.bounds.elements.size() > 1)
          return;
      }
    }
    current_effect.position.x = x;
    current_effect.position.y = y;
    if (current_effect.reference)
      ec_set_position(current_effect.reference, x, y, current_effect.position.z);
  }
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

  x_map_pos=((float)(mouse_x-minimap_x_start)/(float)scale);
  y_map_pos=256-((mouse_y-minimap_y_start)/(float)scale);
  const float z = -2.2f + tile_map[(int)(y_map_pos)*tile_map_size_x+(int)x_map_pos] * 0.2f;
  x_map_pos = x_map_pos * 3 / (256 / tile_map_size_x);
  y_map_pos = y_map_pos * 3 / (256 / tile_map_size_y);
  
  if (left_click <= 1)
  {
    const bool ret = find_bounds_index(x_map_pos, y_map_pos);
    if (!ret)  // Didn't click on anything; create new.
    {
      if ((current_effect.bounds.elements.size() == 0) && (current_effect.position == ec::Vec3(-1.0, -1.0, 0.0)))
        current_effect.position = ec::Vec3(x_map_pos, y_map_pos, z);
      else if (current_effect.bounds.elements.size() < 13)
        current_effect.bounds.elements.insert(current_effect.bounds.elements.begin() + last_ec_index, angle_to(current_effect.position.x, current_effect.position.y, x_map_pos, y_map_pos));
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
      const ec::SmoothPolygonElement new_angle = angle_to(current_effect.position.x, current_effect.position.y, x_map_pos, y_map_pos);
      current_effect.bounds.elements.erase(current_effect.bounds.elements.begin() + last_ec_index);
      int i;
      for (i = 0; i < (int)current_effect.bounds.elements.size(); i++)
      {
        if (new_angle.angle < current_effect.bounds.elements[i].angle)
          break;
      }
      current_effect.bounds.elements.insert(current_effect.bounds.elements.begin() + i, new_angle);
      last_ec_index = i;
    }
  }
  change_eye_candy_effect();
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

  x_map_pos=((float)(mouse_x-minimap_x_start)/(float)scale);
  y_map_pos=256-(((mouse_y-minimap_y_start))/(float)scale);
  x_map_pos = x_map_pos * 3 / (256 / tile_map_size_x);
  y_map_pos = y_map_pos * 3 / (256 / tile_map_size_y);

  if (find_bounds_index(x_map_pos, y_map_pos))
  {
    if (last_ec_index >= 0)  // Clicked on a bounds point; delete it
    {
      current_effect.bounds.elements.erase(current_effect.bounds.elements.begin() + last_ec_index);
    }
    else if (last_ec_index == -1)  // Clicked on the center; cancel the effect
    {
      current_effect = EffectDefinition();
      eye_candy_done_adding_effect();
      cur_mode = mode_tile;
      minimap_on = 0;
    }
  }
}

extern "C" void eye_candy_add_effect()
{
  if (eye_candy_initialized && eye_candy_ready_to_add)
  {
    effects.push_back(current_effect);
    current_effect.reference = NULL;
    switch (current_effect.effect)
    {
      case 1:    // Cloud/Fog
      case 2:    // Fireflies
      case 11:   // Leaves
      case 12:   // Flower Petals
      {
        current_effect.effect = 0;
        cur_mode = mode_tile;
        return;
      }
      default:
      {
        change_eye_candy_effect();
        current_effect.bounds = ec::SmoothPolygonBoundingRange();
      }
    }
  }
}

extern "C" void eye_candy_done_adding_effect()
{
  if (eye_candy_initialized)
  {
    eye_candy_ready_to_add = 0;
    if (current_effect.reference)
    {
      switch (current_effect.effect)
      {
        case 1:    // Cloud/Fog
        case 2:    // Fireflies
        case 11:   // Leaves
        case 12:   // Flower Petals
        {
          effects.push_back(current_effect);
          break;
        }
        default:
        {
          remove_current_eye_candy_effect();
        }
      }
      current_effect.reference = NULL;
      current_effect.bounds = ec::SmoothPolygonBoundingRange();
      current_effect.position = ec::Vec3(-1.0, -1.0, 0.0);
    }
  }
}

extern "C" int eye_candy_get_effect()
{
  return current_effect.effect;
}

void eye_candy_adjust_z(float offset)
{
  current_effect.position.z += offset;
  change_eye_candy_effect();
}

extern "C" void draw_bounds_on_minimap()
{
  glDisable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  for (std::vector<EffectDefinition>::iterator iter = effects.begin(); iter != effects.end(); iter++)
    draw_bound(*iter, false);
  if (eye_candy_ready_to_add)
    draw_bound(current_effect, true);
  glDisable(GL_BLEND);
  glEnable(GL_TEXTURE_2D);
}

void draw_bound(EffectDefinition& eff, bool selected)
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
  if (eff.bounds.elements.size() >= 2)
  {
    std::vector<ec::SmoothPolygonElement>::const_iterator prev_iter = eff.bounds.elements.begin() + (eff.bounds.elements.size() - 1);
    std::vector<ec::SmoothPolygonElement>::const_iterator next_iter = eff.bounds.elements.begin();
    bool wrapped = false;
    for (float f = 0; f < 2 * ec::PI; f += (2 * ec::PI) / 360.0)
    {
      if ((f > next_iter->angle) && (!wrapped))
      {
        prev_iter = next_iter;
        next_iter++;
        if (next_iter == eff.bounds.elements.end())
        {
          wrapped = true;
          next_iter = eff.bounds.elements.begin();
        }
      }
      float percent;
      if (wrapped)
        percent = (f - prev_iter->angle) / (next_iter->angle + (2 * ec::PI) - prev_iter->angle);
      else if (f > prev_iter->angle)
        percent = (f - prev_iter->angle) / (next_iter->angle - prev_iter->angle);
      else
        percent = (f + 2 * ec::PI - prev_iter->angle) / (next_iter->angle + (2 * ec::PI) - prev_iter->angle);
      const float dist = prev_iter->radius * (1.0 - percent) + next_iter->radius * percent;
      const float temp_x = minimap_x_start + ((eff.position.x - dist * sin(f)) * scale) / 3 * (256 / tile_map_size_x);
      const float temp_y = minimap_y_end - ((eff.position.y + dist * cos(f)) * scale) / 3 * (256 / tile_map_size_y);
      glVertex2f(temp_x, temp_y);
    }
  }
  glEnd();

  if (selected)
  {
    glColor4f(1.0, 1.0, 1.0, 0.7);
    glBegin(GL_QUADS);
    for (std::vector<ec::SmoothPolygonElement>::const_iterator iter = eff.bounds.elements.begin(); iter != eff.bounds.elements.end(); iter++)
    {
      const float temp_x = minimap_x_start + ((eff.position.x - iter->radius * sin(iter->angle)) * scale) / 3 * (256 / tile_map_size_x);
      const float temp_y = minimap_y_end - ((eff.position.y + iter->radius * cos(iter->angle)) * scale) / 3 * (256 / tile_map_size_y);
      glVertex2f(temp_x - 2.0 * scale, temp_y - 2.0 * scale);
      glVertex2f(temp_x - 2.0 * scale, temp_y + 2.0 * scale);
      glVertex2f(temp_x + 2.0 * scale, temp_y + 2.0 * scale);
      glVertex2f(temp_x + 2.0 * scale, temp_y - 2.0 * scale);
    }
    
    glColor4f(1.0, 0.8, 0.6, 1.0);
    const float temp_x = minimap_x_start + (eff.position.x * scale) / 3 * (256 / tile_map_size_x);
    const float temp_y = minimap_y_end - (eff.position.y * scale) / 3 * (256 / tile_map_size_y);
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

  if ((fabs(x - current_effect.position.x) < 9.0) && (fabs(y - current_effect.position.y) < 9.0))
  {
    last_ec_index = -1;
    return true;
  }

  for (int i = 0; i < (int)current_effect.bounds.elements.size(); i++)
  {
    std::vector<ec::SmoothPolygonElement>::const_iterator iter = current_effect.bounds.elements.begin() + i;
    const float temp_x = current_effect.position.x - iter->radius * sin(iter->angle);
    const float temp_y = current_effect.position.y + iter->radius * cos(iter->angle);
    if ((fabs(x - temp_x) < 9.0) && (fabs(y - temp_y) < 9.0))
    {
      last_ec_index = i;
      return true;
    }
  }

  float cur_angle = angle_to(current_effect.position.x, current_effect.position.y, x, y).angle;
  last_ec_index = current_effect.bounds.elements.size();
  for (int i = 0; i < (int)current_effect.bounds.elements.size(); i++)
  {
    const float angle = current_effect.bounds.elements[i].angle;
    if (cur_angle < angle)
    {
      last_ec_index = i;
      break;
    }
  }

  return false;
}

ec::SmoothPolygonElement angle_to(float start_x, float start_y, float end_x, float end_y)
{
  const float diff_x = -(end_x - start_x);
  const float diff_y = end_y - start_y;
  float angle = atan2(diff_x, diff_y);
  if (angle < 0)
    angle += 2.0 * ec::PI;
  const float dist = sqrt(diff_x * diff_x + diff_y * diff_y);
  return ec::SmoothPolygonElement(angle, dist);
}

void draw_eye_candy_obj_info()
{
  unsigned char str[128];
  int x_menu,y_menu;
  if (cur_mode!=mode_eye_candy || !eye_candy_confirmed)
    return;

  x_menu=0;
  y_menu=window_height-72;
  //draw a black rectangle
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE,GL_SRC_ALPHA);
  glDisable(GL_TEXTURE_2D);
  glBegin(GL_QUADS);
  glColor4f(0.0f,0.0f,0.0f,0.5f);
  glVertex3i(x_menu,y_menu+70,0);
  glVertex3i(x_menu,y_menu,0);
  glVertex3i(x_menu+600,y_menu,0);
  glVertex3i(x_menu+600,y_menu+70,0);
  glColor3f(1.0f,1.0f,1.0f);
  glEnd();
  glEnable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);

  x_menu+=2;
  y_menu+=2;

  sprintf((char *)str, "X Pos: %03.2f",current_effect.position.x);
  draw_string(x_menu,y_menu,str,1);

  y_menu+=17;
  sprintf((char *)str, "Y Pos: %03.2f",current_effect.position.y);
  draw_string(x_menu,y_menu,str,1);

  y_menu+=17;
  sprintf((char *)str, "Z Pos: %03.2f",current_effect.position.z);
  draw_string(x_menu,y_menu,str,1);
/////////////////////////////////////////////////
  x_menu+=15*12;
  y_menu-=17*2;

  sprintf((char *)str, "Angle   : %03.2f",current_effect.angle);
  draw_string(x_menu,y_menu,str,1);

  y_menu+=17;
  sprintf((char *)str, "Density : %03.2f",current_effect.density);
  draw_string(x_menu,y_menu,str,1);

  y_menu+=17;
  sprintf((char *)str, "Scale   : %03.2f",current_effect.scale);
  draw_string(x_menu,y_menu,str,1);
/////////////////////////////////////////////////
  x_menu+=17*12;
  y_menu-=17*2;

  sprintf((char *)str, "Hue       : %03.2f",current_effect.hue);
  draw_string(x_menu,y_menu,str,1);

  y_menu+=17;
  sprintf((char *)str, "Saturation: %03.2f",current_effect.saturation);
  draw_string(x_menu,y_menu,str,1);

  y_menu+=17;
  sprintf((char *)str, "Height    : %03.2f",current_effect.base_height);
  draw_string(x_menu,y_menu,str,1);
}

void draw_eye_candy_selectors()
{
  glEnable(GL_COLOR);
  glEnable(GL_LIGHTING);
  glDisable(GL_TEXTURE_2D);
  glColor4f(0.5, 0.5, 0.5, 1.0);
  int i=0;
  if (eye_candy_ready_to_add)
    draw_eye_candy_selector(&current_effect, i);
  i++;
  for (std::vector<EffectDefinition>::const_iterator iter = effects.begin(); iter != effects.end(); iter++, i++)
    draw_eye_candy_selector(&(*iter), i);
  glDisable(GL_COLOR);
  glDisable(GL_LIGHTING);
  glEnable(GL_TEXTURE_2D);
}
    
void draw_eye_candy_selector(const EffectDefinition*const effect, const int i)
{
  glPushMatrix();
  glTranslatef(effect->position.x, effect->position.y, effect->position.z);
  glLoadName (MAX_OBJ_3D + i);
  glBegin(GL_QUADS);
  {
    // Front Face
    glNormal3f(0.0, 0.0, 1.0);
    glVertex3f(-0.15f, -0.15f,  0.15f);
    glVertex3f( 0.15f, -0.15f,  0.15f);
    glVertex3f( 0.15f,  0.15f,  0.15f);
    glVertex3f(-0.15f,  0.15f,  0.15f);
    // Back Face
    glNormal3f(0.0, 0.0, -1.0);
    glVertex3f(-0.15f, -0.15f, -0.15f);
    glVertex3f(-0.15f,  0.15f, -0.15f);
    glVertex3f( 0.15f,  0.15f, -0.15f);
    glVertex3f( 0.15f, -0.15f, -0.15f);
    // Top Face
    glNormal3f(0.0, 1.0, 0.0);
    glVertex3f(-0.15f,  0.15f, -0.15f);
    glVertex3f(-0.15f,  0.15f,  0.15f);
    glVertex3f( 0.15f,  0.15f,  0.15f);
    glVertex3f( 0.15f,  0.15f, -0.15f);
    // Bottom Face
    glNormal3f(0.0, -1.0, 0.0);
    glVertex3f(-0.15f, -0.15f, -0.15f);
    glVertex3f( 0.15f, -0.15f, -0.15f);
    glVertex3f( 0.15f, -0.15f,  0.15f);
    glVertex3f(-0.15f, -0.15f,  0.15f);
    // Right face
    glNormal3f(1.0, 0.0, 0.0);
    glVertex3f( 0.15f, -0.15f, -0.15f);
    glVertex3f( 0.15f,  0.15f, -0.15f);
    glVertex3f( 0.15f,  0.15f,  0.15f);
    glVertex3f( 0.15f, -0.15f,  0.15f);
    // Left Face
    glNormal3f(-1.0, 0.0, 0.0);
    glVertex3f(-0.15f, -0.15f, -0.15f);
    glVertex3f(-0.15f, -0.15f,  0.15f);
    glVertex3f(-0.15f,  0.15f,  0.15f);
    glVertex3f(-0.15f,  0.15f, -0.15f);
  }
  glEnd();
  glPopMatrix();
}

void select_eye_candy_effect(int i)
{
  if (i == MAX_OBJ_3D)	// The current selection
    return;
  
  std::vector<EffectDefinition>::iterator iter = effects.begin() + (i - MAX_OBJ_3D - 1);
  if (current_effect.reference)
  {
    ec_recall_effect(current_effect.reference);
    current_effect.reference = NULL;
  }
  current_effect = *iter;
  effects.erase(iter);
  eye_candy_ready_to_add = 1;
}

void kill_eye_candy_effect()
{
  if (current_effect.reference)
  {
    ec_recall_effect(current_effect.reference);
    current_effect.reference = NULL;
  }
  eye_candy_ready_to_add = 0;
}

int get_eye_candy_count()
{
  return effects.size();
}

void deserialize_eye_candy_effect(particles_io* data)
{
  const unsigned char*const code = (const unsigned char*const)data->file_name + 5;

//  std::cout << "Deserialize" << std::endl;  

  EffectDefinition dest;
  
  unsigned char raw_code[54];
  int i = 0;

  while (i < 18)
  {
    raw_code[i * 3]     = ((code[i * 4 + 0] - ' ') >> 0) | ((code[i * 4 + 1] - ' ') << 6);
    raw_code[i * 3 + 1] = ((code[i * 4 + 1] - ' ') >> 2) | ((code[i * 4 + 2] - ' ') << 4);
    raw_code[i * 3 + 2] = ((code[i * 4 + 2] - ' ') >> 4) | ((code[i * 4 + 3] - ' ') << 2);
    i++;
  }
  
  int bounds_count = raw_code[1];
  if (bounds_count > 19)
    bounds_count = 19;
  for (i = 0; i < bounds_count; i++)
  {
    const float angle = raw_code[i * 2 + 2] * (2 * ec::PI) / 256.0f;
    const float dist = raw_code[i * 2 + 3];
    dest.bounds.elements.push_back(ec::SmoothPolygonElement(angle, dist));
  }

  dest.position.x = data->x_pos;
  dest.position.y = data->y_pos;
  dest.position.z = data->z_pos;
  
  switch (raw_code[0])
  {
    case 0x00:	// Campfire
    {
      const float hue = raw_code[41] / 256.0;
      const float saturation = raw_code[42] / 16.0;
      const float scale = raw_code[43] + raw_code[44] / 256.0;
      dest.effect = 0x00;
      dest.hue = hue;
      dest.saturation = saturation;
      dest.scale = scale;
      dest.reference = ec_create_campfire(dest.position.x, dest.position.y, dest.position.z, hue, saturation, 10, scale);
      break;
    }
    case 0x01:	// Cloud
    {
      const float hue = raw_code[41] / 256.0;
      const float saturation = raw_code[42] / 16.0;
      const float density = raw_code[43] + raw_code[44] / 256.0;
      dest.effect = 0x01;
      dest.hue = hue;
      dest.saturation = saturation;
      dest.density = density;
      dest.reference = ec_create_cloud(dest.position.x, dest.position.y, dest.position.z, hue, saturation, density, (ec_bounds)(&dest.bounds), 10);
      break;
    }
    case 0x02:	// Fireflies
    {
      const float hue = raw_code[41] / 256.0;
      const float saturation = raw_code[42] / 16.0;
      const float density = raw_code[43] + raw_code[44] / 256.0;
      const float scale = raw_code[45] + raw_code[46] / 256.0;
      dest.effect = 0x02;
      dest.hue = hue;
      dest.saturation = saturation;
      dest.scale = scale;
      dest.density = density;
      dest.reference = ec_create_fireflies(dest.position.x, dest.position.y, dest.position.z, hue, saturation, density, scale, (ec_bounds)(&dest.bounds));
      break;
    }
    case 0x03:	// Fountain
    {
      const float hue = raw_code[41] / 256.0;
      const float saturation = raw_code[42] / 16.0;
      const float scale = raw_code[43] + raw_code[44] / 256.0;
      const float base_height = raw_code[45] * 8.0 + raw_code[46] / 32.0;
      const int backlit = raw_code[47];
      dest.effect = 0x03;
      dest.hue = hue;
      dest.saturation = saturation;
      dest.scale = scale;
      dest.base_height = base_height;
      dest.reference = ec_create_fountain(dest.position.x, dest.position.y, dest.position.z, hue, saturation, base_height, backlit, scale, 10);
      break;
    }
    case 0x04:	// Lamp
    {
      const float hue = raw_code[41] / 256.0;
      const float saturation = raw_code[42] / 16.0;
      const float scale = raw_code[43] + raw_code[44] / 256.0;
      dest.effect = 0x04;
      dest.hue = hue;
      dest.saturation = saturation;
      dest.scale = scale;
      dest.reference = ec_create_lamp(dest.position.x, dest.position.y, dest.position.z, hue, saturation, scale, 10);
      break;
    }
    case 0x05:	// Magic protection
    {
      const float hue = raw_code[41] / 256.0;
      const float saturation = raw_code[42] / 16.0;
      const float scale = raw_code[43] + raw_code[44] / 256.0;
      dest.effect = 0x05;
      dest.hue = hue;
      dest.saturation = saturation;
      dest.scale = scale;
      dest.reference = ec_create_ongoing_magic_protection(dest.position.x, dest.position.y, dest.position.z, hue, saturation, 10, scale);
      break;
    }
    case 0x06:	// Shield
    {
      const float hue = raw_code[41] / 256.0;
      const float saturation = raw_code[42] / 16.0;
      const float scale = raw_code[43] + raw_code[44] / 256.0;
      dest.effect = 0x06;
      dest.hue = hue;
      dest.saturation = saturation;
      dest.scale = scale;
      dest.reference = ec_create_ongoing_shield(dest.position.x, dest.position.y, dest.position.z, hue, saturation, 10, scale);
      break;
    }
    case 0x07:	// Magic immunity
    {
      const float hue = raw_code[41] / 256.0;
      const float saturation = raw_code[42] / 16.0;
      const float scale = raw_code[43] + raw_code[44] / 256.0;
      dest.effect = 0x07;
      dest.hue = hue;
      dest.saturation = saturation;
      dest.scale = scale;
      dest.reference = ec_create_ongoing_magic_immunity(dest.position.x, dest.position.y, dest.position.z, hue, saturation, 10, scale);
      break;
    }
    case 0x08:	// Poison
    {
      const float hue = raw_code[41] / 256.0;
      const float saturation = raw_code[42] / 16.0;
      const float scale = raw_code[43] + raw_code[44] / 256.0;
      dest.effect = 0x08;
      dest.hue = hue;
      dest.saturation = saturation;
      dest.scale = scale;
      dest.reference = ec_create_ongoing_poison(dest.position.x, dest.position.y, dest.position.z, hue, saturation, 10, scale);
      break;
    }
    case 0x09:	// Smoke
    {
      const float hue = raw_code[41] / 256.0;
      const float saturation = raw_code[42] / 16.0;
      const float density = raw_code[43] + raw_code[44] / 256.0;
      dest.effect = 0x09;
      dest.hue = hue;
      dest.saturation = saturation;
      dest.density = density;
      dest.reference = ec_create_smoke(dest.position.x, dest.position.y, dest.position.z, hue, saturation, density, 10);
      break;
    }
    case 0x0A:	// Teleporter
    {
      const float hue = raw_code[41] / 256.0;
      const float saturation = raw_code[42] / 16.0;
      const float scale = raw_code[43] + raw_code[44] / 256.0;
      dest.effect = 0x0A;
      dest.hue = hue;
      dest.saturation = saturation;
      dest.scale = scale;
      dest.reference = ec_create_teleporter(dest.position.x, dest.position.y, dest.position.z, hue, saturation, scale, 10);
      break;
    }
    case 0x0B:	// Leaves
    {
      const float hue = raw_code[41] / 256.0;
      const float saturation = raw_code[42] / 16.0;
      const float density = raw_code[43] + raw_code[44] / 256.0;
      const float scale = raw_code[45] + raw_code[46] / 256.0;
      dest.effect = 0x0B;
      dest.hue = hue;
      dest.saturation = saturation;
      dest.scale = scale;
      dest.density = density;
      dest.reference = ec_create_wind_leaves(dest.position.x, dest.position.y, dest.position.z, hue, saturation, scale, density, (ec_bounds)(&dest.bounds), 1.0, 0.0, 0.0);
      break;
    }
    case 0x0C:	// Petals
    {
      const float hue = raw_code[41] / 256.0;
      const float saturation = raw_code[42] / 16.0;
      const float density = raw_code[43] + raw_code[44] / 256.0;
      const float scale = raw_code[45] + raw_code[46] / 256.0;
      dest.effect = 0x0C;
      dest.hue = hue;
      dest.saturation = saturation;
      dest.scale = scale;
      dest.density = density;
      dest.reference = ec_create_wind_petals(dest.position.x, dest.position.y, dest.position.z, hue, saturation, scale, density, (ec_bounds)(&dest.bounds), 1.0, 0.0, 0.0);
      break;
    }
    case 0x0D:	// Waterfall
    {
      const float hue = raw_code[41] / 256.0;
      const float saturation = raw_code[42] / 16.0;
      const float density = raw_code[43] + raw_code[44] / 256.0;
      const float base_height = raw_code[45] * 8.0 + raw_code[46] / 32.0;
      const float angle = raw_code[47] * ec::PI / 128.0;
      dest.effect = 0x0D;
      dest.hue = hue;
      dest.saturation = saturation;
      dest.density = density;
      dest.base_height = base_height;
      dest.angle = angle;
//      dest.reference = ec_create_waterfall(dest.position.x, dest.position.y, dest.position.z, hue, saturation);
      break;
    }
    case 0x0E:	// Bees
    {
      const float hue = raw_code[41] / 256.0;
      const float saturation = raw_code[42] / 16.0;
      const float density = raw_code[43] + raw_code[44] / 256.0;
      const float scale = raw_code[45] + raw_code[46] / 256.0;
      dest.effect = 0x0E;
      dest.hue = hue;
      dest.saturation = saturation;
      dest.scale = scale;
      dest.density = density;
//      dest.reference = ec_create_bees(dest.position.x, dest.position.y, dest.position.z, hue, saturation);
      break;
    }
    case 0x0F:	// Portal
    {
      const float hue = raw_code[41] / 256.0;
      const float saturation = raw_code[42] / 16.0;
      const float scale = raw_code[43] + raw_code[44] / 256.0;
      const float angle = raw_code[45] * ec::PI / 128.0;
      dest.effect = 0x0F;
      dest.hue = hue;
      dest.saturation = saturation;
      dest.scale = scale;
      dest.angle = angle;
//      dest.reference = ec_create_portal(dest.position.x, dest.position.y, dest.position.z, hue, saturation);
      break;
    }
    case 0x10:	// Candle
    {
      const float hue = raw_code[41] / 256.0;
      const float saturation = raw_code[42] / 16.0;
      const float scale = raw_code[43] + raw_code[44] / 256.0;
      dest.effect = 0x10;
      dest.hue = hue;
      dest.saturation = saturation;
      dest.scale = scale;
      dest.reference = ec_create_candle(dest.position.x, dest.position.y, dest.position.z, hue, saturation, scale, 10);
      break;
    }
  }
  effects.push_back(dest);
}

void serialize_eye_candy_effect(int index, particles_io* data)
{
//  std::cout << "Serialize" << std::endl;  

  memset((char*)data, 0, sizeof(particles_io));

  std::string unformatted_data(80, '\0');
  unformatted_data[0] = effects[index].effect;
  unformatted_data[1] = effects[index].bounds.elements.size();
  if (unformatted_data[1] > 19)
    unformatted_data[1] = 19;
  for (int i = 0; i < unformatted_data[1]; i++)
  {
    unformatted_data[i * 2 + 2] = (char)((unsigned char)(effects[index].bounds.elements[i].angle / (2 * ec::PI) * 256.0f));
    unformatted_data[i * 2 + 3] = (char)((unsigned char)(effects[index].bounds.elements[i].radius));
  }
  switch (effects[index].effect)
  {
    case 0:    // Fire
      unformatted_data[41] = (char)((unsigned char)(effects[index].hue * 256.0));
      unformatted_data[42] = (char)((unsigned char)(effects[index].saturation * 16.0));
      unformatted_data[43] = (char)((unsigned char)(effects[index].scale));
      unformatted_data[44] = (char)((unsigned char)((int)(effects[index].scale * 256.0) % 256));
      break;
    case 1:    // Cloud/Fog
      unformatted_data[41] = (char)((unsigned char)(effects[index].hue * 256.0));
      unformatted_data[42] = (char)((unsigned char)(effects[index].saturation * 16.0));
      unformatted_data[43] = (char)((unsigned char)(effects[index].density));
      unformatted_data[44] = (char)((unsigned char)((int)(effects[index].density * 256.0) % 256));
      break;
    case 2:    // Fireflies
      unformatted_data[41] = (char)((unsigned char)(effects[index].hue * 256.0));
      unformatted_data[42] = (char)((unsigned char)(effects[index].saturation * 16.0));
      unformatted_data[43] = (char)((unsigned char)(effects[index].density));
      unformatted_data[44] = (char)((unsigned char)((int)(effects[index].density * 256.0) % 256));
      unformatted_data[45] = (char)((unsigned char)(effects[index].scale));
      unformatted_data[46] = (char)((unsigned char)((int)(effects[index].scale * 256.0) % 256));
      break;
    case 3:    // Fountain
      unformatted_data[41] = (char)((unsigned char)(effects[index].hue * 256.0));
      unformatted_data[42] = (char)((unsigned char)(effects[index].saturation * 16.0));
      unformatted_data[43] = (char)((unsigned char)(effects[index].scale));
      unformatted_data[44] = (char)((unsigned char)((int)(effects[index].scale * 256.0) % 256));
      unformatted_data[47] = 0; 	// Backlit
      break;
    case 4:    // Lamp/Torch
      unformatted_data[41] = (char)((unsigned char)(effects[index].hue * 256.0));
      unformatted_data[42] = (char)((unsigned char)(effects[index].saturation * 16.0));
      unformatted_data[43] = (char)((unsigned char)(effects[index].scale));
      unformatted_data[44] = (char)((unsigned char)((int)(effects[index].scale * 256.0) % 256));
      break;
    case 5:    // Magic Protection
      unformatted_data[41] = (char)((unsigned char)(effects[index].hue * 256.0));
      unformatted_data[42] = (char)((unsigned char)(effects[index].saturation * 16.0));
      unformatted_data[43] = (char)((unsigned char)(effects[index].scale));
      unformatted_data[44] = (char)((unsigned char)((int)(effects[index].scale * 256.0) % 256));
      break;
    case 6:    // Shield
      unformatted_data[41] = (char)((unsigned char)(effects[index].hue * 256.0));
      unformatted_data[42] = (char)((unsigned char)(effects[index].saturation * 16.0));
      unformatted_data[43] = (char)((unsigned char)(effects[index].scale));
      unformatted_data[44] = (char)((unsigned char)((int)(effects[index].scale * 256.0) % 256));
      break;
    case 7:    // Magic Immunity
      unformatted_data[41] = (char)((unsigned char)(effects[index].hue * 256.0));
      unformatted_data[42] = (char)((unsigned char)(effects[index].saturation * 16.0));
      unformatted_data[43] = (char)((unsigned char)(effects[index].scale));
      unformatted_data[44] = (char)((unsigned char)((int)(effects[index].scale * 256.0) % 256));
      break;
    case 8:    // Poison
      unformatted_data[41] = (char)((unsigned char)(effects[index].hue * 256.0));
      unformatted_data[42] = (char)((unsigned char)(effects[index].saturation * 16.0));
      unformatted_data[43] = (char)((unsigned char)(effects[index].scale));
      unformatted_data[44] = (char)((unsigned char)((int)(effects[index].scale * 256.0) % 256));
      break;
    case 9:    // Smoke
      unformatted_data[41] = (char)((unsigned char)(effects[index].hue * 256.0));
      unformatted_data[42] = (char)((unsigned char)(effects[index].saturation * 16.0));
      unformatted_data[43] = (char)((unsigned char)(effects[index].density));
      unformatted_data[44] = (char)((unsigned char)((int)(effects[index].density * 256.0) % 256));
      break;
    case 10:  // Teleporter
      unformatted_data[41] = (char)((unsigned char)(effects[index].hue * 256.0));
      unformatted_data[42] = (char)((unsigned char)(effects[index].saturation * 16.0));
      unformatted_data[43] = (char)((unsigned char)(effects[index].scale));
      unformatted_data[44] = (char)((unsigned char)((int)(effects[index].scale * 256.0) % 256));
      break;
    case 11:  // Leaves
      unformatted_data[41] = (char)((unsigned char)(effects[index].hue * 256.0));
      unformatted_data[42] = (char)((unsigned char)(effects[index].saturation * 16.0));
      unformatted_data[43] = (char)((unsigned char)(effects[index].density));
      unformatted_data[44] = (char)((unsigned char)((int)(effects[index].density * 256.0) % 256));
      unformatted_data[45] = (char)((unsigned char)(effects[index].scale));
      unformatted_data[46] = (char)((unsigned char)((int)(effects[index].scale * 256.0) % 256));
      break;
    case 12:  // Flower Petals
      unformatted_data[41] = (char)((unsigned char)(effects[index].hue * 256.0));
      unformatted_data[42] = (char)((unsigned char)(effects[index].saturation * 16.0));
      unformatted_data[43] = (char)((unsigned char)(effects[index].density));
      unformatted_data[44] = (char)((unsigned char)((int)(effects[index].density * 256.0) % 256));
      unformatted_data[45] = (char)((unsigned char)(effects[index].scale));
      unformatted_data[46] = (char)((unsigned char)((int)(effects[index].scale * 256.0) % 256));
      break;
    case 13:  // Waterfall
      unformatted_data[41] = (char)((unsigned char)(effects[index].hue * 256.0));
      unformatted_data[42] = (char)((unsigned char)(effects[index].saturation * 16.0));
      unformatted_data[43] = (char)((unsigned char)(effects[index].density));
      unformatted_data[44] = (char)((unsigned char)((int)(effects[index].density * 256.0) % 256));
      unformatted_data[45] = (char)((unsigned char)(effects[index].base_height / 8.0));
      unformatted_data[46] = (char)((unsigned char)((int)(effects[index].base_height * 32.0) % 256));
      unformatted_data[47] = (char)((unsigned char)(effects[index].angle * 256.0));
      break;
    case 14:  // Bees
      unformatted_data[41] = (char)((unsigned char)(effects[index].hue * 256.0));
      unformatted_data[42] = (char)((unsigned char)(effects[index].saturation * 16.0));
      unformatted_data[43] = (char)((unsigned char)(effects[index].density));
      unformatted_data[44] = (char)((unsigned char)((int)(effects[index].density * 256.0) % 256));
      unformatted_data[45] = (char)((unsigned char)(effects[index].scale));
      unformatted_data[46] = (char)((unsigned char)((int)(effects[index].scale * 256.0) % 256));
      break;
    case 15:  // Portal
      unformatted_data[41] = (char)((unsigned char)(effects[index].hue * 256.0));
      unformatted_data[42] = (char)((unsigned char)(effects[index].saturation * 16.0));
      unformatted_data[43] = (char)((unsigned char)(effects[index].scale));
      unformatted_data[44] = (char)((unsigned char)((int)(effects[index].scale * 256.0) % 256));
      unformatted_data[45] = (char)((unsigned char)(effects[index].angle * 256.0));
      break;
    case 16:    // Candle
      unformatted_data[41] = (char)((unsigned char)(effects[index].hue * 256.0));
      unformatted_data[42] = (char)((unsigned char)(effects[index].saturation * 16.0));
      unformatted_data[43] = (char)((unsigned char)(effects[index].scale));
      unformatted_data[44] = (char)((unsigned char)((int)(effects[index].scale * 256.0) % 256));
      break;
  }

  std::string data_str = "ec://";
  for (int i = 0; i < 18; i++)
  {
    data_str += ' ' + (char)((((unsigned char)unformatted_data[i * 3 + 0] & 0x3F)));
    data_str += ' ' + (char)((((unsigned char)unformatted_data[i * 3 + 0] & 0xC0) >> 6) | (((unsigned char)unformatted_data[i * 3 + 1] & 0x0F) << 2));
    data_str += ' ' + (char)((((unsigned char)unformatted_data[i * 3 + 1] & 0xF0) >> 4) | (((unsigned char)unformatted_data[i * 3 + 2] & 0x03) << 4));
    data_str += ' ' + (char)((((unsigned char)unformatted_data[i * 3 + 2] & 0xFC) >> 2));
  }

  sprintf(data->file_name, "%s", data_str.c_str());
  data->x_pos = effects[index].position.x;
  data->y_pos = effects[index].position.y;
  data->z_pos = effects[index].position.z;
}

void destroy_all_eye_candy()
{
  for (std::vector<EffectDefinition>::iterator iter = effects.begin(); iter != effects.end(); iter++)
  {
    if (iter->reference)
    {
      ec_recall_effect(iter->reference);
      iter->reference = NULL;
    }
  }
  effects.clear();
}

#endif // EYE_CANDY
