/*!
 * \file
 * \ingroup misc
 * \brief the elc file header data format and supporting functions.
 */
#ifndef	_ELC_IO_H_
#define _ELC_IO_H_

#include "../md5.h"
#include "elfilewrapper.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef char magic_number[4];
typedef unsigned char version_number[4];

/*!
 * the header structure for a file.
 */
typedef struct
{
	magic_number magic;	/*!< the file magic number */
	version_number version; /*!< the file version number */
	MD5_DIGEST md5;		/*!< the MD5 digest of the file */
	int header_offset;	/*!< the header offset */
} elc_file_header;

int read_and_check_elc_header(el_file_ptr file, const magic_number magic, version_number *version, const char* filename);

#ifdef __cplusplus
} // extern "C"
#endif

#endif	// _ELC_IO_H_
