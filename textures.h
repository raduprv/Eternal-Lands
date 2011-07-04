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
#ifdef	NEW_TEXTURES
#include "image_loading.h"
#endif	/* NEW_TEXTURES */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef	NEW_TEXTURES

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
 * \brief 	Loads a texture for non-gui use.
 *
 * 		Loads a texture for gui use. Mipmaps and anisotropic filters are used,
 *		size reduction can happens and the texture is compressed if supported.
 *		Also the texture cache is used for it.
 * \param   	file_name The file name of the texture to load.
 * \retval GLuint  	The texture handle in the cache.
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
 * \param   	actor A pointer to the enhanced_actor structure
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

#else	// NEW_TEXTURES

#define TEXTURE_CACHE_MAX 2000
extern texture_cache_struct texture_cache[TEXTURE_CACHE_MAX]; /*!< global texture cache */
typedef struct
{
	Uint8 *texture;	/*!< a pointer to the texture */
	int	x_size;		/*!< the width of the texture in pixels */
	int	y_size;		/*!< the height of the texture in pixels */
	int	has_alpha;	/*!< was an alpha map applied to this texture? */

}texture_struct;

typedef enum
{
	TF_POINT,
	TF_BILINEAR,
	TF_TRILINEAR,
	TF_ANISOTROPIC,
	TF_ANISOTROPIC_AND_MIPMAPS
} texture_filter;

#ifdef NEW_CURSOR
//Turn on and off texture compression (for color cursors. They lose too much detail if compressed)
void enable_compression();
void disable_compression();

#endif // NEW_CURSOR
typedef enum
{
	TF_RGBA4,
	TF_RGB8,
	TF_R5G6B5,
	TF_RGBA8,
	TF_RGB5_A1,
	TF_L8,
	TF_LA8,
	TF_L16,
	TF_LA16,
	TF_RGB32F,
	TF_RGBA32F,
	TF_L32F,
	TF_LA32F,
	TF_RGB16F,
	TF_RGBA16F,
	TF_L16F,
	TF_LA16F
} texture_format;

/*!
 * \ingroup 	load_bmp
 * \brief 	Loads a color-key from a bmp-file.
 *
 * 		Opens an 8-bit bmp-file and loads the file as a color-key (a=(r+b+g)/3). It will generate the texture as well and return the texture ID.
 *
 * \param   	FileName The filename you wish to load the color-key from.
 * \param	alpha The alpha value
 * \retval GLuint  	The texture ID given as a GLuint.
 * \callgraph
 */
GLuint load_bmp8_color_key(texture_cache_struct * tex_cache_entry, int alpha);

/*!
 * \ingroup 	load_bmp
 * \brief 	Loads a bitmap texture with a fixed alpha.
 *
 *      	Loads an 8-bit bitmap texture and sets the alpha value to a.
 *
 * \param   	FileName The filename of the bitmap.
 * \param   	a The alpha value
 * \retval GLuint  	The texture ID as a GLuint.
 * \callgraph
 */
GLuint load_bmp8_fixed_alpha(texture_cache_struct * tex_cache_entry, Uint8 a);

/*!
 * \ingroup 	load_bmp
 * \brief 	Loads a bitmap texture with a fixed alpha and sets fully transparent color.
 *
 *      	Loads an 8-bit bitmap texture and sets the global alpha value to a and the fully transparent color's alpha to 0.
 *
 * \param   	FileName The filename of the bitmap.
 * \param   	a The alpha value
 * \retval GLuint  	The texture ID as a GLuint.
 * \callgraph
 */
GLuint load_bmp8_fixed_alpha_with_transparent_color(texture_cache_struct * tex_cache_entry, Uint8 a,Uint8 tr,Uint8 tg,Uint8 tb);

/*!
 * \ingroup 	reload_bmp
 * \brief 	Reloads a color-key from a bmp-file.
 *
 * 		Opens an 8-bit bmp-file and loads the file as a color-key (a=(r+b+g)/3). It will generate the texture as well and return the texture ID.
 *
 * \param   	FileName The filename you wish to load the color-key from.
 * \param	alpha The alpha value
 * \param	texture The already loaded texture id
 * \retval GLuint  	The texture ID given as a GLuint.
 * \callgraph
 */
GLuint reload_bmp8_color_key(texture_cache_struct * tex_cache_entry, int alpha, GLuint texture);

/*!
 * \ingroup 	reload_bmp
 * \brief 	Loads a bitmap texture with a fixed alpha.
 *
 *      	Loads an 8-bit bitmap texture and sets the alpha value to a.
 *
 * \param   	FileName The filename of the bitmap.
 * \param   	a The alpha value
 * \param	texture The already loaded texture id
 * \retval GLuint  	The texture ID as a GLuint.
 * \callgraph
 */
GLuint reload_bmp8_fixed_alpha(texture_cache_struct * tex_cache_entry, Uint8 a, GLuint texture);

/*!
 * \ingroup 	cache
 * \brief 	Loads a texture into the cache system.
 *
 *      	Loads a texture into the cache system. If Alpha is 0, it will load the texture using the color-key, otherwise it will load it with a fixed alpha given by a.
 *
 * \param   	file_name The filename of the texture you wish to load.
 * \param   	alpha The alpha-value. If 0, it will load the texture as an 8-bit color-key.
 * \retval int  	The position in the texture_cache array
 * \callgraph
 */
int load_texture_cache (const char * file_name, int alpha);

/*!
 * \ingroup 	cache
 * \brief 	Inserts a texture into the cache system.
 *
 *      	Allocates the cache structure for the texture, but does 
 *      	not actually load the texture into texture memory yet.
 *
 * \param   	file_name The filename of the texture you wish to load.
 * \param   	alpha The alpha-value. If 0, it will load the texture as an 8-bit color-key.
 * \retval int  	The position in the texture_cache array
 * \callgraph
 */
int load_texture_cache_deferred (const char * file_name, int alpha);

/*!
 * \ingroup 	cache
 * \brief 	Gets the texture ID from the texture cache.
 *
 *     		Gets the texture ID from the texture cache. It will reload the texture if it was unloaded.
 *
 * \param   	i The position in the texture cache
 * \retval int  	int The texture ID.
 * \callgraph
 */
int		get_texture_id(int i);

/*!
 * \ingroup 	cache
 * \brief 	Binds the texture ID.
 *
 *      	Binds the texture ID if it is different from the last texture.
 *
 * \param   	texture_id OpenGL's texture ID (not the position in the cache system)
 */
#ifndef	USE_INLINE
void	bind_texture_id(int texture_id);
#else	//USE_INLINE
static __inline__ void	bind_texture_id(int texture_id)
{
	if(last_texture!=texture_id)
	{
		last_texture=texture_id;
		glBindTexture(GL_TEXTURE_2D, texture_id);
	}
}
#endif	//USE_INLINE


/*!
 * \ingroup 	cache
 * \brief 	Gets the texture ID from the texture_cache, then sets it if it is different from the last texture.
 *
 *      	Will get the texture ID of the given position in the texture_cache (and reload it if it has been unloaded). Next it will bind the texture ID if it is different from the last texture.
 *
 * \param   	i The position in the texture_cache.
 * \retval int 	The OpenGL texture ID
 * \sa		bind_texture_id
 * \sa		get_texture_id
 * \callgraph
 */
#ifndef	USE_INLINE
int		get_and_set_texture_id(int i);
#else	//USE_INLINE
//inline version doesn't have range checking!
static __inline__ int get_and_set_texture_id(int i)
{
	int	texture_id;

	// do we need to make a hard load or do we already have it?
	if(!texture_cache[i].texture_id)
	{
		texture_id= get_texture_id(i);
	} else {
		texture_id= texture_cache[i].texture_id;
	}
	bind_texture_id(texture_id);


	return(texture_id);
}
#endif	//USE_INLINE

#ifdef MAP_EDITOR2
/*!
 * \ingroup 	load_bmp
 * \brief	Loads the image into the given img_struct
 *
 * 	Loads a bmp file, optionally sets the image x, y as given by the img_struct. Does not generate the OpenGL texture.
 *
 * \param	FileName The filename of the bmp file
 * \param	img The optional img_struct to set the width and height
 * \retval	char* The raw image
 * \todo	Rewrite, it's stupid to pass an img_struct ptr...
 */
char * load_bmp8_color_key_no_texture_img(char * FileName, img_struct * img);
#endif
                               
#ifndef MAP_EDITOR2
/*!
 * \ingroup 	load_bmp
 * \brief 	Loads a remapped skin for the actor
 *
 *      	The function will load an 8-bit bmp and remap it so it fits the given actor
 *
 * \param   	FileName The filename of the texture you wish to open.
 * \param   	a The fixed alpha value
 * \param   	skin The skin colour
 * \param   	hair The hair colour
 * \param   	shirt The shirt colour
 * \param   	pants The pants colour
 * \param   	boots The boots colour
 * \retval GLuint  	The GL texture ID.
 * \callgraph
 */
GLuint	load_bmp8_remapped_skin(char * FileName, Uint8 a, short skin, short hair, short shirt,
							   short pants, short boots);

#ifdef	ELC
/*!
 * \ingroup 	load_bmp
 * \brief 	Loads the actors texture
 *
 *      	Loads the actors texture from the information given in the enhanced actor structure, with the given alpha.
 *
 * \param   	this_actor A pointer to the enhanced_actor structure
 * \param   	a The fixed alpha
 * \retval int 	int The GL texture ID.
 * \sa		load_bmp8_to_coordinates
 * \todo	Hmm, there might be a better way to do this - it can consume a lot of memory, yet many will actors have approximately the same texture. Currently every actor gets it's own texture loaded.
 * \callgraph
 */
int		load_bmp8_enhanced_actor(enhanced_actor *this_actor, Uint8 a);


#endif	//ELC
#endif  //MAP_EDITOR2
#endif	// NEW_TEXTURES

#ifdef __cplusplus
} // extern "C"
#endif

#endif
