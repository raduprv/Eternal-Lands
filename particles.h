/*!
 * \file
 * \brief data structures and functions for the particle system
 * \ingroup particles
 * \todo Finish particles.h documentation
 */
#ifndef __particles_H__
#define __particles_H__

/*!
 * \name Particle system constants
 */
/*! \{ */
#define max_particle_systems 500 /*!< max. number of simultaneous particle systems. */
#define max_particles 2000 /*!< max. number of particles per particle system */
/*! \} */

/*!
 * \name Particle system kinds
 */
/*! \{ */
#define TELEPORTER_PARTICLE_SYS 0 /*!< particle system used by teleportes */
#define TELEPORT_PARTICLE_SYS 1 /*!< particle system used when teleporting by either spell or by using #beam me up */
#define BAG_PARTICLE_SYS 2 /*!< particle system used when bags (dis)appear. */
#define BURST_PARTICLE_SYS 3 /*!< burst particle system */
#define FIRE_PARTICLE_SYS 4 /*!< particle system used by fires/fireplaces */
#define FOUNTAIN_PARTICLE_SYS 5 /*!< particle system used by fountains */
/*! \} */

/*!
 * stores the data of a single particle
 */
typedef struct
{
    /*!
     * \name particle position
     */
    /*! \{ */
	float x;
	float y;
	float z;
    /*! \} */
    
    /*!
     * \name particle colour
     */
    /*! \{ */
	float r;
	float g;
	float b;
	float a;
    /*! \} */

    /*!
     * \name particle velocity coordinates
     */
    /*! \{ */
	float vx,vy,vz;
    /*! \} */

	Uint8 free;
}particle;

/*!
 * the definition part (header) of a particle system.
 */
typedef struct
{
	char file_name[80]; /*!< filename of the particle system */
	int part_sys_type; /*!< type of the particle system. This is one of the values defined in particle system kinds. */
	GLenum sblend,dblend; /*!< blending factors for the src and dst of the particle system */
	int total_particle_no; /*!< current number of particles in the system */
	int ttl;  /*!< ttl should be negative for systems that don't use it */
	int part_texture;
	float part_size;
	int random_func;
    
    /*!
	 * \name Starting values
     */
    /*! \{ */
	float minx,miny,minz;
	float maxx,maxy,maxz;
	float constrain_rad_sq;             /*!< <=0 means not used */
	float vel_minx,vel_miny,vel_minz;
	float vel_maxx,vel_maxy,vel_maxz;
	float minr,ming,minb,mina;
	float maxr,maxg,maxb,maxa;
    /*! \} */
    
    /*!
	 * \name Update values
     */
    /*! \{ */
	float acc_minx,acc_miny,acc_minz;
	float acc_maxx,acc_maxy,acc_maxz;
	float mindr,mindg,mindb,minda;
	float maxdr,maxdg,maxdb,maxda;
	int use_light;
	float lightx,lighty,lightz;
	float lightr,lightg,lightb;
    /*! \} */
}particle_sys_def;

/*!
 * all the data used for a single particle system. \sa particle_sys_def.
 */
typedef struct
{
	particle_sys_def *def; /*!< the header of the particle system */

	int particle_count; /*!< number of current particles in the particle system */

    /*!
     * \name particle system position
     */
    /*! \{ */
	float x_pos;
	float y_pos;
	float z_pos;
    /*! \} */

	int ttl; /*!< indicates how long the particle systems duration should be */

	int light; /*!< If we have a light this will be the position in the lights list */

	particle particles[max_particles]; /*!< an array of particles for this particle system */

}particle_sys;

extern SDL_mutex *particles_list_mutex;	/*!< used for locking between the timer and main threads */
extern int particle_textures[8];
extern particle_sys *particles_list[max_particle_systems]; /*!< array of particle systems */
extern int particles_percentage;

/*!
 * \name randomization of particles
 */
/*! \{ */
#define particle_random(min,max) (min+(max-min)*(rand()/(float)RAND_MAX))
#define particle_random2(min,max) (min+0.5*(max-min)+0.5*(max-min)/(float)((rand()%200)-100+0.5))
/*! \} */


/*!
 * \ingroup mutex
 * \name Particle rendering thread synchronization
 */
/*! @{ */
#define	lock_particles_list()	SDL_LockMutex(particles_list_mutex)
#define	unlock_particles_list()	SDL_UnlockMutex(particles_list_mutex)
/*! @} */



//INITIALIZATION AND CLEANUP FUNCTIONS

/*!
 * \ingroup other
 * \brief destroys all particles of all particles systems and frees up memory used.
 *
 *      Destroys all particles of all particle systems and frees up memory used.
 *
 * \return None
 */
void destroy_all_particles();

/*!
 * \ingroup other
 * \brief destroys all allocated \see particle_def structures
 *
 *      Destroys all allocated \see particle_def structures
 *
 * \return None
 */
void destroy_all_particle_defs();

/*!
 * \ingroup other
 * \brief removes a fire particle system at the given location
 *
 *      Removes a fire particle system at the location (x,y)
 *
 * \param x     x coordinate of the position of the fire to remove
 * \param y     y coordinate of the position of the fire to remove
 * \return None
 */
void remove_fire_at(float x,float y);

/*!
 * \ingroup other
 * \brief initializes the list of particle systems
 *
 *      Initializes the list of particle systems.
 *
 * \return None
 */
extern void	init_particles_list();

/*!
 * \ingroup other
 * \brief end_particles_list
 *
 *      end_particles_list()
 *
 * \return None
 */
extern void	end_particles_list();



//CREATION OF NEW PARTICLES AND SYSTEMS 

/*!
 * \ingroup particles
 * \brief creates a new particle system with the given header at the given position.
 *
 *      Creates a new particle system with the given \see particle_sys_def header at position (x,y,z)
 *
 * \param def   particle system header information
 * \param x     x coordinate of where the particle system should appear
 * \param y     y coordinate of where the particle system should appear
 * \param z     z coordinate of where the particle system should appear
 * \return int
 */
int create_particle_sys(particle_sys_def *def,float x,float y,float z);

/*!
 * \ingroup particles
 * \brief creates a new particle for the given \see particle_sys and returns it in the parameter result
 *
 *      Creates a new particle for the given \see particle_sys particle system and returns it with the parameter result.
 *
 * \param sys       the particle system for which to create a new particle
 * \param result    a pointer to a \see particle struct filled by the function
 * \return None
 */
void create_particle(particle_sys *sys,particle *result);

/*!
 * \ingroup particles
 * \brief adds a new particle system from the file given in file_name at the given position.
 *
 *      Adds a new particle system from the file givein in file_name at the position (x_pos,y_pos,z_pos).
 *
 * \param file_name filename of the file that contains the particle systems description.
 * \param x_pos     x coordinate where the particle system should appear
 * \param y_pos     y coordinate where the particle system should appear
 * \param z_pos     z coordinate where the particle system should appear
 * \return int
 */
int add_particle_sys(char *file_name,float x_pos,float y_pos,float z_pos);

/*!
 * \ingroup particles
 * \brief adds a new particle system from the given file file_name at the specified tile position.
 *
 *      Adds a new particle system from the given file file_name at the specified tile position.
 *
 * \param file_name     filename of the file that contains the particly systems definition.
 * \param x_tile        x coordinate of the tile where the particle system should be added
 * \param y_tile        y coordinate of the tile where the particle system should be added
 * \return int
 */
int add_particle_sys_at_tile(char *file_name,int x_tile,int y_tile);


//RENDERING FUNCTIONS

/*!
 * \ingroup display_utils
 * \brief displays the particle systems
 *
 *      Displays the particle systems
 *
 * \return None
 */
void display_particles();

//UPDATE FUNCTIONS

/*!
 * \ingroup particles
 * \brief updates the given teleporter particle system
 *
 *      Updates the given teleporter particle system
 *
 * \param system_id     the id of the teleporter particle system to update
 * \return None
 */
void update_teleporter_sys(particle_sys *system_id);

/*!
 * \ingroup particles
 * \brief updates the given fire particle system
 *
 *      Updates the given fire particle system
 *
 * \param system_id     the id of the fire particle system to update
 * \return None
 */
void update_fire_sys(particle_sys *system_id);

/*!
 * \ingroup particles
 * \brief updates the given teleport particle system.
 *
 *      Updates the given teleport particle system.
 *
 * \param system_id     the id of the teleport particle system to update
 * \return None
 */
void update_teleport_sys(particle_sys *system_id);

/*!
 * \ingroup particles
 * \brief updates the given bag particle system
 *
 *      Updats the given bag particle system
 *
 * \param system_id     the id of the bag particle system to update
 * \return None
 */
void update_bag_part_sys(particle_sys *system_id);

/*!
 * \ingroup particles
 * \brief updates the given burst particle system
 *
 *      Updates the given burst particle system
 *
 * \param system_id     the id of the burst particle system to update
 * \return None
 */
void update_burst_sys(particle_sys *system_id);

/*!
 * \ingroup particles
 * \brief updates the given fountain particle system
 *
 *      Updates the given fountain particle system
 *
 * \param system_id     the id of the fountain particle system to update.
 * \return None
 */
void update_fountain_sys(particle_sys *system_id);

/*!
 * \ingroup particles
 * \brief updates all particles
 *
 *      Updates all particles
 *
 * \return None
 */
void update_particles();

//MISC HELPER FUNCTIONS
#ifdef ELC
/*!
 * \ingroup misc_utils
 * \brief adds all teleporters from the given list.
 *
 *      Adds all teleporters from the given list.
 *
 * \param teleport_list     an array of teleporter particle systems to add
 * \return None
 */
void add_teleporters_from_list(Uint8 *teleport_list);

/*!
 * \ingroup misc_utils
 * \brief dumps some info about particle systems.
 *
 *      Dumps some info about particle systems.
 *
 * \return None
 */
void dump_part_sys_info();
#endif

/*!
 * \ingroup loadsave
 * \brief saves the given particle system definition to a file
 *
 *      Saves the given particle system definition to a file.
 *
 * \param def       the particle system header to save
 * \return int
 */
int save_particle_def(particle_sys_def *def);

#endif

