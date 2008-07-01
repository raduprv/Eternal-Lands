/*!
 * \file
 * \ingroup particles
 * \brief data structures and functions for the particle system
 */
#ifndef __PARTICLES_H__
#define __PARTICLES_H__

#include "platform.h"
#include "bbox_tree.h"
#include "e3d.h"
#include "threads.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \name Particle system constants
 */
/*! \{ */
#define MAX_PARTICLE_SYSTEMS 500 /*!< max. number of simultaneous particle systems. */
#define MAX_PARTICLES 2000 /*!< max. number of particles per particle system */
/*! \} */

/*!
 * \name Particle system types
 */
/*! \{ */
#define TELEPORTER_PARTICLE_SYS 0
#define TELEPORT_PARTICLE_SYS 1
#define BAG_PARTICLE_SYS 2
#define BURST_PARTICLE_SYS 3
#define FIRE_PARTICLE_SYS 4
#define FOUNTAIN_PARTICLE_SYS 5
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

	/*!
    	* \name Sounds for this system
	*/
	/*! \{ */
	int sound_nr;
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
	int sound; /*!< If we have a sound this will be the sound object */

	particle particles[MAX_PARTICLES]; /*!< an array of particles for this particle system */

#ifdef CLUSTER_INSIDES
	short cluster;
#endif
}particle_sys;

extern SDL_mutex *particles_list_mutex;	/*!< used for locking between the timer and main threads */
extern particle_sys *particles_list[MAX_PARTICLE_SYSTEMS]; /*!< array of particle systems */
extern int particles_percentage;

/*!
 * \ingroup mutex
 * \name Particle rendering thread synchronization -> moved to particles.c
 */
/*! @{ */
#define	LOCK_PARTICLES_LIST()	CHECK_AND_LOCK_MUTEX(particles_list_mutex)
#define	UNLOCK_PARTICLES_LIST()	CHECK_AND_UNLOCK_MUTEX(particles_list_mutex)
/*! @} */



//INITIALIZATION AND CLEANUP FUNCTIONS

/*!
 * \ingroup loadsave
 * \brief	Checks the particle def list for filename, if not found it will be loaded
 * 
 * 	Checks the particle def list for filename, if not found it will be loaded
 *
 * \param	filename The file name
 * \retval 	A pointer to the loaded particle def
 */
particle_sys_def *load_particle_def(const char *filename);

/*!
 * \ingroup other
 * \brief Destroys all particles of all particles systems and frees up memory used.
 *
 *      Destroys all particles of all particle systems and frees up memory used.
 *
 */
void destroy_all_particles();

/*!
 * \ingroup other
 * \brief Adds a fire
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
 * \brief Removes a fire particle system at the given location
 *
 *      Removes a fire particle system at the location (x,y)
 *
 * \param x_tile	x value of the tile the fire is on
 * \param y_tile	y value of the tile the fire is on
 */
void remove_fire_at_tile (Uint16 x_tile, Uint16 y_tile);

/*!
 * \ingroup other
 * \brief Initializes the particle systems
 *
 *      Initializes the list of particle systems, particle system definitions and textures used.
 *
 * \sa init_stuff
 */
void init_particles ();

/*!
 * \ingroup other
 * \brief Destroys all particle systems and frees up the used memory.
 *
 *      Destroys all particle systems and frees up the used memory.
 *
 * \callgraph
 */
void end_particles ();


//CREATION OF NEW PARTICLES AND SYSTEMS 

/*!
 * \ingroup particles
 * \brief Compute the bounding box for a particle system
 *
 *	Compute an axis aligned bounding box for particles sytem \a system_id and store it in \a bbox.
 *
 * \param bbox      Pointer to an AABBOX in which the result is stored
 * \param system_id Pointer to the particle system
 */
void calc_bounding_box_for_particle_sys (AABBOX* bbox, particle_sys *system_id);

/*!
 * \ingroup particles
 * \brief Adds a new particle system from the file given in file_name at the given position.
 *
 *      Adds a new particle system from the file given in file_name at the position (x_pos,y_pos,z_pos).
 *
 * \param file_name filename of the file that contains the particle systems description.
 * \param x_pos	x coordinate where the particle system should appear
 * \param y_pos	y coordinate where the particle system should appear
 * \param z_pos	z coordinate where the particle system should appear
 * \retval int -1 on error, -2 if the eye candy system handled, or the 
 *             index in particles_list otherwise
 * \callgraph
 */
#ifndef	MAP_EDITOR
int add_particle_sys (const char *file_name, float x_pos, float y_pos, float z_pos, unsigned int dynamic);
#else
int add_particle_sys (const char *file_name, float x_pos, float y_pos, float z_pos);
#endif

#ifdef NEW_SOUND
// Wrapper function for map particles
int add_map_particle_sys (const char *file_name, float x_pos, float y_pos, float z_pos, unsigned int dynamic);
#endif // NEW_SOUND
void add_ec_effect_to_e3d(object3d* e3d);

/*!
 * \ingroup particles
 * \brief Adds a new particle system from the given file file_name at the specified tile position.
 *
 *      Adds a new particle system from the given file file_name at the specified tile position.
 *
 * \param file_name	filename of the file that contains the particly systems definition.
 * \param x_tile	x coordinate of the tile where the particle system should be added
 * \param y_tile	y coordinate of the tile where the particle system should be added
 * \retval int -1 on error, -2 if the eye candy system handled, or the 
 *             index in particles_list otherwise
 * \callgraph
 */
#ifndef	MAP_EDITOR
int add_particle_sys_at_tile (const char *file_name, int x_tile, int y_tile, unsigned int dynamic);
#else
int add_particle_sys_at_tile (const char *file_name, int x_tile, int y_tile);
#endif

// Grum: included here for the map editor
void create_particle (particle_sys *sys, particle *result);
#ifndef	MAP_EDITOR
int create_particle_sys (particle_sys_def *def, float x, float y, float z, unsigned int dynamic);
#else
int create_particle_sys (particle_sys_def *def, float x, float y, float z);
#endif

//RENDERING FUNCTIONS

/*!
 * \ingroup display_utils
 * \brief Displays the particle systems
 *
 *      Displays the particle systems
 *
 * \callgraph
 */
void display_particles();

//UPDATE FUNCTIONS

/*!
 * \ingroup particles
 * \brief Updates all particles
 *
 *      Updates all particles
 *
 * \callgraph
 */
void update_particles();

//MISC HELPER FUNCTIONS
/*!
 * \ingroup misc_utils
 * \brief Adds all teleporters from the given list.
 *
 *      Adds all teleporters from the given list.
 *
 * \param teleport_list     an array of teleporter particle systems to add
 *
 * \callgraph
 */
void add_teleporters_from_list (const Uint8 *teleport_list);

/*!
 * \ingroup loadsave
 * \brief Saves the given particle system definition to a file
 *
 *      Saves the given particle system definition to a file.
 *
 * \param def       the particle system header to save
 * \retval int
 * \callgraph
 */
#ifdef MAP_EDITOR
/*!
 * \brief Set the particle texture as the current texture
 *
 *	Set the particle texture \a i as the next texture to be drawn.
 * 
 * \param i The number of the texture file, \c part_texture field of the 
 *          particle system definition.
 */
void get_and_set_particle_texture_id (int i);

int save_particle_def(particle_sys_def *def);
#elif defined(MAP_EDITOR2)
int save_particle_def(particle_sys_def *def);
#endif

#ifdef MAP_EDITOR2
void draw_text_particle_sys(particle_sys *system_id);
void draw_point_particle_sys(particle_sys *system_id);

void update_bag_part_sys(particle_sys *system_id);
void update_teleport_sys(particle_sys *system_id);
void update_teleporter_sys(particle_sys *system_id);
void update_fire_sys(particle_sys *system_id);
void update_burst_sys(particle_sys *system_id);
void update_fountain_sys(particle_sys *system_id);
#endif // MAP_EDITOR2

extern int use_point_particles; /*!< specifies if we use point particles or not */
extern int enable_blood; /*!< specifies whether or not to use the blood special effect in combat */

#ifdef __cplusplus
} // extern "C"
#endif

#endif // __PARTICLES_H__

