/*!
 * \file
 * \ingroup     load
 * \brief 	Texture loading and handling
 */
#ifndef __TEXTURES_H__
#define __TEXTURES_H__

#ifdef MAP_EDITOR2
 #include "../map_editor2/global.h"
#else
 #include "global.h"
#endif


#define TEXTURE_CACHE_MAX 2000
extern texture_cache_struct texture_cache[TEXTURE_CACHE_MAX]; /*!< global texture cache */

/*!
 * \ingroup 	load_bmp
 * \brief 	Loads a color-key from a bmp-file.
 *
 * 		Opens an 8-bit bmp-file and loads the file as a color-key (a=(r+b+g)/3). It will generate the texture as well and return the texture ID.
 *
 * \param   	FileName The filename you wish to load the color-key from.
 * \retval GLuint  	The texture ID given as a GLuint.
 * \callgraph
 */
GLuint load_bmp8_color_key(char * FileName);

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
GLuint load_bmp8_fixed_alpha(char * FileName, Uint8 a);

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
int load_texture_cache (const char * file_name,unsigned char alpha);

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
int load_texture_cache_deferred (const char * file_name,unsigned char alpha);

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
void	bind_texture_id(int texture_id);

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
int		get_and_set_texture_id(int i);

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

#endif
