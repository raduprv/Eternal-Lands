/****************************************************************************
 *            image_loading.h
 *
 * Author: 2011  Daniel Jungmann <el.3d.source@googlemail.com>
 * Copyright: See COPYING file that comes with this distribution
 ****************************************************************************/

#ifndef UUID_41de9803_25ba_428b_8344_727ced850c53
#define UUID_41de9803_25ba_428b_8344_727ced850c53

#include "platform.h"
#include "io/elfilewrapper.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * supported image source formats.
 */
typedef enum
{
	ift_rgba4 = 0,
	ift_rgb8,
	ift_r5g6b5,
	ift_rgba8,
	ift_bgr8,
	ift_bgra8,
	ift_rgb5_a1,
	ift_a8,
	ift_l8,
	ift_la8,
	ift_dxt1,
	ift_dxt3,
	ift_dxt5,
	ift_ati1,
	ift_ati2
} image_format_type;

/**
 * supported texture compression formats.
 */
typedef enum
{
	tct_s3tc = 0x01,
	tct_3dc = 0x02,
	tct_latc = 0x04
} texture_compression_type;

#define MAX_IMAGE_MIPMAPS 16

/**
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
	image_format_type format;		/*!< the format of the image */
	Uint8 alpha;				/*!< the image has an alpha channel */
} image_t;

/**
 * @ingroup textures
 * @brief Checks for an image
 *
 * Replaces the file extension of the file_name with the supported file extensions
 * and then checks if the file exists. If so, copy the name of the file into buffer
 * and returns 1, else clears the buffer and returns 0.
 * @param file_name The file name of to use.
 * @param size The size of the buffer.
 * @param buffer The buffer for the found file name.
 * @return Zero if the file was not found or the buffer is too small, else one.
 * @callgraph
 */
Uint32 check_image_name(const char* file_name, const Uint32 size, char* buffer);

/**
 * @ingroup textures
 * @brief Returns the length of a filename without file extension.
 *
 * Returns the length of a filename without file extension.
 * @param file_name The file name of to use.
 * @return The length of the filename.
 * @callgraph
 */
Uint32 get_file_name_len(const char* file_name);

/**
 * @ingroup textures
 * @brief Loads an image.
 *
 * Loads an image into the struct image. Can decompress and unpack
 * (converts the data to RGBA8) the pixels. Only if needed are the
 * mipmaps loaded. Also the loading can start at a different base
 * level (e.g. the first mipmap). Tries to find and load an alpha
 * map image. You must use free_image to free the memory.
 * @param file_name The file name to use.
 * @param compression Set of texture compressions that can be used.
 * @param unpack Should the image get converted to RGBA8?
 * @param strip_mipmaps Should we strip the mipmaps?
 * @param base_level What base level should we use?
 * @param image The image struct where we store the loaded data.
 * @return Returns one if everything is ok, zero else.
 * @see texture_compression_type
 * @see free_image
 * @callgraph
 */
Uint32 load_image_data(const char* file_name, const Uint32 compression,
	const Uint32 unpack, const Uint32 strip_mipmaps,
	const Uint32 base_level, image_t* image);

/**
 * @ingroup textures
 * @brief Loads an image.
 *
 * Loads an image into the struct image. Can decompress and unpack
 * (converts the data to RGBA8) the pixels. Only if needed are the
 * mipmaps loaded. Also the loading can start at a different base
 * level (e.g. the first mipmap). The file is closed befor the
 * function returns. You must use free_image to free the memory.
 * @param file The file to use.
 * @param compression Set of texture compressions that can be used.
 * @param unpack Should the image get converted to RGBA8?
 * @param strip_mipmaps Should we strip the mipmaps?
 * @param base_level What base level should we use?
 * @param image The image struct where we store the loaded data.
 * @return Returns one if everything is ok, zero else.
 * @see texture_compression_type
 * @see free_image
 * @callgraph
 */
Uint32 load_image_data_file(el_file_ptr file, const Uint32 compression,
	const Uint32 unpack, const Uint32 strip_mipmaps,
	const Uint32 base_level, image_t* image);

/**
 * @ingroup textures
 * @brief Gets image information.
 *
 * Place image information in the image struct. Only width,
 * height, mipmaps, format and alpha are valid, the rest is filled
 * with zeros.
 * @param file The file to use.
 * @param image The image struct where we store the informations data.
 * @return Returns one if everything is ok, zero else.
 * @callgraph
 */
Uint32 get_image_information(el_file_ptr file, image_t* image);

/**
 * @ingroup textures
 * @brief Frees the image data.
 *
 * Frees the image data. This is needed because some special functions are used.
 * @param image The image struct where the memory to free is.
 * @callgraph
 */
void free_image(image_t* image);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* UUID_41de9803_25ba_428b_8344_727ced850c53 */

