/**
 * @file
 * @ingroup misc
 * @brief zip file update functions
 */
#ifndef UUID_f4a0d8d6_2b52_4b9a_ab7d_7eba5a3ae015
#define UUID_f4a0d8d6_2b52_4b9a_ab7d_7eba5a3ae015

#include <SDL.h>
#include "../md5.h"
#include "zip.h"
#include "unzip.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*!
 * \ingroup 	misc
 * \brief 	Copy the current file to zip.
 *
 * 		Copies the current file in the given source zip file to the
 * 		given dest zip file.
 * \param   	source The zip file where the file is taken from.
 * \param   	dest The zip file where the file should be added.
 * \retval Uint32	Returns zero if everything is ok, else one.
 * \callgraph
 */
Uint32 copy_from_zip(unzFile source, zipFile dest);

/*!
 * \ingroup 	misc
 * \brief 	Copy the current file to zip.
 *
 * 		Copies the current file to the given dest zip file.
 * \param   	file_name The file name to use.
 * \param   	size The size of the buffer.
 * \param   	buffer The buffer with the data.
 * \retval Uint32	Returns zero if everything is ok, else one.
 * \callgraph
 */
Uint32 add_to_zip(const char* file_name, const Uint32 size,
	const Uint8* data, zipFile dest, const char* comment);

/*!
 * \ingroup 	misc
 * \brief 	Converts the string to a md5 digest.
 *
 * 		Converts the string to a md5 digest
 * \param   	str The string to use.
 * \param   	digest The md5 digest to update.
 * \retval Uint32	Returns zero if everything is ok, else one.
 * \callgraph
 */
Uint32 convert_string_to_md5_digest(const char* str, MD5_DIGEST digest);

/*!
 * \ingroup 	misc
 * \brief 	Converts the comment string to a md5 digest.
 *
 * 		Converts the comment string to a md5 digest
 * \param   	str The string to use.
 * \param   	digest The md5 digest to update.
 * \retval Uint32	Returns zero if everything is ok, else one.
 * \callgraph
 */
Uint32 convert_comment_string_to_md5_digest(const char* str, MD5_DIGEST digest);

/*!
 * \ingroup 	misc
 * \brief 	Converts the md5 digest to a comment string.
 *
 * 		Converts the md5 digest to a comment string.
 * \param   	digest The md5 digest to use.
 * \param   	size The maximum size fot the string.
 * \param   	str The string to update.
 * \retval Uint32	Returns zero if everything is ok, else one.
 * \callgraph
 */
Uint32 convert_md5_digest_to_comment_string(const MD5_DIGEST digest,
	const Uint32 size, char* str);

Uint32 check_md5_from_zip(unzFile source, const char* file_name,
	const MD5_DIGEST digest);

#ifdef __cplusplus
}
#endif

#endif	/* UUID_f4a0d8d6_2b52_4b9a_ab7d_7eba5a3ae015 */

