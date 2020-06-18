/*!
 * \file
 * \ingroup     load
 * \brief 	Texture loading and handling
 */
#ifndef __TEXTURES_H__
#define __TEXTURES_H__

#include "cache.h"
#include "platform.h"
#ifndef MAP_EDITOR
 #include "actors.h"
 #ifdef USE_INLINE
  #include "draw_scene.h"
 #endif
#endif
#include "image_loading.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	tt_gui = 0,
	tt_font,
	tt_image,
	tt_mesh,
	tt_atlas
} texture_type;

typedef enum
{
	tft_auto = 0,
	tft_rgba4,
	tft_rgb8,
	tft_r5g6b5,
	tft_rgba8,
	tft_rgb5_a1,
	tft_a8,
	tft_l8,
	tft_la8,
	tft_dxt1,
	tft_dxt3,
	tft_dxt5,
	tft_ati1,
	tft_ati2
} texture_format_type;

/*!
 * we use a separate cache structure to cache textures.
 */
typedef struct
{
	char file_name[128];		/*!< the filename of the texture */
	cache_item_struct *cache_ptr;	/*!< a pointer to the cached item */
	GLuint id;			/*!< the id of the texture */
	Uint32 hash;			/*!< hash value of the name */
	Uint32 size;			/*!< size of the texture */
	texture_type type;		/*!< the texture type, needed for loading and unloading */
	Uint8 load_err;			/*!< if true, we tried to load this texture before and failed */
	Uint8 alpha;			/*!< the texture has an alpha channel */
} texture_cache_t;

/*!
 * \ingroup 	textures
 * \brief 	Load a texture
 *
 * 		Loads a texture for gui use. Mipmaps and anisotropic filters are used,
 *		size reduction can happens and the texture is compressed if supported.
 *		Also the texture cache is used for it.
 * \param file_name The file name of the texture to load.
 * \param type      The intended use of the texture
 * \return The identifier in the cache for the texture
 * \callgraph
 */
Uint32 load_texture_cached(const char* file_name, const texture_type type);

/*!
 * \ingroup 	textures
 * \brief 	Reloads the texture cache
 *
 *      	Reloads all textures in the texture cache.
 *
 * \callgraph
 */
void init_texture_cache();

/*!
 * \ingroup 	textures
 * \brief 	Unloads the texture cache
 *
 *      	Unloads all textures in the texture cache.
 *
 * \callgraph
 */
void unload_texture_cache();

/*!
 * \ingroup 	textures
 * \brief 	Reloads the texture cache
 *
 *      	Reloads all textures in the texture cache.
 *
 * \callgraph
 */
void free_texture_cache();

/*!
 * \ingroup 	textures
 * \brief 	Binds the texture
 *
 *      	Binds the texture using the given OpenGL id.
 *
 * \param	id The OpenGL id
 * \callgraph
 */
void bind_texture_id(const GLuint id);

/*!
 * \ingroup 	textures
 * \brief 	Binds the texture
 *
 *      	Binds the texture using the given texture handle.
 *
 * \param	handle The texture handle
 * \callgraph
 */
void bind_texture(const Uint32 handle);

/*!
 * \ingroup 	textures
 * \brief 	Binds the texture
 *
 *      	Binds the texture using the given texture handle. Does not
 *		check if the texture is already bound. Needed, because at the
 *		moment we don't track the active texture unit.
 *
 * \param	handle The texture handle
 * \callgraph
 */
void bind_texture_unbuffered(const Uint32 handle);

/*!
 * \ingroup 	textures
 * \brief 	Gets the texture alpha
 *
 *      	Returns the texture alpha. Is one if the texture has a usefull
 *		alpha value, else zero.
 *
 * \retval	Uint32 The texture alpha.
 * \callgraph
 */
Uint32 get_texture_alpha(const Uint32 handle);

#ifdef	ELC

/*!
 * we use a separate cache structure to cache textures.
 */
typedef struct
{
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
	char eyes_tex[MAX_FILE_PATH];
	char weapon_tex[MAX_FILE_PATH];
	char shield_tex[MAX_FILE_PATH];
	char helmet_tex[MAX_FILE_PATH];
	char neck_tex[MAX_FILE_PATH];
	char cape_tex[MAX_FILE_PATH];
	char hands_tex_save[MAX_FILE_PATH];
} enhanced_actor_images_t;

typedef enum
{
	tst_unloaded = 0,
	tst_image_loading,
	tst_image_loaded,
	tst_texture_loading,
	tst_texture_loaded
} texture_state_type;

#define MAX_ACTOR_NAME 24

/*!
 * we use a separate cache structure to cache textures.
 */
typedef struct
{
	enhanced_actor_images_t files;	/*!< the files used for the texture */
	char name[MAX_ACTOR_NAME];	/*!< used as an uid.... */
	SDL_mutex* mutex;		/*!< the mutex used for this structure */
	image_t image;			/*!< the image for the texture */
	GLuint id;			/*!< the id of the texture */
	GLuint new_id;			/*!< the id of the new texture */
	Uint32 hash;			/*!< hash value of the files */
	Uint32 used;			/*!< if this is used at the moment? */
	Uint32 access_time;		/*!< last time used */
	texture_state_type state;	/*!< the texture states e.g. loading */
} actor_texture_cache_t;

/*!
 * \ingroup 	textures
 * \brief 	Loads the actors texture
 *
 *      	Loads the actors texture from the information given in the
 *		enhanced actor structure.
 *
 * \param actor A pointer to the enhanced_actor structure
 * \param name  The name of the actor
 * \retval	Uint32 The actor texture handle.
 * \callgraph
 */
Uint32 load_enhanced_actor(const enhanced_actor* actor, const char* name);

/*!
 * \ingroup 	textures
 * \brief 	Binds the actors texture
 *
 *      	Binds the actor texture. This is needed, because actors can
 *		share textures and the loading is threaded.
 *
 * \param   	handle The handle of the texture.
 * \param   	alpha The pointer for the alpha.
 * \retval	Uint32 Returns one if the texture is loaded, zero else.
 * \callgraph
 */
Uint32 bind_actor_texture(const Uint32 handle, char* alpha);

/*!
 * \ingroup 	textures
 * \brief 	Frees the actors texture
 *
 *      	Marks the actor texture as unused.
 *
 * \param   	handle The handle of the texture.
 * \callgraph
 */
void free_actor_texture(const Uint32 handle);

/*!
 * \ingroup 	textures
 * \brief 	Returns if the actor texture is ready
 *
 *      	Returns one if the actor texture is ready to use, zero else.
 *
 * \param   	handle The handle of the texture.
 * \retval	Uint32 Returns one if the texture is ready for use, zero else.
 * \callgraph
 */
Uint32 get_actor_texture_ready(const Uint32 handle);

/*!
 * \ingroup 	textures
 * \brief 	Use the new actor texture
 *
 *      	Use the new actor texture that was loaded in background
 *
 * \param   	handle The handle of the texture.
 * \callgraph
 */
void use_ready_actor_texture(const Uint32 handle);

/*!
 * \ingroup 	textures
 * \brief 	Changes the actors texture
 *
 *      	Changes the actors texture from the information given in the
 *		enhanced actor structure.
 *
 * \param   	handle Handle of the texture to change
 * \param   	actor A pointer to the enhanced_actor structure
 * \callgraph
 */
void change_enhanced_actor(const Uint32 handle, enhanced_actor* actor);

/*!
 * \ingroup 	textures
 * \brief 	Unloads the actor texture cache
 *
 *      	Unloads all actor textures in the actor texture cache.
 *
 * \callgraph
 */
void unload_actor_texture_cache();

#endif	//ELC

#ifdef	DEBUG
void dump_texture_cache();
#endif	/* DEBUG */

#ifdef __cplusplus
} // extern "C"
#endif

#endif
