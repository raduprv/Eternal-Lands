/*!
 * \file
 * \ingroup particles
 * \brief data structures and functions for the particle system
 */
#ifndef __PARTICLES_H__
#define __PARTICLES_H__

/*!
 * \name Particle system constants
 */
/*! \{ */
#define MAX_PARTICLE_SYSTEMS 500 /*!< max. number of simultaneous particle systems. */
#define MAX_PARTICLES 2000 /*!< max. number of particles per particle system */
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

#ifdef PARTICLE_SYS_SOUND
	/*!
    	* \name Sounds for this system
	*/
	/*! \{ */
	int sound_nr;
	int positional;
	int loop;
	/*! \} */
#endif
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
	int sound; /*!< If we have a sound this will be the sound object */

	particle particles[MAX_PARTICLES]; /*!< an array of particles for this particle system */

}particle_sys;

extern SDL_mutex *particles_list_mutex;	/*!< used for locking between the timer and main threads */
extern int particle_textures[8];
extern particle_sys *particles_list[MAX_PARTICLE_SYSTEMS]; /*!< array of particle systems */
extern int particles_percentage;

/*!
 * \ingroup mutex
 * \name Particle rendering thread synchronization -> moved to particles.c
 */
/*! @{ */
#define	LOCK_PARTICLES_LIST()	SDL_LockMutex(particles_list_mutex)
#define	UNLOCK_PARTICLES_LIST()	SDL_UnlockMutex(particles_list_mutex)
/*! @} */



//INITIALIZATION AND CLEANUP FUNCTIONS

/*!
 * \ingroup other
 * \brief destroys all particles of all particles systems and frees up memory used.
 *
 *      Destroys all particles of all particle systems and frees up memory used.
 *
 */
void destroy_all_particles();

/*!
 * \ingroup other
 * \brief adds a fire
 *
 *      Adds a fire using the information from the server
 *
 * \param kind		the kind of fire (1: small, 2: big)
 * \param x_tile	the x value of the tile
 * \param y_tile	the y value of the tile
 *
 * \callgraph
 */
void add_fire_at_tile (int kind, Uint16 x_tile, Uint16 y_tile);

/*!
 * \ingroup other
 * \brief removes a fire particle system at the given location
 *
 *      Removes a fire particle system at the location (x,y)
 *
 * \param x_tile	x value of the tile the fire is on
 * \param y_tile	y value of the tile the fire is on
 */
void remove_fire_at_tile (Uint16 x_tile, Uint16 y_tile);

/*!
 * \ingroup other
 * \brief initializes the list of particle systems
 *
 *      Initializes the list of particle systems.
 *
 * \sa init_stuff
 */
extern void	init_particles_list();

/*!
 * \ingroup other
 * \brief end_particles_list
 *
 *      end_particles_list()
 *
 * \callgraph
 */
extern void	end_particles_list();


//CREATION OF NEW PARTICLES AND SYSTEMS 

#ifdef PARTICLE_SYS_SOUND
/*!
 * \ingroup particles
 * \brief adds a new particle system from the file given in file_name at the given position.
 *
 *      Adds a new particle system from the file givein in file_name at the position (x_pos,y_pos,z_pos).
 *
 * \param file_name filename of the file that contains the particle systems description.
 * \param x_pos	x coordinate where the particle system should appear
 * \param y_pos	y coordinate where the particle system should appear
 * \param z_pos	z coordinate where the particle system should appear
 * \retval int
 * \callgraph
 */
int add_particle_sys (char *file_name, float x_pos, float y_pos, float z_pos);
#else
/*!
 * \ingroup particles
 * \brief adds a new particle system from the file given in file_name at the given position.
 *
 *      Adds a new particle system from the file givein in file_name at the position (x_pos,y_pos,z_pos).
 *
 * \param file_name filename of the file that contains the particle systems description.
 * \param x_pos	x coordinate where the particle system should appear
 * \param y_pos	y coordinate where the particle system should appear
 * \param z_pos	z coordinate where the particle system should appear
 * \param sound	the number of the sound file to play, or -1 for no sounds
 * \param positional	boolean flag, indicating whether we shall play the sound positional.
 * \param loop		boolean flag, indicating whether we shall play the sound in a loop.
 * \retval int
 * \callgraph
 */
int add_particle_sys (char *file_name, float x_pos, float y_pos, float z_pos, int sound, int positional, int loop);
#endif

#ifdef PARTICLE_SYS_SOUND
/*!
 * \ingroup particles
 * \brief adds a new particle system from the given file file_name at the specified tile position.
 *
 *      Adds a new particle system from the given file file_name at the specified tile position.
 *
 * \param file_name	filename of the file that contains the particly systems definition.
 * \param x_tile	x coordinate of the tile where the particle system should be added
 * \param y_tile	y coordinate of the tile where the particle system should be added
 * \retval int
 * \callgraph
 */
int add_particle_sys_at_tile (char *file_name, int x_tile, int y_tile);
#else
/*!
 * \ingroup particles
 * \brief adds a new particle system from the given file file_name at the specified tile position.
 *
 *      Adds a new particle system from the given file file_name at the specified tile position.
 *
 * \param file_name	filename of the file that contains the particly systems definition.
 * \param x_tile	x coordinate of the tile where the particle system should be added
 * \param y_tile	y coordinate of the tile where the particle system should be added
 * \param sound		the number of the sound file to play, or -1 for no sounds
 * \param positional	boolean flag, indicating whether we shall play the sound positional.
 * \param loop		boolean flag, indicating whether we shall play the sound in a loop.
 * \retval int
 * \callgraph
 */
int add_particle_sys_at_tile (char *file_name, int x_tile, int y_tile, int sound, int positional, int loop);
#endif

// Grum: included here for the map editor
#ifdef PARTICLE_SYS_SOUND
int create_particle_sys (particle_sys_def *def, float x, float y, float z);
#else
int create_particle_sys (particle_sys_def *def, float x, float y, float z, int sound, int positional, int loop);
#endif // def PARTICLE_SYS_SOUND

//RENDERING FUNCTIONS

/*!
 * \ingroup display_utils
 * \brief displays the particle systems
 *
 *      Displays the particle systems
 *
 * \callgraph
 */
void display_particles();

//UPDATE FUNCTIONS

/*!
 * \ingroup particles
 * \brief updates all particles
 *
 *      Updates all particles
 *
 * \callgraph
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
 *
 * \callgraph
 */
void add_teleporters_from_list(Uint8 *teleport_list);

#endif

#ifdef MAP_EDITOR
/*!
 * \ingroup loadsave
 * \brief saves the given particle system definition to a file
 *
 *      Saves the given particle system definition to a file.
 *
 * \param def       the particle system header to save
 * \retval int
 * \callgraph
 */
int save_particle_def(particle_sys_def *def);
#endif

#endif
