#ifndef __particles_H__
#define __particles_H__

#define max_particle_systems 200


//system kinds
#define TELEPORTER_PARTICLE_SYS 0
#define TELEPORT_IN_PARTICLE_SYS 1
#define BAG_IN_PARTICLE_SYS 3
#define BAG_OUT_PARTICLE_SYS 4
#define CIRCULAR_BURST 5
#define FIRE_PARTICLE_SYS 6

//particle kinds
#define TELEPORTER_PARTICLE 0
#define SPARKS_PARTICLE 1
#define FIRE_PARTICLE 0

typedef struct
{
	float x;
	float y;
	float z;

	float r;
	float g;
	float b;
	float a;

	float dx;
	float dy;
	float dz;

	Uint8 size;
	Uint8 weight;
	Uint8 ttl;
	Uint8 free;
}particle;


typedef struct
{
	int particle_count;
	int total_particle_no;

	float x_pos;
	float y_pos;
	float z_pos;

	int system_ttl;
	int part_sys_type;
	int part_type;

	float r;
	float g;
	float b;
	float a;

	float z_start;


	particle particles[2000];

}particle_sys;


extern SDL_mutex *particles_list_mutex;	//used for locking between the timer and main threads
extern int particle_textures[8];
extern particle_sys *particles_list[max_particle_systems];

void draw_particle_sys(particle_sys *system_id);
//float particle_rand(float max);
#define particle_rand(max) ((float)max/(float)((rand()%200)-100+0.5))
#define fire_particle_rand(max) ((float)max*2.0*(rand()/(float)RAND_MAX-0.5))

particle_sys *create_particle_sys(float x, float y, float z, int sys_type, int part_type, int num_particles, int ttl);
int add_teleporter(float x_pos, float y_pos, float z_pos);
int add_fire(float x_pos, float y_pos, float z_pos);
int add_teleport_in(int x_pos, int y_pos, float start_r, float start_g, float start_b, float start_a);
int add_bag_in(int x_pos, int y_pos);
int add_bag_out(int x_pos, int y_pos);
void display_particles();
void update_teleporter(particle_sys *system_id);
void update_fire(particle_sys *system_id);
void update_teleport_in(particle_sys *system_id);
void update_bag_in(particle_sys *system_id);
void update_bag_out(particle_sys *system_id);
void update_circular_burst(particle_sys *system_id);
void update_particles();
void add_teleporters_from_list(Uint8 *teleport_list);
void destroy_all_particles();
extern void	init_particles_list();
#define	lock_particles_list()	SDL_LockMutex(particles_list_mutex)
#define	unlock_particles_list()	SDL_UnlockMutex(particles_list_mutex)
extern void	end_particles_list();
int add_circular_burst(int x_pos, int y_pos, int particles_no, float base_color_r, float base_color_g, float base_color_b, float base_alpha);

#endif
