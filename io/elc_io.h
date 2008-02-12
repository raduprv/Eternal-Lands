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

typedef char MAGIC_NUMBER[4];
typedef unsigned char VERSION_NUMBER[4];

/*!
 * the header structure for a file.
 */
typedef struct
{
	MAGIC_NUMBER magic;	/*!< the file magic number */
	VERSION_NUMBER version; /*!< the file version number */
	MD5_DIGEST md5;		/*!< the MD5 digest of the file */
	int header_offset;	/*!< the header offset */
} elc_file_header;

int read_and_check_elc_header(el_file_ptr file, const MAGIC_NUMBER magic, const VERSION_NUMBER version, const char* filename);

#ifdef __cplusplus
} // extern "C"
#endif

#endif	// _ELC_IO_H_
