#ifndef __ACTOR_SCRIPTS_H__
#define __ACTOR_SCRIPTS_H__

float unwindAngle_Degrees( float fAngle );
float get_rotation_vector( float fStartAngle, float fEndAngle );
void move_to_next_frame();
void animate_actors();
void next_command();
void destroy_actor(int actor_id);
void destroy_all_actors();
void update_all_actors();
void add_command_to_actor(int actor_id, char command);
void get_actor_damage(int actor_id, Uint8 damage);
void get_actor_heal(int actor_id, Uint8 quantity);
void move_self_forward();
void init_actor_defs();
void you_sit_down();
void you_stand_up();
#endif
