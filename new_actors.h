#ifndef __NEW_ACTORS_H__
#define __NEW_ACTORS_H__

void draw_body_part(md2 *model_data,char *cur_frame, int ghost);
int add_enhanced_actor(enhanced_actor *this_actor,char * frame_name,float x_pos, float y_pos,
					   float z_pos, float z_rot, int actor_id);
void draw_enhanced_actor(actor * actor_id);
void unwear_item_from_actor(int actor_id,Uint8 which_part);
void actor_wear_item(int actor_id,Uint8 which_part, Uint8 which_id);
void add_enhanced_actor_from_server(char * in_data);
void build_glow_color_table();
#endif
