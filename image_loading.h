/****************************************************************************
 *            image_loading.h
 *
 * Author: 2011  Daniel Jungmann <dsj@gmx.net>
 * Copyright: See COPYING file that comes with this distribution
 ****************************************************************************/

#ifndef UUID_41de9803_25ba_428b_8344_727ced850c53
#define UUID_41de9803_25ba_428b_8344_727ced850c53

#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * supported image source formats.
 */
typedef enum
{
	IF_RGBA4 = 0,
	IF_RGB8,
	IF_R5G6B5,
	IF_RGBA8,
	IF_BGR8,
	IF_BGRA8,
	IF_RGB5_A1,
	IF_A8,
	IF_L8,
	IF_LA8,
	IF_DXT1,
	IF_DXT3,
	IF_DXT5,
	IF_ATI1,
	IF_ATI2
} image_format;

#define MAX_IMAGE_MIPMAPS 16

/*!
 * struct for image data.
 */
typedef struct
{
	Uint32 offsets[MAX_IMAGE_MIPMAPS];	/*!< the offsets of the mipmaps relativ to the image pointer */
	Uint32 sizes[MAX_IMAGE_MIPMAPS];	/*!< the sizes of the mipmaps */
	Uint8* image;				/*!< a pointer to the image */
	Uint32 width;				/*!< the width of the image in pixels */
	Uint32 height;				/*!< the height of the image in pixels */
	Uint32 mipmaps;				/*!< the number of mipmaps of the image */
	image_format format;			/*!< the format of the image */
	Uint8 alpha;				/*!< the image has an alpha channel */
} image_struct;

/*!
 * \ingroup 	textures
 * \brief 	Checks for an image
 *
 * 		Replaces the file extension of the file_name with the supported file extensions
 *		and then checks if the file exists. If so, copy the name of the file into buffer
 *		and returns 1, else clears the buffer and returns 0.
 * \param   	file_name The file name of to use.
 * \param   	size The size of the buffer.
 * \param   	buffer The buffer for the found file name.
 * \retval Uint32  	Zero if the file was not found or the buffer is too small, else one.
 * \callgraph
 */
Uint32 check_image_name(const char* file_name, const Uint32 size, char* buffer);

/*!
 * \ingroup 	textures
 * \brief 	Returns the length of a filename without file extension.
 *
 * 		Returns the length of a filename without file extension.
 * \param   	file_name The file name of to use.
 * \retval Uint32	The length of the filename.
 * \callgraph
 */
Uint32 get_file_name_len(const char* file_name);

/*!
 * \ingroup 	textures
 * \brief 	Loads an image.
 *
 * 		Loads an image into the struct image. Can
 *		uncompress and unpack (converts the data to RGBA8) the pixels.
 *		Only if needed are the mipmaps loaded also the loading can
 *		start at a different base level (e.g. the first mipmap).
 * \param   	file_name The file name to use.
 * \param   	uncompress Should the image get uncompressed if it is compressed?
 * \param   	unpack Should the image get converted to RGBA8?
 * \param   	strip_mipmaps Should we strip the mipmaps?
 * \param   	base_level What base level should we use?
 * \param   	compute_alpha Should the alpha value be computes (a = (r + g + b) / 3)?
 * \param   	image The image struct where we store the loaded data.
 * \retval Uint32	Returns one if everything is ok, zero else.
 * \callgraph
 */
Uint32 load_image_data(const char* file_name, const Uint32 uncompress,
	const Uint32 unpack, const Uint32 strip_mipmaps,
	const Uint32 base_level, const Uint32 compute_alpha,
	image_struct* image);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* UUID_41de9803_25ba_428b_8344_727ced850c53 */

