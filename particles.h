#ifndef __particles_H__
#define __particles_H__

#define max_particle_systems 200
#define max_particles 2000

//system kinds
#define TELEPORTER_PARTICLE_SYS 0
#define TELEPORT_PARTICLE_SYS 1
#define BAG_PARTICLE_SYS 2
#define BURST_PARTICLE_SYS 3
#define FIRE_PARTICLE_SYS 4
#define FOUNTAIN_PARTICLE_SYS 5

typedef struct
{
	float x;
	float y;
	float z;

	float r;
	float g;
	float b;
	float a;

	float vx,vy,vz;

	Uint8 free;
}particle;

typedef struct
{
	char file_name[80];
	int part_sys_type;
	GLenum sblend,dblend;
	int total_particle_no;
	int ttl;  // ttl should be negative for systems that don't use it
	int part_texture;
	float part_size;
	int random_func;
	// Starting values
	float minx,miny,minz;
	float maxx,maxy,maxz;
	float constrain_rad_sq;             // <=0 means not used
	float vel_minx,vel_miny,vel_minz;
	float vel_maxx,vel_maxy,vel_maxz;
	float minr,ming,minb,mina;
	float maxr,maxg,maxb,maxa;
	// Update values
	float acc_minx,acc_miny,acc_minz;
	float acc_maxx,acc_maxy,acc_maxz;
	float mindr,mindg,mindb,minda;
	float maxdr,maxdg,maxdb,maxda;
}particle_sys_def;

typedef struct
{
	particle_sys_def *def;

	int particle_count;

	float x_pos;
	float y_pos;
	float z_pos;

	int ttl;

	particle particles[max_particles];

}particle_sys;

extern SDL_mutex *particles_list_mutex;	//used for locking between the timer and main threads
extern int particle_textures[8];
extern particle_sys *particles_list[max_particle_systems];
extern int particles_percentage;

#define particle_random(min,max) (min+(max-min)*(rand()/(float)RAND_MAX))
#define particle_random2(min,max) (min+0.5*(max-min)+0.5*(max-min)/(float)((rand()%200)-100+0.5))

//INITIALIZATION AND CLEANUP FUNCTIONS
#define	lock_particles_list()	SDL_LockMutex(particles_list_mutex)
#define	unlock_particles_list()	SDL_UnlockMutex(particles_list_mutex)
void destroy_all_particles();
void destroy_all_particle_defs();
extern void	init_particles_list();
extern void	end_particles_list();

//CREATION OF NEW PARTICLES AND SYSTEMS 
int create_particle_sys(particle_sys_def *def,float x,float y,float z);
int add_particle_sys(char *file_name,float x_pos,float y_pos,float z_pos);
int add_particle_sys_at_tile(char *file_name,int x_tile,int y_tile);

//RENDERING FUNCTIONS
void display_particles();

//UPDATE FUNCTIONS
void update_teleporter_sys(particle_sys *system_id);
void update_fire_sys(particle_sys *system_id);
void update_teleport_sys(particle_sys *system_id);
void update_bag_part_sys(particle_sys *system_id);
void update_burst_sys(particle_sys *system_id);
void update_fountain_sys(particle_sys *system_id);
void update_particles();

//MISC HELPER FUNCTIONS
#ifdef ELC
void add_teleporters_from_list(Uint8 *teleport_list);
void dump_part_sys_info();
#endif
int save_particle_def(particle_sys_def *def);

#endif
