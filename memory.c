/****************************************************************************
 *            memory.c
 *
 * Author: 2011  Daniel Jungmann <el.3d.source@googlemail.com>
 * Copyright: See COPYING file that comes with this distribution
 ****************************************************************************/

#include "memory.h"
#ifdef	USE_SIMD
#include "mm_malloc.h"
#endif	/* SIMD */

void* malloc_aligned(const Uint64 size, const Uint64 alignment)
{
#ifdef	USE_SIMD
	return _mm_malloc(size, alignment);
#else	/* USE_SIMD */
	return malloc(size);
#endif	/* USE_SIMD */
}

void free_aligned(void* memory)
{
#ifdef	USE_SIMD
	_mm_free(memory);
#else	/* USE_SIMD */
	free(memory);
#endif	/* USE_SIMD */
}

