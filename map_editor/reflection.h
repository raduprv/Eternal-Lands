#ifndef	__REFLECTION_H
#define	__REFLECTION_H

typedef struct
{
  float u;
  float v;

}water_vertex;

extern water_vertex noise_array[16*16];
extern int sky_text_1;
extern float water_deepth_offset;
extern int lake_waves_timer;
extern float water_movement_u;
extern float water_movement_v;

#define is_water_tile(i) (!i || (i>230 && i<255))
// The following macro tests if a _water tile_ is reflecting
#define is_reflecting(i) (i<240)

float mrandom(float max);
void draw_3d_reflection(object3d * object_id);
int find_reflection();
int find_local_reflection(int x_pos,int y_pos,int range);
void display_3d_reflection();
void make_lake_water_noise();
void draw_lake_water_tile(float x_pos, float y_pos);
void draw_lake_tiles();
void draw_sky_background();
void draw_dungeon_sky_background();

#endif	//__REFLECTION_H
