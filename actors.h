#ifndef __actors_H__
#define __actors_H__

extern int yourself;
extern int you_sit;
extern int sit_lock;
extern float name_zoom;

#define HUMAN 1
#define NPC 2
#define COMPUTER_CONTROLLED_HUMAN 3
#define PKABLE_HUMAN 4
#define PKABLE_COMPUTER_CONTROLLED 5

// Max text len to display into bubbles overhead
#define max_current_displayed_text_len	60

typedef struct
{
	float r;
	float g;
	float b;
}glow_color;


//GLOWS
#define GLOW_NONE 0
#define GLOW_FIRE 1
#define GLOW_COLD 2
#define GLOW_THERMAL 3
#define GLOW_MAGIC 4

extern glow_color glow_colors[10];

typedef struct
{
	char legs_fn[32];
	char head_fn[32];
	char torso_fn[32];
	char weapon_fn[32];
	char shield_fn[32];
	char helmet_fn[32];
	char cape_fn[32];

	md2 *legs;
	md2 *head;
	md2 *torso;
	md2 *weapon;
	md2 *shield;
	md2 *helmet;
	md2 *cape;

	char pants_tex[32];
	char boots_tex[32];
	char torso_tex[32];
	char arms_tex[32];
	char hands_tex[32];
	char head_tex[32];
	char hair_tex[32];
	char weapon_tex[32];
	char shield_tex[32];
	char helmet_tex[32];
	char cape_tex[32];

	int weapon_glow;
	int shield_glow;
	int helmet_glow;
	int cape_glow;
	int legs_glow;

	char hands_tex_save[32];
}enhanced_actor;

typedef struct
{
	char model_name[30];
	char skin_name[30];
	int glow;
}body_part;

typedef struct
{
	char model_name[30];
	char skin_name[30];
	char attack_up1[30];
	char attack_down1[30];
	char attack_up2[30];
	char attack_down2[30];
	int glow;
}weapon_part;

typedef struct
{
	char model_name[30];
	char arms_name[30];
	char torso_name[30];
}shirt_part;

typedef struct
{
	char hands_name[30];
	char head_name[30];
}skin_part;

typedef struct
{
	char hair_name[30];
}hair_part;

typedef struct
{
	char boots_name[30];
	int glow;
}boots_part;

typedef struct
{
	char legs_name[30];
	char model_name[30];
	int glow;
}legs_part;

typedef struct
{
	char skin_name[50];
	char file_name[50];
	char walk_frame[20];
	char run_frame[20];
	char die1_frame[20];
	char die2_frame[20];
	char pain1_frame[20];
	char pain2_frame[20];
	char pick_frame[20];
	char drop_frame[20];
	char idle_frame[20];
	char idle_sit_frame[20];
	char harvest_frame[20];
	char attack_cast_frame[20];
	char attack_ranged_frame[20];
	char sit_down_frame[20];
	char stand_up_frame[20];
	char in_combat_frame[20];
	char out_combat_frame[20];
	char combat_idle_frame[20];
	char attack_up_1_frame[20];
	char attack_up_2_frame[20];
	char attack_up_3_frame[20];
	char attack_up_4_frame[20];
	char attack_down_1_frame[20];
	char attack_down_2_frame[20];

	body_part head[5];
	body_part shield[10];
	body_part cape[20];
	body_part helmet[20];
	weapon_part weapon[80];

	shirt_part shirt[18];
	skin_part  skin[4];
	hair_part  hair[6];
	boots_part boots[20];
	legs_part legs[16];

	double walk_speed;
	double run_speed;
	char ghost;
} actor_types;


typedef struct
{
	int actor_id;
	int actor_type;

	double x_pos;
	double y_pos;
	double z_pos;

	int x_tile_pos;
	int y_tile_pos;

	double x_speed;
	double y_speed;
	double z_speed;

	float x_rot;
	float y_rot;
	float z_rot;

	int boots;
	int hair;
	int skin;
	int pants;
	int shirt;

	int is_enhanced_model;
	enhanced_actor *body_parts;

	char remapped_colors;

	char cur_frame[16];

	md2 *model_data;
	int texture_id;
	char skin_name[30];
	char actor_name[30];

	//for movement/animation
	char que[10];
	char last_command;
	char busy;//if the actor is busy executing the current command
	char sitting;
	char fighting;

	double move_x_speed;
	double move_y_speed;
	double move_z_speed;
	int movement_frames_left;
	float rotate_x_speed;
	float rotate_y_speed;
	float rotate_z_speed;
	int rotate_frames_left;
	int after_move_frames_left;

	char moving;
	char rotating;
	char stop_animation;//don't loop trough the current animation (like for die, jump, etc.)
	char stand_idle;
	char sit_idle;
	char dead;
	int damage;
	int damage_ms;
	int cur_health;
	int max_health;
	char ghost;
	int cur_weapon;
	int kind_of_actor;

	char current_displayed_text[max_current_displayed_text_len];
	int current_displayed_text_time_left;
}actor;


extern SDL_mutex *actors_lists_mutex;	//used for locking between the timer and main threads
extern actor *actors_list[1000];
extern int	max_actors;
extern actor_types actors_defs[40];

int get_frame_number(const md2 *model_data, const char *cur_frame);
int add_actor(char * file_name,char * skin_name, char * frame_name,float x_pos,
			  float y_pos, float z_pos, float z_rot, char remappable,
			  short skin_color, short hair_color, short shirt_color,
			  short pants_color, short boots_color, int actor_id);
void draw_actor_banner(actor * actor_id, float offset_z);
void draw_model_halo(md2 *model_data,char *cur_frame, float r, float g, float b);
void draw_model(md2 *model_data,char *cur_frame, int ghost);
void draw_actor(actor * actor_id);
void display_actors();
void add_actor_from_server(char * in_data);
//void draw_interface_body_part(md2 *model_data);
void draw_interface_actor(actor * actor_id,float scale,int x_pos,int y_pos,
						  int z_pos, float x_rot,float y_rot, float z_rot);
actor * add_actor_interface(int actor_type, short skin, short hair,
							short shirt, short pants, short boots, short head);
extern void	init_actors_lists();
#define	lock_actors_lists()	SDL_LockMutex(actors_lists_mutex)
#define	unlock_actors_lists()	SDL_UnlockMutex(actors_lists_mutex)
extern void	end_actors_lists();

void	draw_actor_overtext( actor* actor_ptr );
void	add_displayed_text_to_actor( actor * actor_ptr, const char* text );
actor *	get_actor_ptr_from_id( int actor_id );


#endif
