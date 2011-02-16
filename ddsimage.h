/****************************************************************************
 *            ddsimage.h
 *
 * Author: 2009-2011  Daniel Jungmann <dsj@gmx.net>
 * Copyright: See COPYING file that comes with this distribution
 ****************************************************************************/

#ifndef	UUID_69a52b29_73df_40f9_b176_7bb53e97d035
#define	UUID_69a52b29_73df_40f9_b176_7bb53e97d035

#include "platform.h"
#include "io/elfilewrapper.h"
#ifdef	NEW_TEXTURES
#include "image_loading.h"
#endif	/* NEW_TEXTURES */

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \ingroup 	textures
 * \brief 	Checks if the ID is from a DDS image.
 *
 * 		Checks if the ID is from a DDS image. Must be at least four
 *		bytes.
 * \param   	ID The id to check.
 * \retval Uint32	Returns one if the id is from a dds file, zero else.
 * \callgraph
 */
Uint32 check_dds(const Uint8 *ID);

#ifdef	NEW_TEXTURES
/*!
 * \ingroup 	textures
 * \brief 	Loads a dds image.
 *
 * 		Loads a dds image from file into the struct image. Can
 *		uncompress and unpack (converts the data to RGBA8) the pixels.
 *		Only if needed are the mipmaps loaded also the loading can
 *		start at a different base level (e.g. the first mipmap).
 * \param   	file The file to load from.
 * \param   	uncompress Should the image get uncompressed if it is compressed?
 * \param   	unpack Should the image get converted to RGBA8?
 * \param   	strip_mipmaps Should we strip the mipmaps?
 * \param   	base_level What base level should we use?
 * \param   	image The image struct where we store the loaded data.
 * \retval Uint32	Returns one if everything is ok, zero else.
 * \callgraph
 */
Uint32 load_dds(el_file_ptr file, const Uint32 uncompress, const Uint32 unpack,
	const Uint32 strip_mipmaps, const Uint32 base_level,
	image_t* image);
#else	/* NEW_TEXTURES */
void* load_dds(el_file_ptr file, int *width, int *height);
#endif	/* NEW_TEXTURES */

#ifdef __cplusplus
} // extern "C"
#endif

#endif	/* UUID_69a52b29_73df_40f9_b176_7bb53e97d035 */
