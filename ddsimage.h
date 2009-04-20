/****************************************************************************
 *            ddsimage.h
 *
 * Author: 2009  Daniel Jungmann <dsj@gmx.net>
 * Copyright: See COPYING file that comes with this distribution
 ****************************************************************************/

#ifndef	_DDSIMAGE_H_
#define	_DDSIMAGE_H_

#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

Uint32 check_dds(const Uint8 *ID);
void* load_dds(el_file_ptr file, const char* file_name, int *width, int *height);

#ifdef __cplusplus
} // extern "C"
#endif

#endif	/* _DDSIMAGE_H_ */
