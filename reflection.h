#ifndef __REFLECTION_H__
#define __REFLECTION_H__

typedef struct
{
	float u;
	float v;
	float z;

}water_vertex;

extern water_vertex noise_array[16*16];
extern int sky_text_1;
extern float water_deepth_offset;
extern int lake_waves_timer;
extern float water_movement_u;
extern float water_movement_v;

float mrandom(float max);
void draw_body_part_reflection(md2 *model_data,char *cur_frame, int ghost);
void draw_actor_reflection(actor * actor_id);
void draw_enhanced_actor_reflection(actor * actor_id);
void draw_3d_reflection(object3d * object_id);
int find_reflection();
int find_local_reflection(int x_pos,int y_pos,int range);
void display_3d_reflection();
void make_lake_water_noise();
void draw_lake_water_tile(float x_pos, float y_pos);
void draw_lake_tiles();
void draw_sky_background();
void draw_dungeon_sky_background();

#endif
