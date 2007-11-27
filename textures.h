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

#ifdef NEW_LIGHTING
 #include "lights.h"
 #include "e3d.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif


#define TEXTURE_CACHE_MAX 2000
extern texture_cache_struct texture_cache[TEXTURE_CACHE_MAX]; /*!< global texture cache */
typedef struct
{
	Uint8 *texture;	/*!< a pointer to the texture */
	int	x_size;		/*!< the width of the texture in pixels */
	int	y_size;		/*!< the height of the texture in pixels */
	int	has_alpha;	/*!< was an alpha map applied to this texture? */

#ifdef NEW_LIGHTING
	GLfloat   ambient[4];	/*!< The lighting for when the texture is in shadow */
	GLfloat   diffuse[4];	/*!< The lighting for when the object is lit, but not reflecting */
	GLfloat   specular[4];	/*!< The lighting for when the object is reflecting */
	GLfloat   emission[4];	/*!< The lighting for if the object glows */
	GLfloat   shininess;	/*!< The larger it is, the smaller and more pronounced the specular */
	char      material_set;	/*!< If the material has been set. */
#endif
}texture_struct;

typedef enum
{
	TF_POINT,
	TF_BILINEAR,
	TF_TRILINEAR,
	TF_ANISOTROPIC,
	TF_ANISOTROPIC_AND_MIPMAPS
} texture_filter;

#ifdef SKY_FPV_CURSOR
//Turn on and off texture compression (for color cursors. They lose too much detail if compressed)
void enable_compression();
void disable_compression();

#endif /* SKY_FPV_CURSOR */
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

#ifdef NEW_LIGHTING
	if (use_new_lighting)
	{
                GLfloat temp_ambient[4];
                int j;
/*
                for (j = 0; j < 3; j++)
                {
                  texture_cache[i].ambient[j] = 0.2;
                  texture_cache[i].diffuse[j] = 1.0;
                  texture_cache[i].specular[j] = 0.0;
                  texture_cache[i].emission[j] = 0.0;
                }
                texture_cache[i].ambient[3] = 1.0;
                texture_cache[i].diffuse[3] = 1.0;
                texture_cache[i].specular[3] = 1.0;
                texture_cache[i].emission[3] = 1.0;
                texture_cache[i].shininess = 20.0;
*/
/*
		printf("Filename: %s\n", texture_cache[i].file_name);
		printf("Ambient:  %f, %f, %f, %f\n", texture_cache[i].ambient[0], texture_cache[i].ambient[1], texture_cache[i].ambient[2], texture_cache[i].ambient[3]);
		printf("Diffuse:  %f, %f, %f, %f\n", texture_cache[i].diffuse[0], texture_cache[i].diffuse[1], texture_cache[i].diffuse[2], texture_cache[i].diffuse[3]);
		printf("Specular: %f, %f, %f, %f\n", texture_cache[i].specular[0], texture_cache[i].specular[1], texture_cache[i].specular[2], texture_cache[i].specular[3]);
		printf("Emission: %f, %f, %f, %f\n", texture_cache[i].emission[0], texture_cache[i].emission[1], texture_cache[i].emission[2], texture_cache[i].emission[3]);
		printf("Shininess: %f\n\n", texture_cache[i].shininess);
*/
		if (lighting_contrast == 0.5)
		{
			glMaterialfv(GL_FRONT, GL_AMBIENT, texture_cache[i].ambient);
		}
		else if (lighting_contrast < 0.5)
		{
			for (j = 0; j < 3; j++)
				temp_ambient[j] = texture_cache[i].ambient[j] * 2 * lighting_contrast;
			temp_ambient[3] = texture_cache[i].ambient[3];
			glMaterialfv(GL_FRONT, GL_AMBIENT, temp_ambient);
		}
		else
		{
			for (j = 0; j < 3; j++)
				temp_ambient[j] = texture_cache[i].ambient[j] + texture_cache[i].diffuse[j] * (lighting_contrast - 0.5) * 2;
			temp_ambient[3] = texture_cache[i].ambient[3];
			glMaterialfv(GL_FRONT, GL_AMBIENT, temp_ambient);
		}

		glMaterialfv(GL_FRONT, GL_DIFFUSE, texture_cache[i].diffuse);
		glMaterialfv(GL_FRONT, GL_SPECULAR, texture_cache[i].specular);
		glMaterialfv(GL_FRONT, GL_EMISSION, texture_cache[i].emission);
		glMaterialf(GL_FRONT, GL_SHININESS, texture_cache[i].shininess);
	}
#endif

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

#ifdef NEW_LIGHTING
/*!
 * \ingroup 	load_material_metadata
 * \brief 	Loads material properties for the ingame textures and objects.
 * \param   	file_name The filename of the texture you wish to open.
 */
void load_material_metadata(const char * file_name);

/*!
 * \ingroup 	set_model_metadata
 * \brief 	Applies the loaded material file to a given object
 * \param   	obj The 3d object of interest.
 */
object3d *set_model_metadata(object3d * obj);

void set_scene_metadata(const char * file_name);
void set_part_metadata(body_part * part);
void set_weapon_metadata(weapon_part * part);
void set_shirt_metadata(shirt_part * part);
void set_skin_metadata(skin_part * part);
void set_hair_metadata(hair_part * part);
void set_boots_metadata(boots_part * part);
void set_legs_metadata(legs_part * part);
#endif	//NEW_LIGHTING

#endif	//ELC
#endif  //MAP_EDITOR2

#ifdef __cplusplus
} // extern "C"
#endif

#endif
