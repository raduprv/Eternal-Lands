/*!
 * \file
 * \ingroup	display
 * \brief	This file holds information about actors appearance etc. used for displaying the actors.
 */
#ifndef __ACTORS_H__
#define __ACTORS_H__

#include <SDL_mutex.h>
#include "cal_types.h"
#include "client_serv.h"
#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

#define	MAX_FILE_PATH	128	// the max chars allowed int a path/filename for actor textures/masks
#define MAX_ACTOR_DEFS  256
#define MAX_ACTORS      1000    /*!< The maximum number of actors the client can hold */

extern int yourself; 	/*!< This variable holds the actor_id (as the server sees it, not the position in the actors_list) of your character.*/
extern int you_sit; 	/*!< Specifies if you are currently sitting down.*/
extern int sit_lock; 	/*!< The sit_lock variable holds you in a sitting position.*/
extern float name_zoom; /*!< The name_zoom defines how large the text used for drawing the names should be*/
extern int use_alpha_banner;	/*!< Use_alpha_banner defines if an alpha background is drawn behind the name/health banner.*/

/*!
 * \name	Actor types
 * 		Defines the colour of the name.
 */
/*! \{ */
#define HUMAN 1 			/*!< Draw the actors name in white*/
#define NPC 2				/*!< Draw the actors name in blue*/
#define COMPUTER_CONTROLLED_HUMAN 3	/*!< Draw the actors name in white*/
#define PKABLE_HUMAN 4			/*!< Draw the actors name in red*/
#define PKABLE_COMPUTER_CONTROLLED 5	/*!< Draw the actors name in red*/
/*! \} */

/*! Max text len to display into bubbles overhead*/
#define MAX_CURRENT_DISPLAYED_TEXT_LEN	60

/*!
 * \name	Glow colours
 * 		The colours used for giving the items a glowing halo
 */
/*! \{ */
/*! The colours used in the glowing swords (magic, thermal, ice, fire)*/
typedef struct
{
	float r; /*!< Red (0<=r<=1)*/
	float g; /*!< Green (0<=g<=1)*/
	float b; /*!< Blue (0<=b<=1)*/
}glow_color;

//GLOWS
#define GLOW_NONE 0 	/*!< RGB: 0.0, 0.0, 0.0*/
#define GLOW_FIRE 1 	/*!< RGB: 0.5, 0.1, 0.1*/
#define GLOW_COLD 2 	/*!< RGB: 0.1, 0.1, 0.5*/
#define GLOW_THERMAL 3 	/*!< RGB: 0.5, 0.1, 0.5*/
#define GLOW_MAGIC 4	/*!< RGB: 0.5, 0.4, 0.0*/
extern glow_color glow_colors[10]; /*!< Holds the glow colours defined in GLOW_**/
/*! \} */

/*!
 * The near_actor structure holds information about the actors within range. It is filled once every frame.
 */
struct near_actor {
	int actor;//offset in the actors_list
	int select;
	int buffs;	// The buffs on this actor
	char ghost;//If it's a ghost or not
};

extern int no_near_actors;
#ifdef NEW_SOUND
extern int no_near_enhanced_actors;
#endif // NEW_SOUND
extern struct near_actor near_actors[MAX_ACTORS];

/*!
 * The enhanced actor structure holds information about the actors extensions such as if the actor is wearing any armour, weapons etc.
 */
typedef struct
{
	int uniq_id;
	int guild_id;
    int guild_tag_color;

	int legs_meshindex;
	int head_meshindex;
	int torso_meshindex;
	int weapon_meshindex;
	int shield_meshindex;
	int helmet_meshindex;
	int cape_meshindex;

	/*! \name The texture names*/
	/*! \{ */
	char pants_tex[MAX_FILE_PATH];
	char pants_mask[MAX_FILE_PATH];

	char boots_tex[MAX_FILE_PATH];
	char boots_mask[MAX_FILE_PATH];

	char torso_tex[MAX_FILE_PATH];
	char arms_tex[MAX_FILE_PATH];
	char torso_mask[MAX_FILE_PATH];
	char arms_mask[MAX_FILE_PATH];

	char hands_tex[MAX_FILE_PATH];
	char head_tex[MAX_FILE_PATH];
	char hands_mask[MAX_FILE_PATH];
	char head_mask[MAX_FILE_PATH];

	char head_base[MAX_FILE_PATH];
	char body_base[MAX_FILE_PATH];
	char arms_base[MAX_FILE_PATH];
	char legs_base[MAX_FILE_PATH];
	char boots_base[MAX_FILE_PATH];

	char hair_tex[MAX_FILE_PATH];
	char weapon_tex[MAX_FILE_PATH];
	char shield_tex[MAX_FILE_PATH];
	char helmet_tex[MAX_FILE_PATH];
	char cape_tex[MAX_FILE_PATH];
	char hands_tex_save[MAX_FILE_PATH];
	char has_alpha;//is there alpha masking?
		
	/*! \} */

	/*! \name Specifies the glow of each worn item*/
	/*! \{ */
	int weapon_glow;
	int shield_glow;
	int helmet_glow;
	int cape_glow;
	int legs_glow;
	/*! \} */
}enhanced_actor;

/*! Sets the main model type*/
typedef struct
{
	char model_name[MAX_FILE_PATH];
	char skin_name[MAX_FILE_PATH];
	char skin_mask[MAX_FILE_PATH];
	int glow;
	int mesh_index;
}body_part;

/*! Sets the weapon type (including animation frame names)*/
typedef struct
{
	char model_name[MAX_FILE_PATH];
	char skin_name[MAX_FILE_PATH];
	char skin_mask[MAX_FILE_PATH];
	int glow;
	int mesh_index;

	struct cal_anim cal_attack_up_1_frame;
	struct cal_anim cal_attack_up_2_frame;
	struct cal_anim cal_attack_down_1_frame;
    struct cal_anim cal_attack_down_2_frame;
}weapon_part;

/*! Defines the main models looks*/
typedef struct
{
	char model_name[MAX_FILE_PATH];
	char arms_name[MAX_FILE_PATH];
	char torso_name[MAX_FILE_PATH];
	char arms_mask[MAX_FILE_PATH];
	char torso_mask[MAX_FILE_PATH];
	int mesh_index;
}shirt_part;

/*! Sets the models hands and head*/
typedef struct
{
	char hands_name[MAX_FILE_PATH];
	char head_name[MAX_FILE_PATH];
	char arms_name[MAX_FILE_PATH];
	char body_name[MAX_FILE_PATH];
	char legs_name[MAX_FILE_PATH];
	char feet_name[MAX_FILE_PATH];
	int mesh_index;
}skin_part;

/*! Sets the models hair name*/
typedef struct
{
	char hair_name[MAX_FILE_PATH];
	int mesh_index;
}hair_part;

/*! Holds info about the boots */
typedef struct
{
	char boots_name[MAX_FILE_PATH];
	char boots_mask[MAX_FILE_PATH];
	int glow;
	int mesh_index;
}boots_part;

/*! Holds info about the legs type*/
typedef struct
{
	char legs_name[MAX_FILE_PATH];
	char model_name[MAX_FILE_PATH];
	char legs_mask[MAX_FILE_PATH];
	int glow;
	int mesh_index;
}legs_part;

/*! A structure used when loading the actor definitions
 * \sa init_actor_defs*/

typedef struct cal_anim_group
{
	char name[32];
	int count;
	struct cal_anim anim[16];
} cal_animations;

// TODO: would be nice to make these dynamic
#define ACTOR_HEAD_SIZE   10
#define ACTOR_SHIELD_SIZE (SHIELD_NONE+1)
#define ACTOR_CAPE_SIZE   (CAPE_NONE+1)
#define ACTOR_HELMET_SIZE (HELMET_NONE+1)
#define ACTOR_WEAPON_SIZE 80
#define ACTOR_SHIRT_SIZE  100
#define ACTOR_SKIN_SIZE   10
#define ACTOR_HAIR_SIZE   20
#define ACTOR_BOOTS_SIZE  40
#define ACTOR_LEGS_SIZE   50

typedef struct
{
	/*! \name Model data*/
	/*! \{ */
	int actor_type;
	char actor_name[66];
	char skin_name[MAX_FILE_PATH];
	char file_name[256];
	/*! \} */

	float actor_scale;
	float scale;
	float mesh_scale;
	float skel_scale;
	char skeleton_name[MAX_FILE_PATH];

	struct CalCoreModel *coremodel;
	//Animation indexes
	struct cal_anim_group idle_group[16];//16 animation groups
	int group_count;


	struct cal_anim cal_walk_frame;
	struct cal_anim cal_run_frame;
	struct cal_anim cal_die1_frame;
	struct cal_anim cal_die2_frame;
	struct cal_anim cal_pain1_frame;
	struct cal_anim cal_pain2_frame;
	struct cal_anim cal_pick_frame;
	struct cal_anim cal_drop_frame;
	struct cal_anim cal_idle1_frame;
	struct cal_anim cal_idle2_frame;
	struct cal_anim cal_idle_sit_frame;
	struct cal_anim cal_harvest_frame;
	struct cal_anim cal_attack_cast_frame;
	struct cal_anim cal_attack_ranged_frame;
	struct cal_anim cal_sit_down_frame;
	struct cal_anim cal_stand_up_frame;
	struct cal_anim cal_in_combat_frame;
	struct cal_anim cal_out_combat_frame;
	struct cal_anim cal_combat_idle_frame;
	struct cal_anim cal_attack_up_1_frame;
	struct cal_anim cal_attack_up_2_frame;
	struct cal_anim cal_attack_up_3_frame;
	struct cal_anim cal_attack_up_4_frame;
	struct cal_anim cal_attack_down_1_frame;
	struct cal_anim cal_attack_down_2_frame;
	
	/*! \name The different body parts (different head shapes, different armour/weapon shapes etc.)*/
	/*! \{ */
	body_part head[ACTOR_HEAD_SIZE];
	body_part shield[ACTOR_SHIELD_SIZE];
	body_part cape[ACTOR_CAPE_SIZE];
	body_part helmet[ACTOR_HELMET_SIZE];
	weapon_part weapon[ACTOR_WEAPON_SIZE];
	/*! \} */

	/*! \name Clothing*/
	/*! \{ */
	shirt_part shirt[ACTOR_SHIRT_SIZE];
	skin_part  skin[ACTOR_SKIN_SIZE];
	hair_part  hair[ACTOR_HAIR_SIZE];
	boots_part boots[ACTOR_BOOTS_SIZE];
	legs_part legs[ACTOR_LEGS_SIZE];
	/*! \} */

	/*! \name The current actors walk/run speeds*/
	/*! \{ */
	double walk_speed;
	double run_speed;
	char ghost;
	/*! \} */
} actor_types;

/*!
 * This structure holds data that is frequently accessed by both the timer and render thread. On each new frame the data is copied from the actor structure, partly to prevent timing issues, partly to make sure that the same picture is rendered throughout all frames in the scene.
 */
typedef struct 
{
	int have_tmp;		/*!< Specifies if the temporary structure is ready*/
	
	/*! \name Actor positions*/
	/*! \{ */
	double x_pos;		/*!< The actors x position*/
	double y_pos;		/*!< The actors y position*/
	double z_pos;		/*!< The actors z position*/
	/*! \} */
	
	/*! \name Actors tile position*/
	/*! \{ */
	short x_tile_pos;	/*!< The actors x tile position - i.e. used for getting the heightmap from the map file at the actors position*/
	short y_tile_pos;	/*!< The actors y tile position - i.e. used for getting the heightmap from the map file at the actors position*/
	/*! \} */
	
	/*! \name Actors rotation*/
	/*! \{ */
	float x_rot;		/*!< The actors x rotation...*/
	float y_rot;		/*!< The actors y rotation.*/
	float z_rot;		/*!< The actors z position*/
	/*! \} */
} tmp_actor_data;

/*! The main actor structure.*/
#define	MAX_CMD_QUEUE	20
typedef struct
{
	/*! \name Misc.*/
	/*! \{ */
	int actor_id;		/*!< The actor ID from the server*/
	int actor_type;		/*!< Specifies the type of actor (race, sex etc.)*/
	tmp_actor_data tmp;	/*!< The actors temporary data used for rendering*/
	/*! \} */

	struct CalModel *calmodel;
	struct cal_anim cur_anim;
	unsigned int cur_anim_sound_cookie;		/*!< The currently played animation sound*/
	struct cal_anim cur_idle_anims[16];
	int IsOnIdle;
	float anim_time;
	Uint32	last_anim_update;

	/*! \name Actors positions
	 *  \brief Updated in the timer thread
	 */
	/*! \{ */
	double x_pos;		/*!< Specifies the x position of the actor */
	double y_pos;		/*!< Specifies the y position of the actor */
	double z_pos;		/*!< Specifies the z position of the actor */
	float   scale;      /*!< Specidies the custom scaling for the actor model */

	int x_tile_pos;		/*!< Specifies the x tile position - updated in the timer thread*/
	int y_tile_pos;		/*!< Specifies the y tile position - updated in the timer thread \n*/
	/*! \} */

	/*! \name Actor rotation*/
	/*! \{ */
	float x_rot;		/*!< Sets the current x rotation*/
	float y_rot;		/*!< Sets the current y rotation*/
	float z_rot;		/*!< Sets the current z rotation*/
	/*! \} */
	
	float max_z;

	/*! \name Actors worn item IDs*/
	/*! \{ */
	int boots;		/*!< Sets the boots ID (loaded from the actor_defs array)*/
	int hair;		/*!< Sets the hair ID (loaded from the actor_defs array)*/
	int skin;		/*!< Sets the skin ID (loaded from the actor_defs array)*/
	int pants;		/*!< Sets the pants ID (loaded from the actor_defs array)*/
	int shirt;		/*!< Sets the shirt ID (loaded from the actor_defs array)*/
	int cur_weapon;		/*!< Sets the current weapon of the actor*/
	int cur_shield;		/*!< Sets the current shield of the actor*/
	/*! \} */

	/*! \{ */
	int is_enhanced_model;		/*!< Specifies if we have the enhanced_actor structure below*/
	enhanced_actor *body_parts;	/*!< A pointer to the enhanced actor extension (holds information about weapons, helmets etc)*/
	/*! \} */

	/*! \{ */
	char remapped_colors;	/*!< If the actors colours are remapped it will holds the texture in actor->texture_id*/
	GLuint texture_id;			/*!< Sets the texture ID, if the remapped_colors==1 - remember to glDeleteTextures*/
	char skin_name[256];	/*!< Sets the skin name*/
	char actor_name[256];	/*!< Sets the actors name - holds the guild name as well after a special 127+color character*/
	/*! \} */

	/*! \name Command queue and current animations*/
	/*! \{ */
	actor_commands que[MAX_CMD_QUEUE+1];	/*!< Holds the current command queue*/
	char last_command;	/*!< Holds the last command*/
	char busy;			/*!< if the actor is busy executing the current command*/
	char sitting;		/*!< Specifies if the actor is currently sitting*/
	char fighting;		/*!< Specifies if the actor is currently fighting*/
	/*! \} */

	/*!
	 * \name Movement
	 */
	/*! \{ */
	double move_x_speed;	/*!< Sets the current movement speed in the x direction (used for updating the actor in the timer thread)*/
	double move_y_speed;	/*!< Sets the current movement speed in the y direction (used for updating the actor in the timer thread)*/
	double move_z_speed;	/*!< Sets the current movement speed in the z direction (used for updating the actor in the timer thread)*/
	int movement_frames_left;	/*!< Specifies how many movement frames the actor has to do before it goes idle*/
	float rotate_x_speed;	/*!< Sets the x rotation speed (used for updating the actor in the timer thread)*/
	float rotate_y_speed;	/*!< Sets the y rotation speed (used for updating the actor in the timer thread)*/
	float rotate_z_speed;	/*!< Sets the z rotation speed (used for updating the actor in the timer thread)*/
	int rotate_frames_left;	/*!< Specifies how many rotation frames it needs to do*/
	int after_move_frames_left; /*!< When the actor is done moving, it does a small animation before idleing - specifies how many frames it needs to render of that animation*/
	/*! \} */

	/*! \name Misc. animations*/
	/*! \{ */
	char moving;		/*!< Specifies if the actor is currently on the move*/
	char rotating;		/*!< Specifies if the actor is currently rotating*/
	char stop_animation;	/*!< Don't loop trough the current animation (like for die, jump, etc.)*/
	char stand_idle;	/*!< Sets the actor in an idle stand position*/
	char sit_idle;		/*!< Sets the actor in an idle sit position*/
	char dead;		/*!< Used when the actor is dead (render the dead position)*/
	int damage;		/*!< Sets the damage the actor has been given*/
	int damage_ms;		/*!< Defines the remaining time in which the actor damage will be shown*/
	int last_health_loss;	/*!< Defines the time of damage*/
	int cur_health;		/*!< Sets the current health of the actor*/
	int max_health;		/*!< Sets the maximum health of the actor*/
	char ghost;		/*!< Sets the actor type to ghost (Disable lightning, enable blending (GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA))*/
	char has_alpha;		/*!< is alpha blending needed for this actor? */
	int kind_of_actor;	/*!< Defines the kind_of_actor (NPC, HUMAN, COMPUTER_CONTROLLED_HUMAN, PKABLE, PKABLE_COMPUTER_CONTROLLED)*/
	Uint32 buffs;		/*!<Contains the buffs on this actor as bits (currently only invisibility)*/
	/*! \} */

	/*! \name Overhead text (text bubbles)*/
	/*! \{ */
	char current_displayed_text[MAX_CURRENT_DISPLAYED_TEXT_LEN]; /*!< If the text is displayed in a bubble over the actor, this holds the text*/
	int current_displayed_text_time_left;	/*!< Defines the remaining time the overhead text should be displayed*/
	/*! \} */
	
	/*! \name Unused variables*/
	/*! \{ */
	double x_speed;		/*!< Unused?*/
	double y_speed;		/*!< Unused?*/
	double z_speed;		/*!< Unused?*/
	/*! \} */

#ifdef COUNTERS
	int async_fighting;
	int async_x_tile_pos;
	int async_y_tile_pos;
	int async_z_rot;
#endif

#ifdef CLUSTER_INSIDES
	short cluster;
#endif
}actor;

extern SDL_mutex *actors_lists_mutex;	/*!< Used for locking between the timer and main threads*/
extern actor *actors_list[MAX_ACTORS];	/*!< A list holding all of the actors*/
extern actor *your_actor; /*!< A pointer to your own character, if available. Shares a mutex with \see actors_list */
extern int	max_actors;		/*!< The current number of actors in the actors_list + 1*/
extern actor_types actors_defs[MAX_ACTOR_DEFS];	/*!< The actor definitions*/

/*!
 * \ingroup	display_actors
 * \brief	Draws the actors banner (healthbar, name, etc)
 *
 * 		This function is used for drawing the healthbar, the name, the damage, the healthpoints (cur/max) and the text bubbles
 *
 * \param	actor_id Is a pointer to the actor we wish to draw
 * \param	offset_z Is the z offset, found by the current MD2 frames max_z.
 *
 * \callgraph
 */
void draw_actor_banner(actor * actor_id, float offset_z);

/*!
 * \ingroup	display_actors
 * \brief	The main actor loop - draws all actors within range
 * 
 * 		The function draws the actor if it's within a range of 12*12
 *
 * \callgraph
 */
void display_actors(int banner, int reflections);

/*!
 * \ingroup	network_actors
 * \brief	Adds an actor from the in_data
 *
 * 		Is called when the client gets an ADD_NEW_ACTOR command from the server. Parses the data pointed to by in_data, then adds the actor to the actors list
 *
 * \param	in_data The data from the server
 * \param   len The length of the supplied data
 *
 * \callgraph
 */
void add_actor_from_server (const char * in_data, int len);

/*!
 * \ingroup	display_actors
 * \brief	Inititates the actors_list (sets all pointers to NULL).
 *
 * 		Sets all actor pointers in the actors_list to NULL and creates the actors_list mutex.
 *
 * \sa		actors_list
 * \sa		LOCK_ACTORS_LISTS
 */
extern void	init_actors_lists();

#ifdef MUTEX_DEBUG
/*!
 * \ingroup mutex
 * \name Actor list thread synchronization
 */
/*! @{ */
#define	LOCK_ACTORS_LISTS() 	\
	{\
		fprintf(stderr,"Last locked by: %s %s %d\n",__FILE__,__FUNCTION__,__LINE__);\
		if(SDL_LockMutex(actors_lists_mutex)==-1)fprintf(stderr,"We're fucked!! The mutex on %s %s %d was not locked even though we asked it to!\n",__FILE__,__FUNCTION__,__LINE__);\
	}
#define	UNLOCK_ACTORS_LISTS() 	\
	{\
		fprintf(stderr,"Last unlocked by: %s %s %d\n",__FILE__,__FUNCTION__,__LINE__);\
		if(SDL_UnlockMutex(actors_lists_mutex)==-1)fprintf(stderr,"We're fucked!! The mutex on %s %s %d was not unlocked even though we asked it to!\n",__FILE__,__FUNCTION__,__LINE__);\
	}
/*! @} */
#else
/*!
 * \ingroup mutex
 * \name Actor list thread synchronization
 */
/*! @{ */
#define LOCK_ACTORS_LISTS()	SDL_LockMutex(actors_lists_mutex)
#define UNLOCK_ACTORS_LISTS()	SDL_UnlockMutex(actors_lists_mutex)
/*! @} */
#endif

/*!
 * \ingroup	network_text
 * \brief	Adds the text to the actor given by actor_ptr
 *
 * 		Adds text from the actor to overhead text.
 *
 * \param	actor_ptr A pointer to the actor
 * \param	text The text we wish to add to the current_displayed_text buffer in the actors structure.
 */
void	add_displayed_text_to_actor( actor * actor_ptr, const char* text);

/*!
 * \ingroup	misc_utils
 * \brief	Gets a pointer to the actor given by the actor_id
 *
 * 		The function is used for getting a pointer to the actor with the given actor_id (the server-side actor id).
 *
 * \param	actor_id The server-side actor_id - NOT the position in the actors_list
 * \retval actor*	A pointer to the actor with the given ID. If the actor is not found it returns NULL
 * \sa		pf_get_our_actor
 */
actor *	get_actor_ptr_from_id( int actor_id );

/*!
 * \ingroup	display_actors
 * \brief	Gets the max Z of the given actor
 * 
 * 		Gets the max Z of the given actor.
 *
 * \param	act A pointer to the actor
 * \retval	float The max Z
 */
float cal_get_maxz2(actor *act);

void end_actors_lists(void);

int on_the_move (const actor *act);
		

#ifdef __cplusplus
} // extern "C"
#endif

#endif
