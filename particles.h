#ifndef __particles_H__
#define __particles_H__

#define max_particle_systems 1000


//system kinds
#define TELEPORTER_PARTICLE_SYS 0
#define TELEPORT_IN_PARTICLE_SYS 1
#define TELEPORT_OUT_PARTICLE_SYS 2
#define BAG_IN_PARTICLE_SYS 3
#define BAG_OUT_PARTICLE_SYS 4

//particle kinds
#define TELEPORTER_PARTICLE 0
#define SPARKS_PARTICLE 1

typedef struct
{
	float x;
	float y;
	float z;
	float r;
	float g;
	float b;
	float a;
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


	particle particles[1000];

}particle_sys;


extern int particles_text;
extern particle_sys *particles_list[max_particle_systems];

void draw_particle_sys(particle_sys *system_id);
float particle_rand(float max);
int add_teleporter(float x_pos, float y_pos, float z_pos);
int add_teleport_in(int x_pos, int y_pos);
int add_teleport_out(int x_pos, int y_pos);
int add_bag_in(int x_pos, int y_pos);
int add_bag_out(int x_pos, int y_pos);
void display_particles();
void update_teleporter(particle_sys *system_id);
void update_teleport_in(particle_sys *system_id);
void update_bag_in(particle_sys *system_id);
void update_teleport_out(particle_sys *system_id);
void update_bag_out(particle_sys *system_id);
void update_particles();
void add_teleporters_from_list(Uint8 *teleport_list);
void destroy_all_particles();

#endif
